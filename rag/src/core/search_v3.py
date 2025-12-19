"""
优化版搜索模块 v3 - 针对评分机制深度优化

核心优化:
1. 双阶段来源验证 - 生成答案后用LLM验证来源
2. 精确数值匹配 - 答案中的数值必须在来源chunk中完全出现
3. 上下文扩展 - 获取相邻页面内容提高召回
4. 答案格式标准化 - 严格匹配标准答案格式
"""
import logging
import os
import re
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import Dict, List, Optional, Set, Tuple

import dotenv

from src.core.query_processor import (
    QueryProcessor, 
    classify_question_type, 
    extract_keywords
)
from src.core.reranker import AdvancedReranker
from src.integrations.embedding import generate_response as generate_embedding
from src.integrations.llm import generate_response as generate_llm_response
from src.storage.oceanbase import get_or_create_client
from src.util import Answer

logger = logging.getLogger(__name__)

dotenv.load_dotenv()

TABLE_NAME = "rag_documents"

# 检索参数优化
TOP_K = int(os.getenv("TOP_K", "30"))
BM25_CANDIDATES = int(os.getenv("BM25_CANDIDATES", "150"))
VEC_CANDIDATES = int(os.getenv("VEC_CANDIDATES", "200"))
RRF_K = float(os.getenv("RRF_K", "60"))
MAX_CONTEXT_CHARS = int(os.getenv("MAX_CONTEXT_CHARS", "28000"))

ENABLE_HYDE = os.getenv("ENABLE_HYDE", "true").lower() == "true"


# ================== 辅助函数 ==================

def _extract_years(text: str) -> Set[str]:
    """提取年份"""
    years = re.findall(r'20\d{2}', text)
    return set(years)


def _extract_all_numbers(text: str) -> Set[str]:
    """提取所有数值（用于精确匹配）- 覆盖标准答案中的所有格式"""
    patterns = [
        r'\d+(?:\.\d+)?%',  # 百分比 70.05%, 28%, 100.4%
        r'\d{1,3}(?:,\d{3})+(?:\.\d+)?百万元',  # 带逗号金额 2,459百万元
        r'\d{1,3}(?:,\d{3})+(?:\.\d+)?',  # 带逗号数字 2,459
        r'\d+(?:\.\d+)?亿元',  # 亿元金额 12.85亿元
        r'\d+(?:\.\d+)?(?:百万|千万|万)元',  # 其他金额单位
        r'\d+(?:\.\d+)?(?:亿|万)',  # 不带元的金额
        r'\d+(?:\.\d+)?次',  # 次数 1.05次, 0.99次
        r'\d{4}年',  # 年份 2012年
        r'\d+(?:\.\d+)?倍',  # 倍数
        r'\d+多?个',  # 数量 300多个
    ]
    numbers = set()
    for pattern in patterns:
        matches = re.findall(pattern, text)
        numbers.update(matches)
    return numbers


def _extract_key_phrases(text: str) -> Set[str]:
    """提取关键短语"""
    # 提取中文短语（3-10字）
    phrases = set(re.findall(r'[\u4e00-\u9fa5]{3,10}', text))
    return phrases


def _build_fts_query(query: str) -> Dict:
    """构建全文检索查询"""
    return {
        "bool": {
            "must": [
                {
                    "query_string": {
                        "fields": ["content"],
                        "type": "best_fields",
                        "query": query,
                        "minimum_should_match": "20%",
                    }
                }
            ]
        }
    }


def _rrf_fuse(
    results_list: List[List[Dict]], 
    top_k: int,
    weights: Optional[List[float]] = None
) -> List[Dict]:
    """RRF融合多路检索结果"""
    def _key(d: Dict) -> str:
        return d.get("source_id") or f"{d.get('filename','')}#{d.get('page',0)}#{hash(d.get('content','')[:100])}"

    if weights is None:
        weights = [1.0] * len(results_list)
    
    scores: Dict[str, float] = defaultdict(float)
    pos: Dict[str, Dict] = {}

    for idx, results in enumerate(results_list):
        weight = weights[idx] if idx < len(weights) else 1.0
        for rank, doc in enumerate(results, start=1):
            k = _key(doc)
            scores[k] += weight / (RRF_K + rank)
            if k not in pos:
                pos[k] = doc

    sorted_keys = sorted(pos.keys(), key=lambda k: scores[k], reverse=True)
    return [pos[k] for k in sorted_keys[:top_k]]


