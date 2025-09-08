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
// Created by Wangyunlai on 2021/5/7.
//

#include "condition_filter.h"
#include "common/log/log.h"
#include "common/value.h"
#include "storage/record/record_manager.h"
#include "storage/table/table.h"
#include <math.h>
#include <stddef.h>
#include <string>
#include <cstring>

using namespace common;

/**
 * @brief LIKE操作的模式匹配函数
 * @param text 要匹配的文本
 * @param pattern 模式字符串，支持%和_通配符
 * @return 是否匹配
 */
static bool like_match(const char *text, const char *pattern) {
  if (!text || !pattern) {
    return false;
  }

  const char *t = text;
  const char *p = pattern;
  const char *star_text = nullptr;
  const char *star_pattern = nullptr;

  while (*t) {
    if (*p == '%') {
      // 遇到%，记录当前位置，跳过连续的%
      while (*p == '%') p++;
      if (!*p) return true; // 模式以%结尾，匹配成功

      star_pattern = p;
      star_text = t;
    } else if (*p == '_' || *p == *t) {
      // _匹配任意单个字符，或者字符完全匹配
      p++;
      t++;
    } else if (star_pattern) {
      // 当前字符不匹配，但之前有%，回溯
      p = star_pattern;
      t = ++star_text;
    } else {
      // 不匹配且无法回溯
      return false;
    }
  }

  // 跳过模式末尾的%
  while (*p == '%') p++;

  return !*p; // 模式完全消耗则匹配成功
}

ConditionFilter::~ConditionFilter() {}

DefaultConditionFilter::DefaultConditionFilter()
{
  left_.is_attr     = false;
  left_.attr_length = 0;
  left_.attr_offset = 0;

  right_.is_attr     = false;
  right_.attr_length = 0;
  right_.attr_offset = 0;
}
DefaultConditionFilter::~DefaultConditionFilter() {}

RC DefaultConditionFilter::init(const ConDesc &left, const ConDesc &right, AttrType attr_type, CompOp comp_op)
{
  if (attr_type <= AttrType::UNDEFINED || attr_type >= AttrType::MAXTYPE) {
    LOG_ERROR("Invalid condition with unsupported attribute type: %d", attr_type);
    return RC::INVALID_ARGUMENT;
  }

  if (comp_op < EQUAL_TO || comp_op >= NO_OP) {
    LOG_ERROR("Invalid condition with unsupported compare operation: %d", comp_op);
    return RC::INVALID_ARGUMENT;
  }

  left_      = left;
  right_     = right;
  attr_type_ = attr_type;
  comp_op_   = comp_op;
  return RC::SUCCESS;
}

RC DefaultConditionFilter::init(Table &table, const ConditionSqlNode &condition)
{
  const TableMeta &table_meta = table.table_meta();
  ConDesc          left;
  ConDesc          right;

  AttrType type_left  = AttrType::UNDEFINED;
  AttrType type_right = AttrType::UNDEFINED;

  if (1 == condition.left_is_attr) {
    left.is_attr                = true;
    const FieldMeta *field_left = table_meta.field(condition.left_attr.attribute_name.c_str());
    if (nullptr == field_left) {
      LOG_WARN("No such field in condition. %s.%s", table.name(), condition.left_attr.attribute_name.c_str());
      return RC::SCHEMA_FIELD_MISSING;
    }
    left.attr_length = field_left->len();
    left.attr_offset = field_left->offset();

    type_left = field_left->type();
  } else {
    left.is_attr = false;
    left.value   = condition.left_value;  // 校验type 或者转换类型
    type_left    = condition.left_value.attr_type();

    left.attr_length = 0;
    left.attr_offset = 0;
  }

  if (1 == condition.right_is_attr) {
    right.is_attr                = true;
    const FieldMeta *field_right = table_meta.field(condition.right_attr.attribute_name.c_str());
    if (nullptr == field_right) {
      LOG_WARN("No such field in condition. %s.%s", table.name(), condition.right_attr.attribute_name.c_str());
      return RC::SCHEMA_FIELD_MISSING;
    }
    right.attr_length = field_right->len();
    right.attr_offset = field_right->offset();
    type_right        = field_right->type();
  } else {
    right.is_attr = false;
    right.value   = condition.right_value;
    type_right    = condition.right_value.attr_type();

    right.attr_length = 0;
    right.attr_offset = 0;
  }

  // 校验和转换
  //  if (!field_type_compare_compatible_table[type_left][type_right]) {
  //    // 不能比较的两个字段， 要把信息传给客户端
  //    return RC::SCHEMA_FIELD_TYPE_MISMATCH;
  //  }
  // NOTE：这里没有实现不同类型的数据比较，比如整数跟浮点数之间的对比
  // 但是选手们还是要实现。这个功能在预选赛中会出现
  if (type_left != type_right) {
    return RC::SCHEMA_FIELD_TYPE_MISMATCH;
  }

  return init(left, right, type_left, condition.comp);
}

