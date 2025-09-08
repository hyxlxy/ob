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

#include "sql/operator/physical_operator.h"
#include "sql/parser/parse_defs.h"
#include "sql/expr/expression.h"
#include "sql/expr/tuple.h"
#include <vector>
#include <memory>

/**
 * @brief 排序物理算子
 * @ingroup PhysicalOperator
 */
class SortPhysicalOperator : public PhysicalOperator
{
public:
  SortPhysicalOperator(vector<unique_ptr<Expression>> order_by_expressions,
                       vector<OrderDirection> order_by_directions);
  virtual ~SortPhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::SORT; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override;

  RC tuple_schema(TupleSchema &schema) const override;

private:
  vector<unique_ptr<Expression>> order_by_expressions_;
  vector<OrderDirection>         order_by_directions_;

  vector<ValueListTuple>         sorted_tuples_;
  size_t                         current_index_ = 0;
  bool                           opened_ = false;
  TupleSchema                    tuple_schema_;

  // 排序比较函数
  bool compare_tuples(const ValueListTuple &left, const ValueListTuple &right);
};
