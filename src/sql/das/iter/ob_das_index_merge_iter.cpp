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

#include <cstdint>
#define USING_LOG_PREFIX SQL_DAS
#include "sql/das/iter/ob_das_index_merge_iter.h"
#include "src/sql/das/ob_das_attach_define.h"
namespace oceanbase
{
using namespace common;
namespace sql
{

int ObDASIndexMergeIter::IndexMergeRowStore::init(common::ObIAllocator &allocator,
                                                  const common::ObIArray<ObExpr*> *exprs,
                                                  ObEvalCtx *eval_ctx,
                                                  int64_t max_size)
{
  int ret = OB_SUCCESS;
  if (OB_ISNULL(exprs) || OB_ISNULL(eval_ctx)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected nullptr for init index merge row store", K(ret));
  } else if (OB_ISNULL(store_rows_ =
      static_cast<LastDASStoreRow*>(allocator.alloc(max_size * sizeof(LastDASStoreRow))))) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    LOG_WARN("failed to allocate memory", K(max_size), K(ret));
  } else {
    exprs_ = exprs;
    eval_ctx_ = eval_ctx;
    max_size_ = max_size;
    for (int64_t i = 0; i < max_size_; i++) {
      new (store_rows_ + i) LastDASStoreRow(allocator);
      store_rows_[i].reuse_ = true;
    }
  }

  return ret;
}

int ObDASIndexMergeIter::IndexMergeRowStore::save(bool is_vectorized, int64_t size)
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(size > max_size_) || OB_ISNULL(store_rows_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected error for save store rows", K(size), K_(max_size), K_(store_rows), K(ret));
  } else {
    if (is_vectorized) {
      ObEvalCtx::BatchInfoScopeGuard batch_info_guard(*eval_ctx_);
      batch_info_guard.set_batch_size(size);
      for (int64_t i = 0; OB_SUCC(ret) && i < size; i++) {
        batch_info_guard.set_batch_idx(i);
        if (OB_FAIL(store_rows_[i].save_store_row(*exprs_, *eval_ctx_))) {
          LOG_WARN("index merge iter failed to store rows", K(ret));
        }
      }
    } else if (OB_FAIL(store_rows_[0].save_store_row(*exprs_, *eval_ctx_))) {
      LOG_WARN("index merge iter failed to store rows", K(ret));
    }
  }
  if (OB_SUCC(ret)) {
    cur_idx_ = 0;
    saved_size_ = size;
  }

  return ret;
}

int ObDASIndexMergeIter::IndexMergeRowStore::to_expr()
{
  int ret = OB_SUCCESS;
  ObEvalCtx::BatchInfoScopeGuard batch_info_guard(*eval_ctx_);
  batch_info_guard.set_batch_size(1);
  batch_info_guard.set_batch_idx(0);
  // use deep copy to avoid storage layer sanity check
  if (OB_FAIL(store_rows_[cur_idx_].store_row_->to_expr<true>(*exprs_, *eval_ctx_))) {
    LOG_WARN("index merge iter failed to convert store row to expr", K(ret));
  } else {
    cur_idx_++;
  }
  return ret;
}

const ObDatum *ObDASIndexMergeIter::IndexMergeRowStore::cur_datums()
{
  const ObDatum *datums = nullptr;
  if (OB_LIKELY(cur_idx_ < saved_size_) && OB_NOT_NULL(store_rows_) && OB_NOT_NULL(store_rows_[cur_idx_].store_row_)) {
    datums = store_rows_[cur_idx_].store_row_->cells();
  }
  return datums;
}

void ObDASIndexMergeIter::IndexMergeRowStore::reuse()
{
  cur_idx_ = OB_INVALID_INDEX;
  saved_size_ = 0;
  iter_end_ = false;
}

void ObDASIndexMergeIter::IndexMergeRowStore::reset()
{
  if (OB_NOT_NULL(store_rows_)) {
    for (int64_t i = 0; i < max_size_; i++) {
      store_rows_[i].~LastDASStoreRow();
    }
    store_rows_ = nullptr;
  }
  exprs_ = nullptr;
  eval_ctx_ = nullptr;
  max_size_ = 1;
  saved_size_ = 0;
  cur_idx_ = OB_INVALID_INDEX;
  iter_end_ = false;
}

int ObDASIndexMergeIter::MergeResultBuffer::init(int64_t max_size,
                                                 ObEvalCtx *eval_ctx,
                                                 const common::ObIArray<ObExpr*> *exprs,
                                                 common::ObIAllocator &alloc)
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(max_size <= 0) || OB_ISNULL(eval_ctx) || OB_ISNULL(exprs)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid argument", K(max_size), K(eval_ctx), K(exprs), K(ret));
  } else if (OB_FAIL(result_store_.init(UINT64_MAX, MTL_ID(), ObCtxIds::DEFAULT_CTX_ID, "DASIndexMerge"))) {
    LOG_WARN("failed to init result store", K(ret));
  } else {
    result_store_.set_allocator(alloc);
    max_size_ = max_size;
    eval_ctx_ = eval_ctx;
    exprs_ = exprs;
    alloc_ = &alloc;
    
    // Initialize the heap with the proper allocator
    if (OB_ISNULL(topn_heap_)) {
      typedef common::ObBinaryHeap<TopNRow *, TopNCompare> HeapType;
      topn_heap_ = OB_NEWx(HeapType, (&alloc), compare_, &alloc);
      if (OB_ISNULL(topn_heap_)) {
        ret = OB_ALLOCATE_MEMORY_FAILED;
        LOG_WARN("failed to allocate topn heap", K(ret));
      } else {
        LOG_INFO("topn heap initialized successfully", K_(topn_limit));
      }
    }
  }
  return ret;
}

int ObDASIndexMergeIter::MergeResultBuffer::add_row()
{
  ObEvalCtx::BatchInfoScopeGuard batch_info_guard(*eval_ctx_);
  batch_info_guard.set_batch_size(1);
  batch_info_guard.set_batch_idx(0);
  return result_store_.add_row(*exprs_, eval_ctx_);
}

int ObDASIndexMergeIter::MergeResultBuffer::to_expr(int64_t size)
{
  int ret = OB_SUCCESS;
  int64_t read_size = 0;
  if (OB_FAIL(result_store_.begin(result_store_iter_))) {
    LOG_WARN("failed to begin iterate result store", K(ret));
  } else if (OB_UNLIKELY(!(result_store_iter_.is_valid() && result_store_iter_.has_next()))) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected no available rows", K(ret));
  } else if (OB_FAIL(result_store_iter_.get_next_batch<true>(*exprs_, *eval_ctx_, max_size_, read_size))) {
    LOG_WARN("failed to get next batch from result store", K(ret));
  } else if (OB_UNLIKELY(size != read_size)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected read size not equal to actually size", K(ret));
  }
  return ret;
}

void ObDASIndexMergeIter::MergeResultBuffer::reuse() { reset(); }

