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

#ifndef OB_DAS_TR_MERGE_ITER_H_
#define OB_DAS_TR_MERGE_ITER_H_

#include "sql/das/iter/ob_das_iter.h"
#include "sql/das/ob_das_ir_define.h"
#include "storage/retrieval/ob_i_sparse_retrieval_iter.h"
#include "storage/retrieval/ob_text_daat_iter.h"
#include "storage/retrieval/ob_text_taat_iter.h"
#include "storage/retrieval/ob_sparse_lookup_iter.h"

namespace oceanbase
{
namespace sql
{
struct ObDASIRScanCtDef;
struct ObDASIRScanRtDef;
class ObDASTextRetrievalIterator;
class ObFtsEvalNode;
class ObDASTextRetrievalIter;
class ObDocIdExt;

struct ObDASTRMergeIterParam : public ObDASIterParam
{
  ObDASTRMergeIterParam()
    : ObDASIterParam(DAS_ITER_TEXT_RETRIEVAL_MERGE),
      ir_ctdef_(nullptr),
      ir_rtdef_(nullptr),
      tx_desc_(nullptr),
      snapshot_(nullptr),
      query_tokens_(),
      dim_weights_(),
      max_batch_size_(0),
      boolean_compute_node_(nullptr),
      flags_(0)
  {}

  virtual bool is_valid() const override
  {
    return nullptr != ir_ctdef_ && nullptr != ir_rtdef_;
  }

  const ObDASIRScanCtDef *ir_ctdef_;
  ObDASIRScanRtDef *ir_rtdef_;
  transaction::ObTxDesc *tx_desc_;
  transaction::ObTxReadSnapshot *snapshot_;
  ObArray<ObString> query_tokens_;
  ObArray<double> dim_weights_;
  int64_t max_batch_size_;
  ObFtsEvalNode *boolean_compute_node_;
  union {
    struct {
      uint32_t function_lookup_mode_  : 1;
      uint32_t topk_mode_             : 1;
      uint32_t daat_mode_             : 1;
      uint32_t taat_mode_             : 1;
      uint32_t reserve                : 28;
    };
    uint32_t flags_;
  };
};

class ObGlobalTokenCacheManager {  
private:  
  struct CacheEntry {
    ObTRDaaTTokenCacheIter *iter_;
    ObString token_;
    int64_t access_time_;
    CacheEntry *prev_;
    CacheEntry *next_;
    
    CacheEntry() : iter_(nullptr), token_(), access_time_(0), prev_(nullptr), next_(nullptr) {}
  };
  
  hash::ObHashMap<ObString, CacheEntry*> global_cache_map_;
  common::ObArenaAllocator allocator_;
  CacheEntry *lru_head_;  // 最近使用的
  CacheEntry *lru_tail_;  // 最久未使用的
  int64_t cache_count_;
  static const int64_t MAX_CACHE_SIZE = 100;  // 最多缓存100个token
  lib::ObMutex mutex_;  // 保护并发访问

  void move_to_head(CacheEntry *entry) {
    if (entry == lru_head_) return;
    
    // 从当前位置移除
    if (entry->prev_) entry->prev_->next_ = entry->next_;
    if (entry->next_) entry->next_->prev_ = entry->prev_;
    if (entry == lru_tail_) lru_tail_ = entry->prev_;
    
    // 移到头部
    entry->next_ = lru_head_;
    entry->prev_ = nullptr;
    if (lru_head_) lru_head_->prev_ = entry;
    lru_head_ = entry;
    if (!lru_tail_) lru_tail_ = entry;
  }
  
