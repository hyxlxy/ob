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
// Created by Wangyunlai on 2022/07/05.
//

#include "sql/expr/expression.h"
#include "sql/expr/tuple.h"
#include "sql/expr/arithmetic_operator.hpp"
#include <set>
#include <limits>
#include "common/type/vector_type.h"
#include "common/log/log.h"
#include "sql/stmt/select_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "sql/optimizer/logical_plan_generator.h"
#include "sql/optimizer/physical_plan_generator.h"
#include "sql/operator/project_physical_operator.h"
#include "session/session.h"

// 前向声明
static RC execute_subquery_and_collect_results(Stmt *sub_query, std::vector<Value> &results, const Tuple *parent_tuple = nullptr, const BinderContext *outer_context = nullptr);
#include "common/lang/limits.h"

#include <type_traits>

#include "observer/common/null_like.h"

using namespace std;

// 定义 FieldExpr 的静态成员变量
const Tuple *FieldExpr::current_parent_tuple_ = nullptr;

/**
 * @brief LIKE操作符的模式匹配函数
 * @param text 要匹配的文本
 * @param pattern 模式字符串，支持%（匹配任意字符序列）和_（匹配单个字符）
 * @return 匹配返回true，否则返回false
 */
bool like_match(const char *text, const char *pattern) {
  if (!text || !pattern) {
    return false;
  }

  // 递归实现LIKE匹配
  while (*pattern) {
    if (*pattern == '%') {
      // %匹配任意字符序列（包括空序列）
      pattern++;
      if (!*pattern) {
        return true; // %在末尾，匹配剩余所有字符
      }
      // 尝试匹配0个或多个字符
      while (*text) {
        if (like_match(text, pattern)) {
          return true;
        }
        text++;
      }
      return like_match(text, pattern);
    } else if (*pattern == '_') {
      // _匹配单个字符
      if (!*text) {
        return false; // 文本已结束但模式还有_
      }
      pattern++;
      text++;
    } else {
      // 普通字符，必须完全匹配
      if (*text != *pattern) {
        return false;
      }
      pattern++;
      text++;
    }
  }

  // 模式已结束，检查文本是否也结束
  return *text == '\0';
}

RC FieldExpr::get_value(const Tuple &tuple, Value &value) const
{
  printf("DEBUG: [FIELD] FieldExpr looking for table='%s', field='%s'\n",
         table_name() ? table_name() : "NULL",
         field_name() ? field_name() : "NULL");

  // 检查Field对象是否有效
  if (!field_.table() || !field_.meta()) {
    printf("ERROR: [FIELD] FieldExpr has invalid Field object: table=%p, field=%p\n", 
           field_.table(), field_.meta());
    return RC::INTERNAL;
  }

  RC rc = tuple.find_cell(TupleCellSpec(table_name(), field_name()), value);

  printf("DEBUG: [FIELD] FieldExpr find_cell result: rc=%s\n", strrc(rc));

  // 如果在当前元组中找不到字段，且存在父级元组（相关子查询），尝试从父级元组中查找
  if (rc != RC::SUCCESS && current_parent_tuple_ != nullptr) {
    printf("DEBUG: [FIELD] FieldExpr not found in current tuple, trying parent tuple for correlated subquery\n");
    rc = current_parent_tuple_->find_cell(TupleCellSpec(table_name(), field_name()), value);
    printf("DEBUG: [FIELD] FieldExpr parent tuple find_cell result: rc=%s\n", strrc(rc));
    
    // 如果在直接父级元组中也找不到，尝试从更外层的父级元组中查找
    if (rc != RC::SUCCESS) {
      printf("DEBUG: [FIELD] FieldExpr not found in direct parent tuple, trying database lookup for nested subquery\n");
      
      // 对于多层相关子查询，暂时返回错误
      // 这需要更复杂的架构修改来支持
      printf("DEBUG: [FIELD] Cannot access outer query values in nested subquery for table '%s'\n", table_name());
      return RC::SCHEMA_FIELD_NOT_EXIST;
    }
  }

  if (rc == RC::SUCCESS) {
    if (value.attr_type() == AttrType::INTS) {
      printf("DEBUG: [FIELD] FieldExpr INT result: value_type=%d, int_value=%d\n",
             (int)value.attr_type(), value.get_int());
    } else if (value.attr_type() == AttrType::UNDEFINED) {
      printf("DEBUG: [FIELD] FieldExpr NULL result: value_type=UNDEFINED\n");
    } else {
      printf("DEBUG: [FIELD] FieldExpr result: value_type=%d\n", (int)value.attr_type());
    }
  }

  return rc;
}

bool FieldExpr::equal(const Expression &other) const
{
  if (this == &other) {
    return true;
  }
  if (other.type() != ExprType::FIELD) {
    return false;
  }
  const auto &other_field_expr = static_cast<const FieldExpr &>(other);
  return table_name() == other_field_expr.table_name() && field_name() == other_field_expr.field_name();
}

// TODO: 在进行表达式计算时，`chunk` 包含了所有列，因此可以通过 `field_id` 获取到对应列。
// 后续可以优化成在 `FieldExpr` 中存储 `chunk` 中某列的位置信息。
RC FieldExpr::get_column(Chunk &chunk, Column &column)
{
  if (pos_ != -1) {
    column.reference(chunk.column(pos_));
  } else {
    column.reference(chunk.column(field().meta()->field_id()));
  }
  return RC::SUCCESS;
}

bool ValueExpr::equal(const Expression &other) const
{
  if (this == &other) {
    return true;
  }
  if (other.type() != ExprType::VALUE) {
    return false;
  }
  const auto &other_value_expr = static_cast<const ValueExpr &>(other);
  return value_.compare(other_value_expr.get_value()) == 0;
}

RC ValueExpr::get_value(const Tuple &tuple, Value &value) const
{
  value = value_;
  return RC::SUCCESS;
}

