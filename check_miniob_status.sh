#!/bin/bash

echo "========================================"
echo "检查 MiniOB 状态"
echo "========================================"

# Check if Unix socket exists
if [ -S /tmp/miniob.sock ]; then
    echo "✓ Unix Socket 存在: /tmp/miniob.sock"
else
    echo "✗ Unix Socket 不存在: /tmp/miniob.sock"
    echo ""
    echo "MiniOB 可能没有运行,请启动 MiniOB:"
    echo "  cd build"
    echo "  ./bin/observer -f ../etc/observer.ini"
    exit 1
fi

# Check if process is running
if ps aux | grep -v grep | grep observer > /dev/null; then
    echo "✓ MiniOB 进程正在运行"
    ps aux | grep -v grep | grep observer | head -1
else
    echo "✗ MiniOB 进程未运行"
    exit 1
fi

# Test connection
echo ""
echo "测试连接..."
python3 << 'PYTHON'
import socket
try:
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.connect('/tmp/miniob.sock')
    sock.close()
    print("✓ 连接测试成功")
except Exception as e:
    print(f"✗ 连接测试失败: {e}")
    exit(1)
PYTHON

echo ""
echo "========================================"
echo "✅ MiniOB 运行正常!"
echo "========================================"
echo ""
echo "现在可以在 Langflow 中使用了"
echo "确保 MiniOB 组件的配置:"
echo "  Server socket: /tmp/miniob.sock"

