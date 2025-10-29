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
// Created by AI Assistant on 2024/12/19.
//

#include "sql/stmt/union_stmt.h"
#include "common/log/log.h"
#include "sql/stmt/select_stmt.h"
#include "sql/expr/expression.h"
#include "common/value.h"

using namespace std;
using namespace common;

UnionStmt::~UnionStmt()
{
  // 清理SelectStmt指针
  for (auto *select_stmt : select_statements_) {
    delete select_stmt;
  }
  select_statements_.clear();
}

RC UnionStmt::create(Db *db, UnionSqlNode &union_sql, Stmt *&stmt, const BinderContext *outer_context)
{
  if (nullptr == db) {
    LOG_WARN("invalid argument. db is null");
    return RC::INVALID_ARGUMENT;
  }

  if (union_sql.select_statements.empty()) {
    LOG_WARN("UNION statement has no SELECT statements");
    return RC::INVALID_ARGUMENT;
  }

  // 1. 创建UnionStmt对象
  UnionStmt *union_stmt = new UnionStmt();
  union_stmt->distinct_flags_ = union_sql.distinct_flags;

  // 2. 递归创建所有SELECT语句
  for (auto &select_sql : union_sql.select_statements) {
    SelectStmt *select_stmt = nullptr;
    RC rc = SelectStmt::create(db, *select_sql, (Stmt*&)select_stmt, outer_context);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create SELECT statement for UNION: %s", strrc(rc));
      delete union_stmt;
      return rc;
    }
    union_stmt->select_statements_.push_back(select_stmt);
  }

  // 3. 类型兼容性检查和统一
  RC rc = validate_and_unify_types(union_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to validate and unify types for UNION: %s", strrc(rc));
    delete union_stmt;
    return rc;
  }

  // 4. 提取结果集schema
  rc = extract_result_schema(union_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to extract result schema for UNION: %s", strrc(rc));
    delete union_stmt;
    return rc;
  }

  stmt = union_stmt;
  return RC::SUCCESS;
}

RC UnionStmt::validate_and_unify_types(UnionStmt *union_stmt)
{
  if (union_stmt->select_statements_.empty()) {
    return RC::INVALID_ARGUMENT;
  }

  // 获取第一个SELECT的列信息作为基准
  SelectStmt *first_select = union_stmt->select_statements_[0];
  vector<unique_ptr<Expression>> &first_expressions = first_select->query_expressions();
  
  if (first_expressions.empty()) {
    LOG_ERROR("UNION: first SELECT statement has no expressions");
    return RC::INVALID_ARGUMENT;
  }

  // 检查所有SELECT的列数是否相同
  for (size_t i = 1; i < union_stmt->select_statements_.size(); ++i) {
    SelectStmt *select = union_stmt->select_statements_[i];
    vector<unique_ptr<Expression>> &expressions = select->query_expressions();
    
    if (expressions.size() != first_expressions.size()) {
      LOG_ERROR("UNION: column count mismatch. First SELECT has %zu columns, SELECT %zu has %zu columns",
                first_expressions.size(), i + 1, expressions.size());
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }

    // 检查每列的类型兼容性
    for (size_t j = 0; j < expressions.size(); ++j) {
      AttrType first_type = first_expressions[j]->value_type();
      AttrType current_type = expressions[j]->value_type();
      
      if (!is_type_compatible(first_type, current_type)) {
        LOG_ERROR("UNION: type mismatch at column %zu. First SELECT: %s, SELECT %zu: %s",
                  j + 1, attr_type_to_string(first_type), i + 1, attr_type_to_string(current_type));
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
      }
    }
  }

  return RC::SUCCESS;
}

bool UnionStmt::is_type_compatible(AttrType type1, AttrType type2)
{
  // 相同类型直接兼容
  if (type1 == type2) {
    return true;
  }

  // 数字类型兼容性
  if ((type1 == AttrType::INTS || type1 == AttrType::FLOATS) &&
      (type2 == AttrType::INTS || type2 == AttrType::FLOATS)) {
    return true;
  }

  // 字符串类型兼容性
  if ((type1 == AttrType::CHARS || type1 == AttrType::TEXTS) &&
      (type2 == AttrType::CHARS || type2 == AttrType::TEXTS)) {
    return true;
  }

  // 日期类型兼容性
  if (type1 == AttrType::DATES && type2 == AttrType::DATES) {
    return true;
  }

  // 布尔类型兼容性
  if (type1 == AttrType::BOOLEANS && type2 == AttrType::BOOLEANS) {
    return true;
  }

  // 字符串与数字的兼容性（需要运行时转换）
  if ((type1 == AttrType::CHARS && (type2 == AttrType::INTS || type2 == AttrType::FLOATS)) ||
      (type2 == AttrType::CHARS && (type1 == AttrType::INTS || type1 == AttrType::FLOATS))) {
    return true;
  }

  // 字符串与日期的兼容性
  if ((type1 == AttrType::CHARS && type2 == AttrType::DATES) ||
      (type2 == AttrType::CHARS && type1 == AttrType::DATES)) {
    return true;
  }

  return false;
}

RC UnionStmt::extract_result_schema(UnionStmt *union_stmt)
{
  if (union_stmt->select_statements_.empty()) {
    return RC::INVALID_ARGUMENT;
  }

  // 从第一个SELECT提取列名和类型
  SelectStmt *first_select = union_stmt->select_statements_[0];
  vector<unique_ptr<Expression>> &first_expressions = first_select->query_expressions();
  
  union_stmt->result_types_.clear();
  union_stmt->result_names_.clear();
  
  for (auto &expr : first_expressions) {
    union_stmt->result_types_.push_back(expr->value_type());
    union_stmt->result_names_.push_back(expr->name());
  }

  LOG_INFO("UNION result schema: %zu columns", union_stmt->result_types_.size());
  for (size_t i = 0; i < union_stmt->result_types_.size(); ++i) {
    LOG_INFO("  Column %zu: %s (%s)", i + 1, 
             union_stmt->result_names_[i].c_str(),
             attr_type_to_string(union_stmt->result_types_[i]));
  }

  return RC::SUCCESS;
}
