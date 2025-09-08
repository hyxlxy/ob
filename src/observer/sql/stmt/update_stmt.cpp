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

  // check whether the field exists
  const FieldMeta *field_meta = table->table_meta().field(update.attribute_name.c_str());
  if (nullptr == field_meta) {
    LOG_WARN("no such field in table. db=%s, table=%s, field=%s",
             db->name(), table_name, update.attribute_name.c_str());
    return RC::SCHEMA_FIELD_NOT_EXIST;
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

  // create value copy
  Value *value = new Value();
  *value = update.value;

  // validate value type compatibility
  if (value->attr_type() != field_meta->type()) {
    // try to cast the value to the field type
    Value casted_value;
    rc = Value::cast_to(*value, field_meta->type(), casted_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to cast value from %s to %s",
               attr_type_to_string(value->attr_type()),
               attr_type_to_string(field_meta->type()));
      delete value;
      delete filter_stmt;
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }
    *value = casted_value;
  }

  stmt = new UpdateStmt(table, field_meta, value, filter_stmt);
  return RC::SUCCESS;
}