RC ValueExpr::get_column(Chunk &chunk, Column &column)
{
  column.init(value_);
  return RC::SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////
CastExpr::CastExpr(unique_ptr<Expression> child, AttrType cast_type) : child_(std::move(child)), cast_type_(cast_type)
{}

CastExpr::~CastExpr() {}

RC CastExpr::cast(const Value &value, Value &cast_value) const
{
  RC rc = RC::SUCCESS;
  if (this->value_type() == value.attr_type()) {
    cast_value = value;
    return rc;
  }
  rc = Value::cast_to(value, cast_type_, cast_value);
  return rc;
}

RC CastExpr::get_value(const Tuple &tuple, Value &result) const
{
  Value value;
  RC rc = child_->get_value(tuple, value);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  return cast(value, result);
}

RC CastExpr::try_get_value(Value &result) const
{
  Value value;
  RC rc = child_->try_get_value(value);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  return cast(value, result);
}

////////////////////////////////////////////////////////////////////////////////

ComparisonExpr::ComparisonExpr(CompOp comp, unique_ptr<Expression> left, unique_ptr<Expression> right)
    : comp_(comp), left_(std::move(left)), right_(std::move(right)), filter_unit_(nullptr)
{
}

ComparisonExpr::ComparisonExpr(CompOp comp, unique_ptr<Expression> left, unique_ptr<class Stmt> sub_query)
    : comp_(comp), left_(std::move(left)), sub_query_(std::move(sub_query)), filter_unit_(nullptr)
{
}

ComparisonExpr::ComparisonExpr(CompOp comp, unique_ptr<Expression> left, FilterUnit *filter_unit)
    : comp_(comp), left_(std::move(left)), filter_unit_(filter_unit)
{
  // 创建一个占位符右侧表达式，避免 "invalid right children number" 错误
  Value placeholder_value;
  placeholder_value.set_boolean(true);
  right_ = make_unique<ValueExpr>(placeholder_value);
}

ComparisonExpr::~ComparisonExpr() {}

RC ComparisonExpr::compare_value(const Value &left, const Value &right, bool &result) const
{
  RC rc = RC::SUCCESS;
  result = false;

  // Handle IS [NOT] NULL first (special SQL 3-valued logic)
  if (comp_ == IS_NULL_OP || comp_ == IS_NOT_NULL_OP) {
    bool is_null_sql = left.is_null() || left.attr_type() == AttrType::UNDEFINED || is_null_like_numeric(left);
    result = (comp_ == IS_NULL_OP) ? is_null_sql : !is_null_sql;
    return rc;
  }

  // NULL-like numeric handling for other comparisons: UNKNOWN -> false in WHERE
  if (is_null_like_numeric(left) || is_null_like_numeric(right) || left.is_null() || right.is_null() ||
      left.attr_type() == AttrType::UNDEFINED || right.attr_type() == AttrType::UNDEFINED) {
    return rc; // result=false
  }

  int cmp_result = left.compare(right);
  switch (comp_) {
    case EQUAL_TO: {
      result = (0 == cmp_result);
    } break;
    case LESS_EQUAL: {
      result = (cmp_result <= 0);
    } break;
    case NOT_EQUAL: {
      result = (cmp_result != 0);
    } break;
    case LESS_THAN: {
      result = (cmp_result < 0);
    } break;
    case GREAT_EQUAL: {
      result = (cmp_result >= 0);
    } break;
    case GREAT_THAN: {
      result = (cmp_result > 0);
    } break;
    case LIKE_OP: {
      if (left.attr_type() != AttrType::CHARS || right.attr_type() != AttrType::CHARS) {
        LOG_WARN("LIKE operator can only be used with string types");
        rc = RC::INTERNAL;
        break;
      }
      result = like_match(left.get_string().c_str(), right.get_string().c_str());
    } break;
    case NOT_LIKE_OP: {
      if (left.attr_type() != AttrType::CHARS || right.attr_type() != AttrType::CHARS) {
        LOG_WARN("NOT LIKE operator can only be used with string types");
        rc = RC::INTERNAL;
        break;
      }
      result = !like_match(left.get_string().c_str(), right.get_string().c_str());
    } break;
    default: {
      LOG_WARN("unsupported comparison. %d", comp_);
      rc = RC::INTERNAL;
    } break;
  }

  return rc;
}

bool ComparisonExpr::is_correlated_subquery() const
{
  // 简化的检测：通过检查子查询中是否包含外层查询的表名
  // 这是一个启发式方法，可能不够准确，但可以处理大部分情况
  
  if (!sub_query_ && !filter_unit_) {
    return false;
  }
  
  // 对于标量子查询，我们需要更精确的检测
  // 如果子查询不引用外层查询的表，则认为是非关联的
  
  // 检查右表达式是否是子查询
  if (right_ && right_->type() == ExprType::SUBQUERY) {
    // 对于标量子查询，我们需要检查它是否真的引用了外层查询的表
    // 由于我们无法在解析阶段准确判断，我们采用保守的方法：
    // 如果子查询包含聚合函数，则认为是非关联的
    // 否则，我们假设它是关联的，以确保正确性
    return true; // 保守地假设标量子查询是关联的
  }
  
  // 对于 IN/NOT IN 子查询，我们需要更精确的检测
  // 检查子查询是否真的引用了外层查询的表
  if (filter_unit_ && filter_unit_->sub_query()) {
    // Bug修复：默认情况下IN/NOT IN子查询应该是非关联的
    // 只有当子查询中真的引用了外层表时，才是关联子查询
    // 由于检测外层表引用比较复杂，我们先假设是非关联的
    // 如果执行时出现字段找不到等错误，ComparisonExpr::get_value()会尝试作为关联子查询重新执行
    return false; // 默认假设IN/NOT IN子查询是非关联的
  }
  
  // 如果没有子查询，返回false
  return false;
}

RC ComparisonExpr::try_get_value(Value &cell) const
{
  if (left_->type() == ExprType::VALUE && right_->type() == ExprType::VALUE) {
    ValueExpr *  left_value_expr  = static_cast<ValueExpr *>(left_.get());
    ValueExpr *  right_value_expr = static_cast<ValueExpr *>(right_.get());
    const Value &left_cell        = left_value_expr->get_value();
    const Value &right_cell       = right_value_expr->get_value();

    bool value = false;
    RC   rc    = compare_value(left_cell, right_cell, value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to compare tuple cells. rc=%s", strrc(rc));
    } else {
      cell.set_boolean(value);
    }
    return rc;
  }

  return RC::INVALID_ARGUMENT;
}

RC ComparisonExpr::get_value(const Tuple &tuple, Value &value) const
{
  printf("DEBUG: [COMPARISON] ComparisonExpr::get_value() starting: left_type=%d right_type=%d\n",
         static_cast<int>(left_->type()), static_cast<int>(right_->type()));

  LOG_DEBUG("ComparisonExpr::get_value() starting: left_type=%d right_type=%d",
            static_cast<int>(left_->type()), static_cast<int>(right_->type()));

  Value left_value;
  printf("DEBUG: [COMPARISON] About to call left_->get_value(), left_type=%d\n", static_cast<int>(left_->type()));
  RC rc = left_->get_value(tuple, left_value);
  if (rc != RC::SUCCESS) {
    printf("DEBUG: [COMPARISON] left_->get_value() failed: left_type=%d rc=%s\n",
           static_cast<int>(left_->type()), strrc(rc));
    LOG_WARN("ComparisonExpr::get_value() failed to get value of left expression: left_type=%d rc=%s",
             static_cast<int>(left_->type()), strrc(rc));
    return rc;
  }
  LOG_DEBUG("ComparisonExpr::get_value() left value SUCCESS: left_type=%d value_type=%d",
            static_cast<int>(left_->type()), static_cast<int>(left_value.attr_type()));

  // Handle IS [NOT] NULL operations first
  if (comp_ == IS_NULL_OP || comp_ == IS_NOT_NULL_OP) {
    bool is_null_sql = left_value.is_null() || left_value.attr_type() == AttrType::UNDEFINED || is_null_like_numeric(left_value);
    bool result = (comp_ == IS_NULL_OP) ? is_null_sql : !is_null_sql;
    value.set_boolean(result);
    return RC::SUCCESS;
  }

  // Handle EXISTS and NOT EXISTS operations
  if (comp_ == EXISTS_OP || comp_ == NOT_EXISTS_OP) {
    if (!sub_query_) {
      LOG_WARN("EXISTS/NOT EXISTS operation requires a subquery");
      return RC::INTERNAL;
    }

    // Execute the subquery to check if it returns any rows
    std::vector<Value> sub_query_results;
    bool is_correlated = is_correlated_subquery();
    
    RC rc = execute_subquery_and_collect_results(sub_query_.get(), sub_query_results, 
                                                is_correlated ? &tuple : nullptr, nullptr);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to execute EXISTS subquery: rc=%s", strrc(rc));
      return rc;
    }

    // EXISTS returns true if subquery returns at least one row, false otherwise
    // NOT EXISTS returns the opposite
    bool exists_result = !sub_query_results.empty();
    bool final_result = (comp_ == EXISTS_OP) ? exists_result : !exists_result;
    
    value.set_boolean(final_result);
    return RC::SUCCESS;
  }

  // Handle IN and NOT IN operations (both subquery and values)
  if (comp_ == IN_OP || comp_ == NOT_IN_OP || comp_ == IN_VALUES_OP || comp_ == NOT_IN_VALUES_OP) {
    if (is_null_like_numeric(left_value)) {
      value.set_boolean(false);
      return RC::SUCCESS;
    }

    bool found = false;

    // Handle subquery case
    if (sub_query_ || filter_unit_) {
      // For correlated subqueries, we need to execute for each row
      // For non-correlated subqueries, we can cache the results
      bool is_correlated = is_correlated_subquery();

      std::vector<Value> sub_query_results;

      if (!is_correlated && sub_query_executed_ && !sub_query_results_.empty()) {
        // Use cached results for non-correlated subqueries
        LOG_DEBUG("Using cached subquery results (%d results)", (int)sub_query_results_.size());
        sub_query_results = sub_query_results_;
      } else {
        // Execute subquery (always for correlated, first time for non-correlated)
        RC rc = RC::SUCCESS;

        if (filter_unit_ && filter_unit_->sub_query()) {
          // Use FilterUnit's subquery (preferred)
          LOG_DEBUG("Executing FilterUnit subquery %s", is_correlated ? "(correlated)" : "(first time)");
          // 对于关联子查询，传递 &tuple；对于非关联子查询，传递 nullptr
          rc = execute_subquery_and_collect_results(filter_unit_->sub_query().get(), sub_query_results, is_correlated ? &tuple : nullptr, nullptr);
        } else if (sub_query_) {
          // Use direct subquery
          LOG_DEBUG("Executing direct subquery %s", is_correlated ? "(correlated)" : "(first time)");
          // 对于关联子查询，传递 &tuple；对于非关联子查询，传递 nullptr
          rc = execute_subquery_and_collect_results(sub_query_.get(), sub_query_results, is_correlated ? &tuple : nullptr, nullptr);
        } else {
          LOG_WARN("No subquery found in filter_unit or sub_query");
          return RC::INTERNAL;
        }

        if (rc != RC::SUCCESS) {
          LOG_WARN("Failed to execute subquery for IN/NOT IN operation. rc=%s", strrc(rc));
          // 对于关联子查询或不受支持的子查询，尝试作为关联子查询执行
          if (rc == RC::UNIMPLEMENTED || rc == RC::SCHEMA_TABLE_NOT_EXIST || rc == RC::SCHEMA_FIELD_MISSING) {
            LOG_WARN("Subquery execution failed, trying as correlated subquery");
            // 尝试作为关联子查询执行
            if (filter_unit_ && filter_unit_->sub_query()) {
              rc = execute_subquery_and_collect_results(filter_unit_->sub_query().get(), sub_query_results, &tuple, nullptr);
            } else if (sub_query_) {
              rc = execute_subquery_and_collect_results(sub_query_.get(), sub_query_results, &tuple, nullptr);
            }
            
            if (rc != RC::SUCCESS) {
              LOG_WARN("Correlated subquery execution also failed. rc=%s", strrc(rc));
              // 对于 IN/NOT IN，如果子查询执行失败，NOT IN 返回 true，IN 返回 false
              bool result = (comp_ == NOT_IN_OP || comp_ == NOT_IN_VALUES_OP);
              value.set_boolean(result);
              return RC::SUCCESS;
            }
            // 关联子查询执行成功，继续处理结果
          } else {
            return rc;
          }
        }

        // Cache results only for non-correlated subqueries
        if (!is_correlated) {
          sub_query_results_ = sub_query_results;
          sub_query_executed_ = true;
        }

        LOG_DEBUG("Subquery executed with %d results", (int)sub_query_results.size());
      }



      // Check if left_value is in sub_query_results
      for (const Value &sub_value : sub_query_results) {
        if (left_value.compare(sub_value) == 0) {
          found = true;
          break;
        }
      }
    }
    // Handle list case (existing logic)
    else if (right_) {
      if (right_->type() == ExprType::LIST) {
        ListExpr *list_expr = static_cast<ListExpr *>(right_.get());
        list_expr->reset();
      }

      Value right_value;

      // Check all values in the list
      while (RC::SUCCESS == (rc = right_->get_value(tuple, right_value))) {
        if (left_value.compare(right_value) == 0) {
          found = true;
          break;
        }
      }

      // Handle empty list case
      if (rc == RC::RECORD_EOF && !found) {
        // Empty list or no match found
        bool result = (comp_ == NOT_IN_OP || comp_ == NOT_IN_VALUES_OP); // NOT IN returns true, IN returns false
        value.set_boolean(result);
        return RC::SUCCESS;
      } else if (rc != RC::SUCCESS && rc != RC::RECORD_EOF) {
        return rc;
      }
    }

    // Set result based on whether we found a match
    bool result = found ? (comp_ == IN_OP || comp_ == IN_VALUES_OP) : (comp_ == NOT_IN_OP || comp_ == NOT_IN_VALUES_OP);
    
    // 添加调试信息
    if (comp_ == NOT_IN_OP || comp_ == NOT_IN_VALUES_OP) {
      LOG_WARN("NOT IN DEBUG: left_value=%s, found=%s, result=%s", 
               left_value.to_string().c_str(), 
               found ? "true" : "false", 
               result ? "true" : "false");
    }
    
    value.set_boolean(result);
    return RC::SUCCESS;
  }

  // Regular comparison operations
  Value right_value;
  printf("DEBUG: ComparisonExpr: About to get right value, right_type=%d\n", static_cast<int>(right_->type()));
  
  // 检查右表达式是否是子查询
  if (right_->type() == ExprType::SUBQUERY) {
    printf("DEBUG: ComparisonExpr: Right expression is a subquery, calling get_value\n");
    printf("DEBUG: ComparisonExpr: Subquery type: %s\n", typeid(*right_.get()).name());
  }
  
  rc = right_->get_value(tuple, right_value);
  if (right_->type() == ExprType::SUBQUERY) {
    printf("DEBUG: ComparisonExpr: Subquery result: rc=%s, value=%s\n", strrc(rc), right_value.to_string().c_str());
  }
  if (rc != RC::SUCCESS) {
    printf("ERROR: ComparisonExpr: failed to get value of right expression: right_type=%d rc=%s\n",
             static_cast<int>(right_->type()), strrc(rc));
    return rc;
  }
  printf("DEBUG: ComparisonExpr: Got right value successfully: %s\n", right_value.to_string().c_str());
  LOG_DEBUG("ComparisonExpr::get_value() right value SUCCESS: right_type=%d value_type=%d",
            static_cast<int>(right_->type()), static_cast<int>(right_value.attr_type()));

  // 对齐类型以获得一致的比较语义（与列向量路径保持一致）
  // 规则：数值优先（INT/FLOAT -> 统一为 FLOAT），字符串-数值尝试转数值；否则保留原类型
  auto need_numeric = [](AttrType t) {
    return t == AttrType::INTS || t == AttrType::FLOATS;
  };

  Value lv_cast = left_value;
  Value rv_cast = right_value;

  if (need_numeric(left_value.attr_type()) && need_numeric(right_value.attr_type())) {
    // INT/FLOAT 统一到 FLOAT，避免 int/float 混比误差
    Value tmp;
    if (lv_cast.attr_type() != AttrType::FLOATS) {
      if (Value::cast_to(lv_cast, AttrType::FLOATS, tmp) == RC::SUCCESS) lv_cast = tmp;
    }
    if (rv_cast.attr_type() != AttrType::FLOATS) {
      if (Value::cast_to(rv_cast, AttrType::FLOATS, tmp) == RC::SUCCESS) rv_cast = tmp;
    }
  } else if ((left_value.attr_type() == AttrType::CHARS && need_numeric(right_value.attr_type())) ||
             (right_value.attr_type() == AttrType::CHARS && need_numeric(left_value.attr_type()))) {
    // 字符串与数值比较，尝试将字符串转换为对侧的数值类型
    const AttrType target = need_numeric(left_value.attr_type()) ? left_value.attr_type() : right_value.attr_type();
    Value tmp;
    if (left_value.attr_type() == AttrType::CHARS) {
      if (Value::cast_to(lv_cast, target, tmp) == RC::SUCCESS) lv_cast = tmp; // 失败则保留原样，交给 compare_value 处理
    }
    if (right_value.attr_type() == AttrType::CHARS) {
      if (Value::cast_to(rv_cast, target, tmp) == RC::SUCCESS) rv_cast = tmp;
    }
  }

  bool bool_value = false;
  printf("DEBUG: [COMPARISON] About to compare: left=%s, right=%s, comp_op=%d\n",
         lv_cast.to_string().c_str(), rv_cast.to_string().c_str(), static_cast<int>(comp_));
  rc = compare_value(lv_cast, rv_cast, bool_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("ComparisonExpr::get_value() compare_value failed: rc=%s", strrc(rc));
    return rc;
  }

  printf("DEBUG: [COMPARISON] Comparison result: %s ? %s = %s (comp_op=%d)\n",
         lv_cast.to_string().c_str(), rv_cast.to_string().c_str(),
         bool_value ? "true" : "false", static_cast<int>(comp_));
  LOG_DEBUG("ComparisonExpr::get_value() SUCCESS: result=%s", bool_value ? "true" : "false");

  printf("DEBUG: [COMPARISON] About to set boolean value: %s\n", bool_value ? "true" : "false");
  value.set_boolean(bool_value);
  printf("DEBUG: [COMPARISON] After set_boolean, value=%s, attr_type=%d\n",
         value.to_string().c_str(), static_cast<int>(value.attr_type()));
  printf("DEBUG: [COMPARISON] ComparisonExpr::get_value() returning: rc=%s, value=%s\n",
         strrc(rc), value.to_string().c_str());
  return rc;
}

