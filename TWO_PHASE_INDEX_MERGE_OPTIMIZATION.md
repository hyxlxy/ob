# 两阶段 Index Merge 优化实现文档

## 一、优化背景

### 1.1 问题描述
在标量索引与全文索引混合查询场景下，特别是包含 `LIMIT` 条件的 TOP-N 查询中，传统的 Index Merge 执行计划存在以下问题：

- **执行计划结构**：
  ```
  TOP-N SORT
    └─ INDEX MERGE SCAN
         ├─ FT INDEX SCAN (全文索引)
         └─ IDX BASE_ID (标量索引)
  ```

- **性能瓶颈**：
  - 全文索引和标量索引都按照 Row Key 顺序输出
  - 最终的相关性排序依赖上层的 TOP-N SORT 算子
  - 无法充分利用全文索引的 BM25 算法优势

### 1.2 优化思路
通过两阶段执行策略，将 TOP-N 条件下推到全文索引层，利用 BM25 算法的相关性排序能力，避免上层的排序操作。

---

## 二、两阶段优化方案详解

### 2.1 第一阶段：普通 Index Merge

**目标**：获取 TOP-N 条初步结果，建立得分阈值

**执行流程**：
```
1. 对所有索引（全文索引 + 标量索引）执行普通 Index Merge
2. 返回满足所有条件的前 N 条记录（N = LIMIT 中指定的值）
3. 记录这 N 条记录中全文索引的最小得分：min_score
4. 将这 N 条记录暂存到结果缓冲区
```

**关键数据**：
- `first_phase_min_score_`：第一阶段得到的最小相关性得分
- `top_n_count_`：LIMIT 指定的记录数

### 2.2 第二阶段：BM25 加速迭代

**目标**：利用得分阈值进行裁剪，获取更多候选记录

**执行流程**：
```
1. 将第一阶段的得分阈值 (min_score) 下推给 BM25 迭代器
2. BM25 迭代器根据得分阈值进行裁剪，加速迭代过程
3. 从 BM25 迭代器中获取 N' 条记录（N' > N，放大因子）
4. 对这 N' 条记录使用标量索引进行过滤
5. 如果过滤后的记录数不足 N 条，从第一阶段的结果中补充
6. 对最终结果进行整体排序输出
```

**关键参数**：
- `N'`：放大的记录数，用于补偿过滤损失
- `min_score`：得分阈值，用于 BM25 算法的裁剪

---

## 三、代码实现分析

### 3.1 核心数据结构

#### ObDASIndexMergeIter 类

```cpp
class ObDASIndexMergeIter : public ObDASIter
{
private:
    // 两阶段优化相关成员
    bool enable_two_phase_;              // 是否启用两阶段优化
    bool has_executed_two_phase_;        // 是否已执行两阶段
    double first_phase_min_score_;       // 第一阶段的最小得分
    int64_t top_n_count_;                // TOP-N 的 N 值
    bool has_fts_iter_;                  // 是否有全文索引迭代器
    ObDASTRMergeIter *fts_iter_;         // FTS 迭代器指针
    
    // 结果缓冲区
    MergeResultBuffer result_buffer_;    // 存储第一阶段的结果
};
```

#### MergeResultBuffer 结构体

```cpp
struct MergeResultBuffer
{
    const common::ObIArray<ObExpr*> *exprs_;
    ObEvalCtx *eval_ctx_;
    int64_t max_size_;
    ObChunkDatumStore result_store_;     // 使用 ChunkDatumStore 存储行数据
    ObChunkDatumStore::Iterator result_store_iter_;
};
```

**作用**：
- 临时存储第一阶段的结果行
- 防止表达式内存位置冲突
- 支持后续的结果补充和排序

#### IndexMergeRowStore 结构体

