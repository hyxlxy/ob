#!/usr/bin/env python3
"""
测试阿里云百炼 Qwen API 连接

使用 LangChain 的 ChatOpenAI 接口调用 qwen-max 模型
"""

import os
from langchain_openai import ChatOpenAI

# 从环境变量读取配置
DASHSCOPE_API_KEY = os.getenv("DASHSCOPE_API_KEY", "sk-d9059b15e63f42baada7bb92a0e7a322")
DASHSCOPE_BASE_URL = os.getenv("DASHSCOPE_BASE_URL", "https://dashscope.aliyuncs.com/compatible-mode/v1")
MODEL_NAME = os.getenv("QWEN_MODEL_NAME", "qwen-max")

print("=" * 80)
print("测试阿里云百炼 Qwen API")
print("=" * 80)
print(f"\nAPI Key: {DASHSCOPE_API_KEY[:20]}...")
print(f"Base URL: {DASHSCOPE_BASE_URL}")
print(f"Model: {MODEL_NAME}")
print()

try:
    # 创建聊天模型
    chatLLM = ChatOpenAI(
        api_key=DASHSCOPE_API_KEY,
        base_url=DASHSCOPE_BASE_URL,
        model=MODEL_NAME,
        temperature=0.7,
    )
    
    print("✓ ChatOpenAI 模型已创建")
    
    # 测试简单对话
    print("\n测试 1: 简单对话")
    messages = [
        {"role": "system", "content": "You are a helpful assistant."},
        {"role": "user", "content": "你好,请用一句话介绍你自己"}
    ]
    
    print("发送消息...")
    response = chatLLM.invoke(messages)
    print(f"\n回复: {response.content}")
    
    # 测试技术问答
    print("\n" + "=" * 80)
    print("测试 2: 技术问答")
    messages = [
        {"role": "system", "content": "你是一个数据库专家,擅长解释数据库概念。"},
        {"role": "user", "content": "什么是向量数据库?请简要说明。"}
    ]
    
    print("发送消息...")
    response = chatLLM.invoke(messages)
    print(f"\n回复: {response.content}")
    
    print("\n" + "=" * 80)
    print("✅ 所有测试通过!")
    print("=" * 80)
    
except Exception as e:
    print(f"\n❌ 测试失败: {e}")
    import traceback
    traceback.print_exc()
    exit(1)
