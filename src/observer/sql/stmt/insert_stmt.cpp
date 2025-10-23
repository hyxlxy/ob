/* Copyright (c) 2021OceanBase and/or its affiliates. All rights reserved.
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

#include "sql/stmt/insert_stmt.h"
#include "common/log/log.h"
#include "storage/db/db.h"
#include "storage/table/table.h"

InsertStmt::InsertStmt(Table *table, const Value *values, int value_amount)
    : table_(table), values_(values), value_amount_(value_amount)
{}

InsertStmt::InsertStmt(Table *table, const vector<string> &attr_names, const vector<Value> &values)
    : table_(table)
{
  // 存储列名，用于下游物化时重排
  attribute_names_ = attr_names;
  // 暂存值到内部缓冲（为了与老接口兼容，仍复用 values_/value_amount_）
  // 注意：调用方应在 LogicalPlan 阶段进行重排为完整列序
  static vector<Value> holder; // 仅用于生命周期延续（简化实现，不考虑并发）
  holder = values;
  values_ = holder.data();
  value_amount_ = static_cast<int>(holder.size());
}

RC InsertStmt::create(Db *db, const InsertSqlNode &inserts, Stmt *&stmt)
{
  const char *table_name = inserts.relation_name.c_str();
  if (nullptr == db || nullptr == table_name || inserts.values.empty()) {
    LOG_WARN("invalid argument. db=%p, table_name=%p, value_num=%d",
        db, table_name, static_cast<int>(inserts.values.size()));
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  const TableMeta &table_meta = table->table_meta();
  if (!inserts.attribute_names.empty()) {
    // 列清单插入：根据提供的列名重排到完整列序；未指定列填 NULL
    const int user_cols = table_meta.field_num() - table_meta.sys_field_num();
    if (static_cast<int>(inserts.values.size()) != static_cast<int>(inserts.attribute_names.size())) {
      return RC::SCHEMA_FIELD_MISSING;
    }
    // 构建列名 -> 值 映射
    unordered_map<string, Value> m;
    m.reserve(inserts.attribute_names.size());
    for (size_t i = 0; i < inserts.attribute_names.size(); ++i) {
      m[inserts.attribute_names[i]] = inserts.values[i];
    }
    // 重排为表的物理顺序
    vector<Value> ordered;
    ordered.reserve(user_cols);
    for (int i = table_meta.sys_field_num(); i < table_meta.field_num(); ++i) {
      const FieldMeta *f = table_meta.field(i);
      auto it = m.find(f->name());
      if (it != m.end()) {
        ordered.push_back(it->second);
      } else {
        Value nullv; nullv.set_type(AttrType::UNDEFINED); nullv.set_null(true);
        if (!f->nullable()) {
          return RC::INVALID_ARGUMENT;
        }
        ordered.push_back(nullv);
      }
    }
    stmt = new InsertStmt(table, inserts.attribute_names, ordered);
    return RC::SUCCESS;
  } else {
    // 旧语法：VALUES(...) 必须与所有用户列等长
    const Value *values    = inserts.values.data();
    const int    value_num = static_cast<int>(inserts.values.size());
    const int    field_num = table_meta.field_num() - table_meta.sys_field_num();
    if (field_num != value_num) {
      LOG_WARN("schema mismatch. value num=%d, field num in schema=%d", value_num, field_num);
      return RC::SCHEMA_FIELD_MISSING;
    }
    stmt = new InsertStmt(table, values, value_num);
    return RC::SUCCESS;
  }
}
