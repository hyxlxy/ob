# OceanBase ob_join_order.cpp 关键概念和最佳实践

## 1. 核心概念

### 1.1 访问路径（Access Path）

**定义**: 表示访问表数据的一种方式，包括使用哪个索引、扫描方向、过滤条件等。

**组成部分**:
```cpp
AccessPath {
  table_id_,           // 表ID
  index_id_,           // 索引ID（主键为0）
  order_direction_,    // 扫描方向（ASC/DESC）
  est_cost_info_,      // 成本估算信息
  filter_,             // 过滤条件
  ordering_,           // 输出顺序
  sharding_,           // 分片信息
  parallel_,           // 并行度
}
```

**访问路径的生命周期**:
```
创建 → 成本估算 → 路径剪枝 → 选择最优路径
```

### 1.2 路径关系（Path Relationship）

**定义**: 两条路径之间的支配关系

**关系类型**:
```cpp
enum PathRelationship {
  SAME_COST,           // 成本相同
  FIRST_BETTER,        // 第一条路径更优
  SECOND_BETTER,       // 第二条路径更优
  UNCOMPARABLE,        // 无法比较（Pareto前沿）
};
```

**比较维度**:
1. **成本** - 执行成本越低越好
2. **行数** - 输出行数越少越好
3. **排序属性** - 满足有趣顺序的路径更优
4. **并行度** - 支持更高并行度的路径更优

### 1.3 天际线（Skyline）

**定义**: 在多维空间中，不被任何其他点支配的点的集合。

**在SQL优化中的应用**:
- 维度：成本、行数、排序属性
- 保留所有Pareto最优路径
- 避免过度剪枝

**算法**:
```
1. 对所有路径按成本排序
2. 对每条路径：
   - 检查是否被已选中的路径支配
   - 如果不被支配，加入结果集
   - 移除被当前路径支配的已选路径
3. 返回Pareto前沿路径
```

### 1.4 查询范围（Query Range）

**定义**: 索引可以扫描的行的范围，由WHERE子句的谓词决定。

**范围表示**:
```cpp
ObNewRange {
  start_key_,          // 起始键
  end_key_,            // 结束键
  border_flag_,        // 边界标志
}
```

**范围类型**:
```
1. 等值范围：col = value
   → [value, value]

2. IN范围：col IN (v1, v2, v3)
   → [v1, v1], [v2, v2], [v3, v3]

3. 范围条件：col > value
   → (value, +∞)

4. BETWEEN范围：col BETWEEN a AND b
   → [a, b]

5. 前缀范围：col LIKE 'prefix%'
   → ['prefix', 'prefix\xFF')
```

**范围行数估算**:
```cpp
// 基本公式
range_row_count = table_row_count * selectivity

// 选择率计算
selectivity = (end_value - start_value) / (max_value - min_value)

// 多列范围
selectivity = selectivity_col1 * selectivity_col2 * ...
```

### 1.5 分片信息（Sharding Info）

**定义**: 表示表的数据如何分布在集群中。

**分片类型**:
```cpp
enum ObTableLocationType {
  OB_TBL_LOCATION_UNSHARDED,    // 未分片
  OB_TBL_LOCATION_LOCAL,         // 本地分片
  OB_TBL_LOCATION_REMOTE,        // 远程分片
  OB_TBL_LOCATION_DISTRIBUTED,   // 分布式分片
  OB_TBL_LOCATION_MATCH_ALL,     // 匹配所有分区
};
```

**分片信息包含**:
```cpp
ObShardingInfo {
  location_type_,              // 位置类型
  phy_table_location_info_,    // 物理表位置
  partition_keys_,             // 分区键
  partition_cnt_,              // 分区数量
  server_list_,                // 服务器列表
}
```

**分片的意义**:
- 决定查询是否可以下推到单个分区
- 影响连接的执行方式（分区级连接 vs 全局连接）
- 影响并行度的计算

### 1.6 并行度（Degree of Parallelism, DOP）

**定义**: 查询并行执行时使用的线程或进程数量。

**DOP计算**:
```cpp
DOP = min(
  table_dop,           // 表的DOP
  server_count,        // 服务器数量
  partition_count,     // 分区数量
  user_dop,            // 用户指定的DOP
  available_dop        // 可用DOP
)
```

