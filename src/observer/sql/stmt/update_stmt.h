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

#include "common/sys/rc.h"
#include "sql/stmt/stmt.h"

class Table;
class Db;
class FieldMeta;
class FilterStmt;
class Stmt;

/**
 * @brief 更新语句
 * @ingroup Statement
 */
class UpdateStmt : public Stmt
{
public:
  UpdateStmt() = default;
  UpdateStmt(Table *table, Value *values, int value_amount);
  UpdateStmt(Table *table, const FieldMeta *field_meta, Value *value, FilterStmt *filter_stmt);
  UpdateStmt(Table *table, const FieldMeta *field_meta, Stmt *sub_query_stmt, FilterStmt *filter_stmt);
  UpdateStmt(Table *table, const vector<const FieldMeta *> &field_metas, const vector<Value> &values, FilterStmt *filter_stmt);
  virtual ~UpdateStmt();

  StmtType type() const override { return StmtType::UPDATE; }

public:
  static RC create(Db *db, const UpdateSqlNode &update_sql, Stmt *&stmt);

public:
  Table *table() const { return table_; }
  Value *values() const { return values_; }
  int    value_amount() const { return value_amount_; }

  const FieldMeta *field_meta() const { return field_meta_; }
  Value *value() const { return value_; }
  FilterStmt *filter_stmt() const { return filter_stmt_; }
  bool is_sub_query() const { return sub_query_stmt_ != nullptr; }
  Stmt *sub_query_stmt() const { return sub_query_stmt_; }
  
  // 多字段UPDATE支持
  bool is_multi_field() const { return !field_metas_.empty(); }
  const vector<const FieldMeta *> &field_metas() const { return field_metas_; }
  const vector<Value> &multi_values() const { return multi_values_; }
  const vector<Stmt *> &multi_sub_queries() const { return multi_sub_queries_; }
  const vector<bool>  &multi_is_sub_query() const { return multi_is_sub_query_; }

private:
  Table *table_        = nullptr;
  Value *values_       = nullptr;
  int    value_amount_ = 0;

  // Single field update
  const FieldMeta *field_meta_ = nullptr;
  Value *value_ = nullptr;
  FilterStmt *filter_stmt_ = nullptr;
  Stmt *sub_query_stmt_ = nullptr;
  
  // Multi-field update
  vector<const FieldMeta *> field_metas_;
  vector<Value> multi_values_;
  vector<Stmt *> multi_sub_queries_;
  vector<bool>  multi_is_sub_query_;
};
