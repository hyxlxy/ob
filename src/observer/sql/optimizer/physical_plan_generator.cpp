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
// Created by Wangyunlai on 2022/12/14.
//

#include "common/log/log.h"
#include "sql/expr/expression.h"
#include "session/session.h"
#include "sql/parser/parse.h"
#include "sql/stmt/select_stmt.h"
#include "sql/optimizer/logical_plan_generator.h"
#include "sql/operator/aggregate_vec_physical_operator.h"
#include "sql/operator/calc_logical_operator.h"
#include "sql/operator/calc_physical_operator.h"
#include "sql/operator/delete_logical_operator.h"
#include "sql/operator/delete_physical_operator.h"
#include "sql/operator/explain_logical_operator.h"
#include "sql/operator/explain_physical_operator.h"
#include "sql/operator/expr_vec_physical_operator.h"
#include "sql/operator/group_by_vec_physical_operator.h"
#include "sql/operator/hash_join_physical_operator.h"
#include "sql/operator/index_scan_physical_operator.h"
#include "sql/operator/insert_logical_operator.h"
#include "sql/operator/insert_physical_operator.h"
#include "sql/operator/join_logical_operator.h"
#include "sql/operator/join_physical_operator.h"
#include "sql/operator/predicate_logical_operator.h"
#include "sql/operator/predicate_physical_operator.h"
#include "sql/operator/project_logical_operator.h"
#include "sql/operator/project_physical_operator.h"
#include "sql/operator/project_vec_physical_operator.h"
#include "sql/operator/table_get_logical_operator.h"
#include "sql/operator/table_scan_physical_operator.h"
#include "sql/operator/group_by_logical_operator.h"
#include "sql/operator/group_by_physical_operator.h"
#include "sql/operator/hash_group_by_physical_operator.h"
#include "sql/operator/scalar_group_by_physical_operator.h"
#include "sql/operator/sort_logical_operator.h"
#include "sql/operator/sort_physical_operator.h"
#include "sql/operator/order_by_logical_operator.h"
#include "sql/operator/order_by_physical_operator.h"
#include "sql/operator/limit_logical_operator.h"
#include "sql/operator/limit_physical_operator.h"
#include "sql/operator/table_scan_vec_physical_operator.h"
#include "sql/operator/view_scan_physical_operator.h"
#include "sql/operator/vector_scan_physical_operator.h"
#include "sql/operator/union_logical_operator.h"
#include "sql/operator/union_physical_operator.h"
#include "sql/optimizer/physical_plan_generator.h"

using namespace std;

RC PhysicalPlanGenerator::create(
    LogicalOperator &logical_operator, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  RC rc = RC::SUCCESS;

  switch (logical_operator.type()) {
    case LogicalOperatorType::CALC: {
      return create_plan(static_cast<CalcLogicalOperator &>(logical_operator), oper, session);
    } break;

    case LogicalOperatorType::TABLE_GET: {
      return create_plan(static_cast<TableGetLogicalOperator &>(logical_operator), oper, session);
    } break;

    case LogicalOperatorType::PREDICATE: {
      return create_plan(static_cast<PredicateLogicalOperator &>(logical_operator), oper, session);
    } break;

    case LogicalOperatorType::PROJECTION: {
      return create_plan(static_cast<ProjectLogicalOperator &>(logical_operator), oper, session);
    } break;

    case LogicalOperatorType::INSERT: {
      return create_plan(static_cast<InsertLogicalOperator &>(logical_operator), oper, session);
    } break;

    case LogicalOperatorType::DELETE: {
      return create_plan(static_cast<DeleteLogicalOperator &>(logical_operator), oper, session);
    } break;

    case LogicalOperatorType::EXPLAIN: {
      return create_plan(static_cast<ExplainLogicalOperator &>(logical_operator), oper, session);
    } break;

    case LogicalOperatorType::JOIN: {
      return create_plan(static_cast<JoinLogicalOperator &>(logical_operator), oper, session);
    } break;

    case LogicalOperatorType::GROUP_BY: {
      return create_plan(static_cast<GroupByLogicalOperator &>(logical_operator), oper, session);
    } break;

    case LogicalOperatorType::SORT: {
      return create_plan(static_cast<SortLogicalOperator &>(logical_operator), oper, session);
    } break;

    case LogicalOperatorType::ORDER_BY: {
      return create_plan(static_cast<OrderByLogicalOperator &>(logical_operator), oper, session);
    } break;

    case LogicalOperatorType::LIMIT: {
      return create_plan(static_cast<LimitLogicalOperator &>(logical_operator), oper, session);
    } break;

    case LogicalOperatorType::UNION: {
      return create_plan(static_cast<UnionLogicalOperator &>(logical_operator), oper, session);
    } break;

    default: {
      ASSERT(false, "unknown logical operator type");
      return RC::INVALID_ARGUMENT;
    }
  }
  return rc;
}

