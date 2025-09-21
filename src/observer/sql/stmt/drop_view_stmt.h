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
#include "sql/stmt/stmt.h"

class Db;

/**
 * @brief DROP VIEW语句
 * @ingroup Statement
 * @details 表示删除视图的语句
 */
class DropViewStmt : public Stmt
{
public:
  explicit DropViewStmt(std::string view_name);
  virtual ~DropViewStmt() = default;

  StmtType type() const override { return StmtType::DROP_VIEW; }

  const std::string &view_name() const { return view_name_; }

  static RC create(Db *db, const DropViewSqlNode &drop_view, Stmt *&stmt);

private:
  std::string view_name_;  ///< 视图名称
};
