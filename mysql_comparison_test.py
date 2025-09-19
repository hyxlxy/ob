#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
MySQL 8.0 与 MiniOB 结果对比测试脚本
自动验证 MiniOB 的输出是否与 MySQL 8.0 一致
"""

import subprocess
import tempfile
import os
import sys
import re
from typing import List, Tuple, Dict

class MySQLComparator:
    def __init__(self, mysql_host="localhost", mysql_port=3306, mysql_user="root", mysql_password=""):
        self.mysql_host = mysql_host
        self.mysql_port = mysql_port
        self.mysql_user = mysql_user
        self.mysql_password = mysql_password
        self.miniob_build_path = "../../build"
        
    def run_mysql_command(self, sql: str) -> Tuple[bool, str]:
        """在MySQL中执行SQL命令"""
        try:
            cmd = [
                "mysql",
                f"-h{self.mysql_host}",
                f"-P{self.mysql_port}",
                f"-u{self.mysql_user}",
                "--skip-column-names",
                "--raw",
                "-e", sql
            ]
            
            if self.mysql_password:
                cmd.insert(-2, f"-p{self.mysql_password}")
                
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
            
            if result.returncode == 0:
                return True, result.stdout.strip()
            else:
                return False, result.stderr.strip()
                
        except Exception as e:
            return False, str(e)
    
    def run_miniob_command(self, sql: str) -> Tuple[bool, str]:
        """在MiniOB中执行SQL命令"""
        try:
            observer_path = f"{self.miniob_build_path}/bin/observer"
            
            # 使用echo将SQL传递给observer
            cmd = f'echo "{sql}" | {observer_path} -P cli'
            
            result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=30)
            
            if "SUCCESS" in result.stdout or "|" in result.stdout:
                # 提取实际的查询结果，去掉调试信息
                lines = result.stdout.split('\n')
                result_lines = []
                in_result = False
                
                for line in lines:
                    if '|' in line and not line.startswith('DEBUG'):
                        in_result = True
                        result_lines.append(line.strip())
                    elif in_result and line.strip() == "":
                        break
                        
                return True, '\n'.join(result_lines)
            else:
                return False, result.stdout.strip()
                
        except Exception as e:
            return False, str(e)
    
    def normalize_result(self, result: str) -> List[List[str]]:
        """标准化查询结果格式"""
        if not result.strip():
            return []
            
        lines = result.strip().split('\n')
        normalized = []
        
        for line in lines:
            if '|' in line:
                # 分割列并去除空白
                cols = [col.strip() for col in line.split('|')]
                normalized.append(cols)
                
        return normalized
    
    def compare_results(self, mysql_result: str, miniob_result: str) -> Tuple[bool, str]:
        """对比MySQL和MiniOB的结果"""
        mysql_data = self.normalize_result(mysql_result)
        miniob_data = self.normalize_result(miniob_result)
        
        if len(mysql_data) != len(miniob_data):
            return False, f"行数不匹配: MySQL={len(mysql_data)}, MiniOB={len(miniob_data)}"
        
        for i, (mysql_row, miniob_row) in enumerate(zip(mysql_data, miniob_data)):
            if len(mysql_row) != len(miniob_row):
                return False, f"第{i+1}行列数不匹配: MySQL={len(mysql_row)}, MiniOB={len(miniob_row)}"
            
            for j, (mysql_val, miniob_val) in enumerate(zip(mysql_row, miniob_row)):
                # 处理NULL值的比较
                mysql_val = mysql_val.upper() if mysql_val.upper() != "NULL" else "NULL"
                miniob_val = miniob_val.upper() if miniob_val.upper() != "NULL" else "NULL"
                
                if mysql_val != miniob_val:
                    return False, f"第{i+1}行第{j+1}列不匹配: MySQL='{mysql_val}', MiniOB='{miniob_val}'"
        
        return True, "结果一致"
    
    def test_sql_statement(self, sql: str) -> Dict:
        """测试单个SQL语句"""
        print(f"测试SQL: {sql}")
        
        # 在MySQL中执行
        mysql_success, mysql_result = self.run_mysql_command(sql)
        print(f"MySQL结果: {'成功' if mysql_success else '失败'}")
        if mysql_success:
            print(f"MySQL输出: {mysql_result}")
        else:
            print(f"MySQL错误: {mysql_result}")
        
        # 在MiniOB中执行
        miniob_success, miniob_result = self.run_miniob_command(sql)
        print(f"MiniOB结果: {'成功' if miniob_success else '失败'}")
        if miniob_success:
            print(f"MiniOB输出: {miniob_result}")
        else:
            print(f"MiniOB错误: {miniob_result}")
        
        # 对比结果
        if mysql_success and miniob_success:
            match, message = self.compare_results(mysql_result, miniob_result)
            print(f"对比结果: {message}")
            return {
                "sql": sql,
                "mysql_success": True,
                "miniob_success": True,
                "match": match,
                "message": message,
                "mysql_result": mysql_result,
                "miniob_result": miniob_result
            }
        else:
            return {
                "sql": sql,
                "mysql_success": mysql_success,
                "miniob_success": miniob_success,
                "match": False,
                "message": "执行失败",
                "mysql_result": mysql_result if mysql_success else None,
                "miniob_result": miniob_result if miniob_success else None
            }

def main():
    """主函数：测试IS NULL相关功能"""
    comparator = MySQLComparator()
    
    # 准备测试数据的SQL
    setup_sqls = [
        "DROP DATABASE IF EXISTS test_miniob;",
        "CREATE DATABASE test_miniob;",
        "USE test_miniob;",
        "CREATE TABLE null_table(id int not null, num int null, price float not null, birthday date null);",
        "INSERT INTO null_table VALUES (1, 18, 10.0, '2020-01-01');",
        "INSERT INTO null_table VALUES (2, null, 20.0, '2010-01-11');",
        "INSERT INTO null_table VALUES (3, 12, 30.0, null);",
        "INSERT INTO null_table VALUES (4, 15, 30.0, '2021-01-31');"
    ]
    
    # IS NULL测试用例
    test_sqls = [
        "SELECT * FROM null_table WHERE 1 IS NULL;",
        "SELECT * FROM null_table WHERE 1 IS NOT NULL;",
        "SELECT * FROM null_table WHERE NULL IS NULL;",
        "SELECT * FROM null_table WHERE NULL IS NOT NULL;",
        "SELECT * FROM null_table WHERE num IS NULL;",
        "SELECT * FROM null_table WHERE num IS NOT NULL;",
        "SELECT * FROM null_table WHERE birthday IS NULL;",
        "SELECT * FROM null_table WHERE birthday IS NOT NULL;",
        "SELECT COUNT(*) FROM null_table;",
        "SELECT COUNT(num) FROM null_table;",
        "SELECT COUNT(birthday) FROM null_table;"
    ]
    
    print("=== MySQL 8.0 与 MiniOB 对比测试 ===\n")
    
    # 设置测试环境
    print("设置测试环境...")
    for sql in setup_sqls:
        comparator.run_mysql_command(sql)
    
    # 执行测试
    results = []
    passed = 0
    failed = 0
    
    for sql in test_sqls:
        print(f"\n{'='*60}")
        result = comparator.test_sql_statement(sql)
        results.append(result)
        
        if result["match"]:
            passed += 1
            print("✅ 测试通过")
        else:
            failed += 1
            print("❌ 测试失败")
    
    # 输出总结
    print(f"\n{'='*60}")
    print("测试总结:")
    print(f"总计: {len(test_sqls)} 个测试")
    print(f"通过: {passed} 个")
    print(f"失败: {failed} 个")
    print(f"成功率: {passed/len(test_sqls)*100:.1f}%")
    
    if failed > 0:
        print("\n失败的测试:")
        for result in results:
            if not result["match"]:
                print(f"- {result['sql']}: {result['message']}")
    
    return failed == 0

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
