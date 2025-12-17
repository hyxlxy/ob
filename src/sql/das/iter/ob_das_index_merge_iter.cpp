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
    // 获取 LIMIT Top-K（若存在），用于提前终止
    // 注意：ObDASIndexMergeRtDef 当前不携带 limit 参数，这里保持默认关闭早停（top_n_count_=0）
    // 如需启用，请在上层（如 sort/tsc）下传 limit 或扩展 RtDef 结构。
    top_n_count_ = 0;

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
  produced_rows_ = 0;
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
  // 两阶段：首次取数前配置BMW TopK
  if (enable_two_phase_ && !has_executed_two_phase_) {
    if (OB_FAIL(execute_two_phase())) {
      LOG_WARN("execute_two_phase failed", K(ret));
    }
  }
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
  if (top_n_count_ > 0 && produced_rows_ >= top_n_count_) {
    return OB_ITER_END;
  }
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
            ++produced_rows_;
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
  // 两阶段触发点：首次进入时执行两阶段预备
  if (enable_two_phase_ && !has_executed_two_phase_) {
    if (OB_FAIL(execute_two_phase())) {
      LOG_WARN("failed to execute two-phase preparation", K(ret));
    }
  }
  // {
  //   adlog::DEBUG("INTERSECT rowkey_exprs_ count", K(rowkey_exprs_->count()));  
  //   for (int64_t i = 0; i < rowkey_exprs_->count(); i++) {
  //     // adlog::DEBUG("rowkey_expr", i, std::to_string(*rowkey_exprs_->at(i)));
  //     {
  //       char buf[1024];  
  //       int64_t pos = 0;  
  //       pos = rowkey_exprs_->at(i)->to_string(buf, sizeof(buf));  
  //       adlog::DEBUG("rowkey_expr", i, std::string(buf, pos));
  //     }
  //   }
  // }
  
  count = 0;  
  result_buffer_.reuse();
  
  while (OB_SUCC(ret) && count < capacity) {  
    if (top_n_count_ > 0 && produced_rows_ >= top_n_count_) {
      ret = OB_ITER_END;
      break;
    }
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
      ret = OB_ITER_END;
      adlog::DEBUG("ret = OB_ITER_END");
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
      if (!all_matched) {
        child_stores_.at(output_idx).cur_idx_++;  
        continue;
      }
      for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
        if (output_idx == i) {  
          continue;
        }
        child_stores_.at(i).to_expr();
      }
      child_stores_.at(output_idx).to_expr();
      if (OB_FAIL(save_row_to_result_buffer())) {  
        adlog::DEBUG("failed to save row to result buffer", K(ret)); 
      } else {  
        count += 1;
        ++produced_rows_;
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
  adlog::DEBUG("intersect_get_next_rows :",K(count));
  
  if (OB_ITER_END == ret && count > 0) {  
    ret = OB_SUCCESS;  
  }  
  
  // 两阶段补齐：当本批结束且仍未达TopN，尝试用保底结果补齐当前批次
  // 注意：补齐逻辑会直接修改count和result_buffer_
  if (OB_SUCC(ret) && enable_two_phase_ && second_phase_need_supplement_ && produced_rows_ < top_n_count_) {
    if (OB_FAIL(supplement_with_fallback_rows(count, capacity))) {
      LOG_WARN("failed to supplement with fallback rows", K(ret));
    }
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
    if (top_n_count_ > 0 && produced_rows_ >= top_n_count_) {
      ret = OB_ITER_END;
      break;
    }
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
  int ret = OB_SUCCESS;
  
  // 防止重复执行
  if (has_executed_two_phase_) {
    adlog::DEBUG("Two-phase already executed, skip");
    return OB_SUCCESS;
  }
  has_executed_two_phase_ = true;
  
  LOG_DEBUG("Starting two-phase index merge optimization", K(top_n_count_), K(top_n_amplify_factor_));
  adlog::DEBUG("Starting two-phase index merge optimization", K(top_n_count_), K(top_n_amplify_factor_));
  
  // 第一阶段：保底取数
  if (OB_FAIL(first_merge_phase())) {
    LOG_WARN("First merge phase failed", K(ret));
    adlog::DEBUG("First merge phase failed", K(ret));
    return ret;
  }
  
  // 第二阶段：BMW精准筛选
  if (OB_FAIL(second_merge_phase())) {
    LOG_WARN("Second merge phase failed", K(ret));
    adlog::DEBUG("Second merge phase failed", K(ret));
    return ret;
  }
  
  LOG_DEBUG("Two-phase index merge optimization completed", K(first_phase_min_score_), K(first_phase_fallback_count_));
  adlog::DEBUG("Two-phase index merge optimization completed", K(first_phase_min_score_), K(first_phase_fallback_count_));
  
  return ret;
}

int ObDASIndexMergeIter::first_merge_phase() {
  // 第一阶段：保底取数
  // 使用标准Index Merge逻辑快速取N条满足标量条件的行
  // 同时计算这N条行的全文相关性得分，记录最小得分用于BMW阈值
  int ret = OB_SUCCESS;
  int64_t n = top_n_count_;
  
  LOG_DEBUG("First merge phase: collecting fallback rows", K(n));
  
  // 保存第一阶段的保底行
  if (OB_FAIL(save_first_phase_fallback_rows())) {
    LOG_WARN("Failed to save first phase fallback rows", K(ret));
    return ret;
  }
  
  first_phase_completed_ = true;
  LOG_DEBUG("First merge phase completed", K(first_phase_fallback_count_), K(first_phase_min_score_));
  
  return ret;
}

int ObDASIndexMergeIter::second_merge_phase() {
  // 第二阶段：BMW精准筛选
  // 启用FTS迭代器的TopK模式，设置得分阈值，取n'候选
  // 用标量条件过滤n'候选，凑满N条，不足则用第一阶段保底结果补充
  int ret = OB_SUCCESS;
  
  if (!has_fts_iter_ || OB_ISNULL(fts_iter_)) {
    LOG_DEBUG("No FTS iterator in second phase, skip");
    return OB_SUCCESS;
  }
  
  int64_t n = top_n_count_;
  int64_t n_prime = n * top_n_amplify_factor_;  // 放大因子，可配置
  
  adlog::DEBUG("Second merge phase: enabling BMW TopK mode", K(n), K(n_prime), K(first_phase_min_score_));
  LOG_DEBUG("Second merge phase: enabling BMW TopK mode", K(n), K(n_prime), K(first_phase_min_score_));
  
  // 启用FTS迭代器的TopK模式（BMW算法）
  if (OB_FAIL(fts_iter_->enable_topk_mode(n_prime))) {
    LOG_WARN("Failed to enable topk mode for FTS iterator", K(ret), K(n_prime));
    adlog::DEBUG("Failed to enable topk mode for FTS iterator", K(ret), K(n_prime));
    // 不中断流程，继续执行（TopK启用失败不影响正确性，只是性能优化）
    ret = OB_SUCCESS;
  } else {
    adlog::DEBUG("Successfully enabled topk mode for FTS iterator", K(n_prime));
  }
  
  // 设置BMW的得分阈值（用于剪枝）
  // 这样可以跳过得分低于第一阶段最小得分的候选
  if (first_phase_min_score_ > 0.0) {
    if (OB_FAIL(fts_iter_->set_baseline_threshold(first_phase_min_score_))) {
      LOG_DEBUG("Failed to set baseline threshold for BMW pruning", K(ret), K(first_phase_min_score_));
      // 不中断流程，继续执行（阈值设置失败不影响正确性，只是性能优化）
      ret = OB_SUCCESS;
    } else {
      adlog::DEBUG("Successfully set baseline threshold for BMW pruning", K(first_phase_min_score_));
    }
  }
  
  // 清空子store中的残留数据，避免使用第一阶段的数据
  for (int64_t i = 0; i < child_stores_.count(); ++i) {
    child_stores_.at(i).reuse();
  }
  
  // 重新扫描以启用新的迭代器配置
  if (OB_FAIL(rescan())) {
    LOG_WARN("Failed to rescan after enabling topk mode", K(ret));
    adlog::DEBUG("Failed to rescan after enabling topk mode", K(ret));
    return ret;
  }
  
  // 标记需要补充标志：当第二阶段结果不足N条时，用第一阶段保底结果补充
  second_phase_need_supplement_ = true;
  
  adlog::DEBUG("Second merge phase enabled", K(n_prime), "will supplement with fallback rows if needed");
  LOG_DEBUG("Second merge phase enabled", K(n_prime));
  
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
  
  adlog::DEBUG("init_two_phase: merge_type_=", K(merge_type_), "INDEX_MERGE_INTERSECT=", K(INDEX_MERGE_INTERSECT));
  
  // 只有INTERSECT模式（AND操作）才支持两阶段优化
  if (merge_type_ != INDEX_MERGE_INTERSECT) {
    LOG_DEBUG("Two-phase optimization only supports INTERSECT merge type");
    adlog::DEBUG("Two-phase optimization only supports INTERSECT merge type");
    return OB_SUCCESS;
  }

  // 检查是否存在FTS迭代器
  if (!check_mixed_query_with_fts()) {
    LOG_DEBUG("No FTS iterator found for two-phase optimization");
    adlog::DEBUG("No FTS iterator found for two-phase optimization");
    return OB_SUCCESS;
  }

  if (!has_fts_iter_) {
    LOG_DEBUG("FTS iterator not available");
    adlog::DEBUG("FTS iterator not available");
    return OB_SUCCESS;
  }

  adlog::DEBUG("Found FTS iterator, fts_iter_=", K(fts_iter_));

  // 若上层未传入TopN，尝试从FTS iter读取limit
  if (top_n_count_ <= 0) {
    int64_t fts_limit = 0;
    if (OB_FAIL(fts_iter_->get_topk_limit(fts_limit))) {
      LOG_DEBUG("Failed to get topk limit from FTS iterator", K(ret));
      adlog::DEBUG("Failed to get topk limit from FTS iterator", K(ret));
      ret = OB_SUCCESS; // 忽略错误，继续
    } else if (fts_limit > 0) {
      top_n_count_ = fts_limit;
      LOG_DEBUG("Inferred top_n_count from FTS child", K(top_n_count_));
      adlog::DEBUG("Inferred top_n_count from FTS child", K(top_n_count_));
    }
  }

  // 如果 top_n_count_ 仍未设置，则跳过两阶段
  if (top_n_count_ <= 0) {
    LOG_DEBUG("Top-N count not set, skip two-phase optimization");
    adlog::DEBUG("Top-N count not set, skip two-phase optimization");
    return OB_SUCCESS;
  }

  // 检查是否满足两阶段优化的条件
  if (!check_support_two_phase()) {
    LOG_DEBUG("Two-phase optimization conditions not met");
    adlog::DEBUG("Two-phase optimization conditions not met");
    return OB_SUCCESS;
  }

  // 标记启用两阶段优化
  enable_two_phase_ = true;
  LOG_DEBUG("Two-phase optimization enabled", K(top_n_count_), K(top_n_amplify_factor_));
  adlog::DEBUG("Two-phase optimization enabled", K(top_n_count_), K(top_n_amplify_factor_));

  return ret;
}

int ObDASIndexMergeIter::enable_two_phase_optimization(int64_t top_n_count, int64_t amplify_factor) {
  int ret = OB_SUCCESS;
  
  if (top_n_count <= 0) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("Invalid top_n_count for two-phase optimization", K(ret), K(top_n_count));
  } else if (amplify_factor <= 0) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("Invalid amplify_factor for two-phase optimization", K(ret), K(amplify_factor));
  } else {
    top_n_count_ = top_n_count;
    top_n_amplify_factor_ = amplify_factor;
    LOG_DEBUG("Two-phase optimization configured", K(top_n_count_), K(top_n_amplify_factor_));
  }
  
  return ret;
}

