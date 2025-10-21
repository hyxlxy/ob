/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by AI Assistant on 2024/12/19.
//

#pragma once

#include "common/sys/rc.h"
#include "sql/stmt/stmt.h"
#include "common/value.h"

class SelectStmt;
class Db;
class BinderContext;

/**
 * @brief 表示UNION语句
 * @ingroup Statement
 * @details UNION语句用于合并多个SELECT语句的结果集
 */
class UnionStmt : public Stmt
{
public:
  UnionStmt() = default;
  ~UnionStmt() override;

  StmtType type() const override { return StmtType::UNION; }

public:
  static RC create(Db *db, UnionSqlNode &union_sql, Stmt *&stmt, const BinderContext *outer_context = nullptr);

public:
  // 访问器方法
  const vector<SelectStmt*> &select_statements() const { return select_statements_; }
  const vector<bool> &distinct_flags() const { return distinct_flags_; }
  const vector<AttrType> &result_types() const { return result_types_; }
  const vector<string> &result_names() const { return result_names_; }

private:
  // 类型兼容性检查和统一
  static RC validate_and_unify_types(UnionStmt *union_stmt);
  
  // 检查两个类型是否兼容
  static bool is_type_compatible(AttrType type1, AttrType type2);
  
  // 从第一个SELECT提取结果集schema
  static RC extract_result_schema(UnionStmt *union_stmt);

private:
  vector<SelectStmt*> select_statements_;  ///< 多个SELECT语句
  vector<bool> distinct_flags_;            ///< 每个UNION操作的标志位，true表示去重，false表示不去重
  vector<AttrType> result_types_;          ///< 结果集列类型（由第一个SELECT决定）
  vector<string> result_names_;            ///< 结果集列名（由第一个SELECT决定）
};
