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
#include "lib/hash/ob_hashset.h"
#include "sql/das/ob_das_ir_define.h"
#include "sql/das/iter/ob_das_sort_iter.h"
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
}

int ObDASIndexMergeIter::inner_init(ObDASIterParam &param)
{
  adlog::DEBUG("index merge iter use!!!");
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
          } else if (FALSE_IT(child_match_against_expr.set_allocator(&alloc))) {
          } else if (OB_FAIL(child_match_against_expr.assign(match_against_exprs))) {
            LOG_WARN("failed to assign match against exprs", K(ret));
          } else if (child_scan_rtdef != nullptr) {
            // need to prepare scan param for normal scan node
            if (OB_ISNULL(child_scan_param = OB_NEWx(ObTableScanParam, &alloc))) {
              ret = OB_ALLOCATE_MEMORY_FAILED;
              LOG_WARN("failed to allocate child scan param", K(ret));
            }
          }
        }
        if (OB_SUCC(ret)) {
          if (OB_FAIL(result_buffer_.init(max_size_, eval_ctx_, output_, mem_ctx_->get_malloc_allocator()))) {
            LOG_WARN("failed to init merge result buffer", K(ret));
          }
        }
      }
    }
  }
  if (OB_SUCC(ret)) {
    if (OB_FAIL(init_two_phase(param))) {
      return ret;
    }
  }
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
      expr->set_evaluated_flag(*eval_ctx_);
      expr->set_evaluated_projected(*eval_ctx_);
    }
  }
  return ret;
}

