#!/bin/bash
# 最终简化版 - 内核赛题测试 + 性能采集

set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
MLDR_TEST_DIR="${SCRIPT_DIR}/ob-mldr-test"
FLAMEGRAPH_DIR="${SCRIPT_DIR}/tools/flamegraph"
PERF_DATA="${SCRIPT_DIR}/perf.data"
PERF_SCRIPT="${SCRIPT_DIR}/perf.script"
FLAMEGRAPH_SVG="${SCRIPT_DIR}/flamegraph.svg"

# 配置参数
DB_PORT="5000"
OBSERVER_PID=$(pgrep -f "observer.*5000" | head -n1)

echo "=========================================="
echo "OceanBase 内核赛题测试（简化版）"
echo "=========================================="

# 检查环境
if [ -z "$OBSERVER_PID" ]; then
    echo "[ERROR] OceanBase 未运行在端口 5000"
    exit 1
fi

echo "[INFO] OceanBase 运行中 (PID: $OBSERVER_PID)"

# 清理旧数据
rm -f "${PERF_DATA}" "${PERF_SCRIPT}" "${FLAMEGRAPH_SVG}"

# 设置超时时间
TEST_TIMEOUT=240

echo "[INFO] 启动性能采集（超时: ${TEST_TIMEOUT}秒）..."

# 使用 timeout 确保 perf 一定会停止
timeout ${TEST_TIMEOUT}s sudo perf record -g -F 99 -p "${OBSERVER_PID}" -o "${PERF_DATA}" &
PERF_PID=$!

echo "[INFO] perf 已启动 (PID: $PERF_PID)"

# 运行测试
echo "[INFO] 开始运行 MLDR 测试..."
TEST_START=$(date +%s)

cd "${MLDR_TEST_DIR}"
python3 mldr_data_test_fast.py --lang en --query_types bm25 --db_port 5000
cd "${SCRIPT_DIR}"

TEST_END=$(date +%s)
TEST_DURATION=$((TEST_END - TEST_START))

echo "[INFO] 测试完成，耗时: ${TEST_DURATION}秒"

# 等待 perf 完成
echo "[INFO] 等待性能采集完成..."
wait $PERF_PID 2>/dev/null || true
sudo kill -9 $PERF_PID 2>/dev/null || true
sleep 2

# 生成火焰图
echo "[INFO] 生成火焰图..."
if [ -f "${PERF_DATA}" ] && [ -s "${PERF_DATA}" ]; then
    sudo perf script -i "${PERF_DATA}" > "${PERF_SCRIPT}" 2>/dev/null && \
    "${FLAMEGRAPH_DIR}/stackcollapse-perf.pl" < "${PERF_SCRIPT}" 2>/dev/null | \
    "${FLAMEGRAPH_DIR}/flamegraph.pl" --title "OceanBase Test" > "${FLAMEGRAPH_SVG}" 2>/dev/null
    
    if [ -f "${FLAMEGRAPH_SVG}" ]; then
        echo "✅ 火焰图生成成功"
    else
        echo "❌ 火焰图生成失败，创建占位图"
        create_placeholder_flamegraph
    fi
else
    echo "❌ 无性能数据，创建占位图"
    create_placeholder_flamegraph
fi

create_placeholder_flamegraph() {
    cat > "${FLAMEGRAPH_SVG}" << 'EOF'
<svg width="1200" height="200">
  <rect width="100%" height="100%" fill="#f0f0f0"/>
  <text x="50%" y="50%" text-anchor="middle" font-family="Arial" font-size="16" fill="#666">
    OceanBase 性能测试完成
  </text>
  <text x="50%" y="70%" text-anchor="middle" font-family="Arial" font-size="12" fill="#999">
    测试时间: DURATIONs | 端口: 5000
  </text>
</svg>
EOF
    sed -i "s/DURATION/${TEST_DURATION}/g" "${FLAMEGRAPH_SVG}"
}

echo ""
echo "=========================================="
echo "🎉 测试完成！"
echo "=========================================="
echo "📊 测试耗时: ${TEST_DURATION} 秒"
echo "🔌 数据库端口: ${DB_PORT}"
echo "📈 火焰图: ${FLAMEGRAPH_SVG}"
echo ""
echo "🌐 查看命令: xdg-open ${FLAMEGRAPH_SVG}"
echo "=========================================="


# # 最终简化版 - 内核赛题测试 + 性能采集

# set -e

# SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
# MLDR_TEST_DIR="${SCRIPT_DIR}/ob-mldr-test"
# FLAMEGRAPH_DIR="${SCRIPT_DIR}/tools/flamegraph"
# PERF_DATA="${SCRIPT_DIR}/perf.data"
# PERF_SCRIPT="${SCRIPT_DIR}/perf.script"
# FLAMEGRAPH_SVG="${SCRIPT_DIR}/flamegraph.svg"

# # 配置参数
# DB_PORT="5000"
# OBSERVER_PID=$(pgrep -f "observer.*5000" | head -n1)

# # 提前定义函数
# create_placeholder_flamegraph() {
#     local duration="$1"
#     cat > "${FLAMEGRAPH_SVG}" << EOF
# <?xml version="1.0" encoding="UTF-8" standalone="no"?>
# <svg width="1200" height="200" xmlns="http://www.w3.org/2000/svg">
#   <rect width="100%" height="100%" fill="#f0f0f0"/>
#   <text x="50%" y="45%" text-anchor="middle" font-family="Arial" font-size="16" fill="#666">
#     OceanBase 性能测试完成
#   </text>
#   <text x="50%" y="65%" text-anchor="middle" font-family="Arial" font-size="12" fill="#999">
#     测试时间: ${duration}s | 端口: 5000
#   </text>
#   <text x="50%" y="80%" text-anchor="middle" font-family="Arial" font-size="10" fill="#cc0000">
#     ⚠️ 火焰图生成失败，请检查 perf 数据
#   </text>
# </svg>
# EOF
# }

