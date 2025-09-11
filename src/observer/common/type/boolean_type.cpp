/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/type/boolean_type.h"

#include "common/lang/comparator.h"
#include "common/lang/sstream.h"
#include "common/log/log.h"
#include "common/value.h"

int BooleanType::compare(const Value &left, const Value &right) const
{
  // allow comparing with numeric/char by truthiness
  bool l = left.get_boolean();
  bool r = right.get_boolean();
  if (l == r) return 0;
  return l ? 1 : -1;
}

RC BooleanType::cast_to(const Value &val, AttrType type, Value &result) const
{
  bool b = val.get_boolean();
  switch (type) {
    case AttrType::BOOLEANS: {
      result.set_boolean(b);
      return RC::SUCCESS;
    }
    case AttrType::INTS: {
      result.set_int(b ? 1 : 0);
      return RC::SUCCESS;
    }
    case AttrType::FLOATS: {
      result.set_float(b ? 1.0f : 0.0f);
      return RC::SUCCESS;
    }
    case AttrType::CHARS: {
      result.set_string(b ? "1" : "0");
      return RC::SUCCESS;
    }
    default: {
      LOG_WARN("unsupported cast from boolean to type %d", type);
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }
  }
}

RC BooleanType::set_value_from_str(Value &val, const string &data) const
{
  // accept 1/0, true/false (case-insensitive), and numeric strings
  string s = data;
  // to lower
  for (auto &ch : s) ch = static_cast<char>(::tolower(ch));
  if (s == "1" || s == "true") {
    val.set_boolean(true);
    return RC::SUCCESS;
  }
  if (s == "0" || s == "false") {
    val.set_boolean(false);
    return RC::SUCCESS;
  }
  // Fallback: non-zero numeric => true
  try {
    float f = stof(s);
    val.set_boolean(f >= EPSILON || f <= -EPSILON);
    return RC::SUCCESS;
  } catch (...) {
    // Non-numeric string: treat non-empty as true
    val.set_boolean(!s.empty());
    return RC::SUCCESS;
  }
}

RC BooleanType::to_string(const Value &val, string &result) const
{
  result = val.get_boolean() ? "1" : "0";
  return RC::SUCCESS;
}

int BooleanType::cast_cost(AttrType type)
{
  if (type == AttrType::BOOLEANS) return 0;
  if (type == AttrType::INTS || type == AttrType::FLOATS || type == AttrType::CHARS) return 1;
  return INT32_MAX;
}

