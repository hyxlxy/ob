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

#ifndef OB_TEXT_RETRIEVAL_TOKEN_ITER_H_
#define OB_TEXT_RETRIEVAL_TOKEN_ITER_H_

#include "lib/container/ob_se_array.h"
#include "sql/engine/expr/ob_expr.h"
#include "sql/das/iter/ob_das_scan_iter.h"
#include "sql/das/ob_das_ir_define.h"
#include "ob_block_max_iter.h"
#include "ob_i_sparse_retrieval_iter.h"
#include <cstdint>
namespace oceanbase
{
namespace storage
{
class ObTableScanParam;
class ObBlockMaxScoreIterator;
struct ObTextRetrievalScanIterParam
{
  ObTextRetrievalScanIterParam()
    : allocator_(nullptr),
      mem_context_(nullptr),
      inv_idx_scan_param_(nullptr),
      inv_idx_agg_param_(nullptr),
      fwd_idx_scan_param_(nullptr),
      inv_idx_scan_iter_(nullptr),
      inv_idx_agg_iter_(nullptr),
      fwd_idx_agg_iter_(nullptr),
      inv_idx_agg_expr_(nullptr),
      fwd_idx_agg_expr_(nullptr),
      eval_ctx_(nullptr),
      relevance_expr_(nullptr),
      inv_scan_doc_length_col_(nullptr),
      inv_scan_domain_id_col_(nullptr),
      inv_idx_agg_cache_mode_(false)
  {}

  ObArenaAllocator *allocator_;
  lib::MemoryContext mem_context_;
  ObTableScanParam *inv_idx_scan_param_;
  ObTableScanParam *inv_idx_agg_param_;
  ObTableScanParam *fwd_idx_scan_param_;
  sql::ObDASScanIter *inv_idx_scan_iter_;
  sql::ObDASScanIter *inv_idx_agg_iter_;
  sql::ObDASScanIter *fwd_idx_agg_iter_;
  sql::ObExpr *inv_idx_agg_expr_;
  sql::ObExpr *fwd_idx_agg_expr_;
  sql::ObEvalCtx *eval_ctx_;
  sql::ObExpr *relevance_expr_;
  sql::ObExpr *inv_scan_doc_length_col_;
  sql::ObExpr *inv_scan_domain_id_col_;
  bool inv_idx_agg_cache_mode_;
};

class TokenCacheData {  
public:  
  ObSEArray<double, 64, ModulePageAllocator> relevance_scores_;  
  ObSEArray<ObDocIdExt, 64, ModulePageAllocator> doc_ids_;  
  int64_t total_count_;  
  bool is_completed_;  
    
  TokenCacheData() : total_count_(0), is_completed_(false) {}

  int add_data(double relevance, ObDocIdExt &doc_id, int64_t count) {
    int ret = OB_SUCCESS;  
    if (count <= 0) {  
      return ret;  
    }  
    if (OB_FAIL(relevance_scores_.push_back(relevance))) {  
      LOG_WARN("failed to push back relevance score", K(ret));  
    } else if (OB_FAIL(doc_ids_.push_back(doc_id))) {  
      LOG_WARN("failed to push back doc id", K(ret));  
      relevance_scores_.pop_back();  
    } else {  
      total_count_ += count;  
    }  
    return ret;  
  }  
  
};
class TokenCacheManager {  
public:  
  hash::ObHashMap<ObString, TokenCacheData*> cache_map_;  
  common::ObArenaAllocator allocator_;  
    
public:  
  TokenCacheManager() : allocator_(ObMemAttr(MTL_ID(), "TokenCacheAlloc")) {  
    cache_map_.create(1024, "TokenCache", "TokenCache");  
  }  
    
  ~TokenCacheManager() {  
    for (auto iter = cache_map_.begin(); iter != cache_map_.end(); ++iter) {  
      if (iter->second) {  
        iter->second->~TokenCacheData();  
      }  
    }  
    cache_map_.destroy();  
  }  
    
  static TokenCacheManager& get_instance() {  
    static TokenCacheManager instance;  
    return instance;  
  }  
    
  TokenCacheData* get_cache_data(const ObString &token) {  
    TokenCacheData *cached_data = nullptr;  
    cache_map_.get_refactored(token, cached_data);  
    return cached_data;  
  }  
    
