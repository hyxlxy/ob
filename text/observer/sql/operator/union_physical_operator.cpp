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

#include "sql/operator/union_physical_operator.h"
#include "common/log/log.h"
#include "sql/expr/tuple_cell.h"
#include "sql/expr/tuple.h"
#include "common/lang/string.h"
#include <unordered_set>

using namespace std;

UnionPhysicalOperator::UnionPhysicalOperator(const vector<bool> &distinct_flags)
    : distinct_flags_(distinct_flags), current_result_index_(0), all_processed_(false)
{
}

RC UnionPhysicalOperator::open(Trx *trx)
{
  if (children_.empty()) {
    LOG_WARN("UNION operator has no children");
    return RC::INVALID_ARGUMENT;
  }

  // 打开所有子算子
  for (auto &child : children_) {
    RC rc = child->open(trx);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to open child operator: %s", strrc(rc));
      return rc;
    }
  }

  // 重置状态
  current_result_index_ = 0;
  all_processed_ = false;
  final_results_.clear();
  seen_tuples_.clear();
  current_tuple_.reset();
  current_expressions_.clear();

  return RC::SUCCESS;
}

RC UnionPhysicalOperator::next()
{
  // 如果还没处理完，先完全处理所有子查询
  if (!all_processed_) {
    RC rc = process_all_children();
    if (rc != RC::SUCCESS) {
      return rc;
    }
    all_processed_ = true;
  }
  
  // 输出最终结果
  if (current_result_index_ < final_results_.size()) {
    current_tuple_ = std::move(final_results_[current_result_index_]);
    current_result_index_++;
    return RC::SUCCESS;
  }
  
  return RC::RECORD_EOF;
}

RC UnionPhysicalOperator::close()
{
  // 关闭所有子算子
  for (auto &child : children_) {
    RC rc = child->close();
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to close child operator: %s", strrc(rc));
    }
  }

  // 清理状态
  final_results_.clear();
  seen_tuples_.clear();
  current_tuple_.reset();
  current_expressions_.clear();
  
  return RC::SUCCESS;
}

Tuple *UnionPhysicalOperator::current_tuple()
{
  return current_tuple_.get();
}

RC UnionPhysicalOperator::tuple_schema(TupleSchema &schema) const
{
  if (result_types_.empty() || result_names_.empty()) {
    LOG_WARN("result schema not set for UNION operator");
    return RC::INVALID_ARGUMENT;
  }

  // 清空schema并重新构建
  schema = TupleSchema();
  for (size_t i = 0; i < result_types_.size(); ++i) {
    TupleCellSpec cell_spec(result_names_[i].c_str());
    schema.append_cell(cell_spec);
  }

  return RC::SUCCESS;
}

void UnionPhysicalOperator::set_result_schema(const vector<AttrType> &types, const vector<string> &names)
{
  result_types_ = types;
  result_names_ = names;
}


bool UnionPhysicalOperator::is_duplicate(const Tuple &tuple)
{
  string key = tuple_to_key(tuple);
  return seen_tuples_.find(key) != seen_tuples_.end();
}

string UnionPhysicalOperator::tuple_to_key(const Tuple &tuple)
{
  string key;
  
  for (int i = 0; i < tuple.cell_num(); ++i) {
    Value cell_value;
    RC rc = tuple.cell_at(i, cell_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get cell at index %d: %s", i, strrc(rc));
      continue;
    }

    // 将每个cell的值转换为字符串并连接
    if (i > 0) {
      key += "|";
    }
    
    if (cell_value.is_null()) {
      key += "NULL";
    } else {
      key += cell_value.to_string();
    }
  }

  return key;
}

