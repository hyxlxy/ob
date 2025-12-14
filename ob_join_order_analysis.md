# OceanBase ob_join_order.cpp 完整分析文档

## 文件概述

**文件路径**: `/root/ob/src/sql/optimizer/ob_join_order.cpp`  
**文件大小**: 20,849 行  
**核心功能**: SQL查询优化器中的连接顺序优化和访问路径选择

这是OceanBase数据库SQL优化器的核心模块，负责：
1. 生成和评估表的访问路径（Access Paths）
2. 优化多表连接的执行顺序
3. 选择最优的索引和扫描方式
4. 计算查询成本并进行路径剪枝

---

## 核心类和数据结构

### 1. **Path 类（基类）**
```cpp
class Path {
  // 基础属性
  ObJoinOrder* parent_;           // 父连接顺序节点
  bool is_local_order_;           // 是否本地有序
  ObSEArray<OrderItem> ordering_; // 输出顺序
  double cost_;                   // 路径成本
  double op_cost_;                // 操作成本
  
  // 分片信息
  ObShardingInfo *strong_sharding_;      // 强分片信息
  ObSEArray<ObShardingInfo*> weak_sharding_; // 弱分片信息
  
  // 并行信息
  int64_t parallel_;              // 并行度
  int64_t server_cnt_;            // 服务器数量
  ObSEArray<ObAddr> server_list_; // 服务器列表
  
  // 其他属性
  bool is_pipelined_path_;        // 是否流水线路径
  bool contain_fake_cte_;         // 是否包含假CTE
  bool contain_das_op_;           // 是否包含DAS操作
};
```

**关键方法**:
- `estimate_cost()` - 估算路径成本（纯虚函数）
- `re_estimate_cost()` - 重新估算成本
- `compute_pipeline_info()` - 计算流水线信息
- `get_sharding()` - 获取分片信息

### 2. **AccessPath 类（表扫描路径）**
```cpp
class AccessPath : public Path {
  uint64_t table_id_;             // 表ID
  uint64_t ref_table_id_;         // 参考表ID
  uint64_t index_id_;             // 索引ID
  bool is_global_index_;          // 是否全局索引
  bool use_das_;                  // 是否使用DAS
  
  ObCostTableScanInfo est_cost_info_;  // 成本估算信息
  ObOrderDirection order_direction_;   // 扫描方向
  bool is_get_;                   // 是否为GET操作
  bool is_hash_index_;            // 是否为哈希索引
  
  OptSkipScanState use_skip_scan_; // 跳跃扫描状态
  bool use_column_store_;         // 是否使用列存
  bool can_batch_rescan_;         // 是否可以批量重扫
  
  DomainIndexAccessInfo domain_idx_info_;  // 域索引信息
  VecIndexAccessInfo vec_idx_info_;       // 向量索引信息
};
```

**关键方法**:
- `estimate_cost()` - 估算表扫描成本
- `compute_parallel_degree()` - 计算并行度
- `compute_access_path_batch_rescan()` - 计算批量重扫
- `compute_is_das_dynamic_part_pruning()` - 计算DAS动态分区剪枝

### 3. **JoinPath 类（连接路径）**
```cpp
class JoinPath : public Path {
  // 连接的左右子树
  Path *left_path_;
  Path *right_path_;
  
  // 连接条件
  ObJoinType join_type_;
  ObSEArray<ObRawExpr*> join_quals_;
  
  // 连接成本信息
  double left_output_rows_;
  double right_output_rows_;
  double join_output_rows_;
  
  // 连接方法
  ObJoinAlgo join_algo_;  // NLJ, MJ, HJ等
};
```

**关键方法**:
- `estimate_cost()` - 估算连接成本
- `cost_nest_loop_join()` - 嵌套循环连接成本
- `cost_merge_join()` - 归并连接成本
- `cost_hash_join()` - 哈希连接成本
- `compute_join_path_sharding()` - 计算连接分片
- `compute_join_path_ordering()` - 计算连接输出顺序

