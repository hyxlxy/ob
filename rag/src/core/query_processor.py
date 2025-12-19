"""
Query预处理与改写模块 - 包含HyDE、关键词提取、意图识别等功能
"""
import logging
import re
from typing import Dict, List, Optional, Tuple

from src.integrations.llm import generate_response as generate_llm_response

logger = logging.getLogger(__name__)

# 问题类型定义
QUESTION_TYPES = {
    "time_fact": "时间事实类",      # 成立时间、什么时候
    "numeric_fact": "数值事实类",   # 营业收入、多少、百分比
    "entity_fact": "实体事实类",    # 产品类别、主要产品
    "analysis": "分析性",           # 如何、为什么、原因
    "comparison": "对比性",         # 与...相比、不同
    "list": "列表类",              # 有哪些、包括哪些
}


def classify_question_type(question: str) -> str:
    """
    识别问题类型，用于后续的检索策略选择和重排序权重调整
    针对千味央厨财务报告问题优化
    
    Args:
        question: 用户问题
        
    Returns:
        问题类型标识
    """
    # 时间事实类关键词
    time_keywords = ["什么时候", "成立时间", "哪一年", "何时", "日期", "时间是"]
    
    # 数值事实类关键词 - 扩展覆盖更多数值问题
    numeric_keywords = [
        "多少", "是多少", "分别是多少",
        "营业收入", "收入是", "利润", "净利润",
        "费用率", "毛利率", "周转率", "利用率", "产能利用率",
        "百分比", "比例", "金额", "数额",
        "2018年", "2019年", "2020年", "2021年", "2022年", "2023年", "2024年",
        "预计", "预测",
    ]
    
    # 对比类关键词
    comparison_keywords = ["相比", "对比", "不同", "区别", "差异", "与.*比", "有何不同", "如何.*相比"]
    
    # 列表类关键词
    list_keywords = ["有哪些", "包括哪些", "哪些类别", "什么类别", "主要.*有", "哪些.*类"]
    
    # 分析类关键词
    analysis_keywords = ["如何实施", "为什么", "原因是什么", "主要原因", "怎样", "策略", "驱动因素", "困境是什么"]
    
    # 按优先级匹配 - 调整顺序以更准确分类
    
    # 1. 先检查是否包含具体年份+数值询问 -> 数值类
    if re.search(r'20\d{2}年.*(多少|是多少|分别)', question):
        return "numeric_fact"
    
    # 2. 时间事实类
    for kw in time_keywords:
        if kw in question:
            return "time_fact"
    
    # 3. 对比类 - 检测"有何不同"等模式
    for kw in comparison_keywords:
        if kw in question or re.search(kw, question):
            return "comparison"
    
    # 4. 列表类
    for kw in list_keywords:
        if kw in question or re.search(kw, question):
            return "list"
    
    # 5. 数值事实类
    for kw in numeric_keywords:
        if kw in question:
            return "numeric_fact"
    
    # 6. 分析类
    for kw in analysis_keywords:
        if kw in question:
            return "analysis"
    
    # 7. 默认为实体事实类
    return "entity_fact"


