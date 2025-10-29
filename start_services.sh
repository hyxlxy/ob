#!/bin/bash

# 启动 RAG 系统所需的所有服务

set -e

echo "=========================================="
echo "启动 MiniOB RAG 系统服务"
echo "=========================================="
echo ""

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查 Ollama 是否已安装
if ! command -v ollama &> /dev/null; then
    echo -e "${RED}❌ Ollama 未安装${NC}"
    echo "请访问 https://ollama.ai 安装 Ollama"
    exit 1
fi

echo -e "${GREEN}✓ Ollama 已安装${NC}"

# 检查 Ollama 服务是否运行
if ! pgrep -x "ollama" > /dev/null; then
    echo -e "${YELLOW}⚠ Ollama 服务未运行,正在启动...${NC}"
    
    # 在后台启动 Ollama 服务
    nohup ollama serve > ollama.log 2>&1 &
    OLLAMA_PID=$!
    
    echo "等待 Ollama 服务启动..."
    sleep 3
    
    # 验证服务是否启动成功
    if pgrep -x "ollama" > /dev/null; then
        echo -e "${GREEN}✓ Ollama 服务已启动 (PID: $OLLAMA_PID)${NC}"
    else
        echo -e "${RED}❌ Ollama 服务启动失败${NC}"
        echo "查看日志: cat ollama.log"
        exit 1
    fi
else
    echo -e "${GREEN}✓ Ollama 服务已运行${NC}"
fi

# 检查并下载所需模型
echo ""
echo "检查 Ollama 模型..."

# 检查 bge-m3 嵌入模型
if ! ollama list | grep -q "bge-m3"; then
    echo -e "${YELLOW}⚠ bge-m3 模型未安装,正在下载...${NC}"
    ollama pull bge-m3
    echo -e "${GREEN}✓ bge-m3 模型已下载${NC}"
else
    echo -e "${GREEN}✓ bge-m3 嵌入模型已安装${NC}"
fi

# 检查 qwen 模型 (根据你的配置)
if ! ollama list | grep -q "qwen"; then
    echo -e "${YELLOW}⚠ qwen 模型未安装,正在下载...${NC}"
    echo "这可能需要几分钟时间..."
    ollama pull qwen2.5:latest
    echo -e "${GREEN}✓ qwen 模型已下载${NC}"
else
    echo -e "${GREEN}✓ qwen 语言模型已安装${NC}"
fi

# 显示已安装的模型
echo ""
echo "已安装的 Ollama 模型:"
ollama list

# 检查 MiniOB 服务
echo ""
echo "检查 MiniOB 服务..."

MINIOB_SOCKET="/tmp/miniob.sock"
MINIOB_PORT=6789

if [ -S "$MINIOB_SOCKET" ]; then
    echo -e "${GREEN}✓ MiniOB Unix Socket 存在: $MINIOB_SOCKET${NC}"
elif netstat -tuln 2>/dev/null | grep -q ":$MINIOB_PORT"; then
    echo -e "${GREEN}✓ MiniOB TCP 服务运行在端口 $MINIOB_PORT${NC}"
else
    echo -e "${YELLOW}⚠ MiniOB 服务未运行${NC}"
    echo ""
    echo "请手动启动 MiniOB 服务:"
    echo "  cd build"
    echo "  ./bin/observer -f ../etc/observer.ini"
    echo ""
    echo "或者使用 LSM 引擎:"
    echo "  ./bin/observer -f ../etc/observer.ini -s lsm"
    echo ""
    read -p "是否现在启动 MiniOB? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        if [ -d "build" ]; then
            cd build
            echo "启动 MiniOB observer..."
            nohup ./bin/observer -f ../etc/observer.ini > ../miniob.log 2>&1 &
            MINIOB_PID=$!
            cd ..
            
            echo "等待 MiniOB 启动..."
            sleep 2
            
            if [ -S "$MINIOB_SOCKET" ]; then
                echo -e "${GREEN}✓ MiniOB 服务已启动 (PID: $MINIOB_PID)${NC}"
            else
                echo -e "${RED}❌ MiniOB 服务启动失败${NC}"
                echo "查看日志: cat miniob.log"
                exit 1
            fi
        else
            echo -e "${RED}❌ build 目录不存在,请先编译 MiniOB${NC}"
            exit 1
        fi
    fi
fi

# 测试连接
echo ""
echo "测试服务连接..."

# 测试 Ollama
echo -n "测试 Ollama 嵌入模型... "
if curl -s http://localhost:11434/api/embeddings -d '{"model":"bge-m3","prompt":"test"}' > /dev/null 2>&1; then
    echo -e "${GREEN}✓${NC}"
else
    echo -e "${RED}✗${NC}"
fi

# 测试 MiniOB
if [ -f "test_miniob_connection.py" ]; then
    echo -n "测试 MiniOB 连接... "
    if python3 test_miniob_connection.py > /dev/null 2>&1; then
        echo -e "${GREEN}✓${NC}"
    else
        echo -e "${YELLOW}⚠ (可能需要手动测试)${NC}"
    fi
fi

echo ""
echo "=========================================="
echo -e "${GREEN}✅ 服务启动完成!${NC}"
echo "=========================================="
echo ""
echo "服务状态:"
echo "  • Ollama 服务: http://localhost:11434"
echo "  • MiniOB Socket: $MINIOB_SOCKET"
echo "  • MiniOB TCP: localhost:$MINIOB_PORT"
echo ""
echo "日志文件:"
echo "  • Ollama: ollama.log"
echo "  • MiniOB: miniob.log"
echo ""
echo "下一步:"
echo "  1. 在 Langflow 中刷新页面或重新导入 rag/model.json"
echo "  2. 测试 RAG 工作流"
echo ""
echo "停止服务:"
echo "  pkill ollama"
echo "  pkill observer"
echo ""

