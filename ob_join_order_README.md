# OceanBase ob_join_order.cpp 完整理解指南

## 📚 文档概览

本指南包含4个详细的分析文档，全面覆盖OceanBase SQL优化器中的连接顺序优化模块：

### 文档清单

1. **ob_join_order_analysis.md** - 完整的文件分析
   - 文件概述和核心功能
   - 核心类和数据结构详解
   - 主要功能模块（15个）
   - 关键算法和策略
   - 数据流和调用关系
   - 重要的数据结构

2. **ob_join_order_detailed_functions.md** - 关键函数深度分析
   - 访问路径创建函数详解
   - 查询范围提取函数详解
   - 成本估算函数详解
   - 路径剪枝函数详解
   - 分片信息计算函数详解
   - 并行度计算函数详解
   - 索引合并路径生成函数详解
   - 连接路径生成和优化

3. **ob_join_order_key_concepts.md** - 关键概念和最佳实践
   - 7个核心概念详解
   - 成本模型详解
   - 索引选择策略
   - 连接顺序优化
   - 并行执行优化
   - 最佳实践建议
   - 常见问题和解决方案

4. **ob_join_order_function_index.md** - 函数索引和调用关系
   - 16个函数分类
   - 完整的函数索引表
   - 调用关系图
   - 关键数据流
   - 函数的重要参数

---

## 🎯 快速导航

### 按功能查找

**我想了解访问路径的生成过程**
→ 查看 `ob_join_order_analysis.md` 的 "访问路径生成模块" 部分
→ 查看 `ob_join_order_detailed_functions.md` 的 "访问路径创建函数详解"
→ 查看 `ob_join_order_function_index.md` 的 "访问路径生成的调用链"

**我想了解成本是如何估算的**
→ 查看 `ob_join_order_key_concepts.md` 的 "成本模型详解"
→ 查看 `ob_join_order_detailed_functions.md` 的 "成本估算函数详解"
→ 查看 `ob_join_order_analysis.md` 的 "成本估算模块"

**我想了解路径是如何剪枝的**
→ 查看 `ob_join_order_key_concepts.md` 的 "天际线" 部分
→ 查看 `ob_join_order_analysis.md` 的 "路径剪枝模块"
→ 查看 `ob_join_order_detailed_functions.md` 的 "路径剪枝函数详解"

**我想了解连接顺序优化**
→ 查看 `ob_join_order_key_concepts.md` 的 "连接顺序优化"
→ 查看 `ob_join_order_analysis.md` 的 "连接顺序优化模块"
→ 查看 `ob_join_order_function_index.md` 的 "连接路径函数"

**我想了解并行度计算**
→ 查看 `ob_join_order_key_concepts.md` 的 "并行执行优化"
→ 查看 `ob_join_order_analysis.md` 的 "并行度计算模块"
→ 查看 `ob_join_order_detailed_functions.md` 的 "并行度计算函数详解"

**我想了解特殊索引（FTS、GIS、向量等）**
→ 查看 `ob_join_order_analysis.md` 的 "特殊索引支持模块"
→ 查看 `ob_join_order_function_index.md` 的 "特殊索引处理函数"

**我想找到特定函数的实现**
→ 查看 `ob_join_order_function_index.md` 的 "函数分类索引"

### 按概念查找

**访问路径（Access Path）**
→ `ob_join_order_key_concepts.md` 的 "访问路径" 部分

**路径关系（Path Relationship）**
→ `ob_join_order_key_concepts.md` 的 "路径关系" 部分

**天际线（Skyline）**
→ `ob_join_order_key_concepts.md` 的 "天际线" 部分

**查询范围（Query Range）**
→ `ob_join_order_key_concepts.md` 的 "查询范围" 部分

**分片信息（Sharding Info）**
→ `ob_join_order_key_concepts.md` 的 "分片信息" 部分

**并行度（DOP）**
→ `ob_join_order_key_concepts.md` 的 "并行度" 部分

**有趣顺序（Interesting Order）**
→ `ob_join_order_key_concepts.md` 的 "有趣顺序" 部分

---

## 📊 文件结构概览

