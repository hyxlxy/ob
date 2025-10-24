// duplicate section removed

/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "sql/operator/external_sort_physical_operator.h"

#include <algorithm>
#include <cstring>
#include <filesystem>

#include "common/log/log.h"
#include "event/sql_debug.h"

using namespace std;
using namespace std::filesystem;

ExternalSortPhysicalOperator::ExternalSortPhysicalOperator(vector<unique_ptr<Expression>> order_by_expressions,
    vector<OrderDirection> order_by_directions, size_t memory_limit_bytes)
    : order_by_expressions_(std::move(order_by_expressions)),
      order_by_directions_(std::move(order_by_directions)),
      memory_limit_bytes_(memory_limit_bytes)
{}

ExternalSortPhysicalOperator::~ExternalSortPhysicalOperator() { cleanup_runs(); }

RC ExternalSortPhysicalOperator::open(Trx *trx)
{
  if (opened_) {
    return RC::SUCCESS;
  }

  if (children_.empty()) {
    LOG_WARN("external sort operator has no child");
    return RC::INTERNAL;
  }

  RC rc = children_[0]->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open child operator: %s", strrc(rc));
    return rc;
  }

  rc = children_[0]->tuple_schema(tuple_schema_);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get tuple schema from child: %s", strrc(rc));
    return rc;
  }

  sql_debug("EXTSORT open mem_limit=%zu", memory_limit_bytes_);
  rc = produce_all();
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to produce sorted output: %s", strrc(rc));
    return rc;
  }

  opened_ = true;
  return RC::SUCCESS;
}

RC ExternalSortPhysicalOperator::next()
{
  if (!opened_) {
    return RC::RECORD_EOF;
  }

  if (!merge_mode_) {
    if (mem_cursor_ >= buffer_.size()) {
      return RC::RECORD_EOF;
    }
    current_ = make_unique<ValueListTuple>(buffer_[mem_cursor_]);
    // Ensure specs are present
    if (current_->cell_num() > 0) {
      std::vector<TupleCellSpec> specs;
      specs.reserve(tuple_schema_.cell_num());
      for (int j = 0; j < tuple_schema_.cell_num(); j++) {
        specs.push_back(tuple_schema_.cell_at(j));
      }
      current_->set_names(specs);
    }
    mem_cursor_++;
    return RC::SUCCESS;
  }

  if (heap_.empty()) {
    return RC::RECORD_EOF;
  }

  // pop smallest and refill from that run
  // Extract without copying (top() is const-ref)
  size_t run_index = heap_.top().run_index;
  current_         = std::move(const_cast<HeapItem &>(heap_.top()).tuple);
  heap_.pop();
  // Ensure specs are present
  if (current_ && current_->cell_num() > 0) {
    std::vector<TupleCellSpec> specs;
    specs.reserve(tuple_schema_.cell_num());
    for (int j = 0; j < tuple_schema_.cell_num(); j++) {
      specs.push_back(tuple_schema_.cell_at(j));
    }
    current_->set_names(specs);
  }

  std::unique_ptr<ValueListTuple> repl;
  RC                              rc = read_next_from_run(run_index, repl);
  if (rc == RC::SUCCESS && repl) {
    HeapItem item;
    item.run_index = run_index;
    item.tuple     = std::move(repl);
    heap_.push(std::move(item));
  }
  return RC::SUCCESS;
}

RC ExternalSortPhysicalOperator::close()
{
  cleanup_runs();
  buffer_.clear();
  buffer_bytes_used_ = 0;
  current_.reset();
  mem_cursor_ = 0;
  merge_mode_ = false;

  if (!children_.empty()) {
    return children_[0]->close();
  }
  return RC::SUCCESS;
}

Tuple *ExternalSortPhysicalOperator::current_tuple() { return current_.get(); }

RC ExternalSortPhysicalOperator::tuple_schema(TupleSchema &schema) const
{
  schema = tuple_schema_;
  return RC::SUCCESS;
}

