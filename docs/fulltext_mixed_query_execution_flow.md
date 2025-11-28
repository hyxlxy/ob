# 混合标量全文查询的实际执行流程

## 问题澄清

之前我们可能误解了混合标量全文查询的执行顺序。通过深入分析代码，实际的执行流程是：

## 实际执行流程

### 阶段1：全文索引查询（先执行）

#### 1.1 Token迭代器创建
- **位置**：`src/sql/das/iter/ob_das_text_retrieval_merge_iter.cpp`
- **流程**：
  - 对查询文本进行分词，得到多个token
  - 为每个token创建一个专属的迭代器（`ObDASTextRetrievalIter`）
  - 每个迭代器负责扫描该token对应的倒排索引（inv_index）

#### 1.2 BM25相关性分数计算
- **位置**：`src/storage/retrieval/ob_text_retrieval_iterator.cpp`
- **流程**：
  - 每个token迭代器在扫描倒排索引时，计算BM25相关性分数
  - 分数基于：
    - 词频（TF）
    - 逆文档频率（IDF）
    - 文档长度归一化

#### 1.3 Loser Tree合并文档列表
- **位置**：`src/sql/das/ob_text_retrieval_op.cpp` 和 `src/sql/das/iter/ob_das_text_retrieval_merge_iter.cpp`
- **流程**：
  - 使用Loser Tree（败者树）合并所有token迭代器的文档列表
  - 按doc_id排序
  - 同一文档的多个token分数会累加，得到最终相关性总分
  - **关键代码**：
    ```cpp
    // src/sql/das/ob_text_retrieval_op.cpp:445-478
    int ObTextRetrievalMerge::pull_next_batch_rows()
    {
      // 从各个token迭代器拉取数据
      for (int64_t i = 0; OB_SUCC(ret) && i < next_batch_cnt_; ++i) {
        storage::ObTextRetrievalIterator *iter = token_iters_.at(iter_idx);
        iter->get_next_row();  // 获取下一个文档
        fill_loser_tree_item(*iter, iter_idx, item);
        iter_row_heap_->push(item);  // 推入Loser Tree
      }
      iter_row_heap_->rebuild();  // 重建Loser Tree
    }
    
    // src/sql/das/ob_text_retrieval_op.cpp:480-512
    int ObTextRetrievalMerge::next_disjunctive_document()
    {
      // 合并同一文档的多个token分数
      while (!iter_row_heap_->empty() && !curr_doc_end) {
        iter_row_heap_->top(top_item);
        cur_doc_relevance += top_item->relevance_;  // 累加分数
        iter_row_heap_->pop();
      }
    }
    ```

### 阶段2：标量条件过滤（后执行）

#### 2.1 Doc ID到Rowkey的转换
- **位置**：`src/sql/das/iter/ob_das_iter_utils.cpp`
- **流程**：
  - 全文索引查询返回的是doc_id列表
  - 需要通过doc_id索引表（doc_id_rowkey_index）将doc_id转换为rowkey
  - **关键代码**：
    ```cpp
    // src/sql/das/iter/ob_das_iter_utils.cpp:2188-2211
    // 创建doc_id lookup迭代器
    ObDASCacheLookupIterParam doc_id_lookup_param;
    doc_id_lookup_param.index_table_iter_ = rowkey_scan_iter;
    doc_id_lookup_param.data_table_iter_ = docid_rowkey_table_iter;
    ```

#### 2.2 主表Lookup和标量过滤
- **位置**：`src/sql/das/ob_das_scan_op.cpp` 和 `src/sql/das/iter/ob_das_text_retrieval_merge_iter.cpp`
- **流程**：
  - 使用转换后的rowkey进行主表lookup
  - **标量过滤器（如base_id、id<1000）在此时应用**
  - 通过`pushdown_filters`在主表扫描时执行过滤
  - **关键代码**：
    ```cpp
    // src/sql/das/ob_das_scan_op.cpp:1972-1974
    if (!lookup_ctdef_->pd_expr_spec_.pushdown_filters_.empty()) {
      scan_param_.op_filters_ = &lookup_ctdef_->pd_expr_spec_.pushdown_filters_;
    }
    scan_param_.pd_storage_filters_ = lookup_rtdef_->p_pd_expr_op_->pd_storage_filters_;
    ```

## 执行顺序总结

