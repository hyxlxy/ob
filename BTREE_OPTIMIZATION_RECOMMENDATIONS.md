# B树扫描优化具体建议

## 核心发现

通过分析 OceanBase/seekdb 的代码，发现以下关键问题导致 B 树扫描成为性能瓶颈：

### 1. 索引合并效率低下

**问题根源**: `/root/ob/src/sql/das/iter/ob_das_index_merge_iter.cpp`

当前实现的问题：
- 每个索引独立扫描，没有提前过滤
- 使用行级存储（`LastDASStoreRow`），内存开销大
- 逐行处理，批量优化不足

### 2. B树迭代器批次太小

**问题根源**: `/root/ob/src/storage/memtable/mvcc/ob_keybtree.cpp:1144-1176`

```cpp
int BtreeIterator<BtreeKey, BtreeVal>::scan_batch()
{
  // kv_queue_ 队列满了就停止，批次可能很小
  while (OB_SUCCESS == ret) {
    if (OB_SUCCESS != kv_queue_.push(item)) {
      break;  // 队列满，停止扫描
    }
  }
}
```

**影响**: 频繁的函数调用和上下文切换

### 3. 缺少预取优化

虽然存在 `ObIndexTreePrefetcher` 类（`/root/ob/src/storage/access/ob_index_tree_prefetcher.h`），但在全文索引场景下可能没有充分利用。

## 立即可实施的优化

### 优化 1: 增大 B 树扫描批次

**文件**: `/root/ob/src/storage/memtable/mvcc/ob_keybtree.cpp`

**修改点**: 增加 `kv_queue_` 的容量

```cpp
// 在 BtreeIterator 初始化时
static const int64_t DEFAULT_KV_QUEUE_SIZE = 256;  // 当前可能是 64 或更小
static const int64_t OPTIMIZED_KV_QUEUE_SIZE = 1024; // 增加到 1024

// 修改 scan_batch() 函数
int BtreeIterator<BtreeKey, BtreeVal>::scan_batch()
{
  int ret = OB_SUCCESS;
  if (is_iter_end_) {
    ret = OB_ITER_END;
  } else {
    BtreeKV item;
    int64_t batch_count = 0;
    const int64_t target_batch_size = OPTIMIZED_KV_QUEUE_SIZE;
    
    // 尽量填满队列
    while (OB_SUCCESS == ret && batch_count < target_batch_size) {
      if (OB_FAIL(iter_->get_next(item.key_, item.val_))) {
        is_iter_end_ = true;
        break;
      } else if (OB_SUCCESS != kv_queue_.push(item)) {
        break;
      }
      batch_count++;
    }
  }
  return ret;
}
```

**预期效果**: 减少 30-40% 的函数调用开销

### 优化 2: 标量过滤前置

**文件**: `/root/ob/src/sql/das/iter/ob_das_index_merge_iter.cpp`

**核心思路**: 在索引合并前就应用标量过滤

```cpp
// 在 union_get_next_row() 或 intersect_get_next_row() 中添加早期过滤

int ObDASIndexMergeIter::union_get_next_row_optimized()
{
  int ret = OB_SUCCESS;
  bool found = false;
  
  while (OB_SUCC(ret) && !found) {
    // 1. 从堆中获取下一个候选行
    if (OB_FAIL(get_next_candidate_row())) {
      break;
    }
    
    // 2. 立即应用标量过滤（避免存储和后续处理）
    if (OB_FAIL(apply_scalar_filters())) {
      if (ret == OB_FILTERED_OUT) {
        ret = OB_SUCCESS;  // 被过滤，继续下一行
        continue;
      }
      break;
    }
    
    // 3. 通过过滤，才进行完整的行处理
    found = true;
  }
  
  return ret;
}

// 新增函数：应用标量过滤
int ObDASIndexMergeIter::apply_scalar_filters()
{
  int ret = OB_SUCCESS;
  
  // 检查 base_id in (...)
  if (OB_NOT_NULL(base_id_filter_)) {
    ObDatum &base_id_datum = base_id_expr_->locate_expr_datum(*eval_ctx_);
    if (!base_id_filter_->contains(base_id_datum)) {
      ret = OB_FILTERED_OUT;
      return ret;
    }
  }
  
  // 检查 id < 1000
  if (OB_NOT_NULL(id_range_filter_)) {
    ObDatum &id_datum = id_expr_->locate_expr_datum(*eval_ctx_);
    if (id_datum.get_int() >= id_range_filter_->upper_bound_) {
      ret = OB_FILTERED_OUT;
      return ret;
    }
  }
  
  return ret;
}
```

**预期效果**: 减少 40-60% 的无效行处理

### 优化 3: TopN 提前终止

**文件**: `/root/ob/src/sql/das/iter/ob_das_index_merge_iter.cpp:226-449`