bool ExternalSortPhysicalOperator::HeapComparator::operator()(const HeapItem &a, const HeapItem &b) const
{
  // max-heap by default; invert comparison to get min-heap behavior
  // return true if a should come after b
  // so we use left<right to mean a has higher priority -> return false
  // Here, for priority_queue (largest first), we want smallest first, so return tuple_less(b,a)
  // Implement via temporary ValueListTuple comparison using order expressions
  const ValueListTuple &ta = *a.tuple;
  const ValueListTuple &tb = *b.tuple;

  // compare tb < ta  => if true, tb should come first, so return true (a after b)
  // We'll re-use ExternalSortPhysicalOperator::tuple_less logic pattern locally
  size_t n = std::min(order_exprs->size(), dirs->size());
  for (size_t i = 0; i < n; i++) {
    Value va, vb;
    (*order_exprs)[i]->get_value(ta, va);
    (*order_exprs)[i]->get_value(tb, vb);
    int cmp = va.compare(vb);
    if (cmp != 0) {
      bool asc = (*dirs)[i] == ASC_ORDER;
      // want min-heap: return true if (tb before ta)
      if (asc)
        return cmp > 0;  // tb<ta => cmp<0 => return false; if cmp>0 then tb>ta so a should come before b => return true
      return cmp < 0;
    }
  }
  return false;
}

bool ExternalSortPhysicalOperator::tuple_less(const ValueListTuple &left, const ValueListTuple &right) const
{
  size_t n = std::min(order_by_expressions_.size(), order_by_directions_.size());
  for (size_t i = 0; i < n; i++) {
    Value lv, rv;
    order_by_expressions_[i]->get_value(left, lv);
    order_by_expressions_[i]->get_value(right, rv);
    int cmp = lv.compare(rv);
    if (cmp != 0) {
      if (order_by_directions_[i] == ASC_ORDER)
        return cmp < 0;
      return cmp > 0;
    }
  }
  return false;
}

size_t ExternalSortPhysicalOperator::approximate_tuple_size(const ValueListTuple &t)
{
  // rough estimation: all cell payloads + small overhead per cell
  size_t total = 0;
  for (int i = 0; i < t.cell_num(); i++) {
    Value v;
    if (t.cell_at(i, v) == RC::SUCCESS) {
      total += v.length() + 8;  // 8 bytes overhead per cell
    } else {
      total += 8;
    }
  }
  return total + 32;  // tuple header
}

RC ExternalSortPhysicalOperator::serialize_tuple(std::FILE *fp, const ValueListTuple &t)
{
  // simple length-prefixed encoding by cells
  uint32_t cell_count = static_cast<uint32_t>(t.cell_num());
  if (fwrite(&cell_count, sizeof(cell_count), 1, fp) != 1)
    return RC::IOERR_WRITE;
  for (uint32_t i = 0; i < cell_count; i++) {
    Value v;
    RC    rc = const_cast<ValueListTuple &>(t).cell_at(static_cast<int>(i), v);
    if (rc != RC::SUCCESS)
      return rc;
    uint32_t type = static_cast<uint32_t>(v.attr_type());
    uint32_t len  = static_cast<uint32_t>(v.length());
    if (fwrite(&type, sizeof(type), 1, fp) != 1)
      return RC::IOERR_WRITE;
    if (fwrite(&len, sizeof(len), 1, fp) != 1)
      return RC::IOERR_WRITE;
    if (len > 0) {
      if (fwrite(v.data(), 1, len, fp) != len)
        return RC::IOERR_WRITE;
    }
    // NULL flag is embedded in Value (UNDEFINED->null) via length==0 for fixed types; already carried by type/len/data
    // semantics
  }
  return RC::SUCCESS;
}

