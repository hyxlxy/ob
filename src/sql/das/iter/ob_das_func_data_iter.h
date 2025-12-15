/*
 * Copyright (c) 2025 OceanBase.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OB_DAS_FUNC_DATA_ITER_H_
#define OB_DAS_FUNC_DATA_ITER_H_

#include "sql/das/iter/ob_das_iter.h"
#include "sql/das/iter/ob_das_scan_iter.h"
#include "sql/das/iter/ob_das_text_retrieval_merge_iter.h"
#include "common/ob_tablet_id.h"
#include "share/ob_ls_id.h"
#include "storage/access/ob_dml_param.h"
#include "share/cache/ob_kv_storecache.h"
#include "sql/parser/parse_node_hash.h"

namespace oceanbase
{
namespace sql
{

class ObDASScanCtDef;
class ObDASScanRtDef;
class ObDASFuncDataIterParam final : public ObDASIterParam
{
public:
  ObDASFuncDataIterParam();
  ~ObDASFuncDataIterParam();

  virtual bool is_valid() const override
  {
    return iter_count_ >= 1 && nullptr != tr_merge_iters_;
  }
public:
  ObDASIter **tr_merge_iters_;
  int64_t iter_count_;
  const ObDASScanCtDef *main_lookup_ctdef_;
  ObDASScanRtDef *main_lookup_rtdef_;
  ObDASIter *main_lookup_iter_;
  ObExpr *doc_id_expr_;
  transaction::ObTxDesc *trans_desc_;
  transaction::ObTxReadSnapshot *snapshot_;
};

/**
 * FTS DATA Iter:
 *
 *
 *                  FTS_DATA_Iter                             
 *                /    |     |     \                          
 *             /      |       |       \
 *          /        |         |         \
 *       /          |           |           \                           
 *  TR_ITER1    TR_ITER2    TR_ITER3 ...  MAIN_LOOKUP_ITER(may be null) 
 * 
 **/

class ObDASFuncDataIter final : public ObDASIter
{
public:
  ObDASFuncDataIter();
  ~ObDASFuncDataIter();

  virtual int do_table_scan() override;
  virtual int rescan() override;
  virtual void clear_evaluated_flag() override;
  virtual int set_scan_rowkey(ObEvalCtx *eval_ctx,
                              const ObIArray<ObExpr *> &rowkey_exprs,
                              const ObDASScanCtDef *lookup_ctdef,
                              ObIAllocator *alloc,
                              int64_t group_id) override;
  inline int add_doc_id(const ObDocIdExt &doc_id)
  {
    int ret = OB_SUCCESS;
    int64_t idx = doc_ids_.count();
    if (OB_FAIL(doc_ids_.push_back(std::make_pair(doc_id, idx)))) {
      LOG_WARN("fail to push back doc id", K(ret));
    }
    return ret;
  }

  int64_t get_doc_id_count() const
  {
    return doc_ids_.count();
  }

  void set_tablet_id(const ObTabletID &tablet_id) { main_lookup_tablet_id_ = tablet_id; }
  void set_ls_id(const share::ObLSID &ls_id) { main_lookup_ls_id_ = ls_id; }
  bool has_main_lookup_iter() const { return nullptr != main_lookup_iter_; }
  ObTableScanParam &get_main_lookup_scan_param() { return main_lookup_param_; }
  const ObDASScanCtDef *get_main_lookup_ctdef() { return main_lookup_ctdef_; }
  const ObIArray<std::pair<ObDocIdExt, int>> &get_doc_ids() { return doc_ids_; }
  INHERIT_TO_STRING_KV("ObDASIter", ObDASIter,
                      K(main_lookup_param_),
                      KPC(main_lookup_iter_));
protected:
  virtual int inner_init(ObDASIterParam &param) override;
  virtual int inner_reuse() override;
  virtual int inner_release() override;
  virtual int inner_get_next_row() override;
  virtual int inner_get_next_rows(int64_t &count, int64_t capacity) override;
private:
  common::ObArenaAllocator &get_arena_allocator() { return merge_memctx_->get_arena_allocator(); }
  int init_main_lookup_scan_param(
        ObTableScanParam &param,
        const ObDASScanCtDef *ctdef,
        ObDASScanRtDef *rtdef,
        transaction::ObTxDesc *trans_desc,
        transaction::ObTxReadSnapshot *snapshot);
  int build_tr_merge_iters_rangekey();
  struct FtsDocIdCmp
  {
    FtsDocIdCmp(common::ObDatumCmpFuncType cmp_func, int *ret)
    {
      cmp_func_ = cmp_func;
      err_code_ = ret;
    }

