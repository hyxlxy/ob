"""RAG 系统配置管理。

根据讲座内容，这里集中管理所有的优化参数：
1. 检索参数：TOP_K、BM25_CANDIDATES、VEC_CANDIDATES 等
2. 文本处理参数：max_chunk_size、overlap 等
3. 模型参数：embedding_dim、similarity_threshold 等
4. 性能参数：并发度、缓存等
"""

import os
from typing import Optional

import dotenv

dotenv.load_dotenv()


class RAGConfig:
    """RAG 系统配置类"""
    
    # ============ 检索参数 ============
    # 最终返回的结果数量
    TOP_K: int = int(os.getenv("TOP_K", "15"))
    
    # BM25 检索候选数（通常是 TOP_K 的 3-4 倍）
    BM25_CANDIDATES: int = int(os.getenv("BM25_CANDIDATES", str(TOP_K * 4)))
    
    # 向量检索候选数（通常是 TOP_K 的 5-6 倍）
    VEC_CANDIDATES: int = int(os.getenv("VEC_CANDIDATES", str(TOP_K * 6)))
    
    # RRF 融合的常数项（用于倒数排序融合）
    RRF_K: float = float(os.getenv("RRF_K", "60"))
    
    # 相似度阈值（向量检索）
    SIMILARITY_THRESHOLD: float = float(os.getenv("SIMILARITY_THRESHOLD", "0.2"))
    
    # FTS 最小匹配度（百分比或数字）
    FTS_MIN_SHOULD_MATCH: str = os.getenv("FTS_MIN_SHOULD_MATCH", "30%")
    
    # FTS 搜索字段（逗号分隔）
    FTS_FIELDS: str = os.getenv("FTS_FIELDS", "content,heading")
    
    # 最大上下文字符数
    MAX_CONTEXT_CHARS: int = int(os.getenv("MAX_CONTEXT_CHARS", "20000"))
    
    # ============ 文本处理参数 ============
    # 单个文本块的最大字符数
    MAX_CHUNK_SIZE: int = int(os.getenv("MAX_CHUNK_SIZE", "1200"))
    
    # 文本块之间的重叠字符数
    CHUNK_OVERLAP: int = int(os.getenv("CHUNK_OVERLAP", "150"))
    
    # 是否在文本块前添加标题前缀
    ADD_HEADING_PREFIX: bool = os.getenv("ADD_HEADING_PREFIX", "true").lower() in ("true", "1", "yes")
    
    # ============ 嵌入模型参数 ============
    # 嵌入向量维度（0 表示自动探测）
    EMBEDDING_DIM: int = int(os.getenv("EMBEDDING_DIM", "0"))
    
    # 向量距离度量方式：l2, ip, cosine
    VECTOR_METRIC: str = os.getenv("VECTOR_METRIC", "cosine").lower()
    
    # ============ 性能参数 ============
    # 数据导入阶段的最大并发工作数
    ADD_MAX_WORKERS: int = int(os.getenv("ADD_MAX_WORKERS", "6"))
    
    # 问答阶段的最大并发工作数
    QA_MAX_WORKERS: int = int(os.getenv("QA_MAX_WORKERS", "4"))
    
    # 搜索并发度（BM25 和向量搜索并行）
    SEARCH_MAX_WORKERS: int = int(os.getenv("SEARCH_MAX_WORKERS", "2"))
    
    # ============ VLM 参数 ============
    # 是否启用 VLM（视觉语言模型）进行图表识别
    USE_VLM: bool = os.getenv("USE_VLM", "0") in ("1", "true", "True")
    
    # VLM 处理的 PDF 页面 DPI
    VLM_DPI: int = int(os.getenv("VLM_DPI", "100"))
    
    # ============ OceanBase 连接参数 ============
    OCEANBASE_URI: str = os.getenv("OCEANBASE_URI", "")
    OCEANBASE_USER: str = os.getenv("OCEANBASE_USER", "")
    OCEANBASE_PASSWORD: str = os.getenv("OCEANBASE_PASSWORD", "")
    OCEANBASE_DBNAME: str = os.getenv("OCEANBASE_DBNAME", "")
    
    # ============ 数据库表名 ============
    TABLE_NAME: str = "rag_documents"
    
    # ============ 日志级别 ============
    LOG_LEVEL: str = os.getenv("LOG_LEVEL", "INFO")
    
    @classmethod
    def validate(cls) -> bool:
        """验证配置的合理性"""
        errors = []
        
        # 检查 TOP_K 的合理性
        if cls.TOP_K <= 0:
            errors.append("TOP_K 必须大于 0")
        
        # 检查候选数的合理性
        if cls.BM25_CANDIDATES < cls.TOP_K:
            errors.append(f"BM25_CANDIDATES ({cls.BM25_CANDIDATES}) 应不小于 TOP_K ({cls.TOP_K})")
        
        if cls.VEC_CANDIDATES < cls.TOP_K:
            errors.append(f"VEC_CANDIDATES ({cls.VEC_CANDIDATES}) 应不小于 TOP_K ({cls.TOP_K})")
        
        # 检查向量度量方式
        if cls.VECTOR_METRIC not in ("l2", "ip", "cosine"):
            errors.append(f"VECTOR_METRIC 必须是 l2, ip 或 cosine，当前值：{cls.VECTOR_METRIC}")
        
        # 检查文本块大小
        if cls.MAX_CHUNK_SIZE <= 0:
            errors.append("MAX_CHUNK_SIZE 必须大于 0")
        
        if cls.CHUNK_OVERLAP >= cls.MAX_CHUNK_SIZE:
            errors.append(f"CHUNK_OVERLAP ({cls.CHUNK_OVERLAP}) 应小于 MAX_CHUNK_SIZE ({cls.MAX_CHUNK_SIZE})")
        
        # 检查并发度
        if cls.ADD_MAX_WORKERS <= 0:
            errors.append("ADD_MAX_WORKERS 必须大于 0")
        
        if cls.QA_MAX_WORKERS <= 0:
            errors.append("QA_MAX_WORKERS 必须大于 0")
        
        if errors:
            for error in errors:
                print(f"配置错误：{error}")
            return False
        
        return True
    
    @classmethod
    def print_config(cls):
        """打印当前配置"""
        print("=" * 60)
        print("RAG 系统配置")
        print("=" * 60)
        print(f"检索参数:")
        print(f"  TOP_K: {cls.TOP_K}")
        print(f"  BM25_CANDIDATES: {cls.BM25_CANDIDATES}")
        print(f"  VEC_CANDIDATES: {cls.VEC_CANDIDATES}")
        print(f"  RRF_K: {cls.RRF_K}")
        print(f"  SIMILARITY_THRESHOLD: {cls.SIMILARITY_THRESHOLD}")
        print(f"  FTS_MIN_SHOULD_MATCH: {cls.FTS_MIN_SHOULD_MATCH}")
        print(f"  FTS_FIELDS: {cls.FTS_FIELDS}")
        print(f"  MAX_CONTEXT_CHARS: {cls.MAX_CONTEXT_CHARS}")
        print(f"\n文本处理参数:")
        print(f"  MAX_CHUNK_SIZE: {cls.MAX_CHUNK_SIZE}")
        print(f"  CHUNK_OVERLAP: {cls.CHUNK_OVERLAP}")
        print(f"  ADD_HEADING_PREFIX: {cls.ADD_HEADING_PREFIX}")
        print(f"\n嵌入模型参数:")
        print(f"  EMBEDDING_DIM: {cls.EMBEDDING_DIM}")
        print(f"  VECTOR_METRIC: {cls.VECTOR_METRIC}")
        print(f"\n性能参数:")
        print(f"  ADD_MAX_WORKERS: {cls.ADD_MAX_WORKERS}")
        print(f"  QA_MAX_WORKERS: {cls.QA_MAX_WORKERS}")
        print(f"  SEARCH_MAX_WORKERS: {cls.SEARCH_MAX_WORKERS}")
        print(f"\nVLM 参数:")
        print(f"  USE_VLM: {cls.USE_VLM}")
        print(f"  VLM_DPI: {cls.VLM_DPI}")
        print("=" * 60)


if __name__ == "__main__":
    RAGConfig.print_config()
    if RAGConfig.validate():
        print("\n✓ 配置验证通过")
    else:
        print("\n✗ 配置验证失败")

