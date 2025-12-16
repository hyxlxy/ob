import logging
import os
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import Dict, List, Tuple, Optional
from datetime import datetime

import dotenv

from src.integrations.embedding import generate_response as generate_embedding
from src.integrations.llm import generate_response as generate_llm_response
from src.prompt import QUERY_SYSTEM_PROMPT, QUERY_USER_PROMPT_TEMPLATE
from src.storage.oceanbase import get_or_create_client
from src.util import Answer



logger = logging.getLogger(__name__)

dotenv.load_dotenv()

# 读取环境变量配置（避免引入新模块配置文件）
TABLE_NAME = os.getenv("RAG_TABLE", "rag_documents")
TOP_K = int(os.getenv("TOP_K", "15"))
BM25_CANDIDATES = int(os.getenv("BM25_CANDIDATES", str(TOP_K * 4)))
VEC_CANDIDATES = int(os.getenv("VEC_CANDIDATES", str(TOP_K * 6)))
RRF_K = float(os.getenv("RRF_K", "60"))
MAX_CONTEXT_CHARS = int(os.getenv("MAX_CONTEXT_CHARS", "20000"))
SEARCH_MAX_WORKERS = int(os.getenv("SEARCH_MAX_WORKERS", "2"))


def _rewrite_query(query: str, context_history: str = "") -> str:
    """改进的 Query Rewrite：同义词扩展、短语与日期倾向、上下文工程。
    
    根据讲座内容，Query Rewrite 的目标是：
    1. 补全隐含信息（相对→绝对，模糊→精确）
    2. 术语标准化（口语→专业术语）
    3. 实体消歧与链接
    4. 错误修正
    5. 意图显示化
    6. 快速结构化
    
    针对财报数据优化，包含更多财务相关的同义词。
    """
    q = (query or "").strip()
    if not q:
        return q

    # 扩展的同义词库，包含财报特定的术语
    synonyms = {
        "成立时间": ["成立时间", "成立", "成立于", "创立", "创办", "设立", "注册成立", "成立日期", "成立年份"],
        "主要产品": ["主要产品", "产品", "产品类别", "产品分类", "产品结构", "品类", "主营产品", "产品线"],
        "营收": ["营收", "收入", "营业收入", "销售收入", "总收入", "销售额"],
        "净利润": ["净利", "净利润", "利润", "归母净利润", "净利润率", "纯利"],
        "毛利率": ["毛利率", "毛利", "毛利润", "毛利率"],
        "资产": ["资产", "总资产", "资产周转", "资产规模"],
        "费用": ["费用", "销售费用", "管理费用", "期间费用", "运营费用"],
        "市场": ["市场", "市场份额", "市场地位", "竞争", "市场规模"],
        "增长": ["增长", "增速", "增幅", "提升", "上升", "增加"],
        "同比": ["同比", "同比增长", "同比下降", "年同比"],
        "环比": ["环比", "环比增长", "环比下降", "月环比"],
        "毛利": ["毛利", "毛利润", "毛利率"],
        "现金流": ["现金流", "现金流量", "经营现金流"],
        "ROE": ["ROE", "净资产收益率", "权益回报率"],
        "EPS": ["EPS", "每股收益", "基本每股收益"],
    }

    import re
    
    # 判断是否是时间/年份类问题
    is_time_ask = bool(re.search(r"(时间|日期|哪年|何时|什么时候|年份)", q)) or "成立" in q
    
    # 判断是否是财务数据问题
    is_financial_ask = bool(re.search(r"(收入|利润|费用|资产|毛利|周转|增长|同比|环比|现金流|ROE|EPS)", q))
    
    # 判断是否是对比类问题
    is_compare_ask = bool(re.search(r"(对比|比较|vs|相比|差异|变化|相差)", q))
    
    # 判断是否是原因/归因类问题
    is_cause_ask = bool(re.search(r"(为什么|原因|因为|导致|造成|由于)", q))

    expanded_terms = []
    added = set()

    # 将命中的关键词替换为一个 OR 扩展子句
    for key, syns in synonyms.items():
        if key in q and key not in added:
            added.add(key)
            # 生成短语和近邻短语（~2）
            items = []
            for s in syns:
                if len(s) <= 2:
                    items.append(s)
                else:
                    items.append(f'"{s}"^1.1')
                    items.append(f'"{s}"~2^1.0')
            expanded_terms.append("(" + " OR ".join(items) + ")")

    # 对数字/年份友好：提问中数字强化
    for y in re.findall(r"\d{2,4}", q):
        expanded_terms.append(y)

    # 将原问句也加入（整体短语提升权重）
    if len(q) >= 2:
        expanded_terms.append(f'"{q}"^1.5')
    expanded_terms.append(q)

    # 如果问时间，更强调"年/成立/日期"等词
    if is_time_ask:
        expanded_terms.append("(成立 OR 成立于 OR 创立 OR 注册 OR 日期 OR 年 OR 年份 OR 时间)")
    
    # 如果是财务数据问题，强调数字和百分比
    if is_financial_ask:
        expanded_terms.append("(数据 OR 数值 OR 百分比 OR % OR 万元 OR 亿元 OR 百万 OR 数字)")
    
    # 如果是对比问题，强调对比相关词汇
    if is_compare_ask:
        expanded_terms.append("(对比 OR 比较 OR 差异 OR 相比 OR 相差 OR 变化)")
    
    # 如果是原因问题，强调原因相关词汇
    if is_cause_ask:
        expanded_terms.append("(原因 OR 因素 OR 驱动 OR 影响 OR 导致)")

    # 组合：query_string 会按 OR 解析
    return " ".join(expanded_terms)


