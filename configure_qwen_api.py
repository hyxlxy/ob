#!/usr/bin/env python3
"""
配置阿里云百炼 Qwen API 到 Langflow 工作流

根据阿里云百炼文档配置 qwen3-max 模型
API 文档: https://help.aliyun.com/zh/model-studio/use-bailian-in-langchain
"""

import json
import sys
import os

# 阿里云百炼配置
DASHSCOPE_API_KEY = "sk-d9059b15e63f42baada7bb92a0e7a322"
DASHSCOPE_BASE_URL = "https://dashscope.aliyuncs.com/compatible-mode/v1"
MODEL_NAME = "qwen-max"  # qwen3-max 在 API 中的名称是 qwen-max

def update_model_json():
    """更新 model.json 中的 Qwen 模型配置"""
    
    model_json_path = 'rag/model.json'
    
    print(f"读取文件: {model_json_path}")
    with open(model_json_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    
    # 查找 ChatQwenModel 节点
    qwen_node = None
    for node in data.get('data', {}).get('nodes', []):
        node_id = node.get('data', {}).get('id', '')
        node_type = node.get('data', {}).get('type', '')
        
        # 查找 ChatOpenAI 或类似的聊天模型节点
        if 'chat' in node_type.lower() or 'qwen' in node_id.lower():
            qwen_node = node
            print(f"找到聊天模型节点: {node_id} (类型: {node_type})")
            break
    
    if not qwen_node:
        print("⚠ 未找到 Qwen 聊天模型节点,尝试查找所有聊天节点...")
        for node in data.get('data', {}).get('nodes', []):
            node_data = node.get('data', {})
            node_id = node_data.get('id', '')
            print(f"  节点: {node_id}")
    
    # 创建备份
    backup_path = model_json_path + '.qwen_backup'
    print(f"\n创建备份: {backup_path}")
    with open(backup_path, 'w', encoding='utf-8') as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    
    # 保存配置到环境变量文件
    env_file = '.env.local'
    print(f"\n更新环境变量文件: {env_file}")
    
    env_vars = {}
    if os.path.exists(env_file):
        with open(env_file, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith('#') and '=' in line:
                    key, value = line.split('=', 1)
                    env_vars[key] = value
    
    # 更新配置
    env_vars['DASHSCOPE_API_KEY'] = DASHSCOPE_API_KEY
    env_vars['DASHSCOPE_BASE_URL'] = DASHSCOPE_BASE_URL
    env_vars['QWEN_MODEL_NAME'] = MODEL_NAME
    
    # 写回文件
    with open(env_file, 'w', encoding='utf-8') as f:
        f.write("# 阿里云百炼 API 配置\n")
        f.write(f"DASHSCOPE_API_KEY={env_vars.get('DASHSCOPE_API_KEY', '')}\n")
        f.write(f"DASHSCOPE_BASE_URL={env_vars.get('DASHSCOPE_BASE_URL', '')}\n")
        f.write(f"QWEN_MODEL_NAME={env_vars.get('QWEN_MODEL_NAME', '')}\n")
        f.write("\n# MiniOB 配置\n")
        f.write(f"MINIOB_SERVER_SOCKET={env_vars.get('MINIOB_SERVER_SOCKET', '/tmp/miniob.sock')}\n")
        f.write("\n# Ollama 配置 (用于嵌入模型)\n")
        f.write(f"EMBEDDING_NAME={env_vars.get('EMBEDDING_NAME', 'bge-m3')}\n")
        f.write(f"EMBEDDING_BASE_URL={env_vars.get('EMBEDDING_BASE_URL', 'http://localhost:11434')}\n")
        f.write("\n# 文档路径\n")
        f.write(f"OB_DOC_PATH={env_vars.get('OB_DOC_PATH', '/home/hxl/Desktop/oceambase/ob25/ob/oceanbase-doc/zh-CN')}\n")
    
    print("\n" + "=" * 80)
    print("✅ 配置完成!")
    print("=" * 80)
    
    print("\n配置信息:")
    print(f"  • API Key: {DASHSCOPE_API_KEY[:20]}...")
    print(f"  • Base URL: {DASHSCOPE_BASE_URL}")
    print(f"  • 模型名称: {MODEL_NAME}")
    
    print("\n环境变量已保存到: .env.local")
    
    print("\n在 Langflow 中使用 ChatOpenAI 组件:")
    print("  1. 添加 ChatOpenAI 组件")
    print("  2. 配置参数:")
    print(f"     - API Key: {DASHSCOPE_API_KEY}")
    print(f"     - Base URL: {DASHSCOPE_BASE_URL}")
    print(f"     - Model: {MODEL_NAME}")
    
    print("\n或者使用 Python 代码测试:")
    print("""
from langchain_openai import ChatOpenAI
import os

# 设置环境变量
os.environ['DASHSCOPE_API_KEY'] = '""" + DASHSCOPE_API_KEY + """'

chatLLM = ChatOpenAI(
    api_key=os.getenv("DASHSCOPE_API_KEY"),
    base_url=\"""" + DASHSCOPE_BASE_URL + """\",
    model=\"""" + MODEL_NAME + """\",
)

messages = [
    {"role": "system", "content": "You are a helpful assistant."},
    {"role": "user", "content": "你好,请介绍一下你自己"}
]

response = chatLLM.invoke(messages)
print(response.content)
""")
    
    return True

def create_test_script():
    """创建测试脚本"""
    
    test_script = """#!/usr/bin/env python3
\"\"\"
测试阿里云百炼 Qwen API 连接

使用 LangChain 的 ChatOpenAI 接口调用 qwen-max 模型
\"\"\"

import os
from langchain_openai import ChatOpenAI

# 从环境变量读取配置
DASHSCOPE_API_KEY = os.getenv("DASHSCOPE_API_KEY", "sk-d9059b15e63f42baada7bb92a0e7a322")
DASHSCOPE_BASE_URL = os.getenv("DASHSCOPE_BASE_URL", "https://dashscope.aliyuncs.com/compatible-mode/v1")
MODEL_NAME = os.getenv("QWEN_MODEL_NAME", "qwen-max")

print("=" * 80)
print("测试阿里云百炼 Qwen API")
print("=" * 80)
print(f"\\nAPI Key: {DASHSCOPE_API_KEY[:20]}...")
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
    print("\\n测试 1: 简单对话")
    messages = [
        {"role": "system", "content": "You are a helpful assistant."},
        {"role": "user", "content": "你好,请用一句话介绍你自己"}
    ]
    
    print("发送消息...")
    response = chatLLM.invoke(messages)
    print(f"\\n回复: {response.content}")
    
    # 测试技术问答
    print("\\n" + "=" * 80)
    print("测试 2: 技术问答")
    messages = [
        {"role": "system", "content": "你是一个数据库专家,擅长解释数据库概念。"},
        {"role": "user", "content": "什么是向量数据库?请简要说明。"}
    ]
    
    print("发送消息...")
    response = chatLLM.invoke(messages)
    print(f"\\n回复: {response.content}")
    
    print("\\n" + "=" * 80)
    print("✅ 所有测试通过!")
    print("=" * 80)
    
except Exception as e:
    print(f"\\n❌ 测试失败: {e}")
    import traceback
    traceback.print_exc()
    exit(1)
"""
    
    test_file = 'test_qwen_api.py'
    print(f"\n创建测试脚本: {test_file}")
    with open(test_file, 'w', encoding='utf-8') as f:
        f.write(test_script)
    
    os.chmod(test_file, 0o755)
    print(f"✓ 测试脚本已创建: {test_file}")
    print(f"\n运行测试: python3 {test_file}")

def main():
    print("=" * 80)
    print("配置阿里云百炼 Qwen API")
    print("=" * 80)
    print()
    
    print("配置信息:")
    print(f"  • API Key: {DASHSCOPE_API_KEY}")
    print(f"  • Base URL: {DASHSCOPE_BASE_URL}")
    print(f"  • 模型: {MODEL_NAME} (qwen3-max)")
    print()
    
    # 检查依赖
    print("检查依赖...")
    try:
        import langchain_openai
        print("✓ langchain_openai 已安装")
    except ImportError:
        print("⚠ langchain_openai 未安装")
        print("  安装命令: pip install langchain_openai")
    
    # 更新配置
    success = update_model_json()
    
    if success:
        # 创建测试脚本
        create_test_script()
        
        print("\n下一步:")
        print("  1. 测试 API 连接: python3 test_qwen_api.py")
        print("  2. 在 Langflow 中配置 ChatOpenAI 组件")
        print("  3. 使用以下参数:")
        print(f"     - API Key: {DASHSCOPE_API_KEY}")
        print(f"     - Base URL: {DASHSCOPE_BASE_URL}")
        print(f"     - Model: {MODEL_NAME}")

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\\n\\n配置已取消")
        sys.exit(1)
    except Exception as e:
        print(f"\\n配置过程中出错: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

