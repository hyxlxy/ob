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
// Created by AI Assistant on 2024/12/19.
//

#pragma once

#include "sql/operator/physical_operator.h"
#include "sql/expr/tuple.h"
#include "sql/expr/expression.h"

/**
 * @brief UNION物理算子
 * @ingroup PhysicalOperator
 * @details UNION算子用于合并多个SELECT语句的结果集
 * 支持UNION（去重）和UNION ALL（不去重）两种模式
 */
class UnionPhysicalOperator : public PhysicalOperator
{
public:
  UnionPhysicalOperator(const vector<bool> &distinct_flags = {});
  
  virtual ~UnionPhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::UNION; }
  OpType get_op_type() const override { return OpType::UNION; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override;
  RC tuple_schema(TupleSchema &schema) const override;

  // 设置去重标志位
  void set_distinct_flags(const vector<bool> &distinct_flags) { distinct_flags_ = distinct_flags; }
  const vector<bool> &distinct_flags() const { return distinct_flags_; }

  // 设置结果集的列类型和列名（由resolve阶段确定）
  void set_result_schema(const vector<AttrType> &types, const vector<string> &names);

private:
  // 从当前子算子获取下一行数据
  RC fetch_next_from_child();
  
  // 检查当前行是否已经存在（用于去重）
  bool is_duplicate(const Tuple &tuple);
  
  // 将tuple转换为用于去重比较的key
  string tuple_to_key(const Tuple &tuple);
  
  // 批量处理所有子查询
  RC process_all_children();
  
  // 收集单个子查询的所有结果
  RC collect_child_results(size_t child_index, vector<unique_ptr<Tuple>> &results);
  
  // 对左边结果进行内部去重
  void deduplicate_left_results();

private:
  vector<bool> distinct_flags_;     ///< 每个UNION操作的标志位，true表示去重，false表示不去重
  
  // 批量处理相关
  vector<unique_ptr<Tuple>> final_results_;  ///< 最终结果集
  size_t current_result_index_;              ///< 当前输出位置
  bool all_processed_;                       ///< 是否已处理完毕
  
  // 结果集schema信息
  vector<AttrType> result_types_;    ///< 结果集列类型
  vector<string> result_names_;      ///< 结果集列名
  
  // 去重相关
  unordered_set<string> seen_tuples_; ///< 已见过的tuple集合（用于去重）
  
  // 当前tuple
  unique_ptr<Tuple> current_tuple_;   ///< 当前输出的tuple

  // 为当前输出tuple持久化保存表达式容器，避免悬空引用
  vector<unique_ptr<Expression>> current_expressions_;
};
