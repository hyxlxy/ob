"""Prompts for Vision Language Model (VLM) functionality."""

# Prompt for extracting text from images using VLM
VLM_IMAGE_EXTRACTION_PROMPT = (
    "请仔细阅读这张图片中的所有文字内容，包括图表、表格、标题、正文等，并将关键文字内容简洁地提取出来。"
    "如果图片中包含表格，请以Markdown格式的表格形式输出，确保数值准确。"
    "如果图片中包含图表（如柱状图、折线图、饼图等），请详细描述图表中的趋势、关键数据点和图例含义。"
    "请保持输出精简，避免冗余描述，只提取必要信息。"
)
