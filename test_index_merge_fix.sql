-- OceanBase Index Merge FTS修复验证SQL
-- 测试纯FTS查询（应该正常，作为对照）

EXPLAIN 
SELECT * FROM articles 
WHERE MATCH(content) AGAINST('AI技术')
ORDER BY relevance_score DESC 
LIMIT 10;

-- 期望输出：TEXT RETRIEVAL SCAN with sort_keys, no TOP-N SORT

-- ================================================================

-- 测试混合查询（修复前：TOP-N SORT + TEXT RETRIEVAL）
-- 期望修复后：INDEX MERGE (INTERSECT) + 子节点

EXPLAIN 
/*+ INDEX(articles idx_ft idx_cs) UNION_MERGE(articles idx_ft idx_cs) */
SELECT * FROM articles 
WHERE MATCH(content) AGAINST('AI技术')
  AND status = 'published'
  AND create_time >= '2023-01-01 00:00:00'
ORDER BY relevance_score DESC 
LIMIT 10;

-- 期望输出修复后：
-- ├─ TOP-N SORT  (暂时保留，阶段2再消除)
-- └─ INDEX MERGE (INTERSECT)
--    ├─ TEXT RETRIEVAL SCAN (idx_ft)
--    └─ TABLE SCAN (idx_cs)

-- ================================================================

-- 检查Used Hint是否被采用

EXPLAIN FORMAT=JSON
/*+ INDEX(articles idx_ft idx_cs) UNION_MERGE(articles idx_ft idx_cs) */
SELECT * FROM articles 
WHERE MATCH(content) AGAINST('AI技术')
  AND status = 'published'
ORDER BY relevance_score DESC 
LIMIT 10;

-- 期望JSON中包含：
-- "Used_Hint": ["INDEX(articles idx_ft idx_cs)", "UNION_MERGE(articles idx_ft idx_cs)"]

-- ================================================================

-- 测试OR场景（UNION）

EXPLAIN
/*+ INDEX(articles idx_ft idx_cs) UNION_MERGE(articles idx_ft idx_cs) */
SELECT * FROM articles
WHERE MATCH(content) AGAINST('AI')
   OR (status = 'published' AND create_time >= '2024-01-01')
ORDER BY relevance_score DESC
LIMIT 10;

-- 期望输出：
-- └─ INDEX MERGE (UNION)
--    ├─ TEXT RETRIEVAL SCAN
--    └─ TABLE SCAN

-- ================================================================

-- 验证索引可用性

SHOW INDEX FROM articles;

-- 期望输出包含：
-- idx_ft (FULLTEXT)
-- idx_cs (普通复合索引)

