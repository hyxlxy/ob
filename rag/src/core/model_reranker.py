"""
基于模型的重排序模块 - 使用阿里云 gte-rerank API
参考 LightRAG 的 rerank 实现，支持长文档分块重排序
"""
import asyncio
import logging
import os
from typing import Any, Dict, List, Optional, Tuple

import aiohttp
import dotenv
from tenacity import (
    retry,
    retry_if_exception_type,
    stop_after_attempt,
    wait_exponential,
)

dotenv.load_dotenv()

logger = logging.getLogger(__name__)

# 阿里云重排序 API 配置
DASHSCOPE_API_KEY = os.getenv("OPENAI_API_KEY", "")
RERANK_BASE_URL = os.getenv(
    "RERANK_BASE_URL",
    "https://dashscope.aliyuncs.com/api/v1/services/rerank/text-rerank/text-rerank"
)
RERANK_MODEL = os.getenv("RERANK_MODEL", "gte-rerank")


def chunk_documents_for_rerank(
    documents: List[str],
    max_tokens: int = 480,
    overlap_tokens: int = 32,
) -> Tuple[List[str], List[int]]:
    """
    将超长文档分块用于重排序

    Args:
        documents: 文档列表
        max_tokens: 每块最大 token 数
        overlap_tokens: 块之间重叠的 token 数

    Returns:
        (分块后的文档列表, 原始文档索引映射)
    """
    if overlap_tokens >= max_tokens:
        overlap_tokens = max(0, max_tokens - 1)
        logger.warning(f"overlap_tokens clamped to {overlap_tokens}")

    # 使用字符近似估计（1 token ≈ 2 中文字符或 4 英文字符）
    max_chars = max_tokens * 2
    overlap_chars = overlap_tokens * 2

    chunked_docs = []
    doc_indices = []

    for idx, doc in enumerate(documents):
        if len(doc) <= max_chars:
            chunked_docs.append(doc)
            doc_indices.append(idx)
        else:
            # 分块处理
            start = 0
            while start < len(doc):
                end = min(start + max_chars, len(doc))
                chunk = doc[start:end]
                chunked_docs.append(chunk)
                doc_indices.append(idx)

                if end >= len(doc):
                    break
                start = end - overlap_chars

    return chunked_docs, doc_indices


def aggregate_chunk_scores(
    chunk_results: List[Dict[str, Any]],
    doc_indices: List[int],
    num_original_docs: int,
    aggregation: str = "max",
) -> List[Dict[str, Any]]:
    """
    将分块的重排序得分聚合回原始文档

    Args:
        chunk_results: 分块的重排序结果
        doc_indices: 分块到原始文档的索引映射
        num_original_docs: 原始文档数量
        aggregation: 聚合策略 ("max", "mean", "first")

    Returns:
        原始文档的重排序结果
    """
    doc_scores: Dict[int, List[float]] = {i: [] for i in range(num_original_docs)}

    for result in chunk_results:
        chunk_idx = result["index"]
        score = result["relevance_score"]

        if 0 <= chunk_idx < len(doc_indices):
            original_doc_idx = doc_indices[chunk_idx]
            doc_scores[original_doc_idx].append(score)

    aggregated_results = []
    for doc_idx, scores in doc_scores.items():
        if not scores:
            continue

        if aggregation == "max":
            final_score = max(scores)
        elif aggregation == "mean":
            final_score = sum(scores) / len(scores)
        elif aggregation == "first":
            final_score = scores[0]
        else:
            final_score = max(scores)

        aggregated_results.append({
            "index": doc_idx,
            "relevance_score": final_score,
        })

    aggregated_results.sort(key=lambda x: x["relevance_score"], reverse=True)
    return aggregated_results


