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

#ifndef OBDEV_SRC_SQL_DAS_ITER_OB_DAS_INDEX_MERGE_ITER_H_
#define OBDEV_SRC_SQL_DAS_ITER_OB_DAS_INDEX_MERGE_ITER_H_

#include "sql/das/iter/ob_das_iter.h"
#include "sql/optimizer/ob_join_order.h"
#include "lib/hash/ob_hashset.h"
#include "sql/das/ob_das_ir_define.h"
#include <cstdint>

namespace oceanbase
{
using namespace common;
namespace sql
{

struct ObDASIndexMergeCtDef;
struct ObDASIndexMergeRtDef;

struct ObDASIndexMergeIterParam : public ObDASIterParam
{
public:
  ObDASIndexMergeIterParam()
    : ObDASIterParam(DAS_ITER_INDEX_MERGE),
      merge_type_(INDEX_MERGE_INVALID),
      rowkey_exprs_(nullptr),
      ctdef_(nullptr),
      rtdef_(nullptr),
      child_iters_(nullptr),
      child_scan_rtdefs_(),
      tx_desc_(nullptr),
      snapshot_(nullptr),
      is_reverse_(false)
  {}

  ObIndexMergeType merge_type_; // and 或者 or
  const ExprFixedArray *rowkey_exprs_; // 行键表达式数组，用于判断不同索引的迭代器返回的行是否为同一行
  const ObDASIndexMergeCtDef *ctdef_;
  ObDASIndexMergeRtDef *rtdef_;
  const common::ObIArray<ObDASIter*> *child_iters_;  // 子迭代器数组
  const common::ObIArray<ObDASScanRtDef*> *child_scan_rtdefs_; // 每一个子迭代器的定义（参数
  transaction::ObTxDesc *tx_desc_;
  transaction::ObTxReadSnapshot *snapshot_;
  bool is_reverse_;

  virtual bool is_valid() const
  {
    return rowkey_exprs_ != nullptr &&
           ctdef_ != nullptr &&
           rtdef_ != nullptr &&
           child_iters_ != nullptr &&
           child_scan_rtdefs_ != nullptr &&
           child_iters_->count() == child_scan_rtdefs_->count() &&
           merge_type_ != INDEX_MERGE_INVALID &&
           ObDASIterParam::is_valid();
  }
};
class ObDASTRMergeIter;
class ObDASIndexMergeIter : public ObDASIter
{

public:
  /* used to store and merge child rows */
  struct IndexMergeRowStore // // 临时存储子迭代器的结果
  {
  public:
    IndexMergeRowStore()
      : exprs_(nullptr),
        eval_ctx_(nullptr),
        max_size_(1),
        saved_size_(0),
        cur_idx_(OB_INVALID_INDEX),
        store_rows_(nullptr),
        iter_end_(false)
    {}
    IndexMergeRowStore(const common::ObIArray<ObExpr*> *exprs,
                       ObEvalCtx *eval_ctx,
                       int64_t max_size)
      : exprs_(exprs),
        eval_ctx_(eval_ctx),
        max_size_(max_size),
        saved_size_(0),
        cur_idx_(OB_INVALID_INDEX),
        store_rows_(nullptr),
        iter_end_(false)
    {}

    int init(common::ObIAllocator &allocator,
             const common::ObIArray<ObExpr*> *exprs,
             ObEvalCtx *eval_ctx,
             int64_t max_size);
    int save(bool is_vectorized, int64_t size);
    int to_expr();
    inline bool have_data() const { return cur_idx_ != OB_INVALID_INDEX && cur_idx_ < saved_size_; }
    int64_t rows_cnt() const { return have_data() ? saved_size_ - cur_idx_ : 0; }
    const ObDatum *cur_datums();
    void reuse();
    void reset();
    TO_STRING_KV(K_(exprs),
                 K_(saved_size),
                 K_(cur_idx),
                 K_(iter_end));

  public:
    typedef ObChunkDatumStore::LastStoredRow LastDASStoreRow;
    const common::ObIArray<ObExpr*> *exprs_;
    ObEvalCtx *eval_ctx_;
    int64_t max_size_;
    int64_t saved_size_;
    int64_t cur_idx_; // 当前访问的位置
    LastDASStoreRow *store_rows_;  // 存储行数据的缓冲区
    bool iter_end_;



  };

  /* shared exprs may cause the results on the frame to be overwritten by
   * get_next_rows() of child iters, thus we use a result buffer to temporarily
   * store a batch of result rows and @to_expr() at the end of each iteration.
   */
  // 为了防止表达式的内存位置冲突，所以开了一个Buffer 去存储表达式
  // 
  //
  struct MergeResultBuffer
  {
  public:
    MergeResultBuffer()
      : exprs_(nullptr),
        eval_ctx_(nullptr),
        max_size_(1),
        result_store_("DASIndexMerge"),
        result_store_iter_()
    {}
    int init(int64_t max_size, ObEvalCtx *eval_ctx, const common::ObIArray<ObExpr*> *exprs, common::ObIAllocator &alloc);
    int add_row();
    int to_expr(int64_t size);
    void reuse();
    void reset();

  public:
    const common::ObIArray<ObExpr*> *exprs_;
    ObEvalCtx *eval_ctx_;
    int64_t max_size_;

