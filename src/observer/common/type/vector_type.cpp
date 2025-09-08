/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/type/vector_type.h"
#include "common/value.h"
#include "common/log/log.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <numeric>

using namespace std;

std::vector<float> VectorType::get_vector_data(const Value &val)
{
  std::vector<float> result;
  if (val.attr_type() != AttrType::VECTORS || val.data() == nullptr) {
    return result;
  }

  // 向量数据存储格式：[dimension][float1][float2]...[floatN]
  const char *data = val.data();
  int dimension = *(int*)data;
  const float *float_data = (const float*)(data + sizeof(int));
  
  result.reserve(dimension);
  for (int i = 0; i < dimension; i++) {
    result.push_back(float_data[i]);
  }
  
  return result;
}

void VectorType::set_vector_data(Value &val, const std::vector<float> &vec)
{
  int dimension = vec.size();

  // 检查维度限制
  if (dimension > MAX_VECTOR_DIMENSION) {
    LOG_WARN("Vector dimension exceeds maximum limit of %d: %d", MAX_VECTOR_DIMENSION, dimension);
    return;
  }

  int total_size = sizeof(int) + dimension * sizeof(float);

  char *data = new char[total_size];
  *(int*)data = dimension;
  float *float_data = (float*)(data + sizeof(int));

  for (int i = 0; i < dimension; i++) {
    float_data[i] = vec[i];
  }

  val.set_type(AttrType::VECTORS);
  val.set_data(data, total_size);
  delete[] data; // Value会复制数据
}

RC VectorType::parse_vector_string(const string &str, std::vector<float> &vec)
{
  vec.clear();

  // 去除首尾空格
  string trimmed = str;
  trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
  trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

  // 检查格式：应该以[开头，]结尾
  if (trimmed.empty() || trimmed[0] != '[' || trimmed.back() != ']') {
    LOG_WARN("Invalid vector format: %s", str.c_str());
    return RC::INVALID_ARGUMENT;
  }

  // 提取中间的数字部分
  string content = trimmed.substr(1, trimmed.length() - 2);
  if (content.empty()) {
    return RC::SUCCESS; // 空向量
  }

  // 预分配内存以提高性能
  // 估算向量维度：按逗号数量+1估算
  size_t comma_count = std::count(content.begin(), content.end(), ',');
  size_t estimated_size = comma_count + 1;
  vec.reserve(std::min(estimated_size, static_cast<size_t>(MAX_VECTOR_DIMENSION)));

  // 按逗号分割
  stringstream ss(content);
  string item;

  while (getline(ss, item, ',')) {
    // 去除空格
    item.erase(0, item.find_first_not_of(" \t\n\r"));
    item.erase(item.find_last_not_of(" \t\n\r") + 1);

    if (item.empty()) {
      continue;
    }

    try {
      float value = stof(item);
      vec.push_back(value);

      // 检查维度限制
      if (vec.size() > MAX_VECTOR_DIMENSION) {
        LOG_WARN("Vector dimension exceeds maximum limit of %d: %zu", MAX_VECTOR_DIMENSION, vec.size());
        return RC::INVALID_ARGUMENT;
      }
    } catch (const exception &e) {
      LOG_WARN("Invalid number in vector: %s", item.c_str());
      return RC::INVALID_ARGUMENT;
    }
  }

  return RC::SUCCESS;
}

RC VectorType::vector_to_string(const std::vector<float> &vec, string &result)
{
  stringstream ss;
  ss << "[";
  
  for (size_t i = 0; i < vec.size(); i++) {
    if (i > 0) {
      ss << ",";
    }
    // 保留2位小数，去掉多余的0
    ss << fixed << setprecision(2) << vec[i];
    string num_str = ss.str().substr(ss.str().find_last_of(',') + 1);
    if (i == 0) {
      num_str = ss.str();
    }
    
    // 去掉末尾的0和小数点
    size_t pos = num_str.find_last_not_of('0');
    if (pos != string::npos && num_str[pos] == '.') {
      pos--;
    }
    if (i == 0) {
      ss.str("");
      ss << "[" << num_str.substr(1, pos);
    }
  }
  
  // 重新格式化，确保正确的小数位数
  ss.str("");
  ss << "[";
  for (size_t i = 0; i < vec.size(); i++) {
    if (i > 0) {
      ss << ",";
    }
    
    // 格式化数字：保留2位小数，去掉多余的0
    stringstream num_ss;
    num_ss << fixed << setprecision(2) << vec[i];
    string num_str = num_ss.str();
    
    // 去掉末尾的0
    size_t pos = num_str.find_last_not_of('0');
    if (pos != string::npos && num_str[pos] == '.') {
      pos--;
    }
    num_str = num_str.substr(0, pos + 1);
    
    ss << num_str;
  }
  ss << "]";
  
  result = ss.str();
  return RC::SUCCESS;
}

