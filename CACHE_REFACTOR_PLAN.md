# 缓存合规化改造方案调整

## 当前困境

### 原设计的问题
原自定义缓存使用了**渐进式填充**模式：
1. 第一次遇到 token 时，创建空的缓存迭代器
2. 在查询执行过程中，逐步调用 `add_data()` 填充缓存
3. 填充完成后设置 `have_done_flag_ = true`
4. 后续查询检查标志，如果为 true 则使用缓存

### ob_kvcache 的约束
ob_kvcache 要求：
- `put()` 时必须提供**完整的、不可变的数据**
- 不支持渐进式填充
- Value 必须实现 `deep_copy()`，在放入缓存时会立即复制

## 新方案：两阶段缓存

### 方案 A：简化为单次 put（推荐）

**核心思路**：
1. **第一次查询**：正常执行，不使用缓存
2. **查询结束时**：将完整的 posting list 数据 `put()` 到 ob_kvcache
3. **后续查询**：先 `get()` 缓存，如果命中则直接使用

**优势**：
- 完全符合 ob_kvcache 设计
- 逻辑简单清晰
- 无需中间状态管理

**实现步骤**：
1. 删除 `ObTRDaaTTokenCacheIter` 类（不再需要）
2. 在 `ObTextRetrievalDaaTTokenIter` 中：
   - 查询开始时尝试从 ob_kvcache 获取数据
   - 如果命中，直接使用缓存数据
   - 如果未命中，正常执行查询
   - 查询结束时，将数据放入 ob_kvcache
3. 在 `create_dim_iters()` 中：
   - 尝试从 ob_kvcache 获取缓存
   - 如果命中，创建一个使用缓存数据的迭代器
   - 如果未命中，创建正常的迭代器

### 方案 B：保留强制全扫描（复杂）

如果必须保留"第一次查询就强制全扫描填充缓存"的逻辑：

**实现步骤**：
1. 在 `force_full_scan_for_cache()` 中：
   - 使用临时数组收集所有 posting 数据
   - 全扫描完成后，创建 `ObTokenPostingCacheValue`
   - 调用 `ObTokenPostingCache::put_posting()` 放入缓存
2. 在 `get_next_row()` 中：
   - 先检查 ob_kvcache
   - 如果未命中且需要缓存，触发强制全扫描
   - 全扫描会将数据放入 ob_kvcache
   - 然后从 ob_kvcache 重新获取数据使用

**劣势**：
- 逻辑复杂
- 需要额外的内存来临时存储数据
- 第一次查询会更慢（全扫描 + 缓存写入）

## 推荐实现：方案 A

### 修改点

#### 1. 删除不需要的类
- 删除 `ObTRDaaTTokenCacheIter`
- 删除 `ObGlobalTokenCacheManager`

#### 2. 修改 `ObTextRetrievalDaaTTokenIter`
```
class ObTextRetrievalDaaTTokenIter {
private:
  // 添加缓存相关成员
  bool using_cache_;  // 是否正在使用缓存数据
  const double *cached_relevance_;  // 缓存的相关性数组
  const ObDocIdExt *cached_doc_id_;  // 缓存的文档ID数组
  int64_t cached_count_;  // 缓存的数据量
  int64_t cache_idx_;  // 当前缓存读取位置
  storage::ObTokenPostingCacheHandle cache_handle_;  // 缓存句柄
  
  // 添加方法
  int try_load_from_cache();  // 尝试从缓存加载
  int save_to_cache();  // 保存到缓存
};
```

#### 3. 修改 `create_dim_iters()`
```
// 伪代码
for each token:
  // 尝试从 ob_kvcache 获取
  ObTokenPostingCacheKey key(tenant_id, token);
  ObTokenPostingCacheHandle handle;
  
  if (ObTokenPostingCache::get_instance().get_posting(key, handle) == OB_SUCCESS) {
    // 缓存命中，创建使用缓存的迭代器
    create_cached_iterator(handle);
  } else {
    // 缓存未命中，创建正常迭代器
    create_normal_iterator();
  }
```

#### 4. 初始化缓存
在 observer 启动时初始化缓存：
```cpp
// 在合适的位置（如 observer 初始化）
ObTokenPostingCache::get_instance().init("TokenPostingCache", 1 /*priority*/);
```

## 下一步行动

1. 删除 `ObTRDaaTTokenCacheIter` 相关代码
2. 修改 `ObTextRetrievalDaaTTokenIter` 添加缓存支持
3. 修改 `create_dim_iters()` 使用 ob_kvcache
4. 找到合适的位置初始化缓存
5. 编译测试验证
