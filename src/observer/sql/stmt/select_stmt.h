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
// Created by Wangyunlai on 2022/6/5.
//

#pragma once

#include "common/sys/rc.h"
#include "sql/stmt/stmt.h"
#include "storage/field/field.h"

class FieldMeta;
class FilterStmt;
class Db;
class Table;
class BinderContext;

/**
 * @brief 表示select语句
 * @ingroup Statement
 */
class SelectStmt : public Stmt
{
public:
  SelectStmt() = default;
  ~SelectStmt() override;

  StmtType type() const override { return StmtType::SELECT; }

public:
  static RC create(Db *db, SelectSqlNode &select_sql, Stmt *&stmt, const BinderContext *outer_context = nullptr);

public:
  const vector<Table *> &tables() const { return tables_; }
  const vector<string> &table_aliases() const { return table_aliases_; }
  FilterStmt            *filter_stmt() const { return filter_stmt_; }
  const vector<ConditionSqlNode> &raw_conditions() const { return raw_conditions_; }

  vector<unique_ptr<Expression>> &query_expressions() { return query_expressions_; }
  vector<unique_ptr<Expression>> &group_by() { return group_by_; }
  Expression                     *having_expr() const { return having_expr_.get(); }
  vector<unique_ptr<Expression>> &order_by_expressions() { return order_by_expressions_; }
  vector<OrderDirection>         &order_by_directions() { return order_by_directions_; }
  int                             limit_count() const { return limit_count_; }

private:
  vector<unique_ptr<Expression>> query_expressions_;
  vector<Table *>                tables_;
  vector<string>                 table_aliases_;  // 与tables_一一对应的别名，空字符串表示无别名
  FilterStmt                    *filter_stmt_ = nullptr;
  vector<ConditionSqlNode>       raw_conditions_;  // 存储原始条件，用于处理 SubQueryExpr
  vector<unique_ptr<Expression>> group_by_;
  unique_ptr<Expression>         having_expr_;
  vector<unique_ptr<Expression>> order_by_expressions_;
  vector<OrderDirection>         order_by_directions_;
  int                            limit_count_ = 0;
};
