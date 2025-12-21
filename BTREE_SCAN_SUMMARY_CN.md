# B树扫描优化总结（中文）

## 问题背景

在 MLDR 全文检索性能测试中，火焰图显示 **B 树扫描占据了大量 CPU 时间**。测试查询为：

```sql
SELECT docid_col, MATCH(fulltext_col) AGAINST(text) as _score
FROM items1 
WHERE MATCH(fulltext_col) AGAINST(text)
  AND base_id IN ('base_id_1', 'base_id_2', 'base_id_3')
  AND id < 1000
ORDER BY _score DESC
LIMIT 10
```

## 核心问题分析

### 1. 索引合并效率低（最大问题）

**位置**: `/root/ob/src/sql/das/iter/ob_das_index_merge_iter.cpp`

**问题**:
- 全文索引、base_id 索引、id 范围各自独立扫描 B 树
- 每个索引扫描完后才在内存中做交集/并集
- 大量无效的 B 树扫描和回表操作

**举例**:
```
全文索引扫描 → 10000 个 docid
base_id 索引扫描 → 5000 个 docid  
id < 1000 扫描 → 1000 个 docid

当前做法：扫描 10000 + 5000 + 1000 = 16000 次
优化后：只扫描交集的 100 次（假设交集很小）
```

### 2. B 树迭代器批次太小

**位置**: `/root/ob/src/storage/memtable/mvcc/ob_keybtree.cpp:1144-1176`

**问题**:
- 每次 `scan_batch()` 只扫描很少的行（可能 64 行）
- 频繁的函数调用和上下文切换
- CPU 流水线效率低

**类比**: 就像搬砖，每次只搬 1 块砖，而不是用小推车一次搬 100 块

### 3. 标量过滤太晚

**问题**:
- 全文索引扫描得到所有匹配文档
- 回表获取完整行数据
- 最后才检查 `base_id` 和 `id` 条件

**应该**:
- 在扫描时就过滤掉不满足条件的行
- 减少无效的回表操作

### 4. 缺少提前终止机制

**问题**:
- 查询只需要 TOP 10 结果
- 但会扫描所有匹配的行
- 没有利用分数排序提前停止

## 优化建议（按优先级）

### 🔥 高优先级（立即可做）

#### 1. 增大 B 树扫描批次
```cpp
// 修改 ob_keybtree.cpp
// 从 64 增加到 1024
static const int64_t OPTIMIZED_KV_QUEUE_SIZE = 1024;
```
**预期效果**: 减少 30-40% 函数调用开销

#### 2. 标量过滤前置
```cpp
// 在 B 树扫描循环中立即检查条件
while (btree_iter->get_next(key, value)) {
  // 立即检查 base_id
  if (!base_id_set.contains(key.base_id)) {
    continue;  // 跳过，不回表
  }
  
  // 立即检查 id
  if (key.id >= 1000) {
    continue;  // 跳过，不回表
  }
  
  // 只有通过过滤的才回表
  lookup_main_table(key);
}
```
**预期效果**: 减少 40-60% 无效回表

#### 3. TopN 提前终止
```cpp
// 利用已有的 TopN 堆
if (topn_heap_full && consecutive_low_score_rows > 100) {
  break;  // 提前终止扫描
}
```
**预期效果**: 对 LIMIT 10 查询，减少 50-80% 扫描量

### ⚡ 中优先级（需要一些重构）

#### 4. Bloom Filter 加速
```cpp
// 为 base_id 构建 Bloom Filter
BloomFilter bf;
bf.add("base_id_1");
bf.add("base_id_2");
bf.add("base_id_3");

// 快速过滤
if (!bf.may_contain(current_base_id)) {
  continue;  // 快速跳过
}
```
**预期效果**: 减少 20-30% 精确比较开销

