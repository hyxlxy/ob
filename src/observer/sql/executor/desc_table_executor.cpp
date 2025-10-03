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
#include "sql/stmt/select_stmt.h"
#include "sql/expr/expression.h"
#include "sql/optimizer/logical_plan_generator.h"
#include "sql/optimizer/physical_plan_generator.h"
#include "sql/operator/physical_operator.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "storage/record/record_scanner.h"
#include <vector>

using namespace std;

// Forward declaration
RC execute_sub_query(Stmt *sub_query_stmt, Session *session, Value &result_value);

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

  // Check if table exists
  Table *table = session->get_current_db()->find_table(table_name);
  if (nullptr == table) {
    if (drop_table_stmt->if_exists()) {
      // Table doesn't exist but IF EXISTS was specified, so this is not an error
      LOG_INFO("table %s does not exist, but IF EXISTS was specified", table_name);
      return RC::SUCCESS;
    } else {
      LOG_WARN("no such table. db=%s, table_name=%s", session->get_current_db()->name(), table_name);
      return RC::SCHEMA_TABLE_NOT_EXIST;
    }
  }

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

  // ========== 完全重写简化的UpdateExecutor ==========
  UpdateStmt *update_stmt = static_cast<UpdateStmt *>(stmt);
  Table *table = update_stmt->table();
  Trx *trx = session->current_trx();
  
  // 获取scanner
  RecordScanner *scanner = nullptr;
  RC rc = table->get_record_scanner(scanner, trx, ReadWriteMode::READ_WRITE);
  if (rc != RC::SUCCESS) {
    return rc;
  }
  
  // 收集要更新的记录
  vector<Record> records;
  Record rec;
  FilterStmt *fs = update_stmt->filter_stmt();
  bool has_where = (fs && !fs->filter_units().empty());
  
  while (scanner->next(rec) == RC::SUCCESS) {
    if (!has_where) {
      records.push_back(rec);
      continue;
    }
    
    // 评估WHERE条件
    bool match = true;
    for (const FilterUnit *fu : fs->filter_units()) {
      Value lv, rv;
      
      // 获取左值
      if (fu->left().is_attr()) {
        const FieldMeta *fm = fu->left().field.meta();
        lv.set_type(fm->type());
        lv.set_data(rec.data() + fm->offset(), fm->len());
      } else if (fu->left().is_value()) {
        lv = fu->left().value;
      }
      
      // 获取右值
      if (fu->right().is_attr()) {
        const FieldMeta *fm = fu->right().field.meta();
        rv.set_type(fm->type());
        rv.set_data(rec.data() + fm->offset(), fm->len());
      } else if (fu->right().is_value()) {
        rv = fu->right().value;
      }
      
      // 比较
      int cmp = lv.compare(rv);
      bool cond_match = false;
      switch (fu->comp()) {
        case EQUAL_TO: cond_match = (cmp == 0); break;
        case LESS_THAN: cond_match = (cmp < 0); break;
        case GREAT_THAN: cond_match = (cmp > 0); break;
        case LESS_EQUAL: cond_match = (cmp <= 0); break;
        case GREAT_EQUAL: cond_match = (cmp >= 0); break;
        case NOT_EQUAL: cond_match = (cmp != 0); break;
        case IS_NULL_OP: cond_match = (lv.attr_type() == AttrType::UNDEFINED); break;
        case IS_NOT_NULL_OP: cond_match = (lv.attr_type() != AttrType::UNDEFINED); break;
        default: cond_match = false;
      }
      
      if (!cond_match) {
        match = false;
        break;
      }
    }
    
    if (match) {
      records.push_back(rec);
    }
  }
  scanner->close_scan();
  
  // 执行更新
  if (update_stmt->is_multi_field()) {
    // 多字段UPDATE
    for (Record &old_rec : records) {
      Record new_rec;
      new_rec.copy_data(old_rec.data(), old_rec.len());
      new_rec.set_rid(old_rec.rid());
      
      for (size_t i = 0; i < update_stmt->field_metas().size(); i++) {
        const FieldMeta *fm = update_stmt->field_metas()[i];
        const Value &val = update_stmt->multi_values()[i];
        
        // 简化：直接复制值
        char *data = new_rec.data();
        memset(data + fm->offset(), 0, fm->len());
        if (val.attr_type() != AttrType::UNDEFINED) {
          size_t len = std::min((size_t)val.length(), (size_t)fm->len());
          memcpy(data + fm->offset(), val.data(), len);
        }
      }
      
      rc = table->update_record_with_trx(old_rec, new_rec, trx);
      if (rc != RC::SUCCESS) return rc;
    }
  } else {
    // 单字段UPDATE - 使用旧逻辑
    for (Record &old_rec : records) {
      Record new_rec;
      new_rec.copy_data(old_rec.data(), old_rec.len());
      new_rec.set_rid(old_rec.rid());
      
      char *data = new_rec.data();
      const FieldMeta *fm = update_stmt->field_meta();
      Value *val = update_stmt->value();
      
      memset(data + fm->offset(), 0, fm->len());
      if (val->attr_type() != AttrType::UNDEFINED) {
        size_t len = std::min((size_t)val->length(), (size_t)fm->len());
        memcpy(data + fm->offset(), val->data(), len);
      }
      
      rc = table->update_record_with_trx(old_rec, new_rec, trx);
      if (rc != RC::SUCCESS) return rc;
    }
  }
  
  return RC::SUCCESS;
}

