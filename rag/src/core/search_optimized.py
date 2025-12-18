"""
优化后的搜索模块 - 整合Query预处理、多路检索、智能重排序、来源定位优化
"""
import logging
import os
import re
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import Dict, List, Optional, Tuple

import dotenv

from src.core.query_processor import QueryProcessor, classify_question_type, extract_keywords
from src.core.reranker import AdvancedReranker, extract_numbers_from_text
from src.integrations.embedding import generate_response as generate_embedding
from src.integrations.llm import generate_response as generate_llm_response
from src.prompt import (
    QUERY_SYSTEM_PROMPT, 
    QUERY_USER_PROMPT_TEMPLATE,
    NUMERIC_QUESTION_PROMPT,
    LIST_QUESTION_PROMPT,
    ANALYSIS_QUESTION_PROMPT,
)
from src.storage.oceanbase import get_or_create_client
from src.util import Answer

logger = logging.getLogger(__name__)

dotenv.load_dotenv()

# Table name for storing document chunks
TABLE_NAME = "rag_documents"

# Retrieval parameters (configurable via env)
TOP_K = int(os.getenv("TOP_K", "20"))
BM25_CANDIDATES = int(os.getenv("BM25_CANDIDATES", str(TOP_K * 4)))
VEC_CANDIDATES = int(os.getenv("VEC_CANDIDATES", str(TOP_K * 6)))
RRF_K = float(os.getenv("RRF_K", "60"))
MAX_CONTEXT_CHARS = int(os.getenv("MAX_CONTEXT_CHARS", "24000"))

# 是否启用HyDE
ENABLE_HYDE = os.getenv("ENABLE_HYDE", "true").lower() == "true"


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
                        "minimum_should_match": "30%",
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
    """
    使用倒数排序融合(RRF)合并多路检索结果
    
    Args:
        results_list: 多路检索结果列表
        top_k: 返回的最大结果数
        weights: 各路检索的权重（可选）
        
    Returns:
        融合后的结果列表
    """
    def _key(d: Dict) -> str:
        return d.get("source_id") or f"{d.get('filename','')}#{d.get('page',0)}#{hash(d.get('content',''))}"

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

    # 按得分排序
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
                total += remain
            elif not out:
                out.append(c[:limit] + "...")
                total += limit
            break
    return out


def _keyword_search(client, keywords: List[str], limit: int = 20) -> List[Dict]:
    """
    关键词LIKE检索 - 作为补充检索手段
    
    Args:
        client: 数据库客户端
        keywords: 关键词列表
        limit: 返回的最大结果数
        
    Returns:
        检索结果列表
    """
    if not keywords:
        return []
    
    try:
        # 构建LIKE查询条件
        conditions = []
        for kw in keywords[:5]:  # 限制关键词数量
            conditions.append(f"content LIKE '%{kw}%'")
        
        where_clause = " OR ".join(conditions)
        
        # 使用原生SQL查询
        from sqlalchemy import text
        
        sql = f"""
        SELECT source_id, filename, page, content 
        FROM {TABLE_NAME} 
        WHERE {where_clause}
        LIMIT {limit}
        """
        
        engine = client.perform_raw_text_sql
        results = engine(text(sql))
        
        return [
            {
                "source_id": row[0],
                "filename": row[1],
                "page": row[2],
                "content": row[3],
            }
            for row in results
        ]
    except Exception as e:
        logger.debug(f"Keyword search failed: {e}")
        return []


