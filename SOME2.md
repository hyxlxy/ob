```c++

int ObJoinOrder::create_one_index_merge_path(const uint64_t table_id,
                                             const uint64_t ref_table_id,
                                             const PathHelper &helper,
                                             const ObIArray<ObExprConstraint> &range_expr_constraint,
                                             ObIndexMergeNode* root_node,
                                             IndexMergePath* &index_merge_path)
{
  int ret = OB_SUCCESS;
  const ObDMLStmt *stmt = NULL;
  const TableItem *table_item = NULL;
  ObSEArray<ObRawExpr*, 4> all_match_exprs;
  ObSEArray<ObRawExpr*, 4> rowkey_exprs;
  if (OB_ISNULL(root_node)|| OB_ISNULL(allocator_)
      || OB_ISNULL(get_plan()) || OB_ISNULL(stmt = get_plan()->get_stmt())
      || OB_ISNULL(table_item = stmt->get_table_item_by_id(table_id))) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret), K(root_node), K(allocator_), K(get_plan()), K(stmt), K(table_item));
  } else if (OB_ISNULL(index_merge_path = reinterpret_cast<IndexMergePath*>(allocator_->alloc(sizeof(IndexMergePath))))) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    LOG_WARN("failed to new a index merge path", K(ret));
  } else if (OB_FALSE_IT(index_merge_path = new(index_merge_path) IndexMergePath(table_id, ref_table_id, this))) {
  } else if (OB_FALSE_IT(index_merge_path->root_ = root_node)) {
  } else if (OB_FAIL(stmt->get_match_expr_on_table(table_id, all_match_exprs))) {
    LOG_WARN("failed to get all match exprs", K(ret), K(table_id));
  } else if (OB_FAIL(index_merge_path->est_cost_info_.table_filters_.assign(helper.filters_))) {
    LOG_WARN("failed to assign filters", K(ret));
  } else if (OB_FAIL(index_merge_path->filter_.assign(helper.filters_))) {
    LOG_WARN("failed to assign filters", K(ret));
  } else if (OB_FAIL(index_merge_path->subquery_exprs_.assign(helper.subquery_exprs_))) {
    LOG_WARN("failed to assign subquery exprs", K(ret));
  } else if (OB_FAIL(get_plan()->get_rowkey_exprs(table_id, ref_table_id, rowkey_exprs))) {
    LOG_WARN("failed to get rowkey exprs", K(ret));
  } else if (OB_FAIL(get_index_scan_direction(rowkey_exprs, stmt, get_plan()->get_equal_sets(), index_merge_path->order_direction_))) {
    LOG_WARN("failed to get index scan direction", K(ret));
  } else if (OB_FAIL(root_node->set_scan_direction(index_merge_path->order_direction_))) {
    LOG_WARN("failed to set scan direction for index merge path", K(ret), KPC(index_merge_path));
  } else if (OB_FAIL(ObOptimizerUtil::make_sort_keys(rowkey_exprs,
                                                     index_merge_path->order_direction_,
                                                     index_merge_path->ordering_))) {
    LOG_WARN("failed to make rowkey ordering for index merge path", K(ret));
  } else if (OB_FAIL(stmt->get_column_items(table_id, index_merge_path->est_cost_info_.access_column_items_))) {
    LOG_WARN("failed to get column items", K(ret));
  } else if (OB_FAIL(index_merge_path->equal_param_constraints_.assign(helper.equal_param_constraints_))) {
    LOG_WARN("failed to assign equal param constraints", K(ret));
  } else if (OB_FAIL(index_merge_path->const_param_constraints_.assign(helper.const_param_constraints_))) {
    LOG_WARN("failed to assign equal param constraints", K(ret));
  } else if (OB_FAIL(index_merge_path->expr_constraints_.assign(helper.expr_constraints_))) {
    LOG_WARN("failed to assign expr constraints", K(ret));
  } else if (OB_FAIL(append(index_merge_path->expr_constraints_, range_expr_constraint))) {
    LOG_WARN("failed to append range expr constraints", K(ret));
  } else {
    index_merge_path->table_id_ = table_id;
    index_merge_path->ref_table_id_ = ref_table_id;
    index_merge_path->index_id_ = ref_table_id;
    index_merge_path->use_das_ = true;
    index_merge_path->use_skip_scan_ = OptSkipScanState::SS_DISABLE;
    index_merge_path->table_opt_info_ = helper.table_opt_info_;
    index_merge_path->is_inner_path_ = helper.is_inner_path_;
    index_merge_path->for_update_ = table_item->for_update_;
    index_merge_path->use_column_store_ = false;
    index_merge_path->contain_das_op_ = index_merge_path->use_das_;
    index_merge_path->is_ordered_by_pk_ = true;
    index_merge_path->can_batch_rescan_ = false;
    index_merge_path->parent_ = this;
    index_merge_path->est_cost_info_.index_meta_info_.is_index_back_ = true;
    index_merge_path->est_cost_info_.is_virtual_table_ = is_virtual_table(ref_table_id);
    index_merge_path->est_cost_info_.table_metas_ = &get_plan()->get_basic_table_metas();
    index_merge_path->est_cost_info_.sel_ctx_ = &get_plan()->get_selectivity_ctx();
    index_merge_path->est_cost_info_.rescan_server_list_ = &index_merge_path->server_list_;
    index_merge_path->est_cost_info_.is_rescan_ = helper.is_inner_path_ || get_plan()->get_is_rescan_subplan();
    index_merge_path->est_cost_info_.use_column_store_ = index_merge_path->use_column_store_;
    index_merge_path->domain_idx_info_.set_domain_idx_type(DomainIndexType::FTS_INDEX);
    if (NULL == table_partition_info_) {
      // generate table location for main table
      if (OB_FAIL(compute_table_location(table_id,
                                         ref_table_id,
                                         false,
                                         index_merge_path->table_partition_info_))) {
        LOG_WARN("failed to compute table location", K(ret), K(table_id), K(ref_table_id));
      }
    } else {
      index_merge_path->table_partition_info_ = table_partition_info_;
    }
  }

  // prepare func lookup tr_infos for index merge path, we only need match exprs which is not in index.
  ObSEArray<ObRawExpr*, 4> match_exprs_in_index;
  for (int64_t i = 0; OB_SUCC(ret) && i < root_node->filter_.count(); ++i) {
    ObRawExpr *filter = root_node->filter_.at(i);
    if (OB_ISNULL(filter)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("get unexpected null filter", K(ret), K(i), KPC(root_node));
    } else if (filter->get_expr_type() == T_OP_BOOL && filter->has_flag(CNT_MATCH_EXPR)) {
      ObRawExpr *param_expr = NULL;
      if (OB_UNLIKELY(0 >= filter->get_param_count()) || OB_ISNULL(param_expr = filter->get_param_expr(0))) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected null param expr for bool op", K(ret), KPC(filter));
      } else if (!param_expr->has_flag(IS_MATCH_EXPR)) {
        // do nothing
      } else if (OB_FAIL(match_exprs_in_index.push_back(param_expr))) {
        LOG_WARN("failed to append match expr to array", K(ret));
      }
    }
  }
  for (int64_t i = 0; OB_SUCC(ret) && i < all_match_exprs.count(); ++i) {
    ObRawExpr *cur_expr = all_match_exprs.at(i);
    const MatchExprInfo *match_expr_info = NULL;
    if (OB_ISNULL(cur_expr)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected null match expr", K(ret), K(i));
    } else if (ObOptimizerUtil::find_item(match_exprs_in_index, cur_expr)) {
      // do nothing
    } else if (OB_FAIL(find_match_expr_info(helper.match_expr_infos_, cur_expr, match_expr_info))) {
      LOG_WARN("failed to find match expr info", K(ret), KPC(cur_expr));
    } else if (OB_ISNULL(match_expr_info)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected null match expr info", K(ret));
    } else if (OB_FAIL(index_merge_path->domain_idx_info_.func_lookup_exprs_.push_back(cur_expr))) {
      LOG_WARN("failed to append func lookup exprs", K(ret), KPC(cur_expr));
    } else if (OB_FAIL(index_merge_path->domain_idx_info_.func_lookup_index_ids_.push_back(match_expr_info->inv_idx_id_))) {
      LOG_WARN("failed to append func lookup index id", K(ret));
    }
  }
  return ret;
}
```