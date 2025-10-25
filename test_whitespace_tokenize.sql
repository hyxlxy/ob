-- 测试 whitespace_tokenize 函数

-- 测试1: 基本功能测试
SELECT whitespace_tokenize(" I   am apple  ");
-- 期望输出: ["I","am","apple"]

-- 测试2: NULL值测试
SELECT whitespace_tokenize(NULL);
-- 期望输出: NULL

-- 测试3: 空字符串测试
SELECT whitespace_tokenize("");
-- 期望输出: []

-- 测试4: 只有空格的字符串
SELECT whitespace_tokenize("   ");
-- 期望输出: []

-- 测试5: 单个单词
SELECT whitespace_tokenize("hello");
-- 期望输出: ["hello"]

-- 测试6: 多种空白字符（空格、制表符、换行符）
SELECT whitespace_tokenize("hello	world
test");
-- 期望输出: ["hello","world","test"]

-- 测试7: 前后有空格
SELECT whitespace_tokenize("  hello world  ");
-- 期望输出: ["hello","world"]

-- 测试8: 中间有多个空格
SELECT whitespace_tokenize("one  two   three");
-- 期望输出: ["one","two","three"]