RC PhysicalPlanGenerator::create_plan(
    UnionLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  RC rc = RC::SUCCESS;

  // 构造物理UNION算子并设置distinct_flags与结果schema
  auto union_oper = make_unique<UnionPhysicalOperator>(logical_oper.distinct_flags());
  union_oper->set_result_schema(logical_oper.result_types(), logical_oper.result_names());

  // 递归创建并挂接所有子物理算子
  for (auto &child_logical : logical_oper.children()) {
    unique_ptr<PhysicalOperator> child_physical;
    rc = create(*child_logical, child_physical, session);
    if (OB_FAIL(rc)) {
      LOG_WARN("failed to create child physical operator for UNION: %s", strrc(rc));
      return rc;
    }
    union_oper->add_child(std::move(child_physical));
  }

  oper = std::move(union_oper);
  return rc;
}

RC PhysicalPlanGenerator::create_plan(
    SortLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  RC rc = RC::SUCCESS;

  vector<unique_ptr<Expression>> order_by_expressions;
  for (auto &expr : logical_oper.order_by_expressions()) {
    order_by_expressions.emplace_back(expr->copy());
  }

  auto sort_oper =
      make_unique<SortPhysicalOperator>(std::move(order_by_expressions), logical_oper.order_by_directions());

  ASSERT(logical_oper.children().size() == 1, "sort operator should have 1 child");

  LogicalOperator             &child_oper = *logical_oper.children().front();
  unique_ptr<PhysicalOperator> child_physical_oper;
  rc = create(child_oper, child_physical_oper, session);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to create child physical operator of sort operator. rc=%s", strrc(rc));
    return rc;
  }

  sort_oper->add_child(std::move(child_physical_oper));

  oper = std::move(sort_oper);
  return rc;
}

RC PhysicalPlanGenerator::create_plan(
    OrderByLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  RC rc = RC::SUCCESS;

  vector<unique_ptr<Expression>> order_by_expressions;
  for (auto &expr : logical_oper.order_by_expressions()) {
    order_by_expressions.emplace_back(expr->copy());
  }

  auto order_by_oper =
      make_unique<OrderByPhysicalOperator>(std::move(order_by_expressions), logical_oper.order_by_directions());

  ASSERT(logical_oper.children().size() == 1, "order by operator should have 1 child");

  LogicalOperator             &child_oper = *logical_oper.children().front();
  unique_ptr<PhysicalOperator> child_physical_oper;
  rc = create(child_oper, child_physical_oper, session);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to create child physical operator of order by operator. rc=%s", strrc(rc));
    return rc;
  }

  order_by_oper->add_child(std::move(child_physical_oper));

  oper = std::move(order_by_oper);
  return rc;
}

