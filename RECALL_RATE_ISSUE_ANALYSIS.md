# 两阶段 Index Merge 优化 - 召回率异常下降问题分析

## 问题现象

正确实现的两阶段优化方案**应该能达到要求的召回率**，但当前实现出现了**召回率异常下降**的问题。

---

## 根本原因分析

### 1. 两阶段方法实现为空

**文件位置**：`src/sql/das/iter/ob_das_index_merge_iter.cpp` 第 983-1005 行

**当前代码**：
```cpp
int ObDASIndexMergeIter::execute_two_phase() {
  int ret = first_merge_phase();
  if (ret != OB_SUCCESS) {
    return ret;
  }
  return second_merge_phase();
}

int ObDASIndexMergeIter::first_merge_phase() {
  int ret = OB_SUCCESS;
  return ret;  // ❌ 空实现！
}

int ObDASIndexMergeIter::second_merge_phase() {
  int ret = OB_SUCCESS;
  return ret;  // ❌ 空实现！
}
```

**问题**：
- `first_merge_phase()` 没有实现任何逻辑，直接返回成功
- `second_merge_phase()` 没有实现任何逻辑，直接返回成功
- 两阶段优化实际上没有被执行

### 2. 两阶段优化没有被正确集成到主流程

**问题位置**：`inner_get_next_row()` 和 `inner_get_next_rows()` 方法

**当前代码**（第 582-605 行）：
```cpp
int ObDASIndexMergeIter::inner_get_next_row()
{
  int ret = OB_SUCCESS;
  clear_evaluated_flag();
  if (OB_FAIL((this->*get_next_row_)())) {  // 直接调用 union/intersect 方法
    if (ret != OB_ITER_END) {
      LOG_WARN("index merge iter failed to get next row", K(ret));
    }
  }
  return ret;
}
```

**问题**：
- 没有检查 `enable_two_phase_` 标志
- 没有调用 `execute_two_phase()` 方法
- 即使初始化了两阶段优化，也不会被执行

### 3. 初始化逻辑不完整

**文件位置**：`src/sql/das/iter/ob_das_index_merge_iter.cpp` 第 1007-1060 行

**当前代码**：
```cpp
int ObDASIndexMergeIter::init_two_phase(ObDASIterParam &param) {
  int ret = OB_SUCCESS;
  if (merge_type_ != INDEX_MERGE_INTERSECT) {
    adlog::DEBUG("check_init_two_phase: no and");
    return OB_SUCCESS;
  }

  if (!check_mixed_query_with_fts()) {
    adlog::DEBUG("check_mixed_query_with_fts false");
    return OB_SUCCESS;
  }

  if (!has_fts_iter_) {
    adlog::DEBUG("has_fts_iter_: false");
    return OB_SUCCESS;
  }

  if (!check_top_k()) {
    adlog::DEBUG("check top k false");
    return OB_SUCCESS;
  }

  if (!check_support_two_phase()) {
    adlog::DEBUG("check_support_two_phase false");
    return OB_SUCCESS;
  }

  enable_two_phase_ = true;
  adlog::DEBUG("Two-phase optimization enabled ");
  return ret;
}
```

**问题**：
- `check_top_k()` 实现不完整（第 1048-1056 行）：
  ```cpp
  bool ObDASIndexMergeIter::check_top_k() {
    // if (merge_rtdef_ && merge_rtdef_->limit_param_.is_valid()) {
    //   top_n_count_ = merge_rtdef_->limit_param_.limit_count_;
    //   if (top_n_count_ > 0) {
    //       adlog::DEBUG("Top-K query detected from limit_param: ", top_n_count_);
    //       return true;
    //   }
    // }
    if (top_n_count_ == 0 or top_n_count_ == 114514) {  // ❌ 魔数检查！
      return false;
    }
    return true;
  }
  ```
  - 依赖于 `top_n_count_` 被正确设置
  - 使用魔数 `114514` 作为默认值检查
  - 没有从 `merge_rtdef_->limit_param_` 中提取 LIMIT 值

### 4. 关键成员变量未被正确初始化

**问题**：
- `top_n_count_` 初始化为 `114514`（魔数）
- `enable_two_phase_` 初始化为 `false`
- `has_executed_two_phase_` 初始化为 `false`
- `first_phase_min_score_` 未初始化
- `fts_iter_` 初始化为 `nullptr`

