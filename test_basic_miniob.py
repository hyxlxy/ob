#!/usr/bin/env python3
import socket
import select
import time

def test_miniob():
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
        print(f"→ {sql}")
    
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
    
    # 测试基本命令
    tests = [
        "SHOW TABLES;",
        "CREATE TABLE test_simple (id INT, name TEXT);",
        "INSERT INTO test_simple VALUES (1, 'hello');",
        "SELECT * FROM test_simple;",
        "DROP TABLE test_simple;",
    ]
    
    for sql in tests:
        send_sql(sql)
        response = recv_response()
        print(f"← {response}\n")
    
    sock.close()

if __name__ == "__main__":
    test_miniob()
