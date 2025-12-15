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

#ifndef OCEANBASE_STORAGE_RETRIEVAL_OB_FT_TERM_STAT_CACHE_H_
#define OCEANBASE_STORAGE_RETRIEVAL_OB_FT_TERM_STAT_CACHE_H_

#include "lib/alloc/alloc_assist.h"
#include "lib/hash_func/murmur_hash.h"
#include "lib/ob_errno.h"
#include "lib/string/ob_string.h"
#include "lib/utility/ob_print_utils.h"
#include "share/cache/ob_kv_storecache.h"
#include "share/cache/ob_kvcache_struct.h"

namespace oceanbase
{
namespace storage
{

/*
  轻量级全文 term 统计缓存：
  - Key: (tenant_id, index_id, schema_version, collation, token_hash, token_len)
  - Value: doc_freq（该 term 的文档频次），创建时间（可用于调试）
  说明：
  - 仅缓存可复用的元数据，非查询结果；遵循题目约束。
  - 通过 LRU 淘汰；不做显式 TTL（可扩展）。
*/

struct ObFTTermStatKey : public common::ObIKVCacheKey
{
public:
  ObFTTermStatKey()
    : tenant_id_(0), index_id_(0), schema_version_(0), cs_type_(0), token_hash_(0), token_len_(0) {}
  ObFTTermStatKey(uint64_t tenant_id,
                  uint64_t index_id,
                  int64_t schema_version,
                  int32_t cs_type,
                  uint64_t token_hash,
                  int32_t token_len)
    : tenant_id_(tenant_id), index_id_(index_id), schema_version_(schema_version),
      cs_type_(cs_type), token_hash_(token_hash), token_len_(token_len) {}

  bool operator==(const ObIKVCacheKey &other) const override
  {
    const ObFTTermStatKey &o = reinterpret_cast<const ObFTTermStatKey &>(other);
    return (&o == this)
      || (tenant_id_ == o.tenant_id_
          && index_id_ == o.index_id_
          && schema_version_ == o.schema_version_
          && cs_type_ == o.cs_type_
          && token_hash_ == o.token_hash_
          && token_len_ == o.token_len_);
  }

  uint64_t hash() const override
  {
    uint64_t h = 0;
    h = murmurhash(&tenant_id_, sizeof(tenant_id_), h);
    h = murmurhash(&index_id_, sizeof(index_id_), h);
    h = murmurhash(&schema_version_, sizeof(schema_version_), h);
    h = murmurhash(&cs_type_, sizeof(cs_type_), h);
    h = murmurhash(&token_hash_, sizeof(token_hash_), h);
    h = murmurhash(&token_len_, sizeof(token_len_), h);
    return h;
  }

  int equal(const ObIKVCacheKey &other, bool &equal) const override
  {
    equal = (*this == other);
    return common::OB_SUCCESS;
  }

  int hash(uint64_t &hash_val) const override
  {
    hash_val = hash();
    return common::OB_SUCCESS;
  }

  uint64_t get_tenant_id() const override { return tenant_id_; }
  int64_t size() const override { return sizeof(ObFTTermStatKey); }

  int deep_copy(char *buf, const int64_t buf_len, ObIKVCacheKey *&key) const override
  {
    int ret = common::OB_SUCCESS;
    if (nullptr == buf || buf_len < size()) {
      ret = common::OB_INVALID_ARGUMENT;
    } else {
      ObFTTermStatKey *k = new (buf) ObFTTermStatKey(tenant_id_, index_id_, schema_version_, cs_type_, token_hash_, token_len_);
      key = k;
    }
    return ret;
  }

  TO_STRING_KV(K_(tenant_id), K_(index_id), K_(schema_version), K_(cs_type), K_(token_hash), K_(token_len));

public:
  uint64_t tenant_id_;
  uint64_t index_id_;
  int64_t schema_version_;
  int32_t cs_type_;
  uint64_t token_hash_;
  int32_t token_len_;
};

struct ObFTTermStatValue : public common::ObIKVCacheValue
{
public:
  ObFTTermStatValue() : doc_freq_(0), create_ts_(0) {}
  ObFTTermStatValue(int64_t df, int64_t ts) : doc_freq_(df), create_ts_(ts) {}
  int64_t size() const override { return sizeof(ObFTTermStatValue); }
  int deep_copy(char *buf, const int64_t buf_len, ObIKVCacheValue *&value) const override
  {
    int ret = common::OB_SUCCESS;
    if (nullptr == buf || buf_len < size()) {
      ret = common::OB_INVALID_ARGUMENT;
    } else {
      ObIKVCacheValue *v = new (buf) ObFTTermStatValue(doc_freq_, create_ts_);
      value = v;
    }
    return ret;
  }
  TO_STRING_KV(K_(doc_freq), K_(create_ts));
  int64_t doc_freq_;
  int64_t create_ts_;
};

class ObFTTermStatCache : public common::ObKVCache<ObFTTermStatKey, ObFTTermStatValue>
{
public:
  ObFTTermStatCache() {}
  virtual ~ObFTTermStatCache() {}

  static ObFTTermStatCache &get_instance()
  {
    static ObFTTermStatCache inst;
    return inst;
  }

  int get_stat(const ObFTTermStatKey &key,
               const ObFTTermStatValue *&value,
               common::ObKVCacheHandle &handle)
  {
    return get(key, value, handle);
  }

  int put_and_fetch_stat(const ObFTTermStatKey &key,
                         const ObFTTermStatValue &val,
                         const ObFTTermStatValue *&pval,
                         common::ObKVCacheHandle &handle)
  {
    return put_and_fetch(key, val, pval, handle);
  }
};

} // namespace storage
} // namespace oceanbase

#endif // OCEANBASE_STORAGE_RETRIEVAL_OB_FT_TERM_STAT_CACHE_H_

