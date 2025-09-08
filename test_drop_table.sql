-- 测试DROP TABLE功能
CREATE DATABASE test_db;
USE test_db;

-- 创建测试表
CREATE TABLE test_table (
    id INT,
    name CHAR(50),
    birth_date DATE
);

-- 插入一些数据
INSERT INTO test_table VALUES (1, 'Alice', '1990-01-01');
INSERT INTO test_table VALUES (2, 'Bob', '1985-05-15');

-- 查看表内容
SELECT * FROM test_table;

-- 创建索引
CREATE INDEX idx_name ON test_table(name);

-- 删除表
DROP TABLE test_table;

-- 尝试查询已删除的表（应该失败）
SELECT * FROM test_table;

EXIT;
