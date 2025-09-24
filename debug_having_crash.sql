-- Test HAVING clause crash debugging
CREATE TABLE t_group_by(id int, score int);
INSERT INTO t_group_by VALUES (1, 2);
INSERT INTO t_group_by VALUES (1, 3);
INSERT INTO t_group_by VALUES (2, 1);
INSERT INTO t_group_by VALUES (2, 4);

-- Test basic GROUP BY first
SELECT id, sum(score) FROM t_group_by GROUP BY id;

-- Test HAVING clause that crashes
SELECT id, sum(score) FROM t_group_by GROUP BY id HAVING sum(score) > 4;

DROP TABLE t_group_by;
