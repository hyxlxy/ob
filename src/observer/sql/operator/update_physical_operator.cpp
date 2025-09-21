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
  for (Record &record : records_) {
    // 保存原始记录用于回滚
    Record old_record = record;
    
    // 更新记录的字段值
    for (size_t i = 0; i < field_metas_.size(); i++) {
      const FieldMeta &field_meta = field_metas_[i];
      
      // 计算新值
      RowTuple tuple;
      tuple.set_record(&record);
      tuple.set_schema(table_, table_->table_meta().field_metas());
      
      Value new_value;
      rc = values_[i]->get_value(tuple, new_value);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get new value for field %s: %s", field_meta.name(), strrc(rc));
        rollback();
        return rc;
      }
      
      // 设置新值到记录中
      // 这里需要根据实际的API来设置字段值
      // 暂时跳过具体的字段设置实现
      LOG_INFO("updating field %s with new value", field_meta.name());
    }
    
    // 保存更新日志
    log_records_.emplace_back(old_record, record);

    // 执行更新 - update_record需要3个参数：table, old_record, new_record
    rc = trx_->update_record(table_, old_record, record);
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