def _normalize_time_expressions(q: str) -> str:
    """将相对时间短语归一化（今年/去年/本季度/上季度等）。仅用于 BM25 改写。"""
    if not q:
        return q
    now = datetime.now()
    y = now.year
    m = now.month
    curr_q = (m - 1) // 3 + 1

    def _replace_quarter(text: str, token: str, delta: int) -> str:
        ny = y
        nq = curr_q + delta
        while nq <= 0:
            ny -= 1
            nq += 4
        while nq > 4:
            ny += 1
            nq -= 4
        return text.replace(token, f"{ny} Q{nq}")

    out = q
    out = out.replace("今年", str(y))
    out = out.replace("本年", str(y))
    out = out.replace("去年", str(y - 1))
    out = out.replace("上年", str(y - 1))
    out = out.replace("前年", str(y - 2))
    out = out.replace("本季度", f"{y} Q{curr_q}")
    out = out.replace("本季", f"{y} Q{curr_q}")
    out = _replace_quarter(out, "上季度", -1)
    out = _replace_quarter(out, "上季", -1)
    out = _replace_quarter(out, "下季度", +1)
    out = _replace_quarter(out, "下季", +1)
    return out


def _pre_rewrite(query: str) -> str:
    """轻量预改写：先做时间归一，再交给 _rewrite_query 做同义/意图扩展（仅用于 BM25）。"""
    return _normalize_time_expressions(query)


def _build_fts_query(query: str, fields: Optional[List[str]] = None) -> Dict:
    """构建 FTS 查询，支持多字段搜索和智能查询改写。
    
    根据讲座内容，这里实现了：
    1. 多字段搜索（content + heading + keywords 可配）
    2. 查询预改写（时间归一）+ 扩展（同义、意图词）
    3. 灵活的匹配策略
    """
    ms = os.getenv("FTS_MIN_SHOULD_MATCH", "30%")
    if fields is None:
        env_fields = os.getenv("FTS_FIELDS", "content,heading,keywords")
        fields = [f.strip() for f in env_fields.split(",") if f.strip()]
        if not fields:
            fields = ["content"]
    
    pre_q = _pre_rewrite(query)
    rewritten_query = _rewrite_query(pre_q)
    
    return {
        "bool": {
            "must": [
                {
                    "query_string": {
                        "fields": fields,
                        "type": "best_fields",
                        "query": rewritten_query,
                        "minimum_should_match": ms,
                        "default_operator": "OR",
                    }
                }
            ]
        }
    }


