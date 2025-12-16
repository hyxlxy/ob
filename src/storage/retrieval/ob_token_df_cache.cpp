/*
 * Copyright (c) 2025 OceanBase.
 */
#include "storage/retrieval/ob_token_df_cache.h"
#include "share/cache/ob_kv_storecache.h"

namespace oceanbase
{
namespace storage
{

ObTokenDFCache g_token_df_cache;

int init_token_df_cache()
{
  static bool inited = false;
  int ret = OB_SUCCESS;
  if (!inited) {
    if (OB_FAIL(g_token_df_cache.init("TOKEN_DF_CACHE", /*priority*/1, /*mem_limit_pct*/10))) {
      // ignore error, keep ret, caller may continue without cache
    } else {
      inited = true;
    }
  }
  return ret;
}

} // namespace storage
} // namespace oceanbase

