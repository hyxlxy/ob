/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "sql/stmt/alter_table_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "common/log/log.h"

RC AlterTableStmt::create(Db *db, const ParsedSqlNode::AlterTableSqlNode &node, Stmt *&stmt)
{
  stmt = nullptr;

  if (db == nullptr) {
    return RC::INVALID_ARGUMENT;
  }

  // 基础校验
  if (node.table_name.empty()) {
    return RC::INVALID_ARGUMENT;
  }

  Table *table = db->find_table(node.table_name.c_str());
  if (table == nullptr) {
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  // 简化校验：仅支持 INT 类型新增/改名语句中给出的类型位，挑战声明不涉及类型修改
  // 变更列名/删除列时校验列是否存在
  if (node.op_type == ParsedSqlNode::AlterOpType::DROP_COLUMN || node.op_type == ParsedSqlNode::AlterOpType::CHANGE_COLUMN_RENAME) {
    const FieldMeta *fm = table->table_meta().field(node.column_name.c_str());
    if (fm == nullptr) {
      return RC::SCHEMA_FIELD_MISSING;
    }
  }

  // 改名时新名不得与现有列重名
  if (node.op_type == ParsedSqlNode::AlterOpType::CHANGE_COLUMN_RENAME) {
    if (node.new_column_name.empty()) return RC::INVALID_ARGUMENT;
    const FieldMeta *exist = table->table_meta().field(node.new_column_name.c_str());
    if (exist != nullptr) return RC::SCHEMA_FIELD_TYPE_MISMATCH; // 复用现有错误码，避免新增
  }

  // 新增列：不可与现有列重名
  if (node.op_type == ParsedSqlNode::AlterOpType::ADD_COLUMN) {
    if (node.column_name.empty()) return RC::INVALID_ARGUMENT;
    const FieldMeta *exist = table->table_meta().field(node.column_name.c_str());
    if (exist != nullptr) return RC::SCHEMA_FIELD_TYPE_MISMATCH;
  }

  auto *s = new AlterTableStmt();
  s->op_type = node.op_type;
  s->table_name = node.table_name;
  s->new_table_name = node.new_table_name;
  s->column_name = node.column_name;
  s->new_column_name = node.new_column_name;
  s->data_type = node.data_type;
  stmt = s;
  return RC::SUCCESS;
}