### 4. **ObIndexMergeNode 类（索引合并树节点）**
```cpp
class ObIndexMergeNode {
  ObIndexMergeType merge_type_;   // 合并类型（UNION/INTERSECT）
  uint64_t index_id_;             // 索引ID
  
  // 子节点
  ObSEArray<ObIndexMergeNode*> children_;
  
  // 谓词信息
  ObSEArray<ObRawExpr*> range_exprs_;
  ObSEArray<ObRawExpr*> filter_exprs_;
  
  double selectivity_;            // 选择率
};
```

### 5. **IndexMergePath 类（索引合并路径）**
继承自AccessPath，表示使用索引合并的访问路径。

### 6. **其他路径类**
- `FunctionTablePath` - 函数表路径
- `JsonTablePath` - JSON表路径
- `TempTablePath` - 临时表路径
- `CteTablePath` - CTE表路径
- `SubQueryPath` - 子查询路径

---

## 主要功能模块

### 1. **访问路径生成模块**

#### 1.1 `create_access_paths()` (行3049)
**功能**: 为单个表生成所有可能的访问路径
**流程**:
```
create_access_paths()
  ├─ 获取表的所有有效索引
  ├─ 对每个索引创建访问路径
  │  ├─ create_one_access_path()
  │  ├─ 提取查询范围
  │  ├─ 计算成本
  │  └─ 进行路径剪枝
  ├─ 创建索引合并路径
  │  └─ create_index_merge_access_paths()
  └─ 返回最终的访问路径列表
```

#### 1.2 `create_one_access_path()` (行1755)
**功能**: 为单个索引创建一条访问路径
**关键步骤**:
1. 初始化AccessPath对象
2. 设置索引信息
3. 提取查询范围（Query Range）
4. 计算索引扫描方向
5. 添加访问过滤条件
6. 初始化成本估算信息
7. 估算行数和成本

#### 1.3 `create_index_merge_access_paths()` (行3217)
**功能**: 创建索引合并访问路径
**支持的合并类型**:
- `INDEX_MERGE_UNION` - 多个索引的并集
- `INDEX_MERGE_INTERSECT` - 多个索引的交集
- `INDEX_MERGE_SCAN` - 扫描合并

**流程**:
```
create_index_merge_access_paths()
  ├─ 获取候选索引合并树
  ├─ 对每棵树生成合并路径
  │  ├─ 为每个索引创建扫描路径
  │  ├─ 计算合并成本
  │  └─ 创建IndexMergePath
  └─ 进行路径剪枝
```

### 2. **查询范围提取模块**

#### 2.1 `check_and_extract_query_range()` (行2496)
**功能**: 检查并提取索引的查询范围
**处理的范围类型**:
- 等值范围 (=)
- IN范围
- 范围条件 (<, >, <=, >=)
- 前缀范围

#### 2.2 `extract_preliminary_query_range()` (行5412)
**功能**: 提取初步查询范围
**支持的索引类型**:
- 普通索引
- 地理索引 (GIS Index)
- 多值索引 (Multivalue Index)
- 全文索引 (FTS Index)

#### 2.3 特殊索引范围提取
- `extract_geo_preliminary_query_range()` (行5548) - 地理索引
- `extract_multivalue_preliminary_query_range()` (行5591) - 多值索引
- `extract_fts_preliminary_query_range()` (行5641) - 全文索引

### 3. **成本估算模块**

#### 3.1 `estimate_cost()` 系列方法

**AccessPath::estimate_cost()** (行7828)
```cpp
// 估算表扫描成本
// 包括：
// 1. 索引扫描成本
// 2. 回表成本（如果需要）
// 3. 过滤成本
// 4. 总体成本
```

**JoinPath::estimate_cost()** (行9355)
```cpp
// 估算连接成本
// 1. 获取左右子树的成本
// 2. 根据连接类型选择成本计算方法
// 3. 计算连接输出行数
// 4. 返回总成本
```

#### 3.2 连接成本计算

**`cost_nest_loop_join()`** (行9637)
```
成本 = 左表成本 + 左表行数 × (右表成本 + 右表行数 × 过滤成本)
```

