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
// Created by Wangyunlai on 2022/12/13.
//

#include "sql/optimizer/comparison_simplification_rule.h"
#include "common/log/log.h"
#include "sql/expr/expression.h"

RC ComparisonSimplificationRule::rewrite(unique_ptr<Expression> &expr, bool &change_made)
{
  RC rc = RC::SUCCESS;

  change_made = false;
  if (expr->type() == ExprType::COMPARISON) {
    Value value;

    ComparisonExpr *cmp_expr = static_cast<ComparisonExpr *>(expr.get());

    printf("DEBUG: [SIMPLIFICATION] ComparisonSimplificationRule: about to call try_get_value on comparison expr\n");
    printf("DEBUG: [SIMPLIFICATION] Left expr type: %d, Right expr type: %d\n",
        static_cast<int>(cmp_expr->left()->type()),
        static_cast<int>(cmp_expr->right()->type()));

    RC sub_rc = cmp_expr->try_get_value(value);
    printf("DEBUG: [SIMPLIFICATION] try_get_value result: rc=%s\n", strrc(sub_rc));

    if (sub_rc == RC::SUCCESS) {
      printf("DEBUG: [SIMPLIFICATION] Replacing comparison expr with constant value: %s\n", value.to_string().c_str());
      unique_ptr<Expression> new_expr(new ValueExpr(value));
      expr.swap(new_expr);
      change_made = true;
      LOG_TRACE("comparison expression is simplified");
    }
  }
  return rc;
}
