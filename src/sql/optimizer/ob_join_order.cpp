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

#define USING_LOG_PREFIX SQL_JO
#include "ob_join_order.h"
#include "sql/optimizer/ob_skyline_prunning.h"
#include "sql/optimizer/ob_log_table_scan.h"
#include "sql/rewrite/ob_transform_utils.h"
#include "sql/optimizer/ob_access_path_estimation.h"
#include "share/stat/ob_opt_stat_manager.h"
#include "sql/rewrite/ob_predicate_deduce.h"
#include "share/vector_index/ob_vector_index_util.h"
#include "sql/rewrite/ob_query_range_define.h"
#include "sql/engine/expr/ob_expr_result_type_util.h"
#include "sql/engine/px/ob_px_util.h"
#include "sql/das/iter/ob_das_text_retrieval_eval_node.h"
#include "share/external_table/ob_external_table_utils.h"
#include "common/ad_log.h"

using namespace oceanbase;
using namespace sql;
using namespace oceanbase::common;
using namespace oceanbase::share;
using namespace oceanbase::storage;
using oceanbase::share::schema::ObTableSchema;
using oceanbase::share::schema::ObColumnSchemaV2;
using oceanbase::share::schema::ObColDesc;
using share::schema::ObSchemaGetterGuard;
using common::ObArray;

class OrderingInfo;
class QueryRangeInfo;

#define OPT_CTX (get_plan()->get_optimizer_context())

ERRSIM_POINT_DEF(EN_FORCE_INDEX_MERGE_PLAN, "Force to use index merge if it is possible");

ObJoinOrder::~ObJoinOrder()
{

}

int ObJoinOrder::fill_query_range_info(const QueryRangeInfo &range_info,
                                       ObCostTableScanInfo &est_cost_info,
                                       bool use_skip_scan)
{
  int ret = OB_SUCCESS;
  const ObQueryRangeArray &ranges = range_info.get_ranges();
  const ObQueryRangeArray &ss_ranges = range_info.get_ss_ranges();
  const ObQueryRangeProvider *provider = range_info.get_query_range_provider();
  ObSEArray<uint64_t, 4> total_range_sizes;
  est_cost_info.ranges_.reset();
  est_cost_info.ss_ranges_.reset();
  est_cost_info.at_most_one_range_ = false;
  est_cost_info.total_range_cnt_ = range_info.get_range_count();
  est_cost_info.unique_range_rowcnt_ = range_info.get_unique_range_rowcnt();
  bool has_exec_param = false;
  if (OB_ISNULL(provider)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected NULL", K(ret), K(provider));
  }
  // maintain query range info
  for(int64_t i = 0; OB_SUCC(ret) && i < ranges.count(); ++i) {
    if (OB_ISNULL(ranges.at(i))) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("range is null", K(ret));
    } else if (OB_FAIL(est_cost_info.ranges_.push_back(*ranges.at(i)))) {
      LOG_WARN("failed to add range", K(ret));
    } else { /*do nothing*/ }
  }
  for(int64_t i = 0; use_skip_scan && OB_SUCC(ret) && i < ss_ranges.count(); ++i) {
    if (OB_ISNULL(ss_ranges.at(i))) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("range is null", K(ret));
    } else if (OB_FAIL(est_cost_info.ss_ranges_.push_back(*ss_ranges.at(i)))) {
      LOG_WARN("failed to add range", K(ret));
    } else { /*do nothing*/ }
  }

  if (OB_SUCC(ret) && ranges.count() > 1) {
    // if there is more than one range and it is exists exec params in ranges_exprs, check at most one range.
    // for (min; max) range extract from range_exprs contain exec params, do nothing now.
    ObSEArray<ObRawExpr*, 4> cur_const_exprs;
    ObSEArray<ObRawExpr*, 16> columns;
    if (OB_FAIL(ObRawExprUtils::extract_column_exprs(provider->get_range_exprs(),
                                                     columns))) {
      LOG_WARN("failed to extract column exprs", K(ret));
    } else if (columns.empty() || !has_exec_param) {
      /* do nothing */
    } else if (OB_FAIL(ObOptimizerUtil::compute_const_exprs(provider->get_range_exprs(),
                                                            cur_const_exprs))) {
      // for inner path, const expr is computed without pushdown filter.
      // need compute const expr by range_exprs.
      LOG_WARN("failed to compute const exprs", K(ret));
    } else {
      bool at_most_one_range = true;
      for (int64_t i = 0; at_most_one_range && i < columns.count(); ++i) {
        at_most_one_range = ObOptimizerUtil::find_equal_expr(cur_const_exprs, columns.at(i),
                                                             get_output_equal_sets());
      }
      est_cost_info.at_most_one_range_ = at_most_one_range;
    }
  }
  return ret;
}

int ObJoinOrder::get_query_range_count(const QueryRangeInfo &range_info,
                                       int64_t &range_cnt)
{
  int ret = OB_SUCCESS;
  const ObQueryRangeArray &ranges = range_info.get_ranges();
  const ObQueryRangeProvider *provider = range_info.get_query_range_provider();
  ObSEArray<uint64_t, 4> total_range_sizes;
  bool has_exec_param = false;
  if (OB_ISNULL(provider)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected NULL", K(ret), K(provider));
  } else if (OB_FAIL(check_has_exec_param(*provider, has_exec_param))) {
    LOG_WARN("failed to check has exec param", K(ret));
  } else if (!has_exec_param) {
    range_cnt = ranges.count();
  } else if (OB_FAIL(provider->get_total_range_sizes(total_range_sizes))) {
    LOG_WARN("failed to get range size", K(ret));
  } else {
    int64_t index_prefix = range_info.get_index_prefix();
    if (-1 == index_prefix || index_prefix >= total_range_sizes.count()) {
      index_prefix = total_range_sizes.count() - 1;
    }
    if (total_range_sizes.empty() || index_prefix < 0 ) {
      range_cnt = ranges.count();
    } else {
      range_cnt = total_range_sizes.at(index_prefix);
    }
  }
  return ret;
}

int ObJoinOrder::compute_table_location_for_index_info_entry(const uint64_t table_id,
                                                            const uint64_t base_table_id,
                                                            IndexInfoEntry *index_info_entry) {
  int ret = OB_SUCCESS;
  ObTablePartitionInfo *table_part_info = NULL;
  if (OB_ISNULL(index_info_entry)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected null", K(index_info_entry));
  } else if (!index_info_entry->is_index_global()) {
    if (NULL == table_partition_info_) {
      // generate table location for main table
      if (OB_FAIL(compute_table_location(table_id_,
                                        table_meta_info_.ref_table_id_,
                                        false,
                                        table_partition_info_))) {
        LOG_WARN("failed to calc table location", K(ret));
      }
    }
    table_part_info = table_partition_info_;
  } else if (OB_FAIL(get_table_partition_info_from_available_access_paths(table_id, base_table_id,
                                                 index_info_entry->get_index_id(), table_part_info))) {
      LOG_WARN("failed to table partition info from available access paths", K(ret));
  } else if (NULL == table_part_info) {
    if (OB_FAIL(compute_table_location(table_id, index_info_entry->get_index_id(), true, table_part_info))) {
      LOG_WARN("failed to calc table location", K(ret));
    }
  }
  if (OB_SUCC(ret)) {
    if (OB_ISNULL(table_part_info)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("get unexpected null", K(ret));
    } else {
      index_info_entry->set_partition_info(table_part_info);
    }
  }
  return ret;
}

int ObJoinOrder::get_table_partition_info_from_available_access_paths(const uint64_t table_id,
                                                                      const uint64_t ref_table_id,
                                                                      const uint64_t index_id,
                                                                      ObTablePartitionInfo *&table_part_info)
{
  int ret = OB_SUCCESS;
  for (int64_t i = 0; OB_SUCC(ret) && i < available_access_paths_.count(); ++i) {
    AccessPath *cur_path = available_access_paths_.at(i);
    if (OB_ISNULL(cur_path)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("get unexpected null", K(ret));
    } else if (table_id == cur_path->table_id_ &&
              ref_table_id == cur_path->ref_table_id_ &&
              index_id == cur_path->index_id_) {
      table_part_info = cur_path->table_partition_info_;
      break;
    }
  }
  return ret;
}

int ObJoinOrder::compute_table_location(const uint64_t table_id,
                                        const uint64_t ref_table_id,
                                        const bool is_global_index,
                                        ObTablePartitionInfo *&table_partition_info)
{
  int ret = OB_SUCCESS;
  ObOptimizerContext *opt_ctx = NULL;
  ObSqlSchemaGuard *sql_schema_guard = NULL;
  const ObDMLStmt *stmt = NULL;
  const ParamStore *params = NULL;
  const TableItem *table_item = NULL;
  ObExecContext *exec_ctx = NULL;
  ObSqlCtx *sql_ctx = NULL;
  ObPhysicalPlanCtx *phy_plan_ctx = NULL;
  ObSQLSessionInfo *session_info = NULL;
  table_partition_info = NULL;
  if (OB_ISNULL(get_plan()) || OB_ISNULL(allocator_) ||
      OB_ISNULL(opt_ctx = &get_plan()->get_optimizer_context()) ||
      OB_ISNULL(sql_schema_guard = opt_ctx->get_sql_schema_guard()) ||
      OB_ISNULL(stmt = get_plan()->get_stmt()) ||
      OB_ISNULL(table_item = stmt->get_table_item_by_id(table_id)) ||
      OB_ISNULL(params = opt_ctx->get_params()) ||
      OB_ISNULL(exec_ctx = opt_ctx->get_exec_ctx()) ||
      OB_ISNULL(sql_ctx = exec_ctx->get_sql_ctx()) ||
      OB_ISNULL(phy_plan_ctx = exec_ctx->get_physical_plan_ctx())
   || OB_ISNULL(session_info = sql_ctx->session_info_)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("get unexpected null", K(get_plan()), K(opt_ctx), K(sql_schema_guard),
        K(stmt), K(params), K(exec_ctx), K(sql_ctx), K(phy_plan_ctx),
        K(session_info), K(allocator_), K(ret));
  } else if (OB_ISNULL(table_partition_info = reinterpret_cast<ObTablePartitionInfo*>(
                       allocator_->alloc(sizeof(ObTablePartitionInfo))))) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    LOG_WARN("failed to allocate memory", K(table_partition_info), K(ret));
  } else {
    table_partition_info = new(table_partition_info) ObTablePartitionInfo(*allocator_);
    const ObDataTypeCastParams dtc_params =
          ObBasicSessionInfo::create_dtc_params(opt_ctx->get_session_info());
    // check whether the ref table will be modified by dml operator
    bool is_dml_table = false;
    const ObDMLStmt *top_stmt = opt_ctx->get_root_stmt();
    if (NULL != top_stmt) {
      if (top_stmt->is_explain_stmt()) {
        top_stmt = static_cast<const ObExplainStmt*>(top_stmt)->get_explain_query_stmt();
      }
      if (OB_ISNULL(top_stmt)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("top stmt should not be null", K(ret));
      } else if (OB_FAIL(top_stmt->check_table_be_modified(ref_table_id, is_dml_table))) {
        LOG_WARN("failed to check table be modified", K(ret));
      }
    }

    //this place restrict_info include sub_query.
    ObArray<ObRawExpr *> correlated_filters;
    ObArray<ObRawExpr *> uncorrelated_filters;
    if (OB_FAIL(ObOptimizerUtil::extract_parameterized_correlated_filters(get_restrict_infos(),
                                                                          correlated_filters,
                                                                          uncorrelated_filters))) {
      LOG_WARN("Failed to extract correlated filters", K(ret));
    } else if (OB_FAIL(table_partition_info->init_table_location(*sql_schema_guard,
                                                                  *stmt,
                                                                  exec_ctx,
                                                                  uncorrelated_filters,
                                                                  table_id,
                                                                  ref_table_id,
                                                                  is_global_index ? NULL : &table_item->part_ids_,
                                                                  dtc_params,
                                                                  is_dml_table,
                                                                  NULL))) {
      LOG_WARN("Failed to initialize table location", K(ret));
    } else if (OB_FAIL(table_partition_info->calculate_phy_table_location_info(*exec_ctx,
                                                                              *params,
                                                                              dtc_params))) {
      LOG_WARN("failed to calculate table location", K(ret));
    } else {
      LOG_TRACE("succeed to calculate base table sharding info", K(table_id), K(ref_table_id),
          K(is_global_index));
    }
  }
  if (OB_SUCC(ret)) {
    //select location for the newly created table partition info
    ObSEArray<ObTablePartitionInfo*, 1> tbl_part_infos;
    if (OB_FAIL(tbl_part_infos.push_back(table_partition_info))) {
      LOG_WARN("push back table partition info failed", K(ret));
    } else if (OB_FAIL(get_plan()->select_location(tbl_part_infos))) {
      LOG_WARN("failed to select location", K(ret));
    }
  }
  return ret;
}


int ObJoinOrder::compute_sharding_info_for_base_paths(ObIArray<AccessPath *> &access_paths)
{
  int ret = OB_SUCCESS;
  // compute path sharding info
  for (int64_t i = 0; OB_SUCC(ret) && i < access_paths.count(); ++i) {
    if (OB_FAIL(set_sharding_info_for_base_path(access_paths, i))) {
      LOG_WARN("failed to compute sharding info for base path", K(ret));
    }
  }
  if (OB_SUCC(ret) && OB_FAIL(prune_paths_due_to_parallel(access_paths))) {
    LOG_WARN("failed to prune path due to parallel", K(ret));
  }
  for (int64_t i = 0; OB_SUCC(ret) && i < access_paths.count(); i++) {
    AccessPath *path = NULL;
    if (OB_ISNULL(path = access_paths.at(i))) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("get unexpected null", K(path), K(ret));
    } else if (OB_FAIL(compute_base_table_path_plan_type(path))) {
      LOG_WARN("failed to compute base table path plan type", K(ret));
    } else if (OB_FAIL(compute_base_table_path_ordering(path))) {
      LOG_WARN("failed to compute base table path ordering", K(ret));
    } else {
      LOG_TRACE("succeed to compute base sharding info", K(*path));
    }
  }
  return ret;
}

// prune paths added because of auto dop:
//  global index and is not inner path / subquery with pushdown filter
//    1. not use das but parallel = 1
//    2. use das and exits same index path with parallel > 1
int ObJoinOrder::prune_paths_due_to_parallel(ObIArray<AccessPath *> &access_paths)
{
  int ret = OB_SUCCESS;
  AccessPath *path = NULL;
  if (access_paths.count() < 2) {
    /* do nothing */
  } else if (OB_ISNULL(path = access_paths.at(0))) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret), K(path));
  } else if (path->is_rescan_path()) {
    /* do nothing */
  } else {
    ObSEArray<AccessPath*, 16> tmp_paths;
    AccessPath *path = NULL;
    bool need_prune = false;
    for (int64_t i = 0; OB_SUCC(ret) && i < access_paths.count(); i++) {
      need_prune = false;
      if (OB_ISNULL(path = access_paths.at(i))) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("get unexpected null", K(path), K(ret));
      } else if (!path->is_global_index_) {
        /* do nothing */
      } else {
        AccessPath *target_path = NULL;
        for (int64_t j = 0; NULL == target_path && j < access_paths.count(); ++j) {
          if (i != j && NULL != access_paths.at(j) && path->index_id_ == access_paths.at(j)->index_id_
              && path->use_column_store_ == access_paths.at(j)->use_column_store_) {
            target_path = access_paths.at(j);
          }
        }
        if (NULL != target_path) {
          need_prune = path->use_das_ ? 1 < target_path->parallel_
                                      : 1 >= path->parallel_;
        }
      }
      if (OB_SUCC(ret) && !need_prune && OB_FAIL(tmp_paths.push_back(path))) {
        LOG_WARN("failed to push back access path", K(ret));
      }
    }
    if (OB_SUCC(ret) && tmp_paths.count() != access_paths.count()
        && OB_FAIL(access_paths.assign(tmp_paths))) {
      LOG_WARN("failed to assign paths", K(ret));
    }
  }
  return ret;
}

int ObJoinOrder::set_sharding_info_for_base_path(ObIArray<AccessPath *> &access_paths,
                                                     const int64_t cur_idx)
{
  int ret = OB_SUCCESS;
  const ObDMLStmt *stmt = NULL;
  ObOptimizerContext *opt_ctx = NULL;
  ObTableLocationType location_type = OB_TBL_LOCATION_UNINITIALIZED;
  AccessPath *path = NULL;
  ObShardingInfo *sharding_info = NULL;
  ObTablePartitionInfo *table_partition_info = NULL;
  ObSqlSchemaGuard *schema_guard = NULL;
  const ObTableSchema *table_schema = NULL;
  IndexInfoEntry *index_info_entry = NULL;
  if (OB_UNLIKELY(access_paths.count() <= cur_idx) ||
      OB_ISNULL(path = access_paths.at(cur_idx)) ||
      OB_ISNULL(table_partition_info = path->table_partition_info_) ||
      OB_ISNULL(get_plan()) || OB_ISNULL(stmt = get_plan()->get_stmt()) ||
      OB_ISNULL(opt_ctx = &get_plan()->get_optimizer_context()) ||
      OB_ISNULL(schema_guard = opt_ctx->get_sql_schema_guard())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret), K(access_paths.count()), K(path), K(table_partition_info),
                                  K(cur_idx), K(get_plan()), K(stmt), K(opt_ctx));
  } else if (path->use_das_) {
    sharding_info = opt_ctx->get_match_all_sharding();
  } else if (OB_FAIL(schema_guard->get_table_schema(table_partition_info->get_ref_table_id(),
                                      table_schema))) {
    LOG_WARN("failed to get table schema", K(ret));
  } else if (OB_ISNULL(table_schema)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret));
  } else if (table_schema->is_external_table()) {
    ObString file_location;
    CK (OB_NOT_NULL(schema_guard->get_schema_guard()));
    if (OB_FAIL(ret)) {
      // do nothing
    } else if (OB_FAIL(ObExternalTableUtils::get_external_file_location(*table_schema, *schema_guard->get_schema_guard(), *allocator_, file_location))) {
      LOG_WARN("failed to get file_location", K(ret));
    } else if (path->parallel_ > 1
        || ObSQLUtils::is_external_files_on_local_disk(file_location)) {
      sharding_info = opt_ctx->get_distributed_sharding();
    } else {
      sharding_info = opt_ctx->get_local_sharding();
    }
  } else if (OB_FAIL(table_partition_info->get_location_type(opt_ctx->get_local_server_addr(),
                                                                    location_type))) {
    LOG_WARN("failed to get location type", K(ret));
  } else if (ObGlobalHint::DEFAULT_PARALLEL < path->parallel_
             && (OB_TBL_LOCATION_LOCAL == location_type || OB_TBL_LOCATION_REMOTE == location_type)) {
    sharding_info = opt_ctx->get_distributed_sharding();
  } else {
    sharding_info = path->strong_sharding_;
  }
  if (OB_FAIL(ret)) {
  } else if (OB_ISNULL(sharding_info)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("failed to compute base table sharding info", K(ret), K(sharding_info));
  } else {
    path->strong_sharding_ = sharding_info;
  }
  return ret;
}

//select location and compute sharding info for index entry
int ObJoinOrder::compute_sharding_info_for_index_info_entry(const uint64_t table_id,
                                              const uint64_t base_table_id,
                                              IndexInfoEntry *index_info_entry)
{
  int ret = OB_SUCCESS;
  ObTablePartitionInfo *part_info = NULL;
  ObTableLocationType location_type = OB_TBL_LOCATION_UNINITIALIZED;
  ObShardingInfo *sharding_info = NULL;
  if (OB_ISNULL(index_info_entry)
      || OB_ISNULL(part_info = index_info_entry->get_partition_info())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret));
  }  else if (NULL != sharding_info_ && !index_info_entry->is_index_global()) {
    //all the local index share the same sharding info
    sharding_info = sharding_info_;
  } else if (OB_FAIL(get_sharding_info_from_available_access_paths(table_id, base_table_id,
                                                            index_info_entry->get_index_id(),
                                                            index_info_entry->is_index_global(),
                                                            sharding_info))) {
    LOG_WARN("failed to get sharding info from available access paths", K(ret));
  } else if (NULL != sharding_info) {
    //do nothing
  } else if (OB_FAIL(part_info->get_location_type(OPT_CTX.get_local_server_addr(),
                                                location_type))) {
    LOG_WARN("failed to get location type", K(ret));
  } else if (OB_FAIL(compute_sharding_info_with_part_info(location_type, part_info, sharding_info))) {
    LOG_WARN("compute sharding info with partition info failed", K(ret));
  }
  if (OB_SUCC(ret)) {
    index_info_entry->set_sharding_info(sharding_info);
    if (NULL == sharding_info_ && !index_info_entry->is_index_global()) {
      sharding_info_ = sharding_info;
    }
  }
  return ret;
}

int ObJoinOrder::compute_sharding_info_with_part_info(ObTableLocationType location_type,
                                                      ObTablePartitionInfo* table_partition_info,
                                                      ObShardingInfo *&sharding_info)
{
  int ret = OB_SUCCESS;
  const ObDMLStmt *stmt = NULL;
  ObOptimizerContext *opt_ctx = NULL;
  ObSQLSessionInfo *session_info = NULL;
  bool is_modified = false;
  if (OB_ISNULL(table_partition_info) ||
      OB_ISNULL(get_plan()) || OB_ISNULL(stmt = get_plan()->get_stmt()) ||
      OB_ISNULL(opt_ctx = &get_plan()->get_optimizer_context()) ||
      OB_ISNULL(session_info = opt_ctx->get_session_info())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret), K(table_partition_info), K(get_plan()), K(stmt), K(opt_ctx), K(session_info));
  } else if (OB_ISNULL(sharding_info = reinterpret_cast<ObShardingInfo*>(
                                       allocator_->alloc(sizeof(ObShardingInfo))))) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    LOG_ERROR("failed to allocate memory", K(ret));
  } else if (OB_FAIL(stmt->check_table_be_modified(table_partition_info->get_ref_table_id(), is_modified))) {
    LOG_WARN("failed to check table be modified", K(ret));
  } else {
    const ObCandiTableLoc &phy_tbl_info = table_partition_info->get_phy_tbl_location_info();
    bool can_reselect_replica = (phy_tbl_info.is_duplicate_table_not_in_dml() &&
        (1 == phy_tbl_info.get_phy_part_loc_info_list().count())
        && !session_info->get_is_in_retry_for_dup_tbl()
        && !is_modified
        && !session_info->is_dup_ls_modified());
    sharding_info = new(sharding_info) ObShardingInfo();
    sharding_info->set_location_type(location_type);
    if (OB_FAIL(sharding_info->init_partition_info(
                                  get_plan()->get_optimizer_context(),
                                  *get_plan()->get_stmt(),
                                  table_partition_info->get_table_id(),
                                  table_partition_info->get_ref_table_id(),
                                  table_partition_info->get_phy_tbl_location_info_for_update()))) {
      LOG_WARN("failed to set partition key", K(ret));
    } else {
      sharding_info->set_can_reselect_replica(can_reselect_replica);
      LOG_TRACE("succeed to compute base table sharding info", K(*sharding_info));
    }
  }
  return ret;
}

int ObJoinOrder::get_sharding_info_from_available_access_paths(const uint64_t table_id,
                                                               const uint64_t ref_table_id,
                                                               const uint64_t index_id,
                                                               bool is_global_index,
                                                               ObShardingInfo *&sharding_info) const
{
  int ret = OB_SUCCESS;
  sharding_info = NULL;
  AccessPath *cur_path = NULL;
  ObOptimizerContext *opt_ctx = NULL;
  if (OB_ISNULL(get_plan()) || OB_ISNULL(opt_ctx = &get_plan()->get_optimizer_context())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected null param", K(ret));
  }
  for (int64_t i = 0; OB_SUCC(ret) && NULL == sharding_info && i < available_access_paths_.count(); ++i) {
    cur_path = available_access_paths_.at(i);
    if (OB_ISNULL(cur_path) || OB_ISNULL(cur_path->strong_sharding_)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("get unexpected null", K(ret), K(cur_path));
    } else if (cur_path->strong_sharding_ != opt_ctx->get_match_all_sharding() &&
                cur_path->strong_sharding_ != opt_ctx->get_distributed_sharding() &&
                cur_path->strong_sharding_ != opt_ctx->get_local_sharding() &&
                table_id == cur_path->table_id_ &&
                ref_table_id == cur_path->ref_table_id_ &&
                is_global_index == cur_path->is_global_index_ &&
                index_id == cur_path->index_id_) {
      sharding_info = cur_path->strong_sharding_;
    }
  }
  return ret;
}

