#!/usr/bin/env python3
"""
测试本地 RAG 环境是否正确配置
"""

import os
import sys
import socket
import json
import requests
from pathlib import Path


def print_header(title):
    print("\n" + "=" * 80)
    print(f"  {title}")
    print("=" * 80)


def print_result(check_name, success, message=""):
    status = "✓" if success else "✗"
    color = "\033[0;32m" if success else "\033[0;31m"
    reset = "\033[0m"
    print(f"{color}{status}{reset} {check_name}")
    if message:
        print(f"  {message}")


def check_environment_variables():
    """检查环境变量"""
    print_header("1. 检查环境变量")
    
    required_vars = {
        "MINIOB_SERVER_SOCKET": "/tmp/miniob.sock",
        "EMBEDDING_NAME": "bge-m3",
        "EMBEDDING_BASE_URL": "http://localhost:11434",
        "OB_DOC_PATH": None,  # 路径可能不同
    }
    
    all_ok = True
    for var, expected in required_vars.items():
        value = os.getenv(var)
        if value:
            if expected and value != expected:
                print_result(f"{var}", True, f"值: {value} (期望: {expected})")
            else:
                print_result(f"{var}", True, f"值: {value}")
        else:
            print_result(f"{var}", False, "未设置")
            all_ok = False
    
    return all_ok


def check_miniob():
    """检查 MiniOB 是否运行"""
    print_header("2. 检查 MiniOB")
    
    socket_path = os.getenv("MINIOB_SERVER_SOCKET", "/tmp/miniob.sock")
    
    if not os.path.exists(socket_path):
        print_result("MiniOB Socket", False, f"Socket 文件不存在: {socket_path}")
        print("  请先启动 MiniOB:")
        print("    cd build")
        print("    ./bin/observer -f ../etc/observer.ini")
        return False
    
    try:
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.settimeout(2)
        sock.connect(socket_path)
        
        # 发送测试 SQL
        test_sql = "SHOW TABLES;"
        sock.send((test_sql + '\0').encode('utf-8'))
        
        # 接收响应
        response = b''
        while True:
            data = sock.recv(8192)
            response += data
            if data.endswith(b'\0'):
                break
        
        sock.close()
        
        result = response[:-1].decode('utf-8')
        print_result("MiniOB 连接", True, f"响应: {result[:100]}...")
        return True
        
    except Exception as e:
        print_result("MiniOB 连接", False, f"错误: {str(e)}")
        return False


def check_ollama():
    """检查 Ollama 服务"""
    print_header("3. 检查 Ollama 服务")
    
    base_url = os.getenv("EMBEDDING_BASE_URL", "http://localhost:11434")
    
    try:
        # 检查 Ollama API
        response = requests.get(f"{base_url}/api/tags", timeout=5)
        if response.status_code == 200:
            models = response.json().get('models', [])
            model_names = [m['name'] for m in models]
            print_result("Ollama API", True, f"可访问")
            
            # 检查 bge-m3 模型
            embedding_name = os.getenv("EMBEDDING_NAME", "bge-m3")
            has_model = any(embedding_name in name for name in model_names)
            
            if has_model:
                print_result(f"{embedding_name} 模型", True, "已安装")
            else:
                print_result(f"{embedding_name} 模型", False, "未安装")
                print(f"  已安装的模型: {', '.join(model_names)}")
                print(f"  安装命令: ollama pull {embedding_name}")
                return False
            
            return True
        else:
            print_result("Ollama API", False, f"HTTP {response.status_code}")
            return False
            
    except requests.exceptions.ConnectionError:
        print_result("Ollama API", False, "无法连接")
        print("  请启动 Ollama 服务:")
        print("    ollama serve")
        return False
    except Exception as e:
        print_result("Ollama API", False, f"错误: {str(e)}")
        return False


def check_documents():
    """检查 OceanBase 文档"""
    print_header("4. 检查 OceanBase 文档")
    
    doc_path = os.getenv("OB_DOC_PATH")
    
    if not doc_path:
        print_result("文档路径", False, "OB_DOC_PATH 未设置")
        return False
    
    doc_path = Path(doc_path)
    
    if not doc_path.exists():
        print_result("文档路径", False, f"路径不存在: {doc_path}")
        print("  请下载 OceanBase 文档:")
        print("    git clone --depth 1 --branch V4.3.5 https://github.com/oceanbase/oceanbase-doc.git")
        return False
    
    # 统计文档数量
    md_files = list(doc_path.glob("**/*.md"))
    
    if len(md_files) == 0:
        print_result("文档文件", False, f"未找到 .md 文件")
        return False
    
    print_result("文档路径", True, f"{doc_path}")
    print_result("文档数量", True, f"找到 {len(md_files)} 个 .md 文件")
    
    return True


