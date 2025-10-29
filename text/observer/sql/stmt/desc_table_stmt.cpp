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
// Created by Wangyunlai on 2023/6/14.
//

#include "sql/stmt/desc_table_stmt.h"
#include "storage/db/db.h"

RC DescTableStmt::create(Db *db, const DescTableSqlNode &desc_table, Stmt *&stmt)
{
  if (db->find_table(desc_table.relation_name.c_str()) == nullptr) {
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }
  stmt = new DescTableStmt(desc_table.relation_name);
  return RC::SUCCESS;
}

RC DropTableStmt::create(Db *db, const DropTableSqlNode &drop_table, Stmt *&stmt)
{
  const char *table_name = drop_table.relation_name.c_str();
  if (nullptr == db || nullptr == table_name) {
    LOG_WARN("invalid argument. db=%p, table_name=%p", db, table_name);
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    if (drop_table.if_exists) {
      // Table doesn't exist but IF EXISTS was specified, so this is not an error
      LOG_INFO("table %s does not exist, but IF EXISTS was specified", table_name);
      stmt = new DropTableStmt(drop_table.relation_name, drop_table.if_exists);
      return RC::SUCCESS;
    } else {
      LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
      return RC::SCHEMA_TABLE_NOT_EXIST;
    }
  }

  stmt = new DropTableStmt(drop_table.relation_name, drop_table.if_exists);
  return RC::SUCCESS;
}
