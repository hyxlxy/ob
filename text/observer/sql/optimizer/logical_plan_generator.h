/* Copyright (c) 2023 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2023/08/16.
//

#pragma once

#include <vector>
#include "common/lang/memory.h"
#include "common/sys/rc.h"
#include "common/type/attr_type.h"
#include "sql/parser/expression_binder.h"

class Stmt;
class CalcStmt;
class SelectStmt;
struct FilterConditionNode;
class FilterStmt;
class InsertStmt;
class DeleteStmt;
class ExplainStmt;
class UnionStmt;
class LogicalOperator;
class Expression;
class Table;

using std::vector;

class LogicalPlanGenerator
{
public:
  LogicalPlanGenerator()          = default;
  virtual ~LogicalPlanGenerator() = default;

  RC create(Stmt *stmt, unique_ptr<LogicalOperator> &logical_operator);

private:
  RC create_plan(CalcStmt *calc_stmt, unique_ptr<LogicalOperator> &logical_operator);
  RC create_plan(SelectStmt *select_stmt, unique_ptr<LogicalOperator> &logical_operator);
  RC create_plan(FilterStmt *filter_stmt, unique_ptr<LogicalOperator> &logical_operator);
  RC create_plan(InsertStmt *insert_stmt, unique_ptr<LogicalOperator> &logical_operator);
  RC create_plan(DeleteStmt *delete_stmt, unique_ptr<LogicalOperator> &logical_operator);
  RC create_plan(ExplainStmt *explain_stmt, unique_ptr<LogicalOperator> &logical_operator);
  RC create_plan(UnionStmt *union_stmt, unique_ptr<LogicalOperator> &logical_operator);

  RC create_group_by_plan(SelectStmt *select_stmt, unique_ptr<LogicalOperator> &logical_operator);

  int implicit_cast_cost(AttrType from, AttrType to);

  // JOIN条件处理辅助函数
  bool is_join_condition(Expression *expr, const vector<Table *> &tables);
  bool involves_tables(Expression *expr, LogicalOperator *left_oper, LogicalOperator *right_oper);

  // 条件树处理方法
  RC create_expression_from_condition_tree(FilterConditionNode *node, unique_ptr<Expression> &expr);

  // HAVING子句聚合表达式收集辅助函数
  void collect_having_aggregates(Expression *expr, vector<Expression *> &aggregate_expressions);

  void set_out_binder(BinderContext one) { out_binder_ = one; }

private:
  BinderContext binder_context_;
  BinderContext out_binder_;
  bool          binder_ctx_initialized_ = false;
};
