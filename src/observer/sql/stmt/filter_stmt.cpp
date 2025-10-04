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
// Created by Wangyunlai on 2022/5/22.
//

#include "sql/stmt/filter_stmt.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "common/sys/rc.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "sql/stmt/select_stmt.h"

FilterStmt::~FilterStmt()
{
  for (FilterUnit *unit : filter_units_) {
    delete unit;
  }
  filter_units_.clear();
  
  if (condition_tree_) {
    delete condition_tree_;
    condition_tree_ = nullptr;
  }
}

RC FilterStmt::create(Db *db, Table *default_table, unordered_map<string, Table *> *tables,
    const ConditionSqlNode *conditions, int condition_num, FilterStmt *&stmt)
{
  RC rc = RC::SUCCESS;
  stmt  = nullptr;

  FilterStmt *tmp_stmt = new FilterStmt();
  
  // 创建条件树
  printf("DEBUG: [FILTER_STMT] About to create condition tree\n");
  rc = create_condition_tree(db, default_table, tables, conditions, condition_num, tmp_stmt->condition_tree_);
  if (rc != RC::SUCCESS) {
    printf("DEBUG: [FILTER_STMT] create_condition_tree failed with rc=%s\n", strrc(rc));
    delete tmp_stmt;
    LOG_WARN("failed to create condition tree. condition_num=%d", condition_num);
    return rc;
  }
  printf("DEBUG: [FILTER_STMT] create_condition_tree succeeded\n");

  // 保持向后兼容：创建传统的filter_units_
  // 注意：即使create_filter_unit失败，我们也不删除条件树，因为条件树已经创建成功
  for (int i = 0; i < condition_num; i++) {
    FilterUnit *filter_unit = nullptr;
    RC filter_unit_rc = create_filter_unit(db, default_table, tables, conditions[i], filter_unit);
    if (filter_unit_rc != RC::SUCCESS) {
      LOG_WARN("failed to create filter unit. condition index=%d, rc=%s", i, strrc(filter_unit_rc));
      // 不删除tmp_stmt，因为条件树已经创建成功
      // 继续处理其他条件
      continue;
    }
    tmp_stmt->filter_units_.push_back(filter_unit);
  }

  stmt = tmp_stmt;
  return rc;
}

RC FilterStmt::create_condition_tree(Db *db, Table *default_table, unordered_map<string, Table *> *tables,
    const ConditionSqlNode *conditions, int condition_num, FilterConditionNode *&tree)
{
  if (condition_num == 0) {
    tree = nullptr;
    return RC::SUCCESS;
  }

  printf("DEBUG: [FILTER_STMT] Creating condition tree with %d conditions\n", condition_num);
  for (int i = 0; i < condition_num; i++) {
    printf("DEBUG: [FILTER_STMT] Condition %d: conjunction_op = %d\n", i, conditions[i].conjunction_op);
  }

  if (condition_num == 1) {
    // 单个条件
    tree = new FilterConditionNode(FilterConditionNode::CONDITION);
    RC rc = create_filter_unit(db, default_table, tables, conditions[0], tree->filter_unit);
    if (rc != RC::SUCCESS) {
      delete tree;
      tree = nullptr;
      return rc;
    }
    return RC::SUCCESS;
  }

  // 多个条件：需要根据conjunction_op来分组
  // 首先检查是否有OR条件
  bool has_or = false;
  for (int i = 0; i < condition_num; i++) {
    if (conditions[i].conjunction_op == OR_OP) {
      has_or = true;
      break;
    }
  }

  if (has_or) {
    // 有OR条件：创建OR组
    tree = new FilterConditionNode(FilterConditionNode::OR_GROUP);
    
    // 将条件按OR分组
    vector<vector<ConditionSqlNode*>> or_groups;
    vector<ConditionSqlNode*> current_group;
    
    for (int i = 0; i < condition_num; i++) {
      current_group.push_back(const_cast<ConditionSqlNode*>(&conditions[i]));
      
      // 如果下一个条件是OR，或者这是最后一个条件
      if (i == condition_num - 1 || conditions[i + 1].conjunction_op == OR_OP) {
        or_groups.push_back(current_group);
        current_group.clear();
      }
    }
    
    // 为每个OR组创建子节点
    for (const auto &group : or_groups) {
      if (group.size() == 1) {
        // 单个条件，直接创建CONDITION节点
        FilterConditionNode *child_tree = new FilterConditionNode(FilterConditionNode::CONDITION);
        RC rc = create_filter_unit(db, default_table, tables, *group[0], child_tree->filter_unit);
        if (rc != RC::SUCCESS) {
          delete child_tree;
          delete tree;
          tree = nullptr;
          return rc;
        }
        tree->children.push_back(child_tree);
      } else {
        // 多个条件，递归创建AND组（因为一个OR组内部是AND关系）
        FilterConditionNode *child_tree = new FilterConditionNode(FilterConditionNode::AND_GROUP);
        for (ConditionSqlNode* condition : group) {
          FilterConditionNode *and_child = new FilterConditionNode(FilterConditionNode::CONDITION);
          RC rc = create_filter_unit(db, default_table, tables, *condition, and_child->filter_unit);
          if (rc != RC::SUCCESS) {
            delete and_child;
            delete child_tree;
            delete tree;
            tree = nullptr;
            return rc;
          }
          child_tree->children.push_back(and_child);
        }
        tree->children.push_back(child_tree);
      }
    }
  } else {
    // 没有OR条件：创建AND组
    tree = new FilterConditionNode(FilterConditionNode::AND_GROUP);
    
    for (int i = 0; i < condition_num; i++) {
      FilterConditionNode *child_tree = nullptr;
      RC rc = create_condition_tree(db, default_table, tables, &conditions[i], 1, child_tree);
      if (rc != RC::SUCCESS) {
        delete tree;
        tree = nullptr;
        return rc;
      }
      tree->children.push_back(child_tree);
    }
  }

  return RC::SUCCESS;
}

