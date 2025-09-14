/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/lang/comparator.h"
#include "common/log/log.h"
#include "common/type/char_type.h"
#include "common/value.h"

int CharType::compare(const Value &left, const Value &right) const
{
  ASSERT(left.attr_type() == AttrType::CHARS, "left type is not chars");
  if (right.attr_type() == AttrType::CHARS) {
    return common::compare_string(
        (void *)left.value_.pointer_value_, left.length_, (void *)right.value_.pointer_value_, right.length_);
  } else if (right.attr_type() == AttrType::INTS) {
    // 将字符串转换为整数进行比较
    int left_val = left.get_int();
    int right_val = right.get_int();
    return common::compare_int((void *)&left_val, (void *)&right_val);
  } else if (right.attr_type() == AttrType::FLOATS) {
    // 将字符串转换为浮点数进行比较
    float left_val = left.get_float();
    float right_val = right.get_float();
    return common::compare_float((void *)&left_val, (void *)&right_val);
  }
  return INT32_MAX;
}

RC CharType::set_value_from_str(Value &val, const string &data) const
{
  val.set_string(data.c_str());
  return RC::SUCCESS;
}

RC CharType::cast_to(const Value &val, AttrType type, Value &result) const
{
  switch (type) {
    case AttrType::CHARS: {
      result.set_string(val.get_string().c_str());
      return RC::SUCCESS;
    }
    case AttrType::INTS: {
      // 字符串转整数
      result.set_int(val.get_int());
      return RC::SUCCESS;
    }
    case AttrType::FLOATS: {
      // 字符串转浮点数
      result.set_float(val.get_float());
      return RC::SUCCESS;
    }
    case AttrType::DATES: {
      // 使用DateType的set_value_from_str方法来转换
      return DataType::type_instance(AttrType::DATES)->set_value_from_str(result, val.get_string());
    }
    default: return RC::UNIMPLEMENTED;
  }
  return RC::SUCCESS;
}

int CharType::cast_cost(AttrType type)
{
  if (type == AttrType::CHARS) {
    return 0;
  }
  if (type == AttrType::DATES) {
    return 1;  // 支持从字符串转换到日期
  }
  if (type == AttrType::INTS) {
    return 2;  // 支持从字符串转换到整数，但成本较高
  }
  if (type == AttrType::FLOATS) {
    return 2;  // 支持从字符串转换到浮点数，但成本较高
  }
  return INT32_MAX;
}

RC CharType::to_string(const Value &val, string &result) const
{
  stringstream ss;
  ss << val.value_.pointer_value_;
  result = ss.str();
  return RC::SUCCESS;
}