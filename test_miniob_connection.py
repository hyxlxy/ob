#!/usr/bin/env python3
"""
MiniOB 连接测试脚本
用于验证 MiniOB 数据库连接和基本操作
"""

import socket
import select
import time
import json
import os
import sys


class MiniOBTester:
    def __init__(self, server_socket=None, server_address='127.0.0.1', server_port=6789):
        self.server_socket = server_socket or os.getenv('MINIOB_SERVER_SOCKET', '')
        self.server_address = server_address
        self.server_port = server_port
        self.socket = None
        self.poller = None
        
    def connect(self):
        """连接到 MiniOB"""
        print("=" * 80)
        print("步骤 1: 连接 MiniOB")
        print("=" * 80)
        
        try:
            if self.server_socket:
                print(f"使用 Unix Socket: {self.server_socket}")
                self.socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
                self.socket.connect(self.server_socket)
            else:
                print(f"使用 TCP 连接: {self.server_address}:{self.server_port}")
                self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.socket.connect((self.server_address, self.server_port))
            
            self.socket.setblocking(False)
            
            self.poller = select.poll()
            self.poller.register(
                self.socket,
                select.POLLIN | select.POLLPRI | select.POLLHUP | select.POLLERR
            )
            
            print("✓ 连接成功")
            return True
            
        except Exception as e:
            print(f"✗ 连接失败: {e}")
            return False
    
    def send_sql(self, sql):
        """发送 SQL 命令"""
        try:
            data = sql.encode('utf-8')
            data_with_null = data + b'\0'
            
            total_sent = 0
            while total_sent < len(data_with_null):
                try:
                    sent = self.socket.send(data_with_null[total_sent:])
                    if sent == 0:
                        raise Exception("Socket connection broken")
                    total_sent += sent
                except BlockingIOError:
                    time.sleep(0.01)
                    continue
            
            print(f"  → 发送 SQL ({len(data_with_null)} bytes)")
            return True
            
        except Exception as e:
            print(f"  ✗ 发送失败: {e}")
            return False
    
    def recv_response(self, timeout=5.0):
        """接收响应"""
        try:
            result = b''
            timeout_ms = int(timeout * 1000)
            
            while True:
                events = self.poller.poll(timeout_ms)
                if not events:
                    raise Exception(f"Poll timeout after {timeout}s")
                
                _, event = events[0]
                if event & (select.POLLHUP | select.POLLERR):
                    raise Exception("Socket error during polling")
                
                data = self.socket.recv(8192)
                if len(data) > 0:
                    result += data
                    if data.endswith(b'\0'):
                        result = result[:-1]
                        decoded = result.decode('utf-8', errors='replace')
                        print(f"  ← 收到响应 ({len(result)} bytes)")
                        return decoded.strip()
                else:
                    raise Exception("Received zero length data")
            
        except Exception as e:
            print(f"  ✗ 接收失败: {e}")
            return None
    
    def exec_sql(self, sql, description=""):
        """执行 SQL 并返回结果"""
        if description:
            print(f"\n{description}")
        print(f"SQL: {sql[:100]}..." if len(sql) > 100 else f"SQL: {sql}")
        
        if not self.send_sql(sql):
            return None
        
        result = self.recv_response()
        if result:
            # 检查是否有错误
            result_lower = result.lower()
            if result_lower.startswith('error') or 'failed' in result_lower:
                print(f"  ✗ SQL 执行错误: {result}")
                return None
            print(f"  ✓ 执行成功")
            if len(result) < 200:
                print(f"  结果: {result}")
            else:
                print(f"  结果: {result[:200]}...")
        return result
    
    def test_basic_operations(self):
        """测试基本操作"""
        print("\n" + "=" * 80)
        print("步骤 2: 测试基本 SQL 操作")
        print("=" * 80)
        
        # 测试 SHOW TABLES
        self.exec_sql("SHOW TABLES;", "2.1 查看表列表")
        
        return True
    
    def test_vector_table(self):
        """测试向量表操作"""
        print("\n" + "=" * 80)
        print("步骤 3: 测试向量表操作")
        print("=" * 80)
        
        # 创建测试表
        create_sql = """
        CREATE TABLE IF NOT EXISTS test_rag_vectors (
            id INT PRIMARY KEY AUTO_INCREMENT,
            content TEXT,
            metadata TEXT,
            embedding VECTOR(1024)
        );
        """
        
        if not self.exec_sql(create_sql, "3.1 创建向量表"):
            return False
        
        # 插入测试数据
        test_embedding = [0.1] * 1024
        embedding_str = json.dumps(test_embedding)
        
        insert_sql = f"""
        INSERT INTO test_rag_vectors (content, metadata, embedding)
        VALUES ('测试文档内容', '{{"source": "test"}}', '{embedding_str}');
        """
        
        if not self.exec_sql(insert_sql, "3.2 插入测试向量"):
            return False
        
        # 查询数据
        select_sql = "SELECT id, content FROM test_rag_vectors LIMIT 5;"
        if not self.exec_sql(select_sql, "3.3 查询数据"):
            return False
        
        # 向量搜索
        search_sql = f"""
        SELECT id, content, L2_DISTANCE(embedding, '{embedding_str}') as distance
        FROM test_rag_vectors
        ORDER BY distance
        LIMIT 3;
        """
        
        if not self.exec_sql(search_sql, "3.4 向量相似度搜索"):
            return False
        
        # 清理测试表
        drop_sql = "DROP TABLE test_rag_vectors;"
        self.exec_sql(drop_sql, "3.5 清理测试表")
        
        return True
    
    def test_batch_insert(self):
        """测试批量插入"""
        print("\n" + "=" * 80)
        print("步骤 4: 测试批量插入")
        print("=" * 80)
        
        # 创建测试表
        create_sql = """
        CREATE TABLE IF NOT EXISTS test_batch_insert (
            id INT PRIMARY KEY AUTO_INCREMENT,
            content TEXT,
            embedding VECTOR(128)
        );
        """
        
        if not self.exec_sql(create_sql, "4.1 创建测试表"):
            return False
        
        # 批量插入 10 条数据
        values_list = []
        for i in range(10):
            content = f"文档 {i}"
            embedding = [0.1 * i] * 128
            embedding_str = json.dumps(embedding)
            values_list.append(f"('{content}', '{embedding_str}')")
        
        batch_insert_sql = f"""
        INSERT INTO test_batch_insert (content, embedding)
        VALUES {', '.join(values_list)};
        """
        
        print(f"4.2 批量插入 10 条数据")
        print(f"SQL 长度: {len(batch_insert_sql)} 字符")
        
        if not self.exec_sql(batch_insert_sql, ""):
            return False
        
        # 验证插入
        count_sql = "SELECT COUNT(*) FROM test_batch_insert;"
        self.exec_sql(count_sql, "4.3 验证插入数量")
        
        # 清理
        drop_sql = "DROP TABLE test_batch_insert;"
        self.exec_sql(drop_sql, "4.4 清理测试表")
        
        return True
    
    def close(self):
        """关闭连接"""
        if self.socket:
            self.socket.close()
            print("\n✓ 连接已关闭")
    
    def run_all_tests(self):
        """运行所有测试"""
        print("\n" + "=" * 80)
        print("MiniOB 连接和功能测试")
        print("=" * 80)
        
        if not self.connect():
            print("\n✗ 测试失败: 无法连接到 MiniOB")
            return False
        
        try:
            if not self.test_basic_operations():
                print("\n✗ 基本操作测试失败")
                return False
            
            if not self.test_vector_table():
                print("\n✗ 向量表测试失败")
                return False
            
            if not self.test_batch_insert():
                print("\n✗ 批量插入测试失败")
                return False
            
            print("\n" + "=" * 80)
            print("✅ 所有测试通过!")
            print("=" * 80)
            print("\nMiniOB 已准备好用于 RAG 工作流")
            return True
            
        except Exception as e:
            print(f"\n✗ 测试过程中出错: {e}")
            import traceback
            traceback.print_exc()
            return False
        
        finally:
            self.close()


def main():
    """主函数"""
    print("MiniOB 连接测试工具")
    print("=" * 80)
    
    # 从环境变量或命令行参数获取配置
    server_socket = os.getenv('MINIOB_SERVER_SOCKET', '')
    server_address = os.getenv('MINIOB_SERVER_ADDRESS', '127.0.0.1')
    server_port = int(os.getenv('MINIOB_SERVER_PORT', '6789'))
    
    print(f"配置:")
    if server_socket:
        print(f"  Socket: {server_socket}")
    else:
        print(f"  地址: {server_address}:{server_port}")
    
    tester = MiniOBTester(
        server_socket=server_socket,
        server_address=server_address,
        server_port=server_port
    )
    
    success = tester.run_all_tests()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()