  int create_cache_data(const ObString &token, TokenCacheData *&cached_data) {  
    int ret = OB_SUCCESS;  
    void *buf = allocator_.alloc(sizeof(TokenCacheData));  
    if (OB_ISNULL(buf)) {  
      ret = OB_ALLOCATE_MEMORY_FAILED;  
    } else {  
      cached_data = new(buf) TokenCacheData();  
      ret = cache_map_.set_refactored(token, cached_data);  
    }  
    return ret;  
  }  
};
using TCManager = TokenCacheManager;
using TCData = TokenCacheData;
class ObTextRetrievalTokenIter final : public ObISparseRetrievalDimIter
{
public:
  ObTextRetrievalTokenIter();
  virtual ~ObTextRetrievalTokenIter() {}

  int init(const ObTextRetrievalScanIterParam &iter_param);
  void reset();
  void reuse();

  // Sparse Retrieval Dimension Iter Interfaces
  virtual int get_next_row() override;
  virtual int get_next_batch(const int64_t capacity, int64_t &count) override;
  virtual int advance_to(const ObDatum &id_datum) override;
  virtual int update_scan_param(const ObString &token, common::ObArenaAllocator &allocator);

  TokenCacheData *own_cache_data_{nullptr};    
  int64_t cache_read_idx_{0};            
  int64_t last_read_count_{0};
  bool use_cache_{false};
  ObString token_name_;
  bool first_init_flag_{false};
  int force_build_cache();
  int init_with_cache();
  int extract_batch_data(int64_t batch_count,
                         ObFixedArray<double, ObIAllocator> &relevances,
                         ObFixedArray<ObDocIdExt, ObIAllocator> &doc_ids);
  int get_next_batch_with_cache(const int64_t capacity, int64_t &count);
  int get_cache_data(int64_t idx, double &relevance, ObDocIdExt &doc_id);  

public:
  sql::ObBitVector *get_skip() { return skip_; }
  int get_token_doc_cnt(int64_t &token_doc_cnt) const;
  double get_max_token_relevance() const { return max_token_relevance_; }
private:
  int init_calc_exprs_in_relevance_expr();
  void clear_batch_wise_evaluated_flag(const int64_t count);
  void clear_row_wise_evaluated_flag();
  int get_next_doc_token_cnt(const bool use_fwd_idx_agg);
  int get_inv_idx_scan_doc_id(ObDocIdExt &doc_id);
  int gen_fwd_idx_scan_range(const ObDocIdExt &doc_id, ObNewRange &scan_range);
  int do_token_cnt_agg(const ObDocIdExt &doc_id);
  int fill_token_cnt_with_doc_len();
  int batch_fill_token_cnt_with_doc_len(const int64_t count);
  int fill_token_doc_cnt();
  int eval_relevance_expr();
  int batch_eval_relevance_expr(const int64_t count);
  int estimate_token_doc_cnt();
  inline bool need_inv_idx_agg() { return inv_idx_agg_iter_ != nullptr; }
  inline bool need_fwd_idx_agg() { return fwd_idx_agg_iter_ != nullptr; }
  inline bool need_calc_relevance() { return inv_idx_agg_iter_ != nullptr; }
  // tools method
  // In ivector2.0, need use the size which is created by precision to alloc the memeory.
  static int set_decimal_int_by_precision(ObDatum &result_datum, const uint64_t decint, const ObPrecision precision);
public:
  static const int64_t FWD_IDX_ROWKEY_COL_CNT = 2;
  static const int64_t INV_IDX_ROWKEY_COL_CNT = 2;
private:
  lib::MemoryContext mem_context_;
  ObArenaAllocator *allocator_;
  ObTableScanParam *inv_idx_scan_param_;
  ObTableScanParam *inv_idx_agg_param_;
  ObTableScanParam *fwd_idx_scan_param_;
  sql::ObDASScanIter *inv_idx_scan_iter_;
  sql::ObDASScanIter *inv_idx_agg_iter_;
  sql::ObDASScanIter *fwd_idx_agg_iter_;
  sql::ObExpr *inv_idx_agg_expr_;
  sql::ObExpr *fwd_idx_agg_expr_;
  sql::ObEvalCtx *eval_ctx_;
  sql::ObExpr *relevance_expr_;
  sql::ObExpr *inv_scan_doc_length_col_; // read from inv_scan_table
  sql::ObExpr *inv_scan_domain_id_col_; // read from inv_scan_table
  sql::ObExpr *doc_token_cnt_expr_; // fill the expr from another expr, and use the expr to calc the relevance_expr_
  common::ObSEArray<sql::ObExpr *, 2> relevance_calc_exprs_;
  sql::ObBitVector *skip_;
  ObObj *fwd_range_objs_;
  int64_t max_batch_size_;
  int64_t token_doc_cnt_;
  double max_token_relevance_;
  ObDocIdExt advance_doc_id_;
  bool token_doc_cnt_calculated_;
  bool inv_idx_agg_cache_mode_;
  bool is_inited_;
  DISALLOW_COPY_AND_ASSIGN(ObTextRetrievalTokenIter);
};

// ADVISE
class ObTRDaaTTokenCacheIter : public ObISRDaaTDimIter {  
public:  
  ObTRDaaTTokenCacheIter(common::ObIAllocator *alloc);  
  virtual ~ObTRDaaTTokenCacheIter() {}  
  