bool DefaultConditionFilter::filter(const Record &rec) const
{
  Value left_value;
  Value right_value;

  if (left_.is_attr) {  // value
    left_value.set_type(attr_type_);
    left_value.set_data(rec.data() + left_.attr_offset, left_.attr_length);
  } else {
    left_value.set_value(left_.value);
  }

  if (right_.is_attr) {
    right_value.set_type(attr_type_);
    right_value.set_data(rec.data() + right_.attr_offset, right_.attr_length);
  } else {
    right_value.set_value(right_.value);
  }

  int cmp_result = left_value.compare(right_value);

  switch (comp_op_) {
    case EQUAL_TO: return 0 == cmp_result;
    case LESS_EQUAL: return cmp_result <= 0;
    case NOT_EQUAL: return cmp_result != 0;
    case LESS_THAN: return cmp_result < 0;
    case GREAT_EQUAL: return cmp_result >= 0;
    case GREAT_THAN: return cmp_result > 0;
    case LIKE_OP: {
      // LIKE操作只支持字符串类型
      if (attr_type_ != AttrType::CHARS) {
        LOG_WARN("LIKE operation is only supported for CHARS type");
        return false;
      }

      // 获取左右值的字符串表示
      std::string left_str, right_str;
      if (left_value.attr_type() == AttrType::CHARS) {
        left_str = left_value.get_string();
      } else {
        return false;
      }

      if (right_value.attr_type() == AttrType::CHARS) {
        right_str = right_value.get_string();
      } else {
        return false;
      }

      return like_match(left_str.c_str(), right_str.c_str());
    }

    default: break;
  }

  LOG_PANIC("Never should print this.");
  return cmp_result;  // should not go here
}

CompositeConditionFilter::~CompositeConditionFilter()
{
  if (memory_owner_) {
    delete[] filters_;
    filters_ = nullptr;
  }
}

RC CompositeConditionFilter::init(const ConditionFilter *filters[], int filter_num, bool own_memory)
{
  filters_      = filters;
  filter_num_   = filter_num;
  memory_owner_ = own_memory;
  return RC::SUCCESS;
}
RC CompositeConditionFilter::init(const ConditionFilter *filters[], int filter_num)
{
  return init(filters, filter_num, false);
}

RC CompositeConditionFilter::init(Table &table, const ConditionSqlNode *conditions, int condition_num)
{
  if (condition_num == 0) {
    return RC::SUCCESS;
  }
  if (conditions == nullptr) {
    return RC::INVALID_ARGUMENT;
  }

  RC                rc                = RC::SUCCESS;
  ConditionFilter **condition_filters = new ConditionFilter *[condition_num];
  for (int i = 0; i < condition_num; i++) {
    DefaultConditionFilter *default_condition_filter = new DefaultConditionFilter();
    rc                                               = default_condition_filter->init(table, conditions[i]);
    if (rc != RC::SUCCESS) {
      delete default_condition_filter;
      for (int j = i - 1; j >= 0; j--) {
        delete condition_filters[j];
        condition_filters[j] = nullptr;
      }
      delete[] condition_filters;
      condition_filters = nullptr;
      return rc;
    }
    condition_filters[i] = default_condition_filter;
  }
  return init((const ConditionFilter **)condition_filters, condition_num, true);
}

bool CompositeConditionFilter::filter(const Record &rec) const
{
  for (int i = 0; i < filter_num_; i++) {
    if (!filters_[i]->filter(rec)) {
      return false;
    }
  }
  return true;
}
