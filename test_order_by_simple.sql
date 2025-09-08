-- 简单的ORDER BY测试
CREATE DATABASE test_db;
USE test_db;

CREATE TABLE test (
    id INT,
    name CHAR(10)
);

INSERT INTO test VALUES (3, 'Charlie');
INSERT INTO test VALUES (1, 'Alice');
INSERT INTO test VALUES (2, 'Bob');

SELECT * FROM test;
SELECT * FROM test ORDER BY id;
SELECT * FROM test ORDER BY name;

EXIT;