def _rrf_fuse(bm25: List[Dict], dense: List[Dict], top_k: int) -> List[Dict]:
    """使用倒数排序融合(RRF)合并两路结果。
    
    根据讲座内容，实现了多路融合策略：
    - BM25（全文检索）：精确匹配，适合关键词和长尾词
    - Dense（向量检索）：语义相似，适合语义理解
    - RRF：平衡两种方法的优势
    """
    def _key(d: Dict) -> str:
        return d.get("source_id") or f"{d.get('filename','')}#{d.get('page',0)}#{hash(d.get('content',''))}"

    scores: Dict[str, float] = defaultdict(float)
    pos: Dict[str, Tuple[int, Dict]] = {}
    
    # 记录每个文档在两路中的排名
    bm25_ranks: Dict[str, int] = {}
    dense_ranks: Dict[str, int] = {}

    for rank, doc in enumerate(bm25, start=1):
        k = _key(doc)
        scores[k] += 1.0 / (RRF_K + rank)
        bm25_ranks[k] = rank
        if k not in pos:
            pos[k] = (rank, doc)

    for rank, doc in enumerate(dense, start=1):
        k = _key(doc)
        scores[k] += 1.0 / (RRF_K + rank)
        dense_ranks[k] = rank
        if k not in pos:
            pos[k] = (rank, doc)

    # 排序：优先按融合分数，其次按两路排名的最小值（更均衡的结果）
    def sort_key(item):
        k = _key(item[1])
        fusion_score = scores[k]
        min_rank = min(bm25_ranks.get(k, float('inf')), dense_ranks.get(k, float('inf')))
        return (fusion_score, -min_rank)  # 分数高、排名靠前的优先
    
    fused = sorted(pos.values(), key=sort_key, reverse=True)
    return [doc for _, doc in fused[:top_k]]


def _truncate_context(chunks: List[str], limit: int) -> List[str]:
    """
    Truncate context to fit within the limit, prioritizing complete chunks.
    """
    out = []
    total = 0
    for c in chunks:
        c_len = len(c)
        if total + c_len <= limit:
            out.append(c)
            total += c_len
        else:
            # If the chunk is very large and we haven't added anything, or we have space for a significant portion
            remain = limit - total
            if remain > 200:  # Only add partial if we can add a meaningful amount
                out.append(c[:remain] + "...")
                total += remain
            elif not out: # If empty, force add at least one chunk (truncated)
                out.append(c[:limit] + "...")
                total += limit
            break
    return out


def _compute_relevance_score(text: str, query: str, heading: str = "", keywords: str = "") -> float:
    """改进的相关性评分：多维度评估
    
    根据讲座内容实现的多维度评分：
    - 关键词匹配度（正文）- 最重要
    - 数字/数据匹配度（对财报特别重要）
    - 字符级重叠
    - 标题匹配加权（若有 heading 字段）
    - 短语匹配度（连续词组）
    - 语义密度（关键词集中度）
    """
    if not text or not query:
        return 0.0
    
    import re
    
    # 统一小写
    t = text.lower()
    q = query.lower()
    h = (heading or "").lower()
    
    # 1. 基于关键词的匹配度（正文）
    splitter = r"[\s，。、\"\"''：:；;（）()【】、《》,.!?]"
    tokens_q = set([w for w in re.split(splitter, q) if w and len(w) > 1])
    tokens_t = set([w for w in re.split(splitter, t) if w and len(w) > 1])
    
    token_score = (len(tokens_q & tokens_t) / len(tokens_q)) if tokens_q else 0.0
    
    # 2. 数字/数据匹配度（正文）- 财报数据特别重要
    numbers_q = set(re.findall(r"\d+(?:\.\d+)?", q))
    numbers_t = set(re.findall(r"\d+(?:\.\d+)?", t))
    number_score = (len(numbers_q & numbers_t) / len(numbers_q)) if numbers_q else 0.0
    
    # 3. 短语匹配度（连续词组）- 提升精确性
    phrase_score = 0.0
    if len(q) >= 4:
        # 尝试匹配长度为 3-6 的短语
        for length in range(min(6, len(q)), 2, -1):
            for i in range(len(q) - length + 1):
                phrase = q[i:i+length]
                if phrase in t and len(phrase) > 2:
                    phrase_score = max(phrase_score, length / len(q))
                    break
    
    # 4. 标题匹配加权（若有 heading）
    if h:
        tokens_h = set([w for w in re.split(splitter, h) if w and len(w) > 1])
        heading_token_overlap = (len(tokens_q & tokens_h) / len(tokens_q)) if tokens_q else 0.0
        heading_boost = 0.25 * heading_token_overlap
    else:
        heading_boost = 0.0
    
    # 5. 字符级重叠（备用）
    set_t = set(t)
    set_q = set(q)
    inter = len(set_t & set_q)
    union = max(1, len(set_q))
    char_score = inter / union
    
    # 6. 语义密度：关键词在文本中的集中度
    if tokens_q and tokens_t:
        matched_tokens = tokens_q & tokens_t
        # 计算匹配词在文本中的位置分布
        text_length = len(t)
        density_score = len(matched_tokens) / max(1, text_length / 100)  # 归一化
        density_score = min(1.0, density_score)
    else:
        density_score = 0.0
    
    # 综合评分：关键词权重最高，数字次之，短语和标题加权，字符级最低
    base = 0.5 * token_score + 0.25 * number_score + 0.15 * phrase_score + 0.1 * char_score
    final_score = min(1.0, base + heading_boost + 0.05 * density_score)
    
    return final_score


