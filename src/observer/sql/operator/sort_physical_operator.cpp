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

#include "sql/operator/sort_physical_operator.h"
#include "common/log/log.h"
#include "sql/expr/tuple.h"
#include "common/type/attr_type.h"
#include "session/session.h"
#include "storage/table/table.h"
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <queue>
#include <memory>

// ==================== LightTuple 实现 ====================

bool LightTuple::compare_with_schema(const LightTuple& other, 
                                    const TupleSchema& schema,
                                    const vector<unique_ptr<Expression>>& order_by_expressions,
                                    const vector<OrderDirection>& order_by_directions) const {
  for (size_t i = 0; i < order_by_expressions.size(); i++) {
    Value left_value, right_value;
    
    // 从cells_和schema获取排序字段值
    RC rc = get_sort_value_from_schema(left_value, schema, order_by_expressions[i]);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to get left sort value for expression %zu: %s", i, strrc(rc));
      return true;
    }
    
    // 🐾 修复：应该使用 other 的数据来获取右侧值
    rc = other.get_sort_value_from_schema(right_value, schema, order_by_expressions[i]);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to get right sort value for expression %zu: %s", i, strrc(rc));
      return false;
    }
    
    // 比较值
    int cmp_result = left_value.compare(right_value);
    if (cmp_result != 0) {
      bool result = (order_by_directions[i] == ASC_ORDER) ? 
                   (cmp_result < 0) : (cmp_result > 0);
      return result;
    }
  }
  return false;
}

bool LightTuple::compare_with_schema_optimized(const LightTuple& other,
                                             const vector<int>& sort_field_indices,
                                             const vector<OrderDirection>& order_by_directions) const {
  for (size_t i = 0; i < sort_field_indices.size(); i++) {
    int field_index = sort_field_indices[i];
    
    // 直接使用索引获取值
    if (field_index >= cells_.size() || field_index >= other.cells_.size()) {
      LOG_WARN("Field index out of range: %d", field_index);
      return true;
    }
    
    const Value& left_value = cells_[field_index];
    const Value& right_value = other.cells_[field_index];
    
    // 比较值
    int cmp_result = left_value.compare(right_value);
    if (cmp_result != 0) {
      bool result = (order_by_directions[i] == ASC_ORDER) ? 
                   (cmp_result < 0) : (cmp_result > 0);
      return result;
    }
  }
  return false;
}

RC LightTuple::get_sort_value_from_schema(Value& value, 
                                         const TupleSchema& schema,
                                         const unique_ptr<Expression>& expr) const {
  // 检查表达式类型
  if (expr->type() == ExprType::FIELD) {
    // 字段表达式：从cells_中获取对应字段值
    FieldExpr* field_expr = static_cast<FieldExpr*>(expr.get());
    
    // 根据字段名在schema中找到索引
    int field_index = find_field_index_in_schema(schema, field_expr);
    if (field_index < 0) {
      LOG_WARN("Field not found in schema: %s", field_expr->field_name());
      return RC::INTERNAL;
    }
    
    // 从cells_获取值
    if (field_index >= cells_.size()) {
      LOG_WARN("Field index out of range: %d", field_index);
      return RC::INTERNAL;
    }
    
    value = cells_[field_index];
    return RC::SUCCESS;
    
  } else if (expr->type() == ExprType::VALUE) {
    // 常量表达式：直接获取值
    ValueExpr* value_expr = static_cast<ValueExpr*>(expr.get());
    value = value_expr->get_value();
    return RC::SUCCESS;
    
  } else {
    // 复杂表达式：需要计算
    LOG_INFO("[LIGHTTUPLE_DEBUG] Complex expression detected: type=%d, name='%s'", 
             static_cast<int>(expr->type()), expr->name());
    
    // 这里需要创建一个临时的ValueListTuple来支持表达式计算
    ValueListTuple temp_tuple;
    temp_tuple.set_cells(cells_);
    
    // 从schema构建specs
    vector<TupleCellSpec> specs;
    for (int i = 0; i < schema.cell_num(); i++) {
      specs.push_back(schema.cell_at(i));
    }
    temp_tuple.set_names(specs);
    
    LOG_INFO("[LIGHTTUPLE_DEBUG] Calling expr->get_value() for complex expression");
    
    // 使用表达式计算值
    RC rc = expr->get_value(temp_tuple, value);
    
    if (rc == RC::SUCCESS) {
      LOG_INFO("[LIGHTTUPLE_DEBUG] Complex expression evaluated: value_type=%d, value='%s'", 
               static_cast<int>(value.attr_type()), value.to_string().c_str());
    } else {
      LOG_WARN("[LIGHTTUPLE_DEBUG] Complex expression evaluation failed: %s", strrc(rc));
    }
    
    return rc;
  }
}

