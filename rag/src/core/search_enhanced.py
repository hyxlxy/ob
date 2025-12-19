"""
增强版搜索模块 - 针对评分机制专门优化

关键优化点：
1. 答案抽取模式 - 从文档中直接抽取答案片段，贴近原文表述
2. 二次来源验证 - 答案生成后反向验证来源准确性
3. 文件名直接匹配 - 基于问题关键词直接匹配最可能的文件
4. 数值/年份完全匹配加权 - 精确匹配大幅提升得分
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
from src.core.reranker import AdvancedReranker, extract_numbers_from_text
from src.integrations.embedding import generate_response as generate_embedding
from src.integrations.llm import generate_response as generate_llm_response
from src.prompt.query import (
    QUERY_SYSTEM_PROMPT,
    QUERY_USER_PROMPT_TEMPLATE,
    NUMERIC_QUESTION_PROMPT,
    TIME_FACT_PROMPT,
    LIST_QUESTION_PROMPT,
    ANALYSIS_QUESTION_PROMPT,
    COMPARISON_QUESTION_PROMPT,
)
from src.storage.oceanbase import get_or_create_client
from src.util import Answer

logger = logging.getLogger(__name__)

dotenv.load_dotenv()

TABLE_NAME = "rag_documents"

# 检索参数 - 针对40分钟时限优化
TOP_K = int(os.getenv("TOP_K", "20"))
BM25_CANDIDATES = int(os.getenv("BM25_CANDIDATES", "60"))
VEC_CANDIDATES = int(os.getenv("VEC_CANDIDATES", "80"))
RRF_K = float(os.getenv("RRF_K", "60"))
MAX_CONTEXT_CHARS = int(os.getenv("MAX_CONTEXT_CHARS", "20000"))

# 关闭HyDE以节省时间（每个问题少一次LLM调用）
ENABLE_HYDE = os.getenv("ENABLE_HYDE", "false").lower() == "true"


def _extract_years(text: str) -> Set[str]:
    """提取文本中的年份"""
    years = re.findall(r'20\d{2}', text)
    return set(years)


def _extract_precise_numbers(text: str) -> Set[str]:
    """提取精确数值（用于完全匹配）"""
    patterns = [
        r'\d+(?:\.\d+)?%',  # 百分比
        r'\d{1,3}(?:,\d{3})*(?:\.\d+)?',  # 带逗号的数字
        r'\d+(?:\.\d+)?(?:亿|万|百万)?(?:元)?',  # 金额
    ]
    numbers = set()
    for pattern in patterns:
        matches = re.findall(pattern, text)
        numbers.update(matches)
    return numbers


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
    """使用倒数排序融合(RRF)合并多路检索结果"""
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
    """截断上下文以适应限制"""
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


def _filename_relevance_boost(
    filename: str,
    question: str,
    keywords: List[str],
    question_type: str,
) -> float:
    """
    计算文件名与问题的相关性加成
    
    针对评分机制：filename必须完全匹配才得25分
    所以需要尽可能精确地选择正确的文件
    """
    boost = 1.0
    filename_lower = filename.lower()
    
    # 1. 关键词直接匹配
    for kw in keywords:
        if kw in filename:
            boost += 0.3
    
    # 2. 年份匹配（文件名中的日期）
    question_years = _extract_years(question)
    # 文件名格式如: 千味央厨-xxx-22033022页.pdf (日期是220330)
    date_match = re.search(r'(\d{2})(\d{2})(\d{2})\d*页', filename)
    if date_match:
        file_year = "20" + date_match.group(1)  # 如 22 -> 2022
        if question_years and file_year in question_years:
            boost += 0.5
    
    # 3. 报告类型匹配
    report_type_boosts = {
        "深度报告": 0.2,
        "深度研究": 0.2,
        "深度分析": 0.2,
        "研究报告": 0.15,
        "公司研究": 0.15,
        "首次覆盖": 0.1,
    }
    for report_type, type_boost in report_type_boosts.items():
        if report_type in filename:
            boost += type_boost
            break
    
    # 4. 问题类型特定匹配
    if question_type == "numeric_fact":
        if "财务" in filename or "报告" in filename:
            boost += 0.15
    elif question_type == "analysis":
        if "深度" in filename or "分析" in filename:
            boost += 0.15
    
    return boost


def _select_best_source_enhanced(
    reranked_chunks: List[Dict],
    answer: str,
    question: str,
    question_type: str,
    keywords: List[str],
) -> Tuple[str, int]:
    """
    增强版来源选择 - 针对评分机制优化
    
    评分机制：
    - filename完全匹配：25分
    - page完全匹配：25分（前提是filename也对）
    
    策略：
    1. 首先找答案中数值/关键信息的精确来源
    2. 然后考虑文件名相关性
    3. 最后考虑重排序得分
    """
    if not reranked_chunks:
        return ("", 0)
    
    # 从答案中提取关键信息
    answer_numbers = _extract_precise_numbers(answer)
    answer_years = _extract_years(answer)
    answer_chinese = set(re.findall(r'[\u4e00-\u9fa5]{3,6}', answer))
    
    # 问题中的数值和年份
    question_numbers = _extract_precise_numbers(question)
    question_years = _extract_years(question)
    
    best_chunk = None
    best_score = -1
    
    # 按文件组织chunks
    file_chunks: Dict[str, List[Dict]] = defaultdict(list)
    for chunk in reranked_chunks:
        filename = chunk.get("filename", "")
        file_chunks[filename].append(chunk)
    
    for chunk in reranked_chunks[:15]:  # 检查前15个结果
        content = chunk.get("content", "")
        filename = chunk.get("filename", "")
        page = chunk.get("page", 0)
        
        score = 0.0
        
        # 策略1: 答案数值精确匹配（最高优先级）
        if answer_numbers:
            chunk_numbers = _extract_precise_numbers(content)
            exact_matches = len(answer_numbers & chunk_numbers)
            if exact_matches > 0:
                # 数值完全匹配是强信号
                score += exact_matches * 30
        
        # 策略2: 答案年份精确匹配
        if answer_years:
            chunk_years = _extract_years(content)
            year_matches = len(answer_years & chunk_years)
            score += year_matches * 15
        
        # 策略3: 问题数值/年份匹配（次优先）
        if question_numbers:
            chunk_numbers = _extract_precise_numbers(content)
            q_matches = len(question_numbers & chunk_numbers)
            score += q_matches * 10
        
        if question_years:
            chunk_years = _extract_years(content)
            q_year_matches = len(question_years & chunk_years)
            score += q_year_matches * 8
        
        # 策略4: 答案中文关键词重叠
        chunk_chinese = set(re.findall(r'[\u4e00-\u9fa5]{3,6}', content))
        chinese_overlap = len(answer_chinese & chunk_chinese)
        score += chinese_overlap * 2
        
        # 策略5: 关键词匹配
        for kw in keywords:
            if kw in content:
                score += 5
        
        # 策略6: 文件名相关性加成
        filename_boost = _filename_relevance_boost(
            filename, question, keywords, question_type
        )
        score *= filename_boost
        
        # 策略7: 重排序得分作为参考
        score += chunk.get("_final_score", 0) * 5
        
        if score > best_score:
            best_score = score
            best_chunk = chunk
    
    if not best_chunk:
        best_chunk = reranked_chunks[0]
    
    # 邻页回溯：如果答案中有数值，检查是否在相邻页有更好匹配
    filename = best_chunk.get("filename", "")
    page = best_chunk.get("page", 0)
    
    if answer_numbers and filename in file_chunks:
        best_num_coverage = 0
        best_page_candidate = page
        
        # 检查当前页及前后两页
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




def _post_process_answer(answer: str, question: str = "") -> str:
    """
    答案后处理 - 使格式贴近标准答案
    
    标准答案格式示例：
    - "千味央厨成立于2012年。"
    - "千味央厨预计2024年的营业收入为2,459百万元。"
    - "千味央厨的主要产品包括油炸类、烘焙类、蒸煮类、菜肴类及其他四个大类。"
    """
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
    
    # 确保以句号结尾（如果没有标点）
    if answer and not answer[-1] in '。.！!？?':
        answer += '。'
    
    # 如果答案太长（超过500字），尝试提取核心部分
    if len(answer) > 500:
        # 保留第一个完整句子或段落
        first_period = answer.find('。')
        if first_period > 0 and first_period < 300:
            # 检查是否需要更多内容
            remaining = answer[first_period+1:]
            if '、' in remaining[:100]:  # 可能有列表
                second_period = remaining.find('。')
                if second_period > 0:
                    answer = answer[:first_period+1+second_period+1]
    
    return answer


def search(question: str) -> Answer:
    """
    增强版RAG搜索函数
    
    针对评分机制优化：
    - 来源精确匹配（50分）
    - 答案语义相似度（50分）
    """
    answer = Answer(question=question)
    logger.info(f"[Enhanced] Processing: '{question[:60]}...'")

    try:
        # ===== 1. Query预处理（简化版，不用HyDE节省时间）=====
        question_type = classify_question_type(question)
        keywords = extract_keywords(question)
        hypothetical_doc = None
        
        # 仅在明确启用时使用HyDE
        if ENABLE_HYDE:
            processor = QueryProcessor(enable_hyde=True)
            query_info = processor.process(question)
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
                "num_candidates": max(VEC_CANDIDATES, 150),
                "query_vector": question_embedding,
                "similarity": float(os.getenv("SIMILARITY_THRESHOLD", "0.15")),
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

        # ===== 5. RRF融合 =====
        results_list = [vec_results, bm25_results]
        weights = [1.3, 1.0]  # 向量检索更高权重
        
        if hyde_results:
            results_list.append(hyde_results)
            weights.append(1.15)
        
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

        # ===== 8. 生成答案（根据问题类型选择prompt）=====
        # 根据问题类型选择最合适的prompt
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
        processed_answer = _post_process_answer(raw_answer)
        answer.answer = processed_answer

        # ===== 9. 增强版来源选择 =====
        best_filename, best_page = _select_best_source_enhanced(
            reranked,
            processed_answer,
            question,
            question_type,
            keywords,
        )
        
        answer.filename = best_filename
        answer.page = best_page

        logger.info(
            f"[Enhanced] Done: '{question[:35]}...' -> {best_filename}:{best_page}"
        )

    except Exception as e:
        logger.error(f"Error in enhanced search: {e}", exc_info=True)
        answer.answer = f"查询过程中发生错误：{str(e)}"

    return answer


# 兼容性别名
def search_enhanced(question: str) -> Answer:
    """增强版搜索函数（别名）"""
    return search(question)
