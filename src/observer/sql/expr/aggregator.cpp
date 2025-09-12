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
  if (value_.attr_type() == AttrType::UNDEFINED) {
    value_ = value;
    return RC::SUCCESS;
  }
  
  ASSERT(value.attr_type() == value_.attr_type(), "type mismatch. value type: %s, value_.type: %s", 
        attr_type_to_string(value.attr_type()), attr_type_to_string(value_.attr_type()));
  
  Value::add(value, value_, value_);
  return RC::SUCCESS;
}

RC SumAggregator::evaluate(Value& result)
{
  result = value_;
  return RC::SUCCESS;
}

RC CountAggregator::accumulate(const Value &value)
{
  // 安全的计数实现，不依赖传入的value内容
  LOG_INFO("DEBUG: CountAggregator::accumulate - before increment, count=%ld", count_);
  count_++;
  LOG_INFO("DEBUG: CountAggregator::accumulate - after increment, count=%ld", count_);
  // 不立即更新value_，在evaluate时再设置
  return RC::SUCCESS;
}

RC CountAggregator::evaluate(Value& result)
{
  // 安全地设置结果
  LOG_INFO("DEBUG: CountAggregator::evaluate - count=%ld", count_);
  result.set_int(static_cast<int>(count_));
  LOG_INFO("DEBUG: CountAggregator::evaluate - result set successfully");
  return RC::SUCCESS;
}

RC MaxAggregator::accumulate(const Value &value)
{
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

RC MaxAggregator::evaluate(Value& result)
{
  result = value_;
  return RC::SUCCESS;
}

RC MinAggregator::accumulate(const Value &value)
{
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

RC MinAggregator::evaluate(Value& result)
{
  result = value_;
  return RC::SUCCESS;
}

RC AvgAggregator::accumulate(const Value &value)
{
  Value::add(value, sum_, sum_);
  count_++;
  return RC::SUCCESS;
}

RC AvgAggregator::evaluate(Value& result)
{
  if (count_ == 0) {
    result.set_float(0.0f);
  } else {
    // 直接计算平均值，避免使用Value::divide
    float sum_val = sum_.get_float();
    float avg_val = sum_val / count_;
    result.set_float(avg_val);
  }
  return RC::SUCCESS;
}
