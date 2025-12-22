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
#define USING_LOG_PREFIX STORAGE

#include "ob_text_retrieval_token_iter.h"
#include "sql/engine/expr/ob_expr_bm25.h"
#include "sql/das/iter/sparse_retrieval/ob_das_tr_merge_iter.h"
#include "common/ad_log.h"
namespace oceanbase
{
namespace storage
{
ObTextRetrievalTokenIter::ObTextRetrievalTokenIter()
  : ObISparseRetrievalDimIter(),
    mem_context_(nullptr),
    allocator_(nullptr),
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
    doc_token_cnt_expr_(nullptr),
    relevance_calc_exprs_(),
    skip_(nullptr),
    fwd_range_objs_(nullptr),
    max_batch_size_(0),
    token_doc_cnt_(0),
    max_token_relevance_(-1.0),
    advance_doc_id_(),
    token_doc_cnt_calculated_(false),
    inv_idx_agg_cache_mode_(false),
    is_inited_(false)
{
}

int ObTextRetrievalTokenIter::init(const ObTextRetrievalScanIterParam &iter_param)
{
  int ret = OB_SUCCESS;
  allocator_ = iter_param.allocator_;
  mem_context_ = iter_param.mem_context_;
  inv_idx_scan_param_ = iter_param.inv_idx_scan_param_;
  inv_idx_agg_param_ = iter_param.inv_idx_agg_param_;
  fwd_idx_scan_param_ = iter_param.fwd_idx_scan_param_;
  inv_idx_scan_iter_ = iter_param.inv_idx_scan_iter_;
  inv_idx_agg_iter_ = iter_param.inv_idx_agg_iter_;
  inv_idx_agg_expr_ = iter_param.inv_idx_agg_expr_;
  fwd_idx_agg_iter_ = iter_param.fwd_idx_agg_iter_;
  fwd_idx_agg_expr_ = iter_param.fwd_idx_agg_expr_;
  eval_ctx_ = iter_param.eval_ctx_;
  relevance_expr_ = iter_param.relevance_expr_;
  inv_scan_doc_length_col_ = iter_param.inv_scan_doc_length_col_;
  inv_scan_domain_id_col_ = iter_param.inv_scan_domain_id_col_;
  max_batch_size_ = OB_MAX(iter_param.eval_ctx_->max_batch_size_, 1);
  inv_idx_agg_cache_mode_ = iter_param.inv_idx_agg_cache_mode_;

  if (OB_ISNULL(inv_idx_scan_iter_) || OB_ISNULL(eval_ctx_) || OB_ISNULL(inv_idx_scan_param_) ||
      OB_ISNULL(inv_scan_domain_id_col_) || OB_ISNULL(allocator_) || OB_ISNULL(mem_context_.ref_context())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("inv_idx_scan_iter or eval_ctx is NULL", K(ret), K_(inv_idx_scan_iter), KPC_(eval_ctx), KPC_(inv_idx_scan_param), KP(allocator_), KP(mem_context_.ref_context()));
  } else if (!need_calc_relevance()) {
  } else if (OB_ISNULL(relevance_expr_) || OB_ISNULL(inv_idx_agg_expr_) || OB_ISNULL(inv_idx_agg_iter_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected null relevance expr", K(ret), KPC_(relevance_expr), KPC_(inv_idx_agg_expr), KPC_(inv_idx_agg_iter));
  } else if (!need_fwd_idx_agg() && (OB_ISNULL(inv_scan_doc_length_col_))) {
  } else if (need_fwd_idx_agg() && (OB_ISNULL(fwd_idx_agg_iter_) || OB_ISNULL(fwd_idx_agg_expr_) || OB_ISNULL(fwd_idx_scan_param_))) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("fwd_idx_agg_iter or fwd_idx_agg_expr is NULL", K(ret), K_(fwd_idx_agg_iter), KPC_(fwd_idx_agg_expr), KPC_(fwd_idx_scan_param));
  } else if (OB_FAIL(init_calc_exprs_in_relevance_expr())) {
    LOG_WARN("failed to init row-wise calc exprs", K(ret));
  } else if (OB_ISNULL(doc_token_cnt_expr_) || OB_UNLIKELY(doc_token_cnt_expr_->datum_meta_.get_type() != ObDecimalIntType && doc_token_cnt_expr_->datum_meta_.get_type() != ObNumberType)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected null expr", K(ret), KPC_(doc_token_cnt_expr), KPC_(inv_scan_doc_length_col), KPC_(eval_ctx));
  } else if (OB_ISNULL(skip_ = to_bit_vector(allocator_->alloc(ObBitVector::memory_size(max_batch_size_))))) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    LOG_WARN("failed to allocate skip bit vector", K(ret));
  } else {
    skip_->init(max_batch_size_);
  }
  is_inited_ = true;
  tem_param_ = iter_param;
  return ret;
}

void ObTextRetrievalTokenIter::reset()
{
  relevance_calc_exprs_.reset();
  token_doc_cnt_calculated_ = false;
  cache_read_idx_ = 0;            
  last_read_count_ = 0;
}

void ObTextRetrievalTokenIter::reuse()
{
  if (inv_idx_agg_cache_mode_ && !inv_idx_agg_param_->need_switch_param_) {
    // do nothing
  } else {
    token_doc_cnt_calculated_ = false;
  }
  cache_read_idx_ = 0;            
  last_read_count_ = 0;
}

int ObTextRetrievalTokenIter::init_calc_exprs_in_relevance_expr()
{
  int ret = OB_SUCCESS;
  if (need_calc_relevance()) {
    if (OB_ISNULL(relevance_expr_)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected null relevance expr", K(ret));
    } else if (OB_FAIL(relevance_calc_exprs_.push_back(relevance_expr_))) {
      LOG_WARN("failed to append relevance expr", K(ret));
    }
    for (int64_t i = 0; OB_SUCC(ret) && i < relevance_expr_->arg_cnt_; ++i) {
      sql::ObExpr *arg_expr = relevance_expr_->args_[i];
      if (OB_ISNULL(arg_expr)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected null arg expr", K(ret));
      } else if (T_FUN_SYS_CAST == arg_expr->type_) {
        // cast expr is evaluated with relevance expr
        if (OB_FAIL(relevance_calc_exprs_.push_back(arg_expr))) {
          LOG_WARN("failed to append cast expr", K(ret));
        }
      }
    }

    if (OB_SUCC(ret)) {
      sql::ObExpr *doc_token_cnt_param_expr = relevance_expr_->args_[sql::ObExprBM25::DOC_TOKEN_CNT_PARAM_IDX];
      if (T_FUN_SYS_CAST == doc_token_cnt_param_expr->type_) {
        doc_token_cnt_param_expr = doc_token_cnt_param_expr->args_[0];
      }
      if (OB_UNLIKELY(doc_token_cnt_param_expr->type_ != T_FUN_SUM)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected doc token cnt expr type", K(ret), KPC(doc_token_cnt_param_expr));
      } else {
        doc_token_cnt_expr_ = doc_token_cnt_param_expr;
        // update the locate datums
        if (max_batch_size_ > 0) {
          doc_token_cnt_expr_->locate_datums_for_update(*eval_ctx_, max_batch_size_);
        }
      }
    }
  }
  return ret;
}

int ObTextRetrievalTokenIter::get_token_doc_cnt(int64_t &token_doc_cnt) const
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(!token_doc_cnt_calculated_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get token doc cnt before inv_idx_agg evaluated", K(ret));
  } else {
    token_doc_cnt = token_doc_cnt_;
  }
  return ret;
}

void ObTextRetrievalTokenIter::clear_row_wise_evaluated_flag()
{
  for (int64_t i = 0; i < relevance_calc_exprs_.count(); ++i) {
    sql::ObExpr *expr = relevance_calc_exprs_.at(i);
    expr->clear_evaluated_flag(*eval_ctx_);
  }
}

int ObTextRetrievalTokenIter::get_next_doc_token_cnt(const bool use_fwd_idx_agg)
{
  int ret = OB_SUCCESS;
  if (use_fwd_idx_agg) {
    sql::ObDocIdExt cur_doc_id;
    if (OB_FAIL(get_inv_idx_scan_doc_id(cur_doc_id))) {
      LOG_WARN("failed to get current doc id", K(ret));
    } else if (OB_FAIL(do_token_cnt_agg(cur_doc_id))) {
      LOG_WARN("failed to do token count agg on fwd index", K(ret));
    }
  } else {
    if (OB_FAIL(fill_token_cnt_with_doc_len())) {
      LOG_WARN("failed to fill token cnt with document length", K(ret));
    }
  }
  return ret;
}

int ObTextRetrievalTokenIter::get_inv_idx_scan_doc_id(ObDocIdExt &doc_id)
{
  int ret = OB_SUCCESS;
  ObDatum &doc_id_datum = inv_scan_domain_id_col_->locate_expr_datum(*eval_ctx_);
  if (OB_FAIL(doc_id.from_datum(doc_id_datum))) {
    LOG_WARN("failed to get doc id", K(ret), K(doc_id_datum));
  }

  return ret;
}

