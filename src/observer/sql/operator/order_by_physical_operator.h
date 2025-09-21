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
#include "sql/parser/parse_defs.h"
#include "sql/expr/tuple.h"
#include "sql/expr/expression_tuple.h"
#include <functional>
#include <vector>

/**
 * @brief ORDER BY物理算子
 * @ingroup PhysicalOperator
 * @details 实现ORDER BY操作，对输入的tuple进行排序
 */
class OrderByPhysicalOperator : public PhysicalOperator
{
public:
  OrderByPhysicalOperator(std::vector<std::unique_ptr<Expression>> order_by_expressions,
                          std::vector<OrderDirection> order_by_directions);
  virtual ~OrderByPhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::ORDER_BY; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override;

private:
  /**
   * @brief 获取并排序所有数据
   */
  RC fetch_and_sort_tuples();

  /**
   * @brief 比较两个tuple的排序键值
   * @param left 左边的tuple和对应的排序键值
   * @param right 右边的tuple和对应的排序键值
   * @return true if left < right according to order by criteria
   */
  bool compare_tuples(const std::pair<std::vector<Value>, std::unique_ptr<Tuple>> &left,
                      const std::pair<std::vector<Value>, std::unique_ptr<Tuple>> &right);

private:
  std::vector<std::unique_ptr<Expression>> order_by_expressions_;  ///< 排序表达式列表
  std::vector<OrderDirection> order_by_directions_;                ///< 排序方向列表

  // 存储排序后的数据：每个元素包含排序键值和对应的tuple
  std::vector<std::pair<std::vector<Value>, std::unique_ptr<Tuple>>> sorted_tuples_;

  size_t current_index_ = 0;  ///< 当前返回的tuple索引
  bool opened_ = false;       ///< 是否已经打开
};