int LightTuple::find_field_index_in_schema(const TupleSchema& schema, 
                                          const FieldExpr* field_expr) const {
  const char* table_name = field_expr->table_name();
  const char* field_name = field_expr->field_name();
  
  // 遍历schema查找匹配的字段
  for (int i = 0; i < schema.cell_num(); i++) {
    const TupleCellSpec& spec = schema.cell_at(i);
    
    // 检查表名和字段名是否匹配
    if (strcmp(spec.table_name(), table_name) == 0 && 
        strcmp(spec.field_name(), field_name) == 0) {
      return i;
    }
  }
  
  return -1;  // 未找到
}

// ==================== SortPhysicalOperator 新增函数实现 ====================

bool SortPhysicalOperator::compare_lightweight_tuples(const LightTuple &left, const LightTuple &right) {
  // 使用优化的比较函数
  if (!sort_field_indices_.empty()) {
    return left.compare_with_schema_optimized(right, sort_field_indices_, order_by_directions_);
  } else {
    // 回退到通用比较函数
    return left.compare_with_schema(right, tuple_schema_, order_by_expressions_, order_by_directions_);
  }
}

ValueListTuple SortPhysicalOperator::convert_to_value_list_tuple(const LightTuple& tuple) const {
  ValueListTuple result;
  result.set_cells(tuple.cells());
  
  // 从tuple_schema_获取specs
  vector<TupleCellSpec> specs;
  for (int i = 0; i < tuple_schema_.cell_num(); i++) {
    specs.push_back(tuple_schema_.cell_at(i));
  }
  result.set_names(specs);
  
  return result;
}

RC SortPhysicalOperator::precompute_sort_indices() {
  sort_field_indices_.clear();
  
  for (const auto& expr : order_by_expressions_) {
    if (expr->type() == ExprType::FIELD) {
      FieldExpr* field_expr = static_cast<FieldExpr*>(expr.get());
      int index = find_field_index_in_schema(tuple_schema_, field_expr);
      if (index < 0) {
        LOG_WARN("Sort field not found in schema: %s", field_expr->field_name());
        return RC::INTERNAL;
      }
      sort_field_indices_.push_back(index);
    } else {
      // 遇到复杂表达式（如函数调用），清空索引列表，回退到通用比较模式
      LOG_INFO("Complex sort expression detected (type=%d), falling back to generic comparison mode", 
               static_cast<int>(expr->type()));
      sort_field_indices_.clear();
      return RC::SUCCESS;  // 返回成功，但索引列表为空，会使用通用比较
    }
  }
  
  return RC::SUCCESS;
}

int SortPhysicalOperator::find_field_index_in_schema(const TupleSchema& schema, 
                                                    const FieldExpr* field_expr) const {
  const char* table_name = field_expr->table_name();
  const char* field_name = field_expr->field_name();
  
  // 遍历schema查找匹配的字段
  for (int i = 0; i < schema.cell_num(); i++) {
    const TupleCellSpec& spec = schema.cell_at(i);
    
    // 检查表名和字段名是否匹配
    if (strcmp(spec.table_name(), table_name) == 0 && 
        strcmp(spec.field_name(), field_name) == 0) {
      return i;
    }
  }
  
  return -1;  // 未找到
}

SortPhysicalOperator::SortPhysicalOperator(vector<unique_ptr<Expression>> order_by_expressions,
                                           vector<OrderDirection> order_by_directions)
    : order_by_expressions_(std::move(order_by_expressions)),
      order_by_directions_(std::move(order_by_directions))
{}

RC SortPhysicalOperator::open(Trx *trx)
{
  if (opened_) {
    return RC::SUCCESS;
  }

  if (children_.empty()) {
    LOG_WARN("sort operator has no child");
    return RC::INTERNAL;
  }

  PhysicalOperator *child = children_[0].get();
  RC rc = child->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open child operator: %s", strrc(rc));
    return rc;
  }

  // 不再从子算子获取schema，而是在open_multi_sort中从第一个tuple构造

  // 设置固定分块大小用于测试
  chunk_size_ = 40000;
  
  rc = open_multi_sort(trx);

  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open sort operator: %s", strrc(rc));
    return rc;
  }

  opened_ = true;
  return RC::SUCCESS;
}

