#!/usr/bin/env python3
"""
诊断 model.json 工作流的问题
检查所有可能导致远程测试失败的原因
"""

import json
import sys

def diagnose_workflow():
    print("=" * 80)
    print("诊断 model.json 工作流")
    print("=" * 80)
    
    try:
        with open('rag/model.json', 'r', encoding='utf-8') as f:
            data = json.load(f)
    except Exception as e:
        print(f"\n❌ 无法读取 model.json: {e}")
        return False
    
    issues = []
    warnings = []
    
    # 检查所有节点
    nodes = data.get('data', {}).get('nodes', [])
    edges = data.get('data', {}).get('edges', [])
    
    print(f"\n找到 {len(nodes)} 个组件, {len(edges)} 个连接")
    
    # 1. 检查 MiniOB 组件
    print("\n" + "=" * 80)
    print("1. 检查 MiniOB 组件")
    print("=" * 80)
    
    miniob_found = False
    for node in nodes:
        if not isinstance(node, dict) or 'data' not in node:
            continue
        
        display_name = node.get('data', {}).get('node', {}).get('display_name', '')
        if display_name == 'MiniOB':
            miniob_found = True
            template = node['data']['node']['template']
            code = template.get('code', {}).get('value', '')
            
            print(f"✓ 找到 MiniOB 组件 (ID: {node.get('id')})")
            
            # 检查代码中的关键部分
            checks = {
                'table_name 设置': 'self.__table_name = "rag_documents"' in code,
                '环境变量支持': 'os.getenv("MINIOB_SERVER_SOCKET")' in code,
                'L2_DISTANCE 使用': 'L2_DISTANCE' in code,
                'VECTOR 类型': 'VECTOR(' in code,
                '单行 INSERT': 'INSERT INTO' in code and 'VALUES' in code,
                '异常处理': 'try:' in code and 'except' in code,
            }
            
            for check_name, result in checks.items():
                if result:
                    print(f"  ✓ {check_name}")
                else:
                    issues.append(f"MiniOB 代码缺少: {check_name}")
                    print(f"  ✗ {check_name}")
            
            # 检查配置
            server_socket = template.get('server_socket', {}).get('value', '')
            print(f"\n  配置:")
            print(f"    server_socket = '{server_socket}'")
            
            if server_socket != '/tmp/miniob.sock':
                warnings.append(f"MiniOB server_socket = '{server_socket}', 建议 '/tmp/miniob.sock'")
    
    if not miniob_found:
        issues.append("未找到 MiniOB 组件")
        print("✗ 未找到 MiniOB 组件")
    
    # 2. 检查 Ollama Embeddings 组件
    print("\n" + "=" * 80)
    print("2. 检查 Ollama Embeddings 组件")
    print("=" * 80)
    
    ollama_found = False
    for node in nodes:
        if not isinstance(node, dict) or 'data' not in node:
            continue
        
        display_name = node.get('data', {}).get('node', {}).get('display_name', '')
        if display_name == 'Ollama Embeddings':
            ollama_found = True
            template = node['data']['node']['template']
            
            model_name = template.get('model_name', {}).get('value', '')
            base_url = template.get('base_url', {}).get('value', '')
            
            print(f"✓ 找到 Ollama Embeddings 组件 (ID: {node.get('id')})")
            print(f"  model_name = '{model_name}'")
            print(f"  base_url = '{base_url}'")
            
            if model_name != '':
                issues.append(f"Ollama Embeddings model_name 不为空: '{model_name}'")
                print(f"  ✗ model_name 应该为空 (当前: '{model_name}')")
            else:
                print(f"  ✓ model_name 为空 (会使用环境变量)")
            
            if base_url != '':
                issues.append(f"Ollama Embeddings base_url 不为空: '{base_url}'")
                print(f"  ✗ base_url 应该为空 (当前: '{base_url}')")
            else:
                print(f"  ✓ base_url 为空 (会使用环境变量)")
    
    if not ollama_found:
        issues.append("未找到 Ollama Embeddings 组件")
        print("✗ 未找到 Ollama Embeddings 组件")
    
    # 3. 检查 Directory 组件
    print("\n" + "=" * 80)
    print("3. 检查 Directory 组件")
    print("=" * 80)
    
    directory_found = False
    for node in nodes:
        if not isinstance(node, dict) or 'data' not in node:
            continue
        
        display_name = node.get('data', {}).get('node', {}).get('display_name', '')
        if display_name == 'Directory':
            directory_found = True
            template = node['data']['node']['template']
            
            path = template.get('path', {}).get('value', '')
            
            print(f"✓ 找到 Directory 组件 (ID: {node.get('id')})")
            print(f"  path = '{path}'")
            
            if path == '':
                print(f"  ✓ path 为空 (会使用环境变量 OB_DOC_PATH)")
            elif 'OB_DOC_PATH' in path or path.startswith('$'):
                print(f"  ✓ path 使用环境变量")
            else:
                issues.append(f"Directory path 硬编码: '{path}'")
                print(f"  ✗ path 硬编码,远程测试会失败")
    
    if not directory_found:
        issues.append("未找到 Directory 组件")
        print("✗ 未找到 Directory 组件")
    
    # 4. 检查 Qwen/LLM 组件
    print("\n" + "=" * 80)
    print("4. 检查 Qwen/LLM 组件")
    print("=" * 80)
    
    qwen_found = False
    for node in nodes:
        if not isinstance(node, dict) or 'data' not in node:
            continue
        
        display_name = node.get('data', {}).get('node', {}).get('display_name', '')
        if 'Qwen' in display_name or 'ChatQwen' in display_name:
            qwen_found = True
            template = node['data']['node']['template']
            
            model_name = template.get('model_name', {}).get('value', '')
            api_key = template.get('qwen_api_key', {}).get('value', '')
            base_url = template.get('qwen_api_base', {}).get('value', '')
            
            print(f"✓ 找到 {display_name} 组件 (ID: {node.get('id')})")
            print(f"  model_name = '{model_name}'")
            print(f"  api_key = {'(已设置)' if api_key else '(空)'}")
            print(f"  base_url = '{base_url}'")
            
            if model_name == '' and api_key == '' and base_url == '':
                print(f"  ✓ 所有配置为空 (会使用环境变量)")
            else:
                if model_name != '':
                    warnings.append(f"Qwen model_name 不为空: '{model_name}'")
                if api_key != '':
                    warnings.append(f"Qwen api_key 已设置")
                if base_url != '':
                    warnings.append(f"Qwen base_url 不为空: '{base_url}'")
    
    if not qwen_found:
        warnings.append("未找到 Qwen/LLM 组件")
        print("⚠️  未找到 Qwen/LLM 组件")
    
    # 5. 检查关键连接
    print("\n" + "=" * 80)
    print("5. 检查组件连接")
    print("=" * 80)
    
    required_connections = {
        'Directory → Split Text': False,
        'Split Text → MiniOB (ingest_data)': False,
        'Ollama Embeddings → MiniOB (embedding_model)': False,
        'Chat Input → MiniOB (search_query)': False,
        'MiniOB → Parser': False,
    }
    
    for edge in edges:
        if 'data' not in edge:
            continue
        
        source = edge['data'].get('sourceHandle', {})
        target = edge['data'].get('targetHandle', {})
        
        source_id = source.get('id', '')
        target_id = target.get('id', '')
        target_field = target.get('fieldName', '')
        
        # 检查关键连接
        if 'Directory' in source_id and 'SplitText' in target_id:
            required_connections['Directory → Split Text'] = True
        elif 'SplitText' in source_id and 'MiniOB' in target_id and target_field == 'ingest_data':
            required_connections['Split Text → MiniOB (ingest_data)'] = True
        elif 'OllamaEmbeddings' in source_id and 'MiniOB' in target_id and target_field == 'embedding_model':
            required_connections['Ollama Embeddings → MiniOB (embedding_model)'] = True
        elif 'ChatInput' in source_id and 'MiniOB' in target_id and target_field == 'search_query':
            required_connections['Chat Input → MiniOB (search_query)'] = True
        elif 'MiniOB' in source_id and 'parser' in target_id:
            required_connections['MiniOB → Parser'] = True
    
    for conn, exists in required_connections.items():
        if exists:
            print(f"  ✓ {conn}")
        else:
            issues.append(f"缺少连接: {conn}")
            print(f"  ✗ {conn}")
    
    # 6. 检查 JSON 结构
    print("\n" + "=" * 80)
    print("6. 检查 JSON 结构")
    print("=" * 80)
    
    if 'data' not in data:
        issues.append("JSON 缺少 'data' 字段")
        print("✗ 缺少 'data' 字段")
    else:
        print("✓ 'data' 字段存在")
    
    if 'nodes' not in data.get('data', {}):
        issues.append("JSON 缺少 'nodes' 字段")
        print("✗ 缺少 'nodes' 字段")
    else:
        print(f"✓ 'nodes' 字段存在 ({len(nodes)} 个节点)")
    
    if 'edges' not in data.get('data', {}):
        issues.append("JSON 缺少 'edges' 字段")
        print("✗ 缺少 'edges' 字段")
    else:
        print(f"✓ 'edges' 字段存在 ({len(edges)} 个连接)")
    
    # 输出总结
    print("\n" + "=" * 80)
    print("诊断结果总结")
    print("=" * 80)
    
    if issues:
        print(f"\n❌ 发现 {len(issues)} 个严重问题:")
        for i, issue in enumerate(issues, 1):
            print(f"  {i}. {issue}")
        print("\n这些问题会导致远程测试失败,必须修复!")
    else:
        print("\n✅ 没有发现严重问题")
    
    if warnings:
        print(f"\n⚠️  发现 {len(warnings)} 个警告:")
        for i, warning in enumerate(warnings, 1):
            print(f"  {i}. {warning}")
        print("\n这些警告可能不影响远程测试,但建议检查")
    else:
        print("\n✅ 没有警告")
    
    # 给出建议
    print("\n" + "=" * 80)
    print("建议")
    print("=" * 80)
    
    if not issues:
        print("\n✅ 配置看起来正确!")
        print("\n如果远程测试仍然失败 (返回 000),可能的原因:")
        print("  1. MiniOB 代码有运行时错误")
        print("  2. 文档路径不存在或无法访问")
        print("  3. Embedding 服务连接失败")
        print("  4. LLM 服务连接失败")
        print("  5. 工作流执行超时")
        print("\n建议:")
        print("  - 查看远程测试的详细日志")
        print("  - 在本地测试 MiniOB 的所有 SQL 功能")
        print("  - 检查 MiniOB 是否支持 1024 维向量")
    else:
        print("\n⚠️  请先修复上述严重问题,然后重新测试")
    
    return len(issues) == 0

if __name__ == '__main__':
    success = diagnose_workflow()
    sys.exit(0 if success else 1)

