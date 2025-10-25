/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "tokenizer/jieba_tokenizer.h"
#include <filesystem>
#include <vector>

// 简化：直接通过公开API调用，不再访问私有成员

static std::string pick_jieba_dict_dir()
{
    namespace fs = std::filesystem;
    const char *env = ::getenv("CPPJIEBA_DICT_DIR");
    std::vector<std::string> cands;
    if (env && *env) cands.emplace_back(env);
    cands.emplace_back("dict");
    cands.emplace_back("./dict");
    cands.emplace_back("../dict");
    cands.emplace_back("deps/3rd/usr/local/dict");
    cands.emplace_back("../deps/3rd/usr/local/dict");
    // absolute fallback to project path (common in this workspace)
    cands.emplace_back("/home/hxl/Desktop/oceambase/miniob-competition-2025/deps/3rd/usr/local/dict");
    cands.emplace_back("/usr/local/share/cppjieba/dict");

    for (const auto &d : cands) {
        fs::path base(d);
        if (fs::exists(base/"jieba.dict.utf8") && fs::exists(base/"hmm_model.utf8")) {
            return base.string();
        }
    }
    return "dict"; // last resort
}

cppjieba::Jieba &JiebaTokenizer::get_instance()
{
    static std::string dict_dir = pick_jieba_dict_dir();
    static cppjieba::Jieba inst(
        (dict_dir + "/jieba.dict.utf8").c_str(),
        (dict_dir + "/hmm_model.utf8").c_str(),
        (dict_dir + "/user.dict.utf8").c_str(),
        (dict_dir + "/idf.utf8").c_str(),
        (dict_dir + "/stop_words.utf8").c_str());
    return inst;
}

RC JiebaTokenizer::cut(std::string &text, std::vector<std::string> &tokens)
{
    // Implement Jieba tokenizer logic here
    // 使用全局单例，避免每次创建加载词典
    auto &jieba = get_instance();
    jieba.Cut(text, tokens);
    // 简化：不做停用词过滤（题目未强制，保持最小可行）
    return RC::SUCCESS;
}