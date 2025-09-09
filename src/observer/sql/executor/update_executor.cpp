/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "sql/executor/update_executor.h"

#include "common/log/log.h"
#include "session/session.h"
#include "sql/executor/sql_result.h"
#include "sql/expr/tuple.h"
#include "sql/stmt/update_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "sql/expr/expression.h"
#include "storage/table/table.h"
#include "storage/table/table_meta.h"
#include "storage/record/record.h"
#include "storage/trx/trx.h"

using std::string;

// Simple LIKE matcher supporting % and _
static bool like_match_simple(const char *text, const char *pattern)
{
  // iterative backtracking algorithm
  const char *t = text;
  const char *p = pattern;
  const char *star = nullptr;
  const char *ss = nullptr;

  while (*t) {
    if (*p == '%' ) { star = p++; ss = t; continue; }
    if (*p == '_' || *p == *t) { ++p; ++t; continue; }
    if (star) { p = star + 1; t = ++ss; continue; }
    return false;
  }
  while (*p == '%') ++p;
  return *p == '\0';
}

static int compare_by_comp(CompOp comp, const Value &lhs, const Value &rhs)
{
  // returns 1 if predicate true, 0 if false
  if (comp == LIKE_OP || comp == NOT_LIKE_OP) {
    if (lhs.attr_type() != AttrType::CHARS || rhs.attr_type() != AttrType::CHARS) {
      return 0;
    }
    bool ok = like_match_simple(lhs.get_string().c_str(), rhs.get_string().c_str());
    return comp == LIKE_OP ? (ok ? 1 : 0) : (ok ? 0 : 1);
  }

  int cmp = Value::compare(lhs, rhs);
  switch (comp) {
    case EQUAL_TO: return cmp == 0; 
    case NOT_EQUAL: return cmp != 0;
    case LESS_THAN: return cmp < 0;
    case GREAT_THAN: return cmp > 0;
    case LESS_EQUAL: return cmp <= 0;
    case GREAT_EQUAL: return cmp >= 0;
    default: return 0;
  }
}

static RC eval_filter_units(Table *table, Record &record, const FilterStmt *filter_stmt, bool &matched)
{
  matched = true;
  if (nullptr == filter_stmt) {
    return RC::SUCCESS;
  }

  RowTuple tuple;
  tuple.set_record(&record);
  tuple.set_schema(table, table->table_meta().field_metas());

  for (FilterUnit *unit : filter_stmt->filter_units()) {
    const FilterObj &left = unit->left();
    const FilterObj &right = unit->right();
    Value lv, rv;

    // left
    if (left.is_expr()) {
      RC rc = left.expr->get_value(tuple, lv);
      if (OB_FAIL(rc)) return rc;
    } else if (left.is_attr()) {
      const FieldMeta *fm = left.field.meta();
      lv.set_type(fm->type());
      lv.set_data(record.data() + fm->offset(), fm->len());
    } else { // value
      lv.set_value(left.value);
    }

    // right
    if (right.is_expr()) {
      RC rc = right.expr->get_value(tuple, rv);
      if (OB_FAIL(rc)) return rc;
    } else if (right.is_attr()) {
      const FieldMeta *fm = right.field.meta();
      rv.set_type(fm->type());
      rv.set_data(record.data() + fm->offset(), fm->len());
    } else {
      rv.set_value(right.value);
    }

    if (!compare_by_comp(unit->comp(), lv, rv)) {
      matched = false;
      return RC::SUCCESS;
    }
  }
  return RC::SUCCESS;
}

RC UpdateExecutor::execute(SQLStageEvent *sql_event)
{
  Session *session = sql_event->session_event()->session();
  Trx     *trx     = session->current_trx();

  UpdateStmt *update_stmt = dynamic_cast<UpdateStmt *>(sql_event->stmt());
  if (nullptr == update_stmt) {
    LOG_WARN("UpdateExecutor got invalid stmt");
    return RC::INVALID_ARGUMENT;
  }

  Table *table = update_stmt->table();
  const FieldMeta *field_meta = update_stmt->field_meta();
  const Value &new_value = *update_stmt->value();
  const FilterStmt *filter_stmt = update_stmt->filter_stmt();

  RecordScanner *scanner = nullptr;
  RC rc = table->get_record_scanner(scanner, trx, ReadWriteMode::READ_WRITE);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to get record scanner. rc=%s", strrc(rc));
    return rc;
  }

  Record record;
  int affected = 0;
  while (RC::SUCCESS == (rc = scanner->next(record))) {
    bool matched = true;
    RC rc2 = eval_filter_units(table, record, filter_stmt, matched);
    if (OB_FAIL(rc2)) { rc = rc2; break; }
    if (!matched) continue;

    // build new record by copying old data then setting target field
    Record new_record;
    rc2 = new_record.copy_data(record.data(), record.len());
    if (OB_FAIL(rc2)) { rc = rc2; break; }

    size_t copy_len = field_meta->len();
    const size_t data_len = new_value.length();
    if (field_meta->type() == AttrType::CHARS) {
      if (copy_len > data_len) copy_len = data_len + 1; // keep trailing as-is
    } else if (field_meta->type() == AttrType::VECTORS) {
      if (data_len > copy_len) { rc = RC::SCHEMA_FIELD_TYPE_MISMATCH; break; }
      copy_len = data_len;
    } else {
      if (data_len < copy_len) copy_len = data_len;
    }
    memcpy(new_record.data() + field_meta->offset(), new_value.data(), copy_len);

    rc2 = table->update_record_with_trx(record, new_record, trx);
    if (OB_FAIL(rc2)) { rc = rc2; break; }
    affected++;
  }

  if (rc == RC::RECORD_EOF) rc = RC::SUCCESS;
  delete scanner;

  sql_event->session_event()->sql_result()->set_affected_rows(affected);
  sql_event->session_event()->sql_result()->set_return_code(rc);
  return rc;
}

