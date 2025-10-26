

#include "sql/stmt/create_view_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "sql/expr/expression.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "storage/table/view.h"
#include "sql/parser/expression_binder.h"
#include <algorithm>

CreateViewStmt::CreateViewStmt(const string &view_name, const vector<string> &attr_names, string def)
    : view_name_(view_name), attr_names_(attr_names), def_(def)
{}

void CreateViewStmt::set_select_stmt(unique_ptr<SelectStmt> &&select_stmt) { select_stmt_ = std::move(select_stmt); }

RC CreateViewStmt::create(Db *db, const CreateViewSqlNode &create_view, Stmt *&stmt)
{
  RC rc = RC::SUCCESS;
  if (nullptr == db) {
    LOG_WARN("invalid argument. db is null");
    return RC::INVALID_ARGUMENT;
  }

  //检测一下view的名字是否重复
  {
    auto &&view = db->find_table(create_view.relation_name.c_str());
    if (view != nullptr) {
      return RC::ERROR;
    }
    auto &&view2 = db->find_view(create_view.relation_name.c_str());
    if (view2 != nullptr) {
      return RC::ERROR;
    }
  }

  unique_ptr<SelectStmt> select_stmt;
  //搞出来select_stmt
  {
    Stmt *sstmt = nullptr;
    rc          = SelectStmt::create(db, create_view.selection->selection, sstmt);
    CHECK_RETURN_RC(rc);
    auto one = dynamic_cast<SelectStmt *>(sstmt);
    select_stmt.reset(one);
  }

  //检测字段是否符合
  if (!create_view.attr_names.empty() and select_stmt->query_expressions().size() != create_view.attr_names.size()) {
    return RC::ERROR;
  }

  //字段非空，判断是否重复
  if (!create_view.attr_names.empty()) {
    //检测字段名字的合法性
    std::unordered_map<string, bool> mp;
    for (auto &&one : create_view.attr_names) {
      mp.insert({one, 1});
    }
    if (mp.size() != create_view.attr_names.size()) {
      return RC::ERROR;
    }
  }

  //开始提取field
  vector<pair<Field, string>> fields = View::get_fields(db, select_stmt.get());
  //开始提取出来table
  vector<Table *> tables;
  {
    set<Table *> table_set;
    for (auto &&[field, alias] : fields) {
      if (field.table() == nullptr) {
        continue;
      }
      const char *table_name = field.table_name();
      if (strlen(table_name) != 0) {
        //存在table_name
        Table *one = db->find_table(table_name);
        //之前已经检测过，都会存在的
        if (one != nullptr) {
          // tables.push_back(one);
          table_set.insert(one);
          continue;
        }
      }
      View *view_one = db->find_view(table_name);
      if (view_one != nullptr) {
        for (auto &&table_one : view_one->base_tables()) {
          // tables.push_back(table_one);
          table_set.insert(table_one);
        }
        continue;
      }
    }
    // cout << "view_base_tables:names:";
    for (auto &&one : table_set) {
      tables.push_back(one);
      cout << one->name() << " ";
    }
    cout << endl;
  }

  bool can_be_update  = true;
  bool have_agg_or_ma = false;
  bool only_one_table = true;
  do {
    //如果多个表，或者有一些特殊的表达式，聚合，算数之类的，就不可以

    if (select_stmt->tables().size() > 1 or tables.size() > 1) {
      //因为有可能tables里面是view，而这个view是非修改的。 所以这里还需要check一下这个base_tables的size
      can_be_update  = false;
      only_one_table = false;
    }

    for (auto &&one : select_stmt->query_expressions()) {
      if (one->type() == ExprType::AGGREGATION or one->type() == ExprType::ARITHMETIC) {
        can_be_update  = false;
        have_agg_or_ma = true;
      }
    }

  } while (0);

  // 如果是空的，那么强制转换，然后把解析到了field放到我们create_view.names里面去
  // 但是要记得，这里需要再判断一次名字是否重复
  if (create_view.attr_names.empty()) {
    vector<string>              names;
    unordered_map<string, bool> flag_mp;
    for (auto &&[one, alias] : fields) {
      if (alias.empty()) {
        names.push_back(one.field_name());
      } else {
        names.push_back(alias);
      }
      flag_mp[one.field_name()] = true;
    }
    if (flag_mp.size() != fields.size()) {
      return RC::ERROR;
    }
    auto &&view2     = const_cast<CreateViewSqlNode &>(create_view);
    view2.attr_names = std::move(names);
  }

  stmt             = new CreateViewStmt(create_view.relation_name, create_view.attr_names, create_view.definition);
  auto &&view_stmt = dynamic_cast<CreateViewStmt *>(stmt);

  view_stmt->set_select_stmt(std::move(select_stmt));
  view_stmt->set_can_be_update(can_be_update);
  view_stmt->set_base_tables(std::move(tables));
  view_stmt->have_agg_or_ma_ = have_agg_or_ma;
  view_stmt->only_one_table_ = only_one_table;
  // if (view_stmt->attr_names().empty()) {
  //   vector<string> names;
  //   for (auto &&one : fields) {
  //     names.push_back(one.field_name());
  //   }
  //   view_stmt->set_attr_names(std::move(names));
  // }
  // view_stmt->set_query_fields(std::move(fields));

  return rc;
}

void CreateViewStmt::set_query_fields(vector<Field> &&exprs) { query_fields_ = std::move(exprs); }