int ObJoinOrder::compute_base_table_path_plan_type(AccessPath *path)
{
  int ret = OB_SUCCESS;
  ObShardingInfo *sharding_info = NULL;
  if (OB_ISNULL(path) || OB_ISNULL(sharding_info = path->strong_sharding_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(path), K(sharding_info), K(ret));
  } else {
    if (sharding_info->get_can_reselect_replica()) {
      path->phy_plan_type_ = ObPhyPlanType::OB_PHY_PLAN_UNINITIALIZED;
    } else if (sharding_info->is_local()) {
      path->phy_plan_type_ = ObPhyPlanType::OB_PHY_PLAN_LOCAL;
    } else if (sharding_info->is_remote()) {
      path->phy_plan_type_ = ObPhyPlanType::OB_PHY_PLAN_REMOTE;
    } else if (sharding_info->is_distributed()) {
      path->phy_plan_type_ = ObPhyPlanType::OB_PHY_PLAN_DISTRIBUTED;
    }
    if (path->use_das_ ||
        (path->is_global_index_ && path->est_cost_info_.index_meta_info_.is_index_back_)) {
      path->location_type_ = ObPhyPlanType::OB_PHY_PLAN_UNCERTAIN;
    }
  }
  return ret;
}

int ObJoinOrder::compute_base_table_path_ordering(AccessPath *path)
{
  int ret = OB_SUCCESS;
  bool is_left_prefix = false;
  bool is_right_prefix = false;
  const ObDMLStmt *stmt = NULL;
  ObSEArray<ObRawExpr*, 8> range_exprs;
  ObSEArray<OrderItem, 8> range_orders;
  if (OB_ISNULL(path) || OB_ISNULL(get_plan()) || OB_ISNULL(get_plan()->get_stmt()) ||
      OB_ISNULL(path->strong_sharding_) || OB_ISNULL(path->table_partition_info_) ||
      OB_ISNULL(stmt = get_plan()->get_stmt()) || OB_ISNULL(stmt->get_query_ctx())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(path), K(ret));
  } else if (OB_FALSE_IT(path->is_local_order_ = false)) {
  } else if (OB_FALSE_IT(path->is_range_order_ = false)) {
  } else if (path->est_cost_info_.index_meta_info_.is_multivalue_index_) {
    // The Json multi-value index scan operator will perform sorting and deduplication based on the primary key or docid.
    // As a result, the final output order of the data may not match the original index table output order.
    // Therefore, it's necessary to add an additional sort operator at the upper level.
    path->ordering_.reset();
  } else if (path->use_das_ &&
             !path->ordering_.empty() &&
             path->table_partition_info_->get_phy_tbl_location_info().get_partition_cnt() > 1) {
    if (get_plan()->get_optimizer_context().is_das_keep_order_enabled()) {
      // when enable das keep order optimization, DAS layer can provide a guarantee of local order,
      // otherwise the order is totally not guaranteed.
      path->is_local_order_ = true;
    } else {
      path->ordering_.reset();
    }
  } else if (path->ordering_.empty() || is_at_most_one_row_ || !path->strong_sharding_->is_distributed()) {
    path->is_local_order_ = false;
  } else if (get_plan()->get_optimizer_context().is_online_ddl()) {
    path->is_local_order_ = true;
  } else if (path->table_partition_info_->get_phy_tbl_location_info().get_phy_part_loc_info_list().count() == 1 &&
            !is_virtual_table(path->ref_table_id_)) {
    path->is_range_order_ = true;
  } else if (OB_FAIL(path->table_partition_info_->get_not_insert_dml_part_sort_expr(*get_plan()->get_stmt(),
                                                                                    &range_exprs))) {
    LOG_WARN("fail to get_not_insert_dml_part_sort_expr", K(ret));
  } else if (range_exprs.empty()) {
    path->is_local_order_ = true;
  } else if (OB_FAIL(ObOptimizerUtil::make_sort_keys(range_exprs,
                                                     path->order_direction_,
                                                     range_orders))) {
    LOG_WARN("failed to make range orders", K(ret));
  } else if (OB_FAIL(ObOptimizerUtil::is_prefix_ordering(range_orders,
                                                         path->ordering_,
                                                         get_output_equal_sets(),
                                                         get_output_const_exprs(),
                                                         is_left_prefix,
                                                         is_right_prefix))) {
    LOG_WARN("failed to find common prefix ordering", K(ret));
  } else if (is_left_prefix || is_right_prefix) {
    path->is_local_order_ = false;
    path->is_range_order_ = true;
  } else {
    path->is_local_order_ = true;
  }
  return ret;
}

int ObJoinOrder::get_explicit_dop_for_path(const uint64_t index_id, int64_t &parallel)
{
  int ret = OB_SUCCESS;
  ObOptimizerContext *opt_ctx = NULL;
  parallel = ObGlobalHint::UNSET_PARALLEL;
  if (OB_ISNULL(get_plan()) || OB_ISNULL(opt_ctx = &get_plan()->get_optimizer_context())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected null param", K(ret));
  } else if (opt_ctx->force_disable_parallel()) {
    parallel = opt_ctx->get_parallel();
  } else if (ObGlobalHint::UNSET_PARALLEL != (parallel = get_plan()->get_log_plan_hint().get_parallel(get_table_id()))) {
    /* do nothing */
  } else if (opt_ctx->is_use_table_dop()) {
    if (OB_FAIL(get_base_path_table_dop(index_id, parallel))) {
      LOG_WARN("failed to get base table dop", K(ret));
    }
  } else if (opt_ctx->is_use_auto_dop()) {
    /* do nothing */
  } else {
    parallel = opt_ctx->get_parallel();
  }
  return ret;
}

int ObJoinOrder::compute_parallel_and_server_info_for_base_paths(ObIArray<AccessPath *> &access_paths)
{
  int ret = OB_SUCCESS;
  ObOptimizerContext *opt_ctx = NULL;
  if (access_paths.empty()) {
    /* do nothing */
  } else if (OB_ISNULL(get_plan()) || OB_ISNULL(opt_ctx = &get_plan()->get_optimizer_context())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected null param", K(ret));
  } else {
    OpParallelRule op_parallel_rule = OpParallelRule::OP_GLOBAL_DOP ;
    int64_t parallel = ObGlobalHint::UNSET_PARALLEL;
    if (opt_ctx->force_disable_parallel()) {  // force disable parallel
      parallel = ObGlobalHint::DEFAULT_PARALLEL;
    } else if (ObGlobalHint::UNSET_PARALLEL != (parallel = get_plan()->get_log_plan_hint().get_parallel(get_table_id()))) {
      op_parallel_rule = OpParallelRule::OP_HINT_DOP;
    } else if (opt_ctx->is_use_auto_dop()) {
      op_parallel_rule = OpParallelRule::OP_AUTO_DOP;
      if (OB_FAIL(compute_access_path_parallel(access_paths, parallel))) {
        LOG_WARN("failed to calculate base path parallel", K(ret));
      }
    } else if (opt_ctx->is_use_table_dop()) {
      op_parallel_rule = OpParallelRule::OP_TABLE_DOP;
    } else if (OB_UNLIKELY(opt_ctx->get_parallel() < ObGlobalHint::DEFAULT_PARALLEL
                           || !opt_ctx->is_parallel_rule_valid())) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("invalid parallel rule", K(ret), K(opt_ctx->get_parallel()), K(opt_ctx->is_parallel_rule_valid()));
    } else {
      parallel = opt_ctx->get_parallel();
    }
    for (int64_t i = 0; OB_SUCC(ret) && i < access_paths.count(); i++) {
      if (OB_FAIL(compute_base_table_parallel_and_server_info(op_parallel_rule,
                                                              parallel, access_paths.at(i)))) {
        LOG_WARN("failed to compute base table parallel and server info", K(ret));
      }
    }
  }
  return ret;
}

int ObJoinOrder::get_base_path_table_dop(uint64_t index_id, int64_t &parallel)
{
  int ret = OB_SUCCESS;
  ObSqlSchemaGuard *schema_guard = NULL;
  const ObTableSchema *table_schema = NULL;
  parallel = ObGlobalHint::DEFAULT_PARALLEL;
  if (OB_ISNULL(get_plan()) ||
      OB_ISNULL(schema_guard = get_plan()->get_optimizer_context().get_sql_schema_guard())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected null param", K(ret), K(get_plan()), K(schema_guard));
  } else if (OB_FAIL(schema_guard->get_table_schema(index_id, table_schema))) {
    LOG_WARN("failed to get table schema", K(ret));
  } else if (OB_ISNULL(table_schema)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret));
  } else if (table_schema->get_dop() > ObGlobalHint::DEFAULT_PARALLEL) {
    parallel = table_schema->get_dop();
  }
  return ret;
}

// just generate random parallel for access paths when enable trace point test path
// alter system set_tp tp_no = 552, error_code = 4016, frequency = 1;
// When trace point is enabled, parallel is only limited by parallel_degree_limit.
int ObJoinOrder::get_random_parallel(const int64_t parallel_degree_limit,
                                     int64_t &parallel)
{
  int ret = OB_SUCCESS;
  parallel = ObGlobalHint::DEFAULT_PARALLEL;
  if (OB_ISNULL(table_partition_info_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected params", K(ret), K(table_partition_info_));
  } else if (ObGlobalHint::DEFAULT_PARALLEL == parallel_degree_limit
             || is_virtual_table(table_partition_info_->get_ref_table_id())) {
    /* do nothing */
    LOG_TRACE("Auto DOP get_random_parallel", K(parallel_degree_limit),
                                    K(table_partition_info_->get_ref_table_id()),
                                    K(is_virtual_table(table_partition_info_->get_ref_table_id())));
  } else {
    const int64_t part_cnt = table_partition_info_->get_phy_tbl_location_info().get_partition_cnt();
    const bool limit_beyond_part_cnt = ObGlobalHint::UNSET_PARALLEL == parallel_degree_limit
                                       || parallel_degree_limit > part_cnt;
    int64_t parallel_type = ObRandom::rand(0, limit_beyond_part_cnt ? 2 : 1);
    switch (parallel_type) {
      case 0: {
        parallel = 1;
        break;
      }
      case 1: {
        if (part_cnt > 1) {
          if (limit_beyond_part_cnt) {
            parallel = ObRandom::rand(2, part_cnt);
          } else {
            parallel = ObRandom::rand(2, parallel_degree_limit);
          }
          break;
        }
      }
      default:  {
        parallel = part_cnt + 1;
        break;
      }
    }
    LOG_TRACE("Auto DOP get_random_parallel", K(parallel_degree_limit), K(part_cnt),
                                            K(parallel_type), K(parallel));
  }
  return ret;
}

int ObJoinOrder::get_parallel_from_available_access_paths(int64_t &parallel) const
{
  int ret = OB_SUCCESS;
  parallel = ObGlobalHint::UNSET_PARALLEL;
  AccessPath *path = NULL;
  for (int64_t i = 0; OB_SUCC(ret) && i < available_access_paths_.count(); ++i) {
    if (OB_ISNULL(path = available_access_paths_.at(i))) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("get unexpected null", K(ret));
    } else if (path->is_inner_path_ || path->use_das_) {
      /* do nothing */
    } else {
      parallel = path->parallel_;
      break;
    }
  }
  return ret;
}

// compute auto dop
int ObJoinOrder::compute_access_path_parallel(ObIArray<AccessPath *> &access_paths,
                                              int64_t &parallel)
{
  int ret = OB_SUCCESS;
  parallel = ObGlobalHint::UNSET_PARALLEL;
  ObOptimizerContext *opt_ctx = NULL;
  ObSQLSessionInfo *session_info = NULL;
  int64_t cur_min_parallel = ObGlobalHint::UNSET_PARALLEL;
  if (OB_ISNULL(get_plan()) || OB_ISNULL(opt_ctx = &get_plan()->get_optimizer_context())
      || OB_ISNULL(session_info = opt_ctx->get_session_info())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected params", K(ret), K(get_plan()), K(opt_ctx), K(session_info));
  } else if (!opt_ctx->get_global_hint().has_parallel_hint() &&
             OB_FAIL(OB_E(EventTable::EN_ENABLE_AUTO_DOP_FORCE_PARALLEL_PLAN) OB_SUCCESS)) {
    ret = OB_SUCCESS;
    if (!session_info->is_user_session()) {
      parallel = ObGlobalHint::DEFAULT_PARALLEL;
    } else if (OB_FAIL(get_random_parallel(opt_ctx->get_session_parallel_degree_limit(), parallel))) {
      LOG_WARN("failed to get random parallel", K(ret));
    }
    LOG_TRACE("Auto DOP trace point", K(session_info->is_user_session()), K(parallel));
  } else if (OB_FAIL(get_parallel_from_available_access_paths(cur_min_parallel))) {
    LOG_WARN("failed to get parallel from available access paths", K(ret));
  } else {
    int64_t calc_parallel = ObGlobalHint::UNSET_PARALLEL;
    int64_t das_path_cnt = 0;
    AccessPath *path = NULL;
    bool finish = false;
    OPT_TRACE_TITLE("begin compute auto dop for table");
    for (int64_t i = 0; !finish && OB_SUCC(ret) && i < access_paths.count(); i++) {
      if (OB_ISNULL(path = access_paths.at(i))) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("get unexpected null", K(path), K(ret));
      } else if (path->use_das_) {
        ++das_path_cnt;
      } else if (OB_FAIL(path->compute_parallel_degree(cur_min_parallel, calc_parallel))) {
        LOG_WARN("failed to compute parallel degree", K(ret));
      } else {
        LOG_TRACE("finish compute one path parallel degree", K(i), K(cur_min_parallel), K(calc_parallel),
                                                            K(path->table_id_), K(path->index_id_));
        cur_min_parallel = calc_parallel;
        finish = ObGlobalHint::DEFAULT_PARALLEL == calc_parallel;
      }
    }
    OPT_TRACE_TITLE("end compute auto dop for table");
    if (OB_SUCC(ret)) {
      parallel = access_paths.count() == das_path_cnt ? ObGlobalHint::DEFAULT_PARALLEL : calc_parallel;
      LOG_TRACE("finish compute paths parallel for Auto DOP", K(parallel), K(das_path_cnt),
                                                            K(access_paths.count()));
    }
  }
  return ret;
}

int ObJoinOrder::compute_base_table_parallel_and_server_info(const OpParallelRule op_parallel_rule,
                                                             const int64_t parallel,
                                                             AccessPath *path)
{
  int ret = OB_SUCCESS;
  int64_t final_parallel = parallel;
  ObSqlSchemaGuard *schema_guard = NULL;
  const ObTableSchema *index_schema = NULL;
  if (OB_ISNULL(get_plan()) || OB_ISNULL(path) || OB_ISNULL(path->table_partition_info_) ||
      OB_ISNULL(schema_guard = get_plan()->get_optimizer_context().get_sql_schema_guard())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected null param", K(ret), K(get_plan()), K(path));
  } else if (OB_FAIL(path->table_partition_info_->get_all_servers(path->server_list_))) {
    LOG_WARN("failed to get all servers", K(ret));
  } else if (path->use_das_) {
    // for das access path, fill server_list_
    path->op_parallel_rule_ = OpParallelRule::OP_DAS_DOP;
    path->parallel_ = ObGlobalHint::DEFAULT_PARALLEL;
    path->server_cnt_ = path->server_list_.count();
    path->available_parallel_ = ObGlobalHint::DEFAULT_PARALLEL;
    if (path->is_index_merge_path()) {
      ObSEArray<AccessPath *, 4> index_merge_scan_ap;
      if(OB_FAIL(static_cast<IndexMergePath*>(path)->get_all_scan_access_paths(index_merge_scan_ap))) {
        LOG_WARN("failed to get all scan access paths", K(ret));
      }
      for (int64_t i = 0; OB_SUCC(ret) && i < index_merge_scan_ap.count(); ++i) {
        if (OB_ISNULL(index_merge_scan_ap.at(i))) {
          ret = OB_ERR_UNEXPECTED;
          LOG_WARN("get unexpected null", K(ret));
        } else if (OB_FAIL(index_merge_scan_ap.at(i)->server_list_.assign(path->server_list_))) {
          LOG_WARN("failed to assign server list", K(ret));
        } else {
          index_merge_scan_ap.at(i)->op_parallel_rule_ = OpParallelRule::OP_DAS_DOP;
          index_merge_scan_ap.at(i)->parallel_ = ObGlobalHint::DEFAULT_PARALLEL;
          index_merge_scan_ap.at(i)->server_cnt_ = path->server_list_.count();
          index_merge_scan_ap.at(i)->available_parallel_ = ObGlobalHint::DEFAULT_PARALLEL;
        }
      }
    }
  } else if (OB_FAIL(schema_guard->get_table_schema(path->index_id_, index_schema))) {
    LOG_WARN("failed to get table schema", K(ret));
  } else if (OB_ISNULL(index_schema)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret));
  } else {
    if (OpParallelRule::OP_TABLE_DOP == op_parallel_rule) {
      final_parallel = index_schema->get_dop();
    }
    if ((index_schema->is_spatial_index() || index_schema->is_vec_index()) && index_schema->get_partition_num() <  final_parallel) {
      final_parallel = index_schema->get_partition_num();
    }
    if (final_parallel < ObGlobalHint::DEFAULT_PARALLEL) {
      final_parallel = ObGlobalHint::DEFAULT_PARALLEL;
    }
    path->op_parallel_rule_ = op_parallel_rule;
    path->parallel_ = final_parallel;
    path->server_cnt_ = path->server_list_.count();
    path->available_parallel_ = std::max(parallel, final_parallel);
  }
  return ret;
}

int ObJoinOrder::get_valid_index_ids_with_no_index_hint(ObSqlSchemaGuard &schema_guard,
                                                        const uint64_t ref_table_id,
                                                        uint64_t *tids,
                                                        const int64_t index_count,
                                                        const ObIArray<uint64_t> &ignore_index_ids,
                                                        ObIArray<uint64_t> &valid_index_ids)
{
  int ret = OB_SUCCESS;
  valid_index_ids.reuse();
  const share::schema::ObTableSchema *index_schema = NULL;
  if (!ObOptimizerUtil::find_item(ignore_index_ids, ref_table_id) &&
      OB_FAIL(valid_index_ids.push_back(ref_table_id))) {
    LOG_WARN("fail to push back index id", K(ret));
  }
  for (int64_t i = 0; OB_SUCC(ret) && i < index_count; ++i) {
    uint64_t index_id = tids[i];
    if (ObOptimizerUtil::find_item(ignore_index_ids, index_id)) {
      /* do nothing */
    } else if (OB_FAIL(schema_guard.get_table_schema(index_id, index_schema))
                || OB_ISNULL(index_schema)) {
      ret = OB_SCHEMA_ERROR;
      LOG_WARN("fail to get table schema", K(index_id), K(ret));
    } else if (index_schema->is_multivalue_index()) {
      /* do nothing */
    } else if (OB_FAIL(valid_index_ids.push_back(index_id))) {
      LOG_WARN("fail to push back index id", K(ret));
    }
  }
  return ret;
}

int ObJoinOrder::extract_geo_schema_info(const uint64_t table_id,
                                         const uint64_t index_id,
                                         ObWrapperAllocator &wrap_allocator,
                                         ColumnIdInfoMapAllocer &map_alloc,
                                         ColumnIdInfoMap &geo_columnInfo_map)
{
  int ret = OB_SUCCESS;
  ObOptimizerContext *opt_ctx = NULL;
  ObSqlSchemaGuard *schema_guard = NULL;
  const share::schema::ObTableSchema *table_schema = NULL;
  const share::schema::ObTableSchema *index_schema = NULL;
  if (OB_ISNULL(get_plan()) ||
      OB_ISNULL(opt_ctx = &get_plan()->get_optimizer_context()) ||
      OB_ISNULL(schema_guard = opt_ctx->get_sql_schema_guard())) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("get unexpected null", K(get_plan()), K(opt_ctx), K(schema_guard), K(ret));
  } else if (OB_FAIL(schema_guard->get_table_schema(table_id, table_schema))) {
    LOG_WARN("fail to get table schema", K(table_id), K(ret));
  } else if (OB_FAIL(schema_guard->get_table_schema(index_id, index_schema))) {
    LOG_WARN("fail to get index schema", K(index_id), K(ret));
  } else if (OB_ISNULL(table_schema) || OB_ISNULL(index_schema)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(table_schema), K(index_schema), K(ret));
  } else if (OB_FAIL(geo_columnInfo_map.create(OB_DEFAULT_SRID_BUKER, &map_alloc, &wrap_allocator))) {
    LOG_WARN("Init column_info_map failed", K(ret));
  } else {
    const ObRowkeyInfo* rowkey_info = NULL;
    const ObColumnSchemaV2 *column_schema = NULL;
    rowkey_info = &index_schema->get_rowkey_info();
    uint64_t column_id = OB_INVALID_ID;
    // traverse index schema
    for (int col_idx = 0;  OB_SUCC(ret) && col_idx < rowkey_info->get_size(); ++col_idx) {
      if (OB_FAIL(rowkey_info->get_column_id(col_idx, column_id))) {
        LOG_WARN("Failed to get column id", K(ret));
      } else if (OB_ISNULL(column_schema = (index_schema->get_column_schema(column_id)))) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("failed to get column schema", K(column_id), K(ret));
      } else if (column_schema->is_spatial_cellid_column()) {
        const ObColumnSchemaV2 *geo_column_schema = NULL;
        uint64_t geo_col_id = column_schema->get_geo_col_id();
        ObGeoColumnInfo column_info;
        if (OB_ISNULL(geo_column_schema = (table_schema->get_column_schema(geo_col_id)))) {
          ret = OB_ERR_UNEXPECTED;
          LOG_WARN("failed to get column schema", K(column_id), K(ret));
        } else if (geo_column_schema->is_geometry()) {
          column_info.srid_ = geo_column_schema->get_srid();
          column_info.cellid_columnId_ = column_id;
          if (OB_FAIL(geo_columnInfo_map.set_refactored(geo_col_id, column_info))) {
            LOG_WARN("failed to set columnId_map", K(geo_col_id), K(ret));
          } else if (OB_FAIL(geo_columnInfo_map.set_refactored(column_id, column_info))) {
            LOG_WARN("failed to set columnId_map", K(column_id), K(ret));
          }
        }
      }
    }
  }
  return ret;
}

