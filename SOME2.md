[2025-12-19 22:51:43.202 DEBUG] create_iter_children_array
[2025-12-19 22:51:43.202 DEBUG] ObExprRangeConverter::convert_in_expr start
[2025-12-19 22:51:43.202 DEBUG] l_expr->is_column_ref_expr()
[2025-12-19 22:51:43.202 DEBUG] r_expr->get_param_count(): 1
[2025-12-19 22:51:43.202 DEBUG] range_node->in_param_count_ 1
[2025-12-19 22:51:43.202 DEBUG] ObExprRangeConverter::convert_in_expr end
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::create_index_merge_access_paths START
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::get_candi_index_merge_trees START
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::get_valid_index_merge_indexes
[2025-12-19 22:51:43.204 DEBUG] 检验每一个候选索引是否都支持索引合并
[2025-12-19 22:51:43.204 DEBUG] 总共候选索引的个数有 0
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::generate_candi_index_merge_trees START
[2025-12-19 22:51:43.204 DEBUG] valid_index_ids 是空的
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::generate_candi_index_merge_trees END
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::get_valid_index_merge_indexes
[2025-12-19 22:51:43.204 DEBUG] 检验每一个候选索引是否都支持索引合并
[2025-12-19 22:51:43.204 DEBUG] 总共候选索引的个数有 8
[2025-12-19 22:51:43.204 DEBUG] 0 有初步满足条件
[2025-12-19 22:51:43.204 DEBUG] 0 有完整满足条件
[2025-12-19 22:51:43.204 DEBUG] 1 有初步满足条件
[2025-12-19 22:51:43.204 DEBUG] 1 有完整满足条件
[2025-12-19 22:51:43.204 DEBUG] 2 有初步满足条件
[2025-12-19 22:51:43.204 DEBUG] 2 有完整满足条件
[2025-12-19 22:51:43.204 DEBUG] 3 有初步满足条件
[2025-12-19 22:51:43.204 DEBUG] 3 有完整满足条件
[2025-12-19 22:51:43.204 DEBUG] 4 有初步满足条件
[2025-12-19 22:51:43.204 DEBUG] 4 有完整满足条件
[2025-12-19 22:51:43.204 DEBUG] 5 有初步满足条件
[2025-12-19 22:51:43.204 DEBUG] 5 有完整满足条件
[2025-12-19 22:51:43.204 DEBUG] 6 有初步满足条件
[2025-12-19 22:51:43.204 DEBUG] 6 有完整满足条件
[2025-12-19 22:51:43.204 DEBUG] 7 有初步满足条件
[2025-12-19 22:51:43.204 DEBUG] 7 有完整满足条件
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::generate_candi_index_merge_trees START
[2025-12-19 22:51:43.204 DEBUG] filters.count() = 6
[2025-12-19 22:51:43.204 DEBUG] column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] valid index num: valid_index_ids.count()8
[2025-12-19 22:51:43.204 DEBUG] index_id : 500024
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500025
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500028
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 17
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500029
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 18
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500030
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500031
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 20
[2025-12-19 22:51:43.204 DEBUG] index_id : 500032
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 21
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 20
[2025-12-19 22:51:43.204 DEBUG] index_id : 500033
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 20
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 21
[2025-12-19 22:51:43.204 DEBUG] column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] valid index num: valid_index_ids.count()8
[2025-12-19 22:51:43.204 DEBUG] index_id : 500024
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500025
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500028
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 17
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500029
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 18
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500030
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500031
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 20
[2025-12-19 22:51:43.204 DEBUG] index_id : 500032
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 21
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 20
[2025-12-19 22:51:43.204 DEBUG] index_id : 500033
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 20
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 21
[2025-12-19 22:51:43.204 DEBUG] column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] valid index num: valid_index_ids.count()8
[2025-12-19 22:51:43.204 DEBUG] index_id : 500024
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500025
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500028
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 17
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500029
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 18
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500030
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500031
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 20
[2025-12-19 22:51:43.204 DEBUG] index_id : 500032
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 21
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 20
[2025-12-19 22:51:43.204 DEBUG] index_id : 500033
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 20
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 21
[2025-12-19 22:51:43.204 DEBUG] column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] valid index num: valid_index_ids.count()8
[2025-12-19 22:51:43.204 DEBUG] index_id : 500024
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500025
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500028
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 17
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500029
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 18
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500030
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 16
[2025-12-19 22:51:43.204 DEBUG] NO FTS NO IN : column_ids.at(0): 16
[2025-12-19 22:51:43.204 DEBUG] index_id : 500031
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 1
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 20
[2025-12-19 22:51:43.204 DEBUG] index_id : 500032
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 21
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 20
[2025-12-19 22:51:43.204 DEBUG] index_id : 500033
[2025-12-19 22:51:43.204 DEBUG] index_column_ids.count() : 2
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 20
[2025-12-19 22:51:43.204 DEBUG] index_column_id : 21
[2025-12-19 22:51:43.204 DEBUG] and_root->node_type_ 2
[2025-12-19 22:51:43.204 DEBUG] and_root->children_ count() 6
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-19 22:51:43.204 DEBUG] Added AND merge tree for 6 conditions
[2025-12-19 22:51:43.204 DEBUG] and_root->node_type_ 2
[2025-12-19 22:51:43.204 DEBUG] and_root->children_ count() 6
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::generate_candi_index_merge_trees END
[2025-12-19 22:51:43.204 DEBUG] generate candi index merge trees, count() 1
[2025-12-19 22:51:43.204 DEBUG] first node type : 2
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::get_candi_index_merge_trees END
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::do_create_index_merge_paths START
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::choose_best_selectivity_branch START
[2025-12-19 22:51:43.204 DEBUG] choose_best_selectivity_branch:: INDEX_MERGE_INTERSECT == candi_node->node_type_
[2025-12-19 22:51:43.204 DEBUG] choose_best_selectivity_branch::  candi_node->children_.count() 6
[2025-12-19 22:51:43.204 DEBUG] choose_best_selectivity_branch::  has_scan_children 1
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::choose_best_selectivity_branch START
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::choose_best_selectivity_branch END
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::choose_best_selectivity_branch START
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::choose_best_selectivity_branch END
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::choose_best_selectivity_branch START
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::choose_best_selectivity_branch END
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::choose_best_selectivity_branch START
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::choose_best_selectivity_branch END
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::choose_best_selectivity_branch START
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::choose_best_selectivity_branch END
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::choose_best_selectivity_branch START
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::choose_best_selectivity_branch END
[2025-12-19 22:51:43.204 DEBUG] ObJoinOrder::choose_best_selectivity_branch END
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree start
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-19 22:51:43.204 DEBUG] ObIndexMergeNode::formalize_index_merge_tree end
[2025-12-19 22:51:43.205 DEBUG] ObExprRangeConverter::convert_in_expr start
[2025-12-19 22:51:43.205 DEBUG] l_expr->is_column_ref_expr()
[2025-12-19 22:51:43.205 DEBUG] r_expr->get_param_count(): 3
[2025-12-19 22:51:43.205 DEBUG] range_node->in_param_count_ 3
[2025-12-19 22:51:43.205 DEBUG] ObExprRangeConverter::convert_in_expr end
[2025-12-19 22:51:43.205 DEBUG] p->topn_limit_ = limit_value : 10
[2025-12-19 22:51:43.205 DEBUG] ObJoinOrder::check_index_merge_paths_contain_fts start
[2025-12-19 22:51:43.205 DEBUG] root_node->children_.count() 6
[2025-12-19 22:51:43.205 DEBUG] contain_fts: 1
[2025-12-19 22:51:43.205 DEBUG] ObJoinOrder::check_index_merge_paths_contain_fts end
[2025-12-19 22:51:43.205 DEBUG] ObJoinOrder::create_index_merge_access_paths END
[2025-12-19 22:51:43.206 DEBUG] ObAccessPathEstimation::classify_paths start
[2025-12-19 22:51:43.206 DEBUG] there is index merge path
[2025-12-19 22:51:43.209 DEBUG] set index merge path cost = 0 ?
[2025-12-19 22:51:43.209 DEBUG] ap->is_index_merge_path
[2025-12-19 22:51:43.210 DEBUG] if (op.use_index_merge()) {
[2025-12-19 22:51:43.210 DEBUG] OB_UNLIKELY(op.use_index_merge() || op.has_es_match()))
[2025-12-19 22:51:43.210 DEBUG] AHAHAHA index_merge
[2025-12-19 22:51:43.210 DEBUG] if (OB_FAIL(generate_index_merge_ctdef(op, tsc_ctdef, index_merge_ctdef)))
[2025-12-19 22:51:43.210 DEBUG] use index merge
[2025-12-19 22:51:43.210 DEBUG] use index merge
[2025-12-19 22:51:43.210 DEBUG] use index merge
[2025-12-19 22:51:43.210 DEBUG] use index merge
[2025-12-19 22:51:43.210 DEBUG] use index merge 22222
[2025-12-19 22:51:43.210 DEBUG] use index merge
[2025-12-19 22:51:43.210 DEBUG] use index merge
[2025-12-19 22:51:43.210 DEBUG] 1 limit_op->get_limit_expr()
[2025-12-19 22:51:43.210 DEBUG] 1 set limit_expr to root_ctdef
[2025-12-19 22:51:43.210 DEBUG] HHHHHHHH true 10
[2025-12-19 22:51:43.210 DEBUG] HHHHHHHHHHHHHHHHHHHHHHHHHH
[2025-12-19 22:51:43.210 DEBUG] if (OB_SUCC(ret) && index_merge_cnt > 0)
[2025-12-19 22:51:43.210 DEBUG] create_index_merge_iter_tree
[2025-12-19 22:51:43.210 DEBUG] create_index_merge_sub_tree
[2025-12-19 22:51:43.210 DEBUG] ObDASIterUtils::create_text_retrieval_sub_tree
[2025-12-19 22:51:43.210 DEBUG] tr merge iterfunction_lookup_mode0topk_mode0daat_mode1taat_mode0
[2025-12-19 22:51:43.210 DEBUG] create_iter_children_array
[2025-12-19 22:51:43.210 DEBUG] ObDASIterUtils::create_cache_lookup_sub_tree
[2025-12-19 22:51:43.210 DEBUG] create_iter_children_array
[2025-12-19 22:51:43.210 DEBUG] (merge_ctdef->merge_node_types_.at(i) == INDEX_MERGE_FTS_INDEX && need_sort)
[2025-12-19 22:51:43.210 DEBUG] init sort iter and child_ctdef->op_type_ == DAS_OP_SORT
[2025-12-19 22:51:43.210 DEBUG] ObDASIterUtils::create_sort_sub_tree
[2025-12-19 22:51:43.210 DEBUG] create_iter_children_array
[2025-12-19 22:51:43.210 DEBUG] index merge iter use!!!
[2025-12-19 22:51:43.210 DEBUG] ObDASIndexMergeIter::inner_init start
[2025-12-19 22:51:43.210 DEBUG] result_buffer_.topn_limit_10
[2025-12-19 22:51:43.210 DEBUG] ObDASIndexMergeIter::inner_init end
[2025-12-19 22:51:43.210 DEBUG] create_iter_children_array
[2025-12-19 22:51:43.210 DEBUG] ObDASIterUtils::create_cache_lookup_sub_tree
[2025-12-19 22:51:43.210 DEBUG] create_iter_children_array
[2025-12-19 22:51:43.212 DEBUG] token:text
[2025-12-19 22:51:43.212 DEBUG] use daat mode
[2025-12-19 22:51:43.212 DEBUG] ObTextDaaTIter::init
[2025-12-19 22:51:43.212 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-19 22:51:43.212 DEBUG] 222 parent_filters_.count() = 0
[2025-12-19 22:51:43.212 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN----------
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] ObDASIndexMergeIter::intersect_get_next_rows start
[2025-12-19 22:51:43.212 DEBUG] result_buffer_.reuse();
[2025-12-19 22:51:43.212 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.212 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.212 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.212 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN----------
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.212 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.212 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.213 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.213 DEBUG] --------------DO_LOOKUP----------
[2025-12-19 22:51:43.213 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-19 22:51:43.213 DEBUG] ctdef->pd_expr_spec_.pushdown_filters_.empty()
[2025-12-19 22:51:43.213 DEBUG] do_index_lookup::scan_param op_filters_ nonono
[2025-12-19 22:51:43.214 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.214 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.214 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.214 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.214 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.214 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.214 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.214 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.214 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.214 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.214 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.214 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.214 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.214 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.214 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.214 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.214 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.214 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.214 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.214 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.214 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.214 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.215 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.215 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.215 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.215 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.215 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.215 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.215 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.215 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.215 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.215 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.215 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.215 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.215 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.215 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.215 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.215 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.215 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.215 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.215 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.215 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.215 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.215 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.215 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.215 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.215 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.215 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.215 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.215 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.216 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.216 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.217 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.217 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.218 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.218 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.218 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.218 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.218 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.218 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.218 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.218 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.218 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.218 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.218 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.218 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.218 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.218 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.218 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.218 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.218 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.218 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.218 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.218 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.218 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.218 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.218 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.218 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.218 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.218 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.218 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.218 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.218 DEBUG] --------------INDEX_SCAN----------
[2025-12-19 22:51:43.218 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.218 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.218 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.218 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.218 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.218 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.218 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.218 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.218 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.218 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.218 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.218 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.218 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.218 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.218 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.218 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.218 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.218 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] 阿巴 count = 0
[2025-12-19 22:51:43.219 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.219 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.219 DEBUG] 阿巴 count = 0
[2025-12-19 22:51:43.219 DEBUG] --------------DO_LOOKUP----------
[2025-12-19 22:51:43.219 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.219 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.219 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.220 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.220 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.221 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.221 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.221 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.221 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.221 DEBUG] --------------INDEX_SCAN----------
[2025-12-19 22:51:43.221 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.221 DEBUG] sparse_retrieval_iter_->get_next_rows()
[2025-12-19 22:51:43.221 DEBUG] 阿巴 count = 0
[2025-12-19 22:51:43.221 DEBUG] --------------FINISHED----------
[2025-12-19 22:51:43.221 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.221 DEBUG] child_iter: i3  child_rows_cnt16
[2025-12-19 22:51:43.221 DEBUG] i: 3child_iter->get_type() == DAS_ITER_SORT
[2025-12-19 22:51:43.221 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.221 DEBUG] child_iter: i5  child_rows_cnt16
[2025-12-19 22:51:43.221 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.221 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.221 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.221 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.221 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.221 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.221 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.221 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.221 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.221 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.221 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.221 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.221 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i3  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] i: 3child_iter->get_type() == DAS_ITER_SORT
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i3  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] i: 3child_iter->get_type() == DAS_ITER_SORT
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i3  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] i: 3child_iter->get_type() == DAS_ITER_SORT
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i3  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] i: 3child_iter->get_type() == DAS_ITER_SORT
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i3  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] i: 3child_iter->get_type() == DAS_ITER_SORT
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i3  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] i: 3child_iter->get_type() == DAS_ITER_SORT
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.222 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i3  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] i: 3child_iter->get_type() == DAS_ITER_SORT
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i3  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] i: 3child_iter->get_type() == DAS_ITER_SORT
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i3  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] i: 3child_iter->get_type() == DAS_ITER_SORT
[2025-12-19 22:51:43.223 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i3  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] i: 3child_iter->get_type() == DAS_ITER_SORT
[2025-12-19 22:51:43.223 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i1  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i0  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i2  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt9
[2025-12-19 22:51:43.223 DEBUG] child_iter: i3  child_rows_cnt16
[2025-12-19 22:51:43.223 DEBUG] i: 3child_iter->get_type() == DAS_ITER_SORT
[2025-12-19 22:51:43.223 DEBUG] ObDASIndexMergeIter::MergeResultBuffer::add_row_with_topn
[2025-12-19 22:51:43.223 DEBUG] add_row_with_topn
[2025-12-19 22:51:43.223 DEBUG] exprs countexprs_->count()2
[2025-12-19 22:51:43.223 DEBUG] eval_ctx ptreval_ctx_0x7f2d6211c0f8
[2025-12-19 22:51:43.223 DEBUG] 直接获取的分数: 2.55616
[2025-12-19 22:51:43.223 DEBUG] checking...
[2025-12-19 22:51:43.223 DEBUG] stored_row cntstored_row->cnt_2
[2025-12-19 22:51:43.223 DEBUG] datumi0stored_row->cells()[i].is_null()0
[2025-12-19 22:51:43.223 DEBUG] datumi1stored_row->cells()[i].is_null()0
[2025-12-19 22:51:43.223 DEBUG] 构建的row的score: 2.55616
[2025-12-19 22:51:43.223 DEBUG] make_topn_row(row): row->score_: 2.55616
[2025-12-19 22:51:43.223 DEBUG] child_iter: i4  child_rows_cnt0
[2025-12-19 22:51:43.223 DEBUG] OB_FAIL(ret)
[2025-12-19 22:51:43.223 DEBUG] intersect_get_next_rows : OB_FAIL(ret) ret == OB_ITER_END
[2025-12-19 22:51:43.223 DEBUG] intersect_get_next_rows :count1
[2025-12-19 22:51:43.223 DEBUG] result_buffer_.to_expr_from_topn(count): 1
[2025-12-19 22:51:43.223 DEBUG] MergeResultBuffer::to_expr_from_topn
[2025-12-19 22:51:43.223 DEBUG] to_expr exprs countexprs_->count()2
[2025-12-19 22:51:43.223 DEBUG] to_expr eval_ctx ptreval_ctx_0x7f2d6211c0f8
[2025-12-19 22:51:43.223 DEBUG] ObDASIndexMergeIter::intersect_get_next_rows end
[2025-12-19 22:51:43.223 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.223 DEBUG] ObDASIndexMergeIter::intersect_get_next_rows start
[2025-12-19 22:51:43.223 DEBUG] result_buffer_.reuse();
[2025-12-19 22:51:43.223 DEBUG] OB_FAIL(ret)
[2025-12-19 22:51:43.223 DEBUG] intersect_get_next_rows : OB_FAIL(ret) ret == OB_ITER_END
[2025-12-19 22:51:43.223 DEBUG] intersect_get_next_rows :count0
[2025-12-19 22:51:43.223 DEBUG] ObDASIndexMergeIter::intersect_get_next_rows end
[2025-12-19 22:51:43.223 DEBUG] --------------DO_LOOKUP----------
[2025-12-19 22:51:43.223 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-12-19 22:51:43.223 DEBUG] ctdef->pd_expr_spec_.pushdown_filters_.empty()
[2025-12-19 22:51:43.223 DEBUG] do_index_lookup::scan_param op_filters_ nonono
[2025-12-19 22:51:43.223 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.223 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.223 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.223 DEBUG] MY_SEPC filter size: 6 count: 1 need_final_limit: 0
[2025-12-19 22:51:43.223 DEBUG] change new_rot_iter to ObDASCacheLookupIter
[2025-12-19 22:51:43.223 DEBUG] 222 parent_filters_.count() = 0
[2025-12-19 22:51:43.223 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-12-19 22:51:43.223 DEBUG] --------------OUTPUT_ROWS----------
[2025-12-19 22:51:43.223 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-12-19 22:51:43.223 DEBUG] --------------INDEX_SCAN----------
[2025-12-19 22:51:43.223 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
[2025-12-19 22:51:43.223 DEBUG] ObDASIndexMergeIter::intersect_get_next_rows start
[2025-12-19 22:51:43.223 DEBUG] result_buffer_.reuse();
[2025-12-19 22:51:43.223 DEBUG] OB_FAIL(ret)
[2025-12-19 22:51:43.223 DEBUG] intersect_get_next_rows : OB_FAIL(ret) ret == OB_ITER_END
[2025-12-19 22:51:43.223 DEBUG] intersect_get_next_rows :count0
[2025-12-19 22:51:43.223 DEBUG] ObDASIndexMergeIter::intersect_get_next_rows end
[2025-12-19 22:51:43.223 DEBUG] --------------FINISHED----------
[2025-12-19 22:51:43.223 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end
[2025-12-19 22:51:43.223 DEBUG] ObTextDaaTIter::reset()
[2025-12-19 22:51:46.516 DEBUG] create_iter_children_array
[2025-12-19 22:51:46.516 DEBUG] --------------ObDASLookupIter::next_rows begin----------
[2025-12-19 22:51:46.516 DEBUG] --------------ObDASLookupIter::next_rows INDEX_SCAN----------
[2025-12-19 22:51:46.516 DEBUG] --------------ObDASLookupIter::next_rows INDEX_SCAN get_next_rows----------
[2025-12-19 22:51:46.516 DEBUG] --------------ObDASLookupIter::next_rows end----------










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