**DOP的影响**:
- 成本 = 单线程成本 / DOP
- 内存使用 = 单线程内存 × DOP
- 响应时间 = 单线程时间 / DOP（理想情况）

### 1.7 有趣顺序（Interesting Order）

**定义**: 查询结果需要的特定顺序，如ORDER BY、GROUP BY、JOIN条件等。

**有趣顺序来源**:
```cpp
enum OrderingFlag {
  ORDER_BY_MATCH,      // 匹配ORDER BY子句
  GROUP_BY_MATCH,      // 匹配GROUP BY子句
  JOIN_MATCH,          // 匹配JOIN条件
  DISTINCT_MATCH,      // 匹配DISTINCT
};
```

**有趣顺序的价值**:
- 如果访问路径提供有趣顺序，可以避免后续排序
- 排序成本通常很高，避免排序可以显著提高性能
- 即使访问路径的总成本较高，提供有趣顺序也可能是最优选择

---

## 2. 成本模型详解

### 2.1 表扫描成本

**成本组成**:
```
total_cost = index_scan_cost + index_back_cost + filter_cost
```

**各部分详解**:

```cpp
// 1. 索引扫描成本
// 根据扫描的行数和索引大小计算
index_scan_cost = (range_row_count / index_height) * io_cost_per_block

// 2. 回表成本（如果需要）
// 从索引回到主表获取其他列
if (need_index_back) {
  index_back_cost = back_row_count * io_cost_per_row
}

// 3. 过滤成本
// 应用WHERE子句中不能用索引的条件
filter_cost = output_row_count * cpu_cost_per_row

// 4. 总成本
total_cost = index_scan_cost + index_back_cost + filter_cost

// 5. 考虑并行执行
if (parallel_ > 1) {
  final_cost = total_cost / parallel_
}
```

### 2.2 连接成本

**三种主要连接方法的成本模型**:

#### 嵌套循环连接（Nested Loop Join）

```cpp
// 基本公式
nlj_cost = left_cost + left_rows × (right_cost + right_rows × filter_cost)

// 优化：批量重扫
if (can_batch_rescan) {
  batch_size = get_batch_size()
  batch_count = (left_rows + batch_size - 1) / batch_size
  nlj_cost = left_cost + batch_count × (right_cost + right_rows × filter_cost)
}

// 特点：
// - 适合左表较小的情况
// - 右表需要多次扫描
// - 可以利用批量重扫优化
```

#### 归并连接（Merge Join）

```cpp
// 基本公式
mj_cost = left_sort_cost + right_sort_cost + scan_cost + compare_cost

// 其中：
left_sort_cost = estimate_sort_cost(left_rows, left_key_size)
right_sort_cost = estimate_sort_cost(right_rows, right_key_size)
scan_cost = (left_rows + right_rows) × cpu_cost_per_row
compare_cost = (left_rows + right_rows) × cpu_cost_per_compare

// 优化：如果输入已有序，可以省略排序
if (left_path_->is_ordered()) {
  mj_cost -= left_sort_cost
}
if (right_path_->is_ordered()) {
  mj_cost -= right_sort_cost
}

// 特点：
// - 适合两个表都较大的情况
// - 如果输入已有序，成本较低
// - 输出结果有序
```

#### 哈希连接（Hash Join）

```cpp
// 基本公式
hj_cost = left_cost + right_cost + hash_build_cost + probe_cost

// 其中：
hash_build_cost = right_rows × cpu_cost_per_row
probe_cost = left_rows × cpu_cost_per_row

// 如果哈希表溢出到磁盘
if (hash_table_size > available_memory) {
  hj_cost += estimate_spill_cost(hash_table_size)
}

// 特点：
// - 适合至少一个表较小的情况
// - 成本主要是CPU成本
// - 可能需要处理哈希表溢出
```

### 2.3 成本模型的参数

**IO成本参数**:
```cpp
io_cost_per_block = 1.0        // 每个块的IO成本
io_cost_per_row = 0.1          // 每行的IO成本
```

**CPU成本参数**:
```cpp
cpu_cost_per_row = 0.01        // 每行的CPU成本
cpu_cost_per_compare = 0.001   // 每次比较的CPU成本
```

**这些参数可以通过优化器参数调整**:
```sql
SET optimizer_io_cost = 1.0;
SET optimizer_cpu_cost = 0.01;
```

---

## 3. 索引选择策略

### 3.1 索引类型和适用场景

