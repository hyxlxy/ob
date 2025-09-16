-- 测试子查询解析问题
-- 首先创建测试数据
CREATE TABLE ssq_1(id int, col1 int, feat1 float);
CREATE TABLE ssq_2(id int, col2 int, feat2 float);

INSERT INTO ssq_1 VALUES (16, 51, 62.18);
INSERT INTO ssq_1 VALUES (17, 52, 63.19);
INSERT INTO ssq_2 VALUES (1, 86, 1.1);
INSERT INTO ssq_2 VALUES (2, 87, 2.2);

-- 测试简单的子查询（这个应该工作）
select ssq_2.col2 from ssq_2;

-- 测试有问题的子查询
select * from ssq_1 where col1 = (select ssq_2.col2 from ssq_2);
