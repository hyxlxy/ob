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
#include <unordered_set>
#include <functional>
#include "sql/stmt/create_view_stmt.h"
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
#include "sql/operator/union_logical_operator.h"
#include "sql/operator/group_by_logical_operator.h"
#include "sql/operator/sort_logical_operator.h"
#include "sql/operator/order_by_logical_operator.h"
#include "sql/operator/limit_logical_operator.h"

#include "sql/stmt/calc_stmt.h"
#include "sql/stmt/delete_stmt.h"
#include "sql/stmt/explain_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "sql/stmt/insert_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "sql/stmt/union_stmt.h"
#include "sql/stmt/stmt.h"

#include "sql/expr/expression_iterator.h"
#include "storage/table/view.h"

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

    case StmtType::UNION: {
      UnionStmt *union_stmt = dynamic_cast<UnionStmt *>(stmt);
      if (union_stmt == nullptr) {
        LOG_WARN("failed to cast Stmt to UnionStmt");
        return RC::INVALID_ARGUMENT;
      }

      rc = create_plan(union_stmt, logical_operator);
    } break;

    case StmtType::CREATE_VIEW: {
      CreateViewStmt *create_view_stmt = static_cast<CreateViewStmt *>(stmt);
      if (create_view_stmt->select_stmt()) {
        auto &&select_stmt = create_view_stmt->select_stmt().get();
        rc                 = create_plan(select_stmt, logical_operator);
        // cout << "create view in select finish" << endl;

      } else {
        rc = RC::UNIMPLEMENTED;
      }
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

  // Initialize binder context with tables and aliases for this SELECT
  binder_context_         = BinderContext();
  binder_ctx_initialized_ = false;
  // binder_ctx_initialized_        = true;
  const vector<Table *> &tables  = select_stmt->tables();
  const vector<string>  &aliases = select_stmt->table_aliases();
  for (size_t i = 0; i < tables.size(); i++) {
    binder_context_.add_table(tables[i]);
    if (i < aliases.size() && !aliases[i].empty()) {
      binder_context_.add_table_with_name(tables[i], aliases[i].c_str());
    }
  }

  // Augment plan tables with any tables referenced by bound expressions in SELECT/GB/HAVING
  struct TableNode
  {
    Table *table;
    string table_alias;
  };
  vector<TableNode> plan_tables;
  for (size_t i = 0; i < tables.size(); i++) {
    plan_tables.push_back({tables[i], aliases[i]});
  }

  {
    std::unordered_set<const Table *> present;
    present.reserve(plan_tables.size());
    for (TableNode &t : plan_tables) {
      present.insert(t.table);
    }

    std::function<void(Expression *)> collect_tables_from_expr;
    collect_tables_from_expr = [&](Expression *expr) {
      if (expr == nullptr)
        return;
      if (expr->type() == ExprType::FIELD) {
        auto *fe = dynamic_cast<FieldExpr *>(expr);
        if (fe != nullptr) {
          const Table *tbl = fe->field().table();
          if (tbl != nullptr && present.find(tbl) == present.end()) {
            // Only augment the physical join table chain; DO NOT mutate binder_context_
            // to avoid changing expression binding/ambiguity for unqualified columns.
            plan_tables.push_back({const_cast<Table *>(tbl), ""});
            present.insert(tbl);
          }
        }
      }
      (void)ExpressionIterator::iterate_child_expr(*expr, [&](unique_ptr<Expression> &child) -> RC {
        collect_tables_from_expr(child.get());
        return RC::SUCCESS;
      });
    };

    for (auto &expr : select_stmt->query_expressions()) {
      collect_tables_from_expr(expr.get());
    }
    for (auto &gexpr : select_stmt->group_by()) {
      collect_tables_from_expr(gexpr.get());
    }
    if (select_stmt->having_expr() != nullptr) {
      collect_tables_from_expr(select_stmt->having_expr());
    }
  }

  unique_ptr<LogicalOperator> table_oper(nullptr);
  last_oper = &table_oper;
  // Defer WHERE predicate creation until after full JOIN chain is built
  unique_ptr<LogicalOperator> predicate_oper;
  RC                          rc = RC::SUCCESS;

  for (TableNode &table_node : plan_tables) {
    Table                      *table = table_node.table;
    string                      alias = table_node.table_alias;
    unique_ptr<LogicalOperator> table_get_oper;

    if (auto &&view = dynamic_cast<View *>(table)) {
      //这个表其实是view
      unique_ptr<LogicalOperator> view_oper;
      SelectStmt                 *view_select_view = dynamic_cast<SelectStmt *>(view->stmt());
      RC                          rc               = create_plan(view_select_view, view_oper);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to create view logical plan: %s", view->name());
        return rc;
      }
      {
        auto &&project_oper = dynamic_cast<ProjectLogicalOperator *>(view_oper.get());
        project_oper->set_view(view);
      }
      // 将视图计划作为当前表操作符
      table_get_oper = std::move(view_oper);
    } else {
      auto &&table_get_tem = std::make_unique<TableGetLogicalOperator>(table, ReadWriteMode::READ_ONLY);
      table_get_tem->set_table_alias(alias);

      table_get_oper = std::move(table_get_tem);
    }
    // (new TableGetLogicalOperator(table, ReadWriteMode::READ_ONLY));
    if (table_oper == nullptr) {
      table_oper = std::move(table_get_oper);
    } else {
      JoinLogicalOperator *join_oper = new JoinLogicalOperator;
      join_oper->add_child(std::move(table_oper));
      join_oper->add_child(std::move(table_get_oper));
      table_oper = unique_ptr<LogicalOperator>(join_oper);
    }
  }

  // Now build WHERE predicate (on top of full JOIN chain)
  if (select_stmt->filter_stmt() != nullptr) {
    rc = create_plan(select_stmt->filter_stmt(), predicate_oper);
    if (OB_FAIL(rc)) {
      LOG_WARN("failed to create predicate logical plan. rc=%s", strrc(rc));
      return rc;
    }
    if (predicate_oper) {
      if (*last_oper) {
        predicate_oper->add_child(std::move(*last_oper));
      } else {
        // For queries without FROM clause but with WHERE clause
        vector<unique_ptr<Expression>> dummy_expressions;
        dummy_expressions.emplace_back(make_unique<ValueExpr>(Value(1)));
        unique_ptr<LogicalOperator> calc_oper(new CalcLogicalOperator(std::move(dummy_expressions)));
        predicate_oper->add_child(std::move(calc_oper));
      }
      last_oper = &predicate_oper;
    }
  }

  unique_ptr<LogicalOperator> group_by_oper;
  rc = create_group_by_plan(select_stmt, group_by_oper);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to create group by logical plan. rc=%s", strrc(rc));
    return rc;
  }

  // HAVING: place predicate after group by (declare at function scope to avoid use-after-scope)
  unique_ptr<LogicalOperator> having_oper;

  if (group_by_oper) {
    if (*last_oper) {
      group_by_oper->add_child(std::move(*last_oper));
    }

    last_oper = &group_by_oper;

    if (select_stmt->having_expr() != nullptr) {
      // bind having expression
      unique_ptr<Expression> having_expr = select_stmt->having_expr()->copy();
      if (binder_ctx_initialized_) {
        ExpressionBinder               binder(binder_context_);
        vector<unique_ptr<Expression>> bound;
        RC                             brc = binder.bind_expression(having_expr, bound);
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
    LOG_INFO("[LOGICAL_PLAN_DEBUG] Creating ORDER BY operator with %zu expressions", 
             select_stmt->order_by_expressions().size());
    
    vector<unique_ptr<Expression>> order_by_expressions;
    for (size_t i = 0; i < select_stmt->order_by_expressions().size(); i++) {
      auto &expr = select_stmt->order_by_expressions()[i];
      
      LOG_INFO("[LOGICAL_PLAN_DEBUG] ORDER BY expression %zu: type=%d, name='%s'", 
               i, static_cast<int>(expr->type()), expr->name());
      
      unique_ptr<Expression> ob = expr->copy();
      if (binder_ctx_initialized_) {
        ExpressionBinder               binder(binder_context_);
        vector<unique_ptr<Expression>> bound;
        
        LOG_INFO("[LOGICAL_PLAN_DEBUG] Binding ORDER BY expression %zu", i);
        
        RC brc = binder.bind_expression(ob, bound);
        if (OB_FAIL(brc)) {
          LOG_WARN("[LOGICAL_PLAN_DEBUG] Failed to bind ORDER BY expression %zu: %s", i, strrc(brc));
          return brc;
        }
        if (bound.size() == 1) {
          ob = std::move(bound[0]);
          LOG_INFO("[LOGICAL_PLAN_DEBUG] ORDER BY expression %zu bound successfully: type=%d", 
                   i, static_cast<int>(ob->type()));
        }
      }
      order_by_expressions.emplace_back(std::move(ob));
    }

    LOG_INFO("[LOGICAL_PLAN_DEBUG] Creating OrderByLogicalOperator");
    
    sort_oper =
        make_unique<OrderByLogicalOperator>(std::move(order_by_expressions), select_stmt->order_by_directions());
    sort_oper->add_child(std::move(*last_oper));

    last_oper = &sort_oper;
    
    LOG_INFO("[LOGICAL_PLAN_DEBUG] ORDER BY operator created successfully");
  } else {
    LOG_INFO("[LOGICAL_PLAN_DEBUG] Skipping ORDER BY: expressions_empty=%d, has_last_oper=%d",
             select_stmt->order_by_expressions().empty(), *last_oper != nullptr);
  }

  // LIMIT: 如果解析阶段提供了 limit_count，则叠加 LIMIT 算子（在 ORDER BY 之后）
  unique_ptr<LogicalOperator> limit_oper;
  if (select_stmt->limit_count() > 0 && *last_oper) {
    limit_oper = make_unique<LimitLogicalOperator>(select_stmt->limit_count());
    limit_oper->add_child(std::move(*last_oper));
    last_oper = &limit_oper;
  }

  // Bind projection expressions
  vector<unique_ptr<Expression>> proj_exprs = std::move(select_stmt->query_expressions());

  if (binder_ctx_initialized_) {
    ExpressionBinder binder(binder_context_);
    for (auto &expr : proj_exprs) {
      vector<unique_ptr<Expression>> bound;
      RC                             brc = binder.bind_expression(expr, bound);
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
  RC                             rc = RC::SUCCESS;
  vector<unique_ptr<Expression>> cmp_exprs;
  bool always_false = false;  // 常量折叠：若任一比较恒为假（AND 语义），整体恒假

  // 优先使用条件树，如果不存在则使用传统的filter_units
  FilterConditionNode *condition_tree = filter_stmt->get_condition_tree();
  if (condition_tree != nullptr) {
    printf("DEBUG: [LOGICAL_PLAN] Using condition tree instead of filter_units\n");
    unique_ptr<Expression> tree_expr;
    rc = create_expression_from_condition_tree(condition_tree, tree_expr);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to create expression from condition tree");
      return rc;
    }
    if (tree_expr) {
      cmp_exprs.emplace_back(std::move(tree_expr));
    }
  } else {
    printf("DEBUG: [LOGICAL_PLAN] Using traditional filter_units\n");
    vector<FilterUnit *> &filter_units = filter_stmt->filter_units();
    for (FilterUnit *filter_unit : filter_units) {

      if (filter_unit == nullptr) {
        ASSERT(0, "no way here");
        continue;
      }

      auto &&left_expr  = filter_unit->left().expr;
      auto &&right_expr = filter_unit->right().expr;

      bool left_is_expr_flag  = filter_unit->left().is_expr();
      bool right_is_expr_flag = filter_unit->right().is_expr();

      //这里主要是在搞子查询
      if ((left_is_expr_flag and left_expr != nullptr and left_expr->type() == ExprType::SUBQUERY) or
          (right_is_expr_flag and right_expr != nullptr and right_expr->type() == ExprType::SUBQUERY)) {
        auto set_sub_logical = [&](SubQueryExpr *&expr) {
          auto                      &&stmt = expr->stmt();
          unique_ptr<LogicalOperator> sub_query_oper;
          // LogicalPlanGenerator        generator;
          // generator.set_out_binder(binder_context_);
          rc = create_plan(stmt.get(), sub_query_oper);
          if (rc != RC::SUCCESS) {
            LOG_WARN("failed to create subquery logical operator. rc=%s", strrc(rc));
            return rc;
          }
          expr->set_logical_operator(std::move(sub_query_oper));
          return rc;
        };

        if (left_expr and left_expr->type() == ExprType::SUBQUERY) {
          auto &&left_expr_no_const = const_cast<Expression *>(left_expr);
          if (auto &&left = dynamic_cast<SubQueryExpr *>(left_expr_no_const)) {
            rc = set_sub_logical(left);
            CHECK_RETURN_RC(rc);
          } else {
            cout << " logical genner error left" << endl;
            return RC::ERROR;
          }
        }
        if (right_expr and right_expr->type() == ExprType::SUBQUERY) {
          auto &&right_expr_no_const = const_cast<Expression *>(right_expr);

          if (auto &&right = dynamic_cast<SubQueryExpr *>(right_expr_no_const)) {
            rc = set_sub_logical(right);
            CHECK_RETURN_RC(rc);
          } else {
            cout << " logical genner error right" << endl;
            return RC::ERROR;
          }
        }

        ComparisonExpr *cmp_expr = new ComparisonExpr(
            filter_unit->comp(), std::move(filter_unit->left_expr_owner_), std::move(filter_unit->right_expr_owner_));
        cmp_exprs.emplace_back(cmp_expr);
        continue;
      }

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
            ExpressionBinder               binder(binder_context_);
            vector<unique_ptr<Expression>> bound_left;
            RC                             brc = binder.bind_expression(left, bound_left);
            if (OB_FAIL(brc))
              return brc;
            if (bound_left.size() == 1) {
              left = std::move(bound_left[0]);
            }
          }

          bool is_not_in = (filter_unit->comp() == NOT_IN_VALUES_OP);
          auto in_expr   = make_unique<InExpr>(std::move(left), filter_unit->right_values(), is_not_in);
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
              ExpressionBinder               binder(binder_context_, &out_binder_);
              vector<unique_ptr<Expression>> bound_left;
              RC                             brc = binder.bind_expression(left, bound_left);
              if (OB_FAIL(brc))
                return brc;
              if (bound_left.size() == 1) {
                left = std::move(bound_left[0]);
              }
            }

            // 创建一个带有FilterUnit的ComparisonExpr
            // 这样我们可以在运行时访问子查询
            auto cmp_expr = make_unique<ComparisonExpr>(
                filter_unit->comp(), std::move(left), const_cast<FilterUnit *>(filter_unit));
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
        ExpressionBinder               binder(binder_context_);
        vector<unique_ptr<Expression>> bound_left;
        vector<unique_ptr<Expression>> bound_right;
        if (left) {
          RC brc = binder.bind_expression(left, bound_left);
          if (OB_FAIL(brc))
            return brc;
          if (bound_left.size() == 1) {
            left = std::move(bound_left[0]);
          }
        }
        if (right) {
          RC brc = binder.bind_expression(right, bound_right);
          if (OB_FAIL(brc))
            return brc;
          if (bound_right.size() == 1) {
            right = std::move(bound_right[0]);
          }
        }
      }

      // Skip type checking for IN operations since ListExpr has UNDEFINED value_type
      // Also skip for IS NULL operations since they don't need type unification
      // Also skip for SubQueryExpr since they have UNDEFINED value_type until execution
      // Also skip for NULL values (UNDEFINED type) since they follow special SQL NULL semantics
      if (filter_unit->comp() != IN_OP && filter_unit->comp() != NOT_IN_OP && filter_unit->comp() != IS_NULL_OP &&
          filter_unit->comp() != IS_NOT_NULL_OP && left->type() != ExprType::SUBQUERY &&
          right->type() != ExprType::SUBQUERY && left->value_type() != AttrType::UNDEFINED &&
          right->value_type() != AttrType::UNDEFINED && left->value_type() != right->value_type()) {
        auto left_to_right_cost = implicit_cast_cost(left->value_type(), right->value_type());
        auto right_to_left_cost = implicit_cast_cost(right->value_type(), left->value_type());
        if (left_to_right_cost <= right_to_left_cost && left_to_right_cost != INT32_MAX) {
          ExprType left_type = left->type();
          auto     cast_expr = make_unique<CastExpr>(std::move(left), right->value_type());
          if (left_type == ExprType::VALUE) {
            Value left_val;
            if (OB_FAIL(rc = cast_expr->try_get_value(left_val))) {
              LOG_WARN("failed to get value from left child", strrc(rc));
              return rc;
            }
            left = make_unique<ValueExpr>(left_val);
          } else {
            left = std::move(cast_expr);
          }
        } else if (right_to_left_cost < left_to_right_cost && right_to_left_cost != INT32_MAX) {
          ExprType right_type = right->type();
          auto     cast_expr  = make_unique<CastExpr>(std::move(right), left->value_type());
          if (right_type == ExprType::VALUE) {
            Value right_val;
            if (OB_FAIL(rc = cast_expr->try_get_value(right_val))) {
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
              ExprType lt        = left->type();
              auto     cast_expr = make_unique<CastExpr>(std::move(left), AttrType::FLOATS);
              if (lt == ExprType::VALUE) {
                Value lval;
                RC    lrc = cast_expr->try_get_value(lval);
                if (OB_FAIL(lrc))
                  return lrc;
                left = make_unique<ValueExpr>(lval);
              } else {
                left = std::move(cast_expr);
              }
            } else if (left->value_type() == AttrType::FLOATS && right->value_type() == AttrType::INTS) {
              ExprType rt        = right->type();
              auto     cast_expr = make_unique<CastExpr>(std::move(right), AttrType::FLOATS);
              if (rt == ExprType::VALUE) {
                Value rval;
                RC    rrc = cast_expr->try_get_value(rval);
                if (OB_FAIL(rrc))
                  return rrc;
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
        auto  tmp = make_unique<ComparisonExpr>(filter_unit->comp(), left->copy(), right->copy());
        Value folded;
        RC    frc = tmp->try_get_value(folded);
        if (frc == RC::SUCCESS) {
          if (!folded.get_boolean()) {
            always_false = true;  // AND 语义下，出现恒假即可确定整体恒假
          }
          // 恒真则直接丢弃该条件，不加入 cmp_exprs
          continue;
        }
        // 如果无法在计划期计算，退化为运行期计算
      }

      ComparisonExpr *cmp_expr = new ComparisonExpr(filter_unit->comp(), std::move(left), std::move(right));
      cmp_exprs.emplace_back(cmp_expr);
    }
  }  // 结束传统filter_units处理的else分支

  unique_ptr<PredicateLogicalOperator> predicate_oper;
  if (always_false) {
    // 整体恒假：构造一个恒为 false 的布尔谓词，执行期将不会输出任何行
    Value                  v(false);
    unique_ptr<Expression> false_expr = make_unique<ValueExpr>(v);
    predicate_oper = unique_ptr<PredicateLogicalOperator>(new PredicateLogicalOperator(std::move(false_expr)));
  } else if (!cmp_exprs.empty()) {
    unique_ptr<ConjunctionExpr> conjunction_expr(new ConjunctionExpr(ConjunctionExpr::Type::AND, cmp_exprs));

    if (!filter_stmt->is_and()) {
      conjunction_expr->set_conjunction_is_or();
    }

    // Bind the full predicate expression tree
    unique_ptr<Expression> pred_expr(conjunction_expr.release());
    if (binder_ctx_initialized_) {
      ExpressionBinder               binder(binder_context_);
      vector<unique_ptr<Expression>> bound;
      RC                             brc = binder.bind_expression(pred_expr, bound);
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
  vector<unique_ptr<Expression>>        &group_by_expressions = select_stmt->group_by();
  vector<Expression *>                   aggregate_expressions;
  vector<unique_ptr<Expression>>        &query_expressions = select_stmt->query_expressions();
  function<RC(unique_ptr<Expression> &)> collector         = [&](unique_ptr<Expression> &expr) -> RC {
    RC rc = RC::SUCCESS;
    if (expr->type() == ExprType::AGGREGATION) {
      expr->set_pos(aggregate_expressions.size() + group_by_expressions.size());
      aggregate_expressions.push_back(expr.get());
    }
    rc = ExpressionIterator::iterate_child_expr(*expr, collector);
    return rc;
  };

  function<RC(unique_ptr<Expression> &)> bind_group_by_expr = [&](unique_ptr<Expression> &expr) -> RC {
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

  bool                                   found_unbound_column = false;
  function<RC(unique_ptr<Expression> &)> find_unbound_column  = [&](unique_ptr<Expression> &expr) -> RC {
    RC rc = RC::SUCCESS;
    if (expr->type() == ExprType::AGGREGATION) {
      // do nothing
    } else if (expr->pos() != -1) {
      // do nothing
    } else if (expr->type() == ExprType::FIELD) {
      found_unbound_column = true;
    } else {
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

  // collect all aggregate expressions from SELECT list
  for (unique_ptr<Expression> &expression : query_expressions) {
    collector(expression);
  }

  // also collect aggregate expressions from HAVING clause (借鉴参考代码思路)
  if (select_stmt->having_expr() != nullptr) {
    ExpressionIterator::collect_having_aggregates(select_stmt->having_expr(), aggregate_expressions);
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

  auto group_by_oper =
      make_unique<GroupByLogicalOperator>(std::move(group_by_expressions), std::move(aggregate_expressions));
  logical_operator = std::move(group_by_oper);
  return RC::SUCCESS;
}

bool LogicalPlanGenerator::is_join_condition(Expression *expr, const vector<Table *> &tables)
{
  if (expr->type() != ExprType::COMPARISON) {
    return false;
  }

  auto comp_expr = dynamic_cast<ComparisonExpr *>(expr);
  if (!comp_expr) {
    return false;
  }

  auto &left  = comp_expr->left();
  auto &right = comp_expr->right();

  // 检查是否是两个字段之间的比较
  if (left->type() == ExprType::FIELD && right->type() == ExprType::FIELD) {
    auto left_field  = dynamic_cast<FieldExpr *>(left.get());
    auto right_field = dynamic_cast<FieldExpr *>(right.get());

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

  auto comp_expr = dynamic_cast<ComparisonExpr *>(expr);
  if (!comp_expr) {
    return false;
  }

  auto &left  = comp_expr->left();
  auto &right = comp_expr->right();

  if (left->type() == ExprType::FIELD && right->type() == ExprType::FIELD) {
    auto left_field  = dynamic_cast<FieldExpr *>(left.get());
    auto right_field = dynamic_cast<FieldExpr *>(right.get());

    if (left_field && right_field) {
      // 简化实现：假设当前JOIN涉及的表就是我们需要的
      // 在实际实现中，可能需要更复杂的逻辑来确定表的归属
      return true;
    }
  }

  return false;
}

RC LogicalPlanGenerator::create_expression_from_condition_tree(FilterConditionNode *node, unique_ptr<Expression> &expr)
{
  if (node == nullptr) {
    expr = nullptr;
    return RC::SUCCESS;
  }

  printf("DEBUG: [LOGICAL_PLAN] Creating expression from condition tree node type: %d\n", static_cast<int>(node->type));

  switch (node->type) {
    case FilterConditionNode::CONDITION: {
      // 单个条件：从FilterUnit创建ComparisonExpr
      if (node->filter_unit == nullptr) {
        LOG_WARN("FilterUnit is null in CONDITION node");
        return RC::INTERNAL;
      }

      FilterUnit *filter_unit = node->filter_unit;

      // 处理IN_VALUES_OP和NOT_IN_VALUES_OP
      if (filter_unit->comp() == IN_VALUES_OP || filter_unit->comp() == NOT_IN_VALUES_OP) {
        const FilterObj       &filter_obj_left = filter_unit->left();
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
            ExpressionBinder               binder(binder_context_);
            vector<unique_ptr<Expression>> bound_left;
            RC                             brc = binder.bind_expression(left, bound_left);
            if (OB_FAIL(brc))
              return brc;
            if (bound_left.size() == 1) {
              left = std::move(bound_left[0]);
            }
          }

          bool is_not_in = (filter_unit->comp() == NOT_IN_VALUES_OP);
          expr           = make_unique<InExpr>(std::move(left), filter_unit->right_values(), is_not_in);
        }
        return RC::SUCCESS;
      }

      // 处理IN_OP和NOT_IN_OP（子查询形式）
      if (filter_unit->comp() == IN_OP || filter_unit->comp() == NOT_IN_OP) {
        if (filter_unit->sub_query()) {
          const FilterObj       &filter_obj_left = filter_unit->left();
          unique_ptr<Expression> left;
          if (filter_obj_left.is_attr()) {
            left = make_unique<FieldExpr>(filter_obj_left.field);
          } else if (filter_obj_left.is_value()) {
            left = make_unique<ValueExpr>(filter_obj_left.value);
          } else if (filter_obj_left.is_expr()) {
            left = filter_obj_left.expr->copy();
          }

          if (left) {
            expr = make_unique<ComparisonExpr>(
                filter_unit->comp(), std::move(left), const_cast<FilterUnit *>(filter_unit));
          }
          return RC::SUCCESS;
        }
      }

      // 普通比较操作
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

      expr = make_unique<ComparisonExpr>(filter_unit->comp(), std::move(left), std::move(right));
      return RC::SUCCESS;
    }

    case FilterConditionNode::AND_GROUP: {
      // AND组：创建ConjunctionExpr with Type::AND
      vector<unique_ptr<Expression>> children;
      for (FilterConditionNode *child : node->children) {
        unique_ptr<Expression> child_expr;
        RC                     rc = create_expression_from_condition_tree(child, child_expr);
        if (rc != RC::SUCCESS) {
          return rc;
        }
        if (child_expr) {
          children.emplace_back(std::move(child_expr));
        }
      }

      if (children.empty()) {
        expr = nullptr;
      } else if (children.size() == 1) {
        expr = std::move(children[0]);
      } else {
        expr = make_unique<ConjunctionExpr>(ConjunctionExpr::Type::AND, children);
      }
      return RC::SUCCESS;
    }

    case FilterConditionNode::OR_GROUP: {
      // OR组：创建ConjunctionExpr with Type::OR
      printf("DEBUG: [LOGICAL_PLAN] Creating OR group with %zu children\n", node->children.size());
      vector<unique_ptr<Expression>> children;
      for (FilterConditionNode *child : node->children) {
        unique_ptr<Expression> child_expr;
        RC                     rc = create_expression_from_condition_tree(child, child_expr);
        if (rc != RC::SUCCESS) {
          return rc;
        }
        if (child_expr) {
          children.emplace_back(std::move(child_expr));
        }
      }

      if (children.empty()) {
        expr = nullptr;
      } else if (children.size() == 1) {
        expr = std::move(children[0]);
      } else {
        printf("DEBUG: [LOGICAL_PLAN] Creating ConjunctionExpr with OR type and %zu children\n", children.size());
        expr = make_unique<ConjunctionExpr>(ConjunctionExpr::Type::OR, children);
      }
      return RC::SUCCESS;
    }

    default: LOG_WARN("Unknown FilterConditionNode type: %d", static_cast<int>(node->type)); return RC::INTERNAL;
  }
}

RC LogicalPlanGenerator::create_plan(UnionStmt *union_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  if (union_stmt == nullptr) {
    LOG_WARN("UnionStmt is null");
    return RC::INVALID_ARGUMENT;
  }

  // 1. 为每个SELECT语句创建逻辑算子
  vector<unique_ptr<LogicalOperator>> child_operators;
  const vector<SelectStmt *>         &select_statements = union_stmt->select_statements();

  for (SelectStmt *select_stmt : select_statements) {
    unique_ptr<LogicalOperator> child_operator;
    RC                          rc = create_plan(select_stmt, child_operator);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create logical plan for SELECT in UNION: %s", strrc(rc));
      return rc;
    }
    child_operators.emplace_back(std::move(child_operator));
  }

  // 2. 创建UNION逻辑算子
  auto union_operator = make_unique<UnionLogicalOperator>(
      union_stmt->distinct_flags(), union_stmt->result_types(), union_stmt->result_names());

  // 3. 将所有子算子添加到UNION算子
  for (auto &child_operator : child_operators) {
    union_operator->add_child(std::move(child_operator));
  }
  // 直接将UNION作为顶层输出
  logical_operator = std::move(union_operator);
  return RC::SUCCESS;
}
