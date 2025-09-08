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
// Created by Wangyunlai on 2022/12/07.
//

#pragma once

#include "sql/operator/logical_operator.h"
#include "sql/parser/parse_defs.h"
#include "sql/expr/expression.h"
#include <vector>
#include <memory>

/**
 * @brief 排序逻辑算子
 * @ingroup LogicalOperator
 */
class SortLogicalOperator : public LogicalOperator
{
public:
  SortLogicalOperator(vector<unique_ptr<Expression>> order_by_expressions, 
                      vector<OrderDirection> order_by_directions);
  virtual ~SortLogicalOperator() = default;

  LogicalOperatorType type() const override { return LogicalOperatorType::SORT; }

  vector<unique_ptr<Expression>>       &order_by_expressions() { return order_by_expressions_; }
  const vector<unique_ptr<Expression>> &order_by_expressions() const { return order_by_expressions_; }
  vector<OrderDirection>               &order_by_directions() { return order_by_directions_; }
  const vector<OrderDirection>         &order_by_directions() const { return order_by_directions_; }

private:
  vector<unique_ptr<Expression>> order_by_expressions_;
  vector<OrderDirection>         order_by_directions_;
};
