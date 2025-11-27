"""Prompts for query/answering functionality."""

# System prompt for the document Q&A assistant
QUERY_SYSTEM_PROMPT = (
    "你是一个专业的文档问答助手。必须严格依据提供的文档内容作答，"
    "不要编造、不要引入未提供的外部知识；若文档缺乏答案，请明确说明无法从文档中得到答案。"
    "回答必须包含明确的来源引用。"
)

# User prompt template for answering questions based on document content
QUERY_USER_PROMPT_TEMPLATE = """请仅基于下述文档片段回答用户问题：

【文档片段开始】
{context_text}
【文档片段结束】

用户问题：{question}

作答要求：
1. 只使用文档片段中的信息，保持表述准确、简洁；
2. 若文档没有相关信息，请说“无法从提供的文档中找到答案”；
3. **必须**在回答中引用相关信息来源，引用格式为：(文件名, 页码)。
   例如："千味央厨成立于2012年 (千味央厨-公司研究报告.pdf, 15)。"
   如果是多个来源，请全部列出。
"""
