-- Test to check if capacity is the issue

-- Query 1: With TopN pushdown (current implementation)
SELECT docid_col, MATCH(fulltext_col) AGAINST('test') as _score
FROM test_insert_en
WHERE MATCH(fulltext_col) AGAINST('test')
  AND base_id IN ('base_id_1','base_id_2','base_id_3')
  AND id < 1000
ORDER BY _score DESC
LIMIT 10;

-- Query 2: Check how many rows match the filter conditions
SELECT COUNT(*) as total_matches
FROM test_insert_en
WHERE MATCH(fulltext_col) AGAINST('test')
  AND base_id IN ('base_id_1','base_id_2','base_id_3')
  AND id < 1000;
