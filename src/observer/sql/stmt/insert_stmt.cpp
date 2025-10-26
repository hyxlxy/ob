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
#include "sql/stmt/select_stmt.h"
#include "common/log/log.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "storage/table/view.h"
#include "sql/expr/expression.h"

InsertStmt::InsertStmt(Table *table, const Value *values, int value_amount)
    : table_(table), values_(values), value_amount_(value_amount)
{}

InsertStmt::InsertStmt(Table *table, const vector<string> &attr_names, const vector<Value> &values) : table_(table)
{
  // 存储列名，用于下游物化时重排
  attribute_names_ = attr_names;
  // 暂存值到内部缓冲（为了与老接口兼容，仍复用 values_/value_amount_）
  // 注意：调用方应在 LogicalPlan 阶段进行重排为完整列序
  static vector<Value> holder;  // 仅用于生命周期延续（简化实现，不考虑并发）
  holder        = values;
  values_       = holder.data();
  value_amount_ = static_cast<int>(holder.size());
}

RC InsertStmt::create(Db *db, InsertSqlNode &inserts, Stmt *&stmt)
{
  const char *table_name = inserts.relation_name.c_str();
  if (nullptr == db || nullptr == table_name || inserts.values.empty()) {
    LOG_WARN("invalid argument. db=%p, table_name=%p, value_num=%d",
        db, table_name, static_cast<int>(inserts.values.size()));
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table           = db->find_table(table_name);
  bool   is_view         = false;
  int    view_field_size = 0;
  if (nullptr == table) {
    View *view = db->find_view(table_name);
    if (view == nullptr) {
      LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
      return RC::SCHEMA_TABLE_NOT_EXIST;
    }

    //说明当前的是view
    is_view = true;

    // ASSERT(view->base_tables().size()==1, "view base_table_size!=1");

    unique_ptr<ParsedSqlNode> node;
    view->get_sql(node);
    Stmt *child_stmt = nullptr;
    RC    rc         = SelectStmt::create(db, node->selection, child_stmt);
    if (rc != RC::SUCCESS) {
      return RC::ERROR;
    }
    view->set_stmt(child_stmt);
    view->set_table_meta(db);
    view->set_table_field_name(db);

    if (!view->can_update()) {
      //此刻我们需要检测一下，插入的列是否都是来自一个表的，如果是的话，那么就可以update
      auto &&opt = view->check_is_update(inserts.attribute_names);
      if (!opt.has_value()) {
        return RC::ERROR;
      }
      string table_name_in_view = opt.value();
      Table *base_one_table     = db->find_table(table_name_in_view.c_str());
      if (base_one_table == nullptr) {
        return RC::ERROR;
      }
      view->set_default_base_table(base_one_table);
    }

    table           = view;
    view_field_size = view->attr_names().size();
  }

  const TableMeta &table_meta = table->table_meta();

  const int field_num = table_meta.field_num() - table_meta.sys_field_num();
  const int value_num = static_cast<int>(inserts.values.size());

  if (field_num != value_num and !is_view) {
    LOG_WARN("schema mismatch. value num=%d, field num in schema=%d", value_num, field_num);
    return RC::SCHEMA_FIELD_MISSING;
  }

  if (value_num != view_field_size and is_view) {

    //先做一下检测，判断插入的列是否都存在
    for (const string &attr_name : inserts.attribute_names) {
      bool found_flag = false;
      for (int i = 0; i < view_field_size; i++) {
        const FieldMeta *f = table_meta.field(i + table_meta.sys_field_num());
        if (attr_name == f->name()) {
          found_flag = true;
          break;
        }
      }
      if (!found_flag) {
        return RC::SCHEMA_FIELD_NOT_EXIST;
      }
    }

    //把node里面的内容重新搞一下
    vector<Value> real_values;
    for (int i = 0; i < field_num; i++) {
      const FieldMeta *f     = table_meta.field(i + table_meta.sys_field_num());
      bool             found = false;
      for (size_t j = 0; j < inserts.attribute_names.size(); j++) {
        if (inserts.attribute_names[j] == f->name()) {
          real_values.push_back(inserts.values[j]);
          found = true;
          break;
        }
      }
      if (!found) {
        Value nullv;
        nullv.set_type(AttrType::UNDEFINED);
        nullv.set_null(true);
        real_values.push_back(nullv);
      }
    }
    inserts.values = std::move(real_values);
    inserts.attribute_names.clear();
  }

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
      const FieldMeta *f  = table_meta.field(i);
      auto             it = m.find(f->name());
      if (it != m.end()) {
        ordered.push_back(it->second);
      } else {
        Value nullv;
        nullv.set_type(AttrType::UNDEFINED);
        nullv.set_null(true);
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
