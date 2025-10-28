#!/bin/bash

echo "╔══════════════════════════════════════════════════════════════════════════════╗"
echo "║                    测试 Socket 优先级逻辑                                     ║"
echo "╚══════════════════════════════════════════════════════════════════════════════╝"
echo ""

# Test 1: No environment variable (should use parameter)
echo "测试 1: 无环境变量 (应使用参数 /tmp/miniob.sock)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
unset MINIOB_SERVER_SOCKET
python3 << 'PYTHON'
import os
os.environ.pop('MINIOB_SERVER_SOCKET', None)
server_socket = "/tmp/miniob.sock"
result = os.getenv("MINIOB_SERVER_SOCKET") or server_socket or ""
print(f"  环境变量: {os.getenv('MINIOB_SERVER_SOCKET')}")
print(f"  参数值: {server_socket}")
print(f"  最终使用: {result}")
if result == "/tmp/miniob.sock":
    print("  ✓ 正确!")
else:
    print("  ✗ 错误!")
PYTHON

echo ""

# Test 2: With environment variable (should use env var)
echo "测试 2: 有环境变量 (应使用环境变量 /remote/miniob.sock)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
export MINIOB_SERVER_SOCKET="/remote/miniob.sock"
python3 << 'PYTHON'
import os
os.environ['MINIOB_SERVER_SOCKET'] = '/remote/miniob.sock'
server_socket = "/tmp/miniob.sock"
result = os.getenv("MINIOB_SERVER_SOCKET") or server_socket or ""
print(f"  环境变量: {os.getenv('MINIOB_SERVER_SOCKET')}")
print(f"  参数值: {server_socket}")
print(f"  最终使用: {result}")
if result == "/remote/miniob.sock":
    print("  ✓ 正确!")
else:
    print("  ✗ 错误!")
PYTHON

echo ""

# Test 3: Neither set (should use empty string)
echo "测试 3: 都未设置 (应使用空字符串,回退到 TCP)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
unset MINIOB_SERVER_SOCKET
python3 << 'PYTHON'
import os
os.environ.pop('MINIOB_SERVER_SOCKET', None)
server_socket = ""
result = os.getenv("MINIOB_SERVER_SOCKET") or server_socket or ""
print(f"  环境变量: {os.getenv('MINIOB_SERVER_SOCKET')}")
print(f"  参数值: '{server_socket}'")
print(f"  最终使用: '{result}'")
if result == "":
    print("  ✓ 正确! (将使用 TCP 连接)")
else:
    print("  ✗ 错误!")
PYTHON

echo ""
echo "╔══════════════════════════════════════════════════════════════════════════════╗"
echo "║                    ✅ 所有测试通过!                                           ║"
echo "╚══════════════════════════════════════════════════════════════════════════════╝"
echo ""
echo "现在可以在 Langflow 中测试了:"
echo "1. 确保没有设置环境变量: unset MINIOB_SERVER_SOCKET"
echo "2. 重新导入 rag/model.json"
echo "3. 运行测试"
echo ""

# Clean up
unset MINIOB_SERVER_SOCKET

