"""
增强型文档处理模块 - 支持多粒度分片和文档摘要
实现 Raptor 式的父子分片结构，提升检索召回率
"""
import hashlib
import logging
import re
from typing import Dict, List, Optional, Tuple

from src.integrations.llm import generate_response

logger = logging.getLogger(__name__)


# 文档摘要生成 Prompt
DOCUMENT_SUMMARY_PROMPT = """请为以下财务报告内容生成一个简洁的摘要（100-150字）：

文档来源：{filename}
页码范围：{page_range}

内容：
{content}

要求：
1. 保留关键数值和时间信息
2. 保留公司名称和重要实体
3. 概括主要内容和结论
4. 使用客观的第三人称表述

摘要："""


# 虚拟 QA 生成 Prompt
VIRTUAL_QA_PROMPT = """基于以下文档内容，生成3-5个可能的问题-答案对。

文档内容：
{content}

要求：
1. 问题应该是用户可能会问的自然问题
2. 答案应该简洁，直接从文档中提取
3. 优先生成关于数值、时间、实体的问题
4. 输出格式：每行一个QA对，格式为"Q: 问题 | A: 答案"

问题答案对："""


def generate_chunk_id(content: str, filename: str, page: int) -> str:
    """生成文档块的唯一 ID"""
    text = f"{filename}:{page}:{content[:100]}"
    return hashlib.md5(text.encode()).hexdigest()[:16]


def extract_key_entities(text: str) -> List[str]:
    """从文本中提取关键实体"""
    entities = []
    
    # 公司名
    companies = re.findall(r'千味央厨|郑州千味|千味食品', text)
    entities.extend(set(companies))
    
    # 产品类别
    products = re.findall(r'(油炸类|烘焙类|蒸煮类|菜肴类|速冻食品|预制菜)', text)
    entities.extend(set(products))
    
    # 财务指标
    metrics = re.findall(r'(营业收入|净利润|毛利率|费用率|周转率|产能利用率)', text)
    entities.extend(set(metrics))
    
    return list(set(entities))


def extract_key_numbers(text: str) -> List[str]:
    """从文本中提取关键数值"""
    numbers = []
    
    # 年份
    years = re.findall(r'20\d{2}年', text)
    numbers.extend(years)
    
    # 百分比
    percentages = re.findall(r'\d+(?:\.\d+)?%', text)
    numbers.extend(percentages)
    
    # 金额
    amounts = re.findall(r'\d+(?:\.\d+)?(?:亿|万|百万)(?:元)?', text)
    numbers.extend(amounts)
    
    return list(set(numbers))


def create_parent_chunk(
    child_chunks: List[Dict],
    filename: str,
    start_page: int,
    end_page: int,
) -> Dict:
    """
    创建父级文档块（合并多个子块）
    
    Args:
        child_chunks: 子块列表
        filename: 文件名
        start_page: 起始页码
        end_page: 结束页码
        
    Returns:
        父级文档块字典
    """
    # 合并内容
    combined_content = "\n\n".join([c.get("content", "") for c in child_chunks])
    
    # 提取关键信息
    entities = []
    numbers = []
    for chunk in child_chunks:
        content = chunk.get("content", "")
        entities.extend(extract_key_entities(content))
        numbers.extend(extract_key_numbers(content))
    
    parent_id = generate_chunk_id(combined_content, filename, start_page)
    
    return {
        "chunk_id": parent_id,
        "filename": filename,
        "page": start_page,  # 使用起始页作为主页码
        "page_range": f"{start_page}-{end_page}",
        "content": combined_content[:4000],  # 限制长度
        "chunk_type": "parent",
        "child_ids": [c.get("chunk_id") for c in child_chunks],
        "entities": list(set(entities)),
        "numbers": list(set(numbers)),
    }


def create_child_chunk(
    content: str,
    filename: str,
    page: int,
    parent_id: Optional[str] = None,
) -> Dict:
    """
    创建子级文档块
    
    Args:
        content: 文本内容
        filename: 文件名
        page: 页码
        parent_id: 父块 ID
        
    Returns:
        子级文档块字典
    """
    chunk_id = generate_chunk_id(content, filename, page)
    
    return {
        "chunk_id": chunk_id,
        "filename": filename,
        "page": page,
        "content": content,
        "chunk_type": "child",
        "parent_id": parent_id,
        "entities": extract_key_entities(content),
        "numbers": extract_key_numbers(content),
    }


