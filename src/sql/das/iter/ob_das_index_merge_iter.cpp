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

#define USING_LOG_PREFIX SQL_DAS
#include "sql/das/iter/ob_das_index_merge_iter.h"
#include "src/sql/das/ob_das_attach_define.h"
#include "sql/das/iter/sparse_retrieval/ob_das_tr_merge_iter.h"
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
    
    lib::ContextParam context_param;
    context_param.set_mem_attr(MTL_ID(), "DASIndexMerge", ObCtxIds::DEFAULT_CTX_ID)
        .set_properties(lib::USE_TL_PAGE_OPTIONAL);
    if (OB_UNLIKELY(merge_type_ != INDEX_MERGE_UNION)) {
      ret = OB_INVALID_ARGUMENT;
      LOG_WARN("invalid merge type", K(merge_type_));
    } else if (OB_FAIL(CURRENT_CONTEXT->CREATE_CONTEXT(mem_ctx_, context_param))) {
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
          } else {
            // Initialize two-phase optimization
            // Detect FT (Text Retrieval) child and enable two-phase only when present
            fts_child_idx_ = -1;
            for (int64_t i = 0; i < child_iters_.count(); ++i) {
              if (OB_NOT_NULL(child_iters_.at(i)) &&
                  child_iters_.at(i)->get_type() == ObDASIterType::DAS_ITER_TEXT_RETRIEVAL_MERGE) {
                fts_child_idx_ = i;
                break;
              }
            }
            two_phase_enabled_ = (fts_child_idx_ >= 0);
            // Can be further controlled by hint/variable; keep simple now
            phase1_limit_ = max_size_ * 2;  // initial oversampling for phase1 buffer
            phase1_min_score_ = 0.0;
            phase1_completed_ = false;
            phase2_started_ = false;
            LOG_TRACE("[IndexMerge] two-phase init", K(two_phase_enabled_), K_(fts_child_idx), K_(phase1_limit));
          }
        }
      }
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
  // Reset two-phase optimization state
  phase1_results_.reuse();
  phase2_results_.reuse();
  phase1_min_score_ = 0.0;
  phase1_completed_ = false;
  phase2_started_ = false;
  // Reset phase1 persistent pool
  phase1_store_.reset();
  phase1_store_size_ = 0;
  phase1_used_.reset();
  phase1_sorted_built_ = false;
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

int ObDASIndexMergeIter::intersect_get_next_row()
{
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
  bool got_row = false;
  while (OB_SUCC(ret) && count < capacity) {
      /* try to fill each child store */
    int64_t output_idx = OB_INVALID_INDEX;
    int cmp_ret = 0;
    int64_t child_rows_cnt = 0;
    for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
      IndexMergeRowStore &child_store = child_stores_.at(i);
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
      bool all_matched = true;
      for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
        if (output_idx == i) {
          /* skip */
        } else if (child_stores_.at(i).iter_end_ || !child_stores_.at(i).have_data()) {
          ret = OB_ITER_END;
        } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
            LOG_WARN("index merge failed to compare row", K(i), K(output_idx), K(ret));
        } else if (cmp_ret == 0) {
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
            count += 1;
          }
        } else {
          child_stores_.at(output_idx).cur_idx_++;
        }
      }
    }
  }

  return ret;
}

