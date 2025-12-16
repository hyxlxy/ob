import glob
import logging
import os
import uuid
import hashlib
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import List, Optional
import re

import dotenv
from pyobvector import VECTOR, FtsIndexParam, FtsParser, VectorIndex
from pyobvector.client.hybrid_search import HybridSearch
from sqlalchemy import VARCHAR, Column, Integer

logger = logging.getLogger(__name__)

try:
    from pypdf import PdfReader
except ImportError:
    try:
        from PyPDF2 import PdfReader
    except ImportError:
        PdfReader = None

from src.integrations.embedding import generate_response as generate_embedding
from src.integrations.vlm import generate_response as generate_vlm_response
from src.parser.pdf import compress_image, pdf_page_to_image, extract_images_from_page
from src.prompt.vlm import VLM_IMAGE_EXTRACTION_PROMPT
from src.split.split import split_text
from src.storage.oceanbase import get_or_create_client

# --- 关键词抽取（轻量规则版，无第三方NLP依赖）---
_FIN_KWS = {
    "营业收入", "收入", "营收", "净利润", "利润", "毛利", "毛利率", "费用率", "销售费用", "管理费用",
    "资产周转", "周转率", "产能", "产能利用率", "同比", "环比", "规模", "份额", "市场份额",
    "油炸类", "烘焙类", "蒸煮类", "菜肴类", "油条", "B端", "小B", "渠道", "客户"
}
_units = ["亿元", "百万元", "万元", "亿元", "%", "pct", "万", "亿"]


def _extract_keywords(text: str, heading: str = "", limit: int = 20) -> str:
    """从正文与标题中提取轻量关键词：
    - 数值+单位（含百分比）
    - 标题拆词（长度2-12）
    - 财报领域词典
    - 高频英文/数字token
    返回用空格分隔的去重短语，最长不超过 limit 项。
    """
    try:
        s = (text or "").strip()
        h = (heading or "").strip()
        tokens = []

        # 1) 数值与单位（如 12.85亿元、35%、2021年）
        num_unit = re.findall(r"\b\d+(?:\.\d+)?\s*(?:%|pct|亿元|百万元|万元|万|亿)\b", s)
        tokens.extend(num_unit)
        years = re.findall(r"\b(20\d{2}|19\d{2})\b", s)
        tokens.extend(years)
        quarters = re.findall(r"\bQ[1-4]\b", s, flags=re.I)
        tokens.extend([q.upper() for q in quarters])

        # 2) 标题拆词（中文/英文/数字）
        if h:
            ht = re.findall(r"[\u4e00-\u9fa5A-Za-z0-9]{2,12}", h)
            tokens.extend(ht)

        # 3) 财报词典命中
        for kw in _FIN_KWS:
            if kw in s or kw in h:
                tokens.append(kw)

        # 4) 英文/数字token
        en = re.findall(r"[A-Za-z]{2,10}", s)
        tokens.extend([t.upper() for t in en])

        # 去重、裁剪
        seen = set()
        out = []
        for t in tokens:
            t = t.strip()
            if not t:
                continue
            if t in seen:
                continue
            seen.add(t)
            out.append(t)
            if len(out) >= limit:
                break
        return " ".join(out)
    except Exception:
        return ""

from src.config import RAGConfig

dotenv.load_dotenv()

# Embedding/Index 配置（可通过环境变量覆盖）
EMBEDDING_DIM = int(os.getenv("EMBEDDING_DIM", "0"))  # 0 表示自动探测
VECTOR_METRIC = os.getenv("VECTOR_METRIC", "cosine")  # l2 | ip | cosine

# Table name for storing document chunks
TABLE_NAME = "rag_documents"