int ObTextRetrievalTokenIter::gen_fwd_idx_scan_range(const ObDocIdExt &doc_id, ObNewRange &scan_range)
{
  int ret = OB_SUCCESS;
  if (nullptr == fwd_range_objs_) {
    common::ObArenaAllocator &ctx_alloc = mem_context_->get_arena_allocator();
    void *buf = nullptr;
    constexpr int64_t obj_cnt = FWD_IDX_ROWKEY_COL_CNT * 2;
    if (OB_ISNULL(buf = ctx_alloc.alloc(sizeof(ObObj) * obj_cnt))) {
      ret = OB_ALLOCATE_MEMORY_FAILED;
      LOG_WARN("failed to allocate memory for rowkey obj", K(ret));
    } else if (OB_ISNULL(fwd_range_objs_ = new (buf) ObObj[obj_cnt])) {
      ret = OB_ALLOCATE_MEMORY_FAILED;
      LOG_WARN("allocate memory failed", K(ret));
    }
  }
  if (OB_FAIL(ret)) {
  } else if (OB_FAIL(doc_id.get_datum().to_obj(fwd_range_objs_[0], inv_scan_domain_id_col_->obj_meta_))) {
    LOG_WARN("failed to set obj", K(ret));
  } else if (OB_FAIL(doc_id.get_datum().to_obj(fwd_range_objs_[2], inv_scan_domain_id_col_->obj_meta_))) {
    LOG_WARN("failed to set obj", K(ret));
  } else if (OB_UNLIKELY(fwd_idx_scan_param_->key_ranges_.count() != 1)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected key range count", K(ret), K(fwd_idx_scan_param_->key_ranges_.count()));
  } else {
    fwd_range_objs_[1].set_min_value();
    fwd_range_objs_[3].set_max_value();
    scan_range.table_id_ = fwd_idx_scan_param_->key_ranges_.at(0).table_id_;
    scan_range.start_key_.assign(fwd_range_objs_, FWD_IDX_ROWKEY_COL_CNT);
    scan_range.end_key_.assign(&fwd_range_objs_[2], FWD_IDX_ROWKEY_COL_CNT);
    scan_range.border_flag_.set_inclusive_start();
    scan_range.border_flag_.set_inclusive_end();
  }
  return ret;
}

int ObTextRetrievalTokenIter::do_token_cnt_agg(const ObDocIdExt &doc_id)
{
  int ret = OB_SUCCESS;
  int64_t token_count = 0;
  ObNewRange scan_range;
  if (OB_FAIL(gen_fwd_idx_scan_range(doc_id, scan_range))) {
    LOG_WARN("failed to generate forward index scan range", K(ret));
  } else if (OB_FAIL(fwd_idx_agg_iter_->reuse())) {
    LOG_WARN("failed to reuse forward index iter", K(ret));
  } else if (FALSE_IT(fwd_idx_scan_param_->key_ranges_.reuse())) {
    LOG_WARN("failed to reuse forward index scan range", K(ret));
  } else if (OB_FAIL(fwd_idx_scan_param_->key_ranges_.push_back(scan_range))){
    LOG_WARN("failed to add forward index scan range", K(ret), K(scan_range));
  } else if (OB_FAIL(fwd_idx_agg_iter_->rescan())) {
    LOG_WARN("failed to rescan forward index", K(ret));
  } else if (OB_FAIL(fwd_idx_agg_iter_->get_next_row())) {
    LOG_WARN("failed to get next row from forward index iterator", K(ret));
  } else {
    if (fwd_idx_agg_expr_->enable_rich_format()
        && is_valid_format(fwd_idx_agg_expr_->get_format(*eval_ctx_))) {
      token_count = fwd_idx_agg_expr_->get_vector(*eval_ctx_)->get_int(0);
    } else {
      token_count = fwd_idx_agg_expr_->locate_expr_datum(*eval_ctx_).get_int();
    }
    LOG_DEBUG("retrieval iterator get token cnt for doc", K(ret), K(doc_id), K(token_count));
  }
  return ret;
}

int ObTextRetrievalTokenIter::fill_token_cnt_with_doc_len()
{
  int ret = OB_SUCCESS;
  const sql::ObExpr *agg_expr = doc_token_cnt_expr_;
  const sql::ObExpr *doc_length_expr = inv_scan_doc_length_col_;
  ObDatum *doc_length_datum = nullptr;
  if (OB_ISNULL(agg_expr) || OB_ISNULL(doc_length_expr) || OB_ISNULL(eval_ctx_)
      || OB_UNLIKELY(agg_expr->datum_meta_.get_type() != ObDecimalIntType && agg_expr->datum_meta_.get_type() != ObNumberType)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected null expr", K(ret), KPC(agg_expr), KP(doc_length_expr), KP(eval_ctx_));
  } else if (OB_FAIL(doc_length_expr->eval(*eval_ctx_, doc_length_datum))) {
    LOG_WARN("failed to evaluate document length expr", K(ret));
  } else {
    ObDatum &agg_datum = agg_expr->locate_datum_for_write(*eval_ctx_);
    if (agg_expr->datum_meta_.get_type() == ObDecimalIntType) {
      if(OB_FAIL(set_decimal_int_by_precision(agg_datum, doc_length_datum->get_uint(), agg_expr->datum_meta_.precision_))) {
        LOG_WARN("fail to set decimal int", K(ret));
      }
    } else {
      const int64_t in_val = doc_length_datum->get_uint64();
      number::ObNumber nmb;
      if (OB_FAIL(nmb.from(in_val, mem_context_->get_arena_allocator()))) {
        LOG_WARN("fail to int_number", K(ret), K(in_val));
      } else {
        agg_datum.set_number(nmb);
      }
    }

  }
  return ret;
}

int ObTextRetrievalTokenIter::fill_token_doc_cnt()
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(inv_idx_agg_expr_->datum_meta_.get_type() != ObIntType)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected null expr", K(ret), KP_(inv_idx_agg_expr), KP_(eval_ctx));
  } else {
    ObEvalCtx::BatchInfoScopeGuard guard(*eval_ctx_);
    guard.set_batch_idx(0);
    ObDatum &doc_cnt_datum = inv_idx_agg_expr_->locate_datum_for_write(*eval_ctx_);
    doc_cnt_datum.set_int(token_doc_cnt_);
  }
  return ret;
}

int ObTextRetrievalTokenIter::eval_relevance_expr()
{
  int ret = OB_SUCCESS;
  ObDatum *relevance_datum = nullptr;
  if (OB_FAIL(relevance_expr_->eval(*eval_ctx_, relevance_datum))) {
    LOG_WARN("failed to evaluate relevance", K(ret));
  }
  return ret;
}

int ObTextRetrievalTokenIter::get_next_row()
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("retrieval token iterator not inited", K(ret));
  } else if (!token_doc_cnt_calculated_ && OB_FAIL(estimate_token_doc_cnt())) {
    LOG_WARN("failed to estimate token doc cnt", K(ret));
  } else if (OB_FAIL(inv_idx_scan_iter_->get_next_row())) {
    if (OB_UNLIKELY(OB_ITER_END != ret)) {
      LOG_WARN("failed to get next row from inverted index", K(ret), K_(inv_idx_scan_param), KPC_(inv_idx_scan_iter));
    }
  } else {
    if (need_calc_relevance()) {
      clear_row_wise_evaluated_flag();
      if (OB_FAIL(get_next_doc_token_cnt(need_fwd_idx_agg()))) {
        LOG_WARN("failed to get next doc token count", K(ret));
      } else if (OB_FAIL(fill_token_doc_cnt())) {
        LOG_WARN("failed to get token doc cnt", K(ret));
      } else if (OB_FAIL(eval_relevance_expr())) {
        LOG_WARN("failed to evaluate simarity expr", K(ret));
      }
    }
  }
  return ret;
}

void ObTextRetrievalTokenIter::clear_batch_wise_evaluated_flag(const int64_t count)
{
  int64_t max_size = OB_MAX(max_batch_size_, count);
  for (int64_t i = 0; i < relevance_calc_exprs_.count(); ++i) {
    sql::ObExpr *expr = relevance_calc_exprs_.at(i);
    expr->get_evaluated_flags(*eval_ctx_).reset(count);
  }
}

int ObTextRetrievalTokenIter::batch_fill_token_cnt_with_doc_len(const int64_t count)
{
  int ret = OB_SUCCESS;
  const sql::ObExpr *agg_expr = doc_token_cnt_expr_;
  const sql::ObExpr *doc_length_expr = inv_scan_doc_length_col_;
  if (need_fwd_idx_agg()) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unsupported fwd_idx_agg", K(ret));
  } else if (OB_FAIL(doc_length_expr->eval_batch(*eval_ctx_, *skip_, count))) {
    LOG_WARN("failed to evaluate document length expr", K(ret));
  } else if (OB_UNLIKELY(!doc_length_expr->is_batch_result())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected no batch expr", K(ret), KP(doc_length_expr));
  } else {
    const ObDatum *datums = doc_length_expr->locate_batch_datums(*eval_ctx_);
    ObDatum *agg_datum = agg_expr->locate_batch_datums(*eval_ctx_);
    for (int64_t i = 0; OB_SUCC(ret) && i < count; ++i) {
      if (OB_LIKELY(!skip_->at(i))) {
        if (agg_expr->datum_meta_.get_type() == ObDecimalIntType) {
          if (OB_FAIL(set_decimal_int_by_precision(agg_datum[i], datums[i].get_uint(), agg_expr->datum_meta_.precision_))) {
            LOG_WARN("fail to set decimal int", K(ret));
          }
        } else {
          const int64_t in_val = datums[i].get_uint64();
          number::ObNumber nmb;
          if (OB_FAIL(nmb.from(in_val, mem_context_->get_arena_allocator()))) {
            LOG_WARN("fail to int_number", K(ret), K(in_val));
          } else {
            agg_datum[i].set_number(nmb);
          }
        }
      }
    }
  }
  return ret;
}

int ObTextRetrievalTokenIter::batch_eval_relevance_expr(const int64_t count)
{
  int ret = OB_SUCCESS;
  if (OB_FAIL(relevance_expr_->eval_batch(*eval_ctx_, *skip_, count))) {
    LOG_WARN("failed to evaluate relevance", K(ret));
  }
  return ret;
}

