# OceanBase ob_join_order.cpp 函数索引和调用关系

## 1. 函数分类索引

### 1.1 访问路径生成函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `create_access_paths()` | 3049 | 为表生成所有访问路径 | 主入口 |
| `create_one_access_path()` | 1755 | 为单个索引创建访问路径 | 被create_access_paths调用 |
| `create_index_merge_access_paths()` | 3217 | 创建索引合并路径 | 被create_access_paths调用 |
| `create_one_index_merge_path()` | 4063 | 创建单个索引合并路径 | 被create_index_merge_access_paths调用 |
| `build_access_path_for_scan_node()` | 3982 | 为扫描节点构建访问路径 | 被create_index_merge_access_paths调用 |
| `generate_base_paths()` | 10212 | 生成基表路径 | 主入口 |
| `generate_normal_base_table_paths()` | 10640 | 生成普通基表路径 | 被generate_base_paths调用 |
| `generate_base_table_paths()` | 10657 | 生成基表路径（带PathHelper） | 被generate_normal_base_table_paths调用 |
| `generate_json_table_paths()` | 10262 | 生成JSON表路径 | 被generate_base_paths调用 |
| `generate_function_table_paths()` | 10392 | 生成函数表路径 | 被generate_base_paths调用 |
| `generate_cte_table_paths()` | 10613 | 生成CTE表路径 | 被generate_base_paths调用 |

### 1.2 查询范围提取函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `check_and_extract_query_range()` | 2496 | 检查并提取查询范围 | 被create_one_access_path调用 |
| `get_query_range_info()` | 1003 | 获取查询范围信息 | 被check_and_extract_query_range调用 |
| `extract_preliminary_query_range()` | 5412 | 提取初步查询范围 | 被get_query_range_info调用 |
| `extract_geo_preliminary_query_range()` | 5548 | 提取地理索引范围 | 被extract_preliminary_query_range调用 |
| `extract_multivalue_preliminary_query_range()` | 5591 | 提取多值索引范围 | 被extract_preliminary_query_range调用 |
| `extract_fts_preliminary_query_range()` | 5641 | 提取全文索引范围 | 被extract_preliminary_query_range调用 |
| `get_candi_range_expr()` | 5704 | 获取候选范围表达式 | 被extract_preliminary_query_range调用 |
| `fill_query_range_info()` | 54 | 填充查询范围信息 | 被get_query_range_info调用 |
| `get_query_range_count()` | 118 | 获取查询范围计数 | 被fill_query_range_info调用 |
| `get_preliminary_prefix_info()` | 1173 | 获取初步前缀信息 | 被get_query_range_info调用 |

### 1.3 成本估算函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `estimate_cost()` (AccessPath) | 7828 | 估算表扫描成本 | 被add_path调用 |
| `estimate_cost()` (JoinPath) | 9355 | 估算连接成本 | 被add_path调用 |
| `estimate_cost()` (IndexMergePath) | 4255 | 估算索引合并成本 | 被add_path调用 |
| `re_estimate_cost()` (AccessPath) | 7878 | 重新估算表扫描成本 | 被compute_cost_and_prune_access_path调用 |
| `re_estimate_cost()` (JoinPath) | 9390 | 重新估算连接成本 | 被compute_cost_and_prune_access_path调用 |
| `cost_nest_loop_join()` | 9637 | 计算嵌套循环连接成本 | 被estimate_cost调用 |
| `cost_merge_join()` | 9771 | 计算归并连接成本 | 被estimate_cost调用 |
| `cost_hash_join()` | 9878 | 计算哈希连接成本 | 被estimate_cost调用 |
| `estimate_rowcount_for_access_path()` | 4498 | 估算访问路径的行数 | 被estimate_cost调用 |
| `compute_table_rowcount_info()` | 4517 | 计算表的行数信息 | 被estimate_cost调用 |

