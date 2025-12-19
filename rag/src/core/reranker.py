"""
智能重排序模块 - 实现文件级+Chunk级双重评分
"""
import logging
import re
from collections import defaultdict
from typing import Dict, List, Optional, Tuple

logger = logging.getLogger(__name__)


def extract_numbers_from_text(text: str) -> List[str]:
    """提取文本中的数字（包括百分比、金额等）"""
    # 匹配各种数字格式
    patterns = [
        r'\d+(?:\.\d+)?%',       # 百分比
        r'\d+(?:,\d{3})*(?:\.\d+)?(?:亿|万|百万|千万)?(?:元)?',  # 金额
        r'\d{4}年',              # 年份
        r'\d+(?:\.\d+)?(?:次|倍|个)?',  # 其他数值
    ]
    
    numbers = []
    for pattern in patterns:
        matches = re.findall(pattern, text)
        numbers.extend(matches)
    
    return numbers


def extract_precise_numbers(text: str) -> set:
    """提取精确数值（用于完全匹配评分）- 优化版"""
    patterns = [
        r'\d+(?:\.\d+)?%',  # 百分比如 70.05%
        r'\d{1,3}(?:,\d{3})+(?:\.\d+)?',  # 带逗号的数字 如 2,459
        r'\d+(?:\.\d+)?(?:亿|万|百万|千万)(?:元)?',  # 金额 如 12.85亿元
        r'\d+(?:\.\d+)?次',  # 次数 如 1.05次
        r'\d{4}年',  # 年份 如 2012年
        r'\d+(?:\.\d+)?(?:百万元|万元|元)',  # 更精确的金额匹配
    ]
    numbers = set()
    for pattern in patterns:
        matches = re.findall(pattern, text)
        numbers.update(matches)
    return numbers


def calculate_keyword_density(content: str, keywords: List[str]) -> float:
    """
    计算关键词密度得分
    
    Args:
        content: 文档内容
        keywords: 关键词列表
        
    Returns:
        密度得分 (0-1)
    """
    if not keywords or not content:
        return 0.0
    
    content_lower = content.lower()
    
    # 计算关键词匹配数
    matched_keywords = 0
    total_matches = 0
    
    for kw in keywords:
        kw_lower = kw.lower()
        count = content_lower.count(kw_lower)
        if count > 0:
            matched_keywords += 1
            total_matches += min(count, 3)  # 每个关键词最多计3次
    
    # 综合得分：关键词覆盖率 * 出现频次因子
    coverage = matched_keywords / len(keywords)
    frequency_factor = min(total_matches / (len(keywords) * 2), 1.0)
    
    return (coverage * 0.7 + frequency_factor * 0.3)


def calculate_semantic_relevance(content: str, question: str) -> float:
    """
    计算语义相关性得分（基于规则的简化版本）
    
    Args:
        content: 文档内容
        question: 问题
        
    Returns:
        相关性得分 (0-1)
    """
    # 提取问题中的核心词汇
    question_words = set(re.findall(r'[\u4e00-\u9fa5]{2,}', question))
    content_words = set(re.findall(r'[\u4e00-\u9fa5]{2,}', content))
    
    if not question_words:
        return 0.5
    
    # 计算词汇重叠
    overlap = len(question_words & content_words)
    overlap_ratio = overlap / len(question_words)
    
    return min(overlap_ratio, 1.0)


