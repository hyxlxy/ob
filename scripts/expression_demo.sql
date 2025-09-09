-- Demo SQL for verifying arithmetic expressions in WHERE
-- Creates a sample table and runs checks for constant and column expressions

-- 1) Prepare demo table
DROP TABLE IF EXISTS exp_table;
CREATE TABLE exp_table(
  id   INT,
  col1 INT,
  col2 INT,
  col3 FLOAT,
  col4 FLOAT
);

-- 2) Insert sample rows (subset similar to your data)
INSERT INTO exp_table VALUES (1, 8, 5, 3.17, 3.42);
INSERT INTO exp_table VALUES (3, 3, 1, 3.55, 4.18);
INSERT INTO exp_table VALUES (3, 6, 2, 2.62, 3.42);
INSERT INTO exp_table VALUES (3, 7, 8, 5.17, 6.45);
INSERT INTO exp_table VALUES (3, 8, 3, 6.59, 4.34);
INSERT INTO exp_table VALUES (4, 6, 8, 5.86, 3.36);
INSERT INTO exp_table VALUES (6, 6, 8, 9.69, 8.07);
INSERT INTO exp_table VALUES (6, 6, 9, 3.03, 1.15);
INSERT INTO exp_table VALUES (9, 7, 3, 7.67, 7.86);

-- 3) Quick checks
-- 3.1 Arithmetic division semantics
SELECT 9/2;                 -- expect 4.5
SELECT 9 = 9/2;             -- expect 0 (false)

-- 3.2 WHERE with constant predicate: expect 0 rows
SELECT COUNT(*) FROM exp_table WHERE 9 = 9/2;  -- expect 0
SELECT *        FROM exp_table WHERE 9 = 9/2;  -- expect 0 rows

-- 3.3 WHERE with true constant: expect all rows
SELECT COUNT(*) FROM exp_table WHERE 9 = 9;    -- expect 9 (number of inserted rows)

-- 3.4 Column expressions in WHERE
SELECT COUNT(*) FROM exp_table WHERE col1 - 9 > 9;  -- should work without crash

EXIT;