int VectorType::compare(const Value &left, const Value &right) const
{
  std::vector<float> left_vec = get_vector_data(left);
  std::vector<float> right_vec = get_vector_data(right);

  // 首先比较维度
  if (left_vec.size() != right_vec.size()) {
    return left_vec.size() < right_vec.size() ? -1 : 1;
  }

  // 对于高维向量，使用更高效的比较方法
  size_t dimension = left_vec.size();
  if (dimension > 1000) {
    // 高维向量：先比较前几个维度，如果相等再全量比较
    size_t sample_size = min(dimension, static_cast<size_t>(100));
    for (size_t i = 0; i < sample_size; i++) {
      if (left_vec[i] < right_vec[i]) {
        return -1;
      } else if (left_vec[i] > right_vec[i]) {
        return 1;
      }
    }

    // 如果采样相等，进行全量比较
    for (size_t i = sample_size; i < dimension; i++) {
      if (left_vec[i] < right_vec[i]) {
        return -1;
      } else if (left_vec[i] > right_vec[i]) {
        return 1;
      }
    }
  } else {
    // 低维向量：直接字典序比较
    for (size_t i = 0; i < dimension; i++) {
      if (left_vec[i] < right_vec[i]) {
        return -1;
      } else if (left_vec[i] > right_vec[i]) {
        return 1;
      }
    }
  }

  return 0;
}

RC VectorType::add(const Value &left, const Value &right, Value &result) const
{
  std::vector<float> left_vec = get_vector_data(left);
  std::vector<float> right_vec = get_vector_data(right);
  
  if (left_vec.size() != right_vec.size()) {
    LOG_WARN("Vector dimensions mismatch: %zu vs %zu", left_vec.size(), right_vec.size());
    return RC::INVALID_ARGUMENT;
  }
  
  std::vector<float> result_vec;
  result_vec.reserve(left_vec.size());
  
  for (size_t i = 0; i < left_vec.size(); i++) {
    result_vec.push_back(left_vec[i] + right_vec[i]);
  }
  
  set_vector_data(result, result_vec);
  return RC::SUCCESS;
}

RC VectorType::subtract(const Value &left, const Value &right, Value &result) const
{
  std::vector<float> left_vec = get_vector_data(left);
  std::vector<float> right_vec = get_vector_data(right);
  
  if (left_vec.size() != right_vec.size()) {
    LOG_WARN("Vector dimensions mismatch: %zu vs %zu", left_vec.size(), right_vec.size());
    return RC::INVALID_ARGUMENT;
  }
  
  std::vector<float> result_vec;
  result_vec.reserve(left_vec.size());
  
  for (size_t i = 0; i < left_vec.size(); i++) {
    result_vec.push_back(left_vec[i] - right_vec[i]);
  }
  
  set_vector_data(result, result_vec);
  return RC::SUCCESS;
}

RC VectorType::multiply(const Value &left, const Value &right, Value &result) const
{
  std::vector<float> left_vec = get_vector_data(left);
  std::vector<float> right_vec = get_vector_data(right);
  
  if (left_vec.size() != right_vec.size()) {
    LOG_WARN("Vector dimensions mismatch: %zu vs %zu", left_vec.size(), right_vec.size());
    return RC::INVALID_ARGUMENT;
  }
  
  std::vector<float> result_vec;
  result_vec.reserve(left_vec.size());
  
  for (size_t i = 0; i < left_vec.size(); i++) {
    result_vec.push_back(left_vec[i] * right_vec[i]);
  }
  
  set_vector_data(result, result_vec);
  return RC::SUCCESS;
}

RC VectorType::divide(const Value &left, const Value &right, Value &result) const
{
  std::vector<float> left_vec = get_vector_data(left);
  std::vector<float> right_vec = get_vector_data(right);

  if (left_vec.size() != right_vec.size()) {
    LOG_WARN("Vector dimensions mismatch: %zu vs %zu", left_vec.size(), right_vec.size());
    return RC::INVALID_ARGUMENT;
  }

  std::vector<float> result_vec;
  result_vec.reserve(left_vec.size());

  for (size_t i = 0; i < left_vec.size(); i++) {
    if (right_vec[i] == 0.0f) {
      LOG_WARN("Division by zero in vector element %zu", i);
      return RC::INVALID_ARGUMENT;
    }
    result_vec.push_back(left_vec[i] / right_vec[i]);
  }

  set_vector_data(result, result_vec);
  return RC::SUCCESS;
}

RC VectorType::negative(const Value &val, Value &result) const
{
  std::vector<float> vec = get_vector_data(val);
  std::vector<float> result_vec;
  result_vec.reserve(vec.size());

  for (size_t i = 0; i < vec.size(); i++) {
    result_vec.push_back(-vec[i]);
  }

  set_vector_data(result, result_vec);
  return RC::SUCCESS;
}

