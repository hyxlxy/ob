# BMW 迭代器阈值设置功能详细分析

## 概述

本文档详细分析了 BMW（Block-Max WAND）迭代器与 DAAT、TAAT 迭代器在阈值设置和调整功能方面的实现差异。

## 结论

**BMW 迭代器具有完整的阈值设置和动态调整功能，而 DAAT 和 TAAT 迭代器没有此功能。**

## 详细分析

### 1. BMW 迭代器的阈值实现

#### 1.1 成员变量

BMW 迭代器在 `ob_sparse_bmw_iter.h` 中定义了阈值相关的成员变量：

```cpp
// 可选的外部阈值，来自上层（例如 baseline top-N 的最小分数）
// 当 has_external_min_score_ 为 true 时，BMW top-k 搜索可以使用 external_min_score_
// 作为初始下界进行剪枝
double external_min_score_;
bool has_external_min_score_;
```

**位置：** `src/storage/retrieval/ob_sparse_bmw_iter.h:116-117`

#### 1.2 初始化过程

在 `init()` 函数中，BMW 迭代器从参数中读取阈值：

```cpp
int ObSRBMWIterImpl::init(
    ObSparseRetrievalMergeParam &iter_param,
    ObIArray<ObISRDaaTDimIter *> &dim_iters,
    ObIAllocator &iter_allocator,
    ObSRDaaTRelevanceCollector &relevance_collector)
{
  // ...
  // record optional external threshold for later pruning
  external_min_score_ = iter_param.external_min_score_;
  has_external_min_score_ = iter_param.has_external_min_score_;
  // ...
}
```

**位置：** `src/storage/retrieval/ob_sparse_bmw_iter.cpp:59-61`

#### 1.3 阈值获取方法

BMW 迭代器提供了 `get_top_k_threshold()` 方法来动态计算当前阈值：

```cpp
double get_top_k_threshold()
{
  double heap_threshold = top_k_heap_.empty() ? 0.0 : top_k_heap_.top().relevance_;
  if (has_external_min_score_) {
    return heap_threshold > external_min_score_ ? heap_threshold : external_min_score_;
  } else {
    return heap_threshold;
  }
}
```

**位置：** `src/storage/retrieval/ob_sparse_bmw_iter.h:52-60`

**逻辑说明：**
- 如果堆为空，阈值为 0.0
- 如果堆不为空，阈值为堆顶元素的相关性分数
- 如果设置了外部阈值（`has_external_min_score_` 为 true），则返回 `max(heap_threshold, external_min_score_)`
- 这样可以确保阈值不会低于外部传入的最小分数，实现更激进的剪枝

#### 1.4 阈值的使用场景

BMW 迭代器在多个关键位置使用阈值进行剪枝：

##### 1.4.1 处理收集的行 (`process_collected_row`)

```cpp
int ObSRBMWIterImpl::process_collected_row(const ObDatum &id_datum, const double relevance)
{
  // ...
  const double top_k_threshold = get_top_k_threshold();
  if (relevance <= top_k_threshold) {
    // not a new top k candidate - 剪枝：相关性分数不够高
  } else {
    // 更新 top-k 堆
  }
}
```

**位置：** `src/storage/retrieval/ob_sparse_bmw_iter.cpp:263-264`

##### 1.4.2 查找下一个枢轴 (`next_pivot`)

```cpp
int ObSRBMWIterImpl::next_pivot(int64_t &pivot_iter_idx)
{
  const double top_k_threshold = get_top_k_threshold();
  // ...
  found_pivot = max_score > top_k_threshold && curr_id_end;
  // ...
  if (max_score <= top_k_threshold) {
    ret = OB_ITER_END;  // 剪枝：最大分数不够高
  }
}
```

**位置：** `src/storage/retrieval/ob_sparse_bmw_iter.cpp:282, 302, 307`

##### 1.4.3 评估枢轴范围 (`evaluate_pivot_range`)

```cpp
int ObSRBMWIterImpl::evaluate_pivot_range(const int64_t pivot_iter_idx, bool &is_candidate)
{
  // ...
  is_candidate = block_max_score > get_top_k_threshold();
  // 剪枝：块最大分数不够高，跳过整个范围
}
```

**位置：** `src/storage/retrieval/ob_sparse_bmw_iter.cpp:490`

##### 1.4.4 生成下一个候选范围 (`try_generate_next_range_from_merge_heap`)

```cpp
int ObSRBMWIterImpl::try_generate_next_range_from_merge_heap(...)
{
  const double top_k_threshold = get_top_k_threshold();
  while (OB_SUCC(ret) && max_score <= top_k_threshold && !curr_range_reach_end && !merge_heap_->empty()) {
    // 跳过所有 max_score <= threshold 的范围
  }
  if (max_score > top_k_threshold) {
    // found a new top k candidate range
    is_candidate_range = true;
  }
}
```

