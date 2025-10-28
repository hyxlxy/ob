#!/usr/bin/env python3
"""
探索 MiniOB 数据库支持的功能
"""
import socket
import select
import time

def connect_miniob():
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.connect('/tmp/miniob.sock')
    sock.setblocking(False)
    
    poller = select.poll()
    poller.register(sock, select.POLLIN | select.POLLPRI | select.POLLHUP | select.POLLERR)
    
    return sock, poller

def send_sql(sock, sql):
    data = sql.encode('utf-8') + b'\0'
    total_sent = 0
    while total_sent < len(data):
        try:
            sent = sock.send(data[total_sent:])
            total_sent += sent
        except BlockingIOError:
            time.sleep(0.01)

def recv_response(sock, poller):
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

def test_sql(sock, poller, description, sql):
    print(f"\n{description}")
    print(f"SQL: {sql}")
    send_sql(sock, sql)
    response = recv_response(sock, poller)
    print(f"结果: {response}")
    return response

print("=" * 80)
print("探索 MiniOB 数据库功能")
print("=" * 80)

sock, poller = connect_miniob()

# 1. 查看现有表
print("\n" + "=" * 80)
print("1. 查看现有表")
print("=" * 80)
test_sql(sock, poller, "显示所有表", "SHOW TABLES;")

# 2. 测试基本数据类型
print("\n" + "=" * 80)
print("2. 测试支持的数据类型")
print("=" * 80)

test_sql(sock, poller, "删除测试表", "DROP TABLE IF EXISTS type_test;")

# INT
response = test_sql(sock, poller, "测试 INT 类型", 
    "CREATE TABLE type_test (id INT);")

if "SUCCESS" in response:
    test_sql(sock, poller, "删除", "DROP TABLE type_test;")
    
# TEXT
response = test_sql(sock, poller, "测试 TEXT 类型", 
    "CREATE TABLE type_test (content TEXT);")

if "SUCCESS" in response:
    test_sql(sock, poller, "删除", "DROP TABLE type_test;")

# VARCHAR
response = test_sql(sock, poller, "测试 VARCHAR 类型", 
    "CREATE TABLE type_test (name VARCHAR(100));")

if "SUCCESS" in response:
    test_sql(sock, poller, "删除", "DROP TABLE type_test;")

# FLOAT
response = test_sql(sock, poller, "测试 FLOAT 类型", 
    "CREATE TABLE type_test (value FLOAT);")

if "SUCCESS" in response:
    test_sql(sock, poller, "删除", "DROP TABLE type_test;")

# DATE
response = test_sql(sock, poller, "测试 DATE 类型", 
    "CREATE TABLE type_test (dt DATE);")

if "SUCCESS" in response:
    test_sql(sock, poller, "删除", "DROP TABLE type_test;")

# 3. 测试约束
print("\n" + "=" * 80)
print("3. 测试约束支持")
print("=" * 80)

# PRIMARY KEY
response = test_sql(sock, poller, "测试 PRIMARY KEY", 
    "CREATE TABLE type_test (id INT PRIMARY KEY);")

if "SUCCESS" in response:
    test_sql(sock, poller, "删除", "DROP TABLE type_test;")

# AUTO_INCREMENT
response = test_sql(sock, poller, "测试 AUTO_INCREMENT", 
    "CREATE TABLE type_test (id INT PRIMARY KEY AUTO_INCREMENT);")

if "SUCCESS" in response:
    test_sql(sock, poller, "删除", "DROP TABLE type_test;")

# NOT NULL
response = test_sql(sock, poller, "测试 NOT NULL", 
    "CREATE TABLE type_test (id INT NOT NULL);")

if "SUCCESS" in response:
    test_sql(sock, poller, "删除", "DROP TABLE type_test;")

# UNIQUE
response = test_sql(sock, poller, "测试 UNIQUE", 
    "CREATE TABLE type_test (id INT UNIQUE);")

