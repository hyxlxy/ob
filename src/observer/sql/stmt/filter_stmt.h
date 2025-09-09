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
// Created by Wangyunlai on 2022/5/22.
//

#pragma once

#include "common/lang/unordered_map.h"
#include "common/lang/vector.h"
#include "sql/expr/expression.h"
#include "sql/parser/parse_defs.h"
#include "sql/stmt/stmt.h"

class Db;
class Table;
class FieldMeta;

struct FilterObj
{
  enum Type {
    ATTR,
    VALUE,
    EXPR
  };

  Type type;
  Field field;
  Value value;
  const Expression* expr;

  void init_attr(const Field &field)
  {
    type = ATTR;
    this->field = field;
  }

  void init_value(const Value &value)
  {
    type = VALUE;
    this->value = value;
  }

  void init_expr(const Expression* expression)
  {
    type = EXPR;
    this->expr = expression;
  }

  bool is_attr() const { return type == ATTR; }
  bool is_value() const { return type == VALUE; }
  bool is_expr() const { return type == EXPR; }
};

class FilterUnit
{
public:
  FilterUnit() = default;
  ~FilterUnit() {}

  void set_comp(CompOp comp) { comp_ = comp; }

  CompOp comp() const { return comp_; }

  void set_left(const FilterObj &obj) { left_ = obj; }
  void set_right(const FilterObj &obj) { right_ = obj; }

  // Own and set expression pointers to ensure lifetime safety
  void set_left_expr(std::unique_ptr<Expression> expr) {
    left_expr_owner_ = std::move(expr);
    left_.init_expr(left_expr_owner_.get());
  }
  void set_right_expr(std::unique_ptr<Expression> expr) {
    right_expr_owner_ = std::move(expr);
    right_.init_expr(right_expr_owner_.get());
  }

  const FilterObj &left() const { return left_; }
  const FilterObj &right() const { return right_; }

private:
  CompOp    comp_ = NO_OP;
  FilterObj left_;
  FilterObj right_;
  std::unique_ptr<Expression> left_expr_owner_;
  std::unique_ptr<Expression> right_expr_owner_;
};

/**
 * @brief Filter/谓词/过滤语句
 * @ingroup Statement
 */
class FilterStmt
{
public:
  FilterStmt() = default;
  virtual ~FilterStmt();

public:
  const vector<FilterUnit *> &filter_units() const { return filter_units_; }

public:
  static RC create(Db *db, Table *default_table, unordered_map<string, Table *> *tables,
      const ConditionSqlNode *conditions, int condition_num, FilterStmt *&stmt);

  static RC create_filter_unit(Db *db, Table *default_table, unordered_map<string, Table *> *tables,
      const ConditionSqlNode &condition, FilterUnit *&filter_unit);

private:
  vector<FilterUnit *> filter_units_;  // 默认当前都是AND关系
};
