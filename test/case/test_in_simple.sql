CREATE TABLE sub_t1(id int, col1 int);
CREATE TABLE sub_t2(id int, col2 int);
INSERT INTO sub_t1 VALUES (2, 20);
INSERT INTO sub_t2 VALUES (2, 20);
SELECT * FROM sub_t1 WHERE col1 IN (SELECT col2 FROM sub_t2);
