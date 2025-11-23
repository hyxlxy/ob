import logging
import os
from collections import defaultdict
from typing import Dict, List, Tuple

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
TOP_K = int(os.getenv("TOP_K", "5"))
BM25_CANDIDATES = int(os.getenv("BM25_CANDIDATES", str(TOP_K * 4)))
VEC_CANDIDATES = int(os.getenv("VEC_CANDIDATES", str(TOP_K * 6)))
RRF_K = float(os.getenv("RRF_K", "60"))  # RRF 常数项
MAX_CONTEXT_CHARS = int(os.getenv("MAX_CONTEXT_CHARS", "6000"))


def _build_fts_query(query: str) -> Dict:
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


def _truncate_context(chunks: List[str], limit: int) -> List[str]:
    out = []
    total = 0
    for c in chunks:
        if total + len(c) <= limit:
            out.append(c)
            total += len(c)
        else:
            remain = max(0, limit - total)
            if remain > 50:  # 至少保留一些字符
                out.append(c[:remain])
                total += remain
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

        # 3) 分别执行 BM25 与 向量检索
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
        bm25_results = client.search(index=TABLE_NAME, body=bm25_request) or []
        vec_results = client.search(index=TABLE_NAME, body=vec_request) or []

        if not bm25_results and not vec_results:
            logger.warning(f"No search results found for question: '{question[:50]}...'")
            answer.answer = "抱歉，我没有找到相关的信息来回答这个问题。"
            return answer

        # 4) RRF 融合并去重
        fused = _rrf_fuse(bm25_results, vec_results, TOP_K)
        if not fused:
            logger.warning("RRF 融合结果为空，回退使用BM25或向量结果")
            fused = (bm25_results or vec_results)[:TOP_K]

        # 5) 组装上下文与元数据
        contexts: List[str] = []
        meta: List[Tuple[str, int]] = []
        seen = set()
        for r in fused:
            content = r.get("content", "").strip()
            filename = r.get("filename", "")
            page = r.get("page", 0)
            key = (filename, page)
            if content and key not in seen:
                contexts.append(content)
                meta.append(key)
                seen.add(key)

        if not contexts:
            logger.warning("No valid contexts extracted after fusion")
            answer.answer = "抱歉，我没有找到相关的信息来回答这个问题。"
            return answer

        # 控制上下文总长度，避免提示过长
        contexts = _truncate_context(contexts, MAX_CONTEXT_CHARS)
        context_text = "\n\n".join(contexts)

        # 采用首条的文件与页作为主引用
        if meta:
            answer.filename, answer.page = meta[0]

        # 6) 生成答案
        logger.debug("Generating answer using LLM model ...")
        prompt = QUERY_USER_PROMPT_TEMPLATE.format(
            context_text=context_text,
            question=question,
        )

        messages = [
            {"role": "system", "content": QUERY_SYSTEM_PROMPT},
            {"role": "user", "content": prompt},
        ]

        answer.answer = generate_llm_response(messages)
        logger.debug(
            f"Query completed: question='{question[:50]}...', filename='{answer.filename}', page={answer.page}"
        )

    except Exception as e:
        logger.error(f"Error in query: {e}", exc_info=True)
        answer.answer = f"查询过程中发生错误：{str(e)}"

    return answer