RC ComparisonExpr::eval(Chunk &chunk, vector<uint8_t> &select)
{
  RC rc = RC::SUCCESS;

  // Fast path: both sides are constant-foldable -> compute once and broadcast
  {
    Value lv, rv;
    RC lc = left_->try_get_value(lv);
    RC rc2 = right_->try_get_value(rv);
    if (lc == RC::SUCCESS && rc2 == RC::SUCCESS) {
      bool b = false;
      rc = compare_value(lv, rv, b);
      if (rc != RC::SUCCESS) {
        return rc;
      }
      // Combine with existing mask (AND semantics across multiple predicates)
      const uint8_t bit = static_cast<uint8_t>(b ? 1 : 0);
      for (size_t i = 0; i < select.size(); i++) {
        select[i] &= bit;
      }
      return RC::SUCCESS;
    }
  }

  Column left_column;
  Column right_column;

  rc = left_->get_column(chunk, left_column);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
    return rc;
  }
  rc = right_->get_column(chunk, right_column);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
    return rc;
  }
  // Determine common type for comparison (MySQL-style numeric promotion)
  AttrType ltype = left_column.attr_type();
  AttrType rtype = right_column.attr_type();
  AttrType comm  = AttrType::UNDEFINED;
  if ((ltype == AttrType::INTS || ltype == AttrType::FLOATS) && (rtype == AttrType::INTS || rtype == AttrType::FLOATS)) {
    comm = (ltype == AttrType::FLOATS || rtype == AttrType::FLOATS) ? AttrType::FLOATS : AttrType::INTS;
  } else if (ltype == AttrType::CHARS && rtype == AttrType::CHARS) {
    comm = AttrType::CHARS;
  } else if ((ltype == AttrType::CHARS && (rtype == AttrType::INTS || rtype == AttrType::FLOATS)) ||
             (rtype == AttrType::CHARS && (ltype == AttrType::INTS || ltype == AttrType::FLOATS))) {
    // 字符串与数字比较：尝试将字符串转换为数字进行比较
    // 如果有浮点数参与，使用浮点数比较；否则使用整数比较
    if (ltype == AttrType::FLOATS || rtype == AttrType::FLOATS) {
      comm = AttrType::FLOATS;
    } else {
      comm = AttrType::INTS;
    }
  } else {
    LOG_WARN("unsupported compare types %d vs %d", ltype, rtype);
    return RC::INTERNAL;
  }

  // Cast columns to common type if needed
  auto cast_one = [&](const Column &src, Column &dst, AttrType to_type) {
    if (src.attr_type() == to_type) { dst.reference(src); return; }
    dst.init(to_type, src.attr_len(), src.count());
    dst.set_column_type(src.column_type());
    const int n = src.count();
    for (int i = 0; i < n; i++) {
      Value v = src.get_value(src.column_type() == Column::Type::CONSTANT_COLUMN ? 0 : i);
      Value cv;
      RC c = Value::cast_to(v, to_type, cv);
      if (c != RC::SUCCESS) { cv.set_type(to_type); }
      if (to_type == AttrType::FLOATS) {
        float f = cv.get_float();
        dst.append(reinterpret_cast<char *>(&f), 1);
      } else if (to_type == AttrType::INTS) {
        int x = cv.get_int();
        dst.append(reinterpret_cast<char *>(&x), 1);
      } else if (to_type == AttrType::CHARS) {
        auto s = cv.get_string();
        int len = dst.attr_len();
        std::string buf(len, '\0');
        memcpy(buf.data(), s.data(), std::min((int)s.size(), len));
        dst.append(buf.data(), 1);
      }
    }
    dst.set_count(n);
  };

  Column lc, rcc;
  const Column *lp = &left_column;
  const Column *rp = &right_column;
  if (ltype != comm) { cast_one(left_column, lc, comm); lp = &lc; }
  if (rtype != comm) { cast_one(right_column, rcc, comm); rp = &rcc; }

  // Robust per-row evaluation (handles constants/vectors uniformly and respects NULL-like)
  const bool lconst = lp->column_type() == Column::Type::CONSTANT_COLUMN;
  const bool rconst = rp->column_type() == Column::Type::CONSTANT_COLUMN;
  const int n = std::max(lp->count(), rp->count());
  if ((int)select.size() < n) {
    select.resize(n, 1);
  }
  for (int i = 0; i < n; i++) {
    Value lv = lp->get_value(lconst ? 0 : i);
    Value rv = rp->get_value(rconst ? 0 : i);
    bool b = false;
    RC c = compare_value(lv, rv, b);
    if (c != RC::SUCCESS) {
      return c;
    }
    select[i] &= static_cast<uint8_t>(b ? 1 : 0);
  }
  return RC::SUCCESS;
}