**B树索引**:
- 适用于范围查询、排序、分组
- 支持前缀匹配
- 支持多列索引

**哈希索引**:
- 适用于等值查询
- 不支持范围查询
- 查询速度快

**全文索引（FTS）**:
- 适用于文本搜索
- 支持关键词查询、短语查询
- 支持布尔查询

**地理索引（GIS）**:
- 适用于空间数据查询
- 支持范围查询、距离查询
- 支持几何操作

**向量索引**:
- 适用于向量相似度查询
- 支持近似最近邻搜索
- 用于AI/ML应用

**多值索引**:
- 适用于数组/JSON数据
- 支持元素查询
- 支持路径查询

### 3.2 索引选择的启发式规则

**规则1：唯一索引优先**
```
如果存在唯一索引且查询条件匹配唯一索引的所有列，
则选择唯一索引（可能无需回表）
```

**规则2：最小化回表行数**
```
选择能够最小化回表行数的索引
回表成本通常很高，应该优先考虑
```

**规则3：利用有趣顺序**
```
如果索引提供有趣顺序（ORDER BY、GROUP BY），
即使成本较高也应该考虑
避免后续排序操作
```

**规则4：选择最窄的索引**
```
对于相同成本的索引，选择列数最少的索引
更窄的索引占用更少的内存和存储空间
```

**规则5：考虑索引合并**
```
如果单个索引成本较高，考虑使用索引合并
多个索引的并集或交集可能成本更低
```

### 3.3 索引合并的条件

**UNION合并适用**:
```
WHERE (col1 = value1) OR (col2 = value2)

可以使用：
- 索引1（col1）
- 索引2（col2）
- 合并结果（UNION）
```

**INTERSECT合并适用**:
```
WHERE (col1 = value1) AND (col2 = value2)

可以使用：
- 索引1（col1）
- 索引2（col2）
- 合并结果（INTERSECT）
```

**合并成本计算**:
```cpp
// UNION合并
union_cost = index1_cost + index2_cost + union_cost
union_rows = index1_rows + index2_rows - duplicate_rows

// INTERSECT合并
intersect_cost = index1_cost + index2_cost + intersect_cost
intersect_rows = index1_rows ∩ index2_rows
```

---

## 4. 连接顺序优化

### 4.1 连接顺序的重要性

**连接顺序影响**:
```
1. 中间结果的大小
   - 小的中间结果 → 低成本
   - 大的中间结果 → 高成本

2. 内存使用
   - 小的中间结果 → 低内存
   - 大的中间结果 → 高内存

3. 缓存效率
   - 小的中间结果 → 更好的缓存命中
   - 大的中间结果 → 缓存命中率低

4. 并行执行效率
   - 不同的连接顺序 → 不同的并行策略
```

**例子**:
```sql
SELECT * FROM A, B, C
WHERE A.id = B.a_id AND B.id = C.b_id

连接顺序1：(A JOIN B) JOIN C
- 中间结果1 = A JOIN B (假设100行)
- 中间结果2 = 中间结果1 JOIN C (假设1000行)
- 总成本 = 100 + 1000 = 1100

连接顺序2：(B JOIN C) JOIN A
- 中间结果1 = B JOIN C (假设10000行)
- 中间结果2 = 中间结果1 JOIN A (假设10000行)
- 总成本 = 10000 + 10000 = 20000

连接顺序1更优！
```

### 4.2 连接顺序的搜索空间

**搜索空间大小**:
```
n个表的连接顺序数 = (2n-2)! / (n-1)!

例如：
- 3个表：12种顺序
- 4个表：576种顺序
- 5个表：161,280种顺序
- 10个表：17,643,225,600种顺序
```

**搜索策略**:
```
1. 穷举搜索（Exhaustive Search）
   - 适用于表数较少（< 10）
   - 保证找到最优顺序
   - 时间复杂度：O(n!)

2. 动态规划（Dynamic Programming）
   - 适用于表数中等（< 15）
   - 时间复杂度：O(3^n)
   - OceanBase使用此方法

3. 启发式搜索（Heuristic Search）
   - 适用于表数较多（> 15）
   - 不保证最优，但速度快
   - 时间复杂度：O(n^2)或O(n^3)
```

### 4.3 动态规划算法

