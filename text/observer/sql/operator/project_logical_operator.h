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
// Created by WangYunlai on 2022/12/08.
//

#pragma once

#include "sql/expr/expression.h"
#include "sql/operator/logical_operator.h"
#include "storage/field/field.h"
class View;

/**
 * @brief project 表示投影运算
 * @ingroup LogicalOperator
 * @details 从表中获取数据后，可能需要过滤，投影，连接等等。
 */
class ProjectLogicalOperator : public LogicalOperator
{
public:
  //这里面的expressions是select里面的我们要查询的表达式，所以别名都存到了这个里面。
  ProjectLogicalOperator(vector<unique_ptr<Expression>> &&expressions);
  virtual ~ProjectLogicalOperator() = default;

  LogicalOperatorType         type() const override { return LogicalOperatorType::PROJECTION; }
  OpType                      get_op_type() const override { return OpType::LOGICALPROJECTION; }
  unique_ptr<LogicalProperty> find_log_prop(const vector<LogicalProperty *> &log_props) override;

  vector<unique_ptr<Expression>>       &expressions() { return expressions_; }
  const vector<unique_ptr<Expression>> &expressions() const { return expressions_; }
  void                                  set_view(View *view) { view_ = view; }
  View                                 *view_ = nullptr;
};
