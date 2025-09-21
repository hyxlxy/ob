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

#include "common/sys/rc.h"
#include "sql/operator/physical_operator.h"
#include "storage/record/record_manager.h"
#include "sql/expr/expression_tuple.h"
#include "storage/table/view.h"

/**
 * @brief 视图扫描物理算子
 * @ingroup PhysicalOperator
 */
class ViewScanPhysicalOperator : public PhysicalOperator
{
public:
  ViewScanPhysicalOperator(View *view, std::string alias) : view_(view) {
    // tuple_.set_table_alias(alias); // 该方法不存在，暂时注释
  }

  ~ViewScanPhysicalOperator() override = default;

  std::string param() const override;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::VIEW_SCAN; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override;

  void set_predicates(std::vector<std::unique_ptr<Expression>> &&exprs);

private:
  RC filter(RowTuple &tuple, bool &result);

  RC init();

  // 把查询的结果转为当前视图的记录，就相当于从视图表中取出来一个记录
  RC next_record();

private:
  View                                        *view_ = nullptr;
  Trx                                         *trx_  = nullptr;
  std::vector<std::unique_ptr<Expression>>     predicates_;
  RowTuple                                     tuple_;
  std::unique_ptr<PhysicalOperator>            child_operator_;
  bool                                         opened_ = false;
};
