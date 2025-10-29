#!/bin/bash

set -e  # Exit on error

echo "╔══════════════════════════════════════════════════════════════════════════════╗"
echo "║                    本地 RAG 测试环境安装脚本                                  ║"
echo "╚══════════════════════════════════════════════════════════════════════════════╝"
echo ""

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查是否以 root 运行
if [ "$EUID" -eq 0 ]; then 
    echo -e "${RED}请不要以 root 用户运行此脚本${NC}"
    exit 1
fi

# 1. 检查 Python 版本
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "1. 检查 Python 版本"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

PYTHON_VERSION=$(python3 --version | awk '{print $2}')
echo "当前 Python 版本: $PYTHON_VERSION"

if [[ "$PYTHON_VERSION" == "3.12.3" ]]; then
    echo -e "${GREEN}✓ Python 版本正确 (与远程测试环境一致)${NC}"
else
    echo -e "${YELLOW}⚠ Python 版本不是 3.12.3, 但可以继续${NC}"
fi
echo ""

# 2. 安装 Ollama
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "2. 安装 Ollama (Embedding 模型服务)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if command -v ollama &> /dev/null; then
    echo -e "${GREEN}✓ Ollama 已安装${NC}"
    ollama --version
else
    echo "正在安装 Ollama..."
    curl -fsSL https://ollama.com/install.sh | sh
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ Ollama 安装成功${NC}"
    else
        echo -e "${RED}✗ Ollama 安装失败${NC}"
        exit 1
    fi
fi
echo ""

# 3. 启动 Ollama 服务
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "3. 启动 Ollama 服务"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# 检查 Ollama 是否已经在运行
if pgrep -x "ollama" > /dev/null; then
    echo -e "${GREEN}✓ Ollama 服务已在运行${NC}"
else
    echo "启动 Ollama 服务..."
    nohup ollama serve > /tmp/ollama.log 2>&1 &
    sleep 3
    
    if pgrep -x "ollama" > /dev/null; then
        echo -e "${GREEN}✓ Ollama 服务启动成功${NC}"
    else
        echo -e "${RED}✗ Ollama 服务启动失败${NC}"
        echo "查看日志: tail -f /tmp/ollama.log"
        exit 1
    fi
fi
echo ""

# 4. 下载 bge-m3 模型
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "4. 下载 bge-m3 Embedding 模型 (~2GB, 可能需要几分钟)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if ollama list | grep -q "bge-m3"; then
    echo -e "${GREEN}✓ bge-m3 模型已存在${NC}"
else
    echo "正在下载 bge-m3 模型..."
    ollama pull bge-m3
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ bge-m3 模型下载成功${NC}"
    else
        echo -e "${RED}✗ bge-m3 模型下载失败${NC}"
        exit 1
    fi
fi
echo ""

# 5. 测试 Ollama 服务
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "5. 测试 Ollama 服务"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

