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
#include "sql/das/iter/ob_das_cache_lookup_iter.h"
#include "sql/engine/ob_exec_context.h"
#include "sql/das/ob_das_ir_define.h"
#include "storage/concurrency_control/ob_data_validation_service.h"

namespace oceanbase
{
using namespace common;
namespace sql
{

int ObDASCacheLookupIter::IndexProjRowStore::init(common::ObIAllocator &allocator,
                                                  const common::ObIArray<ObExpr*> &exprs,
                                                  ObEvalCtx *eval_ctx,
                                                  int64_t max_size)
{
  int ret = OB_SUCCESS;
  if (OB_ISNULL(eval_ctx)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected nullptr for init index proj row store", K(ret));
  } else if (OB_ISNULL(store_rows_ =
      static_cast<ObChunkDatumStore::LastStoredRow*>(allocator.alloc(max_size * sizeof(ObChunkDatumStore::LastStoredRow))))) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    LOG_WARN("failed to allocate memory", K(max_size), K(ret));
  } else if (FALSE_IT(index_scan_proj_exprs_.set_allocator(&allocator))) {
  } else if (OB_FAIL(index_scan_proj_exprs_.assign(exprs))) {
    LOG_WARN("failed to assign index scan proj exprs", K(max_size), K(ret));
  } else {
    eval_ctx_ = eval_ctx;
    max_size_ = max_size;
    for (int64_t i = 0; i < max_size_; i++) {
      new (store_rows_ + i) ObChunkDatumStore::LastStoredRow(allocator);
      store_rows_[i].reuse_ = true;
    }
  }

  return ret;
}

int ObDASCacheLookupIter::IndexProjRowStore::save(bool is_vectorized, int64_t size)
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(index_scan_proj_exprs_.count() == 0)) {
    // do nothing
  } else if (OB_UNLIKELY(size + saved_size_ > max_size_) ||
             OB_ISNULL(store_rows_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected error for save store rows", K(size), K(max_size_), K(store_rows_), K(ret));
  } else {
    if (is_vectorized) {
      ObEvalCtx::BatchInfoScopeGuard batch_info_guard(*eval_ctx_);
      batch_info_guard.set_batch_size(size);
      for (int64_t i = 0; OB_SUCC(ret) && i < size; i++) {
        batch_info_guard.set_batch_idx(i);
        if (OB_FAIL(store_rows_[i + saved_size_].save_store_row(index_scan_proj_exprs_, *eval_ctx_))) {
          LOG_WARN("cache lookup iter failed to store rows", K(ret));
        }
      }
    } else if (OB_FAIL(store_rows_[saved_size_].save_store_row(index_scan_proj_exprs_, *eval_ctx_))) {
      LOG_WARN("cache lookup iter failed to store rows", K(ret));
    }
  }
  if (OB_SUCC(ret)) {
    cur_idx_ = 0;
    saved_size_ += size;
  }

  return ret;
}

