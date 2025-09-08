#!/usr/bin/env python3
"""
自动化SQL测试程序
使用方法: python3 auto_test.py [sql_file]
如果不指定sql_file，默认使用test.sql
"""

import subprocess
import sys
import time
import os
import signal
import threading
from queue import Queue, Empty

class SQLTester:
    def __init__(self, observer_path="./build_debug/bin/observer", obclient_path="./build_debug/bin/obclient"):
        self.observer_path = observer_path
        self.obclient_path = obclient_path
        self.observer_process = None
        
    def start_observer(self):
        """启动observer服务器"""
        print("启动observer服务器...")
        try:
            self.observer_process = subprocess.Popen(
                [self.observer_path],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                preexec_fn=os.setsid  # 创建新的进程组
            )
            # 等待服务器启动
            time.sleep(2)
            print("observer服务器已启动")
            return True
        except Exception as e:
            print(f"启动observer失败: {e}")
            return False
    
    def stop_observer(self):
        """停止observer服务器"""
        if self.observer_process:
            print("停止observer服务器...")
            try:
                # 终止整个进程组
                os.killpg(os.getpgid(self.observer_process.pid), signal.SIGTERM)
                self.observer_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                # 如果5秒内没有终止，强制杀死
                os.killpg(os.getpgid(self.observer_process.pid), signal.SIGKILL)
                self.observer_process.wait()
            except Exception as e:
                print(f"停止observer时出错: {e}")
            self.observer_process = None
            print("observer服务器已停止")
    
    def execute_sql_one_by_one(self, sql_statements):
        """逐条执行SQL语句，返回每条语句的结果"""
        results = []

        for i, sql in enumerate(sql_statements):
            print(f"执行第 {i+1} 条SQL: {sql[:50]}{'...' if len(sql) > 50 else ''}")

            try:
                # 启动obclient进程
                process = subprocess.Popen(
                    [self.obclient_path],
                    stdin=subprocess.PIPE,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True
                )

                # 发送SQL并获取结果
                stdout, stderr = process.communicate(input=sql + '\n', timeout=15)

                # 分析结果
                success = "SUCCESS" in stdout and process.returncode == 0
                result = {
                    'sql': sql,
                    'success': success,
                    'stdout': stdout,
                    'stderr': stderr,
                    'returncode': process.returncode
                }
                results.append(result)

                # 显示结果
                if success:
                    print("  ✅ 成功")
                else:
                    print("  ❌ 失败")
                    if stderr:
                        print(f"    错误: {stderr.strip()}")

                # 如果有查询结果，显示出来
                lines = stdout.split('\n')
                for line in lines:
                    if '|' in line and 'Welcome' not in line and 'Copyright' not in line and 'Learn' not in line and 'SUCCESS' not in line:
                        print(f"    {line}")

            except subprocess.TimeoutExpired:
                process.kill()
                result = {
                    'sql': sql,
                    'success': False,
                    'stdout': '',
                    'stderr': '执行超时',
                    'returncode': -1
                }
                results.append(result)
                print("  ❌ 超时")

            except Exception as e:
                result = {
                    'sql': sql,
                    'success': False,
                    'stdout': '',
                    'stderr': f'执行出错: {e}',
                    'returncode': -1
                }
                results.append(result)
                print(f"  ❌ 出错: {e}")

        return results
    
    def parse_sql_file(self, filename):
        """解析SQL文件，返回SQL语句列表"""
        if not os.path.exists(filename):
            print(f"SQL文件不存在: {filename}")
            return []
        
        with open(filename, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 简单的SQL语句分割（以分号为分隔符）
        statements = []
        current_statement = ""
        
        for line in content.split('\n'):
            line = line.strip()
            # 跳过注释行
            if line.startswith('--') or line.startswith('#') or not line:
                continue
            
            current_statement += line + " "
            
            # 如果行以分号结尾，表示一个完整的SQL语句
            if line.endswith(';'):
                statements.append(current_statement.strip())
                current_statement = ""
        
        # 如果最后还有未完成的语句
        if current_statement.strip():
            statements.append(current_statement.strip())
        
        return statements
    
    def run_test(self, sql_file):
        """运行测试"""
        print(f"开始测试SQL文件: {sql_file}")
        print("=" * 60)
        
        # 解析SQL文件
        statements = self.parse_sql_file(sql_file)
        if not statements:
            print("没有找到有效的SQL语句")
            return
        
        print(f"找到 {len(statements)} 条SQL语句")
        print("-" * 60)
        
        # 启动observer
        if not self.start_observer():
            return
        
        try:
            # 逐条执行SQL语句
            results = self.execute_sql_one_by_one(statements)

            # 统计结果
            success_count = sum(1 for r in results if r['success'])
            total_count = len(results)

            print("\n" + "=" * 60)
            print("测试总结:")
            print(f"总共执行: {total_count} 条SQL语句")
            print(f"成功: {success_count} 条")
            print(f"失败: {total_count - success_count} 条")

            if total_count - success_count > 0:
                print("\n失败的语句:")
                for i, result in enumerate(results):
                    if not result['success']:
                        print(f"  {i+1}. {result['sql'][:60]}{'...' if len(result['sql']) > 60 else ''}")
                        if result['stderr']:
                            print(f"     错误: {result['stderr']}")

            if success_count == total_count:
                print("\n🎉 所有测试都通过了！")
            else:
                print(f"\n⚠️  有 {total_count - success_count} 个测试失败")

        finally:
            # 停止observer
            self.stop_observer()
        
        print("=" * 60)
        print("测试完成")

def main():
    # 获取SQL文件名
    sql_file = "test.sql"
    if len(sys.argv) > 1:
        sql_file = sys.argv[1]
    
    # 创建测试器并运行测试
    tester = SQLTester()
    tester.run_test(sql_file)

if __name__ == "__main__":
    main()