bool ObDASIndexMergeIter::check_support_two_phase() {
  // 检查是否满足两阶段优化的条件
  // 当前版本支持所有满足基本条件的查询
  // 可根据需要扩展条件检查（如：检查是否有标量过滤、是否有排序等）
  return true;
}

bool ObDASIndexMergeIter::check_mixed_query_with_fts() {
  has_fts_iter_ = false;
  fts_iter_ = nullptr;

  adlog::DEBUG("check_mixed_query_with_fts: child_iters_.count()=", K(child_iters_.count()));
  
  for (int64_t i = 0; i < child_iters_.count(); i++) {
    ObDASIter *child = child_iters_.at(i);
    if (child == nullptr) {
      adlog::DEBUG("check_mixed_query_with_fts: child", K(i), "is null");
      continue;
    }
    
    ObDASIterType child_type = child->get_type();
    adlog::DEBUG("check_mixed_query_with_fts: child", K(i), "type=", K(child_type), "DAS_ITER_TEXT_RETRIEVAL_MERGE=", K(DAS_ITER_TEXT_RETRIEVAL_MERGE));
    
    if (child_type == DAS_ITER_TEXT_RETRIEVAL_MERGE) {
      // 找到FTS迭代器
      has_fts_iter_ = true;
      fts_iter_ = static_cast<ObDASTRMergeIter *>(child);
      adlog::DEBUG("Found FTS iterator at index", K(i), "fts_iter_=", K(fts_iter_));
      break;
    }
  }

  adlog::DEBUG("check_mixed_query_with_fts result: has_fts_iter_=", K(has_fts_iter_), "fts_iter_=", K(fts_iter_));
  return has_fts_iter_;
}