RC SortPhysicalOperator::next()
{
  if (!opened_) {
    return RC::RECORD_EOF;
  }

  return next_multi_sort();
}

RC SortPhysicalOperator::close()
{
  if (!opened_) {
    return RC::SUCCESS;
  }

  // 清理资源
  opened_ = false;

  // 清理临时文件
  cleanup_temp_files();

  if (!children_.empty()) {
    return children_[0]->close();
  }
  return RC::SUCCESS;
}

Tuple *SortPhysicalOperator::current_tuple()
{
  if (!opened_) {
    return nullptr;
  }

  // 多路排序：从当前分块返回
  if (current_chunk_pos_ == 0 || current_chunk_pos_ > current_chunk_.size()) {
    return nullptr;
  }
  
  // 将LightTuple转换为ValueListTuple返回给上层
  current_value_tuple_ = convert_to_value_list_tuple(current_chunk_[current_chunk_pos_ - 1]);
  return &current_value_tuple_;
}

RC SortPhysicalOperator::tuple_schema(TupleSchema &schema) const
{
  schema = tuple_schema_;
  return RC::SUCCESS;
}


bool SortPhysicalOperator::compare_value_tuples(const ValueListTuple &left, const ValueListTuple &right)
{
  for (size_t i = 0; i < order_by_expressions_.size(); i++) {
    Value left_value;
    Value right_value;

    LOG_INFO("[SORT_DEBUG] Evaluating order expression %zu for left tuple: type=%d, name='%s'", 
             i, static_cast<int>(order_by_expressions_[i]->type()), order_by_expressions_[i]->name());

    RC rc = order_by_expressions_[i]->get_value(left, left_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("[SORT_DEBUG] Failed to get value from left tuple for order by expression %zu: %s", i, strrc(rc));
      return true;
    }
    
    LOG_INFO("[SORT_DEBUG] Left value: type=%d, value='%s'", 
             static_cast<int>(left_value.attr_type()), left_value.to_string().c_str());

    LOG_INFO("[SORT_DEBUG] Evaluating order expression %zu for right tuple", i);
    
    rc = order_by_expressions_[i]->get_value(right, right_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("[SORT_DEBUG] Failed to get value from right tuple for order by expression %zu: %s", i, strrc(rc));
      return false;
    }
    
    LOG_INFO("[SORT_DEBUG] Right value: type=%d, value='%s'", 
             static_cast<int>(right_value.attr_type()), right_value.to_string().c_str());

    int cmp_result = left_value.compare(right_value);
    if (cmp_result != 0) {
      bool result = (order_by_directions_[i] == ASC_ORDER) ? (cmp_result < 0) : (cmp_result > 0);
      return result;
    }
  }
  return false;
}


// ==================== 多路排序方法 ====================

