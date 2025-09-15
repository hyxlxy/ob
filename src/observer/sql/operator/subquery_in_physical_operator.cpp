/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Augment Agent on 2025/09/14.
//

#include "sql/operator/subquery_in_physical_operator.h"
#include "sql/stmt/select_stmt.h"
#include "sql/optimizer/logical_plan_generator.h"
#include "sql/optimizer/physical_plan_generator.h"
#include "session/session.h"
#include "common/log/log.h"

SubQueryInPhysicalOperator::SubQueryInPhysicalOperator(unique_ptr<Expression> left_expr, 
                                                       unique_ptr<Stmt> sub_query, 
                                                       bool is_not_in)
    : left_expr_(std::move(left_expr)), 
      sub_query_(std::move(sub_query)), 
      is_not_in_(is_not_in),
      sub_query_executed_(false),
      trx_(nullptr)
{
}

RC SubQueryInPhysicalOperator::open(Trx *trx)
{
  if (children_.empty()) {
    return RC::INVALID_ARGUMENT;
  }

  trx_ = trx;
  
  // Execute the sub-query first
  RC rc = execute_sub_query(trx, sub_query_results_);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to execute sub-query. rc=%s", strrc(rc));
    return rc;
  }
  sub_query_executed_ = true;

  // Open the child operator
  return children_[0]->open(trx);
}

RC SubQueryInPhysicalOperator::next()
{
  if (!sub_query_executed_) {
    return RC::INTERNAL;
  }

  RC rc = RC::SUCCESS;
  while (RC::SUCCESS == (rc = children_[0]->next())) {
    Tuple *tuple = children_[0]->current_tuple();
    if (nullptr == tuple) {
      continue;
    }

    bool condition_result = false;
    rc = evaluate_condition(*tuple, condition_result);
    if (rc != RC::SUCCESS) {
      return rc;
    }

    if (condition_result) {
      return RC::SUCCESS;
    }
  }

  return rc;
}

RC SubQueryInPhysicalOperator::close()
{
  sub_query_executed_ = false;
  sub_query_results_.clear();
  
  if (!children_.empty()) {
    return children_[0]->close();
  }
  return RC::SUCCESS;
}

Tuple *SubQueryInPhysicalOperator::current_tuple()
{
  if (children_.empty()) {
    return nullptr;
  }
  return children_[0]->current_tuple();
}

RC SubQueryInPhysicalOperator::tuple_schema(TupleSchema &schema) const
{
  if (children_.empty()) {
    return RC::INVALID_ARGUMENT;
  }
  return children_[0]->tuple_schema(schema);
}

RC SubQueryInPhysicalOperator::execute_sub_query(Trx *trx, std::vector<Value> &sub_query_results)
{
  if (!sub_query_) {
    return RC::INVALID_ARGUMENT;
  }

  // Create logical plan for sub-query
  LogicalPlanGenerator logical_plan_generator;
  unique_ptr<LogicalOperator> logical_operator;
  RC rc = logical_plan_generator.create(sub_query_.get(), logical_operator);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create logical plan for sub-query. rc=%s", strrc(rc));
    return rc;
  }

  // Create physical plan for sub-query
  PhysicalPlanGenerator physical_plan_generator;
  unique_ptr<PhysicalOperator> physical_operator;
  Session *session = Session::current_session();
  rc = physical_plan_generator.create(*logical_operator, physical_operator, session);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create physical plan for sub-query. rc=%s", strrc(rc));
    return rc;
  }

  // Execute the sub-query
  rc = physical_operator->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open sub-query physical operator. rc=%s", strrc(rc));
    return rc;
  }

  // Collect all results from the sub-query
  while (RC::SUCCESS == (rc = physical_operator->next())) {
    Tuple *tuple = physical_operator->current_tuple();
    if (tuple == nullptr) {
      continue;
    }

    // Assume the sub-query returns a single column
    Value value;
    rc = tuple->cell_at(0, value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get value from sub-query result. rc=%s", strrc(rc));
      physical_operator->close();
      return rc;
    }

    sub_query_results.push_back(value);
  }

  physical_operator->close();

  if (rc == RC::RECORD_EOF) {
    rc = RC::SUCCESS;
  }

  return rc;
}

RC SubQueryInPhysicalOperator::evaluate_condition(const Tuple &tuple, bool &result)
{
  if (!left_expr_) {
    return RC::INVALID_ARGUMENT;
  }

  Value left_value;
  RC rc = left_expr_->get_value(tuple, left_value);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  // Check if left_value is in sub_query_results
  bool found = false;
  for (const Value &sub_value : sub_query_results_) {
    if (left_value.compare(sub_value) == 0) {
      found = true;
      break;
    }
  }

  // Set result based on whether we found a match and the operation type
  result = found ? !is_not_in_ : is_not_in_;
  return RC::SUCCESS;
}