---

## 召回率下降的具体原因

### 场景分析

假设查询为：`SELECT * FROM table WHERE MATCH(content) AGAINST('keyword') AND status = 1 LIMIT 10`

**预期行为**：
1. 第一阶段：执行普通 Index Merge，获取 10 条初步结果，记录最小得分
2. 第二阶段：利用 BM25 算法和得分阈值加速，获取更多候选记录
3. 过滤和补充：确保最终返回 10 条最相关的记录

**实际行为**：
1. ❌ `execute_two_phase()` 从未被调用
2. ❌ `first_merge_phase()` 为空实现
3. ❌ `second_merge_phase()` 为空实现
4. ✓ 直接执行普通的 `intersect_get_next_row()`
5. ✓ 返回的是按 Row Key 顺序的前 10 条记录，**不是按相关性排序**

### 召回率下降的原因

由于两阶段优化没有被执行，系统退化为：
- 普通的 Index Merge（按 Row Key 顺序）
- 上层依赖 TOP-N SORT 进行排序
- 但如果 TOP-N SORT 也有问题或被优化掉，就会返回错误的结果

**可能的具体问题**：
1. 返回的 10 条记录不是相关性最高的 10 条
2. 某些相关性高的记录被遗漏
3. 某些相关性低的记录被错误地包含

---

## 修复方案

### 第一步：完成 `first_merge_phase()` 实现

```cpp
int ObDASIndexMergeIter::first_merge_phase() {
  int ret = OB_SUCCESS;
  int64_t count = 0;
  
  // 执行普通的 Index Merge，获取 top_n_count_ 条记录
  while (OB_SUCC(ret) && count < top_n_count_) {
    // 调用普通的 intersect_get_next_row 或 union_get_next_row
    if (OB_FAIL((this->*get_next_row_)())) {
      if (OB_ITER_END == ret) {
        ret = OB_SUCCESS;  // 记录数不足也是可以接受的
        break;
      } else {
        LOG_WARN("first merge phase failed to get next row", K(ret));
        return ret;
      }
    }
    
    // 保存行到结果缓冲区
    if (OB_FAIL(save_row_to_result_buffer())) {
      LOG_WARN("failed to save row to result buffer", K(ret));
      return ret;
    }
    
    // 更新最小得分（从全文索引的 MATCH_AGAINST 表达式中提取）
    if (OB_FAIL(update_min_score_from_fts())) {
      LOG_WARN("failed to update min score", K(ret));
      return ret;
    }
    
    count++;
  }
  
  LOG_DEBUG("first merge phase completed", K(count), K_(first_phase_min_score));
  return OB_SUCCESS;
}
```

### 第二步：完成 `second_merge_phase()` 实现

```cpp
int ObDASIndexMergeIter::second_merge_phase() {
  int ret = OB_SUCCESS;
  
  if (OB_ISNULL(fts_iter_)) {
    LOG_WARN("fts_iter is null in second phase");
    return OB_ERR_UNEXPECTED;
  }
  
  // 将得分阈值下推给 FTS 迭代器
  if (OB_FAIL(fts_iter_->set_score_threshold(first_phase_min_score_))) {
    LOG_WARN("failed to set score threshold", K_(first_phase_min_score), K(ret));
    return ret;
  }
  
  // 计算放大的 N'
  int64_t n_prime = top_n_count_ * AMPLIFICATION_FACTOR;  // 例如 1.5
  int64_t filtered_count = 0;
  
  // 从 FTS 迭代器获取 N' 条记录
  while (OB_SUCC(ret) && filtered_count < n_prime) {
    if (OB_FAIL(fts_iter_->get_next_row())) {
      if (OB_ITER_END == ret) {
        ret = OB_SUCCESS;
        break;
      } else {
        LOG_WARN("second merge phase failed to get next row", K(ret));
        return ret;
      }
    }
    
    // 使用标量索引过滤
    if (OB_FAIL(apply_scalar_filters())) {
      LOG_WARN("failed to apply scalar filters", K(ret));
      return ret;
    }
    
    // 如果通过过滤，保存到结果缓冲区
    if (OB_SUCC(ret)) {
      if (OB_FAIL(save_row_to_result_buffer())) {
        LOG_WARN("failed to save filtered row", K(ret));
        return ret;
      }
      filtered_count++;
    }
  }
  
  // 如果过滤后的记录数不足 top_n_count_，从第一阶段补充
  if (filtered_count < top_n_count_) {
    int64_t need_supplement = top_n_count_ - filtered_count;
    if (OB_FAIL(supplement_from_first_phase(need_supplement))) {
      LOG_WARN("failed to supplement from first phase", K(need_supplement), K(ret));
      return ret;
    }
  }
  
  // 最终排序
  if (OB_FAIL(sort_final_results())) {
    LOG_WARN("failed to sort final results", K(ret));
    return ret;
  }
  
  LOG_DEBUG("second merge phase completed", K(filtered_count));
  return OB_SUCCESS;
}
```

