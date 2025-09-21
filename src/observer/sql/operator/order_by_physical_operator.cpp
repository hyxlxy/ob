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

#include "sql/operator/order_by_physical_operator.h"
#include "common/log/log.h"
#include "sql/expr/tuple.h"
#include <algorithm>

OrderByPhysicalOperator::OrderByPhysicalOperator(std::vector<std::unique_ptr<Expression>> order_by_expressions,
                                                 std::vector<OrderDirection> order_by_directions)
    : order_by_expressions_(std::move(order_by_expressions)),
      order_by_directions_(std::move(order_by_directions))
{}

RC OrderByPhysicalOperator::open(Trx *trx)
{
  if (opened_) {
    return RC::SUCCESS;
  }

  if (children_.empty()) {
    LOG_WARN("order by operator has no child");
    return RC::INTERNAL;
  }

  PhysicalOperator *child = children_[0].get();
  RC rc = child->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open child operator: %s", strrc(rc));
    return rc;
  }

  // 获取并排序所有数据
  rc = fetch_and_sort_tuples();
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to fetch and sort tuples: %s", strrc(rc));
    return rc;
  }

  opened_ = true;
  return RC::SUCCESS;
}

RC OrderByPhysicalOperator::next()
{
  if (!opened_) {
    return RC::INTERNAL;
  }

  if (current_index_ >= sorted_tuples_.size()) {
    return RC::RECORD_EOF;
  }

  current_index_++;
  return RC::SUCCESS;
}

RC OrderByPhysicalOperator::close()
{
  if (!opened_) {
    return RC::SUCCESS;
  }

  sorted_tuples_.clear();
  current_index_ = 0;
  opened_ = false;

  if (!children_.empty()) {
    return children_[0]->close();
  }

  return RC::SUCCESS;
}

Tuple *OrderByPhysicalOperator::current_tuple()
{
  if (!opened_ || current_index_ == 0 || current_index_ > sorted_tuples_.size()) {
    return nullptr;
  }

  return sorted_tuples_[current_index_ - 1].second.get();
}

RC OrderByPhysicalOperator::fetch_and_sort_tuples()
{
  PhysicalOperator *child = children_[0].get();
  
  // 收集所有tuple和对应的排序键值
  while (true) {
    RC rc = child->next();
    if (rc == RC::RECORD_EOF) {
      break;
    }
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get next tuple from child: %s", strrc(rc));
      return rc;
    }

    Tuple *tuple = child->current_tuple();
    if (tuple == nullptr) {
      LOG_WARN("child operator returned null tuple");
      return RC::INTERNAL;
    }

    // 创建tuple的副本
    auto tuple_copy = std::make_unique<ValueListTuple>();

    // 复制tuple的所有cell
    vector<Value> cells;
    vector<TupleCellSpec> specs;
    for (int i = 0; i < tuple->cell_num(); i++) {
      Value value;
      rc = tuple->cell_at(i, value);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get cell value: %s", strrc(rc));
        return rc;
      }

      TupleCellSpec spec;
      rc = tuple->spec_at(i, spec);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get cell spec: %s", strrc(rc));
        return rc;
      }

      cells.push_back(value);
      specs.push_back(spec);
    }

    tuple_copy->set_cells(cells);
    tuple_copy->set_names(specs);

    // 计算排序键值
    std::vector<Value> sort_keys;
    for (const auto &order_expr : order_by_expressions_) {
      Value sort_value;
      rc = order_expr->get_value(*tuple, sort_value);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to evaluate order by expression: %s", strrc(rc));
        return rc;
      }
      sort_keys.push_back(sort_value);
    }

    sorted_tuples_.emplace_back(std::move(sort_keys), std::move(tuple_copy));
  }

  // 排序
  std::sort(sorted_tuples_.begin(), sorted_tuples_.end(),
            [this](const auto &left, const auto &right) {
              return compare_tuples(left, right);
            });

  return RC::SUCCESS;
}

bool OrderByPhysicalOperator::compare_tuples(
    const std::pair<std::vector<Value>, std::unique_ptr<Tuple>> &left,
    const std::pair<std::vector<Value>, std::unique_ptr<Tuple>> &right)
{
  const auto &left_keys = left.first;
  const auto &right_keys = right.first;

  for (size_t i = 0; i < order_by_directions_.size() && i < left_keys.size() && i < right_keys.size(); i++) {
    const Value &left_value = left_keys[i];
    const Value &right_value = right_keys[i];

    int cmp_result = left_value.compare(right_value);

    if (cmp_result != 0) {
      // 根据排序方向决定比较结果
      if (order_by_directions_[i] == ASC_ORDER) {
        return cmp_result < 0;
      } else {
        return cmp_result > 0;
      }
    }
  }

  return false;  // 所有键值都相等
}
