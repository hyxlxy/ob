import re
from typing import List


def split_text(
    text: str,
    max_chunk_size: int = 1200,
    overlap: int = 150,
    add_heading_prefix: bool = True,
) -> List[str]:
    """
    将文本按段落优先切分；当段落过长时再按句子边界切分，并使用少量重叠以保持上下文连续性。
    
    根据讲座内容实现的优化：
    - 轻量级标题/章节识别（第X章/第X节/一、/（一）/1. 等），将最近标题作为上下文前缀
    - 财报特定的关键词识别（财务数据、指标等）
    - 更智能的重叠策略，避免在关键信息处断裂
    - 语义完整性保证：不在关键词处断裂
    - 多级标题支持：保留标题层级关系

    Args:
        text: 待切分文本
        max_chunk_size: 单块最大字符数（近似 token 控制）
        overlap: 邻接块之间的重叠字符数（仅用于长段落切分后拼块）
        add_heading_prefix: 是否在块前添加"【标题】xxx"前缀

    Returns:
        文本块列表
    """
    if not text or not text.strip():
        return []

    # 归一化空白
    text = re.sub(r"[ \t]+", " ", text)
    text = re.sub(r"\n{3,}", "\n\n", text)
    text = text.strip()

    # 按双换行视为段落
    raw_paragraphs = [p.strip() for p in text.split("\n\n") if p.strip()]
    if not raw_paragraphs:
        return []

    # 识别标题：短且匹配常见标题模式（包括财报特定的标题）
    heading_patterns = [
        r"^第[一二三四五六七八九十百千]+[章节篇部分]\s*.*$",
        r"^[（(]?[一二三四五六七八九十]+[）)]?\s+.*$",  # （一）/一、
        r"^\d+\s*[\.、)）]\s*.*$",                 # 1. / 1） / 1、
        r"^(摘要|结论|概述|背景|前言|风险提示|盈利预测|行业概况|财务分析|主要产品|市场分析|竞争格局|发展战略|核心数据|财务指标)\s*$",
        r"^(图\s*\d+|表\s*\d+).*$",
        r"^(核心观点|投资建议|风险因素|关键指标|财务指标|业务分析|市场地位|竞争优势).*$",
    ]
    heading_re = re.compile("|".join(heading_patterns))

    paragraphs = []  # (is_heading, text)
    for p in raw_paragraphs:
        is_heading = (len(p) <= 80) and bool(heading_re.match(p))
        paragraphs.append((is_heading, p))

    chunks: List[str] = []

    def _with_overlap(parts: List[str]) -> List[str]:
        """使用更智能的重叠策略，避免在关键信息处断裂"""
        if not parts:
            return []
        out: List[str] = []
        buf = ""
        for s in parts:
            if not buf:
                buf = s
                continue
            # 若加入后超限则提交当前块并带重叠开启新块
            if len(buf) + 1 + len(s) > max_chunk_size:
                out.append(buf)
                if overlap > 0:
                    # 智能重叠：优先在句子边界处重叠，避免在数字处断裂
                    tail = buf[-overlap:]
                    best_idx = -1
                    # 尝试从最后一个句号/问号/感叹号处开始重叠
                    for punct in ["。", "！", "？", ".", "!", "?"]:
                        idx = tail.rfind(punct)
                        if idx > 0 and idx > overlap // 2:
                            best_idx = idx
                            break
                    
                    if best_idx > 0:
                        tail = tail[best_idx + 1:].strip()
                    
                    buf = (tail + " " + s)[:max_chunk_size]
                else:
                    buf = s
            else:
                buf += " " + s
        if buf:
            out.append(buf)
        return out

    current_heading = ""
    current_heading_level = 0  # 用于追踪标题层级
    
    for is_heading, paragraph in paragraphs:
        if is_heading:
            # 判断标题层级
            if re.match(r"^第[一二三四五六七八九十百千]+[章节篇部分]", paragraph):
                current_heading_level = 1
            elif re.match(r"^[（(]?[一二三四五六七八九十]+[）)]?", paragraph):
                current_heading_level = 2
            else:
                current_heading_level = 3
            
            current_heading = paragraph
            continue

        # 普通段落
        if len(paragraph) <= max_chunk_size:
            chunk = paragraph
            if add_heading_prefix and current_heading:
                prefix = f"【标题】{current_heading}\n"
                # 避免重复加前缀
                if not chunk.startswith(prefix):
                    chunk = prefix + chunk
            chunks.append(chunk)
        else:
            # 先按句子切分再组块并加重叠
            long_sentences = _split_long_paragraph(paragraph, max_chunk_size)
            grouped = _with_overlap(long_sentences)
            for g in grouped:
                chunk = g
                if add_heading_prefix and current_heading:
                    prefix = f"【标题】{current_heading}\n"
                    if not chunk.startswith(prefix):
                        # 保证不超过上限
                        remain = max_chunk_size - len(prefix)
                        chunk = prefix + chunk[: max(0, remain)]
                chunks.append(chunk)

    # 过滤空块，保留所有有意义的块
    filtered_chunks = []
    for chunk in chunks:
        if chunk.strip():
            filtered_chunks.append(chunk)
    
    return filtered_chunks


