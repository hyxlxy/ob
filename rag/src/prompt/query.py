"""Prompts for query/answering functionality."""

# System prompt for the document Q&A assistant
QUERY_SYSTEM_PROMPT = (
    "你是一个专业的文档问答助手，只输出JSON格式。"
)

# User prompt template for answering questions based on document content
# 优化：要求LLM以JSON格式输出，包含answer、filename、page三个字段
QUERY_USER_PROMPT_TEMPLATE = """基于下述文档片段回答问题。每个片段标注了【来源：文件名，第X页】。

{context_text}

问题：{question}

直接输出JSON（不要```json标记，不要任何其他文字）：
{{"answer": "简洁回答", "filename": "来源文件名.pdf", "page": 页码数字}}

关键要求：
- filename必须从上述片段的【来源：...】中完整复制，包含.pdf后缀
- page必须是整数
- 选择最直接回答问题的那个片段的来源"""
