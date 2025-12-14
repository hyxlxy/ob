[2025-12-15 00:38:18.917 DEBUG] create_iter_children_array
[2025-12-15 00:38:18.917 DEBUG] --------------ObDASLookupIter::next_rows begin----------
[2025-12-15 00:38:18.917 DEBUG] --------------ObDASLookupIter::next_rows INDEX_SCAN----------
[2025-12-15 00:38:18.917 DEBUG] --------------ObDASLookupIter::next_rows INDEX_SCAN get_next_rows----------
[2025-12-15 00:38:18.917 DEBUG] --------------ObDASLookupIter::next_rows end----------
[2025-12-15 00:38:19.003 DEBUG] ObExprRangeConverter::convert_in_expr start
[2025-12-15 00:38:19.003 DEBUG] l_expr->is_column_ref_expr()
[2025-12-15 00:38:19.003 DEBUG] r_expr->get_param_count(): 1
[2025-12-15 00:38:19.003 DEBUG] range_node->in_param_count_ 1
[2025-12-15 00:38:19.003 DEBUG] ObExprRangeConverter::convert_in_expr end
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::create_index_merge_access_paths START
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::get_candi_index_merge_trees START
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::get_valid_index_merge_indexes
[2025-12-15 00:38:19.003 DEBUG] 检验每一个候选索引是否都支持索引合并
[2025-12-15 00:38:19.003 DEBUG] 总共候选索引的个数有 0
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::generate_candi_index_merge_trees START
[2025-12-15 00:38:19.003 DEBUG] valid_index_ids 是空的
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::generate_candi_index_merge_trees END
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::get_valid_index_merge_indexes
[2025-12-15 00:38:19.003 DEBUG] 检验每一个候选索引是否都支持索引合并
[2025-12-15 00:38:19.003 DEBUG] 总共候选索引的个数有 7
[2025-12-15 00:38:19.003 DEBUG] 0 有初步满足条件
[2025-12-15 00:38:19.003 DEBUG] 0 有完整满足条件
[2025-12-15 00:38:19.003 DEBUG] 1 有初步满足条件
[2025-12-15 00:38:19.003 DEBUG] 1 有完整满足条件
[2025-12-15 00:38:19.003 DEBUG] 2 有初步满足条件
[2025-12-15 00:38:19.003 DEBUG] 2 有完整满足条件
[2025-12-15 00:38:19.003 DEBUG] 3 有初步满足条件
[2025-12-15 00:38:19.003 DEBUG] 3 有完整满足条件
[2025-12-15 00:38:19.003 DEBUG] 4 有初步满足条件
[2025-12-15 00:38:19.003 DEBUG] 4 有完整满足条件
[2025-12-15 00:38:19.003 DEBUG] 5 有初步满足条件
[2025-12-15 00:38:19.003 DEBUG] 5 有完整满足条件
[2025-12-15 00:38:19.003 DEBUG] 6 有初步满足条件
[2025-12-15 00:38:19.003 DEBUG] 6 有完整满足条件
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::generate_candi_index_merge_trees START
[2025-12-15 00:38:19.003 DEBUG] filters.count() = 3
[2025-12-15 00:38:19.003 DEBUG] column_ids.at(0): 16
[2025-12-15 00:38:19.003 DEBUG] valid index num: valid_index_ids.count()7
[2025-12-15 00:38:19.003 DEBUG] index_id : 500059
[2025-12-15 00:38:19.003 DEBUG] index_column_ids.count() : 1
[2025-12-15 00:38:19.003 DEBUG] index_column_id : 16
[2025-12-15 00:38:19.003 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-15 00:38:19.003 DEBUG] index_id : 500062
[2025-12-15 00:38:19.003 DEBUG] index_column_ids.count() : 2
[2025-12-15 00:38:19.003 DEBUG] index_column_id : 17
[2025-12-15 00:38:19.003 DEBUG] index_column_id : 16
[2025-12-15 00:38:19.003 DEBUG] index_id : 500063
[2025-12-15 00:38:19.003 DEBUG] index_column_ids.count() : 2
[2025-12-15 00:38:19.003 DEBUG] index_column_id : 18
[2025-12-15 00:38:19.003 DEBUG] index_column_id : 16
[2025-12-15 00:38:19.003 DEBUG] index_id : 500064
[2025-12-15 00:38:19.003 DEBUG] index_column_ids.count() : 1
[2025-12-15 00:38:19.003 DEBUG] index_column_id : 16
[2025-12-15 00:38:19.003 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-15 00:38:19.003 DEBUG] index_id : 500065
[2025-12-15 00:38:19.003 DEBUG] index_column_ids.count() : 1
[2025-12-15 00:38:19.003 DEBUG] index_column_id : 20
[2025-12-15 00:38:19.003 DEBUG] index_id : 500066
[2025-12-15 00:38:19.003 DEBUG] index_column_ids.count() : 2
[2025-12-15 00:38:19.003 DEBUG] index_column_id : 21
[2025-12-15 00:38:19.003 DEBUG] index_column_id : 20
[2025-12-15 00:38:19.003 DEBUG] index_id : 500067
[2025-12-15 00:38:19.003 DEBUG] index_column_ids.count() : 2
[2025-12-15 00:38:19.003 DEBUG] index_column_id : 20
[2025-12-15 00:38:19.003 DEBUG] index_column_id : 21
[2025-12-15 00:38:19.003 DEBUG] and_root->node_type_ 2
[2025-12-15 00:38:19.003 DEBUG] and_root->children_ count() 3
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:38:19.003 DEBUG] Added AND merge tree for 3 conditions
[2025-12-15 00:38:19.003 DEBUG] and_root->node_type_ 2
[2025-12-15 00:38:19.003 DEBUG] and_root->children_ count() 3
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::generate_candi_index_merge_trees END
[2025-12-15 00:38:19.003 DEBUG] generate candi index merge trees, count() 1