```cpp
struct IndexMergeRowStore
{
    const common::ObIArray<ObExpr*> *exprs_;
    ObEvalCtx *eval_ctx_;
    int64_t max_size_;
    int64_t saved_size_;                 // 已保存的行数
    int64_t cur_idx_;                    // 当前访问位置
    LastDASStoreRow *store_rows_;        // 存储行数据的缓冲区
    bool iter_end_;                      // 迭代是否结束
};
```

**作用**：
- 存储子迭代器的结果行
- 支持行的保存和恢复
- 用于 Index Merge 的行合并

### 3.2 核心方法

#### 1. 初始化两阶段优化

```cpp
int init_two_phase(ObDASIterParam &param)
```

**功能**：
- 检查是否支持两阶段优化
- 初始化 FTS 迭代器
- 设置相关标志位

**检查条件**：
```cpp
bool check_support_two_phase()
{
    // 1. 必须是混合查询（包含全文索引和标量索引）
    if (!check_mixed_query_with_fts()) return false;
    
    // 2. 必须有 TOP-K 条件
    if (!check_top_k()) return false;
    
    // 3. 必须有 FTS 迭代器
    if (!has_fts_iter_) return false;
    
    return true;
}
```

#### 2. 第一阶段执行

```cpp
int first_merge_phase()
```

**执行步骤**：
1. 执行普通的 Index Merge（AND/OR 合并）
2. 获取前 N 条记录
3. 记录最小得分：`first_phase_min_score_`
4. 将结果保存到 `result_buffer_`

**伪代码**：
```cpp
int first_merge_phase() {
    int64_t count = 0;
    while (count < top_n_count_) {
        // 执行普通 merge
        if (OB_FAIL(inner_get_next_row())) break;
        
        // 保存行到 result_buffer_
        save_row_to_result_buffer();
        
        // 更新最小得分
        update_min_score();
        count++;
    }
    return OB_SUCCESS;
}
```

#### 3. 第二阶段执行

```cpp
int second_merge_phase()
```

**执行步骤**：
1. 将 `first_phase_min_score_` 下推给 FTS 迭代器
2. 从 FTS 迭代器获取 N' 条记录（N' = N * 放大因子）
3. 对每条记录使用标量索引进行过滤
4. 如果过滤后的记录数不足 N，从第一阶段结果中补充
5. 进行最终排序

**伪代码**：
```cpp
int second_merge_phase() {
    int64_t n_prime = get_amplified_top_n();  // 获取放大的 N'
    int64_t filtered_count = 0;
    
    // 从 FTS 迭代器获取 N' 条记录
    while (filtered_count < n_prime) {
        if (OB_FAIL(fts_iter_->get_next_row())) break;
        
        // 使用标量索引过滤
        if (pass_scalar_index_filter()) {
            filtered_count++;
        }
    }
    
    // 如果不足 N 条，从第一阶段补充
    if (filtered_count < top_n_count_) {
        supplement_from_first_phase(top_n_count_ - filtered_count);
    }
    
    // 最终排序
    sort_final_results();
    return OB_SUCCESS;
}
```

#### 4. 两阶段主控制方法

```cpp
int execute_two_phase()
```

**执行流程**：
```cpp
int execute_two_phase() {
    // 第一阶段：普通 merge
    if (OB_FAIL(first_merge_phase())) return ret;
    
    // 第二阶段：BM25 加速
    if (OB_FAIL(second_merge_phase())) return ret;
    
    has_executed_two_phase_ = true;
    return OB_SUCCESS;
}
```

### 3.3 辅助方法

#### 获取 TOP-N 数值

```cpp
int64_t get_top_n_count()
```

**功能**：
- 从查询计划中提取 LIMIT 值
- 返回 TOP-N 的 N 值

#### 检查混合查询

```cpp
bool check_mixed_query_with_fts()
```

**功能**：
- 检查是否同时包含全文索引和标量索引
- 验证是否满足混合查询条件

#### 检查 TOP-K 条件

```cpp
bool check_top_k()
```