RC ExternalSortPhysicalOperator::deserialize_tuple(std::FILE *fp, std::unique_ptr<ValueListTuple> &out)
{
  uint32_t cell_count = 0;
  if (fread(&cell_count, sizeof(cell_count), 1, fp) != 1) {
    if (feof(fp))
      return RC::RECORD_EOF;
    return RC::IOERR_READ;
  }
  std::vector<Value> cells;
  cells.reserve(cell_count);
  for (uint32_t i = 0; i < cell_count; i++) {
    uint32_t type = 0, len = 0;
    if (fread(&type, sizeof(type), 1, fp) != 1)
      return RC::IOERR_READ;
    if (fread(&len, sizeof(len), 1, fp) != 1)
      return RC::IOERR_READ;
    Value v;
    v.set_type(static_cast<AttrType>(type));
    if (len > 0) {
      std::string buf;
      buf.resize(len);
      if (fread(buf.data(), 1, len, fp) != len)
        return RC::IOERR_READ;
      // For string-like types, prefer set_string to avoid ambiguity
      if (v.attr_type() == AttrType::CHARS || v.attr_type() == AttrType::TEXTS) {
        v.set_string(buf.data(), len);
      } else {
        // Use const char* overload
        v.set_data(buf.c_str(), static_cast<int>(len));
      }
    } else {
      // length 0 -> may be null/fixed zero
      if (v.attr_type() == AttrType::UNDEFINED) {
        v.set_type(AttrType::UNDEFINED);
        v.set_null(true);
      } else {
        v.set_data((const char *)nullptr, 0);
      }
    }
    cells.emplace_back(v);
  }
  auto t = make_unique<ValueListTuple>();
  t->set_cells(cells);
  out = std::move(t);
  return RC::SUCCESS;
}

RC ExternalSortPhysicalOperator::prepare_temp_dir()
{
  // place under data/miniob/tmp if possible; fallback to current directory
  try {
    path base = current_path();
    // Don't depend on absolute deploy layout; create a local temp directory
    temp_dir_ = (base / "external_sort_tmp").string();
    create_directories(temp_dir_);
  } catch (...) {
    LOG_WARN("failed to prepare temp directory");
    return RC::IOERR_OPEN;
  }
  return RC::SUCCESS;
}

RC ExternalSortPhysicalOperator::flush_run()
{
  if (buffer_.empty())
    return RC::SUCCESS;
  size_t run_rows_before_cut = buffer_.size();
  if (top_k_ > 0 && buffer_.size() > top_k_) {
    // Partial select smallest top_k_ in buffer to reduce spill size
    std::nth_element(buffer_.begin(),
        buffer_.begin() + static_cast<long>(top_k_),
        buffer_.end(),
        [&](const ValueListTuple &a, const ValueListTuple &b) { return tuple_less(a, b); });
    buffer_.resize(top_k_);
  }
  std::sort(buffer_.begin(), buffer_.end(), [&](const ValueListTuple &a, const ValueListTuple &b) {
    return tuple_less(a, b);
  });

  if (temp_dir_.empty()) {
    RC rc = prepare_temp_dir();
    if (rc != RC::SUCCESS)
      return rc;
  }

  char fname[256];
  snprintf(fname, sizeof(fname), "%s/run_%zu.bin", temp_dir_.c_str(), runs_.size());
  std::FILE *fp = fopen(fname, "wb");
  if (!fp) {
    LOG_WARN("failed to open run file %s", fname);
    return RC::IOERR_OPEN;
  }

  for (const auto &t : buffer_) {
    RC rc = serialize_tuple(fp, t);
    if (rc != RC::SUCCESS) {
      fclose(fp);
      return rc;
    }
  }
  fclose(fp);
  runs_.push_back(RunFile{fname, nullptr});
  sql_debug("EXTSORT flushed run_idx=%zu rows=%zu", runs_.size() - 1, run_rows_before_cut);
  buffer_.clear();
  buffer_bytes_used_ = 0;
  return RC::SUCCESS;
}

RC ExternalSortPhysicalOperator::finalize_runs()
{
  if (runs_.empty()) {
    // all data in-memory: keep buffer_ sorted order
    if (top_k_ > 0 && buffer_.size() > top_k_) {
      std::nth_element(buffer_.begin(),
          buffer_.begin() + static_cast<long>(top_k_),
          buffer_.end(),
          [&](const ValueListTuple &a, const ValueListTuple &b) { return tuple_less(a, b); });
      buffer_.resize(top_k_);
    }
    std::sort(buffer_.begin(), buffer_.end(), [&](const ValueListTuple &a, const ValueListTuple &b) {
      return tuple_less(a, b);
    });
    // ensure specs present for ValueListTuple in-memory output
    std::vector<TupleCellSpec> specs;
    specs.reserve(tuple_schema_.cell_num());
    for (int j = 0; j < tuple_schema_.cell_num(); j++) {
      specs.push_back(tuple_schema_.cell_at(j));
    }
    for (auto &t : buffer_) {
      t.set_names(specs);
    }
    merge_mode_ = false;
    mem_cursor_ = 0;
    sql_debug("EXTSORT finalize in_mem rows=%zu", buffer_.size());
    return RC::SUCCESS;
  }
  // there are run files: switch to merge mode
  merge_mode_ = true;
  sql_debug("EXTSORT finalize merge runs=%zu", runs_.size());
  return open_runs_for_merge();
}

