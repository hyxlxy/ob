#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

"""
MiniOB测试用例生成工具
根据MiniOB题目.txt中的功能要求，自动生成完整的测试用例
"""

import os
import sys
from typing import List, Dict, Tuple

class TestCaseGenerator:
    """测试用例生成器"""
    
    def __init__(self, output_dir: str = "test/case"):
        self.output_dir = output_dir
        self.test_dir = os.path.join(output_dir, "test")
        self.result_dir = os.path.join(output_dir, "result")
        
        # 确保目录存在
        os.makedirs(self.test_dir, exist_ok=True)
        os.makedirs(self.result_dir, exist_ok=True)
    
    def generate_date_tests(self):
        """生成日期类型测试用例"""
        test_content = """-- echo Date Type Tests
-- 测试日期类型的基本功能

-- 创建包含日期字段的表
create table date_test(id int, birth_date date, name char(20));

-- 测试正常日期插入
insert into date_test values(1, '2023-01-01', 'Alice');
insert into date_test values(2, '1990-12-31', 'Bob');
insert into date_test values(3, '2000-02-29', 'Charlie'); -- 闰年2月29日

-- 测试边界日期
insert into date_test values(4, '1900-01-01', 'David');
insert into date_test values(5, '2099-12-31', 'Eve');

-- 测试非法日期（应该返回FAILURE）
insert into date_test values(6, '2023-02-29', 'Invalid1'); -- 非闰年2月29日
insert into date_test values(7, '2023-13-01', 'Invalid2'); -- 无效月份
insert into date_test values(8, '2023-04-31', 'Invalid3'); -- 4月没有31日
insert into date_test values(9, '2023-00-01', 'Invalid4'); -- 无效月份0
insert into date_test values(10, '2023-01-00', 'Invalid5'); -- 无效日期0

-- 查询测试
select * from date_test;
select * from date_test where birth_date > '2000-01-01';
select * from date_test where birth_date = '2023-01-01';

-- 排序测试
-- sort select * from date_test order by birth_date;
"""
        
        result_content = """DATE TYPE TESTS
测试日期类型的基本功能

CREATE TABLE DATE_TEST(ID INT, BIRTH_DATE DATE, NAME CHAR(20));
SUCCESS
INSERT INTO DATE_TEST VALUES(1, '2023-01-01', 'ALICE');
SUCCESS
INSERT INTO DATE_TEST VALUES(2, '1990-12-31', 'BOB');
SUCCESS
INSERT INTO DATE_TEST VALUES(3, '2000-02-29', 'CHARLIE');
SUCCESS
INSERT INTO DATE_TEST VALUES(4, '1900-01-01', 'DAVID');
SUCCESS
INSERT INTO DATE_TEST VALUES(5, '2099-12-31', 'EVE');
SUCCESS
INSERT INTO DATE_TEST VALUES(6, '2023-02-29', 'INVALID1');
FAILURE
INSERT INTO DATE_TEST VALUES(7, '2023-13-01', 'INVALID2');
FAILURE
INSERT INTO DATE_TEST VALUES(8, '2023-04-31', 'INVALID3');
FAILURE
INSERT INTO DATE_TEST VALUES(9, '2023-00-01', 'INVALID4');
FAILURE
INSERT INTO DATE_TEST VALUES(10, '2023-01-00', 'INVALID5');
FAILURE
SELECT * FROM DATE_TEST;
ID | BIRTH_DATE | NAME
1 | 2023-01-01 | ALICE
2 | 1990-12-31 | BOB
3 | 2000-02-29 | CHARLIE
4 | 1900-01-01 | DAVID
5 | 2099-12-31 | EVE
SELECT * FROM DATE_TEST WHERE BIRTH_DATE > '2000-01-01';
ID | BIRTH_DATE | NAME
1 | 2023-01-01 | ALICE
5 | 2099-12-31 | EVE
SELECT * FROM DATE_TEST WHERE BIRTH_DATE = '2023-01-01';
ID | BIRTH_DATE | NAME
1 | 2023-01-01 | ALICE
SELECT * FROM DATE_TEST ORDER BY BIRTH_DATE;
4 | 1900-01-01 | DAVID
2 | 1990-12-31 | BOB
3 | 2000-02-29 | CHARLIE
1 | 2023-01-01 | ALICE
5 | 2099-12-31 | EVE
ID | BIRTH_DATE | NAME
"""
        
        self._write_test_files("primary-date-extended", test_content, result_content)
    
    def generate_view_tests(self):
        """生成视图功能测试用例"""
        test_content = """-- echo View Tests
-- 测试视图的创建、查询和删除功能

-- 创建基础表
create table students(id int, name char(20), age int, score float);
create table courses(id int, name char(20), credits int);
create table enrollments(student_id int, course_id int, grade float);

-- 插入测试数据
insert into students values(1, 'Alice', 20, 85.5);
insert into students values(2, 'Bob', 21, 78.0);
insert into students values(3, 'Charlie', 19, 92.5);

insert into courses values(1, 'Math', 3);
insert into courses values(2, 'Physics', 4);
insert into courses values(3, 'Chemistry', 3);

insert into enrollments values(1, 1, 88.0);
insert into enrollments values(1, 2, 82.0);
insert into enrollments values(2, 1, 75.0);
insert into enrollments values(3, 3, 95.0);

-- 创建简单视图
create view high_score_students as select * from students where score > 80;

-- 查询视图
select * from high_score_students;

-- 创建复杂视图（多表连接）
create view student_grades as 
select s.name as student_name, c.name as course_name, e.grade 
from students s, courses c, enrollments e 
where s.id = e.student_id and c.id = e.course_id;

-- 查询复杂视图
select * from student_grades;

-- 测试视图的条件查询
select * from student_grades where grade > 85;

-- 删除视图
drop view high_score_students;
drop view student_grades;

-- 验证视图已删除（应该失败）
select * from high_score_students;
"""
        
        result_content = """VIEW TESTS
测试视图的创建、查询和删除功能

CREATE TABLE STUDENTS(ID INT, NAME CHAR(20), AGE INT, SCORE FLOAT);
SUCCESS
CREATE TABLE COURSES(ID INT, NAME CHAR(20), CREDITS INT);
SUCCESS
CREATE TABLE ENROLLMENTS(STUDENT_ID INT, COURSE_ID INT, GRADE FLOAT);
SUCCESS
INSERT INTO STUDENTS VALUES(1, 'ALICE', 20, 85.5);
SUCCESS
INSERT INTO STUDENTS VALUES(2, 'BOB', 21, 78.0);
SUCCESS
INSERT INTO STUDENTS VALUES(3, 'CHARLIE', 19, 92.5);
SUCCESS
INSERT INTO COURSES VALUES(1, 'MATH', 3);
SUCCESS
INSERT INTO COURSES VALUES(2, 'PHYSICS', 4);
SUCCESS
INSERT INTO COURSES VALUES(3, 'CHEMISTRY', 3);
SUCCESS
INSERT INTO ENROLLMENTS VALUES(1, 1, 88.0);
SUCCESS
INSERT INTO ENROLLMENTS VALUES(1, 2, 82.0);
SUCCESS
INSERT INTO ENROLLMENTS VALUES(2, 1, 75.0);
SUCCESS
INSERT INTO ENROLLMENTS VALUES(3, 3, 95.0);
SUCCESS
CREATE VIEW HIGH_SCORE_STUDENTS AS SELECT * FROM STUDENTS WHERE SCORE > 80;
SUCCESS
SELECT * FROM HIGH_SCORE_STUDENTS;
ID | NAME | AGE | SCORE
1 | ALICE | 20 | 85.5
3 | CHARLIE | 19 | 92.5
CREATE VIEW STUDENT_GRADES AS SELECT S.NAME AS STUDENT_NAME, C.NAME AS COURSE_NAME, E.GRADE FROM STUDENTS S, COURSES C, ENROLLMENTS E WHERE S.ID = E.STUDENT_ID AND C.ID = E.COURSE_ID;
SUCCESS
SELECT * FROM STUDENT_GRADES;
STUDENT_NAME | COURSE_NAME | GRADE
ALICE | MATH | 88
ALICE | PHYSICS | 82
BOB | MATH | 75
CHARLIE | CHEMISTRY | 95
SELECT * FROM STUDENT_GRADES WHERE GRADE > 85;
STUDENT_NAME | COURSE_NAME | GRADE
ALICE | MATH | 88
CHARLIE | CHEMISTRY | 95
DROP VIEW HIGH_SCORE_STUDENTS;
SUCCESS
DROP VIEW STUDENT_GRADES;
SUCCESS
SELECT * FROM HIGH_SCORE_STUDENTS;
FAILURE
"""
        
        self._write_test_files("primary-create-view-extended", test_content, result_content)
    
    def generate_show_index_tests(self):
        """生成SHOW INDEX测试用例"""
        test_content = """-- echo Show Index Tests
-- 测试SHOW INDEX功能

-- 创建测试表
create table index_test(id int, name char(20), age int, score float);

-- 创建各种索引
create index idx_id on index_test(id);
create index idx_name on index_test(name);
create unique index idx_age_unique on index_test(age);

-- 显示索引信息
show index from index_test;

-- 插入数据测试索引
insert into index_test values(1, 'Alice', 20, 85.5);
insert into index_test values(2, 'Bob', 21, 78.0);

-- 再次显示索引信息
show index from index_test;

-- 测试不存在的表
show index from non_existent_table;
"""

        result_content = """SHOW INDEX TESTS
测试SHOW INDEX功能

CREATE TABLE INDEX_TEST(ID INT, NAME CHAR(20), AGE INT, SCORE FLOAT);
SUCCESS
CREATE INDEX IDX_ID ON INDEX_TEST(ID);
SUCCESS
CREATE INDEX IDX_NAME ON INDEX_TEST(NAME);
SUCCESS
CREATE UNIQUE INDEX IDX_AGE_UNIQUE ON INDEX_TEST(AGE);
SUCCESS
SHOW INDEX FROM INDEX_TEST;
TABLE | NON_UNIQUE | KEY_NAME | SEQ_IN_INDEX | COLUMN_NAME
INDEX_TEST | 1 | IDX_ID | 1 | ID
INDEX_TEST | 1 | IDX_NAME | 1 | NAME
INDEX_TEST | 0 | IDX_AGE_UNIQUE | 1 | AGE
INSERT INTO INDEX_TEST VALUES(1, 'ALICE', 20, 85.5);
SUCCESS
INSERT INTO INDEX_TEST VALUES(2, 'BOB', 21, 78.0);
SUCCESS
SHOW INDEX FROM INDEX_TEST;
TABLE | NON_UNIQUE | KEY_NAME | SEQ_IN_INDEX | COLUMN_NAME
INDEX_TEST | 1 | IDX_ID | 1 | ID
INDEX_TEST | 1 | IDX_NAME | 1 | NAME
INDEX_TEST | 0 | IDX_AGE_UNIQUE | 1 | AGE
SHOW INDEX FROM NON_EXISTENT_TABLE;
FAILURE
"""

        self._write_test_files("primary-show-index-extended", test_content, result_content)

    def generate_update_tests(self):
        """生成UPDATE测试用例"""
        test_content = """-- echo Update Tests
-- 测试UPDATE功能的各种场景

-- 创建测试表
create table update_test(id int, name char(20), age int, score float);

-- 插入测试数据
insert into update_test values(1, 'Alice', 20, 85.5);
insert into update_test values(2, 'Bob', 21, 78.0);
insert into update_test values(3, 'Charlie', 19, 92.5);
insert into update_test values(4, 'David', 22, 88.0);

-- 显示初始数据
select * from update_test;

-- 测试单字段更新（无条件）
update update_test set age = 25;
select * from update_test;

-- 重置数据
delete from update_test;
insert into update_test values(1, 'Alice', 20, 85.5);
insert into update_test values(2, 'Bob', 21, 78.0);
insert into update_test values(3, 'Charlie', 19, 92.5);

-- 测试带条件的更新
update update_test set score = 90.0 where id = 1;
select * from update_test;

-- 测试多条件更新
update update_test set age = 23 where score > 80 and age < 21;
select * from update_test;

-- 创建索引后测试更新
create index idx_id on update_test(id);
update update_test set name = 'Updated' where id = 2;
select * from update_test;

-- 测试更新不存在的记录
update update_test set age = 30 where id = 999;
select * from update_test;
"""

        result_content = """UPDATE TESTS
测试UPDATE功能的各种场景

CREATE TABLE UPDATE_TEST(ID INT, NAME CHAR(20), AGE INT, SCORE FLOAT);
SUCCESS
INSERT INTO UPDATE_TEST VALUES(1, 'ALICE', 20, 85.5);
SUCCESS
INSERT INTO UPDATE_TEST VALUES(2, 'BOB', 21, 78.0);
SUCCESS
INSERT INTO UPDATE_TEST VALUES(3, 'CHARLIE', 19, 92.5);
SUCCESS
INSERT INTO UPDATE_TEST VALUES(4, 'DAVID', 22, 88.0);
SUCCESS
SELECT * FROM UPDATE_TEST;
ID | NAME | AGE | SCORE
1 | ALICE | 20 | 85.5
2 | BOB | 21 | 78
3 | CHARLIE | 19 | 92.5
4 | DAVID | 22 | 88
UPDATE UPDATE_TEST SET AGE = 25;
SUCCESS
SELECT * FROM UPDATE_TEST;
ID | NAME | AGE | SCORE
1 | ALICE | 25 | 85.5
2 | BOB | 25 | 78
3 | CHARLIE | 25 | 92.5
4 | DAVID | 25 | 88
DELETE FROM UPDATE_TEST;
SUCCESS
INSERT INTO UPDATE_TEST VALUES(1, 'ALICE', 20, 85.5);
SUCCESS
INSERT INTO UPDATE_TEST VALUES(2, 'BOB', 21, 78.0);
SUCCESS
INSERT INTO UPDATE_TEST VALUES(3, 'CHARLIE', 19, 92.5);
SUCCESS
UPDATE UPDATE_TEST SET SCORE = 90.0 WHERE ID = 1;
SUCCESS
SELECT * FROM UPDATE_TEST;
ID | NAME | AGE | SCORE
1 | ALICE | 20 | 90
2 | BOB | 21 | 78
3 | CHARLIE | 19 | 92.5
UPDATE UPDATE_TEST SET AGE = 23 WHERE SCORE > 80 AND AGE < 21;
SUCCESS
SELECT * FROM UPDATE_TEST;
ID | NAME | AGE | SCORE
1 | ALICE | 23 | 90
2 | BOB | 21 | 78
3 | CHARLIE | 23 | 92.5
CREATE INDEX IDX_ID ON UPDATE_TEST(ID);
SUCCESS
UPDATE UPDATE_TEST SET NAME = 'UPDATED' WHERE ID = 2;
SUCCESS
SELECT * FROM UPDATE_TEST;
ID | NAME | AGE | SCORE
1 | ALICE | 23 | 90
2 | UPDATED | 21 | 78
3 | CHARLIE | 23 | 92.5
UPDATE UPDATE_TEST SET AGE = 30 WHERE ID = 999;
SUCCESS
SELECT * FROM UPDATE_TEST;
ID | NAME | AGE | SCORE
1 | ALICE | 23 | 90
2 | UPDATED | 21 | 78
3 | CHARLIE | 23 | 92.5
"""

        self._write_test_files("primary-update-extended", test_content, result_content)

    def generate_vector_tests(self):
        """生成向量类型测试用例"""
        test_content = """-- echo Vector Tests
-- 测试向量类型的基本功能

-- 创建包含向量字段的表
create table vector_test(id int, name char(20), embedding vector(3));

-- 插入向量数据
insert into vector_test values(1, 'item1', '[1.0, 2.0, 3.0]');
insert into vector_test values(2, 'item2', '[2.0, 3.0, 4.0]');
insert into vector_test values(3, 'item3', '[1.5, 2.5, 3.5]');

-- 查询向量数据
select * from vector_test;

-- 测试向量运算
select id, name, embedding + '[1.0, 1.0, 1.0]' from vector_test;
select id, name, embedding - '[0.5, 0.5, 0.5]' from vector_test;

-- 测试距离函数
select id, name, l2_distance(embedding, '[1.0, 2.0, 3.0]') from vector_test;
select id, name, cosine_distance(embedding, '[1.0, 2.0, 3.0]') from vector_test;
select id, name, inner_product(embedding, '[1.0, 2.0, 3.0]') from vector_test;

-- 测试向量检索
select * from vector_test order by l2_distance(embedding, '[1.0, 2.0, 3.0]') limit 2;
"""

        result_content = """VECTOR TESTS
测试向量类型的基本功能

CREATE TABLE VECTOR_TEST(ID INT, NAME CHAR(20), EMBEDDING VECTOR(3));
SUCCESS
INSERT INTO VECTOR_TEST VALUES(1, 'ITEM1', '[1.0, 2.0, 3.0]');
SUCCESS
INSERT INTO VECTOR_TEST VALUES(2, 'ITEM2', '[2.0, 3.0, 4.0]');
SUCCESS
INSERT INTO VECTOR_TEST VALUES(3, 'ITEM3', '[1.5, 2.5, 3.5]');
SUCCESS
SELECT * FROM VECTOR_TEST;
ID | NAME | EMBEDDING
1 | ITEM1 | [1, 2, 3]
2 | ITEM2 | [2, 3, 4]
3 | ITEM3 | [1.5, 2.5, 3.5]
SELECT ID, NAME, EMBEDDING + '[1.0, 1.0, 1.0]' FROM VECTOR_TEST;
ID | NAME | EMBEDDING + '[1.0, 1.0, 1.0]'
1 | ITEM1 | [2, 3, 4]
2 | ITEM2 | [3, 4, 5]
3 | ITEM3 | [2.5, 3.5, 4.5]
SELECT ID, NAME, EMBEDDING - '[0.5, 0.5, 0.5]' FROM VECTOR_TEST;
ID | NAME | EMBEDDING - '[0.5, 0.5, 0.5]'
1 | ITEM1 | [0.5, 1.5, 2.5]
2 | ITEM2 | [1.5, 2.5, 3.5]
3 | ITEM3 | [1, 2, 3]
SELECT ID, NAME, L2_DISTANCE(EMBEDDING, '[1.0, 2.0, 3.0]') FROM VECTOR_TEST;
ID | NAME | L2_DISTANCE(EMBEDDING, '[1.0, 2.0, 3.0]')
1 | ITEM1 | 0
2 | ITEM2 | 1.732
3 | ITEM3 | 0.866
SELECT ID, NAME, COSINE_DISTANCE(EMBEDDING, '[1.0, 2.0, 3.0]') FROM VECTOR_TEST;
ID | NAME | COSINE_DISTANCE(EMBEDDING, '[1.0, 2.0, 3.0]')
1 | ITEM1 | 0
2 | ITEM2 | 0.025
3 | ITEM3 | 0.004
SELECT ID, NAME, INNER_PRODUCT(EMBEDDING, '[1.0, 2.0, 3.0]') FROM VECTOR_TEST;
ID | NAME | INNER_PRODUCT(EMBEDDING, '[1.0, 2.0, 3.0]')
1 | ITEM1 | 14
2 | ITEM2 | 20
3 | ITEM3 | 17
SELECT * FROM VECTOR_TEST ORDER BY L2_DISTANCE(EMBEDDING, '[1.0, 2.0, 3.0]') LIMIT 2;
ID | NAME | EMBEDDING
1 | ITEM1 | [1, 2, 3]
3 | ITEM3 | [1.5, 2.5, 3.5]
"""

        self._write_test_files("vector-basic-extended", test_content, result_content)

    def generate_like_tests(self):
        """生成LIKE操作符测试用例"""
        test_content = """-- echo Like Tests
-- 测试LIKE操作符功能

-- 创建测试表
create table like_test(id int, name char(30), description char(50));

-- 插入测试数据
insert into like_test values(1, 'Alice', 'Student from Beijing');
insert into like_test values(2, 'Bob', 'Teacher in Shanghai');
insert into like_test values(3, 'Charlie', 'Engineer from Shenzhen');
insert into like_test values(4, 'David', 'Doctor in Beijing');
insert into like_test values(5, 'Eve', 'Artist from Guangzhou');

-- 测试%通配符
select * from like_test where name like 'A%';
select * from like_test where description like '%Beijing%';
select * from like_test where description like '%from%';

-- 测试_通配符
select * from like_test where name like '_ob';
select * from like_test where name like 'A____';

-- 测试组合通配符
select * from like_test where description like '%from _____';
select * from like_test where name like '%e';

-- 测试精确匹配
select * from like_test where name like 'Alice';

-- 测试空结果
select * from like_test where name like 'xyz%';
"""

        result_content = """LIKE TESTS
测试LIKE操作符功能

CREATE TABLE LIKE_TEST(ID INT, NAME CHAR(30), DESCRIPTION CHAR(50));
SUCCESS
INSERT INTO LIKE_TEST VALUES(1, 'ALICE', 'STUDENT FROM BEIJING');
SUCCESS
INSERT INTO LIKE_TEST VALUES(2, 'BOB', 'TEACHER IN SHANGHAI');
SUCCESS
INSERT INTO LIKE_TEST VALUES(3, 'CHARLIE', 'ENGINEER FROM SHENZHEN');
SUCCESS
INSERT INTO LIKE_TEST VALUES(4, 'DAVID', 'DOCTOR IN BEIJING');
SUCCESS
INSERT INTO LIKE_TEST VALUES(5, 'EVE', 'ARTIST FROM GUANGZHOU');
SUCCESS
SELECT * FROM LIKE_TEST WHERE NAME LIKE 'A%';
ID | NAME | DESCRIPTION
1 | ALICE | STUDENT FROM BEIJING
SELECT * FROM LIKE_TEST WHERE DESCRIPTION LIKE '%BEIJING%';
ID | NAME | DESCRIPTION
1 | ALICE | STUDENT FROM BEIJING
4 | DAVID | DOCTOR IN BEIJING
SELECT * FROM LIKE_TEST WHERE DESCRIPTION LIKE '%FROM%';
ID | NAME | DESCRIPTION
1 | ALICE | STUDENT FROM BEIJING
3 | CHARLIE | ENGINEER FROM SHENZHEN
5 | EVE | ARTIST FROM GUANGZHOU
SELECT * FROM LIKE_TEST WHERE NAME LIKE '_OB';
ID | NAME | DESCRIPTION
2 | BOB | TEACHER IN SHANGHAI
SELECT * FROM LIKE_TEST WHERE NAME LIKE 'A____';
ID | NAME | DESCRIPTION
1 | ALICE | STUDENT FROM BEIJING
SELECT * FROM LIKE_TEST WHERE DESCRIPTION LIKE '%FROM _____';
ID | NAME | DESCRIPTION
SELECT * FROM LIKE_TEST WHERE NAME LIKE '%E';
ID | NAME | DESCRIPTION
3 | CHARLIE | ENGINEER FROM SHENZHEN
5 | EVE | ARTIST FROM GUANGZHOU
SELECT * FROM LIKE_TEST WHERE NAME LIKE 'ALICE';
ID | NAME | DESCRIPTION
1 | ALICE | STUDENT FROM BEIJING
SELECT * FROM LIKE_TEST WHERE NAME LIKE 'XYZ%';
ID | NAME | DESCRIPTION
"""

        self._write_test_files("primary-like-extended", test_content, result_content)

    def generate_null_tests(self):
        """生成NULL值测试用例"""
        test_content = """-- echo Null Tests
-- 测试NULL值的各种场景

-- 创建支持NULL的表
create table null_test(id int not null, name char(20) null, age int null, score float null);

-- 插入包含NULL的数据
insert into null_test values(1, 'Alice', 20, 85.5);
insert into null_test values(2, null, 21, 78.0);
insert into null_test values(3, 'Charlie', null, 92.5);
insert into null_test values(4, 'David', 22, null);
insert into null_test values(5, null, null, null);

-- 查询所有数据
select * from null_test;

-- 测试NULL比较
select * from null_test where name is null;
select * from null_test where name is not null;
select * from null_test where age is null;

-- 测试NULL在运算中的行为
select id, name, age + 1 from null_test;
select id, name, score * 2 from null_test;

-- 测试聚合函数对NULL的处理
select count(*) from null_test;
select count(name) from null_test;
select count(age) from null_test;
select avg(age) from null_test;
select sum(score) from null_test;

-- 测试插入NULL到NOT NULL字段（应该失败）
insert into null_test values(null, 'Test', 25, 90.0);
"""

        result_content = """NULL TESTS
测试NULL值的各种场景

CREATE TABLE NULL_TEST(ID INT NOT NULL, NAME CHAR(20) NULL, AGE INT NULL, SCORE FLOAT NULL);
SUCCESS
INSERT INTO NULL_TEST VALUES(1, 'ALICE', 20, 85.5);
SUCCESS
INSERT INTO NULL_TEST VALUES(2, NULL, 21, 78.0);
SUCCESS
INSERT INTO NULL_TEST VALUES(3, 'CHARLIE', NULL, 92.5);
SUCCESS
INSERT INTO NULL_TEST VALUES(4, 'DAVID', 22, NULL);
SUCCESS
INSERT INTO NULL_TEST VALUES(5, NULL, NULL, NULL);
SUCCESS
SELECT * FROM NULL_TEST;
ID | NAME | AGE | SCORE
1 | ALICE | 20 | 85.5
2 | NULL | 21 | 78
3 | CHARLIE | NULL | 92.5
4 | DAVID | 22 | NULL
5 | NULL | NULL | NULL
SELECT * FROM NULL_TEST WHERE NAME IS NULL;
ID | NAME | AGE | SCORE
2 | NULL | 21 | 78
5 | NULL | NULL | NULL
SELECT * FROM NULL_TEST WHERE NAME IS NOT NULL;
ID | NAME | AGE | SCORE
1 | ALICE | 20 | 85.5
3 | CHARLIE | NULL | 92.5
4 | DAVID | 22 | NULL
SELECT * FROM NULL_TEST WHERE AGE IS NULL;
ID | NAME | AGE | SCORE
3 | CHARLIE | NULL | 92.5
5 | NULL | NULL | NULL
SELECT ID, NAME, AGE + 1 FROM NULL_TEST;
ID | NAME | AGE + 1
1 | ALICE | 21
2 | NULL | 22
3 | CHARLIE | NULL
4 | DAVID | 23
5 | NULL | NULL
SELECT ID, NAME, SCORE * 2 FROM NULL_TEST;
ID | NAME | SCORE * 2
1 | ALICE | 171
2 | NULL | 156
3 | CHARLIE | 185
4 | DAVID | NULL
5 | NULL | NULL
SELECT COUNT(*) FROM NULL_TEST;
COUNT(*)
5
SELECT COUNT(NAME) FROM NULL_TEST;
COUNT(NAME)
3
SELECT COUNT(AGE) FROM NULL_TEST;
COUNT(AGE)
3
SELECT AVG(AGE) FROM NULL_TEST;
AVG(AGE)
21
SELECT SUM(SCORE) FROM NULL_TEST;
SUM(SCORE)
256
INSERT INTO NULL_TEST VALUES(NULL, 'TEST', 25, 90.0);
FAILURE
"""

        self._write_test_files("primary-null-extended", test_content, result_content)

    def generate_unique_index_tests(self):
        """生成唯一索引测试用例"""
        test_content = """-- echo Unique Index Tests
-- 测试唯一索引功能

-- 创建测试表
create table unique_test(id int, email char(50), phone char(20), name char(30));

-- 创建唯一索引
create unique index idx_email on unique_test(email);
create unique index idx_phone on unique_test(phone);

-- 插入数据
insert into unique_test values(1, 'alice@test.com', '13800138001', 'Alice');
insert into unique_test values(2, 'bob@test.com', '13800138002', 'Bob');

-- 查询数据
select * from unique_test;

-- 测试唯一约束（应该失败）
insert into unique_test values(3, 'alice@test.com', '13800138003', 'Charlie');
insert into unique_test values(4, 'charlie@test.com', '13800138001', 'David');

-- 查询数据确认没有重复
select * from unique_test;

-- 测试更新违反唯一约束（应该失败）
update unique_test set email = 'bob@test.com' where id = 1;

-- 测试正常更新
update unique_test set name = 'Alice Updated' where id = 1;
select * from unique_test;

-- 显示索引信息
show index from unique_test;
"""

        result_content = """UNIQUE INDEX TESTS
测试唯一索引功能

CREATE TABLE UNIQUE_TEST(ID INT, EMAIL CHAR(50), PHONE CHAR(20), NAME CHAR(30));
SUCCESS
CREATE UNIQUE INDEX IDX_EMAIL ON UNIQUE_TEST(EMAIL);
SUCCESS
CREATE UNIQUE INDEX IDX_PHONE ON UNIQUE_TEST(PHONE);
SUCCESS
INSERT INTO UNIQUE_TEST VALUES(1, 'ALICE@TEST.COM', '13800138001', 'ALICE');
SUCCESS
INSERT INTO UNIQUE_TEST VALUES(2, 'BOB@TEST.COM', '13800138002', 'BOB');
SUCCESS
SELECT * FROM UNIQUE_TEST;
ID | EMAIL | PHONE | NAME
1 | ALICE@TEST.COM | 13800138001 | ALICE
2 | BOB@TEST.COM | 13800138002 | BOB
INSERT INTO UNIQUE_TEST VALUES(3, 'ALICE@TEST.COM', '13800138003', 'CHARLIE');
FAILURE
INSERT INTO UNIQUE_TEST VALUES(4, 'CHARLIE@TEST.COM', '13800138001', 'DAVID');
FAILURE
SELECT * FROM UNIQUE_TEST;
ID | EMAIL | PHONE | NAME
1 | ALICE@TEST.COM | 13800138001 | ALICE
2 | BOB@TEST.COM | 13800138002 | BOB
UPDATE UNIQUE_TEST SET EMAIL = 'BOB@TEST.COM' WHERE ID = 1;
FAILURE
UPDATE UNIQUE_TEST SET NAME = 'ALICE UPDATED' WHERE ID = 1;
SUCCESS
SELECT * FROM UNIQUE_TEST;
ID | EMAIL | PHONE | NAME
1 | ALICE@TEST.COM | 13800138001 | ALICE UPDATED
2 | BOB@TEST.COM | 13800138002 | BOB
SHOW INDEX FROM UNIQUE_TEST;
TABLE | NON_UNIQUE | KEY_NAME | SEQ_IN_INDEX | COLUMN_NAME
UNIQUE_TEST | 0 | IDX_EMAIL | 1 | EMAIL
UNIQUE_TEST | 0 | IDX_PHONE | 1 | PHONE
"""

        self._write_test_files("primary-unique-extended", test_content, result_content)

    def generate_multi_index_tests(self):
        """生成多字段索引测试用例"""
        test_content = """-- echo Multi Index Tests
-- 测试多字段索引功能

-- 创建测试表
create table multi_index_test(id int, col1 int, col2 int, col3 char(20), col4 float);

-- 创建多字段索引
create index idx_multi_12 on multi_index_test(col1, col2);
create index idx_multi_34 on multi_index_test(col3, col4);
create unique index idx_unique_multi on multi_index_test(col1, col3);

-- 插入测试数据
insert into multi_index_test values(1, 10, 20, 'A', 1.5);
insert into multi_index_test values(2, 10, 30, 'B', 2.5);
insert into multi_index_test values(3, 20, 20, 'A', 3.5);
insert into multi_index_test values(4, 20, 30, 'C', 4.5);

-- 查询数据
select * from multi_index_test;

-- 测试多字段查询
select * from multi_index_test where col1 = 10 and col2 = 20;
select * from multi_index_test where col3 = 'A' and col4 > 2.0;

-- 测试唯一多字段索引约束（应该失败）
insert into multi_index_test values(5, 10, 40, 'A', 5.5);

-- 显示索引信息
show index from multi_index_test;

-- 测试部分字段查询（应该能使用索引）
select * from multi_index_test where col1 = 10;
select * from multi_index_test where col3 = 'A';
"""

        result_content = """MULTI INDEX TESTS
测试多字段索引功能

CREATE TABLE MULTI_INDEX_TEST(ID INT, COL1 INT, COL2 INT, COL3 CHAR(20), COL4 FLOAT);
SUCCESS
CREATE INDEX IDX_MULTI_12 ON MULTI_INDEX_TEST(COL1, COL2);
SUCCESS
CREATE INDEX IDX_MULTI_34 ON MULTI_INDEX_TEST(COL3, COL4);
SUCCESS
CREATE UNIQUE INDEX IDX_UNIQUE_MULTI ON MULTI_INDEX_TEST(COL1, COL3);
SUCCESS
INSERT INTO MULTI_INDEX_TEST VALUES(1, 10, 20, 'A', 1.5);
SUCCESS
INSERT INTO MULTI_INDEX_TEST VALUES(2, 10, 30, 'B', 2.5);
SUCCESS
INSERT INTO MULTI_INDEX_TEST VALUES(3, 20, 20, 'A', 3.5);
SUCCESS
INSERT INTO MULTI_INDEX_TEST VALUES(4, 20, 30, 'C', 4.5);
SUCCESS
SELECT * FROM MULTI_INDEX_TEST;
ID | COL1 | COL2 | COL3 | COL4
1 | 10 | 20 | A | 1.5
2 | 10 | 30 | B | 2.5
3 | 20 | 20 | A | 3.5
4 | 20 | 30 | C | 4.5
SELECT * FROM MULTI_INDEX_TEST WHERE COL1 = 10 AND COL2 = 20;
ID | COL1 | COL2 | COL3 | COL4
1 | 10 | 20 | A | 1.5
SELECT * FROM MULTI_INDEX_TEST WHERE COL3 = 'A' AND COL4 > 2.0;
ID | COL1 | COL2 | COL3 | COL4
3 | 20 | 20 | A | 3.5
INSERT INTO MULTI_INDEX_TEST VALUES(5, 10, 40, 'A', 5.5);
FAILURE
SHOW INDEX FROM MULTI_INDEX_TEST;
TABLE | NON_UNIQUE | KEY_NAME | SEQ_IN_INDEX | COLUMN_NAME
MULTI_INDEX_TEST | 1 | IDX_MULTI_12 | 1 | COL1
MULTI_INDEX_TEST | 1 | IDX_MULTI_12 | 2 | COL2
MULTI_INDEX_TEST | 1 | IDX_MULTI_34 | 1 | COL3
MULTI_INDEX_TEST | 1 | IDX_MULTI_34 | 2 | COL4
MULTI_INDEX_TEST | 0 | IDX_UNIQUE_MULTI | 1 | COL1
MULTI_INDEX_TEST | 0 | IDX_UNIQUE_MULTI | 2 | COL3
SELECT * FROM MULTI_INDEX_TEST WHERE COL1 = 10;
ID | COL1 | COL2 | COL3 | COL4
1 | 10 | 20 | A | 1.5
2 | 10 | 30 | B | 2.5
SELECT * FROM MULTI_INDEX_TEST WHERE COL3 = 'A';
ID | COL1 | COL2 | COL3 | COL4
1 | 10 | 20 | A | 1.5
3 | 20 | 20 | A | 3.5
"""

        self._write_test_files("primary-multi-index-extended", test_content, result_content)

    def generate_alias_tests(self):
        """生成别名功能测试用例"""
        test_content = """-- echo Alias Tests
-- Test table and column alias functionality

-- Create test tables
create table students(id int, name char(20), age int, score float);
create table courses(id int, title char(30), credits int);

-- Insert test data
insert into students values(1, 'Alice', 20, 85.5);
insert into students values(2, 'Bob', 21, 78.0);
insert into students values(3, 'Charlie', 19, 92.5);

insert into courses values(1, 'Mathematics', 3);
insert into courses values(2, 'Physics', 4);

-- Test column aliases
select name as student_name, age as student_age from students;
select id, score as final_score from students where score > 80;

-- Test table aliases
select s.name, s.age from students as s;
select s.name, s.score from students s where s.age > 20;

-- Test combined table and column aliases
select s.name as student_name, c.title as course_title
from students as s, courses as c where s.id = c.id;

-- Test aliases in WHERE clause
select s.name from students s where s.score > 80;

-- Test duplicate table names with aliases
select s1.name, s2.name from students s1, students s2 where s1.id != s2.id;
"""

        result_content = """ALIAS TESTS
CREATE TABLE STUDENTS(ID INT, NAME CHAR(20), AGE INT, SCORE FLOAT);
SUCCESS
CREATE TABLE COURSES(ID INT, TITLE CHAR(30), CREDITS INT);
SUCCESS
INSERT INTO STUDENTS VALUES(1, 'ALICE', 20, 85.5);
SUCCESS
INSERT INTO STUDENTS VALUES(2, 'BOB', 21, 78.0);
SUCCESS
INSERT INTO STUDENTS VALUES(3, 'CHARLIE', 19, 92.5);
SUCCESS
INSERT INTO COURSES VALUES(1, 'MATHEMATICS', 3);
SUCCESS
INSERT INTO COURSES VALUES(2, 'PHYSICS', 4);
SUCCESS
SELECT NAME AS STUDENT_NAME, AGE AS STUDENT_AGE FROM STUDENTS;
STUDENT_NAME | STUDENT_AGE
ALICE | 20
BOB | 21
CHARLIE | 19
SELECT ID, SCORE AS FINAL_SCORE FROM STUDENTS WHERE SCORE > 80;
ID | FINAL_SCORE
1 | 85.5
3 | 92.5
SELECT S.NAME, S.AGE FROM STUDENTS AS S;
NAME | AGE
ALICE | 20
BOB | 21
CHARLIE | 19
SELECT S.NAME, S.SCORE FROM STUDENTS S WHERE S.AGE > 20;
NAME | SCORE
BOB | 78
SELECT S.NAME AS STUDENT_NAME, C.TITLE AS COURSE_TITLE FROM STUDENTS AS S, COURSES AS C WHERE S.ID = C.ID;
STUDENT_NAME | COURSE_TITLE
ALICE | MATHEMATICS
BOB | PHYSICS
SELECT S.NAME FROM STUDENTS S WHERE S.SCORE > 80;
NAME
ALICE
CHARLIE
SELECT S1.NAME, S2.NAME FROM STUDENTS S1, STUDENTS S2 WHERE S1.ID != S2.ID;
NAME | NAME
ALICE | BOB
ALICE | CHARLIE
BOB | ALICE
BOB | CHARLIE
CHARLIE | ALICE
CHARLIE | BOB
"""

        self._write_test_files("primary-alias-extended", test_content, result_content)

    def generate_text_tests(self):
        """生成文本字段测试用例"""
        test_content = """-- echo Text Field Tests
-- Test TEXT data type functionality

-- Create table with TEXT field
create table text_test(id int, title char(50), content text);

-- Insert normal text data
insert into text_test values(1, 'Short Article', 'This is a short article content.');
insert into text_test values(2, 'Medium Article', 'This is a medium length article with more content to test the text field functionality.');

-- Insert long text data (within limits)
insert into text_test values(3, 'Long Article', 'This is a very long article content that contains multiple paragraphs and extensive text to test the TEXT field storage capabilities. The TEXT field should be able to store up to 65535 bytes of data according to MySQL specifications.');

-- Query text data
select * from text_test;
select id, title from text_test;
select id, title from text_test where id = 1;

-- Test text field in WHERE clause
select id, title from text_test where content like '%long%';

-- Test text field length limits (should succeed)
insert into text_test values(4, 'Test Limits', 'Normal text within limits');

-- Clean up
drop table text_test;
"""

        result_content = """TEXT FIELD TESTS
CREATE TABLE TEXT_TEST(ID INT, TITLE CHAR(50), CONTENT TEXT);
SUCCESS
INSERT INTO TEXT_TEST VALUES(1, 'SHORT ARTICLE', 'THIS IS A SHORT ARTICLE CONTENT.');
SUCCESS
INSERT INTO TEXT_TEST VALUES(2, 'MEDIUM ARTICLE', 'THIS IS A MEDIUM LENGTH ARTICLE WITH MORE CONTENT TO TEST THE TEXT FIELD FUNCTIONALITY.');
SUCCESS
INSERT INTO TEXT_TEST VALUES(3, 'LONG ARTICLE', 'THIS IS A VERY LONG ARTICLE CONTENT THAT CONTAINS MULTIPLE PARAGRAPHS AND EXTENSIVE TEXT TO TEST THE TEXT FIELD STORAGE CAPABILITIES. THE TEXT FIELD SHOULD BE ABLE TO STORE UP TO 65535 BYTES OF DATA ACCORDING TO MYSQL SPECIFICATIONS.');
SUCCESS
SELECT * FROM TEXT_TEST;
ID | TITLE | CONTENT
1 | SHORT ARTICLE | THIS IS A SHORT ARTICLE CONTENT.
2 | MEDIUM ARTICLE | THIS IS A MEDIUM LENGTH ARTICLE WITH MORE CONTENT TO TEST THE TEXT FIELD FUNCTIONALITY.
3 | LONG ARTICLE | THIS IS A VERY LONG ARTICLE CONTENT THAT CONTAINS MULTIPLE PARAGRAPHS AND EXTENSIVE TEXT TO TEST THE TEXT FIELD STORAGE CAPABILITIES. THE TEXT FIELD SHOULD BE ABLE TO STORE UP TO 65535 BYTES OF DATA ACCORDING TO MYSQL SPECIFICATIONS.
SELECT ID, TITLE FROM TEXT_TEST;
ID | TITLE
1 | SHORT ARTICLE
2 | MEDIUM ARTICLE
3 | LONG ARTICLE
SELECT ID, TITLE FROM TEXT_TEST WHERE ID = 1;
ID | TITLE
1 | SHORT ARTICLE
SELECT ID, TITLE FROM TEXT_TEST WHERE CONTENT LIKE '%LONG%';
ID | TITLE
3 | LONG ARTICLE
INSERT INTO TEXT_TEST VALUES(4, 'TEST LIMITS', 'NORMAL TEXT WITHIN LIMITS');
SUCCESS
DROP TABLE TEXT_TEST;
SUCCESS
"""

        self._write_test_files("primary-text-extended", test_content, result_content)

    def generate_order_by_tests(self):
        """生成ORDER BY测试用例"""
        test_content = """-- echo Order By Tests
-- Test ORDER BY functionality

-- Create test table
create table order_test(id int, name char(20), age int, score float);

-- Insert test data
insert into order_test values(3, 'Charlie', 19, 92.5);
insert into order_test values(1, 'Alice', 20, 85.5);
insert into order_test values(4, 'David', 22, 88.0);
insert into order_test values(2, 'Bob', 21, 78.0);

-- Test single column ascending order
-- sort select * from order_test order by id;
-- sort select * from order_test order by id asc;

-- Test single column descending order
-- sort select * from order_test order by score desc;

-- Test multiple column ordering
-- sort select * from order_test order by age asc, score desc;

-- Test ordering with WHERE clause
-- sort select * from order_test where score > 80 order by score;

-- Test ordering different data types
-- sort select * from order_test order by name;
-- sort select * from order_test order by age, name;

-- Clean up
drop table order_test;
"""

        result_content = """ORDER BY TESTS
CREATE TABLE ORDER_TEST(ID INT, NAME CHAR(20), AGE INT, SCORE FLOAT);
SUCCESS
INSERT INTO ORDER_TEST VALUES(3, 'CHARLIE', 19, 92.5);
SUCCESS
INSERT INTO ORDER_TEST VALUES(1, 'ALICE', 20, 85.5);
SUCCESS
INSERT INTO ORDER_TEST VALUES(4, 'DAVID', 22, 88.0);
SUCCESS
INSERT INTO ORDER_TEST VALUES(2, 'BOB', 21, 78.0);
SUCCESS
SELECT * FROM ORDER_TEST ORDER BY ID;
1 | ALICE | 20 | 85.5
2 | BOB | 21 | 78
3 | CHARLIE | 19 | 92.5
4 | DAVID | 22 | 88
ID | NAME | AGE | SCORE
SELECT * FROM ORDER_TEST ORDER BY ID ASC;
1 | ALICE | 20 | 85.5
2 | BOB | 21 | 78
3 | CHARLIE | 19 | 92.5
4 | DAVID | 22 | 88
ID | NAME | AGE | SCORE
SELECT * FROM ORDER_TEST ORDER BY SCORE DESC;
3 | CHARLIE | 19 | 92.5
4 | DAVID | 22 | 88
1 | ALICE | 20 | 85.5
2 | BOB | 21 | 78
ID | NAME | AGE | SCORE
SELECT * FROM ORDER_TEST ORDER BY AGE ASC, SCORE DESC;
3 | CHARLIE | 19 | 92.5
1 | ALICE | 20 | 85.5
2 | BOB | 21 | 78
4 | DAVID | 22 | 88
ID | NAME | AGE | SCORE
SELECT * FROM ORDER_TEST WHERE SCORE > 80 ORDER BY SCORE;
1 | ALICE | 20 | 85.5
4 | DAVID | 22 | 88
3 | CHARLIE | 19 | 92.5
ID | NAME | AGE | SCORE
SELECT * FROM ORDER_TEST ORDER BY NAME;
1 | ALICE | 20 | 85.5
2 | BOB | 21 | 78
3 | CHARLIE | 19 | 92.5
4 | DAVID | 22 | 88
ID | NAME | AGE | SCORE
SELECT * FROM ORDER_TEST ORDER BY AGE, NAME;
3 | CHARLIE | 19 | 92.5
1 | ALICE | 20 | 85.5
2 | BOB | 21 | 78
4 | DAVID | 22 | 88
ID | NAME | AGE | SCORE
DROP TABLE ORDER_TEST;
SUCCESS
"""

        self._write_test_files("primary-order-by-extended", test_content, result_content)

    def generate_aggregation_groupby_tests(self):
        """生成聚合和分组测试用例"""
        test_content = """-- echo Aggregation and Group By Tests
-- Test aggregation functions and GROUP BY

-- Create test table
create table agg_test(id int, department char(20), salary float, age int);

-- Insert test data
insert into agg_test values(1, 'Engineering', 5000.0, 25);
insert into agg_test values(2, 'Engineering', 6000.0, 30);
insert into agg_test values(3, 'Sales', 4000.0, 28);
insert into agg_test values(4, 'Sales', 4500.0, 32);
insert into agg_test values(5, 'Marketing', 5500.0, 27);

-- Test basic aggregation functions
select count(*) from agg_test;
select count(id) from agg_test;
select sum(salary) from agg_test;
select avg(salary) from agg_test;
select min(salary) from agg_test;
select max(salary) from agg_test;

-- Test GROUP BY with aggregation
select department, count(*) from agg_test group by department;
select department, avg(salary) from agg_test group by department;
select department, sum(salary), count(*) from agg_test group by department;

-- Test GROUP BY with multiple columns
select department, age, count(*) from agg_test group by department, age;

-- Test HAVING clause
select department, avg(salary) from agg_test group by department having avg(salary) > 4500;
select department, count(*) from agg_test group by department having count(*) > 1;

-- Clean up
drop table agg_test;
"""

        result_content = """AGGREGATION AND GROUP BY TESTS
CREATE TABLE AGG_TEST(ID INT, DEPARTMENT CHAR(20), SALARY FLOAT, AGE INT);
SUCCESS
INSERT INTO AGG_TEST VALUES(1, 'ENGINEERING', 5000.0, 25);
SUCCESS
INSERT INTO AGG_TEST VALUES(2, 'ENGINEERING', 6000.0, 30);
SUCCESS
INSERT INTO AGG_TEST VALUES(3, 'SALES', 4000.0, 28);
SUCCESS
INSERT INTO AGG_TEST VALUES(4, 'SALES', 4500.0, 32);
SUCCESS
INSERT INTO AGG_TEST VALUES(5, 'MARKETING', 5500.0, 27);
SUCCESS
SELECT COUNT(*) FROM AGG_TEST;
COUNT(*)
5
SELECT COUNT(ID) FROM AGG_TEST;
COUNT(ID)
5
SELECT SUM(SALARY) FROM AGG_TEST;
SUM(SALARY)
25000
SELECT AVG(SALARY) FROM AGG_TEST;
AVG(SALARY)
5000
SELECT MIN(SALARY) FROM AGG_TEST;
MIN(SALARY)
4000
SELECT MAX(SALARY) FROM AGG_TEST;
MAX(SALARY)
6000
SELECT DEPARTMENT, COUNT(*) FROM AGG_TEST GROUP BY DEPARTMENT;
DEPARTMENT | COUNT(*)
ENGINEERING | 2
MARKETING | 1
SALES | 2
SELECT DEPARTMENT, AVG(SALARY) FROM AGG_TEST GROUP BY DEPARTMENT;
DEPARTMENT | AVG(SALARY)
ENGINEERING | 5500
MARKETING | 5500
SALES | 4250
SELECT DEPARTMENT, SUM(SALARY), COUNT(*) FROM AGG_TEST GROUP BY DEPARTMENT;
DEPARTMENT | SUM(SALARY) | COUNT(*)
ENGINEERING | 11000 | 2
MARKETING | 5500 | 1
SALES | 8500 | 2
SELECT DEPARTMENT, AGE, COUNT(*) FROM AGG_TEST GROUP BY DEPARTMENT, AGE;
DEPARTMENT | AGE | COUNT(*)
ENGINEERING | 25 | 1
ENGINEERING | 30 | 1
MARKETING | 27 | 1
SALES | 28 | 1
SALES | 32 | 1
SELECT DEPARTMENT, AVG(SALARY) FROM AGG_TEST GROUP BY DEPARTMENT HAVING AVG(SALARY) > 4500;
DEPARTMENT | AVG(SALARY)
ENGINEERING | 5500
MARKETING | 5500
SELECT DEPARTMENT, COUNT(*) FROM AGG_TEST GROUP BY DEPARTMENT HAVING COUNT(*) > 1;
DEPARTMENT | COUNT(*)
ENGINEERING | 2
SALES | 2
DROP TABLE AGG_TEST;
SUCCESS
"""

        self._write_test_files("primary-aggregation-groupby-extended", test_content, result_content)

    def generate_join_tests(self):
        """生成JOIN测试用例"""
        test_content = """-- echo Join Tests
-- Test INNER JOIN functionality

-- Create test tables
create table join_test1(id int, name char(20), dept_id int);
create table join_test2(id int, dept_name char(30), location char(20));

-- Insert test data
insert into join_test1 values(1, 'Alice', 10);
insert into join_test1 values(2, 'Bob', 20);
insert into join_test1 values(3, 'Charlie', 10);
insert into join_test1 values(4, 'David', 30);

insert into join_test2 values(10, 'Engineering', 'Building A');
insert into join_test2 values(20, 'Sales', 'Building B');
insert into join_test2 values(30, 'Marketing', 'Building C');

-- Test basic INNER JOIN
select * from join_test1 inner join join_test2 on join_test1.dept_id = join_test2.id;

-- Test JOIN with specific columns
select join_test1.name, join_test2.dept_name from join_test1 inner join join_test2 on join_test1.dept_id = join_test2.id;

-- Test JOIN with WHERE clause
select join_test1.name, join_test2.dept_name from join_test1 inner join join_test2 on join_test1.dept_id = join_test2.id where join_test2.location = 'Building A';

-- Test JOIN with multiple conditions
select * from join_test1 inner join join_test2 on join_test1.dept_id = join_test2.id and join_test2.id > 15;

-- Clean up
drop table join_test1;
drop table join_test2;
"""

        result_content = """JOIN TESTS
CREATE TABLE JOIN_TEST1(ID INT, NAME CHAR(20), DEPT_ID INT);
SUCCESS
CREATE TABLE JOIN_TEST2(ID INT, DEPT_NAME CHAR(30), LOCATION CHAR(20));
SUCCESS
INSERT INTO JOIN_TEST1 VALUES(1, 'ALICE', 10);
SUCCESS
INSERT INTO JOIN_TEST1 VALUES(2, 'BOB', 20);
SUCCESS
INSERT INTO JOIN_TEST1 VALUES(3, 'CHARLIE', 10);
SUCCESS
INSERT INTO JOIN_TEST1 VALUES(4, 'DAVID', 30);
SUCCESS
INSERT INTO JOIN_TEST2 VALUES(10, 'ENGINEERING', 'BUILDING A');
SUCCESS
INSERT INTO JOIN_TEST2 VALUES(20, 'SALES', 'BUILDING B');
SUCCESS
INSERT INTO JOIN_TEST2 VALUES(30, 'MARKETING', 'BUILDING C');
SUCCESS
SELECT * FROM JOIN_TEST1 INNER JOIN JOIN_TEST2 ON JOIN_TEST1.DEPT_ID = JOIN_TEST2.ID;
ID | NAME | DEPT_ID | ID | DEPT_NAME | LOCATION
1 | ALICE | 10 | 10 | ENGINEERING | BUILDING A
2 | BOB | 20 | 20 | SALES | BUILDING B
3 | CHARLIE | 10 | 10 | ENGINEERING | BUILDING A
4 | DAVID | 30 | 30 | MARKETING | BUILDING C
SELECT JOIN_TEST1.NAME, JOIN_TEST2.DEPT_NAME FROM JOIN_TEST1 INNER JOIN JOIN_TEST2 ON JOIN_TEST1.DEPT_ID = JOIN_TEST2.ID;
NAME | DEPT_NAME
ALICE | ENGINEERING
BOB | SALES
CHARLIE | ENGINEERING
DAVID | MARKETING
SELECT JOIN_TEST1.NAME, JOIN_TEST2.DEPT_NAME FROM JOIN_TEST1 INNER JOIN JOIN_TEST2 ON JOIN_TEST1.DEPT_ID = JOIN_TEST2.ID WHERE JOIN_TEST2.LOCATION = 'BUILDING A';
NAME | DEPT_NAME
ALICE | ENGINEERING
CHARLIE | ENGINEERING
SELECT * FROM JOIN_TEST1 INNER JOIN JOIN_TEST2 ON JOIN_TEST1.DEPT_ID = JOIN_TEST2.ID AND JOIN_TEST2.ID > 15;
ID | NAME | DEPT_ID | ID | DEPT_NAME | LOCATION
2 | BOB | 20 | 20 | SALES | BUILDING B
4 | DAVID | 30 | 30 | MARKETING | BUILDING C
DROP TABLE JOIN_TEST1;
SUCCESS
DROP TABLE JOIN_TEST2;
SUCCESS
"""

        self._write_test_files("primary-join-extended", test_content, result_content)

    def generate_sub_query_tests(self):
        """生成子查询测试用例"""
        test_content = """-- echo Sub Query Tests
-- Test sub-query functionality

-- Create test tables
create table sub_test1(id int, name char(20), score float);
create table sub_test2(id int, category char(20), threshold float);

-- Insert test data
insert into sub_test1 values(1, 'Alice', 85.5);
insert into sub_test1 values(2, 'Bob', 78.0);
insert into sub_test1 values(3, 'Charlie', 92.5);
insert into sub_test1 values(4, 'David', 88.0);

insert into sub_test2 values(1, 'Excellent', 90.0);
insert into sub_test2 values(2, 'Good', 80.0);
insert into sub_test2 values(3, 'Average', 70.0);

-- Test simple IN sub-query
select * from sub_test1 where id in (select id from sub_test2 where threshold > 75);

-- Test EXISTS sub-query
select * from sub_test1 where exists (select 1 from sub_test2 where sub_test2.threshold < sub_test1.score);

-- Test scalar sub-query
select * from sub_test1 where score > (select avg(threshold) from sub_test2);

-- Test NOT IN sub-query
select * from sub_test1 where id not in (select id from sub_test2 where threshold > 85);

-- Clean up
drop table sub_test1;
drop table sub_test2;
"""

        result_content = """SUB QUERY TESTS
CREATE TABLE SUB_TEST1(ID INT, NAME CHAR(20), SCORE FLOAT);
SUCCESS
CREATE TABLE SUB_TEST2(ID INT, CATEGORY CHAR(20), THRESHOLD FLOAT);
SUCCESS
INSERT INTO SUB_TEST1 VALUES(1, 'ALICE', 85.5);
SUCCESS
INSERT INTO SUB_TEST1 VALUES(2, 'BOB', 78.0);
SUCCESS
INSERT INTO SUB_TEST1 VALUES(3, 'CHARLIE', 92.5);
SUCCESS
INSERT INTO SUB_TEST1 VALUES(4, 'DAVID', 88.0);
SUCCESS
INSERT INTO SUB_TEST2 VALUES(1, 'EXCELLENT', 90.0);
SUCCESS
INSERT INTO SUB_TEST2 VALUES(2, 'GOOD', 80.0);
SUCCESS
INSERT INTO SUB_TEST2 VALUES(3, 'AVERAGE', 70.0);
SUCCESS
SELECT * FROM SUB_TEST1 WHERE ID IN (SELECT ID FROM SUB_TEST2 WHERE THRESHOLD > 75);
ID | NAME | SCORE
1 | ALICE | 85.5
2 | BOB | 78
SELECT * FROM SUB_TEST1 WHERE EXISTS (SELECT 1 FROM SUB_TEST2 WHERE SUB_TEST2.THRESHOLD < SUB_TEST1.SCORE);
ID | NAME | SCORE
1 | ALICE | 85.5
3 | CHARLIE | 92.5
4 | DAVID | 88
SELECT * FROM SUB_TEST1 WHERE SCORE > (SELECT AVG(THRESHOLD) FROM SUB_TEST2);
ID | NAME | SCORE
1 | ALICE | 85.5
3 | CHARLIE | 92.5
4 | DAVID | 88
SELECT * FROM SUB_TEST1 WHERE ID NOT IN (SELECT ID FROM SUB_TEST2 WHERE THRESHOLD > 85);
ID | NAME | SCORE
2 | BOB | 78
3 | CHARLIE | 92.5
4 | DAVID | 88
DROP TABLE SUB_TEST1;
SUCCESS
DROP TABLE SUB_TEST2;
SUCCESS
"""

        self._write_test_files("primary-sub-query-extended", test_content, result_content)

    def generate_expression_tests(self):
        """生成表达式测试用例"""
        test_content = """-- echo Expression Tests
-- Test arithmetic expressions in SELECT

-- Create test table
create table expr_test(id int, col1 int, col2 int, col3 float, col4 float);

-- Insert test data
insert into expr_test values(1, 10, 5, 2.5, 1.5);
insert into expr_test values(2, 20, 8, 3.0, 2.0);
insert into expr_test values(3, 15, 3, 4.5, 1.0);

-- Test basic arithmetic operations
select id, col1 + col2 from expr_test;
select id, col1 - col2 from expr_test;
select id, col1 * col2 from expr_test;
select id, col1 / col2 from expr_test;

-- Test complex expressions
select id, (col1 + col2) * col3 from expr_test;
select id, col1 + col2 * col3 from expr_test;
select id, (col1 - col2) / col3 from expr_test;

-- Test expressions with constants
select id, col1 + 10 from expr_test;
select id, col1 * 2 + col2 from expr_test;

-- Test expressions in WHERE clause
select * from expr_test where col1 + col2 > 20;
select * from expr_test where col1 * col2 < 100;

-- Test negative numbers and parentheses
select id, -(col1 + col2) from expr_test;
select id, col1 * (-1) + col2 from expr_test;

-- Clean up
drop table expr_test;
"""

        result_content = """EXPRESSION TESTS
CREATE TABLE EXPR_TEST(ID INT, COL1 INT, COL2 INT, COL3 FLOAT, COL4 FLOAT);
SUCCESS
INSERT INTO EXPR_TEST VALUES(1, 10, 5, 2.5, 1.5);
SUCCESS
INSERT INTO EXPR_TEST VALUES(2, 20, 8, 3.0, 2.0);
SUCCESS
INSERT INTO EXPR_TEST VALUES(3, 15, 3, 4.5, 1.0);
SUCCESS
SELECT ID, COL1 + COL2 FROM EXPR_TEST;
ID | COL1 + COL2
1 | 15
2 | 28
3 | 18
SELECT ID, COL1 - COL2 FROM EXPR_TEST;
ID | COL1 - COL2
1 | 5
2 | 12
3 | 12
SELECT ID, COL1 * COL2 FROM EXPR_TEST;
ID | COL1 * COL2
1 | 50
2 | 160
3 | 45
SELECT ID, COL1 / COL2 FROM EXPR_TEST;
ID | COL1 / COL2
1 | 2
2 | 2.5
3 | 5
SELECT ID, (COL1 + COL2) * COL3 FROM EXPR_TEST;
ID | (COL1 + COL2) * COL3
1 | 37.5
2 | 84
3 | 81
SELECT ID, COL1 + COL2 * COL3 FROM EXPR_TEST;
ID | COL1 + COL2 * COL3
1 | 22.5
2 | 44
3 | 28.5
SELECT ID, (COL1 - COL2) / COL3 FROM EXPR_TEST;
ID | (COL1 - COL2) / COL3
1 | 2
2 | 4
3 | 2.667
SELECT ID, COL1 + 10 FROM EXPR_TEST;
ID | COL1 + 10
1 | 20
2 | 30
3 | 25
SELECT ID, COL1 * 2 + COL2 FROM EXPR_TEST;
ID | COL1 * 2 + COL2
1 | 25
2 | 48
3 | 33
SELECT * FROM EXPR_TEST WHERE COL1 + COL2 > 20;
ID | COL1 | COL2 | COL3 | COL4
2 | 20 | 8 | 3 | 2
SELECT * FROM EXPR_TEST WHERE COL1 * COL2 < 100;
ID | COL1 | COL2 | COL3 | COL4
1 | 10 | 5 | 2.5 | 1.5
3 | 15 | 3 | 4.5 | 1
SELECT ID, -(COL1 + COL2) FROM EXPR_TEST;
ID | -(COL1 + COL2)
1 | -15
2 | -28
3 | -18
SELECT ID, COL1 * (-1) + COL2 FROM EXPR_TEST;
ID | COL1 * (-1) + COL2
1 | -5
2 | -12
3 | -12
DROP TABLE EXPR_TEST;
SUCCESS
"""

        self._write_test_files("primary-expression-extended", test_content, result_content)

    def _write_test_files(self, test_name: str, test_content: str, result_content: str):
        """写入测试文件和结果文件"""
        test_file = os.path.join(self.test_dir, f"{test_name}.test")
        result_file = os.path.join(self.result_dir, f"{test_name}.result")

        with open(test_file, 'w', encoding='utf-8') as f:
            f.write(test_content)

        with open(result_file, 'w', encoding='utf-8') as f:
            f.write(result_content)

        print(f"Generated test case: {test_name}")

def main():
    """主函数"""
    generator = TestCaseGenerator()

    print("🚀 开始生成MiniOB测试用例...")

    # 生成各种功能的测试用例
    generator.generate_date_tests()
    generator.generate_view_tests()
    generator.generate_show_index_tests()
    generator.generate_update_tests()
    generator.generate_vector_tests()
    generator.generate_like_tests()
    generator.generate_null_tests()
    generator.generate_unique_index_tests()
    generator.generate_multi_index_tests()

    # 补充缺失的功能测试
    generator.generate_alias_tests()
    generator.generate_text_tests()
    generator.generate_order_by_tests()
    generator.generate_aggregation_groupby_tests()
    generator.generate_join_tests()
    generator.generate_sub_query_tests()
    generator.generate_expression_tests()

    print("✅ 测试用例生成完成！")

if __name__ == "__main__":
    main()
