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

#include "ob_token_posting_cache.h"
#include "lib/oblog/ob_log_module.h"

namespace oceanbase
{
namespace storage
{

// 初始化 Token Posting Cache
// 应该在 observer 启动时调用
int init_token_posting_cache()
{
  int ret = OB_SUCCESS;
  const char *cache_name = "TokenPostingCache";
  const int64_t priority = 1;  // 优先级
  
  if (OB_FAIL(ObTokenPostingCache::get_instance().init(cache_name, priority))) {
    STORAGE_LOG(WARN, "Failed to init token posting cache", K(ret));
  } else {
    STORAGE_LOG(INFO, "Token posting cache initialized successfully");
  }
  
  return ret;
}

} // namespace storage
} // namespace oceanbase
