#!/bin/bash

# 测试缓存功能的脚本

echo "========================================="
echo "Token Cache 测试脚本"
echo "========================================="

# 检查日志文件
LOG_FILE="/data/obcluster/log/observer.log"

if [ ! -f "$LOG_FILE" ]; then
    echo "错误: 日志文件不存在: $LOG_FILE"
    exit 1
fi

echo ""
echo "1. 清空旧日志（保留最后1000行）..."
tail -1000 "$LOG_FILE" > "${LOG_FILE}.tmp"
mv "${LOG_FILE}.tmp" "$LOG_FILE"

echo ""
echo "2. 运行测试查询..."
cd /root/ob/ob-mldr-test

# 运行一个简单的测试查询（只测试几条）
python3 mldr_data_test.py --limit 5

echo ""
echo "3. 检查缓存相关日志..."
echo ""
echo "=== Token Cache 创建 ==="
grep "\[TokenCache\] Creating new cache" "$LOG_FILE" | tail -20

echo ""
echo "=== Token Cache 使用 ==="
grep "\[TokenCache\] Using cached iterator" "$LOG_FILE" | tail -20

echo ""
echo "=== Token Cache 全扫描 ==="
grep "\[TokenCache\] Force full scan" "$LOG_FILE" | tail -20

echo ""
echo "=== Token Cache 数据添加 ==="
grep "\[TokenCache\] Added data to cache" "$LOG_FILE" | tail -20

echo ""
echo "=== Token Cache LRU淘汰 ==="
grep "\[TokenCache\] Evicted LRU" "$LOG_FILE" | tail -20

echo ""
echo "=== Cache Iterator 状态 ==="
grep "\[CacheIter\]" "$LOG_FILE" | tail -20

echo ""
echo "========================================="
echo "测试完成！"
echo "========================================="
