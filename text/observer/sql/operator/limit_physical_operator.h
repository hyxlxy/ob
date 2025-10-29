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

#include "sql/operator/physical_operator.h"

/**
 * @brief LIMIT物理算子
 * @ingroup PhysicalOperator
 * @details 实现LIMIT操作，限制返回的记录数量
 */
class LimitPhysicalOperator : public PhysicalOperator
{
public:
  LimitPhysicalOperator(int limit_count);
  virtual ~LimitPhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::LIMIT; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override;

  int limit_count() const { return limit_count_; }

private:
  int limit_count_;      ///< 限制返回的记录数量
  int returned_count_;   ///< 已经返回的记录数量
};
