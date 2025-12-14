# OceanBase ob_join_order.cpp 关键函数深度分析

## 1. 访问路径创建函数详解

### 1.1 `create_access_paths()` 详细流程

**位置**: 行3049  
**签名**: `int ObJoinOrder::create_access_paths(const uint64_t table_id, ...)`

**功能**: 为单个表生成所有可能的访问路径（扫描方式）

**详细流程**:

```cpp
1. 初始化
   ├─ 创建PathHelper对象
   ├─ 初始化访问路径数组
   └─ 获取表的元信息

2. 获取有效索引
   ├─ get_valid_index_ids()
   │  ├─ 获取表的所有索引
   │  ├─ 检查索引可用性
   │  ├─ 应用索引提示（USE INDEX, IGNORE INDEX等）
   │  └─ 返回有效索引列表
   │
   └─ 特殊处理
      ├─ 虚拟表启发式处理
      ├─ 非唯一索引剪枝
      └─ 索引子集检查

3. 为每个索引创建访问路径
   ├─ create_one_access_path()
   │  ├─ 初始化AccessPath对象
   │  ├─ 设置索引基本信息
   │  ├─ 提取查询范围
   │  ├─ 计算扫描方向
   │  ├─ 添加访问过滤条件
   │  ├─ 初始化成本估算信息
   │  └─ 估算成本
   │
   └─ add_path()
      ├─ 检查路径有效性
      ├─ 与现有路径比较
      ├─ 进行路径剪枝
      └─ 添加到路径列表

4. 创建索引合并路径
   ├─ create_index_merge_access_paths()
   │  ├─ 获取候选索引合并树
   │  ├─ 为每棵树生成合并路径
   │  └─ 进行路径剪枝
   │
   └─ 可选：向量索引处理
      └─ process_vec_index_info()

5. 计算路径属性
   ├─ compute_sharding_info_for_base_paths()
   │  └─ 为每条路径计算分片信息
   │
   ├─ compute_parallel_and_server_info_for_base_paths()
   │  └─ 为每条路径计算并行度和服务器信息
   │
   └─ compute_cost_and_prune_access_path()
      ├─ 重新计算成本
      ├─ 进行最终路径剪枝
      └─ 返回最优路径集合
```

**关键决策点**:

```cpp
// 1. 索引选择
if (is_virtual_table) {
  // 虚拟表只能使用主键
  use_primary_key_only = true;
} else if (has_index_hint) {
  // 应用用户提示
  use_hinted_indexes = true;
} else {
  // 自动选择所有有效索引
  use_all_valid_indexes = true;
}

// 2. 索引合并决策
if (can_use_index_merge && enable_index_merge) {
  create_index_merge_paths = true;
}

// 3. 特殊索引处理
if (has_fts_index && fts_predicate_exists) {
  process_fts_index = true;
}
if (has_vector_index && vector_predicate_exists) {
  process_vector_index = true;
}
```

---

### 1.2 `create_one_access_path()` 详细实现

**位置**: 行1755  
**签名**: `int ObJoinOrder::create_one_access_path(const uint64_t table_id, ...)`

**功能**: 为单个索引创建一条访问路径

**详细步骤**:

