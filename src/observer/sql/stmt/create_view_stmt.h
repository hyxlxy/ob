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
// Created by AI Assistant on 2024/12/20.
//

#pragma once

#include <string>
#include <vector>
#include "sql/stmt/stmt.h"

class Db;
class SelectStmt;

/**
 * @brief CREATE VIEW语句
 * @ingroup Statement
 * @details 表示创建视图的语句
 */
class CreateViewStmt : public Stmt
{
public:
  CreateViewStmt(std::string view_name, std::vector<std::string> attr_names, SelectStmt *select_stmt);
  virtual ~CreateViewStmt();

  StmtType type() const override { return StmtType::CREATE_VIEW; }

  const std::string &view_name() const { return view_name_; }
  const std::vector<std::string> &attr_names() const { return attr_names_; }
  SelectStmt *select_stmt() const { return select_stmt_; }

  static RC create(Db *db, const CreateViewSqlNode &create_view, Stmt *&stmt);

private:
  std::string view_name_;                ///< 视图名称
  std::vector<std::string> attr_names_;  ///< 属性名称列表（可选）
  SelectStmt *select_stmt_;              ///< SELECT语句
};
