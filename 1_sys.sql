DROP TABLE IF EXISTS test_insert_en;
CREATE TABLE test_insert_en (
id BIGINT PRIMARY KEY,
base_id VARCHAR(64),
docid_col BIGINT,
fulltext_col TEXT,
FULLTEXT KEY fts_fulltext_col(fulltext_col)
) CHARSET = utf8mb4;
INSERT INTO test_insert_en (id, base_id, docid_col, fulltext_col) VALUES
(1, 'base_id_1', 1001, 'PFOA was proposed for listing under the Stockholm Convention.'),
(2, 'base_id_2', 1002, 'Deutsche Bank sold its stake to Commerzbank in a given year.'),
(3, 'base_id_3', 1003, 'Seafood Watch was awarded by Bon Appetit magazine.');
-- 2) 基线：正确写法（显式 NATURAL LANGUAGE 模式，LIMIT 整型）应成功
-- 用于确认表与全文索引正常
SELECT
docid_col,
MATCH(fulltext_col) AGAINST('Stockholm Convention' IN NATURAL LANGUAGE MODE) AS score
FROM test_insert_en
WHERE MATCH(fulltext_col) AGAINST('Stockholm Convention' IN NATURAL LANGUAGE MODE)
AND base_id IN ('base_id_1','base_id_2','base_id_3')
AND id < 1000
ORDER BY score DESC
LIMIT 10;
-- 3) 复现A：缺失检索模式 + 含布尔操作符，通常触发 1210 Invalid argument
-- 说明：OceanBase 对不显式模式、且包含 + / - 等布尔操作符的查询，常以 Invalid argument 方式报错。
SELECT
docid_col,
MATCH(fulltext_col) AGAINST('+PFOA -Convention') AS score
FROM test_insert_en
WHERE MATCH(fulltext_col) AGAINST('+PFOA -Convention')
ORDER BY score DESC
LIMIT 10;
-- 4) 复现B：AGAINST(NULL)，参数为 NULL，触发 1210 Invalid argument
SELECT
docid_col,
MATCH(fulltext_col) AGAINST(NULL IN NATURAL LANGUAGE MODE) AS score
FROM test_insert_en
WHERE MATCH(fulltext_col) AGAINST(NULL IN NATURAL LANGUAGE MODE)
ORDER BY score DESC
LIMIT 10;
-- 5) 复现C：AGAINST('') 空查询串，有环境会报 1210（不同版本可能返回0分）
-- 若未报错，可跳过；此用例与系统参数/版本有关
SELECT
docid_col,
MATCH(fulltext_col) AGAINST('' IN NATURAL LANGUAGE MODE) AS score
FROM test_insert_en
WHERE MATCH(fulltext_col) AGAINST('' IN NATURAL LANGUAGE MODE)
ORDER BY score DESC
LIMIT 10;
-- 6) 复现D：LIMIT 非法类型（字符串 '20x'），触发 1210 Invalid argument
SELECT
docid_col,
MATCH(fulltext_col) AGAINST('Stockholm Convention' IN NATURAL LANGUAGE MODE) AS score
FROM test_insert_en
WHERE MATCH(fulltext_col) AGAINST('Stockholm Convention' IN NATURAL LANGUAGE MODE)
ORDER BY score DESC
LIMIT '20x';
-- 7) 复现E：预编译参数场景模拟（与应用最接近）
-- 用 PREPARE/EXECUTE 模拟 Python 占位符（%s）绑定类型错误的情况
-- 7.1 模拟 LIMIT 绑定为非法字符串
PREPARE stmt1 FROM
"SELECT docid_col,
MATCH(fulltext_col) AGAINST(? IN NATURAL LANGUAGE MODE) AS score
FROM test_insert_en
WHERE MATCH(fulltext_col) AGAINST(? IN NATURAL LANGUAGE MODE)
ORDER BY score DESC
LIMIT ?";
SET @q := 'What organization proposed listing PFOA under the Stockholm Convention';
SET @k := '20x'; -- 非法 LIMIT
EXECUTE stmt1 USING @q, @q, @k; -- 预期 1210 Invalid argument
DEALLOCATE PREPARE stmt1;
-- 7.2 模拟 AGAINST 绑定为 NULL
PREPARE stmt2 FROM
"SELECT docid_col,
MATCH(fulltext_col) AGAINST(? IN NATURAL LANGUAGE MODE) AS score
FROM test_insert_en
WHERE MATCH(fulltext_col) AGAINST(? IN NATURAL LANGUAGE MODE)
ORDER BY score DESC
LIMIT 10";
SET @q := NULL; -- 非法查询文本
EXECUTE stmt2 USING @q, @q; -- 预期 1210 Invalid argument
DEALLOCATE PREPARE stmt2;