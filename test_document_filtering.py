#!/usr/bin/env python3
"""
测试文档过滤逻辑
"""

from langchain_core.documents import Document

print("=" * 70)
print("测试文档过滤逻辑")
print("=" * 70)
print()

# 模拟各种类型的文档
test_documents = [
    Document(page_content="This is a valid document", metadata={}),
    Document(page_content="", metadata={}),  # 空字符串
    Document(page_content="   ", metadata={}),  # 只有空格
    Document(page_content="\n\n\n", metadata={}),  # 只有换行
    Document(page_content="\t\t", metadata={}),  # 只有制表符
    Document(page_content="  \n  \t  ", metadata={}),  # 混合空白字符
    Document(page_content="Another valid document", metadata={}),
    Document(page_content="Valid with spaces  ", metadata={}),
]

print(f"测试文档总数: {len(test_documents)}")
print()

# 应用过滤逻辑
valid_documents = []
empty_count = 0

for i, doc in enumerate(test_documents):
    if hasattr(doc, 'page_content') and doc.page_content is not None:
        content = str(doc.page_content).strip()
        if content and len(content) > 0:
            valid_documents.append(doc)
            print(f"✓ 文档 {i}: 有效 - '{content[:50]}'")
        else:
            empty_count += 1
            print(f"✗ 文档 {i}: 空内容(strip后) - 原始长度: {len(str(doc.page_content))}")
    else:
        empty_count += 1
        print(f"✗ 文档 {i}: 无 page_content 或为 None")

print()
print(f"有效文档: {len(valid_documents)}")
print(f"空文档: {empty_count}")
print()

# 提取 page_contents
page_contents = []
final_valid_documents = []

for doc in valid_documents:
    content = str(doc.page_content).strip()
    if content and len(content) > 0:
        page_contents.append(content)
        final_valid_documents.append(doc)

print(f"最终 page_contents 数量: {len(page_contents)}")
print()

# 验证所有 page_contents
print("验证 page_contents:")
invalid_count = 0
for i, content in enumerate(page_contents):
    if not content or not isinstance(content, str) or len(content.strip()) == 0:
        print(f"  ✗ 索引 {i}: 无效 - type={type(content)}, len={len(content) if content else 0}")
        invalid_count += 1
    else:
        print(f"  ✓ 索引 {i}: 有效 - len={len(content)}, preview='{content[:30]}...'")

print()
if invalid_count == 0:
    print("✅ 所有 page_contents 都是有效的非空字符串!")
else:
    print(f"❌ 发现 {invalid_count} 个无效的 page_contents")

print()
print("=" * 70)

