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

#include "common/type/data_type.h"
#include <vector>
#include <sstream>
#include <iomanip>

// 向量最大维度限制（与测试约定：>2048 视为非法）
static const int MAX_VECTOR_DIMENSION = 2048;

/**
 * @brief 向量类型
 * @ingroup DataType
 */
class VectorType : public DataType
{
public:
  VectorType() : DataType(AttrType::VECTORS) {}
  virtual ~VectorType() {}

  int compare(const Value &left, const Value &right) const override;

  RC add(const Value &left, const Value &right, Value &result) const override;
  RC subtract(const Value &left, const Value &right, Value &result) const override;
  RC multiply(const Value &left, const Value &right, Value &result) const override;
  RC divide(const Value &left, const Value &right, Value &result) const override;
  RC negative(const Value &val, Value &result) const override;

  RC cast_to(const Value &val, AttrType type, Value &result) const override;
  RC set_value_from_str(Value &val, const string &data) const override;
  RC to_string(const Value &val, string &result) const override;
  int cast_cost(AttrType type) override;

  // 向量特有的方法
  static RC parse_vector_string(const string &str, std::vector<float> &vec);
  static RC vector_to_string(const std::vector<float> &vec, string &result);
  static RC l2_distance(const Value &left, const Value &right, Value &result);
  static RC cosine_distance(const Value &left, const Value &right, Value &result);
  static RC inner_product(const Value &left, const Value &right, Value &result);

private:
  static std::vector<float> get_vector_data(const Value &val);
  static void set_vector_data(Value &val, const std::vector<float> &vec);
};