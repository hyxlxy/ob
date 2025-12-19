"""Prompts for Vision Language Model (VLM) functionality."""

# Prompt for extracting text from images using VLM - 针对财务报告优化
VLM_IMAGE_EXTRACTION_PROMPT = (
    "这是千味央厨公司财务报告的一页。请仔细提取以下内容：\n"
    "1. 所有数值数据，必须完全精确（如2,459百万元、12.85亿元、70.05%、1.05次）\n"
    "2. 表格内容，保持原格式，数值准确\n"
    "3. 图表数据点和趋势描述\n"
    "4. 关键时间信息（成立时间、预测年份等）\n"
    "5. 产品类别、业务模式等重要术语\n\n"
    "输出要求：\n"
    "- 数值必须一字不差，保留原文格式\n"
    "- 表格用Markdown格式输出\n"
    "- 保持简洁，只提取关键信息"
)
