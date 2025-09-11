#!/bin/bash

# 快速手动测试脚本
# 用法: ./quick_test.sh [build|build_debug]

BUILD_DIR=${1:-build}
PORT=6789
PROJECT_DIR="$(pwd)"

cd "$PROJECT_DIR"

echo "==> 使用构建目录: $BUILD_DIR"

# 检查二进制是否存在
if [ ! -f "$BUILD_DIR/bin/observer" ]; then
    echo "错误: $BUILD_DIR/bin/observer 不存在，请先编译"
    echo "编译命令:"
    echo "  mkdir -p $BUILD_DIR"
    echo "  cd $BUILD_DIR"
    if [[ "$BUILD_DIR" == *debug* ]]; then
      echo "  cmake .. -DCMAKE_BUILD_TYPE=Debug"
    else
      echo "  cmake .. -DCMAKE_BUILD_TYPE=Release"
    fi
    echo "  make -j4"
    exit 1
fi

# 清理旧进程（同端口）
echo "==> 清理旧的 observer 进程..."
pkill -f "observer.*-P $PORT" >/dev/null 2>&1 || true
sleep 1

# 清理数据目录（当前目录下的 miniob）
echo "==> 清理数据目录..."
rm -rf miniob

# 启动服务器（前台运行，方便 Ctrl+C 停止）
echo "==> 启动 observer (端口 $PORT)..."
"$BUILD_DIR/bin/observer" -f etc/observer.ini -P $PORT &
OBSERVER_PID=$!

# 等待启动
sleep 2

# 检查是否启动成功
if ! kill -0 $OBSERVER_PID 2>/dev/null; then
    echo "错误: observer 启动失败"
    exit 1
fi

echo "==> Observer 已启动 (PID: $OBSERVER_PID)"
echo "==> 现在可以连接客户端测试:"
echo "    $BUILD_DIR/bin/obclient -p $PORT"
echo ""
echo "==> 示例 SQL:"
echo "    create table exp_table(id int, col1 int, col2 int, col3 float, col4 float);"
echo "    insert into exp_table VALUES (1, 6, 9, 1.07, 4.33);"
echo "    insert into exp_table VALUES (4, 6, 5, 9.87, 3.0);"
echo "    insert into exp_table VALUES (1, 9, 8, 3.62, 6.47);"
echo "    select * from exp_table where -0 < col1 - col2;"
echo ""
echo "==> 按 Ctrl+C 停止服务器"

trap "echo '==> 停止服务器...'; kill $OBSERVER_PID; exit 0" INT
wait $OBSERVER_PID
