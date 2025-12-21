# B树扫描优化分析报告

## 背景
在 MLDR 全文检索测试中，火焰图显示 B 树扫描占据大量 CPU 时间。查询场景为：
```sql
SELECT docid_col, MATCH(fulltext_col) AGAINST(text) as _score
FROM items1 
WHERE MATCH(fulltext_col) AGAINST(text)
  and base_id in ('base_id_1', 'base_id_2', 'base_id_3')
  and id < 1000
ORDER BY _score DESC
LIMIT 10
```

## 核心问题分析

### 1. **索引合并（Index Merge）的低效性**

**位置**: `/root/ob/src/sql/das/iter/ob_das_index_merge_iter.cpp`

**问题点**:
- **行级存储开销大**: 每次合并都需要完整存储行数据
  ```cpp
  // 第 53-79 行：IndexMergeRowStore::save()
  // 为每一行分配 LastDASStoreRow，包含完整的列数据
  if (OB_FAIL(store_rows_[i].save_store_row(*exprs_, *eval_ctx_))) {
    LOG_WARN("index merge iter failed to store rows", K(ret));
  }
  ```

- **重复的 B 树扫描**: 
  - 全文索引扫描得到 docid 列表
  - 标量索引（base_id, id）各自独立扫描
  - 最后在 Index Merge 层做交集/并集
  - **每个索引都要完整扫描 B 树，没有提前终止机制**

- **缺少批量优化**: 
  ```cpp
  // 第 421-441 行：do_one_merge_round()
  // 每次只处理一行，没有批量处理
  int ObDASIndexMergeIter::do_one_merge_round(int64_t &count)
  ```

### 2. **B树迭代器的通用性开销**

**位置**: `/root/ob/src/storage/memtable/mvcc/ob_keybtree.cpp`

**问题点**:

- **路径查找开销** (第 355-388 行):
  ```cpp
  template<typename BtreeKey, typename BtreeVal>
  int ScanHandle<BtreeKey, BtreeVal>::find_path(BtreeNode *root, BtreeKey key)
  {
    // 每次查找都要遍历整个路径
    while (OB_NOT_NULL(root) && OB_SUCCESS == ret) {
      root->prefetch();  // CPU prefetch
      if (OB_FAIL(root->find_pos(this->get_comp(), key, is_found, pos, index))) {
        break;
      }
      // ... 继续向下查找
    }
  }
  ```

- **扫描批次小** (第 1144-1176 行):
  ```cpp
  int BtreeIterator<BtreeKey, BtreeVal>::scan_batch()
  {
    // kv_queue_ 的批次大小可能不够大
    while (OB_SUCCESS == ret) {
      if (OB_SUCCESS != kv_queue_.push(item)) {
        break;  // 队列满了就停止
      }
    }
  }
  ```

### 3. **全文索引扫描的串行化**

**位置**: `/root/ob/src/sql/das/iter/sparse_retrieval/ob_das_match_iter.cpp`

**问题点**:

- **逐行填充堆** (第 443-503 行):
  ```cpp
  int ObDASMatchIter::fill_merge_heap()
  {
    for (int64_t i = 0; OB_SUCC(ret) && i < next_round_cnt_; ++i) {
      // 每个子迭代器逐个调用 get_next_row()
      if (OB_FAIL(dim_iter->get_next_row())) {
        // ...
      }
      // 计算相关性分数
      // 推入堆
    }
  }
  ```

- **分数计算与扫描耦合**: 每次 B 树扫描后立即计算分数，无法批量处理

### 4. **标量过滤的后置执行**

**当前流程**:
1. 全文索引扫描 → 得到所有匹配文档的 docid
2. 回表查询 → 通过 docid 获取完整行（包括 base_id, id）
3. 标量过滤 → 在回表后才应用 `base_id in (...)` 和 `id < 1000`

**问题**: 
- 大量无效的 B 树扫描和回表操作
- 标量条件应该在全文索引扫描时就过滤

## 优化建议

### 优化方向 1: **Bitmap 索引合并**

**原理**: 将多个索引的结果转换为 bitmap，使用位运算快速求交集/并集

```cpp
// 伪代码
class ObBitmapIndexMerge {
  // 1. 全文索引扫描 → bitmap1
  Bitmap fulltext_bitmap;
  
  // 2. base_id 索引扫描 → bitmap2  
  Bitmap base_id_bitmap;
  
  // 3. id 范围扫描 → bitmap3
  Bitmap id_range_bitmap;
  
  // 4. 位运算求交集（极快）
  Bitmap result = fulltext_bitmap & base_id_bitmap & id_range_bitmap;
  
  // 5. 只回表 result 中的行
};
```

**优点**:
- 减少 B 树扫描次数
- 位运算比行级比较快 10-100 倍
- 内存占用小（1 bit per row）

### 优化方向 2: **标量过滤前置到全文索引扫描**

**修改位置**: `/root/ob/src/sql/das/iter/sparse_retrieval/ob_das_tr_merge_iter.cpp`

