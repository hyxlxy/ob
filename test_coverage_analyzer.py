#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

"""
MiniOB测试覆盖率分析工具
分析测试用例覆盖情况，识别测试盲点
"""

import os
import re
import json
from typing import Dict, List, Set, Tuple
from collections import defaultdict

class TestCoverageAnalyzer:
    """测试覆盖率分析器"""
    
    def __init__(self, test_dir: str = "test/case/test", requirements_file: str = "MiniOB题目.txt"):
        self.test_dir = test_dir
        self.requirements_file = requirements_file
        
        # 定义功能模块和关键词映射
        self.feature_keywords = {
            "date": ["date", "日期", "闰年", "2038", "1970"],
            "drop-table": ["drop table", "删除表", "清除表"],
            "update": ["update", "更新", "set"],
            "like": ["like", "通配符", "%", "_"],
            "expression": ["expression", "表达式", "算数", "calc"],
            "vector_basic": ["vector", "向量", "embedding", "l2_distance", "cosine_distance"],
            "vector_format": ["vector", "[1,2,3]", "列表"],
            "join-tables": ["join", "inner join", "多表"],
            "simple-sub-query": ["子查询", "in", "exists", "not in"],
            "multi-index": ["多字段索引", "create index", "col1,col2"],
            "unique": ["unique index", "唯一索引", "重复"],
            "null": ["null", "空值", "is null", "not null"],
            "update-select": ["update", "select", "子查询更新"],
            "alias": ["alias", "别名", "as"],
            "text": ["text", "文本", "65535", "超长"],
            "order-by": ["order by", "排序", "asc", "desc"],
            "aggregation_and_groupby": ["group by", "聚合", "max", "min", "count", "avg", "sum", "having"],
            "create-view": ["view", "视图", "create view", "drop view"],
            "create-table-select": ["create table", "as select", "复制表"],
            "vector_high_dim": ["16000", "高维", "向量"],
            "vector_rewrite": ["ivf-flat", "向量索引", "order by", "limit"],
            "vector_search": ["向量检索", "精确检索", "ann"],
            "complex-sub-query": ["复杂子查询", "关联", "聚合函数"],
            "update-mvcc": ["mvcc", "多版本", "并发", "事务"],
            "big-order-by": ["大数据", "排序", "内存", "优化"],
            "ann_benchmark": ["ann-benchmarks", "性能", "qps", "recall"]
        }
    
    def analyze_requirements(self) -> Dict[str, List[str]]:
        """分析需求文件，提取功能要求"""
        requirements = {}
        
        if not os.path.exists(self.requirements_file):
            print(f"需求文件不存在: {self.requirements_file}")
            return requirements
        
        with open(self.requirements_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 按功能模块分割
        sections = re.split(r'\n([a-zA-Z_-]+)\n', content)
        
        current_feature = None
        for i, section in enumerate(sections):
            if i % 2 == 1:  # 功能名称
                current_feature = section.strip()
                requirements[current_feature] = []
            elif i % 2 == 0 and current_feature:  # 功能描述
                requirements[current_feature] = section.strip().split('\n')
        
        return requirements
    
    def analyze_test_cases(self) -> Dict[str, Dict[str, any]]:
        """分析现有测试用例"""
        test_cases = {}
        
        if not os.path.exists(self.test_dir):
            print(f"测试目录不存在: {self.test_dir}")
            return test_cases
        
        for filename in os.listdir(self.test_dir):
            if filename.endswith('.test'):
                test_name = filename[:-5]  # 移除.test后缀
                test_path = os.path.join(self.test_dir, filename)
                
                with open(test_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                
                test_cases[test_name] = {
                    'content': content,
                    'sql_commands': self._extract_sql_commands(content),
                    'features_covered': list(self._identify_features(content)),
                    'line_count': len(content.split('\n')),
                    'command_count': len(self._extract_sql_commands(content))
                }
        
        return test_cases
    
    def _extract_sql_commands(self, content: str) -> List[str]:
        """提取SQL命令"""
        commands = []
        lines = content.split('\n')
        
        for line in lines:
            line = line.strip()
            if line and not line.startswith('--'):
                commands.append(line)
        
        return commands
    
    def _identify_features(self, content: str) -> Set[str]:
        """识别测试用例覆盖的功能"""
        features = set()
        content_lower = content.lower()
        
        for feature, keywords in self.feature_keywords.items():
            for keyword in keywords:
                if keyword.lower() in content_lower:
                    features.add(feature)
                    break
        
        return features
    
    def generate_coverage_report(self) -> Dict[str, any]:
        """生成覆盖率报告"""
        requirements = self.analyze_requirements()
        test_cases = self.analyze_test_cases()
        
        # 计算功能覆盖情况
        feature_coverage = {}
        covered_features = set()
        
        for test_name, test_info in test_cases.items():
            for feature in test_info['features_covered']:
                if feature not in feature_coverage:
                    feature_coverage[feature] = []
                feature_coverage[feature].append(test_name)
                covered_features.add(feature)
        
        # 识别未覆盖的功能
        all_features = set(self.feature_keywords.keys())
        uncovered_features = all_features - covered_features
        
        # 计算覆盖率统计
        total_features = len(all_features)
        covered_count = len(covered_features)
        coverage_percentage = (covered_count / total_features) * 100 if total_features > 0 else 0
        
        report = {
            'summary': {
                'total_features': total_features,
                'covered_features': covered_count,
                'uncovered_features': len(uncovered_features),
                'coverage_percentage': round(coverage_percentage, 2),
                'total_test_cases': len(test_cases)
            },
            'feature_coverage': feature_coverage,
            'uncovered_features': list(uncovered_features),
            'test_cases': test_cases,
            'requirements': requirements
        }
        
        return report
    
    def print_coverage_report(self):
        """打印覆盖率报告"""
        report = self.generate_coverage_report()
        
        print("=" * 80)
        print("🔍 MiniOB测试覆盖率分析报告")
        print("=" * 80)
        
        # 总体统计
        summary = report['summary']
        print(f"\n📊 总体统计:")
        print(f"   总功能模块数: {summary['total_features']}")
        print(f"   已覆盖功能数: {summary['covered_features']}")
        print(f"   未覆盖功能数: {summary['uncovered_features']}")
        print(f"   覆盖率: {summary['coverage_percentage']}%")
        print(f"   测试用例总数: {summary['total_test_cases']}")
        
        # 已覆盖功能
        print(f"\n✅ 已覆盖功能 ({len(report['feature_coverage'])}个):")
        for feature, test_cases in sorted(report['feature_coverage'].items()):
            print(f"   {feature}: {', '.join(test_cases)}")
        
        # 未覆盖功能
        if report['uncovered_features']:
            print(f"\n❌ 未覆盖功能 ({len(report['uncovered_features'])}个):")
            for feature in sorted(report['uncovered_features']):
                print(f"   {feature}")
        
        # 测试用例详情
        print(f"\n📋 测试用例详情:")
        for test_name, test_info in sorted(report['test_cases'].items()):
            features = ', '.join(sorted(test_info['features_covered'])) if test_info['features_covered'] else '无'
            print(f"   {test_name}: {test_info['command_count']}条命令, 覆盖功能: {features}")
        
        print("\n" + "=" * 80)

def main():
    """主函数"""
    analyzer = TestCoverageAnalyzer()
    analyzer.print_coverage_report()
    
    # 保存详细报告到JSON文件
    report = analyzer.generate_coverage_report()
    with open('test_coverage_report.json', 'w', encoding='utf-8') as f:
        json.dump(report, f, ensure_ascii=False, indent=2)
    
    print("📄 详细报告已保存到: test_coverage_report.json")

if __name__ == "__main__":
    main()
