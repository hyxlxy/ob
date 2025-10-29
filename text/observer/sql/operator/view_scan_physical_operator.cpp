#include "sql/operator/view_scan_physical_operator.h"
#include "common/log/log.h"

std::string ViewScanPhysicalOperator::param() const { return "ViewScan"; }

RC ViewScanPhysicalOperator::open(Trx *trx) { 
  return RC::UNIMPLEMENTED; 
}

RC ViewScanPhysicalOperator::next() { 
  return RC::RECORD_EOF; 
}

RC ViewScanPhysicalOperator::close() { 
  return RC::SUCCESS; 
}

Tuple *ViewScanPhysicalOperator::current_tuple() { 
  return &tuple_; 
}

void ViewScanPhysicalOperator::set_predicates(std::vector<std::unique_ptr<Expression>> &&exprs) {
  // 暂未实现
}

RC ViewScanPhysicalOperator::filter(RowTuple &tuple, bool &result) {
  result = true;
  return RC::SUCCESS;
}