RC ExternalSortPhysicalOperator::open_runs_for_merge()
{
  // open each run file for reading and push first record to heap
  heap_ = std::priority_queue<HeapItem, std::vector<HeapItem>, HeapComparator>(
      HeapComparator{&order_by_expressions_, &order_by_directions_});

  for (size_t i = 0; i < runs_.size(); i++) {
    runs_[i].fp = fopen(runs_[i].path.c_str(), "rb");
    if (!runs_[i].fp)
      return RC::IOERR_OPEN;
    std::unique_ptr<ValueListTuple> t;
    RC                              rc = deserialize_tuple(runs_[i].fp, t);
    if (rc == RC::SUCCESS && t) {
      // attach specs from output schema
      std::vector<TupleCellSpec> specs;
      specs.reserve(tuple_schema_.cell_num());
      for (int j = 0; j < tuple_schema_.cell_num(); j++) {
        specs.push_back(tuple_schema_.cell_at(j));
      }
      t->set_names(specs);
      HeapItem item;
      item.run_index = i;
      item.tuple     = std::move(t);
      heap_.push(std::move(item));
    } else if (rc == RC::RECORD_EOF) {
      // empty run; ignore
    } else if (rc != RC::SUCCESS) {
      return rc;
    }
  }
  // If LIMIT top_k_ is set, we still perform full k-way merge but
  // the consumer should stop after top_k_ rows. This operator leaves truncation to caller.
  return RC::SUCCESS;
}

RC ExternalSortPhysicalOperator::read_next_from_run(size_t run_index, std::unique_ptr<ValueListTuple> &out)
{
  if (run_index >= runs_.size() || runs_[run_index].fp == nullptr)
    return RC::RECORD_EOF;
  return deserialize_tuple(runs_[run_index].fp, out);
}

void ExternalSortPhysicalOperator::cleanup_runs()
{
  for (auto &r : runs_) {
    if (r.fp) {
      fclose(r.fp);
      r.fp = nullptr;
    }
    if (!r.path.empty()) {
      std::error_code ec;
      remove(r.path, ec);
    }
  }
  runs_.clear();
  if (!temp_dir_.empty()) {
    std::error_code ec;
    remove_all(temp_dir_, ec);
    temp_dir_.clear();
  }
}

RC ExternalSortPhysicalOperator::produce_all()
{
  buffer_.clear();
  buffer_bytes_used_ = 0;

  RC     rc       = RC::SUCCESS;
  size_t produced = 0;
  while ((rc = children_[0]->next()) == RC::SUCCESS) {
    Tuple *t = children_[0]->current_tuple();
    if (t == nullptr)
      return RC::INTERNAL;

    // materialize to ValueListTuple
    ValueListTuple        vt;
    vector<Value>         cells;
    vector<TupleCellSpec> specs;
    for (int i = 0; i < t->cell_num(); i++) {
      Value         v;
      TupleCellSpec s;
      RC            r1 = t->cell_at(i, v);
      RC            r2 = t->spec_at(i, s);
      if (r1 != RC::SUCCESS || r2 != RC::SUCCESS)
        return RC::INTERNAL;
      cells.push_back(v);
      specs.push_back(s);
    }
    vt.set_cells(cells);
    vt.set_names(specs);

    size_t sz = approximate_tuple_size(vt);
    buffer_.emplace_back(std::move(vt));
    buffer_bytes_used_ += sz;

    if (buffer_bytes_used_ >= memory_limit_bytes_) {
      RC fr = flush_run();
      if (fr != RC::SUCCESS)
        return fr;
    }
    if ((++produced % 10000) == 0) {
      sql_debug("EXTSORT fetched=%zu mem_bytes=%zu", produced, buffer_bytes_used_);
    }
  }

  if (rc != RC::RECORD_EOF) {
    return rc;
  }

  // flush tail and prepare for output
  RC fr = flush_run();
  if (fr != RC::SUCCESS)
    return fr;
  return finalize_runs();
}
