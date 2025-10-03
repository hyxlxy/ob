/***************************************************************
 *                                                             *
 * @Author      : Koschei                                      *
 * @Email       : nitianzero@gmail.com                         *
 * @Date        : 2024/10/13                                     *
 * @Description : Brief description of the file's purpose      *
 *                                                             *
 * Copyright (c) 2024 Koschei                                  *
 * All rights reserved.                                        *
 *                                                             *
 ***************************************************************/

#include "sql/operator/view_scan_physical_operator.h"
#include "event/sql_debug.h"
#include "storage/table/table.h"
#include "sql/expr/expression_tuple.h"
#include "storage/table/view.h"
#include "sql/stmt/select_stmt.h"
#include "sql/optimizer/logical_plan_generator.h"
#include "sql/optimizer/physical_plan_generator.h"

RC ViewScanPhysicalOperator::init()
{
  // 暂时简化：视图扫描未完全实现
  LOG_WARN("View scan not fully implemented");
  return RC::UNIMPLEMENTED;
}

RC ViewScanPhysicalOperator::open(Trx *trx)
{
  RC rc = RC::SUCCESS;

  if (select_expr_ == nullptr) {
    rc = init();
    if (OB_FAIL(rc)) {
      return rc;
    }
  }

  rc = select_expr_->open(trx);
  if (rc == RC::SUCCESS) {
    // View::table_meta()不存在，暂时跳过
    // tuple_.set_schema(view_, nullptr);
  }
  trx_ = trx;
  return rc;
}

RC ViewScanPhysicalOperator::next()
{
  RC rc = RC::SUCCESS;

  bool filter_result = false;
  while (OB_SUCC(rc = next_record())) {
    LOG_TRACE("got a record. rid=%s", current_record_.rid().to_string().c_str());

    tuple_.set_record(&current_record_);
    rc = filter(tuple_, filter_result);
    if (rc != RC::SUCCESS) {
      LOG_TRACE("record filtered failed=%s", strrc(rc));
      return rc;
    }

    if (filter_result) {
      sql_debug("get a tuple: %s", tuple_.to_string().c_str());
      break;
    } else {
      sql_debug("a tuple is filtered: %s", tuple_.to_string().c_str());
    }
  }
  return rc;
}

RC ViewScanPhysicalOperator::close() { return select_expr_->close(); }

Tuple *ViewScanPhysicalOperator::current_tuple() { return &tuple_; }

string ViewScanPhysicalOperator::param() const { return view_->name(); }

void ViewScanPhysicalOperator::set_predicates(vector<unique_ptr<Expression>> &&exprs)
{
  predicates_ = std::move(exprs);
}

RC ViewScanPhysicalOperator::filter(RowTuple &tuple, bool &result)
{
  RC          rc = RC::SUCCESS;
  Value       value;
  Tuple      *tp = &tuple;
  JoinedTuple jt;
  jt.set_left(&tuple);
  jt.set_right(const_cast<Tuple *>(parent_tuple_));
  if (parent_tuple_) {
    tp = &jt;
  }
  for (unique_ptr<Expression> &expr : predicates_) {
    rc = expr->get_value(*tp, value);
    if (rc != RC::SUCCESS) {
      close();
      return rc;
    }

    bool tmp_result = value.get_boolean();
    if (!tmp_result) {
      result = false;
      return rc;
    }
  }

  result = true;
  return rc;
}

RC ViewScanPhysicalOperator::next_record()
{
  RC rc = select_expr_->next();
  if (OB_FAIL(rc)) {
    return rc;
  }

  auto tuple = select_expr_->current_tuple();
  // RowTuple::set_base_rids()不存在，暂时跳过
  // 构造一个 view 的 record
  int                cell_num = tuple->cell_num();
  std::vector<Value> values(cell_num);
  for (int i = 0; i < cell_num; i++) {
    Value cell;
    rc = tuple->cell_at(i, cell);
    if (OB_FAIL(rc)) {
      return rc;
    }
    values[i] = std::move(cell);
  }

  // View::make_record()不存在，暂时跳过
  return RC::UNIMPLEMENTED;
}
