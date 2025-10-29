#!/usr/bin/env python3
"""
模拟官方测试插入 OceanBase 文档到 MiniOB 向量数据库

这个脚本会:
1. 读取少量 OceanBase 文档 (节约时间)
2. 分割文本为小块
3. 使用 Ollama bge-m3 生成嵌入向量
4. 插入到 MiniOB 向量数据库
5. 测试向量检索功能
"""

import os
import sys
import json
import socket
import select
from pathlib import Path
from typing import List, Dict, Any

# 配置
MINIOB_SOCKET = "/tmp/miniob.sock"
EMBEDDING_MODEL = os.getenv("EMBEDDING_NAME", "bge-m3")
EMBEDDING_BASE_URL = os.getenv("EMBEDDING_BASE_URL", "http://localhost:11434")
DOC_PATH = os.getenv("OB_DOC_PATH", "/home/hxl/Desktop/oceambase/ob25/ob/oceanbase-doc/zh-CN")
CHUNK_SIZE = int(os.getenv("SIM_CHUNK_SIZE", "500"))  # 每个文本块的字符数
CHUNK_OVERLAP = int(os.getenv("SIM_CHUNK_OVERLAP", "50"))  # 文本块之间的重叠字符数
MAX_DOCS = int(os.getenv("SIM_MAX_DOCS", "2"))  # 最多处理的文档数量 (默认2，快速验证)
CHUNKS_PER_DOC = int(os.getenv("SIM_CHUNKS_PER_DOC", "1"))  # 每个文档最多插入的块数 (默认1)

class MiniOBClient:
    """MiniOB 客户端"""
    
    def __init__(self, socket_path: str):
        self.socket_path = socket_path
        self.sock = None
    
    def connect(self):
        """连接到 MiniOB"""
        try:
            self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            self.sock.connect(self.socket_path)
            print(f"✓ 已连接到 MiniOB: {self.socket_path}")
            return True
        except Exception as e:
            print(f"✗ 连接 MiniOB 失败: {e}")
            return False
    
    def execute(self, sql: str) -> str:
        """执行 SQL 语句"""
        try:
            # 发送 SQL (以 null 字节结尾)
            self.sock.sendall((sql + '\0').encode('utf-8'))
            
            # 接收响应
            response = b''
            while True:
                ready = select.select([self.sock], [], [], 5.0)
                if ready[0]:
                    chunk = self.sock.recv(4096)
                    if not chunk:
                        break
                    response += chunk
                    if b'\0' in chunk:
                        break
                else:
                    break
            
            # 解码响应
            result = response.decode('utf-8').rstrip('\0')
            return result
        except Exception as e:
            return f"ERROR: {e}"
    
    def close(self):
        """关闭连接"""
        if self.sock:
            self.sock.close()
            print("✓ 已断开 MiniOB 连接")

class OllamaEmbeddings:
    """Ollama 嵌入模型客户端"""
    
    def __init__(self, model: str, base_url: str):
        self.model = model
        self.base_url = base_url.rstrip('/')
    
    def embed_query(self, text: str) -> List[float]:
        """生成文本的嵌入向量"""
        import requests
        
        url = f"{self.base_url}/api/embeddings"
        payload = {
            "model": self.model,
            "prompt": text
        }
        
        try:
            response = requests.post(url, json=payload, timeout=30)
            response.raise_for_status()
            data = response.json()
            return data['embedding']
        except Exception as e:
            print(f"✗ 生成嵌入向量失败: {e}")
            return []

def split_text(text: str, chunk_size: int = 500, chunk_overlap: int = 50) -> List[str]:
    """分割文本为小块"""
    chunks = []
    start = 0
    text_length = len(text)
    
    while start < text_length:
        end = start + chunk_size
        chunk = text[start:end]
        
        if chunk.strip():
            chunks.append(chunk.strip())
        
        start = end - chunk_overlap
    
    return chunks