### 1.4 路径剪枝函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `compute_cost_and_prune_access_path()` | 4734 | 计算成本并剪枝访问路径 | 被generate_base_table_paths调用 |
| `add_path()` | 6706 | 向连接顺序添加路径 | 被create_one_access_path调用 |
| `add_recycled_paths()` | 6779 | 添加回收的路径 | 被add_path调用 |
| `compute_path_relationship()` | 6849 | 计算两条路径的关系 | 被add_path调用 |
| `compute_join_path_relationship()` | 7039 | 计算连接路径的关系 | 被add_path调用 |
| `skyline_prunning_index()` | 2732 | 使用天际线算法剪枝索引 | 被compute_cost_and_prune_access_path调用 |
| `prune_index_merge_path()` | 4189 | 剪枝索引合并路径 | 被create_index_merge_access_paths调用 |
| `prune_candi_index_merge_trees()` | 3787 | 剪枝候选索引合并树 | 被get_candi_index_merge_trees调用 |
| `prune_paths_due_to_parallel()` | 333 | 由于并行原因剪枝路径 | 被compute_parallel_and_server_info_for_base_paths调用 |

### 1.5 分片信息计算函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `compute_sharding_info_for_base_paths()` | 301 | 为基表路径计算分片信息 | 被generate_base_table_paths调用 |
| `compute_sharding_info_for_index_info_entry()` | 443 | 为索引信息条目计算分片 | 被compute_sharding_info_for_base_paths调用 |
| `compute_sharding_info_with_part_info()` | 480 | 基于分区信息计算分片 | 被compute_sharding_info_for_index_info_entry调用 |
| `set_sharding_info_for_base_path()` | 380 | 为基表路径设置分片信息 | 被compute_sharding_info_for_base_paths调用 |
| `get_sharding_info_from_available_access_paths()` | 525 | 从可用访问路径获取分片信息 | 被compute_sharding_info_for_index_info_entry调用 |
| `compute_table_location()` | 208 | 计算表的位置 | 被compute_sharding_info_for_index_info_entry调用 |
| `compute_table_location_for_index_info_entry()` | 149 | 为索引信息条目计算表位置 | 被compute_sharding_info_for_index_info_entry调用 |
| `get_table_partition_info_from_available_access_paths()` | 187 | 从可用访问路径获取表分区信息 | 被compute_table_location_for_index_info_entry调用 |

### 1.6 并行度计算函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `compute_parallel_and_server_info_for_base_paths()` | 668 | 为基表路径计算并行度和服务器信息 | 被generate_base_table_paths调用 |
| `compute_base_table_parallel_and_server_info()` | 851 | 计算基表的并行度和服务器信息 | 被compute_parallel_and_server_info_for_base_paths调用 |
| `compute_access_path_parallel()` | 797 | 计算访问路径的并行度 | 被compute_base_table_parallel_and_server_info调用 |
| `get_base_path_table_dop()` | 708 | 获取基表路径的DOP | 被compute_base_table_parallel_and_server_info调用 |
| `get_explicit_dop_for_path()` | 644 | 获取路径的显式DOP | 被get_base_path_table_dop调用 |
| `get_random_parallel()` | 732 | 获取随机并行度 | 被compute_base_table_parallel_and_server_info调用 |
| `get_parallel_from_available_access_paths()` | 777 | 从可用访问路径获取并行度 | 被compute_base_table_parallel_and_server_info调用 |
| `compute_parallel_degree()` (AccessPath) | 7532 | 计算访问路径的并行度 | 被compute_access_path_parallel调用 |

### 1.7 索引选择函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `get_valid_index_ids()` | 4542 | 获取有效的索引ID | 被create_access_paths调用 |
| `get_valid_index_ids_with_no_index_hint()` | 913 | 获取没有索引提示的有效索引ID | 被get_valid_index_ids调用 |
| `add_table_by_heuristics()` | 1194 | 使用启发式方法添加表 | 被get_valid_index_ids调用 |
| `virtual_table_heuristics()` | 1242 | 虚拟表启发式处理 | 被add_table_by_heuristics调用 |
| `try_prune_non_unique_index()` | 1299 | 尝试剪枝非唯一索引 | 被add_table_by_heuristics调用 |
| `check_index_subset()` | 1334 | 检查索引子集 | 被try_prune_non_unique_index调用 |
| `get_valid_index_merge_indexes()` | 3330 | 获取有效的索引合并索引 | 被get_candi_index_merge_trees调用 |