echo "测试 Ollama API..."
RESPONSE=$(curl -s http://localhost:11434/api/tags)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Ollama API 正常${NC}"
    echo "已安装的模型:"
    echo "$RESPONSE" | python3 -m json.tool 2>/dev/null | grep '"name"' || echo "$RESPONSE"
else
    echo -e "${RED}✗ Ollama API 无法访问${NC}"
    exit 1
fi
echo ""

# 6. 检查 Langflow
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "6. 检查 Langflow"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# 检查是否已安装 langflow
if pip3 show langflow &> /dev/null; then
    INSTALLED_VERSION=$(pip3 show langflow | grep Version | awk '{print $2}')
    echo -e "${GREEN}✓ Langflow 已安装 (版本: $INSTALLED_VERSION)${NC}"
else
    echo -e "${YELLOW}⚠ Langflow 未安装${NC}"
    echo "如需安装,运行: pip3 install langflow -U"
fi
echo ""

# 7. 下载 OceanBase 文档
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "7. 下载 OceanBase 文档 (测试数据)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

DOC_DIR="./oceanbase-doc"

if [ -d "$DOC_DIR" ]; then
    echo -e "${GREEN}✓ OceanBase 文档目录已存在${NC}"
    
    read -p "是否重新下载文档? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -rf "$DOC_DIR"
    else
        echo -e "${GREEN}✓ 跳过文档下载${NC}"
        SKIP_DOC=1
    fi
fi

if [ -z "$SKIP_DOC" ]; then
    echo "克隆 OceanBase 文档仓库 (V4.3.5)..."
    git clone --depth 1 --branch V4.3.5 https://github.com/oceanbase/oceanbase-doc.git "$DOC_DIR"
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ OceanBase 文档下载成功${NC}"
        echo "文档路径: $(pwd)/$DOC_DIR/zh-CN"
    else
        echo -e "${RED}✗ OceanBase 文档下载失败${NC}"
        exit 1
    fi
fi
echo ""

# 8. 创建环境变量配置文件
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "8. 创建环境变量配置文件"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

ENV_FILE=".env.local"

cat > "$ENV_FILE" << 'ENVEOF'
# MiniOB 配置
export MINIOB_SERVER_SOCKET=/tmp/miniob.sock

# Embedding 模型配置
export EMBEDDING_NAME=bge-m3
export EMBEDDING_BASE_URL=http://localhost:11434

# OceanBase 文档路径
export OB_DOC_PATH=$(pwd)/oceanbase-doc/zh-CN

# LLM 配置 (本地测试可以留空,使用 Ollama 的其他模型)
# 如果要测试完整流程,需要配置真实的 LLM API
export LLM_NAME=qwen-max
export LLM_API_KEY=your_api_key_here
export LLM_BASE_URL=https://dashscope.aliyuncs.com/compatible-mode/v1

# 测评服务器 (本地测试不需要)
# export QA_SERVER_GET_QUESTION_URL=
# export QA_SERVER_POST_ANSWER_URL=
ENVEOF

echo -e "${GREEN}✓ 环境变量配置文件已创建: $ENV_FILE${NC}"
echo ""

# 9. 检查 MiniOB
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "9. 检查 MiniOB"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ -f "./build/bin/observer" ]; then
    echo -e "${GREEN}✓ MiniOB 已编译${NC}"
    echo "可执行文件: ./build/bin/observer"
else
    echo -e "${YELLOW}⚠ MiniOB 未编译${NC}"
    echo "请先编译 MiniOB:"
    echo "  mkdir -p build && cd build"
    echo "  cmake .. && make -j4"
fi
echo ""

# 10. 总结
echo "╔══════════════════════════════════════════════════════════════════════════════╗"
echo "║                            安装完成!                                          ║"
echo "╚══════════════════════════════════════════════════════════════════════════════╝"
echo ""
echo -e "${GREEN}✓ 所有依赖已安装${NC}"
echo ""
echo "下一步操作:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "1. 加载环境变量:"
echo "   source .env.local"
echo ""
echo "2. 启动 MiniOB:"
echo "   cd build"
echo "   ./bin/observer -f ../etc/observer.ini"
echo ""
echo "3. 启动 Langflow:"
echo "   langflow run"
echo ""
echo "4. 在浏览器中打开 Langflow:"
echo "   http://localhost:7860"
echo ""
echo "5. 导入工作流:"
echo "   在 Langflow 界面中导入 rag/model.json"
echo ""
echo "6. 运行测试"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "环境变量配置文件: $ENV_FILE"
echo "OceanBase 文档路径: $(pwd)/$DOC_DIR/zh-CN"
echo ""
echo "如需帮助,请查看:"
echo "  - FINAL_FIX_METADATA_ISSUE.md"
echo "  - MINIOB_FEATURE_CHECKLIST.md"
echo "  - LOCAL_SETUP_GUIDE.md (如果存在)"
echo ""