**功能**：
- 检查查询是否包含 LIMIT 条件
- 验证是否为 TOP-K 查询

---

## 四、优化效果分析

### 4.1 性能提升

| 指标 | 传统 Index Merge | 两阶段优化 | 提升 |
|------|-----------------|-----------|------|
| 排序成本 | 高（上层 TOP-N SORT） | 低（下推到 BM25） | ✓ |
| 全文索引利用率 | 低 | 高（充分利用 BM25） | ✓ |
| 迭代次数 | 多（需要排序） | 少（BM25 直接排序） | ✓ |

### 4.2 权衡与取舍

#### 优点：
1. **性能提升**：避免上层 TOP-N SORT，利用 BM25 相关性排序
2. **索引利用**：充分发挥全文索引的排序能力
3. **减少 I/O**：BM25 算法的得分阈值裁剪减少不必要的数据访问

#### 缺点：
1. **召回率下降**：
   - 第一阶段的结果不一定是得分最高的 N 条
   - 补充机制可能导致最终结果不是最优的 N 条记录
   - 这是一种"牺牲召回率换取查询性能"的权衡

2. **实现复杂度**：
   - 需要维护两个阶段的状态
   - 需要管理结果缓冲区
   - 需要处理补充逻辑

### 4.3 适用场景

✓ **适用**：
- 包含 LIMIT 条件的查询
- 混合全文索引和标量索引的查询
- 对查询性能要求高、对召回率容忍度高的场景

✗ **不适用**：
- 需要精确排序的查询
- 对召回率要求严格的场景
- 只有单个索引的查询

---

## 五、实现细节

### 5.1 结果缓冲区管理

#### 初始化

```cpp
int MergeResultBuffer::init(int64_t max_size, 
                            ObEvalCtx *eval_ctx, 
                            const common::ObIArray<ObExpr*> *exprs, 
                            common::ObIAllocator &alloc)
{
    // 初始化 ChunkDatumStore
    result_store_.init(max_size, alloc);
    exprs_ = exprs;
    eval_ctx_ = eval_ctx;
    max_size_ = max_size;
    return OB_SUCCESS;
}
```

#### 添加行

```cpp
int MergeResultBuffer::add_row()
{
    // 将当前表达式的值保存到 result_store_
    return result_store_.add_row(*exprs_, *eval_ctx_);
}
```

#### 恢复表达式

```cpp
int MergeResultBuffer::to_expr(int64_t size)
{
    // 从 result_store_ 恢复 size 行到表达式
    return result_store_iter_.get_next_row(*exprs_, *eval_ctx_, size);
}
```

### 5.2 行存储机制

#### IndexMergeRowStore 的作用

```cpp
struct IndexMergeRowStore {
    // 保存行数据
    int save(bool is_vectorized, int64_t size) {
        // 将当前行保存到 store_rows_
    }
    
    // 恢复行数据到表达式
    int to_expr() {
        // 从 store_rows_ 恢复数据到表达式
    }
    
    // 检查是否有数据
    bool have_data() const {
        return cur_idx_ < saved_size_;
    }
};
```

### 5.3 状态管理

#### 标志位说明

| 标志位 | 含义 | 用途 |
|--------|------|------|
| `enable_two_phase_` | 是否启用两阶段 | 控制是否执行优化 |
| `has_executed_two_phase_` | 是否已执行 | 防止重复执行 |
| `has_fts_iter_` | 是否有 FTS 迭代器 | 检查前置条件 |
| `iter_end_` | 迭代是否结束 | 控制迭代流程 |

---

## 六、执行流程图