def extract_keywords(question: str) -> List[str]:
    """
    从问题中提取关键词，用于BM25检索和重排序
    针对千味央厨财务报告问题优化
    
    Args:
        question: 用户问题
        
    Returns:
        关键词列表（按重要性排序）
    """
    keywords = []
    
    # 1. 添加公司名
    if "千味央厨" in question:
        keywords.append("千味央厨")
    
    # 2. 提取年份（非常重要，用于定位具体数据）
    years = re.findall(r'\d{4}年', question)
    keywords.extend(years)
    
    # 3. 提取百分比和金额数值
    numbers = re.findall(r'\d+(?:\.\d+)?%|\d+(?:\.\d+)?(?:亿|万|百万)?元', question)
    keywords.extend(numbers)
    
    # 4. 提取专业术语（按优先级排序）
    # 高优先级：核心业务术语
    high_priority_terms = [
        "产能利用率", "资产周转率", "销售费用率", "管理费用率", "期间费用率",
        "财务报表预测", "收入结构", "产品线", "定制生产模式", "通用品生产模式",
        "三高一低", "预制菜市场", "餐饮供应链", "大客户优先",
        "营业收入", "毛利率", "费用率", "周转率", "净利润",
    ]
    
    # 中优先级：产品和业务术语
    medium_priority_terms = [
        "成立时间", "财务报表", "财务报告",
        "销售费用", "管理费用", "产品定价", "市场研究", "产品创新",
        "油炸类", "烘焙类", "蒸煮类", "菜肴类", "油条",
        "大客户", "小B端", "大B端", "预制菜", "速冻",
        "定制生产", "通用品", "驱动因素", "冷链物流",
    ]
    
    # 低优先级：通用术语
    low_priority_terms = [
        "产品", "类别", "餐饮", "供应链", "定价", "市场", "渠道",
        "成立", "收入", "利润", "困境", "策略", "模式", "案例",
        "创新", "场景", "细分",
    ]
    
    # 按优先级提取
    for term_list in [high_priority_terms, medium_priority_terms, low_priority_terms]:
        for term in term_list:
            if term in question and term not in keywords:
                # 检查是否已被更长的词包含
                is_subset = False
                for existing in keywords:
                    if term in existing and term != existing:
                        is_subset = True
                        break
                if not is_subset:
                    keywords.append(term)
    
    # 5. 对于特定问题模式，添加额外关键词
    if "如何" in question or "怎样" in question:
        if "实施" not in keywords:
            keywords.append("实施")
    if "不同" in question or "区别" in question:
        if "区别" not in keywords:
            keywords.append("区别")
    if "原因" in question or "为什么" in question:
        if "原因" not in keywords:
            keywords.append("原因")
    
    # 去重保持顺序
    seen = set()
    unique_keywords = []
    for kw in keywords:
        if kw not in seen:
            seen.add(kw)
            unique_keywords.append(kw)
    
    return unique_keywords[:12]  # 增加关键词数量限制


def generate_hypothetical_document(question: str, question_type: str) -> str:
    """
    HyDE (Hypothetical Document Embeddings) - 生成假设性答案文档
    
    通过让LLM生成一个假设性的答案，然后用这个答案的embedding来检索，
    可以更好地匹配文档中的内容表述方式。
    
    Args:
        question: 用户问题
        question_type: 问题类型
        
    Returns:
        假设性文档内容
    """
    # 根据问题类型定制prompt - 优化版
    type_prompts = {
        "time_fact": """问题：{question}

生成一段财务报告中关于千味央厨成立/创办时间的描述，例如：
"千味央厨成立于2012年，是一家专注于速冻面米制品的企业..."
直接输出内容：""",

        "numeric_fact": """问题：{question}

生成一段财务报告中的数值描述，要求包含具体数字和单位，例如：
"千味央厨2021年实现营业收入12.85亿元，同比增长..."
"预计2024年营业收入为2,459百万元..."
直接输出内容：""",

        "list": """问题：{question}

生成一段财务报告中关于产品类别/项目列表的描述，例如：
"千味央厨的主要产品包括油炸类、烘焙类、蒸煮类、菜肴类及其他四个大类..."
直接输出内容：""",

        "comparison": """问题：{question}

生成一段财务报告中的对比分析描述，例如：
"千味央厨的定制生产模式通过市场调研、立项等环节，而通用品生产模式则..."
直接输出内容：""",

        "analysis": """问题：{question}

生成一段财务报告中的分析性描述，包含原因、策略、驱动因素等，例如：
"千味央厨坚持大客户优先理念，快速响应需求。公司内部形成对接大客户的内部小组..."
直接输出内容：""",

        "entity_fact": """问题：{question}

生成一段财务报告中关于公司特定事项的描述，例如：
"千味央厨的产品定价主要根据成本加成法..."
直接输出内容：""",
    }
    
    prompt_template = type_prompts.get(question_type, type_prompts["entity_fact"])
    prompt = prompt_template.format(question=question)

    try:
        response = generate_llm_response(
            [{"role": "user", "content": prompt}],
            model="qwen-turbo",  # 使用更快的模型
            max_retries=2,
            timeout=20.0,
        )
        return response.strip()
    except Exception as e:
        logger.warning(f"HyDE generation failed: {e}")
        # 降级：直接返回问题本身作为查询
        return question