**位置：** `src/storage/retrieval/ob_sparse_bmw_iter.cpp:561, 564, 610`

#### 1.5 阈值的动态调整

BMW 迭代器的阈值是**动态调整**的：

1. **初始阶段**：如果设置了 `external_min_score_`，初始阈值为外部阈值
2. **堆填充阶段**：随着 top-k 堆的填充，阈值逐渐提高（堆顶元素的相关性分数）
3. **搜索阶段**：每次处理新行或评估枢轴时，阈值都会更新为 `max(heap_threshold, external_min_score_)`

这种动态调整机制使得 BMW 迭代器能够：
- 在早期阶段利用外部阈值进行剪枝
- 随着搜索的进行，利用已找到的 top-k 结果提高阈值
- 实现更激进的剪枝，减少不必要的计算

### 2. DAAT 迭代器的阈值实现

#### 2.1 类继承关系

```cpp
class ObSRDaaTIterImpl : public ObISparseRetrievalMergeIter
```

**位置：** `src/storage/retrieval/ob_sparse_daat_iter.h:65`

#### 2.2 成员变量检查

查看 `ObSRDaaTIterImpl` 的成员变量定义：

```cpp
protected:
  ObIAllocator *iter_allocator_;
  ObSparseRetrievalMergeParam *iter_param_;
  ObIArray<ObISRDaaTDimIter *> *dim_iters_;
  ObSRMergeCmp merge_cmp_;
  ObSRMergeLoserTree *merge_heap_;
  ObSRDaaTRelevanceCollector *relevance_collector_;
  // ... 其他成员变量
```

**位置：** `src/storage/retrieval/ob_sparse_daat_iter.h:93-102`

**结论：** DAAT 迭代器**没有** `external_min_score_` 和 `has_external_min_score_` 成员变量。

#### 2.3 初始化过程检查

查看 `ObSRDaaTIterImpl::init()` 函数：

```cpp
int ObSRDaaTIterImpl::init(
    ObSparseRetrievalMergeParam &iter_param,
    ObIArray<ObISRDaaTDimIter *> &dim_iters,
    ObIAllocator &iter_allocator,
    ObSRDaaTRelevanceCollector &relevance_collector)
{
  // ... 初始化各种成员变量
  // 没有读取或使用 external_min_score_ 和 has_external_min_score_
}
```

**位置：** `src/storage/retrieval/ob_sparse_daat_iter.cpp:90-150`

**结论：** DAAT 迭代器在初始化时**不会**读取或使用阈值参数。

#### 2.4 搜索过程检查

DAAT 迭代器使用 Loser Tree 进行合并，但没有使用阈值进行剪枝的逻辑。

**结论：** DAAT 迭代器**没有**阈值设置和调整功能。

### 3. TAAT 迭代器的阈值实现

#### 3.1 类继承关系

```cpp
class ObSRTaaTIterImpl : public ObISparseRetrievalMergeIter
```

**位置：** `src/storage/retrieval/ob_sparse_taat_iter.h:29`

#### 3.2 成员变量检查

查看 `ObSRTaaTIterImpl` 的成员变量定义：

```cpp
protected:
  ObIAllocator *iter_allocator_;
  ObSparseRetrievalMergeParam *iter_param_;
  ObISparseRetrievalDimIter *dim_iter_;
  int64_t partition_cnt_;
  sql::ObChunkDatumStore **datum_stores_;
  // ... 其他成员变量
```

**位置：** `src/storage/retrieval/ob_sparse_taat_iter.h:55-69`

**结论：** TAAT 迭代器**没有** `external_min_score_` 和 `has_external_min_score_` 成员变量。

#### 3.3 初始化过程检查

查看 `ObSRTaaTIterImpl::init()` 函数：

```cpp
int ObSRTaaTIterImpl::init(
    ObSparseRetrievalMergeParam &iter_param,
    ObISparseRetrievalDimIter &dim_iter,
    ObIAllocator &iter_allocator)
{
  // ... 初始化各种成员变量
  // 没有读取或使用 external_min_score_ 和 has_external_min_score_
}
```

**位置：** `src/storage/retrieval/ob_sparse_taat_iter.cpp:46-72`

**结论：** TAAT 迭代器在初始化时**不会**读取或使用阈值参数。

#### 3.4 搜索过程检查

TAAT 迭代器使用哈希表进行聚合，但没有使用阈值进行剪枝的逻辑。

**结论：** TAAT 迭代器**没有**阈值设置和调整功能。

### 4. 参数结构中的阈值字段

虽然 `ObSparseRetrievalMergeParam` 结构体中定义了阈值相关字段：

```cpp
struct ObSparseRetrievalMergeParam
{
  // ...
  // optional external score threshold for BMW top-k search.
  // when has_external_min_score_ is true, BMW iterator can use external_min_score_
  // as an initial lower bound to prune low score candidates more aggressively.
  double external_min_score_;
  bool has_external_min_score_;
};
```

