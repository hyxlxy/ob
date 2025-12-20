#!/bin/bash

# 两阶段检索日志提取脚本
# 用于从 observer.log 中提取关键信息

LOG_FILE="/data/obcluster/log/observer.log"

if [ ! -f "$LOG_FILE" ]; then
    echo "❌ 日志文件不存在: $LOG_FILE"
    exit 1
fi

echo "=========================================="
echo "两阶段检索日志分析"
echo "=========================================="

# 统计 Phase1 参数
echo ""
echo "=== Phase1 参数配置 ==="
grep "\[Two-Phase Retrieval\] Phase1 parameters" "$LOG_FILE" | tail -n 10 | while read line; do
    # 提取 matched_capacity 和 scan_limit
    matched=$(echo "$line" | grep -oP 'matched_capacity=\K\d+')
    scan=$(echo "$line" | grep -oP 'scan_limit=\K\d+')
    echo "  粗排候选数: $matched, 扫描上限: $scan"
done

# 统计 Phase1 完成情况
echo ""
echo "=== Phase1 完成统计（最近10次）==="
grep "\[Two-Phase Retrieval\] Phase1 (coarse ranking) complete" "$LOG_FILE" | tail -n 10 | while read line; do
    # 提取候选数和分数
    count=$(echo "$line" | grep -oP 'candidates_count=\K\d+')
    top_score=$(echo "$line" | grep -oP 'top_score=\K[0-9.]+')
    last_score=$(echo "$line" | grep -oP 'last_score=\K[0-9.]+')
    echo "  候选数: $count, 最高分: $top_score, 最低分: $last_score"
done

# 统计总体情况
echo ""
echo "=== 总体统计 ==="
total_phase1=$(grep -c "\[Two-Phase Retrieval\] Phase1 (coarse ranking) complete" "$LOG_FILE")
echo "  Phase1 完成次数: $total_phase1"

# 计算平均候选数
avg_candidates=$(grep "\[Two-Phase Retrieval\] Phase1 (coarse ranking) complete" "$LOG_FILE" | \
    grep -oP 'candidates_count=\K\d+' | \
    awk '{sum+=$1; count++} END {if(count>0) print sum/count; else print 0}')
echo "  平均候选数: $avg_candidates"

echo ""
echo "=========================================="
echo "提示："
echo "1. 如果候选数远小于 matched_capacity，说明扫描不足"
echo "2. 如果最高分和最低分差距很小，说明区分度不够"
echo "3. 建议调整参数："
echo "   - 增加 scan_limit（如 20000）"
echo "   - 调整 matched_capacity（如 2000）"
echo "=========================================="
