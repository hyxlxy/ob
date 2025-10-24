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

#include "sql/executor/create_view_executor.h"

#include "common/log/log.h"
#include "event/session_event.h"
#include "event/sql_event.h"
#include "session/session.h"
#include "sql/stmt/create_view_stmt.h"
#include "storage/db/db.h"
#include "storage/trx/trx.h"
#include "storage/table/view.h"

RC CreateViewExecutor::execute(SQLStageEvent *sql_event)
{
  Stmt    *stmt    = sql_event->stmt();
  Session *session = sql_event->session_event()->session();
  RC       rc      = RC::SUCCESS;
  ASSERT(stmt->type() == StmtType::CREATE_VIEW,
            "create table executor can not run this command: %d",
            static_cast<int>(stmt->type()));

  //
  //以下内容是检测

  auto &&physical_operator = sql_event->physical_operator();
  if (physical_operator) {
    // 检测一下select是否有没有问题
    // 这里sql_event里的物理操作符存放的是as之后的查询语句
    rc = physical_operator->open(session->current_trx());

    CHECK_RETURN_RC(rc);
    while ((rc = physical_operator->next()) == RC::SUCCESS)
      ;
    if (rc != RC::RECORD_EOF)
      return rc;
    physical_operator->close();
  }

  CreateViewStmt *create_view_stmt = static_cast<CreateViewStmt *>(stmt);
  //   const string   &view_name        = create_view_stmt->view_name();

  {
    Table *table = session->get_current_db()->find_table("ad_views");

    auto make_values = [&](vector<Value>        &values,
                           const string         &name,
                           const vector<string> &attr_names,
                           bool                  can_be_update,
                           string                def) {
      Value one;
      one.set_string(name.c_str());
      values.push_back(std::move(one));

      one.set_int(can_be_update);
      values.push_back(std::move(one));

      string attr_sum;
      for (auto &&attr : attr_names) {
        attr_sum += attr;
        attr_sum += ",";
      }
      one.set_string(attr_sum.c_str());
      values.push_back(one);

      one.set_string(def.c_str());
      // cout << "view_vlaues: def: " << one.get_string() << endl;
      values.push_back(one);
    };
    std::vector<Value> values;
    auto             &&stmt = create_view_stmt;
    make_values(values, stmt->view_name(), stmt->attr_names(), stmt->can_be_update(), stmt->def());
    Record record;
    rc = table->make_record(values.size(), values.data(), record);
    CHECK_RETURN_RC(rc);

    rc = session->current_trx()->insert_record(table, record);
    CHECK_RETURN_RC(rc);
    // View *new_view = new View();
    rc = session->get_current_db()->create_view(
        stmt->view_name(), stmt->can_be_update(), stmt->attr_names(), stmt->def(), stmt->base_tables());
    CHECK_RETURN_RC(rc);
  }

  return RC::SUCCESS;
}