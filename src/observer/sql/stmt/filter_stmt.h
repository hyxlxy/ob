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
  enum Type
  {
    ATTR,
    VALUE,
    EXPR
  };

  Type              type;
  Field             field;
  Value             value;
  const Expression *expr = nullptr;

  void init_attr(const Field &field)
  {
    type        = ATTR;
    this->field = field;
  }

  void init_value(const Value &value)
  {
    type        = VALUE;
    this->value = value;
  }

  void init_expr(const Expression *expression)
  {
    type       = EXPR;
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
  void set_left_expr(std::unique_ptr<Expression> expr)
  {
    left_expr_owner_ = std::move(expr);
    left_.init_expr(left_expr_owner_.get());
  }
  void set_right_expr(std::unique_ptr<Expression> expr)
  {
    right_expr_owner_ = std::move(expr);
    right_.init_expr(right_expr_owner_.get());
  }

  const FilterObj &left() const { return left_; }
  FilterObj       &left() { return left_; }
  const FilterObj &right() const { return right_; }
  FilterObj       &right() { return right_; }

  // 支持IN操作的值列表
  void                      set_right_values(const std::vector<Value> &values) { right_values_ = values; }
  const std::vector<Value> &right_values() const { return right_values_; }

  // 支持子查询
  void set_sub_query(std::unique_ptr<class Stmt> sub_query) { sub_query_ = std::move(sub_query); }
  const std::unique_ptr<class Stmt> &sub_query() const { return sub_query_; }

  std::unique_ptr<Expression> left_expr_owner_;
  std::unique_ptr<Expression> right_expr_owner_;

private:
  CompOp                      comp_ = NO_OP;
  FilterObj                   left_;
  FilterObj                   right_;
  std::vector<Value>          right_values_;  // 用于IN/NOT IN操作的值列表
  std::unique_ptr<class Stmt> sub_query_;     // 用于子查询操作
};

/**
 * @brief 表示一个过滤条件节点
 */
struct FilterConditionNode
{
  enum Type
  {
    CONDITION,  ///< 单个条件
    AND_GROUP,  ///< AND组
    OR_GROUP    ///< OR组
  };

  Type                          type;
  FilterUnit                   *filter_unit;  ///< 单个条件（当type == CONDITION时）
  vector<FilterConditionNode *> children;     ///< 子条件（当type == AND_GROUP或OR_GROUP时）

  FilterConditionNode(Type t) : type(t), filter_unit(nullptr) {}
  ~FilterConditionNode()
  {
    if (filter_unit)
      delete filter_unit;
    for (auto child : children)
      delete child;
  }
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
  vector<FilterUnit *>       &filter_units() { return filter_units_; }
  FilterConditionNode        *get_condition_tree() const { return condition_tree_; }

public:
  static RC create(Db *db, Table *default_table, unordered_map<string, Table *> *tables, ConditionSqlNode *conditions,
      int condition_num, FilterStmt *&stmt, BinderContext *bind_context = nullptr);

  static RC create_filter_unit(Db *db, Table *default_table, unordered_map<string, Table *> *tables,
      ConditionSqlNode &condition, FilterUnit *&filter_unit, BinderContext *bind_context = nullptr);

  // 新增：创建条件树
  static RC create_condition_tree(Db *db, Table *default_table, unordered_map<string, Table *> *tables,
      ConditionSqlNode *conditions, int condition_num, FilterConditionNode *&tree);

private:
  vector<FilterUnit *> filter_units_;              // 默认当前都是AND关系（保持向后兼容）
  FilterConditionNode *condition_tree_ = nullptr;  // 新增：条件树结构
};
