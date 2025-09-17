/* Copyright (c) 2023 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2023/08/16.
//

#include "sql/optimizer/logical_plan_generator.h"

#include "common/log/log.h"
#include "sql/parser/expression_binder.h"
#include "session/session.h"
#include "storage/db/db.h"

#include "sql/operator/calc_logical_operator.h"
#include "sql/operator/delete_logical_operator.h"
#include "sql/operator/explain_logical_operator.h"
#include "sql/operator/insert_logical_operator.h"
#include "sql/operator/join_logical_operator.h"
#include "sql/operator/logical_operator.h"
#include "sql/operator/predicate_logical_operator.h"
#include "sql/operator/project_logical_operator.h"
#include "sql/operator/table_get_logical_operator.h"
#include "sql/operator/group_by_logical_operator.h"
#include "sql/operator/sort_logical_operator.h"

#include "sql/stmt/calc_stmt.h"
#include "sql/stmt/delete_stmt.h"
#include "sql/stmt/explain_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "sql/stmt/insert_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "sql/stmt/stmt.h"

#include "sql/expr/expression_iterator.h"

using namespace std;
using namespace common;

RC LogicalPlanGenerator::create(Stmt *stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  RC rc = RC::SUCCESS;
  switch (stmt->type()) {
    case StmtType::CALC: {
      CalcStmt *calc_stmt = static_cast<CalcStmt *>(stmt);

      rc = create_plan(calc_stmt, logical_operator);
    } break;

    case StmtType::SELECT: {
      SelectStmt *select_stmt = static_cast<SelectStmt *>(stmt);

      rc = create_plan(select_stmt, logical_operator);
    } break;

    case StmtType::INSERT: {
      InsertStmt *insert_stmt = static_cast<InsertStmt *>(stmt);

      rc = create_plan(insert_stmt, logical_operator);
    } break;

    case StmtType::DELETE: {
      DeleteStmt *delete_stmt = static_cast<DeleteStmt *>(stmt);

      rc = create_plan(delete_stmt, logical_operator);
    } break;

    case StmtType::EXPLAIN: {
      ExplainStmt *explain_stmt = static_cast<ExplainStmt *>(stmt);

      rc = create_plan(explain_stmt, logical_operator);
    } break;
    default: {
      rc = RC::UNIMPLEMENTED;
    }
  }
  return rc;
}

RC LogicalPlanGenerator::create_plan(CalcStmt *calc_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  logical_operator.reset(new CalcLogicalOperator(std::move(calc_stmt->expressions())));
  return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(SelectStmt *select_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  unique_ptr<LogicalOperator> *last_oper = nullptr;

  // Initialize binder context with tables for this SELECT
  binder_context_ = BinderContext();
  binder_ctx_initialized_ = true;
  const vector<Table *> &tables = select_stmt->tables();
  for (Table *table : tables) {
    binder_context_.add_table(table);
  }

  unique_ptr<LogicalOperator> table_oper(nullptr);
  last_oper = &table_oper;
  unique_ptr<LogicalOperator> predicate_oper;

  RC rc = create_plan(select_stmt->filter_stmt(), predicate_oper);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to create predicate logical plan. rc=%s", strrc(rc));
    return rc;
  }

  // 如果 FilterStmt 为空但有原始条件，尝试处理包含 SubQueryExpr 的条件
  if (!predicate_oper && !select_stmt->raw_conditions().empty()) {
    rc = create_predicate_from_raw_conditions(select_stmt->raw_conditions(), predicate_oper, tables);
    if (OB_FAIL(rc)) {
      LOG_WARN("failed to create predicate from raw conditions. rc=%s", strrc(rc));
      return rc;
    }
  }

  for (Table *table : tables) {

    unique_ptr<LogicalOperator> table_get_oper(new TableGetLogicalOperator(table, ReadWriteMode::READ_ONLY));
    if (table_oper == nullptr) {
      table_oper = std::move(table_get_oper);
    } else {
      JoinLogicalOperator *join_oper = new JoinLogicalOperator;
      join_oper->add_child(std::move(table_oper));
      join_oper->add_child(std::move(table_get_oper));
      table_oper = unique_ptr<LogicalOperator>(join_oper);
    }
  }


  if (predicate_oper) {
    if (*last_oper) {
      predicate_oper->add_child(std::move(*last_oper));
    } else {
      // For queries without FROM clause but with WHERE clause (e.g., "select 1 where 1 in (1,2)")
      // we need to create a dummy CalcLogicalOperator as the child of PredicateLogicalOperator
      vector<unique_ptr<Expression>> dummy_expressions;
      dummy_expressions.emplace_back(make_unique<ValueExpr>(Value(1))); // dummy expression
      unique_ptr<LogicalOperator> calc_oper(new CalcLogicalOperator(std::move(dummy_expressions)));
      predicate_oper->add_child(std::move(calc_oper));
    }

    last_oper = &predicate_oper;
  }

  unique_ptr<LogicalOperator> group_by_oper;
  rc = create_group_by_plan(select_stmt, group_by_oper);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to create group by logical plan. rc=%s", strrc(rc));
    return rc;
  }

  if (group_by_oper) {
    if (*last_oper) {
      group_by_oper->add_child(std::move(*last_oper));
    }

    last_oper = &group_by_oper;

    // HAVING: place predicate after group by
    unique_ptr<LogicalOperator> having_oper;
    if (select_stmt->having_expr() != nullptr) {
      // bind having expression
      unique_ptr<Expression> having_expr = select_stmt->having_expr()->copy();
      if (binder_ctx_initialized_) {
        ExpressionBinder binder(binder_context_);
        vector<unique_ptr<Expression>> bound;
        RC brc = binder.bind_expression(having_expr, bound);
        if (OB_FAIL(brc)) {
          return brc;
        }
        if (bound.size() == 1) {
          having_expr = std::move(bound[0]);
        }
      }
      having_oper = make_unique<PredicateLogicalOperator>(std::move(having_expr));
      having_oper->add_child(std::move(*last_oper));
      last_oper = &having_oper;
    }
  }

  // 创建ORDER BY算子（只有在有子算子的情况下才创建）
  unique_ptr<LogicalOperator> sort_oper;
  if (!select_stmt->order_by_expressions().empty() && *last_oper) {
    vector<unique_ptr<Expression>> order_by_expressions;
    for (auto &expr : select_stmt->order_by_expressions()) {
      unique_ptr<Expression> ob = expr->copy();
      if (binder_ctx_initialized_) {
        ExpressionBinder binder(binder_context_);
        vector<unique_ptr<Expression>> bound;
        RC brc = binder.bind_expression(ob, bound);
        if (OB_FAIL(brc)) {
          return brc;
        }
        if (bound.size() == 1) {
          ob = std::move(bound[0]);
        }
      }
      order_by_expressions.emplace_back(std::move(ob));
    }

    sort_oper = make_unique<SortLogicalOperator>(std::move(order_by_expressions),
                                                 select_stmt->order_by_directions());
    sort_oper->add_child(std::move(*last_oper));

    last_oper = &sort_oper;
  }

  // Bind projection expressions
  vector<unique_ptr<Expression>> proj_exprs = std::move(select_stmt->query_expressions());
  if (binder_ctx_initialized_) {
    ExpressionBinder binder(binder_context_);
    for (auto &expr : proj_exprs) {
      vector<unique_ptr<Expression>> bound;
      RC brc = binder.bind_expression(expr, bound);
      if (OB_FAIL(brc)) {
        return brc;
      }
      if (!bound.empty()) {
        // For SELECT list, star may expand to multiple expressions.
        // Replace current expr with the first, and append the rest.
        auto first = std::move(bound[0]);
        // if more than one, insert after current
        if (bound.size() > 1) {
          // insert remaining after first
          for (size_t i = 1; i < bound.size(); ++i) {
            proj_exprs.emplace_back(std::move(bound[i]));
          }
        }
        expr = std::move(first);
      }
    }
  }

  auto project_oper = make_unique<ProjectLogicalOperator>(std::move(proj_exprs));
  if (*last_oper) {
    project_oper->add_child(std::move(*last_oper));
  }

  logical_operator = std::move(project_oper);
  return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(FilterStmt *filter_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  RC                                  rc = RC::SUCCESS;
  vector<unique_ptr<Expression>> cmp_exprs;
  bool always_false = false;  // 常量折叠：若任一比较恒为假（AND 语义），整体恒假
  const vector<FilterUnit *>    &filter_units = filter_stmt->filter_units();
  for (const FilterUnit *filter_unit : filter_units) {
    // 处理IN_VALUES_OP和NOT_IN_VALUES_OP，创建InExpr
    if (filter_unit->comp() == IN_VALUES_OP || filter_unit->comp() == NOT_IN_VALUES_OP) {
      const FilterObj &filter_obj_left = filter_unit->left();

      unique_ptr<Expression> left;
      if (filter_obj_left.is_attr()) {
        left = make_unique<FieldExpr>(filter_obj_left.field);
      } else if (filter_obj_left.is_value()) {
        left = make_unique<ValueExpr>(filter_obj_left.value);
      } else if (filter_obj_left.is_expr()) {
        left = filter_obj_left.expr->copy();
      }

      if (left) {
        // Bind expression to actual table fields if context is available
        if (binder_ctx_initialized_) {
          ExpressionBinder binder(binder_context_);
          vector<unique_ptr<Expression>> bound_left;
          RC brc = binder.bind_expression(left, bound_left);
          if (OB_FAIL(brc)) return brc;
          if (bound_left.size() == 1) {
            left = std::move(bound_left[0]);
          }
        }

        bool is_not_in = (filter_unit->comp() == NOT_IN_VALUES_OP);
        auto in_expr = make_unique<InExpr>(std::move(left), filter_unit->right_values(), is_not_in);
        cmp_exprs.emplace_back(std::move(in_expr));
      }
      continue;
    }

    // 处理IN_OP和NOT_IN_OP（子查询形式）
    if (filter_unit->comp() == IN_OP || filter_unit->comp() == NOT_IN_OP) {
      if (filter_unit->sub_query()) {
        // 对于子查询IN/NOT IN操作，创建带有子查询的ComparisonExpr
        LOG_INFO("Creating ComparisonExpr with subquery for IN/NOT IN operation");

        const FilterObj &filter_obj_left = filter_unit->left();

        unique_ptr<Expression> left;
        if (filter_obj_left.is_attr()) {
          left = make_unique<FieldExpr>(filter_obj_left.field);
        } else if (filter_obj_left.is_value()) {
          left = make_unique<ValueExpr>(filter_obj_left.value);
        } else if (filter_obj_left.is_expr()) {
          left = filter_obj_left.expr->copy();
        }

        if (left) {
          // Bind expression to actual table fields if context is available
          if (binder_ctx_initialized_) {
            ExpressionBinder binder(binder_context_);
            vector<unique_ptr<Expression>> bound_left;
            RC brc = binder.bind_expression(left, bound_left);
            if (OB_FAIL(brc)) return brc;
            if (bound_left.size() == 1) {
              left = std::move(bound_left[0]);
            }
          }

          // 创建一个带有FilterUnit的ComparisonExpr
          // 这样我们可以在运行时访问子查询
          auto cmp_expr = make_unique<ComparisonExpr>(filter_unit->comp(), std::move(left), const_cast<FilterUnit*>(filter_unit));
          cmp_exprs.emplace_back(std::move(cmp_expr));
        }
        continue;
      }
    }

    const FilterObj &filter_obj_left  = filter_unit->left();
    const FilterObj &filter_obj_right = filter_unit->right();

    unique_ptr<Expression> left;
    if (filter_obj_left.is_attr()) {
      left = make_unique<FieldExpr>(filter_obj_left.field);
    } else if (filter_obj_left.is_value()) {
      left = make_unique<ValueExpr>(filter_obj_left.value);
    } else if (filter_obj_left.is_expr()) {
      left = filter_obj_left.expr->copy();
    }

    unique_ptr<Expression> right;
    if (filter_obj_right.is_attr()) {
      right = make_unique<FieldExpr>(filter_obj_right.field);
    } else if (filter_obj_right.is_value()) {
      right = make_unique<ValueExpr>(filter_obj_right.value);
    } else if (filter_obj_right.is_expr()) {
      right = filter_obj_right.expr->copy();
    }

    // Bind expressions to actual table fields if context is available
    if (binder_ctx_initialized_) {
      ExpressionBinder binder(binder_context_);
      vector<unique_ptr<Expression>> bound_left;
      vector<unique_ptr<Expression>> bound_right;
      if (left) {
        RC brc = binder.bind_expression(left, bound_left);
        if (OB_FAIL(brc)) return brc;
        if (bound_left.size() == 1) {
          left = std::move(bound_left[0]);
        }
      }
      if (right) {
        RC brc = binder.bind_expression(right, bound_right);
        if (OB_FAIL(brc)) return brc;
        if (bound_right.size() == 1) {
          right = std::move(bound_right[0]);
        }
      }
    }

    // Skip type checking for IN operations since ListExpr has UNDEFINED value_type
    if (filter_unit->comp() != IN_OP && filter_unit->comp() != NOT_IN_OP &&
        left->value_type() != right->value_type()) {
      auto left_to_right_cost = implicit_cast_cost(left->value_type(), right->value_type());
      auto right_to_left_cost = implicit_cast_cost(right->value_type(), left->value_type());
      if (left_to_right_cost <= right_to_left_cost && left_to_right_cost != INT32_MAX) {
        ExprType left_type = left->type();
        auto cast_expr = make_unique<CastExpr>(std::move(left), right->value_type());
        if (left_type == ExprType::VALUE) {
          Value left_val;
          if (OB_FAIL(rc = cast_expr->try_get_value(left_val)))
          {
            LOG_WARN("failed to get value from left child", strrc(rc));
            return rc;
          }
          left = make_unique<ValueExpr>(left_val);
        } else {
          left = std::move(cast_expr);
        }
      } else if (right_to_left_cost < left_to_right_cost && right_to_left_cost != INT32_MAX) {
        ExprType right_type = right->type();
        auto cast_expr = make_unique<CastExpr>(std::move(right), left->value_type());
        if (right_type == ExprType::VALUE) {
          Value right_val;
          if (OB_FAIL(rc = cast_expr->try_get_value(right_val)))
          {
            LOG_WARN("failed to get value from right child", strrc(rc));
            return rc;
          }
          right = make_unique<ValueExpr>(right_val);
        } else {
          right = std::move(cast_expr);
        }

      } else {
        // Fallback: if both sides are numeric, promote to FLOATS instead of failing
        auto is_numeric = [](AttrType t) { return t == AttrType::INTS || t == AttrType::FLOATS; };
        if (is_numeric(left->value_type()) && is_numeric(right->value_type())) {
          // Cast the INT side to FLOAT
          if (left->value_type() == AttrType::INTS && right->value_type() == AttrType::FLOATS) {
            ExprType lt = left->type();
            auto cast_expr = make_unique<CastExpr>(std::move(left), AttrType::FLOATS);
            if (lt == ExprType::VALUE) {
              Value lval;
              RC lrc = cast_expr->try_get_value(lval);
              if (OB_FAIL(lrc)) return lrc;
              left = make_unique<ValueExpr>(lval);
            } else {
              left = std::move(cast_expr);
            }
          } else if (left->value_type() == AttrType::FLOATS && right->value_type() == AttrType::INTS) {
            ExprType rt = right->type();
            auto cast_expr = make_unique<CastExpr>(std::move(right), AttrType::FLOATS);
            if (rt == ExprType::VALUE) {
              Value rval;
              RC rrc = cast_expr->try_get_value(rval);
              if (OB_FAIL(rrc)) return rrc;
              right = make_unique<ValueExpr>(rval);
            } else {
              right = std::move(cast_expr);
            }
          }
        } else {
          rc = RC::UNSUPPORTED;
          LOG_WARN("unsupported cast from %s to %s", attr_type_to_string(left->value_type()), attr_type_to_string(right->value_type()));
          return rc;
        }
      }
    }

    // Constant predicate folding: if both sides are constants, evaluate immediately
    if (left->type() == ExprType::VALUE && right->type() == ExprType::VALUE) {
      auto tmp = make_unique<ComparisonExpr>(filter_unit->comp(), left->copy(), right->copy());
      Value folded;
      RC frc = tmp->try_get_value(folded);
      if (frc == RC::SUCCESS) {
        if (!folded.get_boolean()) {
          always_false = true; // AND 语义下，出现恒假即可确定整体恒假
        }
        // 恒真则直接丢弃该条件，不加入 cmp_exprs
        continue;
      }
      // 如果无法在计划期计算，退化为运行期计算
    }

    ComparisonExpr *cmp_expr = new ComparisonExpr(filter_unit->comp(), std::move(left), std::move(right));
    cmp_exprs.emplace_back(cmp_expr);
  }

  unique_ptr<PredicateLogicalOperator> predicate_oper;
  if (always_false) {
    // 整体恒假：构造一个恒为 false 的布尔谓词，执行期将不会输出任何行
    Value v(false);
    unique_ptr<Expression> false_expr = make_unique<ValueExpr>(v);
    predicate_oper = unique_ptr<PredicateLogicalOperator>(new PredicateLogicalOperator(std::move(false_expr)));
  } else if (!cmp_exprs.empty()) {
    unique_ptr<ConjunctionExpr> conjunction_expr(new ConjunctionExpr(ConjunctionExpr::Type::AND, cmp_exprs));

    // Bind the full predicate expression tree
    unique_ptr<Expression> pred_expr(conjunction_expr.release());
    if (binder_ctx_initialized_) {
      ExpressionBinder binder(binder_context_);
      vector<unique_ptr<Expression>> bound;
      RC brc = binder.bind_expression(pred_expr, bound);
      if (OB_FAIL(brc)) {
        return brc;
      }
      if (bound.size() == 1) {
        pred_expr = std::move(bound[0]);
      }
    }

    predicate_oper = unique_ptr<PredicateLogicalOperator>(new PredicateLogicalOperator(std::move(pred_expr)));
  } else {
    // 全部恒真：不需要谓词算子
  }

  logical_operator = std::move(predicate_oper);
  return rc;
}

int LogicalPlanGenerator::implicit_cast_cost(AttrType from, AttrType to)
{
  if (from == to) {
    return 0;
  }
  return DataType::type_instance(from)->cast_cost(to);
}

RC LogicalPlanGenerator::create_plan(InsertStmt *insert_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  Table        *table = insert_stmt->table();
  vector<Value> values(insert_stmt->values(), insert_stmt->values() + insert_stmt->value_amount());

  InsertLogicalOperator *insert_operator = new InsertLogicalOperator(table, values);
  logical_operator.reset(insert_operator);
  return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(DeleteStmt *delete_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  Table                      *table       = delete_stmt->table();
  FilterStmt                 *filter_stmt = delete_stmt->filter_stmt();
  unique_ptr<LogicalOperator> table_get_oper(new TableGetLogicalOperator(table, ReadWriteMode::READ_WRITE));

  unique_ptr<LogicalOperator> predicate_oper;

  RC rc = create_plan(filter_stmt, predicate_oper);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  unique_ptr<LogicalOperator> delete_oper(new DeleteLogicalOperator(table));

  if (predicate_oper) {
    predicate_oper->add_child(std::move(table_get_oper));
    delete_oper->add_child(std::move(predicate_oper));
  } else {
    delete_oper->add_child(std::move(table_get_oper));
  }

  logical_operator = std::move(delete_oper);
  return rc;
}

RC LogicalPlanGenerator::create_plan(ExplainStmt *explain_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  unique_ptr<LogicalOperator> child_oper;

  Stmt *child_stmt = explain_stmt->child();

  RC rc = create(child_stmt, child_oper);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create explain's child operator. rc=%s", strrc(rc));
    return rc;
  }

  logical_operator = unique_ptr<LogicalOperator>(new ExplainLogicalOperator);
  logical_operator->add_child(std::move(child_oper));
  return rc;
}

RC LogicalPlanGenerator::create_group_by_plan(SelectStmt *select_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  vector<unique_ptr<Expression>> &group_by_expressions = select_stmt->group_by();
  vector<Expression *> aggregate_expressions;
  vector<unique_ptr<Expression>> &query_expressions = select_stmt->query_expressions();
  function<RC(unique_ptr<Expression>&)> collector = [&](unique_ptr<Expression> &expr) -> RC {
    RC rc = RC::SUCCESS;
    if (expr->type() == ExprType::AGGREGATION) {
      expr->set_pos(aggregate_expressions.size() + group_by_expressions.size());
      aggregate_expressions.push_back(expr.get());
    }
    rc = ExpressionIterator::iterate_child_expr(*expr, collector);
    return rc;
  };

  function<RC(unique_ptr<Expression>&)> bind_group_by_expr = [&](unique_ptr<Expression> &expr) -> RC {
    RC rc = RC::SUCCESS;
    for (size_t i = 0; i < group_by_expressions.size(); i++) {
      auto &group_by = group_by_expressions[i];
      if (expr->type() == ExprType::AGGREGATION) {
        break;
      } else if (expr->equal(*group_by)) {
        expr->set_pos(i);
        continue;
      } else {
        rc = ExpressionIterator::iterate_child_expr(*expr, bind_group_by_expr);
      }
    }
    return rc;
  };

 bool found_unbound_column = false;
  function<RC(unique_ptr<Expression>&)> find_unbound_column = [&](unique_ptr<Expression> &expr) -> RC {
    RC rc = RC::SUCCESS;
    if (expr->type() == ExprType::AGGREGATION) {
      // do nothing
    } else if (expr->pos() != -1) {
      // do nothing
    } else if (expr->type() == ExprType::FIELD) {
      found_unbound_column = true;
    }else {
      rc = ExpressionIterator::iterate_child_expr(*expr, find_unbound_column);
    }
    return rc;
  };
  

  for (unique_ptr<Expression> &expression : query_expressions) {
    bind_group_by_expr(expression);
  }

  for (unique_ptr<Expression> &expression : query_expressions) {
    find_unbound_column(expression);
  }

  // collect all aggregate expressions
  for (unique_ptr<Expression> &expression : query_expressions) {
    collector(expression);
  }

  if (group_by_expressions.empty() && aggregate_expressions.empty()) {
    // 既没有group by也没有聚合函数，不需要group by
    return RC::SUCCESS;
  }

  if (found_unbound_column) {
    LOG_WARN("column must appear in the GROUP BY clause or must be part of an aggregate function");
    return RC::INVALID_ARGUMENT;
  }

  // 如果只需要聚合，但是没有group by 语句，需要生成一个空的group by 语句

  auto group_by_oper = make_unique<GroupByLogicalOperator>(std::move(group_by_expressions),
                                                           std::move(aggregate_expressions));
  logical_operator = std::move(group_by_oper);
  return RC::SUCCESS;
}

bool LogicalPlanGenerator::is_join_condition(Expression *expr, const vector<Table *> &tables)
{
  if (expr->type() != ExprType::COMPARISON) {
    return false;
  }

  auto comp_expr = dynamic_cast<ComparisonExpr*>(expr);
  if (!comp_expr) {
    return false;
  }

  auto &left = comp_expr->left();
  auto &right = comp_expr->right();

  // 检查是否是两个字段之间的比较
  if (left->type() == ExprType::FIELD && right->type() == ExprType::FIELD) {
    auto left_field = dynamic_cast<FieldExpr*>(left.get());
    auto right_field = dynamic_cast<FieldExpr*>(right.get());

    if (left_field && right_field) {
      // 检查是否来自不同的表
      return left_field->field().table() != right_field->field().table();
    }
  }

  return false;
}

bool LogicalPlanGenerator::involves_tables(Expression *expr, LogicalOperator *left_oper, LogicalOperator *right_oper)
{
  if (expr->type() != ExprType::COMPARISON) {
    return false;
  }

  auto comp_expr = dynamic_cast<ComparisonExpr*>(expr);
  if (!comp_expr) {
    return false;
  }

  auto &left = comp_expr->left();
  auto &right = comp_expr->right();

  if (left->type() == ExprType::FIELD && right->type() == ExprType::FIELD) {
    auto left_field = dynamic_cast<FieldExpr*>(left.get());
    auto right_field = dynamic_cast<FieldExpr*>(right.get());

    if (left_field && right_field) {
      // 简化实现：假设当前JOIN涉及的表就是我们需要的
      // 在实际实现中，可能需要更复杂的逻辑来确定表的归属
      return true;
    }
  }

  return false;
}

RC LogicalPlanGenerator::create_predicate_from_raw_conditions(const vector<ConditionSqlNode> &conditions, unique_ptr<LogicalOperator> &logical_operator)
{
  // 默认使用 csq_1 表，这是为了向后兼容
  vector<Table*> default_tables;
  Db *db = Session::current_session()->get_current_db();
  if (db) {
    Table *table = db->find_table("csq_1");
    if (table) {
      default_tables.push_back(table);
    }
  }

  return create_predicate_from_raw_conditions(conditions, logical_operator, default_tables);
}

RC LogicalPlanGenerator::create_predicate_from_raw_conditions(const vector<ConditionSqlNode> &conditions, unique_ptr<LogicalOperator> &logical_operator, const vector<Table*> &tables)
{
  printf("DEBUG: [LOGICAL_PLAN] create_predicate_from_raw_conditions called with %d conditions and %zu tables\n", (int)conditions.size(), tables.size());

  if (conditions.empty()) {
    return RC::SUCCESS;
  }

  // 目前只处理单个条件的情况
  if (conditions.size() > 1) {
    LOG_WARN("Multiple conditions with SubQueryExpr not yet supported");
    return RC::UNIMPLEMENTED;
  }

  const ConditionSqlNode &condition = conditions[0];

  // 检查是否包含 SubQueryExpr
  bool has_subquery = false;
  if (condition.left_expr && condition.left_expr->type() == ExprType::SUBQUERY) {
    has_subquery = true;
    printf("DEBUG: [LOGICAL_PLAN] Found SubQueryExpr in left expression\n");
  }
  if (condition.right_expr && condition.right_expr->type() == ExprType::SUBQUERY) {
    has_subquery = true;
    printf("DEBUG: [LOGICAL_PLAN] Found SubQueryExpr in right expression\n");
  }

  if (!has_subquery) {
    LOG_WARN("No SubQueryExpr found in conditions");
    return RC::INTERNAL;
  }

  // 创建 ComparisonExpr 来处理包含 SubQueryExpr 的条件
  CompOp comp_op = condition.comp;
  unique_ptr<Expression> left_expr;
  unique_ptr<Expression> right_expr;

  // 对于包含 SubQueryExpr 的条件，我们需要先绑定表达式，然后再复制
  // 这是因为原始表达式可能是 UnboundFieldExpr，需要转换为 FieldExpr

  printf("DEBUG: [LOGICAL_PLAN] Original left expression type=%d, right expression type=%d\n",
         (int)condition.left_expr->type(), (int)condition.right_expr->type());

  // 复制左表达式
  if (condition.left_expr) {
    if (condition.left_expr->type() == ExprType::UNBOUND_FIELD) {
      printf("DEBUG: [LOGICAL_PLAN] Left expression is UNBOUND_FIELD, need to bind it first\n");

      // 对于 UNBOUND_FIELD，我们需要手动绑定它
      // 创建一个临时的绑定上下文
      BinderContext binder_context;

      // 使用传入的表列表
      if (tables.empty()) {
        LOG_WARN("No tables provided for binding");
        return RC::INTERNAL;
      }

      for (Table* table : tables) {
        binder_context.add_table(table);
        printf("DEBUG: [LOGICAL_PLAN] Added table '%s' to binder context\n", table->name());
      }

      // 创建表达式绑定器
      ExpressionBinder expression_binder(binder_context);

      // 绑定左表达式
      unique_ptr<Expression> left_expr_copy = condition.left_expr->copy();
      vector<unique_ptr<Expression>> bound_expressions;
      RC rc = expression_binder.bind_expression(left_expr_copy, bound_expressions);
      if (rc != RC::SUCCESS) {
        LOG_WARN("Failed to bind left expression. rc=%s", strrc(rc));
        return rc;
      }

      if (bound_expressions.empty()) {
        LOG_WARN("No bound expressions returned");
        return RC::INTERNAL;
      }

      left_expr = std::move(bound_expressions[0]);
      printf("DEBUG: [LOGICAL_PLAN] Successfully bound left expression, type=%d\n", (int)left_expr->type());
    } else {
      left_expr = condition.left_expr->copy();
      printf("DEBUG: [LOGICAL_PLAN] Successfully copied left expression, type=%d\n", (int)left_expr->type());
    }

    if (!left_expr) {
      LOG_WARN("Failed to copy left expression");
      return RC::INTERNAL;
    }
  }

  // 复制右表达式
  if (condition.right_expr) {
    if (condition.right_expr->type() == ExprType::UNBOUND_FIELD) {
      printf("DEBUG: [LOGICAL_PLAN] Right expression is UNBOUND_FIELD, need to bind it first\n");

      // 对于 UNBOUND_FIELD，我们需要手动绑定它
      // 创建一个临时的绑定上下文
      BinderContext binder_context;

      // 使用传入的表列表
      if (tables.empty()) {
        LOG_WARN("No tables provided for binding");
        return RC::INTERNAL;
      }

      for (Table* table : tables) {
        binder_context.add_table(table);
        printf("DEBUG: [LOGICAL_PLAN] Added table '%s' to binder context for right expr\n", table->name());
      }

      // 创建表达式绑定器
      ExpressionBinder expression_binder(binder_context);

      // 绑定右表达式
      unique_ptr<Expression> right_expr_copy = condition.right_expr->copy();
      vector<unique_ptr<Expression>> bound_expressions;
      RC rc = expression_binder.bind_expression(right_expr_copy, bound_expressions);
      if (rc != RC::SUCCESS) {
        LOG_WARN("Failed to bind right expression. rc=%s", strrc(rc));
        return rc;
      }

      if (bound_expressions.empty()) {
        LOG_WARN("No bound expressions returned for right expr");
        return RC::INTERNAL;
      }

      right_expr = std::move(bound_expressions[0]);
      printf("DEBUG: [LOGICAL_PLAN] Successfully bound right expression, type=%d\n", (int)right_expr->type());
    } else {
      right_expr = condition.right_expr->copy();
      if (!right_expr) {
        LOG_WARN("Failed to copy right expression");
        return RC::INTERNAL;
      }
      printf("DEBUG: [LOGICAL_PLAN] Successfully copied right expression, type=%d\n", (int)right_expr->type());
    }
  }

  if (!left_expr || !right_expr) {
    LOG_WARN("Failed to create expressions for predicate");
    return RC::INTERNAL;
  }

  // 创建 ComparisonExpr
  auto comparison_expr = make_unique<ComparisonExpr>(comp_op, std::move(left_expr), std::move(right_expr));
  printf("DEBUG: [LOGICAL_PLAN] Created ComparisonExpr with comp_op=%d\n", (int)comp_op);

  // 创建 PredicateLogicalOperator（使用单个表达式）
  logical_operator = make_unique<PredicateLogicalOperator>(std::move(comparison_expr));

  printf("DEBUG: [LOGICAL_PLAN] Created PredicateLogicalOperator successfully\n");
  return RC::SUCCESS;
}
