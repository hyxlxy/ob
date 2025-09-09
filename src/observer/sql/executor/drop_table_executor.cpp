/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "sql/executor/drop_table_executor.h"
#include "sql/stmt/drop_table_stmt.h"
#include "sql/executor/sql_result.h"
#include "session/session.h"
#include "storage/db/db.h"

RC DropTableExecutor::execute(SQLStageEvent *sql_event)
{
  Session       *session = sql_event->session_event()->session();
  Db            *db      = session->get_current_db();
  Stmt          *stmt    = sql_event->stmt();
  DropTableStmt *drop    = dynamic_cast<DropTableStmt *>(stmt);
  if (nullptr == drop) {
    LOG_WARN("DropTableExecutor got invalid stmt");
    return RC::INVALID_ARGUMENT;
  }

  RC rc = db->drop_table(drop->table_name().c_str());
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to drop table %s. rc=%s", drop->table_name().c_str(), strrc(rc));
    return rc;
  }

  sql_event->session_event()->sql_result()->set_return_code(rc);
  return rc;
}

