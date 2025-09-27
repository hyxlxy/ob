-- Detailed debug test for OR subquery
CREATE TABLE csq_1(id int, col1 int, feat1 float);
CREATE TABLE csq_2(id int, col2 int, feat2 float);
CREATE TABLE csq_3(id int, col3 int, feat3 float);

INSERT INTO csq_1 VALUES (32, 77, 59.79);
INSERT INTO csq_1 VALUES (1, 10, 5.5);
INSERT INTO csq_1 VALUES (2, 20, 15.3);

INSERT INTO csq_2 VALUES (1, 5, 10.0);
INSERT INTO csq_2 VALUES (2, 15, 20.5);
INSERT INTO csq_2 VALUES (3, 25, 30.8);

INSERT INTO csq_3 VALUES (1, 2, 11.0);
INSERT INTO csq_3 VALUES (3, 6, 16.5);
INSERT INTO csq_3 VALUES (5, 5, 14.6);

-- Test each condition separately for each row
SELECT 'Row 32: condition1' as test, 
       CASE WHEN (SELECT MAX(feat2) FROM csq_2) > 59.79 THEN 'TRUE' ELSE 'FALSE' END as result;

SELECT 'Row 32: condition2' as test,
       CASE WHEN (SELECT MIN(col3) FROM csq_3) < 77 THEN 'TRUE' ELSE 'FALSE' END as result;

SELECT 'Row 1: condition1' as test, 
       CASE WHEN (SELECT MAX(feat2) FROM csq_2) > 5.5 THEN 'TRUE' ELSE 'FALSE' END as result;

SELECT 'Row 1: condition2' as test,
       CASE WHEN (SELECT MIN(col3) FROM csq_3) < 10 THEN 'TRUE' ELSE 'FALSE' END as result;

SELECT 'Row 2: condition1' as test, 
       CASE WHEN (SELECT MAX(feat2) FROM csq_2) > 15.3 THEN 'TRUE' ELSE 'FALSE' END as result;

SELECT 'Row 2: condition2' as test,
       CASE WHEN (SELECT MIN(col3) FROM csq_3) < 20 THEN 'TRUE' ELSE 'FALSE' END as result;

-- Test the OR condition for each row
SELECT 'Row 32: OR result' as test,
       CASE WHEN (SELECT MAX(feat2) FROM csq_2) > 59.79 OR (SELECT MIN(col3) FROM csq_3) < 77 THEN 'TRUE' ELSE 'FALSE' END as result;

SELECT 'Row 1: OR result' as test,
       CASE WHEN (SELECT MAX(feat2) FROM csq_2) > 5.5 OR (SELECT MIN(col3) FROM csq_3) < 10 THEN 'TRUE' ELSE 'FALSE' END as result;

SELECT 'Row 2: OR result' as test,
       CASE WHEN (SELECT MAX(feat2) FROM csq_2) > 15.3 OR (SELECT MIN(col3) FROM csq_3) < 20 THEN 'TRUE' ELSE 'FALSE' END as result;

DROP TABLE csq_1;
DROP TABLE csq_2;
DROP TABLE csq_3;
