"""
优化版搜索模块 v2 - 集成所有优化策略

核心优化：
1. LLM 关键词提取（高/低级关键词）
2. 模型重排序（gte-rerank）
3. 多策略来源选择
4. 增强的答案后处理
"""
import logging
import os
import re
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import Dict, List, Optional, Set, Tuple

import dotenv

from src.core.keyword_extractor import EnhancedKeywordExtractor
from src.core.model_reranker import model_rerank_chunks
from src.core.query_processor import QueryProcessor, classify_question_type
from src.core.reranker import AdvancedReranker
from src.integrations.embedding import generate_response as generate_embedding
from src.integrations.llm import generate_response as generate_llm_response
from src.prompt.query import (
    ANALYSIS_QUESTION_PROMPT,
    COMPARISON_QUESTION_PROMPT,
    LIST_QUESTION_PROMPT,
    NUMERIC_QUESTION_PROMPT,
    QUERY_SYSTEM_PROMPT,
    QUERY_USER_PROMPT_TEMPLATE,
    TIME_FACT_PROMPT,
)
from src.storage.oceanbase import get_or_create_client
from src.util import Answer

logger = logging.getLogger(__name__)

dotenv.load_dotenv()

TABLE_NAME = "rag_documents"

# 检索参数
TOP_K = int(os.getenv("TOP_K", "25"))
BM25_CANDIDATES = int(os.getenv("BM25_CANDIDATES", str(TOP_K * 5)))
VEC_CANDIDATES = int(os.getenv("VEC_CANDIDATES", str(TOP_K * 8)))
RRF_K = float(os.getenv("RRF_K", "60"))
MAX_CONTEXT_CHARS = int(os.getenv("MAX_CONTEXT_CHARS", "28000"))

ENABLE_HYDE = os.getenv("ENABLE_HYDE", "true").lower() == "true"
ENABLE_MODEL_RERANK = os.getenv("ENABLE_MODEL_RERANK", "true").lower() == "true"
ENABLE_LLM_KEYWORDS = os.getenv("ENABLE_LLM_KEYWORDS", "false").lower() == "true"


def _extract_years(text: str) -> Set[str]:
    """提取文本中的年份"""
    years = re.findall(r'20\d{2}', text)
    return set(years)


def _extract_precise_numbers(text: str) -> Set[str]:
    """提取精确数值"""
    patterns = [
        r'\d+(?:\.\d+)?%',
        r'\d{1,3}(?:,\d{3})*(?:\.\d+)?',
        r'\d+(?:\.\d+)?(?:亿|万|百万)?(?:元)?',
    ]
    numbers = set()
    for pattern in patterns:
        matches = re.findall(pattern, text)
        numbers.update(matches)
    return numbers


