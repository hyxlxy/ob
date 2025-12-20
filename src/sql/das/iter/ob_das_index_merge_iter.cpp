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
     
     // topn_limit_ = max_size;  // 设置堆容量限制  
     // enable_topn_ = (topn_limit_ > 0);
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
   if (enable_topn_) { 
     topn_heap_.reset();
   }
 }
 //ADVISE
 int ObDASIndexMergeIter::MergeResultBuffer::add_row_with_topn() {
   adlog::DEBUG("ObDASIndexMergeIter::MergeResultBuffer::add_row_with_topn");
   int ret = OB_SUCCESS;
 
   auto make_topn_row = [&](TopNRow*& row) -> int {
     int ret = OB_SUCCESS;
 
 
     double score = 0.0;  
     ObSEArray<ObExpr *, 4> match_against_exprs;  
       
     for (int64_t i = 0; OB_SUCC(ret) && i < exprs_->count(); ++i) {  
       ObExpr *expr = exprs_->at(i);  
       if (OB_ISNULL(expr)) {  
         ret = OB_ERR_UNEXPECTED;  
       } else if (T_FUN_MATCH_AGAINST == expr->type_) {  
         if (OB_FAIL(match_against_exprs.push_back(expr))) {  
           LOG_WARN("failed to push back match against expr", K(ret));  
         }  
       }  
     }  
       
     if (OB_SUCC(ret) && !match_against_exprs.empty()) {  
       ObExpr *score_expr = match_against_exprs.at(0);  
       ObDatum &score_datum = score_expr->locate_expr_datum(*eval_ctx_);  
       if (!score_datum.is_null()) {  
         score = score_datum.get_double();  
       }  
       adlog::DEBUG("直接获取的分数: ", score);  
     }  
 
     ObChunkDatumStore::StoredRow *stored_row = nullptr;  
     if (OB_FAIL(result_store_.add_row(*exprs_, eval_ctx_, &stored_row))) {  
       return ret;  
     } else {
       // 在add_row成功后检查
       adlog::DEBUG("checking...");
       if (OB_SUCC(ret) && OB_NOT_NULL(stored_row)) {  
         adlog::DEBUG("stored_row cnt", K(stored_row->cnt_));  
         for (int64_t i = 0; i < stored_row->cnt_; ++i) {  
           adlog::DEBUG("datum", K(i), K(stored_row->cells()[i].is_null()));  
         }  
       }
     }
 
     adlog::DEBUG("构建的row的score: ", score);
     row = OB_NEWx(TopNRow, alloc_, stored_row, score);   
 
     return ret;
   };
 
   TopNRow *row = nullptr;
   adlog::DEBUG("add_row_with_topn");
   adlog::DEBUG("exprs count", K(exprs_->count()));  
   adlog::DEBUG("eval_ctx ptr", K(eval_ctx_)); 
   if (OB_FAIL(make_topn_row(row))) {
     return ret;
   }
   adlog::DEBUG("make_topn_row(row): row->score_: ",row->score_);
   if (topn_heap_.empty() or topn_heap_.count() < topn_limit_) {
     if (OB_FAIL(topn_heap_.push(row))) {
       adlog::DEBUG("index merge iter: topn_heap push error");
     } else {
       adlog::DEBUG("push topn_heap: row->score_: ",row->score_);
     }
     return ret;
   }
 
   TopNRow *heap_top = topn_heap_.top();  
   if (OB_NOT_NULL(heap_top)) {  
   }else{ // heap top竟然是空指针
     adlog::DEBUG("NO WAY!!!! heap top nullptr");
     return ret;
   }
 
   if (row->score_ <= heap_top->score_) {
     adlog::DEBUG("row->score_ <= heap_top->score_ :", row->score_ , "  ",heap_top->score_);
     alloc_->free(row);
     // ret = OB_ERR_UNEXPECTED;
     return ret;
   }
   topn_heap_.pop();
   topn_heap_.push(row);
   adlog::DEBUG("row->score_ > heap_top->score_ :", row->score_ ,"  ",heap_top->score_);
 
   alloc_->free(heap_top);  
   // if (OB_FAIL(topn_heap_.replace_top(row))) {  
   //   LOG_WARN("failed to replace heap top", K(ret));  
   // } else {    
   // }  
   
   
   return ret;
 }
 
 
 int ObDASIndexMergeIter::MergeResultBuffer::to_expr_from_topn(int64_t &size)  
 {
   int ret = OB_SUCCESS;
   adlog::DEBUG("MergeResultBuffer::to_expr_from_topn");
   adlog::DEBUG("to_expr exprs count", K(exprs_->count()));  
   adlog::DEBUG("to_expr eval_ctx ptr", K(eval_ctx_));
   common::ObArray<TopNRow *> sorted_rows;
   adlog::DEBUG(K(topn_heap_.count()));
   while (topn_heap_.count() > 0) {
     auto &&top = topn_heap_.top();
     if (OB_FAIL(sorted_rows.push_back(top))) {
       adlog::DEBUG("ERROR! sorted_row push_back error");
     }
     topn_heap_.pop();
   }
 
   if (OB_FAIL(ret)) {
     adlog::DEBUG("MergeResultBuffer::to_expr_from_topn OB_FAIL(ret)");
     return ret;
   }
   adlog::DEBUG(K(sorted_rows.count()));
   std::sort(sorted_rows.begin(), sorted_rows.end(),
             [](const TopNRow *a, const TopNRow *b) {
               return a->score_ > b->score_; //
             });
 
 //   { // debug
 // /*
 
 //       } else if (OB_FAIL(result_store_.init(UINT64_MAX, MTL_ID(), ObCtxIds::DEFAULT_CTX_ID, "DASIndexMerge"))) {
 //     LOG_WARN("failed to init result store", K(ret));
 //   } else {
 //     result_store_.set_allocator(alloc);
 // */
 //     ObChunkDatumStore debug_store("DebugTopN");
 //     if (OB_FAIL(debug_store.init(UINT64_MAX, MTL_ID(), ObCtxIds::DEFAULT_CTX_ID, "DASIndexMergeDebug"))) {
 //       LOG_WARN("debug store init failed", K(ret));
 //       return ret;
 //     } else {
 //       debug_store.set_allocator(*alloc_);
 //     }
 
 //     {
 //       ObEvalCtx::BatchInfoScopeGuard guard(*eval_ctx_);
 //       guard.set_batch_size(1);
 //       for (int64_t i = 0; OB_SUCC(ret) && i < sorted_rows.count(); ++i) {
 //         guard.set_batch_idx(0);
 //         TopNRow *row = sorted_rows.at(i);
 //         if (OB_NOT_NULL(row) && OB_NOT_NULL(row->row_data_)) {
 //           // 先把行写入 eval_ctx_
 //           if (OB_FAIL(row->row_data_->to_expr<true>(*exprs_, *eval_ctx_))) {
 //             LOG_WARN("to_expr failed", K(i));
 //           } else if (OB_FAIL(debug_store.add_row(*exprs_, eval_ctx_))) {
 //             LOG_WARN("debug_store add_row failed", K(i));
 //           }
 //         }
 //       }
 //     }
 
 //     // 用标准路径输出
 //     ObChunkDatumStore::Iterator iter;
 //     int64_t read_size = 0;
 //     if (OB_SUCC(ret)) {
 //       if (OB_FAIL(debug_store.begin(iter))) {
 //         LOG_WARN("debug_store begin failed", K(ret));
 //       } else if (OB_FAIL(iter.get_next_batch<true>(*exprs_, *eval_ctx_, 
 //                                                   sorted_rows.count(), read_size))) {
 //         LOG_WARN("get_next_batch from debug_store failed", K(ret));
 //       } else {
 //         size = read_size;
 //       }
 //     }
 //     adlog::DEBUG("ObChunkDatumStore::Iterator iter read_size: ",read_size);
 //   }
   {  
     ObEvalCtx::BatchInfoScopeGuard batch_info_guard(*eval_ctx_);  
     batch_info_guard.set_batch_size(sorted_rows.count());  
       
     for (int64_t i = 0; OB_SUCC(ret) && i < sorted_rows.count(); ++i) {  
       batch_info_guard.set_batch_idx(i);  
       TopNRow *row = sorted_rows.at(i);  
       if (OB_ISNULL(row) || OB_ISNULL(row->row_data_)) {  
         ret = OB_ERR_UNEXPECTED;  
         LOG_WARN("unexpected null row", K(ret));  
       } else if (OB_FAIL(row->row_data_->to_expr<true>(*exprs_, *eval_ctx_))) {  
         LOG_WARN("failed to convert row to expr", K(ret));
       } else {
         adlog::DEBUG("row->row_data_->to_expr<true>(*exprs_, *eval_ctx_) ok!");
         adlog::DEBUG("index_merge里输出的 i: " , i, " score: ", row->score_);
       }
     }  
       
     if (OB_SUCC(ret)) {  
       size = sorted_rows.count();  
     }
 
     for (auto &&one : sorted_rows) {
       // delete one;
       alloc_->free(one);
 
     }
     
   }  
   // topn_heap_.reset();  
   // result_store_iter_.reset();
   // result_store_.reset();
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
           } else {
             if (merge_rtdef_->enable_topn_pushdown_) {
               result_buffer_.enable_topn_ = true;
               result_buffer_.topn_limit_ = merge_rtdef_->topn_limit_;
               // adlog::DEBUG("result_buffer_.topn_limit_ =  ",)
               // result_buffer_.enable_topn_ = false;
               adlog::DEBUG(K(result_buffer_.topn_limit_));
             } else {
               result_buffer_.enable_topn_ = false;
               adlog::DEBUG("result_buffer_.topn_limit _ 无");
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
 
 int ObDASIndexMergeIter::save_row_to_result_buffer() {
   // adlog::DEBUG()
   // if (!result_buffer_.enable_topn_ or true) {
   if (!result_buffer_.enable_topn_) {
     return result_buffer_.add_row();
   }
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
 
   if (result_buffer_.enable_topn_) {
     capacity += 100;
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
         if (result_buffer_.enable_topn_) { 
           count = result_buffer_.topn_heap_.count();
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