int ObTextRetrievalTokenIter::get_next_batch_with_cache(const int64_t capacity,
                                                        int64_t &count) {
  int ret = OB_SUCCESS;  
  count = 0;
  // TokenCacheHandle handle;  // 局部变量
  // if (!own_cache_data_) {
  //   auto && cache_data = cache_mgr.get_cache_data(token_name_);
  //   if (cache_data) {
  //     own_cache_data_ = cache_data;
  //   }
  // }
  
  int64_t available_count = own_cache_data_->total_count_ - (cache_read_idx_ + last_read_count_ - 1 + 1);
  adlog::DEBUG("ObTextRetrievalTokenIter::get_next_batch_with_cache start");
  adlog::DEBUG("available_count: ", available_count);
  
  bool have_to_more_data_flag = false;

  if (available_count < capacity and (!own_cache_data_->is_completed_)) {
    ret = load_more_data_to_cache(capacity - available_count);
    if (ret == OB_ITER_END) {
      own_cache_data_->is_completed_ = true;
      adlog::DEBUG("map里的缓存迭代器走到头了");
      ret = OB_SUCCESS;
    }
  }
  available_count = own_cache_data_->total_count_ - (cache_read_idx_ + last_read_count_ - 1 + 1);
  int64_t read_count = std::min(available_count, capacity);  
  adlog::DEBUG("token缓存里面： cache_read_idx_: ", cache_read_idx_," read_count: ",read_count, " total: ",own_cache_data_->total_count_);
    
  if (read_count <= 0) {  
    ret = OB_ITER_END;  
  } else {  
    count = read_count;
    cache_read_idx_ += last_read_count_;
    last_read_count_ = read_count;
  }


  return ret;
}

int ObTextRetrievalTokenIter::load_more_data_to_cache(int64_t need_data_cnt) {
  // 获取缓存管理器实例
  adlog::DEBUG("开始尝试让缓存追加数据");
  int ret = OB_SUCCESS;
  // TokenCacheHandle handle;  // 局部变量 
  // TokenCacheManager &cache_mgr = TokenCacheManager::get_instance();  
    
  // 获取存储的原始迭代器  
  ObTextRetrievalTokenIter *origin_iter = handle_.get_origin_iter();  
  if (OB_ISNULL(origin_iter)) {  
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("failed to get origin iter from cache manager", K(ret),
             K(token_name_));
    adlog::DEBUG("怎么会 origin_iter 会 空指针");
  } else {  
    // 获取或创建缓存数据  
    TokenCacheData *cache_data = handle_.get_cache_data();  
    if (OB_ISNULL(cache_data)) {

      adlog::DEBUG("ERROR! 竟然没有cache_data");
    }  
      
    // 从原始迭代器扫描数据并添加到缓存  
    int64_t loaded_count = 0;  
    while (OB_SUCC(ret) && loaded_count < need_data_cnt) {  
      int64_t batch_count = 0;  
      // int64_t capacity = std::min(need_data_cnt - loaded_count, origin_iter->max_batch_size_);  
        
      if (OB_FAIL(origin_iter->get_next_batch(origin_iter->max_batch_size_, batch_count))) {  
        if (OB_UNLIKELY(OB_ITER_END != ret)) {  
          LOG_WARN("failed to get next batch from origin iter", K(ret));  
        } else {
          // ret = OB_SUCCESS;
          adlog::DEBUG("map里的 缓存迭代器 到的末尾");
          break; // 到达末尾，正常结束  
        }  
      } else if (batch_count > 0) {  
        // 提取数据并添加到缓存  
        if (OB_FAIL(extract_and_add_data_to_cache(*origin_iter, *cache_data, batch_count))) {  
          LOG_WARN("failed to extract and add data to cache", K(ret));  
        } else {  
          loaded_count += batch_count;  
        }  
      }  
    }  
      
    if (OB_SUCC(ret)) {  
      LOG_DEBUG("successfully loaded data to cache", K(loaded_count), K(token_name_));  
      adlog::DEBUG("应该是追加成功了 loaded_count: ", loaded_count, "  token_name_: ", token_name_);
    }  
  }  
    
  return ret;  
}

int ObTextRetrievalTokenIter::extract_and_add_data_to_cache(  
  ObTextRetrievalTokenIter &iter,   
  TokenCacheData &cache_data,   
  int64_t batch_count) {  
  int ret = OB_SUCCESS;  
    
  // 获取相关性和文档ID表达式  
  ObExpr *relevance_expr = iter.relevance_expr_;  
  ObExpr *doc_id_expr = iter.inv_scan_domain_id_col_;  
    
  if (OB_ISNULL(relevance_expr) || OB_ISNULL(doc_id_expr)) {  
    ret = OB_INVALID_ARGUMENT;  
    LOG_WARN("invalid expr", K(ret), KP(relevance_expr), KP(doc_id_expr));  
  } else {  
    // 评估表达式获取数据  
    if (OB_FAIL(relevance_expr->eval_batch(*iter.eval_ctx_, *iter.get_skip(), batch_count))) {  
      LOG_WARN("failed to evaluate relevance expr", K(ret));  
    } else {  
      const ObDatumVector &relevance_datum = relevance_expr->locate_expr_datumvector(*iter.eval_ctx_);  
      const ObDatumVector &doc_id_datum = doc_id_expr->locate_expr_datumvector(*iter.eval_ctx_);  
        
      // 逐个添加到缓存  
      for (int64_t i = 0; OB_SUCC(ret) && i < batch_count; ++i) {  
        if (OB_LIKELY(!iter.get_skip()->at(i))) {  
          double relevance = relevance_datum.at(i)->get_double();  
          ObDocIdExt doc_id;  
          if (OB_FAIL(doc_id.from_datum(*doc_id_datum.at(i)))) {  
            LOG_WARN("failed to get doc id", K(ret));  
          } else if (OB_FAIL(cache_data.add_data(relevance, doc_id, 1))) {  
            LOG_WARN("failed to add data to cache", K(ret));  
          }  
        }  
      }  
    }  
  }  
    
  return ret;  
}


//ADVISE


int ObTextRetrievalTokenIter::init_with_cache() {
  adlog::DEBUG("初始化缓存 开始");
  int ret = OB_SUCCESS;

  TCManager::get_instance().get_cache_data(token_name_, handle_);
  
  auto &&cache_data = handle_.get_cache_data();

  if (cache_data == nullptr) {
    adlog::DEBUG("之前没有过 缓存迭代器");
    // TokenCacheData *new_cache_data = nullptr;  
    if (OB_FAIL(TCManager::get_instance().create_cache_data(token_name_, handle_))) {  
      return ret;
    }
    own_cache_data_ = handle_.get_cache_data();
    // new一个新的token_iter，用于推进缓存
    TCManager::get_instance().create_token_iter(token_name_, tem_param_, handle_);
    // auto && origin_iter = TCManager::get_instance().get_origin_iter(token_name_);
    auto &&origin_iter = handle_.get_origin_iter();
    if (origin_iter) { 
      origin_iter->first_init_flag_ = true;
      origin_iter->use_cache_ = false;
    }
  } else {
    adlog::DEBUG("之前存在 缓存迭代器");
    own_cache_data_ = cache_data;
  }
  cache_read_idx_ = 0;
  last_read_count_ = 0;
  if(own_cache_data_ and handle_.get_origin_iter()){
    use_cache_ = true;
  } else {
    use_cache_ = false;
  }

  adlog::DEBUG("初始化缓存 结束");
  return ret;
}

//advise
int ObTextRetrievalTokenIter::get_next_batch(const int64_t capacity,
                                             int64_t &count) {
  adlog::DEBUG("token 的 get_next_batch 开始 : ",K(capacity));

  if (!first_init_flag_) {
    init_with_cache();
    first_init_flag_ = true;
  }


  if(use_cache_){
    // 判断一下在map里面存的是不是自己
    // auto &&iter = TCManager::get_instance().get_origin_iter(token_name_);
    // if (iter and iter != this) {
      return get_next_batch_with_cache(capacity, count);
    // }
  }

  // 下面已经是map里面存放的初代迭代器才会走的路径了
  int ret = OB_SUCCESS;
  count = 0;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("retrieval token iterator not inited", K(ret));
  } else if (!token_doc_cnt_calculated_ && OB_FAIL(estimate_token_doc_cnt())) {
    LOG_WARN("failed to estimate token doc cnt", K(ret));
  } else if (OB_FAIL(inv_idx_scan_iter_->get_next_rows(count, OB_MIN(max_batch_size_, capacity)))) {
    if (OB_UNLIKELY(OB_ITER_END != ret)) {
      LOG_WARN("failed to get next rows from inverted index", K(ret), KPC_(inv_idx_scan_param), KPC_(inv_idx_scan_iter));
    } else if (count != 0) {
      ret = OB_SUCCESS;
    }
  }

  if (OB_FAIL(ret)) {
  } else if (need_calc_relevance()) {
    const ObBitVector *skip = NULL;
    PRINT_VECTORIZED_ROWS(SQL, DEBUG, *eval_ctx_, *inv_idx_scan_param_->output_exprs_, count, skip);
    clear_batch_wise_evaluated_flag(count);
    if (OB_FAIL(batch_fill_token_cnt_with_doc_len(count))) {
      LOG_WARN("failed to fill batch token cnt with document length", K(ret));
    } else if (OB_FAIL(fill_token_doc_cnt())) {
      LOG_WARN("failed to get token doc cnt", K(ret));
    } else if (OB_FAIL(batch_eval_relevance_expr(count))) {
      LOG_WARN("failed to evaluate simarity expr", K(ret));
    }
  }
  return ret;
}

int ObTextRetrievalTokenIter::get_cache_data(int64_t idx, double &relevance, ObDocIdExt &doc_id) {  
  int ret = OB_SUCCESS;  
  if (use_cache_ and own_cache_data_ and (cache_read_idx_ + idx) < own_cache_data_->total_count_) {  
    relevance = own_cache_data_->relevance_scores_.at(cache_read_idx_ + idx);  
    doc_id = own_cache_data_->doc_ids_.at(cache_read_idx_ + idx);  
  } else {  
    ret = OB_ARRAY_OUT_OF_RANGE;  
  }  
  return ret;  
}

