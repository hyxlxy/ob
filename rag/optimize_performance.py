#!/usr/bin/env python3
"""
RAG 系统性能优化和测试脚本

这个脚本可以帮助你：
1. 验证配置
2. 测试不同的参数组合
3. 评估性能指标
4. 生成优化建议
"""

import json
import logging
import os
import sys
import time
from typing import Dict, List, Tuple

import dotenv

# 设置日志
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
)
logger = logging.getLogger(__name__)

dotenv.load_dotenv()

from src.config import RAGConfig
from src.core.search import search
from src.util import read_questions_json


def print_header(title: str):
    """打印标题"""
    print("\n" + "=" * 70)
    print(f"  {title}")
    print("=" * 70)


def validate_config():
    """验证配置"""
    print_header("配置验证")
    
    if not RAGConfig.validate():
        logger.error("配置验证失败")
        return False
    
    logger.info("✓ 配置验证通过")
    RAGConfig.print_config()
    return True


def test_single_query(question: str) -> Tuple[str, float]:
    """测试单个查询"""
    start_time = time.time()
    try:
        answer = search(question)
        elapsed = time.time() - start_time
        return answer.answer, elapsed
    except Exception as e:
        logger.error(f"查询失败：{e}")
        return "", 0.0


def benchmark_queries(questions_file: str, num_queries: int = 5) -> Dict:
    """对查询进行基准测试"""
    print_header("查询性能基准测试")
    
    try:
        questions = read_questions_json(questions_file)
    except Exception as e:
        logger.error(f"无法读取问题文件：{e}")
        return {}
    
    if not questions:
        logger.warning("问题列表为空")
        return {}
    
    # 限制查询数量
    test_questions = questions[:min(num_queries, len(questions))]
    
    logger.info(f"将测试 {len(test_questions)} 个查询")
    
    times = []
    for i, question in enumerate(test_questions, 1):
        logger.info(f"[{i}/{len(test_questions)}] 测试查询：{question[:50]}...")
        answer, elapsed = test_single_query(question)
        times.append(elapsed)
        logger.info(f"  耗时：{elapsed:.2f}s")
    
    # 计算统计信息
    avg_time = sum(times) / len(times) if times else 0
    min_time = min(times) if times else 0
    max_time = max(times) if times else 0
    
    results = {
        "num_queries": len(test_questions),
        "avg_time": avg_time,
        "min_time": min_time,
        "max_time": max_time,
        "total_time": sum(times),
        "queries_per_second": len(test_questions) / sum(times) if sum(times) > 0 else 0,
    }
    
    print_header("基准测试结果")
    print(f"查询数量：{results['num_queries']}")
    print(f"平均耗时：{results['avg_time']:.2f}s")
    print(f"最小耗时：{results['min_time']:.2f}s")
    print(f"最大耗时：{results['max_time']:.2f}s")
    print(f"总耗时：{results['total_time']:.2f}s")
    print(f"吞吐量：{results['queries_per_second']:.2f} queries/s")
    
    return results


def generate_recommendations():
    """生成优化建议"""
    print_header("优化建议")
    
    recommendations = []
    
    # 检查 TOP_K
    if RAGConfig.TOP_K > 20:
        recommendations.append(
            f"⚠ TOP_K={RAGConfig.TOP_K} 较大，可能影响性能。建议降低到 15。"
        )
    
    # 检查候选数
    if RAGConfig.BM25_CANDIDATES > RAGConfig.TOP_K * 5:
        recommendations.append(
            f"⚠ BM25_CANDIDATES={RAGConfig.BM25_CANDIDATES} 较大，可能影响性能。"
            f"建议设置为 TOP_K * 3-4 = {RAGConfig.TOP_K * 3}-{RAGConfig.TOP_K * 4}。"
        )
    
    if RAGConfig.VEC_CANDIDATES > RAGConfig.TOP_K * 8:
        recommendations.append(
            f"⚠ VEC_CANDIDATES={RAGConfig.VEC_CANDIDATES} 较大，可能影响性能。"
            f"建议设置为 TOP_K * 5-6 = {RAGConfig.TOP_K * 5}-{RAGConfig.TOP_K * 6}。"
        )
    
    # 检查文本块大小
    if RAGConfig.MAX_CHUNK_SIZE < 500:
        recommendations.append(
            f"⚠ MAX_CHUNK_SIZE={RAGConfig.MAX_CHUNK_SIZE} 较小，可能导致语义不完整。"
            f"建议增加到 1000-1500。"
        )
    
    if RAGConfig.MAX_CHUNK_SIZE > 2000:
        recommendations.append(
            f"⚠ MAX_CHUNK_SIZE={RAGConfig.MAX_CHUNK_SIZE} 较大，可能影响检索精度。"
            f"建议降低到 1200-1500。"
        )
    
    # 检查重叠
    if RAGConfig.CHUNK_OVERLAP < 50:
        recommendations.append(
            f"⚠ CHUNK_OVERLAP={RAGConfig.CHUNK_OVERLAP} 较小，可能导致上下文丢失。"
            f"建议增加到 100-200。"
        )
    
    # 检查并发度
    if RAGConfig.ADD_MAX_WORKERS > 16:
        recommendations.append(
            f"⚠ ADD_MAX_WORKERS={RAGConfig.ADD_MAX_WORKERS} 较大，可能导致资源竞争。"
            f"建议降低到 4-8。"
        )
    
    if RAGConfig.QA_MAX_WORKERS > 8:
        recommendations.append(
            f"⚠ QA_MAX_WORKERS={RAGConfig.QA_MAX_WORKERS} 较大，可能导致资源竞争。"
            f"建议降低到 2-4。"
        )
    
    if not recommendations:
        print("✓ 配置看起来不错，没有明显的优化建议。")
    else:
        for i, rec in enumerate(recommendations, 1):
            print(f"{i}. {rec}")
    
    return recommendations


def profile_memory_usage():
    """分析内存使用"""
    print_header("内存使用分析")
    
    try:
        import psutil
        process = psutil.Process(os.getpid())
        
        # 获取内存信息
        mem_info = process.memory_info()
        print(f"RSS 内存：{mem_info.rss / 1024 / 1024:.2f} MB")
        print(f"VMS 内存：{mem_info.vms / 1024 / 1024:.2f} MB")
        
        # 获取 CPU 信息
        cpu_percent = process.cpu_percent(interval=1)
        print(f"CPU 使用率：{cpu_percent:.2f}%")
        
    except ImportError:
        logger.warning("需要安装 psutil 来分析内存使用：pip install psutil")


def main():
    """主函数"""
    print_header("RAG 系统性能优化工具")
    
    # 1. 验证配置
    if not validate_config():
        sys.exit(1)
    
    # 2. 生成优化建议
    generate_recommendations()
    
    # 3. 内存分析
    profile_memory_usage()
    
    # 4. 性能基准测试（如果提供了问题文件）
    questions_file = "./data/questions.json"
    if os.path.exists(questions_file):
        try:
            benchmark_queries(questions_file, num_queries=3)
        except Exception as e:
            logger.error(f"基准测试失败：{e}")
    else:
        logger.warning(f"问题文件不存在：{questions_file}")
    
    print_header("优化工具完成")


if __name__ == "__main__":
    main()