@retry(
    stop=stop_after_attempt(3),
    wait=wait_exponential(multiplier=1, min=2, max=30),
    retry=(
        retry_if_exception_type(aiohttp.ClientError)
        | retry_if_exception_type(aiohttp.ClientResponseError)
    ),
)
async def ali_rerank_async(
    query: str,
    documents: List[str],
    top_n: Optional[int] = None,
    api_key: Optional[str] = None,
    model: str = "gte-rerank",
    base_url: Optional[str] = None,
    enable_chunking: bool = True,
    max_tokens_per_doc: int = 480,
) -> List[Dict[str, Any]]:
    """
    使用阿里云 DashScope API 进行文档重排序

    Args:
        query: 查询文本
        documents: 待重排序的文档列表
        top_n: 返回的最大结果数
        api_key: API 密钥
        model: 重排序模型名称
        base_url: API 端点
        enable_chunking: 是否启用长文档分块
        max_tokens_per_doc: 每个文档的最大 token 数

    Returns:
        重排序结果列表，每个元素包含 index 和 relevance_score
    """
    if not documents:
        return []

    api_key = api_key or DASHSCOPE_API_KEY
    base_url = base_url or RERANK_BASE_URL

    if not api_key:
        logger.warning("No API key for reranking, returning original order")
        return [{"index": i, "relevance_score": 1.0 - i * 0.01} for i in range(len(documents))]

    headers = {
        "Content-Type": "application/json",
        "Authorization": f"Bearer {api_key}",
    }

    # 处理长文档分块
    original_documents = documents
    doc_indices = None
    original_top_n = top_n

    if enable_chunking:
        documents, doc_indices = chunk_documents_for_rerank(
            documents, max_tokens=max_tokens_per_doc
        )
        logger.debug(f"Chunked {len(original_documents)} docs into {len(documents)} chunks")
        # 分块时禁用 API 级别的 top_n
        if top_n is not None:
            top_n = None

    # 阿里云格式的请求体
    payload = {
        "model": model,
        "input": {
            "query": query,
            "documents": documents,
        },
        "parameters": {},
    }

    if top_n is not None:
        payload["parameters"]["top_n"] = top_n

    logger.debug(f"Rerank request: {len(documents)} documents, model: {model}")

    async with aiohttp.ClientSession() as session:
        async with session.post(base_url, headers=headers, json=payload) as response:
            if response.status != 200:
                error_text = await response.text()
                logger.error(f"Rerank API error {response.status}: {error_text[:200]}")
                # 返回原始顺序作为降级
                return [{"index": i, "relevance_score": 1.0 - i * 0.01} for i in range(len(original_documents))]

            response_json = await response.json()

            # 阿里云格式：{"output": {"results": [...]}}
            results = response_json.get("output", {}).get("results", [])

            if not results:
                logger.warning("Rerank API returned empty results")
                return [{"index": i, "relevance_score": 1.0 - i * 0.01} for i in range(len(original_documents))]

            # 标准化返回格式
            standardized_results = [
                {"index": result["index"], "relevance_score": result["relevance_score"]}
                for result in results
            ]

            # 如果启用了分块，聚合得分
            if enable_chunking and doc_indices:
                standardized_results = aggregate_chunk_scores(
                    standardized_results,
                    doc_indices,
                    len(original_documents),
                    aggregation="max",
                )
                # 应用原始 top_n 限制
                if original_top_n is not None and len(standardized_results) > original_top_n:
                    standardized_results = standardized_results[:original_top_n]

            return standardized_results


def ali_rerank_sync(
    query: str,
    documents: List[str],
    top_n: Optional[int] = None,
    api_key: Optional[str] = None,
    model: str = "gte-rerank",
    enable_chunking: bool = True,
    max_tokens_per_doc: int = 480,
) -> List[Dict[str, Any]]:
    """
    同步版本的阿里云重排序 API 调用
    """
    try:
        loop = asyncio.get_event_loop()
    except RuntimeError:
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)

    return loop.run_until_complete(
        ali_rerank_async(
            query=query,
            documents=documents,
            top_n=top_n,
            api_key=api_key,
            model=model,
            enable_chunking=enable_chunking,
            max_tokens_per_doc=max_tokens_per_doc,
        )
    )


def model_rerank_chunks(
    query: str,
    chunks: List[Dict],
    top_n: Optional[int] = None,
    enable_model_rerank: bool = True,
) -> List[Dict]:
    """
    对检索到的 chunks 进行模型重排序

    Args:
        query: 查询文本
        chunks: 检索到的文档块列表
        top_n: 返回的最大结果数
        enable_model_rerank: 是否启用模型重排序

    Returns:
        重排序后的文档块列表
    """
    if not chunks or not enable_model_rerank:
        return chunks[:top_n] if top_n else chunks

    # 提取文档内容用于重排序
    documents = [chunk.get("content", "") for chunk in chunks]

    try:
        # 调用重排序 API
        rerank_results = ali_rerank_sync(
            query=query,
            documents=documents,
            top_n=top_n,
            enable_chunking=True,
            max_tokens_per_doc=480,
        )

        # 根据重排序结果重新排列 chunks
        reranked_chunks = []
        for result in rerank_results:
            idx = result["index"]
            if 0 <= idx < len(chunks):
                chunk = chunks[idx].copy()
                chunk["_model_rerank_score"] = result["relevance_score"]
                reranked_chunks.append(chunk)

        logger.info(f"Model reranking completed: {len(chunks)} -> {len(reranked_chunks)} chunks")
        return reranked_chunks

    except Exception as e:
        logger.error(f"Model reranking failed: {e}")
        return chunks[:top_n] if top_n else chunks


if __name__ == "__main__":
    # 测试
    import asyncio

    async def test():
        query = "千味央厨的成立时间是什么时候？"
        documents = [
            "千味央厨成立于2012年，总部位于郑州。",
            "公司主要产品包括油炸类、烘焙类、蒸煮类等速冻食品。",
            "千味央厨2021年营业收入达到12.85亿元。",
        ]

        results = await ali_rerank_async(query, documents, top_n=3)
        print("Rerank results:")
        for r in results:
            print(f"  Index: {r['index']}, Score: {r['relevance_score']:.4f}")
            print(f"  Doc: {documents[r['index']][:50]}...")

    asyncio.run(test())