int ObTextRetrievalTokenIter::advance_to(const ObDatum &id_datum)
{
  int ret = OB_SUCCESS;
  if (OB_FAIL(advance_doc_id_.from_datum(id_datum))) {
    LOG_WARN("failed to get doc id", K(ret), K(id_datum));
  } else if (OB_UNLIKELY(inv_idx_scan_param_->key_ranges_.count() != 1)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected key range count", K(ret), K(inv_idx_scan_param_->key_ranges_.count()));
  } else {
    ObRowkey start_rowkey = inv_idx_scan_param_->key_ranges_.at(0).start_key_;
    ObRowkey end_rowkey = inv_idx_scan_param_->key_ranges_.at(0).end_key_;
    if (&start_rowkey.get_obj_ptr()[2] != &end_rowkey.get_obj_ptr()[0]) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected rowkey", K(ret), K(start_rowkey), K(end_rowkey));
    }
    // obj_ptr[0] is token, obj_ptr[1] is docid
    ObObj *obj_ptr = start_rowkey.get_obj_ptr();
    ObNewRange scan_range;
    if (OB_FAIL(ret)) {
    } else if (OB_FAIL(advance_doc_id_.get_datum().to_obj(obj_ptr[1], inv_scan_domain_id_col_->obj_meta_))) {
      LOG_WARN("failed to set obj", K(ret));
    } else {
      scan_range.table_id_ = inv_idx_scan_param_->key_ranges_.at(0).table_id_;
      scan_range.start_key_.assign(obj_ptr, INV_IDX_ROWKEY_COL_CNT);
      scan_range.end_key_.assign(&obj_ptr[2], INV_IDX_ROWKEY_COL_CNT);
      scan_range.border_flag_.set_inclusive_start();
      scan_range.border_flag_.set_inclusive_end();
    }
    if (OB_FAIL(ret)) {
    } else if (OB_FAIL(inv_idx_scan_iter_->reuse())) {
      LOG_WARN("failed to reuse inverted index scan iterator", K(ret));
    } else if (OB_UNLIKELY(!inv_idx_scan_param_->key_ranges_.empty())) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected non-empty scan range", K(ret), K(inv_idx_scan_param_->key_ranges_));
    } else if (OB_FAIL(inv_idx_scan_param_->key_ranges_.push_back(scan_range))) {
      LOG_WARN("failed to push back scan range", K(ret));
    } else if (OB_FAIL(inv_idx_scan_iter_->rescan())) {
      LOG_WARN("failed to rescan inverted index", K(ret));
    }
  }
  return ret;
}

int ObTextRetrievalTokenIter::update_scan_param(const ObString &token, common::ObArenaAllocator &allocator)
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(inv_idx_agg_param_->key_ranges_.count() != 1)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected key range count", K(ret), K(inv_idx_agg_param_->key_ranges_.count()));
  } else {
    ObNewRange scan_range = inv_idx_agg_param_->key_ranges_.at(0);
    ObObj tmp_obj;
    tmp_obj.set_string(ObVarcharType, token);
    tmp_obj.set_meta_type(scan_range.start_key_.get_obj_ptr()->meta_);
    if (scan_range.start_key_.get_obj_ptr() + 2 != scan_range.end_key_.get_obj_ptr()) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected rowkey", K(ret), K_(scan_range.start_key), K_(scan_range.end_key));
    } else if (OB_FAIL(ob_write_obj(allocator, tmp_obj, *scan_range.start_key_.get_obj_ptr()))) {
      LOG_WARN("failed to write obj", K(ret));
    } else if (OB_FAIL(ob_write_obj(allocator, tmp_obj, *scan_range.end_key_.get_obj_ptr()))) {
      LOG_WARN("failed to write obj", K(ret));
    } else if (!need_inv_idx_agg()) {
      // skip inverted index aggregate iterator
    } else if (OB_FAIL(inv_idx_agg_iter_->reuse())) {
      LOG_WARN("failed to reuse inverted index aggregate iterator", K(ret));
    } else if (OB_UNLIKELY(!inv_idx_agg_param_->key_ranges_.empty())) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected non-empty scan range", K(ret));
    } else if (OB_FAIL(inv_idx_agg_param_->key_ranges_.push_back(scan_range))) {
      LOG_WARN("failed to push back scan range", K(ret));
    } else if (OB_FAIL(inv_idx_agg_iter_->rescan())) {
      LOG_WARN("failed to rescan inverted aggregate index", K(ret));
    }

    if (OB_FAIL(ret)) {
    } else if (OB_UNLIKELY(inv_idx_scan_param_->key_ranges_.empty())) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected empty key ranges", K(ret));
    } else if (inv_idx_scan_param_->key_ranges_.at(0).start_key_.get_obj_ptr()
        == inv_idx_scan_param_->key_ranges_.at(0).end_key_.get_obj_ptr()) {
      // function lookup mode
      ObSEArray<ObNewRange, 4> scan_ranges;
      scan_ranges.assign(inv_idx_scan_param_->key_ranges_);
      for (int64_t i = 0; OB_SUCC(ret) && i < scan_ranges.count(); ++i) {
        if (OB_FAIL(ob_write_obj(allocator, tmp_obj, *scan_ranges.at(i).start_key_.get_obj_ptr()))) {
          LOG_WARN("failed to write obj", K(ret));
        }
      }
      if (FAILEDx(inv_idx_scan_iter_->reuse())) {
        LOG_WARN("failed to reuse inverted index scan iterator", K(ret));
      } else if (OB_UNLIKELY(!inv_idx_scan_param_->key_ranges_.empty())) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected non-empty scan range", K(ret));
      } else if (OB_FAIL(inv_idx_scan_param_->key_ranges_.assign(scan_ranges))) {
        LOG_WARN("failed to push back scan range", K(ret));
      } else if (OB_FAIL(inv_idx_scan_iter_->rescan())) {
        LOG_WARN("failed to rescan inverted index", K(ret));
      }
    } else {
      // non-function lookup mode
      if (OB_FAIL(inv_idx_scan_iter_->reuse())) {
        LOG_WARN("failed to reuse inverted index scan iterator", K(ret));
      } else if (OB_UNLIKELY(!inv_idx_scan_param_->key_ranges_.empty())) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected non-empty scan range", K(ret));
      } else if (OB_FAIL(inv_idx_scan_param_->key_ranges_.push_back(scan_range))) {
        LOG_WARN("failed to push back scan range", K(ret));
      } else if (OB_FAIL(inv_idx_scan_iter_->rescan())) {
        LOG_WARN("failed to rescan inverted index", K(ret));
      }
    }
    if (OB_SUCC(ret)) {
      reuse();
    }
  }
  return ret;
}

int ObTextRetrievalTokenIter::set_decimal_int_by_precision(ObDatum &result_datum,
                                                           const uint64_t decint,
                                                           const ObPrecision precision)
{
  int ret = OB_SUCCESS;
  if (precision <= MAX_PRECISION_DECIMAL_INT_64) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected precision, precision is too short", K(ret), K(precision));
  } else if (precision <= MAX_PRECISION_DECIMAL_INT_128) {
    const int128_t result = decint;
    result_datum.set_decimal_int(result);
  } else if (precision <= MAX_PRECISION_DECIMAL_INT_256) {
    const int256_t result = decint;
    result_datum.set_decimal_int(result);
  } else {
    const int512_t result = decint;
    result_datum.set_decimal_int(result);
  }
  return ret;
}

int ObTextRetrievalTokenIter::estimate_token_doc_cnt()
{
  int ret = OB_SUCCESS;
  int64_t logical_row_cnt = 0;
  int64_t physical_row_cnt = 0;
  ObSEArray<ObEstRowCountRecord, 1> est_records;
  ObArenaAllocator allocator;
  const int64_t timeout_us = THIS_WORKER.get_timeout_remain();
  ObAccessService *access_service = NULL;
  storage::ObTableScanRange table_scan_range;
  ObSimpleBatch batch;
  batch.type_ = ObSimpleBatch::T_SCAN;
  batch.range_ = &inv_idx_agg_param_->key_ranges_.at(0);
  ObTableScanParam est_param;
  est_param.index_id_ = inv_idx_agg_param_->index_id_;
  est_param.scan_flag_ = inv_idx_agg_param_->scan_flag_;
  est_param.tablet_id_ = inv_idx_agg_param_->tablet_id_;
  est_param.ls_id_ = inv_idx_agg_param_->ls_id_;
  est_param.tx_id_ = inv_idx_agg_param_->tx_id_;
  est_param.schema_version_ = inv_idx_agg_param_->schema_version_;
  est_param.frozen_version_ = GET_BATCH_ROWS_READ_SNAPSHOT_VERSION;
  if (OB_ISNULL(access_service = MTL(ObAccessService *))) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret), K(access_service));
  } else if (OB_FAIL(table_scan_range.init(*inv_idx_agg_param_, batch, allocator))) {
    STORAGE_LOG(WARN, "Failed to init table scan range", K(ret), K(batch));
  } else if (OB_FAIL(access_service->estimate_row_count(est_param,
                                                        table_scan_range,
                                                        timeout_us,
                                                        est_records,
                                                        logical_row_cnt,
                                                        physical_row_cnt))) {
    LOG_TRACE("OPT:[STORAGE EST FAILED, USE STAT EST]", "storage_ret", ret);
  } else {
    token_doc_cnt_ = logical_row_cnt;
    token_doc_cnt_calculated_ = true;
    sql::ObExpr *total_doc_cnt_param_expr = relevance_expr_->args_[sql::ObExprBM25::TOTAL_DOC_CNT_PARAM_IDX];
    if (OB_ISNULL(total_doc_cnt_param_expr)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected null total doc cnt expr", K(ret));
    } else {
      int64_t total_doc_cnt = 0;
      if (total_doc_cnt_param_expr->enable_rich_format()
          && is_valid_format(total_doc_cnt_param_expr->get_format(*eval_ctx_))) {
        total_doc_cnt = total_doc_cnt_param_expr->get_vector(*eval_ctx_)->get_int(0);
      } else {
        total_doc_cnt = total_doc_cnt_param_expr->locate_expr_datum(*eval_ctx_, 0).get_int();
      }
      max_token_relevance_ = sql::ObExprBM25::query_token_weight(token_doc_cnt_, total_doc_cnt);
    }
  }
  return ret;
}

