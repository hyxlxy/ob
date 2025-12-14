[2025-12-15 00:37:18.986 DEBUG] create_iter_children_array
[2025-12-15 00:37:18.986 DEBUG] --------------ObDASLookupIter::next_rows begin----------
[2025-12-15 00:37:18.986 DEBUG] --------------ObDASLookupIter::next_rows INDEX_SCAN----------
[2025-12-15 00:37:18.986 DEBUG] --------------ObDASLookupIter::next_rows INDEX_SCAN get_next_rows----------
[2025-12-15 00:37:18.986 DEBUG] --------------ObDASLookupIter::next_rows end----------
[2025-12-15 00:37:19.563 DEBUG] ObExprRangeConverter::convert_in_expr start
[2025-12-15 00:37:19.563 DEBUG] l_expr->is_column_ref_expr()
[2025-12-15 00:37:19.563 DEBUG] r_expr->get_param_count(): 1
[2025-12-15 00:37:19.563 DEBUG] range_node->in_param_count_ 1
[2025-12-15 00:37:19.563 DEBUG] ObExprRangeConverter::convert_in_expr end
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::create_index_merge_access_paths START
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::get_candi_index_merge_trees START
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::get_valid_index_merge_indexes
[2025-12-15 00:37:19.563 DEBUG] 检验每一个候选索引是否都支持索引合并
[2025-12-15 00:37:19.563 DEBUG] 总共候选索引的个数有 0
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::generate_candi_index_merge_trees START
[2025-12-15 00:37:19.563 DEBUG] valid_index_ids 是空的
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::generate_candi_index_merge_trees END
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::get_valid_index_merge_indexes
[2025-12-15 00:37:19.563 DEBUG] 检验每一个候选索引是否都支持索引合并
[2025-12-15 00:37:19.563 DEBUG] 总共候选索引的个数有 7
[2025-12-15 00:37:19.563 DEBUG] 0 有初步满足条件
[2025-12-15 00:37:19.563 DEBUG] 0 有完整满足条件
[2025-12-15 00:37:19.563 DEBUG] 1 有初步满足条件
[2025-12-15 00:37:19.563 DEBUG] 1 有完整满足条件
[2025-12-15 00:37:19.563 DEBUG] 2 有初步满足条件
[2025-12-15 00:37:19.563 DEBUG] 2 有完整满足条件
[2025-12-15 00:37:19.563 DEBUG] 3 有初步满足条件
[2025-12-15 00:37:19.563 DEBUG] 3 有完整满足条件
[2025-12-15 00:37:19.563 DEBUG] 4 有初步满足条件
[2025-12-15 00:37:19.563 DEBUG] 4 有完整满足条件
[2025-12-15 00:37:19.563 DEBUG] 5 有初步满足条件
[2025-12-15 00:37:19.563 DEBUG] 5 有完整满足条件
[2025-12-15 00:37:19.563 DEBUG] 6 有初步满足条件
[2025-12-15 00:37:19.563 DEBUG] 6 有完整满足条件
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::generate_candi_index_merge_trees START
[2025-12-15 00:37:19.563 DEBUG] filters.count() = 1
[2025-12-15 00:37:19.563 DEBUG] column_ids.at(0): 16
[2025-12-15 00:37:19.563 DEBUG] valid index num: valid_index_ids.count()7
[2025-12-15 00:37:19.563 DEBUG] index_id : 500059
[2025-12-15 00:37:19.563 DEBUG] index_column_ids.count() : 1
[2025-12-15 00:37:19.563 DEBUG] index_column_id : 16
[2025-12-15 00:37:19.563 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-15 00:37:19.563 DEBUG] index_id : 500062
[2025-12-15 00:37:19.563 DEBUG] index_column_ids.count() : 2
[2025-12-15 00:37:19.563 DEBUG] index_column_id : 17
[2025-12-15 00:37:19.563 DEBUG] index_column_id : 16
[2025-12-15 00:37:19.563 DEBUG] index_id : 500063
[2025-12-15 00:37:19.563 DEBUG] index_column_ids.count() : 2
[2025-12-15 00:37:19.563 DEBUG] index_column_id : 18
[2025-12-15 00:37:19.563 DEBUG] index_column_id : 16
[2025-12-15 00:37:19.563 DEBUG] index_id : 500064
[2025-12-15 00:37:19.563 DEBUG] index_column_ids.count() : 1
[2025-12-15 00:37:19.563 DEBUG] index_column_id : 16
[2025-12-15 00:37:19.563 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-15 00:37:19.563 DEBUG] index_id : 500065
[2025-12-15 00:37:19.563 DEBUG] index_column_ids.count() : 1
[2025-12-15 00:37:19.563 DEBUG] index_column_id : 20
[2025-12-15 00:37:19.563 DEBUG] index_id : 500066
[2025-12-15 00:37:19.563 DEBUG] index_column_ids.count() : 2
[2025-12-15 00:37:19.563 DEBUG] index_column_id : 21
[2025-12-15 00:37:19.563 DEBUG] index_column_id : 20
[2025-12-15 00:37:19.563 DEBUG] index_id : 500067
[2025-12-15 00:37:19.563 DEBUG] index_column_ids.count() : 2
[2025-12-15 00:37:19.563 DEBUG] index_column_id : 20
[2025-12-15 00:37:19.563 DEBUG] index_column_id : 21
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:37:19.563 DEBUG] candi_index_trees push_back了一个node
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::generate_candi_index_merge_trees END
[2025-12-15 00:37:19.563 DEBUG] generate candi index merge trees, count() 1



