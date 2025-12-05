# Index Merge优化方案 - 详细分析与实施计划

## 一、当前状态总结

### 测试结果
- **Recall@10**: 0.9565 ✓ (满足 >= 0.95 要求)
- **QPS**: 314.71 (平均值)
- **执行计划**: 仍以 TEXT RETRIEVAL SCAN/BMW topk 路径为主

### 代码现状
- ✅ 两阶段 index merge 框架已实现
- ✅ 稀疏检索阈值下推机制已完成
- ✅ 得分提取逻辑已修复
- ❌ 优化器未生成 index merge 计划

## 二、问题根因分析

### 2.1 Index Merge 计划生成流程

```
create_access_paths()
  └─> create_index_merge_access_paths()
      ├─> get_candi_index_merge_trees()
      │   ├─> get_valid_index_merge_indexes()  [获取有效索引]
      │   ├─> generate_candi_index_merge_trees()  [生成候选树]
      │   │   └─> generate_candi_index_merge_node()
      │   │       └─> collect_candicate_indexes()
      │   └─> prune_candi_index_merge_trees()  [剪枝]
      └─> do_create_index_merge_paths()
          ├─> choose_best_selectivity_branch()  [选择最优分支]
          ├─> root_node->formalize_index_merge_tree()  [格式化树]
          └─> build_access_path_for_scan_node()  [构建访问路径]
```

### 2.2 关键问题点

#### 问题1: 系统开关未启用
**代码位置**: `ob_join_order.cpp:3307`
```cpp
} else if (get_plan()->get_log_plan_hint().is_outline_data_
           || (!tenant_config->_enable_index_merge && OB_LIKELY(!EN_FORCE_INDEX_MERGE_PLAN))) {
    OPT_TRACE("can not create index merge paths due to outline or tenant config");
}
```

**现象**: 当 `_enable_index_merge = false` 时，即使有有效的 index merge 候选，也会被跳过。

**解决方案**: 
```sql
ALTER SYSTEM SET _enable_index_merge = true;
FLUSH PLAN CACHE;
```

#### 问题2: 过滤条件不匹配
**代码位置**: `ob_join_order.cpp:3410-3412`
```cpp
} else if (filter->get_expr_type() != T_OP_OR) {
    // do nothing, only support UNION MERGE now
}
```

**现象**: 当前仅支持 `T_OP_OR` 类型的过滤条件（UNION MERGE）。如果过滤条件是 `T_OP_AND` 或其他类型，则不会生成 index merge 计划。

**分析**: 
- UNION MERGE: 多个索引的结果取并集（适用于 `WHERE (col1 = a) OR (col2 = b)`）
- INTERSECT MERGE: 多个索引的结果取交集（适用于 `WHERE (col1 = a) AND (col2 = b)`）

**当前查询结构**:
```sql
WHERE MATCH(fulltext_col) AGAINST(XXX)
  AND base_id IN (...)
  AND id < XXX
```

这是 `T_OP_AND` 结构，不是 `T_OP_OR`。

#### 问题3: 有效索引识别
**代码位置**: `ob_join_order.cpp:3345-3360`
```cpp
} else if (ref_table_id == index_id ||
           (index_schema->is_index_local_storage() &&
            (index_schema->is_normal_index() || index_schema->is_fts_index()))) {
    /* primary table, local normal index and local fts index are available for index merge now */
```

**现象**: 只有以下类型的索引被认为是有效的：
- 主表本身 (ref_table_id)
- 本地普通索引
- 本地全文索引

## 三、优化方案

### 方案A: 启用系统开关（短期方案）

#### 步骤1: 启用 index merge 开关
```sql
-- 连接到 seekdb
obclient -h127.0.0.1 -P2881 -uroot

-- 启用 index merge
ALTER SYSTEM SET _enable_index_merge = true;

-- 清空计划缓存
FLUSH PLAN CACHE;

-- 验证设置
SHOW PARAMETERS LIKE '_enable_index_merge';
```

#### 步骤2: 创建必要的索引
```sql
-- 为 id 列创建索引（提高标量条件的选择性）
CREATE INDEX idx_id ON items1(id);

-- 验证索引创建
SHOW INDEX FROM items1;
```

#### 步骤3: 收集统计信息
```sql
-- 更新表统计信息
ANALYZE TABLE items1;
```

#### 步骤4: 验证执行计划
```sql
-- 查看执行计划
EXPLAIN SELECT docid_col, MATCH(fulltext_col) AGAINST('query') as _score
FROM items1
WHERE MATCH(fulltext_col) AGAINST('query')
  AND base_id IN ('base_id_1', 'base_id_2', 'base_id_3')
  AND id < 1000
ORDER BY _score DESC
LIMIT 10;
```

