-- 测试聚合函数
CREATE DATABASE test_db;
USE test_db;

CREATE TABLE students (
    id INT,
    name CHAR(20),
    age INT,
    score FLOAT
);

INSERT INTO students VALUES (1, 'Alice', 20, 85.5);
INSERT INTO students VALUES (2, 'Bob', 22, 78.0);
INSERT INTO students VALUES (3, 'Charlie', 19, 92.5);
INSERT INTO students VALUES (4, 'David', 21, 88.0);
INSERT INTO students VALUES (5, 'Eve', 20, 95.0);

-- 查看原始数据
SELECT * FROM students;

-- 测试COUNT(*)
SELECT COUNT(*) FROM students;

-- 测试COUNT(column)
SELECT COUNT(age) FROM students;

-- 测试SUM
SELECT SUM(score) FROM students;

-- 测试AVG
SELECT AVG(score) FROM students;

-- 测试MAX
SELECT MAX(score) FROM students;

-- 测试MIN
SELECT MIN(score) FROM students;

EXIT;