int ObDASCacheLookupIter::IndexProjRowStore::to_expr(int64_t size)
{
  int ret = OB_SUCCESS;
  if ((OB_UNLIKELY(index_scan_proj_exprs_.count() == 0))) {
    // do nothing
  } else if (OB_UNLIKELY(cur_idx_ == OB_INVALID_INDEX) ||
             OB_UNLIKELY(size + cur_idx_ > saved_size_) ||
             OB_ISNULL(store_rows_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected error for convert store rows", K(size), K(max_size_), K(store_rows_), K(ret));  
  } else {
    ObEvalCtx::BatchInfoScopeGuard batch_info_guard(*eval_ctx_);
    batch_info_guard.set_batch_size(size);
    for (int64_t i = 0; OB_SUCC(ret) && i < size; i++) {
      batch_info_guard.set_batch_idx(i);
      if (OB_FAIL(store_rows_[cur_idx_].store_row_->to_expr<true>(index_scan_proj_exprs_, *eval_ctx_))) {
        LOG_WARN("cache lookup iter failed to convert store row to expr", K(ret));
      } else {
        cur_idx_++;
      }
    }
  }
  return ret;
}

void ObDASCacheLookupIter::IndexProjRowStore::IndexProjRowStore::reuse()
{
  cur_idx_ = OB_INVALID_INDEX;
  saved_size_ = 0;
  iter_end_ = false;
}

void ObDASCacheLookupIter::IndexProjRowStore::IndexProjRowStore::reset()
{
  if (OB_NOT_NULL(store_rows_)) {
    for (int64_t i = 0; i < max_size_; i++) {
      store_rows_[i].ObChunkDatumStore::LastStoredRow::~LastStoredRow();
    }
    store_rows_ = nullptr;
  }
  index_scan_proj_exprs_.reset();
  eval_ctx_ = nullptr;
  max_size_ = 1;
  saved_size_ = 0;
  cur_idx_ = OB_INVALID_INDEX;
  iter_end_ = false;
}

int ObDASCacheLookupIter::inner_init(ObDASIterParam &param)
{
  int ret = OB_SUCCESS;
  ObDASCacheLookupIterParam &lookup_param = static_cast<ObDASCacheLookupIterParam&>(param);
  const int64_t default_batch_row_count = lookup_param.default_batch_row_count_;
  const int64_t simulate_batch_row_cnt = - EVENT_CALL(EventTable::EN_TABLE_LOOKUP_BATCH_ROW_COUNT);
  const bool use_simulate_batch_row_cnt = simulate_batch_row_cnt > 0 && simulate_batch_row_cnt < default_batch_row_count;

  lookup_param.default_batch_row_count_  = use_simulate_batch_row_cnt ? simulate_batch_row_cnt : default_batch_row_count;
  LOG_DEBUG("simulate lookup row batch count", K(simulate_batch_row_cnt), K(default_batch_row_count));

  if (OB_FAIL(ObDASLocalLookupIter::inner_init(param))) {
    LOG_WARN("failed to init das lookup iter", K(ret));
  } else if (lookup_param.index_scan_proj_exprs_.count() > 0 &&
             OB_FAIL(index_proj_rows_.init(store_allocator_,
                                           lookup_param.index_scan_proj_exprs_, 
                                           lookup_param.eval_ctx_,
                                           lookup_param.default_batch_row_count_))) {
    LOG_WARN("failed to init index proj rows", K(ret));
  }

  return ret;
}

int ObDASCacheLookupIter::inner_reuse()
{
  int ret = OB_SUCCESS;
  if (OB_FAIL(ObDASLocalLookupIter::inner_reuse())) {
    LOG_WARN("failed to reuse das lookup iter", K(ret));
  } else {
    index_proj_rows_.reuse();
  }
  return ret;
}

int ObDASCacheLookupIter::inner_release()
{
  int ret = OB_SUCCESS;
  if (OB_FAIL(ObDASLocalLookupIter::inner_release())) {
    LOG_WARN("failed to release das lookup iter", K(ret));
  } else {
    index_proj_rows_.reset();
    store_allocator_.reset();
  }
  return ret;
}

void ObDASCacheLookupIter::reset_lookup_state()
{
  ObDASLookupIter::reset_lookup_state();
  index_proj_rows_.reuse();
}

int ObDASCacheLookupIter::inner_get_next_row()
{
  int ret = OB_SUCCESS;
  bool got_next_row = false;
  const bool need_index_proj = index_proj_rows_.index_scan_proj_exprs_.count() > 0;
  OB_ASSERT(need_index_proj ? index_proj_rows_.max_size_ == default_batch_row_count_ : true);

  do {
    switch (state_) {
      case INDEX_SCAN: {
        reset_lookup_state();
        while (OB_SUCC(ret) && !index_end_ && lookup_rowkey_cnt_ < default_batch_row_count_) {
          index_table_iter_->clear_evaluated_flag();
          if (OB_FAIL(index_table_iter_->get_next_row())) {
            if (OB_UNLIKELY(OB_ITER_END != ret)) {
              LOG_WARN("failed to get next row from index table", K(ret));
            } else {
              index_end_ = true;
              ret = OB_SUCCESS;
            }
          } else if (OB_FAIL(add_rowkey())) {
            LOG_WARN("failed to add row key", K(ret));
          } else if (need_index_proj && OB_FAIL(index_proj_rows_.save(false, 1))) { // if need to project, save it
              LOG_WARN("save index proj rows failed", K(ret));
          } else {
            ++lookup_rowkey_cnt_;
          }
        }

        if (OB_SUCC(ret)) {
          if (OB_LIKELY(lookup_rowkey_cnt_ > 0)) {
            state_ = DO_LOOKUP;
          } else {
            state_ = FINISHED;
          }
        }
        break;
      }

      case DO_LOOKUP: {
        if (OB_FAIL(do_index_lookup())) {
          LOG_WARN("failed to do index lookup", K(ret));
        } else {
          state_ = OUTPUT_ROWS;
        }
        break;
      }

      case OUTPUT_ROWS: {
        data_table_iter_->clear_evaluated_flag();
        if (OB_FAIL(data_table_iter_->get_next_row())) {
          if (OB_LIKELY(OB_ITER_END == ret)) {
            ret = OB_SUCCESS;
            if (OB_FAIL(check_index_lookup())) {
              LOG_WARN("failed to check table lookup", K(ret));
            } else {
              state_ = INDEX_SCAN;
            }
          } else {
            LOG_WARN("failed to get next row from data table", K(ret));
          }
        } else {
          got_next_row = true;
          ++lookup_row_cnt_;
          if (got_next_row && index_proj_rows_.have_data() &&
              OB_FAIL(index_proj_rows_.to_expr(1))) {
            LOG_WARN("failed to convert store row to expr", K(ret));
          }
        }
        break;
      }

      case FINISHED: {
        ret = OB_ITER_END;
        break;
      }
      default: {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected lookup state", K_(state));
      }
    }
  } while (!got_next_row && OB_SUCC(ret));

  return ret;
}

int ObDASCacheLookupIter::inner_get_next_rows(int64_t &count, int64_t capacity)
{
  int ret = OB_SUCCESS;
  bool get_next_rows = false;
  const bool need_index_proj = index_proj_rows_.index_scan_proj_exprs_.count() > 0;
  adlog::DEBUG("ObDASCacheLookupIter::inner_get_next_rows -----begin");
  do {
    switch (state_) {
      case INDEX_SCAN: {
        {
          // static bool flag = true;
          // if (flag) {
            adlog::DEBUG("--------------INDEX_SCAN----------");
            // flag=false;
          // }
        }
        reset_lookup_state();
        int64_t storage_count = 0;
        int64_t index_capacity = 0;
        while (OB_SUCC(ret) && !index_end_ && lookup_rowkey_cnt_ < default_batch_row_count_) {
          storage_count = 0;
          index_capacity = std::min(capacity, std::min(max_size_, default_batch_row_count_ - lookup_rowkey_cnt_));
          index_table_iter_->clear_evaluated_flag();
          adlog::DEBUG("--------------INDEX_SCAN: index_table_iter_->get_next_rows-----------");

          // {
          //   // 在这里去做把data_table_iter的range先放到index_table_iter里面

          // //ObDASTRMergeIter
          // if (auto &&index_table_it =
          //         dynamic_cast<ObDASTRMergeIter *>(index_table_iter_)) {
          //   adlog::DEBUG(
          //       "index_table_iter -> index_table_it : ObDASTRMergeIter");
          //   if (auto &&data_table_it =
          //         dynamic_cast<ObDASScanIter *>(data_table_iter_)) {
          //     adlog::DEBUG("data_table_iter -> data_table_it : ObDASScanIter");
          //     // 代表找到了内层的index_it 和 data_it
          //     // 这里先获得到 index_it 的 底层的merge_iter
          //     storage::ObISparseRetrievalMergeIter *sparse_merge_iter =
          //         index_table_it->get_sparse_retrieval_iter();

          //     if (auto &&daat_iter =
          //             dynamic_cast<ObSRDaaTIterImpl *>(sparse_merge_iter)) {
          //       adlog::DEBUG("have got index_table_iter's sparse_merge_iter, daat_iter's impl");
          //       // 打算把id_less的内容挪到 ObSRDaaTIterImpl 里面去，这样reset和reuse可以继续
          //       // get_scan_param
          //       storage::ObTableScanParam &scan_param = data_table_it->get_scan_param();
          //       adlog::DEBUG("scan_param.key_ranges_.count(): ",scan_param.key_ranges_.count());
          //       for (int i = 0; i < scan_param.key_ranges_.count(); i++) {
          //         const ObNewRange &range = scan_param.key_ranges_.at(i);
                  
          //         // 获取起始和结束键值
          //         const ObRowkey &start_key = range.start_key_;
          //         const ObRowkey &end_key = range.end_key_;
                  

          //         // 获取键中的各个列值 (假设 id 是第0列)
          //             // 检查是否有对象且数量大于0
          //         if (start_key.get_obj_cnt() > 0 && end_key.get_obj_cnt() > 0) {
          //           // 获取对象指针
          //           const ObObj *start_obj_ptr = start_key.get_obj_ptr();
          //           const ObObj *end_obj_ptr = end_key.get_obj_ptr();
                    
          //           if (nullptr != start_obj_ptr && nullptr != end_obj_ptr) {
          //               // 获取第一列的值 (id列)
          //               const ObObj &start_id_obj = start_obj_ptr[0];
          //               const ObObj &end_id_obj = end_obj_ptr[0];
                        
          //               if (true) {
          //                 int64_t min_id = start_id_obj.get_int();
          //                 int64_t max_id = end_id_obj.get_int();
          //                 daat_iter->set_id_less(max_id + 1); // 应该是不用的，先暂时算上，反正不会错
          //                 // 现在你知道 id 的范围是 [min_id, max_id]
          //                 adlog::DEBUG("ID range: [", min_id,",  ",
          //                               max_id,"]");
          //               }
          //           }
          //         }

          //       }

          //       adlog::DEBUG("ID range have over");
          //     }
              
          //   }
          // }
          
          
          //   adlog::DEBUG("index or data table iter change end!");
          // }


          if (OB_FAIL(index_table_iter_->get_next_rows(storage_count, index_capacity))) {
            if (OB_UNLIKELY(OB_ITER_END != ret)) {
              LOG_WARN("failed to get next rows from index table", K(ret));
            } else {
              if (storage_count == 0) {
                index_end_ = true;
              }
              ret = OB_SUCCESS;
            }
          }
          if (OB_SUCC(ret) && storage_count > 0) {
            if (OB_FAIL(add_rowkeys(storage_count))) {
              LOG_WARN("failed to add row keys", K(ret));
            } else if (need_index_proj && OB_FAIL(index_proj_rows_.save(true, storage_count))) { // if need to project, save it
              LOG_WARN("save index proj rows failed", K(ret));
            } else {
              lookup_rowkey_cnt_ += storage_count;
            } 
          }
        }

        if (OB_SUCC(ret)) {
          if (OB_LIKELY(lookup_rowkey_cnt_ > 0)) {
            state_ = DO_LOOKUP;
          } else {
            state_ = FINISHED;
          }
        }
        break;
      }

      case DO_LOOKUP: {
        {
          // static bool flag = true;
          // if (flag) {
            adlog::DEBUG("--------------DO_LOOKUP----------");
            // flag=false;
          // }
        }

        // if()

        if (OB_FAIL(do_index_lookup())) {
          LOG_WARN("failed to do index lookup", K(ret));
        } else {
          state_ = OUTPUT_ROWS;
        }
        break;
      }

      case OUTPUT_ROWS: {
        adlog::DEBUG("--------------OUTPUT_ROWS----------");

        count = 0;
        data_table_iter_->clear_evaluated_flag();

        adlog::DEBUG("--------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------");

        if (OB_FAIL(data_table_iter_->get_next_rows(count, capacity))) {
          if (OB_LIKELY(OB_ITER_END == ret)) {
            ret = OB_SUCCESS;
            if (count > 0) {
              lookup_row_cnt_ += count;
              get_next_rows = true;
            } else {
              if (OB_FAIL(check_index_lookup())) {
                LOG_WARN("failed to check table lookup", K(ret));
              } else {
                state_ = INDEX_SCAN;
              }
            }
          } else {
            LOG_WARN("failed to get next rows from data table", K(ret));
          }
        } else {
          lookup_row_cnt_ += count;
          get_next_rows = true;
        }

        if (OB_FAIL(ret)) {
        } else if (get_next_rows && index_proj_rows_.have_data() &&
                   OB_FAIL(index_proj_rows_.to_expr(count))) {
          LOG_WARN("failed to convert store row to expr", K(ret));
        }
        break;
      }

      case FINISHED: {
        adlog::DEBUG("--------------FINISHED----------");

        ret = OB_ITER_END;
        break;
      }
    }
  } while (!get_next_rows && OB_SUCC(ret));
  adlog::DEBUG("ObDASCacheLookupIter::inner_get_next_rows -----end");

  return ret;
}

}  // namespace sql
}  // namespace oceanbase