### 第三步：修复 `check_top_k()` 实现

```cpp
bool ObDASIndexMergeIter::check_top_k() {
  // 从 merge_rtdef_ 中提取 LIMIT 值
  if (OB_NOT_NULL(merge_rtdef_) && merge_rtdef_->limit_param_.is_valid()) {
    top_n_count_ = merge_rtdef_->limit_param_.limit_count_;
    if (top_n_count_ > 0) {
      LOG_DEBUG("Top-K query detected from limit_param", K_(top_n_count));
      return true;
    }
  }
  
  // 如果没有有效的 limit_param_，检查是否有其他方式设置的 top_n_count_
  if (top_n_count_ > 0 && top_n_count_ != 114514) {
    LOG_DEBUG("Top-K query detected from top_n_count", K_(top_n_count));
    return true;
  }
  
  LOG_DEBUG("Not a Top-K query");
  return false;
}
```

### 第四步：在 `inner_get_next_row()` 中集成两阶段优化

```cpp
int ObDASIndexMergeIter::inner_get_next_row()
{
  int ret = OB_SUCCESS;
  clear_evaluated_flag();
  
  // 如果启用了两阶段优化且尚未执行
  if (enable_two_phase_ && !has_executed_two_phase_) {
    if (OB_FAIL(execute_two_phase())) {
      LOG_WARN("failed to execute two phase optimization", K(ret));
      return ret;
    }
    has_executed_two_phase_ = true;
    // 从结果缓冲区返回第一行
    if (OB_FAIL(result_buffer_.to_expr(1))) {
      LOG_WARN("failed to get first row from result buffer", K(ret));
      return ret;
    }
    return OB_SUCCESS;
  }
  
  // 如果已执行两阶段优化，从结果缓冲区继续返回
  if (has_executed_two_phase_) {
    if (OB_FAIL(result_buffer_.to_expr(1))) {
      if (OB_ITER_END == ret) {
        return ret;  // 结果已全部返回
      }
      LOG_WARN("failed to get next row from result buffer", K(ret));
      return ret;
    }
    return OB_SUCCESS;
  }
  
  // 否则执行普通的 Index Merge
  if (OB_FAIL((this->*get_next_row_)())) {
    if (ret != OB_ITER_END) {
      LOG_WARN("index merge iter failed to get next row", K(ret));
    }
  }
  return ret;
}
```

### 第五步：在 `inner_get_next_rows()` 中集成两阶段优化

