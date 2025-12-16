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

#ifndef OB_SCALAR_BITMAP_FILTER_H_
#define OB_SCALAR_BITMAP_FILTER_H_

#include "lib/allocator/ob_allocator.h"
#include "lib/container/ob_se_array.h"
#include "lib/ob_errno.h"
#include "lib/time/ob_time_utility.h"
#include "storage/retrieval/ob_scalar_bitmap_cache.h"

namespace oceanbase
{
namespace sql
{

/*
  标量位图过滤器：
  - 支持 base_id IN 列表的位图并集
  - 支持 id < 阈值的范围裁剪
  - 提供高效的 contains(docid) 检查
  
  使用流程：
  1. 构造 + init(allocator)
  2. add_base_id_value(value) 添加 base_id IN 列表
  3. set_id_upper_bound(bound) 设置 id < 阈值
  4. prepare() 构建最终位图（缓存 miss 时构建）
  5. contains(docid) 检查是否在位图中
  6. reset() 清理
*/

struct ObScalarBitmapFilterStats
{
  ObScalarBitmapFilterStats()
    : cache_hit_cnt_(0), cache_miss_cnt_(0), union_elem_cnt_(0),
      final_elem_cnt_(0), build_time_us_(0), union_time_us_(0)
  {}

  void reset()
  {
    cache_hit_cnt_ = 0;
    cache_miss_cnt_ = 0;
    union_elem_cnt_ = 0;
    final_elem_cnt_ = 0;
    build_time_us_ = 0;
    union_time_us_ = 0;
  }

  int64_t cache_hit_cnt_;
  int64_t cache_miss_cnt_;
  int64_t union_elem_cnt_;
  int64_t final_elem_cnt_;
  int64_t build_time_us_;
  int64_t union_time_us_;

  TO_STRING_KV(K_(cache_hit_cnt), K_(cache_miss_cnt), K_(union_elem_cnt),
               K_(final_elem_cnt), K_(build_time_us), K_(union_time_us));
};

class ObScalarBitmapFilter
{
public:
  static constexpr int64_t MAX_UNION_ELEMS = 5000000; // 5M docids ≈ 40MB
  static constexpr bool ENABLE_SCALAR_BITMAP_FILTER = true; // 硬编码开关

  ObScalarBitmapFilter()
    : allocator_(nullptr), base_id_values_(), id_upper_bound_(-1),
      final_bitmap_(nullptr), final_bitmap_cnt_(0), stats_(), is_prepared_(false)
  {}

  ~ObScalarBitmapFilter() { reset(); }

  int init(common::ObIAllocator &allocator)
  {
    int ret = common::OB_SUCCESS;
    allocator_ = &allocator;
    return ret;
  }

  int add_base_id_value(const common::ObString &value)
  {
    int ret = common::OB_SUCCESS;
    if (OB_ISNULL(allocator_)) {
      ret = common::OB_NOT_INIT;
    } else if (OB_FAIL(base_id_values_.push_back(value))) {
      LOG_WARN("failed to add base_id value", K(ret));
    }
    return ret;
  }

  void set_id_upper_bound(int64_t bound)
  {
    id_upper_bound_ = bound;
  }

  int prepare(uint64_t tenant_id, uint64_t table_id, uint64_t index_id,
              int64_t schema_version, uint64_t col_id)
  {
    int ret = common::OB_SUCCESS;
    if (is_prepared_) {
      return ret; // 已准备过，直接返回
    }

    if (OB_ISNULL(allocator_) || base_id_values_.empty()) {
      return ret; // 无需过滤
    }

    int64_t start_time = common::ObTimeUtility::current_time();

    // 第一步：从缓存获取或构建各 base_id 的位图
    common::ObSEArray<const storage::ObScalarBMVal *, 4> bitmaps;
    common::ObSEArray<common::ObKVCacheHandle, 4> handles;

    for (int64_t i = 0; OB_SUCC(ret) && i < base_id_values_.count(); ++i) {
      const common::ObString &base_id = base_id_values_.at(i);
      uint64_t value_hash = common::murmurhash(base_id.ptr(), base_id.length(), 0);
      storage::ObScalarBMKey key(tenant_id, table_id, index_id, schema_version, col_id, value_hash);

      const storage::ObScalarBMVal *val = nullptr;
      common::ObKVCacheHandle handle;

      // 尝试从缓存获取
      int tmp_ret = storage::ObScalarBMCache::get_instance().get_bitmap(key, val, handle);
      if (common::OB_SUCCESS == tmp_ret && nullptr != val) {
        // 缓存命中
        ++stats_.cache_hit_cnt_;
        if (OB_FAIL(bitmaps.push_back(val))) {
          LOG_WARN("failed to push bitmap", K(ret));
        } else if (OB_FAIL(handles.push_back(handle))) {
          LOG_WARN("failed to push handle", K(ret));
        }
      } else {
        // 缓存未命中，标记需要构建
        ++stats_.cache_miss_cnt_;
        // TODO: 这里需要调用 DAS 扫描 idx_base_id 构建位图，暂时跳过
        // 为了最小可用版本，先返回空位图或跳过该值
        LOG_DEBUG( "[FTCache] ScalarBM miss for base_id", K(base_id));
      }
    }

    // 第二步：对所有位图做 OR 并集
    if (OB_SUCC(ret) && !bitmaps.empty()) {
      int64_t union_start = common::ObTimeUtility::current_time();
      if (OB_FAIL(union_bitmaps(bitmaps))) {
        STORAGE_LOG(WARN, "failed to union bitmaps", K(ret));
      }
      stats_.union_time_us_ = common::ObTimeUtility::current_time() - union_start;
    }

    // 第三步：应用 id < 阈值的范围裁剪
    if (OB_SUCC(ret) && id_upper_bound_ > 0 && nullptr != final_bitmap_) {
      apply_id_range_filter();
    }

    stats_.build_time_us_ = common::ObTimeUtility::current_time() - start_time;
    is_prepared_ = true;

    STORAGE_LOG(DEBUG, "[FTCache] ScalarBM prepared", K_(stats));

    return ret;
  }

