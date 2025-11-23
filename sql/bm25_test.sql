SET NAMES utf8mb4;
USE test;

CREATE TABLE IF NOT EXISTS items1 (
  id BIGINT AUTO_INCREMENT,
  base_id VARCHAR(255) NOT NULL,
  docid_col VARCHAR(255) NOT NULL,
  fulltext_col LONGTEXT,
  PRIMARY KEY(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE INDEX idx_base_id ON items1(base_id);
CREATE INDEX idx_docid ON items1(docid_col);
CREATE /*+ parallel(90) */ FULLTEXT INDEX ft_fulltext ON items1(fulltext_col);

SELECT docid_col,
       MATCH(fulltext_col) AGAINST('payment fail' IN NATURAL LANGUAGE MODE) AS _score
FROM items1
WHERE MATCH(fulltext_col) AGAINST('payment fail' IN NATURAL LANGUAGE MODE)
  AND base_id IN ('base_id_1','base_id_2','base_id_3')
  AND id < 1000000
ORDER BY _score DESC
LIMIT 10;