#include "sql/das/iter/ob_das_two_phase_index_merge_iter.h"
#include "sql/engine/expr/ob_expr.h"
#include "lib/oblog/ob_log_module.h"

namespace oceanbase
{
namespace sql
{

ObDASTwoPhaseIndexMergeIter::ObDASTwoPhaseIndexMergeIter()  
  : ObDASIter(ObDASIterType::DAS_ITER_TWO_PHASE_INDEX_MERGE),  
    child_iters_(nullptr),  
    child_cnt_(0),  
    bmw_iter_(nullptr),  
    final_result_idx_(0),  
    state_(PhaseState::INIT),  
    min_score_(0.0),  
    first_stage_limit_(0),  
    second_stage_limit_(0),
    scalar_filters_(nullptr),
    match_against_exprs_(nullptr)
{  
  // 注：output_ 和 eval_ctx_ 由基类 ObDASIter 初始化
}  
  
ObDASTwoPhaseIndexMergeIter::~ObDASTwoPhaseIndexMergeIter()  
{  
}  
  
int ObDASTwoPhaseIndexMergeIter::inner_init(ObDASIterParam &param)  
{  
  int ret = OB_SUCCESS;  
  if (IS_INIT) {  
    ret = OB_INIT_TWICE;  
  } else {  
    TwoPhaseParam &two_phase_param = static_cast<TwoPhaseParam&>(param);  
      
    child_iters_ = two_phase_param.child_iters_;  
    child_cnt_ = two_phase_param.child_cnt_;  
    first_stage_limit_ = two_phase_param.first_stage_limit_;  
    second_stage_limit_ = two_phase_param.second_stage_limit_;
    scalar_filters_ = two_phase_param.scalar_filters_;
    match_against_exprs_ = two_phase_param.match_against_exprs_;
    // 从基类参数获取 eval_ctx_ 和 output_
    // 这些已经在 ObDASIter::init() 中设置
      
    if (OB_ISNULL(child_iters_) || child_cnt_ <= 0) {  
      ret = OB_ERR_UNEXPECTED;  
      LOG_WARN("invalid child iters", K(ret), K(child_cnt_));  
    } else if (OB_ISNULL(eval_ctx_) || OB_ISNULL(output_)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("invalid eval_ctx or output", K(ret));
    } else {  
      first_stage_allocator_.set_tenant_id(MTL_ID());  
      state_ = PhaseState::FIRST_STAGE;  
      is_inited_ = true;
      
      LOG_INFO("[BMW Two-Phase] inner_init complete",
               K_(first_stage_limit), K_(second_stage_limit),
               "has_scalar_filters", (scalar_filters_ != nullptr && !scalar_filters_->empty()),
               "has_match_exprs", (match_against_exprs_ != nullptr && !match_against_exprs_->empty()));
    }  
  }  
  return ret;  
}  
  
int ObDASTwoPhaseIndexMergeIter::inner_reuse()  
{  
  int ret = OB_SUCCESS;  
  first_stage_results_.reset();  
  final_results_.reset();  
  final_result_idx_ = 0;  
  min_score_ = 0.0;  
  state_ = PhaseState::FIRST_STAGE;  
    
  if (OB_NOT_NULL(bmw_iter_)) {  
    bmw_iter_->reuse();  
  }  
    
  for (int64_t i = 0; i < child_cnt_; i++) {  
    if (OB_NOT_NULL(child_iters_[i])) {  
      child_iters_[i]->reuse();  
    }  
  }  
  return ret;  
}  
  
int ObDASTwoPhaseIndexMergeIter::inner_release()  
{  
  int ret = OB_SUCCESS;  
  first_stage_results_.reset();  
  final_results_.reset();  
  first_stage_allocator_.reset();  
    
  if (OB_NOT_NULL(bmw_iter_)) {  
    bmw_iter_->release();  
    bmw_iter_ = nullptr;  
  }  
    
  for (int64_t i = 0; i < child_cnt_; i++) {  
    if (OB_NOT_NULL(child_iters_[i])) {  
      child_iters_[i]->release();  
    }  
  }  
    
  child_iters_ = nullptr;  
  child_cnt_ = 0;  
  return ret;  
}  
  
int ObDASTwoPhaseIndexMergeIter::inner_get_next_row()  
{  
  int ret = OB_SUCCESS;  
    
  switch (state_) {  
    case PhaseState::INIT:  
      ret = OB_ERR_UNEXPECTED;  
      break;  
        
    case PhaseState::FIRST_STAGE:  
      if (OB_FAIL(execute_first_stage())) {  
        LOG_WARN("failed to execute first stage", K(ret));  
      } else {  
        state_ = PhaseState::SECOND_STAGE;  
      }  
      break;  
        
    case PhaseState::SECOND_STAGE:  
      if (OB_FAIL(execute_second_stage())) {  
        LOG_WARN("failed to execute second stage", K(ret));  
      } else {  
        state_ = PhaseState::OUTPUT;  
      }  
      break;  
        
    case PhaseState::OUTPUT:  
      if (OB_FAIL(get_next_result_from_buffer())) {  
        if (OB_ITER_END != ret) {  
          LOG_WARN("failed to get next result", K(ret));  
        }  
      }  
      break;  
        
    case PhaseState::FINISHED:  
      ret = OB_ITER_END;  
      break;  
  }  
    
  return ret;  
}  
  
int ObDASTwoPhaseIndexMergeIter::execute_first_stage()  
{  
  int ret = OB_SUCCESS;  
  int64_t count = 0;  
    
  // 从所有子迭代器收集数据  
  while (OB_SUCC(ret) && count < first_stage_limit_) {  
    bool has_data = false;  
      
    for (int64_t i = 0; OB_SUCC(ret) && i < child_cnt_ && count < first_stage_limit_; i++) {  
      const ObDatumRow *row = nullptr;  
      if (OB_FAIL(child_iters_[i]->get_next_row(row))) {  
        if (OB_ITER_END == ret) {  
          ret = OB_SUCCESS;  
          continue;  
        } else {  
          LOG_WARN("failed to get next row from child iter", K(ret), K(i));  
        }  
      } else {  
        ScoredRow scored_row;  
        scored_row.row_ = row;  
        scored_row.score_ = calculate_score(row);  
          
        if (OB_FAIL(first_stage_results_.push_back(scored_row))) {  
          LOG_WARN("failed to push back scored row", K(ret));  
        } else {  
          // 更新最小分数  
          if (count == 0 || scored_row.score_ < min_score_) {  
            min_score_ = scored_row.score_;  
          }  
          count++;  
          has_data = true;  
        }  
      }  
    }  
      
    if (!has_data) {  
      break; // 所有迭代器都已结束  
    }  
  }  
    
  // 对第一阶段结果排序  
  if (OB_SUCC(ret) && !first_stage_results_.empty()) {  
    std::sort(first_stage_results_.begin(), first_stage_results_.end());  
  }  
    
  return ret;  
}  
  
int ObDASTwoPhaseIndexMergeIter::execute_second_stage()  
{  
  int ret = OB_SUCCESS;  
  int64_t bmw_count = 0;
  int64_t filtered_count = 0;
    
  // TODO: 创建 BMW 迭代器
  // 这里需要根据实际的 BMW 迭代器接口实现
  // 使用 min_score_ 作为阈值
  
  // 目前先从第一阶段结果中应用标量过滤
  // 完整的 BMW 集成将在后续实现
  
  LOG_INFO("[BMW Two-Phase] Phase2 starting",
           "first_stage_results", first_stage_results_.count(),
           K_(min_score), K_(second_stage_limit_));
  
  // 从第一阶段结果中应用标量过滤
  for (int64_t i = 0; OB_SUCC(ret) && i < first_stage_results_.count() 
                      && final_results_.count() < second_stage_limit_; i++) {
    const ScoredRow &scored_row = first_stage_results_[i];
    
    // 应用标量过滤
    if (apply_scalar_filter(scored_row.row_)) {
      if (OB_FAIL(final_results_.push_back(scored_row))) {
        LOG_WARN("[BMW Two-Phase] failed to push filtered result", K(ret));
      } else {
        filtered_count++;
      }
    }
  }
  
  // 如果过滤后结果不足，从第一阶段补充（不过滤）
  if (OB_SUCC(ret) && final_results_.count() < second_stage_limit_) {  
    int64_t need_count = second_stage_limit_ - final_results_.count();  
    int64_t copy_count = std::min(need_count, first_stage_results_.count());  
    
    LOG_INFO("[BMW Two-Phase] 补充保底结果",
             "filtered_count", filtered_count,
             "need_count", need_count,
             "copy_count", copy_count);
      
    for (int64_t i = 0; OB_SUCC(ret) && i < copy_count && final_results_.count() < second_stage_limit_; i++) {  
      // 跳过已经添加的结果
      bool already_added = false;
      for (int64_t j = 0; j < final_results_.count(); j++) {
        if (final_results_[j].row_ == first_stage_results_[i].row_) {
          already_added = true;
          break;
        }
      }
      
      if (!already_added) {
        if (OB_FAIL(final_results_.push_back(first_stage_results_[i]))) {  
          LOG_WARN("[BMW Two-Phase] failed to copy from first stage", K(ret));  
        }
      }
    }  
  }  
  
  // 对最终结果排序（按分数降序）
  if (OB_SUCC(ret) && !final_results_.empty()) {
    std::sort(final_results_.begin(), final_results_.end());
  }
  
  LOG_INFO("[BMW Two-Phase] Phase2 complete",
           "final_results", final_results_.count(),
           "filtered_count", filtered_count,
           "补充比例", (final_results_.count() > 0 ? 
                      (filtered_count * 100.0 / final_results_.count()) : 0.0));
    
  return ret;  
}  
  
double ObDASTwoPhaseIndexMergeIter::calculate_score(const ObDatumRow *row)  
{  
  // 从 MATCH AGAINST 表达式中提取分数
  double score = 0.0;
  
  if (OB_ISNULL(match_against_exprs_) || match_against_exprs_->empty()) {
    // 没有 MATCH AGAINST 表达式，返回 0.0
    return 0.0;
  }
  
  // 获取第一个 MATCH AGAINST 表达式（通常只有一个）
  ObExpr *score_expr = match_against_exprs_->at(0);
  if (OB_ISNULL(score_expr) || OB_ISNULL(eval_ctx_)) {
    return 0.0;
  }
  
  // 使用 locate_expr_datum 获取已经填充的 datum
  const ObDatum &score_datum = score_expr->locate_expr_datum(*eval_ctx_);
  
  // 处理 NULL：表示没有匹配
  if (score_datum.is_null()) {
    return 0.0;
  }
  
  // 提取分数
  if (score_datum.len_ == sizeof(double)) {
    score = score_datum.get_double();
  } else if (score_datum.len_ == 0) {
    // len=0 表示没有匹配
    score = 0.0;
  } else {
    // 其他情况，尝试读取
    score = 0.0;
    LOG_WARN("[BMW Two-Phase] invalid score datum length",
             "len", score_datum.len_, "expected", sizeof(double));
  }
  
  return score;
}  
  
int ObDASTwoPhaseIndexMergeIter::get_next_result_from_buffer()  
{  
  int ret = OB_SUCCESS;  
    
  if (final_result_idx_ >= final_results_.count()) {  
    ret = OB_ITER_END;  
  } else {  
    const ScoredRow &scored_row = final_results_[final_result_idx_];  
      
    // 将行数据设置到输出表达式  
    for (int64_t i = 0; OB_SUCC(ret) && i < output_->count(); i++) {  
      ObExpr *expr = output_->at(i);  
      expr->locate_expr_datum(*eval_ctx_) = scored_row.row_->storage_datums_[i];  
    }  
      
    final_result_idx_++;  
  }  
    
  return ret;  
}

bool ObDASTwoPhaseIndexMergeIter::apply_scalar_filter(const ObDatumRow *row)
{
  // 如果没有标量过滤条件，全部通过
  if (OB_ISNULL(scalar_filters_) || scalar_filters_->empty()) {
    return true;
  }
  
  if (OB_ISNULL(row) || OB_ISNULL(eval_ctx_) || OB_ISNULL(output_)) {
    return false;
  }
  
  // 将行数据设置到输出表达式
  for (int64_t i = 0; i < output_->count() && i < row->count_; i++) {
    ObExpr *expr = output_->at(i);
    if (OB_NOT_NULL(expr)) {
      expr->locate_expr_datum(*eval_ctx_) = row->storage_datums_[i];
      expr->get_eval_info(*eval_ctx_).evaluated_ = true;
    }
  }
  
  // 求值所有过滤条件
  for (int64_t i = 0; i < scalar_filters_->count(); i++) {
    ObExpr *filter = scalar_filters_->at(i);
    if (OB_ISNULL(filter)) {
      continue;
    }
    
    ObDatum *result = nullptr;
    int ret = filter->eval(*eval_ctx_, result);
    
    if (OB_FAIL(ret) || OB_ISNULL(result)) {
      return false; // 求值失败，过滤掉
    }
    
    if (result->is_null() || !result->get_bool()) {
      return false; // 过滤条件不满足
    }
  }
  
  return true; // 所有条件满足
}
  
int ObDASTwoPhaseIndexMergeIter::inner_get_next_rows(int64_t &count, int64_t capacity)  
{  
  int ret = OB_SUCCESS;  
  count = 0;  
    
  while (OB_SUCC(ret) && count < capacity) {  
    if (OB_FAIL(inner_get_next_row())) {  
      if (OB_ITER_END == ret) {  
        ret = OB_SUCCESS;  
        break;  
      }  
    } else {  
      count++;  
    }  
  }  
    
  return ret;  
}

} // namespace sql
} // namespace oceanbase