**基本思想**:
```
dp[S] = 最优的连接顺序，用于连接集合S中的所有表

递推关系：
dp[S] = min(
  dp[S1] JOIN dp[S2]
) for all S1 ∪ S2 = S, S1 ∩ S2 = ∅

边界条件：
dp[{i}] = 表i的最优访问路径
```

**时间复杂度**:
```
O(3^n)：因为对于每个表的子集，需要枚举所有分割方式
```

---

## 5. 并行执行优化

### 5.1 并行度的选择

**DOP选择的考虑因素**:

```cpp
1. 硬件资源
   ├─ CPU核心数
   ├─ 内存大小
   └─ 网络带宽

2. 数据量
   ├─ 表的大小
   ├─ 中间结果的大小
   └─ 最终结果的大小

3. 查询复杂度
   ├─ 连接数量
   ├─ 聚合操作
   └─ 排序操作

4. 系统负载
   ├─ 当前运行的查询数
   ├─ 系统CPU使用率
   └─ 内存使用率
```

**DOP计算算法**:

```cpp
// 1. 获取基础DOP
base_dop = min(cpu_count, server_count)

// 2. 根据表大小调整
if (table_size < SMALL_TABLE_THRESHOLD) {
  dop = 1  // 小表不需要并行
} else if (table_size < MEDIUM_TABLE_THRESHOLD) {
  dop = base_dop / 2
} else {
  dop = base_dop
}

// 3. 根据分区数量调整
dop = min(dop, partition_count)

// 4. 应用用户约束
if (user_specified_dop > 0) {
  dop = user_specified_dop
}

// 5. 应用系统约束
dop = min(dop, available_dop)
```

### 5.2 并行执行的成本模型

**并行成本计算**:

```cpp
// 单线程成本
single_thread_cost = ...

// 并行成本
parallel_cost = single_thread_cost / dop + communication_cost

// 其中communication_cost包括：
// - 数据分发成本
// - 数据收集成本
// - 同步成本
```

**并行收益**:

```cpp
// 理想情况（无通信开销）
speedup = dop

// 实际情况（有通信开销）
speedup = dop / (1 + communication_ratio)

// 其中communication_ratio = communication_cost / single_thread_cost
```

### 5.3 批量重扫优化

**适用场景**:
```
嵌套循环连接中，右表需要多次扫描
```

**优化原理**:
```
不是对每一行左表数据都扫描一次右表，
而是收集一批左表数据，一次性扫描右表

这样可以：
1. 减少右表的扫描次数
2. 提高缓存命中率
3. 减少上下文切换
```

**成本计算**:

```cpp
// 不使用批量重扫
nlj_cost = left_rows × right_cost

// 使用批量重扫
batch_size = 1000  // 每批处理的行数
batch_count = (left_rows + batch_size - 1) / batch_size
nlj_cost = batch_count × right_cost

// 成本节省
savings = (left_rows - batch_count) × right_cost
```

---

## 6. 最佳实践

### 6.1 创建高效索引的建议

**1. 选择合适的索引列**
```
- 选择选择率高的列（能够过滤大量行）
- 选择经常在WHERE子句中出现的列
- 选择经常在JOIN条件中出现的列
- 选择经常在ORDER BY中出现的列
```

**2. 确定索引列的顺序**
```
对于多列索引，按照以下顺序排列列：
1. 等值条件的列（最前）
2. 范围条件的列
3. 排序条件的列（最后）

例如：WHERE a = 1 AND b > 2 ORDER BY c
最优索引：(a, b, c)
```

**3. 考虑索引的宽度**
```
- 更窄的索引占用更少空间
- 更窄的索引更新更快
- 但可能需要回表
- 需要在空间和性能之间平衡
```

**4. 避免创建冗余索引**
```
- 如果已有索引(a, b)，不需要创建索引(a)
- 索引(a)是索引(a, b)的前缀
- 前缀索引可以被原索引使用
```

### 6.2 优化查询的建议

**1. 使用索引提示**
```sql
-- 强制使用特定索引
SELECT * FROM t1 USE INDEX (idx_name) WHERE ...

-- 忽略特定索引
SELECT * FROM t1 IGNORE INDEX (idx_name) WHERE ...

-- 强制全表扫描
SELECT * FROM t1 USE INDEX () WHERE ...
```

**2. 调整连接顺序**
```sql
-- 使用STRAIGHT_JOIN强制连接顺序
SELECT * FROM a STRAIGHT_JOIN b ON a.id = b.a_id
STRAIGHT_JOIN c ON b.id = c.b_id

-- 这样强制连接顺序为：a → b → c
```