int ObJoinOrder::get_query_range_info(const uint64_t table_id,
                                      const uint64_t base_table_id,
                                      const uint64_t index_id,
                                      QueryRangeInfo &range_info,
                                      PathHelper &helper)
{
  int ret = OB_SUCCESS;
  const ObDMLStmt *stmt = NULL;
  ObOptimizerContext *opt_ctx = NULL;
  ObSqlSchemaGuard *schema_guard = NULL;
  ObExecContext *exec_ctx = NULL;
  ObQueryRangeProvider *query_range_provider = NULL;
  const share::schema::ObTableSchema *index_schema = NULL;
  const share::schema::ObTableSchema *table_schema = NULL;
  ObQueryRangeArray &ranges = range_info.get_ranges();
  ObQueryRangeArray &ss_ranges = range_info.get_ss_ranges();
  ObIArray<ColumnItem> &range_columns = range_info.get_range_columns();
  bool is_geo_index = false;
  bool is_multi_index = false;
  bool is_fts_index = false;
  bool is_domain_index = false;
  ObWrapperAllocator wrap_allocator(*allocator_);
  ColumnIdInfoMapAllocer map_alloc(OB_MALLOC_NORMAL_BLOCK_SIZE, wrap_allocator);
  ColumnIdInfoMap domain_columnInfo_map;
  if (OB_ISNULL(get_plan()) ||
      OB_ISNULL(opt_ctx = &get_plan()->get_optimizer_context()) ||
      OB_ISNULL(schema_guard = opt_ctx->get_sql_schema_guard()) ||
      OB_ISNULL(exec_ctx = opt_ctx->get_exec_ctx())) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("get unexpected null", K(get_plan()), K(opt_ctx), K(schema_guard), K(exec_ctx), K(ret));
  } else if (OB_FAIL(schema_guard->get_table_schema(index_id, index_schema,
                                    ObSqlSchemaGuard::is_link_table(get_plan()->get_stmt(), table_id)))) {
    LOG_WARN("fail to get table schema", K(index_id), K(ret));
  } else if (OB_FAIL(schema_guard->get_table_schema(base_table_id, table_schema,
                                    ObSqlSchemaGuard::is_link_table(get_plan()->get_stmt(), table_id)))) {
    LOG_WARN("fail to get table schema", K(index_id), K(ret));
  } else if (OB_ISNULL(index_schema) || OB_ISNULL(table_schema)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(index_schema), K(ret));
  } else if (OB_FAIL(get_plan()->get_index_column_items(opt_ctx->get_expr_factory(),
                                                        table_id, *index_schema, range_columns))) {
    LOG_WARN("failed to generate rowkey column items", K(ret));
  } else if ((is_geo_index = index_schema->is_spatial_index())
              && OB_FAIL(extract_geo_schema_info(base_table_id,
                                                 index_id,
                                                 wrap_allocator,
                                                 map_alloc,
                                                 domain_columnInfo_map))) {
    LOG_WARN("failed to extract geometry schema info", K(ret), K(table_id), K(index_id));
  } else if (FALSE_IT(is_multi_index = index_schema->is_multivalue_index())) {
  } else if (FALSE_IT(is_fts_index = index_schema->is_fts_index_aux())) {
  } else {
    const ObSQLSessionInfo *session = opt_ctx->get_session_info();
    const ObDataTypeCastParams dtc_params = ObBasicSessionInfo::create_dtc_params(session);
    bool all_single_value_range = false;
    int64_t equal_prefix_count = 0;
    int64_t equal_prefix_null_count = 0;
    int64_t range_prefix_count = 0;
    bool contain_always_false = false;
    bool has_exec_param = false;
    int64_t out_index_prefix = -1;
    bool is_domain_index = (is_geo_index || is_multi_index || is_fts_index);
    int64_t range_cnt = 0;

    common::ObSEArray<ObRawExpr *, 4> agent_table_filter;
    if (!is_domain_index && OB_FAIL(extract_preliminary_query_range(range_columns,
                                                                 helper.filters_,
                                                                 range_info.get_expr_constraints(),
                                                                 table_id,
                                                		             query_range_provider,
                                                                 index_id,
                                                                 index_schema,
                                                                 out_index_prefix))) {
      LOG_WARN("failed to extract query range", K(ret), K(index_id));
    } else if (is_geo_index && OB_FAIL(extract_geo_preliminary_query_range(range_columns,
                                                                      helper.filters_,
                                                                      domain_columnInfo_map,
                                                                      query_range_provider))) {
      LOG_WARN("failed to extract query range", K(ret), K(index_id));
    } else if (is_multi_index
               && OB_FAIL(extract_multivalue_preliminary_query_range(range_columns,
                                                                     helper.filters_,
                                                                     query_range_provider))) {
      LOG_WARN("failed to extract query range", K(ret), K(index_id));
    } else if(is_fts_index && OB_FAIL(extract_fts_preliminary_query_range(range_columns,
                                                                          helper.filters_,
                                                                          table_schema,
                                                                          index_schema,
                                                                          helper,
                                                                          query_range_provider))) {
      LOG_WARN("failed to extract query range", K(ret), K(index_id));
    } else if (OB_ISNULL(query_range_provider)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("get unexpected null", K(query_range_provider), K(ret));
    } else if (OB_FAIL(query_range_provider->get_tablet_ranges(*allocator_,
                                                               *exec_ctx,
                                                               ranges,
                                                               all_single_value_range,
                                                               dtc_params))) {
      LOG_WARN("failed to final extract query range", K(ret));
    } else if (OB_FAIL(query_range_provider->get_ss_tablet_ranges(*allocator_,
                                                                  *exec_ctx,
                                                                  ss_ranges,
                                                                  dtc_params))) {
      LOG_WARN("failed to final extract index skip query range", K(ret));
    } else if (OB_FAIL(ObOptimizerUtil::check_prefix_ranges_count(range_info.get_ranges(),
                                                                  equal_prefix_count,
                                                                  range_prefix_count,
                                                                  contain_always_false))) {
      LOG_WARN("failed to compute query range prefix count", K(ret));
    } else if (OB_FAIL(check_has_exec_param(*query_range_provider, has_exec_param))) {
      LOG_WARN("failed to check has exec param", K(ret));
    } else if (!has_exec_param) {
      // No exec param then use real query range to calculate equal prefix count
      // If there is exec param then use the shape of query range to calculate equal prefix count
      range_info.set_equal_prefix_count(equal_prefix_count);
      range_info.set_range_prefix_count(range_prefix_count);
      range_info.set_contain_always_false(contain_always_false);
    } else if (OB_FAIL(get_preliminary_prefix_info(*query_range_provider, range_info))) {
      LOG_WARN("failed to get preliminary prefix info", K(ret));
    }
    if (OB_SUCC(ret)) {
      range_info.set_valid();
      range_info.set_pre_range_graph(static_cast<ObPreRangeGraph*>(query_range_provider));
      range_info.set_index_column_count(index_schema->is_index_table() ?
                                        index_schema->get_index_column_num() :
                                        index_schema->get_rowkey_column_num());
      range_info.set_index_prefix(out_index_prefix);
      int64_t index_equal_prefix_cnt = MIN(range_info.get_index_column_count(), equal_prefix_count);
      if (OB_FAIL(ObOptimizerUtil::check_equal_prefix_null_count(range_info.get_ranges(),
                                                                 index_equal_prefix_cnt,
                                                                 equal_prefix_null_count))) {

      } else if (OB_FAIL(get_query_range_count(range_info, range_cnt))) {
        LOG_WARN("failed to get query range count", K(ret));
      } else {
        range_info.set_range_count(range_cnt);
        range_info.set_equal_prefix_null_count(equal_prefix_null_count);
      }
      LOG_TRACE("succeed to get query range", K(ranges), K(ss_ranges), K(helper.filters_),
                  KPC(query_range_provider), K(range_columns),
                  K(query_range_provider->get_range_exprs()), K(table_id), K(index_id));
    } else {
      if (NULL != query_range_provider) {
        query_range_provider->~ObQueryRangeProvider();
        query_range_provider = NULL;
      }
    }
  }
  return ret;
}

int ObJoinOrder::check_has_exec_param(const ObQueryRangeProvider &query_range,
                                      bool &has_exec_param)
{
  int ret = OB_SUCCESS;
  has_exec_param = false;
  const ObIArray<ObRawExpr*> &range_exprs = query_range.get_range_exprs();
  for (int64_t i = 0; OB_SUCC(ret) && !has_exec_param && i < range_exprs.count(); ++i) {
    ObRawExpr *expr = range_exprs.at(i);
    if (OB_ISNULL(expr)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected null expr", K(ret));
    } else if (expr->has_flag(CNT_DYNAMIC_PARAM)) {
      has_exec_param = true;
    }
  }
  return ret;
}

int ObJoinOrder::get_preliminary_prefix_info(ObQueryRangeProvider &query_range_provider,
                                             QueryRangeInfo &range_info)
{
  int ret = OB_SUCCESS;
  int64_t equal_prefix_count = 0;
  int64_t range_prefix_count = 0;
  bool contain_always_false = false;
  if (OB_FAIL(query_range_provider.get_prefix_info(equal_prefix_count,
                                                   range_prefix_count,
                                                   contain_always_false))) {
    LOG_WARN("failed to get prefix info");
  } else {
    range_info.set_equal_prefix_count(equal_prefix_count);
    range_info.set_range_prefix_count(range_prefix_count);
    range_info.set_contain_always_false(contain_always_false);
    LOG_TRACE("success to get preliminary prefix info", K(equal_prefix_count),
                      K(range_prefix_count), K(contain_always_false));
  }
  return ret;
}

int ObJoinOrder::add_table_by_heuristics(const uint64_t table_id,
                                         const uint64_t ref_table_id,
                                         const ObIndexInfoCache &index_info_cache,
                                         const ObIArray<uint64_t> &candi_index_ids,
                                         ObIArray<uint64_t> &valid_index_ids,
                                         PathHelper &helper)
{
  int ret = OB_SUCCESS;
  uint64_t index_to_use = OB_INVALID_ID;
  if (OB_UNLIKELY(OB_INVALID_ID == table_id) ||
      OB_UNLIKELY(OB_INVALID_ID == ref_table_id) ||
      OB_ISNULL(helper.table_opt_info_)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("Invalid table id", K(table_id), K(ref_table_id), K(ret));
  } else {
    if (is_virtual_table(ref_table_id)) {
      // check virtual table heuristics
      if (OB_FAIL(virtual_table_heuristics(table_id, ref_table_id, index_info_cache,
                                           candi_index_ids, index_to_use))) {
        LOG_WARN("failed to check virtual table heuristics", K(table_id), K(ref_table_id), K(ret));
      } else if (OB_INVALID_ID != index_to_use) {
        helper.table_opt_info_->optimization_method_ = OptimizationMethod::RULE_BASED;
        helper.table_opt_info_->heuristic_rule_ = HeuristicRule::VIRTUAL_TABLE_HEURISTIC;
        OPT_TRACE("choose index using heuristics for virtual table", index_to_use);
        LOG_TRACE("OPT:[RBO] choose index using heuristics for virtual table",
                  K(table_id),
                  K(ref_table_id),
                  K(index_to_use));
      }
    }
    if (OB_SUCC(ret)) {
      if (OB_INVALID_ID != index_to_use) {
        if (OB_FAIL(valid_index_ids.push_back(index_to_use))) {
          LOG_WARN("failed to push back index id", K(ret));
        } else {
          OPT_TRACE("table added using heuristics", index_to_use);
          LOG_TRACE("OPT:[RBO] table added using heuristics",
               K(table_id), K(ref_table_id), K(index_to_use));
        }
      } else {
        OPT_TRACE("table not added using heuristics", ref_table_id);
        LOG_TRACE("OPT:[RBO] table not added using heuristics", K(table_id), K(ref_table_id));
      }
    }
  }
  return ret;
}

int ObJoinOrder::virtual_table_heuristics(const uint64_t table_id,
                                          const uint64_t ref_table_id,
                                          const ObIndexInfoCache &index_info_cache,
                                          const ObIArray<uint64_t> &valid_index_ids,
                                          uint64_t &index_to_use)
{
  int ret = OB_SUCCESS;
  ObSqlSchemaGuard *schema_guard = NULL;
  if (OB_ISNULL(get_plan())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("null pointer", K(get_plan()));
  } else if (OB_ISNULL(schema_guard = OPT_CTX.get_sql_schema_guard())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("schema guard should not be null", K(ret));
  } else if (OB_UNLIKELY(OB_INVALID_ID == table_id)
      || OB_UNLIKELY(OB_INVALID_ID == ref_table_id)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("Invalid table id", K(table_id), K(ref_table_id), K(ret));
  } else {
    // for virtual table, choose any index to use
    uint64_t idx_id = OB_INVALID_ID;
    const ObTableSchema *index_schema = NULL;
    const QueryRangeInfo *query_range_info = NULL;
    LOG_TRACE("OPT:[VT] begin search index for virtual table", K(valid_index_ids.count()));
    for (int64_t i = 0; OB_SUCC(ret) && OB_INVALID_ID == idx_id &&  i < valid_index_ids.count(); ++i) {
      if (OB_UNLIKELY(OB_INVALID_ID == valid_index_ids.at(i))) {
        LOG_WARN("index id invalid", K(table_id), K(valid_index_ids.at(i)), K(ret));
      } else if (OB_FAIL(schema_guard->get_table_schema(valid_index_ids.at(i), index_schema))
                 || OB_ISNULL(index_schema)) {
        LOG_WARN("fail to get table schema", K(index_schema), K(valid_index_ids.at(i)), K(ret));
      } else if (!index_schema->is_index_table()) {
        /*do nothing*/
      } else if (OB_FAIL(index_info_cache.get_query_range(table_id, valid_index_ids.at(i),
                                                          query_range_info))) {
        LOG_WARN("failed to get query range", K(ret));
      } else if (OB_ISNULL(query_range_info)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("query_range_info should not be null", K(ret));
      } else if (query_range_info->is_index_column_get() ||
          (query_range_info->has_valid_range_condition() &&
          virtual_table_index_can_range_scan(valid_index_ids.at(i)))) {
        idx_id = valid_index_ids.at(i);
        LOG_TRACE("OPT:[VT] found index to use", K(idx_id));
      } else {
        LOG_TRACE("OPT:[VT] not found index to use", K(idx_id));
      }
    }
    if (OB_INVALID_ID == idx_id) {
      index_to_use = ref_table_id;
    } else {
      index_to_use = idx_id;
    }
  }

  return ret;
}

int ObJoinOrder::try_prune_non_unique_index(const uint64_t table_id,
                                            const ObIndexInfoCache &index_info_cache,
                                            const common::ObIArray<uint64_t> &valid_index_ids,
                                            common::ObIArray<uint64_t> &candidate_index_ids,
                                            bool &use_unique_index)
{
  int ret = OB_SUCCESS;
  use_unique_index = false;
  for(int64_t i = 0; OB_SUCC(ret) && i < valid_index_ids.count(); ++i) {
    uint64_t index_id = valid_index_ids.at(i);
    IndexInfoEntry *index_info_entry = nullptr;
    const OrderingInfo *temp_ordering_info = nullptr;
    if (OB_FAIL(index_info_cache.get_index_info_entry(table_id,
                                                      index_id,
                                                      index_info_entry))) {
      LOG_WARN("failed to get query range info", K(table_id), K(index_id), K(ret));
    } else if (OB_ISNULL(index_info_entry)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("index info is NULL", K(ret));
    } else if (index_info_entry->get_range_info().get_unique_range_rowcnt() > 0) {
      if (OB_FAIL(candidate_index_ids.push_back(index_id))) {
        LOG_WARN("failed to push back", K(ret));
      } else {
        use_unique_index = true;
      }
    }
  }
  if (OB_SUCC(ret) && candidate_index_ids.empty()) {
    if (OB_FAIL(candidate_index_ids.assign(valid_index_ids))) {
      LOG_WARN("failed to assign", K(ret));
    }
  }
  return ret;
}

int ObJoinOrder::check_index_subset(const OrderingInfo *first_ordering_info,
                                    const int64_t first_index_key_count,
                                    const OrderingInfo *second_ordering_info,
                                    const int64_t second_index_key_count,
                                    DominateRelation &status)
{
  int ret = OB_SUCCESS;
  status = DominateRelation::OBJ_UNCOMPARABLE;
  bool is_subset = false;
  if (OB_ISNULL(first_ordering_info) || OB_ISNULL(second_ordering_info)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("Invalid table id", K(first_ordering_info), K(second_ordering_info));
  } else if (first_index_key_count <= second_index_key_count) {
    if (OB_FAIL(ObOptimizerUtil::prefix_subset_exprs(first_ordering_info->get_index_keys(),
                                                     first_index_key_count,
                                                     second_ordering_info->get_index_keys(),
                                                     second_index_key_count,
                                                     is_subset))) {
      LOG_WARN("failed to compare index keys", K(ret));
    } else if (is_subset) {
      status = first_index_key_count == second_index_key_count ?
          DominateRelation::OBJ_EQUAL : DominateRelation::OBJ_LEFT_DOMINATE;
    } else {
      status = DominateRelation::OBJ_UNCOMPARABLE;
    }
  } else {
    if (OB_FAIL(ObOptimizerUtil::prefix_subset_exprs(second_ordering_info->get_index_keys(),
                                                     second_index_key_count,
                                                     first_ordering_info->get_index_keys(),
                                                     first_index_key_count,
                                                     is_subset))) {
      LOG_WARN("failed to compare index keys", K(ret));
    } else if (is_subset) {
      status = DominateRelation::OBJ_RIGHT_DOMINATE;
    } else {
      status = DominateRelation::OBJ_UNCOMPARABLE;
    }
  }
  LOG_TRACE("check index subset", K(*first_ordering_info), K(*second_ordering_info),
      K(first_index_key_count), K(second_index_key_count));
  return ret;
}

int ObJoinOrder::will_use_das(const uint64_t table_id,
                             const uint64_t index_id,
                             const ObIndexInfoCache &index_info_cache,
                             PathHelper &helper,
                             bool &create_das_path,
                             bool &create_basic_path)
{
  int ret = OB_SUCCESS;
  create_das_path = false;
  create_basic_path = false;
  if (OB_ISNULL(get_plan())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected null", K(ret), K(get_plan()));
  } else if (OB_FAIL(check_exec_force_use_das(table_id, create_das_path, create_basic_path))) {
    LOG_WARN("failed to check exec force use das", K(ret));
  } else if (create_das_path || create_basic_path) {
    LOG_TRACE("will use das by execution", K(create_das_path), K(create_basic_path));
  } else if (OB_FAIL(get_plan()->get_log_plan_hint().check_use_das(table_id, create_das_path, create_basic_path))) {
    LOG_WARN("failed to check hint use das", K(ret));
  } else if (create_das_path || create_basic_path) {
    LOG_TRACE("will use das by hint", K(create_das_path), K(create_basic_path));
  } else if (OB_UNLIKELY(!get_plan()->get_optimizer_context().is_enable_distributed_das_scan())) {
    create_das_path = false;
    create_basic_path = true;
    LOG_TRACE("disable das scan by tenant config", K(create_das_path), K(create_basic_path));
  } else if (OB_FAIL(check_opt_rule_use_das(table_id,
                                            index_id,
                                            index_info_cache,
                                            helper.filters_,
                                            (get_plan()->get_is_rescan_subplan() || helper.is_inner_path_),
                                            create_das_path,
                                            create_basic_path))) {
    LOG_WARN("failed to check opt rule use das", K(ret));
  } else {
    LOG_TRACE("will use das by opt rule", K(create_das_path), K(create_basic_path));
  }
  return ret;
}

int ObJoinOrder::check_exec_force_use_das(const uint64_t table_id,
                                          bool &create_das_path,
                                          bool &create_basic_path)
{
  int ret = OB_SUCCESS;
  create_das_path = false;
  create_basic_path = false;
  const TableItem *table_item = nullptr;
  ObSQLSessionInfo *session_info = NULL;
  if (OB_ISNULL(get_plan()) || OB_ISNULL(get_plan()->get_stmt()) ||
      OB_ISNULL(session_info = get_plan()->get_optimizer_context().get_session_info())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(get_plan()), K(session_info), K(ret));
  } else if (OB_ISNULL(table_item = get_plan()->get_stmt()->get_table_item_by_id(table_id))) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("table_item is null", K(ret), K(table_id));
  } else {
    ObOptimizerContext &opt_ctx = get_plan()->get_optimizer_context();
    // TODO: access virtual table by remote das task is not supported, it will report 4016 error in execute server
    // Ensure that the following scenarios will not combined with virtual table
    bool force_das_tsc = opt_ctx.in_nested_sql() || //contain nested sql(pl udf or in nested sql), trigger or foreign key in the top sql not force to use DAS TSC
                         opt_ctx.has_pl_udf() ||
                         opt_ctx.has_dblink() ||
                         opt_ctx.has_subquery_in_function_table() ||  //has function table
                         opt_ctx.has_cursor_expression() ||
                         (opt_ctx.has_var_assign() && session_info->is_var_assign_use_das_enabled()) ||
                         (opt_ctx.is_batched_multi_stmt() && table_item->is_basic_table()) || //batch update table(multi queries or arraybinding)
                         (table_item->for_update_ && table_item->skip_locked_ && session_info->get_pl_context()) // select for update skip locked stmt in PL use das force
                         ;
    bool is_select_sample_scan = get_plan()->get_stmt()->is_select_stmt()
                                && ((NULL != table_item->sample_info_ && !table_item->sample_info_->is_no_sample()) //block(row) sample scan do not support DAS TSC
                                    || (opt_ctx.is_online_ddl() && opt_ctx.get_root_stmt()->is_insert_stmt()) // online ddl plan use sample table scan, create index not support DAS TSC
                                   );

    if (EXTERNAL_TABLE == table_item->table_type_
        || is_select_sample_scan
        || is_virtual_table(table_item->ref_id_)) {
      create_das_path = false;
      create_basic_path = true;
    } else if (force_das_tsc) { //this sql force to use DAS TSC
      create_das_path = true;
      create_basic_path = false;
    }
  }
  return ret;
}

int ObJoinOrder::check_opt_rule_use_das(const uint64_t table_id,
                                        const uint64_t index_id,
                                        const ObIndexInfoCache &index_info_cache,
                                        const ObIArray<ObRawExpr*> &filters,
                                        const bool is_rescan,
                                        bool &create_das_path,
                                        bool &create_basic_path)
{
  int ret = OB_SUCCESS;
  create_das_path = false;
  create_basic_path = false;
  IndexInfoEntry *index_info_entry = NULL;
  if (is_rescan) {
    create_das_path = true;
    create_basic_path = (table_meta_info_.is_broadcast_table_
                         || (OB_SUCCESS != (OB_E(EventTable::EN_GENERATE_PLAN_WITH_NLJ) OB_SUCCESS)))
                        ? false : true;
  } else if (OB_FAIL(index_info_cache.get_index_info_entry(table_id, index_id, index_info_entry))) {
    LOG_WARN("failed to get index info entry", K(table_id), K(index_id), K(ret));
  } else if (OB_ISNULL(index_info_entry)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected null", K(ret), K(index_info_entry));
  } else if (OB_FAIL(check_use_das_by_false_startup_filter(*index_info_entry, filters, create_das_path))) {
    LOG_WARN("failed to check use das by false startup filter", K(ret));
  } else if (create_das_path) {
    /* do nothing */
  } else if (index_info_entry->is_index_global() && index_info_entry->is_index_back()) {
    int64_t explicit_dop = ObGlobalHint::UNSET_PARALLEL;
    if (OB_FAIL(get_explicit_dop_for_path(index_id, explicit_dop))) {
      LOG_WARN("failed to get explicit dop", K(ret));
    } else if (ObGlobalHint::UNSET_PARALLEL == explicit_dop) {
      // for global index use auto dop, create das path and basic path, after get auto dop result, prune unnecessary path
      create_das_path = true;
      create_basic_path = true;
    } else if (ObGlobalHint::DEFAULT_PARALLEL == explicit_dop) {
      create_das_path = true;
    } else {
      create_basic_path = true;
    }
  } else {
    create_basic_path = true;
  }
  return ret;
}

int ObJoinOrder::check_use_das_by_false_startup_filter(const IndexInfoEntry &index_info_entry,
                                                       const ObIArray<ObRawExpr*> &filters,
                                                       bool &use_das)
{
  int ret = OB_SUCCESS;
  use_das = false;
  if (OB_ISNULL(index_info_entry.get_sharding_info()) || OB_ISNULL(get_plan())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(index_info_entry.get_sharding_info()), K(get_plan()), K(ret));
  } else if (!index_info_entry.get_sharding_info()->is_sharding()) {
    /* do nothing */
  } else {
    ObRawExpr *expr = NULL;
    for (int64_t i = 0; !use_das && OB_SUCC(ret) && i < filters.count(); ++i) {
      if (OB_ISNULL(expr = filters.at(i))) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpect null expr", K(ret));
      } else if (T_BOOL != expr->get_expr_type()) {
        /* do nothing */
      } else if (OB_FAIL(ObOptimizerUtil::check_is_static_false_expr(get_plan()->get_optimizer_context(),
                                                                     *expr,
                                                                     use_das))) {
        LOG_WARN("failed to check is static false", K(ret));
      }
    }
  }

  return ret;
}