[2025-12-15 00:38:19.003 DEBUG] first node type : 2
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::get_candi_index_merge_trees END
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::do_create_index_merge_paths START
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::choose_best_selectivity_branch START
[2025-12-15 00:38:19.003 DEBUG] choose_best_selectivity_branch:: INDEX_MERGE_INTERSECT == candi_node->node_type_
[2025-12-15 00:38:19.003 DEBUG] choose_best_selectivity_branch::  candi_node->children_.count() 3
[2025-12-15 00:38:19.003 DEBUG] choose_best_selectivity_branch::  has_scan_children 1
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::choose_best_selectivity_branch START
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::choose_best_selectivity_branch END
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::choose_best_selectivity_branch START
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::choose_best_selectivity_branch END
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::choose_best_selectivity_branch START
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::choose_best_selectivity_branch END
[2025-12-15 00:38:19.003 DEBUG] ObJoinOrder::choose_best_selectivity_branch END
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:38:19.003 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:38:19.004 DEBUG] ObExprRangeConverter::convert_in_expr start
[2025-12-15 00:38:19.004 DEBUG] l_expr->is_column_ref_expr()
[2025-12-15 00:38:19.004 DEBUG] r_expr->get_param_count(): 3
[2025-12-15 00:38:19.004 DEBUG] range_node->in_param_count_ 3
[2025-12-15 00:38:19.004 DEBUG] ObExprRangeConverter::convert_in_expr end
[2025-12-15 00:38:19.004 DEBUG] ObJoinOrder::do_create_index_merge_paths END
[2025-12-15 00:38:19.004 DEBUG] ObJoinOrder::check_index_merge_paths_contain_fts start
[2025-12-15 00:38:19.004 DEBUG] root_node->children_.count() 3
[2025-12-15 00:38:19.004 DEBUG] contain_fts: 1
[2025-12-15 00:38:19.004 DEBUG] ObJoinOrder::check_index_merge_paths_contain_fts end
[2025-12-15 00:38:19.004 DEBUG] ObJoinOrder::create_index_merge_access_paths END