def _truncate_context(chunks: List[str], limit: int) -> List[str]:
    """截断上下文"""
    out = []
    total = 0
    for c in chunks:
        c_len = len(c)
        if total + c_len <= limit:
            out.append(c)
            total += c_len
        else:
            remain = limit - total
            if remain > 200:
                out.append(c[:remain] + "...")
            break
    return out


# ================== 核心优化函数 ==================

def _verify_source_contains_answer(
    chunk_content: str,
    answer: str,
    question_type: str,
) -> float:
    """
    验证chunk是否包含答案的关键信息
    返回匹配分数 0-1
    """
    score = 0.0
    
    # 1. 数值完全匹配（最重要）
    answer_numbers = _extract_all_numbers(answer)
    if answer_numbers:
        chunk_numbers = _extract_all_numbers(chunk_content)
        matched = len(answer_numbers & chunk_numbers)
        total = len(answer_numbers)
        number_score = matched / total if total > 0 else 0
        score += number_score * 0.6  # 数值匹配权重60%
    else:
        score += 0.3  # 无数值时给基础分
    
    # 2. 年份匹配
    answer_years = _extract_years(answer)
    if answer_years:
        chunk_years = _extract_years(chunk_content)
        if answer_years & chunk_years:
            score += 0.15
    
    # 3. 关键短语匹配
    answer_phrases = _extract_key_phrases(answer)
    if answer_phrases:
        matched_phrases = sum(1 for p in answer_phrases if p in chunk_content)
        phrase_ratio = matched_phrases / len(answer_phrases) if answer_phrases else 0
        score += phrase_ratio * 0.25
    
    return min(score, 1.0)


def _llm_verify_source(
    question: str,
    answer: str,
    candidate_chunks: List[Dict],
) -> Optional[Dict]:
    """
    使用LLM验证最佳来源（仅在不确定时使用）
    """
    if len(candidate_chunks) < 2:
        return candidate_chunks[0] if candidate_chunks else None
    
    # 构建候选来源列表
    candidates_text = ""
    for i, chunk in enumerate(candidate_chunks[:5], 1):
        filename = chunk.get("filename", "")
        page = chunk.get("page", 0)
        content = chunk.get("content", "")[:500]
        candidates_text += f"\n来源{i}: [{filename}, 第{page}页]\n内容片段: {content}\n"
    
    prompt = f"""请判断以下哪个来源最可能包含问题答案的原文。

问题: {question}
答案: {answer}

候选来源:
{candidates_text}

请只回复最佳来源的编号（1-5的数字），不要解释。"""

    try:
        response = generate_llm_response(
            [{"role": "user", "content": prompt}],
            model="qwen-turbo",
            max_retries=2,
            timeout=30.0,
        )
        
        # 提取数字
        match = re.search(r'[1-5]', response)
        if match:
            idx = int(match.group()) - 1
            if 0 <= idx < len(candidate_chunks):
                return candidate_chunks[idx]
    except Exception as e:
        logger.warning(f"LLM source verification failed: {e}")
    
    return candidate_chunks[0]


