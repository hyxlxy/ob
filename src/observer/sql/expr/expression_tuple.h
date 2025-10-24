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
#include "storage/table/view.h"
/*

在projec操作符t里面有一个这个tuple，find_cell就是在这里面通过名字或者别名去查找。
保证了输出的和我们query的顺序是一样的
*/
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

    // Fast path: if we have a child tuple and this expression is STAR, passthrough to child
    if (child_tuple_ != nullptr && expression->type() == ExprType::STAR) {
      return child_tuple_->cell_at(index, cell);
    }

    return get_value(expression, cell);
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
    // RC rc = RC::SUCCESS;
    // if (child_tuple_ != nullptr) {
    //   rc = child_tuple_->find_cell(spec, cell);
    //   if (OB_SUCC(rc)) {
    //     return rc;
    //   }
    // }

    // rc = RC::NOTFOUND;
    // for (const ExprPointerType &expression : expressions_) {
    //   if (0 == strcmp(spec.alias(), expression->name())) {
    //     rc = get_value(expression, cell);
    //     break;
    //   }
    // }

    // return rc;
    RC rc = RC::SUCCESS;

    if (child_tuple_ != nullptr) {
      if (view_ == nullptr) {

        rc = child_tuple_->find_cell(spec, cell);
        if (OB_SUCC(rc)) {
          return rc;
        }
      } else {
        // TupleCellSpec &spec2 = const_cast<TupleCellSpec &>(spec);
        auto it = view_->attr_from_table_and_field().find(string(spec.field_name()));
        // assert(it != view->attr_from_table_and_field().end());
        if (it == view_->attr_from_table_and_field().end()) {
          cout << "expression_tuple:find_cell:error! it == end()" << endl;
          cout << "need to find: " << spec.field_name() << endl;
          cout << "attr_from_table_and_field size: " << view_->attr_from_table_and_field().size() << endl;
          cout << "----------------------" << endl;
          for (auto &&[a, b] : view_->attr_from_table_and_field()) {
            cout << a << " " << b.first << " " << b.second << endl;
          }
          cout << "----------------------" << endl;

        } else {
          TupleCellSpec new_spec{it->second.first.c_str(), it->second.second.c_str()};
          rc = child_tuple_->find_cell(new_spec, cell);
          if (OB_SUCC(rc)) {
            return rc;
          }
        }
      }
    }

    rc = RC::NOTFOUND;
    for (const ExprPointerType &expression : expressions_) {

      if (0 == strcmp(spec.alias(), expression->name())) {
        // cout << "sepc.alias is same as expression.name(): " << spec.alias() << endl;
        rc = get_value(expression, cell);
        break;
      }
      if (0 == strcmp(spec.alias(), expression->alias())) {
        // cout << "sepc.alias is same as expression.name(): " << spec.alias() << endl;
        rc = get_value(expression, cell);
        break;
      }
      if (0 == strcmp(spec.field_name(), expression->alias())) {
        // cout << "sepc.alias is same as expression.name(): " << spec.alias() << endl;
        rc = get_value(expression, cell);
        break;
      }
      if (0 == strcmp(spec.field_name(), expression->name())) {
        // cout << "sepc.alias is same as expression.name(): " << spec.alias() << endl;
        rc = get_value(expression, cell);
        break;
      }
    }

    return rc;
  }
  void         set_view(View *view) { view_ = view; }
  const Tuple *child_tuple() const { return child_tuple_; }

private:
  RC get_value(const ExprPointerType &expression, Value &value) const
  {
    RC rc = RC::SUCCESS;
    if (child_tuple_ != nullptr) {
      rc = expression->get_value(*child_tuple_, value);
    } else {
      rc = expression->try_get_value(value);
    }
    if (rc != RC::SUCCESS) {
      LOG_WARN("ExpressionTuple get_value failed. expr_type=%d name=%s child_tuple_null=%d rc=%s",
               static_cast<int>(expression->type()), expression->name(), child_tuple_ == nullptr, strrc(rc));
    }
    return rc;
  }

private:
  const vector<ExprPointerType> &expressions_;
  View                          *view_        = nullptr;
  const Tuple                   *child_tuple_ = nullptr;
};
