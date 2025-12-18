import argparse
import logging
import os
from datetime import datetime
from typing import List

from src.core import add, search
from src.util import Answer, read_questions_json, write_answers

logger = logging.getLogger(__name__)


def parse_args():
    """
    Parse command line arguments.

    Returns:
        argparse.Namespace: Parsed arguments containing:
            - dataset: Path to dataset directory
            - questions: Path to questions/test JSON file
            - output: Path to output JSON file
    """
    parser = argparse.ArgumentParser(
        description="RAG system for question answering",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
    python3.10 main.py --dataset="./data/dataset/" \\
                       --questions="./data/questions.json" \\
                       --output="./data/output.json"
        """,
    )

    parser.add_argument(
        "--dataset",
        type=str,
        default="./data/dataset/",
        help="Path to dataset directory (default: ./data/dataset/)",
    )

    parser.add_argument(
        "--questions",
        type=str,
        default="./data/questions.json",
        help="Path to questions/test JSON file (default: ./data/questions.json)",
    )

    parser.add_argument(
        "--output",
        type=str,
        default="./data/output.json",
        help="Path to output JSON file (default: ./data/output.json)",
    )

    args = parser.parse_args()

    # Validate paths
    if not os.path.isdir(args.dataset):
        raise ValueError(f"Dataset directory does not exist: {args.dataset}")

    if not os.path.isfile(args.questions):
        raise ValueError(f"Questions file does not exist: {args.questions}")

    # Create output directory if it doesn't exist
    output_dir = os.path.dirname(args.output)
    if output_dir and not os.path.exists(output_dir):
        os.makedirs(output_dir, exist_ok=True)

    return args


if __name__ == "__main__":
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
    )

    args = parse_args()

    logger.info(f"Dataset directory: {args.dataset}")
    logger.info(f"Questions file: {args.questions}")
    logger.info(f"Output file: {args.output}")

    logger.info("Starting data preparation...")
    add(
        dataset_dir=args.dataset,
        max_worker=8,
    )
    logger.info("Data preparation completed.")

    answers: List[Answer] = []

    logger.info("Starting question answering...")
    questions: List[str] = read_questions_json(args.questions)
    logger.info(f"Loaded {len(questions)} questions from {args.questions}")

    import time
    from concurrent.futures import ThreadPoolExecutor, as_completed
    
    # 根据问题数量动态调整并行度
    # 较少问题时可以更激进，较多问题时要控制并发避免限流
    if len(questions) <= 5:
        max_workers = 2
        delay_between = 0.5
    elif len(questions) <= 15:
        max_workers = 3
        delay_between = 0.3
    else:
        max_workers = 2  # 大量问题时减少并发
        delay_between = 0.2
    
    logger.info(f"Using {max_workers} workers with {delay_between}s delay")
    
    answers = [None] * len(questions)
    
    def process_question(idx_question):
        idx, question = idx_question
        try:
            result = search(question)
            return idx, result
        except Exception as e:
            logger.error(f"Error processing question {idx}: {e}")
            return idx, Answer(
                question=question,
                answer=f"处理问题时发生错误: {str(e)}",
                filename="",
                page=0
            )
    
    # 使用 ThreadPoolExecutor 并行处理
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = []
        for idx, question in enumerate(questions):
            future = executor.submit(process_question, (idx, question))
            futures.append(future)
            # 添加小延迟避免API限流
            if idx < len(questions) - 1:
                time.sleep(delay_between)
        
        # 收集结果
        for future in as_completed(futures):
            try:
                idx, result = future.result()
                answers[idx] = result
            except Exception as e:
                logger.error(f"Future failed: {e}")
    
    # 确保所有答案都有值
    for idx, ans in enumerate(answers):
        if ans is None:
            answers[idx] = Answer(
                question=questions[idx],
                answer="未能生成答案",
                filename="",
                page=0
            )

    logger.info(f"Completed processing {len(answers)} questions.")
    logger.info(f"Writing answers to {args.output}...")
    write_answers(answers, args.output)
    logger.info("All tasks completed successfully.")
