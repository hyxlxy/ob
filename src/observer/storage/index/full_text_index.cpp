/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "full_text_index.h"
#include "storage/table/table.h"
#include "tokenizer/jieba_tokenizer.h"
#include <algorithm>
#include <cmath>

RC FullTextIndex::create(Table *table, const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta)
{
  (void)file_name; // 未使用
  if (inited_) return RC::RECORD_OPENNED;
  Index::init(index_meta, field_meta);
  table_ = table;
  clear_all_();
  tokenizer_raw_ = new JiebaTokenizer();
  inited_ = true;
  return RC::SUCCESS;
}

RC FullTextIndex::open(Table *table, const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta)
{
  (void)file_name; // 未使用
  if (!inited_) {
    Index::init(index_meta, field_meta);
    tokenizer_raw_ = new JiebaTokenizer();
    inited_ = true;
  }
  table_ = table;
  return RC::SUCCESS;
}

RC FullTextIndex::close()
{
  clear_all_();
  if (tokenizer_raw_ != nullptr) {
    delete tokenizer_raw_;
    tokenizer_raw_ = nullptr;
  }
  return RC::SUCCESS;
}

RC FullTextIndex::insert_entry(const char *record, const RID *rid)
{
  if (!inited_ || table_ == nullptr || rid == nullptr) return RC::INTERNAL;
  if (field_meta_.type() != AttrType::TEXTS && field_meta_.type() != AttrType::CHARS) return RC::UNSUPPORTED;

  std::string text;
  if (field_meta_.type() == AttrType::CHARS) {
    const char *ptr = record + field_meta_.offset();
    int         len = field_meta_.len() - (field_meta_.nullable() ? 1 : 0);
    text.assign(ptr, strnlen(ptr, len));
  } else {
    // TEXT: 读取 LOB，头部: offset(8)+length(4)[+nullable]
    const char *head = record + field_meta_.offset();
    uint64_t off     = *(const uint64_t *)head;
    uint32_t len     = *(const uint32_t *)(head + sizeof(uint64_t));
    if (len > 0) {
      table_->read_text_lob(off, len, text);
    } else {
      text.clear();
    }
  }

  std::vector<std::string> tokens;
  std::string to_cut = text;
  if (tokenizer_raw_ != nullptr) {
    tokenizer_raw_->cut(to_cut, tokens);
  }

  std::unordered_map<std::string, int> tf;
  for (const auto &t : tokens) tf[t]++;

  int doc_tokens = 0;
  for (const auto &kv : tf) {
    const std::string &term = kv.first;
    int               freq  = kv.second;
    doc_tokens += freq;
    auto &posting = postings_[term];
    posting[*rid] = freq;
    auto &info = dict_[term];
    if (info.inserted_docs.insert(*rid).second) {
      info.document_frequency_++;
    }
  }

  doc_len_[*rid] = doc_tokens;
  tot_tokens_ += doc_tokens;
  record_num_++;
  avg_doc_len_ = record_num_ > 0 ? (static_cast<double>(tot_tokens_) / record_num_) : 0.0;
  return RC::SUCCESS;
}

RC FullTextIndex::delete_entry(const char *record, const RID *rid)
{
  (void)record;
  if (!inited_ || rid == nullptr) return RC::INTERNAL;
  int removed_tokens = 0;
  for (auto it = postings_.begin(); it != postings_.end(); ++it) {
    auto pit = it->second.find(*rid);
    if (pit != it->second.end()) {
      removed_tokens += pit->second;
      it->second.erase(pit);
      auto &info = dict_[it->first];
      info.inserted_docs.erase(*rid);
      if (info.document_frequency_ > 0) info.document_frequency_--;
    }
  }
  auto dl_it = doc_len_.find(*rid);
  if (dl_it != doc_len_.end()) {
    tot_tokens_ -= dl_it->second;
    doc_len_.erase(dl_it);
  } else {
    tot_tokens_ -= removed_tokens;
  }
  if (record_num_ > 0) record_num_--;
  avg_doc_len_ = record_num_ > 0 ? (static_cast<double>(tot_tokens_) / record_num_) : 0.0;
  return RC::SUCCESS;
}

RC FullTextIndex::sync() { return RC::SUCCESS; }

IndexScanner *FullTextIndex::create_scanner(const char *left_key, int left_len, bool left_inclusive,
                                            const char *right_key, int right_len, bool right_inclusive)
{
  (void)left_key; (void)left_len; (void)left_inclusive; (void)right_key; (void)right_len; (void)right_inclusive;
  return nullptr;
}

std::vector<RID> FullTextIndex::ann_search(const std::string &base_text, size_t limit)
{
  (void)base_text; (void)limit; return {};
}

void FullTextIndex::clear_all_()
{
  postings_.clear();
  dict_.clear();
  doc_len_.clear();
  record_num_ = 0;
  tot_tokens_ = 0;
  avg_doc_len_ = 0.0;
}

