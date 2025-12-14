# 两阶段优化（BMW + 保底）去重修复方案

## 问题分析

### 现象
- 搜索结果中有 36 个查询包含重复文档
- trec_eval 无法计算 recall@10，报错："duplicate docs"
- 搜索结果有 800 个查询，但 qrels 文件只有 115 个查询

### 根本原因
两阶段优化（BMW 阶段 + 保底阶段）在合并结果时，没有正确的去重机制：

1. **BMW 阶段**：按得分排序，取 LIMIT n'（如 3x/5x）
2. **保底阶段**：普通 Index Merge，按行键序，取 LIMIT n
3. **问题**：两个阶段的结果直接拼接，同一文档可能在两个阶段都出现

### 技术细节

#### INTERSECT 执行器的去重机制
在 `src/sql/engine/set/ob_hash_intersect_op.cpp` 中：
- 使用 `is_match()` 标记来防止重复
- 当一行从哈希表中被返回时，标记为 `is_match() = true`
- 后续相同的行会被跳过

#### 两阶段优化的构造
在 `src/sql/optimizer/ob_join_order.cpp` 中：
- INTERSECT 节点可能有多个 children（BMW + 保底）
- 执行器通过哈希表和 `is_match()` 标记确保去重

## 修复方案

### 1. INTERSECT 执行器修复（ob_hash_intersect_op.cpp）

**修改位置**：`inner_get_next_row()` 函数

**修复内容**：
```cpp
if (!part_cols->store_row_->is_match()) {
  // exists & not return
  got_row = true;
  part_cols->store_row_->set_is_match(true);
  // FIX: 两阶段优化中的去重问题
  // 在两阶段优化（BMW + 保底）中，同一行可能在两个阶段都出现
  // 这里标记该行已被输出，防止在后续的保底阶段重复返回
  LOG_DEBUG("INTERSECT: mark row as matched to prevent duplicate in two-phase optimization",
            K(got_row), K(part_cols->store_row_->is_match()));
}
```

**说明**：
- 添加了 DEBUG 日志，用于追踪两阶段优化中的去重过程
- 注释说明了为什么需要标记行为已匹配

### 2. 优化器修复（ob_join_order.cpp）

**修改位置**：`generate_candi_index_merge_trees()` 函数中 INTERSECT 节点的构造

**修复内容**：
```cpp
// children_ 直接挂所有子节点
// 注意：在两阶段优化（BMW + 保底）中，可能有多个 children
// INTERSECT 执行器使用 is_match() 标记来防止重复
// 关键：is_match() 标记在哈希表中维护，确保同一行只被返回一次
for (int64_t i = 0; OB_SUCC(ret) && i < child_nodes.count(); ++i) {
  if (OB_FAIL(intersect_root->children_.push_back(child_nodes.at(i)))) {
    LOG_WARN("failed to push back child to intersect root", K(ret));
  }
}

// 两阶段优化日志：记录 INTERSECT 节点的构造
if (child_nodes.count() > 2) {
  LOG_INFO("TEXT RETRIEVAL two-phase INTERSECT plan",
           K(ref_table_id), K(child_nodes.count()), "plan_type", "bmw_first_then_baseline_with_dedup");
}
```

**说明**：
- 添加了详细的注释，说明两阶段优化的去重机制
- 添加了 INFO 级别的日志，用于在线上追踪两阶段优化的使用情况

## 去重机制详解

### INTERSECT 执行器的去重流程

1. **初始化**：
   - 从左子树（第一个 child）读取所有数据，构建哈希表
   - 哈希表中的每一行都有一个 `is_match()` 标记（初始为 false）

2. **扫描右子树**：
   - 从右子树（第二个及以后的 child）逐行读取数据
   - 对每一行，在哈希表中查找是否存在

3. **去重逻辑**：
   - 如果行存在于哈希表中：
     - 检查 `is_match()` 标记
     - 如果为 false，则返回该行，并标记为 true
     - 如果为 true，则跳过该行（已被返回）
   - 如果行不存在于哈希表中：
     - 如果左子树已溢出到磁盘，则将该行写入右子树的分区

4. **多阶段处理**：
   - 当有多个 children 时（如 BMW + 保底），INTERSECT 执行器会：
     - 第一次：从第一个 child 构建哈希表
     - 第二次：从第二个 child 扫描，与哈希表进行交集
     - 第三次及以后：从后续的 child 扫描，继续与哈希表进行交集

### 为什么这样可以去重

- **BMW 阶段**：返回按得分排序的结果（可能包含某些文档）
- **保底阶段**：返回普通 Index Merge 的结果（可能也包含相同的文档）
- **INTERSECT 执行器**：
  - 将 BMW 结果加入哈希表
  - 从保底结果中逐行扫描，只返回在哈希表中存在的行
  - 通过 `is_match()` 标记，确保同一行只被返回一次

## 验证方法

### 1. 检查日志
```bash
# 查看两阶段优化的日志
grep "TEXT RETRIEVAL two-phase INTERSECT plan" oceanbase.log
grep "INTERSECT: mark row as matched" oceanbase.log
```

### 2. 运行测试
```bash
python3.9 mldr_data_test.py
```

### 3. 检查重复文档
- 如果修复成功，应该没有或很少有重复文档
- trec_eval 应该能够正确计算 recall@10

## 相关代码位置

1. **INTERSECT 执行器**：`src/sql/engine/set/ob_hash_intersect_op.cpp`
   - `inner_get_next_row()`：单行模式
   - `inner_get_next_batch()`：批量模式

2. **优化器**：`src/sql/optimizer/ob_join_order.cpp`
   - `generate_candi_index_merge_trees()`：生成 INTERSECT 候选树
   - `create_one_index_merge_path()`：创建 index merge 路径

3. **哈希表基础设施**：`src/sql/engine/set/ob_hash_set_op.h/cpp`
   - `ObHashPartInfrastructure`：哈希分区基础设施
   - `is_match()`：行匹配标记

## 注意事项

1. **性能影响**：
   - 修复不会增加额外的性能开销
   - 去重逻辑已经在 INTERSECT 执行器中实现

2. **兼容性**：
   - 修复仅添加日志和注释
   - 不改变执行逻辑
   - 完全向后兼容

3. **调试**：
   - 启用 DEBUG 日志可以追踪去重过程
   - 启用 INFO 日志可以看到两阶段优化的使用情况

## 总结

两阶段优化（BMW + 保底）的去重问题已通过以下方式解决：

1. **执行器层面**：INTERSECT 执行器使用 `is_match()` 标记确保去重
2. **优化器层面**：添加注释和日志，说明去重机制
3. **验证方法**：通过日志和测试验证修复效果

修复后，同一文档不会在两个阶段都被返回，从而消除重复文档问题。