[2025-12-15 00:37:19.563 DEBUG] first node type : 1
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::get_candi_index_merge_trees END
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::do_create_index_merge_paths START
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::choose_best_selectivity_branch START
[2025-12-15 00:37:19.563 DEBUG] choose_best_selectivity_branch:: INDEX_MERGE_UNION == candi_node->node_type_
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::choose_best_selectivity_branch START
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::choose_best_selectivity_branch END
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::choose_best_selectivity_branch START
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::choose_best_selectivity_branch END
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::choose_best_selectivity_branch START
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::choose_best_selectivity_branch END
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::choose_best_selectivity_branch END
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:37:19.563 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-15 00:37:19.563 DEBUG] ObExprRangeConverter::convert_in_expr start
[2025-12-15 00:37:19.563 DEBUG] l_expr->is_column_ref_expr()
[2025-12-15 00:37:19.563 DEBUG] r_expr->get_param_count(): 3
[2025-12-15 00:37:19.563 DEBUG] range_node->in_param_count_ 3
[2025-12-15 00:37:19.563 DEBUG] ObExprRangeConverter::convert_in_expr end
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::do_create_index_merge_paths END
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::check_index_merge_paths_contain_fts start
[2025-12-15 00:37:19.563 DEBUG] root_node->children_.count() 3
[2025-12-15 00:37:19.563 DEBUG] contain_fts: 1
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::check_index_merge_paths_contain_fts end
[2025-12-15 00:37:19.563 DEBUG] ObJoinOrder::create_index_merge_access_paths END



