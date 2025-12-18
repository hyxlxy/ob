#!/usr/bin/env python3
"""
快速单问题测试脚本 - 用于验证RAG优化效果

用法:
    cd rag && python3 test_single.py "千味央厨的成立时间是什么时候？"
    cd rag && python3 test_single.py  # 测试所有示例问题
"""
import argparse
import json
import logging
import sys
import os

# 添加项目根目录到路径
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from src.core import search
from src.core.query_processor import classify_question_type, extract_keywords

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)


def test_single_question(question: str, show_details: bool = True):
    """测试单个问题"""
    print(f"\n{'='*60}")
    print(f"问题: {question}")
    print(f"{'='*60}")
    
    # 分析问题类型
    q_type = classify_question_type(question)
    keywords = extract_keywords(question)
    
    if show_details:
        print(f"问题类型: {q_type}")
        print(f"关键词: {keywords}")
    
    # 执行搜索
    result = search(question)
    
    print(f"\n答案: {result.answer}")
    print(f"来源: {result.filename}")
    print(f"页码: {result.page}")
    
    return result


def compare_with_answer(result, answer_data: dict):
    """与标准答案对比"""
    print(f"\n--- 对比标准答案 ---")
    print(f"标准答案: {answer_data['answer']}")
    print(f"标准文件: {answer_data['filename']}")
    print(f"标准页码: {answer_data['page']}")
    
    filename_match = result.filename == answer_data['filename']
    page_match = result.page == answer_data['page'] if filename_match else False
    
    print(f"\n文件匹配: {'✓' if filename_match else '✗'}")
    print(f"页码匹配: {'✓' if page_match else '✗'}")
    
    # 估算得分
    score = 0
    if filename_match:
        score += 25
    if page_match:
        score += 25
    print(f"来源得分: {score}/50")
    
    return filename_match, page_match


def main():
    parser = argparse.ArgumentParser(description="RAG单问题测试")
    parser.add_argument("question", nargs="?", help="要测试的问题")
    parser.add_argument("--all", action="store_true", help="测试所有示例问题")
    parser.add_argument("--compare", action="store_true", help="与标准答案对比")
    args = parser.parse_args()
    
    # 加载标准答案用于对比
    answers_map = {}
    try:
        with open("./data/answer.json", "r", encoding="utf-8") as f:
            answers = json.load(f)
            answers_map = {a["question"]: a for a in answers}
    except:
        pass
    
    if args.question:
        # 测试单个问题
        result = test_single_question(args.question)
        if args.compare and args.question in answers_map:
            compare_with_answer(result, answers_map[args.question])
    elif args.all:
        # 测试所有问题
        try:
            with open("./data/questions.json", "r", encoding="utf-8") as f:
                questions = json.load(f)
        except:
            print("无法加载 questions.json")
            return
        
        total_filename_match = 0
        total_page_match = 0
        
        for q in questions:
            question = q["question"]
            result = test_single_question(question, show_details=False)
            
            if question in answers_map:
                fm, pm = compare_with_answer(result, answers_map[question])
                if fm:
                    total_filename_match += 1
                if pm:
                    total_page_match += 1
        
        print(f"\n{'='*60}")
        print(f"总计: {len(questions)} 个问题")
        print(f"文件匹配: {total_filename_match}/{len(questions)}")
        print(f"页码匹配: {total_page_match}/{len(questions)}")
    else:
        # 测试几个示例问题
        example_questions = [
            "千味央厨的成立时间是什么时候？",
            "千味央厨的主要产品有哪些类别？",
            "千味央厨在2021年的营业收入是多少？",
        ]
        
        for q in example_questions:
            result = test_single_question(q)
            if q in answers_map:
                compare_with_answer(result, answers_map[q])


if __name__ == "__main__":
    main()
