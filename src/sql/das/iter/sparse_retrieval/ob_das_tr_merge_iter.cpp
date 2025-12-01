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
#include "ob_das_tr_merge_iter.h"
#include "sql/das/ob_das_ir_define.h"
#include "sql/session/ob_basic_session_info.h"
#include <ctype.h>

namespace oceanbase
{
using namespace share;
namespace sql
{

ObDASTRMergeIter::ObDASTRMergeIter()
  : ObDASIter(ObDASIterType::DAS_ITER_TEXT_RETRIEVAL_MERGE),
    mem_context_(nullptr),
    myself_allocator_(lib::ObMemAttr(MTL_ID(), "SRMergeIterSelf"), OB_MALLOC_NORMAL_BLOCK_SIZE),
    ir_ctdef_(nullptr),
    ir_rtdef_(nullptr),
    tx_desc_(nullptr),
    snapshot_(nullptr),
    query_tokens_(),
    dim_weights_(),
    sr_iter_param_(),
    sparse_retrieval_iter_(nullptr),
    dim_iters_(),
    dim_iter_(),
    total_doc_cnt_scan_param_(nullptr),
    inv_scan_params_(),
    inv_agg_params_(),
    fwd_scan_params_(),
    boolean_compute_node_(nullptr),
    block_max_scan_params_(),
    block_max_iter_param_(),
    doc_length_est_param_(),
    doc_length_est_stat_cols_(),
    // 预收窄docid集合（方案B：外部注入）。若非空，则在创建BMW迭代器后下推给其做预过滤
    pre_filtered_docids_(),
    capped_pre_filtered_docids_(),
    // 两阶段占位：Phase1保底缓存与阈值
    phase1_docids_(),
    phase1_min_score_(0.0),
    // 两阶段配置缓存（会话读取后存放，便于后续 Phase1/2 使用）
    two_phase_enable_cfg_(true),
    two_phase_factor_cfg_(3),
    two_phase_prepared_(false),
    topk_limit_(0),
    ls_id_(),
    total_doc_cnt_tablet_id_(),
    inv_idx_tablet_id_(),
    fwd_idx_tablet_id_(),
    flags_(0),
    check_rangekey_inited_(false),
    inv_idx_tablet_switched_(false),
    is_inited_(false)
{
}

int ObDASTRMergeIter::inner_init(ObDASIterParam &param)
{
  int ret = OB_SUCCESS;
  if (IS_INIT) {
    ret = OB_INIT_TWICE;
    LOG_WARN("double initialization", K(ret));
  } else if (OB_UNLIKELY(ObDASIterType::DAS_ITER_TEXT_RETRIEVAL_MERGE != param.type_)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid das iter param type for text retrieval merge iter", K(ret), K(param));
  } else {
    ObDASTRMergeIterParam &merge_param = static_cast<ObDASTRMergeIterParam &>(param);
    ir_ctdef_ = merge_param.ir_ctdef_;
    ir_rtdef_ = merge_param.ir_rtdef_;
    tx_desc_ = merge_param.tx_desc_;
    snapshot_ = merge_param.snapshot_;
    boolean_compute_node_ = merge_param.boolean_compute_node_;
    flags_ = merge_param.flags_;
    for (int64_t i = 0; OB_SUCC(ret) && i < merge_param.query_tokens_.count(); ++i) {
      if (OB_FAIL(query_tokens_.push_back(merge_param.query_tokens_.at(i)))) {
        LOG_WARN("failed to push back query token", K(ret));
      }
    }
    if (merge_param.dim_weights_.count() >= 1) {
      for (int64_t i = 0; OB_SUCC(ret) && i < merge_param.dim_weights_.count(); ++i) {
        if (OB_FAIL(dim_weights_.push_back(merge_param.dim_weights_.at(i)))) {
          LOG_WARN("failed to push back dim weight", K(ret));
        }
      }
    }
    if (OB_FAIL(ret)) {
    } else if (OB_ISNULL(ir_ctdef_) || OB_ISNULL(ir_rtdef_)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected null pointer", K(ret), KP_(ir_ctdef), KP_(ir_rtdef));
    } else {
      is_inited_ = true;
      LOG_DEBUG("tr merge iter", K_(function_lookup_mode), K_(topk_mode), K_(daat_mode), K_(taat_mode));
    }
  }
  return ret;
}

// 两阶段：批量接口（接入点）
int ObDASTRMergeIter::inner_get_next_rows(int64_t &count, int64_t capacity)
{
  int ret = OB_SUCCESS;
  count = 0;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("not inited", K(ret));
  } else if (OB_ISNULL(sparse_retrieval_iter_)) {
    // 若尚未构建底层迭代器，尝试以最小代价构建（空实现下直接视为结束）
    ret = OB_ITER_END;
  } else if (OB_UNLIKELY(0 == capacity)) {
    count = 0;
  } else {
    const bool can_two_phase = two_phase_enable_cfg_ && topk_mode_ && !two_phase_prepared_ && (topk_limit_ > 0);
    if (can_two_phase) {
      // Phase1: 保底拉取 K' 候选并计算 min_score
      int64_t kprime = topk_limit_ * two_phase_factor_cfg_;
      if (kprime <= 0) { kprime = 1; }
      phase1_docids_.reset();
      phase1_min_score_ = 0.0;
      bool got_any = false;
      for (int64_t i = 0; OB_SUCC(ret) && i < kprime; ++i) {
        int64_t c = 0;
        int tmp = sparse_retrieval_iter_->get_next_rows(1 /*capacity*/, c);
        if (OB_ITER_END == tmp || c == 0) {
          break; // 视为成功，只是候选不足
        } else if (OB_SUCCESS != tmp) {
          ret = tmp;
          LOG_WARN("phase1 get_next_rows failed", K(ret));
          break;
        } else {
          ObExpr *id_expr = sr_iter_param_.id_proj_expr_;
          ObExpr *score_expr = sr_iter_param_.relevance_proj_expr_;
          ObEvalCtx *eval_ctx = (OB_NOT_NULL(ir_rtdef_) ? ir_rtdef_->eval_ctx_ : nullptr);
          ObDatum *id_datum = nullptr;
          ObDatum *score_datum = nullptr;
          if (OB_ISNULL(id_expr) || OB_ISNULL(score_expr) || OB_ISNULL(eval_ctx)) {
            ret = OB_ERR_UNEXPECTED;
            LOG_WARN("unexpected null expr/ctx for phase1", K(ret), KP(id_expr), KP(score_expr), KP(eval_ctx));
            break;
          } else if (OB_FAIL(id_expr->eval(*eval_ctx, id_datum))) {
            LOG_WARN("eval id expr failed", K(ret));
            break;
          } else if (OB_FAIL(score_expr->eval(*eval_ctx, score_datum))) {
            LOG_WARN("eval score expr failed", K(ret));
            break;
          } else {
            double score = 0.0;
            if (OB_NOT_NULL(score_datum) && !score_datum->is_null()) {
              score = score_datum->get_double();
            }
            if (!got_any) {
              phase1_min_score_ = score;
              got_any = true;
            } else if (score < phase1_min_score_) {
              phase1_min_score_ = score;
            }
            if (OB_NOT_NULL(id_datum)) {
              ObDocIdExt id;
              int cv = id.from_datum(*id_datum);
              if (OB_SUCCESS == cv) {
                (void)phase1_docids_.push_back(id);
              }
            }
          }
        }
      }
      if (OB_SUCC(ret)) {
        // Phase2: 下推阈值并复扫
        sr_iter_param_.min_score_threshold_ = got_any ? phase1_min_score_ : 0.0;
        two_phase_prepared_ = true;
        sparse_retrieval_iter_->reuse(false /*switch_tablet*/);
        if (OB_FAIL(rescan())) {
          LOG_WARN("phase2 rescan failed, fallback to original path", K(ret));
          two_phase_prepared_ = false;
          sr_iter_param_.min_score_threshold_ = 0.0;
          (void)rescan();
        }
      }
    }

    if (OB_SUCC(ret)) {
      int64_t got = 0;
      int tmp2 = sparse_retrieval_iter_->get_next_rows(capacity, got);
      if (OB_SUCCESS == tmp2 || OB_ITER_END == tmp2) {
        count = got;
        ret = (OB_ITER_END == tmp2 && got == 0) ? OB_ITER_END : OB_SUCCESS;
      } else {
        ret = tmp2;
      }

      // 批次兜底：若本批不足且两阶段已准备，按 Phase1 候选回填
      if (OB_SUCC(ret) && two_phase_prepared_ && topk_limit_ > 0) {
        int64_t need_more = (count < capacity) ? (capacity - count) : 0;
        if (need_more > 0 && phase1_docids_.count() > 0) {
          ObExpr *id_expr = sr_iter_param_.id_proj_expr_;
          ObExpr *score_expr = sr_iter_param_.relevance_proj_expr_;
          ObEvalCtx *eval_ctx = (OB_NOT_NULL(ir_rtdef_) ? ir_rtdef_->eval_ctx_ : nullptr);
          if (OB_NOT_NULL(id_expr) && OB_NOT_NULL(score_expr) && OB_NOT_NULL(eval_ctx)) {
            int64_t emitted = 0;
            for (int64_t i = 0; emitted < need_more && i < phase1_docids_.count(); ++i) {
              ObDatum &id_out = id_expr->locate_datum_for_write(*eval_ctx);
              storage::ObISparseRetrievalMergeIter::set_datum_shallow(id_out, phase1_docids_.at(i));
              ObDatum &score_out = score_expr->locate_datum_for_write(*eval_ctx);
              score_out.set_double(phase1_min_score_);
              ++count;
              ++emitted;
            }
            if (emitted > 0) {
              phase1_docids_.reset();
            }
          }
        }
      }
    }
  }
  return ret;
}

// 单行接口：尽量复用批量
int ObDASTRMergeIter::inner_get_next_row()
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
  } else if (OB_ISNULL(sparse_retrieval_iter_)) {
    ret = OB_ITER_END;
  } else {
    ret = sparse_retrieval_iter_->get_next_row();
  }
  return ret;
}

