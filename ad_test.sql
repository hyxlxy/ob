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



// 正确
SELECT docid_col, MATCH(fulltext_col) AGAINST('test') as _score FROM test_insert_en WHERE MATCH(fulltext_col) AGAINST('text')  and base_id in ('base_id_1','base_i
d_2','base_id_3') and id < 1000 ORDER BY _score DESC LIMIT 5;
+--------------+--------------------+
| docid_col    | _score             |
+--------------+--------------------+
| doc-en-8205  |  2.997040121248045 |
| doc-en-740   | 2.7540923852835766 |
| doc-en-1632  | 2.5506717135132715 |
| doc-en-15078 | 2.5491624736824576 |
| doc-en-6894  | 2.5439996893520718 |
+--------------+--------------------+
5 rows in set (0.025 sec)


obclient [test]> 
SELECT docid_col, MATCH(fulltext_col) AGAINST('test') as _score FROM test_insert_en WHERE MATCH(fulltext_col) AGAINST('text')  and id < 1000 ORDER BY _score DESC LIMIT 5;
+--------------+--------------------+
| docid_col    | _score             |
+--------------+--------------------+
| doc-en-8205  |  2.997040121248045 |
| doc-en-740   | 2.7540923852835766 |
| doc-en-1632  | 2.5506717135132715 |
| doc-en-15078 | 2.5491624736824576 |
| doc-en-6894  | 2.5439996893520718 |
+--------------+--------------------+
5 rows in set (0.016 sec)

obclient [test]> 


obclient [test]> SELECT docid_col, MATCH(fulltext_col) AGAINST('test') as _score FROM test_insert_en WHERE MATCH(fulltext_col) AGAINST('text')  ORDER BY _score DESC LIMIT 5;
+--------------+-------------------+
| docid_col    | _score            |
+--------------+-------------------+
| doc-en-3910  | 3.177184168059783 |
| doc-en-14089 | 3.170971333845572 |
| doc-en-6897  | 3.151959900550657 |
| doc-en-11211 | 3.149975263248712 |
| doc-en-7737  | 3.075066100102328 |
+--------------+-------------------+
5 rows in set (0.018 sec)


obclient [test]> SELECT docid_col, MATCH(fulltext_col) AGAINST('test') as _score FROM test_insert_en WHERE  base_id in ('base_id_1','base_id_2','base_id_3') and id < 1000 ORDER BY
 _score DESC LIMIT 5;
+--------------+--------------------+
| docid_col    | _score             |
+--------------+--------------------+
| doc-en-11420 |  3.155074827956685 |
| doc-en-42    | 3.1263993351429438 |
| doc-en-14705 | 3.0939018121615347 |
| doc-en-5478  |  3.074712165892507 |
| doc-en-10075 |  3.057608011609973 |
+--------------+--------------------+
5 rows in set (0.013 sec)

obclient [test]> 


obclient [test]> SELECT docid_col, MATCH(fulltext_col) AGAINST('test') as _score FROM test_insert_en WHERE MATCH(fulltext_col) AGAINST('text')  and base_id in ('base_id_1','base_i 
d_2','base_id_3') ORDER BY _score DESC LIMIT 5;
+--------------+-------------------+
| docid_col    | _score            |
+--------------+-------------------+
| doc-en-3910  | 3.177184168059783 |
| doc-en-14089 | 3.170971333845572 |
| doc-en-6897  | 3.151959900550657 |
| doc-en-7737  | 3.075066100102328 |
| doc-en-8205  | 2.997040121248045 |
+--------------+-------------------+
5 rows in set (0.018 sec)