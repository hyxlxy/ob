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
    
    Args:
        question: 用户问题
        
    Returns:
        关键词列表
    """
    keywords = []
    
    # 1. 添加特定实体（如公司名）
    if "千味央厨" in question:
        keywords.append("千味央厨")
    
    # 2. 提取数字（包括年份、百分比等）
    numbers = re.findall(r'\d{4}年|\d+(?:\.\d+)?%|\d+(?:\.\d+)?(?:亿|万|百万)?元?', question)
    keywords.extend(numbers)
    
    # 3. 提取专业术语（预定义的重要词汇，按长度降序排列避免重复）
    important_terms = [
        # 长词优先
        "产能利用率", "资产周转率", "销售费用率", "管理费用率",
        "财务报表预测", "收入结构", "产品线", "定制生产", "通用品生产",
        "三高一低", "预制菜市场", "餐饮供应链",
        # 中等长度
        "营业收入", "毛利率", "费用率", "周转率", "成立时间", "财务报表",
        "销售费用", "管理费用", "产品定价", "市场研究", "产品创新",
        "油炸类", "烘焙类", "蒸煮类", "菜肴类",
        "大客户", "小B端", "大B端", "预制菜", "速冻",
        "定制生产", "通用品", "驱动因素",
        # 短词
        "产品", "类别", "餐饮", "供应链", "定价", "市场", "渠道",
        "成立", "收入", "利润", "困境", "策略", "模式",
    ]
    
    for term in important_terms:
        if term in question and term not in keywords:
            # 检查是否已被更长的词包含
            is_subset = False
            for existing in keywords:
                if term in existing and term != existing:
                    is_subset = True
                    break
            if not is_subset:
                keywords.append(term)
    
    # 去重
    seen = set()
    unique_keywords = []
    for kw in keywords:
        if kw not in seen:
            seen.add(kw)
            unique_keywords.append(kw)
    
    return unique_keywords[:8]  # 限制关键词数量


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
    # 根据问题类型定制prompt
    type_hints = {
        "time_fact": "请直接给出具体的时间/日期信息。",
        "numeric_fact": "请给出具体的数值数据，包括单位。",
        "entity_fact": "请列出相关的实体/名称/类别。",
        "analysis": "请从原因、方式、策略等角度进行分析说明。",
        "comparison": "请对比说明不同点和相同点。",
        "list": "请以列表形式列出所有相关项目。",
    }
    
    hint = type_hints.get(question_type, "")
    
    prompt = f"""你是一个专业的财务报告分析师，正在回答关于"千味央厨"公司的问题。

问题：{question}

请根据你对速冻食品行业和千味央厨公司的了解，生成一段可能出现在财务分析报告中的、回答这个问题的文字内容。
{hint}

要求：
1. 内容要像真实的财务报告或研报中的表述
2. 包含具体的数据、时间或事实（可以是假设的）
3. 使用专业的财务/商业术语
4. 长度控制在100-200字

直接输出内容，不要添加任何解释。"""

    try:
        response = generate_llm_response(
            [{"role": "user", "content": prompt}],
            model="qwen-plus",
            max_retries=2,
            timeout=30.0,
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
