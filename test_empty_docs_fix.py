#!/usr/bin/env python3
"""
测试空文档过滤修复
直接测试 Ollama embeddings 是否能处理空字符串
"""

try:
    from langchain_community.embeddings import OllamaEmbeddings
except ImportError:
    try:
        from langchain.embeddings import OllamaEmbeddings
    except ImportError:
        print("错误: 无法导入 OllamaEmbeddings")
        print("请安装: pip install langchain-community")
        exit(1)

print("=" * 70)
print("测试 Ollama Embeddings 空字符串问题")
print("=" * 70)
print()

# 初始化 Ollama embeddings
print("1. 初始化 Ollama embeddings...")
try:
    embeddings = OllamaEmbeddings(
        model="bge-m3",
        base_url="http://localhost:11434"
    )
    print("   ✓ 初始化成功")
except Exception as e:
    print(f"   ✗ 初始化失败: {e}")
    exit(1)

# 测试 1: 正常文本
print()
print("2. 测试正常文本...")
try:
    result = embeddings.embed_query("This is a test")
    print(f"   ✓ 成功,维度: {len(result)}")
except Exception as e:
    print(f"   ✗ 失败: {e}")

# 测试 2: 空字符串 (应该失败)
print()
print("3. 测试空字符串...")
try:
    result = embeddings.embed_query("")
    print(f"   ✗ 意外成功,维度: {len(result)}")
except Exception as e:
    print(f"   ✓ 预期失败: {e}")

# 测试 3: 只有空格 (应该失败)
print()
print("4. 测试只有空格...")
try:
    result = embeddings.embed_query("   ")
    print(f"   ✗ 意外成功,维度: {len(result)}")
except Exception as e:
    print(f"   ✓ 预期失败: {e}")

# 测试 4: embed_documents 与空字符串
print()
print("5. 测试 embed_documents 包含空字符串...")
try:
    texts = ["Hello", "", "World"]
    results = embeddings.embed_documents(texts)
    print(f"   ✗ 意外成功,结果数: {len(results)}")
except Exception as e:
    print(f"   ✓ 预期失败: {e}")

# 测试 5: embed_documents 过滤后
print()
print("6. 测试 embed_documents 过滤空字符串后...")
try:
    texts = ["Hello", "", "World", "   ", "Test"]
    # 过滤空字符串
    valid_texts = [t for t in texts if t and t.strip()]
    print(f"   原始文本数: {len(texts)}")
    print(f"   过滤后文本数: {len(valid_texts)}")
    results = embeddings.embed_documents(valid_texts)
    print(f"   ✓ 成功,结果数: {len(results)}")
except Exception as e:
    print(f"   ✗ 失败: {e}")

print()
print("=" * 70)
print("结论:")
print("=" * 70)
print()
print("✓ Ollama embeddings 不接受空字符串")
print("✓ 必须在调用 embed_documents 前过滤空字符串")
print("✓ 过滤条件: text and text.strip()")
print()
print("这就是为什么需要 valid_documents 过滤!")
print()