int ObJoinOrder::check_can_use_vec_primary_opt(const uint64_t ref_table_id,
                                              PathHelper &helper,
                                              const QueryRangeInfo &range_info,
                                              bool& is_filter_all_rowkey_col)
{
  int ret = OB_SUCCESS;
  is_filter_all_rowkey_col = false;
  if (OB_NOT_NULL(range_info.get_query_range_provider())) {
    is_filter_all_rowkey_col = range_info.get_query_range_provider()->get_range_exprs().count() == helper.filters_.count();
  }
  return ret;
}

int ObJoinOrder::process_basic_vec_info_for_index_merge_node(const ObDMLStmt *stmt,
                                                            const uint64_t table_id,
                                                            const uint64_t ref_table_id,
                                                            IndexMergePath* index_merge_path,
                                                            ObIndexMergeNode* root_node)
{
  int ret = OB_SUCCESS;
  ObSqlSchemaGuard *schema_guard = nullptr;
  const ObTableSchema *index_schema = nullptr;
  const ObTableSchema *vec_index_schema = nullptr;
  const ObSelectStmt *select_stmt = NULL;
  uint64_t vec_index_id = OB_INVALID_ID;
  bool has_aggr = false;
  bool vector_index_match = false;
  ObRawExpr *vector_expr = nullptr;
  ObIndexType index_type = INDEX_TYPE_MAX;

  if (OB_ISNULL(stmt) || OB_ISNULL(index_merge_path) || OB_ISNULL(root_node)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected nullptr", K(get_plan()), K(stmt));
  } else if ((OB_ISNULL(schema_guard = OPT_CTX.get_sql_schema_guard()))) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("NULL pointer error", K(get_plan()), K(stmt));
  } else if (stmt->is_select_stmt() && FALSE_IT(select_stmt = static_cast<const ObSelectStmt*>(stmt))) {
  } else if (nullptr != select_stmt && FALSE_IT(has_aggr = select_stmt->get_aggr_item_size() > 0)) {
  } else if (OB_FALSE_IT(vector_expr = stmt->get_first_vector_expr())) {
  } else if (OB_FAIL(get_vector_index_tid_from_expr(schema_guard, vector_expr, table_id, ref_table_id, has_aggr, vector_index_match, vec_index_id, index_type))) {
    LOG_WARN("failed to get matched vector index table id", K(ret));
  } else if (vec_index_id == OB_INVALID_ID || !vector_index_match) {
    // not vec expr column, ignore
  } else if (OB_FAIL(index_merge_path->vec_idx_info_.vec_index_ids_.push_back(vec_index_id))) {
    LOG_WARN("failed to push_back vec index tale id", K(ret));
  } else if (OB_FAIL(schema_guard->get_table_schema(vec_index_id, vec_index_schema))) {
    LOG_WARN("failed to get index table schema", K(ret), K(vec_index_id));
  } else if (OB_ISNULL(vec_index_schema)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected nullptr to index schema", K(ret));
  } else if (vec_index_schema->is_vec_hnsw_index()) {
    index_merge_path->vec_idx_info_.inited_ = true;
    index_merge_path->vec_idx_info_.vec_extra_info_.set_vec_idx_type(ObVecIndexType::VEC_INDEX_ADAPTIVE_SCAN);
    double selectivity = 0.5;
    if (FAILEDx(ObVectorIndexUtil::set_vector_index_param(vec_index_schema,
                                                                index_merge_path->vec_idx_info_.vec_extra_info_,
                                                                selectivity,
                                                                vector_expr,
                                                                stmt))) {
      LOG_WARN("failed to set vector index parameters", K(ret));
    } else if (OB_FAIL(ObVectorIndexUtil::set_adaptive_try_path(index_merge_path->vec_idx_info_.vec_extra_info_, false))) {
      LOG_WARN("set vector index adaptive try path", K(ret));
    }
  }
  return ret;
}

int ObJoinOrder::process_vec_index_info(const ObDMLStmt *stmt,
                                        const uint64_t table_id,
                                        const uint64_t ref_table_id,
                                        const uint64_t index_id,
                                        PathHelper &helper,
                                        AccessPath &access_path,
                                        const QueryRangeInfo &range_info)
{
  int ret = OB_SUCCESS;

  bool found_scan_match_expr = false;
  ObRawExpr *vector_expr = nullptr;
  ObSqlSchemaGuard *schema_guard = nullptr;
  const ObTableSchema *index_schema = nullptr;
  const ObTableSchema *vec_index_schema = nullptr;
  const ObSelectStmt *select_stmt = NULL;
  bool vector_index_match = false;
  uint64_t vec_index_id = OB_INVALID_ID;
  bool is_weak_read = false;
  ObSQLSessionInfo *session_info = NULL;
  ObIndexType index_type = INDEX_TYPE_MAX;
  bool is_ipivf = false;

  if (OB_ISNULL(stmt) || OB_ISNULL(schema_guard = OPT_CTX.get_sql_schema_guard())
    || OB_ISNULL(session_info = OPT_CTX.get_session_info())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected nullptr", KP(stmt), KP(schema_guard), KP(session_info), K(ret));
  } else if (OB_FAIL(ObTableLocation::get_is_weak_read(*stmt, session_info, OPT_CTX.get_exec_ctx()->get_sql_ctx(), is_weak_read))) {
    LOG_WARN("get is weak read failed", K(ret));
  } else if (OB_FAIL(schema_guard->get_table_schema(index_id, index_schema))) {
    LOG_WARN("failed to get index table schema", K(ret), K(index_id));
  } else if (OB_ISNULL(index_schema)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected nullptr to index schema", K(ret));
  } else if (!index_schema->is_vec_index()) {
    bool has_aggr = false;
    vector_expr = stmt->get_first_vector_expr();
    if (stmt->is_select_stmt() && FALSE_IT(select_stmt = static_cast<const ObSelectStmt*>(stmt))) {
    } else if (nullptr != select_stmt && FALSE_IT(has_aggr = select_stmt->get_aggr_item_size() > 0)) {
    } else if (OB_FAIL(get_vector_index_tid_from_expr(schema_guard,
                                                      vector_expr,
                                                      table_id,
                                                      ref_table_id,
                                                      has_aggr,
                                                      vector_index_match,
                                                      vec_index_id,
                                                      index_type))) {
      LOG_WARN("failed to get matched vector index table id", K(ret));
    } else if (vec_index_id == OB_INVALID_ID || !vector_index_match) {
      // not vec expr column, ignore
    } else if (OB_FAIL(access_path.vec_idx_info_.vec_index_ids_.push_back(vec_index_id))) {
      LOG_WARN("failed to push_back vec index tale id", K(ret));
    }  else if (OB_FAIL(schema_guard->get_table_schema(vec_index_id, vec_index_schema))) {
      LOG_WARN("failed to get index table schema", K(ret), K(vec_index_id));
    } else if (OB_ISNULL(vec_index_schema)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected nullptr to index schema", K(ret));
    } else {
      is_ipivf = ObVectorIndexUtil::is_sindi_index(vec_index_schema);
      ObVecIndexType pre_vec_type = (vec_index_schema->is_vec_hnsw_index() && helper.vec_index_type_ == ObVecIndexType::VEC_INDEX_ADAPTIVE_SCAN) ?
      ObVecIndexType::VEC_INDEX_ADAPTIVE_SCAN : ObVecIndexType::VEC_INDEX_PRE;
      if (index_schema->is_spatial_index()) {
        access_path.vec_idx_info_.vec_extra_info_.is_spatial_index_ = true;
      } else if (index_schema->is_multivalue_index()) {
        access_path.vec_idx_info_.vec_extra_info_.is_multi_value_index_ = true;
      } else if (index_schema->is_fts_index() && !vec_index_schema->is_vec_hnsw_index()) {
        ret = OB_NOT_SUPPORTED;
        LOG_USER_ERROR(OB_NOT_SUPPORTED, "when using vector index, full-text index is");
      }
      if (OB_FAIL(ret)) {
      } else if (index_id == ref_table_id) { // is primary idx
        bool is_filter_all_rowkey_col = true;
        if (OB_FAIL(check_can_use_vec_primary_opt(ref_table_id, helper, range_info, is_filter_all_rowkey_col))) {
          LOG_WARN("fail to check_can_use_vec_primary_opt", K(ret));
        } else {
          access_path.vec_idx_info_.vec_extra_info_.set_can_use_vec_pri_opt(is_filter_all_rowkey_col);
        }
      } // check filter all rowkey col
      access_path.vec_idx_info_.vec_extra_info_.set_vec_idx_type(pre_vec_type);
      access_path.vec_idx_info_.vec_extra_info_.can_extract_range_ = !(range_info.get_query_range_provider()->get_range_exprs().empty());
      access_path.vec_idx_info_.vec_extra_info_.adaptive_try_path_ = helper.vec_idx_try_path_;
    }
  } else {
    // if adaptive filter, should not come in
    ObVecIndexType post_vec_type = ((helper.filters_.count() > 0 || !stmt->get_subquery_exprs().empty()
    || helper.match_expr_infos_.count() > 0 || helper.is_index_merge_) && index_schema->is_vec_hnsw_index()) ?
    ObVecIndexType::VEC_INDEX_POST_ITERATIVE_FILTER : ObVecIndexType::VEC_INDEX_POST_WITHOUT_FILTER;
    vec_index_schema = index_schema;
    access_path.vec_idx_info_.vec_extra_info_.set_vec_idx_type(post_vec_type);
    vector_index_match = true;
  }

  if (OB_FAIL(ret) || !vector_index_match) {
  } else if (OB_ISNULL(vec_index_schema)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("vec_table_schema unexpected null", K(ret));
  } else {
    double selectivity = 0.0;
    if (OB_FAIL(ObOptSelectivity::calculate_selectivity(get_plan()->get_basic_table_metas(),
                                                        get_plan()->get_selectivity_ctx(),
                                                        get_restrict_infos(),
                                                        selectivity,
                                                        get_plan()->get_predicate_selectivities()))) {
      LOG_WARN("failed to calculate selectivity", K(ret));
    } else if (OB_FAIL(ObVectorIndexUtil::set_vector_index_param(vec_index_schema,
                                                                 access_path.vec_idx_info_.vec_extra_info_,
                                                                 selectivity,
                                                                 vector_expr,
                                                                 stmt))) {
      LOG_WARN("failed to set vector index parameters", K(ret));
    } else {
      access_path.vec_idx_info_.inited_ = true;
      access_path.vec_idx_info_.vec_extra_info_.set_row_count(table_meta_info_.table_row_count_);
    }
    // set vec_extra_info_
    if (OB_FAIL(ret)) {
    } else if (is_weak_read) {
      ret = OB_NOT_SUPPORTED;
      LOG_USER_ERROR(OB_NOT_SUPPORTED, "when using vector index, weak read is");
    } else {
      double selectivity = 0.0;
      if (OB_FAIL(ObOptSelectivity::calculate_selectivity(get_plan()->get_basic_table_metas(),
                                                          get_plan()->get_selectivity_ctx(),
                                                          get_restrict_infos(),
                                                          selectivity,
                                                          get_plan()->get_predicate_selectivities()))) {
        LOG_WARN("failed to calculate selectivity", K(ret));
      } else if (OB_FAIL(ObVectorIndexUtil::set_vector_index_param(vec_index_schema,
                                                                   access_path.vec_idx_info_.vec_extra_info_,
                                                                   selectivity,
                                                                   vector_expr,
                                                                   stmt))) {
        LOG_WARN("failed to set vector index parameters", K(ret));
      } else {
        access_path.domain_idx_info_.set_domain_idx_type(DomainIndexType::VEC_INDEX);
        access_path.vec_idx_info_.vec_extra_info_.set_row_count(table_meta_info_.table_row_count_);
      }
    } // not weak read

    if (OB_FAIL(ret)) {
    } else if (access_path.vec_idx_info_.vec_extra_info_.vec_idx_type_ == ObVecIndexType::VEC_INDEX_ADAPTIVE_SCAN
      && (access_path.vec_idx_info_.vec_extra_info_.adaptive_try_path_ == ObVecIdxAdaTryPath::VEC_PATH_UNCHOSEN
      || access_path.vec_idx_info_.vec_extra_info_.adaptive_try_path_ == ObVecIdxAdaTryPath::VEC_INDEX_PRE_FILTER)
      && OB_FAIL(ObVectorIndexUtil::set_adaptive_try_path(access_path.vec_idx_info_.vec_extra_info_, index_id == ref_table_id, is_ipivf))) {
      LOG_WARN("set vector index adaptive try path", K(ret));
    }
  }
  return ret;
}

int ObJoinOrder::create_one_access_path(const uint64_t table_id,
                                        const uint64_t ref_id,
                                        const uint64_t index_id,
                                        const ObIndexInfoCache &index_info_cache,
                                        PathHelper &helper,
                                        AccessPath *&access_path,
                                        bool use_das,
                                        bool use_column_store,
                                        OptSkipScanState use_skip_scan)
{
  int ret = OB_SUCCESS;
  IndexInfoEntry *index_info_entry = NULL;
  access_path = NULL;
  AccessPath *ap = NULL;
  bool is_nl_with_extended_range = false;
  const TableItem *table_item = nullptr;
  if (OB_UNLIKELY(OB_INVALID_ID == ref_id) || OB_UNLIKELY(OB_INVALID_ID == index_id) ||
      OB_ISNULL(get_plan()) || OB_ISNULL(allocator_) || OB_ISNULL(get_plan()->get_stmt()) ||
      OB_ISNULL(table_item = get_plan()->get_stmt()->get_table_item_by_id(table_id))) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ref_id), K(index_id), K(get_plan()), K(allocator_),
                                    K(table_item), K(ret));
  } else if (OB_FAIL(index_info_cache.get_index_info_entry(table_id, index_id,
                                                           index_info_entry))) {
    LOG_WARN("failed to get index info entry", K(table_id), K(index_id), K(ret));
  } else if (OB_ISNULL(index_info_entry) || OB_ISNULL(index_info_entry->get_partition_info())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("index info entry should not be null", K(ret), KPC(index_info_entry));
  } else if (!helper.force_inner_nl_ && helper.is_inner_path_ &&
             (index_info_entry->get_ordering_info().get_index_keys().count() <= 0)) {
    LOG_TRACE("skip adding inner access path due to wrong index key count",
                K(table_id), K(ref_id), KPC(index_info_entry));
  } else if (OB_ISNULL(ap = reinterpret_cast<AccessPath*>(allocator_->alloc(sizeof(AccessPath))))) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    LOG_ERROR("failed to allocate an AccessPath", K(ret));
  } else {
    LOG_TRACE("OPT:start to create access path",
                K(table_id), K(ref_id), K(index_id), K(helper.is_inner_path_), K(use_das));
    const QueryRangeInfo &range_info = index_info_entry->get_range_info();
    const OrderingInfo &ordering_info = index_info_entry->get_ordering_info();
    ap = new(ap) AccessPath(table_id, ref_id, index_id, this, ordering_info.get_scan_direction());
    ap->is_get_ = range_info.is_get();
    ap->is_global_index_ = index_info_entry->is_index_global();
    ap->table_partition_info_ = index_info_entry->get_partition_info();
    ap->use_das_ = use_das;
    ap->contain_das_op_ = ap->use_das_;
    ap->est_cost_info_.is_das_scan_ = ap->use_das_;
    ap->is_hash_index_ = is_virtual_table(ref_id) && ref_id != index_id;
    ap->est_cost_info_.index_meta_info_.is_index_back_ = index_info_entry->is_index_back();
    ap->est_cost_info_.index_meta_info_.is_unique_index_ = index_info_entry->is_unique_index();
    ap->est_cost_info_.index_meta_info_.is_global_index_ = index_info_entry->is_index_global();
    ap->est_cost_info_.index_meta_info_.is_geo_index_ = index_info_entry->is_index_geo();
    ap->est_cost_info_.index_meta_info_.is_multivalue_index_ = index_info_entry->is_multivalue_index();
    ap->est_cost_info_.index_meta_info_.is_fulltext_index_ = index_info_entry->is_fulltext_index();
    ap->est_cost_info_.index_meta_info_.is_vector_index_ = index_info_entry->is_vector_index();
    ap->est_cost_info_.is_virtual_table_ = is_virtual_table(ref_id);
    ap->est_cost_info_.table_metas_ = &get_plan()->get_basic_table_metas();
    ap->est_cost_info_.sel_ctx_ = &get_plan()->get_selectivity_ctx();
    ap->est_cost_info_.is_unique_ = ap->is_get_ ||
                                     (index_info_entry->is_unique_index()
                                      && range_info.is_index_column_get()
                                      && index_info_entry->is_valid_unique_index());
    ap->est_cost_info_.rescan_server_list_ = &ap->server_list_;
    ap->table_opt_info_ = helper.table_opt_info_;
    ap->is_inner_path_ = helper.is_inner_path_;
    ap->est_cost_info_.is_rescan_ = helper.is_inner_path_ || get_plan()->get_is_rescan_subplan();
    ap->range_prefix_count_ = index_info_entry->get_range_info().get_range_prefix_count();
    ap->interesting_order_info_ = index_info_entry->get_interesting_order_info();
    ap->for_update_ = table_item->for_update_;
    ap->use_skip_scan_ = use_skip_scan;
    ap->index_prefix_ = index_info_entry->get_range_info().get_index_prefix();
    ap->use_column_store_ = use_column_store;
    ap->est_cost_info_.use_column_store_ = use_column_store;
    ap->force_direction_ = index_info_entry->is_force_direction();

    ap->contain_das_op_ = ap->use_das_;
    ap->is_ordered_by_pk_ = (ref_id == index_id) ? true
        : range_info.get_equal_prefix_count() >= range_info.get_index_column_count();
    //set strong_sharding_ ignore dop, update sharding by dop in set_sharding_info_for_base_path()
    ap->strong_sharding_ = index_info_entry->get_sharding_info();
    if (get_plan()->get_stmt()->has_vec_approx() && OB_FAIL(process_vec_index_info(get_plan()->get_stmt(), table_id, ref_id, index_id, helper, *ap, range_info))) {
      LOG_WARN("failed to init vec_index_info", K(ret));
    } else if (OB_FAIL(process_index_for_match_expr(table_id, ref_id, index_id, helper, *ap))) {
      LOG_WARN("failed to process index for match expr", K(ret));
    } else if (OB_FAIL(init_sample_info_for_access_path(ap, table_id, table_item))) {
      LOG_WARN("failed to init sample info", K(ret));
    } else if (OB_FAIL(add_access_filters(ap,
                                          range_info.get_range_columns(),
                                          (nullptr == range_info.get_query_range_provider()) ? nullptr : &range_info.get_query_range_provider()->get_range_exprs(),
                                          (nullptr == range_info.get_query_range_provider()) ? nullptr : &range_info.get_query_range_provider()->get_unprecise_range_exprs(),
                                          helper))) {
      LOG_WARN("failed to add access filters", K(*ap), K(ordering_info.get_index_keys()), K(ret));
    } else if (!helper.is_index_merge_
               && OB_FAIL(get_plan()->get_stmt()->get_column_items(table_id, ap->est_cost_info_.access_column_items_))) {
      LOG_WARN("failed to get column items", K(ret));
    } else if ((!ap->is_global_index_ ||
                !index_info_entry->is_index_back() ||
                get_plan()->get_optimizer_context().is_das_keep_order_enabled())
        // for global index lookup without keep order, the ordering is wrong.
        && OB_FAIL(ObOptimizerUtil::make_sort_keys(ordering_info.get_ordering(),
                                                   ordering_info.get_scan_direction(),
                                                   ap->ordering_))) {
      LOG_WARN("failed to create index keys expression array", K(index_id), K(ret));
    } else if (ordering_info.get_index_keys().count() > 0) {
      ap->pre_range_graph_ = const_cast<ObPreRangeGraph*>(range_info.get_pre_range_graph());
      if (OB_FAIL(ap->index_keys_.assign(ordering_info.get_index_keys()))) {
        LOG_WARN("failed to get index keys", K(ret));
      } else if (OB_FAIL(ap->est_cost_info_.range_columns_.assign(range_info.get_range_columns()))) {
        LOG_WARN("failed to assign range columns", K(ret));
      } else if (OB_FAIL(fill_query_range_info(range_info, ap->est_cost_info_,
                                               OptSkipScanState::SS_DISABLE != use_skip_scan))) {
        LOG_WARN("failed to fill query range info", K(ret));
      } else { /*do nothing*/ }
    } else { /*do nothing*/ }
    ObSEArray<ObRawExpr*, 4> index_merge_access_cols;
    if (OB_SUCC(ret) && helper.is_index_merge_) {
      if (OB_FAIL(get_plan()->get_rowkey_exprs(table_id, ref_id, index_merge_access_cols))) {
        LOG_WARN("failed to get rowkey exprs", K(ret));
      }
      for (int i = 0; OB_SUCC(ret) && i < helper.filters_.count(); ++i) {
        if (OB_FAIL(ObRawExprUtils::extract_column_exprs(helper.filters_.at(i), index_merge_access_cols))) {
          LOG_WARN("failed to extract column exprs", K(ret));
        }
      }
    }
    for (int i = 0; OB_SUCC(ret) && i < ap->est_cost_info_.range_columns_.count(); ++i) {
      ColumnItem &col = ap->est_cost_info_.range_columns_.at(i);
      ObRawExpr* col_expr = NULL;
      if (NULL == (col_expr = get_plan()->get_stmt()->get_column_expr_by_id(col.table_id_, col.column_id_))) {
        // do nothing
      } else if (helper.is_index_merge_ && !ObOptimizerUtil::find_item(index_merge_access_cols, col_expr)) {
        // do nothing
      } else if (OB_FAIL(ap->est_cost_info_.index_access_column_items_.push_back(col))) {
        LOG_WARN("failed to push back column item", K(ret));
      }
    }
    if (OB_SUCC(ret) && helper.is_index_merge_) {
      if (OB_FAIL(ap->est_cost_info_.access_column_items_.assign(ap->est_cost_info_.index_access_column_items_))) {
        LOG_WARN("failed to assign access column item", K(ret));
      }
    }

    if (OB_SUCC(ret)) {
      if (OB_FAIL(fill_filters(ap->filter_,
                               ap->get_query_range_provider(),
                               ap->est_cost_info_,
                               ap->domain_idx_info_,
                               is_nl_with_extended_range,
                               ObSqlSchemaGuard::is_link_table(get_plan()->get_stmt(), table_id),
                               OptSkipScanState::SS_DISABLE != use_skip_scan,
                               ap->index_prefix_))) {
        LOG_WARN("failed to fill filters for cost table info", K(ret));
      } else if (OB_FAIL(init_filter_selectivity(ap->est_cost_info_))) {
        LOG_WARN("failed to calc filter sel", K(ret));
      } else if (!helper.is_inner_path_ &&
                OB_FAIL(increase_diverse_path_count(ap))) {
        LOG_WARN("failed to increase diverse path count", K(ret));
      } else if (OB_FAIL(ap->subquery_exprs_.assign(helper.subquery_exprs_))) {
        LOG_WARN("failed to assign exprs", K(ret));
      } else if (OB_FAIL(ap->equal_param_constraints_.assign(helper.equal_param_constraints_))) {
        LOG_WARN("failed to assign equal param constraints", K(ret));
      } else if (OB_FAIL(ap->const_param_constraints_.assign(helper.const_param_constraints_))) {
        LOG_WARN("failed to assign equal param constraints", K(ret));
      } else if (OB_FAIL(ap->expr_constraints_.assign(range_info.get_expr_constraints()))) {
        LOG_WARN("failed to assign expr constraints", K(ret));
      } else if (OB_FAIL(append(ap->expr_constraints_, helper.expr_constraints_))) {
        LOG_WARN("append expr constraints failed", K(ret));
      } else if (OB_FAIL(init_column_store_est_info(table_id, ref_id, ap->est_cost_info_))) {
        LOG_WARN("failed to init column store est cost info", K(ret));
      } else if (OB_FAIL(ap->compute_access_path_batch_rescan())) {
        LOG_WARN("failed to compute access path batch rescan", K(ret));
      } else {
        access_path = ap;
      }
    }
    LOG_TRACE("OPT:succeed to create one access path",
                K(table_id), K(ref_id), K(index_id), K(helper.is_inner_path_));
  }
  return ret;
}