    bool operator()(const std::pair<ObDocIdExt, int> &a, const std::pair<ObDocIdExt, int> &b) const
    {
      int ret = OB_SUCCESS;
      int tmp_ret = 0;
      if (OB_FAIL(cmp_func_(a.first.get_datum(), b.first.get_datum(), tmp_ret))) {
        LOG_WARN("failed to compare doc id by datum", K(ret));
      }
      *err_code_ = *err_code_ == OB_SUCCESS ? ret : *err_code_;
      return tmp_ret < 0;
    }
    int *err_code_;
  private:
    common::ObDatumCmpFuncType cmp_func_;
  };
public:
  // ---------- Lightweight DocHeader cache (tenant-level) ----------
  struct DocHeaderKey : public common::ObIKVCacheKey
  {
    DocHeaderKey() : tenant_id_(MTL_ID()) {}
    explicit DocHeaderKey(const ObDocIdExt &d) : tenant_id_(MTL_ID()), docid_(d) {}
    uint64_t get_tenant_id() const { return tenant_id_; }
    uint64_t hash() const { return ::oceanbase::common::murmurhash(&tenant_id_, sizeof(tenant_id_), ::oceanbase::common::murmurhash(&docid_, sizeof(docid_), 0)); }
    int64_t size() const { return sizeof(*this); }
    int deep_copy(char *buf, const int64_t buf_len, common::ObIKVCacheKey *&key) const
    {
      int ret = OB_SUCCESS;
      if (OB_UNLIKELY(nullptr == buf || buf_len < size())) {
        ret = OB_INVALID_ARGUMENT;
      } else {
        DocHeaderKey *dst = new (buf) DocHeaderKey();
        dst->tenant_id_ = tenant_id_;
        dst->docid_ = docid_;
        key = dst;
      }
      return ret;
    }
    bool operator==(const common::ObIKVCacheKey &other) const
    {
      const DocHeaderKey &o = reinterpret_cast<const DocHeaderKey &>(other);
      return tenant_id_ == o.tenant_id_ && docid_ == o.docid_;
    }
    uint64_t tenant_id_;
    ObDocIdExt docid_;
  };
  struct DocHeaderVal : public common::ObIKVCacheValue
  {
    DocHeaderVal() : id_(0), base_hash_(0) {}
    DocHeaderVal(int64_t id, uint64_t base_hash) : id_(id), base_hash_(base_hash) {}
    int64_t size() const { return sizeof(*this); }
    int deep_copy(char *buf, const int64_t buf_len, common::ObIKVCacheValue *&value) const
    {
      int ret = OB_SUCCESS;
      if (OB_UNLIKELY(nullptr == buf || buf_len < size())) {
        ret = OB_INVALID_ARGUMENT;
      } else {
        DocHeaderVal *dst = new (buf) DocHeaderVal();
        dst->id_ = id_;
        dst->base_hash_ = base_hash_;
        value = dst;
      }
      return ret;
    }
    int64_t id_;
    uint64_t base_hash_;
  };
  static int ensure_cache_inited();
  static int cache_get(const ObDocIdExt &docid, DocHeaderVal &val, common::ObKVCacheHandle &handle);
  static int cache_put(const ObDocIdExt &docid, const DocHeaderVal &val);
  int try_update_doc_header_cache_batch(int64_t count);

private:
  common::ObDatumCmpFuncType cmp_func_;
  ObDASIter **tr_merge_iters_;
  int64_t iter_count_;
  const ObDASScanCtDef *main_lookup_ctdef_;
  ObDASScanRtDef *main_lookup_rtdef_;
  ObDASIter *main_lookup_iter_;
  ObTabletID main_lookup_tablet_id_;
  share::ObLSID main_lookup_ls_id_;
  storage::ObTableScanParam main_lookup_param_;
  lib::MemoryContext merge_memctx_;
  ObSEArray<std::pair<ObDocIdExt, int>, 4> doc_ids_;
  int64_t read_count_;
  bool start_table_scan_;

  // cache helpers
  static common::ObKVCache<DocHeaderKey, DocHeaderVal> g_doc_header_cache_;
  static bool g_cache_inited_;
};

} // end namespace sql
} // end namespace oceanbase
#endif // OB_DAS_FUNC_DATA_ITER_H_
