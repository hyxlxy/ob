#!/bin/bash
# 调试脚本：在gdb中运行observer并复现问题

set -e

# 清理旧的日志
rm -f /tmp/miniob_gdb.log
rm -rf /tmp/miniob/miniob

# 编译
cd /home/hxl/Desktop/oceambase/miniob-competition-2025
bash build.sh

# 创建测试环境
mkdir -p /tmp/miniob/miniob/data
cp etc/observer.ini /tmp/miniob/miniob/conf/observer.ini 2>/dev/null || mkdir -p /tmp/miniob/miniob/conf && cp etc/observer.ini /tmp/miniob/miniob/conf/

# 在后台运行gdb，捕获输出
echo "Starting observer in GDB..."
gdb -batch -x debug_subquery.gdb /tmp/miniob/build_debug/bin/observer > /tmp/miniob_gdb_output.txt 2>&1 &
GDB_PID=$!

# 等待服务器启动
sleep 2

# 连接并发送测试SQL
echo "Sending test SQL..."
(
  sleep 0.5
  echo "CREATE TABLE ssq_1(id int, col1 int, feat1 float);"
  sleep 0.1
  echo "CREATE TABLE ssq_2(id int, col2 int, feat2 float);"
  sleep 0.1
  echo "INSERT INTO ssq_1 VALUES (42, 90, 50.93);"
  sleep 0.1
  echo "INSERT INTO ssq_2 VALUES (5, 90, 33.45);"
  sleep 0.1
  echo "INSERT INTO ssq_2 VALUES (6, 80, 29.78);"
  sleep 0.1
  echo "INSERT INTO ssq_2 VALUES (7, 70, 22.15);"
  sleep 0.1
  echo "SELECT * FROM ssq_1 WHERE col1 = (SELECT ssq_2.col2 FROM ssq_2);"
  sleep 0.5
  echo "EXIT;"
) | nc localhost 6789 || echo "Connection failed"

# 等待gdb完成
sleep 1
kill $GDB_PID 2>/dev/null || true

echo "==================== GDB Output ===================="
cat /tmp/miniob_gdb_output.txt
echo "==================== Observer Log ===================="
tail -100 /tmp/miniob/miniob/log/observer.log* 2>/dev/null || echo "No log file"

