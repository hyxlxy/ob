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

#include "sql/stmt/create_view_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "storage/db/db.h"
#include "common/log/log.h"
#include "event/sql_debug.h"

CreateViewStmt::CreateViewStmt(std::string view_name, std::vector<std::string> attr_names, SelectStmt *select_stmt)
    : view_name_(std::move(view_name)), attr_names_(std::move(attr_names)), select_stmt_(select_stmt)
{}

CreateViewStmt::~CreateViewStmt()
{
  if (select_stmt_ != nullptr) {
    delete select_stmt_;
    select_stmt_ = nullptr;
  }
}

RC CreateViewStmt::create(Db *db, const CreateViewSqlNode &create_view, Stmt *&stmt)
{
  if (db == nullptr) {
    LOG_WARN("invalid argument. db is null");
    return RC::INVALID_ARGUMENT;
  }

  // 检查视图名是否已存在（作为表或视图）
  if (db->find_table(create_view.relation_name.c_str()) != nullptr) {
    LOG_WARN("table or view already exists: %s", create_view.relation_name.c_str());
    return RC::SCHEMA_TABLE_EXIST;
  }

  // 创建SELECT语句
  SelectStmt *select_stmt = nullptr;
  if (create_view.create_view_select) {
    Stmt *select_stmt_base = nullptr;
    RC rc = SelectStmt::create(db, *create_view.create_view_select, select_stmt_base);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create select statement for view: %s", create_view.relation_name.c_str());
      return rc;
    }
    select_stmt = static_cast<SelectStmt *>(select_stmt_base);
  } else {
    LOG_WARN("create view statement missing select clause");
    return RC::INVALID_ARGUMENT;
  }

  // 验证列名数量匹配
  if (!create_view.attribute_names.empty()) {
    size_t select_column_count = select_stmt->query_expressions().size();
    if (create_view.attribute_names.size() != select_column_count) {
      LOG_WARN("column count mismatch in view definition. specified: %zu, select: %zu", 
               create_view.attribute_names.size(), select_column_count);
      delete select_stmt;
      return RC::INVALID_ARGUMENT;
    }
  }

  stmt = new CreateViewStmt(create_view.relation_name, create_view.attribute_names, select_stmt);
  sql_debug("create view statement: view name %s", create_view.relation_name.c_str());
  return RC::SUCCESS;
}
