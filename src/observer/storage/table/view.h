#pragma once

#include "sql/parser/parse.h"
#include "storage/field/field.h"
#include "storage/table/table.h"
#include <optional>

class Stmt;
class SelectStmt;
/*
对于view来说，我们创建它，除了构造函数以外，还需要搞出来他的basic_tables
*/
class View : public Table
{

public:
  View(string view_name, vector<string> attr_names, bool can_update, int id, string def)
      : view_name_(std::move(view_name)),
        attr_names_(std::move(attr_names)),
        can_update_(can_update),
        view_id_(id),
        def_(def)
  {}
  ~View();
  const string         &view_name() const { return view_name_; }
  const vector<Table *> base_tables() const { return base_tables_; }
  vector<Table *>       base_tables() { return base_tables_; }
  void                  add_base_table(Table *table) { base_tables_.push_back(table); }
  void                  set_base_tables(vector<Table *> tables) { base_tables_ = std::move(tables); }
  bool                  can_update() const { return can_update_; }
  int                   view_id() const { return view_id_; }
  const vector<string> &attr_names() const { return attr_names_; }
  std::unordered_map<string, std::pair<string, string>> &attr_from_table_and_field()
  {
    return attr_from_table_and_field_;
  };

  void get_sql(unique_ptr<ParsedSqlNode> &node)
  {
    ParsedSqlResult parsed_sql_result;
    parse(def_.c_str(), &parsed_sql_result);
    node = std::move(parsed_sql_result.sql_nodes().back());
    parsed_sql_result.sql_nodes().pop_back();
  }
  void  set_stmt(Stmt *one) { stmt_ = one; }
  Stmt *stmt() { return stmt_; }

  void                                    set_stmt_by_self();
  static vector<std::pair<Field, string>> get_fields(Db *db, SelectStmt *stmt_);

  void set_table_meta(Db *db);
  void set_table_field_name(Db *db);

  std::optional<string> check_is_update(vector<string> attr_names);
  void                  set_default_base_table(Table *table) { default_base_table_ = table; }
  Table                *get_default_base_table() const
  {
    if (default_base_table_ == nullptr && !base_tables_.empty()) {
      return base_tables_.front();
    }
    return default_base_table_;
  }

private:
  string          view_name_;
  vector<Table *> base_tables_;
  Table          *default_base_table_{nullptr};
  vector<string>  attr_names_;
  bool            can_update_;  //这里的判断并不完全，需要再加上一些特殊判断
  int             view_id_;
  string          def_;
  Stmt           *stmt_;
  std::unordered_map<string, std::pair<string, string>> attr_from_table_and_field_;
};