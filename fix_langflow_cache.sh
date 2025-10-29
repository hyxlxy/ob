#!/bin/bash
#
# 修复 Langflow 缓存问题
# 清除 Langflow 缓存并重新加载工作流
#

echo "========================================================================"
echo "修复 Langflow 缓存问题"
echo "========================================================================"
echo ""

# 1. 停止 Langflow (如果正在运行)
echo "1. 检查 Langflow 进程..."
LANGFLOW_PID=$(ps aux | grep "langflow run" | grep -v grep | awk '{print $2}')

if [ -n "$LANGFLOW_PID" ]; then
    echo "   发现 Langflow 进程 (PID: $LANGFLOW_PID)"
    echo "   停止 Langflow..."
    kill $LANGFLOW_PID
    sleep 2
    echo "   ✓ Langflow 已停止"
else
    echo "   Langflow 未运行"
fi

# 2. 清除 Langflow 缓存
echo ""
echo "2. 清除 Langflow 缓存..."

# Langflow 缓存位置
CACHE_DIRS=(
    "$HOME/.cache/langflow"
    "$HOME/.langflow"
    "/tmp/langflow"
)

for dir in "${CACHE_DIRS[@]}"; do
    if [ -d "$dir" ]; then
        echo "   清除: $dir"
        rm -rf "$dir"
        echo "   ✓ 已清除"
    fi
done

# 3. 清除 Python 缓存
echo ""
echo "3. 清除 Python __pycache__..."
find . -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null
echo "   ✓ 已清除"

# 4. 验证 model.json 中的修复
echo ""
echo "4. 验证 model.json 中的修复..."
if grep -q "This is a test sentence for embedding dimension detection" rag/model.json; then
    echo "   ✓ 修复代码存在"
else
    echo "   ✗ 修复代码不存在,需要重新应用修复"
    echo ""
    echo "   运行以下命令重新应用修复:"
    echo "   python3 fix_embedding_dimension_check.py"
    exit 1
fi

# 5. 创建新的工作流副本
echo ""
echo "5. 创建工作流副本..."
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
cp rag/model.json "rag/model_${TIMESTAMP}.json"
echo "   ✓ 已创建: rag/model_${TIMESTAMP}.json"

echo ""
echo "========================================================================"
echo "✅ 缓存清除完成!"
echo "========================================================================"
echo ""
echo "下一步:"
echo "  1. 重新启动 Langflow:"
echo "     langflow run"
echo ""
echo "  2. 在浏览器中打开:"
echo "     http://localhost:7860"
echo ""
echo "  3. 导入工作流:"
echo "     - 点击 'Import' 或 '导入'"
echo "     - 选择文件: rag/model.json"
echo "     - 或使用副本: rag/model_${TIMESTAMP}.json"
echo ""
echo "  4. 测试工作流"
echo ""
echo "如果问题仍然存在,请尝试:"
echo "  - 完全关闭浏览器并重新打开"
echo "  - 清除浏览器缓存 (Ctrl+Shift+Delete)"
echo "  - 使用无痕模式 (Ctrl+Shift+N)"
echo ""