[2025-12-15 00:38:19.004 DEBUG] there is index merge path
[2025-12-15 00:38:19.004 DEBUG] set index merge path cost = 0 ?
[2025-12-15 00:38:19.004 DEBUG] ap->is_index_merge_path
[2025-12-15 00:38:19.004 DEBUG] if (op.use_index_merge()) {
[2025-12-15 00:38:19.004 DEBUG] OB_UNLIKELY(op.use_index_merge() || op.has_es_match()))
[2025-12-15 00:38:19.004 DEBUG] AHAHAHA index_merge
[2025-12-15 00:38:19.004 DEBUG] if (OB_FAIL(generate_index_merge_ctdef(op, tsc_ctdef, index_merge_ctdef)))
[2025-12-15 00:38:19.004 DEBUG] use index merge
[2025-12-15 00:38:19.004 DEBUG] use index merge
[2025-12-15 00:38:19.004 DEBUG] use index merge
[2025-12-15 00:38:19.004 DEBUG] use index merge 22222
[2025-12-15 00:38:19.005 DEBUG] if (OB_SUCC(ret) && index_merge_cnt > 0)
[2025-12-15 00:38:19.005 DEBUG] create_index_merge_iter_tree
[2025-12-15 00:38:19.005 DEBUG] create_index_merge_sub_tree
[2025-12-15 00:38:19.005 DEBUG] ObDASIterUtils::create_text_retrieval_sub_tree
[2025-12-15 00:38:19.005 DEBUG] create_iter_children_array
[2025-12-15 00:38:19.005 DEBUG] ObDASIterUtils::create_cache_lookup_sub_tree
[2025-12-15 00:38:19.005 DEBUG] create_iter_children_array
[2025-12-15 00:38:19.005 DEBUG] (merge_ctdef->merge_node_types_.at(i) == INDEX_MERGE_FTS_INDEX && need_sort)
[2025-12-15 00:38:19.005 DEBUG] init sort iter and child_ctdef->op_type_ == DAS_OP_SORT
[2025-12-15 00:38:19.005 DEBUG] ObDASIterUtils::create_sort_sub_tree
[2025-12-15 00:38:19.005 DEBUG] create_iter_children_array
[2025-12-15 00:38:19.005 DEBUG] index merge iter use!!!
[2025-12-15 00:38:19.005 DEBUG] check_mixed_query_with_fts false
[2025-12-15 00:38:19.005 DEBUG] create_iter_children_array
[2025-12-15 00:38:19.005 DEBUG] ObDASIterUtils::create_cache_lookup_sub_tree
[2025-12-15 00:38:19.005 DEBUG] create_iter_children_array
[2025-12-15 00:38:19.005 DEBUG] ObTextDaaTIter::init
[2025-12-15 00:38:19.005 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:38:19.005 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:38:19.005 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:38:19.005 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:38:19.005 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:38:19.005 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:38:19.005 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:38:19.005 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:38:19.005 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:38:19.005 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:38:19.005 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:38:19.005 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:38:19.005 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:38:19.005 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:38:19.005 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:38:19.006 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:38:19.006 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:38:19.006 DEBUG] ctdef->pd_expr_spec_.pushdown_filters_.empty()
[2025-12-15 00:38:19.006 DEBUG] do_index_lookup::scan_param op_filters_ nonono
[2025-12-15 00:38:19.006 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:38:19.006 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:38:19.006 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:38:19.006 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:38:19.006 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:38:19.006 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:38:19.007 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:38:19.007 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:38:19.007 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:38:19.007 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:38:19.007 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:38:19.007 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:38:19.007 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:38:19.007 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:38:19.008 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:38:19.008 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:38:19.008 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:38:19.008 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:38:19.008 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:38:19.008 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:38:19.008 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:38:19.008 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:38:19.008 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:38:19.008 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:38:19.008 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:38:19.008 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:38:19.008 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:38:19.008 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:38:19.008 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:38:19.008 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:38:19.008 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:38:19.008 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:38:19.008 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:38:19.008 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:38:19.008 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:38:19.009 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:38:19.009 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:38:19.009 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:38:19.009 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:38:19.009 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:38:19.009 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:38:19.009 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:38:19.009 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:38:19.009 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:38:19.009 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:38:19.009 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:38:19.009 DEBUG] --------------FINISHED----------
[2025-12-15 00:38:19.009 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:38:19.009 DEBUG] i: 2child_iter->get_type() == DAS_ITER_SORT
[2025-12-15 00:38:19.010 DEBUG] i: 2child_iter->get_type() == DAS_ITER_SORT
[2025-12-15 00:38:19.010 DEBUG] i: 2child_iter->get_type() == DAS_ITER_SORT
[2025-12-15 00:38:19.010 DEBUG] i: 2child_iter->get_type() == DAS_ITER_SORT
[2025-12-15 00:38:19.010 DEBUG] i: 2child_iter->get_type() == DAS_ITER_SORT
[2025-12-15 00:38:19.010 DEBUG] i: 2child_iter->get_type() == DAS_ITER_SORT
[2025-12-15 00:38:19.010 DEBUG] i: 2child_iter->get_type() == DAS_ITER_SORT
[2025-12-15 00:38:19.012 DEBUG] intersect_get_next_rows : OB_FAIL(ret)
[2025-12-15 00:38:19.012 DEBUG] intersect_get_next_rows :count64
[2025-12-15 00:38:19.012 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:38:19.012 DEBUG] intersect_get_next_rows : OB_FAIL(ret)
[2025-12-15 00:38:19.012 DEBUG] intersect_get_next_rows :count0
[2025-12-15 00:38:19.012 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:38:19.012 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:38:19.012 DEBUG] ctdef->pd_expr_spec_.pushdown_filters_.empty()
[2025-12-15 00:38:19.012 DEBUG] do_index_lookup::scan_param op_filters_ nonono
[2025-12-15 00:38:19.012 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:38:19.012 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:38:19.012 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:38:19.012 DEBUG] MY_SEPC filter size: 3 count: 64 need_final_limit: 0
[2025-12-15 00:38:19.012 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:38:19.012 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:38:19.012 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:38:19.012 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:38:19.012 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:38:19.012 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:38:19.012 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:38:19.012 DEBUG] intersect_get_next_rows : OB_FAIL(ret)
[2025-12-15 00:38:19.012 DEBUG] intersect_get_next_rows :count0
[2025-12-15 00:38:19.012 DEBUG] --------------FINISHED----------
[2025-12-15 00:38:19.012 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end




[2025-12-15 01:24:29.421 DEBUG] INTERSECT rowkey_exprs_ countrowkey_exprs_->count()1
[2025-12-15 01:24:29.421 DEBUG] rowkey_expr0{type:"T_REF_COLUMN", datum_meta:{type:"BIGINT", cs_type:"binary", scale:0, precision:20}, obj_meta:{type:"BIGINT", collation:"binary", coercibility:"NUMERIC"}, obj_datum_map:3, flag:67, eval_func:null, eval_batch_func:null, inner_functions:null, inner_func_cnt:0, arg_cnt:0, parent_cnt:1, frame_idx:2, datum_off:6221, res_buf_off:153827, dyn_buf_header_offset:94383, res_buf_len:8, eval_flags_off:89631, pvt_skip_off:9293, expr_ctx_id:4294967295, extra:0, batch_idx_mask:18446744073709551615, vector_header_off:87056, eval_vector_func:null, local_session_var_id:-1, this:0x7fe71b6da5c0}


[2025-12-15 01:24:56.644 DEBUG] UNION rowkey_exprs_ countrowkey_exprs_->count()1
[2025-12-15 01:24:56.644 DEBUG] UNION rowkey_expr0{type:"T_REF_COLUMN", datum_meta:{type:"BIGINT", cs_type:"binary", scale:0, precision:20}, obj_meta:{type:"BIGINT", collation:"binary", coercibility:"NUMERIC"}, obj_datum_map:3, flag:67, eval_func:null, eval_batch_func:null, inner_functions:null, inner_func_cnt:0, arg_cnt:0, parent_cnt:1, frame_idx:2, datum_off:6221, res_buf_off:157064, dyn_buf_header_offset:97620, res_buf_len:8, eval_flags_off:92836, pvt_skip_off:9293, expr_ctx_id:4294967295, extra:0, batch_idx_mask:18446744073709551615, vector_header_off:90160, eval_vector_func:null, local_session_var_id:-1, this:0x7fe71b7b65c0}
