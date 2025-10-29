#!/usr/bin/env python3
"""
MiniOB RAG 系统诊断工具

快速检查所有服务和配置的状态
"""

import os
import socket
import subprocess
import sys
import json
from pathlib import Path

# 颜色定义
class Colors:
    GREEN = '\033[0;32m'
    RED = '\033[0;31m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'  # No Color

def print_header(text):
    print(f"\n{Colors.BLUE}{'=' * 80}{Colors.NC}")
    print(f"{Colors.BLUE}{text}{Colors.NC}")
    print(f"{Colors.BLUE}{'=' * 80}{Colors.NC}\n")

def print_check(name, status, details=""):
    symbol = f"{Colors.GREEN}✓{Colors.NC}" if status else f"{Colors.RED}✗{Colors.NC}"
    print(f"{symbol} {name}")
    if details:
        print(f"  {details}")

def check_command(cmd):
    """检查命令是否存在"""
    try:
        subprocess.run([cmd, '--version'], capture_output=True, check=False)
        return True
    except FileNotFoundError:
        return False

def check_process(name):
    """检查进程是否运行"""
    try:
        result = subprocess.run(['pgrep', '-x', name], capture_output=True, text=True)
        return result.returncode == 0
    except:
        return False

def check_port(port):
    """检查端口是否监听"""
    try:
        result = subprocess.run(['netstat', '-tuln'], capture_output=True, text=True)
        return f':{port}' in result.stdout
    except:
        return False

def check_socket(path):
    """检查 Unix socket 是否存在"""
    import stat
    try:
        return os.path.exists(path) and stat.S_ISSOCK(os.stat(path).st_mode)
    except:
        return False

def check_ollama_models():
    """检查 Ollama 模型"""
    try:
        result = subprocess.run(['ollama', 'list'], capture_output=True, text=True)
        if result.returncode == 0:
            models = {}
            for line in result.stdout.split('\n')[1:]:  # Skip header
                if line.strip():
                    parts = line.split()
                    if parts:
                        models[parts[0]] = True
            return models
        return {}
    except:
        return {}

def test_ollama_embedding():
    """测试 Ollama 嵌入功能"""
    try:
        import requests
        response = requests.post(
            'http://localhost:11434/api/embeddings',
            json={'model': 'bge-m3', 'prompt': 'test'},
            timeout=5
        )
        return response.status_code == 200
    except:
        return False

def test_miniob_connection():
    """测试 MiniOB 连接"""
    socket_path = '/tmp/miniob.sock'
    
    # 尝试 Unix Socket
    if os.path.exists(socket_path):
        try:
            sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            sock.settimeout(2)
            sock.connect(socket_path)
            sock.send(b'SELECT 1;\0')
            result = sock.recv(1024)
            sock.close()
            return True, 'Unix Socket'
        except Exception as e:
            return False, f'Unix Socket 连接失败: {e}'
    
    # 尝试 TCP
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(2)
        sock.connect(('127.0.0.1', 6789))
        sock.send(b'SELECT 1;\0')
        result = sock.recv(1024)
        sock.close()
        return True, 'TCP'
    except Exception as e:
        return False, f'TCP 连接失败: {e}'

def check_file_exists(path, description):
    """检查文件是否存在"""
    exists = os.path.exists(path)
    if exists:
        size = os.path.getsize(path)
        return True, f"{description}: {path} ({size} bytes)"
    else:
        return False, f"{description}: {path} (不存在)"