RC SortPhysicalOperator::open_multi_sort(Trx *trx)
{
  
  PhysicalOperator *child = children_[0].get();
  vector<LightTuple> current_chunk;  // 使用LightTuple
  size_t chunk_count = 0;
  
  // 首先获取第一个tuple来构造schema
  RC rc = child->next();
  if (rc == RC::RECORD_EOF) {
    LOG_INFO("no data to sort, returning empty result");
    return RC::SUCCESS;
  } else if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get first tuple for schema construction: %s", strrc(rc));
    return rc;
  }

  Tuple *first_tuple = child->current_tuple();
  if (first_tuple != nullptr) {
    // 从第一个tuple构造schema
    for (int i = 0; i < first_tuple->cell_num(); i++) {
      TupleCellSpec spec;
      RC spec_rc = first_tuple->spec_at(i, spec);
      if (spec_rc != RC::SUCCESS) {
        LOG_WARN("failed to get spec at %d: %s", i, strrc(spec_rc));
        return spec_rc;
      }
      tuple_schema_.append_cell(spec);
    }
  } else {
    LOG_WARN("first tuple is null, cannot construct schema");
    return RC::INTERNAL;
  }
  
  // 构造完tuple_schema_后，预计算排序字段索引
  rc = precompute_sort_indices();
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to precompute sort indices: %s", strrc(rc));
    return rc;
  }
  
  // 将第一个tuple转换为LightTuple并添加到当前分块
  vector<Value> cells;
  for (int i = 0; i < first_tuple->cell_num(); i++) {
    Value cell;
    rc = first_tuple->cell_at(i, cell);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get cell at %d: %s", i, strrc(rc));
      return rc;
    }
    cells.push_back(cell);
  }

  LightTuple light_tuple(cells);
  current_chunk.push_back(std::move(light_tuple));
  
  
  // 继续处理剩余数据
  while (RC::SUCCESS == (rc = child->next())) {
    Tuple *tuple = child->current_tuple();
    if (tuple != nullptr) {
      // 将tuple转换为LightTuple
      vector<Value> cells;
      for (int i = 0; i < tuple->cell_num(); i++) {
        Value cell;
        rc = tuple->cell_at(i, cell);
        if (rc != RC::SUCCESS) {
          LOG_WARN("failed to get cell at %d: %s", i, strrc(rc));
          return rc;
        }
        cells.push_back(cell);
      }

      LightTuple light_tuple(cells);
      current_chunk.push_back(std::move(light_tuple));
      
      // 如果分块满了，排序并写入文件
      if (current_chunk.size() >= chunk_size_) {
        LOG_INFO("=== CHUNK FULL ===");
        LOG_INFO("Chunk %zu is full with %zu tuples (chunk_size=%zu)", chunk_count + 1, current_chunk.size(), chunk_size_);
        LOG_INFO("Writing chunk to temporary file...");
        
        string filename = generate_temp_filename();
        rc = sort_and_write_chunk(current_chunk, filename);
        if (rc != RC::SUCCESS) {
          LOG_WARN("failed to sort and write chunk: %s", strrc(rc));
          return rc;
        }
        temp_files_.push_back(filename);
        current_chunk.clear();
        chunk_count++;
        
        LOG_INFO("Chunk %zu written to file: %s", chunk_count, filename.c_str());
        LOG_INFO("=== CHUNK WRITTEN ===");
      }
    }
  }


  // 处理最后一个分块
  if (!current_chunk.empty()) {
    LOG_INFO("=== FINAL CHUNK ===");
    LOG_INFO("Final chunk has %zu tuples (chunk_size=%zu)", current_chunk.size(), chunk_size_);
    LOG_INFO("Writing final chunk to temporary file...");
    
    string filename = generate_temp_filename();
    rc = sort_and_write_chunk(current_chunk, filename);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to sort and write final chunk: %s", strrc(rc));
      return rc;
    }
    temp_files_.push_back(filename);
    chunk_count++;
    
    LOG_INFO("Final chunk written to file: %s", filename.c_str());
    LOG_INFO("=== FINAL CHUNK WRITTEN ===");
  }

  
  // 关键步骤：进行多路归并
  if (chunk_count > 1) {
    rc = merge_sorted_files();
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to merge sorted files: %s", strrc(rc));
      return rc;
    }
  } else if (chunk_count == 1) {
    // 只有一个分块，直接加载到内存
    rc = load_next_chunk_from_file();
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to load single chunk: %s", strrc(rc));
    return rc;
    }
  }
  
  // 初始化多路排序状态
  current_file_index_ = 0;
  current_chunk_index_ = 0;
  // current_chunk_pos_ 已经在 merge_sorted_files 或 load_next_chunk_from_file 中正确设置
  
  return RC::SUCCESS;
}

RC SortPhysicalOperator::next_multi_sort()
{
  
  // 如果当前批次还有数据，继续返回
  if (current_chunk_pos_ < current_chunk_.size()) {
    current_chunk_pos_++;
    return RC::SUCCESS;
  }
  
  // 当前批次已读完，检查是否还有更多数据
  if (has_more_data_in_files()) {
    // 重新加载下一批数据
    RC rc = load_chunk_from_merge_queue();
    if (rc == RC::SUCCESS && !current_chunk_.empty()) {
      current_chunk_pos_ = 1;
      return RC::SUCCESS;
    } else {
      LOG_WARN("Failed to load next batch: %s", strrc(rc));
      return rc;
    }
  }
  
  // 没有更多数据
  return RC::RECORD_EOF;
}

// ==================== 分块排序方法 ====================

