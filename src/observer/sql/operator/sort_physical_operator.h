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
// Created by Wangyunlai on 2022/12/07.
//

#pragma once

#include "sql/operator/physical_operator.h"
#include "sql/parser/parse_defs.h"
#include "sql/expr/expression.h"
#include "sql/expr/tuple.h"
#include <fstream>
#include <queue>
#include <functional>
#include "session/session.h"
#include "storage/table/table.h"

/**
 * @brief 轻量级Tuple，只存储cells，不存储specs
 * @ingroup Tuple
 */
class LightTuple {
public:
  LightTuple() = default;
  explicit LightTuple(const vector<Value>& cells) : cells_(cells) {}
  
  // 基本操作
  void set_cells(const vector<Value>& cells) { cells_ = cells; }
  const vector<Value>& cells() const { return cells_; }
  size_t size() const { return cells_.size(); }
  
  // 排序比较函数
  bool compare_with_schema(const LightTuple& other, 
                         const TupleSchema& schema,
                         const vector<unique_ptr<Expression>>& order_by_expressions,
                         const vector<OrderDirection>& order_by_directions) const;
  
  // 优化的比较函数：直接使用索引
  bool compare_with_schema_optimized(const LightTuple& other,
                                   const vector<int>& sort_field_indices,
                                   const vector<OrderDirection>& order_by_directions) const;
  
private:
  vector<Value> cells_;
  
  // 从schema获取排序字段值
  RC get_sort_value_from_schema(Value& value, 
                               const TupleSchema& schema,
                               const unique_ptr<Expression>& expr) const;
  
  // 字段索引查找
  int find_field_index_in_schema(const TupleSchema& schema, 
                                  const FieldExpr* field_expr) const;
};

/**
 * @brief 排序物理算子
 * @ingroup PhysicalOperator
 */
// 前向声明
class SortPhysicalOperator;

// 文件读取器类
class ChunkFileReader {
public:
  ChunkFileReader(SortPhysicalOperator* parent, const string& filename);
  ~ChunkFileReader();
  
  RC open();
  RC next(LightTuple& tuple);               // 使用LightTuple
  void close();
  bool is_eof() const { return eof_; }
  
private:
  static const size_t BUFFER_SIZE = 1000;  // 预读缓冲区大小
  RC refill_buffer();                        // 重新填充缓冲区
  RC read_next_tuple_from_file();           // 从文件读取下一个tuple
  
  SortPhysicalOperator* parent_;
  string filename_;
  std::ifstream file_;
  vector<LightTuple> buffer_;                // 预读缓冲区 (使用LightTuple)
  size_t buffer_pos_ = 0;                   // 当前缓冲区位置
  bool eof_ = false;
  bool opened_ = false;
};

class SortPhysicalOperator : public PhysicalOperator
{
public:
  SortPhysicalOperator(vector<unique_ptr<Expression>> order_by_expressions,
                       vector<OrderDirection> order_by_directions);
  virtual ~SortPhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::SORT; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override;

  RC tuple_schema(TupleSchema &schema) const override;

private:
  vector<unique_ptr<Expression>> order_by_expressions_;
  vector<OrderDirection>         order_by_directions_;

  bool                           opened_ = false;
  TupleSchema                    tuple_schema_;
  
  // 多路排序相关
  vector<string>                 temp_files_;           // 临时文件列表
  size_t                         current_file_index_ = 0;  // 当前读取的文件索引
  size_t                         current_chunk_index_ = 0; // 当前分块索引
  size_t                         chunk_size_ = 0;      // 分块大小
  vector<LightTuple>             current_chunk_;       // 当前分块数据 (使用LightTuple)
  size_t                         current_chunk_pos_ = 0;   // 当前分块位置
  
  // 优化的排序字段索引
  vector<int>                    sort_field_indices_;  // 预计算的排序字段索引
  
  // 当前返回的tuple（转换后的ValueListTuple）
  ValueListTuple                 current_value_tuple_;  // 用于返回给上层算子
  
  // 归并相关
  vector<unique_ptr<ChunkFileReader>> file_readers_;  // 文件读取器列表
  bool                             merge_completed_ = false;  // 归并是否完成
  
  // 优先队列相关
  struct MergeItem {
    LightTuple tuple;            // 使用LightTuple
    size_t file_index;
    SortPhysicalOperator* sort_op;
    
    bool operator>(const MergeItem& other) const {
      bool left_less_than_right = sort_op->compare_lightweight_tuples(tuple, other.tuple);
      return !left_less_than_right;
    }
  };
  std::priority_queue<MergeItem, vector<MergeItem>, std::greater<MergeItem>> merge_queue_;

  // 排序比较函数
  bool compare_value_tuples(const ValueListTuple &left, const ValueListTuple &right);
  
  // LightTuple排序比较函数
  bool compare_lightweight_tuples(const LightTuple &left, const LightTuple &right);
  
  // 转换函数：LightTuple -> ValueListTuple
  ValueListTuple convert_to_value_list_tuple(const LightTuple& tuple) const;
  
  // 预计算排序字段索引
  RC precompute_sort_indices();
  
  // 字段索引查找
  int find_field_index_in_schema(const TupleSchema& schema, const FieldExpr* field_expr) const;
  
  // LightTuple序列化/反序列化
  RC serialize_light_tuple(const LightTuple& tuple, std::ofstream& file);
  RC deserialize_light_tuple(LightTuple& tuple, std::ifstream& file);
  
  // 多路排序方法
  RC open_multi_sort(Trx *trx);
  RC next_multi_sort();
  
  // 流式处理辅助方法
  RC load_chunk_from_merge_queue();
  RC initialize_merge_queue();
  bool has_more_data_in_files();
  
  
  // 分块排序方法
  RC sort_and_write_chunk(const vector<LightTuple>& chunk, const string& filename);
  RC merge_sorted_files();
  
  // 序列化方法
  RC serialize_value(const Value& value, std::ofstream& file);
  RC deserialize_value(Value& value, std::ifstream& file);
  
  // 友元类，允许ChunkFileReader访问私有方法
  friend class ChunkFileReader;
  
  
  // 归并相关方法
  RC load_next_chunk_from_file();
  
  // 工具方法
  RC cleanup_temp_files();
  string generate_temp_filename();
};