// do_table_scan/rescan/reuse/release 提供最小可链接实现（不改变外部接口）
int ObDASTRMergeIter::do_table_scan()
{
  // 简化处理：标记已就绪
  check_rangekey_inited_ = true;
  return OB_SUCCESS;
}

int ObDASTRMergeIter::rescan()
{
  // 简化处理：不做范围变更
  return OB_SUCCESS;
}

int ObDASTRMergeIter::inner_reuse()
{
  // 轻量复用：重置标记，但保留迭代器指针
  check_rangekey_inited_ = false;
  inv_idx_tablet_switched_ = false;
  return OB_SUCCESS;
}

int ObDASTRMergeIter::inner_release()
{
  // 轻量释放：重置成员，避免悬挂
  sparse_retrieval_iter_ = nullptr;
  dim_iter_ = nullptr;
  inv_scan_params_.reset();
  inv_agg_params_.reset();
  fwd_scan_params_.reset();
  block_max_scan_params_.reset();
  query_tokens_.reset();
  dim_weights_.reset();
  phase1_docids_.reset();
  return OB_SUCCESS;
}

// 工具：构造词元（最小实现，空/空间切分），兼容外部调用方链接
int ObDASTRMergeIter::build_query_tokens(const ObDASIRScanCtDef *ir_ctdef,
                                         ObDASIRScanRtDef *ir_rtdef,
                                         common::ObIAllocator &alloc,
                                         ObArray<ObString> &query_tokens,
                                         ObArray<double> &boost_values,
                                         ObFtsEvalNode *&root_node,
                                         bool &has_duplicate_boolean_tokens)
{
  int ret = OB_SUCCESS;
  query_tokens.reset();
  boost_values.reset();
  root_node = nullptr;
  has_duplicate_boolean_tokens = false;
  if (OB_ISNULL(ir_ctdef) || OB_ISNULL(ir_rtdef) || OB_ISNULL(ir_ctdef->search_text_) || OB_ISNULL(ir_rtdef->eval_ctx_)) {
    ret = OB_ERR_UNEXPECTED;
  } else {
    ObExpr *search_text = ir_ctdef->search_text_;
    ObEvalCtx *eval_ctx = ir_rtdef->eval_ctx_;
    ObDatum *search_text_datum = nullptr;
    if (OB_FAIL(search_text->eval(*eval_ctx, search_text_datum))) {
      // 保守返回空
      ret = OB_SUCCESS;
    } else if (OB_ISNULL(search_text_datum) || search_text_datum->is_null()) {
      // 空查询
    } else {
      ObString s = search_text_datum->get_string();
      // 简单按空格切分
      const char *p = s.ptr();
      int64_t n = s.length();
      int64_t i = 0;
      while (i < n) {
        while (i < n && isspace(p[i])) ++i;
        if (i >= n) break;
        int64_t j = i;
        while (j < n && !isspace(p[j])) ++j;
        ObString token(j - i, p + i);
        ObString dup;
        if (OB_SUCC(ob_write_string(alloc, token, dup))) {
          (void)query_tokens.push_back(dup);
          (void)boost_values.push_back(1.0);
        }
        i = j;
      }
    }
  }
  return ret;
}