```cpp
int ObDASIndexMergeIter::inner_get_next_rows(int64_t &count, int64_t capacity)
{
  int ret = OB_SUCCESS;
  clear_evaluated_flag();
  
  // 如果启用了两阶段优化且尚未执行
  if (enable_two_phase_ && !has_executed_two_phase_) {
    if (OB_FAIL(execute_two_phase())) {
      LOG_WARN("failed to execute two phase optimization", K(ret));
      return ret;
    }
    has_executed_two_phase_ = true;
    // 从结果缓冲区返回一批行
    if (OB_FAIL(result_buffer_.to_expr(capacity))) {
      if (OB_ITER_END == ret) {
        count = 0;
        return ret;
      }
      LOG_WARN("failed to get batch from result buffer", K(ret));
      return ret;
    }
    count = capacity;  // 实际应该是返回的实际行数
    return OB_SUCCESS;
  }
  
  // 如果已执行两阶段优化，从结果缓冲区继续返回
  if (has_executed_two_phase_) {
    if (OB_FAIL(result_buffer_.to_expr(capacity))) {
      if (OB_ITER_END == ret) {
        count = 0;
        return ret;
      }
      LOG_WARN("failed to get next batch from result buffer", K(ret));
      return ret;
    }
    count = capacity;
    return OB_SUCCESS;
  }
  
  // 否则执行普通的 Index Merge
  if (OB_FAIL((this->*get_next_rows_)(count, capacity))) {
    if (ret != OB_ITER_END) {
      LOG_WARN("index merge iter failed to get next rows", K(ret));
    }
  }
  LOG_TRACE("[DAS ITER] index merge iter get next rows", K(count), K(capacity), K(ret));
  const ObBitVector *skip = nullptr;
  PRINT_VECTORIZED_ROWS(SQL, DEBUG, *eval_ctx_, *output_, count, skip);

  return ret;
}
```

---

## 关键实现细节

### 1. 得分提取方法

需要实现 `update_min_score_from_fts()` 方法来从 MATCH_AGAINST 表达式中提取得分：

```cpp
int ObDASIndexMergeIter::update_min_score_from_fts() {
  int ret = OB_SUCCESS;
  
  // 从 FTS 迭代器对应的子迭代器的输出表达式中找到 MATCH_AGAINST 表达式
  for (int64_t i = 0; i < child_iters_.count(); i++) {
    if (child_iters_.at(i) == fts_iter_) {
      const ExprFixedArray &match_against_exprs = child_match_against_exprs_.at(i);
      
      for (int64_t j = 0; j < match_against_exprs.count(); j++) {
        ObExpr *expr = match_against_exprs.at(j);
        if (OB_NOT_NULL(expr)) {
          ObDatum &datum = expr->locate_datum_for_read(*eval_ctx_);
          if (!datum.is_null()) {
            double score = datum.get_double();
            if (first_phase_min_score_ == 0 || score < first_phase_min_score_) {
              first_phase_min_score_ = score;
            }
          }
        }
      }
      break;
    }
  }
  
  return ret;
}
```

### 2. 标量索引过滤方法

需要实现 `apply_scalar_filters()` 方法来应用标量索引的过滤条件。

### 3. 结果补充方法

需要实现 `supplement_from_first_phase()` 方法来从第一阶段的结果中补充记录。

### 4. 结果排序方法

需要实现 `sort_final_results()` 方法来对最终结果进行排序。

---

## 验证方案

### 测试用例

1. **基础测试**：验证两阶段优化是否被正确启用
   ```sql
   SELECT * FROM table 
   WHERE MATCH(content) AGAINST('keyword') AND status = 1 
   LIMIT 10;
   ```

2. **召回率测试**：验证返回的 10 条记录是否是相关性最高的
   - 对比普通 Index Merge 的结果
   - 对比全文索引单独查询的结果

3. **性能测试**：验证两阶段优化是否提升了性能
   - 测量查询执行时间
   - 测量 I/O 操作次数

### 调试方法

1. 添加详细的日志输出
2. 检查 `enable_two_phase_` 和 `has_executed_two_phase_` 的值
3. 检查 `first_phase_min_score_` 的值
4. 检查结果缓冲区中的记录数

---

## 总结

### 问题根源

1. **两阶段方法为空实现** - 核心逻辑没有被实现
2. **没有集成到主流程** - 即使初始化了也不会被调用
3. **初始化逻辑不完整** - LIMIT 值提取有问题
4. **成员变量初始化不当** - 使用魔数作为默认值

### 修复要点

1. 实现 `first_merge_phase()` - 执行普通 merge 并记录最小得分
2. 实现 `second_merge_phase()` - 利用 BM25 加速并进行过滤
3. 修复 `check_top_k()` - 正确提取 LIMIT 值
4. 在 `inner_get_next_row/rows()` 中集成两阶段优化
5. 实现辅助方法 - 得分提取、过滤、补充、排序

### 预期效果

- ✓ 召回率恢复到正确水平
- ✓ 两阶段优化正确执行
- ✓ 性能得到提升
- ✓ 返回结果按相关性排序