```
ob_join_order.cpp (20,849 行)
│
├─ 头部（第1-50行）
│  ├─ 版权声明
│  ├─ 包含文件
│  └─ 命名空间
│
├─ 访问路径生成（第54-4228行）
│  ├─ 基本访问路径创建
│  ├─ 索引合并路径创建
│  ├─ 查询范围提取
│  ├─ 成本估算
│  └─ 路径剪枝
│
├─ 分片和并行计算（第301-851行）
│  ├─ 分片信息计算
│  ├─ 并行度计算
│  └─ 服务器信息计算
│
├─ 有序性处理（第557-5293行）
│  ├─ 索引扫描方向
│  ├─ 有趣顺序匹配
│  └─ 表达式重叠检查
│
├─ 特殊索引处理（第944-2703行）
│  ├─ 地理索引
│  ├─ 全文索引
│  ├─ 向量索引
│  └─ 多值索引
│
├─ 连接路径优化（第6706-10006行）
│  ├─ 路径添加和比较
│  ├─ 连接路径生成
│  ├─ 连接成本计算
│  └─ 批量重扫优化
│
├─ 基表路径生成（第10171-10790行）
│  ├─ 基连接顺序初始化
│  ├─ JSON表路径
│  ├─ 函数表路径
│  ├─ CTE表路径
│  └─ 普通基表路径
│
└─ 辅助函数（分散在各处）
   ├─ 内存管理
   ├─ 日志记录
   └─ 错误处理
```

---

## 🔑 关键概念速查表

| 概念 | 定义 | 关键函数 | 文档位置 |
|------|------|--------|--------|
| 访问路径 | 访问表数据的方式 | `create_one_access_path()` | 分析文档 |
| 查询范围 | 索引可以扫描的行范围 | `check_and_extract_query_range()` | 分析文档 |
| 成本估算 | 估算路径的执行成本 | `estimate_cost()` | 深度分析文档 |
| 路径剪枝 | 保留最优路径 | `compute_cost_and_prune_access_path()` | 分析文档 |
| 分片信息 | 表的数据分布 | `compute_sharding_info_for_base_paths()` | 分析文档 |
| 并行度 | 并行执行的线程数 | `compute_parallel_and_server_info_for_base_paths()` | 分析文档 |
| 有趣顺序 | 查询需要的特定顺序 | `check_all_interesting_order()` | 分析文档 |
| 天际线 | Pareto最优路径 | `skyline_prunning_index()` | 概念文档 |
| 索引合并 | 多个索引的并集/交集 | `create_index_merge_access_paths()` | 分析文档 |
| 批量重扫 | 优化嵌套循环连接 | `compute_nlj_batch_rescan()` | 分析文档 |

---

## 💡 学习路径建议

### 初级（理解基本概念）

1. 阅读 `ob_join_order_key_concepts.md` 的前两部分
   - 核心概念（1小时）
   - 成本模型（1小时）

2. 阅读 `ob_join_order_analysis.md` 的概述部分
   - 文件概述（30分钟）
   - 核心类和数据结构（1小时）

3. 运行简单的SQL查询并查看执行计划
   - 理解访问路径的选择
   - 理解成本的计算

### 中级（深入理解实现）

1. 阅读 `ob_join_order_detailed_functions.md`
   - 访问路径创建（1.5小时）
   - 查询范围提取（1小时）
   - 成本估算（1.5小时）

2. 阅读 `ob_join_order_function_index.md`
   - 函数分类索引（1小时）
   - 调用关系图（1小时）

3. 阅读源代码的关键函数
   - `create_access_paths()` - 访问路径生成的主入口
   - `estimate_cost()` - 成本估算的核心
   - `add_path()` - 路径比较和剪枝

### 高级（优化和扩展）

1. 阅读 `ob_join_order_key_concepts.md` 的后三部分
   - 索引选择策略（1小时）
   - 连接顺序优化（1.5小时）
   - 并行执行优化（1小时）

2. 阅读 `ob_join_order_analysis.md` 的所有模块
   - 特别关注特殊索引支持模块
   - 关注性能考虑部分

3. 分析复杂查询的执行计划
   - 理解优化器的决策过程
   - 识别可能的优化机会

4. 修改和扩展优化器
   - 添加新的成本模型
   - 支持新的索引类型
   - 改进路径剪枝策略

---

## 🔍 代码阅读技巧

### 1. 从高层函数开始

```cpp
// 从这些主入口开始
generate_base_paths()           // 生成基表路径
create_access_paths()           // 创建访问路径
estimate_cost()                 // 估算成本
add_path()                      // 添加路径
```

### 2. 跟踪数据流

```
输入：表ID、WHERE子句、ORDER BY子句等
  ↓
访问路径生成：为每个索引创建一条路径
  ↓
成本估算：计算每条路径的成本
  ↓
路径比较：比较不同路径的优劣
  ↓
路径剪枝：保留最优路径
  ↓
输出：最优的访问路径集合
```