void ObDASIndexMergeIter::MergeResultBuffer::reset()
{
  result_store_iter_.reset();
  result_store_.reset();
  
  if (enable_topn_ && topn_heap_ != nullptr) {
    // Free all rows in heap before resetting
    while (topn_heap_->count() > 0) {
      TopNRow *row = topn_heap_->top();
      topn_heap_->pop();
      if (row != nullptr) {
        if (row->row_data_ != nullptr) {
          row->row_data_->~StoredRow();
          alloc_->free(row->row_data_);
        }
        alloc_->free(row);
      }
    }
    topn_heap_->reset();
    
    // DO NOT free the heap itself - it should persist across reset() calls
    // The heap will be freed in the destructor or when the buffer is destroyed
  }
}
//ADVISE
int ObDASIndexMergeIter::MergeResultBuffer::add_row_with_topn()
{
  int ret = OB_SUCCESS;
  double score = 0.0;
  
  LOG_INFO("add_row_with_topn called", K_(enable_topn), K_(topn_limit));
  
  LOG_INFO("add_row_with_topn: starting score extraction", "exprs_count", exprs_->count());
  
  // Debug: print all expressions in output
  for (int64_t i = 0; i < exprs_->count(); ++i) {
    ObExpr *expr = exprs_->at(i);
    if (expr) {
      LOG_INFO("add_row_with_topn: output expr", K(i), "type", expr->type_, "type_name", 
               (expr->type_ == T_FUN_MATCH_AGAINST ? "MATCH_AGAINST" : "OTHER"));
    }
  }
  
  // Safety check
  if (!enable_topn_ || topn_limit_ <= 0) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("add_row_with_topn called but topn not enabled", K(ret), K(enable_topn_), K(topn_limit_));
    LOG_INFO("add_row_with_topn: returning due to topn not enabled");
    return ret;
  }
  
  if (OB_ISNULL(alloc_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("allocator is null", K(ret));
    LOG_INFO("add_row_with_topn: returning due to null allocator");
    return ret;
  }
  
  if (OB_ISNULL(topn_heap_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("topn_heap is null", K(ret));
    LOG_INFO("add_row_with_topn: returning due to null topn_heap");
    return ret;
  }
  
  if (OB_ISNULL(exprs_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("exprs is null", K(ret));
    LOG_INFO("add_row_with_topn: returning due to null exprs");
    return ret;
  }
  
  if (OB_ISNULL(eval_ctx_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("eval_ctx is null", K(ret));
    LOG_INFO("add_row_with_topn: returning due to null eval_ctx");
    return ret;
  }
  
  LOG_INFO("add_row_with_topn: starting score extraction", "exprs_count", exprs_->count());
  
  // Extract score from MATCH AGAINST expression
  ObSEArray<ObExpr *, 4> match_against_exprs;
  for (int64_t i = 0; OB_SUCC(ret) && i < exprs_->count(); ++i) {
    ObExpr *expr = exprs_->at(i);
    LOG_INFO("add_row_with_topn: checking expr", K(i), "expr_ptr", (void*)expr);
    if (OB_ISNULL(expr)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("null expr in exprs", K(ret), K(i));
    } else if (T_FUN_MATCH_AGAINST == expr->type_) {
      LOG_INFO("add_row_with_topn: found MATCH AGAINST expr", K(i));
      if (OB_FAIL(match_against_exprs.push_back(expr))) {
        LOG_WARN("failed to push back match against expr", K(ret));
      }
    }
  }
  
  LOG_INFO("add_row_with_topn: finished expr loop", "match_against_count", match_against_exprs.count(), K(ret));
  
  // Statistics counters
  static int64_t total_rows = 0;
  static int64_t skipped_null = 0;
  static int64_t skipped_len_zero = 0;
  static int64_t skipped_len_invalid = 0;
  static int64_t valid_rows = 0;
  total_rows++;
  
  if (OB_SUCC(ret) && !match_against_exprs.empty()) {
    LOG_INFO("add_row_with_topn: about to extract score from match_against expr");
    ObExpr *score_expr = match_against_exprs.at(0);
    LOG_INFO("add_row_with_topn: got score_expr", "ptr", (void*)score_expr);
    
    if (OB_ISNULL(score_expr)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("score_expr is null", K(ret));
    } else {
      // Use locate_expr_datum to get the datum that should have been filled by to_expr()
      const ObDatum &score_datum = score_expr->locate_expr_datum(*eval_ctx_);
      bool is_evaluated = score_expr->get_eval_info(*eval_ctx_).evaluated_;
      LOG_INFO("add_row_with_topn: located score_datum", "is_null", score_datum.is_null(), 
               "len", score_datum.len_, "ptr", (void*)score_datum.ptr_, "is_evaluated", is_evaluated);
      
      // Handle NULL - this means no match, skip this row
      if (score_datum.is_null()) {
        skipped_null++;
        LOG_INFO("add_row_with_topn: score is null, skipping row", 
                 "total_rows", total_rows, "skipped_null", skipped_null, 
                 "skipped_len_zero", skipped_len_zero, "skipped_len_invalid", skipped_len_invalid,
                 "valid_rows", valid_rows);
        return OB_SUCCESS;
      }
      
      // Check if the datum length is correct for a double
      if (score_datum.len_ == sizeof(double)) {
        // Normal case: valid double score
        score = score_datum.get_double();
        valid_rows++;
        LOG_INFO("add_row_with_topn: extracted score using get_double", K(score));
      } else if (score_datum.len_ == 0) {
        // len=0 means no match (score=0.0)
        // Still add it to heap - don't skip!
        score = 0.0;
        skipped_len_zero++;
        valid_rows++;
        LOG_INFO("add_row_with_topn: len=0, treating as score=0.0", 
                 "total_rows", total_rows, "skipped_len_zero", skipped_len_zero);
      } else {
        // Invalid length - treat as score=0.0 to avoid crash
        score = 0.0;
        skipped_len_invalid++;
        valid_rows++;
        LOG_WARN("add_row_with_topn: invalid datum length, treating as score=0.0", 
                 "len", score_datum.len_, "expected", sizeof(double),
                 "total_rows", total_rows, "skipped_len_invalid", skipped_len_invalid);
      }
    }
  }
  
  LOG_INFO("add_row_with_topn: extracted score", K(score), K(ret),
           "total_rows", total_rows, "skipped_null", skipped_null, 
           "skipped_len_zero", skipped_len_zero, "skipped_len_invalid", skipped_len_invalid,
           "valid_rows", valid_rows);
  
  // Create stored row - this allocates memory that we own
  ObChunkDatumStore::StoredRow *stored_row = nullptr;
  ObEvalCtx::BatchInfoScopeGuard batch_guard(*eval_ctx_);
  batch_guard.set_batch_size(1);
  batch_guard.set_batch_idx(0);
  
  LOG_INFO("add_row_with_topn: building stored row");
  
  if (OB_FAIL(ObChunkDatumStore::StoredRow::build(stored_row, *exprs_, *eval_ctx_, *alloc_))) {
    LOG_WARN("failed to build stored row", K(ret));
    return ret;
  }
  
  LOG_INFO("add_row_with_topn: stored row built successfully");
  
  // Create TopNRow wrapper
  TopNRow *new_row = OB_NEWx(TopNRow, alloc_, stored_row, score);
  if (OB_ISNULL(new_row)) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    LOG_WARN("failed to allocate TopNRow", K(ret));
    if (stored_row != nullptr) {
      stored_row->~StoredRow();
      alloc_->free(stored_row);
    }
    return ret;
  }
  
  LOG_INFO("add_row_with_topn: checking heap", "heap_count", topn_heap_->count(), K_(topn_limit));
  
  // If heap not full, just add
  if (topn_heap_->count() < topn_limit_) {
    if (OB_FAIL(topn_heap_->push(new_row))) {
      LOG_WARN("failed to push to heap", K(ret));
      if (stored_row != nullptr) {
        stored_row->~StoredRow();
        alloc_->free(stored_row);
      }
      alloc_->free(new_row);
    }
    return ret;
  }
  
  // Heap is full - check if new row should replace top
  TopNRow *heap_top = topn_heap_->top();
  if (OB_ISNULL(heap_top)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("heap top is null", K(ret));
    if (stored_row != nullptr) {
      stored_row->~StoredRow();
      alloc_->free(stored_row);
    }
    alloc_->free(new_row);
    return ret;
  }
  
  // If new score is higher than the minimum (top of min-heap), replace it
  if (new_row->score_ > heap_top->score_) {
    topn_heap_->pop();
    
    // Free the old top row
    if (heap_top->row_data_ != nullptr) {
      heap_top->row_data_->~StoredRow();
      alloc_->free(heap_top->row_data_);
    }
    alloc_->free(heap_top);
    
    // Push new row
    if (OB_FAIL(topn_heap_->push(new_row))) {
      LOG_WARN("failed to push to heap after pop", K(ret));
      if (stored_row != nullptr) {
        stored_row->~StoredRow();
        alloc_->free(stored_row);
      }
      alloc_->free(new_row);
    }
  } else {
    // New row doesn't make the cut - free it
    if (stored_row != nullptr) {
      stored_row->~StoredRow();
      alloc_->free(stored_row);
    }
    alloc_->free(new_row);
  }
  
  return ret;
}


int ObDASIndexMergeIter::MergeResultBuffer::to_expr_from_topn(int64_t &size)  
{
  int ret = OB_SUCCESS;
  common::ObArray<TopNRow *> sorted_rows;
  
  if (OB_ISNULL(topn_heap_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("topn_heap is null", K(ret));
    return ret;
  }
  
  // Extract all rows from heap
  while (topn_heap_->count() > 0) {
    TopNRow *top = topn_heap_->top();
    if (OB_FAIL(sorted_rows.push_back(top))) {
      LOG_WARN("failed to push back to sorted_rows", K(ret));
      break;
    }
    topn_heap_->pop();
  }

  if (OB_FAIL(ret)) {
    // Clean up on error
    for (int64_t i = 0; i < sorted_rows.count(); ++i) {
      TopNRow *row = sorted_rows.at(i);
      if (row != nullptr) {
        if (row->row_data_ != nullptr) {
          row->row_data_->~StoredRow();
          alloc_->free(row->row_data_);
        }
        alloc_->free(row);
      }
    }
    return ret;
  }
  
  // Sort by score descending (highest scores first)
  std::sort(sorted_rows.begin(), sorted_rows.end(),
            [](const TopNRow *a, const TopNRow *b) {
              return a->score_ > b->score_;
            });
  
  // 两阶段检索日志：粗排阶段完成
  LOG_INFO("[Two-Phase Retrieval] Phase1 (coarse ranking) complete",
           "candidates_count", sorted_rows.count(),
           "top_score", (sorted_rows.count() > 0 ? sorted_rows.at(0)->score_ : 0.0),
           "last_score", (sorted_rows.count() > 0 ? sorted_rows.at(sorted_rows.count()-1)->score_ : 0.0));

  // Convert sorted rows to expressions in batch
  {
    ObEvalCtx::BatchInfoScopeGuard batch_info_guard(*eval_ctx_);
    batch_info_guard.set_batch_size(sorted_rows.count());
    
    for (int64_t i = 0; OB_SUCC(ret) && i < sorted_rows.count(); ++i) {
      batch_info_guard.set_batch_idx(i);
      TopNRow *row = sorted_rows.at(i);
      if (OB_ISNULL(row) || OB_ISNULL(row->row_data_)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected null row", K(ret), K(i));
      } else if (OB_FAIL(row->row_data_->to_expr<true>(*exprs_, *eval_ctx_))) {
        LOG_WARN("failed to convert row to expr", K(ret), K(i));
      }
    }
    
    if (OB_SUCC(ret)) {
      size = sorted_rows.count();
    }
    
    // Clean up all rows
    for (int64_t i = 0; i < sorted_rows.count(); ++i) {
      TopNRow *row = sorted_rows.at(i);
      if (row != nullptr) {
        if (row->row_data_ != nullptr) {
          row->row_data_->~StoredRow();
          alloc_->free(row->row_data_);
        }
        alloc_->free(row);
      }
    }
  }
  
  return ret;
}  


int ObDASIndexMergeIter::inner_init(ObDASIterParam &param)
{
  adlog::DEBUG("index merge iter use!!!");
  adlog::DEBUG("ObDASIndexMergeIter::inner_init start");
  int ret = OB_SUCCESS;
  if (ObDASIterType::DAS_ITER_INDEX_MERGE != param.type_) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("inner init das iter with bad param type", K(param));
  } else {
    ObDASIndexMergeIterParam &index_merge_param = static_cast<ObDASIndexMergeIterParam&>(param);
    merge_type_ = index_merge_param.merge_type_;
    rowkey_exprs_ = index_merge_param.rowkey_exprs_;
    get_next_row_ = (merge_type_ == INDEX_MERGE_UNION) ?
        &ObDASIndexMergeIter::union_get_next_row : &ObDASIndexMergeIter::intersect_get_next_row;
    get_next_rows_ = (merge_type_ == INDEX_MERGE_UNION) ?
        &ObDASIndexMergeIter::union_get_next_rows : &ObDASIndexMergeIter::intersect_get_next_rows;
    merge_ctdef_ = index_merge_param.ctdef_;
    merge_rtdef_ = index_merge_param.rtdef_;
    tx_desc_ = index_merge_param.tx_desc_;
    snapshot_ = index_merge_param.snapshot_;
    is_reverse_ = index_merge_param.is_reverse_;

    //
    // top_n_count_ = param.limit_param_;
    // adlog::DEBUG("top_n_count: ", top_n_count_);
    
    // CRITICAL FIX: Increase max_size_ when TopN is enabled to match expanded capacity
    if (merge_rtdef_->enable_topn_pushdown_) {
      // Original max_size_ is typically 256, increase it to support larger capacity
      max_size_ = max_size_ * 2 + 200;  // Ensure it's larger than capacity (310)
      LOG_INFO("Increased max_size_ for TopN pushdown", "original", 256, "new", max_size_);
    }
    
    lib::ContextParam context_param;
    context_param.set_mem_attr(MTL_ID(), "DASIndexMerge", ObCtxIds::DEFAULT_CTX_ID)
        .set_properties(lib::USE_TL_PAGE_OPTIONAL);
    if (OB_UNLIKELY(merge_type_ != INDEX_MERGE_UNION and merge_type_ != INDEX_MERGE_INTERSECT)) {
      ret = OB_INVALID_ARGUMENT;
      LOG_WARN("invalid merge type", K(merge_type_));
    } else
      if (OB_FAIL(CURRENT_CONTEXT->CREATE_CONTEXT(mem_ctx_, context_param))) {
      LOG_WARN("failed to create index merge memctx", K(ret));
    } else {
      common::ObArenaAllocator &alloc = mem_ctx_->get_arena_allocator();
      child_iters_.set_allocator(&alloc);
      child_stores_.set_allocator(&alloc);
      child_scan_rtdefs_.set_allocator(&alloc);
      child_scan_params_.set_allocator(&alloc);
      child_tablet_ids_.set_allocator(&alloc);
      child_match_against_exprs_.set_allocator(&alloc);
      int64_t child_cnt = index_merge_param.child_iters_->count();
      if (OB_FAIL(child_iters_.assign(*index_merge_param.child_iters_))) {
        LOG_WARN("failed to assign child iters", K(ret));
      } else if (OB_FAIL(child_scan_rtdefs_.assign(*index_merge_param.child_scan_rtdefs_))) {
        LOG_WARN("failed to assign child scan rtdefs", K(ret));
      } else if (OB_FAIL(child_stores_.prepare_allocate(child_cnt))) {
        LOG_WARN("failed to prepare allocate child stores", K(ret));
      } else if (OB_FAIL(child_scan_params_.prepare_allocate(child_cnt))) {
        LOG_WARN("failed to prepare allocate child scan params", K(ret));
      } else if (OB_FAIL(child_tablet_ids_.prepare_allocate(child_cnt))) {
        LOG_WARN("failed to prepare allocate child tablet ids", K(ret));
      } else if (OB_FAIL(child_match_against_exprs_.prepare_allocate(child_cnt))) {
        LOG_WARN("failed to prepare allocate child match against exprs", K(ret));
      } else {
        ObArray<ObExpr*> match_against_exprs;
        for (int64_t i = 0; OB_SUCC(ret) && i < child_cnt; i++) {
          ObDASIter *child = child_iters_.at(i);
          IndexMergeRowStore &row_store = child_stores_.at(i);
          ObDASScanRtDef *child_scan_rtdef = child_scan_rtdefs_.at(i);
          ObTableScanParam *&child_scan_param = child_scan_params_.at(i);
          ExprFixedArray &child_match_against_expr = child_match_against_exprs_.at(i);
          if (OB_ISNULL(child) || OB_ISNULL(child->get_output())) {
            ret = OB_ERR_UNEXPECTED;
            LOG_WARN("invalid child iter", K(i), K(child), K(ret));
          } else if (OB_FAIL(row_store.init(alloc, child->get_output(), eval_ctx_, max_size_))) {
            LOG_WARN("failed to init row store", K(ret));
          } else if (OB_FAIL(extract_match_against_exprs(*child->get_output(), match_against_exprs))) {
            LOG_WARN("failed to extract match against exprs", K(ret));
          } else {
            LOG_INFO("inner_init: extracted match_against_exprs from child", K(i), 
                     "child_output_count", child->get_output()->count(),
                     "match_count", match_against_exprs.count());
          }
          if (OB_SUCC(ret)) {
            child_match_against_expr.set_allocator(&alloc);
            if (OB_FAIL(child_match_against_expr.assign(match_against_exprs))) {
              LOG_WARN("failed to assign match against exprs", K(ret));
            } else if (child_scan_rtdef != nullptr) {
              // need to prepare scan param for normal scan node
              if (OB_ISNULL(child_scan_param = OB_NEWx(ObTableScanParam, &alloc))) {
                ret = OB_ALLOCATE_MEMORY_FAILED;
                LOG_WARN("failed to allocate child scan param", K(ret));
              }
            }
          }
        }
        if (OB_SUCC(ret)) {
          if (OB_FAIL(result_buffer_.init(max_size_, eval_ctx_, output_, mem_ctx_->get_malloc_allocator()))) {
            LOG_WARN("failed to init merge result buffer", K(ret));
          } else {
            if (merge_rtdef_->enable_topn_pushdown_) {
              result_buffer_.enable_topn_ = true;
              // 两阶段检索：粗排保留 100x 候选（如 limit=10 则保留 1000个）
              result_buffer_.topn_limit_ = merge_rtdef_->topn_limit_ * 100;
            } else {
              result_buffer_.enable_topn_ = false;
            }
          }
        }
      }
    }
  }
  // if (OB_SUCC(ret)) {
  //   if (OB_FAIL(init_two_phase(param))) {
  //     return ret;
  //   }
  // }
  adlog::DEBUG("ObDASIndexMergeIter::inner_init end");

  return ret;
}

int ObDASIndexMergeIter::init_scan_param(const share::ObLSID &ls_id,
                                         const common::ObTabletID &tablet_id,
                                         const sql::ObDASScanCtDef *ctdef,
                                         sql::ObDASScanRtDef *rtdef,
                                         ObTableScanParam &scan_param)
{
  int ret = OB_SUCCESS;
  if (OB_ISNULL(ctdef) || OB_ISNULL(rtdef)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid argument", K(ret), KPC(ctdef), KPC(rtdef), K(ls_id), K(tablet_id));
  } else {
    uint64_t tenant_id = MTL_ID();
    scan_param.tenant_id_ = tenant_id;
    scan_param.key_ranges_.set_attr(ObMemAttr(tenant_id, "ScanParamKR"));
    scan_param.ss_key_ranges_.set_attr(ObMemAttr(tenant_id, "ScanParamSSKR"));
    scan_param.tx_lock_timeout_ = rtdef->tx_lock_timeout_;
    scan_param.index_id_ = ctdef->ref_table_id_;
    scan_param.is_get_ = ctdef->is_get_;
    scan_param.is_for_foreign_check_ = false;
    scan_param.timeout_ = rtdef->timeout_ts_;
    scan_param.scan_flag_ = rtdef->scan_flag_;
    scan_param.reserved_cell_count_ = ctdef->access_column_ids_.count();
    scan_param.allocator_ = &rtdef->stmt_allocator_;
    scan_param.scan_allocator_ = &rtdef->scan_allocator_;
    scan_param.sql_mode_ = rtdef->sql_mode_;
    scan_param.frozen_version_ = rtdef->frozen_version_;
    scan_param.force_refresh_lc_ = rtdef->force_refresh_lc_;
    scan_param.output_exprs_ = &(ctdef->pd_expr_spec_.access_exprs_);
    scan_param.calc_exprs_ = &(ctdef->pd_expr_spec_.calc_exprs_);
    scan_param.aggregate_exprs_ = &(ctdef->pd_expr_spec_.pd_storage_aggregate_output_);
    scan_param.table_param_ = &(ctdef->table_param_);
    scan_param.op_ = rtdef->p_pd_expr_op_;
    scan_param.row2exprs_projector_ = rtdef->p_row2exprs_projector_;
    scan_param.schema_version_ = ctdef->schema_version_;
    scan_param.tenant_schema_version_ = rtdef->tenant_schema_version_;
    scan_param.limit_param_ = rtdef->limit_param_;
    scan_param.need_scn_ = rtdef->need_scn_;
    scan_param.pd_storage_flag_ = ctdef->pd_expr_spec_.pd_storage_flag_.pd_flag_;
    scan_param.fb_snapshot_ = rtdef->fb_snapshot_;
    scan_param.fb_read_tx_uncommitted_ = rtdef->fb_read_tx_uncommitted_;
    scan_param.ls_id_ = ls_id;
    scan_param.tablet_id_ = tablet_id;
    if (!ctdef->pd_expr_spec_.pushdown_filters_.empty()) {
      scan_param.op_filters_ = &ctdef->pd_expr_spec_.pushdown_filters_;
    }
    scan_param.pd_storage_filters_ = rtdef->p_pd_expr_op_->pd_storage_filters_;
    if (OB_NOT_NULL(tx_desc_)) {
      scan_param.tx_id_ = tx_desc_->get_tx_id();
    } else {
      scan_param.tx_id_.reset();
    }

    if (OB_NOT_NULL(snapshot_)) {
      if (OB_FAIL(scan_param.snapshot_.assign(*snapshot_))) {
        LOG_WARN("assign snapshot fail", K(ret));
      }
    } else {
      ret = OB_ERR_UNEXPECTED;
      LOG_ERROR("null snapshot", K(ret), KPC_(snapshot));
    }

    if (FAILEDx(scan_param.column_ids_.assign(ctdef->access_column_ids_))) {
      LOG_WARN("failed to init column ids", K(ret));
    } else if (OB_FAIL(prepare_scan_ranges(scan_param, rtdef))) {
      LOG_WARN("failed to prepare scan ranges", K(ret));
    }
  }
  return ret;
}

int ObDASIndexMergeIter::prepare_scan_ranges(ObTableScanParam &scan_param, const ObDASScanRtDef *rtdef)
{
  int ret = OB_SUCCESS;
  if (OB_ISNULL(rtdef)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected nullptr scan rtdef", K(ret));
  } else if (OB_FAIL(scan_param.key_ranges_.assign(rtdef->key_ranges_))) {
    LOG_WARN("failed to assign key ranges", K(ret));
  } else if (OB_FAIL(scan_param.ss_key_ranges_.assign(rtdef->ss_key_ranges_))) {
    LOG_WARN("failed to assign ss key ranges", K(ret));
  } else if (OB_FAIL(scan_param.mbr_filters_.assign(rtdef->mbr_filters_))) {
    LOG_WARN("failed to assign mbr filters", K(ret));
  }

  LOG_TRACE("index merge iter prepare scan ranges", K(scan_param), KPC(rtdef), K(ret));
  return ret;
}

void ObDASIndexMergeIter::reset_datum_ptr(const common::ObIArray<ObExpr*> *exprs, int64_t size)
{
  if (OB_NOT_NULL(exprs) && size > 0) {
    for (int64_t i = 0; i < exprs->count(); i++) {
      ObExpr *expr = exprs->at(i);
      if (OB_NOT_NULL(expr)) {
        expr->locate_datums_for_update(*eval_ctx_, size);
        ObEvalInfo &info = expr->get_eval_info(*eval_ctx_);
        info.point_to_frame_ = true;
      }
    }
  }
}

int ObDASIndexMergeIter::extract_match_against_exprs(const common::ObIArray<ObExpr*> &exprs, 
                                                     common::ObIArray<ObExpr*> &match_against_exprs)
{
  int ret = OB_SUCCESS;
  match_against_exprs.reuse();
  for (int64_t i = 0; OB_SUCC(ret) && i < exprs.count(); ++i) {
    ObExpr *expr = exprs.at(i);
    if (OB_ISNULL(expr)) {
      ret = OB_ERR_UNEXPECTED;  
      LOG_WARN("unexpected nullptr expr", K(ret));
    } else if (T_FUN_MATCH_AGAINST == expr->type_) {
      if (OB_FAIL(match_against_exprs.push_back(expr))) {
        LOG_WARN("failed to push back match against expr", K(ret));
      }
    }
  }
  return ret;
}

int ObDASIndexMergeIter::fill_default_values_for_union(const common::ObIArray<ObExpr*> &exprs)
{
  int ret = OB_SUCCESS;
  LOG_INFO("fill_default_values_for_union called", "exprs_count", exprs.count());
  for (int64_t i = 0; OB_SUCC(ret) && i < exprs.count(); ++i) {
    ObExpr *expr = exprs.at(i);
    if (OB_ISNULL(expr) || OB_UNLIKELY(T_FUN_MATCH_AGAINST != expr->type_)) {
      ret = OB_INVALID_ARGUMENT;
      LOG_WARN("invalid expr to fill default value", KP(expr), K(ret));
    } else {
      ObEvalCtx::BatchInfoScopeGuard batch_info_guard(*eval_ctx_);
      batch_info_guard.set_batch_size(1);
      batch_info_guard.set_batch_idx(0);
      ObDatum &datum = expr->locate_datum_for_write(*eval_ctx_);
      datum.set_double(0.0);
      LOG_INFO("fill_default_values_for_union: set score to 0.0", K(i), "len_after", datum.len_);
      expr->set_evaluated_flag(*eval_ctx_);
      expr->set_evaluated_projected(*eval_ctx_);
    }
  }
  return ret;
}

int ObDASIndexMergeIter::save_row_to_result_buffer() {
  int ret = OB_SUCCESS;
  
  LOG_INFO("===== save_row_to_result_buffer CALLED =====", 
           "enable_topn", result_buffer_.enable_topn_,
           "merge_type", merge_type_);
  
  // Note: MATCH_AGAINST expressions should already be properly filled by union_get_next_rows()
  // or intersect_get_next_rows() before calling this function.
  // We just save the row to the result buffer.
  
  if (!result_buffer_.enable_topn_) {
    LOG_INFO("save_row: using normal add_row");
    return result_buffer_.add_row();
  }
  LOG_INFO("save_row: using TopN add_row_with_topn");
  return result_buffer_.add_row_with_topn();
}

int ObDASIndexMergeIter::do_table_scan()
{
  int ret = OB_SUCCESS;
  for (int64_t i = 0; OB_SUCC(ret) && i < child_scan_rtdefs_.count(); ++i) {
    ObDASScanRtDef *scan_rtdef = child_scan_rtdefs_.at(i);
    ObDASIter *iter = child_iters_.at(i);
    if (OB_ISNULL(iter)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected nullptr iter", K(ret));
    } else if (scan_rtdef != nullptr) {
      const ObDASScanCtDef *scan_ctdef = static_cast<const ObDASScanCtDef*>(scan_rtdef->ctdef_);
      ObTableScanParam *scan_param = child_scan_params_.at(i);
      if (OB_ISNULL(scan_ctdef) || OB_ISNULL(scan_param)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected nullptr child scan info", K(scan_ctdef), K(scan_param), K(ret));
      } else if (OB_FAIL(init_scan_param(ls_id_, child_tablet_ids_.at(i), scan_ctdef, scan_rtdef, *scan_param))) {
        LOG_WARN("failed to init child scan param", K(ret));
      } else if (OB_FAIL(iter->do_table_scan())) {
        LOG_WARN("child iter failed to do table scan", K(ret));
      }
    } else if (OB_FAIL(iter->do_table_scan())) {
      LOG_WARN("child iter failed to do table scan", K(ret));
    }
  }
  return ret;
}

int ObDASIndexMergeIter::rescan()
{
  int ret = OB_SUCCESS;
  for (int64_t i = 0; OB_SUCC(ret) && i < child_scan_rtdefs_.count(); ++i) {
    ObDASScanRtDef *scan_rtdef = child_scan_rtdefs_.at(i);
    ObDASIter *iter = child_iters_.at(i);
    if (OB_ISNULL(iter)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected nullptr iter", K(ret));
    } else if (scan_rtdef != nullptr) {
      const ObDASScanCtDef *scan_ctdef = static_cast<const ObDASScanCtDef*>(scan_rtdef->ctdef_);
      ObTableScanParam *scan_param = child_scan_params_.at(i);
      if (OB_ISNULL(scan_ctdef) || OB_ISNULL(scan_param)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected nullptr child scan info", K(scan_ctdef), K(scan_param), K(ret));
      } else {
        scan_param->tablet_id_ = child_tablet_ids_.at(scan_ctdef->index_merge_idx_);
        scan_param->ls_id_ = ls_id_;
        if (OB_FAIL(prepare_scan_ranges(*scan_param, scan_rtdef))) {
          LOG_WARN("failed to prepare scan ranges", K(ret));
        } else if (OB_FAIL(iter->rescan())) {
          LOG_WARN("child iter failed to rescan", K(ret));
        }
      }
    } else if (OB_FAIL(iter->rescan())) {
      LOG_WARN("child iter failed to rescan", K(ret));
    }
  }
  return ret;
}

void ObDASIndexMergeIter::clear_evaluated_flag()
{
  for (int64_t i = 0; i < child_iters_.count(); ++i) {
    if (OB_NOT_NULL(child_iters_.at(i))) {
      child_iters_.at(i)->clear_evaluated_flag();
    }
  }
}

int ObDASIndexMergeIter::set_ls_tablet_ids(const ObLSID &ls_id, const ObDASRelatedTabletID &related_tablet_ids)
{
  int ret = OB_SUCCESS;
  ls_id_ = ls_id;
  const ObIArray<ObTabletID> &index_merge_tablet_ids = related_tablet_ids.index_merge_tablet_ids_;
  for (int64_t i = 0; OB_SUCC(ret) && i < child_scan_rtdefs_.count(); ++i) {
    ObDASScanRtDef *scan_rtdef = child_scan_rtdefs_.at(i);
    if (scan_rtdef != nullptr) {
      const ObDASScanCtDef *scan_ctdef = static_cast<const ObDASScanCtDef*>(scan_rtdef->ctdef_);
      if (OB_ISNULL(scan_ctdef)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected nullptr scan ctdef", K(ret));
      } else {
        child_tablet_ids_.at(i) = index_merge_tablet_ids.at(scan_ctdef->index_merge_idx_);
      }
    }
  }
  return ret;
}

int ObDASIndexMergeIter::inner_reuse()
{
  int ret = OB_SUCCESS;
  for (int64_t i = 0; OB_SUCC(ret) && i < child_scan_rtdefs_.count(); ++i) {
    ObDASScanRtDef *scan_rtdef = child_scan_rtdefs_.at(i);
    ObDASIter *iter = child_iters_.at(i);
    if (OB_ISNULL(iter)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected nullptr iter", K(ret));
    } else if (scan_rtdef != nullptr) {
      const ObDASScanCtDef *scan_ctdef = static_cast<const ObDASScanCtDef*>(scan_rtdef->ctdef_);
      ObTableScanParam *scan_param = child_scan_params_.at(i);
      if (OB_ISNULL(scan_ctdef) || OB_ISNULL(scan_param)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected nullptr child scan info", K(scan_ctdef), K(scan_param), K(ret));
      } else {
        const ObTabletID &new_tablet_id = child_tablet_ids_.at(scan_ctdef->index_merge_idx_);
        const ObTabletID &old_tablet_id = scan_param->tablet_id_;
        scan_param->need_switch_param_ = scan_param->need_switch_param_ ||
            (old_tablet_id.is_valid() && old_tablet_id != new_tablet_id);
        scan_param->key_ranges_.reuse();
        scan_param->ss_key_ranges_.reuse();
        scan_param->mbr_filters_.reuse();
        if (OB_FAIL(iter->reuse())) {
          LOG_WARN("child iter failed to reuse", K(ret));
        }
      }
    } else if (OB_FAIL(iter->reuse())) {
      LOG_WARN("child iter failed to reuse", K(ret));
    }
  }
  for (int64_t i = 0; i < child_stores_.count(); ++i) {
    child_stores_.at(i).reuse();
  }
  result_buffer_.reuse();
  return ret;
}

int ObDASIndexMergeIter::inner_release()
{

  int ret = OB_SUCCESS;
  child_iters_.reset();
  child_scan_rtdefs_.reset();
  child_tablet_ids_.reset();
  for (int64_t i = 0; i < child_stores_.count(); ++i) {
    child_stores_.at(i).reset();
  }
  child_stores_.reset();
  for (int64_t i = 0; i < child_scan_params_.count(); ++i) {
    ObTableScanParam *scan_param = child_scan_params_.at(i);
    if (scan_param != nullptr) {
      scan_param->destroy_schema_guard();
      scan_param->snapshot_.reset();
      scan_param->destroy();
    }
  }
  for (int64_t i = 0; i < child_match_against_exprs_.count(); ++i) {
    child_match_against_exprs_.at(i).reset();
  }
  child_match_against_exprs_.reset();
  result_buffer_.reset();
  if (OB_NOT_NULL(mem_ctx_)) {
    mem_ctx_->reset_remain_one_page();
    DESTROY_CONTEXT(mem_ctx_);
    mem_ctx_ = nullptr;
  }
  return ret;
}

int ObDASIndexMergeIter::inner_get_next_row()
{
  int ret = OB_SUCCESS;
  LOG_INFO("===== inner_get_next_row CALLED (SINGLE ROW MODE) =====",
           "merge_type", merge_type_);
  clear_evaluated_flag();
  if (OB_FAIL((this->*get_next_row_)())) {
    if (ret != OB_ITER_END) {
      LOG_WARN("index merge iter failed to get next row", K(ret));
    }
  }
  return ret;
}

int ObDASIndexMergeIter::inner_get_next_rows(int64_t &count, int64_t capacity)
{
  int ret = OB_SUCCESS;
  LOG_INFO("===== inner_get_next_rows CALLED =====", K(count), K(capacity), 
           "merge_type", (merge_type_ == INDEX_MERGE_UNION ? "UNION" : "INTERSECT"));
  clear_evaluated_flag();
  if (OB_FAIL((this->*get_next_rows_)(count, capacity))) {
    if (ret != OB_ITER_END) {
      LOG_WARN("index merge iter failed to get next rows", K(ret));
    }
  }
  LOG_TRACE("[DAS ITER] index merge iter get next rows", K(count), K(capacity), K(ret));
  const ObBitVector *skip = nullptr;
  PRINT_VECTORIZED_ROWS(SQL, DEBUG, *eval_ctx_, *output_, count, skip);

  return ret;
}

int ObDASIndexMergeIter::compare(int64_t cur_idx, int64_t &output_idx, int &cmp_ret)
{
  int ret = OB_SUCCESS;
  cmp_ret = 0;
  if (cur_idx == OB_INVALID_INDEX || !child_stores_.at(cur_idx).have_data()) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid idx", K(cur_idx), K(ret));
  } else if (output_idx == OB_INVALID_INDEX) {
    output_idx = cur_idx;
  } else {
    const ObDatum *cur_datums = child_stores_.at(cur_idx).cur_datums();
    const ObDatum *output_datums = child_stores_.at(output_idx).cur_datums();
    if (OB_ISNULL(cur_datums) || OB_ISNULL(output_datums)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected nullptr", K(cur_datums), K(output_datums));
    }
    ObObj cur_obj;
    ObObj output_obj;
    for (int64_t i = 0; (cmp_ret == 0) && OB_SUCC(ret) && i < rowkey_exprs_->count(); i++) {
      const ObExpr *expr = rowkey_exprs_->at(i);
      if (OB_ISNULL(expr)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected nullptr", K(ret));
      } else if (OB_FAIL(cur_datums[i].to_obj(cur_obj, expr->obj_meta_, expr->obj_datum_map_))) {
        LOG_WARN("failed to convert left datum to obj", K(i), KPC(expr), K(ret));
      } else if (OB_FAIL(output_datums[i].to_obj(output_obj, expr->obj_meta_, expr->obj_datum_map_))) {
        LOG_WARN("failed to convert right datum to obj", K(i), KPC(expr), K(ret));
      } else if (OB_FAIL(cur_obj.check_collation_free_and_compare(output_obj, cmp_ret))) {
          LOG_WARN("failed to compare cur obj with output obj", K(ret));
      } else if (cmp_ret != 0) {
        if (cmp_ret < 0) {
          output_idx = OB_UNLIKELY(is_reverse_) ? output_idx : cur_idx;
        } else {
          output_idx = OB_UNLIKELY(is_reverse_) ? cur_idx : output_idx;
        }
      }
    }
  }
  return ret;
}

int ObDASIndexMergeIter::intersect_get_next_row() {
  adlog::DEBUG("NO WAY !!!");
  int ret = OB_SUCCESS;
  bool got_row = false;
  while (OB_SUCC(ret) && !got_row) {
      /* try to fill each child store */
    int64_t output_idx = OB_INVALID_INDEX;
    int cmp_ret = 0;
    for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
      IndexMergeRowStore &child_store = child_stores_.at(i);
      if (!child_store.have_data()) {
        if (!child_store.iter_end_) {
          ObDASIter *child_iter = child_iters_.at(i);
          if (OB_ISNULL(child_iter)) {
            ret = OB_ERR_UNEXPECTED;
            LOG_WARN("unexpected nullptr", K(i));
          } else if (OB_FAIL(child_iter->get_next_row())) {
            if (OB_ITER_END == ret) {
              child_store.iter_end_ = true;
              ret = OB_SUCCESS;
            } else {
              LOG_WARN("failed to get next row from child iter", K(ret));
            }
          } else if (OB_FAIL(child_store.save(false, 1))) {
            LOG_WARN("failed to save child row", K(ret));
          } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
            LOG_WARN("index merge failed to compare row", K(i), K(output_idx), K(ret));
          }
        }
      } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
        LOG_WARN("index merge failed to compare row", K(i), K(output_idx), K(ret));
      }
    }

    if (OB_FAIL(ret)) {
    } else if (output_idx == OB_INVALID_INDEX) {
      /* no available row, stop the entire process */
      ret = OB_ITER_END;
    } else {
      /* find all available rows */
      bool all_matched = true;
      for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
        if (output_idx == i) {
          /* skip */
        } else if (child_stores_.at(i).iter_end_ || !child_stores_.at(i).have_data()) {
          ret = OB_ITER_END;
        } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
            LOG_WARN("index merge failed to compare row", K(i), K(output_idx), K(ret));
        } else if (cmp_ret == 0) {
          // FIXME: only one store will project data in intersect get next row(s).
          child_stores_.at(i).cur_idx_++;
        } else {
          all_matched = false;
        }
      }
      if (OB_SUCC(ret)) {
        if (all_matched) {
          /* found available row in each child store, output */
          if (OB_FAIL(child_stores_.at(output_idx).to_expr())) {
            LOG_WARN("index merge failed to convert row to expr", K(ret));
          } else {
            got_row = true;
          }
        } else {
          child_stores_.at(output_idx).cur_idx_++;
        }
      }
    }
  }

  return ret;
}


