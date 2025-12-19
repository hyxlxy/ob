"""
LLM关键词提取模块 - 借鉴 LightRAG 的高/低级关键词提取策略
用于改进检索时的 query-document 对齐
"""
import json
import logging
import re
from typing import Dict, List, Optional, Tuple

from src.integrations.llm import generate_response

logger = logging.getLogger(__name__)

# LLM 关键词提取 Prompt（参考 LightRAG）
KEYWORD_EXTRACTION_PROMPT = """---角色---
你是一个专业的关键词提取专家，专门为 RAG 检索系统分析用户查询。你的任务是从用户查询中提取两类关键词：

1. **high_level_keywords**: 高层次概念或主题，捕捉用户的核心意图、主题领域或问题类型
2. **low_level_keywords**: 具体实体或细节，识别具体的实体、专有名词、技术术语、产品名称或具体项目

---约束---
1. **输出格式**: 必须输出有效的 JSON 对象，不要包含任何解释文字或 markdown 代码块
2. **来源**: 所有关键词必须从用户查询中显式提取
3. **简洁有意义**: 关键词应该是简洁的词语或有意义的短语
4. **语言**: 所有提取的关键词必须是中文，专有名词保留原文

---示例---

查询: "千味央厨2024年的预计营业收入是多少？"

输出:
{{"high_level_keywords": ["财务预测", "营业收入", "年度数据"], "low_level_keywords": ["千味央厨", "2024年", "营业收入"]}}

查询: "千味央厨的主要产品有哪些类别？"

输出:
{{"high_level_keywords": ["产品分类", "主营业务"], "low_level_keywords": ["千味央厨", "产品类别", "主要产品"]}}

---实际数据---
用户查询: {query}

---输出---
"""


def extract_keywords_with_llm(
    question: str,
    model: str = "qwen-turbo",
    timeout: float = 30.0,
) -> Dict[str, List[str]]:
    """
    使用 LLM 提取高/低级关键词
    
    Args:
        question: 用户问题
        model: LLM 模型名称
        timeout: 超时时间
        
    Returns:
        包含 high_level_keywords 和 low_level_keywords 的字典
    """
    prompt = KEYWORD_EXTRACTION_PROMPT.format(query=question)
    
    try:
        response = generate_response(
            [{"role": "user", "content": prompt}],
            model=model,
            max_retries=2,
            timeout=timeout,
        )
        
        # 清理响应并解析 JSON
        response = response.strip()
        # 移除可能的 markdown 代码块
        if response.startswith("```"):
            response = re.sub(r'^```(?:json)?\s*', '', response)
            response = re.sub(r'\s*```$', '', response)
        
        result = json.loads(response)
        
        return {
            "high_level_keywords": result.get("high_level_keywords", []),
            "low_level_keywords": result.get("low_level_keywords", []),
        }
        
    except json.JSONDecodeError as e:
        logger.warning(f"Failed to parse LLM keyword response: {e}")
        return {"high_level_keywords": [], "low_level_keywords": []}
    except Exception as e:
        logger.warning(f"LLM keyword extraction failed: {e}")
        return {"high_level_keywords": [], "low_level_keywords": []}


def extract_keywords_rule_based(question: str) -> Dict[str, List[str]]:
    """
    基于规则的关键词提取（作为 LLM 的降级方案）
    
    Args:
        question: 用户问题
        
    Returns:
        包含 high_level_keywords 和 low_level_keywords 的字典
    """
    high_level = []
    low_level = []
    
    # 高层次概念映射
    high_level_patterns = {
        r'财务|收入|利润|毛利|费用': '财务指标',
        r'预测|预计|展望': '财务预测',
        r'产品|类别|分类': '产品分类',
        r'成立|创立|创办': '公司历史',
        r'客户|渠道|市场': '市场渠道',
        r'产能|生产|工厂': '生产能力',
        r'对比|比较|差异': '对比分析',
        r'原因|为什么|如何': '原因分析',
    }
    
    for pattern, concept in high_level_patterns.items():
        if re.search(pattern, question):
            high_level.append(concept)
    
    # 低层次实体提取
    # 1. 公司名
    if "千味央厨" in question:
        low_level.append("千味央厨")
    
    # 2. 年份
    years = re.findall(r'20\d{2}年?', question)
    low_level.extend(years)
    
    # 3. 数值类关键词
    numeric_terms = [
        "营业收入", "净利润", "毛利率", "费用率", "周转率",
        "产能利用率", "销售费用", "管理费用",
    ]
    for term in numeric_terms:
        if term in question:
            low_level.append(term)
    
    # 4. 产品类别
    product_terms = ["油炸类", "烘焙类", "蒸煮类", "菜肴类"]
    for term in product_terms:
        if term in question:
            low_level.append(term)
    
    # 5. 其他实体
    entity_terms = [
        "大B端", "小B端", "预制菜", "速冻", "餐饮供应链",
        "定制生产", "通用品",
    ]
    for term in entity_terms:
        if term in question:
            low_level.append(term)
    
    return {
        "high_level_keywords": list(set(high_level)),
        "low_level_keywords": list(set(low_level)),
    }


