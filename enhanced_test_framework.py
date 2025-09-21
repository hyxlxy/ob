#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

"""
MiniOB增强测试框架
扩展原有测试框架，添加性能测试、并发测试、压力测试等高级功能
"""

import os
import sys
import time
import threading
import subprocess
import statistics
from typing import List, Dict, Tuple, Optional
from concurrent.futures import ThreadPoolExecutor, as_completed
import json

class PerformanceTestRunner:
    """性能测试运行器"""
    
    def __init__(self, observer_path: str = "./build_debug/bin/observer"):
        self.observer_path = observer_path
        self.results = []
    
    def run_performance_test(self, sql_commands: List[str], iterations: int = 100) -> Dict[str, any]:
        """运行性能测试"""
        print(f"🚀 开始性能测试 ({iterations}次迭代)...")
        
        execution_times = []
        
        for i in range(iterations):
            start_time = time.time()
            
            # 执行SQL命令
            success = self._execute_sql_commands(sql_commands)
            
            end_time = time.time()
            execution_time = end_time - start_time
            
            if success:
                execution_times.append(execution_time)
            
            if (i + 1) % 10 == 0:
                print(f"   完成 {i + 1}/{iterations} 次测试")
        
        # 计算统计信息
        if execution_times:
            stats = {
                'total_iterations': iterations,
                'successful_iterations': len(execution_times),
                'success_rate': len(execution_times) / iterations * 100,
                'avg_time': statistics.mean(execution_times),
                'min_time': min(execution_times),
                'max_time': max(execution_times),
                'median_time': statistics.median(execution_times),
                'std_dev': statistics.stdev(execution_times) if len(execution_times) > 1 else 0
            }
        else:
            stats = {
                'total_iterations': iterations,
                'successful_iterations': 0,
                'success_rate': 0,
                'avg_time': 0,
                'min_time': 0,
                'max_time': 0,
                'median_time': 0,
                'std_dev': 0
            }
        
        return stats
    
    def _execute_sql_commands(self, commands: List[str]) -> bool:
        """执行SQL命令"""
        try:
            # 创建临时SQL文件
            temp_sql = '\n'.join(commands)
            
            # 使用echo和管道执行命令
            process = subprocess.run(
                f'echo "{temp_sql}" | {self.observer_path} -P cli',
                shell=True,
                capture_output=True,
                text=True,
                timeout=30
            )
            
            return process.returncode == 0
        except Exception:
            return False

class ConcurrencyTestRunner:
    """并发测试运行器"""
    
    def __init__(self, observer_path: str = "./build_debug/bin/observer"):
        self.observer_path = observer_path
    
    def run_concurrency_test(self, sql_commands: List[str], num_threads: int = 5, 
                           iterations_per_thread: int = 20) -> Dict[str, any]:
        """运行并发测试"""
        print(f"🔄 开始并发测试 ({num_threads}个线程, 每线程{iterations_per_thread}次)...")
        
        results = []
        start_time = time.time()
        
        with ThreadPoolExecutor(max_workers=num_threads) as executor:
            # 提交任务
            futures = []
            for i in range(num_threads):
                future = executor.submit(self._thread_worker, sql_commands, iterations_per_thread, i)
                futures.append(future)
            
            # 收集结果
            for future in as_completed(futures):
                thread_result = future.result()
                results.append(thread_result)
        
        end_time = time.time()
        total_time = end_time - start_time
        
        # 汇总统计
        total_operations = sum(r['successful_operations'] for r in results)
        total_attempts = sum(r['total_attempts'] for r in results)
        
        stats = {
            'num_threads': num_threads,
            'iterations_per_thread': iterations_per_thread,
            'total_time': total_time,
            'total_operations': total_operations,
            'total_attempts': total_attempts,
            'success_rate': (total_operations / total_attempts * 100) if total_attempts > 0 else 0,
            'operations_per_second': total_operations / total_time if total_time > 0 else 0,
            'thread_results': results
        }
        
        return stats
    
    def _thread_worker(self, commands: List[str], iterations: int, thread_id: int) -> Dict[str, any]:
        """线程工作函数"""
        successful = 0
        total = iterations
        
        for i in range(iterations):
            try:
                # 为每个线程创建独特的数据
                modified_commands = [cmd.replace('test_table', f'test_table_{thread_id}') for cmd in commands]
                
                temp_sql = '\n'.join(modified_commands)
                process = subprocess.run(
                    f'echo "{temp_sql}" | {self.observer_path} -P cli',
                    shell=True,
                    capture_output=True,
                    text=True,
                    timeout=10
                )
                
                if process.returncode == 0:
                    successful += 1
                    
            except Exception:
                pass
        
        return {
            'thread_id': thread_id,
            'successful_operations': successful,
            'total_attempts': total,
            'success_rate': (successful / total * 100) if total > 0 else 0
        }