int ObJoinOrder::init_sample_info_for_access_path(AccessPath *ap,
                                                  const uint64_t table_id,
                                                  const TableItem *table_item)
{
  int ret = OB_SUCCESS;
  if (OB_ISNULL(get_plan()) || OB_ISNULL(get_plan()->get_stmt())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpect null stmt", K(ret));
  } else if (!get_plan()->get_stmt()->is_select_stmt()) {
    // do nothing
    // sample scan doesn't support DML other than SELECT.
  } else {
    const ObSelectStmt *stmt = static_cast<const ObSelectStmt *>(get_plan()->get_stmt());
    const SampleInfo *sample_info = table_item->sample_info_;

    if (sample_info != NULL) {
      ap->sample_info_ = *sample_info;
      ap->sample_info_.table_id_ = ap->get_index_table_id();
    } else if (get_plan()->get_optimizer_context().is_online_ddl() &&
               get_plan()->get_optimizer_context().get_root_stmt()->is_insert_stmt() &&
               !get_plan()->get_optimizer_context().is_heap_table_ddl()) {
      ap->sample_info_.method_ = SampleInfo::SampleMethod::DDL_BLOCK_SAMPLE;
      ap->sample_info_.scope_ = SampleInfo::SAMPLE_ALL_DATA;
      ap->sample_info_.percent_ = (double)get_plan()->get_optimizer_context().get_px_object_sample_rate() / 1000;
      ap->sample_info_.table_id_ = ap->get_index_table_id();
    }
    ap->est_cost_info_.sample_info_ = ap->sample_info_;
  }
  return ret;
}

int ObJoinOrder::init_filter_selectivity(ObCostTableScanInfo &est_cost_info)
{
  int ret = OB_SUCCESS;
  ObLogPlan *plan = get_plan();
  if (OB_ISNULL(plan)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpect null plan", K(ret));
  } else if (OB_FAIL(ObOptSelectivity::calculate_selectivity(plan->get_basic_table_metas(),
                                                             plan->get_selectivity_ctx(),
                                                             est_cost_info.prefix_filters_,
                                                             est_cost_info.prefix_filter_sel_,
                                                             plan->get_predicate_selectivities()))) {
    LOG_WARN("failed to calculate selectivity", K(ret));
  } else if (OB_FAIL(ObOptSelectivity::calculate_selectivity(plan->get_basic_table_metas(),
                                                             plan->get_selectivity_ctx(),
                                                             est_cost_info.pushdown_prefix_filters_,
                                                             est_cost_info.pushdown_prefix_filter_sel_,
                                                             plan->get_predicate_selectivities()))) {
    LOG_WARN("failed to calculate selectivity", K(ret));
  } else if (OB_FAIL(ObOptSelectivity::calculate_selectivity(plan->get_basic_table_metas(),
                                                             plan->get_selectivity_ctx(),
                                                             est_cost_info.ss_postfix_range_filters_,
                                                             est_cost_info.ss_postfix_range_filters_sel_,
                                                             plan->get_predicate_selectivities()))) {
    LOG_WARN("failed to calculate selectivity", K(ret));
  } else if (OB_FAIL(ObOptSelectivity::calculate_selectivity(plan->get_basic_table_metas(),
                                                             plan->get_selectivity_ctx(),
                                                             est_cost_info.postfix_filters_,
                                                             est_cost_info.postfix_filter_sel_,
                                                             plan->get_predicate_selectivities()))) {
    LOG_WARN("failed to calculate selectivity", K(ret));
  } else if (OB_FAIL(ObOptSelectivity::calculate_selectivity(plan->get_basic_table_metas(),
                                                             plan->get_selectivity_ctx(),
                                                             est_cost_info.table_filters_,
                                                             est_cost_info.table_filter_sel_,
                                                             plan->get_predicate_selectivities()))) {
    LOG_WARN("failed to calculate selectivity", K(ret));
  }
  return ret;
}

int ObJoinOrder::init_column_store_est_info(const uint64_t table_id,
                                            const uint64_t ref_id,
                                            ObCostTableScanInfo &est_cost_info)
{
  int ret = OB_SUCCESS;
  bool index_back_will_use_row_store = false;
  bool index_back_will_use_column_store = false;
  if (OB_ISNULL(get_plan())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpect null plan", K(ret));
  } else if (OB_FAIL(get_plan()->will_use_column_store(OB_INVALID_ID,
                                                       ref_id,
                                                       ref_id,
                                                       index_back_will_use_column_store,
                                                       index_back_will_use_row_store))) {
    LOG_WARN("failed to check will use column store", K(ret));
  } else if (est_cost_info.use_column_store_ || !index_back_will_use_row_store) {
    FilterCompare filter_compare(get_plan()->get_predicate_selectivities());
    lib::ob_sort(est_cost_info.table_filters_.begin(), est_cost_info.table_filters_.end(), filter_compare);
    ObSqlBitSet<> used_column_ids;
    est_cost_info.index_back_with_column_store_ = !index_back_will_use_row_store;
    const OptTableMetas& table_opt_meta = get_plan()->get_basic_table_metas();
    ObIArray<ObCostColumnGroupInfo> &index_scan_column_group_infos = est_cost_info.index_scan_column_group_infos_;
    ObIArray<ObCostColumnGroupInfo> &index_back_column_group_infos = est_cost_info.index_meta_info_.is_index_back_ ?
                                                          est_cost_info.index_back_column_group_infos_
                                                          : est_cost_info.index_scan_column_group_infos_;
    //add column group with prefix filters
    if (OB_FAIL(init_column_store_est_info_with_filter(table_id,
                                                        est_cost_info,
                                                        table_opt_meta,
                                                        est_cost_info.prefix_filters_,
                                                        index_scan_column_group_infos,
                                                        used_column_ids,
                                                        filter_compare,
                                                        false))) {
      LOG_WARN("failed to init column store est info with filter", K(ret));
    }
    else if (OB_FAIL(init_column_store_est_info_with_filter(table_id,
                                                            est_cost_info,
                                                            table_opt_meta,
                                                            est_cost_info.pushdown_prefix_filters_,
                                                            index_scan_column_group_infos,
                                                            used_column_ids,
                                                            filter_compare,
                                                            false))) {
      LOG_WARN("failed to init column store est info with filter", K(ret));
    }
    //add column group with postfix filters
    else if (OB_FAIL(init_column_store_est_info_with_filter(table_id,
                                                            est_cost_info,
                                                            table_opt_meta,
                                                            est_cost_info.postfix_filters_,
                                                            index_scan_column_group_infos,
                                                            used_column_ids,
                                                            filter_compare,
                                                            true))) {
      LOG_WARN("failed to init column store est info with filter", K(ret));
    }
    //add column group with index back filters
    else if (OB_FAIL(init_column_store_est_info_with_filter(table_id,
                                                            est_cost_info,
                                                            table_opt_meta,
                                                            est_cost_info.table_filters_,
                                                            index_back_column_group_infos,
                                                            used_column_ids,
                                                            filter_compare,
                                                            true))) {
      LOG_WARN("failed to init column store est info with filter", K(ret));
    }
    //add other column group
    else if (OB_FAIL(init_column_store_est_info_with_other_column(table_id,
                                                                  est_cost_info,
                                                                  table_opt_meta,
                                                                  used_column_ids))) {
      LOG_WARN("failed to init column store est info with other column", K(ret));
    } else if (index_scan_column_group_infos.empty()) {
      //add dummy column group cost info for nil access exprs
      ObCostColumnGroupInfo cg_info;
      if (OB_FAIL(index_scan_column_group_infos.push_back(cg_info))) {
        LOG_WARN("failed to push back column group info", K(ret));
      }
    }
  }
  return ret;
}

int ObJoinOrder::init_column_store_est_info_with_filter(const uint64_t table_id,
                                                        ObCostTableScanInfo &est_cost_info,
                                                        const OptTableMetas& table_opt_meta,
                                                        ObIArray<ObRawExpr*> &filters,
                                                        ObIArray<ObCostColumnGroupInfo> &column_group_infos,
                                                        ObSqlBitSet<> &used_column_ids,
                                                        FilterCompare &filter_compare,
                                                        const bool use_filter_sel)
{
  int ret = OB_SUCCESS;
  ObSEArray<ObRawExpr*, 4> filter_columns;
  if (OB_ISNULL(get_plan())
      || OB_ISNULL(get_plan()->get_stmt())
      || OB_ISNULL(OPT_CTX.get_query_ctx())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("NULL pointer error", K(get_plan()), K(OPT_CTX.get_query_ctx()), K(ret));
  }
  for (int i = 0; OB_SUCC(ret) && i < filters.count(); ++i) {
    ObRawExpr *filter = filters.at(i);
    filter_columns.reuse();
    if (OB_FAIL(ObRawExprUtils::extract_column_exprs(filter,
                                                    filter_columns))) {
      LOG_WARN("failed to extract column exprs", K(ret));
    }
    //init column group info
    for (int j = 0; OB_SUCC(ret) && j < filter_columns.count(); ++j) {
      ObRawExpr *expr = filter_columns.at(j);
      if (OB_ISNULL(expr)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpect null expr", K(ret));
      } else if (expr->is_column_ref_expr()) {
        ObColumnRefRawExpr* col_expr = static_cast<ObColumnRefRawExpr*>(expr);
        ObDMLStmt *stmt = const_cast<ObDMLStmt*>(get_plan()->get_stmt());
        ColumnItem *col_item = stmt->get_column_item(table_id, col_expr->get_column_id());
        const OptColumnMeta* col_opt_meta = table_opt_meta.get_column_meta_by_table_id(
                                    table_id,
                                    col_expr->get_column_id());
        if (used_column_ids.has_member(col_expr->get_column_id())) {
          //do nothing
        } else if (OB_FAIL(used_column_ids.add_member(col_expr->get_column_id()))) {
          LOG_WARN("failed to add memeber", K(ret));
        } else if (OB_ISNULL(col_opt_meta) || OB_ISNULL(col_item)) {
          ret = OB_ERR_UNEXPECTED;
          LOG_WARN("unexpect null column meta", K(ret));
        } else {
          ObCostColumnGroupInfo cg_info;
          cg_info.micro_block_count_ = col_opt_meta->get_cg_micro_blk_cnt();
          cg_info.column_id_ = col_expr->get_column_id();
          cg_info.skip_rate_ = col_opt_meta->get_cg_skip_rate();
          cg_info.skip_filter_sel_ = 1 - ((1-filter_compare.get_selectivity(filter)) * cg_info.skip_rate_);
          if (OB_FAIL(cg_info.access_column_items_.push_back(*col_item))) {
            LOG_WARN("failed to push back filter", K(ret));
          } else if (OB_FAIL(column_group_infos.push_back(cg_info))) {
            LOG_WARN("failed to push back column group info", K(ret));
          }
          LOG_TRACE("COST:init_column_store_est_info_with_other_column",K(cg_info.skip_rate_),K(col_opt_meta->get_cg_skip_rate()), K(filter_compare.get_selectivity(filter)));
        }

      }
    }
    //distribute filter
    int max_pos = -1;
    for (int j = 0; OB_SUCC(ret) && j < filter_columns.count(); ++j) {
      ObRawExpr *expr = filter_columns.at(j);
      if (OB_ISNULL(expr)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpect null expr", K(ret));
      } else if (expr->is_column_ref_expr()) {
        ObColumnRefRawExpr* col_expr = static_cast<ObColumnRefRawExpr*>(expr);
        int find_pos = -1;
        for (int k = 0; OB_SUCC(ret) && find_pos < 0 && k < column_group_infos.count(); ++k) {
          ObCostColumnGroupInfo &cg_info = column_group_infos.at(k);
          if (cg_info.column_id_ == col_expr->get_column_id()) {
            find_pos = k;
          }
        }
        if (OB_FAIL(ret)) {
        } else if (find_pos < 0) {
          //ignore index column group
        } else if (find_pos > max_pos) {
          max_pos = find_pos;
        }
      }
    }
    if (OB_FAIL(ret) || filter_columns.empty()) {
    } else if (max_pos < 0 || max_pos >= column_group_infos.count()) {
      //table filter with index column group
    } else if (OB_FAIL(column_group_infos.at(max_pos).filters_.push_back(filter))) {
      LOG_WARN("failed to push back filter", K(ret));
    } else if (use_filter_sel) {
      column_group_infos.at(max_pos).filter_sel_ *= filter_compare.get_selectivity(filter);
    }
  }
  return ret;
}

int ObJoinOrder::init_column_store_est_info_with_other_column(const uint64_t table_id,
                                                              ObCostTableScanInfo &est_cost_info,
                                                              const OptTableMetas& table_opt_meta,
                                                              ObSqlBitSet<> &used_column_ids)
{
  int ret = OB_SUCCESS;
  ObIArray<ObCostColumnGroupInfo> &column_group_infos = est_cost_info.index_meta_info_.is_index_back_ ?
                                          est_cost_info.index_back_column_group_infos_
                                          : est_cost_info.index_scan_column_group_infos_;
  if (OB_ISNULL(OPT_CTX.get_query_ctx())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("NULL pointer error", K(OPT_CTX.get_query_ctx()), K(ret));
  }
  for (int i = 0; OB_SUCC(ret) && i < est_cost_info.access_column_items_.count(); ++i) {
    uint64_t column_id = est_cost_info.access_column_items_.at(i).column_id_;
    const OptColumnMeta* col_opt_meta = table_opt_meta.get_column_meta_by_table_id(
                                    table_id,
                                    column_id);
    ObRawExpr *expr = est_cost_info.access_column_items_.at(i).expr_;
    if (OB_ISNULL(expr)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpect null expr", K(ret));
    } else if (used_column_ids.has_member(column_id)) {
      //do nothing
    } else if (!expr->is_explicited_reference()) {
      //do nothing
    } else if (OB_ISNULL(col_opt_meta)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpect null column meta", K(ret));
    } else {
      ObCostColumnGroupInfo cg_info;
      cg_info.micro_block_count_ = col_opt_meta->get_cg_micro_blk_cnt();
      cg_info.skip_rate_ = col_opt_meta->get_cg_skip_rate();
      cg_info.skip_filter_sel_ =  cg_info.skip_rate_;
      cg_info.skip_filter_sel_ = 1;
      cg_info.column_id_ = column_id;
      if (OB_FAIL(cg_info.access_column_items_.push_back(est_cost_info.access_column_items_.at(i)))) {
        LOG_WARN("failed to push back filter", K(ret));
      } else if (OB_FAIL(column_group_infos.push_back(cg_info))) {
        LOG_WARN("failed to push back column group info", K(ret));
      }
    }
  }
  return ret;
}

int ObJoinOrder::get_access_path_ordering(const uint64_t table_id,
                                          const uint64_t ref_table_id,
                                          const uint64_t index_id,
                                          common::ObIArray<ObRawExpr*> &index_keys,
                                          common::ObIArray<ObRawExpr*> &ordering,
                                          ObOrderDirection &direction,
                                          bool &force_direction,
                                          const bool is_index_back)
{
  int ret = OB_SUCCESS;
  direction = default_asc_direction();
  const ObDMLStmt *stmt = NULL;
  ObOptimizerContext *opt_ctx = NULL;
  ObSqlSchemaGuard *schema_guard = NULL;
  const ObTableSchema *index_schema = NULL;
  ObOrderDirection hint_direction = UNORDERED;
  force_direction = false;
  if (OB_ISNULL(get_plan())
      || OB_ISNULL(stmt = get_plan()->get_stmt())
      || OB_ISNULL(opt_ctx = &get_plan()->get_optimizer_context())
      || OB_ISNULL(schema_guard = opt_ctx->get_sql_schema_guard())
      || OB_ISNULL(opt_ctx->get_query_ctx())) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("NULL pointer error",
             K(get_plan()), K(stmt), K(opt_ctx),  K(schema_guard), K(ret));
  } else if (OB_FAIL(schema_guard->get_table_schema(index_id, index_schema, ObSqlSchemaGuard::is_link_table(stmt, table_id)))) {
    LOG_WARN("fail to get table schema", K(ref_table_id), K(index_schema), K(ret));
  } else if (OB_ISNULL(index_schema)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("index schema should not be null", K(index_schema), K(ret));
  } else if (OB_FAIL(get_plan()->get_rowkey_exprs(table_id, *index_schema, index_keys))) {
    LOG_WARN("NULL pointer error", K(ret), K(table_id), K(ref_table_id), K(index_id));
  } else if (!index_schema->is_ordered()) {
    // for virtual table, we have HASH index which offers no ordering on index keys
  } else if (index_schema->is_global_index_table() && is_index_back && !opt_ctx->is_das_keep_order_enabled()) {
    // for global index lookup without keep order, the ordering is wrong.
  } else if (OB_FAIL(append(ordering, index_keys))) {
    LOG_WARN("failed to append index ordering expr", K(ret));
  } else if (OB_FAIL(get_plan()->get_log_plan_hint().check_scan_direction(*opt_ctx->get_query_ctx(),
                                                                          table_id,
                                                                          index_id,
                                                                          hint_direction))) {
    LOG_WARN("failed to check scan direction", K(ret), K(table_id));
  } else if (UNORDERED != hint_direction) {
    direction = hint_direction;
    force_direction = true;
  } else if (OB_FAIL(get_index_scan_direction(ordering, stmt,
                                              get_plan()->get_equal_sets(), direction))) {
    LOG_WARN("failed to get index scan direction", K(ret));
  }
  return ret;
}

/**
 * Get the order for index scan
 * First check the order by in window function, then check the order by in stmt
 */
int ObJoinOrder::get_index_scan_direction(const ObIArray<ObRawExpr *> &keys,
                                          const ObDMLStmt *stmt,
                                          const EqualSets &equal_sets,
                                          ObOrderDirection &index_direction)
{
  int ret = OB_SUCCESS;
  index_direction = default_asc_direction();
  bool check_order_by = true;
  int64_t order_match_count = 0;
  if (OB_ISNULL(stmt) || OB_ISNULL(get_plan())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("stmt is null", K(ret), K(stmt), K(get_plan()));
  } else if (stmt->is_select_stmt()) {
    const ObSelectStmt *sel_stmt = static_cast<const ObSelectStmt *>(stmt);
    int64_t max_order_match_count = 0;
    const ObWinFunRawExpr *win_expr = NULL;
    for (int64_t i = 0; OB_SUCC(ret) && i < sel_stmt->get_window_func_count(); ++i) {
      bool full_covered = false;
      int64_t partition_match_count = 0;
      ObOrderDirection tmp_direction = default_asc_direction();
      if (OB_ISNULL(win_expr = sel_stmt->get_window_func_expr(i))) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("get null win expr", K(ret));
      } else if (OB_FAIL(ObOptimizerUtil::prefix_subset_exprs(win_expr->get_partition_exprs(),
                                                              keys,
                                                              equal_sets,
                                                              get_plan()->get_const_exprs(),
                                                              full_covered,
                                                              &partition_match_count))) {
        LOG_WARN("check is covered by ordering failed", K(ret));
      } else if (!full_covered) {
        check_order_by = false;
      } else if (win_expr->get_order_items().count() > 0) {
        check_order_by = false;
        if (OB_FAIL(get_direction_in_order_by(win_expr->get_order_items(),
                                              keys,
                                              partition_match_count,
                                              equal_sets,
                                              get_plan()->get_const_exprs(),
                                              tmp_direction,
                                              order_match_count))) {
          LOG_WARN("failed to match order-by against index", K(ret));
        } else if (order_match_count > max_order_match_count) {
          max_order_match_count = order_match_count;
          index_direction = tmp_direction;
        }
      }
    }
  }
  if (OB_SUCC(ret) && check_order_by) {
    ObOrderDirection tmp_direction = default_asc_direction();
    if (OB_FAIL(get_direction_in_order_by(stmt->get_order_items(),
                                          keys,
                                          0, // index start offset
                                          equal_sets,
                                          get_plan()->get_const_exprs(),
                                          tmp_direction,
                                          order_match_count))) {
      LOG_WARN("failed to match order-by against index", K(ret));
    } else if (order_match_count > 0) {
      index_direction = tmp_direction;
    }
  }
  return ret;
}

int ObJoinOrder::get_direction_in_order_by(const ObIArray<OrderItem> &order_by,
                                           const ObIArray<ObRawExpr *> &index_keys,
                                           const int64_t index_start_offset,
                                           const EqualSets &equal_sets,
                                           const ObIArray<ObRawExpr *> &const_exprs,
                                           ObOrderDirection &direction,
                                           int64_t &order_match_count)
{
  int ret = OB_SUCCESS;
  order_match_count = 0;
  int64_t order_offset = order_by.count();
  int64_t index_offset = index_start_offset;
  bool is_const = true;
  // Find the first non-const order item
  for (int64_t i = 0; OB_SUCC(ret) && is_const && i < order_by.count(); ++i) {
    if (OB_FAIL(ObOptimizerUtil::is_const_expr(order_by.at(i).expr_, equal_sets,
                                               const_exprs, is_const))) {
      LOG_WARN("failed to check is_const_expr", K(ret));
    } else if (!is_const) {
      // mysql only has nulls_first_asc and nulls_last_desc, both can use the index order
      if (is_ascending_direction(order_by.at(i).order_type_)) {
        direction = default_asc_direction();
      } else {
        direction = default_desc_direction();
      }
      order_offset = i;
    }
  }
  // Calculate the maximum match for index and order items, skip const expressions
  ObRawExpr *index_expr = NULL;
  ObRawExpr *order_expr = NULL;
  int64_t order_start = order_offset;
  while (OB_SUCC(ret) && index_offset < index_keys.count() && order_offset < order_by.count()) {
    if (OB_ISNULL(index_expr = index_keys.at(index_offset)) ||
        OB_ISNULL(order_expr = order_by.at(order_offset).expr_)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("exprs have null", K(ret), K(index_expr), K(order_expr));
    } else if (ObOptimizerUtil::is_expr_equivalent(index_expr, order_expr, equal_sets) &&
               (is_ascending_direction(order_by.at(order_offset).order_type_)
                == is_ascending_direction(direction))) {
      ++index_offset;
      ++order_offset;
    } else if (OB_FAIL(ObOptimizerUtil::is_const_expr(order_expr, equal_sets,
                                                      const_exprs, is_const))) {
      LOG_WARN("failed to check order_expr is const expr", K(ret));
    } else if (is_const) {
      ++order_offset;
    } else if (OB_FAIL(ObOptimizerUtil::is_const_expr(index_expr, equal_sets,
                                                      const_exprs, is_const))) {
      LOG_WARN("failed to check index_expr is const expr", K(ret));
    } else if (is_const) {
      ++index_offset;
    } else {
      break;
    }
  }
  if (OB_SUCC(ret)) {
    order_match_count = order_offset - order_start;
  }
  return ret;
}