def main():
    print_header("MiniOB RAG 系统诊断")
    
    # 1. 检查必需的命令
    print_header("1. 检查必需的命令")
    print_check("Python 3", sys.version_info >= (3, 7), f"版本: {sys.version.split()[0]}")
    print_check("Ollama", check_command('ollama'))
    print_check("CMake", check_command('cmake'))
    print_check("Make", check_command('make'))
    
    # 2. 检查 Ollama 服务
    print_header("2. 检查 Ollama 服务")
    ollama_running = check_process('ollama')
    print_check("Ollama 进程", ollama_running)
    print_check("Ollama API (端口 11434)", check_port(11434))
    
    if ollama_running:
        models = check_ollama_models()
        print_check("bge-m3 模型", 'bge-m3' in models)
        print_check("qwen 模型", any('qwen' in m for m in models.keys()))
        
        print("\n  已安装的模型:")
        for model in models.keys():
            print(f"    • {model}")
        
        # 测试嵌入功能
        embedding_works = test_ollama_embedding()
        print_check("嵌入功能测试", embedding_works)
    else:
        print(f"  {Colors.YELLOW}⚠ Ollama 未运行,跳过模型检查{Colors.NC}")
        print(f"  启动命令: ollama serve")
    
    # 3. 检查 MiniOB 服务
    print_header("3. 检查 MiniOB 服务")
    miniob_running = check_process('observer')
    print_check("MiniOB 进程 (observer)", miniob_running)
    print_check("Unix Socket (/tmp/miniob.sock)", check_socket('/tmp/miniob.sock'))
    print_check("TCP 端口 (6789)", check_port(6789))
    
    if miniob_running or check_socket('/tmp/miniob.sock'):
        success, details = test_miniob_connection()
        print_check("MiniOB 连接测试", success, details)
    else:
        print(f"  {Colors.YELLOW}⚠ MiniOB 未运行{Colors.NC}")
        print(f"  启动命令: cd build && ./bin/observer -f ../etc/observer.ini")
    
    # 4. 检查项目文件
    print_header("4. 检查项目文件")
    
    files_to_check = [
        ('rag/model.json', 'Langflow 工作流'),
        ('.env.local', '环境配置'),
        ('build/bin/observer', 'MiniOB 可执行文件'),
        ('etc/observer.ini', 'MiniOB 配置文件'),
        ('test_miniob_connection.py', '连接测试脚本'),
        ('fix_embedding_dimension_check.py', '修复脚本'),
        ('start_services.sh', '启动脚本'),
    ]
    
    for path, desc in files_to_check:
        exists, details = check_file_exists(path, desc)
        print_check(desc, exists, details if not exists else "")
    
    # 5. 检查 model.json 是否已修复
    print_header("5. 检查 model.json 修复状态")
    
    if os.path.exists('rag/model.json'):
        try:
            with open('rag/model.json', 'r', encoding='utf-8') as f:
                content = f.read()
                
            # 检查是否包含修复后的代码
            fixed = 'This is a test sentence for embedding dimension detection' in content
            print_check("嵌入维度检测已修复", fixed)
            
            if not fixed:
                print(f"  {Colors.YELLOW}⚠ 需要运行修复脚本: python3 fix_embedding_dimension_check.py{Colors.NC}")
            
            # 检查备份文件
            backup_exists = os.path.exists('rag/model.json.backup')
            print_check("备份文件存在", backup_exists, "rag/model.json.backup")
            
        except Exception as e:
            print_check("读取 model.json", False, str(e))
    
    # 6. 检查环境变量
    print_header("6. 检查环境配置")
    
    if os.path.exists('.env.local'):
        try:
            with open('.env.local', 'r') as f:
                env_vars = {}
                for line in f:
                    line = line.strip()
                    if line and not line.startswith('#') and '=' in line:
                        key, value = line.split('=', 1)
                        env_vars[key] = value
            
            important_vars = [
                'MINIOB_SERVER_SOCKET',
                'EMBEDDING_NAME',
                'EMBEDDING_BASE_URL',
                'OB_DOC_PATH',
                'LLM_NAME',
            ]
            
            for var in important_vars:
                if var in env_vars:
                    print_check(var, True, env_vars[var])
                else:
                    print_check(var, False, "未设置")
            
            # 检查文档路径是否存在
            if 'OB_DOC_PATH' in env_vars:
                doc_path = env_vars['OB_DOC_PATH']
                doc_exists = os.path.exists(doc_path)
                print_check("文档目录存在", doc_exists, doc_path)
                
        except Exception as e:
            print_check("读取 .env.local", False, str(e))
    else:
        print(f"  {Colors.YELLOW}⚠ .env.local 文件不存在{Colors.NC}")
    
    # 7. 总结和建议
    print_header("7. 诊断总结")
    
    issues = []
    
    if not check_command('ollama'):
        issues.append("❌ Ollama 未安装 - 访问 https://ollama.ai 安装")
    elif not ollama_running:
        issues.append("⚠ Ollama 未运行 - 运行: ollama serve")
    else:
        models = check_ollama_models()
        if 'bge-m3' not in models:
            issues.append("⚠ bge-m3 模型未安装 - 运行: ollama pull bge-m3")
        if not any('qwen' in m for m in models.keys()):
            issues.append("⚠ qwen 模型未安装 - 运行: ollama pull qwen2.5:latest")
    
    if not miniob_running and not check_socket('/tmp/miniob.sock'):
        issues.append("⚠ MiniOB 未运行 - 运行: cd build && ./bin/observer -f ../etc/observer.ini")
    
    if not os.path.exists('build/bin/observer'):
        issues.append("❌ MiniOB 未编译 - 运行: mkdir build && cd build && cmake .. && make")
    
    if os.path.exists('rag/model.json'):
        with open('rag/model.json', 'r') as f:
            if 'This is a test sentence for embedding dimension detection' not in f.read():
                issues.append("⚠ model.json 未修复 - 运行: python3 fix_embedding_dimension_check.py")
    
    if issues:
        print(f"{Colors.YELLOW}发现以下问题:{Colors.NC}\n")
        for issue in issues:
            print(f"  {issue}")
        print(f"\n{Colors.YELLOW}建议操作:{Colors.NC}")
        print(f"  1. 运行启动脚本: ./start_services.sh")
        print(f"  2. 运行修复脚本: python3 fix_embedding_dimension_check.py")
        print(f"  3. 查看故障排除指南: cat TROUBLESHOOTING.md")
    else:
        print(f"{Colors.GREEN}✅ 所有检查通过!系统已准备就绪。{Colors.NC}")
        print(f"\n下一步:")
        print(f"  1. 在 Langflow 中刷新页面或重新导入 rag/model.json")
        print(f"  2. 测试 RAG 工作流")
    
    print()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print(f"\n\n{Colors.YELLOW}诊断已取消{Colors.NC}")
        sys.exit(1)
    except Exception as e:
        print(f"\n{Colors.RED}诊断过程中出错: {e}{Colors.NC}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

