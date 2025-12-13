#!/bin/bash
# 停止已有进程
pkill -f "observer.*5000" || true
sleep 2

# 启动新进程（确保包含端口5000）
./build_release/src/observer/observer \
    -P 5000  &

echo "OceanBase 启动在端口 5000，PID: $!"
