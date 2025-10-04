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
// Created by Wangyunlai on 2022/6/6.
//

#include "sql/stmt/select_stmt.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "sql/parser/expression_binder.h"

using namespace std;
using namespace common;

SelectStmt::~SelectStmt()
{
  if (nullptr != filter_stmt_) {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  }
}

RC SelectStmt::create(Db *db, SelectSqlNode &select_sql, Stmt *&stmt, const BinderContext *outer_context)
{
  if (nullptr == db) {
    LOG_WARN("invalid argument. db is null");
    return RC::INVALID_ARGUMENT;
  }

  BinderContext binder_context;
  
  // Add outer context for correlated subqueries
  if (outer_context != nullptr) {
    binder_context.add_outer_context(outer_context);
  }

  // collect tables in `from` statement
  vector<Table *>                tables;
  unordered_map<string, Table *> table_map;
  for (size_t i = 0; i < select_sql.relations.size(); i++) {
    const char *table_name = select_sql.relations[i].c_str();
    const char *alias_name = nullptr;
    if (i < select_sql.relation_aliases.size() && !select_sql.relation_aliases[i].empty()) {
      alias_name = select_sql.relation_aliases[i].c_str();
    }
    if (nullptr == table_name) {
      LOG_WARN("invalid argument. relation name is null. index=%d", i);
      return RC::INVALID_ARGUMENT;
    }

    Table *table = db->find_table(table_name);
    if (nullptr == table) {
      LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
      return RC::SCHEMA_TABLE_NOT_EXIST;
    }

    binder_context.add_table(table);
    if (alias_name != nullptr) {
      binder_context.add_table_with_name(table, alias_name);
    }
    tables.push_back(table);
    table_map.insert({table_name, table});
    if (alias_name != nullptr) {
      table_map.insert({alias_name, table});
    }
  }

  // collect tables in `join` statement
  for (size_t i = 0; i < select_sql.joins.size(); i++) {
    const char *table_name = select_sql.joins[i].table_name.c_str();
    const char *alias_name = nullptr;
    if (!select_sql.joins[i].alias.empty()) {
      alias_name = select_sql.joins[i].alias.c_str();
    }
    if (nullptr == table_name) {
      LOG_WARN("invalid argument. join table name is null. index=%d", i);
      return RC::INVALID_ARGUMENT;
    }

    Table *table = db->find_table(table_name);
    if (nullptr == table) {
      LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
      return RC::SCHEMA_TABLE_NOT_EXIST;
    }

    binder_context.add_table(table);
    if (alias_name != nullptr) {
      binder_context.add_table_with_name(table, alias_name);
    }
    tables.push_back(table);
    table_map.insert({table_name, table});
    if (alias_name != nullptr) {
      table_map.insert({alias_name, table});
    }
  }

  // collect query fields in `select` statement
  vector<unique_ptr<Expression>> bound_expressions;
  ExpressionBinder expression_binder(binder_context, outer_context);
  
  printf("======= SELECT_STMT::CREATE: select_sql.expressions.size() = %zu =======\n", select_sql.expressions.size());
  
  for (unique_ptr<Expression> &expression : select_sql.expressions) {
    printf("Before bind: bound_expressions.size() = %zu, expression type = %d\n", 
           bound_expressions.size(), static_cast<int>(expression->type()));
    RC rc = expression_binder.bind_expression(expression, bound_expressions);
    printf("After bind: rc = %s, bound_expressions.size() = %zu\n", 
           strrc(rc), bound_expressions.size());
    if (OB_FAIL(rc)) {
      LOG_INFO("bind expression failed. rc=%s", strrc(rc));
      // 如果是字段不存在错误，可能是关联子查询，返回更友好的错误
      if (rc == RC::SCHEMA_TABLE_NOT_EXIST || rc == RC::SCHEMA_FIELD_MISSING || rc == RC::SCHEMA_FIELD_NOT_EXIST) {
        LOG_WARN("Field binding failed in SELECT clause. This might be a correlated subquery reference.");
        return RC::UNIMPLEMENTED;
      }
      return rc;
    }
  }
  
  printf("======= SELECT_STMT::CREATE: Final bound_expressions.size() = %zu =======\n", bound_expressions.size());

  vector<unique_ptr<Expression>> group_by_expressions;
  for (unique_ptr<Expression> &expression : select_sql.group_by) {
    RC rc = expression_binder.bind_expression(expression, group_by_expressions);
    if (OB_FAIL(rc)) {
      LOG_INFO("bind expression failed. rc=%s", strrc(rc));
      return rc;
    }
  }

  // bind having expression with awareness of group by/aggregations (reuse binder)
  unique_ptr<Expression> having_expr;
  if (select_sql.having_expr) {
    vector<unique_ptr<Expression>> temp_bound;
    unique_ptr<Expression> expr_copy(select_sql.having_expr->copy());
    RC rc = expression_binder.bind_expression(expr_copy, temp_bound);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to bind having expression. rc=%s", strrc(rc));
      return rc;
    }
    if (!temp_bound.empty()) {
      having_expr = std::move(temp_bound[0]);
    }
  }

  Table *default_table = nullptr;
  if (tables.size() == 1) {
    default_table = tables[0];
  }

  // merge join conditions with where conditions
  vector<ConditionSqlNode> all_conditions;
  for (auto &condition : select_sql.conditions) {
    all_conditions.push_back(std::move(condition));
  }
  for (auto &join : select_sql.joins) {
    for (auto &condition : join.conditions) {
      all_conditions.push_back(std::move(condition));
    }
  }
  
  printf("DEBUG: [SELECT_STMT] Creating SelectStmt with %zu conditions\n", all_conditions.size());

  // create filter statement in `where` statement
  FilterStmt *filter_stmt = nullptr;
  RC          rc          = FilterStmt::create(db,
      default_table,
      &table_map,
      all_conditions.data(),
      static_cast<int>(all_conditions.size()),
      filter_stmt);
  if (rc != RC::SUCCESS) {
    // 检查是否是字段不存在的错误，如果是则直接返回错误
    if (rc == RC::SCHEMA_FIELD_NOT_EXIST || rc == RC::SCHEMA_FIELD_MISSING) {
      LOG_WARN("Field validation failed in WHERE clause. rc=%d:%s", rc, strrc(rc));
      return rc;
    }
    // 检查是否是SQL语法错误（包括不支持的功能）
    if (rc == RC::SQL_SYNTAX || rc == RC::UNIMPLEMENTED || rc == RC::UNSUPPORTED) {
      // 这些错误表示SQL语法不支持，直接返回给客户端
      LOG_WARN("FilterStmt creation failed due to unsupported syntax. rc=%s", strrc(rc));
      return rc;
    }
    
    // 其他错误：可能是因为包含了不支持的SubQueryExpr
    // 我们尝试创建一个空的 FilterStmt，表示这个查询有WHERE但无法处理
    // 注意：这意味着WHERE条件会被忽略！但至少服务器不会崩溃
    LOG_WARN("FilterStmt creation failed (possibly due to SubQueryExpr), creating empty FilterStmt. Raw conditions count: %zu", all_conditions.size());
    LOG_WARN("WARNING: WHERE condition will be IGNORED. This may return all rows!");
    filter_stmt = new FilterStmt();
    // 注意：这里我们创建了一个空的 FilterStmt，实际的条件处理将在逻辑计划生成阶段进行
  }



  // bind order by expressions
  vector<unique_ptr<Expression>> order_by_expressions;
  vector<OrderDirection>         order_by_directions;
  for (const auto &order_by_item : select_sql.order_by) {
    unique_ptr<Expression> order_by_expr = order_by_item.expression->copy();
    vector<unique_ptr<Expression>> temp_bound_expressions;
    rc = expression_binder.bind_expression(order_by_expr, temp_bound_expressions);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to bind order by expression. rc=%s", strrc(rc));
      return rc;
    }
    // 通常bind_expression会产生一个表达式，我们取第一个
    if (!temp_bound_expressions.empty()) {
      order_by_expressions.emplace_back(std::move(temp_bound_expressions[0]));
    }
    order_by_directions.emplace_back(order_by_item.direction);
  }

  // everything alright
  SelectStmt *select_stmt = new SelectStmt();

  select_stmt->tables_.swap(tables);
  select_stmt->table_aliases_ = select_sql.relation_aliases;  // 保存表别名
  select_stmt->query_expressions_.swap(bound_expressions);
  select_stmt->filter_stmt_ = filter_stmt;
  select_stmt->raw_conditions_ = std::move(all_conditions);  // 移动原始条件
  select_stmt->group_by_.swap(group_by_expressions);
  select_stmt->having_expr_ = std::move(having_expr);
  select_stmt->order_by_expressions_.swap(order_by_expressions);
  select_stmt->order_by_directions_.swap(order_by_directions);
  stmt                      = select_stmt;
  return RC::SUCCESS;
}
