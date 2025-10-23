/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "sql/executor/alter_table_executor.h"
#include "common/log/log.h"
#include "event/session_event.h"
#include "event/sql_event.h"
#include "session/session.h"
#include "sql/stmt/alter_table_stmt.h"
#include "storage/db/db.h"
#include "storage/common/meta_util.h"
#include "storage/table/table.h"
#include <filesystem>

RC AlterTableExecutor::execute(SQLStageEvent *sql_event)
{
  Stmt    *stmt    = sql_event->stmt();
  Session *session = sql_event->session_event()->session();
  ASSERT(stmt->type() == StmtType::ALTER_TABLE,
      "alter table executor can not run this command: %d",
      static_cast<int>(stmt->type()));

  AlterTableStmt *alter_stmt = static_cast<AlterTableStmt *>(stmt);

  Db *db = session->get_current_db();
  Table *table = db->find_table(alter_stmt->table_name.c_str());
  if (table == nullptr) {
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  // 目前先实现 RENAME TO 和 CHANGE COLUMN 改名；ADD/DROP 留出返回未实现
  if (alter_stmt->op_type == ParsedSqlNode::AlterOpType::RENAME_TABLE) {
    const std::string &old_name = alter_stmt->table_name;
    const std::string &new_name = alter_stmt->new_table_name;
    if (new_name.empty()) return RC::INVALID_ARGUMENT;
    if (old_name == new_name) return RC::SUCCESS; // no-op rename should succeed
    if (db->find_table(new_name.c_str()) != nullptr) return RC::SCHEMA_TABLE_EXIST;
    // 全量委托 DB：执行文件改名、刷新元数据内部名称与 opened_tables_ 映射
    return db->rename_table(old_name.c_str(), new_name.c_str());
  }

  if (alter_stmt->op_type == ParsedSqlNode::AlterOpType::CHANGE_COLUMN_RENAME) {
    // 只改名不改类型：委托给 Table 层完成元数据原子更新与索引同步
    const TableMeta &meta = table->table_meta();
    const FieldMeta *field = meta.field(alter_stmt->column_name.c_str());
    if (field == nullptr) return RC::SCHEMA_FIELD_MISSING;

    return table->alter_change_column_rename(alter_stmt->column_name.c_str(), alter_stmt->new_column_name.c_str());
  }

  if (alter_stmt->op_type == ParsedSqlNode::AlterOpType::ADD_COLUMN || alter_stmt->op_type == ParsedSqlNode::AlterOpType::DROP_COLUMN) {
    if (alter_stmt->op_type == ParsedSqlNode::AlterOpType::ADD_COLUMN) {
      // 语义：仅支持 INT 新列，默认值 0
      return table->alter_add_column_int(alter_stmt->column_name.c_str());
    } else {
      return table->alter_drop_column(alter_stmt->column_name.c_str());
    }
  }

  return RC::UNIMPLEMENTED;
}


