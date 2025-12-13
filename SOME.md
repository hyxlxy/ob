*****************************************2025-12-5(lingchen) *********************************************************

目前进度：
 实现出了index_merge，但是有错误，召回率低。

发现，当不是混合检索的时候，是没有问题的。
也就是说，where条件里面，如果只有全文，或者全是标量筛选，是正确的。





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