### 1.8 索引合并树生成函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `get_candi_index_merge_trees()` | 3272 | 获取候选索引合并树 | 被create_index_merge_access_paths调用 |
| `generate_candi_index_merge_trees()` | 3403 | 生成候选索引合并树 | 被get_candi_index_merge_trees调用 |
| `generate_candi_index_merge_node()` | 3508 | 生成候选索引合并节点 | 被generate_candi_index_merge_trees调用 |
| `collect_candicate_indexes()` | 3588 | 收集候选索引 | 被generate_candi_index_merge_trees调用 |
| `check_candi_index_trees_match_hint()` | 3689 | 检查候选索引树是否匹配提示 | 被get_candi_index_merge_trees调用 |
| `check_one_candi_node_match_hint()` | 3720 | 检查单个候选节点是否匹配提示 | 被check_candi_index_trees_match_hint调用 |

### 1.9 特殊索引处理函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `process_vec_index_info()` | 1605 | 处理向量索引信息 | 被create_one_access_path调用 |
| `process_basic_vec_info_for_index_merge_node()` | 1551 | 处理索引合并节点的向量信息 | 被process_vec_index_info调用 |
| `check_can_use_vec_primary_opt()` | 1538 | 检查是否可以使用向量主索引优化 | 被process_vec_index_info调用 |
| `extract_geo_schema_info()` | 944 | 提取地理模式信息 | 被create_one_access_path调用 |
| `cal_dimension_info()` | 2597 | 计算维度信息 | 被extract_geo_schema_info调用 |
| `is_vector_inv_index_tid()` | 2703 | 检查是否为向量倒排索引 | 被process_vec_index_info调用 |
| `will_use_skip_scan()` | 4413 | 判断是否使用跳跃扫描 | 被create_one_access_path调用 |
| `will_use_das()` | 1377 | 判断是否使用DAS | 被create_one_access_path调用 |
| `check_exec_force_use_das()` | 1416 | 检查执行强制使用DAS | 被will_use_das调用 |
| `check_opt_rule_use_das()` | 1463 | 检查优化规则使用DAS | 被will_use_das调用 |
| `check_use_das_by_false_startup_filter()` | 1508 | 通过虚假启动过滤检查DAS | 被will_use_das调用 |

### 1.10 有序性处理函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `get_access_path_ordering()` | 2236 | 获取访问路径的输出顺序 | 被create_one_access_path调用 |
| `compute_base_table_path_ordering()` | 582 | 计算基表路径的有序性 | 被get_access_path_ordering调用 |
| `compute_base_table_path_plan_type()` | 557 | 计算基表路径的计划类型 | 被get_access_path_ordering调用 |
| `get_index_scan_direction()` | 2293 | 获取索引扫描方向 | 被compute_base_table_path_ordering调用 |
| `get_direction_in_order_by()` | 2359 | 获取ORDER BY中的方向 | 被get_index_scan_direction调用 |
| `check_all_interesting_order()` | 5002 | 检查所有有趣顺序 | 被compute_base_table_path_ordering调用 |
| `is_join_match()` | 5103 | 检查是否与JOIN匹配 | 被check_all_interesting_order调用 |
| `compute_join_path_ordering()` | 8693 | 计算连接路径的输出顺序 | 被JoinPath::estimate_cost调用 |

### 1.11 过滤条件处理函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `add_access_filters()` | 2421 | 添加访问过滤条件 | 被create_one_access_path调用 |
| `init_filter_selectivity()` | 1967 | 初始化过滤选择率 | 被create_one_access_path调用 |
| `check_expr_overlap_index()` | 5219 | 检查表达式与索引重叠 | 被add_access_filters调用 |
| `extract_filter_column_ids()` | 5235 | 提取过滤列ID | 被add_access_filters调用 |
| `check_exprs_overlap_index()` | 5293 | 检查多个表达式与索引重叠 | 被add_access_filters调用 |
| `check_exprs_overlap_gis_index()` | 5321 | 检查表达式与GIS索引重叠 | 被add_access_filters调用 |
| `check_exprs_overlap_multivalue_index()` | 5363 | 检查表达式与多值索引重叠 | 被add_access_filters调用 |

### 1.12 列存储处理函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `init_column_store_est_info()` | 2008 | 初始化列存储估算信息 | 被create_one_access_path调用 |
| `init_column_store_est_info_with_filter()` | 2094 | 使用过滤条件初始化列存储估算 | 被init_column_store_est_info调用 |
| `init_column_store_est_info_with_other_column()` | 2190 | 使用其他列初始化列存储估算 | 被init_column_store_est_info调用 |