RC UnionPhysicalOperator::process_all_children()
{
  // 1. 收集所有子查询的完整结果
  vector<vector<unique_ptr<Tuple>>> all_child_results;
  for (size_t i = 0; i < children_.size(); i++) {
    vector<unique_ptr<Tuple>> child_result;
    RC rc = collect_child_results(i, child_result);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to collect results from child %zu: %s", i, strrc(rc));
      return rc;
    }
    all_child_results.push_back(std::move(child_result));
  }
  
  // 2. 按照UNION语义合并结果
  final_results_.clear();
  seen_tuples_.clear();
  
  for (size_t i = 0; i < all_child_results.size(); i++) {
    bool need_distinct = (i > 0) ? distinct_flags_[i-1] : false;
    
    if (need_distinct) {
      // 如果当前UNION是去重的，先对左边结果进行内部去重
      deduplicate_left_results();
    }
    
    for (auto &tuple : all_child_results[i]) {
      if (need_distinct && is_duplicate(*tuple)) {
        continue; // 跳过重复
      }
      
      final_results_.push_back(std::move(tuple));
      string key = tuple_to_key(*final_results_.back());
      seen_tuples_.insert(key);
    }
  }
  
  return RC::SUCCESS;
}

RC UnionPhysicalOperator::collect_child_results(size_t child_index, vector<unique_ptr<Tuple>> &results)
{
  if (child_index >= children_.size()) {
    LOG_WARN("invalid child index: %zu", child_index);
    return RC::INVALID_ARGUMENT;
  }
  
  PhysicalOperator *child = children_[child_index].get();
  results.clear();
  
  while (true) {
    RC rc = child->next();
    if (rc == RC::SUCCESS) {
      // 获取子算子的tuple
      Tuple *child_tuple = child->current_tuple();
      if (child_tuple == nullptr) {
        LOG_WARN("child operator returned null tuple");
        return RC::INTERNAL;
      }

      // 直接使用ValueListTuple，避免Expression的生命周期问题
      auto tuple = make_unique<ValueListTuple>();
      vector<Value> cells;
      vector<TupleCellSpec> specs;
      
      // 将子算子的数据转换为结果集类型
      for (size_t i = 0; i < result_types_.size(); ++i) {
        if (i >= child_tuple->cell_num()) {
          LOG_WARN("column count mismatch in UNION");
          return RC::SCHEMA_FIELD_TYPE_MISMATCH;
        }

        Value cell_value;
        rc = child_tuple->cell_at(i, cell_value);
        if (rc != RC::SUCCESS) {
          LOG_WARN("failed to get cell at index %zu: %s", i, strrc(rc));
          return rc;
        }

        // 类型转换
        Value converted_value;
        if (cell_value.attr_type() != result_types_[i]) {
          rc = Value::cast_to(cell_value, result_types_[i], converted_value);
          if (rc != RC::SUCCESS) {
            LOG_WARN("failed to cast value from %s to %s: %s", 
                     attr_type_to_string(cell_value.attr_type()),
                     attr_type_to_string(result_types_[i]),
                     strrc(rc));
            return rc;
          }
        } else {
          converted_value = cell_value;
        }

        cells.push_back(converted_value);
        specs.push_back(TupleCellSpec(result_names_[i].c_str()));
      }
      
      tuple->set_cells(cells);
      tuple->set_names(specs);
      results.push_back(std::move(tuple));
      
    } else if (rc == RC::RECORD_EOF) {
      break;
    } else {
      LOG_WARN("failed to fetch from child operator: %s", strrc(rc));
      return rc;
    }
  }
  
  return RC::SUCCESS;
}

void UnionPhysicalOperator::deduplicate_left_results()
{
  // 对左边结果进行内部去重
  vector<unique_ptr<Tuple>> deduplicated;
  unordered_set<string> left_seen;
  
  for (auto &tuple : final_results_) {
    string key = tuple_to_key(*tuple);
    if (left_seen.find(key) == left_seen.end()) {
      deduplicated.push_back(std::move(tuple));
      left_seen.insert(key);
    }
  }
  
  final_results_ = std::move(deduplicated);
  seen_tuples_ = left_seen;
}
