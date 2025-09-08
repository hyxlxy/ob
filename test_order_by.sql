-- 测试ORDER BY功能
-- 1. 创建数据库和表
CREATE DATABASE test_db;
USE test_db;

CREATE TABLE students (
    id INT,
    name CHAR(20),
    age INT,
    score FLOAT
);

-- 2. 插入测试数据
INSERT INTO students VALUES (1, 'Alice', 20, 85.5);
INSERT INTO students VALUES (2, 'Bob', 22, 78.0);
INSERT INTO students VALUES (3, 'Charlie', 19, 92.5);
INSERT INTO students VALUES (4, 'David', 21, 88.0);
INSERT INTO students VALUES (5, 'Eve', 20, 95.0);

-- 3. 查看原始数据
SELECT * FROM students;

-- 4. 测试基本ORDER BY（升序）
SELECT * FROM students ORDER BY age;

-- 5. 测试ORDER BY ASC（显式升序）
SELECT * FROM students ORDER BY score ASC;

-- 6. 测试ORDER BY DESC（降序）
SELECT * FROM students ORDER BY score DESC;

-- 7. 测试多字段排序
SELECT * FROM students ORDER BY age, score DESC;

-- 8. 测试ORDER BY与投影结合
SELECT name, score FROM students ORDER BY score DESC;

EXIT;