int ObDASIndexMergeIter::intersect_get_next_rows(int64_t &count, int64_t capacity)  
{
  int ret = OB_SUCCESS;
  adlog::DEBUG("ObDASIndexMergeIter::intersect_get_next_rows start");

  
  count = 0;  
  result_buffer_.reuse();
  adlog::DEBUG("result_buffer_.reuse();");

  int64_t matched_capacity = capacity;
  int64_t scan_limit = capacity * 10;
  int64_t scanned_rows = 0;
  
  if (result_buffer_.enable_topn_) {
    // 两阶段检索参数：
    // matched_capacity: 粗排候选数（如 1000）
    // scan_limit: 扫描行数上限（如 10000）
    matched_capacity = result_buffer_.topn_limit_;  // 直接使用 topn_limit
    scan_limit = matched_capacity * 10;  // 扫描 10x 候选数的行
    LOG_INFO("[Two-Phase Retrieval] Phase1 parameters",
             K(matched_capacity), K(scan_limit));
  }
  
  while (OB_SUCC(ret) && count < matched_capacity && scanned_rows < scan_limit) {  
    /* try to fill each child store */  
    int64_t output_idx = OB_INVALID_INDEX;  
    int cmp_ret = 0;  
    int64_t child_rows_cnt = 0;  
      
    for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
      IndexMergeRowStore &child_store = child_stores_.at(i);
      child_rows_cnt = 0;
      if (!child_store.have_data()) {
        if (!child_store.iter_end_) {
          ObDASIter *child_iter = child_iters_.at(i);
          if (OB_ISNULL(child_iter)) {
            ret = OB_ERR_UNEXPECTED;
            adlog::DEBUG("unexpected nullptr", K(i));
          } else {

            ret = child_iter->get_next_rows(child_rows_cnt, capacity);

            if (OB_FAIL(ret)) {
              if (OB_ITER_END != ret) {
                adlog::DEBUG("WARNING child_iter->get_next_rows(child_rows_cnt, capacity);");
              }
            }
            adlog::DEBUG("child_iter: ",K(i),"  ", K(child_rows_cnt));

            if (OB_ITER_END == ret && child_rows_cnt > 0) {
              ret = OB_SUCCESS;
            }
            if (OB_SUCC(ret)) {
              if (OB_FAIL(child_store.save(true, child_rows_cnt))) {
                adlog::DEBUG("WARNING failed to save child rows", K(child_rows_cnt), K(ret));
              } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
                adlog::DEBUG("WARNING index merge failed to compare row", K(i), K(output_idx), K(ret));
              } else if (child_iter->get_type() == DAS_ITER_SORT) {
                adlog::DEBUG("i: ",i,"child_iter->get_type() == DAS_ITER_SORT" );
                reset_datum_ptr(child_iter->get_output(), child_rows_cnt);
              }
            } else if (OB_ITER_END == ret) {
              child_store.iter_end_ = true;
              ret = OB_SUCCESS;
            } else {
              adlog::DEBUG("WARNING failed to get next rows from child iter", K(ret));
            }
          }
        }
      } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
        adlog::DEBUG("WARNING index merge failed to compare row", K(i), K(output_idx), K(ret));
      }
    }

    if (OB_FAIL(ret)) {
      adlog::DEBUG("WARNING OB_FAIL(RET)");
    } else if (output_idx == OB_INVALID_INDEX) {
      // No more data from children
      // If we have collected some matches, break and return them
      // Otherwise, return OB_ITER_END
      if (count > 0) {
        LOG_INFO("INTERSECT: No more data, but have matches to return", 
                 "count", count, "scanned_rows", scanned_rows);
        break;  // Exit loop and return collected data
      } else {
        ret = OB_ITER_END;
        adlog::DEBUG("ret = OB_ITER_END");
      }
    } else {    
      bool all_matched = true;
      for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
        if (output_idx == i) {  
          continue;
        }
        if ((!child_stores_.at(i).have_data())) {
            all_matched = false;  // 这个child没有数据，不匹配
            ret = OB_ITER_END;
        } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {  
          adlog::DEBUG("index merge failed to compare row", K(i), K(output_idx), K(ret));  
        } else if (cmp_ret == 0) {  
          
        } else { 
          all_matched = false;  // rowkey不匹配  
        }
      }

      if (OB_FAIL(ret)) {
        adlog::DEBUG("OB_FAIL(ret)");
        continue;
      }
      
      // Count this as a scanned row (we checked all children for a match)
      scanned_rows++;
      
      if (!all_matched) {
        child_stores_.at(output_idx).cur_idx_++;  
        continue;  // Not a match, continue scanning
      }
      // CRITICAL FIX: Call to_expr() for ALL children to fill their expressions
      LOG_INFO("===== INTERSECT FIX: calling to_expr() for all children =====", 
               "output_idx", output_idx, "child_count", child_stores_.count());
      
      for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
        if (OB_FAIL(child_stores_.at(i).to_expr())) {
          LOG_WARN("failed to convert store row to expr", K(ret), K(i));
        }
      }
      
      // CRITICAL FIX PART 2: Copy MATCH_AGAINST values from child exprs to output_
      // Problem: output_ may have different ObExpr* objects than child exprs, even for the same expression
      // Solution: For each MATCH_AGAINST in output_, find the corresponding expr in child that has it,
      // and copy the datum value. IMPORTANT: Keep batch context (batch_idx=0) active until save_row_to_result_buffer!
      if (OB_SUCC(ret)) {
        ObEvalCtx::BatchInfoScopeGuard batch_guard(*eval_ctx_);
        batch_guard.set_batch_size(1);
        batch_guard.set_batch_idx(0);
        
        for (int64_t out_idx = 0; OB_SUCC(ret) && out_idx < output_->count(); ++out_idx) {
          ObExpr *output_expr = output_->at(out_idx);
          if (OB_NOT_NULL(output_expr) && T_FUN_MATCH_AGAINST == output_expr->type_) {
            // Find this MATCH_AGAINST in child exprs
            bool found = false;
            for (int64_t child_idx = 0; !found && child_idx < child_stores_.count(); ++child_idx) {
              const ObIArray<ObExpr*> *child_exprs = child_stores_.at(child_idx).exprs_;
              if (child_exprs != nullptr) {
                for (int64_t child_expr_idx = 0; !found && child_expr_idx < child_exprs->count(); ++child_expr_idx) {
                  ObExpr *child_expr = child_exprs->at(child_expr_idx);
                  if (child_expr != nullptr && child_expr->type_ == T_FUN_MATCH_AGAINST) {
                    // Found MATCH_AGAINST in child - copy its value to output_
                    const ObDatum &child_datum = child_expr->locate_expr_datum(*eval_ctx_);
                    ObDatum &output_datum = output_expr->locate_datum_for_write(*eval_ctx_);
                    output_datum = child_datum;  // Copy datum
                    output_expr->set_evaluated_flag(*eval_ctx_);
                    found = true;
                    LOG_INFO("INTERSECT: Copied MATCH_AGAINST from child to output", 
                             "child_idx", child_idx, "len", child_datum.len_,
                             "score", (child_datum.len_ == sizeof(double) ? child_datum.get_double() : 0.0));
                  }
                }
              }
            }
            if (!found) {
              LOG_WARN("INTERSECT: MATCH_AGAINST in output_ not found in any child!", "out_idx", out_idx);
            }
          }
        }
        
        // IMPORTANT: Call save_row_to_result_buffer INSIDE the BatchInfoScopeGuard
        // to ensure batch_idx=0 is still active when add_row_with_topn accesses the datum
        if (OB_FAIL(save_row_to_result_buffer())) {
            LOG_WARN("failed to save row to result buffer", K(ret));
        } else {
          count += 1;
          if (result_buffer_.enable_topn_ && result_buffer_.topn_heap_ != nullptr) { 
            count = result_buffer_.topn_heap_->count();
          }
        }
      }

    }  
  }

  if(OB_FAIL(ret)){
    if (ret == OB_ITER_END) {
      adlog::DEBUG("intersect_get_next_rows : OB_FAIL(ret) ret == OB_ITER_END");
    } else {
      adlog::DEBUG("intersect_get_next_rows : OB_FAIL(ret)");
    }
  }
  adlog::DEBUG("count: ",count," capity: ", capacity);
  adlog::DEBUG("intersect_get_next_rows :",K(count));
  
  if (OB_ITER_END == ret && count > 0) {  
    ret = OB_SUCCESS;  
  }  
  if (OB_SUCC(ret) && count > 0) {

    if (result_buffer_.enable_topn_) {
      adlog::DEBUG("result_buffer_.to_expr_from_topn(count): ", count);
      int64_t tmp;
      result_buffer_.to_expr_from_topn(tmp);
    } else if (OB_FAIL(result_buffer_.to_expr(count))) {
      adlog::DEBUG("WARNING! failed to convert result buffer to exprs", K(ret));  
      LOG_WARN("failed to convert result buffer to exprs", K(ret));  
    }  
  }  

  adlog::DEBUG("ObDASIndexMergeIter::intersect_get_next_rows end");

  return ret;  
}