def create_table_if_not_exists(client: HybridSearch, vector_dim: int):
    """Create the documents table if it doesn't exist (idempotent)."""
    logger.info(f"Ensuring table '{TABLE_NAME}' exists (dim={vector_dim})...")

    # 根据度量选择索引参数
    metric = (VECTOR_METRIC or "l2").lower()
    if metric not in {"l2", "ip", "cosine"}:
        metric = "l2"
    vec_index_param = f"distance={metric}, type=hnsw, lib=vsag"

    try:
        client.create_table(
            table_name=TABLE_NAME,
            columns=[
                Column("id", Integer, primary_key=True, autoincrement=True),
                Column("source_id", VARCHAR(64)),  # Unique identifier for each chunk
                Column("filename", VARCHAR(512)),  # PDF filename
                Column("page", Integer),  # Page number (1-indexed)
                Column("heading", VARCHAR(512)),  # Section heading/title if available
                Column("keywords", VARCHAR(1024)),  # Extracted keywords for BM25 boosting
                Column("content", VARCHAR(8192)),  # Text content
                Column("vector", VECTOR(vector_dim)),  # Embedding vector
            ],
            indexes=[
                VectorIndex("vec_idx", "vector", params=vec_index_param),
            ],
            mysql_charset="utf8mb4",
            mysql_collate="utf8mb4_unicode_ci",
            mysql_organization="heap",
        )
        logger.info(f"Table '{TABLE_NAME}' created.")
    except Exception as e:
        # 已存在时忽略
        logger.debug(f"Create table skipped or failed (may exist): {e}")

    # 创建/确保全文索引（优先 content+heading+keywords，其次 content+heading，最后 content）
    try:
        client.create_fts_idx_with_fts_index_param(
            table_name=TABLE_NAME,
            fts_idx_param=FtsIndexParam(
                index_name="fts_idx_all",
                field_names=["content", "heading", "keywords"],
                parser_type=FtsParser.IK,
            ),
        )
        logger.info("FTS index ensured: fts_idx_all (content, heading, keywords)")
    except Exception as e:
        logger.debug(f"Create FTS(all) failed, fallback to content+heading: {e}")
        try:
            client.create_fts_idx_with_fts_index_param(
                table_name=TABLE_NAME,
                fts_idx_param=FtsIndexParam(
                    index_name="fts_idx_content_heading",
                    field_names=["content", "heading"],
                    parser_type=FtsParser.IK,
                ),
            )
            logger.info("FTS index ensured: fts_idx_content_heading")
        except Exception as e2:
            logger.debug(f"Create FTS(content+heading) failed, fallback to content only: {e2}")
            try:
                client.create_fts_idx_with_fts_index_param(
                    table_name=TABLE_NAME,
                    fts_idx_param=FtsIndexParam(
                        index_name="fts_idx_content",
                        field_names=["content"],
                        parser_type=FtsParser.IK,
                    ),
                )
                logger.info("FTS index ensured: fts_idx_content")
            except Exception as e3:
                logger.debug(f"Create FTS index skipped or failed (may exist): {e3}")


def _file_already_indexed(client: HybridSearch, filename: str) -> bool:
    """检查指定文件是否已被索引，避免重复入库。
    通过 HybridSearch 的 terms 过滤按 filename 查询，若返回记录则认为已入库。
    注意：若无二级索引，该操作可能为全表扫描，但在本赛题数据量下可接受。
    """
    try:
        body = {
            "query": {
                "bool": {
                    "must": [
                        {"terms": {"filename": [filename]}}
                    ]
                }
            },
            "from": 0,
            "size": 1,
        }
        res = client.search(index=TABLE_NAME, body=body) or []
        return len(res) > 0
    except Exception as e:
        logger.debug(f"检测文件是否已入库失败，将继续处理。filename={filename}, err={e}")
        return False


def _make_source_id(filename: str, page: int, content: str) -> str:
    """基于文件名+页码+内容哈希生成稳定的 source_id，用于去重。"""
    h = hashlib.md5()
    h.update(f"{filename}#{page}#".encode("utf-8"))
    h.update(content.encode("utf-8"))
    return h.hexdigest()


def _get_existing_source_ids(client: HybridSearch, ids: List[str]) -> set:
    """批量查询已存在的 source_id，避免重复插入。"""
    existing = set()
    if not ids:
        return existing
    batch = 512
    for i in range(0, len(ids), batch):
        sub = ids[i : i + batch]
        body = {
            "query": {
                "bool": {
                    "must": [
                        {"terms": {"source_id": sub}}
                    ]
                }
            },
            "from": 0,
            "size": len(sub),
        }
        try:
            res = client.search(index=TABLE_NAME, body=body) or []
            for r in res:
                sid = r.get("source_id")
                if sid:
                    existing.add(sid)
        except Exception as e:
            logger.debug(f"batch check existing source_id failed: {e}")
    return existing


