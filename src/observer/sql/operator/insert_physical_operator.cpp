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
// Created by WangYunlai on 2021/6/9.
//

#include "sql/operator/insert_physical_operator.h"
#include "sql/stmt/insert_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "storage/table/table.h"
#include "storage/table/view.h"
#include "storage/trx/trx.h"

using namespace std;

InsertPhysicalOperator::InsertPhysicalOperator(Table *table, vector<Value> &&values)
    : table_(table), values_(std::move(values))
{}

RC InsertPhysicalOperator::open(Trx *trx)
{
  Record record;

  if (auto &&view = dynamic_cast<View *>(table_)) {
    //当前的table其实是view
    //那么就把base_table里的table取出来。
    //但是里面有一些列是不存在的，是null，所以这里需要特殊处理一下values。
    table_ = view->get_default_base_table();
    Db *db = table_->db();

    vector<pair<Field, string>> fields = view->get_fields(db, dynamic_cast<SelectStmt *>(view->stmt()));
    // values_.clear();
    vector<Value> new_values;
    for (auto &&base_table_field : *(table_->table_meta().field_metas())) {

      bool find_flag = false;

      for (size_t i = 0; i < fields.size(); i++) {
        auto &&[field, alias] = fields[i];
        if ((field.table() != nullptr) and string(field.field_name()) == string(base_table_field.name())) {
          //代表找到了
          find_flag = true;
          new_values.push_back(values_[i]);  // 因为原本的values_是按照视图的顺序放的
        }
      }

      if (find_flag == false) {
        //就放一个null进去
        Value one;
        one.set_null();
        one.set_type(AttrType::UNDEFINED);
        new_values.push_back(std::move(one));
      }
      //
    }
    values_ = std::move(new_values);
  }

  RC rc = table_->make_record(static_cast<int>(values_.size()), values_.data(), record);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to make record. rc=%s", strrc(rc));
    for (size_t i = 0; i < values_.size(); i++) {
      LOG_INFO("DEBUG: INSERT make_record fail at i=%zu type=%d val=%s", i, (int)values_[i].attr_type(), values_[i].to_string().c_str());
    }
    return rc;
  }

  // 打印各字段写入后的数据片段（向量字段重点）
  const TableMeta &tm                       = table_->table_meta();
  const int        normal_field_start_index = tm.sys_field_num();
  for (int i = 0; i < (int)values_.size(); i++) {
    const FieldMeta *field = tm.field(i + normal_field_start_index);
    if (field->type() == AttrType::VECTORS) {
      const char *ptr = record.data() + field->offset();
      if (field->len() >= (int)sizeof(int)) {
        int dim = *(const int *)ptr;
        LOG_INFO("DEBUG: INSERT vector field=%s dim_in_record=%d", field->name(), dim);
      }
    }
  }

  rc = trx->insert_record(table_, record);
  // rc = table_->insert_record_with_trx(record, trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to insert record by transaction. rc=%s", strrc(rc));
  }
  return rc;
}

RC InsertPhysicalOperator::next() { return RC::RECORD_EOF; }

RC InsertPhysicalOperator::close() { return RC::SUCCESS; }
