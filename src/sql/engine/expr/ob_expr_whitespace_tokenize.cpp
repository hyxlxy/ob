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

    const uint16_t subschema_id = expr.obj_meta_.get_subschema_id();
    ObIArrayType *arr_obj = NULL;
    ObArrayBinary *binary_array = NULL;

    if (OB_FAIL(ObArrayExprUtils::construct_array_obj(tmp_allocator, ctx, subschema_id, arr_obj, false))) {
      LOG_WARN("construct array obj failed", K(ret), K(subschema_id));
    } else if (OB_ISNULL(binary_array = static_cast<ObArrayBinary *>(arr_obj))) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("binary array is null", K(ret), K(subschema_id));
    } else {
      const char *str = input_text.ptr();
      const int32_t len = input_text.length();
      int32_t start = 0;
      bool in_word = false;

      for (int32_t i = 0; i <= len && OB_SUCC(ret); ++i) {
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
              const ObString word(word_len, str + start);
              if (OB_FAIL(binary_array->push_back(word))) {
                LOG_WARN("failed to push token into array", K(ret));
              }
            }
            in_word = false;
          }
        }
      }

      if (OB_SUCC(ret)) {
        ObString res_str;
        if (OB_FAIL(ObArrayExprUtils::set_array_res(arr_obj, arr_obj->get_raw_binary_len(), expr, ctx, res_str))) {
          LOG_WARN("get array binary string failed", K(ret));
        } else {
          expr_datum.set_string(res_str);
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

  ObSQLSessionInfo *session = const_cast<ObSQLSessionInfo *>(type_ctx.get_session());
  ObExecContext *exec_ctx = OB_ISNULL(session) ? NULL : session->get_cur_exec_ctx();

  if (OB_ISNULL(exec_ctx)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("exec ctx is null", K(ret));
  } else {
    if (ob_is_string_type(text.get_type())) {
      if (text.get_charset_type() != CHARSET_UTF8MB4) {
        text.set_calc_collation_type(CS_TYPE_UTF8MB4_BIN);
      }
    } else if (!ob_is_null(text.get_type())) {
      ret = OB_ERR_INVALID_TYPE_FOR_OP;
      LOG_USER_ERROR(OB_ERR_INVALID_TYPE_FOR_OP, "VARCHAR", ob_obj_type_str(text.get_type()));
    }

    if (OB_SUCC(ret)) {
      uint16_t subschema_id;
      ObDataType res_data_type;
      ObObjMeta meta;
      meta.set_varchar();
      res_data_type.set_meta_type(meta);
      res_data_type.set_length(OB_MAX_VARCHAR_LENGTH / 4);
      if (OB_FAIL(exec_ctx->get_subschema_id_by_collection_elem_type(ObNestedType::OB_ARRAY_TYPE,
                                                                     res_data_type,
                                                                     subschema_id))) {
        LOG_WARN("failed to get collection subschema id", K(ret));
      } else {
        type.set_collection(subschema_id);
      }
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

