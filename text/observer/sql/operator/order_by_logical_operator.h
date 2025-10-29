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
// Created by AI Assistant on 2024/12/20.
//

#pragma once

#include "sql/operator/logical_operator.h"
#include "sql/parser/parse_defs.h"
#include "sql/expr/expression.h"

/**
 * @brief ORDER BY逻辑算子
 * @ingroup LogicalOperator
 * @details 用于表示ORDER BY操作的逻辑计划节点
 */
class OrderByLogicalOperator : public LogicalOperator
{
public:
  OrderByLogicalOperator(std::vector<std::unique_ptr<Expression>> order_by_expressions,
                         std::vector<OrderDirection> order_by_directions);
  virtual ~OrderByLogicalOperator() = default;

  LogicalOperatorType type() const override { return LogicalOperatorType::ORDER_BY; }

  const std::vector<std::unique_ptr<Expression>> &order_by_expressions() const { return order_by_expressions_; }
  std::vector<std::unique_ptr<Expression>> &order_by_expressions() { return order_by_expressions_; }

  const std::vector<OrderDirection> &order_by_directions() const { return order_by_directions_; }
  std::vector<OrderDirection> &order_by_directions() { return order_by_directions_; }

private:
  std::vector<std::unique_ptr<Expression>> order_by_expressions_;  ///< 排序表达式列表
  std::vector<OrderDirection> order_by_directions_;                ///< 排序方向列表
};