int ObDASIndexMergeIter::extract_relevance_score_from_row(double &score) {
  // 从当前行的表达式中提取全文相关性得分
  // 遍历 output_ 表达式查找 MATCH_AGAINST 表达式
  int ret = OB_SUCCESS;
  score = 0.0;
  
  if (OB_ISNULL(output_)) {
    LOG_DEBUG("output_ is null, cannot extract relevance score");
    return OB_SUCCESS;
  }
  
  for (int64_t i = 0; OB_SUCC(ret) && i < output_->count(); ++i) {
    ObExpr *expr = output_->at(i);
    if (OB_NOT_NULL(expr) && T_FUN_MATCH_AGAINST == expr->type_) {
      // 使用 locate_expr_datum 获取表达式的 datum
      const ObDatum &datum = expr->locate_expr_datum(*eval_ctx_);
      if (!datum.is_null()) {
        double expr_score = datum.get_double();
        score = MAX(score, expr_score); // 取最大得分
        adlog::DEBUG("Extracted relevance score from MATCH_AGAINST", K(expr_score), K(score));
      }
    }
  }
  
  return ret;
}

int ObDASIndexMergeIter::save_first_phase_fallback_rows() {
  // 第一阶段：保底取数
  // 直接从子迭代器取N条结果，保存到fallback缓存，记录min_score
  int ret = OB_SUCCESS;
  int64_t n = top_n_count_;
  int64_t count = 0;
  
  adlog::DEBUG("First phase: collecting", K(n), "rows for fallback");
  
  // 分配保底行缓存（预分配 n 行）
  if (OB_ISNULL(first_phase_fallback_rows_)) {
    common::ObArenaAllocator &alloc = mem_ctx_->get_arena_allocator();
    first_phase_fallback_max_ = n;
    if (OB_ISNULL(first_phase_fallback_rows_ = 
        static_cast<ObChunkDatumStore::LastStoredRow*>(
            alloc.alloc(n * sizeof(ObChunkDatumStore::LastStoredRow))))) {
      ret = OB_ALLOCATE_MEMORY_FAILED;
      LOG_WARN("failed to allocate fallback rows buffer", K(ret), K(n));
    } else {
      for (int64_t i = 0; i < n; ++i) {
        new (&first_phase_fallback_rows_[i]) ObChunkDatumStore::LastStoredRow(alloc);
        first_phase_fallback_rows_[i].reuse_ = true;
      }
    }
  }
  
  // 第一阶段：直接使用普通Index Merge逻辑取N条行
  // 这里复用intersect_get_next_rows的核心逻辑，但不递归调用
  while (OB_SUCC(ret) && count < n) {
    int64_t output_idx = OB_INVALID_INDEX;
    int cmp_ret = 0;
    
    // 填充各child store
    for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
      IndexMergeRowStore &child_store = child_stores_.at(i);
      if (!child_store.have_data()) {
        if (!child_store.iter_end_) {
          ObDASIter *child_iter = child_iters_.at(i);
          if (OB_ISNULL(child_iter)) {
            ret = OB_ERR_UNEXPECTED;
            LOG_WARN("unexpected nullptr child iter", K(i), K(ret));
          } else {
            int64_t child_rows_cnt = 0;
            ret = child_iter->get_next_rows(child_rows_cnt, n - count);
            if (OB_ITER_END == ret && child_rows_cnt > 0) {
              ret = OB_SUCCESS;
            }
            if (OB_SUCC(ret)) {
              if (OB_FAIL(child_store.save(true, child_rows_cnt))) {
                LOG_WARN("failed to save child rows", K(child_rows_cnt), K(ret));
              } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
                LOG_WARN("failed to compare row", K(i), K(output_idx), K(ret));
              }
            } else if (OB_ITER_END == ret) {
              child_store.iter_end_ = true;
              ret = OB_SUCCESS;
            }
          }
        }
      } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
        LOG_WARN("failed to compare row", K(i), K(output_idx), K(ret));
      }
    }
    
    if (OB_FAIL(ret)) {
      break;
    } else if (output_idx == OB_INVALID_INDEX) {
      // 所有child都已结束
      ret = OB_ITER_END;
      break;
    } else {
      // 检查是否所有child都有匹配的行
      bool all_matched = true;
      for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
        if (output_idx == i) {
          continue;
        }
        if (!child_stores_.at(i).have_data()) {
          all_matched = false;
          break;
        } else if (OB_FAIL(compare(i, output_idx, cmp_ret))) {
          LOG_WARN("failed to compare row", K(i), K(output_idx), K(ret));
        } else if (cmp_ret != 0) {
          all_matched = false;
          break;
        }
      }
      
      if (OB_FAIL(ret)) {
        break;
      } else if (!all_matched) {
        // rowkey不匹配，跳过当前行
        child_stores_.at(output_idx).cur_idx_++;
      } else {
        // 找到一条匹配的行，保存到fallback缓存
        // 先转换表达式到eval_ctx
        for (int64_t i = 0; OB_SUCC(ret) && i < child_stores_.count(); i++) {
          if (output_idx != i) {
            if (OB_FAIL(child_stores_.at(i).to_expr())) {
              LOG_WARN("failed to convert child row to expr", K(ret), K(i));
            }
          }
        }
        if (OB_SUCC(ret)) {
          if (OB_FAIL(child_stores_.at(output_idx).to_expr())) {
            LOG_WARN("failed to convert output row to expr", K(ret));
          }
        }
        
        // 保存到fallback缓存
        if (OB_SUCC(ret)) {
          ObEvalCtx::BatchInfoScopeGuard batch_info_guard(*eval_ctx_);
          batch_info_guard.set_batch_size(1);
          batch_info_guard.set_batch_idx(0);
          if (OB_FAIL(first_phase_fallback_rows_[count].save_store_row(*output_, *eval_ctx_))) {
            LOG_WARN("failed to save fallback row", K(ret), K(count));
          } else {
            // 提取并记录该行的得分
            double row_score = 0.0;
            if (OB_FAIL(extract_relevance_score_from_row(row_score))) {
              LOG_WARN("failed to extract relevance score", K(ret));
              // 不中断流程，继续处理
              ret = OB_SUCCESS;
            } else {
              // 只在有有效得分时更新min_score
              if (row_score > 0.0) {
                if (first_phase_min_score_ == 0.0 || row_score < first_phase_min_score_) {
                  first_phase_min_score_ = row_score;
                }
              }
              adlog::DEBUG("First phase row", K(count), "score=", K(row_score), "min_score=", K(first_phase_min_score_));
            }
            count++;
          }
        }
      }
    }
  }
  
  first_phase_fallback_count_ = count;
  first_phase_fallback_cursor_ = 0;
  
  adlog::DEBUG("First phase completed with", K(count), "rows, min_score=", K(first_phase_min_score_));
  
  return ret;
}