def process_pdf_file(
    pdf_file: str,
    client: HybridSearch,
) -> int:
    """
    Process a single PDF file with fixed workflow for each page:
    PDF to image -> VLM extraction -> Split -> Embed -> Store

    Args:
        pdf_file: Path to the PDF file
        client: OceanBase client for database operations

    Returns:
        Number of chunks inserted
    """
    filename = os.path.basename(pdf_file)
    logger.info(f"Processing {filename}...")

    try:
        # Get total number of pages from PDF
        try:
            if PdfReader is None:
                raise ImportError("PdfReader not available")
            reader = PdfReader(pdf_file)
            total_pages = len(reader.pages)
        except Exception as e:
            logger.error(f"Failed to read PDF {filename}: {e}")
            return 0

        logger.debug(f"  Total pages: {total_pages}")

        # Process each page with fixed workflow
        chunks_to_insert = []
        for page_num in range(1, total_pages + 1):
            page_image_path = None
            compressed_image_path = None
            page_text = ""

            try:
                # Step 1: Try direct text extraction from PDF
                try:
                    # Use the already instantiated reader
                    raw_text = reader.pages[page_num - 1].extract_text() or ""
                    page_text = raw_text.strip()
                except Exception as e:
                    logger.debug(
                        f"PdfReader extract_text failed for page {page_num}, will fallback to VLM: {e}"
                    )
                    page_text = ""

                # Step 2: Logic to determine if we need VLM
                # If text is sparse (< 500 chars), it might be a chart/image-heavy page.
                # We append VLM description to the extracted text to capture visual info.
                if len(page_text) < 500 and RAGConfig.USE_VLM: 
                    page_image_path = pdf_page_to_image(pdf_file, page_num, dpi=RAGConfig.VLM_DPI)
                    logger.debug(
                        f"Page {page_num} text len={len(page_text)} (<500), engaging VLM. Image: {page_image_path}"
                    )

                    try:
                        vlm_text = generate_vlm_response(
                            VLM_IMAGE_EXTRACTION_PROMPT, images=page_image_path
                        )
                        if vlm_text.strip():
                            logger.info(
                                f"Extracted visual info from page {page_num} using VLM"
                            )
                            if page_text:
                                page_text += f"\n\n【页面视觉内容描述】\n{vlm_text}"
                            else:
                                page_text = vlm_text
                    except Exception as e:
                        # Check if it's a 502 error (likely due to large image)
                        error_str = str(e).lower()
                        is_502_error = (
                            "502" in error_str
                            or "bad gateway" in error_str
                            or (hasattr(e, "status_code") and e.status_code == 502)
                        )

                        if is_502_error:
                            logger.warning(
                                f"502 error detected for page {page_num} of {filename}. "
                                f"Attempting to compress image and retry..."
                            )
                            try:
                                # Compress the image and retry
                                compressed_image_path = compress_image(
                                    page_image_path,
                                    max_size_mb=5.0,
                                    max_dimension=2048,
                                )
                                vlm_text = generate_vlm_response(
                                    VLM_IMAGE_EXTRACTION_PROMPT,
                                    images=compressed_image_path,
                                )
                                if vlm_text.strip():
                                    logger.info(
                                        f"Successfully extracted text from compressed page {page_num} image"
                                    )
                                    if page_text:
                                        page_text += f"\n\n【页面视觉内容描述】\n{vlm_text}"
                                    else:
                                        page_text = vlm_text
                            except Exception as retry_e:
                                logger.warning(
                                    f"Failed to extract text from compressed page {page_num} image of {filename}: {retry_e}"
                                )
                        else:
                            logger.warning(
                                f"Failed to extract text from page {page_num} image of {filename}: {e}"
                            )
                    finally:
                         # Clean up temporary image files
                        if compressed_image_path and os.path.exists(compressed_image_path):
                            try:
                                os.unlink(compressed_image_path)
                            except Exception as e:
                                logger.debug(f"Failed to delete compressed image: {e}")
                        if page_image_path and os.path.exists(page_image_path):
                            try:
                                os.unlink(page_image_path)
                            except Exception as e:
                                logger.debug(f"Failed to delete page image: {e}")

            except Exception as e:
                logger.warning(
                    f"Failed to process page {page_num} of {filename}: {e}"
                )


            # Skip if no content extracted
            if not page_text.strip():
                logger.debug(f"No content extracted from page {page_num}, skipping")
                continue

            # Step 3: Split text
            text_chunks = split_text(
                page_text,
                max_chunk_size=RAGConfig.MAX_CHUNK_SIZE,
                overlap=RAGConfig.CHUNK_OVERLAP,
                add_heading_prefix=RAGConfig.ADD_HEADING_PREFIX,
            )

            # Step 4 & 5: Embed -> Store
            # Filter valid chunks
            valid_chunks = [c for c in text_chunks if c.strip()]
            if not valid_chunks:
                continue

            try:
                # Batch generate embeddings
                embeddings = generate_embedding(valid_chunks)
            except Exception as e:
                logger.warning(
                    f"Failed to generate embeddings for page {page_num} of {filename}: {e}"
                )
                continue
            
            # Verify embedding count matches chunk count
            if len(embeddings) != len(valid_chunks):
                 logger.warning(
                    f"Embedding count mismatch for page {page_num} of {filename}: expected {len(valid_chunks)}, got {len(embeddings)}"
                )
                 continue

            import re as _re
            for i, chunk_text in enumerate(valid_chunks):
                embedding = embeddings[i]

                # Extract heading prefix like: 【标题】xxx\n
                heading_val = ""
                _m = _re.match(r"^【标题】(.+?)\n(.*)$", chunk_text, flags=_re.S)
                if _m:
                    heading_val = (_m.group(1) or "").strip()[:510]
                    chunk_body = (_m.group(2) or "").strip()
                else:
                    chunk_body = chunk_text

                # Create stable source_id based on filename+page+content (without heading)
                sid = _make_source_id(filename, page_num, chunk_body)

                # Extract keywords
                kws = _extract_keywords(chunk_body, heading_val)

                chunks_to_insert.append(
                    {
                        "source_id": sid,
                        "filename": filename,
                        "page": page_num,
                        "heading": heading_val,
                        "keywords": kws[:1020] if kws else "",
                        "content": chunk_body[:8190],  # Truncate if too long
                        "vector": embedding,
                    }
                )

        # 去重：过滤已存在的 source_id
        if chunks_to_insert:
            sids = [c["source_id"] for c in chunks_to_insert]
            existed = _get_existing_source_ids(client, sids)
            if existed:
                before = len(chunks_to_insert)
                chunks_to_insert = [c for c in chunks_to_insert if c["source_id"] not in existed]
                after = len(chunks_to_insert)
                logger.info(
                    f"Dedup by source_id for {filename}: {before-after} already exist, {after} to insert"
                )

        # Batch insert chunks
        chunks_inserted = 0
        if chunks_to_insert:
            # Insert in batches to avoid memory issues
            batch_size = 100
            for i in range(0, len(chunks_to_insert), batch_size):
                batch = chunks_to_insert[i : i + batch_size]
                try:
                    client.insert(table_name=TABLE_NAME, data=batch)
                except Exception as e:
                    # 兼容老表无 heading/keywords 列的情况：逐级回退
                    logger.warning(f"Insert failed, will try fallback without heading/keywords. err={e}")
                    try:
                        batch_wo_both = [
                            {k: v for k, v in item.items() if k not in ("heading", "keywords")}
                            for item in batch
                        ]
                        client.insert(table_name=TABLE_NAME, data=batch_wo_both)
                    except Exception as e2:
                        logger.error(f"Retry insert without heading/keywords failed: {e2}")
                        raise e2
                chunks_inserted += len(batch)
                logger.debug(
                    f"Inserted {len(batch)} chunks from {filename} (total: {chunks_inserted})"
                )
            logger.info(
                f"Completed processing {filename}: inserted {chunks_inserted} chunks"
            )

        return chunks_inserted

    except Exception as e:
        logger.error(f"Error processing {filename}: {e}", exc_info=True)
        return 0


