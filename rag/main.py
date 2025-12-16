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
    # 允许通过环境变量配置并发度，以兼顾限速与时延
    add_workers = int(os.getenv("ADD_MAX_WORKERS", "6"))
    add(
        dataset_dir=args.dataset,
        max_worker=add_workers,
    )
    logger.info("Data preparation completed.")

    answers: List[Answer] = []

    logger.info("Starting question answering...")
    questions: List[str] = read_questions_json(args.questions)
    logger.info(f"Loaded {len(questions)} questions from {args.questions}")

    from concurrent.futures import ThreadPoolExecutor
    
    # 允许通过环境变量配置问答阶段并发度，默认4
    qa_workers = int(os.getenv("QA_MAX_WORKERS", "4"))
    with ThreadPoolExecutor(max_workers=qa_workers) as executor:
        # 使用 executor.map 保序
        results = list(executor.map(search, questions))
    
    answers = results

    logger.info(f"Completed processing {len(answers)} questions.")
    logger.info(f"Writing answers to {args.output}...")
    write_answers(answers, args.output)
    logger.info("All tasks completed successfully.")
