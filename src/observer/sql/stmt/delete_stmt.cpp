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

#include "sql/stmt/delete_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "common/log/log.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "sql/parser/expression_binder.h"
#include "storage/table/view.h"

DeleteStmt::DeleteStmt(Table *table, FilterStmt *filter_stmt) : table_(table), filter_stmt_(filter_stmt) {}

DeleteStmt::~DeleteStmt()
{
  if (nullptr != filter_stmt_) {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  }
}

RC DeleteStmt::create(Db *db, DeleteSqlNode &delete_sql, Stmt *&stmt)
{
  const char *table_name = delete_sql.relation_name.c_str();
  if (nullptr == db || nullptr == table_name) {
    LOG_WARN("invalid argument. db=%p, table_name=%p", db, table_name);
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  BinderContext bind_context;
  Table        *table = db->find_table(table_name);
  if (nullptr == table) {
    View *view = db->find_view(table_name);
    if (view == nullptr) {
      LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
      return RC::SCHEMA_TABLE_NOT_EXIST;
    }

    //说明当前的是view

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

      vector<string> all_names;

      for (ConditionSqlNode &one : delete_sql.conditions) {
        if (one.left_expr != nullptr && one.left_expr->name() != nullptr) {
          Value val;
          if (one.left_expr->try_get_value(val) != RC::SUCCESS) {
            string expr_name = one.left_expr->name();
            if (expr_name.find('.') != string::npos) {
              expr_name = expr_name.substr(expr_name.find('.') + 1);
            }
            all_names.push_back(expr_name);
          }
        }
        if (one.right_expr != nullptr && one.right_expr->name() != nullptr) {
          Value val;
          if (one.right_expr->try_get_value(val) != RC::SUCCESS) {

            string expr_name = one.right_expr->name();
            if (expr_name.find('.') != string::npos) {
              expr_name = expr_name.substr(expr_name.find('.') + 1);
            }
            all_names.push_back(expr_name);
          }
        }
      }

      //此刻我们需要检测一下，插入的列是否都是来自一个表的，如果是的话，那么就可以update
      auto &&opt = view->check_is_update(all_names);
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

    table = view->get_default_base_table();
  }

  unordered_map<string, Table *> table_map;
  table_map.insert(pair<string, Table *>(string(table_name), table));
  bind_context.add_table(table);

  FilterStmt *filter_stmt = nullptr;
  RC          rc          = FilterStmt::create(db,
      table,
      &table_map,
      delete_sql.conditions.data(),
      static_cast<int>(delete_sql.conditions.size()),
      filter_stmt,
      &bind_context);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create filter statement. rc=%d:%s", rc, strrc(rc));
    return rc;
  }

  stmt = new DeleteStmt(table, filter_stmt);
  return rc;
}