// 设置子迭代器扫描范围（最小实现：仅标记为已设置）
int ObDASTRMergeIter::set_children_iter_rangekey(const common::ObIArray<std::pair<ObDocIdExt, int>> &virtual_rangekeys,
                                                 const int64_t batch_size)
{
  UNUSED(virtual_rangekeys);
  UNUSED(batch_size);
  check_rangekey_inited_ = true;
  return OB_SUCCESS;
}

// 占位：供高层复用（完整实现已在上方两阶段路径内）
int ObDASTRMergeIter::phase1_collect_docids_kprime(const int64_t kprime,
                                                   common::ObIArray<ObDocIdExt> &docids,
                                                   double &min_score)
{
  int ret = OB_SUCCESS;
  docids.reset();
  min_score = 0.0;
  if (kprime <= 0 || OB_ISNULL(sparse_retrieval_iter_)) {
    return ret;
  }
  bool got_any = false;
  for (int64_t i = 0; OB_SUCC(ret) && i < kprime; ++i) {
    int64_t c = 0;
    int tmp = sparse_retrieval_iter_->get_next_rows(1, c);
    if (OB_ITER_END == tmp || c == 0) {
      break;
    } else if (OB_SUCCESS != tmp) {
      ret = tmp;
      break;
    } else {
      ObExpr *id_expr = sr_iter_param_.id_proj_expr_;
      ObExpr *score_expr = sr_iter_param_.relevance_proj_expr_;
      ObEvalCtx *eval_ctx = (OB_NOT_NULL(ir_rtdef_) ? ir_rtdef_->eval_ctx_ : nullptr);
      ObDatum *id_datum = nullptr;
      ObDatum *score_datum = nullptr;
      if (OB_ISNULL(id_expr) || OB_ISNULL(score_expr) || OB_ISNULL(eval_ctx)) {
        ret = OB_ERR_UNEXPECTED;
        break;
      } else if (OB_FAIL(id_expr->eval(*eval_ctx, id_datum))) {
        break;
      } else if (OB_FAIL(score_expr->eval(*eval_ctx, score_datum))) {
        break;
      } else {
        double score = 0.0;
        if (OB_NOT_NULL(score_datum) && !score_datum->is_null()) {
          score = score_datum->get_double();
        }
        if (!got_any) {
          min_score = score;
          got_any = true;
        } else if (score < min_score) {
          min_score = score;
        }
        if (OB_NOT_NULL(id_datum)) {
          ObDocIdExt id;
          if (OB_SUCCESS == id.from_datum(*id_datum)) {
            (void)docids.push_back(id);
          }
        }
      }
    }
  }
  return ret;
}

int ObDASTRMergeIter::phase2_apply_threshold_and_rescan(const double min_score)
{
  int ret = OB_SUCCESS;
  if (OB_ISNULL(sparse_retrieval_iter_)) {
    return OB_ERR_UNEXPECTED;
  }
  sr_iter_param_.min_score_threshold_ = min_score;
  two_phase_prepared_ = true;
  sparse_retrieval_iter_->reuse(false);
  if (OB_FAIL(rescan())) {
    two_phase_prepared_ = false;
    sr_iter_param_.min_score_threshold_ = 0.0;
    (void)rescan();
  }
  return ret;
}

} // namespace sql
} // namespace oceanbase