### 1.13 批量重扫函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `compute_access_path_batch_rescan()` | 8107 | 计算访问路径的批量重扫 | 被create_one_access_path调用 |
| `compute_nlj_batch_rescan()` | 9155 | 计算嵌套循环连接的批量重扫 | 被estimate_cost调用 |
| `need_compare_batch_rescan()` | 7136 | 判断是否需要比较批量重扫 | 被compute_nlj_batch_rescan调用 |
| `check_right_has_gi_or_exchange()` | 9207 | 检查右表是否有GI或EXCHANGE | 被compute_nlj_batch_rescan调用 |

### 1.14 DAS动态分区剪枝函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `compute_is_das_dynamic_part_pruning()` | 8203 | 计算DAS动态分区剪枝 | 被create_one_access_path调用 |

### 1.15 连接路径函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `compute_join_path_sharding()` | 8492 | 计算连接路径的分片 | 被JoinPath构造调用 |
| `compute_join_path_property()` | 8666 | 计算连接路径的属性 | 被estimate_cost调用 |
| `compute_join_path_plan_type()` | 8744 | 计算连接路径的计划类型 | 被estimate_cost调用 |
| `compute_join_path_parallel_and_server_info()` | 8931 | 计算连接路径的并行度和服务器信息 | 被estimate_cost调用 |
| `compute_hash_hash_sharding_info()` | 8780 | 计算哈希-哈希分片信息 | 被compute_join_path_sharding调用 |
| `compute_join_path_info()` | 9372 | 计算连接路径信息 | 被estimate_cost调用 |

### 1.16 其他辅助函数

| 函数名 | 行号 | 功能 | 调用关系 |
|--------|------|------|--------|
| `init_base_join_order()` | 10171 | 初始化基连接顺序 | 被generate_base_paths调用 |
| `init_ambient_card()` | 10241 | 初始化环境基数 | 被generate_base_paths调用 |
| `fill_index_info_entry()` | 2808 | 填充索引信息条目 | 被create_access_paths调用 |
| `fill_index_info_cache()` | 2924 | 填充索引信息缓存 | 被fill_index_info_entry调用 |
| `refine_unique_range_rowcnt()` | 2953 | 精化唯一范围行数 | 被fill_index_info_entry调用 |
| `fill_opt_info_index_name()` | 4914 | 填充优化信息索引名 | 被create_access_paths调用 |
| `revise_output_rows_after_creating_path()` | 4783 | 创建路径后修订输出行数 | 被compute_cost_and_prune_access_path调用 |
| `check_is_base_table()` | Path::check_is_base_table | 检查是否为基表 | 被add_path调用 |

---

## 2. 调用关系图

### 2.1 访问路径生成的调用链

