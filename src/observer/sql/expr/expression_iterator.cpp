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
// Created by Wangyunlai on 2024/05/30.
//

#include "sql/expr/expression_iterator.h"
#include "sql/expr/expression.h"
#include "common/log/log.h"

using namespace std;

RC ExpressionIterator::iterate_child_expr(Expression &expr, function<RC(unique_ptr<Expression> &)> callback)
{
  RC rc = RC::SUCCESS;

  switch (expr.type()) {
    case ExprType::CAST: {
      auto &cast_expr = static_cast<CastExpr &>(expr);
      if (cast_expr.child()) {
        rc = callback(cast_expr.child());
      }
    } break;

    case ExprType::COMPARISON: {
      auto &comparison_expr = static_cast<ComparisonExpr &>(expr);
      if (comparison_expr.left()) {
        rc = callback(comparison_expr.left());
      }
      if (OB_SUCC(rc) && comparison_expr.right()) {
        rc = callback(comparison_expr.right());
      }
    } break;

    case ExprType::CONJUNCTION: {
      auto &conjunction_expr = static_cast<ConjunctionExpr &>(expr);
      for (auto &child : conjunction_expr.children()) {
        if (child) {
          rc = callback(child);
        }
        if (OB_FAIL(rc)) {
          break;
        }
      }
    } break;

    case ExprType::ARITHMETIC: {
      auto &arithmetic_expr = static_cast<ArithmeticExpr &>(expr);
      if (arithmetic_expr.left()) {
        rc = callback(arithmetic_expr.left());
      }
      if (OB_SUCC(rc) && arithmetic_expr.right()) {
        rc = callback(arithmetic_expr.right());
      }
    } break;

    case ExprType::AGGREGATION: {
      auto &aggregate_expr = static_cast<AggregateExpr &>(expr);
      if (aggregate_expr.child()) {
        rc = callback(aggregate_expr.child());
      }
    } break;

    case ExprType::FUNCTION: {
      auto &func_expr = static_cast<FunctionExpr &>(expr);
      for (auto &arg : func_expr.args()) {
        if (arg) {
          rc = callback(arg);
        }
        if (OB_FAIL(rc)) {
          break;
        }
      }
    } break;

    case ExprType::SUBQUERY: {
      // SubQueryExpr doesn't have child expressions that need iteration
      // The subquery itself is handled separately
    } break;

    case ExprType::IN_VALUES: {
      // TODO: Add support for IN_VALUES expression iteration if needed
    } break;

    case ExprType::SUBQUERY_IN: {
      // TODO: Add support for SUBQUERY_IN expression iteration if needed
    } break;

    case ExprType::LIST: {
      // TODO: Add support for LIST expression iteration if needed
    } break;

    case ExprType::UNBOUND_AGGREGATION: {
      // TODO: Add support for UNBOUND_AGGREGATION expression iteration if needed
    } break;

    case ExprType::NONE:
    case ExprType::STAR:
    case ExprType::UNBOUND_FIELD:
    case ExprType::FIELD:
    case ExprType::VALUE: {
      // Do nothing
    } break;

    default: {
      ASSERT(false, "Unknown expression type");
    }
  }

  return rc;
}