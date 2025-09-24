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
// Created by Wangyunlai on 2024/5/31.
//

#pragma once

#include "common/lang/vector.h"
#include "sql/expr/tuple.h"
#include "common/value.h"
#include "common/sys/rc.h"

template <typename ExprPointerType>
class ExpressionTuple : public Tuple
{
public:
  ExpressionTuple(const vector<ExprPointerType> &expressions) : expressions_(expressions) {}
  virtual ~ExpressionTuple() = default;

  void set_tuple(const Tuple *tuple) { child_tuple_ = tuple; }

  int cell_num() const override { return static_cast<int>(expressions_.size()); }

  RC cell_at(int index, Value &cell) const override
  {
    if (index < 0 || index >= cell_num()) {
      return RC::INVALID_ARGUMENT;
    }

    const ExprPointerType &expression = expressions_[index];

    printf("DEBUG: [EXPRESSION_TUPLE] cell_at(%d): expr_type=%d, expr_name='%s'\n",
           index, static_cast<int>(expression->type()), expression->name());

    // Fast path: if we have a child tuple and this expression is STAR, passthrough to child
    if (child_tuple_ != nullptr && expression->type() == ExprType::STAR) {
      printf("DEBUG: [EXPRESSION_TUPLE] STAR expression, passing through to child\n");
      return child_tuple_->cell_at(index, cell);
    }

    printf("DEBUG: [EXPRESSION_TUPLE] About to call get_value for expr_type=%d\n",
           static_cast<int>(expression->type()));
    RC rc = get_value(expression, cell);
    printf("DEBUG: [EXPRESSION_TUPLE] get_value returned: rc=%s, value=%s\n",
           strrc(rc), rc == RC::SUCCESS ? cell.to_string().c_str() : "N/A");
    return rc;
  }

  RC spec_at(int index, TupleCellSpec &spec) const override
  {
    if (index < 0 || index >= cell_num()) {
      return RC::INVALID_ARGUMENT;
    }

    const ExprPointerType &expression = expressions_[index];

    // If STAR remains here (not expanded by binder), forward spec from child tuple
    if (child_tuple_ != nullptr && expression->type() == ExprType::STAR) {
      return child_tuple_->spec_at(index, spec);
    }

    spec = TupleCellSpec(expression->name());
    return RC::SUCCESS;
  }

  RC find_cell(const TupleCellSpec &spec, Value &cell) const override
  {
    printf("DEBUG: [EXPRESSION_TUPLE] find_cell() called: spec.alias()='%s'\n", spec.alias());

    RC rc = RC::SUCCESS;
    if (child_tuple_ != nullptr) {
      printf("DEBUG: [EXPRESSION_TUPLE] Trying child_tuple_->find_cell()\n");
      rc = child_tuple_->find_cell(spec, cell);
      if (OB_SUCC(rc)) {
        printf("DEBUG: [EXPRESSION_TUPLE] Found in child_tuple: value=%s\n", cell.to_string().c_str());
        return rc;
      }
      printf("DEBUG: [EXPRESSION_TUPLE] Not found in child_tuple: rc=%s\n", strrc(rc));
    }

    rc = RC::NOTFOUND;
    for (const ExprPointerType &expression : expressions_) {
      printf("DEBUG: [EXPRESSION_TUPLE] Checking expression: name='%s', type=%d\n",
             expression->name(), static_cast<int>(expression->type()));
      if (0 == strcmp(spec.alias(), expression->name())) {
        printf("DEBUG: [EXPRESSION_TUPLE] Found matching expression, calling get_value()\n");
        rc = get_value(expression, cell);
        printf("DEBUG: [EXPRESSION_TUPLE] get_value() returned: rc=%s, value=%s\n",
               strrc(rc), rc == RC::SUCCESS ? cell.to_string().c_str() : "N/A");
        break;
      }
    }

    printf("DEBUG: [EXPRESSION_TUPLE] find_cell() returning: rc=%s\n", strrc(rc));
    return rc;
  }

private:
  RC get_value(const ExprPointerType &expression, Value &value) const
  {
    printf("DEBUG: [EXPRESSION_TUPLE] get_value() called: expr_type=%d, expr_name='%s', child_tuple_null=%d\n",
           static_cast<int>(expression->type()), expression->name(), child_tuple_ == nullptr);

    RC rc = RC::SUCCESS;
    if (child_tuple_ != nullptr) {
      printf("DEBUG: [EXPRESSION_TUPLE] Calling expression->get_value(*child_tuple_, value)\n");
      rc = expression->get_value(*child_tuple_, value);
      printf("DEBUG: [EXPRESSION_TUPLE] expression->get_value() returned: rc=%s, value=%s\n",
             strrc(rc), rc == RC::SUCCESS ? value.to_string().c_str() : "N/A");
    } else {
      printf("DEBUG: [EXPRESSION_TUPLE] Calling expression->try_get_value(value)\n");
      rc = expression->try_get_value(value);
      printf("DEBUG: [EXPRESSION_TUPLE] expression->try_get_value() returned: rc=%s, value=%s\n",
             strrc(rc), rc == RC::SUCCESS ? value.to_string().c_str() : "N/A");
    }
    if (rc != RC::SUCCESS) {
      // Help diagnose which expression failed to produce a value
      LOG_WARN("ExpressionTuple get_value failed. expr_type=%d name=%s child_tuple_null=%d rc=%s",
               static_cast<int>(expression->type()), expression->name(), child_tuple_ == nullptr, strrc(rc));
    }
    return rc;
  }

private:
  const vector<ExprPointerType> &expressions_;
  const Tuple                   *child_tuple_ = nullptr;
};