int ObDASIndexMergeIter::union_get_next_row() {
  adlog::DEBUG("ObDASIndexMergeIter::union_get_next_row");
  
  int ret = OB_SUCCESS;
  /* try to fill each child store */
  int64_t output_idx = OB_INVALID_INDEX;
  int cmp_ret = 0;
  for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
    IndexMergeRowStore &child_store = child_stores_.at(i);
    if (!child_store.have_data()) {
      if (!child_store.iter_end_) {
        ObDASIter *child_iter = child_iters_.at(i);
        if (OB_ISNULL(child_iter)) {
          ret = OB_ERR_UNEXPECTED;
          LOG_WARN("unexpected nullptr", K(i));
        } else if (OB_FAIL(child_iter->get_next_row())) {
          if (OB_ITER_END == ret) {
            child_store.iter_end_ = true;
            ret = OB_SUCCESS;
          } else {
            LOG_WARN("failed to get next row from child iter", K(ret));
          }
        } else if (OB_FAIL(child_store.save(false, 1))) {
          LOG_WARN("failed to save child row", K(ret));
        } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
          LOG_WARN("index merge failed to compare row", K(i), K(output_idx), K(ret));
        }
      }
    } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
      LOG_WARN("index merge failed to compare row", K(i), K(output_idx), K(ret));
    }
  }

  // TODO: use min-heap optimization
  if (OB_FAIL(ret)) {
  } else if (output_idx == OB_INVALID_INDEX) {
    /* no available row, stop the entire process */
    ret = OB_ITER_END;
  } else {
    /* find all available rows */
    bool need_fill_default_value = false;
    for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
      need_fill_default_value = false;
      if (output_idx == i) {
        /* skip */
      } else if (child_stores_.at(i).have_data()) {
        if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
          LOG_WARN("index merge failed to compare row", K(i), K(output_idx), K(ret));
        } else if (cmp_ret == 0) {
          if (OB_FAIL(child_stores_.at(i).to_expr())) {
            LOG_WARN("failed to convert store row to expr", K(ret));
          }
        } else {
          // this child does not have a corresponding rowkey
          need_fill_default_value = true;
        }
      } else {
        // this child does not have available rows
        need_fill_default_value = true;
      }
      if (OB_SUCC(ret) && need_fill_default_value) {
        if (OB_FAIL(fill_default_values_for_union(child_match_against_exprs_.at(i)))) {
          LOG_WARN("failed to fill default values for union", K(ret), K(i));
        }
      }
    }
    if (OB_SUCC(ret)) {
      if (OB_FAIL(child_stores_.at(output_idx).to_expr())) {
        LOG_WARN("failed to convert store row to expr", K(ret));
      } else {
        // Evaluate MATCH AGAINST expressions from output_ (not from child)
        // because MATCH AGAINST may not be in child->get_output()
        LOG_INFO("union_get_next_row: checking output_ for MATCH AGAINST", "output_count", output_->count());
        int match_count = 0;
        for (int64_t i = 0; OB_SUCC(ret) && i < output_->count(); ++i) {
          ObExpr *expr = output_->at(i);
          LOG_INFO("union_get_next_row: checking expr", K(i), "expr_type", (expr ? expr->type_ : -1));
          if (OB_NOT_NULL(expr) && T_FUN_MATCH_AGAINST == expr->type_) {
            match_count++;
            LOG_INFO("union_get_next_row: found MATCH AGAINST expr, evaluating", K(i));
            ObDatum *datum = nullptr;
            if (OB_FAIL(expr->eval(*eval_ctx_, datum))) {
              LOG_WARN("failed to evaluate match against expr in union_get_next_row", K(ret), K(i));
            } else {
              LOG_INFO("union_get_next_row: evaluated match_against expr from output_", K(i), 
                       "is_null", datum->is_null(), "len", datum->len_, 
                       "score", (datum->len_ == sizeof(double) ? datum->get_double() : 0.0));
            }
          }
        }
        LOG_INFO("union_get_next_row: finished checking output_", "match_count", match_count);
      }
    }
  }
  return ret;
}

