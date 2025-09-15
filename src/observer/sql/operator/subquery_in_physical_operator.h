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

#pragma once

#include "sql/operator/physical_operator.h"
#include "sql/expr/expression.h"
#include "sql/stmt/stmt.h"

/**
 * @brief 子查询IN物理算子
 * @ingroup PhysicalOperator
 */
class SubQueryInPhysicalOperator : public PhysicalOperator
{
public:
  SubQueryInPhysicalOperator(unique_ptr<Expression> left_expr, 
                             unique_ptr<Stmt> sub_query, 
                             bool is_not_in = false);

  virtual ~SubQueryInPhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::PREDICATE; }
  OpType               get_op_type() const override { return OpType::FILTER; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override;

  RC tuple_schema(TupleSchema &schema) const override;

private:
  RC execute_sub_query(Trx *trx, std::vector<Value> &sub_query_results);
  RC evaluate_condition(const Tuple &tuple, bool &result);

private:
  unique_ptr<Expression> left_expr_;
  unique_ptr<Stmt> sub_query_;
  bool is_not_in_;
  std::vector<Value> sub_query_results_;
  bool sub_query_executed_;
  Trx *trx_;
};