```
generate_base_paths()
├─ generate_normal_base_table_paths()
│  └─ generate_base_table_paths(PathHelper)
│     ├─ create_access_paths()
│     │  ├─ get_valid_index_ids()
│     │  │  └─ get_valid_index_ids_with_no_index_hint()
│     │  │     ├─ add_table_by_heuristics()
│     │  │     │  ├─ virtual_table_heuristics()
│     │  │     │  └─ try_prune_non_unique_index()
│     │  │     │     └─ check_index_subset()
│     │  │     └─ fill_opt_info_index_name()
│     │  │
│     │  ├─ for each index:
│     │  │  └─ create_one_access_path()
│     │  │     ├─ check_and_extract_query_range()
│     │  │     │  └─ get_query_range_info()
│     │  │     │     ├─ extract_preliminary_query_range()
│     │  │     │     │  ├─ extract_geo_preliminary_query_range()
│     │  │     │     │  ├─ extract_multivalue_preliminary_query_range()
│     │  │     │     │  └─ extract_fts_preliminary_query_range()
│     │  │     │     ├─ fill_query_range_info()
│     │  │     │     └─ get_preliminary_prefix_info()
│     │  │     │
│     │  │     ├─ get_access_path_ordering()
│     │  │     │  └─ compute_base_table_path_ordering()
│     │  │     │     ├─ compute_base_table_path_plan_type()
│     │  │     │     ├─ get_index_scan_direction()
│     │  │     │     │  └─ get_direction_in_order_by()
│     │  │     │     └─ check_all_interesting_order()
│     │  │     │
│     │  │     ├─ add_access_filters()
│     │  │     │  ├─ check_expr_overlap_index()
│     │  │     │  ├─ extract_filter_column_ids()
│     │  │     │  ├─ check_exprs_overlap_index()
│     │  │     │  ├─ check_exprs_overlap_gis_index()
│     │  │     │  └─ check_exprs_overlap_multivalue_index()
│     │  │     │
│     │  │     ├─ init_filter_selectivity()
│     │  │     ├─ init_column_store_est_info()
│     │  │     │  ├─ init_column_store_est_info_with_filter()
│     │  │     │  └─ init_column_store_est_info_with_other_column()
│     │  │     │
│     │  │     ├─ extract_geo_schema_info()
│     │  │     │  └─ cal_dimension_info()
│     │  │     │
│     │  │     ├─ will_use_skip_scan()
│     │  │     ├─ will_use_das()
│     │  │     │  ├─ check_exec_force_use_das()
│     │  │     │  ├─ check_opt_rule_use_das()
│     │  │     │  └─ check_use_das_by_false_startup_filter()
│     │  │     │
│     │  │     ├─ process_vec_index_info()
│     │  │     │  ├─ check_can_use_vec_primary_opt()
│     │  │     │  └─ process_basic_vec_info_for_index_merge_node()
│     │  │     │
│     │  │     ├─ compute_is_das_dynamic_part_pruning()
│     │  │     ├─ estimate_cost()
│     │  │     └─ add_path()
│     │  │        ├─ compute_path_relationship()
│     │  │        └─ add_recycled_paths()
│     │  │
│     │  ├─ create_index_merge_access_paths()
│     │  │  ├─ get_candi_index_merge_trees()
│     │  │  │  ├─ generate_candi_index_merge_trees()
│     │  │  │  │  ├─ generate_candi_index_merge_node()
│     │  │  │  │  └─ collect_candicate_indexes()
│     │  │  │  ├─ check_candi_index_trees_match_hint()
│     │  │  │  │  └─ check_one_candi_node_match_hint()
│     │  │  │  └─ prune_candi_index_merge_trees()
│     │  │  │
│     │  │  ├─ for each merge tree:
│     │  │  │  ├─ build_access_path_for_scan_node()
│     │  │  │  └─ create_one_index_merge_path()
│     │  │  │     └─ estimate_cost()
│     │  │  │
│     │  │  └─ prune_index_merge_path()
│     │  │
│     │  └─ fill_index_info_entry()
│     │     ├─ fill_index_info_cache()
│     │     └─ refine_unique_range_rowcnt()
│     │
│     ├─ compute_sharding_info_for_base_paths()
│     │  ├─ compute_sharding_info_for_index_info_entry()
│     │  │  ├─ compute_sharding_info_with_part_info()
│     │  │  ├─ compute_table_location_for_index_info_entry()
│     │  │  │  └─ compute_table_location()
│     │  │  └─ get_sharding_info_from_available_access_paths()
│     │  │
│     │  └─ set_sharding_info_for_base_path()
│     │
│     ├─ compute_parallel_and_server_info_for_base_paths()
│     │  ├─ compute_base_table_parallel_and_server_info()
│     │  │  ├─ compute_access_path_parallel()
│     │  │  │  └─ compute_parallel_degree() (AccessPath)
│     │  │  ├─ get_base_path_table_dop()
│     │  │  │  └─ get_explicit_dop_for_path()
│     │  │  ├─ get_random_parallel()
│     │  │  └─ get_parallel_from_available_access_paths()
│     │  │
│     │  └─ prune_paths_due_to_parallel()
│     │
│     └─ compute_cost_and_prune_access_path()
│        ├─ skyline_prunning_index()
│        └─ revise_output_rows_after_creating_path()
│
├─ generate_json_table_paths()
├─ generate_function_table_paths()
└─ generate_cte_table_paths()
```

### 2.2 成本估算的调用链

