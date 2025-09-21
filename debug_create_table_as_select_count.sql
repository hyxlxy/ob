-- Test CREATE TABLE AS SELECT row count issue
CREATE TABLE test_source(id int, name char(10));

-- Insert exactly 5 rows
INSERT INTO test_source VALUES (1, 'Alice');
INSERT INTO test_source VALUES (2, 'Bob');
INSERT INTO test_source VALUES (3, 'Charlie');
INSERT INTO test_source VALUES (4, 'David');
INSERT INTO test_source VALUES (5, 'Eve');

-- Verify source table has 5 rows
SELECT COUNT(*) FROM test_source;

-- Create table as select
CREATE TABLE test_target AS SELECT * FROM test_source;

-- Check if target table has the same count (should be 5, not 6)
SELECT COUNT(*) FROM test_target;

-- Show all rows to verify
SELECT * FROM test_target;

-- Clean up
DROP TABLE test_target;
DROP TABLE test_source;
