-- 测试向量基础功能
-- 1. 测试创建包含向量类型的表
CREATE TABLE vectors (
    id INT,
    name CHAR(20),
    embedding VECTOR(3)
);

-- 查看表结构
DESC vectors;

-- 2. 测试插入向量数据
INSERT INTO vectors VALUES (1, 'vec1', '[1.0, 2.0, 3.0]');
INSERT INTO vectors VALUES (2, 'vec2', '[4.0, 5.0, 6.0]');
INSERT INTO vectors VALUES (3, 'vec3', '[1.5, 2.5, 3.5]');

-- 查看插入的数据
SELECT * FROM vectors;

-- 3. 测试向量算术运算（如果支持）
-- SELECT id, name, embedding + embedding FROM vectors;

-- 4. 测试向量距离函数（如果支持）
-- SELECT L2_DISTANCE('[1.0, 2.0, 3.0]', '[4.0, 5.0, 6.0]');

EXIT;