[2025-12-15 00:37:19.563 DEBUG] there is index merge path
[2025-12-15 00:37:19.564 DEBUG] set index merge path cost = 0 ?
[2025-12-15 00:37:19.564 DEBUG] ap->is_index_merge_path
[2025-12-15 00:37:19.564 DEBUG] if (op.use_index_merge()) {
[2025-12-15 00:37:19.564 DEBUG] OB_UNLIKELY(op.use_index_merge() || op.has_es_match()))
[2025-12-15 00:37:19.564 DEBUG] AHAHAHA index_merge
[2025-12-15 00:37:19.564 DEBUG] if (OB_FAIL(generate_index_merge_ctdef(op, tsc_ctdef, index_merge_ctdef)))
[2025-12-15 00:37:19.564 DEBUG] use index merge
[2025-12-15 00:37:19.564 DEBUG] use index merge
[2025-12-15 00:37:19.564 DEBUG] use index merge
[2025-12-15 00:37:19.564 DEBUG] use index merge 22222
[2025-12-15 00:37:19.564 DEBUG] if (OB_SUCC(ret) && index_merge_cnt > 0)
[2025-12-15 00:37:19.564 DEBUG] create_index_merge_iter_tree
[2025-12-15 00:37:19.564 DEBUG] create_index_merge_sub_tree
[2025-12-15 00:37:19.564 DEBUG] ObDASIterUtils::create_text_retrieval_sub_tree
[2025-12-15 00:37:19.564 DEBUG] create_iter_children_array
[2025-12-15 00:37:19.564 DEBUG] ObDASIterUtils::create_cache_lookup_sub_tree
[2025-12-15 00:37:19.564 DEBUG] create_iter_children_array
[2025-12-15 00:37:19.564 DEBUG] (merge_ctdef->merge_node_types_.at(i) == INDEX_MERGE_FTS_INDEX && need_sort)
[2025-12-15 00:37:19.564 DEBUG] init sort iter and child_ctdef->op_type_ == DAS_OP_SORT
[2025-12-15 00:37:19.564 DEBUG] ObDASIterUtils::create_sort_sub_tree
[2025-12-15 00:37:19.564 DEBUG] create_iter_children_array
[2025-12-15 00:37:19.564 DEBUG] index merge iter use!!!
[2025-12-15 00:37:19.564 DEBUG] check_init_two_phase: no and
[2025-12-15 00:37:19.564 DEBUG] create_iter_children_array
[2025-12-15 00:37:19.564 DEBUG] ObDASIterUtils::create_cache_lookup_sub_tree
[2025-12-15 00:37:19.564 DEBUG] create_iter_children_array
[2025-12-15 00:37:19.564 DEBUG] ObTextDaaTIter::init
[2025-12-15 00:37:19.564 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.564 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.564 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.564 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.564 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.564 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.565 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.565 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.565 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.565 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:37:19.565 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.565 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:37:19.565 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.565 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:37:19.565 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.565 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:37:19.565 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:37:19.565 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:37:19.565 DEBUG] ctdef->pd_expr_spec_.pushdown_filters_.empty()
[2025-12-15 00:37:19.565 DEBUG] do_index_lookup::scan_param op_filters_ nonono
[2025-12-15 00:37:19.565 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.565 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.566 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.566 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.566 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.566 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.566 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.566 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.566 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.566 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.567 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.567 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.567 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.567 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.567 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.567 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.567 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.567 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.567 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.567 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.567 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:37:19.567 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.567 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:37:19.568 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.568 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:37:19.568 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.568 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:37:19.568 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:37:19.568 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:37:19.568 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.568 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.568 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.568 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.568 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.568 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.568 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.568 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.568 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.568 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.569 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.569 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.569 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.569 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.569 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.569 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.569 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-15 00:37:19.569 DEBUG] --------------FINISHED----------
[2025-12-15 00:37:19.569 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.569 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.569 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.569 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.569 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.569 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.569 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.569 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:37:19.569 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:37:19.569 DEBUG] ctdef->pd_expr_spec_.pushdown_filters_.empty()
[2025-12-15 00:37:19.569 DEBUG] do_index_lookup::scan_param op_filters_ nonono
[2025-12-15 00:37:19.569 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.569 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.570 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.570 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.570 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.570 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.570 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.570 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.570 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.570 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.570 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.570 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.570 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.570 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.570 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.570 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.570 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.570 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.571 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.571 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.571 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.571 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.571 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.571 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.571 DEBUG] MY_SEPC filter size: 1 count: 232 need_final_limit: 0
[2025-12-15 00:37:19.571 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.571 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.571 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.571 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.571 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.571 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.571 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.571 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.571 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.571 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.571 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.571 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.571 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.571 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.571 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:37:19.571 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:37:19.571 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.571 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.572 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.572 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.572 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.572 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.572 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.572 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.572 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.572 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.572 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.572 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.572 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.572 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.572 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.572 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.572 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.572 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.572 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.572 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.572 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.572 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.572 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.573 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.573 DEBUG] MY_SEPC filter size: 1 count: 232 need_final_limit: 0
[2025-12-15 00:37:19.573 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.573 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.573 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.573 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.573 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.573 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.573 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.573 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.573 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.573 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.573 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.573 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.573 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.573 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.573 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:37:19.573 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:37:19.573 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.573 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.573 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.573 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.573 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.573 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.573 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.573 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.573 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.574 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.574 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.574 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.574 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.574 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.574 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.574 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.574 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.574 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.574 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.574 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.574 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.574 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.574 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.574 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.574 DEBUG] MY_SEPC filter size: 1 count: 232 need_final_limit: 0
[2025-12-15 00:37:19.575 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.575 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.575 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.575 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.575 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.575 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.575 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.575 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.575 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.575 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.575 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.575 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.575 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.575 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.575 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:37:19.575 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:37:19.575 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.575 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.575 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.575 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.575 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.575 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.575 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.575 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.575 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.576 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.576 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.576 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.576 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.576 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.576 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.576 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.576 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.576 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.576 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.576 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.576 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.576 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.576 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.576 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.576 DEBUG] MY_SEPC filter size: 1 count: 232 need_final_limit: 0
[2025-12-15 00:37:19.576 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.576 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.576 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.576 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.576 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.576 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.576 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.576 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.576 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.576 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.577 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.577 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.577 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.577 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.577 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:37:19.577 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:37:19.577 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.577 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.577 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.577 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.577 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.577 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.577 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.577 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.577 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.578 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.578 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.578 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.578 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.578 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.578 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.578 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.578 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.578 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.578 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.578 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.578 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.578 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.578 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.579 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.579 DEBUG] MY_SEPC filter size: 1 count: 232 need_final_limit: 0
[2025-12-15 00:37:19.579 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.579 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.579 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.579 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.579 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.579 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.579 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.579 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.579 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.579 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.579 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.579 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.579 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.579 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.579 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:37:19.579 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:37:19.579 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.579 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.580 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.580 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.580 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.580 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.580 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.580 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.580 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.580 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.580 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.580 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.580 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.580 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.580 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.580 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.580 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.580 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.580 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.580 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.580 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.580 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.580 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.581 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.581 DEBUG] MY_SEPC filter size: 1 count: 232 need_final_limit: 0
[2025-12-15 00:37:19.581 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.581 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.581 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.581 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.581 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.581 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.581 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.581 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.581 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.581 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.581 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.581 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.581 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.581 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.581 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:37:19.581 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:37:19.581 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.581 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.581 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.581 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.581 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.581 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.581 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.581 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.581 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.582 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.582 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.582 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.582 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.582 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.582 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.582 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.582 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.582 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.582 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.582 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.582 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.582 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.582 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.582 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.582 DEBUG] MY_SEPC filter size: 1 count: 232 need_final_limit: 0
[2025-12-15 00:37:19.582 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.582 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.582 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.582 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.582 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.582 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.582 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.582 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.583 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.583 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.583 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.583 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.583 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.583 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.583 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:37:19.583 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:37:19.583 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.583 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.583 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.583 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.583 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.583 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.583 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.583 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.583 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.584 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.584 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.584 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.584 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.584 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.584 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.584 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.584 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.584 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.584 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.584 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.584 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.584 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.584 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.584 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.584 DEBUG] MY_SEPC filter size: 1 count: 232 need_final_limit: 0
[2025-12-15 00:37:19.584 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.584 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.584 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.584 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.584 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.584 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.584 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.584 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.585 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.585 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.585 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.585 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.585 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.585 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.585 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:37:19.585 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:37:19.585 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.585 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.585 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.585 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.585 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.585 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.585 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.585 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.585 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.586 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.586 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.586 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.586 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.586 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.586 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.586 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.586 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.586 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.586 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.586 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.586 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.586 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.586 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.586 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.586 DEBUG] MY_SEPC filter size: 1 count: 232 need_final_limit: 0
[2025-12-15 00:37:19.586 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.586 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.586 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.586 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.586 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.586 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.586 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.586 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.586 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.586 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.586 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.586 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.587 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.587 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.587 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:37:19.587 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:37:19.587 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.587 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.587 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.587 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.587 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.587 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.587 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.587 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.587 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.587 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.587 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.587 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.587 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.587 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.587 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.587 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.588 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.588 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.588 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.588 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.588 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.588 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.588 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.588 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.588 DEBUG] MY_SEPC filter size: 1 count: 232 need_final_limit: 0
[2025-12-15 00:37:19.588 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.588 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.588 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.588 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.588 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.588 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.588 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.588 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.588 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.588 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.588 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.588 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.588 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.588 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.588 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:37:19.588 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:37:19.588 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.588 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.589 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.589 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.589 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.589 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.589 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.589 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.589 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.589 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.589 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.589 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.589 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.589 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.589 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.589 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.590 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.590 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.590 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.590 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.590 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.590 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.590 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.590 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.590 DEBUG] MY_SEPC filter size: 1 count: 232 need_final_limit: 0
[2025-12-15 00:37:19.590 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.590 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.590 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.590 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.590 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.590 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.590 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.590 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.590 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.590 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.590 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.590 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.590 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.590 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.590 DEBUG] --------------DO_LOOKUP----------
[2025-12-15 00:37:19.590 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-15 00:37:19.590 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.590 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.591 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.591 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.591 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.591 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.591 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.591 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.591 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.591 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.591 DEBUG] MY_SEPC filter size: 1 count: 256 need_final_limit: 0
[2025-12-15 00:37:19.591 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.591 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.591 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.591 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.591 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.591 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-15 00:37:19.591 DEBUG] MY_SEPC filter size: 1 count: 117 need_final_limit: 0
[2025-12-15 00:37:19.591 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-15 00:37:19.591 DEBUG] 222 parent_filters_.count() = 0
[2025-12-15 00:37:19.591 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-15 00:37:19.591 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-15 00:37:19.591 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-15 00:37:19.591 DEBUG] --------------INDEX_SCAN----------
[2025-12-15 00:37:19.591 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-15 00:37:19.591 DEBUG] ObDASIndexMergeIter::union_get_next_rows
[2025-12-15 00:37:19.591 DEBUG] --------------FINISHED----------
[2025-12-15 00:37:19.591 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
