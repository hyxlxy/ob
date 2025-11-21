#!/bin/bash
set -e

echo "=== Perf 诊断工具 ==="
echo "开始时间: $(date)"

# 1. 检查系统状态
echo -e "\n1. 系统状态检查"
echo "负载: $(uptime)"
echo "内存: $(free -h | grep Mem)"
echo "磁盘空间:"
df -h /tmp /root

# 2. 检查 OceanBase 进程状态
echo -e "\n2. OceanBase 进程状态"
OBSERVER_PID=$(pgrep -f "observer.*5000")
if [ -z "$OBSERVER_PID" ]; then
    echo "❌ OceanBase 未运行"
    exit 1
fi
echo "✅ OceanBase 运行中 (PID: $OBSERVER_PID)"
ps -p $OBSERVER_PID -o pid,state,etime,cmd

# 3. 测试 perf 基本功能
echo -e "\n3. 测试 perf 基本功能"
echo "Perf 版本:"
perf --version

# 4. 简单 perf 测试（5秒）
echo -e "\n4. 运行简单 perf 测试（5秒）"
timeout 10 perf record -g -F 99 -p $OBSERVER_PID -o test_simple.data sleep 5
if [ $? -eq 0 ]; then
    echo "✅ 简单测试成功"
    ls -la test_simple.data
else
    echo "❌ 简单测试失败"
fi

# 5. 检查进程状态详细信息
echo -e "\n5. 进程状态详情"
ps -eo pid,state,etime,cmd | grep -E "(perf|observer)" | grep -v grep

# 6. 检查系统限制
echo -e "\n6. 系统限制检查"
ulimit -a
echo "Perf 事件权限:"
cat /proc/sys/kernel/perf_event_paranoid

# 7. 检查内核消息
echo -e "\n7. 最近的内核消息"
dmesg | tail -20 | grep -i -E "(perf|oob|segfault)"

# 8. 测试数据文件生成
echo -e "\n8. 测试数据文件生成"
if [ -f test_simple.data ]; then
    echo "尝试读取 perf 数据..."
    perf script -i test_simple.data 2>&1 | head -5
    echo "文件信息:"
    file test_simple.data
else
    echo "❌ 测试数据文件未生成"
fi

# 清理
rm -f test_simple.data

echo -e "\n=== 诊断完成 ==="
