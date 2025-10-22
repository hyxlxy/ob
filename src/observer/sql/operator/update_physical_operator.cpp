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
// removed custom setter; use Table::set_value_to_record for consistency

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
      
      // 设置新值到记录中，使用表实际元数据中的 FieldMeta，避免长度/偏移不一致
      const FieldMeta *real_fm = table_->table_meta().field(field_meta.name());
      if (real_fm == nullptr) {
        LOG_WARN("failed to find field meta by name %s", field_meta.name());
        rollback();
        return RC::SCHEMA_FIELD_NOT_EXIST;
      }
      // 执行期对 TEXT 做真实长度计算与上限校验，并传递纠正后的值
      const Value *pv = &new_value;
      Value adjusted;
      if (real_fm->type() == AttrType::TEXTS && !new_value.is_null()) {
        size_t eff_len = new_value.length();
        if (new_value.attr_type() == AttrType::CHARS && eff_len == 0 && new_value.data() != nullptr) {
          eff_len = ::strlen(new_value.data());
        }
        LOG_DEBUG("UPDATE TEXT field=%s eff_len=%zu attr_type=%d", real_fm->name(), eff_len, (int)new_value.attr_type());
        if (eff_len > 65535) {
          LOG_WARN("TEXT too long on update: %zu > 65535", eff_len);
          rollback();
          return RC::INVALID_ARGUMENT;
        }
        if (new_value.attr_type() == AttrType::CHARS) {
          adjusted.set_string(new_value.data(), static_cast<int>(eff_len));
          pv = &adjusted;
        }
      }
      rc = table_->set_value_to_record(new_record.data(), *pv, real_fm);
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
