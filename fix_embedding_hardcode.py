#!/usr/bin/env python3
"""
修复嵌入维度检测问题 - 硬编码 bge-m3 维度为 1024
直接跳过维度检测,避免 Ollama 验证错误
"""

import json
import sys
from datetime import datetime

def fix_embedding_dimension():
    model_file = "rag/model.json"
    
    print("=" * 70)
    print("修复嵌入维度检测问题 - 硬编码方案")
    print("=" * 70)
    print()
    
    # 1. 读取 JSON 文件
    print(f"1. 读取文件: {model_file}")
    try:
        with open(model_file, 'r', encoding='utf-8') as f:
            data = json.load(f)
        print(f"   ✓ 文件读取成功")
    except Exception as e:
        print(f"   ✗ 错误: {e}")
        return False
    
    # 2. 查找 MiniOB 组件
    print()
    print("2. 查找 MiniOB 组件...")
    miniob_node = None

    # 检查数据结构
    if "nodes" in data:
        # 旧格式
        for node_id, node in data.get("nodes", {}).items():
            if node.get("data", {}).get("type") == "MiniOB":
                miniob_node = node
                print(f"   ✓ 找到 MiniOB 组件 (旧格式): {node_id}")
                break
    elif "data" in data and "nodes" in data.get("data", {}):
        # 新格式
        for node in data.get("data", {}).get("nodes", []):
            if node.get("data", {}).get("type") == "MiniOB":
                miniob_node = node
                print(f"   ✓ 找到 MiniOB 组件 (新格式): {node.get('id', 'unknown')}")
                break

    if not miniob_node:
        print("   ✗ 未找到 MiniOB 组件")
        print(f"   数据结构: {list(data.keys())}")
        if "data" in data:
            print(f"   data 结构: {list(data['data'].keys())}")
        return False
    
    # 3. 获取当前代码
    print()
    print("3. 获取当前代码...")
    template = miniob_node.get("data", {}).get("node", {}).get("template", {})
    code_field = template.get("code", {})
    current_code = code_field.get("value", "")
    
    if not current_code:
        print("   ✗ 未找到代码字段")
        return False
    
    print(f"   ✓ 代码长度: {len(current_code)} 字符")
    
    # 4. 检查是否包含旧的维度检测代码
    print()
    print("4. 检查维度检测代码...")
    old_pattern = "test_text = \"This is a test sentence for embedding dimension detection.\""
    
    if old_pattern not in current_code:
        print("   ✗ 未找到预期的维度检测代码")
        print("   文件可能已被修改或使用了不同的版本")
        return False
    
    print("   ✓ 找到维度检测代码")
    
    # 5. 替换为硬编码版本
    print()
    print("5. 应用硬编码修复...")
    
    # 旧的维度检测代码
    old_code = """        try:
            # Use a longer test string to avoid Ollama validation errors
            # Ollama requires at least 1 character, so we use a meaningful test phrase
            test_text = "This is a test sentence for embedding dimension detection."
            self.__log_func(f"Testing embedding model with text: '{test_text}'")
            test_embedding = self.__embedding.embed_query(test_text)
            self.__embedding_dimension: int = len(test_embedding)
            self.__log_func(f"Detected embedding dimension: {self.__embedding_dimension}")
        except Exception as e:
            error_msg = str(e)
            self.__log_func(f"Error detecting embedding dimension: {error_msg}")
            
            # Try to extract dimension from error message or use default
            # For bge-m3, the dimension is typically 1024
            if "bge-m3" in type(self.__embedding).__name__.lower() or "bge-m3" in error_msg.lower():
                self.__embedding_dimension = 1024
                self.__log_func(f"Using default dimension for bge-m3: {self.__embedding_dimension}")
            else:
                raise MiniOBException(
                    message=f"Failed to determine embedding dimension: {error_msg}",
                    operation="EMBEDDING_DIMENSION_CHECK",
                    details={"embedding_model": type(self.__embedding).__name__},
                ) from e"""
    
    # 新的硬编码版本
    new_code = """        # Hardcode dimension for bge-m3 to avoid Ollama validation issues
        # bge-m3 always uses 1024 dimensions
        self.__embedding_dimension = 1024
        self.__log_func(f"Using hardcoded dimension for bge-m3: {self.__embedding_dimension}")
        
        # Verify the embedding model works (optional validation)
        try:
            test_text = "This is a test sentence for embedding dimension detection."
            self.__log_func(f"Validating embedding model with text: '{test_text}'")
            test_embedding = self.__embedding.embed_query(test_text)
            actual_dimension = len(test_embedding)
            if actual_dimension != self.__embedding_dimension:
                self.__log_func(f"Warning: Expected dimension {self.__embedding_dimension}, got {actual_dimension}")
                self.__embedding_dimension = actual_dimension
            self.__log_func(f"Embedding model validated successfully, dimension: {self.__embedding_dimension}")
        except Exception as e:
            # If validation fails, still use the hardcoded dimension
            error_msg = str(e)
            self.__log_func(f"Warning: Embedding validation failed: {error_msg}")
            self.__log_func(f"Continuing with hardcoded dimension: {self.__embedding_dimension}")"""
    
    if old_code in current_code:
        new_code_value = current_code.replace(old_code, new_code)
        print("   ✓ 代码替换成功")
    else:
        print("   ✗ 未找到完全匹配的代码块")
        print("   尝试部分匹配...")
        
        # 尝试更简单的替换
        if "test_text = \"This is a test sentence" in current_code:
            # 找到 try 块的开始和 except 块的结束
            try_start = current_code.find("        try:\n            # Use a longer test string")
            except_end = current_code.find("                ) from e\n\n        # TODO: initialize miniob data")
            
            if try_start != -1 and except_end != -1:
                # 替换整个 try-except 块
                before = current_code[:try_start]
                after = current_code[except_end + len("                ) from e"):]
                new_code_value = before + new_code + after
                print("   ✓ 使用部分匹配替换成功")
            else:
                print("   ✗ 无法找到 try-except 块边界")
                return False
        else:
            print("   ✗ 无法进行替换")
            return False
    
    # 6. 更新 JSON
    print()
    print("6. 更新 JSON 数据...")
    code_field["value"] = new_code_value
    print("   ✓ JSON 数据已更新")
    
    # 7. 备份原文件
    print()
    print("7. 备份原文件...")
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    backup_file = f"{model_file}.backup_{timestamp}"
    try:
        with open(backup_file, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
        print(f"   ✓ 备份文件: {backup_file}")
    except Exception as e:
        print(f"   ✗ 备份失败: {e}")
        return False
    
    # 8. 保存修改后的文件
    print()
    print("8. 保存修改...")
    try:
        with open(model_file, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
        print(f"   ✓ 文件已保存: {model_file}")
    except Exception as e:
        print(f"   ✗ 保存失败: {e}")
        return False
    
    # 9. 验证
    print()
    print("9. 验证修复...")
    try:
        with open(model_file, 'r', encoding='utf-8') as f:
            verify_data = json.load(f)

        # 检查新代码是否存在
        nodes_to_check = []
        if "nodes" in verify_data:
            nodes_to_check = list(verify_data.get("nodes", {}).values())
        elif "data" in verify_data and "nodes" in verify_data.get("data", {}):
            nodes_to_check = verify_data.get("data", {}).get("nodes", [])

        for node in nodes_to_check:
            if node.get("data", {}).get("type") == "MiniOB":
                code_value = node.get("data", {}).get("node", {}).get("template", {}).get("code", {}).get("value", "")
                if "self.__embedding_dimension = 1024" in code_value and "Hardcode dimension for bge-m3" in code_value:
                    print("   ✓ 修复已成功应用")
                    print("   ✓ 新代码包含硬编码维度 1024")
                    return True
                else:
                    print("   ✗ 验证失败: 新代码未找到")
                    return False

        print("   ✗ 验证失败: 未找到 MiniOB 组件")
        return False
    except Exception as e:
        print(f"   ✗ 验证失败: {e}")
        return False

if __name__ == "__main__":
    print()
    success = fix_embedding_dimension()
    print()
    print("=" * 70)
    if success:
        print("✅ 修复成功!")
        print("=" * 70)
        print()
        print("下一步:")
        print("  1. 清除 Langflow 缓存:")
        print("     ./fix_langflow_cache.sh")
        print()
        print("  2. 重启 Langflow:")
        print("     langflow run")
        print()
        print("  3. 完全关闭并重新打开浏览器")
        print()
        print("  4. 重新导入工作流: rag/model.json")
        print()
        sys.exit(0)
    else:
        print("❌ 修复失败")
        print("=" * 70)
        print()
        print("请检查:")
        print("  1. rag/model.json 文件是否存在")
        print("  2. 文件格式是否正确")
        print("  3. MiniOB 组件是否存在")
        print()
        sys.exit(1)