```cpp
Step 1: 初始化AccessPath对象
├─ 分配内存
├─ 设置table_id和ref_table_id
├─ 设置index_id
└─ 初始化所有成员变量

Step 2: 提取查询范围
├─ check_and_extract_query_range()
│  ├─ 分析WHERE子句中的谓词
│  ├─ 识别可以用于索引的谓词
│  ├─ 提取范围条件
│  │  ├─ 等值条件 (col = value)
│  │  ├─ IN条件 (col IN (v1, v2, ...))
│  │  ├─ 范围条件 (col > value, col < value等)
│  │  └─ 前缀条件 (col LIKE 'prefix%')
│  │
│  └─ 计算范围行数
│     ├─ 使用统计信息估算
│     ├─ 考虑多列范围的交集
│     └─ 处理执行参数
│
└─ 设置est_cost_info中的范围信息

Step 3: 计算索引扫描方向
├─ get_index_scan_direction()
│  ├─ 检查ORDER BY子句
│  ├─ 检查GROUP BY子句
│  ├─ 检查JOIN条件
│  ├─ 判断是否可以升序扫描
│  ├─ 判断是否可以降序扫描
│  └─ 选择最优方向
│
└─ 设置order_direction_

Step 4: 添加访问过滤条件
├─ add_access_filters()
│  ├─ 识别不能用于索引的谓词
│  ├─ 这些谓词需要在扫描后进行过滤
│  ├─ 计算过滤选择率
│  └─ 添加到filter_数组
│
└─ 设置path->filter_

Step 5: 初始化成本估算信息
├─ init_filter_selectivity()
│  ├─ 计算过滤条件的选择率
│  ├─ 使用统计信息或默认值
│  └─ 存储在est_cost_info中
│
├─ 初始化列存储信息（如果使用列存储）
│  ├─ init_column_store_est_info()
│  ├─ init_column_store_est_info_with_filter()
│  └─ init_column_store_est_info_with_other_column()
│
└─ 初始化样本扫描信息
   └─ init_sample_info_for_access_path()

Step 6: 估算成本
├─ estimate_cost()
│  ├─ 估算索引扫描成本
│  │  ├─ 根据范围行数和索引大小
│  │  ├─ 考虑缓存命中率
│  │  └─ 计算IO成本
│  │
│  ├─ 估算回表成本（如果需要）
│  │  ├─ 检查是否需要回表
│  │  ├─ 计算回表行数
│  │  └─ 计算回表IO成本
│  │
│  ├─ 估算过滤成本
│  │  ├─ 计算CPU成本
│  │  └─ 考虑过滤选择率
│  │
│  └─ 计算总成本
│     └─ cost = index_cost + back_cost + filter_cost
│
└─ 设置path->cost_

Step 7: 特殊处理
├─ 跳跃扫描检查
│  └─ will_use_skip_scan()
│
├─ DAS检查
│  └─ will_use_das()
│
├─ 向量索引处理
│  └─ process_vec_index_info()
│
└─ 全文索引处理
   └─ extract_fts_preliminary_query_range()

Step 8: 返回AccessPath
└─ 返回创建的路径对象
```

**成本估算公式**:

```cpp
// 基本公式
total_cost = index_scan_cost + index_back_cost + filter_cost

// 索引扫描成本
index_scan_cost = (range_row_count / index_height) * io_cost_per_block

// 回表成本（如果需要）
if (need_index_back) {
  index_back_cost = back_row_count * io_cost_per_row
}

// 过滤成本
filter_cost = output_row_count * cpu_cost_per_row

// 最终成本
final_cost = total_cost / parallel_degree
```

---

## 2. 查询范围提取函数详解

### 2.1 `check_and_extract_query_range()` 详细实现

**位置**: 行2496  
**签名**: `int ObJoinOrder::check_and_extract_query_range(...)`

**功能**: 检查并提取索引的查询范围

**处理流程**:

```cpp
1. 准备工作
   ├─ 获取索引的列信息
   ├─ 获取WHERE子句的谓词
   ├─ 初始化QueryRangeProvider
   └─ 初始化范围数组

2. 范围提取
   ├─ 对于每个索引列
   │  ├─ 查找相关的谓词
   │  ├─ 提取范围条件
   │  │  ├─ 等值条件
   │  │  │  └─ col = value  →  [value, value]
   │  │  │
   │  │  ├─ IN条件
   │  │  │  └─ col IN (v1, v2, v3)  →  [v1, v1], [v2, v2], [v3, v3]
   │  │  │
   │  │  ├─ 范围条件
   │  │  │  ├─ col > value   →  (value, +∞)
   │  │  │  ├─ col >= value  →  [value, +∞)
   │  │  │  ├─ col < value   →  (-∞, value)
   │  │  │  └─ col <= value  →  (-∞, value]
   │  │  │
   │  │  ├─ BETWEEN条件
   │  │  │  └─ col BETWEEN a AND b  →  [a, b]
   │  │  │
   │  │  └─ LIKE条件（仅对字符串列）
   │  │     └─ col LIKE 'prefix%'  →  ['prefix', 'prefix\xFF')
   │  │
   │  └─ 合并多个条件
   │     ├─ 同一列的多个条件取交集
   │     └─ 不同列的条件按索引顺序组合
   │
   └─ 处理执行参数
      ├─ 检查范围中是否包含执行参数
      ├─ 如果包含，标记为动态范围
      └─ 计算参数化范围的行数

3. 范围优化
   ├─ 范围合并
   │  ├─ 合并相邻的范围
   │  └─ 消除重复范围
   │
   ├─ 范围排序
   │  └─ 按起始值排序
   │
   └─ 范围行数计算
      ├─ 使用统计信息
      ├─ 考虑选择率
      └─ 计算最终行数

4. 特殊处理
   ├─ 跳跃扫描范围
   │  └─ will_use_skip_scan()
   │
   ├─ 前缀范围
   │  └─ get_preliminary_prefix_info()
   │
   └─ 虚假范围
      └─ is_false_range()

5. 返回结果
   └─ 设置est_cost_info中的范围信息
```