int ObJoinOrder::add_access_filters(AccessPath *path,
                                    const ObIArray<ColumnItem> &range_cols,
                                    const ObIArray<ObRawExpr *> *range_exprs,
                                    const common::ObIArray<ObRawExpr*> *unprecise_range_exprs,
                                    PathHelper &helper)
{
  int ret = OB_SUCCESS;
  const ObDMLStmt *stmt = NULL;
  const ObIArray<ObRawExpr *> &restrict_infos = helper.filters_;
  ObSEArray<uint64_t, 4> range_col_ids;
  ObSEArray<ObRawExpr *, 4> dummy;
  ObSEArray<ObRawExpr *, 4> remove_dup;
  if (OB_ISNULL(get_plan()) || OB_ISNULL(stmt = get_plan()->get_stmt()) || OB_ISNULL(path)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("NULL pointer error", K(get_plan()), K(stmt), K(path), K(ret));
  } else {
    for (int i = 0; OB_SUCC(ret) && i < range_cols.count(); i++) {
      if (OB_FAIL(range_col_ids.push_back(range_cols.at(i).column_id_))) {
        LOG_WARN("failed to push back column_id", K(ret), K(i));
      }
    }
    for (int64_t i = 0; OB_SUCC(ret) && i < restrict_infos.count(); i++) {
      bool found = false;
      for (int64_t j = 0; OB_SUCC(ret) && !found && j < deduced_exprs_info_.count(); j++) {
        const DeducedExprInfo &deduced_expr_info = deduced_exprs_info_.at(j);
        if (ObOptimizerUtil::is_expr_equivalent(deduced_expr_info.deduced_expr_,
                                                restrict_infos.at(i),
                                                get_plan()->get_equal_sets())) {
          found = true;
          bool extract_unprecise_range = false;
          // old query range unprecise check
          if (OB_FAIL(can_extract_unprecise_range(path->table_id_,
                                                  deduced_expr_info.deduced_expr_,
                                                  range_col_ids,
                                                  dummy,
                                                  extract_unprecise_range))) {
            LOG_WARN("failed to check can extract unprecise range", K(ret));
          } else if (extract_unprecise_range ||
                     (OB_NOT_NULL(range_exprs) &&
                     ObOptimizerUtil::find_equal_expr(*range_exprs, restrict_infos.at(i))) ||
                     (OB_NOT_NULL(unprecise_range_exprs) &&
                     ObOptimizerUtil::find_equal_expr(*unprecise_range_exprs, restrict_infos.at(i)))) {
            if (path->est_cost_info_.index_meta_info_.is_multivalue_index_ && !deduced_expr_info.is_precise_) {
              // deduced new pred for multivalue needn't add into new filter, just remain orginal filter is ok
            } else if (OB_FAIL(path->filter_.push_back(restrict_infos.at(i)))) {
              LOG_WARN("push back error", K(ret));
            } else if (OB_FAIL(append(helper.const_param_constraints_, deduced_expr_info.const_param_constraints_))) {
              LOG_WARN("append failed", K(ret));
            } else if (deduced_expr_info.is_precise_) {
              ObRawExpr *raw_expr = deduced_expr_info.deduced_from_expr_;
              if (OB_ISNULL(raw_expr)) {
                ret = OB_ERR_UNEXPECTED;
                LOG_WARN("raw expr is null", K(ret));
              } else if (OB_FAIL(remove_dup.push_back(raw_expr))) {
                LOG_WARN("push back error", K(ret));
              }
            } else {
              //do nothing
            }
          }
        }
      }
      if (OB_SUCC(ret) && !found) {
        if (OB_FAIL(path->filter_.push_back(restrict_infos.at(i)))) {
          LOG_WARN("push back error", K(ret));
        }
      }
    }
    if (FAILEDx(ObOptimizerUtil::remove_item(path->filter_, remove_dup))) {
      LOG_WARN("remove dup failed", K(ret));
    }
  }
  return ret;
}

int ObJoinOrder::check_and_extract_query_range(const uint64_t table_id,
                                               const uint64_t index_table_id,
                                               const IndexInfoEntry &index_info_entry,
                                               const ObIArray<ObRawExpr*> &index_keys,
                                               const ObIndexInfoCache &index_info_cache,
                                               bool &contain_always_false,
                                               ObIArray<uint64_t> &prefix_range_ids,
                                               ObIArray<ObRawExpr *> &restrict_infos)
{
  int ret = OB_SUCCESS;
  //do some quick check
  bool expr_match = false; //some condition on index
  contain_always_false = false;
  bool is_special_index = index_info_entry.is_index_geo() ||
                          index_info_entry.is_multivalue_index() ||
                          index_info_entry.is_fulltext_index();
  if (index_info_entry.is_multivalue_index() &&
      OB_FAIL(check_exprs_overlap_multivalue_index(table_id, index_table_id, restrict_infos, index_keys, expr_match))) {
    LOG_WARN("get_range_columns failed", K(ret));
  } else if (index_info_entry.is_index_geo() &&
             OB_FAIL(check_exprs_overlap_gis_index(restrict_infos, index_keys, expr_match))) {
    LOG_WARN("check quals match gis index error", K(restrict_infos), K(index_keys));
  } else if (index_info_entry.is_fulltext_index() &&
             OB_FALSE_IT(expr_match = index_info_entry.get_range_info().is_valid())) {
  } else if (!is_special_index && OB_FAIL(check_exprs_overlap_index(restrict_infos, index_keys, expr_match))) {
    LOG_WARN("check quals match index error", K(restrict_infos), K(index_keys));
  } else if (expr_match) {
    prefix_range_ids.reset();
    const QueryRangeInfo *query_range_info = NULL;
    if (OB_FAIL(index_info_cache.get_query_range(table_id, index_table_id,
                                                 query_range_info))) {
      LOG_WARN("get_range_columns failed", K(ret));
    } else if (OB_ISNULL(query_range_info)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("query_range_info should not be null", K(ret));
    } else {
      contain_always_false = query_range_info->get_contain_always_false();
      uint64_t range_prefix_count = query_range_info->get_range_prefix_count();
      const ObIArray<ColumnItem> &range_columns = query_range_info->get_range_columns();
      if (index_info_entry.is_index_geo()) {
        range_prefix_count = 1;
      }
      ObSEArray<uint64_t, 4> range_col_ids;
      if (OB_UNLIKELY(range_prefix_count > range_columns.count())) {
        ret = OB_INVALID_ARGUMENT;
        LOG_WARN("range prefix count is invalid", K(range_prefix_count), K(ret));
      } else {
        for (int i = 0; OB_SUCC(ret) && i < range_prefix_count; ++i) {
          if (OB_FAIL(prefix_range_ids.push_back(range_columns.at(i).column_id_))) {
            LOG_WARN("failed to push back column_id", K(ret), K(i));
          }
        }
        // deal with unprecise dynamic range generated on execution stage
        for (int i = 0; OB_SUCC(ret) && i < range_columns.count(); i++) {
          if (OB_FAIL(range_col_ids.push_back(range_columns.at(i).column_id_))) {
            LOG_WARN("failed to push back column_id", K(ret), K(i));
          }
        }
        for (int i = 0; OB_SUCC(ret) && i < restrict_infos.count(); i++) {
          bool can_extract = false;
          ObSEArray<ObRawExpr *, 4> unprecise_exprs;
          ObSEArray<uint64_t, 4> expr_col_ids;
          if (OB_ISNULL(restrict_infos.at(i))) {
            ret = OB_ERR_UNEXPECTED;
            LOG_WARN("restrict_infos.at(i) is null", K(ret), K(i));
          } else if (OB_NOT_NULL(query_range_info->get_pre_range_graph()) &&
                     OB_FAIL(unprecise_exprs.assign(query_range_info->get_pre_range_graph()->get_unprecise_range_exprs()))) {
            LOG_WARN("failed to assign unprecise range exprs", K(ret));
          } else if (OB_FAIL(can_extract_unprecise_range(table_id,
                                                        restrict_infos.at(i),
                                                        range_col_ids,
                                                        unprecise_exprs,
                                                        can_extract))) {
            LOG_WARN("failed to check can extract unprecise range", K(ret));
          } else if (!can_extract) {
            // do nothing
          } else if (OB_FAIL(ObOptimizerUtil::extract_column_ids(restrict_infos.at(i), table_id, expr_col_ids))) {
            LOG_WARN("failed to extract column ids", K(ret));
          } else if (!ObOptimizerUtil::is_subset(expr_col_ids, range_col_ids)) {
            // do nothing
          } else if (OB_FAIL(append_array_no_dup(prefix_range_ids, expr_col_ids))) {
            LOG_WARN("failed to append array no dup", K(ret));
          }
        }
      }
    }
  } //not match, do nothing
  LOG_TRACE("extract prefix range ids", K(ret),
            K(contain_always_false), K(expr_match),
            K(prefix_range_ids));
  return ret;
}

/*
 * Calculate dimension information
 * @table_id table_id obtained from table_item
 * @data_table_id actual table_id
 * @index_table_id table_id of the index
 * @stmt
 * @index_dim records information of three dimensions
 * */
int ObJoinOrder::cal_dimension_info(const uint64_t table_id, //alias table id
                                    const uint64_t data_table_id, //real table id
                                    const uint64_t index_table_id,
                                    const ObDMLStmt *stmt,
                                    ObIndexSkylineDim &index_dim,
                                    const ObIndexInfoCache &index_info_cache,
                                    ObIArray<ObRawExpr *> &restrict_infos,
                                    bool use_unique_index,
                                    bool ignore_order_dim,
                                    bool ignore_index_back_dim)
{
  int ret = OB_SUCCESS;
  ObSqlSchemaGuard *guard = NULL;
  IndexInfoEntry *index_info_entry = NULL;
  if (OB_ISNULL(get_plan()) || OB_ISNULL(guard = OPT_CTX.get_sql_schema_guard()) || OB_ISNULL(stmt) || OB_ISNULL(OPT_CTX.get_query_ctx())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret), K(get_plan()), K(guard), K(stmt));
  } else if (OB_FAIL(index_info_cache.get_index_info_entry(table_id,
                                                           index_table_id,
                                                           index_info_entry))) {
    LOG_WARN("failed to get index info entry", K(ret), K(data_table_id), K(index_table_id));
  } else if (OB_ISNULL(index_info_entry)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("index info entry should not be null", K(ret));
  } else {
    ObSEArray<uint64_t, 8> filter_column_ids;
    bool is_index_back = ignore_index_back_dim ? false : index_info_entry->is_index_back();
    const OrderingInfo *ordering_info = &index_info_entry->get_ordering_info();
    ObSEArray<uint64_t, 8> interest_column_ids;
    ObSEArray<bool, 8> const_column_info;
    ObSEArray<uint64_t, 8> prefix_range_ids;  //for query range compare
    bool contain_always_false = false;
    int64_t range_cnt = index_info_entry->get_range_info().get_range_count();
    if (OB_FAIL(extract_interesting_column_ids(ordering_info->get_index_keys(),
                                               index_info_entry->get_interesting_order_prefix_count(),
                                               interest_column_ids,
                                               const_column_info))) {
      LOG_WARN("failed to extract interest column ids", K(ret));
    } else if (OB_FAIL(check_and_extract_query_range(table_id,
                                                     index_table_id,
                                                     *index_info_entry,
                                                     ordering_info->get_index_keys(),
                                                     index_info_cache,
                                                     contain_always_false,
                                                     prefix_range_ids,
                                                     restrict_infos))) {
      LOG_WARN("check_and_extract query range failed", K(ret));
    } else {
      const ObTableSchema *index_schema = NULL;
      if (OB_FAIL(guard->get_table_schema(index_table_id, index_schema))) {
        LOG_WARN("failed to get table schema", K(ret));
      } else if (OB_ISNULL(index_schema)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("index schema should not be null", K(ret));
      } else if (OB_FAIL(extract_filter_column_ids(restrict_infos,
                                                   data_table_id == index_table_id,
                                                   *index_schema,
                                                   filter_column_ids))) {
        LOG_WARN("extract filter column ids failed", K(ret));
      }
      if (OB_SUCC(ret)) {
        /*
         * Add four dimensions of information
         * (1) Whether to go back to the table
         * (2) interesting_order
         * (3) query range
         *     a. for unique index, we compare the range rows
         *     b. for non unique index, we compare the range prefix column
         * (4) sharding
         * */
        bool can_extract_range = prefix_range_ids.count() > 0 || contain_always_false;
        bool is_get = index_info_entry->get_range_info().is_get();
        if (!ignore_index_back_dim &&
            OB_FAIL(index_dim.add_index_back_dim(is_index_back,
                                                 *allocator_))) {
          LOG_WARN("add index back dim failed", K(is_index_back), K(ret));
        } else if (!ignore_order_dim &&
                   OB_FAIL(index_dim.add_interesting_order_dim(is_index_back,
                                                               can_extract_range,
                                                               filter_column_ids,
                                                               interest_column_ids,
                                                               const_column_info,
                                                               *allocator_))) {
          LOG_WARN("add interesting order dim failed", K(interest_column_ids), K(ret));
        } else if (!use_unique_index &&
                   OB_FAIL(index_dim.add_query_range_dim(prefix_range_ids,
                                                         *allocator_,
                                                         contain_always_false))) {
          LOG_WARN("add query range dimension failed", K(ret));
        } else if (use_unique_index &&
                   OB_FAIL(index_dim.add_unique_range_dim(range_cnt,
                                                          *allocator_))) {
          LOG_WARN("add query range dimension failed", K(ret));
        } else if (OB_FAIL(index_dim.add_sharding_info_dim(index_info_entry->get_sharding_info(),
                                                             is_get && 1 == range_cnt,
                                                             *allocator_))) {
          LOG_WARN("add partition num dimension failed");
        } else {
          index_dim.set_is_get(is_get);
        }
      }
    }
  }
  return ret;
}

int ObJoinOrder::is_vector_inv_index_tid(const uint64_t index_table_id, bool& is_vec_tid)
{
  int ret = OB_SUCCESS;
  ObSqlSchemaGuard *schema_guard = NULL;
  const ObTableSchema *index_schema = NULL;
  if (OB_ISNULL(schema_guard = OPT_CTX.get_sql_schema_guard())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret), K(get_plan()), K(schema_guard));
  } else if (OB_FAIL(schema_guard->get_table_schema(index_table_id, index_schema))) {
    LOG_WARN("failed to get table schema", K(ret));
  } else if (OB_ISNULL(index_schema)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("index schema should not be null", K(ret));
  } else {
    is_vec_tid = index_schema->is_vec_index();
  }
  return ret;
}

/*
 * Trim index
 * @table_id table_id obtained from table_item
 * @base_table_id actual table_id
 * @stmt
 * @do_prunning flag to mark whether to do skyline branch pruning
 * @index_info_cache cache of index query range
 * @valid_index_ids available index ids
 * @skyline_index_ids trimmed index ids
 * */
int ObJoinOrder::skyline_prunning_index(const uint64_t table_id,
                                        const uint64_t base_table_id,
                                        const ObDMLStmt *stmt,
                                        const ObIndexInfoCache &index_info_cache,
                                        const ObIArray<uint64_t> &valid_index_ids,
                                        ObIArray<uint64_t> &skyline_index_ids,
                                        ObIArray<ObRawExpr *> &restrict_infos,
                                        bool ignore_index_back_dim)
{
  int ret = OB_SUCCESS;
  ObSEArray<uint64_t, 4> valid_unique_index_ids;
  ObSEArray<uint64_t, 4> candidate_index_ids;
  bool use_unique_index = false;
  OPT_TRACE_TITLE("BEGIN SKYLINE INDEX PRUNNING");
  if (OB_FAIL(try_prune_non_unique_index(table_id,
                                         index_info_cache,
                                         valid_index_ids,
                                         candidate_index_ids,
                                         use_unique_index))) {
    LOG_WARN("failed to prune non unique index", K(ret));
  } else {
    OPT_TRACE("candidate valid index ids", candidate_index_ids);
    LOG_TRACE("succeed to get valid index ids",
        K(candidate_index_ids), K(use_unique_index));
    // Dimension statistics information
    ObSkylineDimRecorder recorder;
    bool has_add = false;
    for (int64_t i = 0; OB_SUCC(ret) && i < candidate_index_ids.count(); ++i) {
      const uint64_t tid = candidate_index_ids.at(i);
      LOG_TRACE("cal dimension info of index", K(tid));
      ObIndexSkylineDim *index_dim = NULL;
      if (OB_FAIL(ObSkylineDimFactory::get_instance().create_skyline_dim(*allocator_, index_dim))) {
        LOG_WARN("failed to create index skylined dimension", K(ret));
      } else if (OB_ISNULL(index_dim)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("index dimension should not be null", K(ret));
      } else if (FALSE_IT(index_dim->set_index_id(tid))) {
      // Calculate dimension information, encapsulated in ObSkylineIndexDim
      } else if (OB_FAIL(cal_dimension_info(table_id,
                                            base_table_id,
                                            tid, stmt,
                                            *index_dim, index_info_cache,
                                            restrict_infos,
                                            use_unique_index,
                                            use_unique_index, /*ignore intersting order*/
                                            ignore_index_back_dim))) {
        LOG_WARN("Failed to cal dimension info", K(ret), "index_id", candidate_index_ids, K(i));
      } else if (stmt->has_vec_approx()) {
        // check tid is vec_index
        bool is_vec_tid = false;
        if (OB_FAIL(is_vector_inv_index_tid(tid, is_vec_tid))) {
          LOG_WARN("Failed to check is vec index tid", K(ret), K(tid));
        } else {
          index_dim->set_can_prunning(false);
        }
      }

      if (OB_FAIL(ret)) {
      } else if (OB_FAIL(recorder.add_index_dim(*index_dim, has_add))) {
        LOG_WARN("failed to add index dimension", K(ret));
      }
    }
    if (OB_SUCC(ret)) {
      skyline_index_ids.reset();
      // Get the cropped index id
      if (OB_FAIL(recorder.get_dominated_idx_ids(skyline_index_ids))) {
        LOG_WARN("get dominated idx ids failed", K(ret));
      } else {
        LOG_TRACE("after prunning remain index ids", K(skyline_index_ids));
        OPT_TRACE("after prunning remain index ids", skyline_index_ids);
      }
    }
  }
  return ret;
}

int ObJoinOrder::fill_index_info_entry(const uint64_t table_id,
                                       const uint64_t base_table_id,
                                       const uint64_t index_id,
                                       IndexInfoEntry *&index_entry,
                                       PathHelper &helper)
{
  int ret = OB_SUCCESS;
  const ObTableSchema *index_schema = NULL;
  ObSqlSchemaGuard *schema_guard = NULL;
  const ObDMLStmt *stmt = NULL;
  index_entry = NULL;
  if (OB_ISNULL(get_plan()) || OB_ISNULL(stmt = get_plan()->get_stmt()) ||
      OB_ISNULL(schema_guard = OPT_CTX.get_sql_schema_guard()) ||
      OB_ISNULL(allocator_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(get_plan()), K(stmt), K(schema_guard), K(ret));
  }
  if (OB_FAIL(ret)) {
    // do nothing
  } else if (OB_FAIL(schema_guard->get_table_schema(index_id, index_schema, ObSqlSchemaGuard::is_link_table(stmt, table_id)))) {
    LOG_WARN("failed to get index schema", K(ret));
  } else if (OB_ISNULL(index_schema)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret), K(table_id), K(base_table_id), K(index_id));
  } else {
    void *ptr = allocator_->alloc(sizeof(IndexInfoEntry));
    if (OB_ISNULL(ptr)) {
      ret = OB_ALLOCATE_MEMORY_FAILED;
      LOG_WARN("entry should not be null", K(ret));
    } else {
      IndexInfoEntry *entry = new (ptr) IndexInfoEntry();
      ObOrderDirection direction = default_asc_direction();
      bool is_index_back = false;
      bool is_unique_index = false;
      bool is_index_global = false;
      bool is_index_geo = index_schema->is_spatial_index();
      entry->set_index_id(index_id);
      int64_t interesting_order_info = OrderingFlag::NOT_MATCH;
      int64_t max_prefix_count = 0;
      bool force_direction = false;
      if (OB_FAIL(get_simple_index_info(table_id, base_table_id, index_id,
                                               is_unique_index, is_index_back, is_index_global))) {
        LOG_WARN("failed to get simple index info", K(ret));
      } else if (OB_FAIL(get_access_path_ordering(table_id, base_table_id, index_id,
                                           entry->get_ordering_info().get_index_keys(),
                                           entry->get_ordering_info().get_ordering(),
                                           direction,
                                           force_direction,
                                           is_index_back))) {
        LOG_WARN("get access path ordering ", K(ret));
      } else {
        entry->set_is_index_global(is_index_global);
        entry->set_is_index_geo(is_index_geo);
        entry->set_is_index_back(is_index_back);
        entry->set_is_unique_index(is_unique_index);
        entry->set_is_fulltext_index(index_schema->is_fts_index_aux());
        entry->set_is_multivalue_index(index_schema->is_multivalue_index_aux());
        entry->set_is_vector_index(index_schema->is_vec_index());
        entry->get_ordering_info().set_scan_direction(direction);
        entry->set_force_direction(force_direction);
      }
      if (OB_SUCC(ret)) {
        ObSEArray<OrderItem, 4> index_ordering;
        ObIArray<ObRawExpr*> &ordering_expr = entry->get_ordering_info().get_ordering();
        for (int64_t i = 0; OB_SUCC(ret) && i < ordering_expr.count(); ++i) {
          OrderItem order_item(ordering_expr.at(i), direction);
          if (OB_FAIL(index_ordering.push_back(order_item))) {
            LOG_WARN("failed to push back order item", K(ret));
          }
        }
        if (OB_FAIL(ret) || helper.is_inner_path_) {
          // The ordering of inner path can not be preserved
        } else if (OB_FAIL(check_all_interesting_order(index_ordering,
                                                       stmt,
                                                       max_prefix_count,
                                                       interesting_order_info))) {
          LOG_WARN("failed to check all interesting order", K(ret));
        } else {
          entry->set_interesting_order_prefix_count(max_prefix_count);
          entry->set_interesting_order_info(interesting_order_info);
        }
      }
      if (OB_SUCC(ret)) {
        if ((index_id == base_table_id && is_virtual_table(index_id)
             && entry->get_ordering_info().get_index_keys().count() <= 0)) {
           //ignore extract query range
          LOG_TRACE("ignore virtual table", K(base_table_id), K(index_id));
        } else if (OB_FAIL(get_query_range_info(table_id,
                                                base_table_id,
                                                index_id,
                                                entry->get_range_info(),
                                                helper))) {
          LOG_WARN("failed to get query range", K(ret), K(table_id), K(base_table_id), K(index_id));
        } else {
          LOG_TRACE("finish extract query range", K(table_id), K(index_id));
        }
      }
      if (OB_SUCC(ret)) {
        if (OB_FAIL(compute_table_location_for_index_info_entry(table_id, base_table_id, entry))) {
          LOG_WARN("compte table location for index info entry failed", K(ret));
        } else if (OB_FAIL(compute_sharding_info_for_index_info_entry(table_id, base_table_id, entry))) {
          LOG_WARN("compte sharding info for index info entry failed", K(ret));
        }
      }
      if (OB_SUCC(ret)) {
        index_entry = entry;
      }
      if (OB_FAIL(ret) && OB_NOT_NULL(entry)) {
        entry->~IndexInfoEntry();
      }
    }
  }
  return ret;
}

//fill all index info entry
int ObJoinOrder::fill_index_info_cache(const uint64_t table_id,
                                       const uint64_t base_table_id,
                                       const ObIArray<uint64_t> &valid_index_ids,
                                       ObIndexInfoCache &index_info_cache,
                                       PathHelper &helper)
{
  int ret = OB_SUCCESS;
  index_info_cache.set_table_id(table_id);
  index_info_cache.set_base_table_id(base_table_id);
  for (int64_t i = 0; OB_SUCC(ret) && i < valid_index_ids.count(); i++) {
    IndexInfoEntry *index_info_entry = NULL;
    if (OB_FAIL(fill_index_info_entry(table_id, base_table_id,
                                      valid_index_ids.at(i), index_info_entry, helper))) {
      LOG_WARN("fill_index_info_entry failed", K(ret));
    } else if (OB_ISNULL(index_info_entry)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("fill index info entry failed", K(ret));
    } else if (OB_FAIL(index_info_cache.add_index_info_entry(index_info_entry))) {
      LOG_WARN("failed to add index info entry", K(ret));
    } else {
      LOG_TRACE("succeed to fill index index entry", K(*index_info_entry));
    }
  }
  if (FAILEDx(refine_unique_range_rowcnt(table_id, valid_index_ids, index_info_cache))) {
    LOG_WARN("failed to adjust range rows", K(ret));
  }
  return ret;
}

