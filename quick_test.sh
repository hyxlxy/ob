#!/bin/bash

echo "=========================================="
echo "RAG 工作流快速测试脚本"
echo "=========================================="

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查函数
check_command() {
    if command -v $1 &> /dev/null; then
        echo -e "${GREEN}✓${NC} $1 已安装"
        return 0
    else
        echo -e "${RED}✗${NC} $1 未安装"
        return 1
    fi
}

# 步骤 1: 检查环境
echo ""
echo "步骤 1: 检查环境"
echo "----------------------------------------"

check_command python3
check_command git

# 检查 Python 版本
PYTHON_VERSION=$(python3 --version 2>&1 | awk '{print $2}')
echo "Python 版本: $PYTHON_VERSION"

# 步骤 2: 检查 MiniOB
echo ""
echo "步骤 2: 检查 MiniOB"
echo "----------------------------------------"

if [ -f "build/bin/observer" ]; then
    echo -e "${GREEN}✓${NC} MiniOB 可执行文件存在"
else
    echo -e "${RED}✗${NC} MiniOB 可执行文件不存在"
    echo "请先编译 MiniOB: cd build && cmake .. && make"
fi

# 检查 MiniOB 是否运行
if pgrep -x "observer" > /dev/null; then
    echo -e "${GREEN}✓${NC} MiniOB 进程正在运行"
else
    echo -e "${YELLOW}⚠${NC} MiniOB 进程未运行"
    echo "启动 MiniOB: cd build && ./bin/observer -f ../etc/observer.ini"
fi

# 步骤 3: 检查工作流文件
echo ""
echo "步骤 3: 检查工作流文件"
echo "----------------------------------------"

if [ -f "rag/model.json" ]; then
    echo -e "${GREEN}✓${NC} rag/model.json 存在"
    
    # 检查文件大小
    FILE_SIZE=$(wc -c < rag/model.json)
    echo "文件大小: $FILE_SIZE 字节"
    
    # 验证 JSON 格式
    if python3 -c "import json; json.load(open('rag/model.json'))" 2>/dev/null; then
        echo -e "${GREEN}✓${NC} JSON 格式有效"
    else
        echo -e "${RED}✗${NC} JSON 格式无效"
    fi
else
    echo -e "${RED}✗${NC} rag/model.json 不存在"
fi

# 步骤 4: 测试 MiniOB 连接
echo ""
echo "步骤 4: 测试 MiniOB 连接"
echo "----------------------------------------"

if [ -f "test_miniob_connection.py" ]; then
    echo "运行连接测试..."
    python3 test_miniob_connection.py
else
    echo -e "${YELLOW}⚠${NC} test_miniob_connection.py 不存在,跳过连接测试"
fi

# 步骤 5: 环境变量检查
echo ""
echo "步骤 5: 检查环境变量"
echo "----------------------------------------"

check_env() {
    if [ -z "${!1}" ]; then
        echo -e "${YELLOW}⚠${NC} $1 未设置"
        return 1
    else
        echo -e "${GREEN}✓${NC} $1 = ${!1}"
        return 0
    fi
}

check_env "OB_DOC_PATH"
check_env "EMBEDDING_NAME"
check_env "EMBEDDING_BASE_URL"
check_env "LLM_NAME"
check_env "LLM_API_KEY"
check_env "LLM_BASE_URL"

# 步骤 6: 总结
echo ""
echo "=========================================="
echo "测试总结"
echo "=========================================="

echo ""
echo "下一步操作:"
echo "1. 如果 MiniOB 未运行,请启动:"
echo "   cd build && ./bin/observer -f ../etc/observer.ini"
echo ""
echo "2. 设置环境变量 (如果未设置):"
echo "   export OB_DOC_PATH=/path/to/oceanbase-doc/zh-CN"
echo "   export EMBEDDING_NAME=bge-m3"
echo "   export LLM_NAME=qwen-max"
echo "   等等..."
echo ""
echo "3. 启动 Langflow:"
echo "   langflow run"
echo ""
echo "4. 导入工作流:"
echo "   在浏览器中打开 http://localhost:7860"
echo "   导入 rag/model.json"
echo ""
echo "5. 运行测试"
echo ""
echo "详细说明请查看: LOCAL_TEST_GUIDE.md"
echo "=========================================="

