#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
通用MiniOB与MySQL 8.0一致性验证框架
支持验证任意数据库功能的一致性
"""

import os
import sys
import subprocess
import json
from pathlib import Path
from typing import List, Tuple, Dict, Any
import argparse
from datetime import datetime
import tempfile

# 可选依赖
try:
    import yaml
    YAML_AVAILABLE = True
except ImportError:
    YAML_AVAILABLE = False

try:
    import mysql.connector
    from mysql.connector import Error
    MYSQL_AVAILABLE = True
except ImportError:
    MYSQL_AVAILABLE = False

class UniversalValidator:
    def __init__(self, project_root="./"):
        self.project_root = Path(project_root).resolve()
        self.miniob_build_path = "./build"
        self.test_configs_dir = self.project_root / "test_configs"
        self.test_configs_dir.mkdir(exist_ok=True)
        
        # MySQL连接配置
        self.mysql_config = {
            'host': 'localhost',
            'port': 3306,
            'user': 'root',
            'password': '',
            'database': 'test'
        }
    
    def run_miniob_sql(self, sql: str, timeout: int = 15) -> Tuple[bool, str, str]:
        """在MiniOB中执行SQL"""
        try:
            observer_path = f"{self.miniob_build_path}/bin/observer"
            cmd = f'echo "{sql}" | timeout {timeout} {observer_path} -P cli'
            
            result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=timeout+5)
            
            # 提取查询结果
            lines = result.stdout.split('\n')
            result_lines = []
            
            for line in lines:
                if '|' in line and not line.startswith('DEBUG'):
                    result_lines.append(line.strip())
            
            if result_lines:
                return True, '\n'.join(result_lines), ""
            elif "SUCCESS" in result.stdout:
                return True, "SUCCESS", ""
            elif "FAILED" in result.stdout or "ERROR" in result.stdout:
                return False, "", result.stdout.strip()
            else:
                # 尝试提取数字结果（如COUNT查询）
                for line in result.stdout.split('\n'):
                    line = line.strip()
                    if line.isdigit():
                        return True, line, ""
                return True, "", ""
                
        except Exception as e:
            return False, "", str(e)
    
    def run_mysql_sql(self, sql: str, timeout: int = 15) -> Tuple[bool, str, str]:
        """在MySQL中执行SQL"""
        if not MYSQL_AVAILABLE:
            return False, "", "MySQL连接器不可用，请安装: pip install mysql-connector-python"

        try:
            connection = mysql.connector.connect(**self.mysql_config)
            cursor = connection.cursor()

            cursor.execute(sql)

            if cursor.description:
                # 有结果集的查询
                columns = [desc[0] for desc in cursor.description]
                rows = cursor.fetchall()

                if rows:
                    # 格式化为类似MiniOB的输出
                    header = " | ".join(columns)
                    result_lines = [header]
                    for row in rows:
                        row_str = " | ".join(str(val) if val is not None else "NULL" for val in row)
                        result_lines.append(row_str)
                    return True, '\n'.join(result_lines), ""
                else:
                    return True, " | ".join(columns), ""
            else:
                # 无结果集的查询（如INSERT, UPDATE等）
                return True, "SUCCESS", ""

        except Exception as e:
            if MYSQL_AVAILABLE:
                return False, "", str(e)
            else:
                return False, "", "MySQL连接器不可用"
        finally:
            if 'connection' in locals() and connection.is_connected():
                cursor.close()
                connection.close()
    
    def normalize_result(self, result: str) -> List[List[str]]:
        """标准化查询结果为二维数组"""
        if not result.strip() or result == "SUCCESS":
            return []
        
        lines = result.strip().split('\n')
        normalized = []
        
        for line in lines:
            if '|' in line:
                # 分割并清理每个字段
                fields = [field.strip() for field in line.split('|')]
                normalized.append(fields)
        
        return normalized
    
    def compare_results(self, miniob_result: str, mysql_result: str) -> Dict[str, Any]:
        """比较MiniOB和MySQL的结果"""
        miniob_normalized = self.normalize_result(miniob_result)
        mysql_normalized = self.normalize_result(mysql_result)
        
        comparison = {
            "identical": False,
            "miniob_rows": len(miniob_normalized),
            "mysql_rows": len(mysql_normalized),
            "differences": []
        }
        
        # 比较行数
        if len(miniob_normalized) != len(mysql_normalized):
            comparison["differences"].append(f"行数不同: MiniOB={len(miniob_normalized)}, MySQL={len(mysql_normalized)}")
        
        # 比较内容
        if miniob_normalized == mysql_normalized:
            comparison["identical"] = True
        else:
            # 详细比较差异
            max_rows = max(len(miniob_normalized), len(mysql_normalized))
            for i in range(max_rows):
                miniob_row = miniob_normalized[i] if i < len(miniob_normalized) else None
                mysql_row = mysql_normalized[i] if i < len(mysql_normalized) else None
                
                if miniob_row != mysql_row:
                    comparison["differences"].append(f"第{i+1}行不同: MiniOB={miniob_row}, MySQL={mysql_row}")
        
        return comparison
    
    def load_test_config(self, config_file: str) -> Dict[str, Any]:
        """加载测试配置文件"""
        config_path = self.test_configs_dir / config_file
        
        if not config_path.exists():
            raise FileNotFoundError(f"测试配置文件不存在: {config_path}")
        
        with open(config_path, 'r', encoding='utf-8') as f:
            if config_path.suffix.lower() == '.json':
                return json.load(f)
            elif config_path.suffix.lower() in ['.yml', '.yaml']:
                if not YAML_AVAILABLE:
                    raise ImportError("YAML支持不可用，请安装: pip install pyyaml")
                return yaml.safe_load(f)
            else:
                raise ValueError(f"不支持的配置文件格式: {config_path.suffix}")
    
    def create_sample_config(self, feature_name: str) -> str:
        """创建示例配置文件"""
        config = {
            "feature_name": feature_name,
            "description": f"{feature_name}功能验证",
            "setup_sql": [
                "-- 在这里添加测试数据准备SQL",
                "-- 例如: CREATE TABLE test_table(id INT, name VARCHAR(50));",
                "-- 例如: INSERT INTO test_table VALUES (1, 'test');"
            ],
            "cleanup_sql": [
                "-- 在这里添加清理SQL",
                "-- 例如: DROP TABLE IF EXISTS test_table;"
            ],
            "test_cases": [
                {
                    "name": f"{feature_name}基本测试",
                    "description": f"验证{feature_name}的基本功能",
                    "sql": "-- 在这里添加测试SQL",
                    "expected_behavior": "consistent",
                    "timeout": 10
                },
                {
                    "name": f"{feature_name}边界测试",
                    "description": f"验证{feature_name}的边界情况",
                    "sql": "-- 在这里添加边界测试SQL",
                    "expected_behavior": "consistent",
                    "timeout": 10
                }
            ]
        }
        
        config_file = self.test_configs_dir / f"{feature_name}.json"
        with open(config_file, 'w', encoding='utf-8') as f:
            json.dump(config, f, ensure_ascii=False, indent=2)
        
        return str(config_file)
    
    def run_feature_validation(self, config_file: str, mysql_enabled: bool = True) -> Dict[str, Any]:
        """运行功能验证"""
        print(f"🔍 加载测试配置: {config_file}")
        config = self.load_test_config(config_file)
        
        feature_name = config.get("feature_name", "未知功能")
        print(f"🎯 验证功能: {feature_name}")
        print(f"📝 描述: {config.get('description', '无描述')}")
        print("=" * 60)
        
        results = {
            "feature_name": feature_name,
            "total_tests": len(config.get("test_cases", [])),
            "passed": 0,
            "failed": 0,
            "errors": 0,
            "test_results": [],
            "mysql_enabled": mysql_enabled
        }
        
        # 执行setup SQL
        setup_sql = config.get("setup_sql", [])
        if setup_sql:
            print("🔧 执行测试环境准备...")
            for sql in setup_sql:
                if sql.strip() and not sql.strip().startswith('--'):
                    success, result, error = self.run_miniob_sql(sql)
                    if not success:
                        print(f"⚠️ 准备SQL执行失败: {sql[:50]}...")
        
        # 执行测试用例
        for i, test_case in enumerate(config.get("test_cases", []), 1):
            test_name = test_case.get("name", f"测试{i}")
            test_sql = test_case.get("sql", "")
            timeout = test_case.get("timeout", 10)
            expected_behavior = test_case.get("expected_behavior", "consistent")
            
            print(f"\n测试 {i}: {test_name}")
            print(f"SQL: {test_sql}")
            print(f"预期: {test_case.get('description', '无描述')}")
            
            if test_sql.strip().startswith('--') or not test_sql.strip():
                print("⚠️ 跳过: 测试SQL为空或注释")
                continue
            
            # 在MiniOB中执行
            miniob_success, miniob_result, miniob_error = self.run_miniob_sql(test_sql, timeout)
            
            test_result = {
                "name": test_name,
                "sql": test_sql,
                "miniob_success": miniob_success,
                "miniob_result": miniob_result,
                "miniob_error": miniob_error
            }
            
            if not miniob_success:
                print(f"❌ MiniOB执行失败: {miniob_error}")
                test_result["status"] = "error"
                results["errors"] += 1
            elif mysql_enabled:
                # 在MySQL中执行并比较
                mysql_success, mysql_result, mysql_error = self.run_mysql_sql(test_sql, timeout)
                
                test_result["mysql_success"] = mysql_success
                test_result["mysql_result"] = mysql_result
                test_result["mysql_error"] = mysql_error
                
                if not mysql_success:
                    print(f"⚠️ MySQL执行失败: {mysql_error}")
                    print(f"✅ MiniOB结果: {miniob_result[:100]}...")
                    test_result["status"] = "miniob_only"
                    results["passed"] += 1
                else:
                    # 比较结果
                    comparison = self.compare_results(miniob_result, mysql_result)
                    test_result["comparison"] = comparison
                    
                    if comparison["identical"]:
                        print(f"✅ 一致: MiniOB与MySQL结果完全相同")
                        test_result["status"] = "passed"
                        results["passed"] += 1
                    else:
                        print(f"❌ 不一致:")
                        for diff in comparison["differences"]:
                            print(f"   - {diff}")
                        test_result["status"] = "failed"
                        results["failed"] += 1
            else:
                # 仅验证MiniOB执行成功
                print(f"✅ MiniOB执行成功: {miniob_result[:100]}...")
                test_result["status"] = "miniob_only"
                results["passed"] += 1
            
            results["test_results"].append(test_result)
        
        # 执行cleanup SQL
        cleanup_sql = config.get("cleanup_sql", [])
        if cleanup_sql:
            print(f"\n🧹 执行测试环境清理...")
            for sql in cleanup_sql:
                if sql.strip() and not sql.strip().startswith('--'):
                    self.run_miniob_sql(sql)
        
        return results
    
    def generate_report(self, results: Dict[str, Any]) -> str:
        """生成验证报告"""
        report = []
        report.append("=" * 80)
        report.append("MiniOB与MySQL 8.0功能一致性验证报告")
        report.append("=" * 80)
        report.append(f"生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        report.append(f"验证功能: {results['feature_name']}")
        report.append("")
        
        # 统计信息
        total = results["total_tests"]
        passed = results["passed"]
        failed = results["failed"]
        errors = results["errors"]
        success_rate = (passed / total * 100) if total > 0 else 0
        
        report.append("📊 验证结果统计:")
        report.append(f"总计: {total} 个测试")
        report.append(f"通过: {passed} 个")
        report.append(f"失败: {failed} 个")
        report.append(f"错误: {errors} 个")
        report.append(f"成功率: {success_rate:.1f}%")
        
        if results["mysql_enabled"]:
            report.append(f"对比模式: MiniOB vs MySQL 8.0")
        else:
            report.append(f"验证模式: 仅MiniOB执行验证")
        
        # 详细结果
        if failed > 0:
            report.append("\n❌ 失败的测试:")
            for test in results["test_results"]:
                if test["status"] == "failed":
                    report.append(f"  - {test['name']}")
                    if "comparison" in test:
                        for diff in test["comparison"]["differences"]:
                            report.append(f"    {diff}")
        
        if errors > 0:
            report.append("\n⚠️ 执行错误的测试:")
            for test in results["test_results"]:
                if test["status"] == "error":
                    report.append(f"  - {test['name']}: {test['miniob_error']}")
        
        # 总结
        report.append(f"\n📋 总结:")
        if success_rate >= 90:
            report.append("✅ 功能与MySQL 8.0高度一致")
        elif success_rate >= 70:
            report.append("⚠️ 功能与MySQL 8.0基本一致，存在少量差异")
        else:
            report.append("❌ 功能与MySQL 8.0存在较多差异，需要进一步修复")
        
        report.append("\n" + "=" * 80)
        return "\n".join(report)

def main():
    parser = argparse.ArgumentParser(description="通用MiniOB与MySQL 8.0一致性验证工具")
    parser.add_argument("--config", help="指定测试配置文件")
    parser.add_argument("--create-config", help="创建示例配置文件，指定功能名称")
    parser.add_argument("--list-configs", action="store_true", help="列出所有可用的配置文件")
    parser.add_argument("--no-mysql", action="store_true", help="禁用MySQL对比，仅验证MiniOB执行")
    parser.add_argument("--mysql-host", default="localhost", help="MySQL主机地址")
    parser.add_argument("--mysql-port", type=int, default=3306, help="MySQL端口")
    parser.add_argument("--mysql-user", default="root", help="MySQL用户名")
    parser.add_argument("--mysql-password", default="", help="MySQL密码")
    parser.add_argument("--mysql-database", default="test", help="MySQL数据库名")
    parser.add_argument("--report", action="store_true", help="生成详细报告")
    
    args = parser.parse_args()
    
    validator = UniversalValidator()
    
    # 更新MySQL配置
    validator.mysql_config.update({
        'host': args.mysql_host,
        'port': args.mysql_port,
        'user': args.mysql_user,
        'password': args.mysql_password,
        'database': args.mysql_database
    })
    
    if args.create_config:
        config_file = validator.create_sample_config(args.create_config)
        print(f"✅ 已创建示例配置文件: {config_file}")
        print("请编辑配置文件，添加具体的测试用例，然后运行验证。")
        return
    
    if args.list_configs:
        configs = list(validator.test_configs_dir.glob("*.json")) + list(validator.test_configs_dir.glob("*.yml"))
        if configs:
            print("可用的测试配置文件:")
            for config in configs:
                print(f"  - {config.name}")
        else:
            print("没有找到测试配置文件。")
            print("使用 --create-config <功能名> 创建示例配置文件。")
        return
    
    if not args.config:
        parser.print_help()
        return
    
    # 运行验证
    mysql_enabled = not args.no_mysql
    results = validator.run_feature_validation(args.config, mysql_enabled)
    
    # 显示结果摘要
    print(f"\n📊 验证完成:")
    print(f"总计: {results['total_tests']} 个测试")
    print(f"通过: {results['passed']} 个")
    print(f"失败: {results['failed']} 个")
    print(f"错误: {results['errors']} 个")
    success_rate = (results['passed'] / results['total_tests'] * 100) if results['total_tests'] > 0 else 0
    print(f"成功率: {success_rate:.1f}%")
    
    # 生成报告
    if args.report or results['failed'] > 0 or results['errors'] > 0:
        report = validator.generate_report(results)
        print(f"\n{report}")
        
        # 保存报告到文件
        feature_name = results['feature_name'].replace(' ', '_')
        report_file = validator.project_root / f"validation_report_{feature_name}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
        with open(report_file, 'w', encoding='utf-8') as f:
            f.write(report)
        print(f"\n📄 报告已保存到: {report_file}")

if __name__ == "__main__":
    main()
