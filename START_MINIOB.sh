#!/bin/bash

# MiniOB 服务器启动脚本
# 用于启动 MiniOB 数据库服务器，支持 TCP 和 Unix socket 连接

set -e

echo "=========================================="
echo "MiniOB 服务器启动脚本"
echo "=========================================="

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_debug"

# 检查 observer 可执行文件是否存在
if [ ! -f "$BUILD_DIR/bin/observer" ]; then
    echo "❌ 错误: 找不到 observer 可执行文件"
    echo "   路径: $BUILD_DIR/bin/observer"
    echo "   请先编译项目: ./build.sh"
    exit 1
fi

# 创建字典文件符号链接
if [ ! -L "$BUILD_DIR/dict" ]; then
    echo "📝 创建字典文件符号链接..."
    cd "$BUILD_DIR"
    ln -sf ../deps/3rd/cppjieba/dict dict
    cd "$SCRIPT_DIR"
fi

# 检查字典文件
if [ ! -f "$BUILD_DIR/dict/jieba.dict.utf8" ]; then
    echo "❌ 错误: 找不到字典文件"
    echo "   路径: $BUILD_DIR/dict/jieba.dict.utf8"
    exit 1
fi

# 启动参数
PORT=${1:-6789}
SOCKET=${2:-/tmp/miniob.sock}

echo "📋 启动参数:"
echo "   TCP 端口: $PORT"
echo "   Unix socket: $SOCKET"

# 清理旧的 socket 文件
if [ -S "$SOCKET" ]; then
    echo "🧹 清理旧的 socket 文件..."
    rm -f "$SOCKET"
fi

# 启动 MiniOB 服务器
echo "🚀 启动 MiniOB 服务器..."
cd "$BUILD_DIR"
./bin/observer -p "$PORT" -s "$SOCKET" 2>&1 &
OBSERVER_PID=$!

# 等待服务器启动
sleep 3

# 检查进程是否运行
if ps -p $OBSERVER_PID > /dev/null; then
    echo "✅ MiniOB 服务器已启动 (PID: $OBSERVER_PID)"
    echo ""
    echo "📊 连接信息:"
    echo "   TCP: 127.0.0.1:$PORT"
    echo "   Unix socket: $SOCKET"
    echo ""
    echo "💡 提示:"
    echo "   - 使用 Ctrl+C 停止服务器"
    echo "   - 查看日志: tail -f observer.log"
    echo ""
    
    # 保持进程运行
    wait $OBSERVER_PID
else
    echo "❌ MiniOB 服务器启动失败"
    exit 1
fi

