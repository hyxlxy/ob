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

#include "common/log/log.h"
#include "common/types.h"
#include "sql/stmt/create_table_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "event/sql_debug.h"

RC CreateTableStmt::create(Db *db, const CreateTableSqlNode &create_table, Stmt *&stmt)
{
  StorageFormat storage_format = get_storage_format(create_table.storage_format.c_str());
  if (storage_format == StorageFormat::UNKNOWN_FORMAT) {
    return RC::INVALID_ARGUMENT;
  }

  // Check if this is CREATE TABLE AS SELECT
  if (create_table.as_select != nullptr) {
    // Create SelectStmt from the AS SELECT clause
    Stmt *select_stmt = nullptr;
    RC rc = SelectStmt::create(db, *create_table.as_select, select_stmt);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create select statement for CREATE TABLE AS SELECT. rc=%s", strrc(rc));
      return rc;
    }

    stmt = new CreateTableStmt(create_table.relation_name, static_cast<SelectStmt*>(select_stmt), storage_format);
    sql_debug("create table as select statement: table name %s", create_table.relation_name.c_str());
  } else {
    // Regular CREATE TABLE
    stmt = new CreateTableStmt(create_table.relation_name, create_table.attr_infos, create_table.primary_keys, storage_format);
    sql_debug("create table statement: table name %s", create_table.relation_name.c_str());
  }

  return RC::SUCCESS;
}

StorageFormat CreateTableStmt::get_storage_format(const char *format_str) {
  StorageFormat format = StorageFormat::UNKNOWN_FORMAT;
  if (strlen(format_str) == 0) {
    format = StorageFormat::ROW_FORMAT;
  } else if (0 == strcasecmp(format_str, "ROW")) {
    format = StorageFormat::ROW_FORMAT;
  } else if (0 == strcasecmp(format_str, "PAX")) {
    format = StorageFormat::PAX_FORMAT;
  } else {
    format = StorageFormat::UNKNOWN_FORMAT;
  }
  return format;
}
