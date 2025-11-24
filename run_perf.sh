#!/bin/bash
# 修复版 - 内核赛题测试 + 性能采集

set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
MLDR_TEST_DIR="${SCRIPT_DIR}/ob-mldr-test"
FLAMEGRAPH_DIR="${SCRIPT_DIR}/tools/flamegraph"
PERF_DATA="${SCRIPT_DIR}/perf.data"
PERF_SCRIPT="${SCRIPT_DIR}/perf.script"
FLAMEGRAPH_SVG="${SCRIPT_DIR}/flamegraph.svg"
PERF_LOG="${SCRIPT_DIR}/perf.log"

# 配置参数
DB_PORT="5000"
OBSERVER_PID=$(pgrep -f "observer.*5000" | head -n1)

echo "=========================================="
echo "OceanBase 内核赛题测试（修复版）"
echo "=========================================="

# 检查环境
if [ -z "$OBSERVER_PID" ]; then
    echo "[ERROR] OceanBase 未运行在端口 5000"
    exit 1
fi

echo "[INFO] OceanBase 运行中 (PID: $OBSERVER_PID)"

# 清理旧数据
rm -f "${PERF_DATA}" "${PERF_SCRIPT}" "${FLAMEGRAPH_SVG}" "${PERF_LOG}"

# 设置超时时间
TEST_TIMEOUT=240

create_placeholder_flamegraph() {
    local duration="$1"
    cat > "${FLAMEGRAPH_SVG}" << EOF
<svg width="1200" height="200">
  <rect width="100%" height="100%" fill="#f0f0f0"/>
  <text x="50%" y="50%" text-anchor="middle" font-family="Arial" font-size="16" fill="#666">
    OceanBase 性能测试完成
  </text>
  <text x="50%" y="70%" text-anchor="middle" font-family="Arial" font-size="12" fill="#999">
    测试时间: ${duration}s | 端口: 5000
  </text>
</svg>
EOF
}

# 方案1：使用 nohup 和重定向（推荐）
echo "[INFO] 启动性能采集（超时: ${TEST_TIMEOUT}秒）..."

# 使用 nohup 和重定向，避免输出到终端
nohup timeout ${TEST_TIMEOUT} perf record -g -F 99 -p "${OBSERVER_PID}" -o "${PERF_DATA}" > "${PERF_LOG}" 2>&1 &
PERF_PID=$!

echo "[INFO] perf 已启动 (PID: $PERF_PID)，日志输出到: ${PERF_LOG}"

# 运行测试
echo "[INFO] 开始运行 MLDR 测试..."
TEST_START=$(date +%s)

cd "${MLDR_TEST_DIR}"
if python3 mldr_data_test_fast.py --lang en --query_types bm25 --db_port 5000; then
    TEST_RESULT="成功"
else
    TEST_RESULT="失败"
fi
cd "${SCRIPT_DIR}"

TEST_END=$(date +%s)
TEST_DURATION=$((TEST_END - TEST_START))

echo "[INFO] 测试${TEST_RESULT}，耗时: ${TEST_DURATION}秒"

# 等待 perf 完成或超时
echo "[INFO] 等待性能采集完成..."
wait $PERF_PID 2>/dev/null || true

# 确保 perf 进程已停止
pkill -f "perf record -g -F 99 -p ${OBSERVER_PID}" 2>/dev/null || true
sleep 2

# 生成火焰图
echo "[INFO] 生成火焰图..."
if [ -f "${PERF_DATA}" ] && [ -s "${PERF_DATA}" ]; then
    echo "[INFO] 处理性能数据..."
    if perf script -i "${PERF_DATA}" > "${PERF_SCRIPT}" 2>> "${PERF_LOG}"; then
        if [ -f "${FLAMEGRAPH_DIR}/stackcollapse-perf.pl" ] && [ -f "${FLAMEGRAPH_DIR}/flamegraph.pl" ]; then
            "${FLAMEGRAPH_DIR}/stackcollapse-perf.pl" < "${PERF_SCRIPT}" 2>> "${PERF_LOG}" | \
            "${FLAMEGRAPH_DIR}/flamegraph.pl" --title "OceanBase Test" > "${FLAMEGRAPH_SVG}" 2>> "${PERF_LOG}"
            
            if [ -f "${FLAMEGRAPH_SVG}" ]; then
                echo "✅ 火焰图生成成功: ${FLAMEGRAPH_SVG}"
            else
                echo "❌ 火焰图生成失败"
                create_placeholder_flamegraph "${TEST_DURATION}"
            fi
        else
            echo "❌ FlameGraph 工具未找到"
            create_placeholder_flamegraph "${TEST_DURATION}"
        fi
    else
        echo "❌ 处理性能数据失败"
        create_placeholder_flamegraph "${TEST_DURATION}"
    fi
else
    echo "❌ 无性能数据或数据文件为空"
    create_placeholder_flamegraph "${TEST_DURATION}"
fi

echo ""
echo "=========================================="
echo "🎉 测试完成！"
echo "=========================================="
echo "📊 测试耗时: ${TEST_DURATION} 秒"
echo "📋 测试结果: ${TEST_RESULT}"
echo "🔌 数据库端口: ${DB_PORT}"
echo "📈 火焰图: ${FLAMEGRAPH_SVG}"
echo "📄 性能日志: ${PERF_LOG}"
echo ""
echo "🌐 查看火焰图: xdg-open '${FLAMEGRAPH_SVG}' 2>/dev/null || echo '请手动打开: ${FLAMEGRAPH_SVG}'"
echo "=========================================="