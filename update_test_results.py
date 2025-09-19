#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
自动更新测试结果文件脚本
基于当前MiniOB的实际输出更新.result文件
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path

class TestResultUpdater:
    def __init__(self, project_root="./"):
        self.project_root = Path(project_root).resolve()
        self.test_case_dir = self.project_root / "test" / "case" / "test"
        self.result_dir = self.project_root / "test" / "case" / "result"
        self.miniob_test_script = self.project_root / "test" / "case" / "miniob_test.py"
        
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
            # 使用--report-only模式生成新结果
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
    
    def validate_result_file(self, test_name: str) -> bool:
        """验证生成的结果文件"""
        result_file = self.result_dir / f"{test_name}.result"
        
        if not result_file.exists():
            print(f"❌ 结果文件不存在: {result_file}")
            return False
        
        try:
            with open(result_file, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # 检查文件是否包含预期的内容
            if len(content.strip()) == 0:
                print(f"❌ 结果文件为空: {result_file}")
                return False
            
            # 检查是否包含SUCCESS或有效的查询结果
            if "SUCCESS" in content or "|" in content:
                print(f"✅ 结果文件验证通过: {result_file}")
                return True
            else:
                print(f"⚠️ 结果文件可能不完整: {result_file}")
                return False
                
        except Exception as e:
            print(f"❌ 读取结果文件失败: {e}")
            return False
    
    def show_diff(self, test_name: str):
        """显示新旧结果文件的差异"""
        result_file = self.result_dir / f"{test_name}.result"
        backup_file = self.result_dir / f"{test_name}.result.backup"
        
        if not backup_file.exists():
            print("⚠️ 没有备份文件，无法显示差异")
            return
        
        try:
            cmd = ["diff", "-u", str(backup_file), str(result_file)]
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.stdout:
                print(f"\n📊 {test_name} 的变化:")
                print("=" * 60)
                print(result.stdout)
                print("=" * 60)
            else:
                print(f"✅ {test_name} 没有变化")
                
        except Exception as e:
            print(f"❌ 显示差异失败: {e}")
    
    def update_test_case(self, test_name: str) -> bool:
        """更新单个测试用例的结果"""
        print(f"\n🔄 更新测试用例: {test_name}")
        print("-" * 40)
        
        # 1. 备份原结果文件
        if not self.backup_result_file(test_name):
            return False
        
        # 2. 生成新的结果
        if not self.generate_new_result(test_name):
            return False
        
        # 3. 验证新结果
        if not self.validate_result_file(test_name):
            return False
        
        # 4. 显示差异
        self.show_diff(test_name)
        
        print(f"✅ 成功更新测试用例: {test_name}")
        return True
    
    def update_multiple_tests(self, test_names: list) -> dict:
        """更新多个测试用例"""
        results = {}
        
        print(f"🚀 开始更新 {len(test_names)} 个测试用例")
        print("=" * 60)
        
        for test_name in test_names:
            results[test_name] = self.update_test_case(test_name)
        
        # 输出总结
        print(f"\n📊 更新总结:")
        print("=" * 60)
        
        success_count = sum(1 for success in results.values() if success)
        total_count = len(test_names)
        
        print(f"总计: {total_count} 个测试用例")
        print(f"成功: {success_count} 个")
        print(f"失败: {total_count - success_count} 个")
        
        if success_count < total_count:
            print("\n❌ 失败的测试用例:")
            for test_name, success in results.items():
                if not success:
                    print(f"  - {test_name}")
        
        return results
    
    def list_available_tests(self) -> list:
        """列出所有可用的测试用例"""
        test_files = list(self.test_case_dir.glob("*.test"))
        test_names = [f.stem for f in test_files]
        return sorted(test_names)

def main():
    """主函数"""
    updater = TestResultUpdater()
    
    if len(sys.argv) < 2:
        print("用法:")
        print("  python3 update_test_results.py <test_name>        # 更新单个测试")
        print("  python3 update_test_results.py --all              # 更新所有测试")
        print("  python3 update_test_results.py --list             # 列出所有测试")
        print("  python3 update_test_results.py primary-null       # 更新NULL相关测试")
        print("  python3 update_test_results.py primary-expression # 更新表达式测试")
        sys.exit(1)
    
    command = sys.argv[1]
    
    if command == "--list":
        tests = updater.list_available_tests()
        print("可用的测试用例:")
        for test in tests:
            print(f"  - {test}")
        sys.exit(0)
    
    elif command == "--all":
        tests = updater.list_available_tests()
        print(f"将更新所有 {len(tests)} 个测试用例")
        
        # 询问确认
        response = input("确认继续? (y/N): ")
        if response.lower() != 'y':
            print("已取消")
            sys.exit(0)
        
        results = updater.update_multiple_tests(tests)
        success_count = sum(1 for success in results.values() if success)
        
        if success_count == len(tests):
            print("\n🎉 所有测试用例更新成功!")
            sys.exit(0)
        else:
            print(f"\n⚠️ 部分测试用例更新失败 ({success_count}/{len(tests)})")
            sys.exit(1)
    
    else:
        # 更新指定的测试用例
        test_name = command
        available_tests = updater.list_available_tests()
        
        if test_name not in available_tests:
            print(f"❌ 测试用例不存在: {test_name}")
            print("可用的测试用例:")
            for test in available_tests:
                print(f"  - {test}")
            sys.exit(1)
        
        success = updater.update_test_case(test_name)
        
        if success:
            print(f"\n🎉 测试用例 {test_name} 更新成功!")
            sys.exit(0)
        else:
            print(f"\n❌ 测试用例 {test_name} 更新失败!")
            sys.exit(1)

if __name__ == "__main__":
    main()