int ObDASIndexMergeIter::supplement_with_fallback_rows(int64_t &count, int64_t capacity) {
  // 第二阶段结束后，如果结果不足N条，用第一阶段的保底结果补齐
  int ret = OB_SUCCESS;
  int64_t need_cnt = top_n_count_ - produced_rows_;
  
  adlog::DEBUG("supplement_with_fallback_rows start", K(need_cnt), K(produced_rows_), K(top_n_count_), K(count), K(capacity));
  
  if (need_cnt <= 0 || first_phase_fallback_count_ <= 0) {
    adlog::DEBUG("supplement_with_fallback_rows: no need to supplement", K(need_cnt), K(first_phase_fallback_count_));
    return OB_SUCCESS;
  }
  
  int64_t can_supplement = MIN(need_cnt, first_phase_fallback_count_ - first_phase_fallback_cursor_);
  can_supplement = MIN(can_supplement, capacity - count);
  
  adlog::DEBUG("Supplementing with fallback rows", K(can_supplement), K(need_cnt), K(count), K(capacity), K(first_phase_fallback_cursor_), K(first_phase_fallback_count_));
  
  if (can_supplement <= 0) {
    adlog::DEBUG("supplement_with_fallback_rows: can_supplement <= 0");
    return OB_SUCCESS;
  }
  
  // 从fallback缓存中取行，转换为表达式
  ObEvalCtx::BatchInfoScopeGuard batch_info_guard(*eval_ctx_);
  batch_info_guard.set_batch_size(1);
  for (int64_t i = 0; OB_SUCC(ret) && i < can_supplement; ++i) {
    batch_info_guard.set_batch_idx(0);
    int64_t idx = first_phase_fallback_cursor_ + i;
    
    if (OB_UNLIKELY(idx < 0 || idx >= first_phase_fallback_count_)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("invalid fallback row index", K(ret), K(idx), K(first_phase_fallback_count_));
      adlog::DEBUG("invalid fallback row index", K(ret), K(idx), K(first_phase_fallback_count_));
      break;
    }
    
    if (OB_ISNULL(first_phase_fallback_rows_[idx].store_row_)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("null store row in fallback", K(ret), K(idx));
      adlog::DEBUG("null store row in fallback", K(ret), K(idx));
      break;
    }
    
    if (OB_FAIL(first_phase_fallback_rows_[idx].store_row_->to_expr<true>(*output_, *eval_ctx_))) {
      LOG_WARN("failed to convert fallback row to expr", K(ret), K(idx));
      adlog::DEBUG("failed to convert fallback row to expr", K(ret), K(idx));
    } else if (OB_FAIL(save_row_to_result_buffer())) {
      LOG_WARN("failed to save supplemented row to result buffer", K(ret));
      adlog::DEBUG("failed to save supplemented row to result buffer", K(ret));
    } else {
      count += 1;
      ++produced_rows_;
      adlog::DEBUG("Supplemented row", K(i), K(count), K(produced_rows_));
    }
  }
  
  first_phase_fallback_cursor_ += can_supplement;
  
  adlog::DEBUG("supplement_with_fallback_rows end", K(count), K(produced_rows_), K(first_phase_fallback_cursor_));
  
  return ret;
}

}  // namespace sql
}  // namespace oceanbase
