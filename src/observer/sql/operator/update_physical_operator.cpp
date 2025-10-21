/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "sql/operator/update_physical_operator.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"
#include "common/log/log.h"
#include "storage/field/field_meta.h"
#include "common/value.h"


/**
 * @brief 设置字段值到记录中
 * @param record 要更新的记录
 * @param field_meta 字段元数据
 * @param value 新值
 */
static RC set_field_value_to_record(Record *record, const FieldMeta &field_meta, const Value &value)
{
  if (!record) {
    return RC::INVALID_ARGUMENT;
  }

  char *record_data = record->data();
  if (!record_data) {
    return RC::INVALID_ARGUMENT;
  }

  // Handle NULL values
  if (value.attr_type() == AttrType::UNDEFINED) {
    if (field_meta.nullable()) {
      // For nullable fields, clear the field and set NULL flag in last byte
      // First clear the entire field
      memset(record_data + field_meta.offset(), 0, field_meta.len());
      // Then set NULL flag at the last byte
      record_data[field_meta.offset() + field_meta.len() - 1] = '1';  // Set NULL flag
    } else {
      LOG_WARN("Cannot set NULL to NOT NULL field: %s", field_meta.name());
      return RC::INVALID_ARGUMENT;
    }
    return RC::SUCCESS;  // Return here! Don't continue to copy data
  }

  // Check type compatibility and convert if necessary
  Value actual_value;
  if (field_meta.type() != value.attr_type()) {
    RC rc = Value::cast_to(value, field_meta.type(), actual_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to cast value for field %s: %s", field_meta.name(), strrc(rc));
      return rc;
    }
  } else {
    actual_value = value;
  }

  size_t       copy_len = field_meta.len();
  const size_t data_len = actual_value.length();

  if (field_meta.type() == AttrType::CHARS || field_meta.type() == AttrType::TEXTS) {
    // For CHARS and TEXT types, we need to clear the entire field first to avoid leftover data
    // Clear the entire field space
    memset(record_data + field_meta.offset(), 0, field_meta.len());
    
    // Calculate how much data we can write
    if (field_meta.nullable()) {
      copy_len = field_meta.len() - 1;  // Reserve last byte for NULL flag
    }
    if (field_meta.type() == AttrType::CHARS) {
      if (copy_len > data_len) {
        copy_len = data_len;
      }
    }
    if (field_meta.type() == AttrType::TEXTS) {
      size_t limit = copy_len;
      if (data_len > limit) {
        LOG_WARN("TEXT data length %zu exceeds limit %zu for field %s",
                 data_len, limit, field_meta.name());
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
      }
      copy_len = data_len;
    }
  } else if (field_meta.type() == AttrType::VECTORS) {
    // For VECTORS type, handle nullable fields specially
    if (field_meta.nullable()) {
      copy_len = field_meta.len() - 1;  // Reserve last byte for NULL flag
    }
    // 对于向量类型，需要检查数据长度是否超过字段定义的长度
    if (data_len > copy_len) {
      LOG_WARN("Vector data length (%zu) exceeds field length (%zu) for field %s",
               data_len, copy_len, field_meta.name());
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }
    // 对于向量类型，使用实际数据长度
    copy_len = data_len;
  } else {
    // For fixed-length types (INT, FLOAT, DATE, etc.), always use full data length
    copy_len = data_len;
  }

  memcpy(record_data + field_meta.offset(), actual_value.data(), copy_len);

  // For nullable fields, set NULL flag to '0' (not NULL)
  if (field_meta.nullable()) {
    if (field_meta.type() == AttrType::CHARS || field_meta.type() == AttrType::TEXTS || 
        field_meta.type() == AttrType::VECTORS) {
      // For variable-length types, NULL flag is at the end
      record_data[field_meta.offset() + field_meta.len() - 1] = '0';
    } else {
      // For fixed-length types, we need to place NULL flag after the data
      // but within the field's allocated space
      if (copy_len < field_meta.len()) {
        record_data[field_meta.offset() + field_meta.len() - 1] = '0';
      }
    }
  }

  return RC::SUCCESS;
}

RC UpdatePhysicalOperator::open(Trx *trx)
{
  if (children_.empty()) {
    return RC::SUCCESS;
  }

  std::unique_ptr<PhysicalOperator> &child = children_[0];

  RC rc = child->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open child operator: %s", strrc(rc));
    return rc;
  }

  trx_ = trx;

  // 收集需要更新的记录
  while (OB_SUCC(rc = child->next())) {
    Tuple *tuple = child->current_tuple();
    if (nullptr == tuple) {
      LOG_WARN("failed to get current record: %s", strrc(rc));
      return rc;
    }

    RowTuple *row_tuple = static_cast<RowTuple *>(tuple);
    Record   &record    = row_tuple->record();
    records_.emplace_back(record);
  }

  child->close();

  if (rc != RC::RECORD_EOF) {
    LOG_WARN("failed to get records from child operator: %s", strrc(rc));
    return rc;
  }

  // 执行更新操作
  for (Record &old_record : records_) {
    // 创建新记录并复制原记录数据
    Record new_record;
    rc = new_record.copy_data(old_record.data(), old_record.len());
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to copy record data: %s", strrc(rc));
      rollback();
      return rc;
    }
    new_record.set_rid(old_record.rid());
    
    // 更新记录的字段值
    for (size_t i = 0; i < field_metas_.size(); i++) {
      const FieldMeta &field_meta = field_metas_[i];
      
      // 使用原始记录计算新值（因为表达式可能依赖原始值）
      RowTuple tuple;
      tuple.set_record(&old_record);
      tuple.set_schema(table_, table_->table_meta().field_metas());
      
      Value new_value;
      rc = values_[i]->get_value(tuple, new_value);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get new value for field %s: %s", field_meta.name(), strrc(rc));
        rollback();
        return rc;
      }
      
      // 设置新值到记录中
      rc = set_field_value_to_record(&new_record, field_meta, new_value);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to set field value for field %s: %s", field_meta.name(), strrc(rc));
        rollback();
        return rc;
      }
    }
    
    // 保存更新日志
    log_records_.emplace_back(old_record, new_record);

    // 执行更新
    rc = trx_->update_record(table_, old_record, new_record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to update record: %s", strrc(rc));
      rollback();
      return rc;
    }
  }

  return RC::SUCCESS;
}

RC UpdatePhysicalOperator::next()
{
  return RC::RECORD_EOF;
}

RC UpdatePhysicalOperator::close()
{
  return RC::SUCCESS;
}

void UpdatePhysicalOperator::rollback()
{
  // 回滚已经更新的记录
  for (auto &log_record : log_records_) {
    // Record &old_record = log_record.first;
    // 这里应该调用事务的回滚方法，但为了简化，我们暂时不实现
    // trx_->rollback_update(table_, old_record);
    (void)log_record; // 避免未使用变量警告
  }
  log_records_.clear();
}
