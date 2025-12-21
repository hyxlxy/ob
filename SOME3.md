obclient [test]>   SELECT id,base_id ,  docid_col, MATCH(fulltext_col) AGAINST('text') as _score FROM test_insert_en WHERE     id<1000 and base_id in ('base_id_1','base_id_2', 'base_id_3') and MATCH(fulltext_col) AGAINST('text') and id < 1000 ORDER BY _score DESC limit 10;
+-----+-----------+--------------+--------------------+
| id  | base_id   | docid_col    | _score             |
+-----+-----------+--------------+--------------------+
|  17 | base_id_1 | doc-en-14104 | 3.6712478278194753 |
| 809 | base_id_2 | doc-en-16325 |  3.662114094665175 |
| 135 | base_id_1 | doc-en-9908  | 3.6457901932021235 |
| 320 | base_id_1 | doc-en-7032  | 3.6247728427465273 |
| 471 | base_id_1 | doc-en-5742  | 3.5770686335447186 |
| 841 | base_id_2 | doc-en-4360  |   3.53673317394405 |
| 232 | base_id_1 | doc-en-4910  | 3.4015741122762164 |
| 382 | base_id_1 | doc-en-14867 |  3.333069482076175 |
| 576 | base_id_2 | doc-en-1021  | 3.3232487456484945 |
| 509 | base_id_2 | doc-en-16007 |  3.320338849298478 |
+-----+-----------+--------------+--------------------+


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
  
  // Initialize two-phase FTS optimization if applicable (best-effort, do not block plan)
  if (OB_SUCC(ret)) {
    int tmp_ret = init_two_phase_fts_opt(table_id, stmt, root_node, index_merge_path);
    if (OB_SUCCESS != tmp_ret) {
      LOG_TRACE("skip two-phase fts opt init", K(tmp_ret));
    }
  }
  
  return ret;
}

int ObJoinOrder::init_two_phase_fts_opt(const uint64_t table_id,
                                        const ObDMLStmt *stmt,
                                        ObIndexMergeNode* root_node,
                                        IndexMergePath* index_merge_path)
{
  int ret = OB_SUCCESS;
  bool has_fts_index = false;
  bool has_scalar_filter = false;
  int64_t limit_count = -1;
  
  if (OB_ISNULL(stmt) || OB_ISNULL(root_node) || OB_ISNULL(index_merge_path)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("get unexpected null", K(ret), KP(stmt), KP(root_node), KP(index_merge_path));
  } else {
    // Check if index merge contains FTS index
    ObSEArray<ObIndexMergeNode*, 8> node_stack;
    if (OB_FAIL(node_stack.push_back(root_node))) {
      LOG_WARN("failed to push back root node", K(ret));
    }
    
    while (OB_SUCC(ret) && !node_stack.empty() && !has_fts_index) {
      ObIndexMergeNode *node = node_stack.at(node_stack.count() - 1);
      node_stack.pop_back();
      
      if (OB_ISNULL(node)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("get unexpected null node", K(ret));
      } else if (node->is_merge_node()) {
        for (int64_t i = 0; OB_SUCC(ret) && i < node->children_.count(); ++i) {
          if (OB_FAIL(node_stack.push_back(node->children_.at(i)))) {
            LOG_WARN("failed to push back child node", K(ret));
          }
        }
      } else if (INDEX_MERGE_FTS_INDEX == node->node_type_) {
        has_fts_index = true;
      }
    }
    
    // Check if there are scalar filters (non-FTS filters)
    if (OB_SUCC(ret) && has_fts_index) {
      // Count scalar filters (filters that are not MATCH expressions)
      for (int64_t i = 0; i < index_merge_path->filter_.count(); ++i) {
        ObRawExpr *filter = index_merge_path->filter_.at(i);
        if (OB_NOT_NULL(filter) && !filter->has_flag(CNT_MATCH_EXPR)) {
          has_scalar_filter = true;
          break;
        }
      }
    }
    
    // Check if query has LIMIT clause
    if (OB_SUCC(ret) && has_fts_index && has_scalar_filter && (NULL != stmt->get_limit_expr())) {
      // Extract LIMIT value using ObTransformUtils
      ObOptimizerContext *opt_ctx = NULL;
      const ParamStore *param_store = NULL;
      ObExecContext *exec_ctx = NULL;
      int64_t limit_value = -1;
      bool is_null_value = false;
      if (OB_ISNULL(get_plan()) || OB_ISNULL(opt_ctx = &get_plan()->get_optimizer_context()) ||
          OB_ISNULL(param_store = opt_ctx->get_params()) || OB_ISNULL(exec_ctx = opt_ctx->get_exec_ctx())) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected null optimizer context", K(ret), KP(get_plan()), KP(opt_ctx), KP(param_store), KP(exec_ctx));
      } else if (OB_FAIL(ObTransformUtils::get_limit_value(const_cast<ObRawExpr *>(stmt->get_limit_expr()),
                                                           param_store,
                                                           exec_ctx,
                                                           allocator_,
                                                           limit_value,
                                                           is_null_value))) {
        LOG_WARN("failed to get limit value", K(ret));
      } else if (!is_null_value && limit_value > 0 && limit_value <= 100) { // Only optimize small LIMIT
        limit_count = limit_value;
        index_merge_path->enable_two_phase_fts_opt_ = true;
        index_merge_path->fts_limit_count_ = limit_count;
        // Amplify factor: default 3x
        index_merge_path->fts_amplify_factor_ = 3;
        LOG_TRACE("Enable two-phase FTS optimization", K(table_id), K(limit_count), K(index_merge_path->fts_amplify_factor_));
      }
    }
  }
  
  return ret;
}

```