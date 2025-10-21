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
// Created by AI Assistant on 2024/12/19.
//

#pragma once

#include "sql/operator/logical_operator.h"
#include "common/value.h"

/**
 * @brief UNION逻辑算子
 * @ingroup LogicalOperator
 * @details UNION逻辑算子用于合并多个SELECT语句的结果集
 * 它位于projection算子之下，有多个子算子（每个SELECT对应一个子算子）
 */
class UnionLogicalOperator : public LogicalOperator
{
public:
  UnionLogicalOperator(const vector<bool> &distinct_flags,
                      const vector<AttrType> &result_types,
                      const vector<string> &result_names);
  
  virtual ~UnionLogicalOperator() = default;

  LogicalOperatorType type() const override { return LogicalOperatorType::UNION; }
  OpType get_op_type() const override { return OpType::UNION; }

  // 访问器方法
  const vector<bool> &distinct_flags() const { return distinct_flags_; }
  const vector<AttrType> &result_types() const { return result_types_; }
  const vector<string> &result_names() const { return result_names_; }

private:
  vector<bool> distinct_flags_;     ///< 每个UNION操作的标志位，true表示去重，false表示不去重
  vector<AttrType> result_types_;    ///< 结果集列类型（由第一个SELECT决定）
  vector<string> result_names_;      ///< 结果集列名（由第一个SELECT决定）
};
