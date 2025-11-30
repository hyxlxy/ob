-- Plan check script: TR Block-Max TopK + Index Merge + session knobs (safe to re-run)

-- 0) Choose database
CREATE DATABASE IF NOT EXISTS test;
USE test;

-- 1) Clean & create table (minimal)
DROP TABLE IF EXISTS t_demo;
CREATE TABLE t_demo (
  id INT PRIMARY KEY,
  doc_content VARCHAR(4096),
  category_id INT,
  status INT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 2) Indexes: fulltext + scalar composite
CREATE FULLTEXT INDEX idx_ft ON t_demo(doc_content);
CREATE INDEX idx_cs ON t_demo(category_id, status, id);

-- 3) Seed data (small)
INSERT INTO t_demo VALUES
(1, 'oceanbase is a distributed database', 100, 1),
(2, 'oceanbase supports fulltext search', 100, 1),
(3, 'hello world oceanbase', 101, 1),
(4, 'oceanbase optimization', 100, 0),
(5, 'test oceanbase query', 100, 1),
(6, 'mysql database', 100, 1),
(7, 'oracle database', 101, 1);

ANALYZE TABLE t_demo;

-- 4) Two-phase BMW knobs (session variables)
SET @ob_tr_topk_oversample = 3;
SET @ob_tr_min_score_threshold = 0.0;
SELECT @ob_tr_topk_oversample AS over_sample, @ob_tr_min_score_threshold AS min_score_thres;

-- 5) Pure FTS TopK (should be no LOG_SORT; TR Block-Max handles TopN)
EXPLAIN EXTENDED
SELECT
  t.id,
  MATCH(doc_content) AGAINST('oceanbase database') AS _score
FROM t_demo t
WHERE MATCH(doc_content) AGAINST('oceanbase database')
ORDER BY _score DESC
LIMIT 5;

-- Run it
SELECT
  t.id,
  MATCH(doc_content) AGAINST('oceanbase database') AS _score
FROM t_demo t
WHERE MATCH(doc_content) AGAINST('oceanbase database')
ORDER BY _score DESC
LIMIT 5;

-- 6) Mixed baseline (no union merge)
EXPLAIN EXTENDED
SELECT
  t.id, t.category_id, t.status,
  MATCH(doc_content) AGAINST('oceanbase database') AS _score
FROM t_demo t
WHERE MATCH(doc_content) AGAINST('oceanbase database')
  AND t.category_id = 100
  AND t.status = 1
  AND t.id < 100000000
ORDER BY _score DESC
LIMIT 5;

-- 7) Mixed with UNION_MERGE variants
-- A) Most common (no QB binding)
EXPLAIN EXTENDED
SELECT /*+ UNION_MERGE(t idx_ft idx_cs) */
  t.id, t.category_id, t.status,
  MATCH(doc_content) AGAINST('oceanbase database') AS _score
FROM t_demo t
WHERE MATCH(doc_content) AGAINST('oceanbase database')
  AND t.category_id = 100
  AND t.status = 1
  AND t.id < 100000000
ORDER BY _score DESC
LIMIT 5;

-- B) With QB_NAME binding (no @ usage)
EXPLAIN EXTENDED
SELECT /*+ QB_NAME(SEL1) UNION_MERGE(SEL1 t idx_ft idx_cs) */
  t.id, t.category_id, t.status,
  MATCH(doc_content) AGAINST('oceanbase database') AS _score
FROM t_demo t
WHERE MATCH(doc_content) AGAINST('oceanbase database')
  AND t.category_id = 100
  AND t.status = 1
  AND t.id < 100000000
ORDER BY _score DESC
LIMIT 5;

-- C) Swap index order
EXPLAIN EXTENDED
SELECT /*+ UNION_MERGE(t idx_cs idx_ft) */
  t.id, t.category_id, t.status,
  MATCH(doc_content) AGAINST('oceanbase database') AS _score
FROM t_demo t
WHERE MATCH(doc_content) AGAINST('oceanbase database')
  AND t.category_id = 100
  AND t.status = 1
  AND t.id < 100000000
ORDER BY _score DESC
LIMIT 5;

-- D) Run one variant
SELECT /*+ UNION_MERGE(t idx_ft idx_cs) */
  t.id, t.category_id, t.status,
  MATCH(doc_content) AGAINST('oceanbase database') AS _score
FROM t_demo t
WHERE MATCH(doc_content) AGAINST('oceanbase database')
  AND t.category_id = 100
  AND t.status = 1
  AND t.id < 100000000
ORDER BY _score DESC
LIMIT 5;