int ObDASIndexMergeIter::union_get_next_rows(int64_t &count, int64_t capacity) {
  LOG_INFO("===== union_get_next_rows CALLED =====", K(count), K(capacity));
  adlog::DEBUG("ObDASIndexMergeIter::union_get_next_rows");
  int ret = OB_SUCCESS;

  {
    {
      // 在 intersect_get_next_rows 开始处添加  
      adlog::DEBUG("UNION rowkey_exprs_ count", K(rowkey_exprs_->count()));  
      for (int64_t i = 0; i < rowkey_exprs_->count(); i++) {
        // adlog::DEBUG("rowkey_expr", i, std::to_string(*rowkey_exprs_->at(i)));
        {
          char buf[1024];  
          int64_t pos = 0;  
          pos = rowkey_exprs_->at(i)->to_string(buf, sizeof(buf));  
          adlog::DEBUG("UNION rowkey_expr", i, std::string(buf, pos));
        }
      }
      // // 检查 rowkey_exprs_ 是否包含主键表达式  
      // for (int64_t i = 0; i < rowkey_exprs_->count(); i++) {  
      //   ObExpr *expr = rowkey_exprs_->at(i);  
      //   if (expr->type_ == T_REF_COLUMN) {  
      //     ObColumnRefRawExpr *col_expr = static_cast<ObColumnRefRawExpr*>(expr);  
      //     adlog::DEBUG("UNION Column name ",i," ", K(col_expr->get_column_name()));  
      //   }  
      // }  
    }
  }

  count = 0;
  result_buffer_.reuse();
  
  if (result_buffer_.enable_topn_) {
    // 两阶段检索：UNION 模式也使用粗排候选数
    capacity = result_buffer_.topn_limit_;
  }
  
  while (OB_SUCC(ret) && count < capacity) {
    /* try to fill each child store */
    int64_t output_idx = OB_INVALID_INDEX;
    int cmp_ret = 0;
    int64_t child_rows_cnt = 0;
    for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
      IndexMergeRowStore &child_store = child_stores_.at(i);
      child_rows_cnt = 0;
      if (!child_store.have_data()) {
        if (!child_store.iter_end_) {
          ObDASIter *child_iter = child_iters_.at(i);
          if (OB_ISNULL(child_iter)) {
            ret = OB_ERR_UNEXPECTED;
            LOG_WARN("unexpected nullptr", K(i));
          } else {
            ret = child_iter->get_next_rows(child_rows_cnt, capacity);
            if (OB_ITER_END == ret && child_rows_cnt > 0) {
              ret = OB_SUCCESS;
            }
            adlog::DEBUG("child_iter: ",K(i),"  ", K(child_rows_cnt));
            if (OB_SUCC(ret)) {
              if (OB_FAIL(child_store.save(true, child_rows_cnt))) {
                LOG_WARN("failed to save child rows", K(child_rows_cnt), K(ret));
              } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
                LOG_WARN("index merge failed to compare row", K(i), K(output_idx), K(ret));
              } else if (child_iter->get_type() == DAS_ITER_SORT) {
                reset_datum_ptr(child_iter->get_output(), child_rows_cnt);
              }
            } else if (OB_ITER_END == ret) {
              child_store.iter_end_ = true;
              ret = OB_SUCCESS;
            } else {
              LOG_WARN("failed to get next rows from child iter", K(ret));
            }
          }
        }
      } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
        LOG_WARN("index merge failed to compare row", K(i), K(output_idx), K(ret));
      }
    }

    if (OB_FAIL(ret)) {
    } else if (output_idx == OB_INVALID_INDEX) {
      /* no available row, stop the entire process */
      ret = OB_ITER_END;
    } else {
      /* find all available rows */
      bool need_fill_default_value = false;
      for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
        need_fill_default_value = false;
        if (output_idx == i) {
          /* skip */
        } else if (child_stores_.at(i).have_data()) {
          if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
            LOG_WARN("index merge failed to compare row", K(i), K(output_idx), K(ret));
          } else if (cmp_ret == 0) {
            if (OB_FAIL(child_stores_.at(i).to_expr())) {
              LOG_WARN("failed to convert store row to expr", K(ret));  
            }
          } else {
            // this child does not have a corresponding rowkey
            need_fill_default_value = true;
          }
        } else {
          // this child does not have available rows
          need_fill_default_value = true;
        }
        if (OB_SUCC(ret) && need_fill_default_value) {
          if (OB_FAIL(fill_default_values_for_union(child_match_against_exprs_.at(i)))) {
            LOG_WARN("failed to fill default values for union", K(ret), K(i));
          }
        }
      }
      if (OB_SUCC(ret)) {
        // CRITICAL FIX: Call to_expr() for ALL children to fill their expressions
        LOG_INFO("===== UNION FIX: calling to_expr() for all children =====", 
                 "output_idx", output_idx, "child_count", child_stores_.count());
        
        for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
          if (child_stores_.at(i).have_data()) {
            if (OB_FAIL(child_stores_.at(i).to_expr())) {
              LOG_WARN("failed to convert store row to expr", K(ret), K(i));
            }
          }
        }
        
        // CRITICAL FIX PART 2: Copy MATCH_AGAINST values from child exprs to output_
        // Same fix as INTERSECT mode - must copy datum values to output_ expressions
        if (OB_SUCC(ret)) {
          ObEvalCtx::BatchInfoScopeGuard batch_guard(*eval_ctx_);
          batch_guard.set_batch_size(1);
          batch_guard.set_batch_idx(0);
          
          for (int64_t out_idx = 0; OB_SUCC(ret) && out_idx < output_->count(); ++out_idx) {
            ObExpr *output_expr = output_->at(out_idx);
            if (OB_NOT_NULL(output_expr) && T_FUN_MATCH_AGAINST == output_expr->type_) {
              // Find this MATCH_AGAINST in child exprs
              bool found = false;
              for (int64_t child_idx = 0; !found && child_idx < child_stores_.count(); ++child_idx) {
                const ObIArray<ObExpr*> *child_exprs = child_stores_.at(child_idx).exprs_;
                if (child_exprs != nullptr) {
                  for (int64_t child_expr_idx = 0; !found && child_expr_idx < child_exprs->count(); ++child_expr_idx) {
                    ObExpr *child_expr = child_exprs->at(child_expr_idx);
                    if (child_expr != nullptr && child_expr->type_ == T_FUN_MATCH_AGAINST) {
                      // Found MATCH_AGAINST in child - copy its value to output_
                      const ObDatum &child_datum = child_expr->locate_expr_datum(*eval_ctx_);
                      ObDatum &output_datum = output_expr->locate_datum_for_write(*eval_ctx_);
                      output_datum = child_datum;  // Copy datum
                      output_expr->set_evaluated_flag(*eval_ctx_);
                      found = true;
                      LOG_INFO("UNION: Copied MATCH_AGAINST from child to output", 
                               "child_idx", child_idx, "len", child_datum.len_,
                               "score", (child_datum.len_ == sizeof(double) ? child_datum.get_double() : 0.0));
                    }
                  }
                }
              }
              if (!found) {
                LOG_WARN("UNION: MATCH_AGAINST in output_ not found in any child!", "out_idx", out_idx);
              }
            }
          }
          
          // IMPORTANT: Call save_row_to_result_buffer INSIDE the BatchInfoScopeGuard
          // to ensure batch_idx=0 is still active when add_row_with_topn accesses the datum
          if (OB_FAIL(save_row_to_result_buffer())) {
            LOG_WARN("failed to save row to result buffer", K(ret));
          } else {
            count += 1;
            if (result_buffer_.enable_topn_ && result_buffer_.topn_heap_ != nullptr) { 
              count = result_buffer_.topn_heap_->count();
            }
          }
        }
      }
    }
  }

  if (OB_ITER_END == ret && count > 0) {
    ret = OB_SUCCESS;
  }
  if (OB_SUCC(ret) && count > 0) {
    if (result_buffer_.enable_topn_) {
      LOG_INFO("UNION: using TopN to_expr_from_topn", K(count));
      int64_t tmp;
      result_buffer_.to_expr_from_topn(tmp);
    } else if (OB_FAIL(result_buffer_.to_expr(count))) {
      LOG_WARN("failed to convert result buffer to exprs", K(ret));
    }
  }
  LOG_INFO("===== union_get_next_rows RETURNING =====", K(ret), K(count));
  return ret;
}
// ADVISE

