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

#include "sql/operator/table_scan_physical_operator.h"
#include "event/sql_debug.h"
#include "storage/table/table.h"

using namespace std;

RC TableScanPhysicalOperator::open(Trx *trx)
{
  RC rc = table_->get_record_scanner(record_scanner_, trx, mode_);
  if (rc == RC::SUCCESS) {
    tuple_.set_schema(table_, table_->table_meta().field_metas());
  }
  trx_ = trx;
  return rc;
}

RC TableScanPhysicalOperator::next()
{
  RC rc = RC::SUCCESS;

  bool filter_result = false;
  static int scan_counter = 0;

  while (OB_SUCC(rc = record_scanner_->next(current_record_))) {
    scan_counter++;

    // 防止无限循环，每10000次扫描输出调试信息
    if (scan_counter % 10000 == 0) {
      printf("DEBUG: TableScanPhysicalOperator::next scanned %d records\n", scan_counter);
      fflush(stdout);
    }

    // 如果扫描次数过多，强制退出
    if (scan_counter > 100000) {
      printf("DEBUG: TableScanPhysicalOperator::next exceeded maximum scan limit, breaking\n");
      fflush(stdout);
      return RC::RECORD_EOF;
    }

    LOG_TRACE("got a record. rid=%s", current_record_.rid().to_string().c_str());

    tuple_.set_record(&current_record_);
    rc = filter(tuple_, filter_result);
    if (rc != RC::SUCCESS) {
      LOG_TRACE("record filtered failed=%s", strrc(rc));
      return rc;
    }

    if (filter_result) {
      sql_debug("get a tuple: %s", tuple_.to_string().c_str());
      break;
    } else {
      sql_debug("a tuple is filtered: %s", tuple_.to_string().c_str());
    }
  }
  return rc;
}

RC TableScanPhysicalOperator::close() {
  RC rc = RC::SUCCESS;
  if (record_scanner_ != nullptr) {
    rc = record_scanner_->close_scan();
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to close record scanner");
    }
    delete record_scanner_;
    record_scanner_ = nullptr;
  }
  return rc;

}

Tuple *TableScanPhysicalOperator::current_tuple()
{
  tuple_.set_record(&current_record_);
  return &tuple_;
}

string TableScanPhysicalOperator::param() const { return table_->name(); }

RC TableScanPhysicalOperator::tuple_schema(TupleSchema &schema) const
{
  const TableMeta &table_meta = table_->table_meta();
  const int field_num = table_meta.field_num();
  for (int i = table_meta.sys_field_num(); i < field_num; i++) {
    const FieldMeta *field_meta = table_meta.field(i);
    schema.append_cell(TupleCellSpec(table_->name(), field_meta->name(), field_meta->name()));
  }
  return RC::SUCCESS;
}

void TableScanPhysicalOperator::set_predicates(vector<unique_ptr<Expression>> &&exprs)
{
  predicates_ = std::move(exprs);
}

RC TableScanPhysicalOperator::filter(RowTuple &tuple, bool &result)
{
  RC    rc = RC::SUCCESS;
  Value value;

  static int filter_counter = 0;
  filter_counter++;

  for (unique_ptr<Expression> &expr : predicates_) {
    rc = expr->get_value(tuple, value);
    if (rc != RC::SUCCESS) {
      return rc;
    }

    bool tmp_result = value.get_boolean();

    // 每1000次调用输出一次调试信息
    if (filter_counter % 1000 == 0) {
      printf("DEBUG: TableScanPhysicalOperator::filter called %d times, result=%s\n",
             filter_counter, tmp_result ? "true" : "false");
      fflush(stdout);
    }

    if (!tmp_result) {
      result = false;
      return rc;
    }
  }

  result = true;
  return rc;
}
