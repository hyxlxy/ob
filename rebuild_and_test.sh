#!/bin/bash

set -e  # Exit on error

echo "=========================================="
echo "Step 1: Compiling OceanBase"
echo "=========================================="
cd /root/ob
bash build.sh release -DOB_USE_CCACHE=ON --make -j3

echo ""
echo "=========================================="
echo "Step 2: Destroying old cluster"
echo "=========================================="
./tools/deploy/obd.sh destroy rm -n obcluster || true

echo ""
echo "=========================================="
echo "Step 3: Preparing deployment"
echo "=========================================="
./tools/deploy/obd.sh prepare -p /tmp/obtest

echo ""
echo "=========================================="
echo "Step 4: Deploying new cluster"
echo "=========================================="
# Use expect or printf to handle the interactive prompt
./tools/deploy/obd.sh deploy -c /data/obcluster.yaml

echo ""
echo "=========================================="
echo "Step 5: Waiting for cluster to be ready"
echo "=========================================="
sleep 5

echo ""
echo "=========================================="
echo "Step 6: Running MLDR test"
echo "=========================================="
cd /root/ob/ob-mldr-test
python3.9 mldr_data_test.py

echo ""
echo "=========================================="
echo "Test Complete!"
echo "=========================================="
