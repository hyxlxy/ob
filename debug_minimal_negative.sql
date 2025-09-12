-- 最小的负数表达式测试
create table minimal_test(id int, val int);
insert into minimal_test VALUES (1, 5);

-- 测试1: 基础查询
select * from minimal_test;

-- 测试2: SELECT 中的负数（应该工作）
select -val from minimal_test;

-- 测试3: WHERE 中的字段负数（目前失败）
select id from minimal_test where -val < 0;