template <typename T>
RC ComparisonExpr::compare_column(const Column &left, const Column &right, vector<uint8_t> &result) const
{
  RC rc = RC::SUCCESS;

  bool left_const  = left.column_type() == Column::Type::CONSTANT_COLUMN;
  bool right_const = right.column_type() == Column::Type::CONSTANT_COLUMN;
  int n = 0;
  if (left_const && right_const) {
    n = left.count();
    compare_result<T, true, true>((T *)left.data(), (T *)right.data(), left.count(), result, comp_);
  } else if (left_const && !right_const) {
    n = right.count();
    compare_result<T, true, false>((T *)left.data(), (T *)right.data(), right.count(), result, comp_);
  } else if (!left_const && right_const) {
    n = left.count();
    compare_result<T, false, true>((T *)left.data(), (T *)right.data(), left.count(), result, comp_);
  } else {
    n = left.count();
    compare_result<T, false, false>((T *)left.data(), (T *)right.data(), left.count(), result, comp_);
  }
  // Apply NULL-like mask for float comparisons: if either side equals sentinel, force false
  if constexpr (std::is_same<T, float>::value) {
    const float null_like = numeric_limits<float>::max();
    const float *lptr = (const float *)left.data();
    const float *rptr = (const float *)right.data();
    for (int i = 0; i < n; i++) {
      const float lv = left_const ? lptr[0] : lptr[i];
      const float rv = right_const ? rptr[0] : rptr[i];
      if (lv == null_like || rv == null_like) {
        result[i] &= 0;
      }
    }
  }
  return rc;
}


////////////////////////////////////////////////////////////////////////////////
ConjunctionExpr::ConjunctionExpr(Type type, vector<unique_ptr<Expression>> &children)
    : conjunction_type_(type), children_(std::move(children))
{}

RC ConjunctionExpr::get_value(const Tuple &tuple, Value &value) const
{
  RC rc = RC::SUCCESS;
  if (children_.empty()) {
    value.set_boolean(true);
    return rc;
  }

  printf("DEBUG: [CONJUNCTION] Starting %s evaluation with %zu children\n", 
         conjunction_type_ == Type::AND ? "AND" : "OR", children_.size());
  fflush(stdout);

  Value tmp_value;
  for (size_t i = 0; i < children_.size(); ++i) {
    const unique_ptr<Expression> &expr = children_[i];
    printf("DEBUG: [CONJUNCTION] Evaluating child %zu (type: %d)\n", i, static_cast<int>(expr->type()));
    
    rc = expr->get_value(tuple, tmp_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get value by child expression. rc=%s", strrc(rc));
      return rc;
    }
    bool bool_value = tmp_value.get_boolean();
    printf("DEBUG: [CONJUNCTION] Child %zu result: %s\n", i, bool_value ? "true" : "false");
    fflush(stdout);
    
    if ((conjunction_type_ == Type::AND && !bool_value) || (conjunction_type_ == Type::OR && bool_value)) {
      printf("DEBUG: [CONJUNCTION] Short-circuit: returning %s\n", bool_value ? "true" : "false");
      fflush(stdout);
      value.set_boolean(bool_value);
      return rc;
    }
  }

  bool default_value = (conjunction_type_ == Type::AND);
  printf("DEBUG: [CONJUNCTION] No short-circuit: returning default %s\n", default_value ? "true" : "false");
  fflush(stdout);
  value.set_boolean(default_value);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////

RC ConjunctionExpr::validate() const {
  for (const auto &child : children_) {
    RC rc = child->validate();
    if (rc != RC::SUCCESS) return rc;
  }
  return RC::SUCCESS;
}

RC ComparisonExpr::validate() const {
  LOG_DEBUG("ComparisonExpr::validate() starting: left_type=%d right_type=%d",
            static_cast<int>(left_->type()), static_cast<int>(right_->type()));

  RC rc = left_->validate();
  if (rc != RC::SUCCESS) {
    LOG_WARN("ComparisonExpr::validate() left validation failed: left_type=%d rc=%s",
             static_cast<int>(left_->type()), strrc(rc));
    return rc;
  }
  LOG_DEBUG("ComparisonExpr::validate() left validation SUCCESS: left_type=%d",
            static_cast<int>(left_->type()));

  rc = right_->validate();
  if (rc != RC::SUCCESS) {
    LOG_WARN("ComparisonExpr::validate() right validation failed: right_type=%d rc=%s",
             static_cast<int>(right_->type()), strrc(rc));
    return rc;
  }
  LOG_DEBUG("ComparisonExpr::validate() right validation SUCCESS: right_type=%d",
            static_cast<int>(right_->type()));

  LOG_DEBUG("ComparisonExpr::validate() SUCCESS");
  return RC::SUCCESS;
}

RC ArithmeticExpr::validate() const {
  if (arithmetic_type_ == ArithmeticExpr::Type::NEGATIVE) {
    // Allow NEGATIVE for any expression, not just constants
    // This supports complex expressions like -(col1 + col2), -col1, etc.
    if (!left_) {
      LOG_WARN("ArithmeticExpr::validate() NEGATIVE has no left child");
      return RC::INTERNAL;
    }
    RC rc = left_->validate();
    if (rc != RC::SUCCESS) {
      LOG_WARN("ArithmeticExpr::validate() NEGATIVE left validation failed: rc=%s", strrc(rc));
      return rc;
    }
    LOG_DEBUG("ArithmeticExpr::validate() NEGATIVE validation SUCCESS for expr_type=%d",
              static_cast<int>(left_->type()));
    return RC::SUCCESS;
  }
  // binary or other unary: validate children
  RC rc = left_->validate();
  if (rc != RC::SUCCESS) {
    LOG_WARN("ArithmeticExpr::validate() left validation failed: rc=%s", strrc(rc));
    return rc;
  }
  if (right_) {
    rc = right_->validate();
    if (rc != RC::SUCCESS) {
      LOG_WARN("ArithmeticExpr::validate() right validation failed: rc=%s", strrc(rc));
      return rc;
    }
  }
  return RC::SUCCESS;
}

ArithmeticExpr::ArithmeticExpr(ArithmeticExpr::Type type, Expression *left, Expression *right)
    : arithmetic_type_(type), left_(left), right_(right)
{}
ArithmeticExpr::ArithmeticExpr(ArithmeticExpr::Type type, unique_ptr<Expression> left, unique_ptr<Expression> right)
    : arithmetic_type_(type), left_(std::move(left)), right_(std::move(right))
{}

bool ArithmeticExpr::equal(const Expression &other) const
{
  if (this == &other) {
    return true;
  }
  if (type() != other.type()) {
    return false;
  }
  auto &other_arith_expr = static_cast<const ArithmeticExpr &>(other);
  return arithmetic_type_ == other_arith_expr.arithmetic_type() && left_->equal(*other_arith_expr.left_) &&
         right_->equal(*other_arith_expr.right_);
}
AttrType ArithmeticExpr::value_type() const
{
  // Unary operator: propagate child's type directly (supports vector NEGATIVE)
  if (!right_) {
    return left_->value_type();
  }

  const AttrType lt = left_->value_type();
  const AttrType rt = right_->value_type();

  // Vector arithmetic: if any operand is a vector, the result is a vector
  // Supported for +, -, *, / (element-wise) as implemented in VectorType
  if (lt == AttrType::VECTORS || rt == AttrType::VECTORS) {
    return AttrType::VECTORS;
  }

  // Scalar arithmetic: keep INT when both are INT and not division; otherwise FLOAT
  if (lt == AttrType::INTS && rt == AttrType::INTS && arithmetic_type_ != Type::DIV) {
    return AttrType::INTS;
  }
  return AttrType::FLOATS;
}

RC ArithmeticExpr::calc_value(const Value &left_value, const Value &right_value, Value &value) const
{
  RC rc = RC::SUCCESS;

  const AttrType target_type = value_type();
  value.set_type(target_type);





  switch (arithmetic_type_) {
    case Type::ADD: {
      rc = Value::add(left_value, right_value, value);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to add values. rc=%s", strrc(rc));
      }
    } break;

    case Type::SUB: {
      rc = Value::subtract(left_value, right_value, value);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to subtract values. rc=%s", strrc(rc));
      }
    } break;

    case Type::MUL: {
      rc = Value::multiply(left_value, right_value, value);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to multiply values. rc=%s", strrc(rc));
      }
    } break;

    case Type::DIV: {
      rc = Value::divide(left_value, right_value, value);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to divide values. rc=%s", strrc(rc));
      }
    } break;

    case Type::NEGATIVE: {
      // 直接使用操作数的类型实例进行负数运算
      AttrType operand_type = left_value.attr_type();
      value.set_type(operand_type);

      rc = DataType::type_instance(operand_type)->negative(left_value, value);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to negate value. rc=%s", strrc(rc));
      }
    } break;

    default: {
      rc = RC::INTERNAL;
      LOG_WARN("unsupported arithmetic type. %d", arithmetic_type_);
    } break;
  }

  return rc;
}