**当前代码已有 TopN 堆，但缺少提前终止逻辑**

```cpp
int ObDASIndexMergeIter::MergeResultBuffer::add_row_with_topn()
{
  // ... 现有代码 ...
  
  // 新增：计算最小可接受分数
  if (topn_heap_->count() >= topn_limit_) {
    TopNRow *heap_top = topn_heap_->top();
    min_acceptable_score_ = heap_top->score_;
    
    // 如果当前分数太低，直接跳过
    if (score < min_acceptable_score_ * 0.5) {  // 阈值可调
      return OB_SUCCESS;  // 跳过这一行
    }
  }
  
  // ... 继续现有逻辑 ...
}

// 在迭代器层面添加提前终止
int ObDASIndexMergeIter::union_get_next_rows(int64_t &count, int64_t capacity)
{
  // 如果 TopN 堆已满，且连续 N 行都低于阈值，可以提前终止
  static const int64_t EARLY_STOP_THRESHOLD = 100;
  int64_t consecutive_low_score_count = 0;
  
  while (count < capacity) {
    if (OB_FAIL(get_next_row())) {
      break;
    }
    
    // 检查是否应该提前终止
    if (enable_topn_ && topn_heap_full_) {
      if (current_score < min_acceptable_score_) {
        consecutive_low_score_count++;
        if (consecutive_low_score_count >= EARLY_STOP_THRESHOLD) {
          // 连续太多低分行，提前终止
          ret = OB_ITER_END;
          break;
        }
      } else {
        consecutive_low_score_count = 0;  // 重置计数器
      }
    }
    
    count++;
  }
  
  return ret;
}
```

**预期效果**: 对于 LIMIT 10 的查询，可能减少 50-80% 的扫描量

### 优化 4: 使用 Bloom Filter 加速标量过滤

**文件**: 新增 `/root/ob/src/sql/das/iter/ob_das_scalar_bloom_filter.h`

```cpp
class ObScalarBloomFilter {
public:
  // 为 base_id 构建 Bloom Filter
  int build_from_values(const ObIArray<ObString> &values) {
    // 使用标准 Bloom Filter 算法
    for (int64_t i = 0; i < values.count(); ++i) {
      uint64_t hash = murmurhash64(values.at(i));
      set_bit(hash % bit_array_size_);
      set_bit((hash >> 32) % bit_array_size_);
    }
  }
  
  // 快速检查是否可能存在
  bool may_contain(const ObString &value) const {
    uint64_t hash = murmurhash64(value);
    return test_bit(hash % bit_array_size_) && 
           test_bit((hash >> 32) % bit_array_size_);
  }
  
private:
  uint64_t *bit_array_;
  int64_t bit_array_size_;
};
```

**使用场景**: 在 B 树扫描时，先用 Bloom Filter 快速过滤

```cpp
// 在扫描循环中
while (OB_SUCC(btree_iter->get_next(key, value))) {
  // 快速检查（几个 CPU 周期）
  if (!base_id_bloom_filter_.may_contain(key.base_id_)) {
    continue;  // 快速跳过
  }
  
  // 精确检查（较慢）
  if (base_id_set_.contains(key.base_id_)) {
    // 处理这一行
  }
}
```

**预期效果**: 减少 20-30% 的精确比较开销

### 优化 5: 预取优化

**文件**: `/root/ob/src/storage/memtable/mvcc/ob_keybtree.cpp`

**利用 CPU 预取指令**

```cpp
template<typename BtreeKey, typename BtreeVal>
int ScanHandle<BtreeKey, BtreeVal>::scan_forward()
{
  int ret = OB_SUCCESS;
  BtreeNode* node = nullptr;
  int pos = 0;
  
  while(OB_SUCCESS == ret) {
    if (OB_FAIL(path_.pop(node, pos))) {
      ret = OB_ITER_END;
    } else if ((pos = node->get_next_active_child(pos)) >= node->size(index)) {
      // do nothing
    } else {
      path_.push(node, pos);
      if (node->is_leaf()) {
        break;
      } else {
        BtreeNode* child = (BtreeNode*)node->get_val(pos);
        child->prefetch();  // 已有预取
        
        // 新增：预取下一个兄弟节点
        if (pos + 1 < node->size(index)) {
          BtreeNode* next_sibling = (BtreeNode*)node->get_val(pos + 1);
          next_sibling->prefetch();  // 预取下一个
        }
        
        if (child->is_leaf()) {
          index->load(child->get_index());
        }
        path_.push(child, -1);
      }
    }
  }
  return ret;
}
```

**预期效果**: 减少 10-20% 的 cache miss

## 中期优化（需要更多重构）

### 优化 6: Bitmap 索引合并

**新增文件**: `/root/ob/src/sql/das/iter/ob_das_bitmap_merge_iter.cpp`