def score_file_relevance(
    filename: str,
    question: str,
    question_type: str,
    keywords: List[str],
) -> float:
    """
    文件级评分 - 评估整个文件与问题的相关性
    
    评分维度：
    1. 文件名关键词匹配
    2. 报告类型权威性
    3. 时效性（根据文件名中的日期）
    4. 主题覆盖度
    
    Args:
        filename: 文件名
        question: 问题
        question_type: 问题类型
        keywords: 关键词列表
        
    Returns:
        文件级得分 (0-1)
    """
    score = 0.0
    
    # 1. 文件名关键词匹配 (权重: 0.4)
    filename_lower = filename.lower()
    keyword_match = 0
    for kw in keywords:
        if kw.lower() in filename_lower:
            keyword_match += 1
    keyword_score = min(keyword_match / max(len(keywords), 1), 1.0)
    score += keyword_score * 0.4
    
    # 2. 报告类型权威性 (权重: 0.25)
    # 深度报告 > 研究报告 > 公司报告 > 其他
    report_type_scores = {
        "深度报告": 1.0,
        "深度研究": 0.95,
        "深度分析": 0.9,
        "研究报告": 0.85,
        "公司研究": 0.8,
        "投资价值": 0.75,
        "首次覆盖": 0.7,
    }
    
    report_score = 0.5  # 默认得分
    for report_type, type_score in report_type_scores.items():
        if report_type in filename:
            report_score = type_score
            break
    score += report_score * 0.25
    
    # 3. 时效性 (权重: 0.15)
    # 提取文件名中的年份
    year_match = re.search(r'(\d{2})(\d{2})(\d{2})', filename)
    if year_match:
        year = int(year_match.group(1))
        # 假设20-25年为较新，越新越好
        if year >= 23:
            time_score = 1.0
        elif year >= 22:
            time_score = 0.8
        elif year >= 21:
            time_score = 0.6
        else:
            time_score = 0.4
    else:
        time_score = 0.5
    score += time_score * 0.15
    
    # 4. 问题类型加权 (权重: 0.2)
    # 根据问题类型调整权重
    type_bonus = 0.5
    if question_type == "numeric_fact":
        # 数值问题更倾向于深度报告和研究报告
        if "深度" in filename or "研究" in filename:
            type_bonus = 0.8
    elif question_type == "comparison":
        # 对比问题倾向于深度分析
        if "分析" in filename or "对比" in filename:
            type_bonus = 0.8
    elif question_type == "list":
        # 列表类问题可能在多种报告中
        type_bonus = 0.6
    
    score += type_bonus * 0.2
    
    return min(score, 1.0)


def score_chunk_relevance(
    content: str,
    question: str,
    question_type: str,
    keywords: List[str],
    file_score: float,
) -> float:
    """
    Chunk级评分 - 评估单个文本块与问题的相关性
    
    评分维度：
    1. 关键词密度
    2. 语义相关性
    3. 信息完整性
    4. 精确度（数值类问题）
    5. 文件级加成
    6. 【新增】问题数值精确匹配
    
    Args:
        content: Chunk内容
        question: 问题
        question_type: 问题类型
        keywords: 关键词列表
        file_score: 所属文件的得分
        
    Returns:
        Chunk级得分 (0-1)
    """
    score = 0.0
    
    # 1. 关键词密度 (权重: 0.30)
    keyword_density = calculate_keyword_density(content, keywords)
    score += keyword_density * 0.30
    
    # 2. 语义相关性 (权重: 0.20)
    semantic_score = calculate_semantic_relevance(content, question)
    score += semantic_score * 0.20
    
    # 3. 信息完整性 (权重: 0.10)
    has_complete_sentences = content.count('。') >= 1 or content.count('.') >= 1
    has_data = bool(re.search(r'\d+', content))
    completeness = (0.5 if has_complete_sentences else 0) + (0.5 if has_data else 0)
    score += completeness * 0.10
    
    # 4. 精确度 - 针对数值类问题 (权重: 0.15)
    precision_score = 0.5
    if question_type == "numeric_fact":
        numbers = extract_numbers_from_text(content)
        if numbers:
            precision_score = min(len(numbers) / 3, 1.0)
    elif question_type == "time_fact":
        years = re.findall(r'\d{4}年|\d{2}年', content)
        if years:
            precision_score = 0.9
    elif question_type == "list":
        list_markers = content.count('、') + content.count('；') + content.count('，')
        if list_markers >= 2:
            precision_score = 0.8
    score += precision_score * 0.15
    
    # 5. 【新增】问题数值精确匹配 (权重: 0.15)
    # 如果问题中有具体数值/年份，chunk中也有，则大幅加分
    question_numbers = extract_precise_numbers(question)
    if question_numbers:
        chunk_numbers = extract_precise_numbers(content)
        exact_matches = len(question_numbers & chunk_numbers)
        if exact_matches > 0:
            # 精确匹配是强信号
            score += min(exact_matches * 0.15, 0.15)
    else:
        # 没有具体数值时，按普通权重
        score += 0.075
    
    # 6. 文件级加成 (权重: 0.10)
    score += file_score * 0.10
    
    return min(score, 1.0)