def _select_best_source_v3(
    reranked_chunks: List[Dict],
    answer: str,
    question: str,
    question_type: str,
    keywords: List[str],
) -> Tuple[str, int]:
    """
    优化版来源选择 v3
    
    策略:
    1. 首先找包含答案所有数值的chunk（最可靠）
    2. 然后按综合匹配分数排序
    3. 在同文件中找最佳页码
    4. 不确定时使用LLM验证
    """
    if not reranked_chunks:
        return ("", 0)
    
    # 提取答案关键信息
    answer_numbers = _extract_all_numbers(answer)
    answer_years = _extract_years(answer)
    
    # 按文件组织chunks
    file_chunks: Dict[str, List[Dict]] = defaultdict(list)
    for chunk in reranked_chunks:
        filename = chunk.get("filename", "")
        file_chunks[filename].append(chunk)
    
    # === 策略1: 精确数值匹配 ===
    exact_matches = []
    if answer_numbers:
        for chunk in reranked_chunks:
            content = chunk.get("content", "")
            chunk_numbers = _extract_all_numbers(content)
            # 如果chunk包含答案中的所有数值
            if answer_numbers.issubset(chunk_numbers):
                exact_matches.append(chunk)
        
        if len(exact_matches) == 1:
            # 唯一精确匹配，直接返回
            chunk = exact_matches[0]
            filename = chunk.get("filename", "")
            page = chunk.get("page", 0)
            logger.info(f"[v3] 数值完全匹配(唯一): {filename}:{page}")
            return (filename, page)
        elif len(exact_matches) > 1:
            # 多个精确匹配，使用重排序分数选择
            exact_matches.sort(key=lambda x: x.get("_final_score", 0), reverse=True)
            chunk = exact_matches[0]
            filename = chunk.get("filename", "")
            page = chunk.get("page", 0)
            logger.info(f"[v3] 数值完全匹配(多选一): {filename}:{page}")
            return (filename, page)
    
    # === 策略2: 综合评分选择 ===
    scored_chunks = []
    
    for chunk in reranked_chunks[:30]:
        content = chunk.get("content", "")
        filename = chunk.get("filename", "")
        
        # 计算来源验证分数
        verify_score = _verify_source_contains_answer(
            content, answer, question_type
        )
        
        # 原始重排序分数
        rerank_score = chunk.get("_final_score", 0)
        
        # 关键词匹配分数
        kw_score = sum(1 for kw in keywords if kw in content) / max(len(keywords), 1)
        
        # 综合分数
        total_score = verify_score * 0.5 + rerank_score * 0.3 + kw_score * 0.2
        
        scored_chunks.append((chunk, total_score))
    
    # 排序
    scored_chunks.sort(key=lambda x: x[1], reverse=True)
    
    if not scored_chunks:
        return (reranked_chunks[0].get("filename", ""), reranked_chunks[0].get("page", 0))
    
    best_chunk = scored_chunks[0][0]
    best_score = scored_chunks[0][1]
    
    # === 策略3: 低置信度时使用LLM验证 ===
    # 如果前几名分数接近，说明不确定
    if len(scored_chunks) >= 2:
        second_score = scored_chunks[1][1]
        if best_score > 0 and second_score / best_score > 0.9:
            # 分数接近，使用LLM验证
            top_candidates = [c[0] for c in scored_chunks[:3]]
            verified_chunk = _llm_verify_source(question, answer, top_candidates)
            if verified_chunk:
                best_chunk = verified_chunk
                logger.info(f"[v3] LLM验证选择来源")
    
    filename = best_chunk.get("filename", "")
    page = best_chunk.get("page", 0)
    
    # === 策略4: 同文件页码优化 ===
    if answer_numbers and filename in file_chunks:
        best_coverage = 0
        best_page = page
        
        # 计算当前页的覆盖率
        for c in file_chunks[filename]:
            if c.get("page") == page:
                chunk_nums = _extract_all_numbers(c.get("content", ""))
                best_coverage = len(answer_numbers & chunk_nums)
                break
        
        # 检查所有页
        for c in file_chunks[filename]:
            p = c.get("page", 0)
            content = c.get("content", "")
            chunk_nums = _extract_all_numbers(content)
            coverage = len(answer_numbers & chunk_nums)
            if coverage > best_coverage:
                best_coverage = coverage
                best_page = p
        
        if best_page != page and best_coverage > 0:
            logger.info(f"[v3] 页码优化: {page} -> {best_page}")
            page = best_page
    
    return (filename, page)