def rewrite_query(question: str, question_type: str) -> str:
    """
    Query改写 - 将用户问题转换为更适合检索的形式
    
    Args:
        question: 原始问题
        question_type: 问题类型
        
    Returns:
        改写后的查询
    """
    # 移除问号和其他标点
    query = question.rstrip("？?。.！!")
    
    # 添加上下文信息
    if "千味央厨" not in query:
        query = f"千味央厨{query}"
    
    return query


def expand_query_terms(question: str, keywords: List[str]) -> List[str]:
    """
    查询扩展 - 添加同义词和相关术语
    
    Args:
        question: 原始问题
        keywords: 提取的关键词
        
    Returns:
        扩展后的查询词列表
    """
    # 财务报告常见同义词映射
    synonyms = {
        "营业收入": ["营收", "收入", "销售收入", "主营业务收入"],
        "毛利率": ["毛利", "毛利润率", "销售毛利率"],
        "费用率": ["费用占比", "费用比率"],
        "产能利用率": ["产能", "利用率", "产能使用率"],
        "成立时间": ["成立", "创立", "创办", "设立"],
        "产品": ["产品线", "主营产品", "核心产品"],
        "客户": ["客户群", "客户结构", "主要客户"],
    }
    
    expanded = list(keywords)
    for kw in keywords:
        if kw in synonyms:
            expanded.extend(synonyms[kw])
    
    return list(set(expanded))


class QueryProcessor:
    """查询处理器 - 封装所有查询预处理功能"""
    
    def __init__(self, enable_hyde: bool = True):
        """
        Args:
            enable_hyde: 是否启用HyDE
        """
        self.enable_hyde = enable_hyde
    
    def process(self, question: str) -> Dict:
        """
        处理查询，返回所有预处理结果
        
        Args:
            question: 原始问题
            
        Returns:
            包含以下字段的字典:
            - question: 原始问题
            - question_type: 问题类型
            - keywords: 关键词列表
            - expanded_keywords: 扩展后的关键词
            - rewritten_query: 改写后的查询
            - hypothetical_doc: HyDE生成的假设性文档（如果启用）
        """
        # 1. 识别问题类型
        question_type = classify_question_type(question)
        logger.debug(f"Question type: {question_type}")
        
        # 2. 提取关键词
        keywords = extract_keywords(question)
        logger.debug(f"Keywords: {keywords}")
        
        # 3. 扩展关键词
        expanded_keywords = expand_query_terms(question, keywords)
        logger.debug(f"Expanded keywords: {expanded_keywords}")
        
        # 4. 改写查询
        rewritten_query = rewrite_query(question, question_type)
        logger.debug(f"Rewritten query: {rewritten_query}")
        
        # 5. 生成HyDE假设性文档
        hypothetical_doc = None
        if self.enable_hyde:
            hypothetical_doc = generate_hypothetical_document(question, question_type)
            logger.debug(f"HyDE doc length: {len(hypothetical_doc)}")
        
        return {
            "question": question,
            "question_type": question_type,
            "keywords": keywords,
            "expanded_keywords": expanded_keywords,
            "rewritten_query": rewritten_query,
            "hypothetical_doc": hypothetical_doc,
        }
