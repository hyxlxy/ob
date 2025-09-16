/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/type/char_type.h"
#include "common/type/float_type.h"
#include "common/type/integer_type.h"
#include "common/type/data_type.h"
#include "common/type/vector_type.h"
#include "common/type/boolean_type.h"
#include "common/value.h"
#include "common/log/log.h"
#include <sstream>
#include <iomanip>

array<unique_ptr<DataType>, static_cast<int>(AttrType::MAXTYPE)> DataType::type_instances_ = {
    make_unique<UndefinedType>(),
    make_unique<CharType>(),
    make_unique<IntegerType>(),
    make_unique<FloatType>(),
    make_unique<DateType>(),
    make_unique<VectorType>(),
    make_unique<BooleanType>(),
};

// DateType implementation
int DateType::compare(const Value &left, const Value &right) const
{
  ASSERT(left.attr_type() == AttrType::DATES && right.attr_type() == AttrType::DATES,
         "invalid type");

  int left_date = left.get_date();
  int right_date = right.get_date();

  if (left_date < right_date) {
    return -1;
  } else if (left_date > right_date) {
    return 1;
  } else {
    return 0;
  }
}

RC DateType::cast_to(const Value &val, AttrType type, Value &result) const
{
  switch (type) {
    case AttrType::CHARS: {
      string date_str;
      RC rc = to_string(val, date_str);
      if (rc != RC::SUCCESS) {
        return rc;
      }
      result.set_string(date_str.c_str());
      return RC::SUCCESS;
    }
    default:
      return RC::UNSUPPORTED;
  }
}

int DateType::cast_cost(AttrType type)
{
  if (type == AttrType::DATES) {
    return 0;
  }
  if (type == AttrType::CHARS) {
    return 1;  // 支持从字符串转换到日期
  }
  return INT32_MAX;
}

RC DateType::set_value_from_str(Value &val, const string &data) const
{
  int date_int;
  RC rc = parse_date_string(data, date_int);
  if (rc != RC::SUCCESS) {
    return rc;
  }
  val.set_date(date_int);
  return RC::SUCCESS;
}

RC DateType::to_string(const Value &val, string &result) const
{
  ASSERT(val.attr_type() == AttrType::DATES, "invalid type");
  return format_date_int(val.get_date(), result);
}

bool DateType::is_leap_year(int year)
{
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int DateType::days_in_month(int year, int month)
{
  static const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if (month < 1 || month > 12) {
    return 0;
  }

  if (month == 2 && is_leap_year(year)) {
    return 29;
  }

  return days[month];
}

bool DateType::is_valid_date(int year, int month, int day)
{
  if (year < 1000 || year > 9999) {
    return false;
  }

  if (month < 1 || month > 12) {
    return false;
  }

  if (day < 1 || day > days_in_month(year, month)) {
    return false;
  }

  return true;
}

RC DateType::parse_date_string(const string &date_str, int &date_int)
{
  // 支持多种日期格式: YYYY-MM-DD, YYYY-M-DD, YYYY-MM-D, YYYY-M-D
  size_t first_dash = date_str.find('-');
  size_t second_dash = date_str.find('-', first_dash + 1);

  if (first_dash == string::npos || second_dash == string::npos) {
    LOG_WARN("Invalid date format: %s, expected YYYY-MM-DD or similar", date_str.c_str());
    return RC::INVALID_ARGUMENT;
  }

  // 解析年、月、日
  string year_str = date_str.substr(0, first_dash);
  string month_str = date_str.substr(first_dash + 1, second_dash - first_dash - 1);
  string day_str = date_str.substr(second_dash + 1);

  // 验证年份长度必须是4位
  if (year_str.length() != 4) {
    LOG_WARN("Invalid year format: %s, expected 4 digits", year_str.c_str());
    return RC::INVALID_ARGUMENT;
  }

  // 验证月份和日期长度（1-2位）
  if (month_str.length() == 0 || month_str.length() > 2 ||
      day_str.length() == 0 || day_str.length() > 2) {
    LOG_WARN("Invalid month/day format in date: %s", date_str.c_str());
    return RC::INVALID_ARGUMENT;
  }

  int year, month, day;
  try {
    year = std::stoi(year_str);
    month = std::stoi(month_str);
    day = std::stoi(day_str);
  } catch (const std::exception &e) {
    LOG_WARN("Failed to parse date: %s", date_str.c_str());
    return RC::INVALID_ARGUMENT;
  }

  // 验证日期合法性
  if (!is_valid_date(year, month, day)) {
    LOG_WARN("Invalid date: %04d-%02d-%02d", year, month, day);
    return RC::INVALID_ARGUMENT;
  }

  // 转换为YYYYMMDD格式
  date_int = year * 10000 + month * 100 + day;
  return RC::SUCCESS;
}

RC DateType::format_date_int(int date_int, string &date_str)
{
  int year = date_int / 10000;
  int month = (date_int % 10000) / 100;
  int day = date_int % 100;

  // 验证日期合法性
  if (!is_valid_date(year, month, day)) {
    LOG_WARN("Invalid date integer: %d", date_int);
    return RC::INVALID_ARGUMENT;
  }

  // 格式化为YYYY-MM-DD
  std::ostringstream oss;
  oss << std::setfill('0') << std::setw(4) << year << "-"
      << std::setw(2) << month << "-"
      << std::setw(2) << day;

  date_str = oss.str();
  return RC::SUCCESS;
}

RC UndefinedType::to_string(const Value &val, string &result) const
{
  ASSERT(val.attr_type() == AttrType::UNDEFINED, "invalid type");
  LOG_DEBUG("UndefinedType::to_string called, returning NULL");
  result = "NULL";
  return RC::SUCCESS;
}