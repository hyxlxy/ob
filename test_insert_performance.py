#!/usr/bin/env python3
"""
测试不同批次大小的插入性能
"""
import socket
import select
import time
import json

def test_insert_performance():
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.connect('/tmp/miniob.sock')
    sock.setblocking(False)
    
    poller = select.poll()
    poller.register(sock, select.POLLIN | select.POLLPRI | select.POLLHUP | select.POLLERR)
    
    def send_sql(sql):
        data = sql.encode('utf-8') + b'\0'
        total_sent = 0
        start_time = time.time()
        while total_sent < len(data):
            try:
                sent = sock.send(data[total_sent:])
                total_sent += sent
            except BlockingIOError:
                time.sleep(0.01)
        send_time = time.time() - start_time
        return send_time
    
    def recv_response():
        result = b''
        start_time = time.time()
        while True:
            events = poller.poll(10000)
            if not events:
                return None, 0
            _, event = events[0]
            if event & (select.POLLHUP | select.POLLERR):
                return None, 0
            data = sock.recv(8192)
            if len(data) > 0:
                result += data
                if data.endswith(b'\0'):
                    recv_time = time.time() - start_time
                    return result[:-1].decode('utf-8', errors='replace').strip(), recv_time
    
    print("=" * 80)
    print("插入性能测试")
    print("=" * 80)
    
    # 创建测试表
    print("\n准备: 创建测试表")
    send_sql("DROP TABLE IF EXISTS perf_test;")
    recv_response()
    
    send_sql("CREATE TABLE perf_test (id INT, content TEXT, embedding VECTOR(1024));")
    response, _ = recv_response()
    print(f"创建表: {response}")
    
    # 测试不同批次大小
    batch_sizes = [1, 5, 10, 20, 50]
    total_docs = 50  # 总共插入50个文档
    
    results = []
    
    for batch_size in batch_sizes:
        print(f"\n{'=' * 80}")
        print(f"测试批次大小: {batch_size}")
        print(f"{'=' * 80}")
        
        # 清空表
        send_sql("DELETE FROM perf_test;")
        recv_response()
        
        total_time = 0
        total_send_time = 0
        total_recv_time = 0
        batch_count = 0
        
        # 生成测试数据
        for batch_start in range(0, total_docs, batch_size):
            batch_end = min(batch_start + batch_size, total_docs)
            batch_docs = batch_end - batch_start
            
            # 构建批量INSERT
            values_list = []
            for i in range(batch_start, batch_end):
                content = f"文档内容 {i}"
                # 生成1024维向量
                embedding = [0.001 * i] * 1024
                embedding_str = json.dumps(embedding)
                values_list.append(f"({i}, '{content}', '{embedding_str}')")
            
            insert_sql = f"INSERT INTO perf_test (id, content, embedding) VALUES {', '.join(values_list)};"
            
            # 执行插入
            batch_start_time = time.time()
            send_time = send_sql(insert_sql)
            response, recv_time = recv_response()
            batch_total_time = time.time() - batch_start_time
            
            total_send_time += send_time
            total_recv_time += recv_time
            total_time += batch_total_time
            batch_count += 1
            
            sql_size = len(insert_sql)
            print(f"  批次 {batch_count}: {batch_docs} 文档, SQL大小: {sql_size:,} 字节, "
                  f"发送: {send_time:.3f}s, 接收: {recv_time:.3f}s, 总计: {batch_total_time:.3f}s")
        
        # 统计
        avg_time_per_batch = total_time / batch_count
        avg_time_per_doc = total_time / total_docs
        docs_per_second = total_docs / total_time if total_time > 0 else 0
        
        print(f"\n总结:")
        print(f"  总文档数: {total_docs}")
        print(f"  批次数: {batch_count}")
        print(f"  总时间: {total_time:.3f}s")
        print(f"  平均每批: {avg_time_per_batch:.3f}s")
        print(f"  平均每文档: {avg_time_per_doc:.4f}s")
        print(f"  吞吐量: {docs_per_second:.2f} 文档/秒")
        
        results.append({
            'batch_size': batch_size,
            'total_time': total_time,
            'docs_per_second': docs_per_second,
            'avg_per_doc': avg_time_per_doc
        })
    
    # 清理
    send_sql("DROP TABLE perf_test;")
    recv_response()
    sock.close()
    
    # 性能对比
    print("\n" + "=" * 80)
    print("性能对比总结")
    print("=" * 80)
    print(f"{'批次大小':<10} {'总时间(s)':<12} {'吞吐量(文档/秒)':<20} {'每文档(ms)':<15}")
    print("-" * 80)
    
    for r in results:
        print(f"{r['batch_size']:<10} {r['total_time']:<12.3f} {r['docs_per_second']:<20.2f} {r['avg_per_doc']*1000:<15.2f}")
    
    # 找出最快的
    best = max(results, key=lambda x: x['docs_per_second'])
    print(f"\n✅ 最佳批次大小: {best['batch_size']} (吞吐量: {best['docs_per_second']:.2f} 文档/秒)")
    
    # 当前配置 (batch_size=10)
    current = next((r for r in results if r['batch_size'] == 10), None)
    if current:
        print(f"\n📊 当前配置 (batch_size=10):")
        print(f"   吞吐量: {current['docs_per_second']:.2f} 文档/秒")
        print(f"   插入73个文档预计耗时: {73 / current['docs_per_second']:.2f} 秒")

if __name__ == "__main__":
    test_insert_performance()
