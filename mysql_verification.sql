-- MySQL 8.0 验证脚本
-- 用于验证MiniOB的行为是否与MySQL 8.0一致

-- 1. COUNT函数NULL值处理测试
DROP TABLE IF EXISTS count_test;
CREATE TABLE count_test(id INT NOT NULL, name VARCHAR(20), birthday DATE);
INSERT INTO count_test VALUES(1, 'Alice', '2020-01-01');
INSERT INTO count_test VALUES(2, 'Bob', NULL);
INSERT INTO count_test VALUES(3, NULL, '2021-01-01');
INSERT INTO count_test VALUES(4, 'David', NULL);
INSERT INTO count_test VALUES(5, NULL, NULL);

SELECT COUNT(*) FROM count_test;
SELECT COUNT(name) FROM count_test;
SELECT COUNT(birthday) FROM count_test;

-- 2. 字段验证错误测试
DROP TABLE IF EXISTS select_meta;
CREATE TABLE select_meta(id INT, name VARCHAR(10));
INSERT INTO select_meta VALUES(1, 'test');

-- 这些应该返回错误
-- SELECT no_column FROM select_meta;
-- SELECT * FROM select_meta WHERE no_column='001';

-- 3. 表达式计算精度测试
DROP TABLE IF EXISTS exp_table;
CREATE TABLE exp_table(id INT, col1 INT, col2 INT, col3 FLOAT, col4 FLOAT);
INSERT INTO exp_table VALUES(3, 10, 5, 6.12, 400);

SELECT id,-(col2*(-1)+3)+(col4+7)*(col1+col3*5),(1+col2)*col3*1 
FROM exp_table 
WHERE -(col2*(-2)+2)+(col4+9)*(col1+col3*5) > (7+col2)*col3*1;

-- 4. 基本数据类型测试
DROP TABLE IF EXISTS basic_types;
CREATE TABLE basic_types(
    id INT PRIMARY KEY,
    name VARCHAR(50),
    age INT,
    score FLOAT,
    birth_date DATE,
    is_active BOOLEAN
);

INSERT INTO basic_types VALUES(1, 'Alice', 25, 95.5, '1998-01-01', TRUE);
INSERT INTO basic_types VALUES(2, 'Bob', NULL, 87.2, '1999-02-15', FALSE);
INSERT INTO basic_types VALUES(3, 'Charlie', 30, NULL, NULL, TRUE);

SELECT * FROM basic_types;
SELECT COUNT(*), COUNT(age), COUNT(score), COUNT(birth_date) FROM basic_types;

-- 5. 聚合函数测试
SELECT 
    COUNT(*) as total_count,
    COUNT(age) as age_count,
    SUM(age) as age_sum,
    AVG(age) as age_avg,
    MIN(age) as age_min,
    MAX(age) as age_max,
    SUM(score) as score_sum,
    AVG(score) as score_avg
FROM basic_types;

-- 6. ORDER BY测试
SELECT * FROM basic_types ORDER BY age;
SELECT * FROM basic_types ORDER BY age DESC;
SELECT * FROM basic_types ORDER BY score, age;

-- 7. WHERE条件测试
SELECT * FROM basic_types WHERE age > 25;
SELECT * FROM basic_types WHERE age IS NULL;
SELECT * FROM basic_types WHERE age IS NOT NULL;
SELECT * FROM basic_types WHERE name LIKE 'A%';

-- 8. GROUP BY测试
DROP TABLE IF EXISTS group_test;
CREATE TABLE group_test(
    department VARCHAR(20),
    employee VARCHAR(20),
    salary FLOAT
);

INSERT INTO group_test VALUES('IT', 'Alice', 5000);
INSERT INTO group_test VALUES('IT', 'Bob', 5500);
INSERT INTO group_test VALUES('HR', 'Charlie', 4500);
INSERT INTO group_test VALUES('HR', 'David', 4800);
INSERT INTO group_test VALUES('IT', 'Eve', NULL);

SELECT department, COUNT(*), COUNT(salary), AVG(salary) 
FROM group_test 
GROUP BY department;

-- 清理
DROP TABLE IF EXISTS count_test;
DROP TABLE IF EXISTS select_meta;
DROP TABLE IF EXISTS exp_table;
DROP TABLE IF EXISTS basic_types;
DROP TABLE IF EXISTS group_test;
