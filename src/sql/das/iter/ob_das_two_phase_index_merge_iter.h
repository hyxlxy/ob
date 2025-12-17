// 头文件：ob_das_two_phase_index_merge_iter.h

#include "sql/das/iter/ob_das_index_merge_iter.h"
#include "sql/das/iter/ob_das_iter.h"
#include <cstdint>
#include <cstdio>

class ObDASTwoPhaseIndexMergeIter : public ObDASIter  
{  
public:  
  struct TwoPhaseParam : public ObDASIterParam  
  {  
    TwoPhaseParam() : ObDASIterParam(DAS_ITER_TWO_PHASE_INDEX_MERGE),  
                      first_stage_limit_(0),  
                      second_stage_limit_(0),  
                      child_iters_(nullptr),  
                      child_cnt_(0) {}  
      
    int64_t first_stage_limit_;    // 第一阶段限制(3*n)  
    int64_t second_stage_limit_;   // 第二阶段top-n  
    ObDASIter **child_iters_;      // 子迭代器数组  
    int64_t child_cnt_;            // 子迭代器数量  
  };  
  
  struct ScoredRow  
  {  
    const ObDatumRow *row_;  
    double score_;  
    bool operator<(const ScoredRow &other) const { return score_ > other.score_; }  
  };  
  
  ObDASTwoPhaseIndexMergeIter();  
  virtual ~ObDASTwoPhaseIndexMergeIter();  
  
protected:  
  virtual int inner_init(ObDASIterParam &param) override;  
  virtual int inner_reuse() override;  
  virtual int inner_release() override;  
  virtual int inner_get_next_row() override;  
  virtual int inner_get_next_rows(int64_t &count, int64_t capacity) override;  
  
private:  
  // 第一阶段执行  
  int execute_first_stage();  
  // 第二阶段执行  
  int execute_second_stage();  
  // 计算行分数  
  double calculate_score(const ObDatumRow *row);  
  // 从缓冲区获取下一个结果  
  int get_next_result_from_buffer();  
  
  // 子迭代器管理  
  ObDASIter **child_iters_;  
  int64_t child_cnt_;  
    
  // 第一阶段结果存储  
  common::ObSEArray<ScoredRow, 1024> first_stage_results_;  
  common::ObArenaAllocator first_stage_allocator_;  
    
  // 第二阶段BMW迭代器  
  ObDASIter *bmw_iter_;  
    
  // 最终结果缓冲区  
  common::ObSEArray<ScoredRow, 1024> final_results_;  
  int64_t final_result_idx_;  
    
  enum class PhaseState {  
    INIT,  
    FIRST_STAGE,  
    SECOND_STAGE,  
    OUTPUT,  
    FINISHED  
  };  
  PhaseState state_;  
    
  double min_score_;  
  int64_t first_stage_limit_;  
  int64_t second_stage_limit_;  
};