ObTextRetrievalDaaTTokenIter::ObTextRetrievalDaaTTokenIter()
  : ObISRDaaTDimIter(),
    allocator_(nullptr),
    token_iter_(nullptr),
    eval_ctx_(nullptr),
    relevance_expr_(nullptr),
    inv_scan_domain_id_col_(nullptr),
    max_batch_size_(1),
    cur_idx_(-1),
    count_(0),
    relevance_(),
    doc_id_(),
    cmp_func_(nullptr),
    is_inited_(false)
{
}

int ObTextRetrievalDaaTTokenIter::init(const ObTextRetrievalScanIterParam &iter_param)
{
  int ret = OB_SUCCESS;
  void *buf = nullptr;
  allocator_ = iter_param.allocator_;
  if (OB_ISNULL(iter_param.eval_ctx_) || OB_ISNULL(iter_param.inv_scan_domain_id_col_) || OB_ISNULL(allocator_)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid argument", K(ret), KP(iter_param.eval_ctx_), KP(iter_param.inv_scan_domain_id_col_), KP(allocator_));
  } else if (OB_ISNULL(buf = allocator_->alloc(sizeof(ObTextRetrievalTokenIter)))) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    LOG_WARN("failed to allocate memory", K(ret));
  } else {
    token_iter_ = new (buf) ObTextRetrievalTokenIter();
    if (OB_FAIL(token_iter_->init(iter_param))) {
      LOG_WARN("failed to init token iter", K(ret));
    } else {
      eval_ctx_ = iter_param.eval_ctx_;
      relevance_expr_ = iter_param.relevance_expr_;
      inv_scan_domain_id_col_ = iter_param.inv_scan_domain_id_col_;
      max_batch_size_ = OB_MAX(iter_param.eval_ctx_->max_batch_size_, 1);
      sql::ObExprBasicFuncs *basic_funcs = ObDatumFuncs::get_basic_func(inv_scan_domain_id_col_->datum_meta_.type_, CS_TYPE_BINARY);
      cmp_func_ = lib::is_oracle_mode() ? basic_funcs->null_last_cmp_ : basic_funcs->null_first_cmp_;
      if (OB_ISNULL(cmp_func_)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("failed to init IRIterLoserTreeCmp", K(ret));
      } else if (FALSE_IT(relevance_.set_allocator(allocator_))) {
      } else if (OB_FAIL(relevance_.init(max_batch_size_))) {
        LOG_WARN("failed to init next batch iter idxes array", K(ret));
      } else if (OB_FAIL(relevance_.prepare_allocate(max_batch_size_))) {
        LOG_WARN("failed to prepare allocate next batch iter idxes array", K(ret));
      } else if (FALSE_IT(doc_id_.set_allocator(allocator_))) {
      } else if (OB_FAIL(doc_id_.init(max_batch_size_))) {
        LOG_WARN("failed to init next batch iter idxes array", K(ret));
      } else if (OB_FAIL(doc_id_.prepare_allocate(max_batch_size_))) {
        LOG_WARN("failed to prepare allocate next batch iter idxes array", K(ret));
      } else {
        is_inited_ = true;

        // init_with_cache(); // 希望可以使用缓存
      }
    }
  }
  return ret;
}

void ObTextRetrievalDaaTTokenIter::reuse()
{
  cur_idx_ = -1;
  count_ = 0;
  token_iter_->reuse();
  cache_read_idx_ = -1;

  // if (own_cache_iter_) {
  //   own_cache_iter_->~ObTRDaaTTokenCacheIter();
  //   TokenCacheMg::get_instance().allocator_.free(own_cache_iter_);
  //   own_cache_iter_ = nullptr;
  // }
}

void ObTextRetrievalDaaTTokenIter::reset()
{
  relevance_.reset();
  doc_id_.reset();
  token_iter_->reset();
  cur_idx_ = -1;
  cache_read_idx_ = -1;
  count_ = 0;
  // if (own_cache_iter_) {
  //   own_cache_iter_->~ObTRDaaTTokenCacheIter();
  //   TokenCacheMg::get_instance().allocator_.free(own_cache_iter_);
  //   own_cache_iter_ = nullptr;
  // }

}
ObTextRetrievalDaaTTokenIter::~ObTextRetrievalDaaTTokenIter() {
  // if (own_cache_iter_) {
  //   own_cache_iter_->~ObTRDaaTTokenCacheIter();
  //   TokenCacheMg::get_instance().allocator_.free(own_cache_iter_);
  //   own_cache_iter_ = nullptr;
  // }
}

/////////////////////advise
// ADCacheData    ADManager

int ADCacheData::add_data(const ObFixedArray<double, ObIAllocator> &relevance,
                        const ObFixedArray<ObDocIdExt, ObIAllocator> &doc_id,
                        int64_t count) {

  adlog::DEBUG("ADCacheData::add_data start");
  int ret = OB_SUCCESS;  
  if (OB_UNLIKELY(count <= 0 || relevance.count() < count || doc_id.count() < count)) {  
    ret = OB_INVALID_ARGUMENT;  
    adlog::DEBUG("invalid count or array size", K(ret), K(count),   
              K(relevance.count()), K(doc_id.count()));  
  } else {  
      
    for (int64_t i = 0; OB_SUCC(ret) && i < count; ++i) {  
      if (OB_FAIL(relevance_scores_.push_back(relevance[i]))) {  
        adlog::DEBUG("failed to push back relevance", K(ret));  
      } else if (OB_FAIL(doc_ids_.push_back(doc_id[i]))) {  
        adlog::DEBUG("failed to push back doc_id", K(ret));  
      }  
    }  
      
    if (OB_SUCC(ret)) {  
      total_count_ += count;    
    }
  }

  adlog::DEBUG("ADCacheData::add_data end : ", K(total_count_));

    
  return ret; 
}

int ObTextRetrievalDaaTTokenIter::force_build_cache() {
  int ret = OB_SUCCESS;
  adlog::DEBUG(" 强制构建缓存 开始");
  // 创建缓存数据， 这个cache_data就是放到map里面的内容，我们写数据就直接写到cache_data里面
  ADCacheData *cache_data;
  ADManager::get_one().create_cache_data(token_name_, cache_data);

  auto &&save_data = [&]() ->void {
    if (OB_FAIL(relevance_expr_->eval_batch(*eval_ctx_, *token_iter_->get_skip(), count_))) {
      LOG_WARN("failed to evaluate relevance", K(ret));
    } else {
      cur_idx_ = 0;
      const ObDatumVector &relevance_datum = relevance_expr_->locate_expr_datumvector(*eval_ctx_);
      const ObDatumVector &doc_id_datum = inv_scan_domain_id_col_->locate_expr_datumvector(*eval_ctx_);
      for (int64_t i = 0; OB_SUCC(ret) && i < count_; ++i) {
        if (OB_LIKELY(!token_iter_->get_skip()->at(i))) {
          relevance_[i] = relevance_datum.at(i)->get_double();
          if (OB_FAIL(doc_id_[i].from_datum(*doc_id_datum.at(i)))) {
            LOG_WARN("failed to get doc id", K(ret), K(doc_id_datum.at(i)));
          }
        }
      }
    }
    // 上面是正常的存储数据
    // 然后要存储到缓存里面
    cache_data->add_data(relevance_, doc_id_, count_);
  };
  // token_iter_->reuse();
  // 强制扫描所有数据  
  while (OB_SUCC(ret)) {  
    if (OB_FAIL(token_iter_->get_next_batch(max_batch_size_, count_))) {
      if (OB_UNLIKELY(OB_ITER_END != ret)) {
        adlog::DEBUG("WARNING! 缓存迭代器 failed to get batch rows from inverted index");
      } else if (count_ != 0) {
        adlog::DEBUG("WARNING!    no wat");
        return ret;
      } else {
        adlog::DEBUG("阿巴 count = 0");
      }
      break;
    }
    adlog::DEBUG("缓存 开始写入一批数据 ", K(count_));
    save_data();
  }
  // token_iter_->reuse();

  if (ret == OB_ITER_END) {
    ret = OB_SUCCESS;
  }
  if (OB_SUCC(ret)) {  
    cache_data->is_completed_ = true;
    adlog::DEBUG("缓存构建 完成");
  }
  adlog::DEBUG(" 强制构建缓存 结束");

  return ret;
}
int ObTextRetrievalDaaTTokenIter::init_with_cache() {
  adlog::DEBUG("初始化缓存 开始");
  int ret = OB_SUCCESS;  
      
  ADManager &cache_mgr = ADManager::get_one();  
  auto && cache_data = cache_mgr.get_cache_data(token_name_);  
    
  if (cache_data && cache_data->is_completed_) {  
    use_cache_ = true;
    cache_read_idx_ = -1;
    own_cache_data_ = cache_data;
    adlog::DEBUG("原本有好了的缓存 直接使用");
  } else {
    // 如果没有的话，这里就要进行判断了.
    // 如果原本就没有缓存的话，那么它会担负起 强制缓存的内容
    // 如果是有缓存的， 但是没有完成（这种情况不太可能）
    // 或者说 是因为被淘汰了，那么走之前的路径
    if (cache_data == nullptr) {
      force_build_cache();
      use_cache_ = true;
      cache_read_idx_ = -1;
      own_cache_data_ = cache_mgr.get_cache_data(token_name_);
      adlog::DEBUG("初始化缓存 结束");
      return ret;
    }
    use_cache_ = false;
    adlog::DEBUG("没有使用缓存");
  }
  adlog::DEBUG("初始化缓存 结束");
  
  return ret;  
}