**范围表示**:

```cpp
// 范围用ObNewRange表示
struct ObNewRange {
  ObRowkey start_key_;      // 起始键
  ObRowkey end_key_;        // 结束键
  bool border_flag_;        // 边界标志
  
  // 边界标志含义：
  // INCLUSIVE_START | INCLUSIVE_END  →  [start, end]
  // INCLUSIVE_START | EXCLUSIVE_END  →  [start, end)
  // EXCLUSIVE_START | INCLUSIVE_END  →  (start, end]
  // EXCLUSIVE_START | EXCLUSIVE_END  →  (start, end)
};
```

**范围行数估算**:

```cpp
// 基本公式
range_row_count = table_row_count * selectivity

// 选择率计算
selectivity = (end_value - start_value) / (max_value - min_value)

// 对于IN条件
selectivity = sum(selectivity_for_each_value)

// 对于多列范围
selectivity = selectivity_col1 * selectivity_col2 * ...
```

---

### 2.2 `extract_preliminary_query_range()` 详细实现

**位置**: 行5412  
**签名**: `int ObJoinOrder::extract_preliminary_query_range(...)`

**功能**: 提取初步查询范围

**处理的索引类型**:

```cpp
1. 普通B树索引
   ├─ 支持所有范围条件
   ├─ 支持前缀匹配
   └─ 支持多列范围

2. 地理索引（GIS）
   ├─ extract_geo_preliminary_query_range()
   ├─ 支持空间范围查询
   └─ 支持距离查询

3. 多值索引
   ├─ extract_multivalue_preliminary_query_range()
   ├─ 支持数组元素查询
   └─ 支持JSON路径查询

4. 全文索引（FTS）
   ├─ extract_fts_preliminary_query_range()
   ├─ 支持关键词查询
   └─ 支持短语查询

5. 向量索引
   ├─ 支持向量距离查询
   └─ 支持向量相似度查询
```

**流程**:

```cpp
1. 识别索引类型
   ├─ 检查索引的元数据
   ├─ 确定索引支持的操作
   └─ 选择相应的处理方法

2. 提取范围
   ├─ 遍历WHERE子句的谓词
   ├─ 识别与索引相关的谓词
   ├─ 根据索引类型提取范围
   └─ 存储在QueryRangeInfo中

3. 优化范围
   ├─ 消除冗余范围
   ├─ 合并相邻范围
   └─ 排序范围

4. 返回结果
   └─ 返回QueryRangeInfo对象
```

---

## 3. 成本估算函数详解

### 3.1 `estimate_cost()` 详细实现

**位置**: 行7828（AccessPath）, 行9355（JoinPath）

**表扫描成本估算**:

```cpp
int AccessPath::estimate_cost() {
  // 1. 获取基本信息
  double table_rows = est_cost_info_.table_meta_info_->table_row_count_;
  double range_rows = est_cost_info_.logical_query_range_row_count_;
  double filter_selectivity = est_cost_info_.filter_selectivity_;
  
  // 2. 估算索引扫描成本
  double index_scan_cost = estimate_index_scan_cost(
    range_rows,
    est_cost_info_.index_meta_info_->index_height_,
    est_cost_info_.index_meta_info_->index_size_
  );
  
  // 3. 估算回表成本（如果需要）
  double back_cost = 0.0;
  if (need_index_back()) {
    double back_rows = range_rows * filter_selectivity;
    back_cost = estimate_index_back_cost(
      back_rows,
      est_cost_info_.table_meta_info_->avg_row_size_
    );
  }
  
  // 4. 估算过滤成本
  double output_rows = range_rows * filter_selectivity;
  double filter_cost = estimate_filter_cost(
    output_rows,
    est_cost_info_.filter_exprs_.count()
  );
  
  // 5. 计算总成本
  double total_cost = index_scan_cost + back_cost + filter_cost;
  
  // 6. 考虑并行执行
  if (parallel_ > 1) {
    total_cost = total_cost / parallel_;
  }
  
  // 7. 设置成本
  cost_ = total_cost;
  op_cost_ = filter_cost;
  
  return OB_SUCCESS;
}
```

**连接成本估算**:

```cpp
int JoinPath::estimate_cost() {
  // 1. 获取左右子树的成本和行数
  double left_cost = left_path_->get_cost();
  double left_rows = left_path_->get_path_output_rows();
  double right_cost = right_path_->get_cost();
  double right_rows = right_path_->get_path_output_rows();
  
  // 2. 根据连接类型选择成本计算方法
  double join_cost = 0.0;
  switch (join_algo_) {
    case ObJoinAlgo::NESTED_LOOP_JOIN:
      join_cost = cost_nest_loop_join(left_rows, right_rows, right_cost);
      break;
    case ObJoinAlgo::MERGE_JOIN:
      join_cost = cost_merge_join(left_rows, right_rows);
      break;
    case ObJoinAlgo::HASH_JOIN:
      join_cost = cost_hash_join(left_rows, right_rows);
      break;
  }
  
  // 3. 计算连接输出行数
  double join_output_rows = estimate_join_output_rows(
    left_rows,
    right_rows,
    join_quals_
  );
  
  // 4. 计算总成本
  double total_cost = left_cost + right_cost + join_cost;
  
  // 5. 考虑并行执行
  if (parallel_ > 1) {
    total_cost = total_cost / parallel_;
  }
  
  // 6. 设置成本
  cost_ = total_cost;
  
  return OB_SUCCESS;
}
```

**嵌套循环连接成本**:

```cpp
// cost_nest_loop_join()
// 成本 = 左表成本 + 左表行数 × (右表成本 + 右表行数 × 过滤成本)

double left_cost = left_path_->get_cost();
double left_rows = left_path_->get_path_output_rows();
double right_cost = right_path_->get_cost();
double right_rows = right_path_->get_path_output_rows();
double filter_cost = estimate_filter_cost(join_quals_.count());

double nlj_cost = left_cost + 
                  left_rows * (right_cost + right_rows * filter_cost);

// 如果右表可以批量重扫
if (can_batch_rescan_) {
  // 批量重扫可以减少右表的扫描次数
  int64_t batch_size = get_batch_size();
  int64_t batch_count = (left_rows + batch_size - 1) / batch_size;
  nlj_cost = left_cost + 
             batch_count * (right_cost + right_rows * filter_cost);
}
```

**归并连接成本**:

```cpp
// cost_merge_join()
// 成本 = 左表排序成本 + 右表排序成本 + 扫描成本 + 比较成本

double left_sort_cost = estimate_sort_cost(left_rows, left_key_size);
double right_sort_cost = estimate_sort_cost(right_rows, right_key_size);
double scan_cost = (left_rows + right_rows) * cpu_cost_per_row;
double compare_cost = (left_rows + right_rows) * cpu_cost_per_compare;

double mj_cost = left_sort_cost + right_sort_cost + scan_cost + compare_cost;

// 如果左右表已经有序，可以省略排序成本
if (left_path_->is_ordered()) {
  mj_cost -= left_sort_cost;
}
if (right_path_->is_ordered()) {
  mj_cost -= right_sort_cost;
}
```

**哈希连接成本**:

```cpp
// cost_hash_join()
// 成本 = 左表成本 + 右表成本 + 哈希表构建成本 + 探测成本

double left_cost = left_path_->get_cost();
double left_rows = left_path_->get_path_output_rows();
double right_cost = right_path_->get_cost();
double right_rows = right_path_->get_path_output_rows();

double hash_build_cost = right_rows * cpu_cost_per_row;
double probe_cost = left_rows * cpu_cost_per_row;
double compare_cost = left_rows * right_rows * cpu_cost_per_compare;

double hj_cost = left_cost + right_cost + 
                 hash_build_cost + probe_cost + compare_cost;

// 如果哈希表可以放在内存中
if (hash_table_size <= available_memory) {
  // 成本不变
} else {
  // 需要溢出到磁盘，增加IO成本
  hj_cost += estimate_spill_cost(hash_table_size);
}
```

---

## 4. 路径剪枝函数详解

### 4.1 `compute_cost_and_prune_access_path()` 详细实现

**位置**: 行4734  
**签名**: `int ObJoinOrder::compute_cost_and_prune_access_path(PathHelper &helper, ...)`

**功能**: 计算成本并剪枝访问路径

**剪枝策略**:

```cpp
1. 成本剪枝
   ├─ 移除成本过高的路径
   ├─ 阈值 = 最优路径成本 × cost_threshold
   └─ 移除所有成本 > 阈值的路径

2. 支配剪枝
   ├─ 如果路径A的成本更低且行数更少
   ├─ 则路径B被路径A支配
   └─ 移除被支配的路径B

3. 天际线剪枝
   ├─ 在多维空间中找到Pareto最优路径
   ├─ 维度：成本、行数、排序属性
   └─ 保留所有非支配路径

4. 有趣顺序保留
   ├─ 保留满足有趣顺序的路径
   ├─ 即使成本较高
   └─ 避免后续排序操作
```

**实现细节**:

```cpp
int ObJoinOrder::compute_cost_and_prune_access_path(PathHelper &helper, ...) {
  // 1. 计算所有路径的成本
  for (int64_t i = 0; i < access_paths_.count(); ++i) {
    AccessPath *path = access_paths_.at(i);
    if (OB_FAIL(path->estimate_cost())) {
      LOG_WARN("failed to estimate cost", K(ret));
    }
  }
  
  // 2. 初始化剪枝候选集
  ObSEArray<AccessPath*> pruned_paths;
  
  // 3. 应用成本剪枝
  double min_cost = DBL_MAX;
  for (int64_t i = 0; i < access_paths_.count(); ++i) {
    min_cost = min(min_cost, access_paths_.at(i)->get_cost());
  }
  double cost_threshold = min_cost * COST_THRESHOLD_RATIO;
  
  for (int64_t i = 0; i < access_paths_.count(); ++i) {
    AccessPath *path = access_paths_.at(i);
    if (path->get_cost() <= cost_threshold) {
      pruned_paths.push_back(path);
    }
  }
  
  // 4. 应用天际线剪枝
  ObSEArray<AccessPath*> skyline_paths;
  if (OB_FAIL(skyline_prunning_index(table_id_, pruned_paths, skyline_paths))) {
    LOG_WARN("failed to do skyline pruning", K(ret));
  }
  
  // 5. 保留有趣顺序路径
  for (int64_t i = 0; i < pruned_paths_.count(); ++i) {
    AccessPath *path = pruned_paths_.at(i);
    if (path->has_interesting_order()) {
      // 检查是否已在skyline_paths中
      bool found = false;
      for (int64_t j = 0; j < skyline_paths.count(); ++j) {
        if (skyline_paths.at(j) == path) {
          found = true;
          break;
        }
      }
      if (!found) {
        skyline_paths.push_back(path);
      }
    }
  }
  
  // 6. 返回最终路径集合
  access_paths_.reset();
  access_paths_.assign(skyline_paths);
  
  return OB_SUCCESS;
}
```

---

## 5. 分片信息计算函数详解

### 5.1 `compute_sharding_info_for_base_paths()` 详细实现

**位置**: 行301  
**签名**: `int ObJoinOrder::compute_sharding_info_for_base_paths(...)`

**功能**: 为基表路径计算分片信息

**分片类型**:

```cpp
enum ObTableLocationType {
  OB_TBL_LOCATION_UNSHARDED = 0,    // 未分片（单表）
  OB_TBL_LOCATION_LOCAL,             // 本地分片
  OB_TBL_LOCATION_REMOTE,            // 远程分片
  OB_TBL_LOCATION_DISTRIBUTED,       // 分布式分片
  OB_TBL_LOCATION_MATCH_ALL,         // 匹配所有分区
};
```

**计算流程**:

```cpp
1. 获取表的分区信息
   ├─ 获取表的分区类型
   ├─ 获取分区数量
   ├─ 获取分区键
   └─ 获取分区位置

2. 确定分片类型
   ├─ 如果表未分片
   │  └─ location_type = OB_TBL_LOCATION_UNSHARDED
   │
   ├─ 如果表分片且所有分区在本地
   │  └─ location_type = OB_TBL_LOCATION_LOCAL
   │
   ├─ 如果表分片且所有分区在同一远程服务器
   │  └─ location_type = OB_TBL_LOCATION_REMOTE
   │
   ├─ 如果表分片且分区分布在多个服务器
   │  └─ location_type = OB_TBL_LOCATION_DISTRIBUTED
   │
   └─ 如果查询条件匹配所有分区
      └─ location_type = OB_TBL_LOCATION_MATCH_ALL

3. 创建ObShardingInfo对象
   ├─ 设置location_type
   ├─ 设置分区键
   ├─ 设置分区位置信息
   └─ 设置其他分片属性

4. 为每条路径设置分片信息
   └─ path->set_strong_sharding(sharding_info)
```