def pick_best_source(
    reranked_chunks: List[Dict],
    answer: str,
    question: str,
    question_type: str,
    keywords: List[str],
) -> Tuple[str, int]:
    """
    多策略来源选择 - 确保页面匹配准确
    
    策略优先级：
    1. 数值完全匹配（最高优先级）
    2. 关键词高度重叠
    3. 重排序Top1
    
    同时支持邻页回溯，处理跨页内容
    
    Args:
        reranked_chunks: 重排序后的文档块
        answer: LLM生成的答案
        question: 原始问题
        question_type: 问题类型
        keywords: 关键词列表
        
    Returns:
        (filename, page) 元组
    """
    if not reranked_chunks:
        return ("", 0)
    
    # 从答案中提取关键信息
    answer_numbers = extract_numbers_from_text(answer)
    answer_words = set(re.findall(r'[\u4e00-\u9fa5]{2,4}', answer))
    
    # 按文件组织chunks，用于邻页回溯
    file_chunks: Dict[str, List[Dict]] = defaultdict(list)
    for chunk in reranked_chunks:
        filename = chunk.get("filename", "")
        file_chunks[filename].append(chunk)
    
    best_match = None
    best_score = -1
    
    for chunk in reranked_chunks[:10]:  # 检查前10个
        content = chunk.get("content", "")
        filename = chunk.get("filename", "")
        page = chunk.get("page", 0)
        
        score = 0.0
        
        # 策略1: 数值覆盖率（最重要）
        if answer_numbers:
            chunk_numbers = extract_numbers_from_text(content)
            matched = sum(1 for n in answer_numbers if n in chunk_numbers)
            num_score = matched / len(answer_numbers)
            score += num_score * 50  # 高权重
        
        # 策略2: 关键词重叠
        content_words = set(re.findall(r'[\u4e00-\u9fa5]{2,4}', content))
        word_overlap = len(answer_words & content_words)
        score += word_overlap * 2
        
        # 策略3: 问题关键词匹配
        for kw in keywords:
            if kw in content:
                score += 3
        
        # 策略4: 重排序得分加成
        score += chunk.get("_final_score", 0) * 10
        
        if score > best_score:
            best_score = score
            best_match = (filename, page)
    
    # 邻页回溯：检查最佳匹配的前后页
    if best_match and answer_numbers:
        filename, page = best_match
        candidates = [(filename, page)]
        
        # 添加前后页
        for p in [page - 1, page + 1]:
            if p > 0:
                candidates.append((filename, p))
        
        # 在候选页中找数值覆盖率最高的
        best_coverage = 0
        best_candidate = best_match
        
        for fname, p in candidates:
            for chunk in file_chunks.get(fname, []):
                if chunk.get("page") == p:
                    content = chunk.get("content", "")
                    chunk_numbers = extract_numbers_from_text(content)
                    matched = sum(1 for n in answer_numbers if n in chunk_numbers)
                    coverage = matched / len(answer_numbers) if answer_numbers else 0
                    
                    if coverage > best_coverage:
                        best_coverage = coverage
                        best_candidate = (fname, p)
        
        return best_candidate
    
    return best_match or (reranked_chunks[0].get("filename", ""), reranked_chunks[0].get("page", 0))


def post_process_answer(answer: str, question_type: str) -> str:
    """
    答案后处理 - 确保格式符合评分标准
    
    Args:
        answer: LLM生成的原始答案
        question_type: 问题类型
        
    Returns:
        处理后的答案
    """
    # 移除来源引用（评分系统单独处理）
    # 格式如：(文件名, 页码) 或 （文件名，页码）
    answer = re.sub(r'[（\(][^）\)]*\.pdf[^）\)]*[）\)]', '', answer)
    
    # 移除冗余开头
    redundant_starts = [
        "根据文档，", "根据提供的文档，", "根据文档内容，",
        "根据资料，", "根据提供的资料，",
        "文档显示，", "从文档中可以看出，",
    ]
    for start in redundant_starts:
        if answer.startswith(start):
            answer = answer[len(start):]
            break
    
    # 确保答案简洁
    answer = answer.strip()
    
    # 移除多余的换行
    answer = re.sub(r'\n{2,}', '\n', answer)
    
    return answer