**`cost_merge_join()`** (行9771)
```
成本 = 左表排序成本 + 右表排序成本 + 扫描成本 + 比较成本
```

**`cost_hash_join()`** (行9878)
```
成本 = 左表成本 + 右表成本 + 哈希表构建成本 + 探测成本
```

#### 3.3 `re_estimate_cost()` (行7878, 9390)
**功能**: 在运行时重新估算成本
**应用场景**:
- 动态参数值已知
- 需要调整并行度
- 需要考虑批量重扫

### 4. **路径剪枝模块**

#### 4.1 `compute_cost_and_prune_access_path()` (行4734)
**功能**: 计算成本并剪枝访问路径
**剪枝策略**:
1. **成本剪枝**: 移除成本过高的路径
2. **支配剪枝**: 移除被其他路径完全支配的路径
3. **天际线剪枝**: 保留Pareto最优路径

#### 4.2 `skyline_prunning_index()` (行2732)
**功能**: 使用天际线算法剪枝索引
**维度**:
- 成本
- 行数
- 排序属性

#### 4.3 `prune_index_merge_path()` (行4189)
**功能**: 剪枝索引合并路径

### 5. **分片信息计算模块**

#### 5.1 `compute_sharding_info_for_base_paths()` (行301)
**功能**: 为基表路径计算分片信息
**包括**:
- 表的分片类型
- 分片键
- 分区信息

#### 5.2 `compute_sharding_info_with_part_info()` (行480)
**功能**: 基于分区信息计算分片

#### 5.3 `set_sharding_info_for_base_path()` (行380)
**功能**: 为基表路径设置分片信息

### 6. **并行度计算模块**

#### 6.1 `compute_parallel_and_server_info_for_base_paths()` (行668)
**功能**: 为基表路径计算并行度和服务器信息
**流程**:
```
compute_parallel_and_server_info_for_base_paths()
  ├─ 获取表的DOP（Degree of Parallelism）
  ├─ 检查并行规则
  ├─ 计算可用的并行度
  ├─ 获取执行服务器列表
  └─ 设置到路径中
```

#### 6.2 `compute_base_table_parallel_and_server_info()` (行851)
**功能**: 计算基表的并行度和服务器信息

#### 6.3 `get_base_path_table_dop()` (行708)
**功能**: 获取基表的DOP

#### 6.4 `get_random_parallel()` (行732)
**功能**: 获取随机并行度

### 7. **索引合并树生成模块**

#### 7.1 `get_candi_index_merge_trees()` (行3272)
**功能**: 获取候选索引合并树

#### 7.2 `generate_candi_index_merge_trees()` (行3403)
**功能**: 生成候选索引合并树
**算法**:
1. 收集所有候选索引
2. 根据谓词树构建合并树
3. 进行树的规范化

#### 7.3 `generate_candi_index_merge_node()` (行3508)
**功能**: 生成单个合并树节点

#### 7.4 `collect_candicate_indexes()` (行3588)
**功能**: 收集候选索引

### 8. **向量索引支持模块**

#### 8.1 `process_vec_index_info()` (行1605)
**功能**: 处理向量索引信息
**支持**:
- 向量索引扫描
- 向量索引合并
- 向量距离计算

#### 8.2 `check_can_use_vec_primary_opt()` (行1538)
**功能**: 检查是否可以使用向量主索引优化

#### 8.3 `process_basic_vec_info_for_index_merge_node()` (行1551)
**功能**: 处理索引合并节点的向量信息

### 9. **特殊索引支持模块**

#### 9.1 全文索引 (FTS - Full Text Search)
- `extract_fts_preliminary_query_range()` - 提取FTS范围
- `check_index_merge_paths_contain_fts()` - 检查合并路径是否包含FTS

#### 9.2 地理索引 (GIS)
- `extract_geo_schema_info()` - 提取地理模式信息
- `extract_geo_preliminary_query_range()` - 提取地理范围
- `check_exprs_overlap_gis_index()` - 检查表达式与GIS索引重叠

#### 9.3 多值索引
- `extract_multivalue_preliminary_query_range()` - 提取多值范围
- `check_exprs_overlap_multivalue_index()` - 检查多值索引重叠