---

## 6. 并行度计算函数详解

### 6.1 `compute_parallel_and_server_info_for_base_paths()` 详细实现

**位置**: 行668  
**签名**: `int ObJoinOrder::compute_parallel_and_server_info_for_base_paths(...)`

**功能**: 为基表路径计算并行度和服务器信息

**并行度计算公式**:

```cpp
DOP = min(
  table_dop,           // 表的DOP
  server_count,        // 服务器数量
  partition_count,     // 分区数量
  user_dop,            // 用户指定的DOP
  available_dop        // 可用DOP
)
```

**计算流程**:

```cpp
1. 获取表的DOP
   ├─ get_base_path_table_dop()
   ├─ 检查表的显式DOP设置
   ├─ 检查优化器参数
   └─ 返回表的DOP

2. 获取服务器信息
   ├─ 获取服务器数量
   ├─ 获取服务器列表
   └─ 计算可用服务器数量

3. 计算最终并行度
   ├─ 获取表的分区数量
   ├─ 计算DOP = min(table_dop, server_count, partition_count)
   └─ 应用用户约束

4. 为每条路径设置并行信息
   ├─ path->set_parallel(dop)
   ├─ path->set_server_list(server_list)
   └─ path->set_server_cnt(server_count)

5. 重新估算成本
   └─ 考虑并行执行的成本
```

---

## 7. 索引合并路径生成函数详解

### 7.1 `create_index_merge_access_paths()` 详细实现

**位置**: 行3217  
**签名**: `int ObJoinOrder::create_index_merge_access_paths(...)`

**功能**: 创建索引合并访问路径

**支持的合并类型**:

```cpp
1. UNION合并
   ├─ 多个索引的并集
   ├─ 适用于OR条件
   ├─ 成本 = 各索引成本之和 + 去重成本
   └─ 行数 = 各索引行数之和 - 重复行数

2. INTERSECT合并
   ├─ 多个索引的交集
   ├─ 适用于AND条件
   ├─ 成本 = 各索引成本之和 + 交集成本
   └─ 行数 = 各索引行数的交集

3. SCAN合并
   ├─ 扫描合并
   ├─ 适用于复杂条件
   └─ 成本 = 全表扫描成本 + 过滤成本
```

**生成流程**:

```cpp
1. 获取候选索引合并树
   ├─ get_candi_index_merge_trees()
   ├─ 分析WHERE子句的谓词树
   ├─ 识别可以合并的索引
   └─ 生成合并树

2. 为每棵树生成合并路径
   ├─ for each merge tree:
   │  ├─ 为每个索引创建扫描路径
   │  │  └─ build_access_path_for_scan_node()
   │  │
   │  ├─ 计算合并成本
   │  │  ├─ 估算各索引的成本
   │  │  ├─ 估算合并操作的成本
   │  │  └─ 计算总成本
   │  │
   │  └─ 创建IndexMergePath
   │     └─ create_one_index_merge_path()
   │
   └─ 进行路径剪枝
      └─ prune_index_merge_path()

3. 返回最终路径集合
```

---

## 8. 连接路径生成和优化

### 8.1 连接路径的完整生成过程

```cpp
// 1. 创建连接路径
JoinPath *join_path = new JoinPath(
  left_path,
  right_path,
  join_type,
  join_quals
);

// 2. 计算连接分片信息
join_path->compute_join_path_sharding();
// 根据左右子树的分片信息和连接条件
// 计算连接结果的分片信息

// 3. 计算连接输出顺序
join_path->compute_join_path_ordering();
// 根据连接方法和左右子树的顺序
// 计算连接结果的输出顺序

// 4. 计算连接并行信息
join_path->compute_join_path_parallel_and_server_info();
// 根据左右子树的并行度
// 计算连接的并行度和服务器信息

// 5. 估算连接成本
join_path->estimate_cost();
// 根据连接类型和左右子树的成本
// 计算连接的总成本

// 6. 添加到路径列表
add_path(join_path);
// 与现有路径比较，进行路径剪枝
```

---

## 总结

这些关键函数构成了OceanBase SQL优化器的核心：

1. **访问路径生成** - 为表生成所有可能的扫描方式
2. **成本估算** - 精确估算每条路径的执行成本
3. **路径剪枝** - 使用多种策略保留最优路径
4. **连接优化** - 选择最优的连接顺序和方法
5. **并行优化** - 计算最优的并行度

理解这些函数的实现细节对于优化SQL查询性能至关重要。