def read_markdown_files(doc_path: str, max_files: int = 5) -> List[Dict[str, Any]]:
    """读取 Markdown 文档"""
    documents = []
    doc_dir = Path(doc_path)
    
    if not doc_dir.exists():
        print(f"✗ 文档目录不存在: {doc_path}")
        return documents
    
    # 优先读取向量搜索相关的文档
    priority_dirs = [
        "640.ob-vector-search",
        "200.quickstart",
        "100.learn-more-about-oceanbase"
    ]
    
    md_files = []
    
    # 先从优先目录中查找
    for priority_dir in priority_dirs:
        priority_path = doc_dir / priority_dir
        if priority_path.exists():
            md_files.extend(list(priority_path.rglob("*.md")))
            if len(md_files) >= max_files:
                break
    
    # 如果不够,从其他目录补充
    if len(md_files) < max_files:
        all_files = list(doc_dir.rglob("*.md"))
        for f in all_files:
            if f not in md_files:
                md_files.append(f)
                if len(md_files) >= max_files:
                    break
    
    # 读取文件内容
    for md_file in md_files[:max_files]:
        try:
            with open(md_file, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # 跳过太短的文档
            if len(content) < 100:
                continue
            
            documents.append({
                'path': str(md_file.relative_to(doc_dir)),
                'content': content,
                'title': md_file.stem
            })
            
            print(f"✓ 读取文档: {md_file.name}")
        except Exception as e:
            print(f"✗ 读取文档失败 {md_file}: {e}")
    
    return documents

def setup_database(client: MiniOBClient):
    """设置数据库和表"""
    print("\n" + "=" * 80)
    print("设置数据库")
    print("=" * 80)
    
    # 创建数据库
    print("\n1. 创建数据库 test_rag...")
    result = client.execute("CREATE DATABASE test_rag;")
    print(f"   结果: {result[:100]}")
    
    # 使用数据库
    print("\n2. 使用数据库 test_rag...")
    result = client.execute("USE test_rag;")
    print(f"   结果: {result[:100]}")
    
    # 删除旧表 (如果存在)
    print("\n3. 删除旧表 (如果存在)...")
    result = client.execute("DROP TABLE IF EXISTS ob_docs;")
    print(f"   结果: {result[:100]}")
    
    # 创建表
    print("\n4. 创建向量表 ob_docs...")
    create_table_sql = """
    CREATE TABLE ob_docs (
        id INT,
        content TEXT,
        embedding VECTOR(1024),
        metadata TEXT
    );
    """
    result = client.execute(create_table_sql)
    print(f"   结果: {result[:100]}")
    
    # 创建向量索引
    print("\n5. 创建向量索引...")
    create_index_sql = """
    CREATE VECTOR INDEX ob_docs_idx ON ob_docs (embedding)
    WITH (distance=l2, type=ivfflat, lists=10);
    """
    result = client.execute(create_index_sql)
    print(f"   结果: {result[:100]}")
    
    print("\n✓ 数据库设置完成")

def insert_documents(client: MiniOBClient, embeddings: OllamaEmbeddings, documents: List[Dict[str, Any]]):
    """插入文档到数据库"""
    print("\n" + "=" * 80)
    print("插入文档")
    print("=" * 80)
    
    doc_id = 1
    total_chunks = 0
    
    for doc in documents:
        print(f"\n处理文档: {doc['title']}")
        
        # 分割文本
        chunks = split_text(doc['content'], CHUNK_SIZE, CHUNK_OVERLAP)
        print(f"  分割为 {len(chunks)} 个文本块")
        
        # 插入每个文本块
        for i, chunk in enumerate(chunks[:CHUNKS_PER_DOC]):  # 每个文档最多插入指定数量的块
            # 生成嵌入向量
            print(f"  [{i+1}/{min(len(chunks), CHUNKS_PER_DOC)}] 生成嵌入向量...", end='', flush=True)
            embedding = embeddings.embed_query(chunk)
            
            if not embedding:
                print(" 失败,跳过")
                continue
            
            # 准备元数据
            metadata = {
                'source': doc['path'],
                'title': doc['title'],
                'chunk_index': i
            }
            
            # 构建 INSERT 语句
            embedding_str = '[' + ','.join(map(str, embedding)) + ']'
            metadata_str = json.dumps(metadata, ensure_ascii=False).replace("'", "''")
            chunk_str = chunk.replace("'", "''")
            
            insert_sql = f"""
            INSERT INTO ob_docs (id, content, embedding, metadata)
            VALUES ({doc_id}, '{chunk_str}', '{embedding_str}', '{metadata_str}');
            """
            
            # 执行插入
            result = client.execute(insert_sql)
            
            if "SUCCESS" in result or "success" in result.lower():
                print(f" ✓ 插入成功 (ID: {doc_id})")
                doc_id += 1
                total_chunks += 1
            else:
                print(f" ✗ 插入失败: {result[:100]}")
    
    print(f"\n✓ 共插入 {total_chunks} 个文本块")
    return total_chunks

def test_search(client: MiniOBClient, embeddings: OllamaEmbeddings):
    """测试向量检索"""
    print("\n" + "=" * 80)
    print("测试向量检索")
    print("=" * 80)
    
    # 测试查询
    test_queries = [
        "什么是向量数据库?",
        "如何创建向量索引?",
        "OceanBase 的特点是什么?"
    ]
    
    for query in test_queries:
        print(f"\n查询: {query}")
        
        # 生成查询向量
        print("  生成查询向量...", end='', flush=True)
        query_embedding = embeddings.embed_query(query)
        
        if not query_embedding:
            print(" 失败")
            continue
        
        print(" ✓")
        
        # 执行向量检索
        embedding_str = '[' + ','.join(map(str, query_embedding)) + ']'
        search_sql = f"""
        SELECT id, content, metadata, L2_DISTANCE(embedding, '{embedding_str}') AS distance
        FROM ob_docs
        ORDER BY distance
        LIMIT 3;
        """
        
        print("  执行检索...", end='', flush=True)
        result = client.execute(search_sql)
        print(" ✓")
        
        print(f"\n  结果:\n{result[:500]}")

def main():
    print("=" * 80)
    print("模拟官方测试 - 插入 OceanBase 文档到 MiniOB")
    print("=" * 80)
    
    # 检查依赖
    print("\n检查依赖...")
    try:
        import requests
        print("✓ requests 已安装")
    except ImportError:
        print("✗ requests 未安装")
        print("  安装: pip install requests --break-system-packages")
        return
    
    # 连接 MiniOB
    print("\n连接 MiniOB...")
    client = MiniOBClient(MINIOB_SOCKET)
    if not client.connect():
        print("\n请确保 MiniOB 正在运行:")
        print("  ./build/bin/observer -f etc/observer.ini")
        return
    
    # 初始化嵌入模型
    print("\n初始化嵌入模型...")
    embeddings = OllamaEmbeddings(EMBEDDING_MODEL, EMBEDDING_BASE_URL)
    print(f"✓ 使用模型: {EMBEDDING_MODEL}")
    
    # 读取文档
    print(f"\n读取文档 (最多 {MAX_DOCS} 个)...")
    documents = read_markdown_files(DOC_PATH, MAX_DOCS)
    
    if not documents:
        print(f"\n✗ 未找到文档,请检查路径: {DOC_PATH}")
        print("\n如果文档未下载,运行:")
        print("  ./setup_local_test_env.sh")
        client.close()
        return
    
    print(f"\n✓ 读取了 {len(documents)} 个文档")
    
    try:
        # 设置数据库
        setup_database(client)
        
        # 插入文档
        total_chunks = insert_documents(client, embeddings, documents)
        
        if total_chunks > 0:
            # 测试检索
            test_search(client, embeddings)
            
            print("\n" + "=" * 80)
            print("✅ 测试完成!")
            print("=" * 80)
            print(f"\n统计:")
            print(f"  • 处理文档数: {len(documents)}")
            print(f"  • 插入文本块: {total_chunks}")
            print(f"  • 数据库: test_rag")
            print(f"  • 表名: ob_docs")
            
            print("\n你现在可以:")
            print("  1. 在 Langflow 中测试 RAG 工作流")
            print("  2. 使用 test_rag 数据库进行查询")
            print("  3. 查看插入的数据: SELECT * FROM test_rag.ob_docs LIMIT 5;")
        else:
            print("\n✗ 没有成功插入任何数据")
    
    finally:
        # 关闭连接
        client.close()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\n\n测试已取消")
        sys.exit(1)
    except Exception as e:
        print(f"\n测试过程中出错: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

