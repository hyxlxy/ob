#!/bin/bash
# 修复版 - 内核赛题测试 + 性能采集

set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
MLDR_TEST_DIR="${SCRIPT_DIR}/ob-mldr-test"
FLAMEGRAPH_DIR="${SCRIPT_DIR}/tools/flamegraph"
PERF_DATA="${SCRIPT_DIR}/perf.data"
PERF_SCRIPT="${SCRIPT_DIR}/perf.script"
FLAMEGRAPH_SVG="${SCRIPT_DIR}/flamegraph.svg"

# 自动下载 FlameGraph 工具
setup_flamegraph() {
    if [ ! -d "${FLAMEGRAPH_DIR}" ]; then
        echo "[INFO] 下载 FlameGraph 工具..."
        mkdir -p "${SCRIPT_DIR}/tools"
        cd "${SCRIPT_DIR}/tools"
        if command -v git &> /dev/null; then
            git clone https://github.com/brendangregg/FlameGraph.git flamegraph
        else
            echo "[ERROR] 请先安装 git: yum install git -y"
            exit 1
        fi
        cd "${SCRIPT_DIR}"
    fi
    
    if [ ! -f "${FLAMEGRAPH_DIR}/stackcollapse-perf.pl" ] || [ ! -f "${FLAMEGRAPH_DIR}/flamegraph.pl" ]; then
        echo "[ERROR] FlameGraph 工具不完整，请重新下载"
        exit 1
    fi
    
    chmod +x "${FLAMEGRAPH_DIR}"/*.pl
    echo "[INFO] FlameGraph 工具就绪"
}

create_placeholder_flamegraph() {
    local duration="$1"
    local reason="$2"
    cat > "${FLAMEGRAPH_SVG}" << EOF
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg width="1200" height="200" xmlns="http://www.w3.org/2000/svg">
  <rect width="100%" height="100%" fill="#f0f0f0"/>
  <text x="50%" y="45%" text-anchor="middle" font-family="Arial" font-size="16" fill="#666">
    OceanBase 性能测试完成
  </text>
  <text x="50%" y="65%" text-anchor="middle" font-family="Arial" font-size="12" fill="#999">
    测试时间: ${duration}s | 端口: 5000
  </text>
  <text x="50%" y="80%" text-anchor="middle" font-family="Arial" font-size="10" fill="#cc0000">
    ⚠️ 火焰图生成失败: ${reason}
  </text>
</svg>
EOF
}

echo "=========================================="
echo "OceanBase 内核赛题测试（修复版）"
echo "=========================================="

# 设置 FlameGraph 工具
setup_flamegraph

# 检查环境
DB_PORT="5000"
OBSERVER_PID=$(pgrep -f "observer.*5000" | head -n1)

if [ -z "$OBSERVER_PID" ]; then
    echo "[ERROR] OceanBase 未运行在端口 5000"
    exit 1
fi

echo "[INFO] OceanBase 运行中 (PID: $OBSERVER_PID)"

# 清理旧数据
rm -f "${PERF_DATA}" "${PERF_SCRIPT}" "${FLAMEGRAPH_SVG}" "${SCRIPT_DIR}/folded.txt"

# 设置超时时间
TEST_TIMEOUT=222

echo "[INFO] 启动性能采集（超时: ${TEST_TIMEOUT}秒）..."

# 使用 timeout 确保 perf 一定会停止
timeout ${TEST_TIMEOUT}s perf record -g -F 99 -p "${OBSERVER_PID}" -o "${PERF_DATA}" &
PERF_PID=$!

echo "[INFO] perf 已启动 (PID: $PERF_PID)"

# 运行测试
echo "[INFO] 开始运行 MLDR 测试..."
TEST_START=$(date +%s)

cd "${MLDR_TEST_DIR}"
python3 mldr_data_test_fast.py --lang en --query_types bm25 --db_port 5000 || true
cd "${SCRIPT_DIR}"

TEST_END=$(date +%s)
TEST_DURATION=$((TEST_END - TEST_START))

echo "[INFO] 测试完成，耗时: ${TEST_DURATION}秒"

# 优雅停止 perf
echo "[INFO] 停止性能采集..."
if kill -0 $PERF_PID 2>/dev/null; then
    kill -SIGINT $PERF_PID 2>/dev/null
    sleep 5
fi

# 确保进程结束
kill -9 $PERF_PID 2>/dev/null || true
wait $PERF_PID 2>/dev/null || true

# 生成火焰图
echo "[INFO] 生成火焰图..."
if [ -f "${PERF_DATA}" ] && [ -s "${PERF_DATA}" ]; then
    echo "[INFO] 处理 perf 数据..."
    
    # 修复权限问题
    if [ -O "${PERF_DATA}" ]; then
        perf script -i "${PERF_DATA}" > "${PERF_SCRIPT}" 2>&1
    else
        sudo perf script -i "${PERF_DATA}" > "${PERF_SCRIPT}" 2>&1
    fi
    
    if [ $? -eq 0 ] && [ -s "${PERF_SCRIPT}" ]; then
        echo "[INFO] 生成折叠数据..."
        
        # 检查 perf script 输出格式
        if head -5 "${PERF_SCRIPT}" | grep -q "obser"; then
            "${FLAMEGRAPH_DIR}/stackcollapse-perf.pl" "${PERF_SCRIPT}" > "${SCRIPT_DIR}/folded.txt" 2>&1
            
            if [ $? -eq 0 ] && [ -s "${SCRIPT_DIR}/folded.txt" ]; then
                echo "[INFO] 生成SVG火焰图..."
                
                # 检查折叠数据格式
                FOLDED_LINES=$(wc -l < "${SCRIPT_DIR}/folded.txt")
                echo "[DEBUG] 折叠数据行数: ${FOLDED_LINES}"
                
                if [ "${FOLDED_LINES}" -gt 10 ]; then
                    # 生成火焰图，保存错误信息
                    FLAME_ERROR="${SCRIPT_DIR}/flame_error.log"
                    "${FLAMEGRAPH_DIR}/flamegraph.pl" --title "OceanBase Test (${TEST_DURATION}s)" \
                        "${SCRIPT_DIR}/folded.txt" > "${FLAMEGRAPH_SVG}" 2>"${FLAME_ERROR}"
                    
                    FLAME_EXIT_CODE=$?
                    
                    if [ $FLAME_EXIT_CODE -eq 0 ] && [ -f "${FLAMEGRAPH_SVG}" ]; then
                        # 更宽松的SVG验证
                        if head -5 "${FLAMEGRAPH_SVG}" | grep -q -E "(svg|SVG|xml)" ; then
                            echo "✅ 火焰图生成成功: ${FLAMEGRAPH_SVG}"
                            # 清理中间文件
                            rm -f "${SCRIPT_DIR}/folded.txt" "${PERF_SCRIPT}" "${FLAME_ERROR}"
                            
                            # 验证SVG文件
                            if command -v xmllint >/dev/null 2>&1; then
                                if xmllint --noout "${FLAMEGRAPH_SVG}" 2>/dev/null; then
                                    echo "✅ SVG 文件格式验证通过"
                                else
                                    echo "⚠️ SVG 文件格式验证警告，但文件已生成"
                                fi
                            else
                                echo "⚠️ 跳过 SVG 验证 (xmllint 未安装)"
                            fi
                        else
                            echo "❌ 生成的SVG文件格式错误"
                            echo "[DEBUG] SVG文件开头:"
                            head -5 "${FLAMEGRAPH_SVG}"
                            echo "[DEBUG] flamegraph.pl 错误输出:"
                            cat "${FLAME_ERROR}"
                            create_placeholder_flamegraph "${TEST_DURATION}" "SVG格式无效"
                        fi
                    else
                        echo "❌ 火焰图生成失败，退出码: $FLAME_EXIT_CODE"
                        echo "[DEBUG] flamegraph.pl 错误输出:"
                        cat "${FLAME_ERROR}"
                        create_placeholder_flamegraph "${TEST_DURATION}" "生成命令失败"
                    fi
                else
                    echo "❌ 折叠数据行数太少 (${FOLDED_LINES})，无法生成有意义的火焰图"
                    create_placeholder_flamegraph "${TEST_DURATION}" "数据量不足"
                fi
            else
                echo "❌ 数据折叠失败"
                echo "[DEBUG] stackcollapse 输出:"
                "${FLAMEGRAPH_DIR}/stackcollapse-perf.pl" "${PERF_SCRIPT}" 2>&1 | head -10
                create_placeholder_flamegraph "${TEST_DURATION}" "数据折叠失败"
            fi
        else
            echo "❌ perf script 输出不包含 OceanBase 相关数据"
            create_placeholder_flamegraph "${TEST_DURATION}" "无OceanBase数据"
        fi
    else
        echo "❌ perf script 执行失败"
        create_placeholder_flamegraph "${TEST_DURATION}" "perf script失败"
    fi
else
    echo "❌ 无性能数据或文件为空"
    create_placeholder_flamegraph "${TEST_DURATION}" "无性能数据"
fi

echo ""
echo "=========================================="
echo "🎉 测试完成！"
echo "=========================================="
echo "📊 测试耗时: ${TEST_DURATION} 秒"
echo "🔌 数据库端口: ${DB_PORT}"
echo "📈 火焰图: ${FLAMEGRAPH_SVG}"
echo ""
echo "🌐 查看命令: xdg-open '${FLAMEGRAPH_SVG}' || open '${FLAMEGRAPH_SVG}'"
echo "=========================================="