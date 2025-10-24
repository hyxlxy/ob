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
// Created by Wangyunlai on 2023/6/13.
//

#pragma once

#include "common/lang/string.h"
#include "common/lang/vector.h"
#include "sql/stmt/stmt.h"
// #include "sql/stmt/select_stmt.h"

class Db;
class SelectStmt;
class Field;
class Table;
/**
 * @brief 表示创建视图的语句
 * @ingroup Statement
 * @details 虽然解析成了stmt，但是与原始的SQL解析后的数据也差不多
 */
class CreateViewStmt : public Stmt
{
public:
  CreateViewStmt(const string &view_name, const vector<string> &attr_names, std::string def);
  virtual ~CreateViewStmt() = default;

  StmtType type() const override { return StmtType::CREATE_VIEW; }

  const string         &view_name() const { return view_name_; }
  const vector<string> &attr_names() const { return attr_names_; }

  unique_ptr<SelectStmt> &select_stmt() { return select_stmt_; }
  bool                    can_be_update() { return can_be_update_; }

  const vector<Table *> base_tables() const { return base_tables_; }

  const string &def() const { return def_; }

  void      set_select_stmt(unique_ptr<SelectStmt> &&select_stmt);
  void      set_can_be_update(bool fl) { can_be_update_ = fl; }
  void      set_query_fields(vector<Field> &&exprs);
  void      set_attr_names(const vector<string> &names) { attr_names_ = names; }
  void      set_def(std::string def) { def_ = std::move(def); }
  void      set_base_tables(vector<Table *> tables) { base_tables_ = std::move(tables); }
  static RC create(Db *db, const CreateViewSqlNode &create_view, Stmt *&stmt);

private:
  string                 view_name_;
  vector<string>         attr_names_;
  string                 def_;
  unique_ptr<SelectStmt> select_stmt_;
  bool                   can_be_update_;
  vector<Field>          query_fields_;
  vector<Table *>        base_tables_;
};
