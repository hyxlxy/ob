#!/usr/bin/env python3
"""
修复空文档问题 - 过滤掉空的 page_content
在生成嵌入之前过滤掉空文档
"""

import json
import sys
from datetime import datetime

def fix_empty_documents():
    model_file = "rag/model.json"
    
    print("=" * 70)
    print("修复空文档问题 - 过滤空 page_content")
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
    
    if "data" in data and "nodes" in data.get("data", {}):
        for node in data.get("data", {}).get("nodes", []):
            if node.get("data", {}).get("type") == "MiniOB":
                miniob_node = node
                print(f"   ✓ 找到 MiniOB 组件: {node.get('id', 'unknown')}")
                break
    
    if not miniob_node:
        print("   ✗ 未找到 MiniOB 组件")
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
    
    # 4. 检查是否需要修复
    print()
    print("4. 检查 add_documents 方法...")
    
    if "page_contents: List[str] = [document.page_content for document in documents]" not in current_code:
        print("   ✗ 未找到预期的代码")
        return False
    
    print("   ✓ 找到需要修复的代码")
    
    # 5. 应用修复
    print()
    print("5. 应用修复...")
    
    # 旧代码
    old_code = """        self.__log_func(f"Processing {len(documents)} documents for insertion")
        page_contents: List[str] = [document.page_content for document in documents]
        self.__log_func("Generating embeddings for documents...")
        try:
            embeddings: List[List[float]] = self.__embedding.embed_documents(
                page_contents
            )
        except Exception as e:
            raise MiniOBException(
                message=f"Failed to generate embeddings for documents: {str(e)}",
                operation="GENERATE_EMBEDDINGS",
                details={"document_count": len(page_contents)},
            ) from e"""
    
    # 新代码 - 过滤空文档
    new_code = """        self.__log_func(f"Processing {len(documents)} documents for insertion")
        
        # Filter out documents with empty page_content to avoid Ollama validation errors
        valid_documents = []
        for i, doc in enumerate(documents):
            if doc.page_content and doc.page_content.strip():
                valid_documents.append(doc)
            else:
                self.__log_func(f"Warning: Skipping document {i} with empty page_content")
        
        if not valid_documents:
            self.__log_func("Warning: No valid documents to insert after filtering")
            return []
        
        self.__log_func(f"Filtered to {len(valid_documents)} valid documents (removed {len(documents) - len(valid_documents)} empty documents)")
        
        page_contents: List[str] = [document.page_content for document in valid_documents]
        self.__log_func("Generating embeddings for documents...")
        try:
            embeddings: List[List[float]] = self.__embedding.embed_documents(
                page_contents
            )
        except Exception as e:
            raise MiniOBException(
                message=f"Failed to generate embeddings for documents: {str(e)}",
                operation="GENERATE_EMBEDDINGS",
                details={"document_count": len(page_contents)},
            ) from e"""
    
    if old_code in current_code:
        new_code_value = current_code.replace(old_code, new_code)
        print("   ✓ 代码替换成功")
    else:
        print("   ✗ 未找到完全匹配的代码块")
        return False
    
    # 还需要更新后续使用 documents 的地方为 valid_documents
    print()
    print("6. 更新文档引用...")
    
    # 在 add_documents 方法中,后续使用 documents 的地方需要改为 valid_documents
    old_ref1 = "for batch_start in range(0, len(documents), batch_size):"
    new_ref1 = "for batch_start in range(0, len(valid_documents), batch_size):"
    
    old_ref2 = "batch_end = min(batch_start + batch_size, len(documents))"
    new_ref2 = "batch_end = min(batch_start + batch_size, len(valid_documents))"
    
    old_ref3 = "batch_documents = documents[batch_start:batch_end]"
    new_ref3 = "batch_documents = valid_documents[batch_start:batch_end]"
    
    old_ref4 = 'self.__log_func(f"Successfully inserted {len(inserted_ids)} documents in {(len(documents) + batch_size - 1) // batch_size} batches")'
    new_ref4 = 'self.__log_func(f"Successfully inserted {len(inserted_ids)} documents in {(len(valid_documents) + batch_size - 1) // batch_size} batches")'
    
    old_ref5 = 'details={"document_count": len(documents), "inserted_count": len(inserted_ids)},'
    new_ref5 = 'details={"document_count": len(valid_documents), "inserted_count": len(inserted_ids)},'
    
    replacements = [
        (old_ref1, new_ref1),
        (old_ref2, new_ref2),
        (old_ref3, new_ref3),
        (old_ref4, new_ref4),
        (old_ref5, new_ref5),
    ]
    
    for old, new in replacements:
        if old in new_code_value:
            new_code_value = new_code_value.replace(old, new)
            print(f"   ✓ 已更新引用")
        else:
            print(f"   ⚠ 未找到引用: {old[:50]}...")
    
    # 7. 更新 JSON
    print()
    print("7. 更新 JSON 数据...")
    code_field["value"] = new_code_value
    print("   ✓ JSON 数据已更新")
    
    # 8. 备份原文件
    print()
    print("8. 备份原文件...")
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    backup_file = f"{model_file}.backup_{timestamp}"
    try:
        with open(backup_file, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
        print(f"   ✓ 备份文件: {backup_file}")
    except Exception as e:
        print(f"   ✗ 备份失败: {e}")
        return False
    
    # 9. 保存修改后的文件
    print()
    print("9. 保存修改...")
    try:
        with open(model_file, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
        print(f"   ✓ 文件已保存: {model_file}")
    except Exception as e:
        print(f"   ✗ 保存失败: {e}")
        return False
    
    # 10. 验证
    print()
    print("10. 验证修复...")
    try:
        with open(model_file, 'r', encoding='utf-8') as f:
            verify_data = json.load(f)
        
        for node in verify_data.get("data", {}).get("nodes", []):
            if node.get("data", {}).get("type") == "MiniOB":
                code_value = node.get("data", {}).get("node", {}).get("template", {}).get("code", {}).get("value", "")
                if "valid_documents = []" in code_value and "Filter out documents with empty page_content" in code_value:
                    print("   ✓ 修复已成功应用")
                    print("   ✓ 新代码包含空文档过滤逻辑")
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
    success = fix_empty_documents()
    print()
    print("=" * 70)
    if success:
        print("✅ 修复成功!")
        print("=" * 70)
        print()
        print("修复内容:")
        print("  - 在生成嵌入之前过滤掉空文档")
        print("  - 只处理有内容的文档")
        print("  - 记录被跳过的空文档数量")
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
        print("注意:")
        print("  - 你有 62,519 个文档,这是一个很大的数量")
        print("  - 建议先测试少量文档")
        print("  - 检查文本分割设置(当前产生了很多超长块)")
        print()
        sys.exit(0)
    else:
        print("❌ 修复失败")
        print("=" * 70)
        print()
        sys.exit(1)