  void evict_lru() {
    if (!lru_tail_ || cache_count_ <= MAX_CACHE_SIZE) return;
    
    CacheEntry *to_evict = lru_tail_;
    lru_tail_ = to_evict->prev_;
    if (lru_tail_) lru_tail_->next_ = nullptr;
    else lru_head_ = nullptr;
    
    global_cache_map_.erase_refactored(to_evict->token_);
    if (to_evict->iter_) {
      to_evict->iter_->~ObTRDaaTTokenCacheIter();
    }
    cache_count_--;
    LOG_INFO("[TokenCache] Evicted LRU cache entry", "token", to_evict->token_, K_(cache_count));
  }
  
public:  
  ObGlobalTokenCacheManager(): 
      allocator_(ObMemAttr(MTL_ID(), "TokenCacheAlloc")),
      lru_head_(nullptr),
      lru_tail_(nullptr),
      cache_count_(0) {  
    global_cache_map_.create(1024, ObMemAttr(MTL_ID(), "TokenCache"));  
  }  
  
  ~ObGlobalTokenCacheManager() {  
    CacheEntry *cur = lru_head_;
    while (cur) {
      CacheEntry *next = cur->next_;
      if (cur->iter_) {
        cur->iter_->~ObTRDaaTTokenCacheIter();
      }
      cur = next;
    }
    global_cache_map_.destroy();
  }
  
  static ObGlobalTokenCacheManager& get_instance() {  
    static ObGlobalTokenCacheManager instance;  
    return instance;  
  }

  ObTRDaaTTokenCacheIter* create_cache_iter() {  
    void *buf = allocator_.alloc(sizeof(ObTRDaaTTokenCacheIter));  
    if (OB_NOT_NULL(buf)) {  
      return new(buf) ObTRDaaTTokenCacheIter(&allocator_);  
    }  
    return nullptr;
  }
  
  ObTRDaaTTokenCacheIter* create_iter_from_cache(ObTRDaaTTokenCacheIter *cache_iter) {  
    int ret = OB_SUCCESS;
    ObTRDaaTTokenCacheIter *new_iter = nullptr;  
    if (OB_ISNULL(cache_iter) || !cache_iter->have_done_flag_) {  
      return nullptr;  
    }    
    void *buf = allocator_.alloc(sizeof(ObTRDaaTTokenCacheIter));  
    if (OB_ISNULL(buf)) {  
      return nullptr;  
    }  
    new_iter = new(buf) ObTRDaaTTokenCacheIter(&allocator_);  
    if (OB_FAIL(new_iter->init_from_cache(cache_iter))) {  
      return nullptr;  
    }  
    return new_iter;
  }
  
  ObTRDaaTTokenCacheIter* get_cache_iter(const ObString &token) {
    lib::ObMutexGuard guard(mutex_);
    CacheEntry *entry = nullptr;
    if (OB_SUCCESS == global_cache_map_.get_refactored(token, entry) && entry) {
      move_to_head(entry);
      entry->access_time_ = ObTimeUtility::current_time();
      return entry->iter_;
    }
    return nullptr;  
  }  
    