RC PhysicalPlanGenerator::create_plan(
    LimitLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  RC rc = RC::SUCCESS;

  // Fast path: LIMIT -> ORDER_BY(distance ASC) -> TABLE_GET(table)
  // Try to replace ORDER_BY + TABLE_GET with VECTOR_INDEX_SCAN when a vector index exists
  if (logical_oper.children().size() == 1 && logical_oper.children().front()->type() == LogicalOperatorType::ORDER_BY) {
    auto &order_by_logical = static_cast<OrderByLogicalOperator &>(*logical_oper.children().front());
    auto &ob_exprs         = order_by_logical.order_by_expressions();
    auto &ob_dirs          = order_by_logical.order_by_directions();
    if (ob_exprs.size() == 1 && ob_dirs.size() == 1 && ob_dirs[0] == ASC_ORDER) {
      // pattern check: single distance function
      Expression *expr = ob_exprs[0].get();
      if (expr->type() == ExprType::FUNCTION) {
        auto *func = dynamic_cast<FunctionExpr *>(expr);
        if (func != nullptr) {
          std::string fname = func->function_name();
          for (auto &c : fname)
            c = toupper(c);
          if (fname == "L2_DISTANCE" || fname == "COSINE_DISTANCE" || fname == "INNER_PRODUCT") {
            auto &scan_children = order_by_logical.children();
            if (scan_children.size() == 1 && scan_children[0]->type() == LogicalOperatorType::TABLE_GET) {
              auto  *table_get = static_cast<TableGetLogicalOperator *>(scan_children[0].get());
              Table *table     = table_get->table();
              if (table != nullptr && func->args().size() == 2) {
                FieldExpr *field_expr = nullptr;
                RowTuple   dummy;
                Value      literal;
                if (func->args()[0]->type() == ExprType::FIELD) {
                  field_expr = dynamic_cast<FieldExpr *>(func->args()[0].get());
                  (void)func->args()[1]->get_value(dummy, literal);
                } else if (func->args()[1]->type() == ExprType::FIELD) {
                  field_expr = dynamic_cast<FieldExpr *>(func->args()[1].get());
                  (void)func->args()[0]->get_value(dummy, literal);
                }

                if (field_expr != nullptr) {
                  Index *index = table->find_index_by_field(field_expr->field().field_name());
                  if (index != nullptr && index->is_vector_index()) {
                    // Extract base vector from Value (binary layout: [int dim][float x dim])
                    std::vector<float> base_vector;
                    if (literal.attr_type() == AttrType::VECTORS && literal.data() != nullptr) {
                      const char  *data  = literal.data();
                      int          dim   = *(int *)data;
                      const float *fdata = (const float *)(data + sizeof(int));
                      if (dim > 0 && dim < 20000) {
                        base_vector.assign(fdata, fdata + dim);
                      }
                    }

                    if (!base_vector.empty()) {
                      // Build a top-level VECTOR_INDEX_SCAN only. The operator enforces top-N internally.
                      auto vector_scan = std::make_unique<VectorScanPhysicalOperator>(table,
                          std::string(),
                          index,
                          base_vector,
                          static_cast<size_t>(logical_oper.limit_count()),
                          ReadWriteMode::READ_ONLY);
                      oper             = std::move(vector_scan);
                      return RC::SUCCESS;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // Fallback: build regular LIMIT over its child
  auto limit_oper = make_unique<LimitPhysicalOperator>(logical_oper.limit_count());

  ASSERT(logical_oper.children().size() == 1, "limit operator should have 1 child");

  LogicalOperator             &child_oper = *logical_oper.children().front();
  unique_ptr<PhysicalOperator> child_physical_oper;
  rc = create(child_oper, child_physical_oper, session);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to create child physical operator of limit operator. rc=%s", strrc(rc));
    return rc;
  }

  limit_oper->add_child(std::move(child_physical_oper));

  oper = std::move(limit_oper);
  return rc;
}

RC PhysicalPlanGenerator::create_vec(
    LogicalOperator &logical_operator, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  RC rc = RC::SUCCESS;

  switch (logical_operator.type()) {
    case LogicalOperatorType::TABLE_GET: {
      return create_vec_plan(static_cast<TableGetLogicalOperator &>(logical_operator), oper, session);
    } break;
    case LogicalOperatorType::PROJECTION: {
      return create_vec_plan(static_cast<ProjectLogicalOperator &>(logical_operator), oper, session);
    } break;
    case LogicalOperatorType::GROUP_BY: {
      return create_vec_plan(static_cast<GroupByLogicalOperator &>(logical_operator), oper, session);
    } break;
    case LogicalOperatorType::EXPLAIN: {
      return create_vec_plan(static_cast<ExplainLogicalOperator &>(logical_operator), oper, session);
    } break;
    default: {
      return RC::INVALID_ARGUMENT;
    }
  }
  return rc;
}

RC PhysicalPlanGenerator::create_plan(
    TableGetLogicalOperator &table_get_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  vector<unique_ptr<Expression>> &predicates = table_get_oper.predicates();
  // 看看是否有可以用于索引查找的表达式
  Table *table = table_get_oper.table();

  // 检查是否为视图
  if (table->is_view()) {
    LOG_TRACE("creating plan for view: %s", table->name());

    unique_ptr<TableScanPhysicalOperator> table_scan_oper(
        new TableScanPhysicalOperator(table, ReadWriteMode::READ_ONLY));

    // 添加谓词过滤
    if (!predicates.empty()) {
      // 对于多个谓词，需要将它们组合成一个AND表达式
      // 暂时简化：只使用第一个谓词
      unique_ptr<PredicatePhysicalOperator> predicate_oper(new PredicatePhysicalOperator(std::move(predicates[0])));
      predicate_oper->add_child(std::move(table_scan_oper));
      oper = std::move(predicate_oper);
    } else {
      oper = std::move(table_scan_oper);
    }

    // 对于视图，需要解析并执行视图的SELECT语句
    return create_plan_for_view(table, predicates, oper, session);
  }

  Index     *index      = nullptr;
  ValueExpr *value_expr = nullptr;
  for (auto &expr : predicates) {
    if (expr->type() == ExprType::COMPARISON) {
      auto comparison_expr = static_cast<ComparisonExpr *>(expr.get());
      // 简单处理，就找等值查询
      if (comparison_expr->comp() != EQUAL_TO) {
        continue;
      }

      unique_ptr<Expression> &left_expr  = comparison_expr->left();
      unique_ptr<Expression> &right_expr = comparison_expr->right();
      // 左右比较的一边最少是一个值
      if (left_expr->type() != ExprType::VALUE && right_expr->type() != ExprType::VALUE) {
        continue;
      }

      FieldExpr *field_expr = nullptr;
      if (left_expr->type() == ExprType::FIELD) {
        ASSERT(right_expr->type() == ExprType::VALUE, "right expr should be a value expr while left is field expr");
        field_expr = static_cast<FieldExpr *>(left_expr.get());
        value_expr = static_cast<ValueExpr *>(right_expr.get());
      } else if (right_expr->type() == ExprType::FIELD) {
        ASSERT(left_expr->type() == ExprType::VALUE, "left expr should be a value expr while right is a field expr");
        field_expr = static_cast<FieldExpr *>(right_expr.get());
        value_expr = static_cast<ValueExpr *>(left_expr.get());
      }

      if (field_expr == nullptr) {
        continue;
      }

      const Field &field = field_expr->field();
      index              = table->find_index_by_field(field.field_name());
      if (nullptr != index) {
        break;
      }
    }
  }

  if (index != nullptr) {
    ASSERT(value_expr != nullptr, "got an index but value expr is null ?");

    const Value               &value           = value_expr->get_value();
    IndexScanPhysicalOperator *index_scan_oper = new IndexScanPhysicalOperator(table,
        index,
        table_get_oper.read_write_mode(),
        &value,
        true /*left_inclusive*/,
        &value,
        true /*right_inclusive*/);

    index_scan_oper->set_predicates(std::move(predicates));
    oper = unique_ptr<PhysicalOperator>(index_scan_oper);
    LOG_TRACE("use index scan");
  } else {
    auto table_scan_oper = new TableScanPhysicalOperator(table, table_get_oper.read_write_mode());
    table_scan_oper->set_predicates(std::move(predicates));
    oper = unique_ptr<PhysicalOperator>(table_scan_oper);
    LOG_TRACE("use table scan");
  }

  return RC::SUCCESS;
}

RC PhysicalPlanGenerator::create_plan(
    PredicateLogicalOperator &pred_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  vector<unique_ptr<LogicalOperator>> &children_opers = pred_oper.children();
  ASSERT(children_opers.size() == 1, "predicate logical operator's sub oper number should be 1");

  LogicalOperator &child_oper = *children_opers.front();

  unique_ptr<PhysicalOperator> child_phy_oper;
  RC                           rc = create(child_oper, child_phy_oper, session);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create child operator of predicate operator. rc=%s", strrc(rc));
    return rc;
  }

  vector<unique_ptr<Expression>> &expressions = pred_oper.expressions();
  ASSERT(expressions.size() == 1, "predicate logical operator's children should be 1");

  //上下内容不变，但是要在中间去构造出来子查询的物理算子，这样可以支持子查询多重嵌套
  do {
    auto               &&expr = expressions.front();
    vector<Expression *> exprs;

    //因为rewrite会重写优化。
    //这里有可能是CONJUNCTION，也有可能是COMPARE

    if (expr->type() == ExprType::CONJUNCTION) {
      auto &&con_expr = dynamic_cast<ConjunctionExpr *>(expr.get());
      auto &&childs   = con_expr->children();
      for (auto &&child : childs) {
        if (child->type() == ExprType::COMPARISON) {
          exprs.push_back(child.get());
        }
      }
    } else if (expr->type() == ExprType::COMPARISON) {
      exprs.push_back(expr.get());
    } else {
      break;
    }

    for (auto &one : exprs) {
      if (one->type() != ExprType::COMPARISON) {
        continue;
      }
      auto                  &&expr  = dynamic_cast<ComparisonExpr *>(one);
      unique_ptr<Expression> &left  = expr->left();
      unique_ptr<Expression> &right = expr->right();

      auto set_subquery = [&](unique_ptr<Expression> &expr) {
        ASSERT(expr->type() == ExprType::SUBQUERY, "py gen error in predicate");
        auto &&sub_expr = static_cast<SubQueryExpr *>(expr.get());
        if (sub_expr->physical_operator() != nullptr) {
          LOG_WARN("[UNEXPECTED] subquery physical operator is not null!");
        }
        unique_ptr<PhysicalOperator> py_oper;
        RC                           rc = create(*sub_expr->logical_operator(), py_oper, session);
        CHECK_RETURN_RC(rc);
        sub_expr->set_py_operator(std::move(py_oper));
        return rc;
      };

      if (left and left->type() == ExprType::SUBQUERY) {
        rc = set_subquery(left);
        CHECK_RETURN_RC(rc);
      }
      if (right and right->type() == ExprType::SUBQUERY) {
        rc = set_subquery(right);
        CHECK_RETURN_RC(rc);
      }
    }

  } while (0);

  unique_ptr<Expression> expression = std::move(expressions.front());
  oper = unique_ptr<PhysicalOperator>(new PredicatePhysicalOperator(std::move(expression)));
  oper->add_child(std::move(child_phy_oper));
  return rc;
}

RC PhysicalPlanGenerator::create_plan(
    ProjectLogicalOperator &project_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  vector<unique_ptr<LogicalOperator>> &child_opers = project_oper.children();

  unique_ptr<PhysicalOperator> child_phy_oper;

  RC rc = RC::SUCCESS;
  if (!child_opers.empty()) {
    LogicalOperator *child_oper = child_opers.front().get();

    rc = create(*child_oper, child_phy_oper, session);
    if (OB_FAIL(rc)) {
      LOG_WARN("failed to create project logical operator's child physical operator. rc=%s", strrc(rc));
      return rc;
    }
  }

  auto project_operator = make_unique<ProjectPhysicalOperator>(std::move(project_oper.expressions()));
  if (child_phy_oper) {
    project_operator->add_child(std::move(child_phy_oper));
  }

  oper = std::move(project_operator);

  LOG_TRACE("create a project physical operator");
  return rc;
}

RC PhysicalPlanGenerator::create_plan(
    InsertLogicalOperator &insert_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  Table                  *table           = insert_oper.table();
  vector<Value>          &values          = insert_oper.values();
  InsertPhysicalOperator *insert_phy_oper = new InsertPhysicalOperator(table, std::move(values));
  oper.reset(insert_phy_oper);
  return RC::SUCCESS;
}

RC PhysicalPlanGenerator::create_plan(
    DeleteLogicalOperator &delete_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  vector<unique_ptr<LogicalOperator>> &child_opers = delete_oper.children();

  unique_ptr<PhysicalOperator> child_physical_oper;

  RC rc = RC::SUCCESS;
  if (!child_opers.empty()) {
    LogicalOperator *child_oper = child_opers.front().get();

    rc = create(*child_oper, child_physical_oper, session);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create physical operator. rc=%s", strrc(rc));
      return rc;
    }
  }

  oper = unique_ptr<PhysicalOperator>(new DeletePhysicalOperator(delete_oper.table()));

  if (child_physical_oper) {
    oper->add_child(std::move(child_physical_oper));
  }
  return rc;
}

RC PhysicalPlanGenerator::create_plan(
    ExplainLogicalOperator &explain_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  vector<unique_ptr<LogicalOperator>> &child_opers = explain_oper.children();

  RC rc = RC::SUCCESS;

  unique_ptr<PhysicalOperator> explain_physical_oper(new ExplainPhysicalOperator);
  for (unique_ptr<LogicalOperator> &child_oper : child_opers) {
    unique_ptr<PhysicalOperator> child_physical_oper;
    rc = create(*child_oper, child_physical_oper, session);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create child physical operator. rc=%s", strrc(rc));
      return rc;
    }

    explain_physical_oper->add_child(std::move(child_physical_oper));
  }

  oper = std::move(explain_physical_oper);
  return rc;
}

RC PhysicalPlanGenerator::create_plan(
    JoinLogicalOperator &join_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  RC rc = RC::SUCCESS;

  vector<unique_ptr<LogicalOperator>> &child_opers = join_oper.children();
  if (child_opers.size() != 2) {
    LOG_WARN("join operator should have 2 children, but have %d", child_opers.size());
    return RC::INTERNAL;
  }
  if (session->hash_join_on() && can_use_hash_join(join_oper)) {
    // your code here
  } else {
    unique_ptr<PhysicalOperator> join_physical_oper(new NestedLoopJoinPhysicalOperator());
    for (auto &child_oper : child_opers) {
      unique_ptr<PhysicalOperator> child_physical_oper;
      rc = create(*child_oper, child_physical_oper, session);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to create physical child oper. rc=%s", strrc(rc));
        return rc;
      }

      join_physical_oper->add_child(std::move(child_physical_oper));
    }

    oper = std::move(join_physical_oper);
  }
  return rc;
}

bool PhysicalPlanGenerator::can_use_hash_join(JoinLogicalOperator &join_oper)
{
  // your code here
  return false;
}

RC PhysicalPlanGenerator::create_plan(
    CalcLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  RC rc = RC::SUCCESS;

  CalcPhysicalOperator *calc_oper = new CalcPhysicalOperator(std::move(logical_oper.expressions()));
  oper.reset(calc_oper);
  return rc;
}

RC PhysicalPlanGenerator::create_plan(
    GroupByLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  RC rc = RC::SUCCESS;

  vector<unique_ptr<Expression>>     &group_by_expressions = logical_oper.group_by_expressions();
  unique_ptr<GroupByPhysicalOperator> group_by_oper;
  if (group_by_expressions.empty()) {
    group_by_oper = make_unique<ScalarGroupByPhysicalOperator>(std::move(logical_oper.aggregate_expressions()));
  } else {
    group_by_oper = make_unique<HashGroupByPhysicalOperator>(
        std::move(logical_oper.group_by_expressions()), std::move(logical_oper.aggregate_expressions()));
  }

  ASSERT(logical_oper.children().size() == 1, "group by operator should have 1 child");

  LogicalOperator             &child_oper = *logical_oper.children().front();
  unique_ptr<PhysicalOperator> child_physical_oper;
  rc = create(child_oper, child_physical_oper, session);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to create child physical operator of group by operator. rc=%s", strrc(rc));
    return rc;
  }

  group_by_oper->add_child(std::move(child_physical_oper));

  oper = std::move(group_by_oper);
  return rc;
}

RC PhysicalPlanGenerator::create_vec_plan(
    TableGetLogicalOperator &table_get_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  vector<unique_ptr<Expression>> &predicates = table_get_oper.predicates();
  Table                          *table      = table_get_oper.table();
  TableScanVecPhysicalOperator   *table_scan_oper =
      new TableScanVecPhysicalOperator(table, table_get_oper.read_write_mode());
  table_scan_oper->set_predicates(std::move(predicates));
  oper = unique_ptr<PhysicalOperator>(table_scan_oper);
  LOG_TRACE("use vectorized table scan");

  return RC::SUCCESS;
}

RC PhysicalPlanGenerator::create_vec_plan(
    GroupByLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  RC                           rc            = RC::SUCCESS;
  unique_ptr<PhysicalOperator> physical_oper = nullptr;
  if (logical_oper.group_by_expressions().empty()) {
    physical_oper = make_unique<AggregateVecPhysicalOperator>(std::move(logical_oper.aggregate_expressions()));
  } else {
    physical_oper = make_unique<GroupByVecPhysicalOperator>(
        std::move(logical_oper.group_by_expressions()), std::move(logical_oper.aggregate_expressions()));
  }

  ASSERT(logical_oper.children().size() == 1, "group by operator should have 1 child");

  LogicalOperator             &child_oper = *logical_oper.children().front();
  unique_ptr<PhysicalOperator> child_physical_oper;
  rc = create_vec(child_oper, child_physical_oper, session);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to create child physical operator of group by(vec) operator. rc=%s", strrc(rc));
    return rc;
  }

  physical_oper->add_child(std::move(child_physical_oper));

  oper = std::move(physical_oper);
  return rc;

  return RC::SUCCESS;
}

RC PhysicalPlanGenerator::create_vec_plan(
    ProjectLogicalOperator &project_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  vector<unique_ptr<LogicalOperator>> &child_opers = project_oper.children();

  unique_ptr<PhysicalOperator> child_phy_oper;

  RC rc = RC::SUCCESS;
  if (!child_opers.empty()) {
    LogicalOperator *child_oper = child_opers.front().get();
    rc                          = create_vec(*child_oper, child_phy_oper, session);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create project logical operator's child physical operator. rc=%s", strrc(rc));
      return rc;
    }
  }

  auto project_operator = make_unique<ProjectVecPhysicalOperator>(std::move(project_oper.expressions()));

  if (child_phy_oper != nullptr) {
    vector<Expression *> expressions;
    for (auto &expr : project_operator->expressions()) {
      expressions.push_back(expr.get());
    }
    auto expr_operator = make_unique<ExprVecPhysicalOperator>(std::move(expressions));
    expr_operator->add_child(std::move(child_phy_oper));
    project_operator->add_child(std::move(expr_operator));
  }

  oper = std::move(project_operator);

  LOG_TRACE("create a project physical operator");
  return rc;
}

RC PhysicalPlanGenerator::create_vec_plan(
    ExplainLogicalOperator &explain_oper, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  vector<unique_ptr<LogicalOperator>> &child_opers = explain_oper.children();

  RC rc = RC::SUCCESS;
  // reuse `ExplainPhysicalOperator` in explain vectorized physical plan
  unique_ptr<PhysicalOperator> explain_physical_oper(new ExplainPhysicalOperator);
  for (unique_ptr<LogicalOperator> &child_oper : child_opers) {
    unique_ptr<PhysicalOperator> child_physical_oper;
    rc = create_vec(*child_oper, child_physical_oper, session);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create child physical operator. rc=%s", strrc(rc));
      return rc;
    }

    explain_physical_oper->add_child(std::move(child_physical_oper));
  }

  oper = std::move(explain_physical_oper);
  return rc;
}

RC PhysicalPlanGenerator::create_plan_for_view(
    Table *table, vector<unique_ptr<Expression>> &predicates, unique_ptr<PhysicalOperator> &oper, Session *session)
{
  LOG_TRACE("Creating physical plan for view: %s", table->name());

  // 获取视图的SELECT SQL
  const string &select_sql = table->select_sql();
  if (select_sql.empty()) {
    LOG_ERROR("View %s has empty SELECT SQL", table->name());
    return RC::INTERNAL;
  }

  LOG_DEBUG("View %s SELECT SQL: %s", table->name(), select_sql.c_str());

  try {
    // 解析视图的SELECT语句
    ParsedSqlResult parsed_sql_result;
    RC              rc = parse(select_sql.c_str(), &parsed_sql_result);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to parse view SQL for view %s: %s", table->name(), select_sql.c_str());
      return rc;
    }

    // 检查解析结果
    if (parsed_sql_result.sql_nodes().empty() || parsed_sql_result.sql_nodes()[0]->flag != SCF_SELECT) {
      LOG_ERROR("View SQL is not a SELECT statement for view %s", table->name());
      return RC::SQL_SYNTAX;
    }

    // 获取SELECT节点
    ParsedSqlNode *parsed_node = parsed_sql_result.sql_nodes()[0].get();
    SelectSqlNode &select_node = parsed_node->selection;

    // 创建SelectStmt
    Stmt *select_stmt = nullptr;
    rc                = SelectStmt::create(session->get_current_db(), select_node, select_stmt);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to create SelectStmt for view %s", table->name());
      return rc;
    }

    // 创建逻辑计划
    LogicalPlanGenerator        logical_plan_generator;
    unique_ptr<LogicalOperator> logical_operator;
    rc = logical_plan_generator.create(select_stmt, logical_operator);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to create logical plan for view %s", table->name());
      delete select_stmt;
      return rc;
    }

    // 创建物理计划
    rc = create(*logical_operator, oper, session);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to create physical plan for view %s", table->name());
      delete select_stmt;
      return rc;
    }

    // 如果有额外的谓词，需要添加谓词过滤
    if (!predicates.empty()) {
      LOG_DEBUG("Adding predicates to view query for view %s", table->name());
      // 为了简化，暂时只支持第一个谓词
      // 在完整实现中，应该将所有谓词组合成AND表达式
      unique_ptr<PredicatePhysicalOperator> predicate_oper(new PredicatePhysicalOperator(std::move(predicates[0])));
      predicate_oper->add_child(std::move(oper));
      oper = std::move(predicate_oper);
    }

    delete select_stmt;
    LOG_INFO("Successfully created physical plan for view: %s", table->name());
    return RC::SUCCESS;

  } catch (const std::exception &e) {
    LOG_ERROR("Exception while creating plan for view %s: %s", table->name(), e.what());
    return RC::INTERNAL;
  }
}
