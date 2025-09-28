-- 调试csq_2数据
DROP TABLE IF EXISTS csq_2;

CREATE TABLE csq_2(id int, col2 int, feat2 float);
INSERT INTO csq_2 VALUES (1, 2, 13.0);
INSERT INTO csq_2 VALUES (2, 7, 10.5);
INSERT INTO csq_2 VALUES (5, 3, 12.6);

-- 查看所有数据
SELECT * FROM csq_2;

-- 测试不同的比较条件
SELECT * FROM csq_2 WHERE feat2 >= 10.5;
SELECT * FROM csq_2 WHERE feat2 >= 11.0;
SELECT * FROM csq_2 WHERE feat2 > 10.5;

-- 测试聚合
SELECT MAX(col2) FROM csq_2 WHERE feat2 >= 10.5;
SELECT MAX(col2) FROM csq_2 WHERE feat2 >= 11.0;

-- 测试具体的值比较
SELECT * FROM csq_2 WHERE feat2 = 10.5;

DROP TABLE csq_2;