class StressTestRunner:
    """压力测试运行器"""
    
    def __init__(self, observer_path: str = "./build_debug/bin/observer"):
        self.observer_path = observer_path
    
    def run_stress_test(self, base_commands: List[str], max_data_size: int = 10000) -> Dict[str, any]:
        """运行压力测试"""
        print(f"💪 开始压力测试 (最大数据量: {max_data_size})...")
        
        results = []
        data_sizes = [100, 500, 1000, 2000, 5000, max_data_size]
        
        for size in data_sizes:
            print(f"   测试数据量: {size}")
            
            # 生成大量数据的命令
            stress_commands = self._generate_stress_commands(base_commands, size)
            
            start_time = time.time()
            success = self._execute_stress_commands(stress_commands)
            end_time = time.time()
            
            execution_time = end_time - start_time
            
            result = {
                'data_size': size,
                'execution_time': execution_time,
                'success': success,
                'throughput': size / execution_time if execution_time > 0 and success else 0
            }
            
            results.append(result)
            
            if not success:
                print(f"   ❌ 数据量 {size} 测试失败")
                break
            else:
                print(f"   ✅ 数据量 {size} 测试成功 ({execution_time:.2f}秒)")
        
        return {
            'max_successful_size': max([r['data_size'] for r in results if r['success']], default=0),
            'results': results
        }
    
    def _generate_stress_commands(self, base_commands: List[str], data_size: int) -> List[str]:
        """生成压力测试命令"""
        commands = []
        
        # 添加建表命令
        commands.append("create table stress_test(id int, data char(100), value float);")
        
        # 生成大量插入命令
        for i in range(data_size):
            commands.append(f"insert into stress_test values({i}, 'data_{i}', {i * 1.5});")
        
        # 添加查询命令
        commands.extend([
            "select count(*) from stress_test;",
            "select * from stress_test where id < 10;",
            "select avg(value) from stress_test;",
            "drop table stress_test;"
        ])
        
        return commands
    
    def _execute_stress_commands(self, commands: List[str]) -> bool:
        """执行压力测试命令"""
        try:
            temp_sql = '\n'.join(commands)
            
            process = subprocess.run(
                f'echo "{temp_sql}" | {self.observer_path} -P cli',
                shell=True,
                capture_output=True,
                text=True,
                timeout=300  # 5分钟超时
            )
            
            return process.returncode == 0
        except Exception:
            return False

class EnhancedTestFramework:
    """增强测试框架"""
    
    def __init__(self):
        self.performance_runner = PerformanceTestRunner()
        self.concurrency_runner = ConcurrencyTestRunner()
        self.stress_runner = StressTestRunner()
    
    def run_comprehensive_test(self, test_name: str = "comprehensive"):
        """运行综合测试"""
        print("=" * 80)
        print("🧪 MiniOB增强测试框架 - 综合测试")
        print("=" * 80)
        
        # 基础测试命令
        base_commands = [
            "create table test_table(id int, name char(20), score float);",
            "insert into test_table values(1, 'Alice', 85.5);",
            "insert into test_table values(2, 'Bob', 78.0);",
            "select * from test_table;",
            "select avg(score) from test_table;",
            "drop table test_table;"
        ]
        
        results = {}
        
        # 1. 性能测试
        print("\n1️⃣ 性能测试")
        results['performance'] = self.performance_runner.run_performance_test(base_commands, 50)
        
        # 2. 并发测试
        print("\n2️⃣ 并发测试")
        results['concurrency'] = self.concurrency_runner.run_concurrency_test(base_commands, 3, 10)
        
        # 3. 压力测试
        print("\n3️⃣ 压力测试")
        results['stress'] = self.stress_runner.run_stress_test(base_commands, 1000)
        
        # 保存结果
        self._save_results(results, test_name)
        
        # 打印总结
        self._print_summary(results)
        
        return results
    
    def _save_results(self, results: Dict[str, any], test_name: str):
        """保存测试结果"""
        filename = f"enhanced_test_results_{test_name}_{int(time.time())}.json"
        with open(filename, 'w', encoding='utf-8') as f:
            json.dump(results, f, ensure_ascii=False, indent=2)
        print(f"\n📄 测试结果已保存到: {filename}")
    
    def _print_summary(self, results: Dict[str, any]):
        """打印测试总结"""
        print("\n" + "=" * 80)
        print("📊 测试总结")
        print("=" * 80)
        
        # 性能测试总结
        perf = results['performance']
        print(f"\n🚀 性能测试:")
        print(f"   成功率: {perf['success_rate']:.1f}%")
        print(f"   平均执行时间: {perf['avg_time']:.3f}秒")
        print(f"   最快执行时间: {perf['min_time']:.3f}秒")
        print(f"   最慢执行时间: {perf['max_time']:.3f}秒")
        
        # 并发测试总结
        conc = results['concurrency']
        print(f"\n🔄 并发测试:")
        print(f"   线程数: {conc['num_threads']}")
        print(f"   总成功率: {conc['success_rate']:.1f}%")
        print(f"   每秒操作数: {conc['operations_per_second']:.1f}")
        
        # 压力测试总结
        stress = results['stress']
        print(f"\n💪 压力测试:")
        print(f"   最大成功数据量: {stress['max_successful_size']}")
        
        print("\n" + "=" * 80)

def main():
    """主函数"""
    framework = EnhancedTestFramework()
    framework.run_comprehensive_test()

if __name__ == "__main__":
    main()
