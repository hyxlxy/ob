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

#include "sql/executor/create_table_executor.h"

#include "common/log/log.h"
#include "event/session_event.h"
#include "event/sql_event.h"
#include "session/session.h"
#include "sql/stmt/create_table_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "sql/optimizer/logical_plan_generator.h"
#include "sql/optimizer/physical_plan_generator.h"
#include "sql/operator/physical_operator.h"
#include "storage/db/db.h"
#include "storage/table/table.h"

RC CreateTableExecutor::execute(SQLStageEvent *sql_event)
{
  Stmt    *stmt    = sql_event->stmt();
  Session *session = sql_event->session_event()->session();
  ASSERT(stmt->type() == StmtType::CREATE_TABLE,
      "create table executor can not run this command: %d",
      static_cast<int>(stmt->type()));

  CreateTableStmt *create_table_stmt = static_cast<CreateTableStmt *>(stmt);
  const char *table_name = create_table_stmt->table_name().c_str();

  if (create_table_stmt->is_as_select()) {
    // Handle CREATE TABLE AS SELECT
    return execute_create_table_as_select(session, create_table_stmt);
  } else {
    // Handle regular CREATE TABLE
    RC rc = session->get_current_db()->create_table(table_name, create_table_stmt->attr_infos(), create_table_stmt->primary_keys(), create_table_stmt->storage_format());
    return rc;
  }
}

RC CreateTableExecutor::execute_create_table_as_select(Session *session, CreateTableStmt *create_table_stmt)
{
  const char *table_name = create_table_stmt->table_name().c_str();
  SelectStmt *select_stmt = create_table_stmt->select_stmt();

  LOG_INFO("Starting CREATE TABLE AS SELECT for table: %s", table_name);

  if (select_stmt == nullptr) {
    LOG_ERROR("SelectStmt is null for CREATE TABLE AS SELECT");
    return RC::INTERNAL;
  }

  // Step 1: Execute the SELECT statement to get the data and schema
  LogicalPlanGenerator logical_plan_generator;
  std::unique_ptr<LogicalOperator> logical_operator;
  RC rc = logical_plan_generator.create(select_stmt, logical_operator);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("failed to create logical plan for CREATE TABLE AS SELECT. rc=%s", strrc(rc));
    return rc;
  }

  LOG_INFO("Logical plan created successfully");

  PhysicalPlanGenerator physical_plan_generator;
  std::unique_ptr<PhysicalOperator> physical_operator;
  rc = physical_plan_generator.create(*logical_operator, physical_operator, session);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("failed to create physical plan for CREATE TABLE AS SELECT. rc=%s", strrc(rc));
    return rc;
  }

  LOG_INFO("Physical plan created successfully");

  // Step 2: Open the physical operator to get the result schema
  rc = physical_operator->open(nullptr);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open physical operator for CREATE TABLE AS SELECT. rc=%s", strrc(rc));
    return rc;
  }

  // Step 3: Derive table schema from SELECT result
  vector<AttrInfoSqlNode> attr_infos;

  // Execute the first row to determine schema
  rc = physical_operator->next();
  if (rc != RC::SUCCESS) {
    LOG_ERROR("No data available to determine schema for CREATE TABLE AS SELECT. rc=%s", strrc(rc));
    physical_operator->close();
    return RC::INTERNAL;
  }

  // Get the first tuple to determine schema
  Tuple *first_tuple = physical_operator->current_tuple();
  if (first_tuple == nullptr) {
    LOG_ERROR("Current tuple is null for schema derivation");
    physical_operator->close();
    return RC::INTERNAL;
  }

  int cell_count = first_tuple->cell_num();
  LOG_INFO("Schema derivation: found %d columns", cell_count);

  for (int i = 0; i < cell_count; i++) {
    AttrInfoSqlNode attr_info;

    // Generate column name
    attr_info.name = "col" + std::to_string(i + 1);

    // Get type information from the first tuple
    Value cell_value;
    RC rc = first_tuple->cell_at(i, cell_value);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to get cell value for schema derivation. rc=%s", strrc(rc));
      physical_operator->close();
      return rc;
    }

    // Determine type and length based on the cell value
    attr_info.type = cell_value.attr_type();
    attr_info.nullable = true;

    switch (attr_info.type) {
      case AttrType::INTS:
        attr_info.length = 4;
        break;
      case AttrType::FLOATS:
        attr_info.length = 4;
        break;
      case AttrType::CHARS:
        attr_info.length = cell_value.length() > 0 ? cell_value.length() : 4;
        break;
      case AttrType::DATES:
        attr_info.length = 4;
        break;
      default:
        attr_info.type = AttrType::INTS;
        attr_info.length = 4;
        break;
    }

    LOG_INFO("Column %d: name=%s, type=%d, length=%d", i, attr_info.name.c_str(),
             static_cast<int>(attr_info.type), attr_info.length);

    attr_infos.push_back(attr_info);
  }

  // Step 4: Create the table with derived schema
  vector<string> primary_keys; // No primary keys for CREATE TABLE AS SELECT
  rc = session->get_current_db()->create_table(table_name, attr_infos, primary_keys, create_table_stmt->storage_format());
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create table for CREATE TABLE AS SELECT. rc=%s", strrc(rc));
    physical_operator->close();
    return rc;
  }

  // Step 5: Insert data from SELECT result into the new table
  Table *table = session->get_current_db()->find_table(table_name);
  if (table == nullptr) {
    LOG_WARN("failed to find newly created table %s", table_name);
    physical_operator->close();
    return RC::INTERNAL;
  }

  // Reset the physical operator to start from the beginning
  // We need to close and reopen to get all data, not just continue from where we left off
  physical_operator->close();
  rc = physical_operator->open(nullptr);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to reopen physical operator for data insertion. rc=%s", strrc(rc));
    return rc;
  }

  // Insert each row from the SELECT result
  while ((rc = physical_operator->next()) == RC::SUCCESS) {
    Tuple *tuple = physical_operator->current_tuple();
    if (tuple == nullptr) {
      LOG_WARN("failed to get current tuple from SELECT result");
      rc = RC::INTERNAL;
      break;
    }

    // Convert tuple to record and insert
    vector<Value> values;
    for (int i = 0; i < tuple->cell_num(); i++) {
      Value value;
      rc = tuple->cell_at(i, value);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get cell value from tuple. rc=%s", strrc(rc));
        break;
      }
      values.push_back(value);
    }

    if (rc != RC::SUCCESS) {
      break;
    }

    Record record;
    rc = table->make_record(static_cast<int>(values.size()), values.data(), record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to make record for CREATE TABLE AS SELECT. rc=%s", strrc(rc));
      break;
    }

    rc = table->insert_record(record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to insert record for CREATE TABLE AS SELECT. rc=%s", strrc(rc));
      break;
    }
  }

  physical_operator->close();

  if (rc == RC::RECORD_EOF) {
    rc = RC::SUCCESS; // End of data is expected
  }

  return rc;
}