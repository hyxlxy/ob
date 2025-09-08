#!/usr/bin/env python3
"""
简单的SQL测试程序
使用方法: python3 simple_test.py [sql_file]
"""

import subprocess
import sys
import time
import os
import signal

class SimpleSQLTester:
    def __init__(self, observer_path="./build_debug/bin/observer", obclient_path="./build_debug/bin/obclient"):
        self.observer_path = observer_path
        self.obclient_path = obclient_path
        self.observer_process = None
        
    def start_observer(self):
        """启动observer服务器"""
        print("🚀 启动observer服务器...")
        try:
            self.observer_process = subprocess.Popen(
                [self.observer_path],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                preexec_fn=os.setsid
            )
            time.sleep(3)  # 等待服务器启动
            print("✅ observer服务器已启动")
            return True
        except Exception as e:
            print(f"❌ 启动observer失败: {e}")
            return False
    
    def stop_observer(self):
        """停止observer服务器"""
        if self.observer_process:
            print("🛑 停止observer服务器...")
            try:
                os.killpg(os.getpgid(self.observer_process.pid), signal.SIGTERM)
                self.observer_process.wait(timeout=5)
            except:
                try:
                    os.killpg(os.getpgid(self.observer_process.pid), signal.SIGKILL)
                    self.observer_process.wait()
                except:
                    pass
            self.observer_process = None
            print("✅ observer服务器已停止")
    
    def execute_sql_file(self, sql_file):
        """执行SQL文件"""
        try:
            with open(sql_file, 'r', encoding='utf-8') as f:
                sql_content = f.read()
            
            # 启动obclient进程
            process = subprocess.Popen(
                [self.obclient_path],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            
            # 发送SQL并获取结果
            stdout, stderr = process.communicate(input=sql_content, timeout=60)
            
            return stdout, stderr, process.returncode
            
        except subprocess.TimeoutExpired:
            process.kill()
            return "", "执行超时", -1
        except Exception as e:
            return "", f"执行出错: {e}", -1
    
    def analyze_output(self, stdout):
        """分析输出，统计成功和失败"""
        lines = stdout.split('\n')
        success_count = 0
        failure_count = 0
        results = []
        
        for line in lines:
            line = line.strip()
            if line == "SUCCESS":
                success_count += 1
                results.append("✅ SUCCESS")
            elif line == "FAILURE":
                failure_count += 1
                results.append("❌ FAILURE")
            elif '|' in line and line.count('|') >= 2:  # 查询结果
                results.append(f"📊 {line}")
        
        return success_count, failure_count, results
    
    def run_test(self, sql_file):
        """运行测试"""
        print(f"📋 开始测试SQL文件: {sql_file}")
        print("=" * 60)
        
        if not os.path.exists(sql_file):
            print(f"❌ SQL文件不存在: {sql_file}")
            return
        
        # 启动observer
        if not self.start_observer():
            return
        
        try:
            print(f"📝 执行SQL文件...")
            stdout, stderr, returncode = self.execute_sql_file(sql_file)
            
            print("\n📈 执行结果分析:")
            print("-" * 40)
            
            success_count, failure_count, results = self.analyze_output(stdout)
            
            print(f"✅ 成功: {success_count}")
            print(f"❌ 失败: {failure_count}")
            print(f"📊 总计: {success_count + failure_count}")
            
            if results:
                print("\n📋 详细结果:")
                for result in results:
                    print(f"  {result}")
            
            if stderr and stderr.strip():
                print(f"\n⚠️  错误信息:")
                print(stderr)
            
            print(f"\n🔢 返回码: {returncode}")
            
            if failure_count == 0 and returncode == 0:
                print("\n🎉 所有测试都通过了！")
            else:
                print(f"\n⚠️  有 {failure_count} 个测试失败")
                
        finally:
            self.stop_observer()
        
        print("=" * 60)
        print("🏁 测试完成")

def main():
    sql_file = "test.sql"
    if len(sys.argv) > 1:
        sql_file = sys.argv[1]
    
    tester = SimpleSQLTester()
    tester.run_test(sql_file)

if __name__ == "__main__":
    main()