### 3. 理解关键数据结构

```cpp
// 最重要的三个数据结构
AccessPath {
  table_id_, index_id_,        // 标识
  est_cost_info_,              // 成本信息
  ordering_, filter_,          // 属性
  sharding_, parallel_         // 分布信息
}

ObCostTableScanInfo {
  ranges_,                     // 查询范围
  output_row_count_,           // 输出行数
  logical_query_range_row_count_,  // 逻辑范围行数
  phy_query_range_row_count_,      // 物理范围行数
}

JoinPath {
  left_path_, right_path_,     // 子树
  join_type_, join_quals_,     // 连接信息
  join_output_rows_,           // 输出行数
}
```

### 4. 使用调试技巧

```cpp
// 添加日志输出
LOG_INFO("path info", K(path->get_cost()), K(path->get_path_output_rows()));

// 查看执行计划
EXPLAIN SELECT ...;
EXPLAIN EXTENDED SELECT ...;

// 使用优化器提示
SELECT /*+ INDEX(t1, idx_name) */ * FROM t1 WHERE ...;
SELECT /*+ PARALLEL(4) */ * FROM t1 WHERE ...;
```

---

## 📈 性能优化清单

使用本指南优化SQL查询的步骤：

- [ ] 1. 收集表的统计信息
  ```sql
  ANALYZE TABLE t1;
  ```

- [ ] 2. 查看执行计划
  ```sql
  EXPLAIN SELECT ...;
  ```

- [ ] 3. 识别问题
  - 是否选择了错误的索引？
  - 连接顺序是否最优？
  - 并行度是否足够？

- [ ] 4. 应用优化
  - 创建更好的索引
  - 使用索引提示
  - 调整查询结构

- [ ] 5. 验证效果
  ```sql
  EXPLAIN SELECT ...;
  -- 比较优化前后的成本
  ```

---

## 🐛 常见问题速查

**Q: 为什么选择了错误的索引？**
A: 查看 `ob_join_order_key_concepts.md` 的 "常见问题和解决方案"

**Q: 如何强制使用特定索引？**
A: 查看 `ob_join_order_key_concepts.md` 的 "最佳实践" 部分

**Q: 并行度为什么这么低？**
A: 查看 `ob_join_order_key_concepts.md` 的 "常见问题和解决方案"

**Q: 如何理解成本模型？**
A: 查看 `ob_join_order_key_concepts.md` 的 "成本模型详解"

**Q: 什么是有趣顺序？**
A: 查看 `ob_join_order_key_concepts.md` 的 "有趣顺序" 部分

**Q: 如何找到特定函数？**
A: 查看 `ob_join_order_function_index.md` 的 "函数分类索引"

---

## 📚 相关资源

### OceanBase官方文档
- [OceanBase SQL优化器文档](https://www.oceanbase.com/docs/)
- [OceanBase源代码仓库](https://github.com/oceanbase/oceanbase)

### SQL优化相关书籍
- "Database Internals" by Alex Petrov
- "Query Optimization" by Goetz Graefe
- "The Art of SQL" by Stephane Faroult

### 相关源文件
- `ob_join_order.h` - 头文件，包含类定义
- `ob_log_plan.cpp` - 逻辑计划生成
- `ob_optimizer.cpp` - 优化器主程序
- `ob_opt_est_cost.cpp` - 成本估算

---

## 📝 笔记模板

在学习过程中，可以使用以下模板记录笔记：

```markdown
## 函数名：[function_name]
**位置**: 行[line_number]
**功能**: [简要描述]
**输入参数**: [参数列表]
**输出**: [返回值]
**关键步骤**:
1. [步骤1]
2. [步骤2]
3. [步骤3]
**调用关系**:
- 被[函数A]调用
- 调用[函数B]
**重要代码片段**:
[代码]
**理解要点**:
- [要点1]
- [要点2]
```

---

## 🎓 总结

通过本指南的学习，你将能够：

✅ 理解OceanBase SQL优化器的工作原理
✅ 理解访问路径的生成和选择过程
✅ 理解成本模型和成本估算方法
✅ 理解路径剪枝和优化策略
✅ 理解连接顺序优化
✅ 理解并行执行优化
✅ 优化SQL查询的性能
✅ 扩展和改进优化器

---

## 📞 反馈和改进

如果你发现文档中有错误或需要改进的地方，欢迎提出建议。

---

**最后更新**: 2025年12月14日
**文档版本**: 1.0
**覆盖范围**: ob_join_order.cpp (20,849 行)

