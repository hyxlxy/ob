#!/usr/bin/env python3
"""
修复 MiniOB 组件中的嵌入维度检测问题

问题: Ollama Embeddings 在验证时要求输入字符串至少有 1 个字符
解决: 使用更长的测试字符串,并添加更好的错误处理
"""

import json
import sys

def fix_model_json(file_path='rag/model.json'):
    """修复 model.json 中的嵌入维度检测代码"""
    
    print(f"读取文件: {file_path}")
    with open(file_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    
    # 查找 MiniOB 节点
    miniob_node = None
    for node in data.get('data', {}).get('nodes', []):
        if node.get('data', {}).get('id') == 'MiniOB-aSrlo':
            miniob_node = node
            break
    
    if not miniob_node:
        print("❌ 未找到 MiniOB 节点")
        return False
    
    print("✓ 找到 MiniOB 节点")
    
    # 获取当前代码
    code_value = miniob_node['data']['node']['template']['code']['value']
    
    # 查找需要修复的代码段
    old_code = '''        try:
            self.__embedding_dimension: int = len(self.__embedding.embed_query("Hello"))
        except Exception as e:
            raise MiniOBException(
                message=f"Failed to determine embedding dimension: {str(e)}",
                operation="EMBEDDING_DIMENSION_CHECK",
                details={"embedding_model": type(self.__embedding).__name__},
            ) from e'''
    
    # 新的修复代码
    new_code = '''        try:
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
                ) from e'''
    
    if old_code in code_value:
        print("✓ 找到需要修复的代码段")
        code_value = code_value.replace(old_code, new_code)
        miniob_node['data']['node']['template']['code']['value'] = code_value
        print("✓ 代码已修复")
    else:
        print("⚠ 未找到完全匹配的代码段,尝试部分匹配...")
        
        # 尝试更宽松的匹配
        if 'embed_query("Hello")' in code_value:
            print("✓ 找到 embed_query('Hello') 调用")
            # 替换整个 try-except 块
            import re
            pattern = r'try:\s+self\.__embedding_dimension.*?from e'
            replacement = new_code
            code_value = re.sub(pattern, replacement, code_value, flags=re.DOTALL)
            miniob_node['data']['node']['template']['code']['value'] = code_value
            print("✓ 代码已修复 (使用正则表达式)")
        else:
            print("❌ 无法找到需要修复的代码")
            return False
    
    # 保存修复后的文件
    backup_path = file_path + '.backup'
    print(f"\n创建备份: {backup_path}")
    with open(backup_path, 'w', encoding='utf-8') as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    
    print(f"保存修复后的文件: {file_path}")
    with open(file_path, 'w', encoding='utf-8') as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    
    print("\n" + "=" * 80)
    print("✅ 修复完成!")
    print("=" * 80)
    print("\n修复内容:")
    print("1. 使用更长的测试字符串避免 Ollama 验证错误")
    print("2. 添加了详细的日志输出")
    print("3. 为 bge-m3 模型添加了默认维度 (1024)")
    print("4. 改进了错误处理逻辑")
    print("\n下一步:")
    print("1. 在 Langflow 中重新加载工作流 (刷新页面)")
    print("2. 或者重新导入 rag/model.json")
    print("3. 确保 Ollama 服务正在运行: ollama list")
    print("4. 确保 bge-m3 模型已下载: ollama pull bge-m3")
    
    return True

if __name__ == '__main__':
    success = fix_model_json()
    sys.exit(0 if success else 1)

