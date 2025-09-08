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

#include "sql/operator/sort_physical_operator.h"
#include "common/log/log.h"
#include "sql/expr/tuple.h"
#include <algorithm>

SortPhysicalOperator::SortPhysicalOperator(vector<unique_ptr<Expression>> order_by_expressions,
                                           vector<OrderDirection> order_by_directions)
    : order_by_expressions_(std::move(order_by_expressions)),
      order_by_directions_(std::move(order_by_directions))
{}

RC SortPhysicalOperator::open(Trx *trx)
{
  if (opened_) {
    return RC::SUCCESS;
  }

  if (children_.empty()) {
    LOG_WARN("sort operator has no child");
    return RC::INTERNAL;
  }

  PhysicalOperator *child = children_[0].get();
  RC rc = child->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open child operator: %s", strrc(rc));
    return rc;
  }

  // 获取tuple schema
  rc = child->tuple_schema(tuple_schema_);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get tuple schema from child: %s", strrc(rc));
    return rc;
  }

  // 收集所有的tuple
  sorted_tuples_.clear();
  while (RC::SUCCESS == (rc = child->next())) {
    Tuple *tuple = child->current_tuple();
    if (tuple != nullptr) {
      // 将tuple转换为ValueListTuple
      ValueListTuple value_tuple;
      vector<Value> cells;
      vector<TupleCellSpec> specs;

      for (int i = 0; i < tuple->cell_num(); i++) {
        Value cell;
        TupleCellSpec spec;
        rc = tuple->cell_at(i, cell);
        if (rc != RC::SUCCESS) {
          LOG_WARN("failed to get cell at %d: %s", i, strrc(rc));
          return rc;
        }
        rc = tuple->spec_at(i, spec);
        if (rc != RC::SUCCESS) {
          LOG_WARN("failed to get spec at %d: %s", i, strrc(rc));
          return rc;
        }
        cells.push_back(cell);
        specs.push_back(spec);
      }

      value_tuple.set_cells(cells);
      value_tuple.set_names(specs);
      sorted_tuples_.emplace_back(std::move(value_tuple));
    }
  }

  if (rc != RC::RECORD_EOF) {
    LOG_WARN("failed to get next tuple from child: %s", strrc(rc));
    return rc;
  }

  // 排序
  std::sort(sorted_tuples_.begin(), sorted_tuples_.end(),
            [this](const ValueListTuple &left, const ValueListTuple &right) {
              return compare_tuples(left, right);
            });

  current_index_ = 0;
  opened_ = true;
  return RC::SUCCESS;
}

RC SortPhysicalOperator::next()
{
  if (!opened_) {
    return RC::RECORD_EOF;
  }

  if (current_index_ >= sorted_tuples_.size()) {
    return RC::RECORD_EOF;
  }

  current_index_++;
  return RC::SUCCESS;
}

RC SortPhysicalOperator::close()
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

Tuple *SortPhysicalOperator::current_tuple()
{
  if (!opened_ || current_index_ == 0 || current_index_ > sorted_tuples_.size()) {
    return nullptr;
  }
  return &sorted_tuples_[current_index_ - 1];
}

RC SortPhysicalOperator::tuple_schema(TupleSchema &schema) const
{
  schema = tuple_schema_;
  return RC::SUCCESS;
}

bool SortPhysicalOperator::compare_tuples(const ValueListTuple &left, const ValueListTuple &right)
{
  for (size_t i = 0; i < order_by_expressions_.size(); i++) {
    Value left_value;
    Value right_value;

    RC rc = order_by_expressions_[i]->get_value(left, left_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get value from left tuple for order by expression %zu", i);
      return false;
    }

    rc = order_by_expressions_[i]->get_value(right, right_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get value from right tuple for order by expression %zu", i);
      return false;
    }

    int cmp_result = left_value.compare(right_value);
    if (cmp_result != 0) {
      if (order_by_directions_[i] == ASC_ORDER) {
        return cmp_result < 0;
      } else {
        return cmp_result > 0;
      }
    }
    // 如果相等，继续比较下一个字段
  }

  // 所有字段都相等
  return false;
}
