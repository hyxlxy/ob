import glob
import logging
import os
import uuid
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import List

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
from src.parser.pdf import compress_image, pdf_page_to_image
from src.prompt.vlm import VLM_IMAGE_EXTRACTION_PROMPT, select_prompt_by_content
from src.split.split import split_text
from src.storage.oceanbase import get_or_create_client

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

    # 创建/确保全文索引
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
    except Exception as e:
        logger.debug(f"Create FTS index skipped or failed (may exist): {e}")


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
                if len(page_text) < 500:
                    page_image_path = pdf_page_to_image(pdf_file, page_num, dpi=100)
                    logger.debug(
                        f"Page {page_num} text len={len(page_text)} (<500), engaging VLM. Image: {page_image_path}"
                    )

                    try:
                        # 根据页面内容动态选择最匹配的VLM提示词
                        vlm_prompt = select_prompt_by_content(page_text)
                        vlm_text = generate_vlm_response(
                            vlm_prompt, images=page_image_path
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
                                    vlm_prompt,  # 使用之前选择的提示词
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
            text_chunks = split_text(page_text)

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

            for i, chunk_text in enumerate(valid_chunks):
                embedding = embeddings[i]

                # Create unique source_id
                source_id = str(uuid.uuid4())

                chunks_to_insert.append(
                    {
                        "source_id": source_id,
                        "filename": filename,
                        "page": page_num,
                        "content": chunk_text[:8190],  # Truncate if too long
                        "vector": embedding,
                    }
                )

        # Batch insert chunks
        chunks_inserted = 0
        if chunks_to_insert:
            # Insert in batches to avoid memory issues
            batch_size = 100
            for i in range(0, len(chunks_to_insert), batch_size):
                batch = chunks_to_insert[i : i + batch_size]
                client.insert(table_name=TABLE_NAME, data=batch)
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

    # Process PDF files in parallel
    total_chunks = 0
    with ThreadPoolExecutor(max_workers=max_worker) as executor:
        # Submit all PDF processing tasks
        future_to_pdf = {
            executor.submit(process_pdf_file, pdf_file, client): pdf_file
            for pdf_file in pdf_files
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
