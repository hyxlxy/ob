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

#include "sql/executor/drop_view_executor.h"
#include "sql/stmt/drop_view_stmt.h"
#include "event/sql_event.h"
#include "event/session_event.h"
#include "session/session.h"
#include "storage/db/db.h"
#include "common/log/log.h"

RC DropViewExecutor::execute(SQLStageEvent *sql_event)
{
  Stmt *stmt = sql_event->stmt();
  Session *session = sql_event->session_event()->session();
  
  ASSERT(stmt->type() == StmtType::DROP_VIEW,
         "drop view executor can not run this command: %d",
         static_cast<int>(stmt->type()));

  DropViewStmt *drop_view_stmt = static_cast<DropViewStmt *>(stmt);
  const string &view_name = drop_view_stmt->view_name();

  // 删除视图
  RC rc = session->get_current_db()->drop_view(view_name.c_str());
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to drop view: %s", view_name.c_str());
    return rc;
  }

  LOG_INFO("Successfully dropped view: %s", view_name.c_str());
  return RC::SUCCESS;
}
