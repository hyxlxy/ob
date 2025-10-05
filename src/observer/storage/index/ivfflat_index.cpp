/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "storage/index/ivfflat_index.h"
#include "storage/table/table.h"
#include "storage/record/record_scanner.h"
#include "common/log/log.h"
#include <random>
#include <algorithm>
#include <cmath>

using Vector = std::vector<float>;

// 计算L2距离
static float l2_distance(const Vector &a, const Vector &b)
{
  if (a.size() != b.size()) {
    return std::numeric_limits<float>::max();
  }
  
  float sum = 0.0f;
  for (size_t i = 0; i < a.size(); i++) {
    float diff = a[i] - b[i];
    sum += diff * diff;
  }
  return std::sqrt(sum);
}

RC IvfflatIndex::create(Table *table, const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta)
{
  if (inited_) {
    LOG_WARN("ivfflat index already inited");
    return RC::RECORD_OPENNED;
  }
  Index::init(index_meta, field_meta);
  inited_ = true;
  table_  = table;
  LOG_INFO("Creating ivfflat index %s for table %s", file_name, table->name());
  return RC::SUCCESS;
}

RC IvfflatIndex::open(Table *table, const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta)
{
  if (!inited_) {
    Index::init(index_meta, field_meta);
    inited_ = true;
  }
  table_ = table;
  LOG_INFO("Opening ivfflat index %s for table %s", file_name, table->name());
  return RC::SUCCESS;
}

RC IvfflatIndex::close()
{
  // 简化实现：暂时返回成功
  return RC::SUCCESS;
}

RC IvfflatIndex::insert_entry(const char *record, const RID *rid)
{
  if (!inited_) return RC::INTERNAL;
  if (centroids_.empty()) return RC::SUCCESS; // 未训练，延后
  // 找最近质心放入桶
  // 解析向量：格式 [int dim][float...]
  const int *dim_ptr = reinterpret_cast<const int *>(record + field_meta_.offset());
  if (dim_ptr == nullptr || *dim_ptr <= 0) return RC::SUCCESS;
  int dim = *dim_ptr;
  const float *data = reinterpret_cast<const float *>(record + field_meta_.offset() + sizeof(int));
  Vector vec(dim); std::copy(data, data + dim, vec.begin());
  size_t best = 0; float bestd = std::numeric_limits<float>::max();
  for (size_t i = 0; i < centroids_.size(); i++) {
    float d = l2_distance(vec, centroids_[i]);
    if (d < bestd) { bestd = d; best = i; }
  }
  if (best >= centroids_buckets_.size()) centroids_buckets_.resize(centroids_.size());
  centroids_buckets_[best].emplace_back(std::move(vec), *rid);
  return RC::SUCCESS;
}

RC IvfflatIndex::delete_entry(const char *record, const RID *rid)
{
  // 简化实现：暂时返回成功
  return RC::SUCCESS;
}

IndexScanner *IvfflatIndex::create_scanner(const char *left_key, int left_len, bool left_inclusive,
                                           const char *right_key, int right_len, bool right_inclusive)
{
  // 简化实现：返回空指针
  return nullptr;
}

RC IvfflatIndex::sync()
{
  // 简化实现：暂时返回成功
  return RC::SUCCESS;
}

// 向量搜索：若有质心则近似检索，否则全表精确扫描
RC IvfflatIndex::search(const std::vector<float> &query_vector, size_t limit, std::vector<RID> &result_rids)
{
  result_rids.clear();
  if (!inited_ || table_ == nullptr) return RC::INTERNAL;
  if (!centroids_.empty() && !centroids_buckets_.empty()) {
    // 近似：探测最近 probes_ 个簇
    std::vector<std::pair<float, size_t>> centroid_dists;
    centroid_dists.reserve(centroids_.size());
    for (size_t i = 0; i < centroids_.size(); i++) {
      centroid_dists.emplace_back(l2_distance(query_vector, centroids_[i]), i);
    }
    std::sort(centroid_dists.begin(), centroid_dists.end(), [](auto &a, auto &b){ return a.first < b.first; });
    size_t p = std::min(static_cast<size_t>(probes_), centroid_dists.size());
    std::vector<std::pair<float, RID>> cand;
    for (size_t i = 0; i < p; i++) {
      size_t idx = centroid_dists[i].second;
      for (auto &vr : centroids_buckets_[idx]) {
        float d = l2_distance(query_vector, vr.first);
        cand.emplace_back(d, vr.second);
      }
    }
    std::sort(cand.begin(), cand.end(), [](auto &a, auto &b){ return a.first < b.first; });
    size_t n = std::min(cand.size(), limit);
    result_rids.reserve(n);
    for (size_t i = 0; i < n; i++) result_rids.push_back(cand[i].second);
    return RC::SUCCESS;
  }

  // 兜底：全表精确扫描（遍历所有记录）
  std::vector<std::pair<float, RID>> cand;
  RecordScanner *scanner = nullptr;
  RC rc = table_->get_record_scanner(scanner, nullptr, ReadWriteMode::READ_ONLY);
  if (rc != RC::SUCCESS || scanner == nullptr) return rc != RC::SUCCESS ? rc : RC::INTERNAL;
  rc = scanner->open_scan();
  if (rc != RC::SUCCESS) { scanner->close_scan(); delete scanner; return rc; }
  Record rec;
  while ((rc = scanner->next(rec)) == RC::SUCCESS) {
    const char *record = rec.data();
    const int *dim_ptr = reinterpret_cast<const int *>(record + field_meta_.offset());
    if (!dim_ptr || *dim_ptr <= 0) continue;
    int dim = *dim_ptr;
    const float *data = reinterpret_cast<const float *>(record + field_meta_.offset() + sizeof(int));
    Vector vec(dim); std::copy(data, data + dim, vec.begin());
    float d = l2_distance(query_vector, vec);
    cand.emplace_back(d, rec.rid());
  }
  scanner->close_scan();
  delete scanner;
  if (rc != RC::RECORD_EOF && rc != RC::SUCCESS) return rc;

  std::sort(cand.begin(), cand.end(), [](auto &a, auto &b){ return a.first < b.first; });
  size_t n = std::min(cand.size(), limit);
  result_rids.reserve(n);
  for (size_t i = 0; i < n; i++) result_rids.push_back(cand[i].second);
  return RC::SUCCESS;
}
