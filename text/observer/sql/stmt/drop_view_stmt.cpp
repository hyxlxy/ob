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

#include "sql/stmt/drop_view_stmt.h"
#include "storage/db/db.h"
#include "common/log/log.h"
#include "event/sql_debug.h"

DropViewStmt::DropViewStmt(std::string view_name) : view_name_(std::move(view_name))
{}

RC DropViewStmt::create(Db *db, const DropViewSqlNode &drop_view, Stmt *&stmt)
{
  if (db == nullptr) {
    LOG_WARN("invalid argument. db is null");
    return RC::INVALID_ARGUMENT;
  }

  // 检查视图是否存在
  Table *table = db->find_table(drop_view.relation_name.c_str());
  if (table == nullptr) {
    LOG_WARN("view does not exist: %s", drop_view.relation_name.c_str());
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  // TODO: 检查是否为视图而不是表
  // 当前简化实现，将视图作为特殊的表处理

  stmt = new DropViewStmt(drop_view.relation_name);
  sql_debug("drop view statement: view name %s", drop_view.relation_name.c_str());
  return RC::SUCCESS;
}