  // 实现基类接口  
  virtual int get_next_row() override;  
  virtual int advance_to(const ObDatum &id_datum) override;  
  virtual int get_curr_score(double &score) const override;  
  virtual int get_curr_id(const ObDatum *&datum) const override;  
  virtual int get_dim_max_score(double &score) override;  
  
  int init_from_cache(ObTRDaaTTokenCacheIter *one);
  
  int add_data(const ObFixedArray<double, ObIAllocator> &relevance,
               const ObFixedArray<ObDocIdExt, ObIAllocator> &doc_id,
               int64_t count);

  
  ObSEArray<double,64, ModulePageAllocator > cached_relevance_;  
  ObSEArray<ObDocIdExt,64, ModulePageAllocator > cached_doc_id_;  
  int64_t cur_idx_;
  int64_t total_count_;
  bool is_created_flag_{false};
  bool have_done_flag_{false};
  common::ObIAllocator *allocator_;  
};

class ADCacheData {  
public:  
  ObSEArray<double, 64, ModulePageAllocator> relevance_scores_;  
  ObSEArray<ObDocIdExt, 64, ModulePageAllocator> doc_ids_;  
  int64_t total_count_;  
  bool is_completed_;  
    
  ADCacheData() : total_count_(0), is_completed_(false) {}  
    
  int add_data(const ObFixedArray<double, ObIAllocator> &relevance,
    const ObFixedArray<ObDocIdExt, ObIAllocator> &doc_id,
    int64_t count);
};

class ADManager {  
public:  
  hash::ObHashMap<ObString, ADCacheData*> cache_map_;  
  common::ObArenaAllocator allocator_;
  ADManager() {
    cache_map_.create(1024, "TokenCache", "TokenCache");
  }
public:  
  static ADManager& get_one() {  
    static ADManager instance;  
    return instance;  
  }  
    
  ADCacheData* get_cache_data(const ObString &token) {  
    ADCacheData *cached_data = nullptr;  
    cache_map_.get_refactored(token, cached_data);  
    return cached_data;  
  }  
    
