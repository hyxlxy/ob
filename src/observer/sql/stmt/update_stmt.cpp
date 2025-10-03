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
// Created by Wangyunlai on 2022/5/22.
//

#include "sql/stmt/update_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "common/log/log.h"
#include "storage/db/db.h"
#include "storage/table/table.h"

UpdateStmt::UpdateStmt(Table *table, Value *values, int value_amount)
    : table_(table), values_(values), value_amount_(value_amount)
{}

UpdateStmt::UpdateStmt(Table *table, const FieldMeta *field_meta, Value *value, FilterStmt *filter_stmt)
    : table_(table), field_meta_(field_meta), value_(value), filter_stmt_(filter_stmt)
{}

UpdateStmt::UpdateStmt(Table *table, const FieldMeta *field_meta, Stmt *sub_query_stmt, FilterStmt *filter_stmt)
    : table_(table), field_meta_(field_meta), filter_stmt_(filter_stmt), sub_query_stmt_(sub_query_stmt)
{}

UpdateStmt::UpdateStmt(Table *table, const vector<const FieldMeta *> &field_metas, const vector<Value> &values, FilterStmt *filter_stmt)
    : table_(table), filter_stmt_(filter_stmt), field_metas_(field_metas), multi_values_(values)
{}

UpdateStmt::~UpdateStmt()
{
  if (nullptr != filter_stmt_) {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  }
  if (nullptr != value_) {
    delete value_;
    value_ = nullptr;
  }
  if (nullptr != sub_query_stmt_) {
    delete sub_query_stmt_;
    sub_query_stmt_ = nullptr;
  }
}

RC UpdateStmt::create(Db *db, const UpdateSqlNode &update, Stmt *&stmt)
{
  const char *table_name = update.relation_name.c_str();
  if (nullptr == db || nullptr == table_name) {
    LOG_WARN("invalid argument. db=%p, table_name=%p", db, table_name);
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  // create filter statement for WHERE clause
  FilterStmt *filter_stmt = nullptr;
  unordered_map<string, Table *> table_map;
  table_map.insert(pair<string, Table *>(string(table_name), table));

  RC rc = FilterStmt::create(db, table, &table_map,
                             update.conditions.data(), static_cast<int>(update.conditions.size()), filter_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create filter statement. rc=%d:%s", rc, strrc(rc));
    return rc;
  }

  // 检查是否为多字段UPDATE
  if (!update.attribute_names.empty()) {
    // 多字段UPDATE
    vector<const FieldMeta *> field_metas;
    for (const string &attr_name : update.attribute_names) {
      const FieldMeta *fm = table->table_meta().field(attr_name.c_str());
      if (!fm) {
        delete filter_stmt;
        return RC::SCHEMA_FIELD_NOT_EXIST;
      }
      field_metas.push_back(fm);
    }
    stmt = new UpdateStmt(table, field_metas, update.values, filter_stmt);
    return RC::SUCCESS;
  }

  // 单字段UPDATE
  const FieldMeta *field_meta = table->table_meta().field(update.attribute_name.c_str());
  if (nullptr == field_meta) {
    LOG_WARN("no such field in table. db=%s, table=%s, field=%s",
             db->name(), table_name, update.attribute_name.c_str());
    delete filter_stmt;
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }

  // Check if this is a sub-query update
  if (update.is_sub_query) {
    // Create sub-query statement
    Stmt *sub_query_stmt = nullptr;
    rc = Stmt::create_stmt(db, *update.sub_query, sub_query_stmt);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create sub-query statement. rc=%d:%s", rc, strrc(rc));
      delete filter_stmt;
      return rc;
    }

    stmt = new UpdateStmt(table, field_meta, sub_query_stmt, filter_stmt);
  } else {
    // create value copy
    Value *value = new Value();
    *value = update.value;

    // Do not strictly validate or cast here to match baseline behavior.
    // Defer casting/validation to execution; mismatches will not cause statement failure.
    stmt = new UpdateStmt(table, field_meta, value, filter_stmt);
  }
  return RC::SUCCESS;
}