**位置：** `src/storage/retrieval/ob_i_sparse_retrieval_iter.h:132-136`

**重要说明：** 注释明确说明这些字段是**专门为 BMW top-k 搜索设计的**，用于作为初始下界来更激进地剪枝低分候选。

### 5. 阈值传递流程

阈值从上层传递到 BMW 迭代器的流程：

1. **DAS 层设置** (`ob_das_iter_utils.cpp:1909-1910`)：
   ```cpp
   merge_iter_param.external_min_score_ = external_min_score;
   merge_iter_param.has_external_min_score_ = true;
   ```

2. **Merge Iter 层传递** (`ob_das_tr_merge_iter.cpp:80-81`)：
   ```cpp
   sr_iter_param_.external_min_score_ = merge_param.external_min_score_;
   sr_iter_param_.has_external_min_score_ = merge_param.has_external_min_score_;
   ```

3. **BMW 迭代器接收** (`ob_sparse_bmw_iter.cpp:60-61`)：
   ```cpp
   external_min_score_ = iter_param.external_min_score_;
   has_external_min_score_ = iter_param.has_external_min_score_;
   ```

## 总结对比表

| 特性 | BMW 迭代器 | DAAT 迭代器 | TAAT 迭代器 |
|------|-----------|------------|------------|
| **阈值成员变量** | ✅ 有 (`external_min_score_`, `has_external_min_score_`) | ❌ 无 | ❌ 无 |
| **阈值初始化** | ✅ 从参数读取 | ❌ 不读取 | ❌ 不读取 |
| **阈值获取方法** | ✅ `get_top_k_threshold()` | ❌ 无 | ❌ 无 |
| **阈值动态调整** | ✅ 动态调整（max(heap_threshold, external_min_score_)） | ❌ 无 | ❌ 无 |
| **阈值剪枝** | ✅ 在多个位置使用阈值剪枝 | ❌ 无 | ❌ 无 |
| **外部阈值支持** | ✅ 支持外部传入阈值 | ❌ 不支持 | ❌ 不支持 |

## 设计原因分析

### 为什么只有 BMW 迭代器有阈值功能？

1. **算法特性**：
   - BMW（Block-Max WAND）算法本身就是为了 top-k 查询优化的，需要动态阈值来剪枝
   - DAAT 和 TAAT 算法主要用于全量合并，不需要 top-k 剪枝

2. **性能优化**：
   - BMW 迭代器使用阈值可以：
     - 跳过低分候选
     - 减少不必要的计算
     - 提高 top-k 查询性能
   - DAAT 和 TAAT 迭代器的设计目标不同，不需要这种优化

3. **使用场景**：
   - BMW 迭代器主要用于 top-k 查询场景（如 `LIMIT 10`）
   - DAAT 和 TAAT 迭代器用于全量结果合并

## 代码位置总结

### BMW 迭代器阈值相关代码

1. **头文件**：
   - `src/storage/retrieval/ob_sparse_bmw_iter.h:52-60, 113-117`

2. **实现文件**：
   - `src/storage/retrieval/ob_sparse_bmw_iter.cpp:36-37, 59-61, 83, 95-96, 263, 282, 302, 307, 490, 561, 564, 610`

3. **参数结构**：
   - `src/storage/retrieval/ob_i_sparse_retrieval_iter.h:132-136`

4. **传递流程**：
   - `src/sql/das/iter/ob_das_iter_utils.cpp:1909-1910`
   - `src/sql/das/iter/sparse_retrieval/ob_das_tr_merge_iter.cpp:80-81`

### DAAT/TAAT 迭代器相关代码

1. **DAAT 头文件**：
   - `src/storage/retrieval/ob_sparse_daat_iter.h:65-107`

2. **DAAT 实现**：
   - `src/storage/retrieval/ob_sparse_daat_iter.cpp:90-150`

3. **TAAT 头文件**：
   - `src/storage/retrieval/ob_sparse_taat_iter.h:29-72`

4. **TAAT 实现**：
   - `src/storage/retrieval/ob_sparse_taat_iter.cpp:46-72`

## 结论

**BMW 迭代器具有完整的阈值设置和动态调整功能，而 DAAT 和 TAAT 迭代器没有此功能。**

这是由算法特性和使用场景决定的：
- BMW 迭代器专为 top-k 查询优化，需要阈值进行剪枝
- DAAT 和 TAAT 迭代器用于全量合并，不需要阈值剪枝

阈值功能使得 BMW 迭代器能够：
1. 利用外部传入的初始阈值进行早期剪枝
2. 动态调整阈值（基于 top-k 堆的状态）
3. 在多个关键位置使用阈值进行剪枝，提高性能















