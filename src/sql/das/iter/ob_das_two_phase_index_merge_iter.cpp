#include "sql/das/iter/ob_das_two_phase_index_merge_iter.h"


ObDASTwoPhaseIndexMergeIter::ObDASTwoPhaseIndexMergeIter()  
  : ObDASIter(ObDASIterType::DAS_ITER_TWO_PHASE_INDEX_MERGE),  
    child_iters_(nullptr),  
    child_cnt_(0),  
    bmw_iter_(nullptr),  
    final_result_idx_(0),  
    state_(INIT),  
    min_score_(0.0),  
    first_stage_limit_(0),  
    second_stage_limit_(0)  
{  
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
      
    if (OB_ISNULL(child_iters_) || child_cnt_ <= 0) {  
      ret = OB_ERR_UNEXPECTED;  
      LOG_WARN("invalid child iters", K(ret), K(child_cnt_));  
    } else {  
      first_stage_allocator_.set_tenant_id(MTL_ID());  
      state_ = FIRST_STAGE;  
      is_inited_ = true;  
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
  state_ = FIRST_STAGE;  
    
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
    case INIT:  
      ret = OB_ERR_UNEXPECTED;  
      break;  
        
    case FIRST_STAGE:  
      if (OB_FAIL(execute_first_stage())) {  
        LOG_WARN("failed to execute first stage", K(ret));  
      } else {  
        state_ = SECOND_STAGE;  
      }  
      break;  
        
    case SECOND_STAGE:  
      if (OB_FAIL(execute_second_stage())) {  
        LOG_WARN("failed to execute second stage", K(ret));  
      } else {  
        state_ = OUTPUT;  
      }  
      break;  
        
    case OUTPUT:  
      if (OB_FAIL(get_next_result_from_buffer())) {  
        if (OB_ITER_END != ret) {  
          LOG_WARN("failed to get next result", K(ret));  
        }  
      }  
      break;  
        
    case FINISHED:  
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
    
  // TODO: 创建和执行BMW迭代器  
  // 这里需要根据实际的BMW迭代器接口实现  
  // 使用min_score_作为阈值  
    
  // 如果BMW结果不足，从第一阶段补充  
  if (final_results_.count() < second_stage_limit_) {  
    int64_t need_count = second_stage_limit_ - final_results_.count();  
    int64_t copy_count = min(need_count, first_stage_results_.count());  
      
    for (int64_t i = 0; OB_SUCC(ret) && i < copy_count; i++) {  
      if (OB_FAIL(final_results_.push_back(first_stage_results_[i]))) {  
        LOG_WARN("failed to copy from first stage", K(ret));  
      }  
    }  
  }  
    
  return ret;  
}  
  
double ObDASTwoPhaseIndexMergeIter::calculate_score(const ObDatumRow *row)  
{  
  // TODO: 根据业务需求实现分数计算逻辑  
  // 这里只是一个示例  
  return 0.0;  
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