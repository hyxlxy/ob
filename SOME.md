*****************************************2025-12-15（晚）  *********************************************************
还没有添加TWO_PHASE_INDEX_MERGE的，或者是比较危险的：

- ObDASUtils::is_two_phase_index_merge        大概ok
- ObDASIterUtils::set_index_merge_related_ids
- ObDASIterUtils::set_func_lookup_iter_related_ids
- ObDASIterUtils::create_function_lookup_tree
- ObDasVecScanUtils::init_limit 这个貌似可能不需要？
- ObTscCgService::generate_index_merge_node_ctdef（这个可能不需要？专门给index_merge的？）
- ob_das_def_reg.h里
- ObTableScanOp::prepare_range_for_each_index



有关 DAS_OP_TWO_PHASE_INDEX_MERGE 的内容，搜索一下 DAS_OP_INDEX_MERGE 就可以知道了。

*****************************************2025-12-15  *********************************************************

需要创建:
- ob_das_two_phase_index_merge_iter.h: 创建出来二阶段迭代器。
- 在das_iter_utils.cpp 里面 需要创建出来对应的二阶段tree
- 

join_order.cpp:
- ObJoinOrder::do_create_index_merge_paths 这个函数是对于and_root进行创建IndexMergePath的。
  所以我们也应该在这去创建two_phase_index_merge_path. 

  关于检验是否满足执行二阶段的时候：参考init_two_phase_fts_opt函数