def _select_best_page(
    fused_results: List[Dict],
    question: str
) -> Tuple[str, int]:
    """改进的页面选择逻辑：
    1. 聚合每个页面的相关性得分
    2. 选择得分最高的页面
    3. 如果有多个页面得分相近，优先选择出现最早的
    """
    page_scores: Dict[Tuple[str, int], float] = defaultdict(float)
    page_first_rank: Dict[Tuple[str, int], int] = {}
    
    for rank, result in enumerate(fused_results):
        content = (result.get("content", "") or "").strip()
        filename = result.get("filename", "")
        page = result.get("page", 0)
        
        if not content or not filename:
            continue
        
        key = (filename, page)
        
        # 计算相关性得分（引入标题与关键词加权）
        score = _compute_relevance_score(
            content,
            question,
            heading=(result.get("heading", "") or ""),
            keywords=(result.get("keywords", "") or ""),
        )
        page_scores[key] += score
        
        # 记录该页面首次出现的排名
        if key not in page_first_rank:
            page_first_rank[key] = rank
    
    if not page_scores:
        return ("", 0)
    
    # 选择得分最高的页面，如果得分相同则选择排名最靠前的
    best_key = max(
        page_scores.items(),
        key=lambda x: (x[1], -page_first_rank.get(x[0], float('inf')))
    )[0]
    
    return best_key