def _build_fts_query(query: str, boost_terms: Optional[List[str]] = None) -> Dict:
    """构建全文检索查询"""
    # 基础查询
    query_string = query
    
    # 如果有加权词，添加 boost
    if boost_terms:
        boosted = " ".join([f"({term})^1.5" for term in boost_terms[:5]])
        query_string = f"{query} {boosted}"
    
    return {
        "bool": {
            "must": [
                {
                    "query_string": {
                        "fields": ["content"],
                        "type": "best_fields",
                        "query": query_string,
                        "minimum_should_match": "25%",
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
    """RRF 融合多路检索结果"""
    def _key(d: Dict) -> str:
        return d.get("source_id") or f"{d.get('filename', '')}#{d.get('page', 0)}#{hash(d.get('content', '')[:100])}"

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


def _select_best_source_v2(
    reranked_chunks: List[Dict],
    answer: str,
    question: str,
    question_type: str,
    keywords: Dict[str, List[str]],
) -> Tuple[str, int]:
    """
    增强版来源选择 v2
    
    多策略验证：
    1. 答案数值精确匹配（最高优先级）
    2. 答案关键词重叠
    3. 模型重排序得分
    4. 规则重排序得分
    5. 邻页回溯
    """
    if not reranked_chunks:
        return ("", 0)

    # 提取答案中的关键信息
    answer_numbers = _extract_precise_numbers(answer)
    answer_years = _extract_years(answer)
    answer_chinese = set(re.findall(r'[\u4e00-\u9fa5]{2,6}', answer))

    # 问题中的信息
    question_numbers = _extract_precise_numbers(question)
    question_years = _extract_years(question)

    # 关键词
    low_level_kw = keywords.get("low_level_keywords", [])
    
    best_chunk = None
    best_score = -1

    # 按文件组织 chunks
    file_chunks: Dict[str, List[Dict]] = defaultdict(list)
    for chunk in reranked_chunks:
        filename = chunk.get("filename", "")
        file_chunks[filename].append(chunk)

    for chunk in reranked_chunks[:20]:  # 检查前20个
        content = chunk.get("content", "")
        filename = chunk.get("filename", "")
        
        score = 0.0

        # 策略1: 答案数值精确匹配（最高优先级）
        if answer_numbers:
            chunk_numbers = _extract_precise_numbers(content)
            exact_matches = len(answer_numbers & chunk_numbers)
            if exact_matches > 0:
                score += exact_matches * 50  # 高权重

        # 策略2: 答案年份匹配
        if answer_years:
            chunk_years = _extract_years(content)
            year_matches = len(answer_years & chunk_years)
            score += year_matches * 20

        # 策略3: 问题数值/年份匹配
        if question_numbers:
            chunk_numbers = _extract_precise_numbers(content)
            q_matches = len(question_numbers & chunk_numbers)
            score += q_matches * 15

        if question_years:
            chunk_years = _extract_years(content)
            q_year_matches = len(question_years & chunk_years)
            score += q_year_matches * 10

        # 策略4: 答案关键词重叠
        chunk_chinese = set(re.findall(r'[\u4e00-\u9fa5]{2,6}', content))
        chinese_overlap = len(answer_chinese & chunk_chinese)
        score += chinese_overlap * 3

        # 策略5: 低级关键词匹配
        for kw in low_level_kw:
            if kw in content:
                score += 8

        # 策略6: 模型重排序得分
        model_score = chunk.get("_model_rerank_score", 0)
        score += model_score * 30

        # 策略7: 规则重排序得分
        rule_score = chunk.get("_final_score", 0)
        score += rule_score * 20

        if score > best_score:
            best_score = score
            best_chunk = chunk

    if not best_chunk:
        best_chunk = reranked_chunks[0]

    filename = best_chunk.get("filename", "")
    page = best_chunk.get("page", 0)

    # 邻页回溯：检查相邻页是否有更好的数值匹配
    if answer_numbers and filename in file_chunks:
        best_num_coverage = 0
        best_page_candidate = page

        for p in range(max(1, page - 2), page + 3):
            for chunk in file_chunks[filename]:
                if chunk.get("page") == p:
                    content = chunk.get("content", "")
                    chunk_numbers = _extract_precise_numbers(content)
                    coverage = len(answer_numbers & chunk_numbers)
                    if coverage > best_num_coverage:
                        best_num_coverage = coverage
                        best_page_candidate = p

        if best_page_candidate != page:
            logger.debug(f"邻页回溯: {page} -> {best_page_candidate}")
            page = best_page_candidate

    return (filename, page)


def _post_process_answer_v2(answer: str, question: str = "") -> str:
    """增强版答案后处理"""
    # 移除来源引用
    answer = re.sub(r'[（\(][^）\)]*\.pdf[^）\)]*[）\)]', '', answer)
    answer = re.sub(r'\[来源[^\]]*\]', '', answer)
    answer = re.sub(r'\[[^\]]*第\d+页[^\]]*\]', '', answer)

    # 移除冗余开头
    redundant_starts = [
        "根据文档，", "根据提供的文档，", "根据文档内容，",
        "根据资料，", "根据提供的资料，", "根据以上文档，",
        "文档显示，", "从文档中可以看出，", "从文档可知，",
        "答案是：", "答案：", "回答：",
        "综上所述，", "总结来说，",
    ]
    for start in redundant_starts:
        if answer.startswith(start):
            answer = answer[len(start):]
            break

    # 清理格式
    answer = answer.strip()
    answer = re.sub(r'\n{2,}', '\n', answer)

    # 移除编号列表格式（如果是简单回答）
    if answer.count('\n') <= 1:
        answer = re.sub(r'^\d+[\.、）]\s*', '', answer)

    # 确保以句号结尾
    if answer and answer[-1] not in '。.！!？?':
        answer += '。'

    # 长度限制
    if len(answer) > 500:
        first_period = answer.find('。')
        if first_period > 0 and first_period < 300:
            remaining = answer[first_period + 1:]
            if '、' in remaining[:100]:
                second_period = remaining.find('。')
                if second_period > 0:
                    answer = answer[:first_period + 1 + second_period + 1]

    return answer


def search(question: str) -> Answer:
    """
    优化版 RAG 搜索 v2
    
    集成：
    - LLM 关键词提取
    - HyDE 假设性文档增强
    - 多路检索 + RRF 融合
    - 模型重排序 + 规则重排序
    - 多策略来源选择
    """
    answer = Answer(question=question)
    logger.info(f"[OptimizedV2] Processing: '{question[:60]}...'")

    try:
        # ===== 1. Query 预处理 =====
        processor = QueryProcessor(enable_hyde=ENABLE_HYDE)
        query_info = processor.process(question)

        question_type = query_info["question_type"]
        basic_keywords = query_info["keywords"]
        hypothetical_doc = query_info.get("hypothetical_doc")

        # LLM 关键词提取（可选）
        keyword_extractor = EnhancedKeywordExtractor(use_llm=ENABLE_LLM_KEYWORDS)
        extracted_keywords = keyword_extractor.extract(question)

        logger.debug(f"Type: {question_type}, Keywords: {extracted_keywords}")

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
        rule_reranker = AdvancedReranker()

        # ===== 4. 多路检索 =====
        # BM25 检索（使用低级关键词加权）
        low_level_kw = extracted_keywords.get("low_level_keywords", [])
        fts_query = _build_fts_query(question, boost_terms=low_level_kw)

        bm25_request = {
            "query": fts_query,
            "from": 0,
            "size": BM25_CANDIDATES,
        }

        # 向量检索
        vec_request = {
            "knn": {
                "field": "vector",
                "k": VEC_CANDIDATES,
                "num_candidates": max(VEC_CANDIDATES, 150),
                "query_vector": question_embedding,
                "similarity": float(os.getenv("SIMILARITY_THRESHOLD", "0.15")),
            },
            "from": 0,
            "size": VEC_CANDIDATES,
        }

        # HyDE 向量检索
        hyde_request = None
        if hyde_embedding:
            hyde_request = {
                "knn": {
                    "field": "vector",
                    "k": VEC_CANDIDATES // 2,
                    "num_candidates": max(VEC_CANDIDATES // 2, 80),
                    "query_vector": hyde_embedding,
                    "similarity": float(os.getenv("SIMILARITY_THRESHOLD", "0.15")),
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

        # ===== 5. RRF 融合 =====
        results_list = [vec_results, bm25_results]
        weights = [1.3, 1.0]

        if hyde_results:
            results_list.append(hyde_results)
            weights.append(1.15)

        fused = _rrf_fuse(results_list, TOP_K * 2, weights)

        if not fused:
            fused = (vec_results or bm25_results)[:TOP_K]

        # ===== 6. 规则重排序 =====
        rule_reranked = rule_reranker.rerank(
            chunks=fused,
            question=question,
            question_type=question_type,
            keywords=basic_keywords,
            top_k=TOP_K * 2,
        )

        # ===== 7. 模型重排序（可选）=====
        if ENABLE_MODEL_RERANK:
            try:
                final_reranked = model_rerank_chunks(
                    query=question,
                    chunks=rule_reranked,
                    top_n=TOP_K,
                    enable_model_rerank=True,
                )
            except Exception as e:
                logger.warning(f"Model reranking failed, using rule-based: {e}")
                final_reranked = rule_reranked[:TOP_K]
        else:
            final_reranked = rule_reranked[:TOP_K]

        # ===== 8. 组装上下文 =====
        contexts: List[str] = []
        seen = set()

        for r in final_reranked:
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

        # ===== 9. 生成答案 =====
        if question_type == "numeric_fact":
            prompt_template = NUMERIC_QUESTION_PROMPT
        elif question_type == "time_fact":
            prompt_template = TIME_FACT_PROMPT
        elif question_type == "list":
            prompt_template = LIST_QUESTION_PROMPT
        elif question_type == "comparison":
            prompt_template = COMPARISON_QUESTION_PROMPT
        elif question_type == "analysis":
            prompt_template = ANALYSIS_QUESTION_PROMPT
        else:
            prompt_template = QUERY_USER_PROMPT_TEMPLATE

        prompt = prompt_template.format(
            context_text=context_text,
            question=question,
        )

        messages = [
            {"role": "system", "content": QUERY_SYSTEM_PROMPT},
            {"role": "user", "content": prompt},
        ]

        raw_answer = generate_llm_response(messages)
        processed_answer = _post_process_answer_v2(raw_answer)
        answer.answer = processed_answer

        # ===== 10. 来源选择 =====
        best_filename, best_page = _select_best_source_v2(
            final_reranked,
            processed_answer,
            question,
            question_type,
            extracted_keywords,
        )

        answer.filename = best_filename
        answer.page = best_page

        logger.info(
            f"[OptimizedV2] Done: '{question[:35]}...' -> {best_filename}:{best_page}"
        )

    except Exception as e:
        logger.error(f"Error in optimized search v2: {e}", exc_info=True)
        answer.answer = f"查询过程中发生错误：{str(e)}"

    return answer


# 兼容性别名
def search_optimized_v2(question: str) -> Answer:
    """优化版搜索 v2（别名）"""
    return search(question)


if __name__ == "__main__":
    # 测试
    logging.basicConfig(level=logging.INFO)
    
    test_questions = [
        "千味央厨的成立时间是什么时候？",
        "千味央厨的财务报表预测中，预计2024年的营业收入是多少？",
        "千味央厨的主要产品有哪些类别？",
    ]
    
    for q in test_questions:
        print(f"\n问题: {q}")
        result = search(q)
        print(f"答案: {result.answer}")
        print(f"来源: {result.filename}, 第{result.page}页")
