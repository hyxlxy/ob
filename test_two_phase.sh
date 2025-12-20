#!/bin/bash

# 两阶段检索测试脚本
# 用于验证粗排+精排的效果

echo "=========================================="
echo "两阶段检索测试"
echo "=========================================="

# 1. 编译
echo ""
echo "[1/4] 编译代码..."
cd /root/ob
bash rebuild_and_test.sh

if [ $? -ne 0 ]; then
    echo "❌ 编译失败"
    exit 1
fi

echo "✅ 编译成功"

# 2. 运行测试
echo ""
echo "[2/4] 运行性能测试..."
cd /root/ob/ob-mldr-test
python3 mldr_data_test.py

# 3. 提取日志
echo ""
echo "[3/4] 提取两阶段检索日志..."
echo ""
echo "=== Phase1 参数 ==="
grep "\[Two-Phase Retrieval\] Phase1 parameters" /data/obcluster/log/observer.log | tail -n 5

echo ""
echo "=== Phase1 完成统计 ==="
grep "\[Two-Phase Retrieval\] Phase1 (coarse ranking) complete" /data/obcluster/log/observer.log | tail -n 5

# 4. 总结
echo ""
echo "[4/4] 测试总结"
echo "=========================================="
echo "请检查以上日志，验证："
echo "1. matched_capacity 是否为 1000（粗排候选数）"
echo "2. scan_limit 是否为 10000（扫描上限）"
echo "3. candidates_count 是否接近 1000"
echo "4. top_score 和 last_score 的分数范围"
echo "=========================================="