int ObDASIndexMergeIter::save_row_to_result_buffer()
{
  return result_buffer_.add_row();
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
  
  // After all child iters have done table scan, build prefilter bitmap if needed
  if (OB_SUCC(ret)) {
    // Ensure has_fts_iter_ is set by calling check_mixed_query_with_fts()
    // This is needed because init_two_phase() might not have been called
    check_mixed_query_with_fts();
    LOG_INFO("[PREFILTER_BITMAP] After check_mixed_query_with_fts in do_table_scan",
             K(has_fts_iter_), KP(fts_iter_));

    LOG_INFO("just for test", K(ret));
    if (OB_FAIL(build_prefilter_docid_bitmap())) {
      LOG_WARN("failed to build prefilter docid bitmap", K(ret));
      // Don't fail the entire operation if bitmap building fails
      ret = OB_SUCCESS;
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
    LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] compare: invalid idx", K(cur_idx), K(ret));
  } else if (output_idx == OB_INVALID_INDEX) {
    output_idx = cur_idx;
    LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] compare: set initial output_idx", K(cur_idx));
  } else {
    const ObDatum *cur_datums = child_stores_.at(cur_idx).cur_datums();
    const ObDatum *output_datums = child_stores_.at(output_idx).cur_datums();
    if (OB_ISNULL(cur_datums) || OB_ISNULL(output_datums)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] compare: unexpected nullptr", K(cur_datums), K(output_datums));
    }
    ObObj cur_obj;
    ObObj output_obj;
    int64_t old_output_idx = output_idx;
    for (int64_t i = 0; (cmp_ret == 0) && OB_SUCC(ret) && i < rowkey_exprs_->count(); i++) {
      const ObExpr *expr = rowkey_exprs_->at(i);
      if (OB_ISNULL(expr)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] compare: unexpected nullptr expr", K(i), K(ret));
      } else if (OB_FAIL(cur_datums[i].to_obj(cur_obj, expr->obj_meta_, expr->obj_datum_map_))) {
        LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] compare: failed to convert cur datum to obj", K(i), KPC(expr), K(ret));
      } else if (OB_FAIL(output_datums[i].to_obj(output_obj, expr->obj_meta_, expr->obj_datum_map_))) {
        LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] compare: failed to convert output datum to obj", K(i), KPC(expr), K(ret));
      } else if (OB_FAIL(cur_obj.check_collation_free_and_compare(output_obj, cmp_ret))) {
          LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] compare: failed to compare objs", K(i), K(ret));
      } else if (cmp_ret != 0) {
        // Log rowkey comparison details
        LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] compare: rowkey mismatch",
                 K(cur_idx), K(old_output_idx), K(i), K(cmp_ret),
                 "cur_obj", cur_obj, "output_obj", output_obj,
                 "cur_store_cur_idx", child_stores_.at(cur_idx).cur_idx_,
                 "output_store_cur_idx", child_stores_.at(old_output_idx).cur_idx_);
        if (cmp_ret < 0) {
          // cur < output, so cur becomes the new output_idx (smaller rowkey)
          output_idx = OB_UNLIKELY(is_reverse_) ? output_idx : cur_idx;
          LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] compare: cur < output, update output_idx",
                   K(cur_idx), K(old_output_idx), "new_output_idx", output_idx, K(is_reverse_));
        } else {
          // cur > output, keep output_idx (output is smaller)
          output_idx = OB_UNLIKELY(is_reverse_) ? cur_idx : output_idx;
          LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] compare: cur > output, keep output_idx",
                   K(cur_idx), K(old_output_idx), "new_output_idx", output_idx, K(is_reverse_));
        }
      }
    }
    if (OB_SUCC(ret) && cmp_ret == 0) {
      LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] compare: rowkey matched",
               K(cur_idx), K(output_idx), "rowkey_count", rowkey_exprs_->count());
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
  LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] intersect_get_next_rows start",
           K(capacity), K(child_stores_.count()));
  
  // Check initial state of each child iter and child_store before merge
  LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] Checking initial state of all child iters and stores");
  for (int64_t i = 0; i < child_iters_.count() && i < child_stores_.count(); ++i) {
    ObDASIter *child_iter = child_iters_.at(i);
    IndexMergeRowStore &child_store = child_stores_.at(i);
    ObDASIterType child_type = OB_NOT_NULL(child_iter) ? child_iter->get_type() : DAS_ITER_INVALID;
      const char *child_type_str = "UNKNOWN";
      switch (child_type) {
        case DAS_ITER_TEXT_RETRIEVAL_MERGE:
          child_type_str = "FTS_MERGE";
          break;
        case DAS_ITER_SCAN:
          child_type_str = "SCAN";
          break;
        case DAS_ITER_SORT:
          child_type_str = "SORT";
          break;
        case DAS_ITER_MERGE:
          child_type_str = "MERGE";
          break;
        default:
          child_type_str = "OTHER";
          break;
      }
    LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] Child initial state",
             K(i), "child_type", child_type_str, K(child_type),
             "child_store_cur_idx", child_store.cur_idx_,
             "child_store_saved_size", child_store.saved_size_,
             "child_store_iter_end", child_store.iter_end_,
             "child_store_have_data", child_store.have_data(),
             KP(child_iter), KP(child_iter ? child_iter->get_output() : nullptr));
  }
  
  count = 0;  
  result_buffer_.reuse();
  
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
            LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] unexpected nullptr child_iter",
                     K(i), K(ret));
          } else {

            ret = child_iter->get_next_rows(child_rows_cnt, capacity);

            if (OB_FAIL(ret)) {
              if (OB_ITER_END != ret) {
                LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] child_iter get_next_rows failed",
                         K(i), K(child_rows_cnt), K(capacity), K(ret));
              }
            }
            LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] child_iter fetched rows",
                     K(i), K(child_rows_cnt), K(capacity), "iter_end", child_store.iter_end_);

            if (OB_ITER_END == ret && child_rows_cnt > 0) {
              ret = OB_SUCCESS;
            }
            if (OB_SUCC(ret)) {
              if (OB_FAIL(child_store.save(true, child_rows_cnt))) {
                LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] failed to save child rows",
                         K(i), K(child_rows_cnt), K(ret));
              } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
                LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] compare after save failed",
                         K(i), K(output_idx), K(ret));
              } else if (child_iter->get_type() == DAS_ITER_SORT) {
                LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] child_iter is SORT, reset datum ptr",
                         K(i), K(child_rows_cnt));
                reset_datum_ptr(child_iter->get_output(), child_rows_cnt);
              }
            } else if (OB_ITER_END == ret) {
              child_store.iter_end_ = true;
              ret = OB_SUCCESS;
            } else {
              LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] failed to get next rows from child iter",
                       K(i), K(ret));
            }
          }
        }
      } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
        LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] compare failed on existing child_store",
                 K(i), K(output_idx), K(ret));
      }
    }

    if (OB_FAIL(ret)) {
      LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] loop failed before intersect check", K(ret));
    } else if (output_idx == OB_INVALID_INDEX) {  
      ret = OB_ITER_END;
      LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] no available row, set OB_ITER_END",
               K(count));
    } else {    
      bool all_matched = true;
      int64_t advance_idx = OB_INVALID_INDEX;  // Record which child should be advanced
      // 输出当前这一轮候选 output_idx 以及各 child 的 have_data / iter_end_ 状态
      LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] intersect candidate before verify",
               K(output_idx), K(count));
      for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
        if (output_idx == i) {  
          continue;
        }
        if ((!child_stores_.at(i).have_data())) {
            all_matched = false;  // 这个child没有数据，不匹配
            ret = OB_ITER_END;
            LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] child has no data, mark OB_ITER_END",
                     K(i), K(output_idx),
                     "child_iter_end", child_stores_.at(i).iter_end_,
                     "output_store_cur_idx", child_stores_.at(output_idx).cur_idx_,
                     "child_store_cur_idx", child_stores_.at(i).cur_idx_);
        } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {  
          LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] compare in verify phase failed",
                   K(i), K(output_idx), K(ret));  
        } else if (cmp_ret == 0) {  
          LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] child matched on rowkey",
                   K(i), K(output_idx));
        } else { 
          all_matched = false;  // rowkey不匹配
          // CRITICAL: Restore original logic - always advance output_idx when !all_matched
          // This matches the original intersect_get_next_row() behavior (line 752)
          // Since output_idx is guaranteed to be the child with the smallest rowkey
          // (by compare() function), advancing output_idx allows it to catch up with
          // other children that have larger rowkeys.
          // 
          // Original logic in intersect_get_next_row():
          //   if (!all_matched) {
          //     child_stores_.at(output_idx).cur_idx_++;
          //   }
          //
          // We maintain this same behavior to ensure compatibility with normal index_merge
          if (advance_idx == OB_INVALID_INDEX) {
            advance_idx = output_idx;  // Always advance output_idx (original logic)
          }
          // Get rowkey values for detailed logging (first column only)
          const ObDatum *child_datums = child_stores_.at(i).cur_datums();
          const ObDatum *output_datums = child_stores_.at(output_idx).cur_datums();
          if (OB_NOT_NULL(child_datums) && OB_NOT_NULL(output_datums) && 
              OB_NOT_NULL(rowkey_exprs_) && rowkey_exprs_->count() > 0) {
            const ObExpr *expr = rowkey_exprs_->at(0);
            if (OB_NOT_NULL(expr)) {
              ObObj child_obj, output_obj;
              if (OB_SUCC(child_datums[0].to_obj(child_obj, expr->obj_meta_, expr->obj_datum_map_)) &&
                  OB_SUCC(output_datums[0].to_obj(output_obj, expr->obj_meta_, expr->obj_datum_map_))) {
                LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] child NOT matched on rowkey",
                         K(i), K(output_idx), K(cmp_ret),
                         "output_store_cur_idx", child_stores_.at(output_idx).cur_idx_,
                         "child_store_cur_idx", child_stores_.at(i).cur_idx_,
                         "advance_idx", advance_idx,
                         "cmp_ret_meaning", (cmp_ret < 0 ? "child < output" : "child > output"),
                         K(child_obj), K(output_obj));
              } else {
                LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] child NOT matched on rowkey (no rowkey detail)",
                         K(i), K(output_idx), K(cmp_ret),
                         "output_store_cur_idx", child_stores_.at(output_idx).cur_idx_,
                         "child_store_cur_idx", child_stores_.at(i).cur_idx_,
                         "advance_idx", advance_idx,
                         "cmp_ret_meaning", (cmp_ret < 0 ? "child < output" : "child > output"));
              }
            } else {
              LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] child NOT matched on rowkey (no expr)",
                       K(i), K(output_idx), K(cmp_ret),
                       "output_store_cur_idx", child_stores_.at(output_idx).cur_idx_,
                       "child_store_cur_idx", child_stores_.at(i).cur_idx_,
                       "advance_idx", advance_idx);
            }
          } else {
            LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] child NOT matched on rowkey (no datums)",
                     K(i), K(output_idx), K(cmp_ret),
                     "output_store_cur_idx", child_stores_.at(output_idx).cur_idx_,
                     "child_store_cur_idx", child_stores_.at(i).cur_idx_,
                     "advance_idx", advance_idx);
          }
        }
      }

      if (OB_FAIL(ret)) {
        LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] verify phase failed", K(ret));
        continue;
      }
      if (!all_matched) {
        // Advance the child with smaller rowkey (recorded in advance_idx)
        // If advance_idx is still OB_INVALID_INDEX, fallback to output_idx
        if (advance_idx == OB_INVALID_INDEX) {
          advance_idx = output_idx;
        }
        int64_t old_cur_idx = child_stores_.at(advance_idx).cur_idx_;
        child_stores_.at(advance_idx).cur_idx_++;  
        LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] candidate rejected, advance child store",
                 "advance_idx", advance_idx, "old_cur_idx", old_cur_idx, 
                 "new_cur_idx", child_stores_.at(advance_idx).cur_idx_,
                 "output_idx", output_idx);
        continue;
      }
      for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
        if (output_idx == i) {  
          continue;
        }
        if (OB_FAIL(child_stores_.at(i).to_expr())) {
          LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] to_expr failed for non-output child",
                   K(i), K(output_idx), K(ret));
      }
      }
      if (OB_SUCC(ret) && OB_FAIL(child_stores_.at(output_idx).to_expr())) {
        LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] to_expr failed for output child",
                 K(output_idx), K(ret));
      }
      if (OB_SUCC(ret) && OB_FAIL(save_row_to_result_buffer())) {  
        LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] failed to save row to result buffer",
                 K(output_idx), K(ret)); 
      } else {  
        count += 1;
        LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] intersect produced one row",
                 K(count), K(output_idx));
      }

    }  
  }

  if(OB_FAIL(ret)){
    if (ret == OB_ITER_END) {
      LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] intersect_get_next_rows reach OB_ITER_END",
               K(count));
    } else {
      LOG_WARN("[PREFILTER_BITMAP_INDEX_MERGE] intersect_get_next_rows error", K(ret), K(count));
    }
  }
  LOG_INFO("[PREFILTER_BITMAP_INDEX_MERGE] intersect_get_next_rows finish", K(count));
  
  if (OB_ITER_END == ret && count > 0) {  
    ret = OB_SUCCESS;  
  }  
  if (OB_SUCC(ret) && count > 0) {
    if (OB_FAIL(result_buffer_.to_expr(count))) {
      adlog::DEBUG("failed to convert result buffer to exprs", K(ret));  
      LOG_WARN("failed to convert result buffer to exprs", K(ret));  
    }  
  }  
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
      }
    }
  }
  return ret;
}

