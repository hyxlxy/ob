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

#include "storage/retrieval/ob_token_posting_cache.h"
#include "lib/oblog/ob_log_module.h"
#include "lib/utility/ob_print_utils.h"

namespace oceanbase
{
namespace storage
{

using namespace common;

// ==================== ObTokenPostingCacheKey ====================

ObTokenPostingCacheKey::ObTokenPostingCacheKey()
  : tenant_id_(OB_INVALID_TENANT_ID), token_()
{
}

ObTokenPostingCacheKey::ObTokenPostingCacheKey(const uint64_t tenant_id, const ObString &token)
  : tenant_id_(tenant_id), token_(token)
{
}

bool ObTokenPostingCacheKey::operator ==(const ObIKVCacheKey &other) const
{
  const ObTokenPostingCacheKey &other_key = reinterpret_cast<const ObTokenPostingCacheKey &>(other);
  return tenant_id_ == other_key.tenant_id_ && token_ == other_key.token_;
}

uint64_t ObTokenPostingCacheKey::hash() const
{
  uint64_t hash_value = 0;
  hash_value = murmurhash(&tenant_id_, sizeof(tenant_id_), hash_value);
  hash_value = token_.hash(hash_value);
  return hash_value;
}

int ObTokenPostingCacheKey::equal(const ObIKVCacheKey &other, bool &equal) const
{
  equal = (*this == other);
  return OB_SUCCESS;
}

int ObTokenPostingCacheKey::hash(uint64_t &hash_value) const
{
  hash_value = hash();
  return OB_SUCCESS;
}

int64_t ObTokenPostingCacheKey::size() const
{
  return sizeof(*this) + token_.length();
}

int ObTokenPostingCacheKey::deep_copy(char *buf, const int64_t buf_len, ObIKVCacheKey *&key) const
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(nullptr == buf || buf_len < size())) {
    ret = OB_INVALID_ARGUMENT;
    STORAGE_LOG(WARN, "invalid arguments", K(ret), KP(buf), K(buf_len), "request_size", size());
  } else if (OB_UNLIKELY(!is_valid())) {
    ret = OB_INVALID_DATA;
    STORAGE_LOG(WARN, "invalid token posting cache key", K(ret), K(*this));
  } else {
    ObTokenPostingCacheKey *pkey = new (buf) ObTokenPostingCacheKey();
    pkey->tenant_id_ = tenant_id_;
    
    // deep copy token string
    int64_t pos = sizeof(*this);
    if (token_.length() > 0) {
      char *token_buf = buf + pos;
      MEMCPY(token_buf, token_.ptr(), token_.length());
      pkey->token_.assign_ptr(token_buf, token_.length());
      pos += token_.length();
    }
    
    key = pkey;
  }
  return ret;
}

bool ObTokenPostingCacheKey::is_valid() const
{
  return OB_INVALID_TENANT_ID != tenant_id_ && !token_.empty();
}

// ==================== ObTokenPostingCacheValue ====================

ObTokenPostingCacheValue::ObTokenPostingCacheValue()
  : count_(0), deep_copy_size_(0), relevance_(nullptr), doc_id_(nullptr)
{
}

ObTokenPostingCacheValue::~ObTokenPostingCacheValue()
{
}

int ObTokenPostingCacheValue::init(const double *relevance_array, 
                                    const ObDocIdExt *doc_id_array, 
                                    const int64_t count)
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(count <= 0 || nullptr == relevance_array || nullptr == doc_id_array)) {
    ret = OB_INVALID_ARGUMENT;
    STORAGE_LOG(WARN, "invalid arguments", K(ret), K(count), KP(relevance_array), KP(doc_id_array));
  } else {
    count_ = count;
    deep_copy_size_ = sizeof(*this) + count * (sizeof(double) + sizeof(ObDocIdExt));
    relevance_ = const_cast<double *>(relevance_array);
    doc_id_ = const_cast<ObDocIdExt *>(doc_id_array);
  }
  return ret;
}

int ObTokenPostingCacheValue::get_data(const double *&relevance_array, 
                                        const ObDocIdExt *&doc_id_array, 
                                        int64_t &count) const
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(!is_valid())) {
    ret = OB_NOT_INIT;
    STORAGE_LOG(WARN, "cache value not initialized", K(ret));
  } else {
    relevance_array = relevance_;
    doc_id_array = doc_id_;
    count = count_;
  }
  return ret;
}