#### 5. Bitmap 索引合并
```cpp
// 将索引结果转为 bitmap
Bitmap fulltext_bitmap = scan_fulltext_index();
Bitmap base_id_bitmap = scan_base_id_index();
Bitmap id_bitmap = scan_id_range();

// 位运算求交集（极快）
Bitmap result = fulltext_bitmap & base_id_bitmap & id_bitmap;

// 只回表 result 中的行
```
**预期效果**: 索引合并提升 5-10 倍

### 🚀 低优先级（需要大规模改动）

#### 6. 并行索引扫描
```cpp
// 多个索引并行扫描
parallel_for(each_index) {
  scan_index();
}
```
**预期效果**: 多核 CPU 上提升 2-3 倍

## 代码位置总结

| 组件 | 文件 | 行数 | 问题 |
|------|------|------|------|
| 索引合并 | `sql/das/iter/ob_das_index_merge_iter.cpp` | 421-441 | 逐行处理，效率低 |
| B树扫描 | `storage/memtable/mvcc/ob_keybtree.cpp` | 1144-1176 | 批次太小 |
| 路径查找 | `storage/memtable/mvcc/ob_keybtree.cpp` | 355-388 | 每次都遍历路径 |
| 全文匹配 | `sql/das/iter/sparse_retrieval/ob_das_match_iter.cpp` | 443-503 | 串行填充堆 |
| TopN 堆 | `sql/das/iter/ob_das_index_merge_iter.cpp` | 226-449 | 已有堆，缺提前终止 |

## 性能预估

基于火焰图显示 B 树扫描占大头：

| 优化项 | 预期提升 | 难度 | 风险 |
|--------|---------|------|------|
| 增大批次 | 30-40% | 低 | 低 |
| 标量过滤前置 | 40-60% | 中 | 中 |
| TopN 提前终止 | 50-80% | 低 | 低 |
| Bloom Filter | 20-30% | 中 | 低 |
| Bitmap 合并 | 5-10x | 高 | 中 |
| 并行扫描 | 2-3x | 高 | 高 |

**综合预期**: 总体性能提升 **2-5 倍**

## 实施建议

### 第一步（1-2 天）
1. 增大 B 树扫描批次
2. TopN 提前终止
3. 添加性能计数器

**目标**: 30-50% 提升

### 第二步（3-5 天）
4. 标量过滤前置
5. Bloom Filter 加速

**目标**: 累计 2-3 倍提升

### 第三步（1-2 周）
6. Bitmap 索引合并
7. 并行索引扫描

**目标**: 累计 3-5 倍提升

## 调试建议

### 1. 添加性能计数器
```cpp
LOG_INFO("Btree scan stats",
         "total_scans", total_scan_count,
         "filtered", filtered_count,
         "filter_rate", filtered_count * 100.0 / total_scan_count,
         "avg_batch_size", avg_batch_size);
```

### 2. 火焰图对比
- 优化前: 记录 B 树扫描占比
- 优化后: 验证占比下降

### 3. 单元测试
- 测试标量过滤正确性
- 测试 TopN 结果一致性
- 测试 Bloom Filter 假阳性率

## 关键洞察

1. **减少扫描量比提高扫描速度更重要**
   - 标量过滤前置可以减少 60% 的扫描
   - 比优化扫描代码效果更好

2. **批量处理是王道**
   - 增大批次减少函数调用
   - CPU 流水线效率更高

3. **提前终止很关键**
   - LIMIT 10 不需要扫描所有行
   - 利用分数排序提前停止

4. **Bitmap 是索引合并的利器**
   - 位运算比行级比较快 10-100 倍
   - 内存占用小

## 总结

B 树扫描占大头的根本原因是：
1. **扫描了太多无用的行**（标量过滤太晚）
2. **扫描效率低**（批次太小）
3. **没有提前终止**（LIMIT 没利用好）

通过上述优化，可以将 B 树扫描开销降低 **60-80%**，整体查询性能提升 **2-5 倍**。

## 参考文档

- 详细分析: `BTREE_SCAN_OPTIMIZATION_ANALYSIS.md`
- 具体建议: `BTREE_OPTIMIZATION_RECOMMENDATIONS.md`