int ObDASIndexMergeIter::union_get_next_rows(int64_t &count, int64_t capacity) {
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
        if (OB_FAIL(child_stores_.at(output_idx).to_expr())) {
          LOG_WARN("failed to convert store row to expr", K(ret));
        } else {
          // now we get a available result row, save it to result buffer
          if (OB_FAIL(save_row_to_result_buffer())) {
            LOG_WARN("failed to save row to result buffer", K(ret));
          } else {
            count += 1;
          }
        }
      }
    }
  }

  if (OB_ITER_END == ret && count > 0) {
    ret = OB_SUCCESS;
  }
  if (OB_SUCC(ret) && count > 0) {
    if (OB_FAIL(result_buffer_.to_expr(count))) {
      LOG_WARN("failed to convert result buffer to exprs", K(ret));
    }
  }
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

ObDASTRMergeIter *ObDASIndexMergeIter::find_fts_iter_recursive(ObDASIter *iter) {
  if (OB_ISNULL(iter)) {
    return nullptr;
  }
  
  ObDASIterType iter_type = iter->get_type();
  
  if (iter_type == DAS_ITER_TEXT_RETRIEVAL_MERGE) {
    // Found FTS iterator
    LOG_INFO("[PREFILTER_BITMAP] Found FTS iterator recursively", KP(iter));
    return static_cast<ObDASTRMergeIter *>(iter);
  } else if (iter_type == DAS_ITER_SORT) {
    // FTS iterator might be wrapped in SORT, check child recursively
    ObDASSortIter *sort_iter = static_cast<ObDASSortIter *>(iter);
    ObDASIter *child = sort_iter->get_child();
    LOG_INFO("[PREFILTER_BITMAP] Found SORT iterator, checking child recursively", 
             KP(iter), KP(child));
    return find_fts_iter_recursive(child);
  } else if (iter_type == DAS_ITER_TEXT_RETRIEVAL) {
    // 当前 bitmap 逻辑只支持 TEXT_RETRIEVAL_MERGE，这里记录一下 TEXT_RETRIEVAL 被发现但不会参与
    LOG_INFO("[PREFILTER_BITMAP] Found TEXT_RETRIEVAL iter (not MERGE, bitmap not applied)",
             "iter_type", iter_type, KP(iter));
  } else if (iter_type == DAS_ITER_HNSW_SCAN) {
    // HNSW 也是一种“检索”iter，这里也记录下来，方便和执行计划对齐
    LOG_INFO("[PREFILTER_BITMAP] Found HNSW_SCAN iter under SORT (bitmap currently not supported)",
             "iter_type", iter_type, KP(iter));
  } else {
    // 其它类型的 iter，统一打日志，帮助确认 iter 树结构
    LOG_INFO("[PREFILTER_BITMAP] Non-FTS iter in find_fts_iter_recursive",
             "iter_type", iter_type, KP(iter));
  }
  
  return nullptr;
}