int ObDASIndexMergeIter::execute_two_phase() {
  int ret = first_merge_phase();
  if (ret != OB_SUCCESS) {
    return ret;
  }
  return second_merge_phase();
}
int ObDASIndexMergeIter::first_merge_phase() {
  int ret = OB_SUCCESS;
  return ret;
}
int ObDASIndexMergeIter::second_merge_phase() {
  int ret = OB_SUCCESS;
  return ret;
}

// int64_t ObDASIndexMergeIter::get_top_n_count() {
  
// }

/*
      int check_top_k();
      int check_support_two_phase();
*/
int ObDASIndexMergeIter::init_two_phase(ObDASIterParam &param) {
  int ret = OB_SUCCESS;
  if (merge_type_ != INDEX_MERGE_INTERSECT) {
    adlog::DEBUG("check_init_two_phase: no and");
    return OB_SUCCESS;
  }

  if (!check_mixed_query_with_fts()) {
    adlog::DEBUG("check_mixed_query_with_fts false");
    return OB_SUCCESS;
  }

  if (!has_fts_iter_) {
    adlog::DEBUG("has_fts_iter_: false");
    return OB_SUCCESS;
  }

  if (!check_top_k()) {
    adlog::DEBUG("check top k false");
    return OB_SUCCESS;
  }

  if (!check_support_two_phase()) {
    adlog::DEBUG("check_support_two_phase false");
    return OB_SUCCESS;
  }

  enable_two_phase_ = true;

  adlog::DEBUG("Two-phase optimization enabled ");

  return ret;
}

