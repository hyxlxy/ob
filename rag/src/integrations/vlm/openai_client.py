import logging
import os
import time
import random
from typing import List, Optional, Union

import dotenv
from openai import APIConnectionError, APIError, OpenAI, RateLimitError

from .image_utils import cleanup_temp_files, process_images_to_base64

dotenv.load_dotenv()

logger = logging.getLogger(__name__)

OPENAI_BASE_URL = os.getenv(
    "OPENAI_BASE_URL",
    "https://dashscope-intl.aliyuncs.com/compatible-mode/v1",
)

OPENAI_API_KEY = os.getenv(
    "OPENAI_API_KEY",
    "",
)

# VLM runtime configs (can be overridden by arguments)
VLM_DEFAULT_MODEL = os.getenv("VLM_MODEL", "qwen3-vl-flash")
VLM_FALLBACK_MODEL = os.getenv("VLM_FALLBACK_MODEL", "")
VLM_TEMPERATURE = float(os.getenv("VLM_TEMPERATURE", "0.2"))
VLM_TIMEOUT = float(os.getenv("VLM_TIMEOUT", "300.0"))
VLM_MAX_RETRIES = int(os.getenv("VLM_MAX_RETRIES", "3"))
VLM_BACKOFF_BASE = float(os.getenv("VLM_BACKOFF_BASE", "1.0"))
VLM_MAX_IMAGES_PER_CALL = int(os.getenv("VLM_MAX_IMAGES_PER_CALL", "2"))

# Initialize OpenAI client
vlm_client = OpenAI(
    api_key=OPENAI_API_KEY,
    base_url=OPENAI_BASE_URL,
)


def _build_messages(prompt: str, processed_images: List[str], system_prompt: Optional[str]) -> List[dict]:
    """Build OpenAI-compatible messages with optional system prompt and images."""
    content_blocks = [{"type": "text", "text": prompt}]
    for img_base64 in processed_images:
        image_url = f"data:image/png;base64,{img_base64}"
        content_blocks.append({"type": "image_url", "image_url": {"url": image_url}})

    messages: List[dict] = []
    if system_prompt:
        messages.append({"role": "system", "content": system_prompt})
    messages.append({"role": "user", "content": content_blocks})
    return messages


def _is_retryable_api_error(e: APIError) -> tuple[bool, Optional[int]]:
    status_code = None
    if hasattr(e, "status_code"):
        status_code = e.status_code
    elif hasattr(e, "response") and hasattr(e.response, "status_code"):
        status_code = e.response.status_code

    error_str = str(e).lower()
    # payload too large
    too_large = any(k in error_str for k in ["too large", "content too large", "request entity too large", "413"])
    # gateway/server errors
    retryable_by_status = (status_code is not None and (status_code >= 500 or status_code in (429, 408, 413)))
    retryable_by_text = any(
        k in error_str
        for k in [
            "connection",
            "timeout",
            "rate limit",
            "server error",
            "temporary",
            "unavailable",
            "502",
            "503",
            "504",
            "bad gateway",
            "service unavailable",
            "gateway timeout",
        ]
    )
    return (retryable_by_status or retryable_by_text or too_large, status_code)


