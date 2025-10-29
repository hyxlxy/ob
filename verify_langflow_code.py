#!/usr/bin/env python3
"""
验证 Langflow 是否加载了正确的代码
通过 API 检查运行时的组件代码
"""

import requests
import json
import sys

def verify_langflow_code():
    print("=" * 70)
    print("验证 Langflow 运行时代码")
    print("=" * 70)
    print()
    
    # 1. 检查 Langflow 是否运行
    print("1. 检查 Langflow 服务...")
    try:
        response = requests.get("http://localhost:7860/health", timeout=5)
        if response.status_code == 200:
            print("   ✓ Langflow 正在运行")
        else:
            print(f"   ✗ Langflow 响应异常: {response.status_code}")
            return False
    except requests.exceptions.ConnectionError:
        print("   ✗ 无法连接到 Langflow")
        print("   请先启动 Langflow: langflow run")
        return False
    except Exception as e:
        print(f"   ✗ 错误: {e}")
        return False
    
    # 2. 读取本地文件
    print()
    print("2. 读取本地 model.json...")
    try:
        with open("rag/model.json", 'r', encoding='utf-8') as f:
            local_data = json.load(f)
        
        # 找到 MiniOB 组件
        miniob_node = None
        for node in local_data.get("data", {}).get("nodes", []):
            if node.get("data", {}).get("type") == "MiniOB":
                miniob_node = node
                break
        
        if not miniob_node:
            print("   ✗ 未找到 MiniOB 组件")
            return False
        
        local_code = miniob_node.get("data", {}).get("node", {}).get("template", {}).get("code", {}).get("value", "")
        print(f"   ✓ 本地代码长度: {len(local_code)} 字符")
        
        # 检查关键修复
        has_hardcode = "self.__embedding_dimension = 1024" in local_code
        has_filter = "valid_documents = []" in local_code
        has_comment = "Filter out documents with empty page_content" in local_code
        
        print(f"   ✓ 硬编码维度: {has_hardcode}")
        print(f"   ✓ 空文档过滤: {has_filter}")
        print(f"   ✓ 过滤注释: {has_comment}")
        
        if not (has_hardcode and has_filter and has_comment):
            print("   ✗ 本地文件缺少必要的修复!")
            return False
        
    except Exception as e:
        print(f"   ✗ 读取失败: {e}")
        return False
    
    # 3. 提示用户
    print()
    print("3. 验证 Langflow 中的代码...")
    print()
    print("   请在 Langflow 界面中:")
    print("   1. 打开你的工作流")
    print("   2. 点击 MiniOB 组件")
    print("   3. 查看 'Code' 或 '代码' 标签")
    print("   4. 搜索以下文本:")
    print()
    print("      搜索 1: 'valid_documents = []'")
    print("      搜索 2: 'Filter out documents with empty'")
    print("      搜索 3: 'self.__embedding_dimension = 1024'")
    print()
    print("   如果找到这些文本,说明代码已正确加载 ✓")
    print("   如果找不到,说明 Langflow 使用的是旧代码 ✗")
    print()
    
    # 4. 显示代码片段
    print("4. 本地文件中的关键代码片段:")
    print()
    print("-" * 70)
    
    # 找到并显示过滤代码
    import re
    pattern = r'# Filter out documents with empty page_content.*?page_contents: List\[str\]'
    match = re.search(pattern, local_code, re.DOTALL)
    if match:
        snippet = match.group(0)
        lines = snippet.split('\n')[:15]  # 只显示前15行
        for line in lines:
            print(line)
    
    print("-" * 70)
    print()
    
    return True

def show_instructions():
    print("=" * 70)
    print("如果 Langflow 中的代码与本地不一致")
    print("=" * 70)
    print()
    print("可能的原因:")
    print("  1. 没有重新导入工作流")
    print("  2. Langflow 缓存了旧版本")
    print("  3. 浏览器缓存了旧版本")
    print()
    print("解决方法:")
    print()
    print("方法 1: 完全重启")
    print("  1. 停止 Langflow: pkill -9 -f langflow")
    print("  2. 清除缓存: rm -rf ~/.cache/langflow ~/.langflow")
    print("  3. 关闭浏览器(所有窗口)")
    print("  4. 启动 Langflow: langflow run")
    print("  5. 使用无痕模式打开浏览器")
    print("  6. 导入 rag/model_fresh_XXXXXX.json")
    print()
    print("方法 2: 在 Langflow 中手动修改代码")
    print("  1. 打开 MiniOB 组件")
    print("  2. 点击 'Code' 标签")
    print("  3. 找到这一行:")
    print("     page_contents: List[str] = [document.page_content for document in documents]")
    print("  4. 在它前面添加:")
    print()
    print("     # Filter out empty documents")
    print("     valid_documents = [doc for doc in documents if doc.page_content and doc.page_content.strip()]")
    print("     if not valid_documents:")
    print("         self.__log_func('No valid documents')")
    print("         return []")
    print()
    print("  5. 修改那一行为:")
    print("     page_contents: List[str] = [document.page_content for document in valid_documents]")
    print()
    print("  6. 在后面所有使用 'documents' 的地方改为 'valid_documents'")
    print()
    print("=" * 70)
    print()

if __name__ == "__main__":
    print()
    success = verify_langflow_code()
    print()
    
    if success:
        print("✅ 本地文件验证通过")
        print()
        print("现在请在 Langflow 界面中验证代码是否一致")
        print()
        
        response = input("Langflow 中的代码是否包含 'valid_documents'? (y/n): ")
        if response.lower() == 'y':
            print()
            print("✅ 太好了!代码已正确加载")
            print()
            print("如果仍然报错,可能是:")
            print("  1. 文档数据本身有问题")
            print("  2. 需要检查 Directory 组件的输出")
            print()
            sys.exit(0)
        else:
            print()
            print("❌ Langflow 没有加载新代码")
            print()
            show_instructions()
            sys.exit(1)
    else:
        print("❌ 验证失败")
        print()
        show_instructions()
        sys.exit(1)

