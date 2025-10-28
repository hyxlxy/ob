#!/usr/bin/env python3
"""
插入测试问题到 MiniOB 数据库
这些问题可以在 Langflow 中直接使用

注意: 由于 MiniOB 的限制,我们将问题作为简单的文本记录存储
"""
import socket
import select
import time
import json

# 测试问题数据
test_questions = [
    "在MySQL模式和Oracle模式下,集群名、租户名和用户名的最大长度分别是多少？",
    "ODP连接数据库时有哪些限制？",
    "单个表的行长度、列数、索引个数的最大限制是多少？",
    "在MySQL模式和Oracle模式下,VARCHAR类型的最大长度分别是多少？",
    "物理备库的使用有哪些具体限制？"
]

def connect_miniob():
    """连接到 MiniOB"""
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.connect('/tmp/miniob.sock')
    sock.setblocking(False)
    
    poller = select.poll()
    poller.register(sock, select.POLLIN | select.POLLPRI | select.POLLHUP | select.POLLERR)
    
    return sock, poller

def send_sql(sock, sql):
    """发送 SQL 命令"""
    data = sql.encode('utf-8') + b'\0'
    total_sent = 0
    while total_sent < len(data):
        try:
            sent = sock.send(data[total_sent:])
            if sent == 0:
                raise Exception("Socket connection broken")
            total_sent += sent
        except BlockingIOError:
            time.sleep(0.01)
            continue

def recv_response(sock, poller, timeout=5.0):
    """接收响应"""
    result = b''
    timeout_ms = int(timeout * 1000)

    while True:
        events = poller.poll(timeout_ms)
        if not events:
            raise Exception(f"Poll timeout after {timeout}s")

        _, event = events[0]
        if event & (select.POLLHUP | select.POLLERR):
            raise Exception("Socket error during polling")

        data = sock.recv(8192)
        if len(data) > 0:
            result += data
            if data.endswith(b'\0'):
                return result[:-1].decode('utf-8', errors='replace').strip()

def exec_sql(sock, poller, sql):
    """执行 SQL 并返回结果"""
    send_sql(sock, sql)
    return recv_response(sock, poller)

def main():
    print("=" * 80)
    print("插入测试问题到 MiniOB")
    print("=" * 80)

    # 连接数据库
    print("\n1. 连接 MiniOB...")
    sock, poller = connect_miniob()
    print("   ✓ 连接成功")

    # 创建测试问题表
    print("\n2. 创建测试问题表...")

    # 先删除旧表
    exec_sql(sock, poller, "DROP TABLE IF EXISTS test_questions;")

    # MiniOB 不支持 PRIMARY KEY AUTO_INCREMENT
    create_table_sql = """
    CREATE TABLE test_questions (
        id INT,
        question TEXT
    );
    """

    result = exec_sql(sock, poller, create_table_sql)
    print(f"   {result}")

    # 插入测试问题
    print("\n3. 插入测试问题...")

    for i, question in enumerate(test_questions, 1):
        # SQL 转义,并移除换行符和制表符 (MiniOB 不支持)
        question_escaped = question.replace("'", "''").replace("\n", " ").replace("\t", " ")

        # 单行 INSERT (MiniOB 批量 INSERT 有 BUG)
        insert_sql = f"INSERT INTO test_questions (id, question) VALUES ({i}, '{question_escaped}');"

        result = exec_sql(sock, poller, insert_sql)
        status = "✓" if "SUCCESS" in result else "✗"
        print(f"   [{i}/{len(test_questions)}] {status} {question[:60]}...")

    # 验证插入
    print("\n4. 验证插入结果...")
    result = exec_sql(sock, poller, "SELECT COUNT(*) FROM test_questions;")
    print(f"   {result}")

    # 显示所有问题
    print("\n5. 查询所有测试问题:")
    print("-" * 80)
    result = exec_sql(sock, poller, "SELECT id, question FROM test_questions;")
    print(result)

    # 关闭连接
    sock.close()

    print("\n" + "=" * 80)
    print("✅ 测试问题已成功插入!")
    print("=" * 80)

    print("\n📝 使用方法:")
    print("在 Langflow 中,你可以直接查询这些问题:")
    print("")
    print("  SELECT question FROM test_questions WHERE id = 1;")
    print("  SELECT question FROM test_questions WHERE id = 2;")
    print("  ...")
    print("")
    print("或者查询所有问题:")
    print("  SELECT * FROM test_questions;")
    print("")

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"\n✗ 错误: {e}")
        import traceback
        traceback.print_exc()

