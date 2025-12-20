import json
import logging
import os
import re
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import Dict, List, Tuple, Optional

import dotenv

from src.integrations.embedding import generate_response as generate_embedding
from src.integrations.llm import generate_response as generate_llm_response
from src.prompt import QUERY_SYSTEM_PROMPT, QUERY_USER_PROMPT_TEMPLATE
from src.storage.oceanbase import get_or_create_client
from src.util import Answer

logger = logging.getLogger(__name__)

dotenv.load_dotenv()

# OceanBase connection parameters
OCEANBASE_URI = os.getenv("OCEANBASE_URI")
OCEANBASE_USER = os.getenv("OCEANBASE_USER")
OCEANBASE_PASSWORD = os.getenv("OCEANBASE_PASSWORD")
OCEANBASE_DBNAME = os.getenv("OCEANBASE_DBNAME")

# Table name for storing document chunks
TABLE_NAME = "rag_documents"

# Retrieval parameters (configurable via env)
TOP_K = int(os.getenv("TOP_K", "30"))  # 增加召回数量到30
BM25_CANDIDATES = int(os.getenv("BM25_CANDIDATES", "150"))  # 增加BM25候选
VEC_CANDIDATES = int(os.getenv("VEC_CANDIDATES", "180"))    # 增加向量候选
RRF_K = float(os.getenv("RRF_K", "60"))  # RRF 常数项
MAX_CONTEXT_CHARS = int(os.getenv("MAX_CONTEXT_CHARS", "25000"))  # 增加上下文长度


def _expand_query(query: str) -> str:
    """扩展查询，添加同义词和相关术语"""
    # 财务术语映射
    expansions = {
        "营业收入": "营业收入 OR 营收 OR 收入",
        "净利润": "净利润 OR 归母净利润 OR 利润",
        "毛利率": "毛利率 OR 毛利",
        "产能": "产能 OR 产量 OR 生产能力",
        "产能利用率": "产能利用率 OR 产销率",
        "客户": "客户 OR 大客户 OR 经销商",
        "产品": "产品 OR 品类 OR SKU",
        "费用率": "费用率 OR 销售费用率 OR 管理费用率",
        "成立时间": "成立时间 OR 成立 OR 创立",
        "定价": "定价 OR 定价方法 OR 定价策略",
    }
    
    expanded = query
    for term, expansion in expansions.items():
        if term in query:
            expanded = expanded.replace(term, f"({expansion})")
    
    # 如果查询包含年份，确保也搜索相关年份
    import re
    year_match = re.search(r'(20\d{2})', query)
    if year_match:
        year = year_match.group(1)
        # 添加年份的不同表达形式
        if year not in expanded:
            expanded += f" {year}"
    
    return expanded


def _build_fts_query(query: str) -> Dict:
    # 扩展查询以提高召回
    expanded_query = _expand_query(query)
    return {
        "bool": {
            "should": [
                {
                    "query_string": {
                        "fields": ["content"],
                        "type": "best_fields",
                        "query": expanded_query,
                        "minimum_should_match": "30%",
                        "boost": 2.0,  # 精确匹配权重更高
                    }
                },
                {
                    "query_string": {
                        "fields": ["content"],
                        "type": "phrase",
                        "query": query,
                        "boost": 3.0,  # 短语匹配权重最高
                    }
                }
            ],
            "minimum_should_match": 1,
        }
    }


def _rrf_fuse(bm25: List[Dict], dense: List[Dict], top_k: int) -> List[Dict]:
    """使用倒数排序融合(RRF)合并两路结果。"""
    def _key(d: Dict) -> str:
        return d.get("source_id") or f"{d.get('filename','')}#{d.get('page',0)}#{hash(d.get('content',''))}"

    scores: Dict[str, float] = defaultdict(float)
    pos: Dict[str, Tuple[int, Dict]] = {}

    for rank, doc in enumerate(bm25, start=1):
        k = _key(doc)
        scores[k] += 1.0 / (RRF_K + rank)
        if k not in pos:
            pos[k] = (rank, doc)

    for rank, doc in enumerate(dense, start=1):
        k = _key(doc)
        scores[k] += 1.0 / (RRF_K + rank)
        if k not in pos:
            pos[k] = (rank, doc)

    fused = sorted(pos.values(), key=lambda x: scores[_key(x[1])], reverse=True)
    return [doc for _, doc in fused[:top_k]]


