#!/bin/bash
set -e

echo "=== OceanBase 一键编译脚本 ==="

# 配置参数
BUILD_TYPE="${1:-release}"  # 默认编译release版本，支持debug/release
JOBS="${2:-4}"           # 默认并行编译数
OB_SRC_DIR="$(pwd)"       # OceanBase源码目录

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 日志函数
log_info() { echo -e "${GREEN}[INFO]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# 检查目录
if [ ! -f "build.sh" ]; then
    log_error "请在OceanBase源码根目录运行此脚本"
    exit 1
fi

# 检查依赖
log_info "检查编译依赖..."
if ! command -v gcc &> /dev/null; then
    log_error "gcc 未安装"
    exit 1
fi

if ! command -v cmake &> /dev/null; then
    log_error "cmake 未安装"
    exit 1
fi


# 开始编译
log_info "开始编译 OceanBase (类型: $BUILD_TYPE, 并行数: 4)..."

if [ "$BUILD_TYPE" = "debug" ]; then
    # 调试版本，带完整符号信息
    bash build.sh debug -DOB_DEBUG_SYMBOLS=ON -DADVISE=ON --make -j4
else
    # 发布版本，但仍保留符号用于perf
    bash build.sh release -DOB_DEBUG_SYMBOLS=ON  -DADVISE=ON --make -j4
fi

# 检查编译结果
if [ -f "build_${BUILD_TYPE}/src/observer/observer" ]; then
    log_info "编译成功！"
    file "build_${BUILD_TYPE}/src/observer/observer"
    
    # 创建符号链接方便使用
    # ln -sf "$(pwd)/build_${BUILD_TYPE}/src/observer/observer" ./observer_binary
    # log_info "已创建符号链接: ./observer_binary"
else
    log_error "编译失败，observer二进制文件未找到"
    exit 1
fi

log_info "=== 编译完成 ==="