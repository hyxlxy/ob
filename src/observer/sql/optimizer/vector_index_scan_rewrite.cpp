#include "sql/optimizer/vector_index_scan_rewrite.h"
#include "sql/operator/logical_operator.h"
#include "sql/operator/order_by_logical_operator.h"
#include "sql/operator/limit_logical_operator.h"
#include "sql/operator/table_get_logical_operator.h"
#include "sql/expr/expression.h"
#include "sql/expr/expression_tuple.h"
#include "storage/table/table.h"

// 简化版：捕捉 LIMIT(顶端) + ORDER BY(距离函数升序) + TABLE_GET
RC VectorIndexScanRewrite::rewrite(std::unique_ptr<LogicalOperator> &oper, bool &change_made)
{
  change_made = false;
  auto &children = oper->children();
  if (children.size() != 1) return RC::SUCCESS;

  auto &first = children[0];
  if (first->type() != LogicalOperatorType::LIMIT) return RC::SUCCESS;

  auto *limit_oper = dynamic_cast<LimitLogicalOperator *>(first.get());
  auto &grand = first->children();
  if (grand.size() != 1 || grand[0]->type() != LogicalOperatorType::ORDER_BY) return RC::SUCCESS;

  auto *orderby_oper = dynamic_cast<OrderByLogicalOperator *>(grand[0].get());
  auto &ob_exprs = orderby_oper->order_by_expressions();
  auto &ob_dirs  = orderby_oper->order_by_directions();
  if (ob_exprs.size() != 1 || ob_dirs.size() != 1 || ob_dirs[0] != ASC_ORDER) return RC::SUCCESS;

  // 仅支持普通函数表达式，且为距离函数(L2/COSINE/IP)。此处以函数名判断。
  Expression *expr = ob_exprs[0].get();
  if (expr->type() != ExprType::FUNCTION) return RC::SUCCESS;
  auto *func = dynamic_cast<FunctionExpr *>(expr);
  if (func == nullptr) return RC::SUCCESS;
  std::string fname = func->function_name();
  for (auto &c : fname) c = toupper(c);
  if (!(fname == "L2_DISTANCE" || fname == "COSINE_DISTANCE" || fname == "INNER_PRODUCT")) return RC::SUCCESS;

  // 仅当其子节点为 TABLE_GET 才考虑
  auto &scan_child = grand[0]->children();
  if (scan_child.size() != 1 || scan_child[0]->type() != LogicalOperatorType::TABLE_GET) return RC::SUCCESS;

  auto *table_get = dynamic_cast<TableGetLogicalOperator *>(scan_child[0].get());
  Table *table = table_get->table();
  if (table == nullptr) return RC::SUCCESS;

  // 从函数参数中提取：向量列字段与常量向量
  RowTuple dummy;
  Value    literal;
  FieldExpr *field_expr = nullptr;
  if (func->args().size() != 2) return RC::SUCCESS;
  if (func->args()[0]->type() == ExprType::FIELD) {
    field_expr = dynamic_cast<FieldExpr *>(func->args()[0].get());
    func->args()[1]->get_value(dummy, literal);
  } else if (func->args()[1]->type() == ExprType::FIELD) {
    field_expr = dynamic_cast<FieldExpr *>(func->args()[1].get());
    func->args()[0]->get_value(dummy, literal);
  } else {
    return RC::SUCCESS;
  }

  Index *index = table->find_index_by_field(field_expr->field().field_name());
  if (index == nullptr) {
    return RC::SUCCESS;
  }

  // 当前工程未提供设置 index/base_vector/limit 的接口。
  // 暂不进行结构改写，保留 ORDER BY + LIMIT，后续可在物理阶段识别并替换。
  (void)limit_oper; (void)literal; (void)index;
  return RC::SUCCESS;
}