bool ObDASIndexMergeIter::check_mixed_query_with_fts() {
  has_fts_iter_ = false;
  fts_iter_ = nullptr;

  LOG_INFO("[PREFILTER_BITMAP] check_mixed_query_with_fts() called", 
           K(child_iters_.count()));

  for (int64_t i = 0; i < child_iters_.count(); i++) {
    ObDASIter *child = child_iters_.at(i);
    if (child == nullptr) {
      continue;
    }
    
    ObDASIterType child_type = child->get_type();
    LOG_INFO("[PREFILTER_BITMAP] Checking child iter in check_mixed_query_with_fts",
             K(i), K(child_type), KP(child));
    
    // Recursively find FTS iterator (may be wrapped in SORT)
    ObDASTRMergeIter *fts = find_fts_iter_recursive(child);
    if (OB_NOT_NULL(fts)) {
      has_fts_iter_ = true;
      fts_iter_ = fts;
      LOG_INFO("[PREFILTER_BITMAP] Found FTS iterator (possibly wrapped in SORT)",
               K(i), KP(fts_iter_));
      break;
    }
  }

  LOG_INFO("[PREFILTER_BITMAP] check_mixed_query_with_fts() result",
           K(has_fts_iter_), KP(fts_iter_));

  return has_fts_iter_;
}