RC get_table_and_field(Db *db, Table *default_table, unordered_map<string, Table *> *tables,
    const RelAttrSqlNode &attr, Table *&table, const FieldMeta *&field)
{
  if (common::is_blank(attr.relation_name.c_str())) {
    table = default_table;
  } else if (nullptr != tables) {
    auto iter = tables->find(attr.relation_name);
    if (iter != tables->end()) {
      table = iter->second;
    }
  } else {
    table = db->find_table(attr.relation_name.c_str());
  }
  if (nullptr == table) {
    LOG_WARN("No such table: attr.relation_name: %s", attr.relation_name.c_str());
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  field = table->table_meta().field(attr.attribute_name.c_str());
  if (nullptr == field) {
    LOG_WARN("no such field in table: table %s, field %s", table->name(), attr.attribute_name.c_str());
    table = nullptr;
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }

  return RC::SUCCESS;
}

RC FilterStmt::create_filter_unit(Db *db, Table *default_table, unordered_map<string, Table *> *tables,
    const ConditionSqlNode &condition, FilterUnit *&filter_unit)
{
  RC rc = RC::SUCCESS;

  CompOp comp = condition.comp;
  if (comp < EQUAL_TO || comp >= NO_OP) {
    LOG_WARN("invalid compare operator : %d", comp);
    return RC::INVALID_ARGUMENT;
  }

  // 处理IN_VALUES_OP和NOT_IN_VALUES_OP操作
  if (comp == IN_VALUES_OP || comp == NOT_IN_VALUES_OP) {
    if (condition.right_values.empty()) {
      LOG_WARN("IN/NOT IN operation requires non-empty value list");
      return RC::INVALID_ARGUMENT;
    }

    // 对于IN操作，我们需要创建一个特殊的FilterUnit
    filter_unit = new FilterUnit;

    // 设置左侧表达式或属性
    if (condition.left_expr != nullptr) {
      filter_unit->set_left_expr(condition.left_expr->copy());
    } else if (condition.left_is_attr) {
      Table           *table = nullptr;
      const FieldMeta *field = nullptr;
      rc = get_table_and_field(db, default_table, tables, condition.left_attr, table, field);
      if (rc != RC::SUCCESS) {
        LOG_WARN("cannot find attr");
        delete filter_unit;
        return rc;
      }
      FilterObj filter_obj;
      filter_obj.init_attr(Field(table, field));
      filter_unit->set_left(filter_obj);
    } else {
      FilterObj filter_obj;
      filter_obj.init_value(condition.left_value);
      filter_unit->set_left(filter_obj);
    }

    filter_unit->set_comp(comp);
    filter_unit->set_right_values(condition.right_values);
    return RC::SUCCESS;
  }

  // 处理IN_OP和NOT_IN_OP操作（子查询形式）
  if (comp == IN_OP || comp == NOT_IN_OP) {
    if (condition.sub_query == nullptr) {
      LOG_WARN("IN/NOT IN operation with subquery requires sub_query");
      return RC::INVALID_ARGUMENT;
    }

    // 对于子查询IN操作，我们需要创建一个特殊的FilterUnit
    filter_unit = new FilterUnit;

    // 设置左侧表达式或属性
    if (condition.left_expr != nullptr) {
      filter_unit->set_left_expr(condition.left_expr->copy());
    } else if (condition.left_is_attr) {
      Table           *table = nullptr;
      const FieldMeta *field = nullptr;
      rc = get_table_and_field(db, default_table, tables, condition.left_attr, table, field);
      if (rc != RC::SUCCESS) {
        LOG_WARN("cannot find attr");
        delete filter_unit;
        return rc;
      }
      FilterObj filter_obj;
      filter_obj.init_attr(Field(table, field));
      filter_unit->set_left(filter_obj);
    } else {
      FilterObj filter_obj;
      filter_obj.init_value(condition.left_value);
      filter_unit->set_left(filter_obj);
    }

    // 创建子查询的 Stmt
    Stmt *sub_query_stmt = nullptr;
    if (condition.sub_query->flag != SCF_SELECT) {
      LOG_WARN("sub-query must be a SELECT statement");
      delete filter_unit;
      return RC::INVALID_ARGUMENT;
    }
    printf("DEBUG: [FILTER_STMT] About to create SelectStmt for subquery\n");
    rc = SelectStmt::create(db, condition.sub_query->selection, sub_query_stmt);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create sub-query statement. rc=%s", strrc(rc));
      printf("DEBUG: [FILTER_STMT] SelectStmt::create failed for subquery, rc=%s\n", strrc(rc));
      delete filter_unit;
      // 如果是字段不存在错误，可能是关联子查询，返回更友好的错误
      if (rc == RC::SCHEMA_TABLE_NOT_EXIST || rc == RC::SCHEMA_FIELD_MISSING || rc == RC::SCHEMA_FIELD_NOT_EXIST) {
        LOG_WARN("Subquery contains references to outer query fields (correlated subquery). Returning SQL_SYNTAX.");
        return RC::SQL_SYNTAX;
      }
      return rc;
    }

    filter_unit->set_comp(comp);
    filter_unit->set_sub_query(std::unique_ptr<Stmt>(sub_query_stmt));
    return RC::SUCCESS;
  }

  filter_unit = new FilterUnit;

  // 如果有表达式，尝试将简单场景(字段/常量)降级为属性/常量比较；否则使用表达式比较
  if (condition.left_expr != nullptr && condition.right_expr != nullptr) {
    auto lt = condition.left_expr->type();
    auto rt = condition.right_expr->type();

    auto set_attr_from_unbound = [&](const UnboundFieldExpr *ufe, FilterObj &obj) -> RC {
      Table *tbl = nullptr;
      const FieldMeta *fld = nullptr;
      RelAttrSqlNode attr;
      attr.relation_name = ufe->table_name();
      attr.attribute_name = ufe->field_name();
      RC lrc = get_table_and_field(db, default_table, tables, attr, tbl, fld);
      if (lrc != RC::SUCCESS) {
        LOG_WARN("cannot find attr: %s.%s", attr.relation_name.c_str(), attr.attribute_name.c_str());
        return lrc;
      }
      obj.init_attr(Field(tbl, fld));
      return RC::SUCCESS;
    };

    bool handled_simple = false;

    // case: field op value
    if ((lt == ExprType::UNBOUND_FIELD || lt == ExprType::FIELD) && rt == ExprType::VALUE) {
      FilterObj left_obj;
      if (lt == ExprType::UNBOUND_FIELD) {
        auto *ufe = dynamic_cast<UnboundFieldExpr *>(condition.left_expr.get());
        rc = set_attr_from_unbound(ufe, left_obj);
        if (rc != RC::SUCCESS) return rc;
      } else { // FIELD
        auto *fe = dynamic_cast<FieldExpr *>(condition.left_expr.get());
        left_obj.init_attr(fe->field());
      }
      FilterObj right_obj;
      Value     v;
      rc = condition.right_expr->try_get_value(v);
      if (rc != RC::SUCCESS) {
        // 无法在编译期获取到常量值，退回表达式路径
      } else {
        right_obj.init_value(v);
        filter_unit->set_left(left_obj);
        filter_unit->set_right(right_obj);
        handled_simple = true;
      }
    }

    // case: value op field  => swap sides and invert compare when needed
    if (!handled_simple && lt == ExprType::VALUE && (rt == ExprType::UNBOUND_FIELD || rt == ExprType::FIELD)) {
      FilterObj left_obj;
      if (rt == ExprType::UNBOUND_FIELD) {
        auto *ufe = dynamic_cast<UnboundFieldExpr *>(condition.right_expr.get());
        rc = set_attr_from_unbound(ufe, left_obj);
        if (rc != RC::SUCCESS) return rc;
      } else { // FIELD
        auto *fe = dynamic_cast<FieldExpr *>(condition.right_expr.get());
        left_obj.init_attr(fe->field());
      }
      FilterObj right_obj;
      Value     v;
      rc = condition.left_expr->try_get_value(v);
      if (rc == RC::SUCCESS) {
        right_obj.init_value(v);
        // 交换左右时，需要调整比较符号
        auto swap_comp = [&](CompOp c) {
          switch (c) {
            case LESS_THAN:   return GREAT_THAN;
            case GREAT_THAN:  return LESS_THAN;
            case LESS_EQUAL:  return GREAT_EQUAL;
            case GREAT_EQUAL: return LESS_EQUAL;
            default:          return c; // EQUAL/NOT_EQUAL/LIKE keep as-is
          }
        };
        comp = swap_comp(comp);
        filter_unit->set_left(left_obj);
        filter_unit->set_right(right_obj);
        handled_simple = true;
      }
    }

    if (!handled_simple) {
      // 使用表达式比较：尝试复制表达式并由 FilterUnit 持有
      // 注意：SubQueryExpr无法被安全复制（会返回nullptr）
      // 对于SubQueryExpr，我们需要特殊处理：创建ComparisonExpr来包装它

      unique_ptr<Expression> left_copy;
      unique_ptr<Expression> right_copy;

      // 处理左表达式
      if (condition.left_expr->type() == ExprType::SUBQUERY) {
        // SubQueryExpr不能被复制，所以我们需要创建一个ComparisonExpr来包装整个比较操作
        // 但这需要右表达式也可用，所以我们先标记这种情况
        LOG_WARN("Left expression is SubQueryExpr, need special handling");
        delete filter_unit;
        return RC::UNSUPPORTED;  // 返回不支持的操作
      }
      
      left_copy = condition.left_expr->copy();
      if (!left_copy) {
        LOG_WARN("Failed to copy left expression (type=%d)", static_cast<int>(condition.left_expr->type()));
        delete filter_unit;
        return RC::INTERNAL;
      }

      // 处理右表达式
      right_copy = condition.right_expr->copy();
      if (!right_copy) {
        LOG_WARN("Failed to copy right expression (type=%d)", static_cast<int>(condition.right_expr->type()));
        delete filter_unit;
        return RC::INTERNAL;
      }

      filter_unit->set_left_expr(std::move(left_copy));
      filter_unit->set_right_expr(std::move(right_copy));
    }
  } else {
    // 使用传统的属性和值比较
    if (condition.left_is_attr) {
      Table           *table = nullptr;
      const FieldMeta *field = nullptr;
      rc                     = get_table_and_field(db, default_table, tables, condition.left_attr, table, field);
      if (rc != RC::SUCCESS) {
        LOG_WARN("cannot find attr");
        return rc;
      }
      FilterObj filter_obj;
      filter_obj.init_attr(Field(table, field));
      filter_unit->set_left(filter_obj);
    } else {
      FilterObj filter_obj;
      filter_obj.init_value(condition.left_value);
      filter_unit->set_left(filter_obj);
    }

    if (condition.right_is_attr) {
      Table           *table = nullptr;
      const FieldMeta *field = nullptr;
      rc                     = get_table_and_field(db, default_table, tables, condition.right_attr, table, field);
      if (rc != RC::SUCCESS) {
        LOG_WARN("cannot find attr");
        return rc;
      }
      FilterObj filter_obj;
      filter_obj.init_attr(Field(table, field));
      filter_unit->set_right(filter_obj);
    } else {
      FilterObj filter_obj;
      filter_obj.init_value(condition.right_value);
      filter_unit->set_right(filter_obj);
    }
  }

  filter_unit->set_comp(comp);

  // 检查两个类型是否能够比较
  return rc;
}