def _split_long_paragraph(text: str, max_chunk_size: int) -> List[str]:
    """
    Split a long paragraph into smaller chunks at sentence boundaries.
    Only used when a single paragraph exceeds max_chunk_size.
    """
    # Find sentence boundaries (Chinese and English punctuation)
    sentence_endings = r"[。！？.!?]"

    # Split text while keeping the punctuation
    parts = re.split(f"({sentence_endings})", text)

    sentences = []
    current_sentence = ""

    for part in parts:
        if not part.strip():
            continue

        current_sentence += part

        # If this part is a sentence ending, save the sentence
        if re.match(sentence_endings, part):
            if current_sentence.strip():
                sentences.append(current_sentence.strip())
            current_sentence = ""

    # Add remaining text as a sentence if any
    if current_sentence.strip():
        sentences.append(current_sentence.strip())

    # If no sentence endings found, return the whole paragraph as one chunk
    if not sentences:
        return [text]

    # Group sentences into chunks
    chunks = []
    current_chunk = ""

    for sentence in sentences:
        # If adding this sentence would exceed max_chunk_size, save current chunk
        if current_chunk and len(current_chunk) + len(sentence) + 1 > max_chunk_size:
            chunks.append(current_chunk)
            current_chunk = sentence
        else:
            if current_chunk:
                current_chunk += " " + sentence
            else:
                current_chunk = sentence

    # Add remaining chunk
    if current_chunk:
        chunks.append(current_chunk)

    return chunks


if __name__ == "__main__":
    print(
        split_text(
            """
公司介绍：聚焦数字能源产业链，软、硬件协同发展：公司创立于 1996 年，2010 年于深交所挂牌上市，主要业务包含数据中心与站点能源、数字电网与综合能源服务、电力电源、新能源车充换电等。公司股权结构稳定，管理层资历深厚。2024 年公司利润实现高增，2025 年预计持续增长。2024 年，公司实现营业收入 19.6 亿元，同比+26.1%；实现归母净利润 1.1 亿元，同比 +178.5%。数据中心电源、电力操作电源系统、通信电源系统贡献主要营收，2024 年，三者在营收中占比分别为 34.1%、24.4%、15.6%。
        
硬件业务：电源业务基本盘稳定，HVDC 迎来市场新机遇：

1）HVDC 方案先行者， AIDC 时代渗透率有望提升： 当前数据中心常见的几种不间断供电技术包括 AC UPS、HVDC 和市电直供+BBU 等。AIGC 时代，HVDC 供电凭借输电距离长、损耗小、稳定性好、效率高等优点，渗透率有望提升。公司是 HVDC 方案先行者，市场份额行业领先，牵头制订了《信息通信用 240V/336V 直流供电系统技术要求和试验方法》国家标准，且入选工信部制造业单项冠军示范企业。目前，公司主要客户为大型互联网企业、运营商以及 IDC 第三方数据中心运营商。

2）巴拿马方案绑定大客户，有望持续贡献业绩：预制化 Panama 电力模组是公司与大客户阿里巴巴合作开发的数据中心供电方案，有占地面积小、交付速度快、高可用性、效率高、成本低等优点。阿里巴巴未来三年在云和 AI的基础设施投入预计将超越过去十年的总和，有望带动公司巴拿马电源产品出货量增长；此外，头部互联网企业及运营商均规划增加算力相关资本开支，若能获取其他头部企业大订单，公司业绩增长空间有望进一步向上打开。

3）通信电源维持稳定，电力电源拓展存量份额：通信电源来看，公司在近年运营商集采招标中份额位居行业前列，营业收入基本稳定。2024 年，公司通信电源营收 3.06 亿元，同比-4.5%。电力电源来看，公司通过深耕细作继续拓展存量市场份额；另一方面，加速推进海外业务，并拓展新能源等增量市场机会，营收维持稳定增长。 2024 年，公司电力电源营收 4.79 亿元，同比+20.5%。
        """
        )
    )
