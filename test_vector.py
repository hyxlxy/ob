#!/usr/bin/env python3
import socket
import select
import time
import json

def test_vector():
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.connect('/tmp/miniob.sock')
    sock.setblocking(False)
    
    poller = select.poll()
    poller.register(sock, select.POLLIN | select.POLLPRI | select.POLLHUP | select.POLLERR)
    
    def send_sql(sql):
        data = sql.encode('utf-8') + b'\0'
        total_sent = 0
        while total_sent < len(data):
            try:
                sent = sock.send(data[total_sent:])
                total_sent += sent
            except BlockingIOError:
                time.sleep(0.01)
        print(f"→ SQL ({len(data)} bytes)")
        if len(sql) < 200:
            print(f"  {sql}")
        else:
            print(f"  {sql[:200]}...")
    
    def recv_response():
        result = b''
        while True:
            events = poller.poll(5000)
            if not events:
                return "TIMEOUT"
            _, event = events[0]
            if event & (select.POLLHUP | select.POLLERR):
                return "ERROR"
            data = sock.recv(8192)
            if len(data) > 0:
                result += data
                if data.endswith(b'\0'):
                    return result[:-1].decode('utf-8', errors='replace').strip()
    
    print("=" * 80)
    print("测试向量功能")
    print("=" * 80)
    
    # 测试 1: 创建向量表
    print("\n1. 创建向量表 (VECTOR(10))")
    send_sql("CREATE TABLE test_vec (id INT, vec VECTOR(10));")
    response = recv_response()
    print(f"← {response}\n")
    
    if "SUCCESS" in response or "success" in response.lower():
        # 测试 2: 插入向量数据
        print("2. 插入向量数据")
        vec = json.dumps([0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0])
        send_sql(f"INSERT INTO test_vec VALUES (1, '{vec}');")
        response = recv_response()
        print(f"← {response}\n")
        
        # 测试 3: 查询向量
        print("3. 查询向量")
        send_sql("SELECT * FROM test_vec;")
        response = recv_response()
        print(f"← {response}\n")
        
        # 测试 4: L2_DISTANCE
        print("4. 测试 L2_DISTANCE")
        query_vec = json.dumps([0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0])
        send_sql(f"SELECT id, L2_DISTANCE(vec, '{query_vec}') as dist FROM test_vec;")
        response = recv_response()
        print(f"← {response}\n")
        
        # 清理
        print("5. 清理")
        send_sql("DROP TABLE test_vec;")
        response = recv_response()
        print(f"← {response}\n")
    else:
        print("✗ 创建向量表失败,可能不支持 VECTOR 类型")
    
    sock.close()

if __name__ == "__main__":
    test_vector()