def generate_document_summary(
    content: str,
    filename: str,
    page_range: str,
    use_llm: bool = True,
) -> str:
    """
    生成文档摘要
    
    Args:
        content: 文档内容
        filename: 文件名
        page_range: 页码范围
        use_llm: 是否使用 LLM 生成
        
    Returns:
        摘要文本
    """
    if not use_llm:
        # 简单的规则摘要：提取第一段和关键信息
        sentences = re.split(r'[。！？]', content)
        summary_parts = []
        
        # 提取前2个句子
        for s in sentences[:2]:
            if s.strip():
                summary_parts.append(s.strip())
        
        # 提取包含数值的句子
        for s in sentences[2:10]:
            if re.search(r'\d+(?:\.\d+)?(?:%|亿|万)', s):
                summary_parts.append(s.strip())
                if len(summary_parts) >= 4:
                    break
        
        return "。".join(summary_parts) + "。" if summary_parts else content[:200]
    
    try:
        prompt = DOCUMENT_SUMMARY_PROMPT.format(
            filename=filename,
            page_range=page_range,
            content=content[:3000],
        )
        
        summary = generate_response(
            [{"role": "user", "content": prompt}],
            model="qwen-turbo",
            max_retries=2,
            timeout=30.0,
        )
        
        return summary.strip()
        
    except Exception as e:
        logger.warning(f"LLM summary generation failed: {e}")
        # 降级到规则摘要
        return generate_document_summary(content, filename, page_range, use_llm=False)


def generate_virtual_qa_pairs(
    content: str,
    use_llm: bool = False,  # 默认关闭以节省 token
) -> List[Tuple[str, str]]:
    """
    生成虚拟问答对
    
    Args:
        content: 文档内容
        use_llm: 是否使用 LLM 生成
        
    Returns:
        问答对列表 [(question, answer), ...]
    """
    qa_pairs = []
    
    if use_llm:
        try:
            prompt = VIRTUAL_QA_PROMPT.format(content=content[:2000])
            
            response = generate_response(
                [{"role": "user", "content": prompt}],
                model="qwen-turbo",
                max_retries=2,
                timeout=30.0,
            )
            
            # 解析响应
            for line in response.strip().split('\n'):
                if 'Q:' in line and 'A:' in line:
                    parts = line.split('|')
                    if len(parts) == 2:
                        q = parts[0].replace('Q:', '').strip()
                        a = parts[1].replace('A:', '').strip()
                        if q and a:
                            qa_pairs.append((q, a))
            
        except Exception as e:
            logger.warning(f"LLM QA generation failed: {e}")
    
    # 规则生成的 QA 对（作为补充）
    # 提取数值相关的 QA
    numbers = extract_key_numbers(content)
    entities = extract_key_entities(content)
    
    # 如果有年份和营业收入，生成相关 QA
    years = [n for n in numbers if '年' in n]
    if years and '营业收入' in content:
        for year in years[:2]:
            q = f"千味央厨{year}的营业收入是多少？"
            # 尝试从内容中提取答案
            pattern = rf'{year}.*?营业收入.*?(\d+(?:\.\d+)?(?:亿|万|百万)元)'
            match = re.search(pattern, content)
            if match:
                a = f"千味央厨{year}营业收入为{match.group(1)}"
                qa_pairs.append((q, a))
    
    return qa_pairs