```
┌─────────────────────────────────────────────────────┐
│         检查是否支持两阶段优化                        │
│  - 混合查询？ - TOP-K 条件？ - FTS 迭代器？         │
└────────────────┬────────────────────────────────────┘
                 │
        ┌────────▼────────┐
        │   支持两阶段？   │
        └────┬─────────┬──┘
             │         │
          YES│         │NO
             │         │
    ┌────────▼─┐   ┌──▼──────────────┐
    │ 第一阶段  │   │ 执行普通 Merge  │
    │ 普通Merge │   │ (传统方式)      │
    └────┬──────┘   └─────────────────┘
         │
    ┌────▼──────────────────────────┐
    │ 记录最小得分 (min_score)       │
    │ 保存 N 条结果到缓冲区          │
    └────┬──────────────────────────┘
         │
    ┌────▼──────────────────────────┐
    │ 第二阶段：BM25 加速            │
    │ - 下推得分阈值                 │
    │ - 获取 N' 条记录               │
    │ - 标量索引过滤                 │
    └────┬──────────────────────────┘
         │
    ┌────▼──────────────────────────┐
    │ 结果补充与排序                 │
    │ - 不足 N 条时补充              │
    │ - 最终排序输出                 │
    └────┬──────────────────────────┘
         │
    ┌────▼──────────────────────────┐
    │ 返回最终结果                   │
    └────────────────────────────────┘
```

---

## 七、关键设计决策

### 7.1 为什么需要结果缓冲区？

**问题**：
- 子迭代器返回的行数据存储在表达式的内存位置上
- 多个子迭代器共享表达式会导致数据被覆盖

**解决方案**：
- 使用 `MergeResultBuffer` 和 `IndexMergeRowStore` 临时存储行数据
- 通过 `ChunkDatumStore` 实现高效的行存储和恢复

### 7.2 为什么需要放大因子？

**问题**：
- 第二阶段获取的 N' 条记录可能在标量索引过滤后数量不足

**解决方案**：
- 设置 `N' > N`（例如 N' = N * 1.5）
- 确保过滤后仍有足够的记录

### 7.3 为什么允许召回率下降？

**理由**：
1. 第一阶段的 N 条记录是按 Row Key 顺序的，不是按相关性排序
2. 补充机制从第一阶段结果中选择，可能不是最优的
3. 这是一种明确的性能与准确性的权衡

**适用场景**：
- 用户对排序准确性容忍度高
- 查询性能是首要目标
- 可以通过业务逻辑补偿

---

## 八、扩展与改进方向

### 8.1 动态放大因子

```cpp
// 根据第一阶段的过滤率动态调整 N'
int64_t amplification_factor = 
    calculate_amplification_factor(first_phase_filter_rate);
int64_t n_prime = top_n_count_ * amplification_factor;
```

### 8.2 得分阈值优化

```cpp
// 使用更精细的得分阈值策略
double optimized_threshold = 
    calculate_optimized_threshold(first_phase_min_score_, 
                                  filter_selectivity);
```

### 8.3 多索引权重调整

```cpp
// 为不同索引设置不同的权重
double fts_weight = 0.7;
double scalar_weight = 0.3;
double combined_score = fts_weight * fts_score + scalar_weight * scalar_score;
```

---

## 九、总结

### 核心创新点

1. **两阶段执行策略**：
   - 第一阶段：建立得分阈值基准
   - 第二阶段：利用阈值进行加速迭代

2. **得分阈值下推**：
   - 将 TOP-N 条件转化为得分阈值
   - 下推给 BM25 算法进行裁剪

3. **结果补充机制**：
   - 处理过滤导致的记录不足
   - 保证返回足够的结果

### 适用范围

- ✓ 混合全文索引和标量索引的查询
- ✓ 包含 LIMIT 条件的 TOP-N 查询
- ✓ 对性能要求高的场景

### 性能收益

- 避免上层 TOP-N SORT 操作
- 充分利用 BM25 相关性排序
- 通过得分阈值裁剪减少 I/O

### 权衡与代价

- 召回率可能下降（明确的权衡）
- 实现复杂度增加
- 需要仔细的参数调优

---

**文档版本**：1.0  
**最后更新**：2025-12-05  
**相关文件**：`src/sql/das/iter/ob_das_index_merge_iter.h`

