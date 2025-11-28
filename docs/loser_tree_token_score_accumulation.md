# Loser Tree中"同一个文档的多个token分数累加"详解

## 问题

在Loser Tree中，什么叫"同一个文档的多个token分数会累加"？

## 场景示例

假设用户查询：`MATCH(content) AGAINST('数据库 查询')`

### 1. 查询分词

查询文本被分词为两个token：
- Token 1: "数据库"
- Token 2: "查询"

### 2. 创建迭代器

为每个token创建一个迭代器，扫描倒排索引：

```
迭代器0 (iter_idx=0): 扫描token "数据库" 的倒排索引
迭代器1 (iter_idx=1): 扫描token "查询" 的倒排索引
```

### 3. 倒排索引数据

假设倒排索引中的数据如下：

**Token "数据库" 的倒排索引：**
```
doc_id=100, relevance=0.8
doc_id=200, relevance=0.6
doc_id=300, relevance=0.9
```

**Token "查询" 的倒排索引：**
```
doc_id=100, relevance=0.7  ← 注意：doc_id=100也包含"查询"
doc_id=200, relevance=0.5
doc_id=400, relevance=0.8
```

### 4. Loser Tree合并过程

Loser Tree会按doc_id排序，合并两个迭代器的结果：

```
初始状态：
迭代器0: doc_id=100, relevance=0.8
迭代器1: doc_id=100, relevance=0.7

Loser Tree堆顶（champion）: doc_id=100 (来自迭代器0或1)
```

### 5. 累加过程（关键）

当Loser Tree发现堆顶的doc_id还有其他迭代器也包含时，会累加分数：

```cpp
// src/sql/das/ob_text_retrieval_op.cpp:480-512
int ObTextRetrievalMerge::next_disjunctive_document()
{
  double cur_doc_relevance = 0.0;
  bool curr_doc_end = false;
  
  while (!iter_row_heap_->empty() && !curr_doc_end) {
    // 检查是否是唯一的champion（即没有其他相同doc_id的item）
    if (iter_row_heap_->is_unique_champion()) {
      curr_doc_end = true;  // 当前doc_id已经处理完
    }
    
    // 获取堆顶元素
    iter_row_heap_->top(top_item);
    
    // 【关键】累加分数
    cur_doc_relevance += top_item->relevance_;
    // 第一次：cur_doc_relevance = 0.8 (来自迭代器0)
    // 第二次：cur_doc_relevance = 0.8 + 0.7 = 1.5 (来自迭代器1)
    
    // 弹出堆顶
    iter_row_heap_->pop();
  }
  
  // 最终：doc_id=100的总分数 = 0.8 + 0.7 = 1.5
}
```

## 详细执行流程

### 步骤1：初始化Loser Tree

```
迭代器0: doc_id=100, relevance=0.8, iter_idx=0
迭代器1: doc_id=100, relevance=0.7, iter_idx=1

Loser Tree堆：
  [0] doc_id=100, relevance=0.8, iter_idx=0  ← champion
  [1] doc_id=100, relevance=0.7, iter_idx=1
```

### 步骤2：检查是否是唯一champion

```cpp
if (iter_row_heap_->is_unique_champion()) {
  // 返回false，因为有两个相同doc_id的item
  curr_doc_end = false;
}
```

### 步骤3：累加第一个token的分数

```cpp
iter_row_heap_->top(top_item);  // 获取 doc_id=100, relevance=0.8, iter_idx=0
cur_doc_relevance += top_item->relevance_;  // cur_doc_relevance = 0.8
iter_row_heap_->pop();  // 弹出迭代器0的item
```

### 步骤4：继续处理相同doc_id

```
Loser Tree堆：
  [0] doc_id=100, relevance=0.7, iter_idx=1  ← 新的champion
```

```cpp
// 再次检查
if (iter_row_heap_->is_unique_champion()) {
  // 现在返回true，因为只剩下一个item了
  curr_doc_end = true;
}

iter_row_heap_->top(top_item);  // 获取 doc_id=100, relevance=0.7, iter_idx=1
cur_doc_relevance += top_item->relevance_;  // cur_doc_relevance = 0.8 + 0.7 = 1.5
iter_row_heap_->pop();  // 弹出迭代器1的item
```

### 步骤5：处理下一个文档

```
迭代器0: doc_id=200, relevance=0.6, iter_idx=0
迭代器1: doc_id=200, relevance=0.5, iter_idx=1

重复上述过程：
- doc_id=200的总分数 = 0.6 + 0.5 = 1.1
```

## 关键代码解析

### 1. Loser Tree比较函数