RC SortPhysicalOperator::sort_and_write_chunk(const vector<LightTuple>& chunk, const string& filename)
{
  
  // 复制分块数据并排序
  vector<LightTuple> sorted_chunk = chunk;
  std::sort(sorted_chunk.begin(), sorted_chunk.end(),
            [this](const LightTuple &left, const LightTuple &right) {
              return compare_lightweight_tuples(left, right);
            });

  // 写入临时文件
  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    LOG_WARN("failed to open temp file: %s", filename.c_str());
    return RC::INTERNAL;
  }

  // 直接序列化每个LightTuple，不写入chunk_size字段
  for (size_t i = 0; i < sorted_chunk.size(); i++) {
    RC rc = serialize_light_tuple(sorted_chunk[i], file);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to serialize LightTuple at index %zu", i);
      file.close();
      return rc;
    }
  }
  
  file.close();
  return RC::SUCCESS;
}

RC SortPhysicalOperator::serialize_light_tuple(const LightTuple& tuple, std::ofstream& file)
{
  // 序列化cell数量
  int cell_count = tuple.cells().size();
  file.write(reinterpret_cast<const char*>(&cell_count), sizeof(cell_count));
  
  // 序列化每个cell
  for (const Value& cell : tuple.cells()) {
    RC rc = serialize_value(cell, file);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to serialize value in LightTuple");
      return rc;
    }
  }
  
  if (!file.good()) {
    LOG_WARN("Failed to serialize LightTuple to file");
    return RC::INTERNAL;
  }
  
  return RC::SUCCESS;
}

RC SortPhysicalOperator::deserialize_light_tuple(LightTuple& tuple, std::ifstream& file)
{
  // 反序列化cell数量
  int cell_count;
  file.read(reinterpret_cast<char*>(&cell_count), sizeof(cell_count));
  if (file.eof()) {
    return RC::RECORD_EOF;
  }
  if (!file.good()) {
    LOG_WARN("Failed to read cell count from file");
    return RC::INTERNAL;
  }
  
  // 反序列化每个cell
  vector<Value> cells;
  for (int i = 0; i < cell_count; i++) {
    Value cell;
    RC rc = deserialize_value(cell, file);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to deserialize cell at index %d", i);
      return rc;
    }
    cells.push_back(cell);
  }
  
  tuple.set_cells(cells);
  
  if (!file.good()) {
    LOG_WARN("Failed to deserialize LightTuple from file");
    return RC::INTERNAL;
  }
  
  return RC::SUCCESS;
}

// ==================== 工具方法 ====================

RC SortPhysicalOperator::cleanup_temp_files()
{
  for (const string& filename : temp_files_) {
    if (std::remove(filename.c_str()) != 0) {
      LOG_WARN("failed to remove temp file: %s", filename.c_str());
    }
  }
  temp_files_.clear();
  return RC::SUCCESS;
}

string SortPhysicalOperator::generate_temp_filename()
{
  static int counter = 0;
  return "/tmp/sort_chunk_" + std::to_string(getpid()) + "_" + std::to_string(counter++);
}

// ==================== 序列化方法 ====================

RC SortPhysicalOperator::serialize_value(const Value& value, std::ofstream& file)
{
  // 写入类型
  AttrType type = value.attr_type();
  file.write(reinterpret_cast<const char*>(&type), sizeof(type));
  
  // 写入长度
  int length = value.length();
  file.write(reinterpret_cast<const char*>(&length), sizeof(length));
  
  // 写入是否为空
  bool is_null = value.is_null();
  file.write(reinterpret_cast<const char*>(&is_null), sizeof(is_null));
  
  // 写入是否拥有数据（根据类型推断）
  bool own_data = (type == AttrType::CHARS || type == AttrType::TEXTS);
  file.write(reinterpret_cast<const char*>(&own_data), sizeof(own_data));
  
  // 写入实际数据
  if (length > 0 && !is_null) {
    file.write(value.data(), length);
  }
  
  if (!file.good()) {
    LOG_WARN("Failed to serialize value to file");
    return RC::INTERNAL;
  }
  
  // 序列化完成
  
  return RC::SUCCESS;
}

