#!/usr/bin/env python3
"""
完整的MiniOB测试覆盖率分析工具
分析MiniOB题目.txt中的所有功能模块，检查对应的测试文件是否存在
"""

import os
import re
from typing import Dict, List, Set

def extract_features_from_miniob_txt() -> List[str]:
    """从MiniOB题目.txt中提取所有功能模块"""
    features = []
    
    with open('MiniOB题目.txt', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 按行分析，查找功能模块标题
    lines = content.split('\n')
    for line in lines:
        line = line.strip()
        # 跳过空行和描述性文本
        if not line or line.startswith('MiniOB') or line.startswith('在') or line.startswith('当前') or line.startswith('实现') or line.startswith('这') or line.startswith('注意') or line.startswith('示例') or line.startswith('功能') or line.startswith('需要') or line.startswith('支持') or line.startswith('要求') or line.startswith('测试') or line.startswith('备注') or line.startswith('有') or line.startswith('考虑'):
            continue
        
        # 识别功能模块名称（通常是单独一行的英文或中英文混合）
        if re.match(r'^[a-zA-Z][a-zA-Z0-9_-]*$', line) or re.match(r'^[a-zA-Z][a-zA-Z0-9_-]*\s*$', line):
            features.append(line.strip())
    
    return features

def scan_test_files() -> Set[str]:
    """扫描test/case/test目录下的所有测试文件"""
    test_files = set()
    test_dir = 'test/case/test'
    
    if os.path.exists(test_dir):
        for file in os.listdir(test_dir):
            if file.endswith('.test'):
                # 移除.test后缀
                test_name = file[:-5]
                test_files.add(test_name)
    
    return test_files

def analyze_coverage():
    """分析测试覆盖率"""
    print("🔍 MiniOB完整测试覆盖率分析")
    print("=" * 50)
    
    # 从题目文件中提取功能模块
    features = extract_features_from_miniob_txt()
    print(f"📋 从MiniOB题目.txt中识别的功能模块: {len(features)}个")
    for i, feature in enumerate(features, 1):
        print(f"  {i:2d}. {feature}")
    
    print("\n" + "=" * 50)
    
    # 扫描测试文件
    test_files = scan_test_files()
    print(f"📁 test/case/test目录下的测试文件: {len(test_files)}个")
    for test_file in sorted(test_files):
        print(f"  - {test_file}")
    
    print("\n" + "=" * 50)
    
    # 分析覆盖情况
    covered_features = []
    missing_features = []
    
    for feature in features:
        # 检查是否有对应的测试文件
        has_test = False
        
        # 检查直接匹配
        if f"primary-{feature}" in test_files or feature in test_files:
            has_test = True
        
        # 检查扩展测试
        if f"primary-{feature}-extended" in test_files:
            has_test = True
        
        # 检查向量相关测试
        if feature.startswith('vector') and any(f"vector" in test for test in test_files):
            has_test = True
        
        # 特殊情况处理
        special_mappings = {
            'join-tables': ['primary-join-tables', 'primary-join-extended'],
            'simple-sub-query': ['primary-simple-sub-query', 'primary-sub-query-extended'],
            'complex-sub-query': ['primary-complex-sub-query'],
            'aggregation_and_groupby': ['primary-aggregation-func', 'primary-group-by', 'primary-aggregation-groupby-extended'],
            'create-view': ['primary-create-view-extended'],
            'create-table-select': ['primary-create-table-select'],
            'order-by': ['primary-order-by', 'primary-order-by-extended'],
            'update-select': ['primary-update-select'],
            'update-mvcc': ['primary-update-mvcc'],
            'big-order-by': ['big-order-by'],
            'ann_benchmark': ['ann-benchmark']
        }
        
        if feature in special_mappings:
            for mapping in special_mappings[feature]:
                if mapping in test_files:
                    has_test = True
                    break
        
        if has_test:
            covered_features.append(feature)
        else:
            missing_features.append(feature)
    
    # 输出覆盖率统计
    coverage_rate = len(covered_features) / len(features) * 100
    print(f"📊 测试覆盖率统计:")
    print(f"  总功能模块: {len(features)}")
    print(f"  已覆盖: {len(covered_features)}")
    print(f"  未覆盖: {len(missing_features)}")
    print(f"  覆盖率: {coverage_rate:.1f}%")
    
    print(f"\n✅ 已覆盖的功能模块 ({len(covered_features)}个):")
    for feature in covered_features:
        print(f"  ✓ {feature}")
    
    print(f"\n❌ 未覆盖的功能模块 ({len(missing_features)}个):")
    for feature in missing_features:
        print(f"  ✗ {feature}")
    
    return missing_features

if __name__ == "__main__":
    missing = analyze_coverage()
    
    if missing:
        print(f"\n🚨 需要创建测试文件的功能模块:")
        for feature in missing:
            print(f"  - {feature}")
    else:
        print(f"\n🎉 所有功能模块都有对应的测试文件！")