  bool contains(uint64_t docid) const
  {
    if (!is_prepared_ || nullptr == final_bitmap_) {
      return true; // 未准备或无位图，默认包含（不过滤）
    }
    return binary_search_docid(docid);
  }

  void reset()
  {
    base_id_values_.reset();
    id_upper_bound_ = -1;
    if (nullptr != final_bitmap_ && nullptr != allocator_) {
      allocator_->free(final_bitmap_);
    }
    final_bitmap_ = nullptr;
    final_bitmap_cnt_ = 0;
    stats_.reset();
    is_prepared_ = false;
  }

  const ObScalarBitmapFilterStats &get_stats() const { return stats_; }

private:
  int union_bitmaps(const common::ObSEArray<const storage::ObScalarBMVal *, 4> &bitmaps)
  {
    int ret = common::OB_SUCCESS;
    if (bitmaps.empty()) {
      return ret;
    }

    // 简化版本：假设每个 bitmap 的格式为 [uint32_t fmt][uint32_t count][uint64_t docids[count]]
    // 这里仅做占位，实际需要根据 ObScalarBMVal 的序列化格式实现

    // 估算最大元素数
    int64_t max_elems = 0;
    for (int64_t i = 0; i < bitmaps.count(); ++i) {
      const storage::ObScalarBMVal *val = bitmaps.at(i);
      if (nullptr != val) {
        // 简化：假设每个 docid 8 字节，size_ 包含格式头
        int64_t elem_cnt = (val->size_ - 8) / sizeof(uint64_t);
        max_elems += elem_cnt;
      }
    }

    if (max_elems > MAX_UNION_ELEMS) {
      STORAGE_LOG(WARN, "[FTCache] ScalarBM union exceeds max size, bypass filter",
                  K(max_elems), K(MAX_UNION_ELEMS));
      return ret; // 超过阈值，禁用过滤
    }

    // 分配内存
    int64_t alloc_size = max_elems * sizeof(uint64_t);
    if (OB_ISNULL(final_bitmap_ = static_cast<uint64_t *>(allocator_->alloc(alloc_size)))) {
      ret = common::OB_ALLOCATE_MEMORY_FAILED;
      STORAGE_LOG(WARN, "failed to allocate memory for final bitmap", K(ret), K(alloc_size));
    } else {
      // 执行 OR 并集（简化版本：仅支持单个位图，多位图需要归并）
      if (1 == bitmaps.count()) {
        const storage::ObScalarBMVal *val = bitmaps.at(0);
        if (nullptr != val && val->size_ > 8) {
          int64_t elem_cnt = (val->size_ - 8) / sizeof(uint64_t);
          MEMCPY(final_bitmap_, static_cast<uint64_t *>(val->bytes_) + 1, elem_cnt * sizeof(uint64_t));
          final_bitmap_cnt_ = elem_cnt;
          stats_.union_elem_cnt_ = elem_cnt;
        }
      } else {
        // TODO: 多位图归并逻辑
        STORAGE_LOG(DEBUG, "[FTCache] ScalarBM multi-bitmap union not implemented yet");
      }
    }

    return ret;
  }

  void apply_id_range_filter()
  {
    if (nullptr == final_bitmap_ || final_bitmap_cnt_ <= 0) {
      return;
    }

    // 在 final_bitmap 上做 upper_bound 查找，找到 >= id_upper_bound_ 的第一个位置
    int64_t left = 0, right = final_bitmap_cnt_;
    while (left < right) {
      int64_t mid = (left + right) / 2;
      if (final_bitmap_[mid] < id_upper_bound_) {
        left = mid + 1;
      } else {
        right = mid;
      }
    }

    // left 即为截断位置，保留 [0, left) 的元素
    final_bitmap_cnt_ = left;
    stats_.final_elem_cnt_ = final_bitmap_cnt_;
  }

  bool binary_search_docid(uint64_t docid) const
  {
    if (nullptr == final_bitmap_ || final_bitmap_cnt_ <= 0) {
      return true;
    }

    int64_t left = 0, right = final_bitmap_cnt_;
    while (left < right) {
      int64_t mid = (left + right) / 2;
      if (final_bitmap_[mid] < docid) {
        left = mid + 1;
      } else {
        right = mid;
      }
    }

    return left < final_bitmap_cnt_ && final_bitmap_[left] == docid;
  }

private:
  common::ObIAllocator *allocator_;
  common::ObSEArray<common::ObString, 4> base_id_values_;
  int64_t id_upper_bound_;
  uint64_t *final_bitmap_;
  int64_t final_bitmap_cnt_;
  ObScalarBitmapFilterStats stats_;
  bool is_prepared_;

  DISALLOW_COPY_AND_ASSIGN(ObScalarBitmapFilter);
};

} // namespace sql
} // namespace oceanbase

#endif // OB_SCALAR_BITMAP_FILTER_H_