int ObJoinOrder::refine_unique_range_rowcnt(const uint64_t table_id,
                                            const common::ObIArray<uint64_t> &valid_index_ids,
                                            ObIndexInfoCache &index_info_cache)
{
  int ret = OB_SUCCESS;
  ObSEArray<const QueryRangeInfo*, 4> unique_range_info;
  ObSEArray<const OrderingInfo*, 4> unique_ordering_info;
  for(int64_t i = 0; OB_SUCC(ret) && i < valid_index_ids.count(); ++i) {
    uint64_t index_id = valid_index_ids.at(i);
    IndexInfoEntry *index_info_entry = nullptr;
    const OrderingInfo *temp_ordering_info = nullptr;
    int64_t range_cnt = INT64_MAX;
    if (OB_FAIL(index_info_cache.get_index_info_entry(table_id,
                                                      index_id,
                                                      index_info_entry))) {
      LOG_WARN("failed to get query range info", K(table_id), K(index_id), K(ret));
    } else if (OB_ISNULL(index_info_entry)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("index info is NULL", K(ret));
    } else if (!index_info_entry->is_valid_unique_index()) {
      // do nothing
    } else if (OB_FAIL(unique_range_info.push_back(&index_info_entry->get_range_info()))) {
      LOG_WARN("failed to push back range info", K(ret));
    } else if (OB_FAIL(index_info_cache.get_access_path_ordering(table_id,
                                                                 index_id,
                                                                 temp_ordering_info))) {
      LOG_WARN("failed to get ordering info", K(table_id), K(index_id), K(ret));
    } else if (OB_ISNULL(temp_ordering_info)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("ordering info is null", K(ret));
    } else if (OB_FAIL(unique_ordering_info.push_back(temp_ordering_info))) {
      LOG_WARN("failed to push back ordering info", K(ret));
    }
    if (OB_SUCC(ret) && (index_info_entry->get_range_info().is_get() ||
                         index_info_entry->is_valid_unique_index())){
      index_info_entry->get_range_info().
          set_unique_range_rowcnt(index_info_entry->get_range_info().get_range_count());
    }
  }
  if (OB_SUCC(ret) && !unique_range_info.empty()) {
    for(int64_t i = 0; OB_SUCC(ret) && i < valid_index_ids.count(); i++) {
      uint64_t index_id = valid_index_ids.at(i);
      IndexInfoEntry *index_info_entry = nullptr;
      const OrderingInfo *temp_ordering_info = nullptr;
      if (OB_FAIL(index_info_cache.get_index_info_entry(table_id,
                                                        index_id,
                                                        index_info_entry))) {
        LOG_WARN("failed to get query range info", K(table_id), K(index_id), K(ret));
      } else if (OB_ISNULL(index_info_entry)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("index info is NULL", K(ret));
      } else if (OB_FAIL(index_info_cache.get_access_path_ordering(table_id,
                                                                  index_id,
                                                                  temp_ordering_info))) {
        LOG_WARN("failed to get ordering info", K(table_id), K(index_id), K(ret));
      } else if (OB_ISNULL(temp_ordering_info)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("ordering info is null", K(ret));
      } else {
        QueryRangeInfo &temp_range_info = index_info_entry->get_range_info();
        for (int64_t j = 0; OB_SUCC(ret) && j < unique_range_info.count(); j++) {
          DominateRelation status = DominateRelation::OBJ_UNCOMPARABLE;
          if (&temp_range_info == unique_range_info.at(j)) {
            // do nothing
          } else if (temp_range_info.get_range_prefix_count()
              >= unique_range_info.at(j)->get_index_column_count()) {
            if (OB_FAIL(check_index_subset(unique_ordering_info.at(j),
                                           unique_range_info.at(j)->get_index_column_count(),
                                           temp_ordering_info,
                                           temp_range_info.get_range_prefix_count(),
                                           status))) {
              LOG_WARN("failed to compare two index", K(ret));
            } else if (status == DominateRelation::OBJ_LEFT_DOMINATE ||
                       status == DominateRelation::OBJ_EQUAL) {
              int64_t unique_rows = unique_range_info.at(j)->get_range_count();
              if (temp_range_info.get_unique_range_rowcnt() < 0) {
                temp_range_info.set_unique_range_rowcnt(unique_rows);
              } else {
                temp_range_info.set_unique_range_rowcnt(
                    std::min(unique_rows, temp_range_info.get_unique_range_rowcnt()));
              }
            }
          }
        }
        LOG_TRACE("succeed to refine index range prefix rowcount",
            K(index_id), K(temp_range_info.get_unique_range_rowcnt()));
        if (temp_range_info.get_unique_range_rowcnt() > 0 ) {
          OPT_TRACE("The max logical range rowcount of index", index_id,
              "is", temp_range_info.get_unique_range_rowcnt());
        }
      }
    }
  }
  return ret;
}

int ObJoinOrder::create_access_paths(const uint64_t table_id,
                                     const uint64_t ref_table_id,
                                     PathHelper &helper,
                                     ObIArray<AccessPath *> &access_paths,
                                     ObIndexInfoCache &index_info_cache)
{
  int ret = OB_SUCCESS;
  ObSEArray<uint64_t, 4> candi_index_ids;
  ObSEArray<uint64_t, 4> valid_index_ids;
  const ObDMLStmt *stmt = NULL;
  ObOptimizerContext *opt_ctx = NULL;
  const ParamStore *params = NULL;
  bool is_valid = true;
  ObSQLSessionInfo *session_info = NULL;
  bool use_index_merge = false;
  bool use_index_merge_by_hint = false;
  ObArray<ObIndexMergeNode*> union_merge_nodes;
  bool ignore_normal_access_path = false;
  if (OB_ISNULL(get_plan()) ||
      OB_ISNULL(stmt = get_plan()->get_stmt()) ||
      OB_ISNULL(opt_ctx = &get_plan()->get_optimizer_context()) ||
      OB_ISNULL(params = opt_ctx->get_params()) ||
      OB_ISNULL(opt_ctx->get_exec_ctx()) ||
      OB_ISNULL(opt_ctx->get_exec_ctx()->get_sql_ctx()) ||
      OB_ISNULL(helper.table_opt_info_) ||
      OB_ISNULL(session_info = opt_ctx->get_session_info())) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("get unexpected null", K(get_plan()), K(opt_ctx), K(params),
        K(stmt), K(ret));
  } else if (OB_FAIL(get_generated_col_index_qual(table_id,
                                                  helper.filters_, helper))) {
    LOG_WARN("get prefix index qual failed");
  } else if (OB_FAIL(init_basic_text_retrieval_info(table_id,
                                                    ref_table_id,
                                                    helper))) {
    LOG_WARN("failed to init basic text retrieval info", K(ret));
  } else if (OB_FAIL(create_index_merge_access_paths(table_id,
                                                     ref_table_id,
                                                     helper,
                                                     index_info_cache,
                                                     access_paths,
                                                     ignore_normal_access_path))) {
    LOG_WARN("failed to create index merge access paths", K(ret));
  } else if (ignore_normal_access_path) {
    // do nothing
  } else if (OB_FAIL(get_valid_index_ids(table_id,
                                         ref_table_id,
                                         helper,
                                         candi_index_ids))) {
    LOG_WARN("failed to get valid index ids", K(ret));
  } else if (OB_FAIL(fill_index_info_cache(table_id, ref_table_id,
                                           candi_index_ids,
                                           index_info_cache,
                                           helper))) {
    LOG_WARN("failed to fill index info cache", K(ret));
  } else if (OB_FAIL(add_table_by_heuristics(table_id, ref_table_id,
                                             index_info_cache,
                                             candi_index_ids,
                                             valid_index_ids,
                                             helper))) {
    LOG_WARN("failed to add table by heuristics", K(ret));
  } else if (valid_index_ids.empty() && OB_FAIL(skyline_prunning_index(table_id,
                                                                       ref_table_id,
                                                                       stmt,
                                                                       index_info_cache,
                                                                       candi_index_ids,
                                                                       valid_index_ids,
                                                                       helper.filters_))) {
    LOG_WARN("failed to pruning_index", K(table_id), K(ref_table_id), K(ret));
  } else {
    helper.table_opt_info_->optimization_method_ = OptimizationMethod::COST_BASED;
    for (int64_t i = 0; OB_SUCC(ret) && i < valid_index_ids.count(); ++i) {
      bool is_create_basic_path = false;
      bool is_create_das_path = false;
      bool use_column_store = false;
      bool use_row_store = false;
      AccessPath *das_row_store_access_path = NULL;
      AccessPath *basic_row_store_access_path = NULL; // the path does not use DAS, maybe optimal sometime.
      AccessPath *das_column_store_access_path = NULL;
      AccessPath *basic_column_store_access_path = NULL;
      OptSkipScanState use_skip_scan = OptSkipScanState::SS_UNSET;
      if (OB_FAIL(will_use_das(table_id,
                               valid_index_ids.at(i),
                               index_info_cache,
                               helper,
                               is_create_das_path,
                               is_create_basic_path))) {
        LOG_WARN("failed to check will use das", K(ret));
      } else if (OB_FAIL(will_use_skip_scan(table_id,
                                            ref_table_id,
                                            valid_index_ids.at(i),
                                            index_info_cache,
                                            helper,
                                            session_info,
                                            use_skip_scan))) {
        LOG_WARN("failed to check will use skip scan", K(ret));
      } else if (OB_FAIL(get_plan()->will_use_column_store(table_id,
                                                          valid_index_ids.at(i),
                                                          ref_table_id,
                                                          use_column_store,
                                                          use_row_store))) {
        LOG_WARN("failed to check will use column store", K(ret));
      } else if (is_create_das_path &&
                 use_row_store &&
                 OB_FAIL(create_one_access_path(table_id,
                                                ref_table_id,
                                                valid_index_ids.at(i),
                                                index_info_cache,
                                                helper,
                                                das_row_store_access_path,
                                                true,
                                                false,
                                                use_skip_scan))) {
        LOG_WARN("failed to make index path", "index_table_id", valid_index_ids.at(i), K(ret));
      } else if (OB_NOT_NULL(das_row_store_access_path) &&
                 OB_FAIL(access_paths.push_back(das_row_store_access_path))) {
        LOG_WARN("failed to push back access path", K(ret));
      } else if (is_create_das_path &&
                 use_column_store &&
                 OB_FAIL(create_one_access_path(table_id,
                                                ref_table_id,
                                                valid_index_ids.at(i),
                                                index_info_cache,
                                                helper,
                                                das_column_store_access_path,
                                                true,
                                                true,
                                                use_skip_scan))) {
        LOG_WARN("failed to make index path", "index_table_id", valid_index_ids.at(i), K(ret));
      } else if (OB_NOT_NULL(das_column_store_access_path) &&
                 OB_FAIL(access_paths.push_back(das_column_store_access_path))) {
        LOG_WARN("failed to push back access path", K(ret));
      } else if (is_create_basic_path &&
                 use_row_store &&
                 OB_FAIL(create_one_access_path(table_id,
                                                ref_table_id,
                                                valid_index_ids.at(i),
                                                index_info_cache,
                                                helper,
                                                basic_row_store_access_path,
                                                false,
                                                false,
                                                use_skip_scan))) {
        LOG_WARN("failed to make index path", "index_table_id", valid_index_ids.at(i), K(ret));
      } else if(OB_NOT_NULL(basic_row_store_access_path) &&
                OB_FAIL(access_paths.push_back(basic_row_store_access_path))) {
        LOG_WARN("failed to push back access path", K(ret));
      } else if (is_create_basic_path &&
                 use_column_store &&
                 OB_FAIL(create_one_access_path(table_id,
                                                ref_table_id,
                                                valid_index_ids.at(i),
                                                index_info_cache,
                                                helper,
                                                basic_column_store_access_path,
                                                false,
                                                true,
                                                use_skip_scan))) {
        LOG_WARN("failed to make index path", "index_table_id", valid_index_ids.at(i), K(ret));
      } else if(OB_NOT_NULL(basic_column_store_access_path) &&
                OB_FAIL(access_paths.push_back(basic_column_store_access_path))) {
        LOG_WARN("failed to push back access path", K(ret));
      }
    }
  }
  return ret;
}

int ObJoinOrder::create_index_merge_access_paths(
    const uint64_t table_id, const uint64_t ref_table_id,
    const PathHelper &helper, ObIndexInfoCache &index_info_cache,
    ObIArray<AccessPath *> &access_paths, bool &ignore_normal_access_path) {
  adlog::DEBUG("ObJoinOrder::create_index_merge_access_paths START");
  int ret = OB_SUCCESS;
  const ObDMLStmt *stmt = NULL;
  ObQueryCtx *query_ctx = NULL;
  ObSEArray<ObIndexMergeNode*, 4> candi_index_trees;
  bool is_match_hint = false;
  bool contain_fts = false;
  ignore_normal_access_path = false;
  LOG_TRACE("check can use index merge begin", K(ref_table_id), K(helper.filters_));
  OPT_TRACE_TITLE("BEGIN CREATE INDEX MERGE PATHS");
  OPT_TRACE("table_id: ", table_id, "ref_table_id: ", ref_table_id);
  OPT_TRACE("full query filters: ", helper.filters_);
  if (OB_ISNULL(get_plan()) ||
      OB_ISNULL(stmt = get_plan()->get_stmt()) ||
      OB_ISNULL(query_ctx = stmt->get_query_ctx())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret), K(get_plan()), K(stmt), K(query_ctx));
  } else if (is_virtual_table(ref_table_id)) {
    OPT_TRACE("can not create index merge paths for virtual table");
  } else if (OB_FAIL(get_candi_index_merge_trees(table_id,
                                                 ref_table_id,
                                                 helper,
                                                 candi_index_trees,
                                                 is_match_hint))) {
    LOG_WARN("failed to get valid index ids", K(ret));
    adlog::DEBUG("failed ObJoinOrder::get_candi_index_merge_trees");
  } else if (candi_index_trees.empty()) {
    // do nothing
    adlog::DEBUG("ObJoinOrder::create_index_merge_access_paths  candi_index_trees.empty()");
  } else if (OB_FAIL(do_create_index_merge_paths(table_id,
                                                 ref_table_id,
                                                 helper,
                                                 index_info_cache,
                                                 candi_index_trees,
                                                 access_paths))) {
    LOG_WARN("failed to create index merge paths", K(ret));
  } else if (OB_UNLIKELY(EN_FORCE_INDEX_MERGE_PLAN)) {
    ignore_normal_access_path = true;
    LOG_TRACE("[EN_FORCE_INDEX_MERGE_PLAN] finish create index merge path ", K(ref_table_id), K(is_match_hint), K(contain_fts), K(access_paths));
  } else if (OB_FAIL(check_index_merge_paths_contain_fts(access_paths, contain_fts))) {
    LOG_WARN("failed to check index merge paths contain FTS", K(ret));
  } else if (!is_match_hint && !contain_fts
             && OB_FAIL(prune_index_merge_path(access_paths))) {
    LOG_WARN("failed to prune index merge path", K(ret));
  } else {
    ignore_normal_access_path = (is_match_hint || contain_fts) && !access_paths.empty();
    LOG_TRACE("finish create index merge path", K(ref_table_id), K(is_match_hint), K(contain_fts), K(access_paths));
  }
  adlog::DEBUG("ObJoinOrder::create_index_merge_access_paths END");

  return ret;
}

int ObJoinOrder::get_candi_index_merge_trees(
    const uint64_t table_id, const uint64_t ref_table_id,
    const PathHelper &helper, ObIArray<ObIndexMergeNode *> &candi_index_trees,
    bool &is_match_hint) {
  adlog::DEBUG("ObJoinOrder::get_candi_index_merge_trees START");
  int ret = OB_SUCCESS;
  ObSEArray<uint64_t, 4> valid_index_ids; // all valid indexes
  ObSEArray<ObSEArray<uint64_t, 4>, 4> valid_index_cols; // column ids in the valid indexes
  omt::ObTenantConfigGuard tenant_config(TENANT_CONF(MTL_ID()));
  is_match_hint = false;
  if (OB_ISNULL(get_plan()) || OB_UNLIKELY(!tenant_config.is_valid())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get invalid plan or tenant config", K(ret), K(get_plan()), K(tenant_config.is_valid()));
  } else if (OB_FAIL(get_valid_index_merge_indexes(table_id,
                                                   ref_table_id,
                                                   valid_index_ids,
                                                   valid_index_cols,
                                                   false /* ignore_hint */ ))) {
    LOG_WARN("failed to get valid index merge ids with hint", K(ret));
  } else if (OB_FAIL(generate_candi_index_merge_trees(ref_table_id,
                                                      helper.filters_,
                                                      valid_index_ids,
                                                      valid_index_cols,
                                                      candi_index_trees))) {
    LOG_WARN("failed to generate candi index merge trees", K(ret));
  } else if (!candi_index_trees.empty()
             && OB_FAIL(check_candi_index_trees_match_hint(table_id, candi_index_trees))) {
    LOG_WARN("failed to check candi index trees match hint", K(ret));
  } else if (!candi_index_trees.empty()) {
    is_match_hint = true;
    OPT_TRACE("generate", candi_index_trees.count(), "candi index merge trees using hint");
    LOG_TRACE("generate candi index merge trees using hint", K(table_id), K(candi_index_trees));
  } else if (get_plan()->get_log_plan_hint().is_outline_data_
             || (!tenant_config->_enable_index_merge && OB_LIKELY(!EN_FORCE_INDEX_MERGE_PLAN))) {
    OPT_TRACE("can not create index merge paths due to outline or tenant config");
  } else if (OB_FAIL(get_valid_index_merge_indexes(table_id,
                                                   ref_table_id,
                                                   valid_index_ids,
                                                   valid_index_cols,
                                                   true /* ignore_hint */ ))) {
    LOG_WARN("failed to get valid index merge ids", K(ret));
  } else if (OB_FAIL(generate_candi_index_merge_trees(ref_table_id,
                                                      helper.filters_,
                                                      valid_index_ids,
                                                      valid_index_cols,
                                                      candi_index_trees))) {
    LOG_WARN("failed to generate candi index merge trees", K(ret));
    adlog::DEBUG("fail generate_candi_index_merge_trees");
  } else if (!candi_index_trees.empty()
             && OB_FAIL(prune_candi_index_merge_trees(candi_index_trees))) {
    LOG_WARN("failed to prune candi index trees", K(ret));
    adlog::DEBUG("fail prune_candi_index_merge_trees");

  } else {
    OPT_TRACE("generate", candi_index_trees.count(), "candi index merge trees");
    LOG_TRACE("generate candi index merge trees", K(table_id),
              K(candi_index_trees));
    adlog::DEBUG("generate candi index merge trees, count() ",candi_index_trees.count());
    if(candi_index_trees.count() > 0){
      auto &&one = candi_index_trees.at(0);
      adlog::DEBUG("first node type : ", (int64_t) one->node_type_);
    }
  }
  adlog::DEBUG("ObJoinOrder::get_candi_index_merge_trees END");

  return ret;
}

int ObJoinOrder::get_valid_index_merge_indexes(
    const uint64_t table_id, const uint64_t ref_table_id,
    ObIArray<uint64_t> &valid_index_ids,
    ObIArray<ObSEArray<uint64_t, 4>> &valid_index_cols, bool ignore_hint) {
  adlog::DEBUG("ObJoinOrder::get_valid_index_merge_indexes");
  int ret = OB_SUCCESS;
  ObSqlSchemaGuard *schema_guard = NULL;
  const ObTableSchema *table_schema = NULL;
  ObSEArray<uint64_t, 4> index_ids;
  ObSEArray<uint64_t, 4> index_column_ids;
  valid_index_ids.reuse();
  valid_index_cols.reuse();
  if (OB_ISNULL(get_plan()) ||
      OB_ISNULL(schema_guard = get_plan()->get_optimizer_context().get_sql_schema_guard())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected nullptr", K(get_plan()), K(schema_guard), K(ret));
  } else if (ignore_hint) {
    ObSEArray<ObAuxTableMetaInfo, 4> simple_index_infos;
    if (OB_FAIL(index_ids.push_back(ref_table_id))) {
      LOG_WARN("failed to push back ref table id", K(ret));
    } else if (OB_FAIL(schema_guard->get_table_schema(ref_table_id, table_schema))) {
      LOG_WARN("failed to get table schema", K(ref_table_id), K(ret));
    } else if (OB_ISNULL(table_schema)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("get unexpected null table schema", K(ret), K(ref_table_id));
    } else if (OB_FAIL(table_schema->get_simple_index_infos(simple_index_infos))) {
      LOG_WARN("failed to get simple index infos", K(ret));
    }
    for (int64_t i = 0; OB_SUCC(ret) && i < simple_index_infos.count(); ++i) {
      if (OB_FAIL(index_ids.push_back(simple_index_infos.at(i).table_id_))) {
        LOG_WARN("failed to push back table id", K(ret), K(i));
      }
    }
  } else {
    const LogTableHint *log_table_hint = get_plan()->get_log_plan_hint().get_log_table_hint(table_id);
    if (NULL == log_table_hint || 2 > log_table_hint->union_merge_list_.count()) {
      // do nothing
    } else if (OB_FAIL(index_ids.assign(log_table_hint->union_merge_list_))) {
      LOG_WARN("failed to assign union merge list", K(ret));
    }
  }

  // 验证每个候选索引是否真正支持索引合并
  adlog::DEBUG("检验每一个候选索引是否都支持索引合并");
  adlog::DEBUG("总共候选索引的个数有 ",index_ids.count());
  for (int64_t i = 0; OB_SUCC(ret) && i < index_ids.count(); ++i) {
    index_column_ids.reuse();
    const ObTableSchema *index_schema = NULL;
    uint64_t index_id = index_ids.at(i);
    if (OB_FAIL(schema_guard->get_table_schema(index_id, index_schema))) {
      LOG_WARN("failed to get index table schema", K(ret), K(index_id));
    } else if (OB_ISNULL(index_schema)) {
      ret = OB_ERR_UNEXPECTED;
      adlog::DEBUG(i," OB_ISNULL(index_schema)");
      LOG_WARN("get unexpected null index schema", K(ret));
    } else if (ref_table_id == index_id ||
               (index_schema->is_index_local_storage() &&
                (index_schema->is_normal_index() || index_schema->is_fts_index()))) {
      /* primary table, local normal index and local fts index are available for
       * index merge now */

      adlog::DEBUG(i," 有初步满足条件");
      
      if (OB_FAIL(index_schema->get_rowkey_column_ids(index_column_ids))) {
        LOG_WARN("failed to get all column ids", K(ret));
      } else if (OB_FAIL(valid_index_ids.push_back(index_id))) {
        LOG_WARN("failed to push back index id", K(ret), K(index_id));
      } else if (OB_FAIL(valid_index_cols.push_back(index_column_ids))) {
        LOG_WARN("failed to push back index column ids", K(ret), K(index_id), K(index_column_ids));
      }
      if (OB_FAIL(ret)) {
        adlog::DEBUG("中间出了错误");
      }
      adlog::DEBUG(i," 有完整满足条件");

    } else if (!ignore_hint) {
      // hint is invalid
      valid_index_ids.reuse();
      valid_index_cols.reuse();
      break;
    }
  }
  LOG_TRACE("get all valid indexes for index merge", K(table_id), K(ref_table_id), K(ignore_hint), K(valid_index_ids), K(valid_index_cols));
  return ret;
}