**3. 使用查询提示**
```sql
-- 指定并行度
SELECT /*+ PARALLEL(4) */ * FROM t1 WHERE ...

-- 指定连接方法
SELECT /*+ USE_NL(t1, t2) */ * FROM t1, t2 WHERE ...
SELECT /*+ USE_HASH(t1, t2) */ * FROM t1, t2 WHERE ...
SELECT /*+ USE_MERGE(t1, t2) */ * FROM t1, t2 WHERE ...

-- 指定索引
SELECT /*+ INDEX(t1, idx_name) */ * FROM t1 WHERE ...
```

**4. 优化WHERE条件**
```
- 将选择率高的条件放在前面
- 避免在WHERE中使用函数（可能无法使用索引）
- 避免使用NOT IN（改用NOT EXISTS）
- 避免使用OR（改用UNION）
```

### 6.3 性能调优的建议

**1. 收集统计信息**
```sql
-- 收集表的统计信息
ANALYZE TABLE t1;

-- 收集索引的统计信息
ANALYZE TABLE t1 COMPUTE STATISTICS;
```

**2. 查看执行计划**
```sql
-- 查看执行计划
EXPLAIN SELECT * FROM t1 WHERE ...;

-- 查看详细执行计划
EXPLAIN EXTENDED SELECT * FROM t1 WHERE ...;
```

**3. 调整优化器参数**
```sql
-- 调整并行度
SET PARALLEL_DEGREE = 4;

-- 调整成本模型参数
SET optimizer_io_cost = 1.0;
SET optimizer_cpu_cost = 0.01;
```

**4. 监控查询性能**
```sql
-- 查看查询的执行时间
SELECT * FROM v$sql WHERE sql_text LIKE '%...%';

-- 查看表的访问统计
SELECT * FROM v$table_stat WHERE table_name = 't1';
```

---

## 7. 常见问题和解决方案

### 7.1 为什么选择了错误的索引？

**可能原因**:
```
1. 统计信息过时
   → 解决：ANALYZE TABLE

2. 索引选择性差
   → 解决：创建更好的索引

3. 成本模型不准确
   → 解决：调整成本参数

4. 优化器有bug
   → 解决：使用索引提示
```

**诊断方法**:
```sql
-- 查看执行计划
EXPLAIN SELECT ...;

-- 查看索引统计
SELECT * FROM information_schema.statistics WHERE table_name = 't1';

-- 强制使用特定索引
SELECT /*+ INDEX(t1, idx_name) */ * FROM t1 WHERE ...;
```

### 7.2 为什么连接顺序不对？

**可能原因**:
```
1. 统计信息不准确
   → 解决：ANALYZE TABLE

2. 中间结果行数估算错误
   → 解决：检查WHERE条件的选择率

3. 优化器搜索空间限制
   → 解决：使用STRAIGHT_JOIN指定顺序
```

**诊断方法**:
```sql
-- 查看执行计划
EXPLAIN SELECT ...;

-- 查看中间结果的行数估算
EXPLAIN EXTENDED SELECT ...;

-- 强制连接顺序
SELECT * FROM a STRAIGHT_JOIN b ON ... STRAIGHT_JOIN c ON ...;
```

### 7.3 为什么并行度太低？

**可能原因**:
```
1. 表太小
   → 解决：并行度自动调整

2. 分区数量少
   → 解决：增加分区数量

3. 系统资源不足
   → 解决：增加硬件资源

4. 用户指定了低并行度
   → 解决：调整PARALLEL_DEGREE参数
```

**诊断方法**:
```sql
-- 查看表的分区数
SELECT partition_count FROM information_schema.tables WHERE table_name = 't1';

-- 查看系统的CPU核心数
SELECT cpu_count FROM v$system_stat;

-- 查看当前的并行度设置
SHOW VARIABLES LIKE 'parallel%';
```

---

## 总结

理解ob_join_order.cpp的关键概念对于优化SQL查询至关重要：

1. **访问路径** - 表示访问表的方式
2. **成本模型** - 估算查询的执行成本
3. **路径剪枝** - 保留最优路径
4. **连接顺序** - 选择最优的连接顺序
5. **并行执行** - 利用多核CPU加速查询

通过理解这些概念和应用最佳实践，可以显著提高SQL查询的性能。