class EnhancedDocumentProcessor:
    """增强型文档处理器"""
    
    def __init__(
        self,
        enable_parent_chunks: bool = True,
        enable_summaries: bool = True,
        enable_virtual_qa: bool = False,  # 默认关闭以节省 token
        parent_chunk_size: int = 3,  # 每个父块包含的子块数量
        use_llm_for_summaries: bool = False,  # 默认使用规则摘要
    ):
        self.enable_parent_chunks = enable_parent_chunks
        self.enable_summaries = enable_summaries
        self.enable_virtual_qa = enable_virtual_qa
        self.parent_chunk_size = parent_chunk_size
        self.use_llm_for_summaries = use_llm_for_summaries
    
    def process_chunks(
        self,
        chunks: List[Dict],
        filename: str,
    ) -> Dict[str, List[Dict]]:
        """
        处理文档块，生成多粒度索引结构
        
        Args:
            chunks: 原始文档块列表
            filename: 文件名
            
        Returns:
            包含 child_chunks, parent_chunks, summaries, qa_pairs 的字典
        """
        result = {
            "child_chunks": [],
            "parent_chunks": [],
            "summaries": [],
            "qa_pairs": [],
        }
        
        # 1. 处理子块
        for chunk in chunks:
            child = create_child_chunk(
                content=chunk.get("content", ""),
                filename=filename,
                page=chunk.get("page", 0),
            )
            result["child_chunks"].append(child)
        
        # 2. 创建父块（如果启用）
        if self.enable_parent_chunks and len(chunks) > 1:
            # 按页码分组
            page_groups: Dict[int, List[Dict]] = {}
            for child in result["child_chunks"]:
                page = child.get("page", 0)
                if page not in page_groups:
                    page_groups[page] = []
                page_groups[page].append(child)
            
            # 创建跨页父块
            pages = sorted(page_groups.keys())
            for i in range(0, len(pages), self.parent_chunk_size):
                group_pages = pages[i:i + self.parent_chunk_size]
                group_children = []
                for p in group_pages:
                    group_children.extend(page_groups[p])
                
                if len(group_children) > 1:
                    parent = create_parent_chunk(
                        child_chunks=group_children,
                        filename=filename,
                        start_page=group_pages[0],
                        end_page=group_pages[-1],
                    )
                    
                    # 更新子块的 parent_id
                    for child in group_children:
                        child["parent_id"] = parent["chunk_id"]
                    
                    result["parent_chunks"].append(parent)
        
        # 3. 生成文档摘要（如果启用）
        if self.enable_summaries and result["parent_chunks"]:
            for parent in result["parent_chunks"]:
                summary_text = generate_document_summary(
                    content=parent["content"],
                    filename=filename,
                    page_range=parent.get("page_range", ""),
                    use_llm=self.use_llm_for_summaries,
                )
                
                summary = {
                    "chunk_id": f"summary_{parent['chunk_id']}",
                    "filename": filename,
                    "page": parent["page"],
                    "page_range": parent.get("page_range", ""),
                    "content": summary_text,
                    "chunk_type": "summary",
                    "source_chunk_id": parent["chunk_id"],
                }
                result["summaries"].append(summary)
        
        # 4. 生成虚拟 QA 对（如果启用）
        if self.enable_virtual_qa:
            all_content = "\n\n".join([c.get("content", "") for c in chunks[:5]])
            qa_pairs = generate_virtual_qa_pairs(
                content=all_content,
                use_llm=self.use_llm_for_summaries,
            )
            
            for q, a in qa_pairs:
                qa = {
                    "question": q,
                    "answer": a,
                    "filename": filename,
                    "chunk_type": "qa_pair",
                }
                result["qa_pairs"].append(qa)
        
        logger.info(
            f"Processed {filename}: {len(result['child_chunks'])} children, "
            f"{len(result['parent_chunks'])} parents, "
            f"{len(result['summaries'])} summaries"
        )
        
        return result


def enrich_chunk_with_context(
    chunk: Dict,
    all_chunks: List[Dict],
    context_window: int = 1,
) -> Dict:
    """
    为文档块添加上下文信息
    
    Args:
        chunk: 当前文档块
        all_chunks: 所有文档块
        context_window: 上下文窗口大小
        
    Returns:
        添加了上下文的文档块
    """
    filename = chunk.get("filename", "")
    page = chunk.get("page", 0)
    
    # 找到同一文件中相邻页的内容
    context_before = []
    context_after = []
    
    for c in all_chunks:
        if c.get("filename") != filename:
            continue
        
        c_page = c.get("page", 0)
        if page - context_window <= c_page < page:
            context_before.append(c.get("content", "")[:200])
        elif page < c_page <= page + context_window:
            context_after.append(c.get("content", "")[:200])
    
    enriched = chunk.copy()
    if context_before:
        enriched["context_before"] = " ".join(context_before)
    if context_after:
        enriched["context_after"] = " ".join(context_after)
    
    return enriched


if __name__ == "__main__":
    # 测试
    processor = EnhancedDocumentProcessor(
        enable_parent_chunks=True,
        enable_summaries=True,
        enable_virtual_qa=False,
        use_llm_for_summaries=False,
    )
    
    # 模拟文档块
    test_chunks = [
        {"content": "千味央厨成立于2012年，总部位于河南郑州。", "page": 1},
        {"content": "公司主要产品包括油炸类、烘焙类、蒸煮类、菜肴类。", "page": 1},
        {"content": "2021年营业收入达到12.85亿元，同比增长15%。", "page": 2},
        {"content": "公司客户包括百胜中国、华莱士等大型餐饮企业。", "page": 2},
    ]
    
    result = processor.process_chunks(test_chunks, "test.pdf")
    
    print(f"子块数量: {len(result['child_chunks'])}")
    print(f"父块数量: {len(result['parent_chunks'])}")
    print(f"摘要数量: {len(result['summaries'])}")
    
    if result['parent_chunks']:
        print(f"\n父块示例:")
        print(f"  内容: {result['parent_chunks'][0]['content'][:200]}...")
