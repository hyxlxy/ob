-- 最小化测试用例：复现 ORDER BY DISTANCE 的 bug
-- 问题：compare_with_schema 中左右 tuple 使用了相同的数据

-- 创建表
DROP TABLE IF EXISTS VEC_TEST;
CREATE TABLE VEC_TEST(ID INT, V VECTOR(2));

-- 插入 3 条数据
-- 目标向量：[0, 0]
-- 距离计算（欧几里得）：
--   ID=10: sqrt(1^2 + 0^2) = 1.0  ← 最近
--   ID=20: sqrt(2^2 + 0^2) = 2.0  ← 第二近
--   ID=30: sqrt(3^2 + 0^2) = 3.0  ← 最远
INSERT INTO VEC_TEST VALUES(10, STRING_TO_VECTOR('[1.0, 0.0]'));
INSERT INTO VEC_TEST VALUES(20, STRING_TO_VECTOR('[2.0, 0.0]'));
INSERT INTO VEC_TEST VALUES(30, STRING_TO_VECTOR('[3.0, 0.0]'));

-- 测试 1：返回最近的一个（应该是 ID=10）
-- 如果有 bug，可能返回错误的 ID
SELECT ID FROM VEC_TEST 
ORDER BY DISTANCE(V, STRING_TO_VECTOR('[0.0, 0.0]'), 'EUCLIDEAN') ASC 
LIMIT 1;
-- 预期输出：10
-- Bug 输出：可能是 20 或 30（因为比较逻辑错误）

-- 测试 2：返回所有记录（应该按距离升序：10, 20, 30）
SELECT ID FROM VEC_TEST 
ORDER BY DISTANCE(V, STRING_TO_VECTOR('[0.0, 0.0]'), 'EUCLIDEAN') ASC;
-- 预期输出：
-- 10
-- 20
-- 30
-- Bug 输出：顺序可能混乱

-- 测试 3：降序（应该是 30, 20, 10）
SELECT ID FROM VEC_TEST 
ORDER BY DISTANCE(V, STRING_TO_VECTOR('[0.0, 0.0]'), 'EUCLIDEAN') DESC;
-- 预期输出：
-- 30
-- 20
-- 10

-- 清理
DROP TABLE VEC_TEST;