```c++
int ObJoinOrder::create_two_phase_index_merge_path(  
    const uint64_t ref_table_id,  
    const ObIArray<ObRawExpr*> &filters,  
    AccessPath *&two_phase_path)  
{  
  int ret = OB_SUCCESS;  
  two_phase_path = nullptr;  
    
  // 检查是否满足两阶段优化条件  
  bool need_two_phase = false;  
  if (OB_FAIL(check_need_two_phase_optimization(filters, need_two_phase))) {  
    LOG_WARN("failed to check need two phase optimization", K(ret));  
  } else if (!need_two_phase) {  
    // 不需要两阶段优化  
  } else if (OB_ISNULL(two_phase_path = OB_NEWx(AccessPath, allocator_))) {  
    ret = OB_ALLOCATE_MEMORY_FAILED;  
    LOG_WARN("failed to allocate access path", K(ret));  
  } else {  
    // 设置两阶段路径属性  
    two_phase_path->type_ = AccessPath::TWO_PHASE_INDEX_MERGE;  
    two_phase_path->table_id_ = table_id_;  
    two_phase_path->ref_table_id_ = ref_table_id;  
    two_phase_path->is_index_merge_ = true;  
    two_phase_path->two_phase_info_.enable_ = true;  
    two_phase_path->two_phase_info_.backup_limit_ = 1000;  // 保底数量  
    two_phase_path->two_phase_info_.topk_limit_ = get_limit_value();  // 最终TOP-N数量  
    // 创建子路径  
    if (OB_FAIL(create_backup_merge_path(filters, two_phase_path->backup_merge_path_))) {  
      LOG_WARN("failed to create backup merge path", K(ret));  
    } else if (OB_FAIL(create_bmw_scan_path(filters, two_phase_path->bmw_scan_path_))) {  
      LOG_WARN("failed to create bmw scan path", K(ret));  
    }  
  }  
    
  return ret;  
}

int ObJoinOrder::do_create_index_merge_paths(const uint64_t table_id,  
                                            const uint64_t ref_table_id,  
                                            const PathHelper &helper,  
                                            ObIndexInfoCache &index_info_cache,  
                                            ObIArray<ObIndexMergeNode*> &candi_index_trees,  
                                            ObIArray<AccessPath*> &access_paths)  
{  
  int ret = OB_SUCCESS;  
    
  // 现有的单阶段索引合并逻辑  
  if (OB_FAIL(generate_normal_index_merge_paths(...))) {  
    LOG_WARN("failed to generate normal index merge paths", K(ret));  
  }  
    
  // 新增：生成两阶段优化路径  
  if (OB_SUCC(ret)) {  
    AccessPath *two_phase_path = nullptr;  
    if (OB_FAIL(create_two_phase_index_merge_path(ref_table_id, helper.filters_, two_phase_path))) {  
      LOG_WARN("failed to create two phase index merge path", K(ret));  
    } else if (OB_NOT_NULL(two_phase_path)) {  
      if (OB_FAIL(access_paths.push_back(two_phase_path))) {  
        LOG_WARN("failed to push back two phase path", K(ret));  
      }  
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


*****************************************2025-12-14 (lingchen) *********************************************************

bclient [test]> explain  select id , base_id from test_insert_en where id < 1000 or base_id in ('base_id_2', 'base_id_1') order by id limit 20;
+------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Query Plan                                                                                                                                                       |
+------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ===========================================================================================                                                                      |
| |ID|OPERATOR                    |NAME                               |EST.ROWS|EST.TIME(us)|                                                                      |
| -------------------------------------------------------------------------------------------                                                                      |
| |0 |DISTRIBUTED INDEX MERGE SCAN|test_insert_en(primary,idx_base_id)|6838    |0           |                                                                      |
| ===========================================================================================                                                                      |
| Outputs & filters:                                                                                                                                               |
| -------------------------------------                                                                                                                            |
|   0 - output([test_insert_en.id], [test_insert_en.base_id]), filter([test_insert_en.id < 1000 OR test_insert_en.base_id IN (cast('base_id_2', VARCHAR(1048576)), |
|        cast('base_id_1', VARCHAR(1048576)))]), rowset=256                                                                                                        |
|       access([test_insert_en.id], [test_insert_en.base_id]), partitions(p0)                                                                                      |
|       limit(20), offset(nil), is_index_back=true, is_global_index=false, keep_ordering=true, use_index_merge=true, filter_before_indexback[false],               |
|       index_name: primary, range_cond([test_insert_en.id < 1000]), filter(nil)                                                                                   |
|       index_name: idx_base_id, range_cond([test_insert_en.base_id IN (cast('base_id_2', VARCHAR(1048576)), cast('base_id_1', VARCHAR(1048576)))]), filter(nil)   |
|       lookup_filter([test_insert_en.id < 1000 OR test_insert_en.base_id IN (cast('base_id_2', VARCHAR(1048576)), cast('base_id_1', VARCHAR(1048576)))])          |
+------------------------------------------------------------------------------------------------------------------------------------------------------------------+
14 rows in set (0.005 sec)

obclient [test]> explain  select id , base_id from test_insert_en where id < 1000 and base_id in ('base_id_2', 'base_id_1') order by id;
+------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Query Plan                                                                                                                                                       |
+------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ===========================================================================================                                                                      |
| |ID|OPERATOR                    |NAME                               |EST.ROWS|EST.TIME(us)|                                                                      |
| -------------------------------------------------------------------------------------------                                                                      |
| |0 |DISTRIBUTED INDEX MERGE SCAN|test_insert_en(primary,idx_base_id)|666     |0           |                                                                      |
| ===========================================================================================                                                                      |
| Outputs & filters:                                                                                                                                               |
| -------------------------------------                                                                                                                            |
|   0 - output([test_insert_en.id], [test_insert_en.base_id]), filter([test_insert_en.id < 1000], [test_insert_en.base_id IN (cast('base_id_2', VARCHAR(1048576)), |
|        cast('base_id_1', VARCHAR(1048576)))]), rowset=256                                                                                                        |
|       access([test_insert_en.id], [test_insert_en.base_id]), partitions(p0)                                                                                      |
|       is_index_back=true, is_global_index=false, keep_ordering=true, use_index_merge=true, filter_before_indexback[false,false],                                 |
|       index_name: primary, range_cond([test_insert_en.id < 1000]), filter(nil)                                                                                   |
|       index_name: idx_base_id, range_cond([test_insert_en.base_id IN (cast('base_id_2', VARCHAR(1048576)), cast('base_id_1', VARCHAR(1048576)))]), filter(nil)   |
|       lookup_filter([test_insert_en.id < 1000], [test_insert_en.base_id IN (cast('base_id_2', VARCHAR(1048576)), cast('base_id_1', VARCHAR(1048576)))])          |
+------------------------------------------------------------------------------------------------------------------------------------------------------------------+
14 rows in set (0.003 sec)

obclient [test]> 



*****************************************2025-12-12 *********************************************************
obclient [test]> SELECT id, docid_col, MATCH(fulltext_col) AGAINST('text') as _score FROM test_insert_en WHERE  MATCH(fulltext_col) AGAINST('text')   and id < 1000   and base_id in ('base_id_1','base_id_2','base_id_3') ORDER BY _score DESC LIMIT 20;
+-----+--------------+--------------------+
| id  | docid_col    | _score             |
+-----+--------------+--------------------+
|  17 | doc-en-14104 | 3.5497468980644196 |
| 809 | doc-en-16325 |  3.542051730602606 |
| 135 | doc-en-9908  |  3.528472961069408 |
| 320 | doc-en-7032  |   3.51157986819706 |
| 471 | doc-en-5742  |  3.474244854320815 |
| 841 | doc-en-4360  | 3.4421817089762614 |
| 232 | doc-en-4910  | 3.3326772452751268 |
| 382 | doc-en-14867 | 3.2771159611444403 |
| 576 | doc-en-1021  | 3.2746315727805717 |
| 509 | doc-en-16007 | 3.2673188667892625 |
| 916 | doc-en-16294 |  3.250831312505436 |
| 290 | doc-en-611   | 3.2265187959802626 |
| 218 | doc-en-8266  | 3.2091540079686767 |
| 694 | doc-en-10860 | 3.1605223352138427 |
| 466 | doc-en-7480  | 3.1580386736063164 |
| 107 | doc-en-16764 | 3.1096939871819913 |
|  84 | doc-en-2907  | 3.0929084813677865 |
| 836 | doc-en-15078 | 3.0511657470616935 |
| 195 | doc-en-11817 | 3.0476304499027553 |
| 646 | doc-en-4002  | 3.0363641964198966 |
+-----+--------------+--------------------+

20 rows in set (0.006 sec)

obclient [test]> 

*****************************************2025-12-4 *********************************************************

create_access_paths -> create_index_merge_access_paths -> get_candi_index_merge_trees       ->  get_valid_index_merge_indexes
                                                                                            ->  generate_candi_index_merge_trees
                                                          !candi_index_trees.empty()        ->  check_candi_index_trees_match_hint
                                                                                            ->  ......
                                                                                            ->  get_valid_index_merge_indexes
                                                                                            ->  generate_candi_index_merge_trees
                                                                                            ->  prune_candi_index_merge_trees

                                                       -> do_create_index_merge_paths       ->  choose_best_selectivity_branch
                                                                                            ->  root_node->formalize_index_merge_tree()
                                                                                            ->  build_access_path_for_scan_node
                                                                                            ->  create_one_index_merge_path
                                                                                            ->  ......
                                                                                            ->  access_paths.push_back(static_cast<AccessPath*>(index_merge_path)))

                                                       -> check_index_merge_paths_contain_fts

                      (!is_match_hint && !contain_fts) -> prune_index_merge_path


由于现在是do_create_index_merge_paths -> root_node->formalize_index_merge_tree()之前，发现有问题，root_的child个数是0.
所以是在这个函数之前有问题。

在generate_candi_index_merge_trees函数结束的时候，还是正常的。
ObJoinOrder::get_candi_index_merge_trees END

怀疑：
get_candi_index_merge_trees  ->  prune_candi_index_merge_trees // 没有问题
do_create_index_merge_paths  ->  choose_best_selectivity_branch // 确实是这里的问题

*****************************************2025-12-4（lingchen） *********************************************************
跑不进index_merge的原因：
  ObJoinOrder::get_candi_index_merge_trees，调用了之后，并没有产生merge_path。candi_index_trees是空的，所以没有生成出来路径。
  大概率是因为 ObJoinOrder::generate_candi_index_merge_trees ，没有生成出来。

*****************************************2025-12-3 *********************************************************

有关设置一个filter的代码内容：

        if (scan_param.allocator_ != nullptr) {
          // 分配一个新的数组对象，使用4作为栈上元素数量（因为最多3个filter）
          void* buf = scan_param.allocator_->alloc(sizeof(common::ObSEArray<sql::ObExpr*, 4>));
          if (buf != nullptr) {
            // 在分配的内存上构造数组对象
            common::ObSEArray<sql::ObExpr*, 4>* new_filters = 
                new(buf) common::ObSEArray<sql::ObExpr*, 4>();
            adlog::DEBUG("new_filters->assign(parent_filters_)");
            // 将parent_filters_的内容复制到新数组
            int ret = new_filters->assign(parent_filters_);
            if (OB_SUCC(ret)) {
              // 使用const_cast修改const指针
              const_cast<sql::ObExprPtrIArray*&>(scan_param.op_filters_) = 
                  static_cast<sql::ObExprPtrIArray*>(new_filters);
            } else {
              LOG_WARN("failed to assign parent filters to new array", K(ret));
            }
          } else {
            LOG_WARN("failed to allocate memory for filters");
          }
        }

*****************************************2025-12-1 *********************************************************
TODO：
要给multipleMerge的Init里面打个日志
给TableScanIterator里也要打日志，看是什么时候初始化的MultipleMerge的。



--------有关get_next_rows的路径---------
TableScanOp:
调用了get_next_batch_with_das函数 (拥有标量筛选的filter)
    函数里的output_是DasMergeIter

DasMergeIter::
调用了inner_get_next_rows -> get_next_seq_rows 函数
                            函数中的scan_op->get_output_result_iter()->get_next_rows(count, capacity);
进入了CacheLookupIter（外层）

CacheLookupIter（外层的data_iter）
中的data_table_iter_（其实是父类LocalLookupIter的父类的DasLookupIter）是DasScanIter

DasScanIter的result_是TableScanIterator

TableScanIterator 再往下是 multipleMerge

-------------------------

--------有关初始化的路径（猜测）------------
外层的CacheLookupIter在执行do_index_lookup -> DASScanIter的do_table_scan -> AccessService (ObITabletScan的派生)的table_scan -> LSTabletService的table_scan 
-> LSTabletService 的inner_table_scan -> TableScanIterator 的init -> TableScanIterator 的 open_iter -> TableScanIterator 的 init_and_open_get_merge_iter 


在 INIT_AND_OPEN_ITER 宏里面有这么一条：
iter->init(main_table_param_, main_table_ctx_, get_table_param_)
这就是init的过程了应该

所以我们需要把 filter 弄到 TableScanIterator 里的main_table_param_就好了

param是从DasScanIter的do_table_scan一路传下来的，

set_scan_param是DasScanIter中修改scan_param的路径，init里不存在初始化他的内容


-----------有关filter的内容-------------
在class ObVTableScanParam中 有一个成员变量：const sql::ObExprPtrIArray *op_filters_;
然后 class ObTableScanParam : public common::ObVTableScanParam
这个ObTableScanParam是前者的派生类，所以里面也是存着filter的 const sql::ObExprPtrIArray *op_filters_


-------关于open的内容
ObDASScanOp里的scan_param_是创建迭代器树的开始。
ObIDASTaskOp::start_das_task是一个虚函数,ObDASScanOp是ObIDASTaskOp的派生类。

DasMergeIter中的das_ref_会执行execute_all_task

*****************************************2025-11-28 *********************************************************

外层: ObDASCacheLookupIter (最终结果获取)
├── index_table_iter_ → 内层 ObDASCacheLookupIter (全文检索+条件验证)  
│   ├── index_table_iter_ → ObDASTRMergeIter -> （ObISparseRetrievalMergeIter 的派生类）ObSRDaaTIterImpl (全文索引扫描)
│   └── data_table_iter_ → ObDASScanIter 
└── data_table_iter_ → ObDASFuncLookupIter (最终结果回表)

do_index_look_up 都是调用 data_table_iter_ 的 do_table_scan

现在已知，内层会返回全文检索的行，内层的回表是找到文档id（系统内部生成）对应的主键id


考虑有可能是log_plan.cpp文件里面的问题，尝试去改写路径

*****************************************2025-11-27，28(lingchen) *********************************************************

一个更加清晰地内容：

有关ObDASCacheLookupIter的嵌套的查询内容：

[2025-11-28 02:25:27.196 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
[2025-11-28 02:25:27.196 DEBUG] --------------INDEX_SCAN----------
[2025-11-28 02:25:27.196 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------

    [2025-11-28 02:25:27.196 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----begin
    [2025-11-28 02:25:27.196 DEBUG] --------------INDEX_SCAN----------
    [2025-11-28 02:25:27.196 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
    [2025-11-28 02:25:27.196 DEBUG] start try id_less_filter_extract
    [2025-11-28 02:25:27.196 DEBUG] now expr args cnt: 1 now expr type: 178
    [2025-11-28 02:25:27.196 DEBUG] not T_OP_AND and not id < xxx
    [2025-11-28 02:25:27.196 DEBUG] now expr args cnt: 2 now expr type: 4617
    [2025-11-28 02:25:27.196 DEBUG] not T_OP_AND and not id < xxx
    [2025-11-28 02:25:27.196 DEBUG] now expr args cnt: 0 now expr type: 89
    [2025-11-28 02:25:27.196 DEBUG] not T_OP_AND and not id < xxx
    [2025-11-28 02:25:27.196 DEBUG] now expr args cnt: 2 now expr type: 501
    [2025-11-28 02:25:27.196 DEBUG] not T_OP_AND and not id < xxx
    [2025-11-28 02:25:27.196 DEBUG] now expr args cnt: 0 now expr type: 26
    [2025-11-28 02:25:27.196 DEBUG] not T_OP_AND and not id < xxx
    [2025-11-28 02:25:27.196 DEBUG] now expr args cnt: 0 now expr type: 5
    [2025-11-28 02:25:27.196 DEBUG] not T_OP_AND and not id < xxx
    [2025-11-28 02:25:27.196 DEBUG] not find id_less_filter
    [2025-11-28 02:25:27.197 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
    [2025-11-28 02:25:27.197 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
    [2025-11-28 02:25:27.197 DEBUG] --------------INDEX_SCAN: index_table_iter_->get_next_rows-----------
    [2025-11-28 02:25:27.197 DEBUG] --------------DO_LOOKUP----------
    [2025-11-28 02:25:27.197 DEBUG] ObDASLocalLookupIter::do_index_lookup()
    [2025-11-28 02:25:27.198 DEBUG] --------------OUTPUT_ROWS----------
    [2025-11-28 02:25:27.198 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
    [2025-11-28 02:25:27.200 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end

[2025-11-28 02:25:27.200 DEBUG] --------------DO_LOOKUP----------
[2025-11-28 02:25:27.200 DEBUG] ObDASLocalLookupIter::do_index_lookup()
[2025-11-28 02:25:27.200 DEBUG] ObDASFuncLookupIter::do_table_scan()
[2025-11-28 02:25:27.201 DEBUG] --------------OUTPUT_ROWS----------
[2025-11-28 02:25:27.201 DEBUG] --------------OUTPUT_ROWS: data_table_iter_->get_next_rows ----------
[2025-11-28 02:25:27.206 DEBUG] ObDASFuncLookupIter::do_index_lookup()
[2025-11-28 02:25:27.206 DEBUG] ObSRDaaTIterImpl::id_count: 2
[2025-11-28 02:25:27.206 DEBUG] ObTextDaaTIter::init
[2025-11-28 02:25:27.214 DEBUG] ObDASCacheLookupIter::inner_get_next_rows -----end


有关于 内层的 data_table_iter  是 ObDASScanIter.
目前来看：
这个嵌套的内容中：

外层: ObDASCacheLookupIter (最终结果获取)
├── index_table_iter_ → 内层 ObDASCacheLookupIter (全文检索+条件验证)  
│   ├── index_table_iter_ → ObDASTRMergeIter -> （ObISparseRetrievalMergeIter 的派生类）ObSRDaaTIterImpl (全文索引扫描)
│   └── data_table_iter_ → ObDASScanIter 
└── data_table_iter_ → ObDASFuncLookupIter (最终结果回表)



所以现在的目标是希望可以在内层的全文索引扫描的时候就把id给筛掉，这样子应该可以快一些。
所以希望可以把data_table_iter里面有关filter或者range的内容提取出来。

那么以下内容是ObDASScanIter中的成员变量
  common::ObITabletScan *tsc_service_;
  common::ObNewRowIterator *result_;
  // must ensure the lifecycle of scan param is longer than scan iter.
  storage::ObTableScanParam *scan_param_;

目前猜测：利用tsc_service_或者scan_param_,但是scan_param_貌似是要do_table_scan之后才初始化的？
      不，应该是猜测scan_param_在之前的某个时候就已经被设定好了，因为在ObDASScanIter中看到了set_scan_param的函数

    然后在scan_param里的基类里有如下变量：
      const sql::ObExprPtrIArray *op_filters_;
      sql::ObPushdownFilterExecutor *pd_storage_filters_;
        在它本身里还有key_range

所以目前的进度就是确定好data_table_iter里面有关filter或者range的内容，然后放进我们之前写好的ObSRDaaTIterImpl里的关于id的less的内容当中.

*****************************************2025-11-26 *********************************************************

[2025-11-26 14:56:08.944 DEBUG] === ObDDLBlockSampleScanOp::open called ===
[2025-11-26 14:56:09.372 DEBUG] === ObDDLBlockSampleScanOp::open called ===
[2025-11-26 14:56:09.715 DEBUG] === ObDDLBlockSampleScanOp::open called ===
[2025-11-26 14:56:10.117 DEBUG] === ObDDLBlockSampleScanOp::open called ===
[2025-11-26 14:56:10.341 DEBUG] === ObDDLBlockSampleScanOp::open called ===
[2025-11-26 14:56:10.342 DEBUG] Filter count: 0
[2025-11-26 14:56:10.342 DEBUG] Need final limit: 0
[2025-11-26 14:56:10.365 DEBUG] OB_TABLE_SCAN_OP: inner_get_next_fts_index_row
[2025-11-26 14:56:10.367 DEBUG] cached_min_pk_val_ 0
[2025-11-26 14:56:10.367 DEBUG] cached_max_pk_val_ 4282320384
[2025-11-26 14:56:10.367 DEBUG] MY_INPUT.key_ranges_.empty(): 0
[2025-11-26 14:56:10.439 DEBUG] have filter: 0
[2025-11-26 14:56:10.439 DEBUG] inner_get_next_fts_index_row::MY_INPUT.key_ranges_.empty() 0
[2025-11-26 14:56:10.564 DEBUG] have filter: 0
[2025-11-26 14:56:10.564 DEBUG] inner_get_next_fts_index_row::MY_INPUT.key_ranges_.empty() 0
[2025-11-26 14:56:10.695 DEBUG] have filter: 0
[2025-11-26 14:56:10.695 DEBUG] inner_get_next_fts_index_row::MY_INPUT.key_ranges_.empty() 0
[2025-11-26 14:56:10.761 DEBUG] have filter: 0



[2025-11-26 17:52:49.221 DEBUG] backup_key_ranges_.count(): 1
[2025-11-26 17:52:49.221 DEBUG] cached_min_pk_val_ 0
[2025-11-26 17:52:49.221 DEBUG] cached_max_pk_val_ 0
[2025-11-26 17:52:49.221 DEBUG] has_initialized_pk_range_ 0
[2025-11-26 17:52:49.224 DEBUG] cached_min_pk_val_ 0
[2025-11-26 17:52:49.224 DEBUG] cached_max_pk_val_ 0
[2025-11-26 17:52:49.224 DEBUG] MY_INPUT.key_ranges_.empty(): 0
[2025-11-26 17:52:49.306 DEBUG] have filter: 0
[2025-11-26 17:52:49.306 DEBUG] inner_get_next_fts_index_row::MY_INPUT.key_ranges_.empty() 0
[2025-11-26 17:52:49.415 DEBUG] have filter: 0
[2025-11-26 17:52:49.415 DEBUG] inner_get_next_fts_index_row::MY_INPUT.key_ranges_.empty() 0
[2025-11-26 17:52:49.508 DEBUG] have filter: 0
[2025-11-26 17:52:49.508 DEBUG] inner_get_next_fts_index_row::MY_INPUT.key_ranges_.empty() 0
[2025-11-26 17:52:49.601 DEBUG] have filter: 0
[2025-11-26 17:52:49.601 DEBUG] inner_get_next_fts_index_row::MY_INPUT.key_ranges_.empty() 0
[2025-11-26 17:52:49.663 DEBUG] have filter: 0


[2025-11-26 18:03:11.059 DEBUG] === ObDDLBlockSampleScanOp::open called ===
[2025-11-26 18:03:11.284 DEBUG] === ObDDLBlockSampleScanOp::open called ===
[2025-11-26 18:03:11.284 DEBUG] After prepare_scan_range, key_ranges count: 1
[2025-11-26 18:03:11.284 DEBUG] Filter count: 0
[2025-11-26 18:03:11.284 DEBUG] Need final limit: 0
[2025-11-26 18:03:11.306 DEBUG] OB_TABLE_SCAN_OP: inner_get_next_fts_index_row
[2025-11-26 18:03:11.306 DEBUG] === inner_get_next_fts_index_row ===
[2025-11-26 18:03:11.306 DEBUG] backup_key_ranges_.count(): 1
[2025-11-26 18:03:11.306 DEBUG] cached_min_pk_val_ 12378095747072
[2025-11-26 18:03:11.306 DEBUG] cached_max_pk_val_ 0
[2025-11-26 18:03:11.306 DEBUG] has_initialized_pk_range_ 0
[2025-11-26 18:03:11.306 DEBUG] range.get_end_key().get_obj_cnt() :1
[2025-11-26 18:03:11.306 DEBUG] end_obj.is_integer_type(): 0
[2025-11-26 18:03:11.308 DEBUG] cached_min_pk_val_ 12378095747072
[2025-11-26 18:03:11.308 DEBUG] cached_max_pk_val_ 0
[2025-11-26 18:03:11.308 DEBUG] MY_INPUT.key_ranges_.empty(): 0
[2025-11-26 18:03:11.382 DEBUG] have filter: 0
[2025-11-26 18:03:11.382 DEBUG] inner_get_next_fts_index_row::MY_INPUT.key_ranges_.empty() 0
[2025-11-26 18:03:11.522 DEBUG] have filter: 0