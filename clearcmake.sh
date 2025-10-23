#!/bin/bash


cd build
rm -rf *

cat > run.sh << 'EOF'
#! /bin/bash

# 确保使用正确的构建工具
if command -v ninja &> /dev/null; then
    BUILD_TOOL="ninja"
    BUILD_FILE="build.ninja"
    GENERATOR="Ninja"
else
    BUILD_TOOL="make"
    BUILD_FILE="Makefile"
    GENERATOR="Unix Makefiles"
fi

# 如果构建文件不存在，则运行 CMake
if [ ! -f "$BUILD_FILE" ]; then
    cmake -G "$GENERATOR" ..
fi

# 执行构建
$BUILD_TOOL && ./bin/observer -P cli
EOF

# 创建 clear.sh 文件
cat > clear.sh << 'EOF'
#! /bin/bash

cd miniob
rm -rf *
cd ..
EOF

# 设置两个脚本为可执行权限
chmod a+x run.sh clear.sh

echo "已创建以下两个脚本并设置 a+x 权限："
echo "1. run.sh"
echo "2. clear.sh"