double FullTextIndex::bm25_score_(const std::unordered_map<std::string, int> &qtf, const RID &rid) const
{
  if (record_num_ <= 0 || avg_doc_len_ <= 0.0) return 0.0;
  auto itdl = doc_len_.find(rid);
  if (itdl == doc_len_.end()) return 0.0;
  const int doc_len = itdl->second;

  static constexpr double k1 = 1.5;
  static constexpr double b  = 0.75;
  static constexpr double k2 = 100.0;
  const double norm = k1 * ((1.0 - b) + b * (static_cast<double>(doc_len) / avg_doc_len_));

  double score = 0.0;
  for (const auto &kv : qtf) {
    const std::string &term = kv.first;
    int qf = kv.second;
    auto pit = postings_.find(term);
    if (pit == postings_.end()) continue;
    auto dit = pit->second.find(rid);
    if (dit == pit->second.end()) continue;
    int tf = dit->second;
    const auto &info_it = dict_.find(term);
    if (info_it == dict_.end()) continue;
    int df = std::max(0, info_it->second.document_frequency_);
    if (df <= 0 || df > record_num_) continue;

    double idf = std::log(((double)record_num_ - df + 0.5) / (df + 0.5));
    double tfw = ((k1 + 1.0) * tf) / (norm + tf);
    double qfw = ((k2 + 1.0) * qf) / (k2 + qf);
    score += idf * tfw * qfw;
  }
  return score;
}

double FullTextIndex::bm25_score_tf_(const std::unordered_map<std::string, int> &qtf,
                                     const std::unordered_map<std::string, int> &tf,
                                     int doc_len) const
{
  if (record_num_ <= 0 || avg_doc_len_ <= 0.0) return 0.0;
  static constexpr double k1 = 1.5;
  static constexpr double b  = 0.75;
  static constexpr double k2 = 100.0;
  const double norm = k1 * ((1.0 - b) + b * (static_cast<double>(doc_len) / avg_doc_len_));
  double score = 0.0;
  for (const auto &kv : qtf) {
    const std::string &term = kv.first;
    int qf = kv.second;
    auto dit = tf.find(term);
    if (dit == tf.end()) continue;
    int tfv = dit->second;
    auto info_it = dict_.find(term);
    if (info_it == dict_.end()) continue;
    int df = std::max(0, info_it->second.document_frequency_);
    if (df <= 0 || df > record_num_) continue;
    double idf = std::log(((double)record_num_ - df + 0.5) / (df + 0.5));
    double tfw = ((k1 + 1.0) * tfv) / (norm + tfv);
    double qfw = ((k2 + 1.0) * qf) / (k2 + qf);
    score += idf * tfw * qfw;
  }
  return score;
}

RC FullTextIndex::search(const std::string &query, size_t limit, std::vector<std::pair<RID,double>> &results)
{
  results.clear();
  if (!inited_) return RC::INTERNAL;
  std::vector<std::string> qtokens;
  std::string q = query;
  if (tokenizer_raw_ != nullptr) tokenizer_raw_->cut(q, qtokens);
  if (qtokens.empty()) return RC::SUCCESS;

  std::unordered_map<std::string, int> qtf;
  for (auto &t : qtokens) qtf[t]++;

  std::unordered_set<RID, RIDHash> cands;
  for (const auto &kv : qtf) {
    auto it = postings_.find(kv.first);
    if (it == postings_.end()) continue;
    for (const auto &pr : it->second) cands.insert(pr.first);
  }
  std::vector<std::pair<RID,double>> scored;
  scored.reserve(cands.size());
  for (const auto &rid : cands) {
    double s = bm25_score_(qtf, rid);
    if (s > 0.0) scored.emplace_back(rid, s);
  }
  std::sort(scored.begin(), scored.end(), [](const auto &a, const auto &b){
    if (a.second == b.second) return RID::compare(&a.first, &b.first) < 0;
    return a.second > b.second;
  });
  size_t n = std::min(limit, scored.size());
  results.assign(scored.begin(), scored.begin() + n);
  return RC::SUCCESS;
}

RC FullTextIndex::score(const std::string &query, const RID &rid, double &out_score)
{
  if (!inited_) return RC::INTERNAL;
  std::vector<std::string> qtokens;
  std::string q = query;
  if (tokenizer_raw_ != nullptr) tokenizer_raw_->cut(q, qtokens);
  std::unordered_map<std::string, int> qtf;
  for (auto &t : qtokens) qtf[t]++;
  out_score = bm25_score_(qtf, rid);
  return RC::SUCCESS;
}

RC FullTextIndex::score_text(const std::string &query, const std::string &text, double &out_score)
{
  if (!inited_) return RC::INTERNAL;
  std::vector<std::string> qtokens;
  std::string q = query;
  if (tokenizer_raw_ != nullptr) tokenizer_raw_->cut(q, qtokens);
  std::unordered_map<std::string, int> qtf;
  for (auto &t : qtokens) qtf[t]++;

  std::vector<std::string> dtokens;
  std::string d = text;
  if (tokenizer_raw_ != nullptr) tokenizer_raw_->cut(d, dtokens);
  std::unordered_map<std::string, int> tf;
  for (auto &t : dtokens) tf[t]++;
  int doc_len = 0; for (auto &kv : tf) doc_len += kv.second;
  out_score = bm25_score_tf_(qtf, tf, doc_len);
  return RC::SUCCESS;
}



