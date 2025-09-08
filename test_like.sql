-- 测试LIKE功能
CREATE DATABASE test_db;
USE test_db;

-- 创建测试表
CREATE TABLE users (
    id INT,
    name CHAR(20),
    email CHAR(50)
);

-- 插入测试数据
INSERT INTO users VALUES (1, 'Alice', 'alice@example.com');
INSERT INTO users VALUES (2, 'Bob', 'bob@test.org');
INSERT INTO users VALUES (3, 'Charlie', 'charlie@example.com');
INSERT INTO users VALUES (4, 'David', 'david@company.net');
INSERT INTO users VALUES (5, 'Eve', 'eve@test.org');

-- 查看所有数据
SELECT * FROM users;

-- 测试LIKE操作 - 以'A'开头的名字
SELECT * FROM users WHERE name LIKE 'A%';

-- 测试LIKE操作 - 包含'li'的名字
SELECT * FROM users WHERE name LIKE '%li%';

-- 测试LIKE操作 - 以'e'结尾的名字
SELECT * FROM users WHERE name LIKE '%e';

-- 测试LIKE操作 - 单字符通配符
SELECT * FROM users WHERE name LIKE '_ob';

-- 测试LIKE操作 - email域名匹配
SELECT * FROM users WHERE email LIKE '%@example.com';

-- 测试LIKE操作 - 复杂模式
SELECT * FROM users WHERE email LIKE '%@%.org';

EXIT;