template <bool LEFT_CONSTANT, bool RIGHT_CONSTANT>
RC ArithmeticExpr::execute_calc(
    const Column &left, const Column &right, Column &result, Type type, AttrType attr_type) const
{
  RC rc = RC::SUCCESS;
  switch (type) {
    case Type::ADD: {
      if (attr_type == AttrType::INTS) {
        binary_operator<LEFT_CONSTANT, RIGHT_CONSTANT, int, AddOperator>(
            (int *)left.data(), (int *)right.data(), (int *)result.data(), result.capacity());
      } else if (attr_type == AttrType::FLOATS) {
        // Use manual double precision calculation for better accuracy
        float *lptr = (float *)left.data();
        float *rptr = (float *)right.data();
        float *optr = (float *)result.data();
        const int n = result.capacity();
        for (int i = 0; i < n; i++) {
          const float lv = LEFT_CONSTANT ? lptr[0] : lptr[i];
          const float rv = RIGHT_CONSTANT ? rptr[0] : rptr[i];
          // Use double precision calculation
          double lv_double = static_cast<double>(lv);
          double rv_double = static_cast<double>(rv);
          double result_double = lv_double + rv_double;
          optr[i] = static_cast<float>(result_double);
        }
      } else {
        rc = RC::UNIMPLEMENTED;
      }
    } break;
    case Type::SUB:
      if (attr_type == AttrType::INTS) {
        binary_operator<LEFT_CONSTANT, RIGHT_CONSTANT, int, SubtractOperator>(
            (int *)left.data(), (int *)right.data(), (int *)result.data(), result.capacity());
      } else if (attr_type == AttrType::FLOATS) {
        // Use manual double precision calculation for better accuracy
        float *lptr = (float *)left.data();
        float *rptr = (float *)right.data();
        float *optr = (float *)result.data();
        const int n = result.capacity();
        for (int i = 0; i < n; i++) {
          const float lv = LEFT_CONSTANT ? lptr[0] : lptr[i];
          const float rv = RIGHT_CONSTANT ? rptr[0] : rptr[i];
          // Use double precision calculation
          double lv_double = static_cast<double>(lv);
          double rv_double = static_cast<double>(rv);
          double result_double = lv_double - rv_double;
          optr[i] = static_cast<float>(result_double);
        }
      } else {
        rc = RC::UNIMPLEMENTED;
      }
      break;
    case Type::MUL:
      if (attr_type == AttrType::INTS) {
        binary_operator<LEFT_CONSTANT, RIGHT_CONSTANT, int, MultiplyOperator>(
            (int *)left.data(), (int *)right.data(), (int *)result.data(), result.capacity());
      } else if (attr_type == AttrType::FLOATS) {
        // Use manual double precision calculation for better accuracy
        float *lptr = (float *)left.data();
        float *rptr = (float *)right.data();
        float *optr = (float *)result.data();
        const int n = result.capacity();
        for (int i = 0; i < n; i++) {
          const float lv = LEFT_CONSTANT ? lptr[0] : lptr[i];
          const float rv = RIGHT_CONSTANT ? rptr[0] : rptr[i];
          // Use double precision calculation
          double lv_double = static_cast<double>(lv);
          double rv_double = static_cast<double>(rv);
          double result_double = lv_double * rv_double;
          optr[i] = static_cast<float>(result_double);
        }
      } else {
        rc = RC::UNIMPLEMENTED;
      }
      break;
    case Type::DIV:
      if (attr_type == AttrType::INTS) {
        binary_operator<LEFT_CONSTANT, RIGHT_CONSTANT, int, DivideOperator>(
            (int *)left.data(), (int *)right.data(), (int *)result.data(), result.capacity());
      } else if (attr_type == AttrType::FLOATS) {
        // Division with NULL-like handling on vectorized path: right==0 -> sentinel (numeric_limits<float>::max())
        // Use double precision for calculation to match MySQL 8.0 behavior
        float *lptr = (float *)left.data();
        float *rptr = (float *)right.data();
        float *optr = (float *)result.data();
        const int n = result.capacity();
        const float null_like = numeric_limits<float>::max();
        for (int i = 0; i < n; i++) {
          const float lv = LEFT_CONSTANT ? lptr[0] : lptr[i];
          const float rv = RIGHT_CONSTANT ? rptr[0] : rptr[i];
          if (rv == 0.0f) {
            optr[i] = null_like;
          } else {
            // Use double precision calculation
            double lv_double = static_cast<double>(lv);
            double rv_double = static_cast<double>(rv);
            double result_double = lv_double / rv_double;
            optr[i] = static_cast<float>(result_double);
          }
        }
      } else {
        rc = RC::UNIMPLEMENTED;
      }
      break;
    case Type::NEGATIVE:
      if (attr_type == AttrType::INTS) {
        unary_operator<LEFT_CONSTANT, int, NegateOperator>((int *)left.data(), (int *)result.data(), result.capacity());
      } else if (attr_type == AttrType::FLOATS) {
        // Use manual double precision calculation for better accuracy
        float *lptr = (float *)left.data();
        float *optr = (float *)result.data();
        const int n = result.capacity();
        for (int i = 0; i < n; i++) {
          const float lv = LEFT_CONSTANT ? lptr[0] : lptr[i];
          // Use double precision calculation
          double lv_double = static_cast<double>(lv);
          double result_double = -lv_double;
          optr[i] = static_cast<float>(result_double);
        }
      } else {
        rc = RC::UNIMPLEMENTED;
      }
      break;
    default: rc = RC::UNIMPLEMENTED; break;
  }
  if (rc == RC::SUCCESS) {
    result.set_count(result.capacity());
  }
  return rc;
}

RC ArithmeticExpr::get_value(const Tuple &tuple, Value &value) const
{
  RC rc = RC::SUCCESS;





  Value left_value;
  Value right_value;

  // 检查是否存在循环引用
  if (left_.get() == this || right_.get() == this) {
    LOG_WARN("ArithmeticExpr: Circular reference detected!");
    value.set_float(0.0f);
    return RC::SUCCESS;
  }

  if (left_) {
    rc = left_->get_value(tuple, left_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
      return rc;
    }
  }
  if (right_) {
    rc = right_->get_value(tuple, right_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
      return rc;
    }
  }

  rc = calc_value(left_value, right_value, value);
  return rc;
}

RC ArithmeticExpr::get_column(Chunk &chunk, Column &column)
{
  RC rc = RC::SUCCESS;
  if (pos_ != -1) {
    column.reference(chunk.column(pos_));
    return rc;
  }

  // Allow unary minus on any expression in vectorized path.

  Column left_column;
  Column right_column;

  rc = left_->get_column(chunk, left_column);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get column of left expression. rc=%s", strrc(rc));
    return rc;
  }
  // 对于一元运算（NEGATIVE），right_ 可能为空，此时仅使用 left_column
  if (right_) {
    rc = right_->get_column(chunk, right_column);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get column of right expression. rc=%s", strrc(rc));
      return rc;
    }
  } else {
    // 占位：对于一元运算，右列未使用。为了重用下游计算接口，这里将 right_column 引用为 left_column。
    right_column.reference(left_column);
  }
  return calc_column(left_column, right_column, column);
}

RC ArithmeticExpr::calc_column(const Column &left_column, const Column &right_column, Column &column) const
{
  RC rc = RC::SUCCESS;

  const AttrType target_type = value_type();

  // Prepare left/right columns with target_type to avoid reinterpreting int memory as float, etc.
  auto cast_one = [&](const Column &src, Column &dst) -> void {
    if (src.attr_type() == target_type) {
      dst.reference(src);
      return;
    }
    // initialize destination with same length per element (ints/floats both 4 in this system)
    dst.init(target_type, src.attr_len(), src.count());
    dst.set_column_type(src.column_type());
    // materialize with per-row cast
    const int n = src.count();
    for (int i = 0; i < n; i++) {
      Value v = src.get_value(src.column_type() == Column::Type::CONSTANT_COLUMN ? 0 : i);
      Value cv;
      RC crc = Value::cast_to(v, target_type, cv);
      if (crc != RC::SUCCESS) {
        // best-effort: keep default 0 on cast failure
        cv.set_type(target_type);
      }
      if (target_type == AttrType::FLOATS) {
        float f = cv.get_float();
        dst.append(reinterpret_cast<char *>(&f), 1);
      } else if (target_type == AttrType::INTS) {
        int x = cv.get_int();
        dst.append(reinterpret_cast<char *>(&x), 1);
      } else {
        // fallback: append zero
        int x = 0;
        dst.append(reinterpret_cast<char *>(&x), 1);
      }
    }
    dst.set_count(n);
  };

  Column lcast;
  Column rcast;
  const Column *lptr = &left_column;
  const Column *rptr = &right_column;

  if (left_column.attr_type() != target_type) {
    cast_one(left_column, lcast);
    lptr = &lcast;
  }
  if (right_column.attr_type() != target_type) {
    cast_one(right_column, rcast);
    rptr = &rcast;
  }

  // initialize result column
  const int out_n = std::max(lptr->count(), rptr->count());
  column.init(target_type, lptr->attr_len(), out_n);

  bool left_const  = lptr->column_type() == Column::Type::CONSTANT_COLUMN;
  bool right_const = rptr->column_type() == Column::Type::CONSTANT_COLUMN;
  if (left_const && right_const) {
    column.set_column_type(Column::Type::CONSTANT_COLUMN);
    rc = execute_calc<true, true>(*lptr, *rptr, column, arithmetic_type_, target_type);
  } else if (left_const && !right_const) {
    column.set_column_type(Column::Type::NORMAL_COLUMN);
    rc = execute_calc<true, false>(*lptr, *rptr, column, arithmetic_type_, target_type);
  } else if (!left_const && right_const) {
    column.set_column_type(Column::Type::NORMAL_COLUMN);
    rc = execute_calc<false, true>(*lptr, *rptr, column, arithmetic_type_, target_type);
  } else {
    column.set_column_type(Column::Type::NORMAL_COLUMN);
    rc = execute_calc<false, false>(*lptr, *rptr, column, arithmetic_type_, target_type);
  }
  // Important: set output row count so downstream comparisons see correct length
  column.set_count(out_n);
  return rc;
}