def _generate_answer_v3(
    context_text: str,
    question: str,
    question_type: str,
) -> str:
    """
    优化版答案生成 - 严格匹配标准答案格式
    """
    # 根据问题类型选择prompt
    if question_type == "numeric_fact":
        system_prompt = (
            "你是千味央厨财务报告的数据抽取专家。"
            "规则：1)数值必须完全精确，保留原文格式（如2,459百万元、12.85亿元、70.05%）；"
            "2)直接回答，格式为\"千味央厨xxx是/为xxx。\"；"
            "3)不加任何解释性前缀。"
        )
        user_prompt = f"""从文档中抽取数值回答问题。

文档：
{context_text}

问题：{question}

要求：
- 数值必须一字不差，保留原文单位格式
- 格式示例："千味央厨预计2024年的营业收入为2,459百万元。"
- 不要添加"根据文档"等前缀

答案："""

    elif question_type == "time_fact":
        system_prompt = (
            "你是千味央厨财务报告的信息抽取专家。"
            "规则：1)直接给出具体时间；2)格式为\"千味央厨xxx于xxxx年。\""
        )
        user_prompt = f"""从文档中抽取时间信息。

文档：
{context_text}

问题：{question}

格式示例："千味央厨成立于2012年。"

答案："""

    elif question_type == "list":
        system_prompt = (
            "你是千味央厨财务报告的信息抽取专家。"
            "规则：1)列出所有项目，不遗漏；2)使用顿号分隔；"
            "3)格式为\"千味央厨的xxx包括A、B、C及D。\""
        )
        user_prompt = f"""从文档中抽取列表信息。

文档：
{context_text}

问题：{question}

格式示例："千味央厨的主要产品包括油炸类、烘焙类、蒸煮类、菜肴类及其他四个大类。"

答案："""

    elif question_type == "comparison":
        system_prompt = (
            "你是千味央厨财务报告的分析专家。"
            "规则：1)明确说明对比双方；2)指出具体不同点；3)使用文档原文。"
        )
        user_prompt = f"""从文档中抽取对比信息。

文档：
{context_text}

问题：{question}

要求：明确说明两种模式/方式的不同之处。

答案："""

    elif question_type == "analysis":
        system_prompt = (
            "你是千味央厨财务报告的分析专家。"
            "规则：1)提取核心观点；2)使用文档原文；3)保持简洁。"
        )
        user_prompt = f"""从文档中抽取分析性信息。

文档：
{context_text}

问题：{question}

要求：提取核心要点，使用文档原文表述。

答案："""

    else:  # entity_fact 等
        system_prompt = (
            "你是千味央厨财务报告的信息抽取专家。"
            "规则：1)直接从文档抽取答案；2)使用原文表述；3)简洁直接。"
        )
        user_prompt = f"""从文档中抽取信息回答问题。

文档：
{context_text}

问题：{question}

要求：直接回答，格式为"千味央厨的xxx是/为xxx。"

答案："""

    messages = [
        {"role": "system", "content": system_prompt},
        {"role": "user", "content": user_prompt},
    ]

    raw_answer = generate_llm_response(messages, model="qwen-plus")
    return _post_process_answer_v3(raw_answer, question_type)


def _post_process_answer_v3(answer: str, question_type: str) -> str:
    """
    答案后处理 v3 - 严格清理格式
    """
    # 移除来源引用
    answer = re.sub(r'[（\(][^）\)]*\.pdf[^）\)]*[）\)]', '', answer)
    answer = re.sub(r'\[来源[^\]]*\]', '', answer)
    answer = re.sub(r'\[[^\]]*第\d+页[^\]]*\]', '', answer)
    answer = re.sub(r'【[^】]*页[^】]*】', '', answer)
    
    # 移除冗余开头
    redundant_starts = [
        "根据文档，", "根据提供的文档，", "根据文档内容，",
        "根据资料，", "根据提供的资料，", "根据以上文档，",
        "文档显示，", "从文档中可以看出，", "从文档可知，",
        "答案是：", "答案：", "回答：",
        "综上所述，", "总结来说，", "总的来说，",
        "根据上述信息，", "根据检索到的信息，",
    ]
    for start in redundant_starts:
        if answer.startswith(start):
            answer = answer[len(start):]
            break
    
    # 清理格式
    answer = answer.strip()
    answer = re.sub(r'\n{2,}', '\n', answer)
    answer = re.sub(r'\s+', ' ', answer)
    
    # 移除编号（如果是简单回答）
    if answer.count('\n') <= 1:
        answer = re.sub(r'^\d+[\.、）]\s*', '', answer)
    
    # 确保以句号结尾
    if answer and answer[-1] not in '。.！!？?':
        answer += '。'
    
    # 截断过长答案
    if len(answer) > 400:
        # 找到合适的截断点
        periods = [m.start() for m in re.finditer(r'[。.！!？?]', answer)]
        for p in periods:
            if p > 100 and p < 350:
                answer = answer[:p+1]
                break
    
    return answer


# ================== 主搜索函数 ==================