```cpp
// src/sql/das/iter/ob_das_text_retrieval_merge_iter.cpp:66-84
int ObIRIterLoserTreeCmp::cmp(
    const ObIRIterLoserTreeItem &l,
    const ObIRIterLoserTreeItem &r,
    int64_t &cmp_ret)
{
  // 比较两个item的doc_id
  // 通过iter_idx从iter_doc_ids_数组中获取doc_id
  cmp_func_(iter_doc_ids_->at(l.iter_idx_).get_datum(), 
            iter_doc_ids_->at(r.iter_idx_).get_datum(), 
            tmp_ret);
  
  // 如果cmp_ret == 0，说明两个item的doc_id相同
  // 如果cmp_ret < 0，说明l的doc_id < r的doc_id
  // 如果cmp_ret > 0，说明l的doc_id > r的doc_id
}
```

### 2. 判断是否是唯一champion

```cpp
// src/storage/access/ob_scan_merge_loser_tree.h
bool is_unique_champion() const {
  // 返回true表示当前champion是唯一的
  // 返回false表示还有其他相同doc_id的item在堆中
}
```

### 3. 累加逻辑

```cpp
// src/sql/das/ob_text_retrieval_op.cpp:488-501
while (!iter_row_heap_->empty() && !curr_doc_end) {
  if (iter_row_heap_->is_unique_champion()) {
    curr_doc_end = true;  // 当前doc_id已经处理完所有token
  }
  
  iter_row_heap_->top(top_item);
  cur_doc_relevance += top_item->relevance_;  // 累加分数
  iter_row_heap_->pop();
}
```

## 完整示例

### 查询：`MATCH(content) AGAINST('数据库 查询')`

### 倒排索引数据

| Token | Doc ID | Relevance |
|-------|--------|-----------|
| "数据库" | 100 | 0.8 |
| "数据库" | 200 | 0.6 |
| "数据库" | 300 | 0.9 |
| "查询" | 100 | 0.7 |
| "查询" | 200 | 0.5 |
| "查询" | 400 | 0.8 |

### Loser Tree合并结果

| Doc ID | Token "数据库"分数 | Token "查询"分数 | 累加总分 |
|--------|-------------------|-----------------|----------|
| 100 | 0.8 | 0.7 | **1.5** |
| 200 | 0.6 | 0.5 | **1.1** |
| 300 | 0.9 | - | **0.9** |
| 400 | - | 0.8 | **0.8** |

### 执行过程

```
1. Loser Tree初始化：
   堆顶: doc_id=100 (来自"数据库", relevance=0.8)
   堆中: doc_id=100 (来自"查询", relevance=0.7)

2. 处理doc_id=100：
   - 累加: 0.8 + 0.7 = 1.5
   - 输出: doc_id=100, relevance=1.5

3. 处理doc_id=200：
   - 累加: 0.6 + 0.5 = 1.1
   - 输出: doc_id=200, relevance=1.1

4. 处理doc_id=300：
   - 只有"数据库"token，分数=0.9
   - 输出: doc_id=300, relevance=0.9

5. 处理doc_id=400：
   - 只有"查询"token，分数=0.8
   - 输出: doc_id=400, relevance=0.8
```

## 为什么需要累加？

### 1. 多token查询的语义

当用户查询包含多个token时（如"数据库 查询"），我们希望找到**同时包含这些token的文档**，并且：
- 包含的token越多，相关性越高
- 每个token的分数都应该贡献到最终分数中

### 2. BM25算法的特性

BM25算法为每个token计算一个相关性分数，但：
- 单个token的分数只反映该token与文档的相关性
- 多个token的分数需要合并，才能反映整个查询与文档的相关性

### 3. 累加方式

代码中使用的是**简单累加**（加法）：
```cpp
cur_doc_relevance += top_item->relevance_;
```

这种方式：
- ✅ 简单高效
- ✅ 符合"包含更多token的文档更相关"的直觉
- ✅ 适合OR查询（disjunctive query）

## 特殊情况

### 1. 只有一个token包含该文档

```
doc_id=300: 只有"数据库"token，分数=0.9
→ 累加结果 = 0.9（只有一个分数）
```

### 2. 多个token都包含该文档

```
doc_id=100: "数据库"(0.8) + "查询"(0.7) = 1.5
→ 累加结果 = 1.5（两个分数相加）
```

### 3. 没有token包含该文档

```
该文档不会出现在任何迭代器的结果中
→ 不会进入Loser Tree
```

## 总结

**"同一个文档的多个token分数会累加"的含义：**

1. **同一个文档**：指相同的doc_id
2. **多个token**：查询被分词为多个token，每个token有独立的迭代器
3. **分数累加**：如果多个token的迭代器都返回了同一个doc_id，那么这些token的relevance分数会被累加（相加）起来，作为该文档的最终相关性分数

**累加的目的：**
- 反映文档与整个查询（多个token）的相关性
- 包含更多查询token的文档会得到更高的总分
- 符合全文检索的语义：匹配更多关键词的文档更相关

