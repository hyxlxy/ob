#!/bin/bash
#
# 强制 Langflow 重新加载组件
#

echo "========================================================================"
echo "强制 Langflow 重新加载"
echo "========================================================================"
echo ""

# 1. 停止 Langflow
echo "1. 停止 Langflow..."
pkill -9 -f "langflow"
sleep 2
echo "   ✓ Langflow 已停止"

# 2. 清除所有缓存和数据库
echo ""
echo "2. 清除所有缓存和数据库..."

# 清除缓存
rm -rf ~/.cache/langflow
rm -rf ~/.langflow
rm -rf /tmp/langflow*
rm -rf /tmp/tmpdir*

# 清除 Python 缓存
find . -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null
find . -type f -name "*.pyc" -delete 2>/dev/null

echo "   ✓ 所有缓存已清除"

# 3. 创建一个新的工作流文件(带时间戳)
echo ""
echo "3. 创建新的工作流文件..."
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
NEW_FLOW="rag/model_fresh_${TIMESTAMP}.json"
cp rag/model.json "$NEW_FLOW"
echo "   ✓ 新文件: $NEW_FLOW"

# 4. 显示下一步
echo ""
echo "========================================================================"
echo "✅ 准备完成!"
echo "========================================================================"
echo ""
echo "现在请按照以下步骤操作:"
echo ""
echo "步骤 1: 完全关闭浏览器"
echo "  - 关闭所有浏览器窗口"
echo "  - 确认浏览器进程已退出"
echo ""
echo "步骤 2: 启动 Langflow"
echo "  langflow run"
echo ""
echo "步骤 3: 打开浏览器(建议使用无痕模式)"
echo "  Chrome: Ctrl+Shift+N"
echo "  Firefox: Ctrl+Shift+P"
echo ""
echo "步骤 4: 访问 Langflow"
echo "  http://localhost:7860"
echo ""
echo "步骤 5: 导入新的工作流文件"
echo "  文件: $NEW_FLOW"
echo ""
echo "步骤 6: 在运行前,打开 MiniOB 组件验证代码"
echo "  - 点击 MiniOB 组件"
echo "  - 查看 Code 标签"
echo "  - 搜索 'valid_documents'"
echo "  - 如果找到,说明代码已更新"
echo ""
echo "========================================================================"
echo ""