    ObChunkDatumStore result_store_;           // 结果存储区
    ObChunkDatumStore::Iterator result_store_iter_;
  };

public:
  ObDASIndexMergeIter()
    : ObDASIter(ObDASIterType::DAS_ITER_INDEX_MERGE),
      merge_type_(INDEX_MERGE_INVALID),
      child_iters_(),
      child_stores_(),
      child_scan_rtdefs_(),
      child_scan_params_(),
      child_tablet_ids_(),
      rowkey_exprs_(nullptr),
      mem_ctx_(),
      get_next_row_(nullptr),
      get_next_rows_(nullptr),
      ls_id_(),
      merge_ctdef_(nullptr),
      merge_rtdef_(nullptr),
      tx_desc_(nullptr),
      snapshot_(nullptr),
      is_reverse_(false),
      child_match_against_exprs_(),
      result_buffer_()
  {}

  virtual ~ObDASIndexMergeIter() {}

public:
  virtual int do_table_scan() override;
  virtual int rescan() override;
  virtual void clear_evaluated_flag() override;
  int set_ls_tablet_ids(const ObLSID &ls_id, const ObDASRelatedTabletID &related_tablet_ids);
  ObTableScanParam *get_child_scan_param(int64_t idx) { return child_scan_params_.at(idx); }

protected:
  virtual int inner_init(ObDASIterParam &param) override;
  virtual int inner_reuse() override;
  virtual int inner_release() override;
  virtual int inner_get_next_row() override;
  virtual int inner_get_next_rows(int64_t &count, int64_t capacity) override;

private:
  int compare(int64_t cur_idx, int64_t &output_idx, int &cmp_ret);
  int intersect_get_next_row();
  int intersect_get_next_rows(int64_t &count, int64_t capacity);
  int union_get_next_row();
  int union_get_next_rows(int64_t &count, int64_t capacity);
  int init_scan_param(const share::ObLSID &ls_id,
                      const common::ObTabletID &tablet_id,
                      const sql::ObDASScanCtDef *ctdef,
                      sql::ObDASScanRtDef *rtdef,
                      ObTableScanParam &scan_param);
  int prepare_scan_ranges(ObTableScanParam &scan_param, const ObDASScanRtDef *rtdef);
  void reset_datum_ptr(const common::ObIArray<ObExpr*> *exprs, int64_t size);
  int extract_match_against_exprs(const common::ObIArray<ObExpr*> &exprs, common::ObIArray<ObExpr*> &match_against_exprs);
  int fill_default_values_for_union(const common::ObIArray<ObExpr*> &exprs);
  int save_row_to_result_buffer();
  // Build bitmap for a single BTree index iter using a temporary ObDASScanIter.
  // This temporary iter is independent of child_iters_ and is destroyed after use,
  // so it will not affect the normal index merge execution.
  int build_bitmap_for_single_btree_iter(int64_t iter_idx,
                                         common::hash::ObHashSet<sql::ObDocIdExt, common::hash::NoPthreadDefendMode> &prefilter_bitmap,
                                         int64_t &iter_rows_scanned,
                                         int64_t &iter_docids_inserted);
  // Build prefilter bitmap from BTree index scan results for scalar conditions (e.g., id < 1000, base_id IN (...))
  // This bitmap will be used to pre-filter doc_ids in full-text search
  int build_prefilter_docid_bitmap();
  // Helper function to recursively find FTS iterator (may be wrapped in SORT)
  ObDASTRMergeIter *find_fts_iter_recursive(ObDASIter *iter);

private:
  ObIndexMergeType merge_type_;
  common::ObFixedArray<ObDASIter*, common::ObIAllocator> child_iters_;
  common::ObFixedArray<IndexMergeRowStore, common::ObIAllocator> child_stores_;
  // now child_scan_rtdefs.count() == child_iters.count(), and so do child_scan_params and
  // child_tablet_ids
  // TODO: eliminate those we actually don't need, include index merge child and fts child.
  
  common::ObFixedArray<ObDASScanRtDef*, common::ObIAllocator> child_scan_rtdefs_;
  common::ObFixedArray<ObTableScanParam*, common::ObIAllocator> child_scan_params_;
  common::ObFixedArray<ObTabletID, common::ObIAllocator> child_tablet_ids_;
  const ExprFixedArray *rowkey_exprs_;
  lib::MemoryContext mem_ctx_;
  int (ObDASIndexMergeIter::*get_next_row_)();
  int (ObDASIndexMergeIter::*get_next_rows_)(int64_t&, int64_t);
  ObLSID ls_id_;
  const ObDASIndexMergeCtDef *merge_ctdef_;
  ObDASIndexMergeRtDef *merge_rtdef_;
  transaction::ObTxDesc *tx_desc_;
  transaction::ObTxReadSnapshot *snapshot_;
  bool is_reverse_;
  // need to fill default value for columns that do not have corresponding output when union merge,
  // only relevance score in fulltext search for now.
  common::ObFixedArray<ExprFixedArray, common::ObIAllocator> child_match_against_exprs_;
  MergeResultBuffer result_buffer_;

public:
      // ADVISE
      bool enable_two_phase_ = false;
      bool has_executed_two_phase_ = false;
      double first_phase_min_score_;
      int64_t top_n_count_ = 114514;
      bool has_fts_iter_ = false;
      ObDASTRMergeIter *fts_iter_;      // FTS迭代器指针
  
      int execute_two_phase();
      int first_merge_phase();
      int second_merge_phase();
      int init_two_phase(ObDASIterParam &param);
      int64_t get_top_n_count();
      bool check_mixed_query_with_fts();
      bool check_top_k();
      bool check_support_two_phase();
};

}  // namespace sql
}  // namespace oceanbase


#endif /* OBDEV_SRC_SQL_DAS_ITER_OB_DAS_INDEX_MERGE_ITER_H_ */