### 10. **跳跃扫描模块**

#### 10.1 `will_use_skip_scan()` (行4413)
**功能**: 判断是否使用跳跃扫描
**条件**:
- 索引的前导列有IN条件
- 后续列有范围条件
- 跳跃扫描成本更低

### 11. **DAS（Distributed Access Service）模块**

#### 11.1 `will_use_das()` (行1377)
**功能**: 判断是否使用DAS
**检查**:
- `check_exec_force_use_das()` - 执行强制DAS
- `check_opt_rule_use_das()` - 优化规则DAS
- `check_use_das_by_false_startup_filter()` - 虚假启动过滤DAS

#### 11.2 `compute_is_das_dynamic_part_pruning()` (行8203)
**功能**: 计算DAS动态分区剪枝

### 12. **连接顺序优化模块**

#### 12.1 `add_path()` (行6706)
**功能**: 向连接顺序添加路径
**逻辑**:
1. 检查路径有效性
2. 与现有路径比较
3. 进行路径剪枝
4. 添加到路径列表

#### 12.2 `compute_path_relationship()` (行6849)
**功能**: 计算两条路径的关系
**关系类型**:
- `SAME_COST` - 相同成本
- `FIRST_BETTER` - 第一条更优
- `SECOND_BETTER` - 第二条更优
- `UNCOMPARABLE` - 无法比较（Pareto前沿）

#### 12.3 `compute_join_path_relationship()` (行7039)
**功能**: 计算连接路径的关系

### 13. **批量重扫模块**

#### 13.1 `compute_access_path_batch_rescan()` (行8107)
**功能**: 计算访问路径的批量重扫
**应用场景**:
- 嵌套循环连接中的内表
- 需要多次扫描同一表

#### 13.2 `need_compare_batch_rescan()` (行7136)
**功能**: 判断是否需要比较批量重扫

#### 13.3 `compute_nlj_batch_rescan()` (行9155)
**功能**: 计算嵌套循环连接的批量重扫

### 14. **有序性处理模块**

#### 14.1 `get_access_path_ordering()` (行2236)
**功能**: 获取访问路径的输出顺序
**返回**:
- 索引扫描的自然顺序
- 排序后的顺序

#### 14.2 `compute_base_table_path_ordering()` (行582)
**功能**: 计算基表路径的有序性

#### 14.3 `get_index_scan_direction()` (行2293)
**功能**: 获取索引扫描方向（升序/降序）

#### 14.4 `compute_join_path_ordering()` (行8693)
**功能**: 计算连接路径的输出顺序

### 15. **有趣顺序匹配模块**

#### 15.1 `check_all_interesting_order()` (行5002, 5045)
**功能**: 检查路径是否满足有趣顺序
**有趣顺序来源**:
- ORDER BY 子句
- GROUP BY 子句
- JOIN 条件

#### 15.2 `is_join_match()` (行5103, 5167)
**功能**: 检查路径是否与JOIN顺序匹配

### 16. **表达式重叠检查模块**

#### 16.1 `check_expr_overlap_index()` (行5219)
**功能**: 检查表达式是否与索引重叠

#### 16.2 `check_exprs_overlap_index()` (行5293)
**功能**: 检查多个表达式与索引重叠

#### 16.3 `check_exprs_overlap_gis_index()` (行5321)
**功能**: 检查表达式与GIS索引重叠

#### 16.4 `check_exprs_overlap_multivalue_index()` (行5363)
**功能**: 检查表达式与多值索引重叠

### 17. **列存储支持模块**

#### 17.1 `init_column_store_est_info()` (行2008)
**功能**: 初始化列存储估算信息

#### 17.2 `init_column_store_est_info_with_filter()` (行2094)
**功能**: 使用过滤条件初始化列存储估算

#### 17.3 `init_column_store_est_info_with_other_column()` (行2190)
**功能**: 使用其他列初始化列存储估算

---

## 关键算法和策略

### 1. **天际线剪枝算法（Skyline Pruning）**

用于在多维空间中找到Pareto最优的路径。

