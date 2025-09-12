-- 简单的COUNT测试
create table simple_count_test(id int);
insert into simple_count_test VALUES (1);
insert into simple_count_test VALUES (2);

-- 测试1: 基础查询
select * from simple_count_test;

-- 测试2: COUNT(*)
select count(*) from simple_count_test;
