"""Prompts for query/answering functionality."""

# System prompt for the document Q&A assistant
QUERY_SYSTEM_PROMPT = (
    "你是一个专业的文档问答助手。必须严格依据提供的文档内容作答，"
    "不要编造、不要引入未提供的外部知识；若文档缺乏答案，请明确说明无法从文档中得到答案。"
)

# User prompt template for answering questions based on document content
# 优化：要求LLM以JSON格式输出，包含answer、filename、page三个字段
QUERY_USER_PROMPT_TEMPLATE = """请仅基于下述文档片段回答用户问题。每个片段都标注了来源（文件名和页码）。

【文档片段开始】
{context_text}
【文档片段结束】

用户问题：{question}

**严格按以下JSON格式输出**（不要输出其他内容）：
```json
{{
    "answer": "你的回答内容（简洁准确，数字必须精准）",
    "filename": "答案所在的PDF文件名（必须是上述片段中标注的文件名，完整复制）",
    "page": 答案所在的页码数字
}}
```

要求：
1. answer：只使用文档片段中的信息，简洁准确，数字必须精准；
2. filename和page：必须是答案**最主要来源**的文件名和页码，从片段标注中选择；
3. 若多个片段包含答案，选择信息最完整、最直接回答问题的那个片段的来源；
4. 若文档没有相关信息，answer填"无法从提供的文档中找到答案"，filename和page使用第一个片段的来源。
"""