```cpp
// 在全文索引扫描时就应用标量过滤
int ObDASTRMergeIter::get_next_row_with_filter() {
  while (true) {
    // 1. 从倒排索引获取下一个 docid
    if (OB_FAIL(get_next_docid(docid))) break;
    
    // 2. 立即检查标量条件（避免回表）
    if (!check_scalar_filter(docid)) continue;
    
    // 3. 只有通过过滤的才回表
    if (OB_FAIL(lookup_main_table(docid))) break;
    
    return OB_SUCCESS;
  }
}
```

**关键**: 需要在倒排索引中存储或快速查询 base_id 和 id

### 优化方向 3: **批量 B 树扫描**

**修改位置**: `/root/ob/src/storage/memtable/mvcc/ob_keybtree.cpp`

```cpp
// 增加批量扫描接口
template<typename BtreeKey, typename BtreeVal>
int BtreeIterator<BtreeKey, BtreeVal>::scan_batch_optimized(
    int64_t batch_size,  // 期望的批次大小
    BtreeKey keys[],     // 输出：批量的 key
    BtreeVal values[],   // 输出：批量的 value
    int64_t &actual_size // 实际返回的数量
) {
  // 1. 预取多个节点（减少 cache miss）
  // 2. 批量解码 key/value
  // 3. 减少函数调用开销
}
```

### 优化方向 4: **TopN 下推优化**（已部分实现）

**当前代码**: `/root/ob/src/sql/das/iter/ob_das_index_merge_iter.cpp` (第 226-449 行)

**已有的 TopN 堆优化**:
```cpp
int ObDASIndexMergeIter::MergeResultBuffer::add_row_with_topn() {
  // 使用最小堆维护 Top-N 结果
  if (topn_heap_->count() < topn_limit_) {
    topn_heap_->push(new_row);
  } else if (new_row->score_ > heap_top->score_) {
    topn_heap_->pop();
    topn_heap_->push(new_row);
  }
}
```

**进一步优化**:
- **提前终止**: 当堆满且当前分数低于堆顶时，可以跳过后续扫描
- **分数阈值**: 动态计算最小可接受分数，提前过滤

### 优化方向 5: **B 树节点缓存优化**

**问题**: 重复访问相同的 B 树节点

**解决方案**:
```cpp
// 在 ObQueryEngine 中增加节点缓存
class ObBtreeNodeCache {
  LRUCache<NodeKey, BtreeNode*> cache_;
  
  BtreeNode* get_or_load(NodeKey key) {
    if (cache_.contains(key)) {
      return cache_.get(key);  // 缓存命中
    }
    // 缓存未命中，加载节点
    BtreeNode* node = load_from_disk(key);
    cache_.put(key, node);
    return node;
  }
};
```

### 优化方向 6: **并行扫描**

对于多个独立的索引扫描，可以并行执行：

```cpp
// 伪代码
void parallel_index_scan() {
  std::vector<std::future<Bitmap>> futures;
  
  // 并行扫描全文索引
  futures.push_back(async(scan_fulltext_index));
  
  // 并行扫描 base_id 索引
  futures.push_back(async(scan_base_id_index));
  
  // 并行扫描 id 范围
  futures.push_back(async(scan_id_range));
  
  // 等待所有扫描完成
  for (auto& f : futures) {
    bitmaps.push_back(f.get());
  }
  
  // 合并结果
  Bitmap result = merge_bitmaps(bitmaps);
}
```

## 优先级排序

### 高优先级（立即可实施）:
1. **标量过滤前置** - 减少无效回表
2. **批量 B 树扫描** - 减少函数调用开销
3. **TopN 提前终止** - 利用已有的堆结构

### 中优先级（需要一定重构）:
4. **Bitmap 索引合并** - 需要修改索引合并逻辑
5. **B 树节点缓存** - 需要增加缓存层

### 低优先级（需要大规模改动）:
6. **并行扫描** - 需要线程池和任务调度

## 性能预估

基于火焰图显示 B 树扫描占大头的情况：

- **标量过滤前置**: 预计减少 30-50% 的 B 树扫描
- **批量扫描优化**: 预计减少 20-30% 的函数调用开销
- **Bitmap 合并**: 预计减少 40-60% 的索引合并时间
- **TopN 提前终止**: 对于 LIMIT 小的查询，预计减少 50-70% 的扫描量

**综合预期**: 总体性能提升 2-5 倍

## 实施建议

1. **先做性能分析**: 使用 perf 确认具体的热点函数
2. **小步快跑**: 优先实施高优先级、低风险的优化
3. **基准测试**: 每个优化都要有对比测试
4. **回归测试**: 确保优化不影响正确性

## 相关代码位置总结

| 组件 | 文件路径 | 关键函数 |
|------|---------|---------|
| 索引合并 | `/root/ob/src/sql/das/iter/ob_das_index_merge_iter.cpp` | `do_one_merge_round()` |
| B树扫描 | `/root/ob/src/storage/memtable/mvcc/ob_keybtree.cpp` | `scan_batch()`, `find_path()` |
| 全文匹配 | `/root/ob/src/sql/das/iter/sparse_retrieval/ob_das_match_iter.cpp` | `fill_merge_heap()` |
| 查询引擎 | `/root/ob/src/storage/memtable/mvcc/ob_query_engine.cpp` | `scan()` |
| 表扫描 | `/root/ob/src/storage/access/ob_table_scan_iterator.cpp` | `open_iter()` |
