-- 测试JOIN功能
CREATE DATABASE test_db;
USE test_db;

-- 创建学生表
CREATE TABLE students (
    id INT,
    name CHAR(20),
    class_id INT
);

-- 创建班级表
CREATE TABLE classes (
    id INT,
    class_name CHAR(20)
);

-- 插入学生数据
INSERT INTO students VALUES (1, 'Alice', 1);
INSERT INTO students VALUES (2, 'Bob', 1);
INSERT INTO students VALUES (3, 'Charlie', 2);
INSERT INTO students VALUES (4, 'David', 2);

-- 插入班级数据
INSERT INTO classes VALUES (1, 'Math');
INSERT INTO classes VALUES (2, 'Science');

-- 查看原始数据
SELECT * FROM students;
SELECT * FROM classes;

-- 测试INNER JOIN
SELECT students.name, classes.class_name 
FROM students 
INNER JOIN classes ON students.class_id = classes.id;

-- 测试简化的JOIN语法
SELECT students.name, classes.class_name 
FROM students 
JOIN classes ON students.class_id = classes.id;

EXIT;
