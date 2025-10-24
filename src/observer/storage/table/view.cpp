
#include "storage/table/view.h"
#include "sql/stmt/select_stmt.h"
#include "sql/expr/expression.h"
#include "storage/db/db.h"

View::~View()
{

  if (stmt_) {
    delete stmt_;
  }
}

struct SaveFieldMeta
{
  set<FieldMeta *> S;
  ~SaveFieldMeta()
  {
    for (auto &&x : S) {
      delete x;
    }
  }
  void add_fieldmeta(FieldMeta *one) { S.insert(one); }
};
static SaveFieldMeta save_fieldmeta;

//第一个是Field，第二个是列的别名
/*
首先先捋清楚：
对于view需要先提取踹select_stmt里面的查询的对应的字段field有哪些
得到他们的（字段+别名)的组合


但是这里一个view里的
*/
vector<pair<Field, string>> View::get_fields(Db *db, SelectStmt *stmt_)
{
  vector<pair<Field, string>> fields;
  auto                      &&select_stmt = dynamic_cast<SelectStmt *>(stmt_);
  for (auto &&expr : select_stmt->query_expressions()) {
    if (expr->type() == ExprType::FIELD) {
      // 此时已经将所有的 UnboundFieldExpr 转换为 FieldExpr
      auto   field_expr = static_cast<FieldExpr *>(expr.get());
      Table *table      = db->find_table(field_expr->table_name());
      if (table != nullptr) {
        Field field{table, field_expr->field().meta()};

        fields.push_back({field, ""});
        if (strlen(expr->alias()) != 0) {
          fields.back().second = expr->alias();
        } else {
          //这里不是expr的name，而是field的name，因为这里的expr本身就是field
          fields.back().second = field.field_name();
        }
      } else {
        //代表目前处于select view的嵌套
        View *view = db->find_view(field_expr->table_name());
        ASSERT(view != nullptr,"no way here view.cpp: get_fields");
        cout << "view.cpp: here view" << endl;
        Field field{view, field_expr->field().meta()};
        fields.push_back({field, ""});
        if (strlen(expr->alias()) != 0) {
          fields.back().second = string{expr->alias()};
        } else {
          fields.back().second = field.field_name();
        }
      }

      continue;
    }
    std::string field_name = expr->name();
    // static map<FieldMeta, bool> mp;
    FieldMeta *filed_meta = new FieldMeta();
    save_fieldmeta.add_fieldmeta(filed_meta);
    filed_meta->init(field_name.c_str(), expr->value_type(), 0, expr->value_length(), true, 0);
    Field field{filed_meta};
    fields.push_back({field, ""});
    if (strlen(expr->alias()) != 0) {
      fields.back().second = expr->alias();
    } else {
      fields.back().second = expr->name();
    }
  }
  return fields;
}

void View::set_table_field_name(Db *db)
{
  // attr_from_table_and_field
  auto &&select_stmt = dynamic_cast<SelectStmt *>(stmt_);
  auto &&fields      = View::get_fields(db, select_stmt);
  for (size_t i = 0; i < fields.size(); i++) {
    auto &&field = fields[i].first;
    if (field.table() == nullptr) {
      attr_from_table_and_field_[attr_names_[i]] = {"", field.field_name()};

    } else {
      attr_from_table_and_field_[attr_names_[i]] = {field.table_name(), field.field_name()};
    }
  }

  //   for (auto &&expr : select_stmt->query_expressions()) {
  // if (expr->type() == ExprType::FIELD) {
  // 此时已经将所有的 UnboundFieldExpr 转换为 FieldExpr
  //   auto field_expr = static_cast<FieldExpr *>(expr.get());
  //   Field field(db->find_table(field_expr->table_name()), field_expr->field().meta());
  // 记录表名
  //   if (strlen(expr->alias()) != 0) {
  // 别名覆盖字段名
  // attr_from_table_and_field_[expr->alias()] = {field_expr->field().table_name(), field_expr->field_name()};
  //   } else {
  // attr_from_table_and_field_[expr->name()] = {field_expr->field().table_name(), field_expr->field_name()};
  //   }

  //   continue;
  // }
  //   }
}

void View::set_table_meta(Db *db)
{

  auto                      &&select_stmt = dynamic_cast<SelectStmt *>(stmt_);
  vector<pair<Field, string>> fields      = View::get_fields(db, select_stmt);

  //   vector<Field> fields;

  //   auto &&select_stmt = dynamic_cast<SelectStmt *>(stmt_);
  //   for (auto &&expr : select_stmt->query_expressions()) {
  //     if (expr->type() == ExprType::FIELD) {
  //       // 此时已经将所有的 UnboundFieldExpr 转换为 FieldExpr
  //       auto  field_expr = static_cast<FieldExpr *>(expr.get());
  //       Field field(db->find_table(field_expr->table_name()), field_expr->field().meta());
  //       fields.push_back(field);
  //       continue;
  //     }
  //     std::string field_name;
  //     field_name = expr->name();
  //     FieldMeta filed_meta;
  //     filed_meta.init(field_name.c_str(), expr->value_type(), 0, expr->value_length(), true, 0);
  //     Field field{&filed_meta};
  //     fields.push_back(field);
  //   }

  std::vector<AttrInfoSqlNode> attr_infos;

  for (size_t i = 0; i < fields.size(); i++) {
    auto &&field = fields[i].first;
    //这里应该是second才对，因为是别名
    AttrInfoSqlNode one{fields[i].second, field.attr_type()};
    // if (field.attr_type() == AttrType::CHARS) {
    one.length = field.meta()->len();
    cout << "make_field: name: " << fields[i].second << " len: " << field.meta()->len() << endl;
    // }
    // if (one.nullable)
    //   one.length++;
    attr_infos.push_back(std::move(one));
    if (attr_names_.empty()) {
      continue;
    }
    attr_infos.back().name = attr_names_[i];
  }

  table_meta_.init(
      view_id_, view_name_.c_str(), nullptr, attr_infos, {}, StorageFormat::ROW_FORMAT, StorageEngine::HEAP);
}