// Helper function to execute sub-query and get a single value
RC execute_sub_query(Stmt *sub_query_stmt, Session *session, Value &result_value)
{
  if (sub_query_stmt->type() != StmtType::SELECT) {
    LOG_WARN("sub-query must be a SELECT statement");
    return RC::INVALID_ARGUMENT;
  }

  SelectStmt *select_stmt = static_cast<SelectStmt *>(sub_query_stmt);

  // Generate logical plan
  LogicalPlanGenerator logical_plan_generator;
  std::unique_ptr<LogicalOperator> logical_operator;
  RC rc = logical_plan_generator.create(select_stmt, logical_operator);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create logical plan for sub-query. rc=%s", strrc(rc));
    return rc;
  }

  // Generate physical plan
  PhysicalPlanGenerator physical_plan_generator;
  std::unique_ptr<PhysicalOperator> physical_operator;
  rc = physical_plan_generator.create(*logical_operator, physical_operator, session);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create physical plan for sub-query. rc=%s", strrc(rc));
    return rc;
  }

  // Open the operator
  rc = physical_operator->open(session->current_trx());
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open physical operator for sub-query. rc=%s", strrc(rc));
    return rc;
  }

  // Get the first (and hopefully only) tuple
  rc = physical_operator->next();
  if (rc != RC::SUCCESS) {
    physical_operator->close();
    if (rc == RC::RECORD_EOF) {
      LOG_WARN("sub-query returned no results");
      return RC::EMPTY;
    }
    LOG_WARN("failed to get result from sub-query. rc=%s", strrc(rc));
    return rc;
  }

  // Get the current tuple
  Tuple *tuple = physical_operator->current_tuple();
  if (tuple == nullptr) {
    physical_operator->close();
    LOG_WARN("sub-query returned null tuple");
    return RC::EMPTY;
  }

  // Get the first cell from the tuple
  if (tuple->cell_num() == 0) {
    physical_operator->close();
    LOG_WARN("sub-query returned empty tuple");
    return RC::EMPTY;
  }

  Value cell_value;
  rc = tuple->cell_at(0, cell_value);
  if (rc != RC::SUCCESS) {
    physical_operator->close();
    LOG_WARN("failed to get cell value from sub-query result. rc=%s", strrc(rc));
    return rc;
  }

  result_value = cell_value;

  // Check if there are more results (sub-query should return only one value)
  RC next_rc = physical_operator->next();
  if (next_rc == RC::SUCCESS) {
    physical_operator->close();
    LOG_WARN("sub-query returned more than one row");
    return RC::INVALID_ARGUMENT; // Use INVALID_ARGUMENT instead of TOO_MANY_ROWS
  }

  physical_operator->close();
  return RC::SUCCESS;
}
