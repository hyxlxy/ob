#!/usr/bin/env python3
"""
将 Langflow 中的 ChatQwen 组件更新为使用 OpenAI 兼容接口

这样可以直接使用阿里云百炼的 API Key,无需安装 dashscope
"""

import json
import sys

# 阿里云百炼配置
DASHSCOPE_API_KEY = "sk-d9059b15e63f42baada7bb92a0e7a322"
DASHSCOPE_BASE_URL = "https://dashscope.aliyuncs.com/compatible-mode/v1"
MODEL_NAME = "qwen-max"

def update_model_json():
    """更新 model.json 中的 ChatQwen 组件配置"""
    
    model_json_path = 'rag/model.json'
    
    print(f"读取文件: {model_json_path}")
    with open(model_json_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    
    # 查找 ChatQwenModel 节点
    qwen_node = None
    for node in data.get('data', {}).get('nodes', []):
        node_id = node.get('data', {}).get('id', '')
        if node_id == 'ChatQwenModel-qvzK9':
            qwen_node = node
            print(f"✓ 找到 ChatQwenModel 节点: {node_id}")
            break
    
    if not qwen_node:
        print("❌ 未找到 ChatQwenModel 节点")
        return False
    
    # 更新节点的模板值
    template = qwen_node['data']['node']['template']
    
    # 更新 API Key
    if 'api_key' in template:
        template['api_key']['value'] = DASHSCOPE_API_KEY
        print(f"✓ 更新 API Key")
    
    # 更新 Base URL
    if 'base_url' in template:
        template['base_url']['value'] = DASHSCOPE_BASE_URL
        print(f"✓ 更新 Base URL: {DASHSCOPE_BASE_URL}")
    
    # 更新 Model Name
    if 'model_name' in template:
        template['model_name']['value'] = MODEL_NAME
        print(f"✓ 更新 Model Name: {MODEL_NAME}")
    
    # 创建备份
    backup_path = model_json_path + '.openai_backup'
    print(f"\n创建备份: {backup_path}")
    with open(backup_path, 'w', encoding='utf-8') as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    
    # 保存修改后的文件
    print(f"保存修改后的文件: {model_json_path}")
    with open(model_json_path, 'w', encoding='utf-8') as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    
    print("\n" + "=" * 80)
    print("✅ 更新完成!")
    print("=" * 80)
    
    print("\n更新内容:")
    print(f"  • API Key: {DASHSCOPE_API_KEY[:20]}...")
    print(f"  • Base URL: {DASHSCOPE_BASE_URL}")
    print(f"  • Model: {MODEL_NAME}")
    
    print("\n注意事项:")
    print("  1. ChatQwen 组件使用 DashScope SDK")
    print("  2. 需要安装: pip install dashscope")
    print("  3. 或者在 Langflow 中添加 ChatOpenAI 组件作为替代")
    
    return True

def create_langflow_instructions():
    """创建 Langflow 配置说明"""
    
    instructions = f"""
# 在 Langflow 中配置 Qwen 模型

## 方法 1: 使用现有的 ChatQwen 组件 (需要 dashscope)

1. 安装依赖:
   ```bash
   pip install dashscope --break-system-packages
   ```

2. 在 Langflow 中打开 ChatQwenModel 组件

3. 配置参数:
   - DashScope API Key: {DASHSCOPE_API_KEY}
   - API Base URL: {DASHSCOPE_BASE_URL}
   - Model Name: {MODEL_NAME}

## 方法 2: 使用 ChatOpenAI 组件 (推荐,无需额外依赖)

1. 在 Langflow 中删除或替换 ChatQwenModel 组件

2. 添加 ChatOpenAI 组件

3. 配置参数:
   - API Key: {DASHSCOPE_API_KEY}
   - Base URL: {DASHSCOPE_BASE_URL}
   - Model: {MODEL_NAME}
   - Temperature: 0.7 (可选)
   - Max Tokens: 1500 (可选)

4. 连接组件:
   - 输入: 从 Prompt 组件连接到 ChatOpenAI 的 input_value
   - 输出: 从 ChatOpenAI 的 text_output 连接到 ChatOutput

## 测试配置

运行以下命令测试 API 连接:
```bash
python3 test_qwen_api.py
```

## 环境变量

配置已保存到 .env.local:
```bash
DASHSCOPE_API_KEY={DASHSCOPE_API_KEY}
DASHSCOPE_BASE_URL={DASHSCOPE_BASE_URL}
QWEN_MODEL_NAME={MODEL_NAME}
```

## 支持的模型

阿里云百炼 OpenAI 兼容接口支持以下模型:
- qwen-max (qwen3-max 的 API 名称)
- qwen-plus
- qwen-turbo
- qwen-long

详细信息: https://help.aliyun.com/zh/model-studio/compatibility-of-openai-with-dashscope
"""
    
    instructions_file = 'LANGFLOW_QWEN_SETUP.md'
    print(f"\n创建配置说明: {instructions_file}")
    with open(instructions_file, 'w', encoding='utf-8') as f:
        f.write(instructions)
    
    print(f"✓ 配置说明已保存: {instructions_file}")

def main():
    print("=" * 80)
    print("更新 Langflow ChatQwen 组件配置")
    print("=" * 80)
    print()
    
    # 更新 model.json
    success = update_model_json()
    
    if success:
        # 创建配置说明
        create_langflow_instructions()
        
        print("\n下一步:")
        print("  1. 查看配置说明: cat LANGFLOW_QWEN_SETUP.md")
        print("  2. 安装 dashscope: pip install dashscope --break-system-packages")
        print("  3. 或在 Langflow 中使用 ChatOpenAI 组件替代")
        print("  4. 在 Langflow 中刷新页面或重新导入 rag/model.json")
        print("  5. 测试 API: python3 test_qwen_api.py")

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\n\n更新已取消")
        sys.exit(1)
    except Exception as e:
        print(f"\n更新过程中出错: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

