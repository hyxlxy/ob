/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include "sql/expr/tuple.h"
#include "sql/operator/physical_operator.h"
#include "storage/record/record_manager.h"
#include "storage/index/ivfflat_index.h"

/**
 * @brief 向量索引扫描物理算子
 * @ingroup PhysicalOperator
 */
class VectorScanPhysicalOperator : public PhysicalOperator
{
public:
  // 向量索引的构造方法
  VectorScanPhysicalOperator(Table *table, std::string table_alias, Index *index, std::vector<float> base_vector,
      size_t limit, ReadWriteMode mode);

  ~VectorScanPhysicalOperator() override = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::VECTOR_INDEX_SCAN; }

  std::string param() const override;

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override;

  void set_predicates(std::vector<std::unique_ptr<Expression>> &&exprs);

private:
  // 与TableScanPhysicalOperator代码相同，可以优化
  RC filter(RowTuple &tuple, bool &result);

private:
  Trx                                         *trx_         = nullptr;
  Table                                       *table_       = nullptr;
  std::string                                  table_alias_;
  Index                                       *index_       = nullptr;
  std::vector<float>                           base_vector_;
  size_t                                       limit_;
  ReadWriteMode                                mode_;
  
  RecordFileHandler                           *record_handler_ = nullptr;
  RowTuple                                     tuple_;
  std::vector<std::unique_ptr<Expression>>     predicates_;
  
  std::vector<RID>                             result_rids_;
  size_t                                       current_index_ = 0;
  bool                                         opened_ = false;
};
