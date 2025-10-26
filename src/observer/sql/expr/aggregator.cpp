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
// Created by Wangyunlai on 2024/05/29.
//

#include "sql/expr/aggregator.h"
#include "common/log/log.h"

RC SumAggregator::accumulate(const Value &value)
{
  // SUM函数应该跳过NULL值
  if (value.attr_type() == AttrType::UNDEFINED) {
    return RC::SUCCESS;
  }

  if (value_.attr_type() == AttrType::UNDEFINED) {
    value_ = value;
    return RC::SUCCESS;
  }

  ASSERT(value.attr_type() == value_.attr_type(), "type mismatch. value type: %s, value_.type: %s",
        attr_type_to_string(value.attr_type()), attr_type_to_string(value_.attr_type()));

  Value::add(value, value_, value_);
  return RC::SUCCESS;
}

RC SumAggregator::evaluate(Value &result)
{
  result = value_;
  return RC::SUCCESS;
}

RC CountAggregator::accumulate(const Value &value)
{
  // COUNT函数应该只计算非NULL值
  // 对于COUNT(*)，传入的value是常量1，不会是NULL
  // 对于COUNT(column)，需要检查value是否为NULL
  if (value.attr_type() != AttrType::UNDEFINED) {
    count_++;
  }
  return RC::SUCCESS;
}

RC CountAggregator::evaluate(Value &result)
{
  // 安全地设置结果
  result.set_int(static_cast<int>(count_));
  return RC::SUCCESS;
}

RC MaxAggregator::accumulate(const Value &value)
{
  // MAX函数应该跳过NULL值
  if (value.attr_type() == AttrType::UNDEFINED) {
    return RC::SUCCESS;
  }

  if (first_) {
    value_ = value;
    first_ = false;
  } else {
    if (value.compare(value_) > 0) {
      value_ = value;
    }
  }
  return RC::SUCCESS;
}

RC MaxAggregator::evaluate(Value &result)
{
  if (first_) {
    // 没有任何非NULL输入，返回 SQL NULL
    result.set_type(AttrType::UNDEFINED);
    result.set_null(true);
    return RC::SUCCESS;
  }
  result = value_;
  return RC::SUCCESS;
}

RC MinAggregator::accumulate(const Value &value)
{
  // MIN函数应该跳过NULL值
  if (value.attr_type() == AttrType::UNDEFINED) {
    return RC::SUCCESS;
  }

  if (first_) {
    value_ = value;
    first_ = false;
  } else {
    if (value.compare(value_) < 0) {
      value_ = value;
    }
  }
  return RC::SUCCESS;
}

RC MinAggregator::evaluate(Value &result)
{
  if (first_) {
    // 没有任何非NULL输入，返回 SQL NULL
    result.set_type(AttrType::UNDEFINED);
    result.set_null(true);
    return RC::SUCCESS;
  }
  result = value_;
  return RC::SUCCESS;
}

RC AvgAggregator::accumulate(const Value &value)
{
  // AVG函数应该跳过NULL值
  if (value.attr_type() == AttrType::UNDEFINED) {
    return RC::SUCCESS;
  }

  Value::add(value, sum_, sum_);
  count_++;
  return RC::SUCCESS;
}

RC AvgAggregator::evaluate(Value &result)
{
  if (count_ == 0) {
    // MySQL 8.0标准：当所有值都是NULL时，AVG返回NULL
    result.set_type(AttrType::UNDEFINED);
    result.set_null(true);
  } else {
    // 直接计算平均值，避免使用Value::divide
    float sum_val = sum_.get_float();
    float avg_val = sum_val / count_;
    result.set_float(avg_val);
  }
  return RC::SUCCESS;
}
