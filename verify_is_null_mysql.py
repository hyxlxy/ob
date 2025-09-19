#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
验证IS NULL功能与MySQL 8.0一致性的简化脚本
不需要实际连接MySQL，而是基于MySQL标准行为进行验证
"""

import subprocess
import re
from typing import List, Tuple, Dict

class ISNullVerifier:
    def __init__(self):
        self.miniob_build_path = "./build"
        
    def run_miniob_sql(self, sql: str) -> Tuple[bool, str]:
        """在MiniOB中执行SQL"""
        try:
            observer_path = f"{self.miniob_build_path}/bin/observer"
            cmd = f'echo "{sql}" | timeout 15 {observer_path} -P cli'

            result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=20)

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
            elif "COUNT(*)" in result.stdout:
                # 处理COUNT查询
                for line in lines:
                    if line.strip().isdigit():
                        return True, line.strip()
                return True, "0"
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
    
    def verify_is_null_tests(self) -> Dict:
        """验证IS NULL相关测试"""
        
        # 使用已有的null_table进行测试
        print("🔧 使用已有的null_table进行测试...")

        # 检查表是否存在
        success, result = self.run_miniob_sql("SELECT COUNT(*) FROM null_table;")
        if not success:
            print("❌ null_table不存在，请先运行primary-null测试用例")
            return {"setup_failed": True}

        print("✅ 找到null_table，开始测试...")
        
        # 定义测试用例和预期行为（基于MySQL 8.0标准）
        # 使用null_table，它有2行数据（从COUNT结果看）
        test_cases = [
            {
                "name": "常量1 IS NULL",
                "sql": "SELECT * FROM null_table WHERE 1 IS NULL;",
                "expected_rows": 0,  # 1不是NULL，所以应该返回0行
                "description": "非NULL常量的IS NULL应该返回false"
            },
            {
                "name": "常量1 IS NOT NULL",
                "sql": "SELECT * FROM null_table WHERE 1 IS NOT NULL;",
                "expected_rows": 2,  # 1不是NULL，所以应该返回所有行
                "description": "非NULL常量的IS NOT NULL应该返回true"
            },
            {
                "name": "NULL IS NULL",
                "sql": "SELECT * FROM null_table WHERE NULL IS NULL;",
                "expected_rows": 2,  # NULL IS NULL为true，返回所有行
                "description": "NULL IS NULL应该返回true"
            },
            {
                "name": "NULL IS NOT NULL",
                "sql": "SELECT * FROM null_table WHERE NULL IS NOT NULL;",
                "expected_rows": 0,  # NULL IS NOT NULL为false，返回0行
                "description": "NULL IS NOT NULL应该返回false"
            },
            {
                "name": "字段num IS NULL",
                "sql": "SELECT * FROM null_table WHERE num IS NULL;",
                "expected_rows": 1,  # 应该有1行num为NULL
                "description": "应该返回num字段为NULL的行"
            },
            {
                "name": "字段num IS NOT NULL",
                "sql": "SELECT * FROM null_table WHERE num IS NOT NULL;",
                "expected_rows": 1,  # 应该有1行num不为NULL
                "description": "应该返回num字段不为NULL的行"
            },
            {
                "name": "字段birthday IS NULL",
                "sql": "SELECT * FROM null_table WHERE birthday IS NULL;",
                "expected_rows": 1,  # 应该有1行birthday为NULL
                "description": "应该返回birthday字段为NULL的行"
            },
            {
                "name": "字符串 IS NULL",
                "sql": "SELECT * FROM null_table WHERE 'hello' IS NULL;",
                "expected_rows": 0,  # 字符串不是NULL
                "description": "非NULL字符串的IS NULL应该返回false"
            },
            {
                "name": "字符串 IS NOT NULL",
                "sql": "SELECT * FROM null_table WHERE 'hello' IS NOT NULL;",
                "expected_rows": 2,  # 字符串不是NULL，返回所有行
                "description": "非NULL字符串的IS NOT NULL应该返回true"
            }
        ]
        
        results = []
        passed = 0
        failed = 0
        
        print("\n🧪 开始验证IS NULL功能...")
        print("=" * 80)
        
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
                print(f"实际结果:\n{result}")
                results.append({
                    "name": test_case['name'], 
                    "success": False,
                    "actual_rows": actual_rows,
                    "expected_rows": expected_rows,
                    "result": result
                })
                failed += 1
        
        # 输出总结
        print(f"\n{'='*80}")
        print("🏁 验证总结:")
        print(f"总计: {len(test_cases)} 个测试")
        print(f"通过: {passed} 个")
        print(f"失败: {failed} 个")
        print(f"成功率: {passed/len(test_cases)*100:.1f}%")
        
        if failed > 0:
            print(f"\n❌ 失败的测试:")
            for result in results:
                if not result["success"]:
                    print(f"  - {result['name']}")
        else:
            print(f"\n🎉 所有IS NULL测试都通过了！与MySQL 8.0行为一致！")
        
        return {
            "total": len(test_cases),
            "passed": passed,
            "failed": failed,
            "success_rate": passed/len(test_cases)*100,
            "results": results
        }

def main():
    """主函数"""
    print("🔍 MiniOB IS NULL功能验证工具")
    print("基于MySQL 8.0标准验证IS NULL操作的正确性")
    print("=" * 80)
    
    verifier = ISNullVerifier()
    results = verifier.verify_is_null_tests()
    
    if "setup_failed" in results:
        print("❌ 测试环境设置失败")
        return False
    
    return results["failed"] == 0

if __name__ == "__main__":
    success = main()
    exit(0 if success else 1)