def generate_response(
    prompt: str,
    images: Optional[Union[str, List[str]]] = None,
    model: Optional[str] = None,
    max_retries: Optional[int] = None,
    initial_backoff: Optional[float] = None,
    temperature: Optional[float] = None,
    timeout: Optional[float] = None,
    system_prompt: Optional[str] = None,
    chunk_images: bool = True,
    max_images_per_call: Optional[int] = None,
    max_tokens: Optional[int] = None,
) -> str:
    """
    调用多模态大模型（VLM）生成结果，带有健壮的错误处理、自动退避与图片分批策略。

    Args:
        prompt: 文本提示词
        images: 可选的图片路径/列表，或已是base64的图片字符串
        model: 模型名称（默认取环境变量 VLM_MODEL）
        max_retries: 最大重试次数（默认取环境变量 VLM_MAX_RETRIES）
        initial_backoff: 初始退避时间（秒，默认取 VLM_BACKOFF_BASE）
        temperature: 采样温度（默认取 VLM_TEMPERATURE）
        timeout: 请求超时（秒，默认取 VLM_TIMEOUT）
        system_prompt: 可选的系统提示词（作为 system 角色）
        chunk_images: 若图片过多则自动分批
        max_images_per_call: 每次调用最多携带的图片数（默认取 VLM_MAX_IMAGES_PER_CALL）
        max_tokens: 最大输出tokens（可选）

    Returns:
        模型输出文本。若进行了分批，将拼接所有批次的结果。
    """
    # Defaults from env
    _model = model or VLM_DEFAULT_MODEL
    _max_retries = VLM_MAX_RETRIES if max_retries is None else max_retries
    _backoff = VLM_BACKOFF_BASE if initial_backoff is None else initial_backoff
    _temperature = VLM_TEMPERATURE if temperature is None else temperature
    _timeout = VLM_TIMEOUT if timeout is None else timeout
    _max_imgs = VLM_MAX_IMAGES_PER_CALL if max_images_per_call is None else max_images_per_call

    # Normalize images parameter
    image_list: List[str] = []
    if images:
        if isinstance(images, str):
            image_list = [images]
        elif isinstance(images, list):
            image_list = images
        else:
            raise TypeError("images must be str or List[str]")

    # If too many images, split into batches
    batches: List[List[str]] = []
    if chunk_images and image_list and _max_imgs > 0 and len(image_list) > _max_imgs:
        for i in range(0, len(image_list), _max_imgs):
            batches.append(image_list[i : i + _max_imgs])
    else:
        batches = [image_list] if image_list else [[]]

    all_outputs: List[str] = []

    # Process each batch sequentially to keep order and avoid long payloads
    for batch_idx, img_batch in enumerate(batches, start=1):
        last_exception: Optional[Exception] = None
        # Pre-process images to base64 (no compression for first try)
        processed_images, temp_files_to_cleanup = process_images_to_base64(img_batch, compress_for_retry=False)

        # Build messages
        messages = _build_messages(prompt, processed_images, system_prompt)

        # Tracking if we already compressed due to a previous failure
        images_compressed = False
        # Tracking if we already switched to fallback model
        used_fallback = False

        for attempt in range(_max_retries + 1):
            try:
                response = vlm_client.chat.completions.create(
                    model=_model,
                    messages=messages,
                    temperature=_temperature,
                    timeout=_timeout,
                    max_tokens=max_tokens,
                )

                if not response.choices or not response.choices[0].message.content:
                    raise ValueError("Empty response from API")

                content = response.choices[0].message.content
                if not content or len(content.strip()) == 0:
                    raise ValueError("Empty response content from API")

                # Clean up temporary files before returning
                cleanup_temp_files(temp_files_to_cleanup)
                all_outputs.append(content)
                break  # success for this batch

            except RateLimitError as e:
                last_exception = e
                if attempt < _max_retries:
                    # Exponential backoff with jitter
                    backoff_time = _backoff * (2 ** attempt) * (1 + 0.2 * random.random())
                    logger.warning(
                        f"Rate limit exceeded. Retrying in {backoff_time:.2f}s... (attempt {attempt + 1}/{_max_retries + 1})"
                    )
                    time.sleep(backoff_time)
                else:
                    logger.error(f"Rate limit error after {_max_retries + 1} attempts")

            except APIConnectionError as e:
                last_exception = e
                if attempt < _max_retries:
                    backoff_time = _backoff * (2 ** attempt) * (1 + 0.2 * random.random())
                    logger.warning(
                        f"Connection error. Retrying in {backoff_time:.2f}s... (attempt {attempt + 1}/{_max_retries + 1})"
                    )
                    time.sleep(backoff_time)
                else:
                    logger.error(f"Connection error after {_max_retries + 1} attempts")

            except APIError as e:
                last_exception = e
                retryable, status_code = _is_retryable_api_error(e)
                error_str = str(e).lower()

                # If payload is too large or 502/5xx, try image compression once
                needs_compress = (not images_compressed) and (
                    (status_code in (502, 503, 504, 413))
                    or ("bad gateway" in error_str)
                    or ("too large" in error_str)
                )

                if needs_compress and img_batch:
                    logger.warning(
                        f"Retrying with compressed images due to error (status={status_code}): {e}"
                    )
                    try:
                        cleanup_temp_files(temp_files_to_cleanup)
                        processed_images, temp_files_to_cleanup = process_images_to_base64(
                            img_batch, compress_for_retry=True
                        )
                        messages = _build_messages(prompt, processed_images, system_prompt)
                        images_compressed = True
                        # Immediate retry without extra backoff
                        continue
                    except Exception as compress_error:
                        logger.error(
                            f"Failed to compress images for retry: {compress_error}. Proceeding with normal retry."
                        )

                # Try fallback model once at the last attempt
                if (attempt == _max_retries) and (not used_fallback) and VLM_FALLBACK_MODEL:
                    logger.warning(
                        f"Switching to fallback model '{VLM_FALLBACK_MODEL}' due to persistent error: {e}"
                    )
                    _model = VLM_FALLBACK_MODEL
                    used_fallback = True
                    # Reset attempts for fallback (one extra try)
                    # small backoff before switching
                    time.sleep(_backoff)
                    # Reset attempt loop (do one more iteration)
                    for _ in range(1):
                        try:
                            response = vlm_client.chat.completions.create(
                                model=_model,
                                messages=messages,
                                temperature=_temperature,
                                timeout=_timeout,
                                max_tokens=max_tokens,
                            )
                            if not response.choices or not response.choices[0].message.content:
                                raise ValueError("Empty response from API")
                            content = response.choices[0].message.content
                            if not content or len(content.strip()) == 0:
                                raise ValueError("Empty response content from API")
                            cleanup_temp_files(temp_files_to_cleanup)
                            all_outputs.append(content)
                            break
                        except Exception as e_fallback:
                            last_exception = e_fallback
                            logger.error(f"Fallback model call failed: {e_fallback}")
                    # End fallback try
                    break  # exit attempt loop after fallback try

                if retryable and attempt < _max_retries:
                    backoff_time = _backoff * (2 ** attempt) * (1 + 0.2 * random.random())
                    logger.warning(
                        f"Error occurred (status={status_code}): {str(e)}. Retrying in {backoff_time:.2f}s... (attempt {attempt + 1}/{_max_retries + 1})"
                    )
                    time.sleep(backoff_time)
                elif not retryable:
                    logger.error(f"Non-retryable error: {e}")
                    cleanup_temp_files(temp_files_to_cleanup)
                    raise e
                else:
                    logger.error(
                        f"Error after {_max_retries + 1} attempts (status={status_code}): {str(e)}"
                    )

            except Exception as e:
                # For unexpected errors, don't retry beyond loop
                last_exception = e
                logger.error(f"Unexpected error: {e}")
                break
        # end for attempt

        else:
            # loop exhausted without break (no success)
            if last_exception:
                cleanup_temp_files(temp_files_to_cleanup)
                raise last_exception

        # Ensure temp files are cleaned if not already
        cleanup_temp_files(temp_files_to_cleanup)
    # end for batch

    # Merge outputs if batched
    if len(all_outputs) == 1:
        return all_outputs[0]

    merged = []
    for i, out in enumerate(all_outputs, start=1):
        merged.append(f"【分批结果 {i}】\n{out}")
    return "\n\n".join(merged)


if __name__ == "__main__":
    # Text-only quick test
    result = generate_response("What is a vision language model?")
    print(f"Text-only response: {result[:200]}...")

    # Image test (uncomment and provide path)
    # result = generate_response(
    #     prompt="Describe this image in detail.",
    #     images=["path/to/image1.jpg", "path/to/image2.jpg"],
    #     system_prompt="You are an expert OCR and chart interpreter.",
    # )
    # print(result)
