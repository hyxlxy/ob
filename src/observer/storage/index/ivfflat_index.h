/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include "storage/index/index.h"
#include <vector>

/**
 * @brief ivfflat 向量索引
 * @ingroup Index
 */
class IvfflatIndex : public Index
{
public:
  using Vector = std::vector<float>;

  IvfflatIndex() = default;
  ~IvfflatIndex() noexcept override = default;

  RC create(Table *table, const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta) override;
  RC open(Table *table, const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta) override;
  RC close();

  RC insert_entry(const char *record, const RID *rid) override;
  RC delete_entry(const char *record, const RID *rid) override;
  IndexScanner *create_scanner(const char *left_key, int left_len, bool left_inclusive,
                               const char *right_key, int right_len, bool right_inclusive) override;
  RC sync() override;

  // 向量搜索方法
  RC search(const std::vector<float> &query_vector, size_t limit, std::vector<RID> &result_rids);

  // 基于已训练质心的近似检索（若未训练则返回空）
  std::vector<RID> ann_search(const std::vector<float> &base_vector, size_t limit);

  // 从记录原始数据中解析出向量值（格式：int维度 + float数组）
  std::vector<float> get_vector_from_record(const char *record) const;

  // 训练状态判断（是否已有质心和桶）
  bool has_trained() const { return !centroids_.empty() && !centroids_buckets_.empty(); }

private:
  bool   inited_ = false;
  Table *table_  = nullptr;
  int    lists_  = 1;
  int    probes_ = 1;

  // 质心与倒排桶
  std::vector<Vector>                             centroids_;
  std::vector<std::vector<std::pair<Vector, RID>>> centroids_buckets_;
};
