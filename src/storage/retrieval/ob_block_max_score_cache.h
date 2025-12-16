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
#ifndef OCEANBASE_STORAGE_RETRIEVAL_OB_BLOCK_MAX_SCORE_CACHE_H_
#define OCEANBASE_STORAGE_RETRIEVAL_OB_BLOCK_MAX_SCORE_CACHE_H_

#include "lib/hash_func/murmur_hash.h"
#include "lib/ob_errno.h"
#include "lib/atomic/ob_atomic.h"
#include "share/cache/ob_kv_storecache.h"
#include "share/cache/ob_kvcache_struct.h"
#include "lib/oblog/ob_log_module.h"

namespace oceanbase
{
namespace storage
{

struct ObBMScoreKey : public common::ObIKVCacheKey
{
  ObBMScoreKey() : tenant_id_(0), index_id_(0), tablet_id_(0), schema_version_(0), token_hash_(0) {}
  ObBMScoreKey(uint64_t tenant_id, uint64_t index_id, uint64_t tablet_id, int64_t schema_version, uint64_t token_hash)
    : tenant_id_(tenant_id), index_id_(index_id), tablet_id_(tablet_id), schema_version_(schema_version), token_hash_(token_hash) {}
  bool operator==(const ObIKVCacheKey &other) const override
  {
    const ObBMScoreKey &o = reinterpret_cast<const ObBMScoreKey &>(other);
    return (&o == this) || (tenant_id_ == o.tenant_id_ && index_id_ == o.index_id_ && tablet_id_ == o.tablet_id_ && schema_version_ == o.schema_version_ && token_hash_ == o.token_hash_);
  }
  uint64_t hash() const override
  {
    uint64_t h = 0;
    h = murmurhash(&tenant_id_, sizeof(tenant_id_), h);
    h = murmurhash(&index_id_, sizeof(index_id_), h);
    h = murmurhash(&tablet_id_, sizeof(tablet_id_), h);
    h = murmurhash(&schema_version_, sizeof(schema_version_), h);
    h = murmurhash(&token_hash_, sizeof(token_hash_), h);
    return h;
  }
  int equal(const ObIKVCacheKey &other, bool &equal) const override { equal = (*this == other); return common::OB_SUCCESS; }
  int hash(uint64_t &hv) const override { hv = hash(); return common::OB_SUCCESS; }
  uint64_t get_tenant_id() const override { return tenant_id_; }
  int64_t size() const override { return sizeof(*this); }
  int deep_copy(char *buf, const int64_t buf_len, ObIKVCacheKey *&key) const override
  {
    if (nullptr == buf || buf_len < size()) return common::OB_INVALID_ARGUMENT;
    key = new (buf) ObBMScoreKey(tenant_id_, index_id_, tablet_id_, schema_version_, token_hash_);
    return common::OB_SUCCESS;
  }
  TO_STRING_KV(K_(tenant_id), K_(index_id), K_(tablet_id), K_(schema_version), K_(token_hash));
  uint64_t tenant_id_;
  uint64_t index_id_;
  uint64_t tablet_id_;
  int64_t schema_version_;
  uint64_t token_hash_;
};

struct ObBMScoreVal : public common::ObIKVCacheValue
{
  ObBMScoreVal() : dim_max_score_(0.0), ts_(0) {}
  ObBMScoreVal(double s, int64_t ts) : dim_max_score_(s), ts_(ts) {}
  int64_t size() const override { return sizeof(*this); }
  int deep_copy(char *buf, const int64_t buf_len, ObIKVCacheValue *&val) const override
  {
    if (nullptr == buf || buf_len < size()) return common::OB_INVALID_ARGUMENT;
    val = new (buf) ObBMScoreVal(dim_max_score_, ts_);
    return common::OB_SUCCESS;
  }
  TO_STRING_KV(K_(dim_max_score), K_(ts));
  double dim_max_score_;
  int64_t ts_;
};

class ObBMScoreCache : public common::ObKVCache<ObBMScoreKey, ObBMScoreVal>
{
public:
  static ObBMScoreCache &get_instance() { static ObBMScoreCache inst; return inst; }
  int get_score(const ObBMScoreKey &key, const ObBMScoreVal *&val, common::ObKVCacheHandle &h) { return get(key, val, h); }
  int put_and_fetch_score(const ObBMScoreKey &key, const ObBMScoreVal &v, const ObBMScoreVal *&out, common::ObKVCacheHandle &h) { return put_and_fetch(key, v, out, h); }
  static void stat_hit() { ATOMIC_INC(&hit_); }
  static void stat_miss() { ATOMIC_INC(&miss_); }
  static void dump_stat()
  {
    FLOG_INFO("[FTCache] BlockMaxScoreCache stat", K(hit_), K(miss_));
  }
private:
  ObBMScoreCache() = default;
  ~ObBMScoreCache() override = default;
  static int64_t hit_;
  static int64_t miss_;
};

inline int64_t ObBMScoreCache::hit_ = 0;
inline int64_t ObBMScoreCache::miss_ = 0;

} // namespace storage
} // namespace oceanbase

#endif



