-- ============================================
-- Index Merge 功能测试用例
-- 测试场景：验证 OR 条件下多个索引的合并（包括 FTS 索引）
-- 
-- 重要说明：
-- 1. Index Merge 目前只支持 OR 条件（UNION MERGE），不支持 AND 条件
-- 2. 每个 OR 分支需要能匹配到不同的索引
-- 3. FTS 索引需要 MATCH...AGAINST 表达式
-- ============================================

-- 1. 创建测试表（包含多个索引）
DROP TABLE IF EXISTS test_index_merge;
CREATE TABLE test_index_merge (id BIGINT PRIMARY KEY,base_id VARCHAR(100), category VARCHAR(50), fulltext_col TEXT, docid_col BIGINT, INDEX idx_base_id (base_id), INDEX idx_category (category), FULLTEXT INDEX ft_fulltext (fulltext_col));

-- 2. 插入更多测试数据（确保索引有效）
INSERT INTO test_index_merge VALUES (1, 'base_id_1', 'tech', 'machine learning deep neural networks', 1001), (2, 'base_id_1', 'science', 'quantum computing algorithms', 1002), (3, 'base_id_2', 'tech', 'artificial intelligence deep learning', 1003), (4, 'base_id_2', 'science', 'machine learning algorithms', 1004), (5, 'base_id_3', 'tech', 'neural networks deep learning', 1005), (6, 'base_id_3', 'science', 'quantum algorithms machine learning', 1006), (7, 'base_id_1', 'tech', 'deep learning neural networks', 1007), (8, 'base_id_2', 'science', 'machine learning quantum computing', 1008), (9, 'base_id_1', 'tech', 'artificial intelligence machine learning', 1009), (10, 'base_id_2', 'science', 'deep learning quantum computing', 1010), (11, 'base_id_3', 'tech', 'neural networks algorithms', 1011), (12, 'base_id_3', 'science', 'machine learning deep neural', 1012);

-- 3. 启用强制 index merge（用于测试）
ALTER SYSTEM set_tp tp_name = EN_FORCE_INDEX_MERGE_PLAN, error_code = 0, frequency = 1;

-- ============================================
-- 测试用例 1: OR 条件 + FTS 索引 + 普通索引
-- 预期：应该使用 index merge，合并 FTS 索引和 base_id 索引
-- 注意：每个 OR 分支必须是独立的简单条件
-- ============================================
SELECT '=== 测试用例 1: FTS OR base_id ===' as test_case;
EXPLAIN SELECT docid_col, MATCH(fulltext_col) AGAINST('machine learning') as _score FROM test_index_merge WHERE MATCH(fulltext_col) AGAINST('machine learning') OR base_id = 'base_id_1';

-- ============================================
-- 测试用例 2: OR 条件 + 多个普通索引
-- 预期：应该使用 index merge，合并 base_id 和 category 索引
-- 注意：每个分支只包含一个索引列的条件
-- ============================================
SELECT '=== 测试用例 2: base_id OR category ===' as test_case;
EXPLAIN SELECT * FROM test_index_merge WHERE base_id = 'base_id_1' OR category = 'tech';

-- ============================================
-- 测试用例 3: OR 条件 + FTS 索引 + 多个普通索引
-- 预期：应该使用 index merge，合并 FTS、base_id 和 category 索引
-- ============================================
SELECT '=== 测试用例 3: FTS OR base_id OR category ===' as test_case;
EXPLAIN SELECT docid_col, MATCH(fulltext_col) AGAINST('deep learning') as _score FROM test_index_merge WHERE MATCH(fulltext_col) AGAINST('deep learning') OR base_id = 'base_id_2' OR category = 'science';

-- ============================================
-- 测试用例 4: 纯 FTS OR 条件（多个不同的 FTS 查询）
-- 注意：这个可能不会触发 index merge，因为通常一个表只有一个 FTS 索引
-- 但如果有多个 FTS 索引，应该可以触发
-- ============================================
SELECT '=== 测试用例 4: FTS OR FTS (可能不触发) ===' as test_case;
EXPLAIN SELECT docid_col FROM test_index_merge WHERE MATCH(fulltext_col) AGAINST('machine learning') OR MATCH(fulltext_col) AGAINST('quantum computing');

