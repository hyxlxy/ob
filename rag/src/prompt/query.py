"""Prompts for query/answering functionality - 通用优化版"""

# System prompt - 强调从文档抽取原文
QUERY_SYSTEM_PROMPT = (
    "你是专业的文档信息抽取专家。"
    "核心规则："
    "1)直接从文档中抽取答案，使用原文表述；"
    "2)数值完全精确，保留原文格式；"
    "3)回答简洁直接，以句号结尾；"
    "4)禁止添加'根据文档'等任何前缀。"
)

# 通用抽取式Prompt
QUERY_USER_PROMPT_TEMPLATE = """从文档中抽取信息回答问题。

文档：
{context_text}

问题：{question}

要求：
- 直接使用文档原文回答
- 数值保持原格式
- 简洁准确

答案："""


# 数值类问题
NUMERIC_QUESTION_PROMPT = """从文档中抽取数值信息回答问题。

文档：
{context_text}

问题：{question}

要求：
- 数值必须与文档完全一致（包括逗号、小数点、单位）
- 直接使用文档原文
- 简洁准确

答案："""


# 时间/事实类问题
TIME_FACT_PROMPT = """从文档中抽取时间或事实信息。

文档：
{context_text}

问题：{question}

要求：
- 直接给出具体时间或事实
- 使用文档原文
- 简洁准确

答案："""


# 列表类问题
LIST_QUESTION_PROMPT = """从文档中抽取列表信息。

文档：
{context_text}

问题：{question}

要求：
- 列出所有相关项目
- 使用文档原文
- 简洁完整

答案："""


# 分析类问题
ANALYSIS_QUESTION_PROMPT = """从文档中抽取分析性信息回答问题。

文档：
{context_text}

问题：{question}

要求：
- 使用文档原文表述
- 如有数据必须精确
- 简洁清晰

答案："""


# 对比类问题
COMPARISON_QUESTION_PROMPT = """从文档中抽取对比信息。

文档：
{context_text}

问题：{question}

要求：
- 明确对比双方及不同点
- 使用文档原文
- 简洁清晰

答案："""