**维度**:
- 成本（Cost）
- 行数（Row Count）
- 排序属性（Ordering）

**算法流程**:
```
1. 对所有路径按成本排序
2. 对每条路径：
   - 检查是否被其他路径支配
   - 如果不被支配，加入结果集
3. 返回Pareto前沿路径
```

### 2. **成本模型**

**表扫描成本**:
```
cost = index_scan_cost + index_back_cost + filter_cost
```

**连接成本**:
```
NLJ: cost = left_cost + left_rows × (right_cost + right_rows × filter_cost)
MJ:  cost = left_sort_cost + right_sort_cost + scan_cost + compare_cost
HJ:  cost = left_cost + right_cost + hash_build_cost + probe_cost
```

### 3. **并行度计算**

**DOP（Degree of Parallelism）计算**:
```
DOP = min(
  表的DOP,
  服务器数量,
  分区数量,
  用户指定的DOP
)
```

### 4. **分片信息计算**

**分片类型**:
- `UNSHARDED` - 未分片
- `SHARDED` - 分片
- `DISTRIBUTED` - 分布式
- `MATCH_ALL` - 匹配所有
- `LOCAL` - 本地

### 5. **索引合并策略**

**UNION合并**:
```
结果 = 索引1的结果 ∪ 索引2的结果 ∪ ...
成本 = 各索引成本之和 + 去重成本
```

**INTERSECT合并**:
```
结果 = 索引1的结果 ∩ 索引2的结果 ∩ ...
成本 = 各索引成本之和 + 交集成本
```

---

## 数据流和调用关系

### 完整的访问路径生成流程

```
ObJoinOrder::generate_base_paths()
  │
  ├─ generate_normal_base_table_paths()
  │  │
  │  └─ generate_base_table_paths(PathHelper &helper)
  │     │
  │     ├─ create_access_paths()
  │     │  │
  │     │  ├─ get_valid_index_ids()
  │     │  │
  │     │  ├─ for each index:
  │     │  │  │
  │     │  │  ├─ create_one_access_path()
  │     │  │  │  │
  │     │  │  │  ├─ check_and_extract_query_range()
  │     │  │  │  ├─ get_access_path_ordering()
  │     │  │  │  ├─ add_access_filters()
  │     │  │  │  ├─ init_filter_selectivity()
  │     │  │  │  └─ estimate_cost()
  │     │  │  │
  │     │  │  └─ add_path()
  │     │  │
  │     │  └─ create_index_merge_access_paths()
  │     │     │
  │     │     ├─ get_candi_index_merge_trees()
  │     │     ├─ for each merge tree:
  │     │     │  │
  │     │     │  ├─ build_access_path_for_scan_node()
  │     │     │  └─ create_one_index_merge_path()
  │     │     │
  │     │     └─ prune_index_merge_path()
  │     │
  │     ├─ compute_sharding_info_for_base_paths()
  │     ├─ compute_parallel_and_server_info_for_base_paths()
  │     └─ compute_cost_and_prune_access_path()
  │
  ├─ generate_json_table_paths()
  ├─ generate_function_table_paths()
  └─ generate_cte_table_paths()
```

### 连接路径生成流程

```
ObJoinOrder::generate_join_paths()
  │
  ├─ for each pair of join orders:
  │  │
  │  ├─ for each left path:
  │  │  │
  │  │  └─ for each right path:
  │  │     │
  │  │     ├─ for each join type:
  │  │     │  │
  │  │     │  ├─ for each join method (NLJ, MJ, HJ):
  │  │     │  │  │
  │  │     │  │  ├─ create_join_path()
  │  │     │  │  ├─ compute_join_path_sharding()
  │  │     │  │  ├─ compute_join_path_ordering()
  │  │     │  │  ├─ compute_join_path_parallel_and_server_info()
  │  │     │  │  ├─ estimate_cost()
  │  │     │  │  │  ├─ cost_nest_loop_join()
  │  │     │  │  │  ├─ cost_merge_join()
  │  │     │  │  │  └─ cost_hash_join()
  │  │     │  │  │
  │  │     │  │  └─ add_path()
  │  │     │  │
  │  │     │  └─ compute_path_relationship()
  │  │     │
  │  │     └─ compute_cost_and_prune_access_path()
  │  │
  │  └─ compute_join_path_relationship()
  │
  └─ return best paths
```

