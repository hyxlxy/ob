#!/bin/bash

# 标量子查询多行检查测试脚本

echo "======================================"
echo "测试标量子查询多行检查功能"
echo "======================================"

# 切换到项目根目录
cd /home/hxl/Desktop/oceambase/miniob-competition-2025

# 编译项目
echo ""
echo "1. 编译项目..."
./build.sh

if [ $? -ne 0 ]; then
    echo "编译失败！"
    exit 1
fi

echo ""
echo "2. 启动observer服务（后台运行）..."
# 先杀死已有的observer进程
pkill -9 observer 2>/dev/null

# 启动observer服务
./build/bin/observer -f etc/observer.ini &
OBSERVER_PID=$!

# 等待服务启动
sleep 2

echo ""
echo "3. 运行标量子查询测试..."
./build/bin/obclient -f test/case/test/scalar_subquery_multirow_test.test

echo ""
echo "4. 停止observer服务..."
kill $OBSERVER_PID 2>/dev/null

echo ""
echo "======================================"
echo "测试完成！"
echo "======================================"
echo ""
echo "预期结果："
echo "  - 测试3应该报错：标量子查询返回多行"
echo "  - 测试6应该报错：标量子查询返回多行"
echo "  - 其他测试应该正常通过"