// Sparse Retrieval Dimension Iter Interfaces
int ObTextRetrievalDaaTTokenIter::get_next_row() {
  int ret = OB_SUCCESS;
  bool need_load = false;
  adlog::DEBUG("dim_iter的 get_next_row 开始");
  
  // if(!init_with_cache_flag_){
  //   init_with_cache();
  //   init_with_cache_flag_ = true;
  // }

  // auto &&mark_have_done = [&]()-> void {
  //   auto &&cache_iter = TokenCacheMg::get_instance().get_cache_iter(token_name_);
  //   if (!cache_iter) {
  //     return ;
  //   }
  //   cache_iter->have_done_flag_ = true;
  //   return;
  // };

  // // 这里强制写
  // {
  //   auto &&cache_iter =
  //       TokenCacheMg::get_instance().get_cache_iter(token_name_);
  //   if (cache_iter == nullptr) {
  //     // 代表还没有人搞这个
  //     adlog::DEBUG("打算强制写入");
  //     auto && new_iter = TokenCacheMg::get_instance().create_cache_iter();
  //     TokenCacheMg::get_instance().cache_iter(token_name_, new_iter);
  //     // 把manager里的token_name对应的缓存迭代器创建出来了
  //     // 接下来就是强制写入
  //     int64_t las_cur_idx = cur_idx_;
  //     while (OB_SUCC(ret)) {
  //       if (OB_LIKELY((++cur_idx_) < count_)) {
  //       } else if (!eval_ctx_->is_vectorized() && (OB_FAIL(token_iter_->get_next_row()))) {
  //         if (OB_UNLIKELY(OB_ITER_END != ret)) {
  //           LOG_WARN("failed to get row from inverted index", K(ret));
  //         }
  //       } else if (!eval_ctx_->is_vectorized() && FALSE_IT(count_ = 1)) {
  //       } else if (eval_ctx_->is_vectorized() && OB_FAIL(token_iter_->get_next_batch(max_batch_size_, count_))) {
  //         if (OB_UNLIKELY(OB_ITER_END != ret)) {
  //           LOG_WARN("failed to get batch rows from inverted index", K(ret));
  //           // mark_have_done();
  //           adlog::DEBUG("WARNING! 缓存迭代器 failed to get batch rows from inverted index");
  //           return ret;
  //         } else if (count_ != 0) {
  //           ret = OB_SUCCESS;
  //           need_load = true;
  //           adlog::DEBUG("ObTextRetrievalDaaTTokenIter::get_next_row() 最后一轮了");
  //           if (OB_NOT_NULL(relevance_expr_)) {
  //             if (OB_FAIL(save_relevances_and_docids())) {
  //               LOG_WARN("failed to evaluate simarity expr", K(ret));
  //             }
  //           } else if (OB_FAIL(save_docids())) {
  //               LOG_WARN("failed to save doc ids", K(ret));
  //           }
  //           mark_have_done();
  //         } else {
  //           mark_have_done();
  //           adlog::DEBUG("阿巴 count = 0");
  //         }

  //         break;

  //       } else {
  //         need_load = true;
  //       }
  //       adlog::DEBUG("开始写入一批数据 ", K(count_));
  //       if (OB_SUCC(ret) && need_load) {
  //         if (OB_NOT_NULL(relevance_expr_)) {
  //           if (OB_FAIL(save_relevances_and_docids())) {
  //             LOG_WARN("failed to evaluate simarity expr", K(ret));
  //           }
  //         } else if (OB_FAIL(save_docids())) {
  //             LOG_WARN("failed to save doc ids", K(ret));
  //         }
  //       }
  //     }
  //     cur_idx_ = las_cur_idx;
  //     // token_iter_->reuse();
  //   }
  // }

  // { //用来创建属于自己的缓存迭代器
  //   auto &&cache_iter =
  //       TokenCacheMg::get_instance().get_cache_iter(token_name_);
  //   if (cache_iter and cache_iter->have_done_flag_) {
  //     // 代表他指的迭代器可以使用了，那么就直接调用它的对应的缓存迭代器吧.ObTRDaaTTokenCacheIter
  //     if (!own_cache_iter_) {
  //       own_cache_iter_ =
  //           TokenCacheMg::get_instance().create_iter_from_cache(cache_iter);
  //       if(own_cache_iter_ == nullptr){
  //         adlog::DEBUG("ERROR own_cache_iter_ == nullptr");
  //       }
  //     }
  //     own_cache_iter_->cur_idx_ = cur_idx_;
  //     ++cur_idx_;
  //     adlog::DEBUG("使用了内置的缓存迭代器");
  //     return own_cache_iter_->get_next_row();
  //   }
  // }

  if (use_cache_) {
    // 如果使用了缓存.
    if (cache_read_idx_ >= own_cache_data_->total_count_ - 1) {
      ret = OB_ITER_END;
    } else { 
      cache_read_idx_++;
    }
    adlog::DEBUG("ObTextRetrievalDaaTTokenIter::get_curr_id 缓存 : idx: ", cache_read_idx_, " total_count: ",own_cache_data_->total_count_ );
    return ret;
  }

  // adlog::DEBUG("没有使用缓存迭代器： ObTextRetrievalDaaTTokenIter::get_next_row()");
  if (OB_LIKELY((++cur_idx_) < count_)) {
  } else if (!eval_ctx_->is_vectorized() && (OB_FAIL(token_iter_->get_next_row()))) {
    if (OB_UNLIKELY(OB_ITER_END != ret)) {
      LOG_WARN("failed to get row from inverted index", K(ret));
    }
  } else if (!eval_ctx_->is_vectorized() && FALSE_IT(count_ = 1)) {
  } else if (eval_ctx_->is_vectorized() && OB_FAIL(token_iter_->get_next_batch(max_batch_size_, count_))) {
    if (OB_UNLIKELY(OB_ITER_END != ret)) {
      LOG_WARN("failed to get batch rows from inverted index", K(ret));
      adlog::DEBUG("WARNING! 缓存迭代器 failed to get batch rows from inverted index");
    } else if (count_ != 0) {
      ret = OB_SUCCESS;
      need_load = true;
      adlog::DEBUG("ObTextRetrievalDaaTTokenIter::get_next_row() 最后一轮了");
      if (OB_NOT_NULL(relevance_expr_)) {
        if (OB_FAIL(save_relevances_and_docids())) {
          LOG_WARN("failed to evaluate simarity expr", K(ret));
        }
      } else if (OB_FAIL(save_docids())) {
          LOG_WARN("failed to save doc ids", K(ret));
      }
      return ret;
    } else {

      adlog::DEBUG("阿巴 no use : count = 0");
    }
  } else {
    need_load = true;
  }
  adlog::DEBUG("ObTextRetrievalDaaTTokenIter 没有走缓存 开始写入一批数据 ", K(count_));
  if (OB_SUCC(ret) && need_load) {
    if (OB_NOT_NULL(relevance_expr_)) {
      if (OB_FAIL(save_relevances_and_docids())) {
        LOG_WARN("failed to evaluate simarity expr", K(ret));
      }
    } else if (OB_FAIL(save_docids())) {
        LOG_WARN("failed to save doc ids", K(ret));
    }
  }
  return ret;
}

int ObTextRetrievalDaaTTokenIter::save_relevances_and_docids()
{
  int ret = OB_SUCCESS;

  if (token_iter_->use_cache_ and token_iter_->own_cache_data_) {
    // 代表我们去使用下层的缓存来接受数据吧
    // 从缓存读取数据  
    int64_t cache_count = token_iter_->own_cache_data_->total_count_ - (token_iter_->cache_read_idx_);  
    adlog::DEBUG("save 保存 ： ",K(cache_count), "  ",K(count_));
    count_ = std::min(cache_count, count_);  
    cur_idx_ = 0;  
      
    for (int64_t i = 0; OB_SUCC(ret) && i < count_; ++i) {  
      if (OB_FAIL(token_iter_->get_cache_data(i, relevance_[i], doc_id_[i]))) {  
        LOG_WARN("failed to get cache data", K(ret), K(i));  
      }  
    }
    
    return ret;
  }

  if (OB_ISNULL(relevance_expr_) || OB_ISNULL(inv_scan_domain_id_col_)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid relevance or doc id expr", K(ret));
  } else if (OB_FAIL(relevance_expr_->eval_batch(*eval_ctx_, *token_iter_->get_skip(), count_))) {
    LOG_WARN("failed to evaluate relevance", K(ret));
  } else {
    cur_idx_ = 0;
    const ObDatumVector &relevance_datum = relevance_expr_->locate_expr_datumvector(*eval_ctx_);
    const ObDatumVector &doc_id_datum = inv_scan_domain_id_col_->locate_expr_datumvector(*eval_ctx_);
    for (int64_t i = 0; OB_SUCC(ret) && i < count_; ++i) {
      if (OB_LIKELY(!token_iter_->get_skip()->at(i))) {
        relevance_[i] = relevance_datum.at(i)->get_double();
        if (OB_FAIL(doc_id_[i].from_datum(*doc_id_datum.at(i)))) {
          LOG_WARN("failed to get doc id", K(ret), K(doc_id_datum.at(i)));
        }
      }
    }
    // 把这个结果也给我放到缓存里面去，哎我焯，希望QPS能多一点吧

    // {
    //   auto &&cache_iter =
    //     TokenCacheMg::get_instance().get_cache_iter(token_name_);
    //   if (OB_SUCC(ret) && cache_iter != nullptr && (!cache_iter->have_done_flag_)) {  
    //     adlog::DEBUG("ObTextRetrievalDaaTTokenIter::save_relevances_and_docids() 开始把数据放进去缓存了");
    //     if (OB_FAIL(cache_iter->add_data(relevance_, doc_id_, count_))) {  
    //       adlog::DEBUG("failed to add data to cache", K(ret));  
    //     }  
    //   }  
    // }
  }
  return ret;
}

int ObTextRetrievalDaaTTokenIter::save_docids()
{
  int ret = OB_SUCCESS;
  if (OB_ISNULL(inv_scan_domain_id_col_)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid relevance or doc id expr", K(ret));
  } else {
    cur_idx_ = 0;
    const ObDatumVector &doc_id_datum = inv_scan_domain_id_col_->locate_expr_datumvector(*eval_ctx_);
    for (int64_t i = 0; OB_SUCC(ret) && i < count_; ++i) {
      if (OB_LIKELY(!token_iter_->get_skip()->at(i))) {
        if (OB_FAIL(doc_id_[i].from_datum(*doc_id_datum.at(i)))) {
          LOG_WARN("failed to get doc id", K(ret));
        };
      }
    }
    // 从火焰图里貌似没看到这个函数的调用，那就先不写缓存了
    adlog::DEBUG("WARNING! ObTextRetrievalDaaTTokenIter::save_docids()");
  }
  return ret;
}