if "SUCCESS" in response:
    test_sql(sock, poller, "删除", "DROP TABLE type_test;")

# 4. 测试 TEXT 字段的限制
print("\n" + "=" * 80)
print("4. 测试 TEXT 字段")
print("=" * 80)

test_sql(sock, poller, "创建 TEXT 表", 
    "CREATE TABLE text_test (id INT, content TEXT);")

# 插入短文本
test_sql(sock, poller, "插入短文本", 
    "INSERT INTO text_test VALUES (1, 'Hello World');")

# 插入长文本
long_text = "这是一段很长的文本。" * 50
escaped_text = long_text.replace("'", "''")
test_sql(sock, poller, f"插入长文本 ({len(long_text)} 字符)", 
    f"INSERT INTO text_test VALUES (2, '{escaped_text}');")

# 插入包含特殊字符的文本
special_text = "包含特殊字符: 单引号', 双引号\", 换行\n, 制表符\t"
escaped_special = special_text.replace("'", "''")
test_sql(sock, poller, "插入特殊字符", 
    f"INSERT INTO text_test VALUES (3, '{escaped_special}');")

# 查询
test_sql(sock, poller, "查询所有数据", 
    "SELECT id, content FROM text_test;")

test_sql(sock, poller, "删除测试表", "DROP TABLE text_test;")

# 5. 测试 IF NOT EXISTS
print("\n" + "=" * 80)
print("5. 测试 IF NOT EXISTS")
print("=" * 80)

test_sql(sock, poller, "CREATE TABLE IF NOT EXISTS", 
    "CREATE TABLE IF NOT EXISTS test_exists (id INT);")

test_sql(sock, poller, "再次创建同名表", 
    "CREATE TABLE IF NOT EXISTS test_exists (id INT);")

test_sql(sock, poller, "删除", "DROP TABLE test_exists;")

# 6. 测试 DROP TABLE IF EXISTS
print("\n" + "=" * 80)
print("6. 测试 DROP TABLE IF EXISTS")
print("=" * 80)

test_sql(sock, poller, "DROP TABLE IF EXISTS (表不存在)", 
    "DROP TABLE IF EXISTS non_existent_table;")

# 7. 测试 DELETE
print("\n" + "=" * 80)
print("7. 测试 DELETE")
print("=" * 80)

test_sql(sock, poller, "创建表", 
    "CREATE TABLE delete_test (id INT, name TEXT);")

test_sql(sock, poller, "插入数据", 
    "INSERT INTO delete_test VALUES (1, 'A'), (2, 'B'), (3, 'C');")

test_sql(sock, poller, "查询", 
    "SELECT * FROM delete_test;")

test_sql(sock, poller, "DELETE WHERE", 
    "DELETE FROM delete_test WHERE id = 2;")

test_sql(sock, poller, "查询删除后", 
    "SELECT * FROM delete_test;")

test_sql(sock, poller, "DELETE 全部", 
    "DELETE FROM delete_test;")

test_sql(sock, poller, "查询清空后", 
    "SELECT * FROM delete_test;")

test_sql(sock, poller, "删除表", "DROP TABLE delete_test;")

# 8. 查看现有的 ad_views 表结构
print("\n" + "=" * 80)
print("8. 查看现有表 ad_views 的结构")
print("=" * 80)

test_sql(sock, poller, "DESC ad_views", "DESC ad_views;")
test_sql(sock, poller, "DESCRIBE ad_views", "DESCRIBE ad_views;")
test_sql(sock, poller, "SHOW CREATE TABLE ad_views", "SHOW CREATE TABLE ad_views;")

# 9. 查询 ad_views 的数据
test_sql(sock, poller, "查询 ad_views 数据", "SELECT * FROM ad_views LIMIT 5;")

sock.close()

print("\n" + "=" * 80)
print("探索完成!")
print("=" * 80)