---

## 重要的数据结构

### 1. **ObCostTableScanInfo** - 表扫描成本信息
```cpp
struct ObCostTableScanInfo {
  ObTableMetaInfo *table_meta_info_;
  ObNewRange ranges_;              // 查询范围
  ObNewRange ss_ranges_;           // 跳跃扫描范围
  double output_row_count_;        // 输出行数
  double logical_query_range_row_count_;  // 逻辑查询范围行数
  double phy_query_range_row_count_;      // 物理查询范围行数
  double index_back_row_count_;    // 回表行数
  // ... 其他成本相关字段
};
```

### 2. **ObShardingInfo** - 分片信息
```cpp
class ObShardingInfo {
  ObTableLocationType location_type_;      // 位置类型
  ObPhyTableLocationInfo *phy_table_location_info_;  // 物理表位置
  ObSEArray<ObRawExpr*> partition_keys_;   // 分区键
  // ... 其他分片相关字段
};
```

### 3. **OrderItem** - 排序项
```cpp
struct OrderItem {
  ObRawExpr *expr_;                // 表达式
  bool is_ascending_;              // 是否升序
  // ... 其他字段
};
```

### 4. **JoinFilterInfo** - 连接过滤信息
```cpp
struct JoinFilterInfo {
  ObSEArray<ObRawExpr*> lexprs_;   // 左连接键
  ObSEArray<ObRawExpr*> rexprs_;   // 右连接键
  ObShardingInfo *sharding_;       // 分片信息
  double row_count_;               // 行数
  double join_filter_selectivity_; // 连接过滤选择率
  // ... 其他字段
};
```

---

## 关键优化技术

### 1. **索引选择优化**
- 支持多种索引类型（B树、哈希、全文、地理、向量等）
- 自动选择最优索引
- 支持索引合并

### 2. **成本估算优化**
- 基于统计信息的精确成本估算
- 支持动态参数的重新估算
- 考虑并行执行的成本

### 3. **并行执行优化**
- 自动计算最优并行度
- 支持分区级并行
- 支持批量重扫优化

### 4. **分片感知优化**
- 计算表的分片信息
- 支持分区级连接
- 支持分布式执行

### 5. **有序性保留**
- 识别索引提供的有序性
- 利用有序性避免排序
- 支持有趣顺序匹配

### 6. **特殊索引支持**
- 全文索引优化
- 地理索引优化
- 向量索引优化
- 多值索引优化

---

## 性能考虑

### 1. **路径数量控制**
- 使用天际线剪枝限制路径数量
- 避免路径爆炸

### 2. **成本计算优化**
- 缓存中间结果
- 避免重复计算
- 使用近似算法

### 3. **内存管理**
- 使用对象池减少分配
- 及时释放不需要的对象

### 4. **并行度选择**
- 考虑CPU数量
- 考虑数据量大小
- 考虑内存限制

---

## 调试和日志

### 日志前缀
```cpp
#define USING_LOG_PREFIX SQL_JO
```

### 常见日志消息
- `failed to create access path` - 访问路径创建失败
- `failed to estimate cost` - 成本估算失败
- `failed to compute sharding info` - 分片信息计算失败
- `failed to compute parallel` - 并行度计算失败

---

## 总结

`ob_join_order.cpp` 是OceanBase SQL优化器的核心模块，实现了：

1. **访问路径生成** - 为表生成所有可能的扫描方式
2. **成本估算** - 精确估算每条路径的执行成本
3. **路径剪枝** - 使用天际线算法保留最优路径
4. **连接优化** - 选择最优的连接顺序和连接方法
5. **并行优化** - 计算最优的并行度和执行策略
6. **特殊索引支持** - 支持多种高级索引类型

该模块的优化质量直接影响SQL查询的执行效率，是数据库性能的关键因素。

