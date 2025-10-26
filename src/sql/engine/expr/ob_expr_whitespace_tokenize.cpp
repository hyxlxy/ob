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
#include "lib/ob_errno.h"
#include "lib/oblog/ob_log_module.h"
#include "lib/string/ob_string.h"
#include "lib/udt/ob_array_type.h"
#include "sql/engine/expr/ob_array_expr_utils.h"
#include "sql/engine/expr/ob_expr_result_type_util.h"
#include "lib/string/ob_string_buffer.h"

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

  if (OB_FAIL(expr.args_[0]->eval(ctx, text_datum))) {
    LOG_WARN("Failed to eval text argument", K(ret));
  } else if (text_datum->is_null()) {
    expr_datum.set_null();
  } else {
    ObString input_text = text_datum->get_string();

    ObEvalCtx::TempAllocGuard tmp_alloc_g(ctx);
    common::ObArenaAllocator &tmp_allocator = tmp_alloc_g.get_allocator();

    const char *str = input_text.ptr();
    const int32_t len = input_text.length();
    int32_t start = 0;
    bool in_word = false;
    bool first = true;

    ObStringBuffer buf(&tmp_allocator);
    if (OB_FAIL(buf.append("["))) {
      LOG_WARN("append left bracket failed", K(ret));
    }
    for (int32_t i = 0; OB_SUCC(ret) && i <= len; ++i) {
      const bool is_space = (i < len) && (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r');
      if (i < len && !is_space) {
        if (!in_word) {
          start = i;
          in_word = true;
        }
      } else {
        if (in_word) {
          const int32_t word_len = i - start;
          if (word_len > 0) {
            if (!first) {
              if (OB_FAIL(buf.append(","))) {
                LOG_WARN("append comma failed", K(ret));
              }
            }
            if (OB_SUCC(ret)) {
              if (OB_FAIL(buf.append(ObString(word_len, str + start)))) {
                LOG_WARN("append token failed", K(ret));
              } else {
                first = false;
              }
            }
          }
          in_word = false;
        }
      }
    }
    if (OB_SUCC(ret) && OB_FAIL(buf.append("]"))) {
      LOG_WARN("append right bracket failed", K(ret));
    }

    if (OB_SUCC(ret)) {
      ObTextStringDatumResult str_result(expr.datum_meta_.type_, &expr, &ctx, &expr_datum);
      if (OB_FAIL(str_result.init(buf.length()))) {
        LOG_WARN("failed to init text result", K(ret), K(buf.length()));
      } else if (OB_FAIL(str_result.append(buf.ptr(), buf.length()))) {
        LOG_WARN("failed to append result", K(ret));
      } else {
        str_result.set_result();
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
  UNUSED(type_ctx);

  if (ob_is_string_type(text.get_type())) {
    if (text.get_charset_type() != CHARSET_UTF8MB4) {
      text.set_calc_collation_type(CS_TYPE_UTF8MB4_BIN);
    }
  } else if (!ob_is_null(text.get_type())) {
    ret = OB_ERR_INVALID_TYPE_FOR_OP;
    LOG_USER_ERROR(OB_ERR_INVALID_TYPE_FOR_OP, "VARCHAR", ob_obj_type_str(text.get_type()));
  }

  if (OB_SUCC(ret)) {
    type.set_type(ObLongTextType);
    type.set_collation_type(CS_TYPE_UTF8MB4_BIN);
    type.set_collation_level(CS_LEVEL_IMPLICIT);
    type.set_accuracy(ObAccuracy::DDL_DEFAULT_ACCURACY[ObLongTextType]);
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