bool ObDASIndexMergeIter::check_top_k() {

  // if (merge_rtdef_ && merge_rtdef_->limit_param_.is_valid()) {
  //   top_n_count_ = merge_rtdef_->limit_param_.limit_count_;
  //   if (top_n_count_ > 0) {
  //       adlog::DEBUG("Top-K query detected from limit_param: ", top_n_count_);
  //       return true;
  //   }
  // }
  if (top_n_count_ == 0 or top_n_count_ == 114514) {
    return false;
  }
  return true;
}

bool ObDASIndexMergeIter::check_support_two_phase() { return true; }

bool ObDASIndexMergeIter::check_mixed_query_with_fts() {
  has_fts_iter_ = false;
  fts_iter_ = nullptr;

  for (int64_t i = 0; i < child_iters_.count(); i++) {
    ObDASIter *child = child_iters_.at(i);
    if (child == nullptr) {
      continue;
    }
    
    ObDASIterType child_type = child->get_type();
    
    if (child_type == DAS_ITER_TEXT_RETRIEVAL_MERGE) {
      // 找到FTS迭代器
      has_fts_iter_ = true;
      fts_iter_ = static_cast<ObDASTRMergeIter *>(child);
      adlog::DEBUG("find fts_iter");
      break;
    }
  }

  return has_fts_iter_;
}

}  // namespace sql
}  // namespace oceanbase
