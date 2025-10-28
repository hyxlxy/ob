#!/bin/bash

echo "╔══════════════════════════════════════════════════════════════════════════════╗"
echo "║                    Ollama 安装和配置脚本                                      ║"
echo "╚══════════════════════════════════════════════════════════════════════════════╝"
echo ""

# Check if ollama is already installed
if command -v ollama &> /dev/null; then
    echo "✓ Ollama 已安装"
    ollama --version
    echo ""
    
    # Check if ollama is running
    if curl -s http://localhost:11434/api/tags > /dev/null 2>&1; then
        echo "✓ Ollama 服务正在运行"
    else
        echo "⚠️  Ollama 已安装但未运行"
        echo ""
        echo "启动 Ollama 服务..."
        ollama serve > /tmp/ollama.log 2>&1 &
        sleep 3
        
        if curl -s http://localhost:11434/api/tags > /dev/null 2>&1; then
            echo "✓ Ollama 服务已启动"
        else
            echo "✗ Ollama 服务启动失败"
            echo "请查看日志: /tmp/ollama.log"
            exit 1
        fi
    fi
else
    echo "⚠️  Ollama 未安装"
    echo ""
    echo "开始安装 Ollama..."
    
    # Install ollama
    curl -fsSL https://ollama.com/install.sh | sh
    
    if [ $? -eq 0 ]; then
        echo "✓ Ollama 安装成功"
    else
        echo "✗ Ollama 安装失败"
        exit 1
    fi
    
    echo ""
    echo "启动 Ollama 服务..."
    ollama serve > /tmp/ollama.log 2>&1 &
    sleep 3
    
    if curl -s http://localhost:11434/api/tags > /dev/null 2>&1; then
        echo "✓ Ollama 服务已启动"
    else
        echo "✗ Ollama 服务启动失败"
        echo "请查看日志: /tmp/ollama.log"
        exit 1
    fi
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "检查 bge-m3 模型..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Check if bge-m3 model is available
if ollama list | grep -q "bge-m3"; then
    echo "✓ bge-m3 模型已存在"
else
    echo "⚠️  bge-m3 模型未找到"
    echo ""
    echo "开始下载 bge-m3 模型 (约 2GB,可能需要几分钟)..."
    ollama pull bge-m3
    
    if [ $? -eq 0 ]; then
        echo "✓ bge-m3 模型下载成功"
    else
        echo "✗ bge-m3 模型下载失败"
        exit 1
    fi
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "验证配置..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Test ollama API
echo ""
echo "测试 Ollama API..."
if curl -s http://localhost:11434/api/tags | grep -q "bge-m3"; then
    echo "✓ Ollama API 正常,bge-m3 模型可用"
else
    echo "✗ Ollama API 测试失败"
    exit 1
fi

echo ""
echo "╔══════════════════════════════════════════════════════════════════════════════╗"
echo "║                    ✅ Ollama 配置完成!                                        ║"
echo "╚══════════════════════════════════════════════════════════════════════════════╝"
echo ""
echo "配置信息:"
echo "  - Ollama 地址: http://localhost:11434"
echo "  - 可用模型: bge-m3"
echo "  - 日志文件: /tmp/ollama.log"
echo ""
echo "下一步:"
echo "  1. 在 Langflow 中重新导入 rag/model.json"
echo "  2. 运行工作流"
echo "  3. 测试问题"
echo ""
echo "如果需要停止 Ollama:"
echo "  pkill ollama"
echo ""

