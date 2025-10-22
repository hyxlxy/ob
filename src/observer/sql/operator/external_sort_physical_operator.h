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

#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "common/sys/rc.h"
#include "sql/expr/expression.h"
#include "sql/expr/tuple.h"
#include "sql/operator/physical_operator.h"
#include "sql/parser/parse_defs.h"

class ExternalSortPhysicalOperator : public PhysicalOperator
{
public:
  ExternalSortPhysicalOperator(std::vector<std::unique_ptr<Expression>> order_by_expressions,
                               std::vector<OrderDirection> order_by_directions,
                               size_t memory_limit_bytes = 300ULL * 1024ULL * 1024ULL);
  virtual ~ExternalSortPhysicalOperator();

  PhysicalOperatorType type() const override { return PhysicalOperatorType::ORDER_BY; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override;
  RC tuple_schema(TupleSchema &schema) const override;

  void set_top_k(size_t k) { top_k_ = k; }

private:
  struct RunFile { std::string path; std::FILE *fp = nullptr; };
  struct HeapItem {
    size_t run_index;
    std::unique_ptr<ValueListTuple> tuple;
    HeapItem() = default;
    HeapItem(const HeapItem &) = delete;
    HeapItem &operator=(const HeapItem &) = delete;
    HeapItem(HeapItem &&) = default;
    HeapItem &operator=(HeapItem &&) = default;
  };
  struct HeapComparator {
    std::vector<std::unique_ptr<Expression>> *order_exprs;
    std::vector<OrderDirection> *dirs;
    bool operator()(const HeapItem &a, const HeapItem &b) const;
  };

  RC produce_all();
  RC flush_run();
  RC finalize_runs();
  RC open_runs_for_merge();
  RC read_next_from_run(size_t run_index, std::unique_ptr<ValueListTuple> &out);
  void cleanup_runs();
  bool tuple_less(const ValueListTuple &left, const ValueListTuple &right) const;
  static size_t approximate_tuple_size(const ValueListTuple &t);
  static RC serialize_tuple(std::FILE *fp, const ValueListTuple &t);
  static RC deserialize_tuple(std::FILE *fp, std::unique_ptr<ValueListTuple> &out);
  RC prepare_temp_dir();

private:
  std::vector<std::unique_ptr<Expression>> order_by_expressions_;
  std::vector<OrderDirection>              order_by_directions_;
  std::vector<ValueListTuple>              buffer_;
  size_t                                  buffer_bytes_used_ = 0;
  std::vector<RunFile>                     runs_;
  std::string                              temp_dir_;
  bool                                     merge_mode_ = false;
  std::priority_queue<HeapItem, std::vector<HeapItem>, HeapComparator> heap_;
  bool                                     opened_ = false;
  size_t                                   mem_cursor_ = 0;
  std::unique_ptr<ValueListTuple>          current_;
  TupleSchema                              tuple_schema_;
  size_t                                   memory_limit_bytes_ = 300ULL * 1024ULL * 1024ULL;
  size_t                                   top_k_ = 0;
};


