#!/usr/bin/env python3
"""
MySQL 8.0兼容性检查和结果文件更正脚本
确保MiniOB的行为与MySQL 8.0完全一致
"""

import os
import subprocess
import sys
from pathlib import Path

class MySQLCompatibilityChecker:
    def __init__(self):
        self.test_dir = "test/case/test"
        self.result_dir = "test/case/result"
        self.mysql_issues = []
        
    def check_mysql_availability(self):
        """检查MySQL是否可用"""
        try:
            result = subprocess.run(['mysql', '--version'], 
                                  capture_output=True, text=True)
            if result.returncode == 0:
                print(f"✅ MySQL available: {result.stdout.strip()}")
                return True
            else:
                print("❌ MySQL not available")
                return False
        except FileNotFoundError:
            print("❌ MySQL command not found")
            return False
    
    def get_mysql_8_expected_results(self):
        """定义MySQL 8.0的期望行为"""
        return {
            # COUNT函数NULL值处理
            'count_null_behavior': {
                'count_star': 'COUNT(*) 包含所有行，包括NULL',
                'count_column': 'COUNT(column) 跳过NULL值',
                'example': 'COUNT(name) with 3 non-NULL, 2 NULL = 3'
            },
            
            # 字段验证
            'field_validation': {
                'nonexistent_field_select': 'ERROR 1054 (42S22): Unknown column',
                'nonexistent_field_where': 'ERROR 1054 (42S22): Unknown column'
            },
            
            # 数据类型
            'data_types': {
                'int': '整数类型',
                'float': '浮点数类型，精度可能有差异',
                'varchar': '变长字符串',
                'date': 'YYYY-MM-DD格式',
                'boolean': 'TRUE/FALSE或1/0'
            },
            
            # 聚合函数
            'aggregation': {
                'sum_null': 'SUM跳过NULL值',
                'avg_null': 'AVG跳过NULL值',
                'min_max_null': 'MIN/MAX跳过NULL值',
                'null_result': '如果所有值都是NULL，结果为NULL'
            }
        }
    
    def analyze_test_case(self, test_file):
        """分析测试用例，识别可能的MySQL兼容性问题"""
        issues = []
        
        with open(test_file, 'r') as f:
            content = f.read().upper()
            
        # 检查COUNT函数使用
        if 'COUNT(' in content:
            issues.append({
                'type': 'COUNT_FUNCTION',
                'description': '需要验证COUNT函数NULL值处理',
                'mysql_behavior': 'COUNT(column)跳过NULL值'
            })
        
        # 检查聚合函数
        agg_functions = ['SUM(', 'AVG(', 'MIN(', 'MAX(']
        for func in agg_functions:
            if func in content:
                issues.append({
                    'type': 'AGGREGATION',
                    'description': f'需要验证{func}函数NULL值处理',
                    'mysql_behavior': f'{func}跳过NULL值'
                })
        
        # 检查数据类型
        if 'CHAR(' in content or 'VARCHAR(' in content:
            issues.append({
                'type': 'STRING_TYPE',
                'description': '需要验证字符串类型行为',
                'mysql_behavior': 'VARCHAR在MySQL中是变长的'
            })
        
        # 检查日期类型
        if 'DATE' in content:
            issues.append({
                'type': 'DATE_TYPE',
                'description': '需要验证日期格式',
                'mysql_behavior': 'MySQL使用YYYY-MM-DD格式'
            })
        
        return issues
    
    def check_all_test_cases(self):
        """检查所有测试用例"""
        test_files = list(Path(self.test_dir).glob("*.test"))
        
        print(f"🔍 检查 {len(test_files)} 个测试文件...")
        
        for test_file in test_files:
            print(f"\n📁 分析: {test_file.name}")
            issues = self.analyze_test_case(test_file)
            
            if issues:
                self.mysql_issues.append({
                    'file': test_file.name,
                    'issues': issues
                })
                
                for issue in issues:
                    print(f"  ⚠️  {issue['type']}: {issue['description']}")
            else:
                print(f"  ✅ 无明显兼容性问题")
    
    def generate_mysql_test_script(self):
        """生成MySQL测试脚本"""
        script_content = """-- MySQL 8.0 兼容性验证脚本
-- 自动生成，用于验证关键功能

-- 设置SQL模式为严格模式
SET sql_mode = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION';

-- 1. COUNT函数NULL值处理验证
DROP TABLE IF EXISTS count_null_test;
CREATE TABLE count_null_test(
    id INT NOT NULL,
    name VARCHAR(20),
    score FLOAT
);

INSERT INTO count_null_test VALUES(1, 'Alice', 95.5);
INSERT INTO count_null_test VALUES(2, NULL, 87.2);
INSERT INTO count_null_test VALUES(3, 'Charlie', NULL);
INSERT INTO count_null_test VALUES(4, NULL, NULL);

SELECT 'COUNT_STAR' as test_type, COUNT(*) as result FROM count_null_test;
SELECT 'COUNT_NAME' as test_type, COUNT(name) as result FROM count_null_test;
SELECT 'COUNT_SCORE' as test_type, COUNT(score) as result FROM count_null_test;

-- 2. 聚合函数NULL值处理验证
SELECT 'SUM_SCORE' as test_type, SUM(score) as result FROM count_null_test;
SELECT 'AVG_SCORE' as test_type, AVG(score) as result FROM count_null_test;
SELECT 'MIN_SCORE' as test_type, MIN(score) as result FROM count_null_test;
SELECT 'MAX_SCORE' as test_type, MAX(score) as result FROM count_null_test;

-- 3. 字符串类型验证
DROP TABLE IF EXISTS string_test;
CREATE TABLE string_test(
    id INT,
    short_text CHAR(10),
    long_text VARCHAR(50)
);

INSERT INTO string_test VALUES(1, 'ABC', 'Hello World');
INSERT INTO string_test VALUES(2, 'XYZ', NULL);

SELECT * FROM string_test;

-- 4. 日期类型验证
DROP TABLE IF EXISTS date_test;
CREATE TABLE date_test(
    id INT,
    birth_date DATE
);

INSERT INTO date_test VALUES(1, '2020-01-01');
INSERT INTO date_test VALUES(2, '1999-12-31');
INSERT INTO date_test VALUES(3, NULL);

SELECT * FROM date_test;
SELECT COUNT(*), COUNT(birth_date) FROM date_test;

-- 清理
DROP TABLE IF EXISTS count_null_test;
DROP TABLE IF EXISTS string_test;
DROP TABLE IF EXISTS date_test;
"""
        
        with open('mysql_compatibility_test.sql', 'w') as f:
            f.write(script_content)
        
        print("📝 生成MySQL兼容性测试脚本: mysql_compatibility_test.sql")
    
    def run_report(self):
        """生成兼容性检查报告"""
        print("\n" + "="*60)
        print("📊 MySQL 8.0 兼容性检查报告")
        print("="*60)
        
        if not self.mysql_issues:
            print("✅ 所有测试文件都没有发现明显的兼容性问题")
            return
        
        print(f"⚠️  发现 {len(self.mysql_issues)} 个文件存在潜在兼容性问题:")
        
        for item in self.mysql_issues:
            print(f"\n📁 {item['file']}:")
            for issue in item['issues']:
                print(f"  • {issue['type']}: {issue['description']}")
                print(f"    MySQL行为: {issue['mysql_behavior']}")
        
        print(f"\n💡 建议:")
        print(f"1. 使用 --report-only 重新生成结果文件前，先验证功能正确性")
        print(f"2. 对比MySQL 8.0的实际输出")
        print(f"3. 特别关注NULL值处理和数据类型转换")
        print(f"4. 验证错误消息格式是否与MySQL一致")

def main():
    checker = MySQLCompatibilityChecker()
    
    print("🚀 开始MySQL 8.0兼容性检查...")
    
    # 检查MySQL可用性
    mysql_available = checker.check_mysql_availability()
    
    # 分析所有测试用例
    checker.check_all_test_cases()
    
    # 生成MySQL测试脚本
    checker.generate_mysql_test_script()
    
    # 生成报告
    checker.run_report()
    
    if mysql_available:
        print(f"\n🔧 下一步操作:")
        print(f"1. 运行: mysql < mysql_compatibility_test.sql")
        print(f"2. 对比输出结果")
        print(f"3. 修正MiniOB的实现")
        print(f"4. 使用 --report-only 重新生成结果文件")

if __name__ == "__main__":
    main()