int ObTextRetrievalDaaTTokenIter::get_next_batch(const int64_t capacity, int64_t &count)
{
  return OB_NOT_IMPLEMENT;
}

int ObTextRetrievalDaaTTokenIter::advance_to(const ObDatum &id_datum)
{
  int ret = OB_SUCCESS;
  int result = 0;
  bool find = false;

  if (cur_idx_ < 0) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected cur_idx", K(ret), K(cur_idx_));
  }
  while (OB_SUCC(ret) && !find) {
    if (cur_idx_ < count_) {
      if (OB_FAIL(cmp_func_(id_datum, doc_id_[cur_idx_].get_datum(), result))) {
        LOG_WARN("failed to compare datum", K(ret));
      } else if (result <= 0) {
        find = true;
      } else {
        ++cur_idx_;
      }
    } else if (OB_FAIL(token_iter_->advance_to(id_datum))) {
      LOG_WARN("failed to advance token iter", K(ret));
    } else if (OB_FAIL(get_next_row())) {
      if (OB_UNLIKELY(OB_ITER_END != ret)) {
        LOG_WARN("failed to get batch rows from inverted index", K(ret));
      } else {
        find = true;
      }
    } else if (cur_idx_ != 0) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected result", K(ret), K(result));
    } else if (OB_FAIL(cmp_func_(id_datum, doc_id_[cur_idx_].get_datum(), result))) {
      LOG_WARN("failed to compare datum", K(ret));
    } else if (result <= 0) {
      find = true;
    } else {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected result", K(ret), K(result));
    }
  }
  return ret;
}

int ObTextRetrievalDaaTTokenIter::get_curr_score(double &score) const
{
  int ret = OB_SUCCESS;

  
  if (use_cache_ ) {
    adlog::DEBUG("ObTextRetrievalDaaTTokenIter::get_curr_id 缓存 : idx: ", cache_read_idx_, " total_count: ",own_cache_data_->total_count_ );

    score = own_cache_data_->relevance_scores_[cache_read_idx_];
    return ret;
  }

  if (OB_UNLIKELY(cur_idx_ >= count_)) {
    ret = OB_ARRAY_OUT_OF_RANGE;
    adlog::DEBUG("array index out of bounds  cur_idx_: ",cur_idx_,"  count_ :", count_);
    LOG_WARN("array index out of bounds", K(ret), K_(cur_idx), K_(count));
  } else if (relevance_expr_) {
    score = relevance_[cur_idx_];
  }
  return ret;
}

int ObTextRetrievalDaaTTokenIter::get_curr_id(const ObDatum *&id_datum) const
{
  int ret = OB_SUCCESS;
  if (use_cache_) {
    adlog::DEBUG("ObTextRetrievalDaaTTokenIter::get_curr_id 缓存 : idx: ", cache_read_idx_, " total_count: ",own_cache_data_->total_count_ );

    id_datum = &(own_cache_data_->doc_ids_[cache_read_idx_]).get_datum();
    return ret;
  }

  if (OB_UNLIKELY(cur_idx_ >= count_)) {
    ret = OB_ARRAY_OUT_OF_RANGE;
    LOG_WARN("array index out of bounds", K(ret), K_(cur_idx), K_(count));
  } else {
    id_datum = &doc_id_[cur_idx_].get_datum();
  }
  return ret;
}

ObTextRetrievalBlockMaxIter::ObTextRetrievalBlockMaxIter()
  : ObISRDimBlockMaxIter(),
    token_iter_(),
    block_max_iter_(),
    block_max_iter_param_(nullptr),
    block_max_scan_param_(nullptr),
    ranking_param_(),
    curr_id_(nullptr),
    max_score_tuple_(nullptr),
    dim_max_score_(0),
    block_max_inited_(false),
    in_shallow_status_(false),
    is_inited_(false)
{
}

int ObTextRetrievalBlockMaxIter::init(
    const ObTextRetrievalScanIterParam &iter_param,
    const ObBlockMaxScoreIterParam &block_max_iter_param,
    ObTableScanParam &scan_param)
{
  int ret = OB_SUCCESS;
  if (IS_INIT) {
    ret = OB_INIT_TWICE;
    LOG_WARN("double initialization", K(ret));
  } else if (OB_UNLIKELY(!block_max_iter_param.is_valid() || !scan_param.is_valid())) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid iter param", K(ret), K(block_max_iter_param), K(scan_param));
  } else if (OB_FAIL(token_iter_.init(iter_param))) {
    LOG_WARN("failed to init token iter", K(ret));
  } else {
    curr_id_ = nullptr;
    block_max_iter_param_ = &block_max_iter_param;
    block_max_scan_param_ = &scan_param;
    max_score_tuple_ = nullptr;
    dim_max_score_ = 0;
    in_shallow_status_ = false;
    ranking_param_.token_freq_col_idx_ = block_max_iter_param.token_freq_col_idx_;
    ranking_param_.doc_length_col_idx_ = block_max_iter_param.doc_length_col_idx_;
    is_inited_ = true;
  }
  return ret;
}

void ObTextRetrievalBlockMaxIter::reset()
{
  token_iter_.reset();
  block_max_iter_.reset();
  curr_id_ = nullptr;
  block_max_iter_param_ = nullptr;
  block_max_scan_param_ = nullptr;
  max_score_tuple_ = nullptr;
  dim_max_score_ = 0;
  in_shallow_status_ = false;
  block_max_inited_ = false;
  is_inited_ = false;
}

void ObTextRetrievalBlockMaxIter::reuse()
{
  token_iter_.reuse();
  block_max_iter_.reset();
  curr_id_ = nullptr;
  max_score_tuple_ = nullptr;
  in_shallow_status_ = false;
  block_max_inited_ = false;
  dim_max_score_ = 0;
}

int ObTextRetrievalBlockMaxIter::get_next_row()
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("not initialized", K(ret));
  } else if (OB_UNLIKELY(in_shallow_status_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected iter status, can not get next row after shallow advance",
        K(ret), K_(in_shallow_status));
  } else if (OB_FAIL(token_iter_.get_next_row())) {
    if (OB_UNLIKELY(OB_ITER_END != ret)) {
      LOG_WARN("failed to get next row", K(ret));
    }
  } else if (OB_FAIL(token_iter_.get_curr_id(curr_id_))) {
    LOG_WARN("failed to get curr id", K(ret));
  }
  return ret;
}

int ObTextRetrievalBlockMaxIter::get_next_batch(const int64_t capacity, int64_t &count)
{
  return OB_NOT_IMPLEMENT;
}

int ObTextRetrievalBlockMaxIter::advance_to(const ObDatum &id_datum)
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("not initialized", K(ret));
  } else if (OB_FAIL(token_iter_.advance_to(id_datum))) {
    if (OB_UNLIKELY(OB_ITER_END != ret)) {
      LOG_WARN("failed to advance to id datum", K(ret));
    }
  } else if (OB_FAIL(token_iter_.get_curr_id(curr_id_))) {
    LOG_WARN("failed to get curr id", K(ret));
  } else {
    in_shallow_status_ = false;
  }
  return ret;
}

int ObTextRetrievalBlockMaxIter::advance_shallow(const ObDatum &id_datum, const bool inclusive)
{
  int ret = OB_SUCCESS;
  LOG_DEBUG("[Sparse Retrieval] advance shallow", K(ret), K(id_datum), K(inclusive));
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("not initialized", K(ret));
  } else if (OB_UNLIKELY(!block_max_inited_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected block max iter not calculated", K(ret), K_(block_max_inited));
  } else if (OB_FAIL(block_max_iter_.advance_to(id_datum, inclusive))) {
    if (OB_UNLIKELY(OB_ITER_END != ret)) {
      LOG_WARN("failed to advance to id datum", K(ret));
    }
  } else if (OB_FAIL(block_max_iter_.get_curr_max_score_tuple(max_score_tuple_))) {
    LOG_WARN("failed to get next max score tuple", K(ret));
  } else {
    // max_score_tuple_->min_domain_id_ should not be smaller than $id_datum
    curr_id_ = max_score_tuple_->min_domain_id_;
    in_shallow_status_ = true;
  }
  return ret;
}

int ObTextRetrievalBlockMaxIter::get_curr_score(double &score) const
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("not initialized", K(ret));
  } else if (OB_UNLIKELY(in_shallow_status_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected iter status, can not get curr score after shallow advance",
        K(ret), K_(in_shallow_status));
  } else if (OB_FAIL(token_iter_.get_curr_score(score))) {
    LOG_WARN("failed to get curr score", K(ret));
  }
  return ret;
}

int ObTextRetrievalBlockMaxIter::get_curr_id(const ObDatum *&id_datum) const
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("not initialized", K(ret));
  } else if (OB_ISNULL(curr_id_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected nullptr to curr id", K(ret), KP_(curr_id));
  } else {
    id_datum = curr_id_;
  }
  return ret;
}

int ObTextRetrievalBlockMaxIter::get_dim_max_score(double &score)
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("not initialized", K(ret));
  } else if (OB_UNLIKELY(!block_max_inited_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected block max iter not calculated", K(ret), K_(block_max_inited));
  } else {
    score = dim_max_score_;
  }
  return ret;
}

