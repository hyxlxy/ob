/*
 * Copyright (c) 2025 OceanBase.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OCEANBASE_STORAGE_OB_TOKEN_POSTING_CACHE_H_
#define OCEANBASE_STORAGE_OB_TOKEN_POSTING_CACHE_H_

#include "share/cache/ob_kv_storecache.h"
#include "lib/string/ob_string.h"
#include "storage/retrieval/ob_i_sparse_retrieval_iter.h"

namespace oceanbase
{
namespace storage
{

// Token Posting Cache Key: 使用 token 字符串作为 key
class ObTokenPostingCacheKey : public common::ObIKVCacheKey
{
public:
  ObTokenPostingCacheKey();
  ObTokenPostingCacheKey(const uint64_t tenant_id, const common::ObString &token);
  virtual ~ObTokenPostingCacheKey() = default;
  
  virtual bool operator ==(const ObIKVCacheKey &other) const override;
  virtual uint64_t hash() const override;
  virtual int equal(const ObIKVCacheKey &other, bool &equal) const override;
  virtual int hash(uint64_t &hash_value) const override;
  virtual uint64_t get_tenant_id() const override { return tenant_id_; }
  virtual int64_t size() const override;
  virtual int deep_copy(char *buf, const int64_t buf_len, ObIKVCacheKey *&key) const override;
  
  bool is_valid() const;
  TO_STRING_KV(K_(tenant_id), K_(token));
  
private:
  uint64_t tenant_id_;
  common::ObString token_;  // token 字符串
  
private:
  DISALLOW_COPY_AND_ASSIGN(ObTokenPostingCacheKey);
};

// Token Posting Cache Value: 存储 posting list 数据
class ObTokenPostingCacheValue : public common::ObIKVCacheValue
{
public:
  ObTokenPostingCacheValue();
  virtual ~ObTokenPostingCacheValue();
  
  // 初始化缓存值
  int init(const double *relevance_array, 
           const ObDocIdExt *doc_id_array, 
           const int64_t count);
  
  // 获取缓存数据
  int get_data(const double *&relevance_array, 
               const ObDocIdExt *&doc_id_array, 
               int64_t &count) const;
  
  virtual int64_t size() const override;
  virtual int deep_copy(char *buf, const int64_t buf_len, ObIKVCacheValue *&value) const override;
  
  bool is_valid() const;
  TO_STRING_KV(K_(count), K_(deep_copy_size));
  
private:
  int64_t count_;           // posting 数量
  int64_t deep_copy_size_;  // deep copy 所需的总大小
  double *relevance_;       // 相关性分数数组
  ObDocIdExt *doc_id_;      // 文档ID数组
  
private:
  DISALLOW_COPY_AND_ASSIGN(ObTokenPostingCacheValue);
};

// Token Posting Cache Value Handle: 用于持有缓存值的引用
struct ObTokenPostingCacheHandle
{
  const ObTokenPostingCacheValue *value_;
  common::ObKVCacheHandle handle_;
  
  ObTokenPostingCacheHandle() : value_(nullptr), handle_() {}
  virtual ~ObTokenPostingCacheHandle() {}
  
  inline bool is_valid() const { return nullptr != value_ && value_->is_valid() && handle_.is_valid(); }
  inline void reset() { value_ = nullptr; handle_.reset(); }
  
  TO_STRING_KV(KP(value_), K(handle_));
};

// Token Posting Cache: 继承 ObKVCache，接入 seekdb 缓存框架
class ObTokenPostingCache : public common::ObKVCache<ObTokenPostingCacheKey, ObTokenPostingCacheValue>
{
public:
  ObTokenPostingCache();
  virtual ~ObTokenPostingCache();
  
  // 初始化缓存
  int init(const char *cache_name, const int64_t priority);
  
  // 获取缓存
  int get_posting(const ObTokenPostingCacheKey &key, ObTokenPostingCacheHandle &handle);
  
  // 放入缓存
  int put_posting(const ObTokenPostingCacheKey &key, const ObTokenPostingCacheValue &value);
  
  // 获取单例
  static ObTokenPostingCache &get_instance();
  
private:
  DISALLOW_COPY_AND_ASSIGN(ObTokenPostingCache);
};

} // namespace storage
} // namespace oceanbase

#endif // OCEANBASE_STORAGE_OB_TOKEN_POSTING_CACHE_H_
