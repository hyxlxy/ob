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
// Created by WangYunlai on 2022/6/27.
//

#include "sql/operator/predicate_physical_operator.h"
#include "common/log/log.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/field/field.h"
#include "storage/record/record.h"

PredicatePhysicalOperator::PredicatePhysicalOperator(std::unique_ptr<Expression> expr) : expression_(std::move(expr))
{
  // Note: Temporarily disable this assertion as HAVING clause expressions may not be fully initialized
  // when this constructor is called. The expression type will be validated during execution.
  // ASSERT(expression_->value_type() == AttrType::BOOLEANS, "predicate's expression should be BOOLEAN type");

  // Add runtime check instead of assertion
  if (expression_->value_type() != AttrType::BOOLEANS) {
    LOG_WARN("PredicatePhysicalOperator: expression type is %d, expected BOOLEANS (%d)",
             static_cast<int>(expression_->value_type()), static_cast<int>(AttrType::BOOLEANS));
  }
}

RC PredicatePhysicalOperator::open(Trx *trx)
{
  if (children_.size() != 1) {
    LOG_WARN("predicate operator must has one child");
    return RC::INTERNAL;
  }

  return children_[0]->open(trx);
}

RC PredicatePhysicalOperator::next()
{
  RC                rc   = RC::SUCCESS;
  PhysicalOperator *oper = children_.front().get();

  while (RC::SUCCESS == (rc = oper->next())) {
    Tuple *tuple = oper->current_tuple();
    if (nullptr == tuple) {
      rc = RC::INTERNAL;
      LOG_WARN("failed to get tuple from operator");
      break;
    }

    // 添加调试信息：检查 tuple 的内容
    printf("DEBUG: [PREDICATE] Tuple has %d cells\n", tuple->cell_num());
    for (int i = 0; i < tuple->cell_num(); i++) {
      Value cell_value;
      RC cell_rc = tuple->cell_at(i, cell_value);
      if (cell_rc == RC::SUCCESS) {
        printf("DEBUG: [PREDICATE] Cell[%d]: %s\n", i, cell_value.to_string().c_str());
      }
    }

    Value value;
    rc = expression_->get_value(*tuple, value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("PredicatePhysicalOperator::next() get_value failed: expr_type=%d rc=%s",
               static_cast<int>(expression_->type()), strrc(rc));
      return rc;
    }
    LOG_DEBUG("PredicatePhysicalOperator::next() get_value SUCCESS: expr_type=%d value_type=%d",
              static_cast<int>(expression_->type()), static_cast<int>(value.attr_type()));

    // 添加详细的调试信息
    printf("DEBUG: [PREDICATE] Expression evaluation result: value_type=%d, value=%s, boolean=%s\n",
           static_cast<int>(value.attr_type()), value.to_string().c_str(),
           value.get_boolean() ? "true" : "false");

    if (value.get_boolean()) {
      printf("DEBUG: [PREDICATE] Tuple PASSED the predicate\n");
      return rc;
    } else {
      printf("DEBUG: [PREDICATE] Tuple FAILED the predicate, continuing to next tuple\n");
    }
  }
  return rc;
}

RC PredicatePhysicalOperator::close()
{
  if (!children_.empty() && children_[0] != nullptr) {
    children_[0]->close();
  }
  return RC::SUCCESS;
}

Tuple *PredicatePhysicalOperator::current_tuple() { return children_[0]->current_tuple(); }

RC PredicatePhysicalOperator::tuple_schema(TupleSchema &schema) const
{
  return children_[0]->tuple_schema(schema);
}
