-- 测试UPDATE功能
CREATE DATABASE test_db;
USE test_db;

-- 创建测试表
CREATE TABLE students (
    id INT,
    name CHAR(20),
    age INT,
    birth_date DATE
);

-- 插入测试数据
INSERT INTO students VALUES (1, 'Alice', 20, '2003-01-01');
INSERT INTO students VALUES (2, 'Bob', 21, '2002-05-15');
INSERT INTO students VALUES (3, 'Charlie', 19, '2004-12-10');

-- 查看原始数据
SELECT * FROM students;

-- 测试UPDATE语句（更新所有记录）
UPDATE students SET age = 25;

-- 查看更新后的数据
SELECT * FROM students;

-- 测试UPDATE语句（更新字符串字段）
UPDATE students SET name = 'Updated';

-- 查看更新后的数据
SELECT * FROM students;

EXIT;