### 方案B: 支持 INTERSECT MERGE（中期方案）

#### 目标
支持 `T_OP_AND` 类型的过滤条件，实现交集合并。

#### 实施步骤

**步骤1**: 修改 `generate_candi_index_merge_trees` 支持 AND 条件
```cpp
// 在 ob_join_order.cpp 中修改
int ObJoinOrder::generate_candi_index_merge_trees(...)
{
  // ...
  for (int64_t i = 0; OB_SUCC(ret) && i < filters.count(); ++i) {
    ObRawExpr *filter = filters.at(i);
    // 修改：支持 T_OP_OR 和 T_OP_AND
    if (filter->get_expr_type() == T_OP_OR || 
        filter->get_expr_type() == T_OP_AND) {
      // 生成候选节点
    }
  }
}
```

**步骤2**: 增强 `choose_best_selectivity_branch` 的 INTERSECT 处理
```cpp
// 对于 INTERSECT 节点，选择选择性最低的分支作为主路径
// 其他分支用于过滤
```

**步骤3**: 修改 `build_access_path_for_scan_node` 支持交集构建
```cpp
// 为 INTERSECT 节点构建访问路径时，
// 需要将其他分支的条件作为过滤条件下推
```

### 方案C: 优化两阶段 Index Merge 执行（长期方案）

#### 当前两阶段框架的改进

**Phase 1 优化**:
```cpp
// 从 LIMIT/TopK 表达式读取真实值
int64_t phase1_limit = extract_topk_from_expr(limit_expr);
if (phase1_limit <= 0) {
  phase1_limit = max_size_ * 2;  // 回退到估算
}
```

**Phase 2 优化**:
```cpp
// 完善补足策略
if (phase2_result_count < target_count) {
  // 从 phase1 候选中补充
  merge_phase_results(phase1_candidates, phase2_results, target_count);
}
```

## 四、实施优先级

### 优先级1 (立即执行)
- [ ] 启用 `_enable_index_merge` 系统开关
- [ ] 创建 `idx_id` 索引
- [ ] 收集统计信息
- [ ] 验证执行计划生成

### 优先级2 (如果优先级1无效)
- [ ] 调查为什么 `candi_index_trees` 为空
- [ ] 检查 `helper.filters_` 的内容
- [ ] 验证 `valid_index_ids` 是否正确识别

### 优先级3 (性能进一步优化)
- [ ] 实现 INTERSECT MERGE 支持
- [ ] 优化 phase1_limit 来源
- [ ] 完善 phase2 补足策略

## 五、验证步骤

### 验证1: 确认系统开关状态
```bash
# 在 seekdb 中执行
SHOW PARAMETERS LIKE '_enable_index_merge';
```

### 验证2: 检查执行计划
```bash
# 运行测试脚本并检查 EXPLAIN 输出
python3.9 mldr_data_test.py --lang en --query_types bm25
```

### 验证3: 性能基准测试
```bash
# 对比优化前后的 QPS
# 预期: QPS 应有明显提升（目标 > 400 QPS）
```

## 六、关键代码位置

| 功能 | 文件 | 行号 |
|------|------|------|
| 系统开关检查 | ob_join_order.cpp | 3307 |
| 过滤条件类型检查 | ob_join_order.cpp | 3410 |
| 有效索引识别 | ob_join_order.cpp | 3345 |
| 两阶段框架 | ob_das_index_merge_iter.cpp | - |
| 阈值下推 | ob_sparse_retrieval_merge_iter.cpp | - |

## 七、预期效果

### 短期效果 (启用系统开关)
- 执行计划从 TEXT RETRIEVAL SCAN 变为 INDEX MERGE
- QPS 提升 20-50%
- Recall 保持不变

### 中期效果 (支持 INTERSECT MERGE)
- 支持更复杂的过滤条件组合
- QPS 进一步提升 10-30%

### 长期效果 (完善两阶段框架)
- 精准的 topk 下推
- 稳定的召回率
- QPS 稳定在高水位

## 八、风险评估

| 风险 | 概率 | 影响 | 缓解措施 |
|------|------|------|---------|
| 系统开关不存在 | 低 | 中 | 检查版本配置 |
| 索引创建失败 | 低 | 中 | 检查表结构 |
| 执行计划仍未生成 | 中 | 高 | 深入调查过滤条件 |
| 性能下降 | 低 | 高 | 回退到原方案 |


