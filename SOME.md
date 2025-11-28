
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