```
estimate_cost() (AccessPath)
├─ 估算索引扫描成本
├─ 估算回表成本（如果需要）
├─ 估算过滤成本
└─ 考虑并行执行

estimate_cost() (JoinPath)
├─ 获取左右子树的成本
├─ 根据连接类型选择成本计算方法
│  ├─ cost_nest_loop_join()
│  │  ├─ 计算左表成本
│  │  ├─ 计算右表成本
│  │  └─ 考虑批量重扫
│  │     └─ compute_nlj_batch_rescan()
│  │        ├─ need_compare_batch_rescan()
│  │        └─ check_right_has_gi_or_exchange()
│  │
│  ├─ cost_merge_join()
│  │  ├─ 计算排序成本
│  │  ├─ 计算扫描成本
│  │  └─ 计算比较成本
│  │
│  └─ cost_hash_join()
│     ├─ 计算哈希表构建成本
│     ├─ 计算探测成本
│     └─ 考虑溢出成本
│
├─ compute_join_path_property()
├─ compute_join_path_plan_type()
├─ compute_join_path_ordering()
├─ compute_join_path_parallel_and_server_info()
└─ compute_join_path_info()
```

---

## 3. 关键数据流

### 3.1 访问路径的生命周期

```
1. 创建阶段
   create_one_access_path()
   ├─ 初始化AccessPath对象
   ├─ 设置基本信息（table_id, index_id等）
   └─ 返回AccessPath

2. 信息提取阶段
   ├─ check_and_extract_query_range()
   │  └─ 提取查询范围
   ├─ get_access_path_ordering()
   │  └─ 确定输出顺序
   ├─ add_access_filters()
   │  └─ 添加过滤条件
   └─ extract_geo_schema_info()
      └─ 提取特殊索引信息

3. 成本估算阶段
   estimate_cost()
   ├─ 估算行数
   ├─ 估算成本
   └─ 返回成本

4. 路径比较阶段
   add_path()
   ├─ compute_path_relationship()
   │  └─ 比较与现有路径的关系
   ├─ 决定是否添加
   └─ 进行路径剪枝

5. 属性计算阶段
   ├─ compute_sharding_info_for_base_paths()
   │  └─ 计算分片信息
   ├─ compute_parallel_and_server_info_for_base_paths()
   │  └─ 计算并行度
   └─ compute_cost_and_prune_access_path()
      └─ 最终剪枝

6. 选择阶段
   ├─ 返回最优路径集合
   └─ 用于后续连接优化
```

---

## 4. 函数的重要参数

### 4.1 PathHelper结构

```cpp
struct PathHelper {
  ObIArray<AccessPath*> &access_paths_;      // 访问路径数组
  ObIArray<ObRawExpr*> &filter_exprs_;       // 过滤表达式
  ObIArray<ObRawExpr*> &pushdown_filters_;   // 下推过滤条件
  // ... 其他字段
};
```

### 4.2 QueryRangeInfo结构

```cpp
struct QueryRangeInfo {
  ObQueryRangeArray ranges_;                 // 查询范围
  ObQueryRangeArray ss_ranges_;              // 跳跃扫描范围
  ObQueryRangeProvider *provider_;           // 范围提供者
  int64_t range_count_;                      // 范围计数
  double unique_range_rowcnt_;               // 唯一范围行数
  // ... 其他字段
};
```

### 4.3 EstimateCostInfo结构

```cpp
struct EstimateCostInfo {
  ObSEArray<JoinFilterInfo> join_filter_infos_;  // 连接过滤信息
  double need_row_count_;                        // 需要的行数
  int64_t need_parallel_;                        // 需要的并行度
  bool need_batch_rescan_;                       // 是否需要批量重扫
  // ... 其他字段
};
```

---

## 总结

这个函数索引提供了ob_join_order.cpp中所有重要函数的完整映射，包括：

1. **访问路径生成** - 为表生成所有可能的扫描方式
2. **查询范围提取** - 从WHERE子句提取索引可用的范围
3. **成本估算** - 估算每条路径的执行成本
4. **路径剪枝** - 保留最优路径
5. **分片计算** - 计算表的分布信息
6. **并行优化** - 计算最优并行度
7. **特殊索引** - 支持多种高级索引类型
8. **连接优化** - 选择最优的连接方式

理解这些函数的调用关系对于深入理解OceanBase的SQL优化器至关重要。