int ObDASIndexMergeIter::union_get_next_row()
{
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

int ObDASIndexMergeIter::union_get_next_rows(int64_t &count, int64_t capacity)
{
  int ret = OB_SUCCESS;
  count = 0;
  
  // Use two-phase optimization if enabled
  if (two_phase_enabled_ && (!phase1_completed_ || phase2_started_)) {
    return union_get_next_rows_two_phase(count, capacity);
  }
  
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

// Two-phase optimization implementation
int ObDASIndexMergeIter::union_get_next_rows_two_phase(int64_t &count, int64_t capacity)
{
  int ret = OB_SUCCESS;
  count = 0;
  
  // Phase 1: Get top N rows with normal index merge (保底)
  if (!phase1_completed_) {
    if (OB_FAIL(phase1_get_top_n_rows(count, capacity))) {
      LOG_WARN("failed to get phase 1 results", K(ret));
    } else {
      phase1_completed_ = true;
      phase2_started_ = true;
      return ret;  // Return phase 1 results first
    }
  }
  
  // Phase 2: Get results with score threshold from BM25 iterator
  int64_t phase2_cnt = 0;
  if (phase2_started_ && phase1_completed_) {
    if (OB_FAIL(phase2_get_with_score_threshold(phase2_cnt, capacity))) {
      if (OB_ITER_END != ret) {
        LOG_WARN("failed to run phase2 with score threshold", K(ret));
      } else {
        ret = OB_SUCCESS; // exhausted BMW
      }
    }
  }

  int64_t total_cnt = phase2_cnt;

  // Supplement from phase1 if BMW produced less than capacity
  if (OB_SUCC(ret) && phase2_cnt < capacity && phase1_store_size_ > 0) {
    const int64_t need = capacity - phase2_cnt;
    int64_t added = 0;
    // build a simple dedup set from phase2 batch docids
    auto exists_in_phase2 = [this](const ObString &docid) -> bool {
      for (int64_t i = 0; i < phase2_batch_docids_.count(); ++i) {
        if (phase2_batch_docids_.at(i) == docid) return true;
      }
      return false;
    };
    // sort phase1 by score only once
    if (!phase1_sorted_built_) {
      std::sort(phase1_results_.begin(), phase1_results_.end(),
        [](const std::pair<double, int64_t> &a, const std::pair<double, int64_t> &b){ return a.first > b.first; });
      phase1_sorted_built_ = true;
    }
    for (int64_t i = 0; OB_SUCC(ret) && i < phase1_results_.count() && added < need; ++i) {
      const int64_t idx = phase1_results_.at(i).second;
      if (idx < 0 || idx >= phase1_rows_.count()) { continue; }
      if (idx < phase1_used_.count() && phase1_used_.at(idx)) { continue; }
      if (idx < phase1_docids_.count() && exists_in_phase2(phase1_docids_.at(idx))) { continue; }
      // materialize this phase1 row to output & buffer
      ObChunkDatumStore::LastStoredRow *p = phase1_rows_.at(idx);
      if (OB_ISNULL(p) || OB_ISNULL(p->store_row_)) { continue; }
      {
        ObEvalCtx::BatchInfoScopeGuard guard(*eval_ctx_);
        guard.set_batch_size(1);
        guard.set_batch_idx(0);
        if (OB_FAIL(p->store_row_->to_expr<true>(*output_, *eval_ctx_))) {
          LOG_WARN("phase1 supplement to_expr failed", K(ret));
          break;
        }
      }
      if (OB_FAIL(save_row_to_result_buffer())) {
        LOG_WARN("save supplement row failed", K(ret));
        break;
      }
      if (idx < phase1_used_.count()) {
        phase1_used_.at(idx) = true;
      }
      ++added;
    }
    total_cnt += added;
  }

  // finalize
  if (OB_SUCC(ret) && total_cnt > 0) {
    if (OB_FAIL(result_buffer_.to_expr(total_cnt))) {
      LOG_WARN("failed to convert result buffer to exprs (2phase)", K(ret));
    }
  } else if (0 == total_cnt) {
    ret = OB_ITER_END;
  }

  count = total_cnt;
  // clear phase2 batch docids for next call
  phase2_batch_docids_.reset();

  return ret;
}

int ObDASIndexMergeIter::phase1_get_top_n_rows(int64_t &count, int64_t capacity)
{
  int ret = OB_SUCCESS;
  count = 0;
  result_buffer_.reuse();
  
  // Run normal union merge to get top N rows (保底)
  int64_t phase1_capacity = phase1_limit_;
  while (OB_SUCC(ret) && count < phase1_capacity) {
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
            ret = child_iter->get_next_rows(child_rows_cnt, phase1_capacity - count);
            if (OB_ITER_END == ret && child_rows_cnt > 0) {
              ret = OB_SUCCESS;
            }
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
      ret = OB_ITER_END;
    } else {
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
            need_fill_default_value = true;
          }
        } else {
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
          // Extract and store the match score
          double score = extract_match_score_from_row(fts_child_idx_);
          if (score < phase1_min_score_ || phase1_min_score_ == 0.0) {
            phase1_min_score_ = score;
          }
          // Persist current output row for later supplement
          ObChunkDatumStore::LastStoredRow *p =
              static_cast<ObChunkDatumStore::LastStoredRow*>(mem_ctx_->get_arena_allocator().alloc(sizeof(ObChunkDatumStore::LastStoredRow)));
          if (OB_ISNULL(p)) {
            ret = OB_ALLOCATE_MEMORY_FAILED;
            LOG_WARN("alloc last stored row failed", K(ret));
          } else {
            new (p) ObChunkDatumStore::LastStoredRow(mem_ctx_->get_arena_allocator());
            p->reuse_ = true;
            if (OB_FAIL(p->save_store_row(*output_, *eval_ctx_))) {
              LOG_WARN("save store row failed", K(ret));
            }
          }
          if (OB_SUCC(ret)) {
            if (phase1_used_.get_data() == nullptr) {
              phase1_used_.set_allocator(&mem_ctx_->get_arena_allocator());
              (void)phase1_used_.init(16);
            }
            if (OB_NOT_NULL(p)) {
              if (OB_FAIL(phase1_rows_.push_back(p))) {
                LOG_WARN("push phase1 row failed", K(ret));
              }
            }
            if (OB_SUCC(ret)) {
              if (OB_FAIL(phase1_scores_.push_back(score))) {
                LOG_WARN("push phase1 score failed", K(ret));
              } else if (OB_FAIL(phase1_used_.push_back(false))) {
                LOG_WARN("push phase1 used flag failed", K(ret));
              } else if (OB_FAIL(phase1_results_.push_back(std::make_pair(score, phase1_rows_.count() - 1)))) {
                LOG_WARN("failed to push phase1 result", K(ret));
              } else {
                ++phase1_store_size_;
              }
            }
          }
          if (OB_SUCC(ret)) {
            // extract and persist docid for dedup
            ObString docid;
            if (OB_FAIL(extract_docid_from_output(docid))) {
              LOG_WARN("extract docid failed", K(ret));
            } else if (OB_FAIL(phase1_docids_.push_back(docid))) {
              LOG_WARN("push phase1 docid failed", K(ret));
            }
            if (OB_SUCC(ret)) {
              if (OB_FAIL(save_row_to_result_buffer())) {
                LOG_WARN("failed to save row to result buffer", K(ret));
              } else {
                // mark used in phase1
                if (phase1_rows_.count() > 0) {
                  const int64_t idx_mark = phase1_rows_.count() - 1;
                  if (idx_mark >= 0 && idx_mark < phase1_used_.count()) {
                    phase1_used_.at(idx_mark) = true;
                  }
                }
                count += 1;
              }
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
    if (OB_FAIL(result_buffer_.to_expr(count))) {
      LOG_WARN("failed to convert result buffer to exprs", K(ret));
    }
  }
  return ret;
}

int ObDASIndexMergeIter::phase2_get_with_score_threshold(int64_t &count, int64_t capacity)
{
  int ret = OB_SUCCESS;
  count = 0;
  result_buffer_.reuse();

  // Require a TR child
  if (fts_child_idx_ < 0 || fts_child_idx_ >= child_iters_.count()) {
    return OB_ITER_END;
  }

  // Downcast and set runtime params
  ObDASIter *fts_child = child_iters_.at(fts_child_idx_);
  ObDASTRMergeIter *tr_merge = dynamic_cast<ObDASTRMergeIter *>(fts_child);
  if (nullptr == tr_merge) {
    return OB_ITER_END;
  }

  // Adaptive over-sampling
  int64_t nprime = phase1_limit_ > 0 ? (phase1_limit_ * 2) : (capacity * 4);
  if (nprime <= 0) { nprime = capacity; }
  const double growth = 1.5;
  const int64_t max_candidates = 100000; // hard cap

  (void)tr_merge->set_min_score_threshold(phase1_min_score_);

  int64_t total_examined = 0;
  int64_t produced_in_this_call = 0;

  while (OB_SUCC(ret) && count < capacity && total_examined < max_candidates) {
    const int64_t pull = MIN(nprime, max_candidates - total_examined);
    (void)tr_merge->set_runtime_topk_limit(pull);

    int64_t child_rows_cnt = 0;
    if (OB_FAIL(fts_child->get_next_rows(child_rows_cnt, pull))) {
      if (OB_ITER_END != ret) {
        LOG_WARN("phase2 fts child get_next_rows failed", K(ret));
      } else {
        ret = OB_SUCCESS; // treat as exhausted
      }
    }

    if (child_rows_cnt <= 0) {
      break; // no more candidates from BMW
    }

    total_examined += child_rows_cnt;

    IndexMergeRowStore &fts_store = child_stores_.at(fts_child_idx_);
    if (OB_FAIL(fts_store.save(true, child_rows_cnt))) {
      LOG_WARN("failed to save fts child rows", K(ret));
      break;
    }

    for (int64_t i = 0; OB_SUCC(ret) && i < child_rows_cnt && count < capacity; ++i) {
      // set pivot to i-th FT row
      fts_store.cur_idx_ = i;
      bool all_matched = true;
      int64_t pivot_idx = fts_child_idx_;
      int cmp_ret = 0;
      for (int64_t c = 0; OB_SUCC(ret) && c < child_stores_.count() && all_matched; ++c) {
        if (c == fts_child_idx_) { continue; }
        IndexMergeRowStore &child_store = child_stores_.at(c);
        // advance child until >= pivot, or exhaust
        while (OB_SUCC(ret)) {
          if (!child_store.have_data()) {
            if (child_store.iter_end_) { all_matched = false; break; }
            int64_t got = 0;
            ObDASIter *child_iter = child_iters_.at(c);
            if (OB_ISNULL(child_iter)) { ret = OB_ERR_UNEXPECTED; LOG_WARN("null child iter", K(c)); break; }
            if (OB_FAIL(child_iter->get_next_rows(got, 1))) {
              if (OB_ITER_END == ret) { child_store.iter_end_ = true; ret = OB_SUCCESS; all_matched = false; }
              else { LOG_WARN("child get_next_rows failed", K(ret)); }
              break;
            } else if (got <= 0) {
              child_store.iter_end_ = true; all_matched = false; break;
            } else if (OB_FAIL(child_store.save(true, got))) {
              LOG_WARN("save child rows failed", K(ret));
              break;
            }
          }
          // compare child current vs pivot (freeze pivot index)
          int64_t tmp_idx = pivot_idx;
          if (OB_FAIL(compare(c, tmp_idx, cmp_ret))) { LOG_WARN("compare failed", K(ret), K(c), K(pivot_idx)); break; }
          if (cmp_ret == 0) {
            break; // this child matched
          } else if (cmp_ret < 0) {
            // child < pivot, advance child cursor
            child_store.cur_idx_++;
          } else {
            // child > pivot, cannot match
            all_matched = false; break;
          }
        }
      }

      phase2_examined_++;
      if (!all_matched) {
        phase2_filtered_out_++;
        continue; // skip this FT row, move to next i
      }

      // project FT row
      if (OB_FAIL(fts_store.to_expr())) {
        LOG_WARN("failed to convert fts store row to expr", K(ret));
      }
      // fill defaults for non-FT children (only relevance for now)
      for (int64_t c = 0; OB_SUCC(ret) && c < child_stores_.count(); ++c) {
        if (c == fts_child_idx_) continue;
        if (OB_FAIL(fill_default_values_for_union(child_match_against_exprs_.at(c)))) {
          LOG_WARN("failed to fill default values for union (phase2)", K(ret), K(c));
        }
      }
      // extract and record docid for dedup in supplement
      if (OB_SUCC(ret)) {
        ObString docid;
        if (OB_FAIL(extract_docid_from_output(docid))) {
          LOG_WARN("extract phase2 docid failed", K(ret));
        } else if (!docid.empty()) {
          if (OB_FAIL(phase2_batch_docids_.push_back(docid))) {
            LOG_WARN("push phase2 docid failed", K(ret));
          }
        }
      }
      if (OB_SUCC(ret)) {
        if (OB_FAIL(save_row_to_result_buffer())) {
          LOG_WARN("failed to save row to result buffer (phase2)", K(ret));
        } else {
          ++count;
          ++produced_in_this_call;
        }
      }
    }

    if (count < capacity) {
      // not enough, increase nprime and continue pulling if candidates remain
      nprime = static_cast<int64_t>(nprime * growth);
      if (nprime <= 0) nprime = capacity; // guard overflow
    }
  }

  LOG_TRACE("[IndexMerge][Phase2] stats", K(count), K(capacity), K(total_examined), K_(phase2_examined), K_(phase2_filtered_out));

  // Do not finalize result buffer here; leave to caller for possible supplement merge
  if (count == 0) {
    // Exhausted FT candidates; disable two-phase and let normal union path continue in subsequent calls
    two_phase_enabled_ = false;
    ret = OB_ITER_END;
  }

  return ret;
}

int ObDASIndexMergeIter::merge_phase_results(int64_t &count, int64_t capacity)
{
  int ret = OB_SUCCESS;
  count = 0;
  result_buffer_.reuse();
  
  // Merge results from phase 1 and phase 2
  // For now, just return phase 1 results
  if (phase1_results_.count() > 0) {
    // Sort phase 1 results by score (descending)
    std::sort(phase1_results_.begin(), phase1_results_.end(),
              [](const std::pair<double, int64_t> &a, const std::pair<double, int64_t> &b) {
                return a.first > b.first;  // Descending order
              });
    
    // Return top capacity results
    count = std::min((int64_t)phase1_results_.count(), capacity);
    // Results are already in the buffer, just return the count
  }
  
  return ret;
}

double ObDASIndexMergeIter::extract_match_score_from_row(int64_t fts_child_idx)
{
  double score = 0.0;
  // 尝试从包含 MATCH AGAINST 的子迭代器输出中获取得分
  // 优先使用传入的 fts_child_idx；若无效，则扫描子列表中第一个包含 T_FUN_MATCH_AGAINST 的子
  int64_t target_idx = fts_child_idx;
  if (target_idx < 0 || target_idx >= child_match_against_exprs_.count() ||
      child_match_against_exprs_.at(target_idx).count() == 0) {
    target_idx = -1;
    for (int64_t i = 0; i < child_match_against_exprs_.count(); ++i) {
      if (child_match_against_exprs_.at(i).count() > 0) { target_idx = i; break; }
    }
  }
  if (target_idx >= 0) {
    const ExprFixedArray &arr = child_match_against_exprs_.at(target_idx);
    if (arr.count() > 0 && OB_NOT_NULL(arr.at(0))) {
      ObExpr *expr = arr.at(0);
      ObDatum &d = expr->locate_expr_datum(*eval_ctx_);
      if (!d.is_null()) {
        // relevance 是 double
        score = d.get_double();
      }
    }
  }
  return score;
}

int ObDASIndexMergeIter::extract_docid_from_output(common::ObString &docid)
{
  int ret = OB_SUCCESS;
  docid.reset();
  if (OB_ISNULL(output_) || output_->count() <= 0 || OB_ISNULL(output_->at(0))) {
    ret = OB_ERR_UNEXPECTED;
  } else {
    ObExpr *first = output_->at(0);
    ObDatum &d = first->locate_expr_datum(*eval_ctx_);
    if (!d.is_null() && first->datum_meta_.type_ == ObVarcharType) {
      if (OB_FAIL(ob_write_string(mem_ctx_->get_arena_allocator(), d.get_string(), docid))) {
        // keep ret
      }
    }
  }
  return ret;
}


}  // namespace sql
}  // namespace oceanbase
