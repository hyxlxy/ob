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

#define USING_LOG_PREFIX STORAGE

#include "ob_text_daat_iter.h"
#include "sql/das/ob_das_ir_define.h"

namespace oceanbase
{
namespace storage
{
int ObTextDaaTIter::init(const ObTextDaaTParam &param)
{
  int ret = OB_SUCCESS;
  if (OB_ISNULL(param.base_param_) || OB_ISNULL(param.dim_iters_) || OB_ISNULL(param.allocator_)
      || OB_ISNULL(param.relevance_collector_)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("unexpected null pointer in param", K(ret), KP_(param.base_param),
             KP_(param.dim_iters), KP_(param.allocator), KP_(param.relevance_collector));
  } else if (OB_FAIL(ObSRDaaTIterImpl::init(*param.base_param_, *param.dim_iters_,
                                            *param.allocator_, *param.relevance_collector_))) {
    LOG_WARN("failed to init sr daat iter", K(ret));
  } else if (OB_FAIL(bm25_param_estimator_.init(param.bm25_param_est_ctx_))) {
    LOG_WARN("failed to init bm25 param estimator", K(ret));
  } else {
    mode_flag_ = param.mode_flag_;
    function_lookup_mode_ = param.function_lookup_mode_;
  }
  return ret;
}

void ObTextDaaTIter::reset()
{
  bm25_param_estimator_.reset();
  // this class can know the type of dim_iters_, but ObSRDaaTIterImpl maybe not
  if (OB_NOT_NULL(dim_iters_)) {
    for (int64_t i = 0; i < dim_iters_->count(); ++i) {
      static_cast<ObTextRetrievalDaaTTokenIter *>(dim_iters_->at(i))->reset();
    }
  }
  ObSRDaaTIterImpl::reset();
}

void ObTextDaaTIter::reuse(const bool switch_tablet)
{
  if ((OB_NOT_NULL(dim_iters_) && 0 == dim_iters_->count())) {
    // do nothing
  } else {
    bm25_param_estimator_.reuse(switch_tablet);
  }
  if (OB_NOT_NULL(dim_iters_)) {
    for (int64_t i = 0; i < dim_iters_->count(); ++i) {
      static_cast<ObTextRetrievalDaaTTokenIter *>(dim_iters_->at(i))->reuse();
    }
  }
  ObSRDaaTIterImpl::reuse(switch_tablet);
}

int ObTextDaaTIter::pre_process()
{
  int ret = OB_SUCCESS;
  if (dim_iters_->count() == 0) {
    ret = OB_ITER_END;
  } else if (iter_param_->need_project_relevance()) {
    if (OB_FAIL(bm25_param_estimator_.do_estimation(*iter_param_->eval_ctx_))) {
      LOG_WARN("failed to do bm25 param estimation", K(ret));
    }
  }
  return ret;
}

ObTextBMWIter::ObTextBMWIter()
  : ObSRBMWIterImpl(),
    bm25_param_estimator_(),
    valid_docids_(nullptr),
    hash_ready_(false)
{
}

void ObTextBMWIter::reuse(const bool switch_tablet)
{
  bm25_param_estimator_.reuse(switch_tablet);
  if (OB_NOT_NULL(dim_iters_)) {
    for (int64_t i = 0; i < dim_iters_->count(); ++i) {
      static_cast<ObTextRetrievalBlockMaxIter *>(dim_iters_->at(i))->reuse();
    }
  }
  if (valid_docid_hash_.created()) {
    (void)valid_docid_hash_.destroy();
  }
  hash_ready_ = false;
  ObSRBMWIterImpl::reuse(switch_tablet);
}

void ObTextBMWIter::reset()
{
  bm25_param_estimator_.reset();
  if (OB_NOT_NULL(dim_iters_)) {
    for (int64_t i = 0; i < dim_iters_->count(); ++i) {
      static_cast<ObTextRetrievalBlockMaxIter *>(dim_iters_->at(i))->reset();
    }
  }
  ObSRBMWIterImpl::reset();
}

int ObTextBMWIter::init(const ObTextDaaTParam &param)
{
  int ret = OB_SUCCESS;
  if (OB_ISNULL(param.base_param_) || OB_ISNULL(param.dim_iters_) || OB_ISNULL(param.allocator_)
      || OB_ISNULL(param.relevance_collector_)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("unexpected null pointer in param", K(ret), KP_(param.base_param),
             KP_(param.dim_iters), KP_(param.allocator), KP_(param.relevance_collector));
  } else if (OB_FAIL(ObSRBMWIterImpl::init(*param.base_param_, *param.dim_iters_,
                                           *param.allocator_, *param.relevance_collector_))) {
    LOG_WARN("failed to init sr bmw iter", K(ret));
  } else if (OB_FAIL(bm25_param_estimator_.init(param.bm25_param_est_ctx_))) {
    LOG_WARN("failed to init bm25 param estimator", K(ret));
  }
  return ret;
}

int ObTextBMWIter::get_next_rows(const int64_t capacity, int64_t &count)
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("not inited", K(ret));
  } else if (dim_iters_->count() == 0) {
    ret = OB_ITER_END;
  } else if (OB_FAIL(bm25_param_estimator_.do_estimation(*iter_param_->eval_ctx_))) {
    LOG_WARN("failed to do bm25 param estimation", K(ret));
  }