RC ArithmeticExpr::try_get_value(Value &value) const
{
  RC rc = RC::SUCCESS;

  Value left_value;
  Value right_value;

  rc = left_->try_get_value(left_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
    return rc;
  }

  if (right_) {
    rc = right_->try_get_value(right_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
      return rc;
    }
  }

  rc = calc_value(left_value, right_value, value);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////

FunctionExpr::FunctionExpr(const char *function_name, vector<unique_ptr<Expression>> args)
    : function_name_(function_name), args_(std::move(args))
{}

unique_ptr<Expression> FunctionExpr::copy() const
{
  vector<unique_ptr<Expression>> args_copy;
  for (const auto &arg : args_) {
    args_copy.emplace_back(arg->copy());
  }
  return make_unique<FunctionExpr>(function_name_.c_str(), std::move(args_copy));
}

AttrType FunctionExpr::value_type() const
{
  // 大多数向量函数返回float类型
  return AttrType::FLOATS;
}

RC FunctionExpr::get_value(const Tuple &tuple, Value &value) const
{
  // 获取所有参数的值
  vector<Value> arg_values;
  arg_values.reserve(args_.size());

  for (const auto &arg : args_) {
    Value arg_value;
    RC rc = arg->get_value(tuple, arg_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get argument value for function %s", function_name_.c_str());
      return rc;
    }
    arg_values.emplace_back(arg_value);
  }

  return try_get_value_with_args(arg_values, value);
}

RC FunctionExpr::try_get_value(Value &value) const
{
  // 尝试获取所有参数的值
  vector<Value> arg_values;
  arg_values.reserve(args_.size());

  for (const auto &arg : args_) {
    Value arg_value;
    RC rc = arg->try_get_value(arg_value);
    if (rc != RC::SUCCESS) {
      return rc; // 如果任何参数无法获取值，则无法计算函数值
    }
    arg_values.emplace_back(arg_value);
  }

  return try_get_value_with_args(arg_values, value);
}

RC FunctionExpr::try_get_value_with_args(const vector<Value> &arg_values, Value &value) const
{
  if (strcasecmp(function_name_.c_str(), "L2_DISTANCE") == 0) {
    if (arg_values.size() != 2) {
      LOG_WARN("L2_DISTANCE function requires exactly 2 arguments, got %zu", arg_values.size());
      return RC::INVALID_ARGUMENT;
    }
    return VectorType::l2_distance(arg_values[0], arg_values[1], value);
  }

  if (strcasecmp(function_name_.c_str(), "COSINE_DISTANCE") == 0) {
    if (arg_values.size() != 2) {
      LOG_WARN("COSINE_DISTANCE function requires exactly 2 arguments, got %zu", arg_values.size());
      return RC::INVALID_ARGUMENT;
    }
    return VectorType::cosine_distance(arg_values[0], arg_values[1], value);
  }

  if (strcasecmp(function_name_.c_str(), "INNER_PRODUCT") == 0) {
    if (arg_values.size() != 2) {
      LOG_WARN("INNER_PRODUCT function requires exactly 2 arguments, got %zu", arg_values.size());
      return RC::INVALID_ARGUMENT;
    }
    return VectorType::inner_product(arg_values[0], arg_values[1], value);
  }

  LOG_WARN("Unknown function: %s", function_name_.c_str());
  return RC::UNIMPLEMENTED;
}

////////////////////////////////////////////////////////////////////////////////

UnboundAggregateExpr::UnboundAggregateExpr(const char *aggregate_name, Expression *child)
    : aggregate_name_(aggregate_name), child_(child)
{}

UnboundAggregateExpr::UnboundAggregateExpr(const char *aggregate_name, unique_ptr<Expression> child)
    : aggregate_name_(aggregate_name), child_(std::move(child))
{}

////////////////////////////////////////////////////////////////////////////////
AggregateExpr::AggregateExpr(Type type, Expression *child) : aggregate_type_(type), child_(child) {}

AggregateExpr::AggregateExpr(Type type, unique_ptr<Expression> child) : aggregate_type_(type), child_(std::move(child))
{}

RC AggregateExpr::get_column(Chunk &chunk, Column &column)
{
  RC rc = RC::SUCCESS;
  if (pos_ != -1) {
    column.reference(chunk.column(pos_));
  } else {
    rc = RC::INTERNAL;
  }
  return rc;
}

bool AggregateExpr::equal(const Expression &other) const
{
  if (this == &other) {
    return true;
  }
  if (other.type() != type()) {
    return false;
  }
  const AggregateExpr &other_aggr_expr = static_cast<const AggregateExpr &>(other);
  return aggregate_type_ == other_aggr_expr.aggregate_type() && child_->equal(*other_aggr_expr.child());
}

unique_ptr<Aggregator> AggregateExpr::create_aggregator() const
{
  unique_ptr<Aggregator> aggregator;
  switch (aggregate_type_) {
    case Type::SUM: {
      aggregator = make_unique<SumAggregator>();
      break;
    }
    case Type::COUNT: {
      aggregator = make_unique<CountAggregator>();
      break;
    }
    case Type::MAX: {
      aggregator = make_unique<MaxAggregator>();
      break;
    }
    case Type::MIN: {
      aggregator = make_unique<MinAggregator>();
      break;
    }
    case Type::AVG: {
      aggregator = make_unique<AvgAggregator>();
      break;
    }
    default: {
      ASSERT(false, "unsupported aggregate type");
      break;
    }
  }
  return aggregator;
}

RC AggregateExpr::get_value(const Tuple &tuple, Value &value) const
{
  // 添加调试信息和安全检查
  printf("DEBUG: [AGGREGATE] AggregateExpr::get_value() called, name='%s'\n", name());

  // 检查是否存在循环引用
  static thread_local int recursion_depth = 0;
  if (recursion_depth > 10) {
    printf("ERROR: [AGGREGATE] Recursion depth exceeded in AggregateExpr::get_value()\n");
    value.set_int(0);
    return RC::INTERNAL;
  }

  recursion_depth++;

  // 首先尝试从tuple中按名称查找聚合结果
  RC rc = tuple.find_cell(TupleCellSpec(name()), value);
  if (rc == RC::SUCCESS) {
    printf("DEBUG: [AGGREGATE] Found aggregate result in tuple: %s\n", value.to_string().c_str());
    recursion_depth--;
    return rc;
  }

  printf("DEBUG: [AGGREGATE] Aggregate result not found in tuple, trying child expression\n");

  // 如果找不到，则从子表达式获取值（用于聚合过程中）
  if (child_) {
    rc = child_->get_value(tuple, value);
    recursion_depth--;
    return rc;
  } else {
    // 对于COUNT(*)，没有子表达式，返回一个虚拟值
    printf("DEBUG: [AGGREGATE] COUNT(*) case, returning 1\n");
    value.set_int(1);
    recursion_depth--;
    return RC::SUCCESS;
  }
}

RC AggregateExpr::type_from_string(const char *type_str, AggregateExpr::Type &type)
{
  RC rc = RC::SUCCESS;
  if (0 == strcasecmp(type_str, "count")) {
    type = Type::COUNT;
  } else if (0 == strcasecmp(type_str, "sum")) {
    type = Type::SUM;
  } else if (0 == strcasecmp(type_str, "avg")) {
    type = Type::AVG;
  } else if (0 == strcasecmp(type_str, "max")) {
    type = Type::MAX;
  } else if (0 == strcasecmp(type_str, "min")) {
    type = Type::MIN;
  } else {
    rc = RC::INVALID_ARGUMENT;
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////////////

InExpr::InExpr(unique_ptr<Expression> left, std::vector<Value> values, bool is_not_in)
    : left_(std::move(left)), values_(std::move(values)), is_not_in_(is_not_in)
{}

InExpr::~InExpr() {}

unique_ptr<Expression> InExpr::copy() const
{
  return make_unique<InExpr>(left_->copy(), values_, is_not_in_);
}

RC InExpr::get_value(const Tuple &tuple, Value &value) const
{
  Value left_value;
  RC rc = left_->get_value(tuple, left_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of left expression in IN. rc=%s", strrc(rc));
    return rc;
  }

  bool found = false;
  for (const Value &val : values_) {
    if (left_value.compare(val) == 0) {
      found = true;
      break;
    }
  }

  bool result = is_not_in_ ? !found : found;
  value.set_boolean(result);
  return RC::SUCCESS;
}

RC InExpr::try_get_value(Value &value) const
{
  Value left_value;
  RC rc = left_->try_get_value(left_value);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  bool found = false;
  for (const Value &val : values_) {
    if (left_value.compare(val) == 0) {
      found = true;
      break;
    }
  }

  bool result = is_not_in_ ? !found : found;
  value.set_boolean(result);
  return RC::SUCCESS;
}

RC InExpr::eval(Chunk &chunk, vector<uint8_t> &select)
{
  RC rc = RC::SUCCESS;

  // Get left column
  Column left_column;
  rc = left_->get_column(chunk, left_column);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get column of left expression in IN. rc=%s", strrc(rc));
    return rc;
  }

  // Evaluate IN condition for each row
  const bool left_const = left_column.column_type() == Column::Type::CONSTANT_COLUMN;
  const int n = left_column.count();
  if ((int)select.size() < n) {
    select.resize(n, 1);
  }

  for (int i = 0; i < n; i++) {
    Value left_value = left_column.get_value(left_const ? 0 : i);

    bool found = false;
    for (const Value &val : values_) {
      if (left_value.compare(val) == 0) {
        found = true;
        break;
      }
    }

    bool result = is_not_in_ ? !found : found;
    select[i] &= static_cast<uint8_t>(result ? 1 : 0);
  }

  return RC::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
ListExpr::ListExpr(std::vector<std::unique_ptr<Expression>> &&exprs) : exprs_(std::move(exprs))
{}

unique_ptr<Expression> ListExpr::copy() const
{
  std::vector<std::unique_ptr<Expression>> copied_exprs;
  for (const auto &expr : exprs_) {
    copied_exprs.emplace_back(expr->copy());
  }
  return make_unique<ListExpr>(std::move(copied_exprs));
}

RC ListExpr::get_value(const Tuple &tuple, Value &value) const
{
  if (current_index_ >= exprs_.size()) {
    return RC::RECORD_EOF;
  }

  RC rc = exprs_[current_index_]->get_value(tuple, value);
  current_index_++;
  return rc;
}

RC ListExpr::try_get_value(Value &value) const
{
  if (current_index_ >= exprs_.size()) {
    return RC::RECORD_EOF;
  }

  RC rc = exprs_[current_index_]->try_get_value(value);
  current_index_++;
  return rc;
}

////////////////////////////////////////////////////////////////////////////////

SubQueryInExpr::SubQueryInExpr(unique_ptr<Expression> left, unique_ptr<class Stmt> sub_query, bool is_not_in)
    : left_(std::move(left)), sub_query_(std::move(sub_query)), is_not_in_(is_not_in)
{}

SubQueryInExpr::~SubQueryInExpr() {}

unique_ptr<Expression> SubQueryInExpr::copy() const
{
  // 对于SubQueryInExpr，我们不能轻易复制Stmt，但可以创建一个新的实例
  // 使用相同的参数，但子查询的Stmt需要重新创建
  // 这是一个限制，但在实际使用中应该避免复制SubQueryInExpr
  LOG_WARN("SubQueryInExpr::copy() is not fully implemented - this may cause issues");
  
  // 创建一个新的SubQueryInExpr，但子查询Stmt为null
  // 这可能会导致运行时错误，但至少不会在编译时崩溃
  auto left_copy = left_ ? left_->copy() : nullptr;
  if (!left_copy) {
    LOG_WARN("Failed to copy left expression in SubQueryInExpr");
    return nullptr;
  }
  
  // 注意：这里我们不能复制sub_query_，因为Stmt的复制比较复杂
  // 在实际使用中，应该避免复制SubQueryInExpr
  return std::make_unique<SubQueryInExpr>(std::move(left_copy), nullptr, is_not_in_);
}

RC SubQueryInExpr::get_value(const Tuple &tuple, Value &value) const
{
  if (!left_) {
    return RC::INVALID_ARGUMENT;
  }

  Value left_value;
  RC rc = left_->get_value(tuple, left_value);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  // Execute subquery if not already executed
  if (!sub_query_executed_) {
    if (!sub_query_) {
      LOG_WARN("SubQueryInExpr has no subquery to execute");
      return RC::INTERNAL;
    }

    // 对于 IN/NOT IN 子查询，通常是非关联的，传递 nullptr
    rc = execute_subquery_and_collect_results(sub_query_.get(), sub_query_results_, nullptr, nullptr);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to execute subquery for IN/NOT IN operation. rc=%s", strrc(rc));
      // 如果子查询执行失败，对于IN返回false，对于NOT IN返回true
      bool result = is_not_in_;
      value.set_boolean(result);
      return RC::SUCCESS;
    }
    sub_query_executed_ = true;
  }

  // Check if left_value is in sub_query results
  bool found = false;
  for (const Value &sub_value : sub_query_results_) {
    if (left_value.compare(sub_value) == 0) {
      found = true;
      break;
    }
  }

  // Set result based on whether we found a match and the operation type
  bool result = found ? !is_not_in_ : is_not_in_;
  value.set_boolean(result);
  return RC::SUCCESS;
}

RC SubQueryInExpr::try_get_value(Value &value) const
{
  // Cannot evaluate subquery without execution context
  return RC::UNIMPLEMENTED;
}

RC SubQueryInExpr::eval(Chunk &chunk, vector<uint8_t> &select)
{
  if (!left_) {
    return RC::INVALID_ARGUMENT;
  }

  // Execute subquery if not already executed
  if (!sub_query_executed_) {
    if (!sub_query_) {
      LOG_WARN("SubQueryInExpr has no subquery to execute");
      return RC::INTERNAL;
    }

    RC rc = execute_subquery_and_collect_results(sub_query_.get(), sub_query_results_, nullptr, nullptr);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to execute subquery for IN/NOT IN operation. rc=%s", strrc(rc));
      return rc;
    }
    sub_query_executed_ = true;
  }

  // Get left expression column
  Column left_column;
  RC rc = left_->get_column(chunk, left_column);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  // Evaluate IN/NOT IN for each row
  int row_count = chunk.rows();
  select.resize(row_count);
  for (int i = 0; i < row_count; i++) {
    Value left_value = left_column.get_value(i);

    // Check if left_value is in sub_query results
    bool found = false;
    for (const Value &sub_value : sub_query_results_) {
      if (left_value.compare(sub_value) == 0) {
        found = true;
        break;
      }
    }

    // Set result based on whether we found a match and the operation type
    select[i] = found ? !is_not_in_ : is_not_in_;
  }

  return RC::SUCCESS;
}

// 执行子查询并收集结果 - 完全重写简化版本
static RC execute_subquery_and_collect_results(Stmt *sub_query, std::vector<Value> &results, const Tuple *parent_tuple, const BinderContext *outer_context)
{
  if (!sub_query) {
    return RC::INVALID_ARGUMENT;
  }

  // 创建逻辑和物理计划
  LogicalPlanGenerator logical_plan_generator;
  unique_ptr<LogicalOperator> logical_operator;
  RC rc = logical_plan_generator.create(sub_query, logical_operator);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  Session *session = Session::current_session();
  if (!session) {
    return RC::INTERNAL;
  }

  PhysicalPlanGenerator physical_plan_generator;
  unique_ptr<PhysicalOperator> physical_operator;
  rc = physical_plan_generator.create(*logical_operator, physical_operator, session);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  // 打开物理操作符
  Trx *trx = session->current_trx();
  if (!trx) {
    return RC::INTERNAL;
  }

  rc = physical_operator->open(trx);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  // 设置父级tuple支持关联子查询
  if (parent_tuple) {
    physical_operator->set_parent_tuple(parent_tuple);
    FieldExpr::set_current_parent_tuple(parent_tuple);
  }

  // 收集结果
  while ((rc = physical_operator->next()) == RC::SUCCESS) {
    Tuple *tuple = physical_operator->current_tuple();
    if (!tuple) {
      continue;
    }

    Value value;
    RC cell_rc = tuple->cell_at(0, value);
    
    // 如果cell_at失败且tuple->cell_num()==0，尝试从child获取
    if (cell_rc != RC::SUCCESS && tuple->cell_num() == 0) {
      if (!physical_operator->children().empty()) {
        PhysicalOperator *child = physical_operator->children()[0].get();
        Tuple *child_tuple = child->current_tuple();
        if (child_tuple && child_tuple->cell_num() > 0) {
          // Bug修复：应该获取第0列（投影的第一列），而不是最后一列
          // 原代码：cell_rc = child_tuple->cell_at(child_tuple->cell_num() - 1, value);
          cell_rc = child_tuple->cell_at(0, value);
        }
      }
    }
    
    if (cell_rc == RC::SUCCESS) {
      results.push_back(value);
    }
  }

  physical_operator->close();
  FieldExpr::set_current_parent_tuple(nullptr);

  return (rc == RC::RECORD_EOF) ? RC::SUCCESS : rc;
}

////////////////////////////////////////////////////////////////////////////////
// SubQueryExpr

SubQueryExpr::SubQueryExpr(SelectSqlNode &select_node) : sql_node_(select_node), outer_context_(nullptr) {}

SubQueryExpr::SubQueryExpr(std::unique_ptr<SelectSqlNode> select_node) 
  : sql_node_(*select_node), owned_sql_node_(std::move(select_node)), outer_context_(nullptr) {}

SubQueryExpr::~SubQueryExpr() = default;

RC SubQueryExpr::generate_select_stmt(Db *db, const std::unordered_map<std::string, BaseTable *> &tables, const BinderContext *outer_context) const
{
  // 仿照普通 select 的执行流程，tables 用来传递别名
  Stmt *stmt = nullptr;
  RC    rc   = SelectStmt::create(db, sql_node_, stmt, outer_context);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to create subquery select statement. return %s", strrc(rc));
    return rc;
  }

  // 确保生成的 stmt 类型为 SELECT 类型
  if (stmt->type() != StmtType::SELECT) {
    LOG_WARN("subquery stmt type is not SELECT.");
    return RC::INVALID_ARGUMENT;
  }

  // 动态转换为 SelectStmt 类型，并进行子查询列数校验
  auto *select_stmt = dynamic_cast<SelectStmt *>(stmt);
  if (select_stmt == nullptr) {
    LOG_WARN("failed to cast subquery stmt to SelectStmt. ");
    return RC::INVALID_ARGUMENT;
  }

  // 子查询不能有超过一个列
  if (select_stmt->query_expressions().size() > 1) {
    LOG_WARN("too many columns in subquery expression.");
    return RC::INVALID_ARGUMENT;
  }

  // 将 select_stmt_ 指针设置为 select_stmt，使用 std::unique_ptr 来管理
  select_stmt_ = std::unique_ptr<SelectStmt>(select_stmt);
  return RC::SUCCESS;
}