  int create_cache_data(const ObString &token, ADCacheData *&cached_data) {  
    int ret = OB_SUCCESS;  
    void *buf = allocator_.alloc(sizeof(ADCacheData));  
    if (OB_ISNULL(buf)) {  
      ret = OB_ALLOCATE_MEMORY_FAILED;  
    } else {  
      cached_data = new(buf) ADCacheData();  
      ret = cache_map_.set_refactored(token, cached_data);  
    }  
    return ret;  
  }  
};

class ObTextRetrievalDaaTTokenIter final : public ObISRDaaTDimIter
{
public:
  ObTextRetrievalDaaTTokenIter();
  virtual ~ObTextRetrievalDaaTTokenIter();
  int init(const ObTextRetrievalScanIterParam &iter_param);
  void reuse();
  void reset();
  virtual int get_next_row() override;
  virtual int get_next_batch(const int64_t capacity, int64_t &count) override;
  virtual int advance_to(const ObDatum &id_datum) override;
  virtual int get_curr_score(double &score) const override;
  virtual int get_curr_id(const ObDatum *&id_datum) const override;
public:
  int get_token_doc_cnt(int64_t &token_doc_cnt) const {
    adlog::DEBUG("WARNING! get_token_doc_cnt(int64_t &token_doc_cnt)");
    return token_iter_->get_token_doc_cnt(token_doc_cnt);
  }
  virtual int get_dim_max_score(double &score) override {
    adlog::DEBUG("WARNING! get_dim_max_score(double &score)");
    int ret = OB_SUCCESS;
    score = token_iter_->get_max_token_relevance();
    if (OB_UNLIKELY(score < 0.0)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected max token relevance", K(ret), K(score));
    }
    return ret;
  }
  // ADVISE
  // mutable ObTRDaaTTokenCacheIter *cache_iter_{nullptr};
  // mutable ObTRDaaTTokenCacheIter *own_cache_iter_{nullptr};
  // bool need_to_fill_cache_iter_flag_{false};

  ADCacheData * own_cache_data_{nullptr};  
  bool use_cache_{false};  
  int64_t cache_read_idx_{-1};
  
  ObString token_name_;
  int force_build_cache();
  int init_with_cache();
  bool init_with_cache_flag_{false};

public:
  int save_relevances_and_docids();
  int save_docids();
  ObArenaAllocator *allocator_;
  ObTextRetrievalTokenIter *token_iter_;
  sql::ObEvalCtx *eval_ctx_;
  sql::ObExpr *relevance_expr_;
  sql::ObExpr *inv_scan_domain_id_col_;
  int64_t max_batch_size_;
  int64_t cur_idx_;
  int64_t count_;
  ObFixedArray<double, ObIAllocator> relevance_; // when ~ObFixedArray(), wikll destory itself
  ObFixedArray<ObDocIdExt, ObIAllocator> doc_id_;
  common::ObDatumCmpFuncType cmp_func_;
  bool is_inited_;
  DISALLOW_COPY_AND_ASSIGN(ObTextRetrievalDaaTTokenIter);
};




class ObTextRetrievalBlockMaxIter final : public ObISRDimBlockMaxIter
{
public:
  ObTextRetrievalBlockMaxIter();
  ~ObTextRetrievalBlockMaxIter() {}
  int init(
      const ObTextRetrievalScanIterParam &iter_param,
      const ObBlockMaxScoreIterParam &block_max_iter_param,
      ObTableScanParam &scan_param);
  void reset();
  void reuse();

  virtual int get_next_row() override;
  virtual int get_next_batch(const int64_t capacity, int64_t &count) override;
  virtual int advance_to(const ObDatum &id_datum) override;

  virtual int get_curr_score(double &score) const override;
  virtual int get_curr_id(const ObDatum *&id_datum) const override;
  virtual int get_dim_max_score(double &score) override;
  virtual int advance_shallow(const ObDatum &id_datum, const bool inclusive) override;
  virtual int get_curr_block_max_info(const ObMaxScoreTuple *&max_score_tuple) override;
  virtual bool in_shallow_status() const override;
  // currently, for text retrieval, total_doc_cnt and token_doc_cnt is required before block max calculation
  int init_block_max_iter(const int64_t total_doc_cnt, const double avg_doc_token_cnt);
private:
  int calc_dim_max_score(
      const ObBlockMaxScoreIterParam &block_max_iter_param,
      const ObBlockMaxBM25RankingParam &ranking_param,
      ObTableScanParam &scan_param);
private:
  ObTextRetrievalDaaTTokenIter token_iter_;
  ObBlockMaxScoreIterator block_max_iter_;
  const ObBlockMaxScoreIterParam *block_max_iter_param_;
  ObTableScanParam *block_max_scan_param_;
  ObBlockMaxBM25RankingParam ranking_param_;
  const ObDatum *curr_id_;
  const ObMaxScoreTuple *max_score_tuple_;
  double dim_max_score_;
  bool block_max_inited_;
  bool in_shallow_status_;
  bool is_inited_;
  DISALLOW_COPY_AND_ASSIGN(ObTextRetrievalBlockMaxIter);
};

} // namespace storage
} // namespace oceanbase


#endif
