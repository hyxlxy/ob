-- 调试数据测试
DROP TABLE IF EXISTS csq_2, csq_3;

CREATE TABLE csq_2(id int, col2 int, feat2 float);
INSERT INTO csq_2 VALUES (1, 2, 13.0);
INSERT INTO csq_2 VALUES (2, 7, 10.5);
INSERT INTO csq_2 VALUES (5, 3, 12.6);

CREATE TABLE csq_3(id int, col3 int, feat3 float);
INSERT INTO csq_3 VALUES (1, 2, 11.0);
INSERT INTO csq_3 VALUES (3, 6, 16.5);
INSERT INTO csq_3 VALUES (5, 5, 14.6);

-- 查看所有数据
SELECT * FROM csq_2;
SELECT * FROM csq_3;

-- 测试条件
SELECT * FROM csq_2 WHERE feat2 >= 11.0;

-- 测试聚合
SELECT MAX(col2) FROM csq_2 WHERE feat2 >= 11.0;

-- 测试嵌套子查询
SELECT MAX(col2) FROM csq_2 WHERE feat2 >= (SELECT MIN(feat3) FROM csq_3);

DROP TABLE csq_2;
DROP TABLE csq_3;