int ObTextRetrievalBlockMaxIter::get_curr_block_max_info(const ObMaxScoreTuple *&max_score_tuple)
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("not initialized", K(ret));
  } else if (OB_UNLIKELY(!block_max_inited_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected block max iter not calculated", K(ret), K_(block_max_inited));
  } else if (OB_ISNULL(max_score_tuple_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected nullptr to max score tuple", K(ret), KP_(max_score_tuple));
  } else {
    max_score_tuple = max_score_tuple_;
  }
  return ret;
}

bool ObTextRetrievalBlockMaxIter::in_shallow_status() const
{
  return in_shallow_status_;
}

int ObTextRetrievalBlockMaxIter::calc_dim_max_score(
    const ObBlockMaxScoreIterParam &block_max_iter_param,
    const ObBlockMaxBM25RankingParam &ranking_param,
    ObTableScanParam &scan_param)
{
  int ret = OB_SUCCESS;
  // Maybe a specialized interface to calculate dimension max score based on statistics is more efficient
  if (OB_FAIL(block_max_iter_.init(ranking_param, block_max_iter_param, scan_param))) {
    LOG_WARN("failed to init block max iter", K(ret));
  }

  while (OB_SUCC(ret)) {
    const ObMaxScoreTuple *max_score_tuple = nullptr;
    if (OB_FAIL(block_max_iter_.get_next(max_score_tuple))) {
      if (OB_UNLIKELY(OB_ITER_END != ret)) {
        LOG_WARN("failed to get next max score tuple", K(ret));
      }
    } else if (OB_ISNULL(max_score_tuple)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected nullptr to max score tuple", K(ret), KP_(max_score_tuple));
    } else {
      dim_max_score_ = std::max(dim_max_score_, max_score_tuple->max_score_);
      LOG_DEBUG("[Text Retrieval] calc dim max score", K(ret), K(dim_max_score_), K(max_score_tuple->max_score_),
        KPC(max_score_tuple->max_domain_id_), KPC(max_score_tuple->min_domain_id_));
    }
  }

  if (OB_LIKELY(OB_ITER_END == ret)) {
    ret = OB_SUCCESS;
    block_max_iter_.reset(); // TODO: reuse or rewind iter
  } else {
    LOG_WARN("failed to calc dim max score", K(ret));
  }
  return ret;
}

int ObTextRetrievalBlockMaxIter::init_block_max_iter(const int64_t total_doc_cnt, const double avg_doc_token_cnt)
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("not initialized", K(ret));
  } else if (OB_UNLIKELY(block_max_inited_)) {
    ret = OB_INIT_TWICE;
    LOG_WARN("block max iter already initalized", K(ret));
  } else if (FALSE_IT(ranking_param_.total_doc_cnt_ = total_doc_cnt)) {
  } else if (FALSE_IT(ranking_param_.avg_doc_token_cnt_ = avg_doc_token_cnt)) {
  } else if (OB_FAIL(token_iter_.get_token_doc_cnt(ranking_param_.doc_freq_))) {
    LOG_WARN("failed to get token doc cnt", K(ret));
  } else if (OB_FAIL(calc_dim_max_score(*block_max_iter_param_, ranking_param_, *block_max_scan_param_))) {
    LOG_WARN("failed to calc dim max score", K(ret));
  } else if (OB_FAIL(block_max_iter_.init(ranking_param_, *block_max_iter_param_, *block_max_scan_param_))) {
    LOG_WARN("failed to init block max iter", K(ret));
  } else {
    block_max_inited_ = true;
  }
  return ret;
}


/*
class ObTRDaaTTokenCacheIter : public ObISRDaaTDimIter {  
public:  
ObTRDaaTTokenCacheIter();  
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

  
  ObFixedArray<double, ObIAllocator> cached_relevance_;  
  ObFixedArray<ObDocIdExt, ObIAllocator> cached_doc_id_;  
  int64_t cur_idx_;
  int64_t total_count_;
  bool is_created_flag_{false};
  bool have_done_flag_{false};
};
*/

ObTRDaaTTokenCacheIter::ObTRDaaTTokenCacheIter(common::ObIAllocator *alloc)
    : cur_idx_(-1), total_count_(0), is_created_flag_(false),
      have_done_flag_(false), allocator_(alloc)  
{  
  // cached_relevance_.set_allocator(allocator_);  
  // cached_doc_id_.set_allocator(allocator_);  
}

int ObTRDaaTTokenCacheIter::get_next_row() {
  adlog::DEBUG("调用了缓存迭代器的 get_next_row");
  int ret = OB_SUCCESS;  
  if (OB_UNLIKELY(!have_done_flag_)) {  
    ret = OB_NOT_INIT;  
    adlog::DEBUG("ObTRDaaTTokenCacheIter::get_next_row cache iter not ready", K(ret));  
  } else if (cur_idx_ >= total_count_ - 1) {
    ret = OB_ITER_END;
    // cur_idx_ = -1;  
    adlog::DEBUG("缓存迭代器： 返回 OB_ITER_END");
  } else {
    ++cur_idx_;
  }  
  return ret;  
}  
  
int ObTRDaaTTokenCacheIter::advance_to(const ObDatum &id_datum) {
  adlog::DEBUG("WARNING! ObTRDaaTTokenCacheIter::advance_to");
  return OB_NOT_INIT;
  // int ret = OB_SUCCESS;  
  // if (OB_UNLIKELY(!have_done_flag_)) {  
  //   ret = OB_NOT_INIT;  
  //   adlog::DEBUG("ObTRDaaTTokenCacheIter::advance_to cache iter not ready", K(ret));  
  // } else {  
  //   while (cur_idx_ < total_count_) {  
  //     const ObDatum &cur_id = cached_doc_id_[cur_idx_].get_datum();  
  //     if (cur_id.compare(id_datum) >= 0) {  
  //       break;  
  //     }  
  //     ++cur_idx_;  
  //   }  
  //   if (cur_idx_ >= total_count_) {  
  //     ret = OB_ITER_END;  
  //   }  
  // }  
  // return ret;  
}  
    
int ObTRDaaTTokenCacheIter::get_curr_score(double &score) const  
{  
  int ret = OB_SUCCESS;  
  if (OB_UNLIKELY(!have_done_flag_ || cur_idx_ >= total_count_ || cur_idx_ < 0)) {  
    ret = OB_ARRAY_OUT_OF_RANGE;  
    adlog::DEBUG("ObTRDaaTTokenCacheIter::get_curr_score invalid cache state", K(ret), K_(cur_idx), K_(total_count));
  } else {
    // adlog::DEBUG("缓存迭代器： ObTRDaaTTokenCacheIter::get_curr_score");
    score = cached_relevance_[cur_idx_];  
  }  
  return ret;  
}  
   
int ObTRDaaTTokenCacheIter::get_curr_id(const ObDatum *&datum) const  
{  
  int ret = OB_SUCCESS;  
  if (OB_UNLIKELY(!have_done_flag_ || cur_idx_ >= total_count_ || cur_idx_ < 0)) {  
    ret = OB_ARRAY_OUT_OF_RANGE;  
    adlog::DEBUG("ObTRDaaTTokenCacheIter::get_curr_id invalid cache state", K(ret), K_(cur_idx), K_(total_count));  
  } else {  
    // adlog::DEBUG("缓存迭代器： ObTRDaaTTokenCacheIter::get_curr_id");
    datum = &cached_doc_id_[cur_idx_].get_datum();  
  }  
  return ret;  
}  
  
int ObTRDaaTTokenCacheIter::get_dim_max_score(double &score)  
{  
  int ret = OB_SUCCESS;  
  if (OB_UNLIKELY(!have_done_flag_ || total_count_ == 0)) {  
    ret = OB_NOT_INIT;  
    adlog::DEBUG("ObTRDaaTTokenCacheIter::get_dim_max_score cache not ready or empty", K(ret), K_(total_count));  
  } else {  

    score = cached_relevance_[0];  
    for (int64_t i = 1; i < total_count_; ++i) {  
      if (cached_relevance_[i] > score) {  
        score = cached_relevance_[i];  
      }  
    }  
  }  
  return ret;  
}  
  
int ObTRDaaTTokenCacheIter::init_from_cache(ObTRDaaTTokenCacheIter *one)  
{  
  int ret = OB_SUCCESS;  
  if (OB_ISNULL(one) || !one->have_done_flag_) {  
    ret = OB_INVALID_ARGUMENT;  
    adlog::DEBUG("invalid cache template", K(ret));  
  } else {  
    if (OB_FAIL(cached_relevance_.assign(one->cached_relevance_))) {  
      LOG_WARN("failed to assign relevance array", K(ret));  
    } else if (OB_FAIL(cached_doc_id_.assign(one->cached_doc_id_))) {  
      LOG_WARN("failed to assign doc_id array", K(ret));  
    } else {  
      total_count_ = one->total_count_;  
      cur_idx_ = -1;  
      have_done_flag_ = true;
      is_created_flag_ = false;
      // adlog::DEBUG("缓存迭代器： 从cache拷贝过来了一个");
    }  
  }  
  return ret;  
}  
  
int ObTRDaaTTokenCacheIter::add_data(const ObFixedArray<double, ObIAllocator> &relevance,  
                                     const ObFixedArray<ObDocIdExt, ObIAllocator> &doc_id,  
                                     int64_t count)  
{
  // adlog::DEBUG("ObTRDaaTTokenCacheIter::add_data start");
  // adlog::DEBUG(K(count));
  int ret = OB_SUCCESS;  
  if (OB_UNLIKELY(count <= 0 || relevance.count() < count || doc_id.count() < count)) {  
    ret = OB_INVALID_ARGUMENT;  
    adlog::DEBUG("invalid count or array size", K(ret), K(count),   
             K(relevance.count()), K(doc_id.count()));  
  } else {  
      
    for (int64_t i = 0; OB_SUCC(ret) && i < count; ++i) {  
      if (OB_FAIL(cached_relevance_.push_back(relevance[i]))) {  
        adlog::DEBUG("failed to push back relevance", K(ret));  
      } else if (OB_FAIL(cached_doc_id_.push_back(doc_id[i]))) {  
        adlog::DEBUG("failed to push back doc_id", K(ret));  
      }  
    }  
      
    if (OB_SUCC(ret)) {  
      total_count_ += count;    
    }
  }
  // adlog::DEBUG("ObTRDaaTTokenCacheIter::add_data end");
  
  return ret;  
}  

} // namespace storage
} // namespace oceanbase
