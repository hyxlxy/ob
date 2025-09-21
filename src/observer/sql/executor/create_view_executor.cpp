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

#include "sql/executor/create_view_executor.h"
#include "sql/stmt/create_view_stmt.h"
#include "event/sql_event.h"
#include "event/session_event.h"
#include "session/session.h"
#include "storage/db/db.h"
#include "common/log/log.h"

/**
 * @brief 从CREATE VIEW SQL中提取SELECT部分
 * @param create_view_sql 完整的CREATE VIEW SQL语句
 * @return SELECT部分的SQL文本
 */
string extract_select_from_create_view(const string &create_view_sql)
{
  string lower_sql = create_view_sql;
  std::transform(lower_sql.begin(), lower_sql.end(), lower_sql.begin(), ::tolower);
  
  // 查找 "as" 关键字
  size_t as_pos = lower_sql.find(" as ");
  if (as_pos == string::npos) {
    return "";
  }
  
  // 返回AS之后的部分
  size_t select_start = as_pos + 4; // " as " 的长度是4
  return create_view_sql.substr(select_start);
}

RC CreateViewExecutor::execute(SQLStageEvent *sql_event)
{
  Stmt *stmt = sql_event->stmt();
  Session *session = sql_event->session_event()->session();
  
  ASSERT(stmt->type() == StmtType::CREATE_VIEW,
         "create view executor can not run this command: %d",
         static_cast<int>(stmt->type()));

  CreateViewStmt *create_view_stmt = static_cast<CreateViewStmt *>(stmt);
  const string &view_name = create_view_stmt->view_name();
  const vector<string> &attr_names = create_view_stmt->attr_names();
  SelectStmt *select_stmt = create_view_stmt->select_stmt();

  if (select_stmt == nullptr) {
    LOG_ERROR("CREATE VIEW statement missing SELECT clause");
    return RC::INVALID_ARGUMENT;
  }

  // 从原始SQL中提取SELECT部分
  string select_sql = extract_select_from_create_view(sql_event->sql());
  if (select_sql.empty()) {
    LOG_ERROR("Failed to extract SELECT clause from CREATE VIEW statement");
    return RC::SQL_SYNTAX;
  }

  // 创建视图
  RC rc = session->get_current_db()->create_view(view_name.c_str(), attr_names, select_sql, select_stmt);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to create view: %s", view_name.c_str());
    return rc;
  }

  LOG_INFO("Successfully created view: %s", view_name.c_str());
  return RC::SUCCESS;
}
