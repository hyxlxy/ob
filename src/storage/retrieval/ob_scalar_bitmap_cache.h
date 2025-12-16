/*
 * Copyright (c) 2025 OceanBase.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */
#ifndef OCEANBASE_STORAGE_RETRIEVAL_OB_SCALAR_BITMAP_CACHE_H_
#define OCEANBASE_STORAGE_RETRIEVAL_OB_SCALAR_BITMAP_CACHE_H_

#include "lib/hash_func/murmur_hash.h"
#include "lib/ob_errno.h"
#include "share/cache/ob_kv_storecache.h"
#include "share/cache/ob_kvcache_struct.h"

namespace oceanbase
{
namespace storage
{
/*
  标量过滤位图缓存（骨架）：
  - Key: (tenant_id, table_id, index_id, schema_version, col_id, value_hash)
  - Value: 压缩位图（自定义格式占位，后续可替换为 Roaring 序列化或页式TmpFile引用）
  说明：
  - 当前仅提供接口与Key/Value定义，便于后续在执行器中接入。
*/

struct ObScalarBMKey : public common::ObIKVCacheKey
{
  ObScalarBMKey() : tenant_id_(0), table_id_(0), index_id_(0), schema_version_(0), col_id_(0), value_hash_(0) {}
  ObScalarBMKey(uint64_t tenant_id, uint64_t table_id, uint64_t index_id, int64_t schema_version,
                uint64_t col_id, uint64_t value_hash)
    : tenant_id_(tenant_id), table_id_(table_id), index_id_(index_id), schema_version_(schema_version), col_id_(col_id), value_hash_(value_hash) {}
  bool operator==(const ObIKVCacheKey &other) const override
  {
    const ObScalarBMKey &o = reinterpret_cast<const ObScalarBMKey &>(other);
    return (&o == this) || (tenant_id_ == o.tenant_id_ && table_id_ == o.table_id_ && index_id_ == o.index_id_
      && schema_version_ == o.schema_version_ && col_id_ == o.col_id_ && value_hash_ == o.value_hash_);
  }
  uint64_t hash() const override
  {
    uint64_t h = 0; h = murmurhash(&tenant_id_, sizeof(tenant_id_), h);
    h = murmurhash(&table_id_, sizeof(table_id_), h);
    h = murmurhash(&index_id_, sizeof(index_id_), h);
    h = murmurhash(&schema_version_, sizeof(schema_version_), h);
    h = murmurhash(&col_id_, sizeof(col_id_), h);
    h = murmurhash(&value_hash_, sizeof(value_hash_), h);
    return h;
  }
  int equal(const ObIKVCacheKey &other, bool &equal) const override { equal = (*this == other); return common::OB_SUCCESS; }
  int hash(uint64_t &hv) const override { hv = hash(); return common::OB_SUCCESS; }
  uint64_t get_tenant_id() const override { return tenant_id_; }
  int64_t size() const override { return sizeof(*this); }
  int deep_copy(char *buf, const int64_t buf_len, ObIKVCacheKey *&key) const override
  {
    if (nullptr == buf || buf_len < size()) return common::OB_INVALID_ARGUMENT;
    key = new (buf) ObScalarBMKey(tenant_id_, table_id_, index_id_, schema_version_, col_id_, value_hash_);
    return common::OB_SUCCESS;
  }
  TO_STRING_KV(K_(tenant_id), K_(table_id), K_(index_id), K_(schema_version), K_(col_id), K_(value_hash));
  uint64_t tenant_id_;
  uint64_t table_id_;
  uint64_t index_id_;
  int64_t schema_version_;
  uint64_t col_id_;
  uint64_t value_hash_;
};

struct ObScalarBMVal : public common::ObIKVCacheValue
{
  ObScalarBMVal() : bytes_(nullptr), size_(0) {}
  ObScalarBMVal(const void *bytes, int64_t size) : bytes_(const_cast<void *>(bytes)), size_(size) {}
  int64_t size() const override { return sizeof(*this) + size_; }
  int deep_copy(char *buf, const int64_t buf_len, ObIKVCacheValue *&val) const override
  {
    if (nullptr == buf || buf_len < size()) return common::OB_INVALID_ARGUMENT;
    char *payload = buf + sizeof(*this);
    if (size_ > 0 && bytes_ != nullptr) { MEMCPY(payload, bytes_, size_); }
    val = new (buf) ObScalarBMVal(payload, size_);
    return common::OB_SUCCESS;
  }
  TO_STRING_KV(K_(size), KP_(bytes));
  void *bytes_;
  int64_t size_;
};

class ObScalarBMCache : public common::ObKVCache<ObScalarBMKey, ObScalarBMVal>
{
public:
  static ObScalarBMCache &get_instance() { static ObScalarBMCache inst; return inst; }
  int get_bitmap(const ObScalarBMKey &key, const ObScalarBMVal *&val, common::ObKVCacheHandle &h) { return get(key, val, h); }
  int put_and_fetch_bitmap(const ObScalarBMKey &key, const ObScalarBMVal &v, const ObScalarBMVal *&out, common::ObKVCacheHandle &h) { return put_and_fetch(key, v, out, h); }
};

} // namespace storage
} // namespace oceanbase

#endif


