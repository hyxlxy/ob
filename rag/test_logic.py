#!/usr/bin/env python3
"""
逻辑测试脚本 - 测试问题分类、关键词提取等逻辑（不依赖数据库）
"""
import sys
import os
import re

# 直接导入需要测试的函数（避免导入数据库相关模块）

# ========== 从 query_processor.py 复制的逻辑 ==========

def classify_question_type(question: str) -> str:
    """识别问题类型"""
    # 时间事实类关键词
    time_keywords = ["什么时候", "成立时间", "哪一年", "何时", "日期", "时间是"]
    
    # 数值事实类关键词
    numeric_keywords = [
        "多少", "是多少", "分别是多少",
        "营业收入", "收入是", "利润", "净利润",
        "费用率", "毛利率", "周转率", "利用率", "产能利用率",
        "百分比", "比例", "金额", "数额",
        "2018年", "2019年", "2020年", "2021年", "2022年", "2023年", "2024年",
        "预计", "预测",
    ]
    
    comparison_keywords = ["相比", "对比", "不同", "区别", "差异", "与.*比", "有何不同"]
    list_keywords = ["有哪些", "包括哪些", "哪些类别", "什么类别", "主要.*有", "哪些.*类"]
    analysis_keywords = ["如何实施", "为什么", "原因是什么", "主要原因", "怎样", "策略", "驱动因素", "困境是什么"]
    
    # 1. 年份+数值询问
    if re.search(r'20\d{2}年.*(多少|是多少|分别)', question):
        return "numeric_fact"
    
    # 2. 时间事实类
    for kw in time_keywords:
        if kw in question:
            return "time_fact"
    
    # 3. 对比类
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
    
    return "entity_fact"


def extract_keywords(question: str) -> list:
    """提取关键词"""
    keywords = []
    
    if "千味央厨" in question:
        keywords.append("千味央厨")
    
    numbers = re.findall(r'\d{4}年|\d+(?:\.\d+)?%|\d+(?:\.\d+)?(?:亿|万|百万)?元?', question)
    keywords.extend(numbers)
    
    important_terms = [
        "产能利用率", "资产周转率", "销售费用率", "管理费用率",
        "财务报表预测", "收入结构", "产品线", "定制生产", "通用品生产",
        "三高一低", "预制菜市场", "餐饮供应链",
        "营业收入", "毛利率", "费用率", "周转率", "成立时间", "财务报表",
        "油炸类", "烘焙类", "蒸煮类", "菜肴类",
        "大客户", "小B端", "大B端", "预制菜", "速冻",
        "定制生产", "通用品", "驱动因素",
        "产品", "类别", "餐饮", "定价", "市场", "渠道",
        "成立", "收入", "利润", "困境", "策略", "模式",
    ]
    
    for term in important_terms:
        if term in question and term not in keywords:
            is_subset = False
            for existing in keywords:
                if term in existing and term != existing:
                    is_subset = True
                    break
            if not is_subset:
                keywords.append(term)
    
    return keywords[:8]


# ========== 测试 ==========

# 测试问题列表
test_questions = [
    "千味央厨的成立时间是什么时候？",
    "千味央厨的财务报表预测中，预计2024年的营业收入是多少？",
    "千味央厨的主要产品有哪些类别？",
    "千味央厨的资产周转率和毛利率与同行业相比如何？其期间费用率表现如何？",
    "千味央厨在2021年的营业收入是多少？",
    "千味央厨的销售费用率和管理费用率如何，主要原因是什么？",
    "千味央厨的产品定价主要采用什么方法？",
    "千味央厨在2021年的产品线和收入结构是怎样的？",
    "千味央厨的定制生产模式与通用品生产模式有何不同？",
    "千味央厨在油条细分场景下的市场研究和产品创新有哪些具体案例？",
    "千味央厨的大客户优先策略如何实施？",
    "千味央厨在中国预制菜市场的驱动因素是什么？",
    "千味央厨面临的餐饮业'三高一低'困境是什么？",
    "千味央厨的产能利用率在2018年和2021年分别是多少？",
]

# 期望的问题类型
expected_types = {
    "千味央厨的成立时间是什么时候？": "time_fact",
    "千味央厨的财务报表预测中，预计2024年的营业收入是多少？": "numeric_fact",
    "千味央厨的主要产品有哪些类别？": "list",
    "千味央厨在2021年的营业收入是多少？": "numeric_fact",
    "千味央厨的定制生产模式与通用品生产模式有何不同？": "comparison",
    "千味央厨的产能利用率在2018年和2021年分别是多少？": "numeric_fact",
}


def main():
    print("=" * 70)
    print("RAG 逻辑测试 - 问题分类与关键词提取")
    print("=" * 70)
    
    correct = 0
    total_expected = len(expected_types)
    
    for q in test_questions:
        q_type = classify_question_type(q)
        keywords = extract_keywords(q)
        
        # 检查是否有期望类型
        expected = expected_types.get(q)
        status = ""
        if expected:
            if q_type == expected:
                status = "✓"
                correct += 1
            else:
                status = f"✗ (期望: {expected})"
        
        print(f"\n问题: {q[:50]}...")
        print(f"  类型: {q_type} {status}")
        print(f"  关键词: {keywords}")
    
    print("\n" + "=" * 70)
    print(f"问题分类准确率: {correct}/{total_expected} ({100*correct/total_expected:.1f}%)")
    print("=" * 70)


if __name__ == "__main__":
    main()
