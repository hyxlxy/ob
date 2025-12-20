"""Prompts for query/answering functionality."""

# System prompt for the document Q&A assistant
QUERY_SYSTEM_PROMPT = (
    "你是一个专业的财务报告分析助手，专注于千味央厨相关文档的问答。"
    "你必须严格基于提供的文档片段回答问题，不能编造信息。"
    "你的输出必须是纯JSON格式，不包含任何markdown标记或其他文字。"
)

# User prompt template for answering questions based on document content
QUERY_USER_PROMPT_TEMPLATE = """你需要基于以下文档片段回答问题。每个片段都标注了【片段X｜来源：文件名，第Y页】。

文档片段：
{context_text}

问题：{question}

回答要求：
1. 仔细阅读所有片段，找到最直接回答问题的信息
2. 如果问题涉及具体数字、时间、比例等，必须精确引用原文数据
3. 答案要简洁准确，直接回答问题核心
4. 必须选择最相关片段的来源作为答案来源
5. 如果多个片段都相关，优先选择信息最完整、最直接的那个

输出格式（纯JSON，不要任何markdown标记）：
{{"answer": "你的答案", "filename": "完整文件名.pdf", "page": 页码}}

注意事项：
- filename必须从上述【来源：...】中完整复制，包含.pdf后缀和所有字符
- page必须是整数
- answer要准确、简洁，优先使用原文表述
- 如果涉及数字，保持原文格式（如"12.85亿元"、"2,459百万元"等）

现在请回答："""
