-- 1) 创建表 test_insert_en（若存在先删除）
DROP TABLE IF EXISTS test_insert_en;

CREATE TABLE test_insert_en (
  id BIGINT AUTO_INCREMENT,
  base_id VARCHAR(255) NOT NULL,
  docid_col VARCHAR(255) NOT NULL,
  fulltext_col LONGTEXT,
  PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 普通索引
CREATE INDEX idx_base_id_en ON test_insert_en(base_id);
CREATE INDEX idx_docid_en ON test_insert_en(docid_col);

-- 全文索引
CREATE FULLTEXT INDEX ft_fulltext_en ON test_insert_en(fulltext_col);

-- 插入一些测试数据（包含 test 关键词）
INSERT INTO test_insert_en (base_id, docid_col, fulltext_col) VALUES
('base_id_1', 'doc_001', 'this is a test document about payment fail'),
('base_id_2', 'doc_002', 'another test article focusing on search performance'),
('base_id_3', 'doc_003', 'irrelevant content no keyword'),
('base_id_1', 'doc_004', 'fulltext test case with multiple test words test'),
('base_id_2', 'doc_005', 'random words and a test token'),
('base_id_3', 'doc_006', 'something else not a match'),
('base_id_1', 'doc_007', 'test appears here'),
('base_id_2', 'doc_008', 'speed up bm25 test evaluation'),
('base_id_3', 'doc_009', 'no test here'),
('base_id_1', 'doc_010', 'final test row');

-- 2) 同样创建一张 items1（方便对照赛题表名测试）
DROP TABLE IF EXISTS items1;

CREATE TABLE items1 (
  id BIGINT AUTO_INCREMENT,
  base_id VARCHAR(255) NOT NULL,
  docid_col VARCHAR(255) NOT NULL,
  fulltext_col LONGTEXT,
  PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE INDEX idx_base_id ON items1(base_id);
CREATE INDEX idx_docid ON items1(docid_col);
CREATE FULLTEXT INDEX ft_fulltext ON items1(fulltext_col);

-- 拷贝一份数据到 items1
INSERT INTO items1 (base_id, docid_col, fulltext_col)
SELECT base_id, docid_col, fulltext_col FROM test_insert_en;

-- 3) 检查 FULLTEXT 索引是否就绪（可选）
-- SHOW INDEX FROM test_insert_en;
-- SHOW INDEX FROM items1;

-- 4) 执行 EXPLAIN 与实际查询（使用 test_insert_en 例子）
EXPLAIN
SELECT
  docid_col,
  MATCH(fulltext_col) AGAINST('test') AS score
FROM test_insert_en
WHERE MATCH(fulltext_col) AGAINST('test')
  AND base_id IN ('base_id_1','base_id_2','base_id_3')
  AND id < 1000
ORDER BY score DESC
LIMIT 10;

SELECT
  docid_col,
  MATCH(fulltext_col) AGAINST('test') AS score
FROM test_insert_en
WHERE MATCH(fulltext_col) AGAINST('test')
  AND base_id IN ('base_id_1','base_id_2','base_id_3')
  AND id < 1000
ORDER BY score DESC
LIMIT 10;