-- Test view execution issue
create database test_db;
use test_db;

-- Create a simple table
CREATE TABLE test_table(id int, name char(10));
INSERT INTO test_table VALUES (1, 'Alice');
INSERT INTO test_table VALUES (2, 'Bob');

-- Create a view
CREATE VIEW test_view AS SELECT id, name FROM test_table;

-- Try to query the view (this should cause POLLHUP error)
SELECT * FROM test_view;