  if (FAILEDx(ObSRBMWIterImpl::get_next_rows(capacity, count))) {
    if (OB_UNLIKELY(OB_ITER_END != ret)) {
      LOG_WARN("failed to get next rows", K(ret));
    }
  }
  return ret;
}

int ObTextBMWIter::init_before_wand_process()
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(!bm25_param_estimator_.is_estimated())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("bm25 param not estimated", K(ret));
  }

  for (int64_t i = 0; OB_SUCC(ret) && i < dim_iters_->count(); ++i) {
    ObTextRetrievalBlockMaxIter *block_max_iter = static_cast<ObTextRetrievalBlockMaxIter *>(dim_iters_->at(i));
    if (OB_FAIL(block_max_iter->init_block_max_iter(
        bm25_param_estimator_.get_total_doc_cnt(), bm25_param_estimator_.get_avg_doc_token_cnt()))) {
      LOG_WARN("failed to init block max iter", K(ret));
    }
  }
  return ret;
}

int ObTextBMWIter::set_valid_docid_set(const common::ObIArray<sql::ObDocIdExt> *set)
{
  int ret = OB_SUCCESS;
  valid_docids_ = set;
  hash_ready_ = false;
  if (nullptr != valid_docids_ && valid_docids_->count() > 0) {
    // 构建hash，使用 ObDocIdExt::hash(uint64_t&) 作为key
    const int64_t bucket_num = valid_docids_->count() * 2 + 1;
    if (valid_docid_hash_.created()) {
      (void)valid_docid_hash_.destroy();
    }
    if (OB_SUCCESS != valid_docid_hash_.create(bucket_num, ObMemAttr(MTL_ID(), "TRDocIdHash"))) {
      // 构建失败不视为致命，回退到线性查找
      hash_ready_ = false;
    } else {
      bool hash_ok = true;
      for (int64_t i = 0; hash_ok && i < valid_docids_->count(); ++i) {
        uint64_t hv = 0;
        if (OB_SUCCESS != valid_docids_->at(i).hash(hv)) {
          hash_ok = false;
          break;
        } else if (OB_SUCCESS != valid_docid_hash_.set_refactored(hv, 1 /*overwrite*/)) {
          hash_ok = false;
          break;
        }
      }
      hash_ready_ = hash_ok;
      if (!hash_ready_) {
        (void)valid_docid_hash_.destroy();
      }
    }
  }
  return ret;
}

bool ObTextBMWIter::accept_docid(const ObDatum &id_datum, const double /*relevance*/)
{
  bool accept = true;
  if (nullptr != valid_docids_ && valid_docids_->count() > 0) {
    sql::ObDocIdExt tmp;
    if (OB_SUCCESS != tmp.from_datum(id_datum)) {
      accept = true; // 遇到异常时回退为接受，避免影响正确性
    } else if (hash_ready_) {
      uint64_t hv = 0;
      if (OB_SUCCESS != tmp.hash(hv)) {
        accept = true; // hash失败回退接受
      } else {
        int ret2 = valid_docid_hash_.exist_refactored(hv);
        accept = (ret2 == OB_HASH_EXIST);
      }
    } else {
      // 线性回退
      accept = false;
      for (int64_t i = 0; !accept && i < valid_docids_->count(); ++i) {
        if (tmp == valid_docids_->at(i)) {
          accept = true;
        }
      }
    }
  }
  return accept;
}

} // namespace storage
} // namespace oceanbase
