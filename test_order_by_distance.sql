-- 测试 ORDER BY DISTANCE 功能
-- 这个测试会暴露之前的 bug：左右 tuple 使用了相同的数据进行比较

-- 1. 创建向量表
DROP TABLE IF EXISTS TEST_VEC;
CREATE TABLE TEST_VEC(ID INT, NAME VARCHAR(20), VEC VECTOR(3));

-- 2. 插入测试数据（使用简单的 3 维向量便于手工验证）
-- 目标向量：[0, 0, 0]
-- 预期距离（欧几里得距离）：
--   ID=1: sqrt(1^2 + 0^2 + 0^2) = 1.0
--   ID=2: sqrt(2^2 + 0^2 + 0^2) = 2.0
--   ID=3: sqrt(3^2 + 0^2 + 0^2) = 3.0
--   ID=4: sqrt(0^2 + 4^2 + 0^2) = 4.0
--   ID=5: sqrt(5^2 + 0^2 + 0^2) = 5.0

INSERT INTO TEST_VEC VALUES(1, 'closest', STRING_TO_VECTOR('[1.0, 0.0, 0.0]'));
INSERT INTO TEST_VEC VALUES(2, 'second', STRING_TO_VECTOR('[2.0, 0.0, 0.0]'));
INSERT INTO TEST_VEC VALUES(3, 'third', STRING_TO_VECTOR('[3.0, 0.0, 0.0]'));
INSERT INTO TEST_VEC VALUES(4, 'fourth', STRING_TO_VECTOR('[0.0, 4.0, 0.0]'));
INSERT INTO TEST_VEC VALUES(5, 'farthest', STRING_TO_VECTOR('[5.0, 0.0, 0.0]'));

-- 3. 测试 ORDER BY DISTANCE（升序，应该返回距离最近的）
-- 预期结果：ID 依次为 1, 2, 3, 4, 5
SELECT ID, NAME FROM TEST_VEC 
ORDER BY DISTANCE(VEC, STRING_TO_VECTOR('[0.0, 0.0, 0.0]'), 'EUCLIDEAN') ASC;

-- 4. 测试 LIMIT 1（应该返回最近的，即 ID=1）
-- 预期结果：1
SELECT ID FROM TEST_VEC 
ORDER BY DISTANCE(VEC, STRING_TO_VECTOR('[0.0, 0.0, 0.0]'), 'EUCLIDEAN') ASC 
LIMIT 1;

-- 5. 测试降序（应该返回距离最远的）
-- 预期结果：ID 依次为 5, 4, 3, 2, 1
SELECT ID, NAME FROM TEST_VEC 
ORDER BY DISTANCE(VEC, STRING_TO_VECTOR('[0.0, 0.0, 0.0]'), 'EUCLIDEAN') DESC;

-- 6. 测试降序 LIMIT 1（应该返回最远的，即 ID=5）
-- 预期结果：5
SELECT ID FROM TEST_VEC 
ORDER BY DISTANCE(VEC, STRING_TO_VECTOR('[0.0, 0.0, 0.0]'), 'EUCLIDEAN') DESC 
LIMIT 1;

-- 7. 测试不同的目标向量
-- 目标向量：[2.0, 0.0, 0.0]
-- 预期距离：
--   ID=1: |2.0 - 1.0| = 1.0 (最近)
--   ID=2: |2.0 - 2.0| = 0.0 (最近！)
--   ID=3: |2.0 - 3.0| = 1.0
--   ID=4: sqrt((2-0)^2 + (0-4)^2) = sqrt(20) ≈ 4.47
--   ID=5: |2.0 - 5.0| = 3.0
-- 预期结果：2
SELECT ID FROM TEST_VEC 
ORDER BY DISTANCE(VEC, STRING_TO_VECTOR('[2.0, 0.0, 0.0]'), 'EUCLIDEAN') ASC 
LIMIT 1;

-- 8. 测试 COSINE 距离
-- 对于单位向量，余弦距离应该能正确排序
SELECT ID, NAME FROM TEST_VEC 
ORDER BY DISTANCE(VEC, STRING_TO_VECTOR('[1.0, 0.0, 0.0]'), 'COSINE') ASC;

-- 9. 清理
DROP TABLE TEST_VEC;