RC SubQueryExpr::generate_logical_oper() const
{
  LogicalPlanGenerator generator;
  RC rc = generator.create(select_stmt_.get(), logical_oper_);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to generate logical operator for subquery. return %s", strrc(rc));
    return rc;
  }
  return RC::SUCCESS;
}

RC SubQueryExpr::generate_physical_oper() const
{
  PhysicalPlanGenerator generator;
  Session *session = Session::current_session();
  RC rc = generator.create(*logical_oper_, physical_oper_, session);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to generate physical operator for subquery. return %s", strrc(rc));
    return rc;
  }
  return rc;
}

bool SubQueryExpr::one_row_ret() const { return res_query.size() <= 1; }

// 子算子树的 open 和 close 逻辑由外部控制
RC SubQueryExpr::open(Trx *trx, const Tuple &tuple)
{
  RC rc = RC::SUCCESS;
  physical_oper_->set_parent_tuple(&tuple);
  rc = physical_oper_->open(trx);

  return rc;
}

RC SubQueryExpr::reset()
{
  visited_index = 0;
  return RC::SUCCESS;
}

bool SubQueryExpr::has_more_row(const Tuple &tuple) const
{
  physical_oper_->set_parent_tuple(&tuple);
  return physical_oper_->next() != RC::RECORD_EOF;
}