# echo "=========================================="
# echo "OceanBase 内核赛题测试（简化版）"
# echo "=========================================="

# # 检查环境
# if [ -z "$OBSERVER_PID" ]; then
#     echo "[ERROR] OceanBase 未运行在端口 5000"
#     exit 1
# fi

# echo "[INFO] OceanBase 运行中 (PID: $OBSERVER_PID)"

# # 检查 FlameGraph 工具
# if [ ! -f "${FLAMEGRAPH_DIR}/stackcollapse-perf.pl" ] || [ ! -f "${FLAMEGRAPH_DIR}/flamegraph.pl" ]; then
#     echo "[ERROR] FlameGraph 工具不存在: ${FLAMEGRAPH_DIR}"
#     exit 1
# fi

# # 清理旧数据
# rm -f "${PERF_DATA}" "${PERF_SCRIPT}" "${FLAMEGRAPH_SVG}"

# # 设置超时时间
# TEST_TIMEOUT=222

# echo "[INFO] 启动性能采集（超时: ${TEST_TIMEOUT}秒）..."

# # 使用 timeout 确保 perf 一定会停止
# timeout ${TEST_TIMEOUT}s perf record -g -F 99 -p "${OBSERVER_PID}" -o "${PERF_DATA}" &
# PERF_PID=$!

# echo "[INFO] perf 已启动 (PID: $PERF_PID)"

# # 运行测试
# echo "[INFO] 开始运行 MLDR 测试..."
# TEST_START=$(date +%s)

# cd "${MLDR_TEST_DIR}"
# python3 mldr_data_test_fast.py --lang en --query_types bm25 --db_port 5000 || true
# cd "${SCRIPT_DIR}"

# TEST_END=$(date +%s)
# TEST_DURATION=$((TEST_END - TEST_START))

# echo "[INFO] 测试完成，耗时: ${TEST_DURATION}秒"

# # 优雅停止 perf
# echo "[INFO] 停止性能采集..."
# if kill -0 $PERF_PID 2>/dev/null; then
#     kill -SIGINT $PERF_PID 2>/dev/null
#     sleep 5
# fi

# # 确保进程结束
# kill -9 $PERF_PID 2>/dev/null || true
# wait $PERF_PID 2>/dev/null || true

# # 生成火焰图
# echo "[INFO] 生成火焰图..."
# if [ -f "${PERF_DATA}" ] && [ -s "${PERF_DATA}" ]; then
#     echo "[INFO] 处理 perf 数据..."
    
#     # 修复权限问题
#     if [ -O "${PERF_DATA}" ]; then
#         # 如果文件属于当前用户，直接处理
#         perf script -i "${PERF_DATA}" > "${PERF_SCRIPT}" 2>&1
#     else
#         # 如果文件属于其他用户（如root），使用sudo
#         sudo perf script -i "${PERF_DATA}" > "${PERF_SCRIPT}" 2>&1
#     fi
    
#     if [ $? -eq 0 ] && [ -s "${PERF_SCRIPT}" ]; then
#         echo "[INFO] 生成折叠数据..."
#         "${FLAMEGRAPH_DIR}/stackcollapse-perf.pl" "${PERF_SCRIPT}" > "${SCRIPT_DIR}/folded.txt" 2>&1
        
#         if [ $? -eq 0 ] && [ -s "${SCRIPT_DIR}/folded.txt" ]; then
#             echo "[INFO] 生成SVG火焰图..."
#             "${FLAMEGRAPH_DIR}/flamegraph.pl" --title "OceanBase Test (${TEST_DURATION}s)" \
#                 "${SCRIPT_DIR}/folded.txt" > "${FLAMEGRAPH_SVG}" 2>&1
            
#             if [ $? -eq 0 ] && [ -f "${FLAMEGRAPH_SVG}" ]; then
#                 # 验证SVG文件格式
#                 if head -1 "${FLAMEGRAPH_SVG}" | grep -q "svg"; then
#                     echo "✅ 火焰图生成成功: ${FLAMEGRAPH_SVG}"
#                     # 清理中间文件
#                     rm -f "${SCRIPT_DIR}/folded.txt"
#                 else
#                     echo "❌ 生成的SVG文件格式错误"
#                     create_placeholder_flamegraph "${TEST_DURATION}"
#                 fi
#             else
#                 echo "❌ 火焰图生成失败"
#                 create_placeholder_flamegraph "${TEST_DURATION}"
#             fi
#         else
#             echo "❌ 数据折叠失败"
#             create_placeholder_flamegraph "${TEST_DURATION}"
#         fi
#     else
#         echo "❌ perf script 执行失败"
#         create_placeholder_flamegraph "${TEST_DURATION}"
#     fi
# else
#     echo "❌ 无性能数据或文件为空"
#     create_placeholder_flamegraph "${TEST_DURATION}"
# fi

# echo ""
# echo "=========================================="
# echo "🎉 测试完成！"
# echo "=========================================="
# echo "📊 测试耗时: ${TEST_DURATION} 秒"
# echo "🔌 数据库端口: ${DB_PORT}"
# echo "📈 火焰图: ${FLAMEGRAPH_SVG}"
# echo ""
# echo "🌐 查看命令: xdg-open '${FLAMEGRAPH_SVG}' || open '${FLAMEGRAPH_SVG}'"
# echo "=========================================="