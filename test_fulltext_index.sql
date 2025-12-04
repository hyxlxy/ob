DROP TABLE IF EXISTS `items1`;
CREATE TABLE `items1` (`id` BIGINT NOT NULL AUTO_INCREMENT, `base_id` VARCHAR(100), `docid_col` VARCHAR(100), `fulltext_col` TEXT, PRIMARY KEY (`id`));
INSERT INTO `items1` (`base_id`, `docid_col`, `fulltext_col`) VALUES ('base_id_1', 'doc_001', 'database search fulltext'), ('base_id_2', 'doc_002', 'fulltext index search'), ('base_id_3', 'doc_003', 'database index performance'), ('base_id_1', 'doc_004', 'search engine fulltext'), ('base_id_2', 'doc_005', 'database fulltext index');
CREATE INDEX idx_base_id ON `items1`(`base_id`);
CREATE INDEX idx_docid ON `items1`(`docid_col`);
CREATE /*+ parallel(90) */ FULLTEXT INDEX ft_fulltext ON `items1`(`fulltext_col`);
SELECT `docid_col`, MATCH(`fulltext_col`) AGAINST('search' IN NATURAL LANGUAGE MODE) as _score FROM `items1` WHERE MATCH(`fulltext_col`) AGAINST('search' IN NATURAL LANGUAGE MODE) AND `base_id` IN ('base_id_1', 'base_id_2', 'base_id_3') AND `id` < 1000 ORDER BY _score DESC LIMIT 10;