RC SubQueryExpr::get_value(const Tuple &tuple, Value &value) const
{
  // 检查是否已经初始化，如果没有则进行初始化
  if (!select_stmt_ || !logical_oper_ || !physical_oper_) {
    RC rc = initialize_subquery();
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to initialize subquery: %s", strrc(rc));
      value.set_type(AttrType::UNDEFINED);
      value.set_null(true);
      return rc;
    }
  }

  // 使用缓存结果
  if (cached_value_.attr_type() != AttrType::UNDEFINED) {
    value = cached_value_;
    return RC::SUCCESS;
  }

  // 获取当前会话和事务
  Session *session = Session::current_session();
  if (!session) {
    LOG_WARN("No current session for subquery execution");
    value.set_type(AttrType::UNDEFINED);
    value.set_null(true);
    return RC::INTERNAL;
  }

  Trx *trx = session->current_trx();
  if (!trx) {
    LOG_WARN("No current transaction for subquery execution");
    value.set_type(AttrType::UNDEFINED);
    value.set_null(true);
    return RC::INTERNAL;
  }

  // 设置父级tuple并打开物理操作符
  physical_oper_->set_parent_tuple(&tuple);
  // 设置 FieldExpr 的静态父级元组，用于相关子查询中的字段查找
  FieldExpr::set_current_parent_tuple(&tuple);
  RC rc = physical_oper_->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to open subquery physical operator: %s", strrc(rc));
    value.set_type(AttrType::UNDEFINED);
    value.set_null(true);
    return rc;
  }

  // 获取第一行数据
  rc = physical_oper_->next();
  if (rc == RC::RECORD_EOF) {
    // 无数据返回NULL
    physical_oper_->close();
    FieldExpr::set_current_parent_tuple(nullptr);
    value.set_type(AttrType::UNDEFINED);
    value.set_null(true);
    return RC::SUCCESS;
  } else if (rc != RC::SUCCESS) {
    physical_oper_->close();
    FieldExpr::set_current_parent_tuple(nullptr);
    return rc;
  }

  // 获取当前tuple的值（先获取第一行的值）
  Tuple *current_tuple = physical_oper_->current_tuple();
  if (!current_tuple) {
    physical_oper_->close();
    // 清理 FieldExpr 的静态父级元组
    FieldExpr::set_current_parent_tuple(nullptr);
    value.set_type(AttrType::UNDEFINED);
    value.set_null(true);
    return RC::SUCCESS;
  }

  // 从tuple获取第一个cell的值
  rc = current_tuple->cell_at(0, value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to get value from subquery tuple: %s", strrc(rc));
    physical_oper_->close();
    FieldExpr::set_current_parent_tuple(nullptr);
    value.set_type(AttrType::UNDEFINED);
    value.set_null(true);
    return RC::SUCCESS;
  }

  // 检查是否有多行数据（标量子查询只能返回0-1行）
  RC next_rc = physical_oper_->next();
  if (next_rc == RC::SUCCESS) {
    // 多行数据，返回错误
    physical_oper_->close();
    FieldExpr::set_current_parent_tuple(nullptr);
    return RC::INTERNAL;
  } else if (next_rc != RC::RECORD_EOF) {
    // 其他错误
    physical_oper_->close();
    // 清理 FieldExpr 的静态父级元组
    FieldExpr::set_current_parent_tuple(nullptr);
    return next_rc;
  }
  
  // 关闭并清理
  physical_oper_->close();
  FieldExpr::set_current_parent_tuple(nullptr);

  // 缓存结果
  cached_value_ = value;
  return RC::SUCCESS;
}

RC SubQueryExpr::close() { return physical_oper_->close(); }

RC SubQueryExpr::try_get_value(Value &value) const 
{ 
  // 对于子查询，如果没有父级tuple，我们尝试直接执行子查询
  // 这通常用于非关联子查询
  if (!select_stmt_ || !logical_oper_ || !physical_oper_) {
    RC rc = initialize_subquery();
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to initialize subquery in try_get_value: %s", strrc(rc));
      value.set_type(AttrType::UNDEFINED);
      value.set_null(true);
      return rc;
    }
  }

  // 获取当前会话和事务
  Session *session = Session::current_session();
  if (!session) {
    LOG_WARN("No current session for subquery try_get_value");
    value.set_type(AttrType::UNDEFINED);
    value.set_null(true);
    return RC::INTERNAL;
  }

  Trx *trx = session->current_trx();
  if (!trx) {
    LOG_WARN("No current transaction for subquery try_get_value");
    value.set_type(AttrType::UNDEFINED);
    value.set_null(true);
    return RC::INTERNAL;
  }

  // 对于非关联子查询，传递nullptr作为父级tuple
  physical_oper_->set_parent_tuple(nullptr);
  RC rc = physical_oper_->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to open subquery physical operator in try_get_value: %s", strrc(rc));
    value.set_type(AttrType::UNDEFINED);
    value.set_null(true);
    return rc;
  }

  // 获取第一行数据
  rc = physical_oper_->next();
  if (rc == RC::RECORD_EOF) {
    // 没有数据，返回NULL
    physical_oper_->close();
    value.set_type(AttrType::UNDEFINED);
    value.set_null(true);
    return RC::SUCCESS;
  } else if (rc != RC::SUCCESS) {
    // 其他错误
    physical_oper_->close();
    value.set_type(AttrType::UNDEFINED);
    value.set_null(true);
    return rc;
  }

  // 获取当前tuple的值
  Tuple *current_tuple = physical_oper_->current_tuple();
  if (!current_tuple) {
    physical_oper_->close();
    value.set_type(AttrType::UNDEFINED);
    value.set_null(true);
    return RC::INTERNAL;
  }

  // 从tuple获取第一个cell的值
  rc = current_tuple->cell_at(0, value);
  physical_oper_->close();
  
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to get value from subquery tuple in try_get_value: %s", strrc(rc));
    value.set_type(AttrType::UNDEFINED);
    value.set_null(true);
    return rc;
  }

  return RC::SUCCESS;
}

ExprType SubQueryExpr::type() const { return ExprType::SUBQUERY; }

AttrType SubQueryExpr::value_type() const { return AttrType::UNDEFINED; }

std::unique_ptr<Expression> SubQueryExpr::deep_copy() const
{
  // 子查询的深拷贝比较复杂，暂时返回 NULL
  return nullptr;
}

RC SubQueryExpr::initialize_subquery() const
{
  // 获取当前会话和数据库
  Session *session = Session::current_session();
  if (!session) {
    LOG_WARN("No current session for subquery initialization");
    return RC::INTERNAL;
  }

  Db *db = session->get_current_db();
  if (!db) {
    LOG_WARN("No current database for subquery initialization");
    return RC::INTERNAL;
  }

  // 创建外层上下文，包含当前数据库的所有表
  BinderContext outer_context;
  
  // 从数据库中获取所有表，用于相关子查询的表绑定
  // 这是一个简化的方法，实际应该只包含外层查询的表
  // 我们需要手动添加表到外层上下文中
  // 由于我们无法直接访问get_all_tables，我们暂时使用空的外层上下文
  // 真正的修复需要在bind_unbound_field_expression中处理
  
  // 生成SelectStmt，传递外层上下文
  RC rc = generate_select_stmt(db, {}, &outer_context);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to generate select statement: %s", strrc(rc));
    return rc;
  }

  // 生成逻辑操作符
  rc = generate_logical_oper();
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to generate logical operator: %s", strrc(rc));
    return rc;
  }

  // 生成物理操作符
  rc = generate_physical_oper();
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to generate physical operator: %s", strrc(rc));
    return rc;
  }

  return RC::SUCCESS;
}