def extract_keywords_hybrid(
    question: str,
    use_llm: bool = True,
    model: str = "qwen-turbo",
) -> Dict[str, List[str]]:
    """
    混合关键词提取：LLM + 规则
    
    Args:
        question: 用户问题
        use_llm: 是否使用 LLM 提取
        model: LLM 模型名称
        
    Returns:
        合并后的关键词字典
    """
    # 规则提取（基础）
    rule_keywords = extract_keywords_rule_based(question)
    
    if not use_llm:
        return rule_keywords
    
    # LLM 提取（增强）
    try:
        llm_keywords = extract_keywords_with_llm(question, model=model)
        
        # 合并去重
        high_level = list(set(
            rule_keywords["high_level_keywords"] + 
            llm_keywords.get("high_level_keywords", [])
        ))
        low_level = list(set(
            rule_keywords["low_level_keywords"] + 
            llm_keywords.get("low_level_keywords", [])
        ))
        
        return {
            "high_level_keywords": high_level,
            "low_level_keywords": low_level,
        }
        
    except Exception as e:
        logger.warning(f"Hybrid keyword extraction fallback to rule-based: {e}")
        return rule_keywords


def build_hybrid_query(
    question: str,
    keywords: Dict[str, List[str]],
) -> Tuple[str, str]:
    """
    基于关键词构建混合检索查询
    
    Args:
        question: 原始问题
        keywords: 提取的关键词
        
    Returns:
        (向量查询文本, BM25 查询文本) 元组
    """
    high_level = keywords.get("high_level_keywords", [])
    low_level = keywords.get("low_level_keywords", [])
    
    # 向量查询：使用高层概念扩展原问题
    vector_query = question
    if high_level:
        vector_query = f"{question} {' '.join(high_level[:3])}"
    
    # BM25 查询：使用低层实体构建精确匹配
    bm25_terms = low_level[:5] if low_level else [question]
    bm25_query = " ".join(bm25_terms)
    
    return vector_query, bm25_query


def generate_virtual_qa_query(
    question: str,
    question_type: str,
) -> str:
    """
    生成虚拟 QA 查询（用于改进 query-document 对齐）
    将用户问题转换为更接近文档表述方式的查询
    
    Args:
        question: 原始问题
        question_type: 问题类型
        
    Returns:
        虚拟查询文本
    """
    # 问题类型到文档表述的映射
    type_templates = {
        "time_fact": "{entity}成立于{placeholder}年，{entity}创立时间是{placeholder}",
        "numeric_fact": "{entity}的{metric}为{placeholder}，{metric}达到{placeholder}",
        "list": "{entity}的{category}包括{items}，主要{category}有{items}",
        "entity_fact": "{entity}的{attribute}是{placeholder}",
        "analysis": "{entity}的{topic}分析，{topic}的主要因素",
        "comparison": "{entity}与{other}的对比，{entity}和{other}的区别",
    }
    
    # 提取问题中的实体和指标
    entity = "千味央厨" if "千味央厨" in question else ""
    
    # 根据问题类型生成虚拟查询
    template = type_templates.get(question_type, "{entity} {question}")
    
    # 简单的模板填充
    virtual_query = question  # 默认使用原问题
    
    if question_type == "time_fact":
        virtual_query = f"{entity}成立时间 {entity}创立于"
    elif question_type == "numeric_fact":
        # 提取指标名称
        metrics = ["营业收入", "净利润", "毛利率", "费用率", "周转率", "产能利用率"]
        for metric in metrics:
            if metric in question:
                virtual_query = f"{entity} {metric} 预计 预测 达到"
                break
    elif question_type == "list":
        virtual_query = f"{entity} 产品 类别 包括 主要"
    
    return virtual_query


class EnhancedKeywordExtractor:
    """增强型关键词提取器"""
    
    def __init__(
        self,
        use_llm: bool = True,
        llm_model: str = "qwen-turbo",
    ):
        self.use_llm = use_llm
        self.llm_model = llm_model
        self._cache: Dict[str, Dict] = {}
    
    def extract(self, question: str) -> Dict[str, List[str]]:
        """提取关键词（带缓存）"""
        if question in self._cache:
            return self._cache[question]
        
        result = extract_keywords_hybrid(
            question,
            use_llm=self.use_llm,
            model=self.llm_model,
        )
        
        self._cache[question] = result
        return result
    
    def build_queries(
        self,
        question: str,
        question_type: str,
    ) -> Dict[str, str]:
        """
        构建多种检索查询
        
        Returns:
            包含 vector_query, bm25_query, virtual_qa_query 的字典
        """
        keywords = self.extract(question)
        vector_query, bm25_query = build_hybrid_query(question, keywords)
        virtual_qa = generate_virtual_qa_query(question, question_type)
        
        return {
            "original": question,
            "vector_query": vector_query,
            "bm25_query": bm25_query,
            "virtual_qa_query": virtual_qa,
            "keywords": keywords,
        }


if __name__ == "__main__":
    # 测试
    questions = [
        "千味央厨的成立时间是什么时候？",
        "千味央厨的财务报表预测中，预计2024年的营业收入是多少？",
        "千味央厨的主要产品有哪些类别？",
    ]
    
    extractor = EnhancedKeywordExtractor(use_llm=False)  # 测试时不用 LLM
    
    for q in questions:
        print(f"\n问题: {q}")
        result = extractor.extract(q)
        print(f"高层关键词: {result['high_level_keywords']}")
        print(f"低层关键词: {result['low_level_keywords']}")