def search(question: str) -> Answer:
    """
    Query the RAG system with a question and return an answer.

    Args:
        question: The question to answer

    Returns:
        Answer object containing the question, answer, filename, and page
    """
    answer = Answer(question=question)
    logger.debug(f"Processing query: '{question[:100]}...'")

    try:
        # 1) 生成查询向量
        logger.debug("Generating embedding for question...")
        question_embedding = generate_embedding(question)

        # 2) 初始化客户端
        client = get_or_create_client()

        # 3) 分别执行 BM25 与 向量检索 (并行)
        # Build BM25 query with env-configured fields (fallback to content only if needed)
        env_fields = os.getenv("FTS_FIELDS", "content,heading,keywords")
        fields = [f.strip() for f in env_fields.split(",") if f.strip()]
        if not fields:
            fields = ["content"]
        bm25_request = {
            "query": _build_fts_query(question, fields=fields),
            "from": 0,
            "size": BM25_CANDIDATES,
        }
        vec_request = {
            "knn": {
                "field": "vector",
                "k": VEC_CANDIDATES,
                "num_candidates": max(VEC_CANDIDATES, 128),
                "query_vector": question_embedding,
                "similarity": float(os.getenv("SIMILARITY_THRESHOLD", "0.2")),
            },
            "from": 0,
            "size": VEC_CANDIDATES,
        }

        logger.debug(
            f"Searching: BM25 size={BM25_CANDIDATES}, VEC size={VEC_CANDIDATES}, TOP_K={TOP_K}"
        )
        
        bm25_results = []
        vec_results = []

        with ThreadPoolExecutor(max_workers=SEARCH_MAX_WORKERS) as executor:
            future_bm25 = executor.submit(client.search, index=TABLE_NAME, body=bm25_request)
            future_vec = executor.submit(client.search, index=TABLE_NAME, body=vec_request)
            
            try:
                bm25_results = future_bm25.result() or []
            except Exception as e:
                logger.error(f"BM25 search failed with fields={fields}: {e}")
                # Fallback to content-only BM25
                try:
                    bm25_request_fallback = {
                        "query": _build_fts_query(question, fields=["content"]),
                        "from": 0,
                        "size": BM25_CANDIDATES,
                    }
                    bm25_results = client.search(index=TABLE_NAME, body=bm25_request_fallback) or []
                    logger.info("BM25 fallback to content-only succeeded")
                except Exception as e2:
                    logger.error(f"BM25 fallback failed: {e2}")

            try:
                vec_results = future_vec.result() or []
            except Exception as e:
                logger.error(f"Vector search failed: {e}")

        if not bm25_results and not vec_results:
            logger.warning(f"No search results found for question: '{question[:50]}...'")
            answer.answer = "抱歉，我没有找到相关的信息来回答这个问题。"
            return answer

        # 4) RRF 融合并去重
        fused = _rrf_fuse(bm25_results, vec_results, TOP_K)
        if not fused:
            logger.warning("RRF 融合结果为空，回退使用BM25或向量结果")
            fused = (bm25_results or vec_results)[:TOP_K]

        # 5) 改进的页面选择逻辑
        best_filename, best_page = _select_best_page(fused, question)
        
        if not best_filename:
            logger.warning("No valid contexts extracted after fusion")
            answer.answer = "抱歉，我没有找到相关的信息来回答这个问题。"
            return answer
        
        answer.filename = best_filename
        answer.page = best_page

        # 6) 组装上下文（优先主引用页），并控制总长度
        contexts: List[str] = []
        meta: List[Tuple[str, int]] = []
        seen = set()
        
        # 先放主引用页的内容
        for r in fused:
            content = (r.get("content", "") or "").strip()
            fn = r.get("filename", "")
            pg = r.get("page", 0)
            key = (fn, pg)
            
            if key == (best_filename, best_page) and content and key not in seen:
                contexts.append(content)
                meta.append(key)
                seen.add(key)
        
        # 再补其他页的内容
        for r in fused:
            content = (r.get("content", "") or "").strip()
            fn = r.get("filename", "")
            pg = r.get("page", 0)
            key = (fn, pg)
            
            if content and key not in seen:
                contexts.append(content)
                meta.append(key)
                seen.add(key)
        
        # 裁剪
        contexts = _truncate_context(contexts, MAX_CONTEXT_CHARS)

        # 将片段与其来源显式拼装，帮助模型生成正确引用
        contexts_with_citation: List[str] = []
        for i, ctx in enumerate(contexts):
            if i < len(meta):
                fn, pg = meta[i]
                prefix = f"【来源】({fn}, {pg})\n"
                contexts_with_citation.append(prefix + ctx)
            else:
                contexts_with_citation.append(ctx)
        
        context_text = "\n\n".join(contexts_with_citation)

        # 7) 生成答案
        logger.debug("Generating answer using LLM model ...")
        prompt = QUERY_USER_PROMPT_TEMPLATE.format(
            context_text=context_text,
            question=question,
        )

        messages = [
            {"role": "system", "content": QUERY_SYSTEM_PROMPT},
            {"role": "user", "content": prompt},
        ]

        ans_text = generate_llm_response(messages)
        
        # 改进的引用处理：确保答案包含明确的来源引用
        import re as _re
        
        # 检查是否已包含引用格式 (文件名, 页码)
        has_citation = bool(_re.search(r"\([^,，]+,\s*\d+\)", ans_text))
        
        if not has_citation:
            # 若未包含引用，则附加主要来源
            # 格式：(文件名, 页码)
            ans_text = ans_text.rstrip() + f"\n\n参考来源：({answer.filename}, {answer.page})"
        
        # 确保答案不为空
        if not ans_text or not ans_text.strip():
            ans_text = "无法从提供的文档中找到答案。"
        
        answer.answer = ans_text

        logger.debug(
            f"Query completed: question='{question[:50]}...', filename='{answer.filename}', page={answer.page}"
        )

    except Exception as e:
        logger.error(f"Error in query: {e}", exc_info=True)
        answer.answer = f"查询过程中发生错误：{str(e)}"

    return answer
