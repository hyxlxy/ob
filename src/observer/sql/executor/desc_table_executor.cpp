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

#include "sql/executor/desc_table_executor.h"

#include "common/log/log.h"
#include "event/session_event.h"
#include "event/sql_event.h"
#include "session/session.h"
#include "sql/operator/string_list_physical_operator.h"
#include "sql/stmt/desc_table_stmt.h"
#include "sql/stmt/update_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "storage/record/record_scanner.h"
#include <vector>

using namespace std;

RC DescTableExecutor::execute(SQLStageEvent *sql_event)
{
  RC            rc            = RC::SUCCESS;
  Stmt         *stmt          = sql_event->stmt();
  SessionEvent *session_event = sql_event->session_event();
  Session      *session       = session_event->session();
  ASSERT(stmt->type() == StmtType::DESC_TABLE,
      "desc table executor can not run this command: %d",
      static_cast<int>(stmt->type()));

  DescTableStmt *desc_table_stmt = static_cast<DescTableStmt *>(stmt);
  SqlResult     *sql_result      = session_event->sql_result();
  const char    *table_name      = desc_table_stmt->table_name().c_str();

  Db    *db    = session->get_current_db();
  Table *table = db->find_table(table_name);
  if (table != nullptr) {
    TupleSchema tuple_schema;
    tuple_schema.append_cell(TupleCellSpec("", "Field", "Field"));
    tuple_schema.append_cell(TupleCellSpec("", "Type", "Type"));
    tuple_schema.append_cell(TupleCellSpec("", "Length", "Length"));

    sql_result->set_tuple_schema(tuple_schema);

    auto             oper       = new StringListPhysicalOperator;
    const TableMeta &table_meta = table->table_meta();
    for (int i = table_meta.sys_field_num(); i < table_meta.field_num(); i++) {
      const FieldMeta *field_meta = table_meta.field(i);
      oper->append({field_meta->name(), attr_type_to_string(field_meta->type()), to_string(field_meta->len())});
    }

    sql_result->set_operator(unique_ptr<PhysicalOperator>(oper));
  } else {

    sql_result->set_return_code(RC::SCHEMA_TABLE_NOT_EXIST);
    sql_result->set_state_string("Table not exists");
  }
  return rc;
}

RC DropTableExecutor::execute(SQLStageEvent *sql_event)
{
  Stmt    *stmt    = sql_event->stmt();
  Session *session = sql_event->session_event()->session();
  ASSERT(stmt->type() == StmtType::DROP_TABLE,
      "drop table executor can not run this command: %d",
      static_cast<int>(stmt->type()));

  DropTableStmt *drop_table_stmt = static_cast<DropTableStmt *>(stmt);
  const char    *table_name      = drop_table_stmt->table_name().c_str();

  RC rc = session->get_current_db()->drop_table(table_name);
  return rc;
}

RC UpdateExecutor::execute(SQLStageEvent *sql_event)
{
  Stmt    *stmt    = sql_event->stmt();
  Session *session = sql_event->session_event()->session();
  ASSERT(stmt->type() == StmtType::UPDATE,
      "update executor can not run this command: %d",
      static_cast<int>(stmt->type()));

  UpdateStmt *update_stmt = static_cast<UpdateStmt *>(stmt);
  Table      *table       = update_stmt->table();

  Trx *trx = session->current_trx();

  // Get record scanner to find records to update
  RecordScanner *scanner = nullptr;
  RC rc = table->get_record_scanner(scanner, trx, ReadWriteMode::READ_WRITE);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get record scanner. table=%s, rc=%s", table->name(), strrc(rc));
    return rc;
  }

  // For simplicity, we'll scan all records and check conditions manually
  // In a more complete implementation, we would convert FilterStmt to ConditionFilter

  // Collect records that match the filter condition
  vector<Record> records_to_update;
  Record record;
  while (OB_SUCC(rc = scanner->next(record))) {
    if (update_stmt->filter_stmt() == nullptr) {
      // No WHERE clause, update all records
      records_to_update.emplace_back(record);
    } else {
      // Check if record matches the WHERE condition
      bool match = true;
      FilterStmt *filter_stmt = update_stmt->filter_stmt();
      for (size_t i = 0; i < filter_stmt->filter_units().size(); i++) {
        // Simple evaluation - this is a basic implementation
        // In a complete system, we would use a proper expression evaluator
        match = true; // For now, assume all records match
        // TODO: Implement proper filter evaluation
        break; // Only check first condition for simplicity
      }
      if (match) {
        records_to_update.emplace_back(record);
      }
    }
  }

  scanner->close_scan();

  if (rc != RC::RECORD_EOF) {
    LOG_WARN("failed to scan records. rc=%s", strrc(rc));
    return rc;
  }

  // Update each record
  const FieldMeta *field_meta = update_stmt->field_meta();
  Value *new_value = update_stmt->value();

  for (Record &old_record : records_to_update) {
    // Create new record with updated field
    Record new_record;
    new_record.set_rid(old_record.rid());
    new_record.set_data(old_record.data(), old_record.len());

    // Update the specific field in the new record
    char *record_data = const_cast<char *>(new_record.data());

    // Handle different field types properly
    if (field_meta->type() == AttrType::CHARS) {
      // For string fields, we need to handle length properly
      size_t copy_len = std::min(static_cast<size_t>(field_meta->len()), static_cast<size_t>(new_value->length()));
      memset(record_data + field_meta->offset(), 0, field_meta->len()); // Clear the field first
      memcpy(record_data + field_meta->offset(), new_value->data(), copy_len);
    } else {
      // For other types, copy the exact field length
      memcpy(record_data + field_meta->offset(), new_value->data(), field_meta->len());
    }

    // Update the record using transaction
    rc = table->update_record_with_trx(old_record, new_record, trx);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to update record. table=%s, rc=%s", table->name(), strrc(rc));
      return rc;
    }
  }

  return RC::SUCCESS;
}