def search(question: str) -> Answer:
    """
    优化后的RAG查询函数
    
    整合了以下优化：
    1. Query预处理与改写（HyDE、关键词提取、意图识别）
    2. 多路检索策略（向量+BM25+关键词）
    3. 智能重排序（文件级+Chunk级双重评分）
    4. 多策略来源选择（确保页面匹配准确）
    5. 答案后处理
    
    Args:
        question: 用户问题
        
    Returns:
        Answer对象
    """
    answer = Answer(question=question)
    logger.info(f"Processing query: '{question[:60]}...'")

    try:
        # ===== 1. Query预处理 =====
        processor = QueryProcessor(enable_hyde=ENABLE_HYDE)
        query_info = processor.process(question)
        
        question_type = query_info["question_type"]
        keywords = query_info["keywords"]
        hypothetical_doc = query_info.get("hypothetical_doc")
        
        logger.debug(f"Question type: {question_type}, Keywords: {keywords[:5]}")

        # ===== 2. 生成查询向量 =====
        question_embedding = generate_embedding(question)
        
        # HyDE: 生成假设性文档的embedding
        hyde_embedding = None
        if hypothetical_doc:
            try:
                hyde_embedding = generate_embedding(hypothetical_doc)
            except Exception as e:
                logger.warning(f"Failed to generate HyDE embedding: {e}")

        # ===== 3. 初始化客户端和重排序器 =====
        client = get_or_create_client()
        reranker = AdvancedReranker()

        # ===== 4. 多路检索 =====
        fts_query = _build_fts_query(question)
        
        # BM25检索请求
        bm25_request = {
            "query": fts_query,
            "from": 0,
            "size": BM25_CANDIDATES,
        }
        
        # 原始查询向量检索
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
        
        # HyDE向量检索请求
        hyde_request = None
        if hyde_embedding:
            hyde_request = {
                "knn": {
                    "field": "vector",
                    "k": VEC_CANDIDATES // 2,
                    "num_candidates": max(VEC_CANDIDATES // 2, 64),
                    "query_vector": hyde_embedding,
                    "similarity": float(os.getenv("SIMILARITY_THRESHOLD", "0.2")),
                },
                "from": 0,
                "size": VEC_CANDIDATES // 2,
            }

        logger.debug(f"Searching: BM25={BM25_CANDIDATES}, VEC={VEC_CANDIDATES}, TOP_K={TOP_K}")
        
        # 并行执行多路检索
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
            logger.warning(f"No search results found for: '{question[:50]}...'")
            answer.answer = "抱歉，我没有找到相关的信息来回答这个问题。"
            return answer

        # ===== 5. RRF融合 =====
        # 权重：原始向量检索 > HyDE > BM25
        results_list = [vec_results, bm25_results]
        weights = [1.2, 1.0]
        
        if hyde_results:
            results_list.append(hyde_results)
            weights.append(1.1)
        
        fused = _rrf_fuse(results_list, TOP_K * 2, weights)
        
        if not fused:
            logger.warning("RRF fusion returned empty, falling back")
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
            key = (filename, page, content[:100])  # 使用内容前100字符作为去重key
            
            if content and key not in seen:
                # 添加来源标注
                source_tag = f"[来源: {filename}, 第{page}页]"
                contexts.append(f"{source_tag}\n{content}")
                seen.add(key)

        if not contexts:
            logger.warning("No valid contexts after reranking")
            answer.answer = "抱歉，我没有找到相关的信息来回答这个问题。"
            return answer

        # 控制上下文长度
        contexts = _truncate_context(contexts, MAX_CONTEXT_CHARS)
        context_text = "\n\n".join(contexts)

        # ===== 8. 生成答案 =====
        logger.debug(f"Generating answer using LLM... (type: {question_type})")
        
        # 根据问题类型选择合适的Prompt
        if question_type in ["numeric_fact", "time_fact"]:
            prompt_template = NUMERIC_QUESTION_PROMPT
        elif question_type == "list":
            prompt_template = LIST_QUESTION_PROMPT
        elif question_type in ["analysis", "comparison"]:
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
        
        # ===== 9. 答案后处理 =====
        processed_answer = post_process_answer(raw_answer, question_type)
        answer.answer = processed_answer

        # ===== 10. 来源定位 =====
        best_filename, best_page = pick_best_source(
            reranked,
            processed_answer,
            question,
            question_type,
            keywords,
        )
        
        answer.filename = best_filename
        answer.page = best_page

        logger.info(
            f"Query completed: question='{question[:40]}...', "
            f"source='{best_filename}', page={best_page}"
        )

    except Exception as e:
        logger.error(f"Error in search: {e}", exc_info=True)
        answer.answer = f"查询过程中发生错误：{str(e)}"

    return answer


# 保持兼容性的别名
def search_optimized(question: str) -> Answer:
    """优化后的搜索函数（别名）"""
    return search(question)
