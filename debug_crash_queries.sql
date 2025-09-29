-- 测试相关子查询崩溃问题
CREATE TABLE csq_1(id int, col1 int, feat1 float);
INSERT INTO csq_1 VALUES (1, 10, 5.5);
INSERT INTO csq_1 VALUES (2, 20, 15.5);
INSERT INTO csq_1 VALUES (3, 30, 25.5);

CREATE TABLE csq_2(id int, feat2 float);
INSERT INTO csq_2 VALUES (1, 10.0);
INSERT INTO csq_2 VALUES (2, 20.0);
INSERT INTO csq_2 VALUES (3, 30.0);

-- 这个查询应该工作，但可能导致崩溃
SELECT * FROM csq_1 WHERE feat1 <> (SELECT MIN(csq_2.feat2) FROM csq_2 WHERE csq_2.feat2 > csq_1.feat1);

-- 测试IN查询崩溃问题
CREATE TABLE ssq_1(id int, col1 int);
INSERT INTO ssq_1 VALUES (1, 10);
INSERT INTO ssq_1 VALUES (2, 20);
INSERT INTO ssq_1 VALUES (3, 30);

-- 这个查询应该工作，但可能导致崩溃
SELECT * FROM ssq_1 WHERE id IN (1, 2, 3);

DROP TABLE csq_1;
DROP TABLE csq_2;
DROP TABLE ssq_1;