-- ============================================
-- 测试用例 5: 多个 base_id OR 条件
-- 预期：应该使用 index merge（同一个索引的不同值）
-- ============================================
SELECT '=== 测试用例 5: base_id OR base_id OR base_id ===' as test_case;
EXPLAIN SELECT * FROM test_index_merge WHERE base_id = 'base_id_1' OR base_id = 'base_id_2' OR base_id = 'base_id_3';

-- ============================================
-- 测试用例 6: FTS + 主键范围 OR
-- 预期：可能使用 index merge（FTS 索引 + 主键索引）
-- 注意：主键索引通常也会被考虑
-- ============================================
SELECT '=== 测试用例 6: FTS OR 主键范围 ===' as test_case;
EXPLAIN SELECT docid_col, MATCH(fulltext_col) AGAINST('neural networks') as _score FROM test_index_merge WHERE MATCH(fulltext_col) AGAINST('neural networks') OR id BETWEEN 1 AND 5;

-- ============================================
-- 测试用例 6A: AND 条件 + FTS 索引 + 普通索引（无 ORDER BY/LIMIT）
-- 目的：验证 FTS AND base_id 是否可以生成 INTERSECT 型 index_merge
-- ============================================
SELECT '=== 测试用例 6A: FTS AND base_id ===' as test_case;
EXPLAIN SELECT docid_col, MATCH(fulltext_col) AGAINST('machine learning') as _score FROM test_index_merge WHERE MATCH(fulltext_col) AGAINST('machine learning') AND base_id = 'base_id_1';

-- ============================================
-- 测试用例 6B: AND 条件 + FTS 索引 + 普通索引 + 主键范围
-- 目的：验证 FTS AND base_id AND id 范围下的 index_merge（INTERSECT）
-- ============================================
SELECT '=== 测试用例 6B: FTS AND base_id AND id range ===' as test_case;
EXPLAIN SELECT docid_col, MATCH(fulltext_col) AGAINST('machine learning') as _score FROM test_index_merge WHERE MATCH(fulltext_col) AGAINST('machine learning') AND base_id IN ('base_id_1', 'base_id_2') AND id < 10;

-- ============================================
-- 禁用强制 index merge（恢复默认行为）
-- ============================================
ALTER SYSTEM set_tp tp_name = EN_FORCE_INDEX_MERGE_PLAN, error_code = 0, frequency = 0;

-- ============================================
-- 测试用例 7: 对比测试 - 不使用强制 index merge
-- ============================================
SELECT '=== 测试用例 7: 对比测试（禁用强制 index merge）===' as test_case;
EXPLAIN SELECT docid_col, MATCH(fulltext_col) AGAINST('machine learning') as _score FROM test_index_merge WHERE MATCH(fulltext_col) AGAINST('machine learning') OR base_id = 'base_id_1';

-- ============================================
-- 测试用例 8: 验证强制 index merge 是否生效
-- 重新启用强制 index merge，然后执行相同的查询
-- ============================================
SELECT '=== 测试用例 8: 重新启用强制 index merge ===' as test_case;
ALTER SYSTEM set_tp tp_name = EN_FORCE_INDEX_MERGE_PLAN, error_code = 0, frequency = 1;

EXPLAIN SELECT docid_col, MATCH(fulltext_col) AGAINST('machine learning') as _score FROM test_index_merge WHERE MATCH(fulltext_col) AGAINST('machine learning') OR base_id = 'base_id_1';

-- ============================================
-- 清理
-- ============================================
-- 禁用强制 index merge
ALTER SYSTEM set_tp tp_name = EN_FORCE_INDEX_MERGE_PLAN, error_code = 0, frequency = 0;

-- 删除测试表（可选）
-- DROP TABLE IF EXISTS test_index_merge;
