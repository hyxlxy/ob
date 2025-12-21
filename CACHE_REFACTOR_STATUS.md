# 缓存合规化改造进度

## ✅ 已完成的工作

### 1. 创建符合规范的 ob_kvcache 组件
- ✅ `ObTokenPostingCacheKey` - 缓存键类
- ✅ `ObTokenPostingCacheValue` - 缓存值类
- ✅ `ObTokenPostingCache` - 缓存管理类
- ✅ 添加到 CMakeLists.txt

### 2. 删除违规的自定义缓存
- ✅ 从 `ob_das_tr_merge_iter.h` 删除 `ObGlobalTokenCacheManager`
- ✅ 从 `ob_text_retrieval_token_iter.h` 删除 `ObTRDaaTTokenCacheIter` 类定义
- ✅ 从 `ob_text_retrieval_token_iter.cpp` 删除 `ObTRDaaTTokenCacheIter` 实现
- ✅ 删除 `cache_iter_` 和 `full_scan_done_` 成员

### 3. 添加新的缓存支持成员
- ✅ 在 `ObTextRetrievalDaaTTokenIter` 中添加：
  - `using_cache_` - 是否使用缓存
  - `cached_relevance_` - 缓存的相关性数组
  - `cached_doc_id_` - 缓存的文档ID数组
  - `cached_count_` - 缓存数据量
  - `cache_read_idx_` - 缓存读取位置
  - `cache_handle_` - ob_kvcache 句柄
  - `current_token_` - 当前token
- ✅ 更新构造函数初始化这些成员

## 🚧 剩余工作

### 1. 实现新的缓存方法（ob_text_retrieval_token_iter.cpp）

需要实现两个新方法：

```cpp
int ObTextRetrievalDaaTTokenIter::try_load_from_cache(const ObString &token)
{
  // 1. 创建缓存 key
  // 2. 调用 ObTokenPostingCache::get_instance().get_posting()
  // 3. 如果命中：
  //    - 设置 using_cache_ = true
  //    - 从 cache_handle_ 获取数据指针
  //    - 设置 cached_relevance_, cached_doc_id_, cached_count_
  //    - 初始化 cache_read_idx_ = -1
  // 4. 返回结果
}

int ObTextRetrievalDaaTTokenIter::save_to_cache(const ObString &token)
{
  // 1. 如果 relevance_.count() == 0，直接返回
  // 2. 创建 ObTokenPostingCacheValue
  // 3. 调用 value.init(relevance_.get_data(), doc_id_.get_data(), relevance_.count())
  // 4. 创建缓存 key
  // 5. 调用 ObTokenPostingCache::get_instance().put_posting(key, value)
  // 6. 记录日志
}
```

### 2. 修改 get_next_row() 方法

```cpp
int ObTextRetrievalDaaTTokenIter::get_next_row()
{
  // 如果正在使用缓存：
  if (using_cache_) {
    if (cache_read_idx_ >= cached_count_ - 1) {
      return OB_ITER_END;
    }
    ++cache_read_idx_;
    return OB_SUCCESS;
  }
  
  // 否则正常执行原有逻辑
  // ...
}
```

### 3. 修改 get_curr_score() 和 get_curr_id()

```cpp
int ObTextRetrievalDaaTTokenIter::get_curr_score(double &score) const
{
  if (using_cache_) {
    if (cache_read_idx_ < 0 || cache_read_idx_ >= cached_count_) {
      return OB_ARRAY_OUT_OF_RANGE;
    }
    score = cached_relevance_[cache_read_idx_];
    return OB_SUCCESS;
  }
  
  // 否则使用原有逻辑
  // ...
}
```

### 4. 修改 save_relevances_and_docids()

删除旧的缓存填充逻辑：
```cpp
// 删除这部分代码：
// if (OB_SUCC(ret) && cache_iter_ != nullptr && (!cache_iter_->have_done_flag_)) {
//   if (OB_FAIL(cache_iter_->add_data(relevance_, doc_id_, count_))) {
//     ...
//   }
// }
```

### 5. 修改 create_dim_iters()（ob_das_tr_merge_iter.cpp）

```cpp
for (int64_t i = 0; OB_SUCC(ret) && i < query_tokens_.count(); ++i) {
  ObTextRetrievalScanIterParam iter_param;
  ObTextRetrievalDaaTTokenIter *dim_iter = nullptr;
  
  // 创建迭代器
  if (OB_FAIL(init_dim_iter_param(iter_param, i))) {
    LOG_WARN("failed to init create tr iter param", K(ret));
  } else if (OB_ISNULL(dim_iter = OB_NEWx(ObTextRetrievalDaaTTokenIter, &myself_allocator_))) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    LOG_WARN("failed to allocate memory for text retrieval daat token iter", K(ret));
  } else if (OB_FAIL(dim_iter->init(iter_param))) {
    LOG_WARN("failed to init text retrieval daat token iter", K(ret));
  } else {
    // 尝试从缓存加载
    const ObString &token = query_tokens_.at(i);
    if (OB_FAIL(dim_iter->try_load_from_cache(token))) {
      if (OB_ENTRY_NOT_EXIST != ret) {
        LOG_WARN("failed to load from cache", K(ret), K(token));
      } else {
        // 缓存未命中，正常执行
        ret = OB_SUCCESS;
        LOG_INFO("[TokenCache] Cache miss for token", K(i), K(token));
      }
    } else {
      LOG_INFO("[TokenCache] Cache hit for token", K(i), K(token));
    }
    
    if (OB_SUCC(ret) && OB_FAIL(dim_iters_.push_back(dim_iter))) {
      LOG_WARN("failed to push back dim iter", K(ret));
    }
  }
}
```

### 6. 在查询结束时保存缓存

需要找到合适的位置（查询结束后）调用 `save_to_cache()`。
可能的位置：
- `ObTextRetrievalDaaTTokenIter` 的析构函数
- 或在 `ObDASTRMergeIter` 的某个结束方法中

### 7. 初始化缓存

需要在 observer 启动时初始化缓存：
```cpp
// 在合适的位置（如 observer 初始化）
storage::ObTokenPostingCache::get_instance().init("TokenPostingCache", 1);
```

可能的位置：
- `src/observer/ob_server.cpp` 的初始化方法
- 或 `src/storage/` 下的某个初始化函数

### 8. 编译和测试

- 编译验证无错误
- 运行测试验证功能正确
- 检查日志确认缓存工作
- 验证 recall@10 达标

## 📝 注意事项

1. **tenant_id 获取**：在创建 `ObTokenPostingCacheKey` 时需要 tenant_id，可以从 `MTL_ID()` 获取

2. **ObString 生命周期**：`current_token_` 需要确保在整个查询期间有效

3. **错误处理**：缓存失败不应影响查询正常执行，应该降级到不使用缓存

4. **内存管理**：`cache_handle_` 会自动管理缓存值的生命周期，不需要手动释放

5. **线程安全**：ob_kvcache 已经处理了线程安全，不需要额外加锁

## 🎯 下一步行动

建议按以下顺序完成：
1. 实现 `try_load_from_cache()` 和 `save_to_cache()`
2. 修改 `get_next_row()`, `get_curr_score()`, `get_curr_id()`
3. 清理 `save_relevances_and_docids()` 中的旧代码
4. 修改 `create_dim_iters()`
5. 添加缓存初始化代码
6. 编译测试

预计还需要 2-3 小时完成剩余工作。
