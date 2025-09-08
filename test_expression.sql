-- 测试表达式功能
-- 1. 测试基本算术表达式
SELECT 1 + 2;
SELECT 10 - 3;
SELECT 4 * 5;
SELECT 20 / 4;
SELECT -5;

-- 2. 测试复杂表达式
SELECT 1 + 2 * 3;
SELECT (1 + 2) * 3;
SELECT 10 / 2 + 3 * 4;

-- 3. 测试浮点数表达式
SELECT 3.14 * 2;
SELECT 10.5 / 2.5;

-- 4. 测试多个表达式
SELECT 1 + 1, 2 * 3, 10 / 2;

-- 5. 测试CALC语句
CALC 1 + 2 * 3;
CALC (10 + 5) / 3;
CALC -7 + 3;

-- 6. 测试表中的表达式
CREATE TABLE numbers (a INT, b INT, c FLOAT);
INSERT INTO numbers VALUES (10, 5, 3.14);
INSERT INTO numbers VALUES (20, 8, 2.71);

-- 查看原始数据
SELECT * FROM numbers;

-- 测试表字段的表达式
SELECT a, b, a + b FROM numbers;
SELECT a, b, a - b, a * b, a / b FROM numbers;
SELECT a, c, a * c FROM numbers;

EXIT;