int ObJoinOrder::generate_candi_index_merge_trees(
    const uint64_t ref_table_id, const ObIArray<ObRawExpr *> &filters,
    ObIArray<uint64_t> &valid_index_ids,
    ObIArray<ObSEArray<uint64_t, 4>> &valid_index_cols,
    ObIArray<ObIndexMergeNode *> &candi_index_trees) {
  adlog::DEBUG("ObJoinOrder::generate_candi_index_merge_trees START");
  int ret = OB_SUCCESS;
  if (valid_index_ids.empty()) {
    // do nothing
    adlog::DEBUG("valid_index_ids 是空的");
  } else {
    adlog::DEBUG("filters.count() = ", filters.count());

    if (filters.count() <= 1) {
      // 处理的是OR的
      for (int64_t i = 0; OB_SUCC(ret) && i < filters.count(); ++i) {
        ObRawExpr *filter = filters.at(i);
        ObIndexMergeNode *candi_node = NULL;
        bool is_valid = false;
        if (OB_ISNULL(filter)) {
          ret = OB_ERR_UNEXPECTED;
          LOG_WARN("get unexpected null filter", K(ret), K(i));
        } else if (filter->get_expr_type() != T_OP_OR and filter->get_expr_type() != T_OP_AND) {
          // do nothing, only support UNION MERGE now // 现在尝试支持了AND
          adlog::DEBUG("filter->get_expr_type() != T_OP_OR and filter->get_expr_type() != T_OP_AND");
          adlog::DEBUG("filter[", i, "] type = ", filter->get_expr_type(), " name = ", filter->get_expr_name());
        } else if (OB_FAIL(generate_candi_index_merge_node(ref_table_id,
                                                         filter,
                                                         valid_index_ids,
                                                         valid_index_cols,
                                                         candi_node,
                                                         is_valid))) {
          LOG_WARN("failed to generate one index merge tree for filter", K(ret), KPC(filter));
        } else if (!is_valid || NULL == candi_node) {
          // do nothing
          adlog::DEBUG("!is_valid || NULL == candi_node");
        } else if (OB_FAIL(candi_node->formalize_index_merge_tree())) {
          LOG_WARN("failed to formalize candi index merge tree", K(ret), KPC(candi_node));
        } else {
          adlog::DEBUG("candi_index_trees push_back了一个node");
          if (OB_FAIL(candi_index_trees.push_back(candi_node))) {
            LOG_WARN("failed to push back candi index tree", K(ret));
          }
        }
      }
    }

    // 多个独立条件的AND组合
    if (candi_index_trees.empty() && filters.count() > 1) {
      // 检查所有filters是否都是可索引的简单条件
      ObSEArray<ObIndexMergeNode*, 4> valid_nodes;
      ObSEArray<bool, 4> valid_flags;
      
      for (int64_t i = 0; OB_SUCC(ret) && i < filters.count(); ++i) {
        ObRawExpr *filter = filters.at(i);
        if (OB_ISNULL(filter) || 
            filter->get_expr_type() == T_OP_OR || 
            filter->get_expr_type() == T_OP_AND) {
          continue; 
        }
        
        ObIndexMergeNode *candi_node = NULL;
        bool is_valid = false;
        if (OB_FAIL(generate_candi_index_merge_node(ref_table_id,
                                                   filter,
                                                   valid_index_ids,
                                                   valid_index_cols,
                                                   candi_node,
                                                   is_valid))) {
          LOG_WARN("failed to generate node for simple filter", K(ret), KPC(filter));
        } else if (is_valid && OB_NOT_NULL(candi_node)) {
          valid_nodes.push_back(candi_node);
          valid_flags.push_back(true);
        } else {
          valid_flags.push_back(false);
        }
      }
          // 如果所有条件都可索引，则构建AND树
      if (valid_flags.count() == filters.count() && valid_flags.count() > 1 && valid_nodes.count() > 1) {
        ObIndexMergeNode *and_root = NULL;
        if (OB_NOT_NULL(and_root = OB_NEWx(ObIndexMergeNode, allocator_))) {
          and_root->node_type_ = INDEX_MERGE_INTERSECT;
          for (int64_t i = 0; i < valid_nodes.count(); ++i) {
            if (OB_FAIL(and_root->children_.push_back(valid_nodes.at(i)))) {
              LOG_WARN("failed to add child to AND root", K(ret));
            }
          }
          adlog::DEBUG("and_root->node_type_ ",(int64_t)and_root->node_type_);
          adlog::DEBUG("and_root->children_ count() ",and_root->children_.count());
          if (OB_FAIL(and_root->formalize_index_merge_tree())) {
            LOG_WARN("failed to formalize AND tree", K(ret));
          } else {
            if (OB_FAIL(candi_index_trees.push_back(and_root))) {
              LOG_WARN("failed to push back AND tree", K(ret));
            } else {
              and_root->gen_code_ = 114514;
              adlog::DEBUG("Added AND merge tree for ", valid_nodes.count(), " conditions");
            }
          }
          adlog::DEBUG("and_root->node_type_ ",(int64_t)and_root->node_type_);
          adlog::DEBUG("and_root->children_ count() ",and_root->children_.count());
        }
      }
    }
  }
  adlog::DEBUG("ObJoinOrder::generate_candi_index_merge_trees END");
  return ret;
}

int ObJoinOrder::generate_candi_index_merge_node(const uint64_t ref_table_id,
                                                 ObRawExpr *filter,
                                                 ObIArray<uint64_t> &valid_index_ids,
                                                 ObIArray<ObSEArray<uint64_t, 4>> &valid_index_cols,
                                                 ObIndexMergeNode *&candi_node,
                                                 bool &is_valid_node)
{
  int ret = OB_SUCCESS;
  candi_node = NULL;
  is_valid_node = false;
  if (OB_ISNULL(filter)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null filter", K(ret));
  } else if (T_OP_OR == filter->get_expr_type() ||
             T_OP_AND == filter->get_expr_type()) {
    // 先分配好内存
    if (OB_ISNULL(candi_node = OB_NEWx(ObIndexMergeNode, allocator_))) {
      ret = OB_ALLOCATE_MEMORY_FAILED;
      LOG_WARN("failed to allocate index merge node", K(ret));
    } else {
      candi_node->node_type_ = (T_OP_OR == filter->get_expr_type()) ? INDEX_MERGE_UNION : INDEX_MERGE_INTERSECT;
      // is_valid_node = (T_OP_OR == filter->get_expr_type()) ? true : false;
      // 为了支持全是AND或者全是OR的事情
      is_valid_node = true;
    }
    for (int64_t i = 0; OB_SUCC(ret) && i < filter->get_param_count(); ++i) {
      ObIndexMergeNode *child = NULL;
      bool is_child_valid = false;
      if (OB_FAIL(THIS_WORKER.check_status())) {
        LOG_WARN("check status failed", K(ret));
      } else if (OB_FAIL(SMART_CALL(generate_candi_index_merge_node(ref_table_id,
                                                                    filter->get_param_expr(i),
                                                                    valid_index_ids,
                                                                    valid_index_cols,
                                                                    child,
                                                                    is_child_valid)))) {
        LOG_WARN("failed to construct index merge node", K(ret));
      } else if (!is_child_valid) {
        if (T_OP_OR == filter->get_expr_type()) {
          is_valid_node = false;
          break;
        }
        if (T_OP_AND == filter->get_expr_type()) {
          is_valid_node = false;
          break;
        }
      } else if (OB_ISNULL(child)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("child node is null", K(ret), KPC(filter));
      } else if (OB_FAIL(candi_node->children_.push_back(child))) {
        LOG_WARN("failed to push back child node", K(ret));
      } else if (T_OP_AND == filter->get_expr_type()) {
        is_valid_node = true;
      }
    }
  } else {
    ObSEArray<uint64_t, 1> candicate_index_tids;
    if (!get_tables().equal(filter->get_relation_ids())) {
      is_valid_node = false;
    } else if (!(filter->has_flag(IS_SIMPLE_COND) ||
                 filter->has_flag(IS_RANGE_COND) ||
                 filter->has_flag(CNT_MATCH_EXPR) ||
                 (filter->has_flag(IS_IN) ))) {
      is_valid_node = false;
    } else if (OB_FAIL(collect_candicate_indexes(ref_table_id,
                                                 filter,
                                                 valid_index_ids,
                                                 valid_index_cols,
                                                 candicate_index_tids))) {
      LOG_WARN("failed to choose candi index for filter", K(ret), K(filter));
    } else if (candicate_index_tids.empty()) {
      is_valid_node = false;
    } else if (OB_ISNULL(candi_node = OB_NEWx(ObIndexMergeNode, allocator_))) {
      ret = OB_ALLOCATE_MEMORY_FAILED;
      LOG_WARN("failed to allocate index merge node", K(ret));
    } else if (OB_FAIL(candi_node->filter_.push_back(filter))) {
      LOG_WARN("failed to push back filter", K(ret));
    } else if (OB_FAIL(candi_node->candicate_index_tids_.assign(candicate_index_tids))) {
      LOG_WARN("failed to assign candicate index tids", K(ret));
    } else {
      candi_node->node_type_ = filter->has_flag(CNT_MATCH_EXPR) ? INDEX_MERGE_FTS_INDEX : INDEX_MERGE_SCAN;
      candi_node->index_tid_ = candicate_index_tids.at(0);
      is_valid_node = true;
    }
  }
  return ret;
}

int ObJoinOrder::collect_candicate_indexes(const uint64_t ref_table_id,
                                           ObRawExpr *filter,
                                           ObIArray<uint64_t> &valid_index_ids,
                                           ObIArray<ObSEArray<uint64_t, 4>> &valid_index_cols,
                                           ObIArray<uint64_t> &candicate_index_tids)
{
  int ret = OB_SUCCESS;
  if (OB_ISNULL(filter) || OB_UNLIKELY(valid_index_ids.count() != valid_index_cols.count())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret), K(filter), K(valid_index_ids), K(valid_index_cols));
  } else if (filter->has_flag(CNT_MATCH_EXPR)) {
    /* direct choose corresponding fts index */
    ObMatchFunRawExpr *match_expr = NULL;
    if (filter->get_expr_type() == T_OP_BOOL && filter->has_flag(CNT_MATCH_EXPR)) {
      ObRawExpr *param_expr = filter->get_param_expr(0);
      if (OB_ISNULL(param_expr)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected null param expr for bool op", K(ret));
      } else if (param_expr->has_flag(IS_MATCH_EXPR)) {
        match_expr = static_cast<ObMatchFunRawExpr*>(param_expr);
      }
    }
    if (OB_SUCC(ret) && OB_NOT_NULL(match_expr)) {
      uint64_t inv_index_tid = OB_INVALID_ID;
      bool is_valid = false;
      if (OB_FAIL(get_matched_inv_index_tid(match_expr, ref_table_id, inv_index_tid))) {
        LOG_WARN("failed to get matched inv index tid", K(match_expr), K(ref_table_id), K(ret));
      } else {
        for (int64_t i = 0; i < valid_index_ids.count(); ++i) {
          if (inv_index_tid == valid_index_ids.at(i)) {
            is_valid = true;
            break;
          }
        }
        if (!is_valid) {
          // do nothing
        } else if (OB_FAIL(candicate_index_tids.push_back(inv_index_tid))) {
          LOG_WARN("failed to push back index id", K(inv_index_tid), K(ret));
        }
      }
    }
  } else {

    if (T_OP_IN == filter->get_expr_type() ) {
      ObRawExpr *left_expr = filter->get_param_expr(0);
      if (OB_ISNULL(left_expr) || !left_expr->is_column_ref_expr()) {
        // skip
      } else {
        ObColumnRefRawExpr *col_expr = static_cast<ObColumnRefRawExpr*>(left_expr);
        uint64_t column_id = col_expr->get_column_id();
    
        for (int64_t i = 0; OB_SUCC(ret) && i < valid_index_ids.count(); ++i) {
          const ObIArray<uint64_t> &index_cols = valid_index_cols.at(i);
          if (!index_cols.empty() && index_cols.at(0) == column_id) {
            if (OB_FAIL(candicate_index_tids.push_back(valid_index_ids.at(i)))) {
              LOG_WARN("failed to push back index id", K(ret));
            }
          }
        }
      }
    }
    else {
      
      /* record all indexes which satisfy the index prefix condition */
      ObSEArray<uint64_t, 4> column_ids;
      if (OB_FAIL(ObRawExprUtils::extract_column_ids(filter, column_ids))) {
        LOG_WARN("failed to extract column ids", K(filter), K(ret));
      } else if (OB_UNLIKELY(column_ids.count() != 1)) {
        adlog::DEBUG("!=1: ", K(column_ids.count()));
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("invalid filter node for index merge", K(filter), K(ret));
        adlog::DEBUG("invalid filter node for index merge", K(ret));
      } else {
        adlog::DEBUG("column_ids.at(0): ",column_ids.at(0));
        adlog::DEBUG("valid index num: ", K(valid_index_ids.count()));

        for (int64_t i = 0; OB_SUCC(ret) && i < valid_index_ids.count(); ++i) {
          const uint64_t index_id = valid_index_ids.at(i);
          adlog::DEBUG("index_id : ", index_id);
          const ObIArray<uint64_t> &index_column_ids = valid_index_cols.at(i);
          adlog::DEBUG("index_column_ids.count() : ", index_column_ids.count());  

          for (int64_t j = 0; j < index_column_ids.count(); ++j) {  
            adlog::DEBUG("index_column_id : ", index_column_ids.at(j));  
          }  
          if (OB_UNLIKELY(index_column_ids.empty())) {
            ret = OB_ERR_UNEXPECTED;
            LOG_WARN("invalid index column ids", K(ret), K(index_id), K(index_column_ids));
          } else if (index_column_ids.at(0) == column_ids.at(0)) {
            adlog::DEBUG("NO FTS NO IN : column_ids.at(0): ",column_ids.at(0));
            if (OB_FAIL(candicate_index_tids.push_back(index_id))) {
              LOG_WARN("failed to push back index id", K(index_id), K(ret));
            }
          }
        }
      }
    }
  }
  return ret;
}

int ObIndexMergeNode::formalize_index_merge_tree()
{
  int ret = OB_SUCCESS;
  adlog::DEBUG("ObIndexMergeNode::formalize_index_merge_tree start");

  if (is_merge_node()) {
    ObSEArray<ObIndexMergeNode*, 2> new_children;
    for (int64_t i = 0; OB_SUCC(ret) && i < children_.count(); ++i) {
      if (OB_ISNULL(children_.at(i))) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("get unexpected null child", K(ret), K(i), K(children_));
        adlog::DEBUG("get unexpected null child");
      } else if (OB_FAIL(SMART_CALL(children_.at(i)->formalize_index_merge_tree()))) {
        LOG_WARN("failed to formalize child node", K(ret), K(i), KPC(children_.at(i)));
      } else if (children_.at(i)->node_type_ == node_type_) {
        for (int64_t j = 0; OB_SUCC(ret) && j < children_.at(i)->children_.count(); ++j) {
          if (OB_FAIL(new_children.push_back(children_.at(i)->children_.at(j)))) {
            LOG_WARN("failed to push back new child", K(ret));
          }
        }
      } else if (OB_FAIL(new_children.push_back(children_.at(i)))) {
        LOG_WARN("failed to push back new child", K(ret));
      }
    }
    if (OB_SUCC(ret) && OB_FAIL(children_.assign(new_children))) {
      LOG_WARN("failed to assign new children", K(ret), K(new_children));
    }
  }
  adlog::DEBUG("ObIndexMergeNode::formalize_index_merge_tree end");

  return ret;
}

/**
 * @brief ObJoinOrder::check_candi_index_trees_match_hint
 *
 * 1. Prune candi index merge trees which can not match the union_merge hint.
 * 2. Choose the index id for each candi index merge node based on union_merge hint.
 */
int ObJoinOrder::check_candi_index_trees_match_hint(const uint64_t table_id,
                                                    ObIArray<ObIndexMergeNode*> &candi_index_trees)
{
  int ret = OB_SUCCESS;
  const LogTableHint *log_table_hint = NULL;
  ObSEArray<uint64_t, 4> index_ids;
  ObSEArray<ObIndexMergeNode*, 4> new_candi_trees;
  if (OB_ISNULL(get_plan())
      || OB_ISNULL(log_table_hint = get_plan()->get_log_plan_hint().get_log_table_hint(table_id))) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret), K(get_plan()), K(log_table_hint));
  } else if (OB_FAIL(index_ids.assign(log_table_hint->union_merge_list_))) {
    LOG_WARN("failed to assign union merge list", K(ret));
  }
  for (int64_t i = 0; OB_SUCC(ret) && i < candi_index_trees.count(); ++i) {
    bool is_match_hint = false;
    int64_t idx = 0;
    if (OB_FAIL(check_one_candi_node_match_hint(candi_index_trees.at(i), index_ids, idx, is_match_hint))) {
      LOG_WARN("failed to check candi tree match hint", K(ret), K(i), KPC(candi_index_trees.at(i)));
    } else if (!is_match_hint) {
      // do nothing
    } else if (OB_FAIL(new_candi_trees.push_back(candi_index_trees.at(i)))) {
      LOG_WARN("failed to push back new candi tree", K(ret));
    }
  }
  if (OB_SUCC(ret) && OB_FAIL(candi_index_trees.assign(new_candi_trees))) {
    LOG_WARN("failed to assign new candi trees", K(ret), K(new_candi_trees));
  }
  return ret;
}

int ObJoinOrder::check_one_candi_node_match_hint(ObIndexMergeNode *candi_node,
                                                 ObIArray<uint64_t> &index_ids,
                                                 int64_t &idx,
                                                 bool &is_match_hint)
{
  int ret = OB_SUCCESS;
  is_match_hint = false;
  if (OB_ISNULL(candi_node)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("candi node is null", K(ret), K(candi_node));
  } else if (idx >= index_ids.count()) {
    // do nothing
  } else {
    switch (candi_node->node_type_) {
      case INDEX_MERGE_UNION: {
        bool is_child_match_hint = true;
        for (int64_t i = 0; OB_SUCC(ret) && is_child_match_hint && i < candi_node->children_.count(); ++i) {
          if (OB_FAIL(SMART_CALL(check_one_candi_node_match_hint(candi_node->children_.at(i), index_ids, idx, is_child_match_hint)))) {
            LOG_WARN("failed to check candi tree match hint", K(ret), K(i), KPC(candi_node->children_.at(i)));
          }
        }
        is_match_hint = is_child_match_hint;
        break;
      }
      case INDEX_MERGE_INTERSECT: {
        for (int64_t i = 0; OB_SUCC(ret) && !is_match_hint && i < candi_node->children_.count(); ++i) {
          int64_t sub_idx = idx;
          if (OB_FAIL(SMART_CALL(check_one_candi_node_match_hint(candi_node->children_.at(i), index_ids, sub_idx, is_match_hint)))) {
            LOG_WARN("failed to check candi tree match hint", K(ret), K(i), KPC(candi_node->children_.at(i)));
          } else if (is_match_hint) {
            ObIndexMergeNode *tmp_node = candi_node->children_.at(i);
            candi_node->children_.reuse();
            idx = sub_idx;
            if (OB_FAIL(candi_node->children_.push_back(tmp_node))) {
              LOG_WARN("failed to push back child node", K(ret));
            }
          }
        }
        break;
      }
      case INDEX_MERGE_SCAN:
      case INDEX_MERGE_FTS_INDEX: {
        for (int64_t i = 0; OB_SUCC(ret) && !is_match_hint && i < candi_node->candicate_index_tids_.count(); ++i) {
          if (candi_node->candicate_index_tids_.at(i) == index_ids.at(idx)) {
            is_match_hint = true;
            candi_node->index_tid_ = index_ids.at(idx);
            ++idx;
          }
        }
        break;
      }
      default: {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("get unexpected candi node type", K(ret), KPC(candi_node));
      }
    }
  }
  return ret;
}

/**
 * @brief ObJoinOrder::prune_candi_index_merge_trees
 *
 * 1. Prune the candi index merge trees which contains only one index.
 * 2. If there is a candi tree contains FTS scan, prune all other candi trees
 *    which only contain normal index scan.
 */
int ObJoinOrder::prune_candi_index_merge_trees(ObIArray<ObIndexMergeNode*> &candi_index_trees)
{
  int ret = OB_SUCCESS;
  ObSEArray<ObIndexMergeNode*, 4> normal_candi_trees;
  ObSEArray<ObIndexMergeNode*, 4> candi_trees_contain_fts;
  if (OB_UNLIKELY(EN_FORCE_INDEX_MERGE_PLAN)) {
    // do nothing
  } else {
    // Prune the candcandi index merge trees which contains only one index.
    for (int64_t i = 0; OB_SUCC(ret) && i < candi_index_trees.count(); ++i) {
      bool is_valid = false;
      bool is_contain_fts = false;
      ObSEArray<ObIndexMergeNode*, 4> node_stack;
      uint64_t first_index_id = OB_INVALID_ID;
      if (OB_FAIL(node_stack.push_back(candi_index_trees.at(i)))) {
        LOG_WARN("failed to push back candi tree node", K(ret));
      }
      while(OB_SUCC(ret) && !node_stack.empty()) {
        ObIndexMergeNode *candi_node = node_stack.at(node_stack.count() - 1);
        node_stack.pop_back();
        if (OB_ISNULL(candi_node)) {
          ret = OB_ERR_UNEXPECTED;
          LOG_WARN("get unexpected null candi node", K(ret), K(i));
        } else if (candi_node->is_merge_node()) {
          for (int64_t j = 0; OB_SUCC(ret) && j < candi_node->children_.count(); ++j) {
            if (OB_FAIL(node_stack.push_back(candi_node->children_.at(j)))) {
              LOG_WARN("failed to push back candi index merge node", K(ret), K(j));
            }
          }
        } else if (INDEX_MERGE_FTS_INDEX == candi_node->node_type_) {
          is_valid = true;
          is_contain_fts = true;
          break;
        } else if (OB_UNLIKELY(candi_node->candicate_index_tids_.empty())) {
          ret = OB_ERR_UNEXPECTED;
          LOG_WARN("get unexpected null candi index id", K(ret), KPC(candi_node));
        } else if (OB_INVALID_ID == first_index_id) {
          first_index_id = candi_node->candicate_index_tids_.at(0);
        } else if (first_index_id != candi_node->candicate_index_tids_.at(0)) {
          is_valid = true;
        }
      }
      if (OB_FAIL(ret) || !is_valid) {
        // do nothing
      } else if (is_contain_fts) {
        if (OB_FAIL(candi_trees_contain_fts.push_back(candi_index_trees.at(i)))) {
          LOG_WARN("failed to push back candi tree", K(ret));
        }
      } else if (OB_FAIL(normal_candi_trees.push_back(candi_index_trees.at(i)))) {
        LOG_WARN("failed to push back new candi tree", K(ret));
      }
    }
    if (OB_FAIL(ret)) {
    } else if (!candi_trees_contain_fts.empty()) {
      if (OB_FAIL(candi_index_trees.assign(candi_trees_contain_fts))) {
        LOG_WARN("failed to assign new candi trees", K(ret), K(candi_trees_contain_fts));
      }
    } else if (OB_FAIL(candi_index_trees.assign(normal_candi_trees))) {
      LOG_WARN("failed to assign new candi trees", K(ret), K(normal_candi_trees));
    }
  }
  return ret;
}
//ADVISE
// 在这个函数里面，是对于 （已经构建好了AND树的and_root，去构建出来路径）
int ObJoinOrder::do_create_index_merge_paths(
    const uint64_t table_id, const uint64_t ref_table_id,
    const PathHelper &helper, ObIndexInfoCache &index_info_cache,
    ObIArray<ObIndexMergeNode *> &candi_index_trees,
    ObIArray<AccessPath *> &access_paths) {
  adlog::DEBUG("ObJoinOrder::do_create_index_merge_paths START");
  int ret = OB_SUCCESS;
  for (int64_t i = 0; OB_SUCC(ret) && i < candi_index_trees.count(); ++i) {
    ObIndexMergeNode* root_node = candi_index_trees.at(i);
    IndexMergePath* index_merge_path = NULL;
    ObSEArray<ObExprConstraint, 4> range_expr_constraint;
    int64_t scan_node_count = 0;
    if (OB_ISNULL(root_node)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("index merge node is null", K(ret), KPC(root_node));
    } else if (OB_FAIL(choose_best_selectivity_branch(root_node))) {
      LOG_WARN("failed to choose best selectivity branch", K(ret), KPC(root_node));
    } else if (OB_FAIL(root_node->formalize_index_merge_tree())) {
      LOG_WARN("failed to formalize index merge tree", K(ret), KPC(root_node));
    } else if (OB_FAIL(build_access_path_for_scan_node(table_id,
                                                       ref_table_id,
                                                       helper,
                                                       index_info_cache,
                                                       range_expr_constraint,
                                                       root_node,
                                                       scan_node_count))) {
      LOG_WARN("failed to build access path for scan node", K(ret));
    } else if (OB_FAIL(create_one_index_merge_path(table_id,
                                                   ref_table_id,
                                                   helper,
                                                   range_expr_constraint,
                                                   root_node,
                                                   index_merge_path))) {
      LOG_WARN("failed to create one index merge path", K(ret), KPC(root_node));
    } else if (OB_ISNULL(index_merge_path)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("index merge path is null", K(ret), KPC(root_node));
    } else if (OB_FALSE_IT(index_merge_path->index_cnt_ = scan_node_count)) {
    } else if (OB_FAIL(access_paths.push_back(static_cast<AccessPath*>(index_merge_path)))) {
      LOG_WARN("failed to push back index merge path", K(ret));
    } else {
      OPT_TRACE("generate one index merge path for", root_node->filter_);
      LOG_TRACE("generate one index merge path", KPC(index_merge_path));
    }
  }