def _detect_embedding_dim() -> int:
    """调用一次嵌入接口以探测维度。"""
    try:
        vec = generate_embedding("dim_probe")
        if isinstance(vec, list) and len(vec) > 0:
            return len(vec)
    except Exception as e:
        logger.warning(f"自动探测嵌入维度失败，将回退至默认1024: {e}")
    return 1024


def add(dataset_dir: str, max_worker: int):
    """
    Prepare RAG data by parsing PDFs, generating embeddings, and storing in OceanBase.

    Args:
        dataset_dir: Directory containing PDF files
        max_worker: Number of parallel workers for processing PDF files (default: 2)
    """
    pdf_files: List[str] = glob.glob(os.path.join(dataset_dir, "*.pdf"))

    if not pdf_files:
        logger.warning(f"No PDF files found in {dataset_dir}")
        return

    logger.info(f"Found {len(pdf_files)} PDF files")

    # Initialize OceanBase client
    client = get_or_create_client()

    # Use embedding dimension from environment variable or auto detect
    vector_dim = EMBEDDING_DIM if EMBEDDING_DIM > 0 else _detect_embedding_dim()
    logger.info(f"Using embedding vector dimension: {vector_dim}")

    create_table_if_not_exists(client, vector_dim)

    # 过滤已入库文件，避免重复导入
    files_to_process: List[str] = []
    for f in pdf_files:
        name = os.path.basename(f)
        if _file_already_indexed(client, name):
            logger.info(f"Skip already indexed file: {name}")
        else:
            files_to_process.append(f)

    if not files_to_process:
        logger.info("No new PDF files to index. Skipping add phase.")
        return

    logger.info(f"Processing {len(files_to_process)} new PDF files ...")

    # Process PDF files in parallel
    total_chunks = 0
    with ThreadPoolExecutor(max_workers=max_worker) as executor:
        # Submit all PDF processing tasks
        future_to_pdf = {
            executor.submit(process_pdf_file, pdf_file, client): pdf_file
            for pdf_file in files_to_process
        }

        # Collect results as they complete
        for future in as_completed(future_to_pdf):
            pdf_file = future_to_pdf[future]
            try:
                chunks_inserted = future.result()
                total_chunks += chunks_inserted
            except Exception as e:
                filename = os.path.basename(pdf_file)
                logger.error(
                    f"Exception occurred while processing {filename}: {e}",
                    exc_info=True,
                )

    logger.info(f"Preparation complete! Total chunks inserted: {total_chunks}")
