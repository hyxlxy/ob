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
// ALTER TABLE statement
//

#pragma once

#include "sql/stmt/stmt.h"
#include "sql/parser/parse_defs.h"

class Db;

class AlterTableStmt : public Stmt
{
public:
  AlterTableStmt() = default;
  virtual ~AlterTableStmt() = default;

  StmtType type() const override { return StmtType::ALTER_TABLE; }

public:
  static RC create(Db *db, const ParsedSqlNode::AlterTableSqlNode &node, Stmt *&stmt);

public:
  ParsedSqlNode::AlterOpType op_type = ParsedSqlNode::AlterOpType::ADD_COLUMN;
  string table_name;
  string new_table_name;   // for RENAME TO
  string column_name;      // for ADD/DROP/CHANGE old
  string new_column_name;  // for CHANGE new
  AttrType data_type = AttrType::UNDEFINED; // simplified in this challenge
};