  int cache_iter(const ObString &token, ObTRDaaTTokenCacheIter *iter) {
    lib::ObMutexGuard guard(mutex_);
    int ret = OB_SUCCESS;
    
    // 检查是否已存在
    CacheEntry *existing = nullptr;
    if (OB_SUCCESS == global_cache_map_.get_refactored(token, existing)) {
      return OB_SUCCESS;  // 已存在，不重复添加
    }
    
    // 需要淘汰旧条目
    if (cache_count_ >= MAX_CACHE_SIZE) {
      evict_lru();
    }
    
    // 创建新条目
    void *buf = allocator_.alloc(sizeof(CacheEntry));
    if (OB_ISNULL(buf)) {
      return OB_ALLOCATE_MEMORY_FAILED;
    }
    
    CacheEntry *entry = new(buf) CacheEntry();
    entry->iter_ = iter;
    
    // 深拷贝token字符串
    char *token_buf = static_cast<char*>(allocator_.alloc(token.length()));
    if (OB_ISNULL(token_buf)) {
      return OB_ALLOCATE_MEMORY_FAILED;
    }
    MEMCPY(token_buf, token.ptr(), token.length());
    entry->token_.assign_ptr(token_buf, token.length());
    entry->access_time_ = ObTimeUtility::current_time();
    
    // 添加到LRU头部
    entry->next_ = lru_head_;
    entry->prev_ = nullptr;
    if (lru_head_) lru_head_->prev_ = entry;
    lru_head_ = entry;
    if (!lru_tail_) lru_tail_ = entry;
    
    // 添加到HashMap
    ret = global_cache_map_.set_refactored(entry->token_, entry, 0);
    if (OB_SUCCESS == ret) {
      cache_count_++;
      LOG_INFO("[TokenCache] Cached new token", "token", entry->token_, K_(cache_count));
    }
    
    return ret;  
  }
};
using TokenCacheMg = ObGlobalTokenCacheManager;

// 强制全扫描标志
static const bool FORCE_FULL_SCAN_FOR_CACHE = true;

class ObDASTRMergeIter : public ObDASIter
{
public:
  ObDASTRMergeIter();
  virtual ~ObDASTRMergeIter() {}
  virtual int do_table_scan() override;
  virtual int rescan() override;

