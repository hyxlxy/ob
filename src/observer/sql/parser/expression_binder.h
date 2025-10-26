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
// Created by Wangyunlai on 2024/05/29.
//

#pragma once

#include <unordered_map>
#include "sql/expr/expression.h"

class BinderContext
{
public:
  BinderContext()          = default;
  virtual ~BinderContext() = default;

  // Add a table into current query context and register its real name
  void add_table(Table *table, const char *alias = nullptr)
  {
    query_tables_.push_back(table);
    if (alias != nullptr) {
      table_aliases_.push_back((string)alias);
    } else {
      table_aliases_.push_back("");
    }
    // if (table != nullptr) {
    //   name_to_table_[table->name()] = table;
    // }
  }

  // Register an additional name (e.g., alias) for a table
  RC add_table_with_name(Table *table, const string &name)
  {
    if (name.empty()) {
      return RC::SUCCESS;
    }
    if (name_to_table_.count(name)) {
      return RC::ERROR;
    }
    name_to_table_[name] = table;
    return RC::SUCCESS;
  }
  RC add_table_with_name(Table *table, const char *name)
  {
    if (name == nullptr) {
      return RC::SUCCESS;
    }
    return add_table_with_name(table, (string)name);
  }
  // Add outer query context for correlated subqueries
  void add_outer_context(const BinderContext *outer_context)
  {
    if (outer_context != nullptr) {
      outer_contexts_.push_back(outer_context);
    }
  }

  Table *find_table(const char *table_name) const;

  const vector<Table *> &query_tables() const { return query_tables_; }
  const vector<string>  &table_aliases() const { return table_aliases_; }
  string                 out_view_alias = "no_exist";

private:
  vector<Table *>                     query_tables_;
  vector<string>                      table_aliases_;
  std::unordered_map<string, Table *> name_to_table_;
  vector<const BinderContext *>       outer_contexts_;  // 外层查询上下文
};

/**
 * @brief 绑定表达式
 * @details 绑定表达式，就是在SQL解析后，得到文本描述的表达式，将表达式解析为具体的数据库对象
 */
class ExpressionBinder
{
public:
  ExpressionBinder(BinderContext &context, const BinderContext *outer_context = nullptr)
      : context_(context), outer_context_(outer_context)
  {}
  virtual ~ExpressionBinder() = default;

  RC bind_expression(unique_ptr<Expression> &expr, vector<unique_ptr<Expression>> &bound_expressions);

private:
  RC bind_star_expression(unique_ptr<Expression> &star_expr, vector<unique_ptr<Expression>> &bound_expressions);
  RC bind_unbound_field_expression(
      unique_ptr<Expression> &unbound_field_expr, vector<unique_ptr<Expression>> &bound_expressions);
  RC bind_field_expression(unique_ptr<Expression> &field_expr, vector<unique_ptr<Expression>> &bound_expressions);
  RC bind_value_expression(unique_ptr<Expression> &value_expr, vector<unique_ptr<Expression>> &bound_expressions);
  RC bind_cast_expression(unique_ptr<Expression> &cast_expr, vector<unique_ptr<Expression>> &bound_expressions);
  RC bind_comparison_expression(
      unique_ptr<Expression> &comparison_expr, vector<unique_ptr<Expression>> &bound_expressions);
  RC bind_conjunction_expression(
      unique_ptr<Expression> &conjunction_expr, vector<unique_ptr<Expression>> &bound_expressions);
  RC bind_arithmetic_expression(
      unique_ptr<Expression> &arithmetic_expr, vector<unique_ptr<Expression>> &bound_expressions);
  RC bind_function_expression(unique_ptr<Expression> &function_expr, vector<unique_ptr<Expression>> &bound_expressions);
  RC bind_aggregate_expression(
      unique_ptr<Expression> &aggregate_expr, vector<unique_ptr<Expression>> &bound_expressions);
  RC bind_list_expression(unique_ptr<Expression> &list_expr, vector<unique_ptr<Expression>> &bound_expressions);

private:
  BinderContext       &context_;
  const BinderContext *outer_context_;
};