RC SortPhysicalOperator::deserialize_value(Value& value, std::ifstream& file)
{
  // 读取类型
  AttrType type;
  file.read(reinterpret_cast<char*>(&type), sizeof(type));
  
  // 检查文件状态
  if (!file.good()) {
    return RC::INTERNAL;
  }
  
  // 检查类型是否有效
  if (type < AttrType::UNDEFINED || type >= AttrType::MAXTYPE) {
    LOG_WARN("Invalid data type: %d (0x%x)", static_cast<int>(type), static_cast<int>(type));
    return RC::INTERNAL;
  }
  
  value.set_type(type);
  
  // 读取长度
  int length;
  file.read(reinterpret_cast<char*>(&length), sizeof(length));
  
  // 读取是否为空
  bool is_null;
  file.read(reinterpret_cast<char*>(&is_null), sizeof(is_null));
  value.set_null(is_null);
  
  // 读取是否拥有数据
  bool own_data;
  file.read(reinterpret_cast<char*>(&own_data), sizeof(own_data));
  
  // 读取实际数据
  if (length > 0 && !is_null) {
    char* data = new char[length];
    file.read(data, length);
    value.set_data(data, length);
    // 总是释放临时分配的内存，因为Value会复制数据
    delete[] data;
  }
  
  if (!file.good()) {
    LOG_WARN("Failed to deserialize value from file");
    return RC::INTERNAL;
  }
  
  // 反序列化完成
  
  return RC::SUCCESS;
}



// ==================== 文件读取器类 ====================

ChunkFileReader::ChunkFileReader(SortPhysicalOperator* parent, const string& filename)
  : parent_(parent), filename_(filename), eof_(false), opened_(false)
{
}

ChunkFileReader::~ChunkFileReader()
{
  close();
}

RC ChunkFileReader::open()
{
  if (opened_) {
    return RC::SUCCESS;
  }
  
  file_.open(filename_, std::ios::binary);
  if (!file_.is_open()) {
    LOG_WARN("Failed to open file: %s", filename_.c_str());
    return RC::INTERNAL;
  }
  
  opened_ = true;
  eof_ = false;
  buffer_pos_ = 0;
  buffer_.clear();
  
  return RC::SUCCESS;
}

RC ChunkFileReader::next(LightTuple& tuple)
{
  if (!opened_) {
    RC rc = open();
    if (rc != RC::SUCCESS) {
      return rc;
    }
  }
  
  if (eof_) {
    return RC::RECORD_EOF;
  }

  // 如果缓冲区空了，重新填充
  if (buffer_pos_ >= buffer_.size()) {
    RC rc = refill_buffer();
    if (rc != RC::SUCCESS) {
      return rc;
    }
  }
  
  // 从缓冲区返回tuple
  tuple = buffer_[buffer_pos_++];
  return RC::SUCCESS;
}

void ChunkFileReader::close()
{
  if (opened_) {
    file_.close();
    opened_ = false;
  }
  buffer_.clear();
  buffer_pos_ = 0;
  eof_ = false;
}

RC ChunkFileReader::refill_buffer()
{
  buffer_.clear();
  buffer_pos_ = 0;
  
  // 批量读取BUFFER_SIZE个tuple
  for (size_t i = 0; i < BUFFER_SIZE; i++) {
    RC rc = read_next_tuple_from_file();
    if (rc == RC::RECORD_EOF) break;
    if (rc != RC::SUCCESS) return rc;
  }
  
  return buffer_.empty() ? RC::RECORD_EOF : RC::SUCCESS;
}

RC ChunkFileReader::read_next_tuple_from_file()
{
  if (file_.eof()) {
    eof_ = true;
    return RC::RECORD_EOF;
  }
  
  // 检查文件状态
  if (!file_.good()) {
    return RC::INTERNAL;
  }
  
  LightTuple tuple;
  RC rc = parent_->deserialize_light_tuple(tuple, file_);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to deserialize LightTuple from file: %s", filename_.c_str());
    return rc;
  }
  
  // 将读取的tuple存储到缓冲区
  buffer_.push_back(tuple);
  return RC::SUCCESS;
}

// ==================== 多路归并算法 ====================

