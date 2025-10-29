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

#pragma once

#include "common/value.h"
#include "common/sys/rc.h"

class Aggregator
{
public:
  virtual ~Aggregator() = default;

  virtual RC accumulate(const Value &value) = 0;
  virtual RC evaluate(Value &result)        = 0;

protected:
  Value value_;
};

class SumAggregator : public Aggregator
{
public:
  RC accumulate(const Value &value) override;
  RC evaluate(Value &result) override;
};

class CountAggregator : public Aggregator
{
public:
  CountAggregator() {
    value_.set_int(0);
    count_ = 0; // 明确初始化计数器
  }

  // 添加安全检查
  bool validate() const {
    return value_.attr_type() == AttrType::INTS && count_ >= 0;
  }

  RC accumulate(const Value &value) override;
  RC evaluate(Value &result) override;

private:
  int64_t count_ = 0; // 独立的计数器，防止Value操作问题
};

class MaxAggregator : public Aggregator
{
public:
  RC accumulate(const Value &value) override;
  RC evaluate(Value &result) override;
private:
  bool first_ = true;
};

class MinAggregator : public Aggregator
{
public:
  RC accumulate(const Value &value) override;
  RC evaluate(Value &result) override;
private:
  bool first_ = true;
};

class AvgAggregator : public Aggregator
{
public:
  AvgAggregator() { sum_.set_float(0.0f); count_ = 0; }
  RC accumulate(const Value &value) override;
  RC evaluate(Value &result) override;
private:
  Value sum_;
  int count_ = 0;
};
