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

#ifndef OB_TEXT_RETRIEVAL_TOKEN_DF_CACHE_H_
#define OB_TEXT_RETRIEVAL_TOKEN_DF_CACHE_H_

#include "lib/hash_func/murmur_hash.h"
#include "lib/utility/ob_print_utils.h"
#include "lib/time/ob_time_utility.h"
#include "share/cache/ob_kv_storecache.h"

namespace oceanbase
{
namespace storage
{

// Key: identify table/index and token content via stable hash.
struct ObTokenDFCacheKey : public common::ObIKVCacheKey
{
  ObTokenDFCacheKey()
    : index_id_(0), tablet_id_(0), schema_version_(0), token_hash_(0), token_len_(0) {}

  void init(uint64_t index_id, uint64_t tablet_id, int64_t schema_version,
            uint64_t token_hash, uint32_t token_len)
  {
    index_id_ = index_id;
    tablet_id_ = tablet_id;
    schema_version_ = schema_version;
    token_hash_ = token_hash;
    token_len_ = token_len;
  }

  bool is_valid() const { return 0 != index_id_ && 0 != token_hash_; }

  uint64_t hash() const
  {
    return common::murmurhash(this, sizeof(*this), 0x9e3779b97f4a7c15ULL);
  }
  int hash(uint64_t &hash_val) const { hash_val = hash(); return OB_SUCCESS; }

  bool operator==(const ObIKVCacheKey &other) const
  {
    const ObTokenDFCacheKey &o = reinterpret_cast<const ObTokenDFCacheKey&>(other);
    return index_id_ == o.index_id_
        && tablet_id_ == o.tablet_id_
        && schema_version_ == o.schema_version_
        && token_hash_ == o.token_hash_
        && token_len_ == o.token_len_;
  }

  uint64_t get_tenant_id() const { return 0; } // use default tenant bucket

  int64_t size() const { return sizeof(*this); }

  int deep_copy(char *buf, const int64_t buf_len, common::ObIKVCacheKey *&key) const
  {
    int ret = OB_SUCCESS;
    if (nullptr == buf || buf_len < size()) {
      ret = OB_INVALID_ARGUMENT;
      COMMON_LOG(WARN, "invalid buf to deep_copy key", K(ret), KP(buf), K(buf_len), K(size()));
    } else {
      ObTokenDFCacheKey *tmp = new (buf) ObTokenDFCacheKey();
      *tmp = *this;
      key = tmp;
    }
    return ret;
  }

  TO_STRING_KV(K(index_id_), K(tablet_id_), K(schema_version_), K(token_hash_), K(token_len_));

  uint64_t index_id_;
  uint64_t tablet_id_;
  int64_t schema_version_;
  uint64_t token_hash_;
  uint32_t token_len_;
};

struct ObTokenDFCacheValue : public common::ObIKVCacheValue
{
  ObTokenDFCacheValue() : token_doc_cnt_(0), create_time_us_(0) {}
  ObTokenDFCacheValue(int64_t doc_cnt, int64_t create_ts)
    : token_doc_cnt_(doc_cnt), create_time_us_(create_ts) {}

  int64_t size() const { return sizeof(*this); }

  int deep_copy(char *buf, const int64_t buf_len, common::ObIKVCacheValue *&value) const
  {
    int ret = OB_SUCCESS;
    if (nullptr == buf || buf_len < size()) {
      ret = OB_INVALID_ARGUMENT;
      COMMON_LOG(WARN, "invalid buf to deep_copy value", K(ret), KP(buf), K(buf_len), K(size()));
    } else {
      ObTokenDFCacheValue *tmp = new (buf) ObTokenDFCacheValue();
      *tmp = *this;
      value = tmp;
    }
    return ret;
  }

  TO_STRING_KV(K(token_doc_cnt_), K(create_time_us_));

  int64_t token_doc_cnt_;
  int64_t create_time_us_;
};

class ObTokenDFCache : public common::ObKVCache<ObTokenDFCacheKey, ObTokenDFCacheValue>
{};

struct ObTokenDFCacheHandle
{
  ObTokenDFCacheHandle() : value_(nullptr), cache_(nullptr), handle_() {}
  const ObTokenDFCacheValue *value_;
  ObTokenDFCache *cache_;
  common::ObKVCacheHandle handle_;

  void reset()
  {
    value_ = nullptr;
    cache_ = nullptr;
    handle_.reset();
  }
  int assign(const ObTokenDFCacheHandle &other)
  {
    int ret = OB_SUCCESS;
    if (OB_FAIL(handle_.assign(other.handle_))) {
      reset();
    } else {
      value_ = other.value_;
      cache_ = other.cache_;
    }
    return ret;
  }
};

// global cache accessor
extern ObTokenDFCache g_token_df_cache;
int init_token_df_cache();

// 5 minutes TTL by default (microseconds)
static const int64_t TOKEN_DF_CACHE_TTL_US = 5L * 60L * 1000L * 1000L;

inline bool token_df_cache_expired(const ObTokenDFCacheValue &v)
{
  return (common::ObTimeUtility::current_time() - v.create_time_us_) > TOKEN_DF_CACHE_TTL_US;
}

} // namespace storage
} // namespace oceanbase

#endif // OB_TEXT_RETRIEVAL_TOKEN_DF_CACHE_H_


