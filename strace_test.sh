#!/bin/bash
# 使用strace追踪observer的系统调用

set -e

# 清理
rm -rf /tmp/miniob/miniob
mkdir -p /tmp/miniob/miniob/data
mkdir -p /tmp/miniob/miniob/conf
cp /home/hxl/Desktop/oceambase/miniob-competition-2025/etc/observer.ini /tmp/miniob/miniob/conf/ 2>/dev/null || true

# 在strace下启动observer
echo "Starting observer with strace..."
strace -f -e trace=exit,exit_group,kill,signal -o /tmp/strace_output.txt /tmp/miniob/build_debug/bin/observer -f /tmp/miniob/miniob/conf/observer.ini > /tmp/observer_stdout.txt 2>&1 &
OBSERVER_PID=$!

# 等待启动
sleep 2

# 发送测试SQL
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
  echo "SELECT * FROM ssq_1 WHERE col1 = (SELECT ssq_2.col2 FROM ssq_2);"
  sleep 1
) | nc localhost 6789 || echo "Connection failed"

# 等待一会儿
sleep 2

# 检查进程是否还在运行
if ps -p $OBSERVER_PID > /dev/null; then
    echo "Observer still running, killing..."
    kill $OBSERVER_PID
fi

# 等待进程结束
wait $OBSERVER_PID 2>/dev/null || true

echo "==================== Observer Exit Code ===================="
echo $?

echo "==================== Strace Output (exit calls) ===================="
grep -E "exit|EXIT" /tmp/strace_output.txt | tail -20

echo "==================== Observer Log (last 30 lines) ===================="
tail -30 /tmp/miniob/miniob/log/observer.log* 2>/dev/null | grep -v "debug lock\|debug unlock\|DEBUG:" | tail -20