class AdvancedReranker:
    """高级重排序器 - 实现双重评分和智能排序"""
    
    def __init__(self, file_weight: float = 0.4, chunk_weight: float = 0.6):
        """
        Args:
            file_weight: 文件级得分权重
            chunk_weight: Chunk级得分权重
        """
        self.file_weight = file_weight
        self.chunk_weight = chunk_weight
        # 缓存文件得分，避免重复计算
        self._file_score_cache: Dict[str, float] = {}
    
    def clear_cache(self):
        """清除文件得分缓存"""
        self._file_score_cache.clear()
    
    def rerank(
        self,
        chunks: List[Dict],
        question: str,
        question_type: str,
        keywords: List[str],
        top_k: int = 15,
    ) -> List[Dict]:
        """
        对检索结果进行重排序
        
        Args:
            chunks: 检索到的文档块列表，每个块包含 content, filename, page 等字段
            question: 问题
            question_type: 问题类型
            keywords: 关键词列表
            top_k: 返回的最大结果数
            
        Returns:
            重排序后的文档块列表
        """
        if not chunks:
            return []
        
        scored_chunks = []
        
        for chunk in chunks:
            filename = chunk.get("filename", "")
            content = chunk.get("content", "")
            
            # 计算文件级得分（使用缓存）
            if filename not in self._file_score_cache:
                file_score = score_file_relevance(
                    filename, question, question_type, keywords
                )
                self._file_score_cache[filename] = file_score
            else:
                file_score = self._file_score_cache[filename]
            
            # 计算Chunk级得分
            chunk_score = score_chunk_relevance(
                content, question, question_type, keywords, file_score
            )
            
            # 综合得分
            final_score = (
                file_score * self.file_weight + 
                chunk_score * self.chunk_weight
            )
            
            scored_chunks.append({
                **chunk,
                "_file_score": file_score,
                "_chunk_score": chunk_score,
                "_final_score": final_score,
            })
        
        # 按综合得分降序排序
        scored_chunks.sort(key=lambda x: x["_final_score"], reverse=True)
        
        return scored_chunks[:top_k]
    
    def get_best_source(
        self,
        reranked_chunks: List[Dict],
        answer: str,
        question_type: str,
    ) -> Tuple[str, int]:
        """
        从重排序结果中选择最佳来源
        
        使用多策略来源选择：
        1. 数值覆盖率优先（针对数值类问题）
        2. 关键词重叠度
        3. 重排序得分
        
        Args:
            reranked_chunks: 重排序后的文档块
            answer: LLM生成的答案
            question_type: 问题类型
            
        Returns:
            (filename, page) 元组
        """
        if not reranked_chunks:
            return ("", 0)
        
        best_chunk = reranked_chunks[0]
        best_score = 0.0
        
        # 从答案中提取数值
        answer_numbers = set(extract_numbers_from_text(answer))
        
        for chunk in reranked_chunks[:5]:  # 只检查前5个
            content = chunk.get("content", "")
            score = chunk.get("_final_score", 0)
            
            # 策略1: 数值覆盖率
            if answer_numbers and question_type in ["numeric_fact", "time_fact"]:
                chunk_numbers = set(extract_numbers_from_text(content))
                overlap = len(answer_numbers & chunk_numbers)
                num_score = overlap / len(answer_numbers) if answer_numbers else 0
                score += num_score * 0.5
            
            # 策略2: 答案关键词重叠
            answer_words = set(re.findall(r'[\u4e00-\u9fa5]{2,}', answer))
            content_words = set(re.findall(r'[\u4e00-\u9fa5]{2,}', content))
            if answer_words:
                word_overlap = len(answer_words & content_words) / len(answer_words)
                score += word_overlap * 0.3
            
            if score > best_score:
                best_score = score
                best_chunk = chunk
        
        return (
            best_chunk.get("filename", ""),
            best_chunk.get("page", 0)
        )