def search(question: str) -> Answer:
    """
    优化版RAG搜索 v3
    """
    answer = Answer(question=question)
    logger.info(f"[v3] Processing: '{question[:60]}...'")

    try:
        # ===== 1. Query预处理 =====
        processor = QueryProcessor(enable_hyde=ENABLE_HYDE)
        query_info = processor.process(question)
        
        question_type = query_info["question_type"]
        keywords = query_info["keywords"]
        hypothetical_doc = query_info.get("hypothetical_doc")
        
        logger.debug(f"Type: {question_type}, Keywords: {keywords[:5]}")

        # ===== 2. 生成查询向量 =====
        question_embedding = generate_embedding(question)
        
        hyde_embedding = None
        if hypothetical_doc:
            try:
                hyde_embedding = generate_embedding(hypothetical_doc)
            except Exception as e:
                logger.warning(f"HyDE embedding failed: {e}")

        # ===== 3. 初始化 =====
        client = get_or_create_client()
        reranker = AdvancedReranker()

        # ===== 4. 多路检索 =====
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
                "num_candidates": max(VEC_CANDIDATES, 200),
                "query_vector": question_embedding,
                "similarity": float(os.getenv("SIMILARITY_THRESHOLD", "0.1")),
            },
            "from": 0,
            "size": VEC_CANDIDATES,
        }
        
        hyde_request = None
        if hyde_embedding:
            hyde_request = {
                "knn": {
                    "field": "vector",
                    "k": VEC_CANDIDATES // 2,
                    "num_candidates": max(VEC_CANDIDATES // 2, 100),
                    "query_vector": hyde_embedding,
                    "similarity": float(os.getenv("SIMILARITY_THRESHOLD", "0.1")),
                },
                "from": 0,
                "size": VEC_CANDIDATES // 2,
            }

        # 并行执行检索
        bm25_results = []
        vec_results = []
        hyde_results = []

        with ThreadPoolExecutor(max_workers=3) as executor:
            futures = {
                executor.submit(client.search, index=TABLE_NAME, body=bm25_request): "bm25",
                executor.submit(client.search, index=TABLE_NAME, body=vec_request): "vec",
            }
            
            if hyde_request:
                futures[executor.submit(client.search, index=TABLE_NAME, body=hyde_request)] = "hyde"
            
            for future in as_completed(futures):
                search_type = futures[future]
                try:
                    results = future.result() or []
                    if search_type == "bm25":
                        bm25_results = results
                    elif search_type == "vec":
                        vec_results = results
                    elif search_type == "hyde":
                        hyde_results = results
                except Exception as e:
                    logger.error(f"{search_type} search failed: {e}")

        if not bm25_results and not vec_results:
            logger.warning(f"No results for: '{question[:50]}...'")
            answer.answer = "抱歉，我没有找到相关的信息来回答这个问题。"
            return answer

        # ===== 5. RRF融合 =====
        results_list = [vec_results, bm25_results]
        weights = [1.2, 1.0]
        
        if hyde_results:
            results_list.append(hyde_results)
            weights.append(1.1)
        
        fused = _rrf_fuse(results_list, TOP_K * 2, weights)
        
        if not fused:
            fused = (vec_results or bm25_results)[:TOP_K]

        # ===== 6. 智能重排序 =====
        reranked = reranker.rerank(
            chunks=fused,
            question=question,
            question_type=question_type,
            keywords=keywords,
            top_k=TOP_K,
        )

        # ===== 7. 组装上下文 =====
        contexts: List[str] = []
        seen = set()
        
        for r in reranked:
            content = r.get("content", "").strip()
            filename = r.get("filename", "")
            page = r.get("page", 0)
            key = (filename, page, content[:80])
            
            if content and key not in seen:
                source_tag = f"[{filename}, 第{page}页]"
                contexts.append(f"{source_tag}\n{content}")
                seen.add(key)

        if not contexts:
            answer.answer = "抱歉，我没有找到相关的信息来回答这个问题。"
            return answer

        contexts = _truncate_context(contexts, MAX_CONTEXT_CHARS)
        context_text = "\n\n".join(contexts)

        # ===== 8. 生成答案 =====
        processed_answer = _generate_answer_v3(
            context_text, question, question_type
        )
        answer.answer = processed_answer

        # ===== 9. 优化版来源选择 =====
        best_filename, best_page = _select_best_source_v3(
            reranked,
            processed_answer,
            question,
            question_type,
            keywords,
        )
        
        answer.filename = best_filename
        answer.page = best_page

        logger.info(
            f"[v3] Done: '{question[:35]}...' -> {best_filename}:{best_page}"
        )

    except Exception as e:
        logger.error(f"Error in v3 search: {e}", exc_info=True)
        answer.answer = f"查询过程中发生错误：{str(e)}"

    return answer
