-- 调试浮点数比较问题
DROP TABLE IF EXISTS test_float;

CREATE TABLE test_float(id int, val float);
INSERT INTO test_float VALUES (1, 10.5);
INSERT INTO test_float VALUES (2, 11.0);
INSERT INTO test_float VALUES (3, 11.5);

-- 测试浮点数比较
SELECT * FROM test_float WHERE val >= 11.0;

-- 测试具体的值
SELECT * FROM test_float WHERE val >= 10.5;

-- 测试边界情况
SELECT * FROM test_float WHERE val > 10.5;

DROP TABLE test_float;

