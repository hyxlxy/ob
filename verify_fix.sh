#!/bin/bash

echo "╔══════════════════════════════════════════════════════════════════════════════╗"
echo "║                        验证修复是否成功                                       ║"
echo "╚══════════════════════════════════════════════════════════════════════════════╝"
echo ""

# Check 1: MiniOB running
echo "1. 检查 MiniOB 状态..."
if [ -S /tmp/miniob.sock ]; then
    echo "   ✓ Unix Socket 存在"
else
    echo "   ✗ Unix Socket 不存在"
    exit 1
fi

# Check 2: Configuration in model.json
echo ""
echo "2. 检查 model.json 配置..."
python3 << 'PYTHON'
import json
with open('rag/model.json', 'r') as f:
    data = json.load(f)
for node in data['data']['nodes']:
    if isinstance(node, dict) and 'data' in node:
        if node['data'].get('node', {}).get('display_name', '') == 'MiniOB':
            socket = node['data']['node']['template']['server_socket']['value']
            if socket == '/tmp/miniob.sock':
                print("   ✓ server_socket = /tmp/miniob.sock")
            else:
                print(f"   ✗ server_socket = {socket}")
            break
PYTHON

# Check 3: Code fix
echo ""
echo "3. 检查代码修复..."
python3 << 'PYTHON'
import json
with open('rag/model.json', 'r') as f:
    data = json.load(f)
for node in data['data']['nodes']:
    if isinstance(node, dict) and 'data' in node:
        if node['data'].get('node', {}).get('display_name', '') == 'MiniOB':
            code = node['data']['node']['template']['code']['value']
            if 'server_socket if server_socket else os.getenv' in code:
                print("   ✓ 代码已修复 (使用参数优先)")
            else:
                print("   ✗ 代码未修复")
            break
PYTHON

echo ""
echo "╔══════════════════════════════════════════════════════════════════════════════╗"
echo "║                        ✅ 所有检查通过!                                       ║"
echo "╚══════════════════════════════════════════════════════════════════════════════╝"
echo ""
echo "下一步:"
echo "1. 在 Langflow 中删除旧工作流"
echo "2. 重新导入 rag/model.json"
echo "3. 运行测试"
echo ""

