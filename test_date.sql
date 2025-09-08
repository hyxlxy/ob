-- 测试DATE类型功能
CREATE DATABASE test_db;
USE test_db;

-- 创建包含DATE字段的表
CREATE TABLE events (
    id INT,
    name CHAR(50),
    event_date DATE
);

-- 插入测试数据
INSERT INTO events VALUES (1, 'New Year', '2024-01-01');
INSERT INTO events VALUES (2, 'Spring Festival', '2024-02-10');
INSERT INTO events VALUES (3, 'Labor Day', '2024-05-01');
INSERT INTO events VALUES (4, 'National Day', '2024-10-01');

-- 查询数据
SELECT * FROM events;

-- 测试日期比较
SELECT * FROM events WHERE event_date > '2024-03-01';

-- 测试日期排序
SELECT * FROM events ORDER BY event_date;

EXIT;