```cpp
class ObDASBitmapMergeIter : public ObDASIter {
public:
  int inner_get_next_rows(int64_t &count, int64_t capacity) override {
    // 1. 从各个索引获取 bitmap
    if (!bitmaps_ready_) {
      for (int64_t i = 0; i < child_cnt_; ++i) {
        if (OB_FAIL(build_bitmap_from_child(i, child_bitmaps_[i]))) {
          return ret;
        }
      }
      
      // 2. 位运算求交集（极快）
      result_bitmap_ = child_bitmaps_[0];
      for (int64_t i = 1; i < child_cnt_; ++i) {
        result_bitmap_.intersect(child_bitmaps_[i]);
      }
      
      bitmaps_ready_ = true;
    }
    
    // 3. 从 bitmap 中批量提取行号
    int64_t batch_count = 0;
    while (batch_count < capacity && bitmap_pos_ < result_bitmap_.size()) {
      if (result_bitmap_.test(bitmap_pos_)) {
        row_ids_[batch_count++] = bitmap_pos_;
      }
      bitmap_pos_++;
    }
    
    // 4. 批量回表
    if (batch_count > 0) {
      if (OB_FAIL(batch_lookup_rows(row_ids_, batch_count))) {
        return ret;
      }
    }
    
    count = batch_count;
    return ret;
  }
  
private:
  ObBitmap child_bitmaps_[MAX_CHILD_COUNT];
  ObBitmap result_bitmap_;
  bool bitmaps_ready_;
  int64_t bitmap_pos_;
};
```

**预期效果**: 索引合并性能提升 5-10 倍

### 优化 7: 并行索引扫描

**修改文件**: `/root/ob/src/sql/das/iter/ob_das_index_merge_iter.cpp`

```cpp
int ObDASIndexMergeIter::parallel_scan_indexes()
{
  int ret = OB_SUCCESS;
  
  // 使用线程池并行扫描
  ObThreadPool thread_pool;
  thread_pool.init(child_cnt_);
  
  for (int64_t i = 0; i < child_cnt_; ++i) {
    thread_pool.submit([this, i]() {
      // 每个索引在独立线程中扫描
      child_iters_[i]->do_table_scan();
    });
  }
  
  thread_pool.wait_all();
  
  return ret;
}
```

**预期效果**: 在多核 CPU 上提升 2-3 倍

## 性能测试计划

### 测试环境
- 数据集: MLDR v1.0-en (corpus.jsonl)
- 查询集: test.jsonl
- 硬件: 与比赛环境一致

### 测试指标
1. **查询延迟**: P50, P95, P99
2. **吞吐量**: QPS
3. **CPU 使用率**: 火焰图分析
4. **内存使用**: 峰值内存

### 测试步骤
1. **基线测试**: 记录当前性能
2. **逐个优化**: 每次只应用一个优化
3. **组合测试**: 测试多个优化的组合效果
4. **回归测试**: 确保正确性

## 实施优先级

### 第一阶段（1-2 天）
1. ✅ 增大 B 树扫描批次
2. ✅ TopN 提前终止
3. ✅ 预取优化

**预期提升**: 30-50%

### 第二阶段（3-5 天）
4. ✅ 标量过滤前置
5. ✅ Bloom Filter 加速

**预期提升**: 50-100%（累计 2-3 倍）

### 第三阶段（1-2 周）
6. ✅ Bitmap 索引合并
7. ✅ 并行索引扫描

**预期提升**: 100-200%（累计 3-5 倍）

## 风险评估

### 低风险优化
- 增大批次
- TopN 提前终止
- 预取优化

**风险**: 几乎无风险，只是参数调整

### 中风险优化
- 标量过滤前置
- Bloom Filter

**风险**: 需要仔细测试正确性

### 高风险优化
- Bitmap 索引合并
- 并行扫描

**风险**: 架构变动大，需要充分测试

## 监控和调试

### 关键指标
```cpp
// 添加性能计数器
struct ObBtreeScanStats {
  int64_t total_scan_count;
  int64_t filtered_count;
  int64_t early_stop_count;
  int64_t batch_size_sum;
  int64_t prefetch_hit_count;
};

// 在关键路径打印
LOG_INFO("Btree scan stats", 
         K(stats.total_scan_count),
         K(stats.filtered_count),
         "filter_rate", stats.filtered_count * 100.0 / stats.total_scan_count,
         "avg_batch_size", stats.batch_size_sum / stats.total_scan_count);
```

### 火焰图对比
- 优化前: 记录 B 树扫描占比
- 优化后: 验证占比下降

## 总结

通过以上优化，预期可以将 B 树扫描的开销降低 60-80%，整体查询性能提升 2-5 倍。

关键是：
1. **减少扫描量**: 标量过滤前置、TopN 提前终止
2. **提高扫描效率**: 增大批次、预取优化
3. **改进合并算法**: Bitmap、并行扫描
