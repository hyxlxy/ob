-- 测试各种表达式的解析
CREATE TABLE test_table(id int, col1 int, col2 int);
INSERT INTO test_table VALUES (1, 5, 3);

-- 测试简单的负数
select * from test_table where -1 < col1;

-- 测试负零
select * from test_table where -0 < col1;

-- 测试算术表达式
select * from test_table where 0 < col1 - col2;

-- 测试组合表达式
select * from test_table where -0 < col1 - col2;