int64_t ObTokenPostingCacheValue::size() const
{
  return deep_copy_size_;
}

int ObTokenPostingCacheValue::deep_copy(char *buf, const int64_t buf_len, ObIKVCacheValue *&value) const
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(nullptr == buf || buf_len < size())) {
    ret = OB_INVALID_ARGUMENT;
    STORAGE_LOG(WARN, "invalid arguments", K(ret), KP(buf), K(buf_len), "request_size", size());
  } else if (OB_UNLIKELY(!is_valid())) {
    ret = OB_INVALID_DATA;
    STORAGE_LOG(WARN, "invalid token posting cache value", K(ret));
  } else {
    ObTokenPostingCacheValue *pvalue = new (buf) ObTokenPostingCacheValue();
    pvalue->count_ = count_;
    pvalue->deep_copy_size_ = deep_copy_size_;
    
    int64_t pos = sizeof(*this);
    
    // deep copy relevance array
    if (count_ > 0 && nullptr != relevance_) {
      double *relevance_buf = reinterpret_cast<double *>(buf + pos);
      MEMCPY(relevance_buf, relevance_, count_ * sizeof(double));
      pvalue->relevance_ = relevance_buf;
      pos += count_ * sizeof(double);
    }
    
    // deep copy doc_id array
    if (count_ > 0 && nullptr != doc_id_) {
      ObDocIdExt *doc_id_buf = reinterpret_cast<ObDocIdExt *>(buf + pos);
      MEMCPY(doc_id_buf, doc_id_, count_ * sizeof(ObDocIdExt));
      pvalue->doc_id_ = doc_id_buf;
      pos += count_ * sizeof(ObDocIdExt);
    }
    
    value = pvalue;
  }
  return ret;
}

bool ObTokenPostingCacheValue::is_valid() const
{
  return count_ > 0 && deep_copy_size_ >= sizeof(*this) && nullptr != relevance_ && nullptr != doc_id_;
}

// ==================== ObTokenPostingCache ====================

ObTokenPostingCache::ObTokenPostingCache()
{
}

ObTokenPostingCache::~ObTokenPostingCache()
{
}

int ObTokenPostingCache::init(const char *cache_name, const int64_t priority)
{
  int ret = OB_SUCCESS;
  const int64_t mem_limit_pct = 10; // 占用 10% 的缓存内存
  if (OB_FAIL((ObKVCache<ObTokenPostingCacheKey, ObTokenPostingCacheValue>::init(cache_name, priority, mem_limit_pct)))) {
    STORAGE_LOG(WARN, "fail to init token posting kv cache", K(ret), K(priority));
  } else {
    STORAGE_LOG(INFO, "Success to init token posting cache", K(cache_name), K(priority));
  }
  return ret;
}

int ObTokenPostingCache::get_posting(const ObTokenPostingCacheKey &key, ObTokenPostingCacheHandle &handle)
{
  int ret = OB_SUCCESS;
  handle.reset();
  if (OB_FAIL(get(key, handle.value_, handle.handle_))) {
    if (OB_ENTRY_NOT_EXIST != ret) {
      STORAGE_LOG(WARN, "fail to get posting from cache", K(ret), K(key));
    }
  }
  return ret;
}

int ObTokenPostingCache::put_posting(const ObTokenPostingCacheKey &key, const ObTokenPostingCacheValue &value)
{
  int ret = OB_SUCCESS;
  const ObTokenPostingCacheValue *pvalue = nullptr;
  ObKVCacheHandle handle;
  const bool overwrite = true;
  if (OB_FAIL(put_and_fetch(key, value, pvalue, handle, overwrite))) {
    if (OB_ENTRY_EXIST != ret) {
      STORAGE_LOG(WARN, "fail to put posting to cache", K(ret), K(key));
    }
  }
  return ret;
}

ObTokenPostingCache &ObTokenPostingCache::get_instance()
{
  static ObTokenPostingCache instance;
  return instance;
}

} // namespace storage
} // namespace oceanbase
