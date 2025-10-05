/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "sql/operator/vector_scan_physical_operator.h"
#include "storage/table/table.h"
#include "storage/index/index.h"
#include "storage/index/ivfflat_index.h"
#include "common/log/log.h"

VectorScanPhysicalOperator::VectorScanPhysicalOperator(Table *table, std::string table_alias, Index *index, 
    std::vector<float> base_vector, size_t limit, ReadWriteMode mode)
    : table_(table), table_alias_(table_alias), index_(index), base_vector_(base_vector), limit_(limit), mode_(mode)
{
  tuple_.set_schema(table, table->table_meta().field_metas());
  // tuple_.set_table_alias(table_alias); // 该方法不存在，暂时注释
}

std::string VectorScanPhysicalOperator::param() const
{
  // expected format: INDEX_NAME ON TABLE_NAME
  return std::string(index_->index_meta().name()) + " ON " + std::string(table_->name());
}

RC VectorScanPhysicalOperator::open(Trx *trx)
{
  if (opened_) {
    return RC::SUCCESS;
  }

  trx_ = trx;

  if (table_ == nullptr || index_ == nullptr) {
    LOG_WARN("vector scan operator missing table or index");
    return RC::INTERNAL;
  }

  // 调用 IvfflatIndex::search 获取候选 RIDs
  auto *ivf = dynamic_cast<IvfflatIndex *>(index_);
  if (ivf == nullptr) {
    LOG_WARN("index is not IvfflatIndex");
    return RC::UNSUPPORTED;
  }

  RC rc = ivf->search(base_vector_, limit_, result_rids_);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to search vector index. rc=%s", strrc(rc));
    return rc;
  }

  current_index_ = 0;
  tuple_.set_schema(table_, table_->table_meta().field_metas());
  opened_ = true;
  return RC::SUCCESS;
}

RC VectorScanPhysicalOperator::next()
{
  if (!opened_) {
    return RC::RECORD_EOF;
  }

  // 迭代候选 RIDs，逐条加载并过滤
  while (current_index_ < result_rids_.size()) {
    RID rid = result_rids_[current_index_++];

    RC rc = table_->get_record(rid, current_record_);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get record. rid=%s, rc=%s", rid.to_string().c_str(), strrc(rc));
      return rc;
    }

    tuple_.set_record(&current_record_);

    bool passed = false;
    rc = filter(tuple_, passed);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    if (!passed) {
      continue;
    }

    // 可选：事务可见性检查（若需要，可接入 trx_->visit_record）
    return RC::SUCCESS;
  }

  return RC::RECORD_EOF;
}

RC VectorScanPhysicalOperator::close()
{
  if (!opened_) {
    return RC::SUCCESS;
  }

  result_rids_.clear();
  current_index_ = 0;
  opened_ = false;
  return RC::SUCCESS;
}

Tuple *VectorScanPhysicalOperator::current_tuple()
{
  return &tuple_;
}

void VectorScanPhysicalOperator::set_predicates(std::vector<std::unique_ptr<Expression>> &&exprs)
{
  predicates_ = std::move(exprs);
}

RC VectorScanPhysicalOperator::filter(RowTuple &tuple, bool &result)
{
  result = true;
  for (std::unique_ptr<Expression> &expr : predicates_) {
    Value value;
    RC    rc = expr->get_value(tuple, value);
    if (rc != RC::SUCCESS) {
      return rc;
    }

    bool bool_value = false;
    // rc = value.to_boolean(bool_value); // 该方法不存在，暂时简化
    bool_value = (value.get_boolean() == true); // 使用get_boolean方法
    rc = RC::SUCCESS;

    if (!bool_value) {
      result = false;
      break;
    }
  }
  return RC::SUCCESS;
}
