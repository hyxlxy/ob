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
#include "common/type/vector_type.h"
#include "common/log/log.h"
#include "common/lang/limits.h"

#include <type_traits>

#include "observer/common/null_like.h"

using namespace std;

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
  return tuple.find_cell(TupleCellSpec(table_name(), field_name()), value);
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
    : comp_(comp), left_(std::move(left)), right_(std::move(right))
{
}

ComparisonExpr::~ComparisonExpr() {}

RC ComparisonExpr::compare_value(const Value &left, const Value &right, bool &result) const
{
  RC rc = RC::SUCCESS;
  result = false;

  // NULL-like numeric handling: if either side is a special NULL-like numeric (e.g. division-by-zero),
  // the comparison yields UNKNOWN -> treated as false in WHERE.
  if (is_null_like_numeric(left) || is_null_like_numeric(right)) {
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
  Value left_value;
  Value right_value;

  RC rc = left_->get_value(tuple, left_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
    return rc;
  }
  rc = right_->get_value(tuple, right_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
    return rc;
  }

  bool bool_value = false;

  rc = compare_value(left_value, right_value, bool_value);
  if (rc == RC::SUCCESS) {
    value.set_boolean(bool_value);
  }
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
      std::fill(select.begin(), select.end(), static_cast<uint8_t>(b ? 1 : 0));
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

  if (comm == AttrType::INTS) {
    rc = compare_column<int>(*lp, *rp, select);
  } else if (comm == AttrType::FLOATS) {
    rc = compare_column<float>(*lp, *rp, select);
  } else {
    LOG_WARN("unsupported vector compare for type %d", comm);
    return RC::INTERNAL;
  }
  return rc;
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

  Value tmp_value;
  for (const unique_ptr<Expression> &expr : children_) {
    rc = expr->get_value(tuple, tmp_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get value by child expression. rc=%s", strrc(rc));
      return rc;
    }
    bool bool_value = tmp_value.get_boolean();
    if ((conjunction_type_ == Type::AND && !bool_value) || (conjunction_type_ == Type::OR && bool_value)) {
      value.set_boolean(bool_value);
      return rc;
    }
  }

  bool default_value = (conjunction_type_ == Type::AND);
  value.set_boolean(default_value);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////

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
      rc = Value::negative(left_value, value);
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
        binary_operator<LEFT_CONSTANT, RIGHT_CONSTANT, float, AddOperator>(
            (float *)left.data(), (float *)right.data(), (float *)result.data(), result.capacity());
      } else {
        rc = RC::UNIMPLEMENTED;
      }
    } break;
    case Type::SUB:
      if (attr_type == AttrType::INTS) {
        binary_operator<LEFT_CONSTANT, RIGHT_CONSTANT, int, SubtractOperator>(
            (int *)left.data(), (int *)right.data(), (int *)result.data(), result.capacity());
      } else if (attr_type == AttrType::FLOATS) {
        binary_operator<LEFT_CONSTANT, RIGHT_CONSTANT, float, SubtractOperator>(
            (float *)left.data(), (float *)right.data(), (float *)result.data(), result.capacity());
      } else {
        rc = RC::UNIMPLEMENTED;
      }
      break;
    case Type::MUL:
      if (attr_type == AttrType::INTS) {
        binary_operator<LEFT_CONSTANT, RIGHT_CONSTANT, int, MultiplyOperator>(
            (int *)left.data(), (int *)right.data(), (int *)result.data(), result.capacity());
      } else if (attr_type == AttrType::FLOATS) {
        binary_operator<LEFT_CONSTANT, RIGHT_CONSTANT, float, MultiplyOperator>(
            (float *)left.data(), (float *)right.data(), (float *)result.data(), result.capacity());
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
        float *lptr = (float *)left.data();
        float *rptr = (float *)right.data();
        float *optr = (float *)result.data();
        const int n = result.capacity();
        const float null_like = numeric_limits<float>::max();
        for (int i = 0; i < n; i++) {
          const float lv = LEFT_CONSTANT ? lptr[0] : lptr[i];
          const float rv = RIGHT_CONSTANT ? rptr[0] : rptr[i];
          optr[i] = (rv == 0.0f ? null_like : (lv / rv));
        }
      } else {
        rc = RC::UNIMPLEMENTED;
      }
      break;
    case Type::NEGATIVE:
      if (attr_type == AttrType::INTS) {
        unary_operator<LEFT_CONSTANT, int, NegateOperator>((int *)left.data(), (int *)result.data(), result.capacity());
      } else if (attr_type == AttrType::FLOATS) {
        unary_operator<LEFT_CONSTANT, float, NegateOperator>(
            (float *)left.data(), (float *)result.data(), result.capacity());
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

  rc = left_->get_value(tuple, left_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
    return rc;
  }

  // 对于一元运算符（如负号），right_可能为nullptr
  if (right_) {
    rc = right_->get_value(tuple, right_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
      return rc;
    }
  }

  return calc_value(left_value, right_value, value);
}

RC ArithmeticExpr::get_column(Chunk &chunk, Column &column)
{
  RC rc = RC::SUCCESS;
  if (pos_ != -1) {
    column.reference(chunk.column(pos_));
    return rc;
  }
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

  return calc_value(left_value, right_value, value);
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
  return tuple.find_cell(TupleCellSpec(name()), value);
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
