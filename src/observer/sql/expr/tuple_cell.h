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
// Created by WangYunlai on 2022/6/7.
//

#pragma once

#include "storage/field/field_meta.h"

class TupleCellSpec final
{
public:
  TupleCellSpec() = default;
  TupleCellSpec(const char *table_name, const char *field_name, const char *alias = nullptr);
  explicit TupleCellSpec(const char *alias);
  explicit TupleCellSpec(const string &alias);

  const char *table_name() const { return table_name_.c_str(); }
  const char *field_name() const { return field_name_.c_str(); }
  const char *alias() const { return alias_.c_str(); }

  bool equals(const TupleCellSpec &other) const
  {
    // 首先检查表名和字段名是否匹配
    // 但是如果两者都为空（如聚合表达式），则需要比较alias
    if (table_name_ == other.table_name_ && field_name_ == other.field_name_) {
      if (table_name_.empty() && field_name_.empty()) {
        // 对于聚合表达式等，table和field都为空，需要比较alias
        return alias_ == other.alias_;
      } else {
        return true;
      }
    }

    // 如果表名和字段名不匹配，但是alias匹配，也认为是相等的
    // 这种情况适用于表达式或者别名字段
    if (!alias_.empty() && !other.alias_.empty() && alias_ == other.alias_) {
      return true;
    }

    // 如果一个有表名一个没有，但字段名相同，也认为匹配
    // 这种情况适用于单表查询时的字段引用
    // 但是如果两个字段名都为空（如聚合表达式），则不应该匹配
    if ((table_name_.empty() || other.table_name_.empty()) && field_name_ == other.field_name_ && !field_name_.empty()) {
      return true;
    }

    return false;
  }

private:
  string table_name_;
  string field_name_;
  string alias_;
};
