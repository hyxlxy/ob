/**
 * Copyright (c) 2024 OceanBase
 * OceanBase CE is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan PubL v2.
 * You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#define USING_LOG_PREFIX SQL_ENG

#include "sql/engine/expr/ob_expr_whitespace_tokenize.h"
#include "lib/json_type/ob_json_base.h"
#include "lib/json_type/ob_json_tree.h"
#include "lib/ob_errno.h"
#include "lib/oblog/ob_log_module.h"
#include "lib/string/ob_string.h"
#include "sql/engine/expr/ob_expr_json_func_helper.h"

namespace oceanbase
{
namespace sql
{

ObExprWhitespaceTokenize::ObExprWhitespaceTokenize(common::ObIAllocator &alloc)
    : ObStringExprOperator(alloc,
                           T_FUN_WHITESPACE_TOKENIZE,
                           N_WHITESPACE_TOKENIZE,
                           1,
                           VALID_FOR_GENERATED_COL)
{
}

ObExprWhitespaceTokenize::~ObExprWhitespaceTokenize() {}

int ObExprWhitespaceTokenize::eval_whitespace_tokenize(const ObExpr &expr, 
                                                         ObEvalCtx &ctx, 
                                                         ObDatum &expr_datum)
{
  int ret = OB_SUCCESS;
  ObDatum *text_datum = nullptr;
  
  // 获取输入参数
  if (OB_FAIL(expr.args_[0]->eval(ctx, text_datum))) {
    LOG_WARN("Failed to eval text argument", K(ret));
  } else if (text_datum->is_null()) {
    // 如果输入为NULL，返回NULL
    expr_datum.set_null();
  } else {
    // 获取输入字符串
    ObString input_text = text_datum->get_string();
    
    // 创建临时allocator
    ObEvalCtx::TempAllocGuard tmp_alloc_g(ctx);
    common::ObArenaAllocator &temp_allocator = tmp_alloc_g.get_allocator();
    
    // 创建JSON数组
    void *buf = temp_allocator.alloc(sizeof(ObJsonArray));
    if (OB_ISNULL(buf)) {
      ret = OB_ALLOCATE_MEMORY_FAILED;
      LOG_WARN("Failed to allocate memory for json array", K(ret));
    } else {
      ObJsonArray *json_array = new (buf) ObJsonArray(&temp_allocator);
      ObIJsonBase *json_result = json_array;
      
      // 对输入文本进行空格分词
      const char *str = input_text.ptr();
      int32_t len = input_text.length();
      int32_t start = 0;
      bool in_word = false;
      
      for (int32_t i = 0; i <= len && OB_SUCC(ret); ++i) {
        bool is_space = (i < len) && (str[i] == ' ' || str[i] == '\t' || 
                                       str[i] == '\n' || str[i] == '\r');
        
        if (i < len && !is_space) {
          // 遇到非空格字符
          if (!in_word) {
            start = i;
            in_word = true;
          }
        } else {
          // 遇到空格或字符串结束
          if (in_word) {
            // 提取单词
            int32_t word_len = i - start;
            if (word_len > 0) {
              // 创建JSON字符串
              void *str_buf = temp_allocator.alloc(sizeof(ObJsonString));
              if (OB_ISNULL(str_buf)) {
                ret = OB_ALLOCATE_MEMORY_FAILED;
                LOG_WARN("Failed to allocate memory for json string", K(ret));
              } else {
                ObString word(word_len, str + start);
                ObJsonString *json_str = new (str_buf) ObJsonString(word.ptr(), word_len);
                if (OB_FAIL(json_array->append(json_str))) {
                  LOG_WARN("Failed to append json string to array", K(ret));
                }
              }
            }
            in_word = false;
          }
        }
      }
      
      // 将JSON结果打包到datum中
      if (OB_SUCC(ret)) {
        if (OB_FAIL(ObJsonExprHelper::pack_json_res(expr, ctx, temp_allocator, 
                                                     json_result, expr_datum))) {
          LOG_WARN("Failed to pack json result", K(ret));
        }
      }
    }
  }
  
  return ret;
}

int ObExprWhitespaceTokenize::calc_result_type1(ObExprResType &type,
                                                  ObExprResType &text,
                                                  common::ObExprTypeCtx &type_ctx) const
{
  int ret = OB_SUCCESS;
  
  // 设置返回类型为JSON
  type.set_json();
  type.set_length(static_cast<common::ObLength>(OB_MAX_TEXT_LENGTH));
  
  // 输入参数应该是字符串类型
  if (ob_is_string_type(text.get_type())) {
    if (text.get_charset_type() != CHARSET_UTF8MB4) {
      text.set_calc_collation_type(CS_TYPE_UTF8MB4_BIN);
    }
  }
  
  return ret;
}

int ObExprWhitespaceTokenize::cg_expr(ObExprCGCtx &op_cg_ctx,
                                       const ObRawExpr &raw_expr,
                                       ObExpr &rt_expr) const
{
  int ret = OB_SUCCESS;
  UNUSED(op_cg_ctx);
  UNUSED(raw_expr);
  
  if (rt_expr.arg_cnt_ != 1) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("Invalid argument count", K(ret), K(rt_expr.arg_cnt_));
  } else {
    rt_expr.eval_func_ = eval_whitespace_tokenize;
  }
  
  return ret;
}

} // namespace sql
} // namespace oceanbase

