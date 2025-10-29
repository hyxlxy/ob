/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include "sql/operator/logical_operator.h"

class Table;

/**
 * @brief 逻辑算子，用于执行update语句
 * @ingroup LogicalOperator
 */
class UpdateLogicalOperator : public LogicalOperator
{
public:
  explicit UpdateLogicalOperator(
      Table *table, std::vector<FieldMeta> field_metas, std::vector<std::unique_ptr<Expression>> values);
  ~UpdateLogicalOperator() override = default;

  LogicalOperatorType                       type() const override { return LogicalOperatorType::UPDATE; }
  Table                                    *table() const { return table_; }
  std::vector<FieldMeta>                   &field_metas() { return field_metas_; }
  std::vector<std::unique_ptr<Expression>> &values() { return values_; }

private:
  Table                                   *table_ = nullptr;
  std::vector<FieldMeta>                   field_metas_;
  std::vector<std::unique_ptr<Expression>> values_;
};