def check_langflow():
    """检查 Langflow"""
    print_header("5. 检查 Langflow")
    
    try:
        import langflow
        version = langflow.__version__
        print_result("Langflow 安装", True, f"版本: {version}")
        return True
    except ImportError:
        print_result("Langflow 安装", False, "未安装")
        print("  安装命令: pip3 install langflow -U")
        return False


def check_model_json():
    """检查 model.json"""
    print_header("6. 检查 model.json")
    
    model_path = Path("rag/model.json")
    
    if not model_path.exists():
        print_result("model.json", False, "文件不存在")
        return False
    
    try:
        with open(model_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        
        # 检查基本结构
        if 'data' not in data:
            print_result("model.json 结构", False, "缺少 'data' 字段")
            return False
        
        nodes = data.get('data', {}).get('nodes', [])
        edges = data.get('data', {}).get('edges', [])
        
        print_result("model.json", True, f"{len(nodes)} 个组件, {len(edges)} 个连接")
        
        # 检查关键组件
        component_names = []
        for node in nodes:
            if isinstance(node, dict) and 'data' in node:
                name = node.get('data', {}).get('node', {}).get('display_name', '')
                if name:
                    component_names.append(name)
        
        required_components = ['MiniOB', 'Ollama Embeddings', 'Directory']
        for comp in required_components:
            if comp in component_names:
                print_result(f"  {comp} 组件", True, "存在")
            else:
                print_result(f"  {comp} 组件", False, "缺失")
        
        return True
        
    except json.JSONDecodeError as e:
        print_result("model.json", False, f"JSON 格式错误: {str(e)}")
        return False
    except Exception as e:
        print_result("model.json", False, f"错误: {str(e)}")
        return False


def test_embedding():
    """测试 Embedding 生成"""
    print_header("7. 测试 Embedding 生成")
    
    base_url = os.getenv("EMBEDDING_BASE_URL", "http://localhost:11434")
    model_name = os.getenv("EMBEDDING_NAME", "bge-m3")
    
    try:
        # 测试生成 embedding
        response = requests.post(
            f"{base_url}/api/embeddings",
            json={
                "model": model_name,
                "prompt": "Hello, world!"
            },
            timeout=30
        )
        
        if response.status_code == 200:
            result = response.json()
            embedding = result.get('embedding', [])
            
            if embedding:
                print_result("Embedding 生成", True, f"维度: {len(embedding)}")
                
                # 检查维度是否为 1024
                if len(embedding) == 1024:
                    print_result("Embedding 维度", True, "1024 (正确)")
                else:
                    print_result("Embedding 维度", False, f"{len(embedding)} (期望 1024)")
                
                return True
            else:
                print_result("Embedding 生成", False, "返回空向量")
                return False
        else:
            print_result("Embedding 生成", False, f"HTTP {response.status_code}")
            return False
            
    except Exception as e:
        print_result("Embedding 生成", False, f"错误: {str(e)}")
        return False


def main():
    print("\n╔══════════════════════════════════════════════════════════════════════════════╗")
    print("║                      本地 RAG 环境测试                                        ║")
    print("╚══════════════════════════════════════════════════════════════════════════════╝")
    
    results = []
    
    # 运行所有检查
    results.append(("环境变量", check_environment_variables()))
    results.append(("MiniOB", check_miniob()))
    results.append(("Ollama", check_ollama()))
    results.append(("文档", check_documents()))
    results.append(("Langflow", check_langflow()))
    results.append(("model.json", check_model_json()))
    results.append(("Embedding", test_embedding()))
    
    # 总结
    print_header("测试总结")
    
    passed = sum(1 for _, success in results if success)
    total = len(results)
    
    for name, success in results:
        status = "✓" if success else "✗"
        color = "\033[0;32m" if success else "\033[0;31m"
        reset = "\033[0m"
        print(f"{color}{status}{reset} {name}")
    
    print(f"\n通过: {passed}/{total}")
    
    if passed == total:
        print("\n✅ 所有检查通过! 环境配置正确,可以开始测试了!")
        print("\n下一步:")
        print("  1. 启动 Langflow: langflow run")
        print("  2. 在浏览器中打开: http://localhost:7860")
        print("  3. 导入 rag/model.json")
        print("  4. 运行工作流测试")
        return 0
    else:
        print(f"\n⚠️  有 {total - passed} 项检查失败,请修复后再测试")
        return 1


if __name__ == '__main__':
    sys.exit(main())

