#!/bin/bash

# 完整的内核赛题测试 + 性能采集脚本（端口改为5000）
set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
MLDR_TEST_DIR="${SCRIPT_DIR}/ob-mldr-test"
FLAMEGRAPH_DIR="${SCRIPT_DIR}/tools/flamegraph"
PERF_DATA="${SCRIPT_DIR}/perf.data"
PERF_SCRIPT="${SCRIPT_DIR}/perf.script"
FLAMEGRAPH_SVG="${SCRIPT_DIR}/flamegraph_kernel.svg"

# 数据库端口（已改为5000）
DB_PORT="5000"

# Java环境变量
JAVA_HOME="/usr/lib/jvm/java-11-openjdk-11.0.25.0.9-2.0.1.an8.x86_64"
JAVA_LIB_PATH="$JAVA_HOME/lib/server"

# 测试脚本参数
TEST_SCRIPT="mldr_data_test_fast.py"
TEST_ARGS=("--lang" "en" "--query_types" "bm25" "--max_insert" "10000" "--max_query" "100" "--skip_warmup" "--test_runs" "1" "--db_port" "${DB_PORT}")

# TEST_PERF_DURATION=300


# 参数处理
if [ $# -gt 0 ]; then
    TEST_SCRIPT="$1"
    shift
    TEST_ARGS=("$@")
    if [[ ! " ${TEST_ARGS[@]} " =~ "--db_port" ]]; then
        TEST_ARGS+=("--db_port" "${DB_PORT}")
    fi
fi

echo "=========================================="
echo "内核赛题测试 + 性能采集工具（端口: ${DB_PORT}）"
echo "=========================================="

if [ $# -eq 0 ]; then
    echo "[INFO] 使用快速测试模式（默认）..."
    echo ""
fi

# 检查 seekdb 是否运行
OBSERVER_PID=$(pgrep -f "observer.*5000" | head -n1)
if [ -z "${OBSERVER_PID}" ]; then
    echo "[ERROR] seekdb 未运行在端口 5000"
    echo "[INFO] 请先启动: ./ad_start_ob.sh"
    exit 1
fi

echo "[INFO] seekdb 正在运行在端口 ${DB_PORT} (PID: ${OBSERVER_PID})"

# 检查测试环境
if [ ! -d "${MLDR_TEST_DIR}" ]; then
    echo "[ERROR] 测试脚本目录不存在: ${MLDR_TEST_DIR}"
    exit 1
fi

if [ ! -f "${MLDR_TEST_DIR}/${TEST_SCRIPT}" ]; then
    echo "[ERROR] 测试脚本不存在: ${MLDR_TEST_DIR}/${TEST_SCRIPT}"
    exit 1
fi

# 设置Python命令
if command -v python3.9 &> /dev/null; then
    PYTHON_CMD=python3.9
elif command -v python3 &> /dev/null; then
    PYTHON_CMD=python3
else
    echo "[ERROR] 未找到 Python 3"
    exit 1
fi

echo "[INFO] 使用 Python: ${PYTHON_CMD}"

# 检查并安装 perf
if ! command -v perf &> /dev/null; then
    echo "[INFO] perf 工具未安装，正在安装..."
    sudo dnf install -y perf
fi

# 检查并下载 FlameGraph
if [ ! -d "${FLAMEGRAPH_DIR}" ]; then
    echo "[INFO] 下载 FlameGraph 工具..."
    mkdir -p "${FLAMEGRAPH_DIR}"
    cd "${FLAMEGRAPH_DIR}"
    if command -v git &> /dev/null; then
        git clone --depth 1 https://github.com/brendangregg/FlameGraph.git .
    else
        wget -q https://github.com/brendangregg/FlameGraph/archive/master.zip
        unzip -q master.zip && mv FlameGraph-master/* . && rm -rf FlameGraph-master master.zip
    fi
    cd "${SCRIPT_DIR}"
fi

# 清理旧数据
rm -f "${PERF_DATA}" "${PERF_SCRIPT}" "${FLAMEGRAPH_SVG}"

echo ""
echo "=========================================="
echo "开始性能采集和测试执行（端口: ${DB_PORT}）"
echo "=========================================="

# 设置Java环境变量
echo "[INFO] 设置Java环境变量..."
export JAVA_HOME="$JAVA_HOME"
export LD_LIBRARY_PATH="$JAVA_LIB_PATH:$LD_LIBRARY_PATH"

# 创建兼容性符号链接
echo "[INFO] 创建兼容性符号链接..."
OLD_JAVA_PATH="/usr/lib/jvm/java-11-openjdk-11.0.22.0.7-0.al8.x86_64/lib/server"
if [ ! -f "${OLD_JAVA_PATH}/libjvm.so" ]; then
    sudo mkdir -p "${OLD_JAVA_PATH}"
    sudo ln -sf "${JAVA_HOME}/lib/server/libjvm.so" "${OLD_JAVA_PATH}/libjvm.so"
    echo "[INFO] 已创建符号链接: ${OLD_JAVA_PATH}/libjvm.so → ${JAVA_HOME}/lib/server/libjvm.so"
else
    echo "[INFO] 符号链接已存在: ${OLD_JAVA_PATH}/libjvm.so"
fi

# 验证Java环境
echo "[INFO] 验证Java环境..."
python -c "
import os
print('JAVA_HOME:', os.environ.get('JAVA_HOME'))
print('LD_LIBRARY_PATH:', os.environ.get('LD_LIBRARY_PATH'))
try:
    from jnius import autoclass
    print('✅ jnius导入成功')
except Exception as e:
    print('❌ jnius导入失败:', e)
    exit(1)
"

# 计算测试时间（根据历史数据估算）
echo "[INFO] 基于历史数据估算测试时间..."
if [ -f "test_duration.log" ]; then
    # 如果有历史数据，使用历史平均值
    HISTORICAL_DURATION=$(tail -1 test_duration.log 2>/dev/null || echo "300")
    PERF_TEST_DURATION=$((HISTORICAL_DURATION + 60))  # 历史时间+1分钟缓冲
else
    # 没有历史数据，使用保守估计
    PERF_TEST_DURATION=400  # 6.5分钟
fi

echo "[INFO] 设置性能采集时间: ${PERF_TEST_DURATION} 秒"

# 启动 perf 采集
echo "[INFO] 启动 perf 性能采集..."
sudo env JAVA_HOME="$JAVA_HOME" LD_LIBRARY_PATH="$JAVA_LIB_PATH:$LD_LIBRARY_PATH" \
    perf record -g -F 99 -p "${OBSERVER_PID}" -o "${PERF_DATA}" -- sleep ${PERF_TEST_DURATION} &
PERF_PID=$!

echo "[INFO] perf 采集已启动 (PID: ${PERF_PID})，将在 ${PERF_TEST_DURATION} 秒后自动停止"

sleep 1
if ! kill -0 ${PERF_PID} 2>/dev/null; then
    echo "[ERROR] perf record 启动失败"
    exit 1
fi

echo "[INFO] perf 采集已启动 (PID: ${PERF_PID})"
sleep 1

# 记录测试开始时间
TEST_START_TIME=$(date +%s)

# 运行测试程序
echo "[INFO] 开始运行测试程序..."
cd "${MLDR_TEST_DIR}"

# 使用env确保Python进程继承正确的环境
env JAVA_HOME="$JAVA_HOME" LD_LIBRARY_PATH="$JAVA_LIB_PATH:$LD_LIBRARY_PATH" \
    ${PYTHON_CMD} "${TEST_SCRIPT}" "${TEST_ARGS[@]}" || {
    TEST_EXIT_CODE=$?
    echo "[WARN] 测试程序执行失败，退出码: ${TEST_EXIT_CODE}"
}

cd "${SCRIPT_DIR}"

# 记录测试结束时间
TEST_END_TIME=$(date +%s)
TEST_DURATION=$((TEST_END_TIME - TEST_START_TIME))

echo ""
echo "[INFO] 测试执行完成，耗时: ${TEST_DURATION} 秒"
echo "[INFO] 停止 perf 采集..."

# 修复后的方式（优雅）：
echo "[INFO] 发送 SIGINT 信号给 perf 进程..."
sudo kill -INT ${PERF_PID} 2>/dev/null

# 给 perf 足够时间写入数据
echo "[INFO] 等待 perf 数据写入（最长5秒）..."
for i in {1..5}; do
    if ! ps -p ${PERF_PID} > /dev/null; then
        echo "[INFO] perf 进程已正常退出"
        break
    fi
    echo "[INFO] 等待中... ($i/5)"
    sleep 1
done

# 如果仍然存在，尝试更温和的方式
if ps -p ${PERF_PID} > /dev/null; then
    echo "[WARN] perf 仍在运行，尝试 SIGTERM..."
    sudo kill -TERM ${PERF_PID} 2>/dev/null
    sleep 10
    
    # 最后手段：SIGKILL，但记录警告
    if ps -p ${PERF_PID} > /dev/null; then
        echo "[ERROR] 必须强制终止 perf，数据可能不完整"
        sudo kill -KILL ${PERF_PID} 2>/dev/null
    fi
fi

# 额外等待确保文件系统同步
sleep 3

# echo "[INFO] 性能数据采集完成，正在生成火焰图..."

# # 生成火焰图
# sudo perf script -i "${PERF_DATA}" > "${PERF_SCRIPT}" || {
#     echo "[ERROR] perf script 失败"
#     exit 1
# }

# "${FLAMEGRAPH_DIR}/stackcollapse-perf.pl" < "${PERF_SCRIPT}" 2>/dev/null | \
#     "${FLAMEGRAPH_DIR}/flamegraph.pl" --title "SeekDB Kernel Competition - Fulltext Search Performance (Port: ${DB_PORT}, Test Duration: ${TEST_DURATION}s)" \
#     --width 1800 --height 20 2>/dev/null > "${FLAMEGRAPH_SVG}" || {
#     echo "[ERROR] 生成火焰图失败"
#     exit 1
# }


# 删除原来的 sleep 3，替换为更智能的等待逻辑

echo "[INFO] 性能数据采集完成，正在生成火焰图..."

# 1. 等待数据写入完成（根据文件大小动态调整）
echo "[INFO] 等待 perf 数据写入完成..."
DATA_SIZE=0
PREV_SIZE=0
STABLE_COUNT=0
MAX_WAIT=30  # 最大等待30秒

for i in $(seq 1 $MAX_WAIT); do
    if [ -f "${PERF_DATA}" ]; then
        DATA_SIZE=$(stat -c%s "${PERF_DATA}" 2>/dev/null || echo 0)
        
        # 检查文件大小是否稳定（连续3次不变）
        if [ "$DATA_SIZE" -eq "$PREV_SIZE" ] && [ "$DATA_SIZE" -gt 0 ]; then
            STABLE_COUNT=$((STABLE_COUNT + 1))
            if [ $STABLE_COUNT -ge 3 ]; then
                echo "[INFO] 数据文件大小已稳定: ${DATA_SIZE} 字节"
                break
            fi
        else
            STABLE_COUNT=0
        fi
        
        PREV_SIZE=$DATA_SIZE
        echo "[INFO] 等待数据写入... ($i/$MAX_WAIT) 当前大小: ${DATA_SIZE} 字节"
    else
        echo "[INFO] 等待数据文件创建... ($i/$MAX_WAIT)"
    fi
    
    sleep 1
done

# 2. 检查数据文件完整性
if [ ! -f "${PERF_DATA}" ] || [ ! -s "${PERF_DATA}" ]; then
    echo "[ERROR] perf.data 文件不存在或为空"
    echo "[INFO] 尝试使用备用数据源..."
    
    # 备用方案：使用最近的有效数据
    if find . -name "perf.data.*" -size +1M | head -1; then
        LATEST_PERF=$(find . -name "perf.data.*" -size +1M -printf "%T@ %p\n" | sort -nr | head -1 | cut -d' ' -f2-)
        echo "[INFO] 使用备用数据文件: ${LATEST_PERF}"
        cp "${LATEST_PERF}" "${PERF_DATA}"
    else
        echo "[ERROR] 无可用性能数据，跳过火焰图生成"
        exit 1
    fi
fi

# 3. 验证文件格式
echo "[INFO] 检查数据文件格式..."
file "${PERF_DATA}" | grep -q "perf data"
if [ $? -ne 0 ]; then
    echo "[WARN] 数据文件格式可能不正确，尝试修复..."
    # 尝试修复损坏的文件
    sudo perf inject -i "${PERF_DATA}" -o "${PERF_DATA}.fixed" 2>/dev/null
    if [ $? -eq 0 ] && [ -s "${PERF_DATA}.fixed" ]; then
        mv "${PERF_DATA}.fixed" "${PERF_DATA}"
        echo "[INFO] 文件修复成功"
    fi
fi

# 4. 生成火焰图（带错误处理）
echo "[INFO] 生成 perf script 输出..."
if ! sudo perf script -i "${PERF_DATA}" > "${PERF_SCRIPT}" 2>/dev/null; then
    echo "[ERROR] perf script 失败，尝试使用简化模式..."
    # 简化模式：只生成基本信息
    sudo perf script -i "${PERF_DATA}" -F comm,pid,tid,time,event,ip,sym 2>/dev/null > "${PERF_SCRIPT}" || {
        echo "[ERROR] 无法生成 perf script 输出"
        # 创建简单的错误报告
        echo "性能数据生成失败" > "${FLAMEGRAPH_SVG}"
        echo "<svg><!-- 火焰图生成失败 --></svg>" >> "${FLAMEGRAPH_SVG}"
        exit 1
    }
fi

# 5. 生成火焰图
echo "[INFO] 生成火焰图..."
if [ -s "${PERF_SCRIPT}" ]; then
    "${FLAMEGRAPH_DIR}/stackcollapse-perf.pl" < "${PERF_SCRIPT}" 2>/dev/null | \
        "${FLAMEGRAPH_DIR}/flamegraph.pl" \
            --title "SeekDB Kernel Competition - Fulltext Search Performance (Port: ${DB_PORT}, Test Duration: ${TEST_DURATION}s)" \
            --width 1800 --height 20 2>/dev/null > "${FLAMEGRAPH_SVG}"
    
    if [ $? -eq 0 ] && [ -s "${FLAMEGRAPH_SVG}" ]; then
        echo "[INFO] ✅ 火焰图生成成功"
    else
        echo "[ERROR] 火焰图生成失败，创建占位符"
        # 创建简单的SVG占位符
        cat > "${FLAMEGRAPH_SVG}" << 'EOF'
<svg width="1800" height="400" xmlns="http://www.w3.org/2000/svg">
  <rect width="100%" height="100%" fill="#f0f0f0"/>
  <text x="50%" y="50%" text-anchor="middle" dominant-baseline="middle" font-family="Arial" font-size="20" fill="#666">
    火焰图生成失败 - 性能数据可能不完整
  </text>
  <text x="50%" y="60%" text-anchor="middle" dominant-baseline="middle" font-family="Arial" font-size="14" fill="#999">
    测试端口: PORT_PLACEHOLDER, 持续时间: DURATION_PLACEHOLDER
  </text>
</svg>
EOF
        # 替换占位符
        sed -i "s/PORT_PLACEHOLDER/${DB_PORT}/g; s/DURATION_PLACEHOLDER/${TEST_DURATION}s/g" "${FLAMEGRAPH_SVG}"
    fi
else
    echo "[ERROR] perf.script 文件为空"
    exit 1
fi






echo ""
echo "=========================================="
echo "✅ 测试和性能采集完成！（端口: ${DB_PORT}）"
echo "=========================================="
echo "测试耗时: ${TEST_DURATION} 秒"
echo "数据库端口: ${DB_PORT}"
echo "火焰图位置: ${FLAMEGRAPH_SVG}"
echo ""
echo "查看方式:"
echo "  1. 在浏览器中打开: file://${FLAMEGRAPH_SVG}"
echo "  2. 或使用命令: xdg-open ${FLAMEGRAPH_SVG}"
echo ""
echo "分析提示:"
echo "  - 重点关注全文检索相关函数:"
echo "    * ob_ft_index_* (全文索引)"
echo "    * match_against_* (MATCH AGAINST)"
echo "    * bm25_* (BM25算法)"
echo "    * ObTableScan* / ObIndexScan* (扫描操作)"
echo "  - 火焰图宽度表示函数占用 CPU 时间"
echo "  - 点击可以放大查看细节"
echo "=========================================="