#!/usr/bin/env python3
"""
测试已插入的数据

验证 OceanBase 文档是否成功插入到 MiniOB
"""

import socket
import select

MINIOB_SOCKET = "/tmp/miniob.sock"

def execute_sql(sql: str) -> str:
    """执行 SQL 并返回结果"""
    try:
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.connect(MINIOB_SOCKET)
        
        # 发送 SQL
        sock.sendall((sql + '\0').encode('utf-8'))
        
        # 接收响应
        response = b''
        while True:
            ready = select.select([sock], [], [], 5.0)
            if ready[0]:
                chunk = sock.recv(4096)
                if not chunk:
                    break
                response += chunk
                if b'\0' in chunk:
                    break
            else:
                break
        
        sock.close()
        return response.decode('utf-8').rstrip('\0')
    except Exception as e:
        return f"ERROR: {e}"

def main():
    print("=" * 80)
    print("测试已插入的数据")
    print("=" * 80)
    
    # 测试查询
    queries = [
        ("查看表结构", "SHOW TABLES;"),
        ("统计记录数", "SELECT COUNT(*) FROM ob_docs;"),
        ("查看前 3 条记录", "SELECT id, SUBSTR(content, 1, 50) as content_preview, SUBSTR(metadata, 1, 100) as metadata FROM ob_docs LIMIT 3;"),
        ("查看所有 ID", "SELECT id FROM ob_docs ORDER BY id;"),
    ]
    
    for title, sql in queries:
        print(f"\n{title}:")
        print(f"SQL: {sql}")
        print("-" * 80)
        result = execute_sql(sql)
        print(result)
        print()

if __name__ == '__main__':
    main()

