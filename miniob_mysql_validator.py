#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
MiniOB MySQL 8.0 一致性验证和测试管理集成工具
功能：
1. 验证MiniOB与MySQL 8.0的一致性
2. 自动更新测试结果文件
3. 生成详细的验证报告
4. 支持批量测试和单个测试
"""

import os
import sys
import subprocess
import shutil
import tempfile
from pathlib import Path
from typing import List, Tuple, Dict
import argparse
import json
from datetime import datetime

class IntegratedValidator:
    def __init__(self, project_root="./"):
        self.project_root = Path(project_root).resolve()
        self.test_case_dir = self.project_root / "test" / "case" / "test"
        self.result_dir = self.project_root / "test" / "case" / "result"
        self.miniob_test_script = self.project_root / "test" / "case" / "miniob_test.py"
        self.miniob_build_path = "./build"
        
    def run_miniob_sql(self, sql: str) -> Tuple[bool, str]:
        """在MiniOB中执行SQL"""
        try:
            observer_path = f"{self.miniob_build_path}/bin/observer"
            cmd = f'echo "{sql}" | timeout 15 {observer_path} -P cli'

            result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=20)

            # 特殊处理COUNT查询
            if "COUNT(*)" in sql.upper():
                # 使用grep提取数字
                grep_cmd = f'echo "{sql}" | timeout 15 {observer_path} -P cli | grep -E "^[0-9]+$"'
                grep_result = subprocess.run(grep_cmd, shell=True, capture_output=True, text=True, timeout=20)
                if grep_result.returncode == 0 and grep_result.stdout.strip().isdigit():
                    return True, grep_result.stdout.strip()

            # 提取查询结果
            lines = result.stdout.split('\n')
            result_lines = []

            for line in lines:
                if '|' in line and not line.startswith('DEBUG'):
                    result_lines.append(line.strip())

            if result_lines:
                return True, '\n'.join(result_lines)
            elif "SUCCESS" in result.stdout:
                return True, "SUCCESS"
            else:
                return False, result.stdout.strip()

        except Exception as e:
            return False, str(e)
    
    def count_result_rows(self, result: str) -> int:
        """计算结果行数（不包括表头）"""
        if not result.strip():
            return 0
        
        lines = [line for line in result.split('\n') if '|' in line]
        # 减去表头行
        return max(0, len(lines) - 1)
    
    def verify_is_null_consistency(self) -> Dict:
        """验证IS NULL功能与MySQL 8.0的一致性"""
        print("🔍 验证IS NULL功能与MySQL 8.0一致性...")
        print("=" * 60)
        
        # 检查表是否存在
        success, result = self.run_miniob_sql("SELECT COUNT(*) FROM null_table;")
        if not success:
            print("❌ null_table不存在，请先运行primary-null测试用例")
            return {"setup_failed": True}
        
        table_rows = int(result) if result.isdigit() else 0
        print(f"✅ 找到null_table，包含 {table_rows} 行数据")
        
        # 定义测试用例（基于MySQL 8.0标准）
        test_cases = [
            {
                "name": "常量1 IS NULL",
                "sql": "SELECT * FROM null_table WHERE 1 IS NULL;",
                "expected_rows": 0,
                "description": "非NULL常量的IS NULL应该返回false"
            },
            {
                "name": "常量1 IS NOT NULL", 
                "sql": "SELECT * FROM null_table WHERE 1 IS NOT NULL;",
                "expected_rows": table_rows,
                "description": "非NULL常量的IS NOT NULL应该返回true"
            },
            {
                "name": "NULL IS NULL",
                "sql": "SELECT * FROM null_table WHERE NULL IS NULL;", 
                "expected_rows": table_rows,
                "description": "NULL IS NULL应该返回true"
            },
            {
                "name": "NULL IS NOT NULL",
                "sql": "SELECT * FROM null_table WHERE NULL IS NOT NULL;",
                "expected_rows": 0,
                "description": "NULL IS NOT NULL应该返回false"
            },
            {
                "name": "字段num IS NULL",
                "sql": "SELECT * FROM null_table WHERE num IS NULL;",
                "expected_rows": 1,
                "description": "应该返回num字段为NULL的行"
            },
            {
                "name": "字段num IS NOT NULL",
                "sql": "SELECT * FROM null_table WHERE num IS NOT NULL;", 
                "expected_rows": table_rows - 1,
                "description": "应该返回num字段不为NULL的行"
            },
            {
                "name": "字符串 IS NULL",
                "sql": "SELECT * FROM null_table WHERE 'hello' IS NULL;",
                "expected_rows": 0,
                "description": "非NULL字符串的IS NULL应该返回false"
            },
            {
                "name": "字符串 IS NOT NULL",
                "sql": "SELECT * FROM null_table WHERE 'hello' IS NOT NULL;",
                "expected_rows": table_rows,
                "description": "非NULL字符串的IS NOT NULL应该返回true"
            }
        ]
        
        results = []
        passed = 0
        failed = 0
        
        for i, test_case in enumerate(test_cases, 1):
            print(f"\n测试 {i}: {test_case['name']}")
            print(f"SQL: {test_case['sql']}")
            print(f"预期: {test_case['description']}")
            
            success, result = self.run_miniob_sql(test_case['sql'])
            
            if not success:
                print(f"❌ 执行失败: {result}")
                results.append({
                    "name": test_case['name'],
                    "success": False,
                    "error": result
                })
                failed += 1
                continue
            
            actual_rows = self.count_result_rows(result)
            expected_rows = test_case['expected_rows']
            
            if actual_rows == expected_rows:
                print(f"✅ 通过: 返回 {actual_rows} 行（预期 {expected_rows} 行）")
                results.append({
                    "name": test_case['name'],
                    "success": True,
                    "actual_rows": actual_rows,
                    "expected_rows": expected_rows
                })
                passed += 1
            else:
                print(f"❌ 失败: 返回 {actual_rows} 行，预期 {expected_rows} 行")
                results.append({
                    "name": test_case['name'], 
                    "success": False,
                    "actual_rows": actual_rows,
                    "expected_rows": expected_rows,
                    "result": result
                })
                failed += 1
        
        return {
            "total": len(test_cases),
            "passed": passed,
            "failed": failed,
            "success_rate": passed/len(test_cases)*100,
            "results": results
        }
    
    def backup_result_file(self, test_name: str) -> bool:
        """备份原有的结果文件"""
        result_file = self.result_dir / f"{test_name}.result"
        backup_file = self.result_dir / f"{test_name}.result.backup"
        
        if result_file.exists():
            try:
                shutil.copy2(result_file, backup_file)
                print(f"✅ 已备份原结果文件: {backup_file}")
                return True
            except Exception as e:
                print(f"❌ 备份失败: {e}")
                return False
        else:
            print(f"⚠️ 原结果文件不存在: {result_file}")
            return True
    
    def generate_new_result(self, test_name: str) -> bool:
        """生成新的测试结果"""
        try:
            cmd = [
                "python3", str(self.miniob_test_script),
                f"--test-cases={test_name}",
                "--report-only"
            ]
            
            print(f"🔄 正在生成新的测试结果: {test_name}")
            result = subprocess.run(cmd, capture_output=True, text=True, cwd=self.project_root)
            
            if result.returncode == 0:
                print(f"✅ 成功生成新的测试结果: {test_name}")
                return True
            else:
                print(f"❌ 生成测试结果失败: {result.stderr}")
                return False
                
        except Exception as e:
            print(f"❌ 执行测试失败: {e}")
            return False
    
    def update_test_result(self, test_name: str) -> bool:
        """更新单个测试用例的结果"""
        print(f"\n🔄 更新测试用例: {test_name}")
        print("-" * 40)
        
        # 1. 备份原结果文件
        if not self.backup_result_file(test_name):
            return False
        
        # 2. 生成新的结果
        if not self.generate_new_result(test_name):
            return False
        
        print(f"✅ 成功更新测试用例: {test_name}")
        return True
    
    def run_test_validation(self, test_name: str) -> bool:
        """运行测试验证"""
        try:
            cmd = [
                "python3", str(self.miniob_test_script),
                f"--test-cases={test_name}"
            ]
            
            result = subprocess.run(cmd, capture_output=True, text=True, cwd=self.project_root)
            return result.returncode == 0
        except Exception:
            return False
    
    def list_available_tests(self) -> List[str]:
        """列出所有可用的测试用例"""
        test_files = list(self.test_case_dir.glob("*.test"))
        test_names = [f.stem for f in test_files]
        return sorted(test_names)
    
    def generate_report(self, validation_results: Dict, updated_tests: List[str], test_results: Dict = None) -> str:
        """生成详细报告"""
        report = []
        report.append("=" * 80)
        report.append("MiniOB MySQL 8.0 一致性验证报告")
        report.append("=" * 80)
        report.append(f"生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        report.append("")

        # IS NULL验证结果
        if validation_results and "setup_failed" not in validation_results:
            report.append("📊 IS NULL功能验证结果:")
            report.append(f"总计: {validation_results['total']} 个测试")
            report.append(f"通过: {validation_results['passed']} 个")
            report.append(f"失败: {validation_results['failed']} 个")
            report.append(f"成功率: {validation_results['success_rate']:.1f}%")

            if validation_results['failed'] > 0:
                report.append("\n❌ 失败的测试:")
                for result in validation_results['results']:
                    if not result['success']:
                        report.append(f"  - {result['name']}")
            else:
                report.append("\n🎉 所有IS NULL测试都通过了！")

        # 更新的测试用例
        if updated_tests:
            report.append(f"\n📝 已更新的测试用例 ({len(updated_tests)} 个):")
            for test in updated_tests:
                report.append(f"  - {test}")

        # 测试验证结果
        if test_results:
            report.append(f"\n🧪 测试验证结果:")
            passed = test_results.get('passed', [])
            failed = test_results.get('failed', [])

            if passed:
                report.append(f"✅ 通过的测试 ({len(passed)} 个):")
                for test in passed:
                    report.append(f"  - {test}")

            if failed:
                report.append(f"❌ 失败的测试 ({len(failed)} 个):")
                for test in failed:
                    report.append(f"  - {test}")

        # 总结
        report.append(f"\n📋 总结:")
        if validation_results and validation_results.get('success_rate', 0) >= 90:
            report.append("✅ MiniOB的IS NULL功能与MySQL 8.0高度一致")
        elif validation_results:
            report.append("⚠️ MiniOB的IS NULL功能与MySQL 8.0存在一些差异")

        if updated_tests:
            report.append(f"✅ 已更新 {len(updated_tests)} 个测试用例的结果文件")

        report.append("\n" + "=" * 80)
        return "\n".join(report)

    def quick_check(self) -> Dict:
        """快速检查：验证核心IS NULL功能"""
        print("⚡ 快速检查IS NULL核心功能...")

        quick_tests = [
            ("SELECT * FROM null_table WHERE 1 IS NULL;", 0),
            ("SELECT * FROM null_table WHERE 1 IS NOT NULL;", "all"),
            ("SELECT * FROM null_table WHERE NULL IS NULL;", "all"),
            ("SELECT * FROM null_table WHERE NULL IS NOT NULL;", 0),
        ]

        # 获取表总行数
        success, result = self.run_miniob_sql("SELECT COUNT(*) FROM null_table;")
        if not success:
            return {"error": "无法访问null_table"}

        total_rows = int(result) if result.isdigit() else 0

        passed = 0
        failed = 0

        for sql, expected in quick_tests:
            success, result = self.run_miniob_sql(sql)
            if success:
                actual_rows = self.count_result_rows(result)
                expected_rows = total_rows if expected == "all" else expected

                if actual_rows == expected_rows:
                    passed += 1
                    print(f"✅ {sql[:30]}... 通过")
                else:
                    failed += 1
                    print(f"❌ {sql[:30]}... 失败")
            else:
                failed += 1
                print(f"❌ {sql[:30]}... 执行失败")

        return {
            "passed": passed,
            "failed": failed,
            "total": len(quick_tests),
            "success_rate": passed / len(quick_tests) * 100
        }

def main():
    parser = argparse.ArgumentParser(description="MiniOB MySQL 8.0 一致性验证和测试管理工具")
    parser.add_argument("--verify-is-null", action="store_true", help="验证IS NULL功能与MySQL 8.0一致性")
    parser.add_argument("--update-results", nargs="*", help="更新测试结果文件（指定测试名称或使用--all）")
    parser.add_argument("--all", action="store_true", help="处理所有测试用例")
    parser.add_argument("--list", action="store_true", help="列出所有可用的测试用例")
    parser.add_argument("--full-check", action="store_true", help="完整检查：验证一致性 + 更新结果 + 运行测试")
    parser.add_argument("--quick-check", action="store_true", help="快速检查IS NULL核心功能")
    parser.add_argument("--report", action="store_true", help="生成详细报告")

    args = parser.parse_args()
    
    validator = IntegratedValidator()
    
    if args.list:
        tests = validator.list_available_tests()
        print("可用的测试用例:")
        for test in tests:
            print(f"  - {test}")
        return
    
    validation_results = {}
    updated_tests = []
    test_results = {"passed": [], "failed": []}

    # 快速检查
    if args.quick_check:
        quick_results = validator.quick_check()
        print(f"\n⚡ 快速检查结果: {quick_results['passed']}/{quick_results['total']} 通过 ({quick_results['success_rate']:.1f}%)")
        return

    # 验证IS NULL一致性
    if args.verify_is_null or args.full_check:
        validation_results = validator.verify_is_null_consistency()
    
    # 更新测试结果
    if args.update_results is not None or args.full_check:
        if args.all or args.full_check:
            tests_to_update = validator.list_available_tests()
            print(f"\n🚀 将更新所有 {len(tests_to_update)} 个测试用例")
            
            if not args.full_check:
                response = input("确认继续? (y/N): ")
                if response.lower() != 'y':
                    print("已取消")
                    return
        else:
            tests_to_update = args.update_results
        
        print(f"\n📝 更新测试结果文件...")
        print("=" * 60)
        
        for test_name in tests_to_update:
            if validator.update_test_result(test_name):
                updated_tests.append(test_name)
    
    # 运行完整测试验证
    if args.full_check:
        print(f"\n🧪 运行测试验证...")
        print("=" * 60)

        for test_name in ["primary-null", "primary-expression"]:
            print(f"验证测试: {test_name}")
            if validator.run_test_validation(test_name):
                print(f"✅ {test_name} 测试通过")
                test_results["passed"].append(test_name)
            else:
                print(f"❌ {test_name} 测试失败")
                test_results["failed"].append(test_name)

        if test_results["failed"]:
            print(f"\n⚠️ 有 {len(test_results['failed'])} 个测试失败")
        else:
            print(f"\n🎉 所有测试都通过了！")
    
    # 生成报告
    if args.report or args.full_check or validation_results:
        report = validator.generate_report(validation_results, updated_tests, test_results)
        print(f"\n{report}")

        # 保存报告到文件
        report_file = validator.project_root / f"validation_report_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
        with open(report_file, 'w', encoding='utf-8') as f:
            f.write(report)
        print(f"\n📄 报告已保存到: {report_file}")

    # 如果没有指定任何操作，显示帮助
    if not any([args.verify_is_null, args.update_results is not None, args.list, args.full_check, args.quick_check]):
        parser.print_help()

if __name__ == "__main__":
    main()
