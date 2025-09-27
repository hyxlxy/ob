-- 创建测试表
CREATE TABLE csq_1(id int, col1 int, feat1 float);
CREATE TABLE csq_2(id int, col2 int, feat2 float);
CREATE TABLE csq_3(id int, col3 int, feat3 float);

-- 插入数据
INSERT INTO csq_1 VALUES (1, 4, 11.2);
INSERT INTO csq_1 VALUES (2, 2, 12.0);
INSERT INTO csq_1 VALUES (3, 3, 13.5);
INSERT INTO csq_1 VALUES (55, 41, 79.73);

INSERT INTO csq_2 VALUES (1, 2, 13.0);
INSERT INTO csq_2 VALUES (2, 7, 10.5);
INSERT INTO csq_2 VALUES (5, 3, 12.6);

INSERT INTO csq_3 VALUES (1, 2, 11.0);
INSERT INTO csq_3 VALUES (3, 6, 16.5);
INSERT INTO csq_3 VALUES (5, 5, 14.6);

-- 步骤1：测试最内层子查询
select 'MIN(feat3) from csq_3:' as test, min(feat3) as result from csq_3;

-- 步骤2：测试中间层子查询
select 'MAX(col2) from csq_2 where feat2 >= 11.0:' as test, max(col2) as result from csq_2 where feat2 >= 11.0;

-- 步骤3：测试完整查询
select 'Complex subquery result:' as test, id, col1, feat1 from csq_1 where col1 > (select max(csq_2.col2) from csq_2 where csq_2.feat2 >= (select min(csq_3.feat3) from csq_3));

-- 步骤4：测试简单子查询
CREATE TABLE ssq_1(id int, col1 int);
CREATE TABLE ssq_2(id int, col2 int);

INSERT INTO ssq_1 VALUES (1, 10);
INSERT INTO ssq_1 VALUES (2, 20);
INSERT INTO ssq_1 VALUES (3, 30);

INSERT INTO ssq_2 VALUES (1, 15);
INSERT INTO ssq_2 VALUES (2, 34);
INSERT INTO ssq_2 VALUES (3, 25);

select 'Simple subquery result:' as test, id, col1 from ssq_1 where id = (select ssq_2.id from ssq_2 where col2 = 34);
