ALTER SYSTEM SET _enable_index_merge=true;


-- 关于test_insert_en的测试语句
SELECT docid_col, MATCH(fulltext_col) AGAINST('test') as _score
FROM test_insert_en
WHERE MATCH(fulltext_col) AGAINST('text')
  and base_id in ('base_id_1', 'base_id_2', 'base_id_3')
  and id < 1000
ORDER BY _score DESC
LIMIT 10;

explain SELECT docid_col, MATCH(fulltext_col) AGAINST('test') as _score FROM test_insert_en WHERE MATCH(fulltext_col) AGAINST('text')   and id < 1000 ORDER BY _score DESC LIMIT 10;