def _parse_llm_json_output(llm_output: str, meta: List[Tuple[str, int]]) -> Dict:
    """
    解析LLM输出的JSON格式，提取answer、filename、page。
    如果解析失败，使用降级策略。
    
    Args:
        llm_output: LLM的原始输出
        meta: 检索结果的元数据列表 [(filename, page), ...]
    
    Returns:
        包含answer、filename、page的字典
    """
    default_filename = meta[0][0] if meta else ""
    default_page = meta[0][1] if meta else 0
    
    try:
        # 尝试从输出中提取JSON（处理markdown代码块）
        json_match = re.search(r'```json\s*([\s\S]*?)\s*```', llm_output)
        if json_match:
            json_str = json_match.group(1)
        else:
            # 尝试直接解析整个输出
            json_str = llm_output.strip()
            # 如果输出包含其他文本，尝试找到JSON对象
            brace_start = json_str.find('{')
            brace_end = json_str.rfind('}')
            if brace_start != -1 and brace_end != -1:
                json_str = json_str[brace_start:brace_end+1]
        
        parsed = json.loads(json_str)
        
        answer_text = parsed.get("answer", "").strip()
        filename = parsed.get("filename", "").strip()
        page = parsed.get("page", 0)
        
        # 验证filename是否在meta中（防止LLM编造）
        valid_filenames = {m[0] for m in meta}
        if filename not in valid_filenames:
            logger.warning(f"LLM输出的filename '{filename}' 不在检索结果中，使用默认值")
            filename = default_filename
            # 尝试匹配page
            for m in meta:
                if m[1] == page:
                    filename = m[0]
                    break
        
        # 确保page是整数
        if isinstance(page, str):
            page = int(page) if page.isdigit() else default_page
        
        return {
            "answer": answer_text if answer_text else llm_output,
            "filename": filename if filename else default_filename,
            "page": page if page else default_page,
        }
        
    except (json.JSONDecodeError, KeyError, ValueError) as e:
        logger.warning(f"JSON解析失败: {e}, 使用降级策略")
        # 降级策略：直接使用LLM输出作为answer，来源取第一条
        return {
            "answer": llm_output.strip(),
            "filename": default_filename,
            "page": default_page,
        }


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
        fts_query = _build_fts_query(question)

        bm25_request = {
            "query": fts_query,
            "from": 0,
            "size": BM25_CANDIDATES,
        }
        vec_request = {
            "knn": {
                "field": "vector",
                "k": VEC_CANDIDATES,
                "num_candidates": max(VEC_CANDIDATES, 128),
                "query_vector": question_embedding,
                # 不强行加文本过滤，避免召回过窄
                # "filter": fts_query,
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

        with ThreadPoolExecutor(max_workers=2) as executor:
            future_bm25 = executor.submit(client.search, index=TABLE_NAME, body=bm25_request)
            future_vec = executor.submit(client.search, index=TABLE_NAME, body=vec_request)
            
            try:
                bm25_results = future_bm25.result() or []
            except Exception as e:
                logger.error(f"BM25 search failed: {e}")

            try:
                vec_results = future_vec.result() or []
            except Exception as e:
                logger.error(f"Vector search failed: {e}")

        if not bm25_results and not vec_results:
            logger.warning(f"No search results found for question: '{question[:50]}...'")
            answer.answer = "抱歉，我没有找到相关的信息来回答这个问题。"
            return answer

        # 4) RRF 融合并去重
        fused = _rrf_fuse(bm25_results, vec_results, TOP_K * 2)  # 先取2倍候选用于重排序
        if not fused:
            logger.warning("RRF 融合结果为空，回退使用BM25或向量结果")
            fused = (bm25_results or vec_results)[:TOP_K * 2]

        # 5) 简单重排序：优先选择包含问题关键词的chunk
        def _simple_rerank(chunks: List[Dict], query: str, top_k: int) -> List[Dict]:
            """基于关键词匹配的简单重排序"""
            # 提取查询中的关键词（去除常见停用词）
            stop_words = {"的", "是", "在", "有", "和", "与", "等", "了", "吗", "呢", "什么", "如何", "怎样", "多少"}
            query_terms = [w for w in query if w not in stop_words and len(w) > 1]
            
            scored_chunks = []
            for chunk in chunks:
                content = chunk.get("content", "")
                # 计算关键词匹配分数
                match_score = sum(1 for term in query_terms if term in content)
                # 结合原始排序位置（越靠前越好）
                position_score = 1.0 / (chunks.index(chunk) + 1)
                total_score = match_score * 2 + position_score
                scored_chunks.append((total_score, chunk))
            
            # 按分数排序并返回top_k
            scored_chunks.sort(key=lambda x: x[0], reverse=True)
            return [chunk for _, chunk in scored_chunks[:top_k]]
        
        # 应用重排序
        fused = _simple_rerank(fused, question, TOP_K)

        # 6) 组装上下文与元数据（带来源标注）
        contexts: List[str] = []
        meta: List[Tuple[str, int]] = []
        seen = set()
        for idx, r in enumerate(fused):
            content = r.get("content", "").strip()
            filename = r.get("filename", "")
            page = r.get("page", 0)
            key = (filename, page)
            if content and key not in seen:
                # 为每个chunk添加来源标注，方便LLM识别
                labeled_content = f"【片段{idx+1}｜来源：{filename}，第{page}页】\n{content}"
                contexts.append(labeled_content)
                meta.append(key)
                seen.add(key)

        if not contexts:
            logger.warning("No valid contexts extracted after fusion")
            answer.answer = "抱歉，我没有找到相关的信息来回答这个问题。"
            return answer

        # 控制上下文总长度，避免提示过长
        contexts = _truncate_context(contexts, MAX_CONTEXT_CHARS)
        context_text = "\n\n".join(contexts)

        # 7) 生成答案（LLM输出JSON格式，包含answer、filename、page）
        logger.debug("Generating answer using LLM model ...")
        prompt = QUERY_USER_PROMPT_TEMPLATE.format(
            context_text=context_text,
            question=question,
        )

        messages = [
            {"role": "system", "content": QUERY_SYSTEM_PROMPT},
            {"role": "user", "content": prompt},
        ]

        llm_output = generate_llm_response(messages)
        
        # 8) 解析LLM输出的JSON，提取answer、filename、page
        parsed = _parse_llm_json_output(llm_output, meta)
        answer.answer = parsed["answer"]
        answer.filename = parsed["filename"]
        answer.page = parsed["page"]
        
        logger.debug(
            f"Query completed: question='{question[:50]}...', filename='{answer.filename}', page={answer.page}"
        )

    except Exception as e:
        logger.error(f"Error in query: {e}", exc_info=True)
        answer.answer = f"查询过程中发生错误：{str(e)}"

    return answer