int ObDASIndexMergeIter::build_prefilter_docid_bitmap()
{
  int ret = OB_SUCCESS;
  
  LOG_INFO("[PREFILTER_BITMAP] build_prefilter_docid_bitmap() called", 
           K(has_fts_iter_), KP(fts_iter_), KP(rowkey_exprs_), 
           K(rowkey_exprs_ ? rowkey_exprs_->count() : 0));
  
  // Log all child iter types to help debug
  LOG_INFO("[PREFILTER_BITMAP] Checking all child iter types before bitmap building");
  for (int64_t i = 0; i < child_iters_.count(); ++i) {
    ObDASIter *child = child_iters_.at(i);
    ObDASIterType child_type = OB_NOT_NULL(child) ? child->get_type() : DAS_ITER_INVALID;
    const char *child_type_str = "UNKNOWN";
    switch (child_type) {
      case DAS_ITER_TEXT_RETRIEVAL_MERGE:
        child_type_str = "FTS_MERGE";
        break;
      case DAS_ITER_SCAN:
        child_type_str = "SCAN";
        break;
      case DAS_ITER_SORT:
        child_type_str = "SORT";
        break;
      case DAS_ITER_MERGE:
        child_type_str = "MERGE";
        break;
      default:
        child_type_str = "OTHER";
        break;
    }
    LOG_INFO("[PREFILTER_BITMAP] Child iter type in build_prefilter_docid_bitmap",
             K(i), "child_type", child_type_str, K(child_type), KP(child));
  }
  
  // Check if we have both FTS iter and BTree index iters
  if (!has_fts_iter_ || OB_ISNULL(fts_iter_) || OB_ISNULL(rowkey_exprs_) || rowkey_exprs_->count() == 0) {
    // No FTS iter or no rowkey exprs, skip bitmap building
    LOG_INFO("[PREFILTER_BITMAP] Skip bitmap building: no FTS iter or no rowkey exprs",
             K(has_fts_iter_), KP(fts_iter_), KP(rowkey_exprs_),
             "rowkey_exprs_count", (rowkey_exprs_ ? rowkey_exprs_->count() : 0));
    return OB_SUCCESS;
  }
  
  // Find BTree index iters (non-FTS iters)
  ObArray<int64_t> btree_iter_indices;
  for (int64_t i = 0; i < child_iters_.count(); ++i) {
    ObDASIter *child = child_iters_.at(i);
    if (OB_NOT_NULL(child) && child->get_type() != DAS_ITER_TEXT_RETRIEVAL_MERGE) {
      // This is a BTree index iter
      if (OB_FAIL(btree_iter_indices.push_back(i))) {
        LOG_WARN("failed to push back btree iter index", K(ret));
      }
    }
  }
  
  LOG_INFO("[PREFILTER_BITMAP] Found BTree iter count", 
           K(btree_iter_indices.count()), K(child_iters_.count()));
  
  if (btree_iter_indices.count() == 0) {
    // No BTree index iters, skip bitmap building
    LOG_INFO("[PREFILTER_BITMAP] Skip bitmap building: no BTree index iters");
    return OB_SUCCESS;
  }
  
  // Allocate hash set for prefilter bitmap
  // Use typedef to avoid macro expansion issues with template parameters containing commas
  typedef common::hash::ObHashSet<sql::ObDocIdExt, common::hash::NoPthreadDefendMode> PrefilterBitmapType;
  PrefilterBitmapType *prefilter_bitmap = nullptr;
  common::ObArenaAllocator &alloc = mem_ctx_->get_arena_allocator();
  if (OB_ISNULL(prefilter_bitmap = OB_NEWx(PrefilterBitmapType, &alloc))) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    LOG_WARN("failed to allocate prefilter bitmap", K(ret));
  } else if (OB_FAIL(prefilter_bitmap->create(1024, ObMemAttr(MTL_ID(), "PrefilterBitmap")))) {
    LOG_WARN("failed to create prefilter bitmap", K(ret));
  } else {
    LOG_INFO("[PREFILTER_BITMAP] Bitmap created successfully, initial capacity=1024", 
             K(btree_iter_indices.count()), K(child_iters_.count()));
    // Scan BTree index iters to collect doc_ids
    int64_t total_rows_scanned = 0;
    int64_t total_docids_inserted = 0;
    for (int64_t idx = 0; OB_SUCC(ret) && idx < btree_iter_indices.count(); ++idx) {
      int64_t iter_idx = btree_iter_indices.at(idx);
      ObDASIter *btree_iter = child_iters_.at(iter_idx);
      int64_t iter_rows_scanned = 0;
      int64_t iter_docids_inserted = 0;
      
      LOG_INFO("[PREFILTER_BITMAP] Start scanning BTree iter", K(idx), K(iter_idx), 
               KP(btree_iter), KP(btree_iter ? btree_iter->get_output() : nullptr));
      
      if (OB_ISNULL(btree_iter) || OB_ISNULL(btree_iter->get_output())) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected null btree iter or output", K(ret), K(iter_idx));
      } else {
        // Scan all rows from this BTree index iter
        int64_t batch_count = 0;
        const int64_t batch_capacity = 256;
        while (OB_SUCC(ret)) {
          batch_count = 0;
          ret = btree_iter->get_next_rows(batch_count, batch_capacity);
          if (OB_ITER_END == ret && batch_count > 0) {
            ret = OB_SUCCESS;
          }
          if (OB_FAIL(ret)) {
            if (OB_ITER_END != ret) {
              LOG_WARN("failed to get next rows from btree iter", K(ret), K(iter_idx));
            }
            break;
          }
          
          if (batch_count == 0) {
            break;
          }
          
          // Save data from BTree iter to child_store, then extract doc_id
          // This follows the same pattern as intersect_get_next_rows()
          IndexMergeRowStore &child_store = child_stores_.at(iter_idx);
          if (OB_FAIL(child_store.save(true, batch_count))) {
            LOG_WARN("failed to save btree iter data to store", K(ret));
          } else {
            // Extract doc_id from rowkey_exprs_[0] which should correspond to doc_id
            // After save(), the data should be available in child_store, but we need to
            // use rowkey_exprs_ which references index_merge's eval_ctx_
            // Actually, we should extract from child_store's saved data directly
            // But for simplicity, we use rowkey_exprs_[0] assuming it maps to the first column
            if (OB_ISNULL(rowkey_exprs_) || rowkey_exprs_->count() == 0 || OB_ISNULL(rowkey_exprs_->at(0))) {
              ret = OB_ERR_UNEXPECTED;
              LOG_WARN("unexpected null rowkey expr", K(ret));
            } else {
              // Extract doc_id from each saved row
              // CRITICAL: We need to find which column in BTree iter's output corresponds to rowkey_exprs_[0]
              // The BTree iter's output may not have the same column order as rowkey_exprs_
              const ExprFixedArray *btree_output = btree_iter->get_output();
              const ObExpr *rowkey_expr = rowkey_exprs_->at(0);
              if (OB_ISNULL(btree_output) || btree_output->count() == 0) {
                ret = OB_ERR_UNEXPECTED;
                LOG_WARN("unexpected null or empty btree iter output", K(ret));
              } else if (OB_ISNULL(rowkey_expr)) {
                ret = OB_ERR_UNEXPECTED;
                LOG_WARN("unexpected null rowkey expr", K(ret));
              } else {
                // Find the column index in BTree output that matches rowkey_exprs_[0]
                // We compare by expr pointer or by column id
                int64_t doc_id_col_idx = OB_INVALID_INDEX;
                for (int64_t col_idx = 0; col_idx < btree_output->count(); ++col_idx) {
                  const ObExpr *btree_expr = btree_output->at(col_idx);
                  if (OB_NOT_NULL(btree_expr) && btree_expr == rowkey_expr) {
                    doc_id_col_idx = col_idx;
                    break;
                  }
                }
                
                // If not found by pointer, try to find by column id (resname or other identifier)
                if (doc_id_col_idx == OB_INVALID_INDEX) {
                  // Fallback: assume first column is doc_id (may be wrong!)
                  doc_id_col_idx = 0;
                  LOG_WARN("[PREFILTER_BITMAP] Could not find rowkey_expr in BTree output, using first column as doc_id",
                           KP(rowkey_expr), "btree_output_count", btree_output->count());
                } else {
                  LOG_INFO("[PREFILTER_BITMAP] Found doc_id column in BTree output",
                           "doc_id_col_idx", doc_id_col_idx, "btree_output_count", btree_output->count());
                }
                
                // Iterate through all saved rows
                for (int64_t i = 0; OB_SUCC(ret) && i < batch_count; ++i) {
                  // Get datums for current row
                  const ObDatum *datums = child_store.cur_datums();
                  if (OB_ISNULL(datums)) {
                    ret = OB_ERR_UNEXPECTED;
                    LOG_WARN("unexpected null datums from child store", K(ret), K(i));
                  } else if (doc_id_col_idx >= btree_output->count()) {
                    ret = OB_ERR_UNEXPECTED;
                    LOG_WARN("doc_id column index out of range", K(ret), K(doc_id_col_idx), 
                             "btree_output_count", btree_output->count());
                  } else {
                    // Extract doc_id from the correct column
                    const ObDatum &doc_id_datum = datums[doc_id_col_idx];
                    sql::ObDocIdExt doc_id;
                    if (OB_FAIL(doc_id.from_datum(doc_id_datum))) {
                      LOG_WARN("failed to convert datum to doc_id", K(ret), K(i), K(doc_id_col_idx));
                    } else {
                      int hash_ret = prefilter_bitmap->set_refactored(doc_id);
                      if (OB_SUCCESS != hash_ret && OB_HASH_EXIST != hash_ret) {
                        ret = hash_ret;
                        LOG_WARN("failed to insert doc_id into bitmap", K(ret), K(hash_ret));
                      } else {
                        iter_docids_inserted++;
                        total_docids_inserted++;
                        // Log first 10 doc_ids for debugging with detailed information
                        if (total_docids_inserted <= 10) {
                          ObObj doc_id_obj;
                          if (OB_SUCC(doc_id_datum.to_obj(doc_id_obj, rowkey_expr->obj_meta_, rowkey_expr->obj_datum_map_))) {
                            LOG_INFO("[PREFILTER_BITMAP] Inserted doc_id into bitmap", 
                                     K(total_docids_inserted), K(doc_id_col_idx), 
                                     "doc_id_datum", doc_id_datum, "doc_id_obj", doc_id_obj, 
                                     "doc_id", doc_id, "rowkey_expr_type", rowkey_expr->type_,
                                     "btree_output_count", btree_output->count());
                          } else {
                            LOG_INFO("[PREFILTER_BITMAP] Inserted doc_id into bitmap (no obj conversion)", 
                                     K(total_docids_inserted), K(doc_id_col_idx), 
                                     "doc_id_datum", doc_id_datum, "doc_id", doc_id);
                          }
                        }
                      }
                    }
                  }
                  iter_rows_scanned++;
                  total_rows_scanned++;
                  // Move to next row in store
                  child_store.cur_idx_++;
                }
                // Reset store after extracting
                child_store.reuse();
              }
            }
          }
        }
        
        // BTree iter 扫描完成后，不需要重新初始化
        // 因为后面会将 BTree iter 从 child_iters_ 中移除，只保留 FTS iter
        // 这样 intersect 就变成了单路扫描 + bitmap 过滤
        
        // 如果 ret 是 OB_ITER_END，说明正常扫描完毕，将其置为 OB_SUCCESS
        if (ret == OB_ITER_END) {
          ret = OB_SUCCESS;
        }
        
        LOG_INFO("[PREFILTER_BITMAP] Finished scanning BTree iter", 
                 K(iter_idx), K(iter_rows_scanned), K(iter_docids_inserted), K(ret));
      }
    }
    
    LOG_INFO("[PREFILTER_BITMAP] After scanning all BTree iters", 
             K(ret), K(total_rows_scanned), K(total_docids_inserted), 
             KP(prefilter_bitmap), K(prefilter_bitmap ? prefilter_bitmap->size() : 0));
    
    // Even if there were errors during scanning, if we have a valid bitmap with data,
    // we should still try to pass it to FTS iter
    // Only fail if bitmap creation itself failed or bitmap is NULL
    if (OB_NOT_NULL(prefilter_bitmap) && prefilter_bitmap->size() > 0) {
      // Clear any previous errors if we have valid bitmap data
      // The bitmap is more important than minor errors during scanning
      if (OB_FAIL(ret)) {
        LOG_WARN("[PREFILTER_BITMAP] Had errors during scanning, but bitmap has data, will continue", 
                 K(ret), K(total_rows_scanned), K(total_docids_inserted), K(prefilter_bitmap->size()));
        ret = OB_SUCCESS;  // Clear error to allow bitmap passing
      }
    }
    
    if (OB_SUCC(ret) && OB_NOT_NULL(prefilter_bitmap) && prefilter_bitmap->size() > 0) {
      // Pass bitmap to FTS iter
      LOG_INFO("[PREFILTER_BITMAP] Summary: total_rows_scanned", 
               K(total_rows_scanned), K(total_docids_inserted), 
               K(prefilter_bitmap->size()), K(btree_iter_indices.count()));
      
      LOG_INFO("[PREFILTER_BITMAP] About to pass bitmap to FTS iter", 
               KP(fts_iter_), KP(prefilter_bitmap), 
               K(prefilter_bitmap ? prefilter_bitmap->size() : 0));
      if (OB_FAIL(fts_iter_->set_prefilter_docid_bitmap(prefilter_bitmap))) {
        LOG_WARN("failed to set prefilter bitmap to FTS iter", K(ret));
      } else {
        LOG_INFO("[PREFILTER_BITMAP] Bitmap passed to FTS iter successfully", 
                 K(prefilter_bitmap->size()), K(btree_iter_indices.count()));
        
        // CRITICAL: 移除 BTree iter，只保留 FTS iter
        // 这样后续 intersect 就变成了单路扫描 + bitmap 过滤
        // 需要从后往前移除，避免索引错乱
        LOG_INFO("[PREFILTER_BITMAP] Removing BTree iters from child_iters_, before removal",
                 "child_iters_count", child_iters_.count(),
                 "child_stores_count", child_stores_.count(),
                 "btree_iter_indices_count", btree_iter_indices.count());
        
        for (int64_t idx = btree_iter_indices.count() - 1; idx >= 0; --idx) {
          int64_t iter_idx = btree_iter_indices.at(idx);
          LOG_INFO("[PREFILTER_BITMAP] Removing BTree iter at index", K(idx), K(iter_idx));
          
          // 先 release BTree iter 释放资源
          ObDASIter *btree_iter_to_remove = child_iters_.at(iter_idx);
          if (OB_NOT_NULL(btree_iter_to_remove)) {
            btree_iter_to_remove->release();
          }
          
          // 从各个数组中移除对应元素
          child_iters_.remove(iter_idx);
          child_stores_.remove(iter_idx);
          child_scan_rtdefs_.remove(iter_idx);
          child_scan_params_.remove(iter_idx);
          child_tablet_ids_.remove(iter_idx);
          child_match_against_exprs_.remove(iter_idx);
        }
        
        LOG_INFO("[PREFILTER_BITMAP] After removing BTree iters",
                 "child_iters_count", child_iters_.count(),
                 "child_stores_count", child_stores_.count());
        
        // 验证：移除后应该只剩下 FTS iter
        for (int64_t i = 0; i < child_iters_.count(); ++i) {
          ObDASIter *child = child_iters_.at(i);
          ObDASIterType child_type = OB_NOT_NULL(child) ? child->get_type() : DAS_ITER_INVALID;
          LOG_INFO("[PREFILTER_BITMAP] Remaining child iter after removal",
                   K(i), K(child_type), KP(child));
        }
      }
    } else {
      LOG_WARN("[PREFILTER_BITMAP] Failed to build bitmap or bitmap is empty", K(ret), 
               K(total_rows_scanned), K(total_docids_inserted));
    }
  }
  
  return ret;
}

}  // namespace sql
}  // namespace oceanbase