  INHERIT_TO_STRING_KV("ObDASIter", ObDASIter, K_(ir_ctdef), K_(ir_rtdef), K_(tx_desc),
      K_(snapshot), K_(query_tokens), K_(sr_iter_param), K_(sparse_retrieval_iter), K_(dim_iters),
      K_(ls_id), K_(total_doc_cnt_tablet_id), K_(inv_idx_tablet_id), K_(fwd_idx_tablet_id),
      K_(flags), K_(check_rangekey_inited), K_(is_inited));
protected:
  virtual int inner_init(ObDASIterParam &param) override;
  virtual int inner_reuse() override;
  virtual int inner_release() override;
  virtual int inner_get_next_row() override;
  virtual int inner_get_next_rows(int64_t &count, int64_t capacity) override;
public:
  int set_related_tablet_ids(
      const ObLSID &ls_id,
      const ObDASFTSTabletID &related_tablet_ids)
  {
    int ret = OB_SUCCESS;
    ls_id_ = ls_id;
    total_doc_cnt_tablet_id_ = related_tablet_ids.domain_id_idx_tablet_id_;
    if (inv_idx_tablet_id_.is_valid() && inv_idx_tablet_id_ != related_tablet_ids.inv_idx_tablet_id_) {
      inv_idx_tablet_switched_ = true;
    }
    inv_idx_tablet_id_ = related_tablet_ids.inv_idx_tablet_id_;
    fwd_idx_tablet_id_ = related_tablet_ids.fwd_idx_tablet_id_;
    return ret;
  }
  static int build_query_tokens(
      const ObDASIRScanCtDef *ir_ctdef,
      ObDASIRScanRtDef *ir_rtdef,
      common::ObIAllocator &alloc,
      ObArray<ObString> &query_tokens,
      ObArray<double> &boost_values,
      ObFtsEvalNode *&root_node,
      bool &has_duplicate_boolean_tokens);
  int set_children_iter_rangekey(const common::ObIArray<std::pair<ObDocIdExt, int>> &virtual_rangekeys, const int64_t batch_size);
  bool is_taat_mode() { return taat_mode_; }
  int get_query_max_score(double &score) {
    return sparse_retrieval_iter_->get_query_max_score(score);
  }
private:
  int init_das_iter_scan_params();
  static int init_das_iter_scan_param(
      const ObLSID &ls_id,
      const ObTabletID &tablet_id,
      const ObDASScanCtDef *ctdef,
      ObDASScanRtDef *rtdef,
      transaction::ObTxDesc *tx_desc,
      transaction::ObTxReadSnapshot *snapshot,
      common::ObArenaAllocator &allocator,
      ObTableScanParam &scan_param);
  static int reuse_das_iter_scan_param(
      const ObLSID &ls_id,
      const ObTabletID &tablet_id,
      ObTableScanParam &scan_param);
  int create_dim_iters();
  int init_dim_iter_param(ObTextRetrievalScanIterParam &iter_param, const int64_t &idx);
  int create_sparse_retrieval_iter();
  int init_daat_iter_param(ObTextDaaTParam &iter_param);
  int init_taat_iter_param(ObTextTaaTParam &iter_param);
  int set_children_iter_rangekey();
  int gen_inv_idx_scan_default_range(const ObString &query_token, ObNewRange &scan_range);
  int gen_inv_idx_scan_one_range(const ObString &query_token, const ObDocIdExt &doc_id, ObNewRange &scan_range);
  int gen_fwd_idx_scan_feak_range(ObNewRange &scan_range);
  int init_topk_limit();
  int init_block_max_iter_param();
  int init_doc_length_est_param();
//ADVISE
public:
  storage::ObISparseRetrievalMergeIter *get_sparse_retrieval_iter() {
    return sparse_retrieval_iter_;
  }
private:
  static const int64_t FWD_IDX_ROWKEY_COL_CNT = 2;
  static const int64_t INV_IDX_ROWKEY_COL_CNT = 2;
private:
  lib::MemoryContext mem_context_;  // clean after release or reuse
  common::ObArenaAllocator myself_allocator_; // clean after release
  const ObDASIRScanCtDef *ir_ctdef_;
  ObDASIRScanRtDef *ir_rtdef_;
  transaction::ObTxDesc *tx_desc_;
  transaction::ObTxReadSnapshot *snapshot_;
  ObArray<ObString> query_tokens_;
  ObArray<double> dim_weights_;
  ObSparseRetrievalMergeParam sr_iter_param_;
  storage::ObISparseRetrievalMergeIter *sparse_retrieval_iter_;
  static const int64_t OB_DEFAULT_QUERY_TOKEN_ITER_CNT = 4;
  typedef ObSEArray<ObISRDaaTDimIter *, OB_DEFAULT_QUERY_TOKEN_ITER_CNT> ObDASTokenRetrievalIterArray;
  ObDASTokenRetrievalIterArray dim_iters_;
  ObISparseRetrievalDimIter *dim_iter_;
  ObTableScanParam *total_doc_cnt_scan_param_;
  ObFixedArray<ObTableScanParam *, ObIAllocator> inv_scan_params_;
  ObFixedArray<ObTableScanParam *, ObIAllocator> inv_agg_params_;
  ObFixedArray<ObTableScanParam *, ObIAllocator> fwd_scan_params_;
  ObFtsEvalNode *boolean_compute_node_;
  ObFixedArray<ObTableScanParam *, ObIAllocator> block_max_scan_params_;
  ObBlockMaxScoreIterParam block_max_iter_param_;
  ObBlockStatScanParam doc_length_est_param_;
  ObFixedArray<ObSkipIndexColMeta, ObIAllocator> doc_length_est_stat_cols_;
  int64_t topk_limit_;
  ObLSID ls_id_;
  ObTabletID total_doc_cnt_tablet_id_;
  ObTabletID inv_idx_tablet_id_;
  ObTabletID fwd_idx_tablet_id_;
  union {
    struct {
      uint32_t function_lookup_mode_  : 1;
      uint32_t topk_mode_             : 1;
      uint32_t daat_mode_             : 1;
      uint32_t taat_mode_             : 1;
      uint32_t reserve                : 28;
    };
    uint32_t flags_;
  };
  bool check_rangekey_inited_;
  bool inv_idx_tablet_switched_;
  bool is_inited_;
  DISALLOW_COPY_AND_ASSIGN(ObDASTRMergeIter);
};

} // namespace sql
} // namespace oceanbase

#endif // OB_DAS_TR_MERGE_ITER_H_