```
1. 查询文本分词 → 得到多个token
   ↓
2. 为每个token创建迭代器，扫描倒排索引
   ↓
3. 计算每个token的BM25相关性分数
   ↓
4. 使用Loser Tree合并所有token的文档列表
   ↓
5. 按doc_id排序，累加同一文档的多个token分数
   ↓
6. 得到doc_id列表（带相关性分数）
   ↓
7. 通过doc_id索引表将doc_id转换为rowkey
   ↓
8. 使用rowkey进行主表lookup
   ↓
9. 【标量过滤器在此应用】如base_id、id<1000等条件
   ↓
10. 返回最终结果
```

## 关键代码位置

### 全文索引查询核心代码

1. **Token迭代器管理**：
   - `src/sql/das/iter/ob_das_text_retrieval_merge_iter.h:169`
   - `ObDASTokenRetrievalIterArray token_iters_;`

2. **Loser Tree实现**：
   - `src/sql/das/ob_text_retrieval_op.cpp:445-478` - `pull_next_batch_rows()`
   - `src/sql/das/ob_text_retrieval_op.cpp:480-512` - `next_disjunctive_document()`
   - `src/sql/das/iter/ob_das_text_retrieval_merge_iter.h:62` - `ObIRIterLoserTree`

3. **BM25分数计算**：
   - `src/storage/retrieval/ob_text_retrieval_iterator.cpp`

### 标量过滤应用代码

1. **Lookup阶段过滤器设置**：
   - `src/sql/das/ob_das_scan_op.cpp:1972-1975`
   - `src/sql/das/iter/ob_das_iter_utils.cpp:249-252`

2. **过滤器分类**：
   - `src/sql/optimizer/ob_log_plan.cpp:3175-3187` - 分离match_filters和non_match_filters
   - `src/sql/optimizer/ob_log_table_scan.cpp:999-1005` - 标量过滤器放入lookup_pushdown_filters

## 为什么是这个顺序？

### 性能考虑

1. **全文索引查询先执行**：
   - 倒排索引通常比主表小得多
   - 可以先快速筛选出相关的doc_id
   - 减少后续需要lookup的数据量

2. **标量过滤后执行**：
   - 标量条件（如base_id、id<1000）需要访问主表的完整行数据
   - 如果先执行标量过滤，需要扫描大量主表数据
   - 先通过全文索引缩小范围，再应用标量过滤，效率更高

### 数据流

```
倒排索引（小） → doc_id列表 → doc_id索引表 → rowkey列表 → 主表（大）→ 标量过滤 → 结果
```

## 与之前分析的差异

### 之前的误解
- 认为先执行标量过滤，再进行全文索引查询
- 认为标量过滤可以在倒排索引扫描之前应用

### 实际情况
- **先执行全文索引查询**，得到doc_id列表
- **后执行标量过滤**，在lookup主表时应用
- 标量过滤无法在倒排索引扫描之前应用，因为倒排索引只存储doc_id和token的映射关系，不包含标量字段的值

## 关于fts_index表

### 是否使用fts_index表？

从代码分析来看：

1. **倒排索引表（inv_index）**：
   - 存储token到doc_id的映射
   - 用于全文索引查询

2. **doc_id索引表（doc_id_rowkey_index）**：
   - 存储doc_id到rowkey的映射
   - 用于lookup阶段

3. **主表（data table）**：
   - 存储完整的行数据
   - 标量过滤在此应用

**结论**：全文索引查询使用的是**倒排索引表（inv_index）**，而不是fts_index表。fts_index可能是一个统称或者历史命名。

## 验证方法

可以通过以下方式验证执行顺序：

1. **查看执行计划**：
   ```sql
   EXPLAIN SELECT * FROM table 
   WHERE MATCH(content) AGAINST('keyword') 
   AND base_id IN (1,2,3);
   ```

2. **查看日志**：
   - 在`ob_text_retrieval_op.cpp`和`ob_das_scan_op.cpp`中添加日志
   - 观察执行顺序

3. **性能分析**：
   - 如果先执行标量过滤，应该看到大量主表扫描
   - 如果先执行全文索引查询，应该看到倒排索引扫描

## 总结

**用户看到的代码逻辑是正确的**：

1. ✅ 先为每个token建立迭代器，扫描倒排索引
2. ✅ 计算BM25相关性分数
3. ✅ 使用Loser Tree合并所有token的文档列表，按doc_id排序
4. ✅ 累加同一文档的多个token分数
5. ✅ **合并后才执行标量条件**（如base_id、id<1000）

这个流程与我们之前分析的情况是一致的，只是执行顺序的细节需要澄清。

