/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "sql/operator/group_by_vec_physical_operator.h"
#include "common/log/log.h"
#include "storage/common/chunk.h"
#include "storage/common/column.h"
#include "sql/expr/expression.h"

GroupByVecPhysicalOperator::GroupByVecPhysicalOperator(
    vector<unique_ptr<Expression>> &&group_by_exprs, vector<Expression *> &&expressions)
    : group_by_expressions_(std::move(group_by_exprs)), aggregate_expressions_(std::move(expressions))
{
  // 从聚合表达式中提取用于计算值的表达式
  for (Expression *expr : aggregate_expressions_) {
    if (expr->type() == ExprType::AGGREGATION) {
      auto *aggregate_expr = static_cast<AggregateExpr *>(expr);
      if (aggregate_expr->child()) {
        value_expressions_.push_back(aggregate_expr->child().get());
      } else {
        // 对于COUNT(*)，没有子表达式，创建一个常量1的表达式
        value_expressions_.push_back(nullptr);
      }
    }
  }
}

RC GroupByVecPhysicalOperator::open(Trx *trx)
{
  ASSERT(children_.size() == 1, "group by operator only support one child, but got %d", children_.size());

  RC rc = children_[0]->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open child operator. rc=%s", strrc(rc));
    return rc;
  }

  // 创建聚合哈希表
  hash_table_ = make_unique<StandardAggregateHashTable>(aggregate_expressions_);

  // 处理所有输入数据
  PhysicalOperator &child = *children_[0];
  while (OB_SUCC(rc = child.next(input_chunk_))) {
    // 重置chunk数据
    group_chunk_.reset_data();
    aggr_chunk_.reset_data();

    // 计算GROUP BY表达式的值
    for (size_t i = 0; i < group_by_expressions_.size(); i++) {
      auto column = make_unique<Column>();
      group_by_expressions_[i]->get_column(input_chunk_, *column);
      group_chunk_.add_column(std::move(column), i);
    }

    // 计算聚合表达式的值
    for (size_t i = 0; i < value_expressions_.size(); i++) {
      auto column = make_unique<Column>();
      if (value_expressions_[i] != nullptr) {
        value_expressions_[i]->get_column(input_chunk_, *column);
      } else {
        // 对于COUNT(*)，创建一个全为1的列
        column->init(AttrType::INTS, sizeof(int), input_chunk_.rows());
        for (int row = 0; row < input_chunk_.rows(); row++) {
          int one = 1;
          column->append_one((char *)&one);
        }
      }
      aggr_chunk_.add_column(std::move(column), i);
    }

    // 将数据添加到哈希表
    rc = hash_table_->add_chunk(group_chunk_, aggr_chunk_);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to add chunk to hash table. rc=%s", strrc(rc));
      return rc;
    }
  }

  if (rc == RC::RECORD_EOF) {
    rc = RC::SUCCESS;
  }

  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to process input data. rc=%s", strrc(rc));
    return rc;
  }

  // 创建扫描器
  scanner_ = make_unique<StandardAggregateHashTable::Scanner>(hash_table_.get());
  scanner_->open_scan();

  opened_ = true;
  return RC::SUCCESS;
}

RC GroupByVecPhysicalOperator::next(Chunk &chunk)
{
  if (!opened_) {
    return RC::RECORD_EOF;
  }

  // 创建输出chunk的列结构
  if (chunk.column_num() == 0) {
    // 添加GROUP BY列
    for (size_t i = 0; i < group_by_expressions_.size(); i++) {
      AttrType attr_type = group_by_expressions_[i]->value_type();
      int attr_len = group_by_expressions_[i]->value_length();
      auto column = make_unique<Column>(attr_type, attr_len);
      chunk.add_column(std::move(column), i);
    }

    // 添加聚合列
    for (size_t i = 0; i < aggregate_expressions_.size(); i++) {
      AttrType attr_type = aggregate_expressions_[i]->value_type();
      int attr_len = aggregate_expressions_[i]->value_length();
      auto column = make_unique<Column>(attr_type, attr_len);
      chunk.add_column(std::move(column), group_by_expressions_.size() + i);
    }
  }

  // 从扫描器获取下一批结果
  RC rc = scanner_->next(chunk);
  return rc;
}

RC GroupByVecPhysicalOperator::close()
{
  if (scanner_) {
    scanner_->close_scan();
    scanner_.reset();
  }

  hash_table_.reset();

  if (!children_.empty()) {
    children_[0]->close();
  }

  opened_ = false;
  LOG_INFO("close group by vec operator");
  return RC::SUCCESS;
}