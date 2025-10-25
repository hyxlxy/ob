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
#include "storage/record/record.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

/**
 * @brief fulltext 全文索引
 * @ingroup Index
 */
struct RID;
class Table;
class Tokenizer;

class FullTextIndex : public Index
{
public:
  FullTextIndex() = default;
  virtual ~FullTextIndex() noexcept = default;

  // 生命周期
  RC create(Table *table, const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta) override;
  RC open(Table *table, const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta) override;
  RC close();

  // 维护
  RC insert_entry(const char *record, const RID *rid) override;
  RC delete_entry(const char *record, const RID *rid) override;
  RC sync() override;

  // 非范围扫描接口（全文检索语义使用自定义 search）
  IndexScanner *create_scanner(const char *left_key, int left_len, bool left_inclusive,
      const char *right_key, int right_len, bool right_inclusive) override;

  // 近似或直接检索接口（简化）
  std::vector<RID> ann_search(const std::string &base_text, size_t limit);
  RC search(const std::string &query, size_t limit, std::vector<std::pair<RID,double>> &results);
  RC score(const std::string &query, const RID &rid, double &out_score);
  RC score_text(const std::string &query, const std::string &text, double &out_score);

private:
  struct DirectoryInfo {
    int document_frequency_ = 0;
    std::unordered_set<RID, RIDHash> inserted_docs; // 仅用于维护df
  };

  void clear_all_();
  double bm25_score_(const std::unordered_map<std::string, int> &qtf, const RID &rid) const;
  double bm25_score_tf_(const std::unordered_map<std::string, int> &qtf,
                        const std::unordered_map<std::string, int> &tf,
                        int doc_len) const;

private:
  bool                       inited_ = false;
  class Table               *table_  = nullptr;   // 前向声明避免头依赖
  class Tokenizer           *tokenizer_raw_ = nullptr; // 运行时持有；实现文件中以Jieba实例化

  // 倒排表：term -> (RID -> tf)
  std::unordered_map<std::string, std::unordered_map<RID, int, RIDHash>> postings_;
  // 词典信息
  std::unordered_map<std::string, DirectoryInfo> dict_;
  // 文档长度
  std::unordered_map<RID, int, RIDHash> doc_len_;
  // 全局统计
  int    record_num_ = 0;     // N
  int    tot_tokens_ = 0;     // 所有文档总词数
  double avg_doc_len_ = 0.0;  // 平均文档长度
};