RC VectorType::cast_to(const Value &val, AttrType type, Value &result) const
{
  if (type == AttrType::VECTORS) {
    result.set_value(val);
    return RC::SUCCESS;
  }
  
  if (type == AttrType::CHARS) {
    std::vector<float> vec = get_vector_data(val);
    string str_result;
    RC rc = vector_to_string(vec, str_result);
    if (rc == RC::SUCCESS) {
      result.set_string(str_result.c_str());
    }
    return rc;
  }
  
  return RC::UNIMPLEMENTED;
}

RC VectorType::set_value_from_str(Value &val, const string &data) const
{
  std::vector<float> vec;
  RC rc = parse_vector_string(data, vec);
  if (rc != RC::SUCCESS) {
    return rc;
  }
  
  set_vector_data(val, vec);
  return RC::SUCCESS;
}

RC VectorType::to_string(const Value &val, string &result) const
{
  std::vector<float> vec = get_vector_data(val);
  return vector_to_string(vec, result);
}

int VectorType::cast_cost(AttrType type)
{
  if (type == AttrType::VECTORS) {
    return 0;
  }
  if (type == AttrType::CHARS) {
    return 1;
  }
  return INT32_MAX;
}

RC VectorType::l2_distance(const Value &left, const Value &right, Value &result)
{
  std::vector<float> left_vec = get_vector_data(left);
  std::vector<float> right_vec = get_vector_data(right);

  if (left_vec.size() != right_vec.size()) {
    LOG_WARN("Vector dimensions mismatch for L2 distance: %zu vs %zu", left_vec.size(), right_vec.size());
    return RC::INVALID_ARGUMENT;
  }

  size_t dimension = left_vec.size();
  float sum = 0.0f;

  // 对于高维向量，使用更高效的计算方法
  if (dimension > 1000) {
    // 高维向量：使用分块计算减少缓存未命中
    const size_t block_size = 64;
    for (size_t block = 0; block < dimension; block += block_size) {
      size_t end = min(block + block_size, dimension);
      for (size_t i = block; i < end; i++) {
        float diff = left_vec[i] - right_vec[i];
        sum += diff * diff;
      }
    }
  } else {
    // 低维向量：直接计算
    for (size_t i = 0; i < dimension; i++) {
      float diff = left_vec[i] - right_vec[i];
      sum += diff * diff;
    }
  }

  float distance = sqrt(sum);

  // 保留2位小数
  distance = round(distance * 100.0f) / 100.0f;

  result.set_float(distance);
  return RC::SUCCESS;
}

RC VectorType::cosine_distance(const Value &left, const Value &right, Value &result)
{
  std::vector<float> left_vec = get_vector_data(left);
  std::vector<float> right_vec = get_vector_data(right);

  if (left_vec.size() != right_vec.size()) {
    LOG_WARN("Vector dimensions mismatch for cosine distance: %zu vs %zu", left_vec.size(), right_vec.size());
    return RC::INVALID_ARGUMENT;
  }

  float dot_product = 0.0f;
  float left_norm = 0.0f;
  float right_norm = 0.0f;

  for (size_t i = 0; i < left_vec.size(); i++) {
    dot_product += left_vec[i] * right_vec[i];
    left_norm += left_vec[i] * left_vec[i];
    right_norm += right_vec[i] * right_vec[i];
  }

  left_norm = sqrt(left_norm);
  right_norm = sqrt(right_norm);

  if (left_norm == 0.0f || right_norm == 0.0f) {
    result.set_float(1.0f); // 如果有零向量，余弦距离为1
    return RC::SUCCESS;
  }

  float cosine_similarity = dot_product / (left_norm * right_norm);
  float distance = 1.0f - cosine_similarity;

  // 保留2位小数
  distance = round(distance * 100.0f) / 100.0f;

  result.set_float(distance);
  return RC::SUCCESS;
}

RC VectorType::inner_product(const Value &left, const Value &right, Value &result)
{
  std::vector<float> left_vec = get_vector_data(left);
  std::vector<float> right_vec = get_vector_data(right);

  if (left_vec.size() != right_vec.size()) {
    LOG_WARN("Vector dimensions mismatch for inner product: %zu vs %zu", left_vec.size(), right_vec.size());
    return RC::INVALID_ARGUMENT;
  }

  float product = 0.0f;
  for (size_t i = 0; i < left_vec.size(); i++) {
    product += left_vec[i] * right_vec[i];
  }

  // 保留2位小数
  product = round(product * 100.0f) / 100.0f;

  result.set_float(product);
  return RC::SUCCESS;
}