RC SortPhysicalOperator::merge_sorted_files()
{
  
  if (temp_files_.empty()) {
    LOG_WARN("No temporary files to merge");
    return RC::INTERNAL;
  }
  
  // 创建文件读取器
  file_readers_.clear();
  for (size_t i = 0; i < temp_files_.size(); i++) {
    const string& filename = temp_files_[i];
    auto reader = make_unique<ChunkFileReader>(this, filename);
    RC rc = reader->open();
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to open file reader for: %s", filename.c_str());
      return rc;
    }
    file_readers_.push_back(std::move(reader));
  }
  
  // 使用优先队列进行多路归并
  // 定义归并项结构
  struct MergeItem {
    LightTuple tuple;            // 使用LightTuple
    size_t file_index;
    SortPhysicalOperator* sort_op; // 添加SortPhysicalOperator指针
    
    // 比较函数
    bool operator>(const MergeItem& other) const {
      // 调用SortPhysicalOperator的compare_lightweight_tuples方法
      // compare_lightweight_tuples返回true表示left < right，返回false表示left > right
      // 但operator>需要返回true表示left > right
      bool left_less_than_right = sort_op->compare_lightweight_tuples(tuple, other.tuple);
      return !left_less_than_right;  // 当left < right时返回false，当left > right时返回true
    }
  };
  
  
  // 初始化优先队列
  RC rc = initialize_merge_queue();
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to initialize merge queue: %s", strrc(rc));
    return rc;
  }
  
  // 只加载chunk_size_个元素到current_chunk_
  rc = load_chunk_from_merge_queue();
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to load chunk from merge queue: %s", strrc(rc));
    return rc;
  }
  
  // 设置状态，准备流式输出
  current_chunk_pos_ = 0;
  merge_completed_ = false;  // 改为false，表示还有更多数据
  
  
  
  return RC::SUCCESS;
}

RC SortPhysicalOperator::load_chunk_from_merge_queue()
{
  current_chunk_.clear();
  current_chunk_pos_ = 0;
  
  // 从优先队列中取出chunk_size_个元素
  while (current_chunk_.size() < chunk_size_ && !merge_queue_.empty()) {
    auto item = merge_queue_.top();
    merge_queue_.pop();
    current_chunk_.push_back(item.tuple);
    
    // 从对应文件读取下一个tuple
    LightTuple next_tuple;
    RC rc = file_readers_[item.file_index]->next(next_tuple);
    if (rc == RC::SUCCESS) {
      merge_queue_.push({next_tuple, item.file_index, this});
    } else if (rc != RC::RECORD_EOF) {
      LOG_WARN("Failed to read next LightTuple from file %zu: %s", item.file_index, strrc(rc));
      return rc;
    }
  }
  
  return RC::SUCCESS;
}

RC SortPhysicalOperator::initialize_merge_queue()
{
  // priority_queue没有clear方法，需要重新构造
  merge_queue_ = std::priority_queue<MergeItem, vector<MergeItem>, std::greater<MergeItem>>();
  
  for (size_t i = 0; i < file_readers_.size(); i++) {
    LightTuple tuple;
    RC rc = file_readers_[i]->next(tuple);
    if (rc == RC::SUCCESS) {
      merge_queue_.push({tuple, i, this});
    } else if (rc != RC::RECORD_EOF) {
      LOG_WARN("Failed to read first LightTuple from file reader %zu: %s", i, strrc(rc));
      return rc;
    }
  }
  
  return RC::SUCCESS;
}

bool SortPhysicalOperator::has_more_data_in_files()
{
  // 检查优先队列是否为空
  if (!merge_queue_.empty()) {
    return true;
  }
  
  // 检查所有文件读取器是否还有数据
  for (size_t i = 0; i < file_readers_.size(); i++) {
    if (!file_readers_[i]->is_eof()) {
      return true;
    }
  }
  return false;
}

RC SortPhysicalOperator::load_next_chunk_from_file()
{
  if (temp_files_.empty()) {
    LOG_WARN("No temporary files to load");
    return RC::INTERNAL;
  }
  
  // 只处理第一个文件（单文件情况）
  string filename = temp_files_[0];
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    LOG_WARN("Failed to open file: %s", filename.c_str());
    return RC::INTERNAL;
  }
  
  // 直接读取所有LightTuple，直到文件结束
  current_chunk_.clear();
  while (!file.eof()) {
    LightTuple tuple;
    RC rc = deserialize_light_tuple(tuple, file);
    if (rc == RC::RECORD_EOF) {
      break; // 文件结束
    }
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to deserialize LightTuple from file: %s", filename.c_str());
      return rc;
    }
    current_chunk_.push_back(tuple);
  }
  
  file.close();
  current_chunk_pos_ = 0;
  
  return RC::SUCCESS;
}
