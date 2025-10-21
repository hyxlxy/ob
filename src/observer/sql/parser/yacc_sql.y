
%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/log/log.h"
#include "common/lang/string.h"
#include "sql/parser/parse_defs.h"
#include "sql/parser/yacc_sql.hpp"
#include "sql/parser/lex_sql.h"
#include "sql/expr/expression.h"
#include "common/type/vector_type.h"
#include "common/type/data_type.h"

using namespace std;

string token_name(const char *sql_string, YYLTYPE *llocp)
{
  return string(sql_string + llocp->first_column, llocp->last_column - llocp->first_column + 1);
}

int yyerror(YYLTYPE *llocp, const char *sql_string, ParsedSqlResult *sql_result, yyscan_t scanner, const char *msg)
{
  unique_ptr<ParsedSqlNode> error_sql_node = make_unique<ParsedSqlNode>(SCF_ERROR);
  error_sql_node->error.error_msg = msg;
  error_sql_node->error.line = llocp->first_line;
  error_sql_node->error.column = llocp->first_column;
  sql_result->add_sql_node(std::move(error_sql_node));
  return 0;
}

ArithmeticExpr *create_arithmetic_expression(ArithmeticExpr::Type type,
                                             Expression *left,
                                             Expression *right,
                                             const char *sql_string,
                                             YYLTYPE *llocp)
{
  ArithmeticExpr *expr = new ArithmeticExpr(type, left, right);
  expr->set_name(token_name(sql_string, llocp));
  return expr;
}

UnboundAggregateExpr *create_aggregate_expression(const char *aggregate_name,
                                           Expression *child,
                                           const char *sql_string,
                                           YYLTYPE *llocp)
{
  UnboundAggregateExpr *expr = new UnboundAggregateExpr(aggregate_name, child);
  expr->set_name(token_name(sql_string, llocp));
  return expr;
}

%}

%define api.pure full
%define parse.error verbose
/** 启用位置标识 **/
%locations
%lex-param { yyscan_t scanner }
/** 这些定义了在yyparse函数中的参数 **/
%parse-param { const char * sql_string }
%parse-param { ParsedSqlResult * sql_result }
%parse-param { void * scanner }

//标识tokens
%token  SEMICOLON
        BY
        LIMIT
        WITH
        CREATE
        DROP
        GROUP
        HAVING
        ORDER
        ASC
        AS
        COUNT
        SUM
        MAX
        MIN
        AVG
        TABLE
        TABLES
        INDEX
        /* allow 'CREATE VECTOR INDEX' */
        /* reuse VECTOR_T token for keyword VECTOR before INDEX */
        IF
        EXISTS
        CALC
        SELECT
        UNION
        UNION_ALL
        DESC
        SHOW
        SYNC
        INSERT
        DELETE
        UPDATE
        LBRACE
        RBRACE
        LSQUARE
        RSQUARE
        COMMA
        TRX_BEGIN
        TRX_COMMIT
        TRX_ROLLBACK
        INT_T
        STRING_T
        FLOAT_T
        DATE_T
        VECTOR_T
        TEXT_T
        HELP
        EXIT
        DOT //QUOTE
        INTO
        VALUES
        FROM
        WHERE
        AND
        OR
        JOIN
        INNER
        ON
        NOT
        NULL_T
        LIKE
        SET
        LOAD
        DATA
        INFILE
        EXPLAIN
        STORAGE
        FORMAT
        PRIMARY
        KEY
        ANALYZE
        IN
        IS

        UNIQUE
        VIEW
        EQ
        LT
        GT
        LE
        GE
        NE

/** union 中定义各种数据类型，真实生成的代码也是union类型，所以不能有非POD类型的数据 **/
%union {
  ParsedSqlNode *                            sql_node;
  ConditionSqlNode *                         condition;
  Value *                                    value;
  enum CompOp                                comp;
  RelAttrSqlNode *                           rel_attr;
  vector<AttrInfoSqlNode> *                  attr_infos;
  AttrInfoSqlNode *                          attr_info;
  Expression *                               expression;
  vector<unique_ptr<Expression>> *           expression_list;
  vector<Value> *                            value_list;
  vector<ConditionSqlNode> *                 condition_list;
  vector<RelAttrSqlNode> *                   rel_attr_list;
  vector<string> *                           relation_list;
  vector<string> *                           key_list;
  char *                                     cstring;
  int                                        number;
  double                                     floats;
  vector<double> *                           float_list;
  vector<OrderBySqlNode> *                   order_by_list;
  OrderBySqlNode *                           order_by_item;
  vector<JoinSqlNode> *                      join_list;
  vector<RelationWithAliasSqlNode> *         relation_with_alias_list;
  RelationWithAliasSqlNode *                 relation_with_alias;
  JoinSqlNode *                              join_item;
  vector<pair<string, Value>> *              update_kv_list;
}

%token <number> NUMBER
%token <floats> FLOAT_LITERAL
%token <cstring> ID
%token <cstring> SSS
//非终结符

/** type 定义了各种解析后的结果输出的是什么类型。类型对应了 union 中的定义的成员变量名称 **/
%type <number>              type
%type <condition>           condition
%type <value>               value
%type <value>               insert_value
%type <number>              number
%type <floats>              float_number
%type <relation_with_alias> relation
%type <comp>                comp_op
%type <rel_attr>            rel_attr
%type <attr_infos>          attr_def_list
%type <attr_info>           attr_def
%type <update_kv_list>      update_kv_list
%type <value_list>          insert_value_list
%type <condition_list>      where
%type <condition_list>      condition_list
%type <cstring>             storage_format
%type <key_list>            primary_key
%type <key_list>            attr_list
%type <relation_with_alias_list>       rel_list
%type <expression>          expression
%type <expression_list>     projection_list
%type <cstring>             opt_as_alias
%type <expression>          aggregate_expr
%type <expression>          count_expr
%type <expression>          sub_query_expr
%type <expression_list>     expression_list
%type <expression_list>     group_by
%type <expression>          having
%type <order_by_list>       order_by
%type <order_by_list>       order_by_item_list
%type <order_by_item>       order_by_item
%type <join_list>           join_list
%type <join_item>           join_item
%type <value>               vector_literal
%type <float_list>          number_list
%type <sql_node>            calc_stmt
%type <sql_node>            select_stmt
%type <sql_node>            union_stmt
%type <number>              limit_clause
%type <sql_node>            insert_stmt
%type <sql_node>            update_stmt
%type <sql_node>            delete_stmt
%type <sql_node>            create_table_stmt
%type <sql_node>            drop_table_stmt
%type <sql_node>            analyze_table_stmt
%type <sql_node>            show_tables_stmt
%type <sql_node>            show_index_stmt
%type <sql_node>            desc_table_stmt
%type <sql_node>            create_index_stmt
/* remove duplicate %type limit_clause declaration */
%type <number>              option_list
%type <number>              option_item
%type <sql_node>            drop_index_stmt
%type <sql_node>            sync_stmt
%type <sql_node>            begin_stmt
%type <sql_node>            commit_stmt
%type <sql_node>            rollback_stmt
%type <sql_node>            load_data_stmt
%type <sql_node>            explain_stmt
%type <sql_node>            set_variable_stmt
%type <sql_node>            create_view_stmt
%type <sql_node>            drop_view_stmt
%type <sql_node>            help_stmt
%type <sql_node>            exit_stmt
%type <sql_node>            command_wrapper
// commands should be a list but I use a single command instead
%type <sql_node>            commands
%type <sql_node>            sub_query

%left EQ LT GT LE GE NE LIKE IN
%left '+' '-'
%left '*' '/'
%left NOT
%nonassoc UMINUS
%%

commands: command_wrapper opt_semicolon  //commands or sqls. parser starts here.
  {
    unique_ptr<ParsedSqlNode> sql_node = unique_ptr<ParsedSqlNode>($1);
    sql_result->add_sql_node(std::move(sql_node));
  }
  ;

command_wrapper:
    calc_stmt
  | select_stmt
  | union_stmt
  | insert_stmt
  | update_stmt
  | delete_stmt
  | create_table_stmt
  | drop_table_stmt
  | analyze_table_stmt
  | show_tables_stmt
  | show_index_stmt
  | desc_table_stmt
  | create_index_stmt
  | drop_index_stmt
  | sync_stmt
  | begin_stmt
  | commit_stmt
  | rollback_stmt
  | load_data_stmt
  | explain_stmt
  | set_variable_stmt
  | create_view_stmt
  | drop_view_stmt
  | help_stmt
  | exit_stmt
    ;

exit_stmt:
    EXIT {
      (void)yynerrs;  // 这么写为了消除yynerrs未使用的告警。如果你有更好的方法欢迎提PR
      $$ = new ParsedSqlNode(SCF_EXIT);
    };

help_stmt:
    HELP {
      $$ = new ParsedSqlNode(SCF_HELP);
    };

sync_stmt:
    SYNC {
      $$ = new ParsedSqlNode(SCF_SYNC);
    }
    ;

begin_stmt:
    TRX_BEGIN  {
      $$ = new ParsedSqlNode(SCF_BEGIN);
    }
    ;

commit_stmt:
    TRX_COMMIT {
      $$ = new ParsedSqlNode(SCF_COMMIT);
    }
    ;

rollback_stmt:
    TRX_ROLLBACK  {
      $$ = new ParsedSqlNode(SCF_ROLLBACK);
    }
    ;

drop_table_stmt:    /*drop table 语句的语法解析树*/
    DROP TABLE ID {
      $$ = new ParsedSqlNode(SCF_DROP_TABLE);
      $$->drop_table.relation_name = $3;
      $$->drop_table.if_exists = false;
    }
    | DROP TABLE IF EXISTS ID {
      $$ = new ParsedSqlNode(SCF_DROP_TABLE);
      $$->drop_table.relation_name = $5;
      $$->drop_table.if_exists = true;
    };

analyze_table_stmt:  /* analyze table 语法的语法解析树*/
    ANALYZE TABLE ID {
      $$ = new ParsedSqlNode(SCF_ANALYZE_TABLE);
      $$->analyze_table.relation_name = $3;
    }
    ;

show_tables_stmt:
    SHOW TABLES {
      $$ = new ParsedSqlNode(SCF_SHOW_TABLES);
    }
    ;

show_index_stmt:
    SHOW INDEX FROM ID
    {
      $$ = new ParsedSqlNode(SCF_SHOW_INDEX);
      $$->show_index.relation_name = $4;
    }
    ;

desc_table_stmt:
    DESC ID  {
      $$ = new ParsedSqlNode(SCF_DESC_TABLE);
      $$->desc_table.relation_name = $2;
    }
    ;

create_index_stmt:    /*create index 语句的语法解析树*/
    CREATE INDEX ID ON ID LBRACE attr_list RBRACE
    {
      $$ = new ParsedSqlNode(SCF_CREATE_INDEX);
      CreateIndexSqlNode &create_index = $$->create_index;
      create_index.index_name = $3;
      create_index.relation_name = $5;
      if ($7 != nullptr) {
        create_index.attribute_names.swap(*$7);
        delete $7;
      }
      create_index.is_unique = false;
    }
    | CREATE UNIQUE INDEX ID ON ID LBRACE attr_list RBRACE
    {
      $$ = new ParsedSqlNode(SCF_CREATE_INDEX);
      CreateIndexSqlNode &create_index = $$->create_index;
      create_index.index_name = $4;
      create_index.relation_name = $6;
      if ($8 != nullptr) {
        create_index.attribute_names.swap(*$8);
        delete $8;
      }
      create_index.is_unique = true;
    }
    | CREATE VECTOR_T INDEX ID ON ID LBRACE attr_list RBRACE /* ignore optional WITH clause below */
    {
      $$ = new ParsedSqlNode(SCF_CREATE_INDEX);
      CreateIndexSqlNode &create_index = $$->create_index;
      create_index.index_name = $4;
      create_index.relation_name = $6;
      if ($8 != nullptr) {
        create_index.attribute_names.swap(*$8);
        delete $8;
      }
      create_index.is_unique = false;
    }
    | CREATE VECTOR_T INDEX ID ON ID LBRACE attr_list RBRACE WITH LBRACE /* swallow options */ option_list RBRACE
    {
      $$ = new ParsedSqlNode(SCF_CREATE_INDEX);
      CreateIndexSqlNode &create_index = $$->create_index;
      create_index.index_name = $4;
      create_index.relation_name = $6;
      if ($8 != nullptr) {
        create_index.attribute_names.swap(*$8);
        delete $8;
      }
      create_index.is_unique = false;
    }
    ;

drop_index_stmt:      /*drop index 语句的语法解析树*/
    DROP INDEX ID ON ID
    {
      $$ = new ParsedSqlNode(SCF_DROP_INDEX);
      $$->drop_index.index_name = $3;
      $$->drop_index.relation_name = $5;
    }
    ;
create_table_stmt:    /*create table 语句的语法解析树*/
    CREATE TABLE ID LBRACE attr_def attr_def_list primary_key RBRACE storage_format
    {
      $$ = new ParsedSqlNode(SCF_CREATE_TABLE);
      CreateTableSqlNode &create_table = $$->create_table;
      create_table.relation_name = $3;
      //free($3);

      vector<AttrInfoSqlNode> *src_attrs = $6;

      if (src_attrs != nullptr) {
        create_table.attr_infos.swap(*src_attrs);
        delete src_attrs;
      }
      create_table.attr_infos.emplace_back(*$5);
      reverse(create_table.attr_infos.begin(), create_table.attr_infos.end());
      delete $5;
      if ($7 != nullptr) {
        create_table.primary_keys.swap(*$7);
        delete $7;
      }
      if ($9 != nullptr) {
        create_table.storage_format = $9;
      }
    }
    | CREATE TABLE ID AS select_stmt
    {
      $$ = new ParsedSqlNode(SCF_CREATE_TABLE);
      CreateTableSqlNode &create_table = $$->create_table;
      create_table.relation_name = $3;
      create_table.as_select = std::make_unique<SelectSqlNode>(std::move($5->selection));
      delete $5;
    }
    | CREATE TABLE ID LBRACE attr_def attr_def_list primary_key RBRACE select_stmt
    {
      $$ = new ParsedSqlNode(SCF_CREATE_TABLE);
      CreateTableSqlNode &create_table = $$->create_table;
      create_table.relation_name = $3;
      
      vector<AttrInfoSqlNode> *src_attrs = $6;
      if (src_attrs != nullptr) {
        create_table.attr_infos.swap(*src_attrs);
        delete src_attrs;
      }
      create_table.attr_infos.emplace_back(*$5);
      reverse(create_table.attr_infos.begin(), create_table.attr_infos.end());
      delete $5;
      
      if ($7 != nullptr) {
        create_table.primary_keys.swap(*$7);
        delete $7;
      }
      
      // Store the SELECT statement for data insertion
      create_table.as_select = std::make_unique<SelectSqlNode>(std::move($9->selection));
      delete $9;
    }
    ;

create_view_stmt:
    CREATE VIEW ID AS select_stmt
    {
      $$ = new ParsedSqlNode(SCF_CREATE_VIEW);
      CreateViewSqlNode &create_view = $$->create_view;
      create_view.relation_name = $3;
      create_view.create_view_select = std::make_unique<SelectSqlNode>(std::move($5->selection));
      delete $5;
    }
    | CREATE VIEW ID LBRACE attr_list RBRACE AS select_stmt
    {
      $$ = new ParsedSqlNode(SCF_CREATE_VIEW);
      CreateViewSqlNode &create_view = $$->create_view;
      create_view.relation_name = $3;
      create_view.attribute_names = std::move(*$5);
      create_view.create_view_select = std::make_unique<SelectSqlNode>(std::move($8->selection));
      delete $5;
      delete $8;
    }
    ;

drop_view_stmt:
    DROP VIEW ID
    {
      $$ = new ParsedSqlNode(SCF_DROP_VIEW);
      $$->drop_view.relation_name = $3;
    }
    ;

attr_def_list:
    /* empty */
    {
      $$ = nullptr;
    }
    | attr_def_list COMMA attr_def
    {
      if ($1 != nullptr) {
        $$ = $1;
      } else {
        $$ = new vector<AttrInfoSqlNode>;
      }
      $$->insert($$->begin(), *$3);
      delete $3;
    }
    ;

attr_def:
    ID type LBRACE number RBRACE
    {
      $$ = new AttrInfoSqlNode;
      $$->type = (AttrType)$2;
      $$->name = $1;
      $$->nullable = false;  // default NOT NULL
      // For VECTOR(n), store bytes for [dimension(int32) + n float32 values]
      if ($$->type == AttrType::VECTORS) {
        $$->length = static_cast<int>(sizeof(int) + ($4 * sizeof(float)));
      } else {
        $$->length = $4;
      }
    }
    | ID type LBRACE number RBRACE NULL_T
    {
      $$ = new AttrInfoSqlNode;
      $$->type = (AttrType)$2;
      $$->name = $1;
      $$->nullable = true;  // explicitly nullable
      // For VECTOR(n), store bytes for [dimension(int32) + n float32 values]
      if ($$->type == AttrType::VECTORS) {
        $$->length = static_cast<int>(sizeof(int) + ($4 * sizeof(float)));
      } else {
        $$->length = $4;
      }
      // Add extra byte for NULL flag for nullable fields
      if ($$->nullable) {
        $$->length++;
      }
    }
    | ID type LBRACE number RBRACE NOT NULL_T
    {
      $$ = new AttrInfoSqlNode;
      $$->type = (AttrType)$2;
      $$->name = $1;
      $$->nullable = false;  // explicitly not nullable
      // For VECTOR(n), store bytes for [dimension(int32) + n float32 values]
      if ($$->type == AttrType::VECTORS) {
        $$->length = static_cast<int>(sizeof(int) + ($4 * sizeof(float)));
      } else {
        $$->length = $4;
      }
    }
    | ID type
    {
      $$ = new AttrInfoSqlNode;
      $$->type = (AttrType)$2;
      $$->name = $1;
      $$->nullable = false;  // default NOT NULL
      // Default length: INT/FLOAT/DATE/STRING fallback 4; VECTOR default header size (dimension); TEXT default 65535
      if ($$->type == AttrType::VECTORS) {
        $$->length = static_cast<int>(sizeof(int));
      } else if ($$->type == AttrType::TEXTS) {
        $$->length = 255;  // TEXT default length (reduced for testing)
      } else {
        $$->length = 4;
      }
    }
    | ID type NULL_T
    {
      $$ = new AttrInfoSqlNode;
      $$->type = (AttrType)$2;
      $$->name = $1;
      $$->nullable = true;  // explicitly nullable
      // Default length: INT/FLOAT/DATE/STRING fallback 4; VECTOR default header size (dimension); TEXT default 65535
      if ($$->type == AttrType::VECTORS) {
        $$->length = static_cast<int>(sizeof(int));
      } else if ($$->type == AttrType::TEXTS) {
        $$->length = 255;  // TEXT default length (reduced for testing)
      } else {
        $$->length = 4;
      }
      // Add extra byte for NULL flag for nullable fields
      if ($$->nullable) {
        $$->length++;
      }
    }
    | ID type NOT NULL_T
    {
      $$ = new AttrInfoSqlNode;
      $$->type = (AttrType)$2;
      $$->name = $1;
      $$->nullable = false;  // explicitly not nullable
      // Default length: INT/FLOAT/DATE/STRING fallback 4; VECTOR default header size (dimension); TEXT default 65535
      if ($$->type == AttrType::VECTORS) {
        $$->length = static_cast<int>(sizeof(int));
      } else if ($$->type == AttrType::TEXTS) {
        $$->length = 255;  // TEXT default length (reduced for testing)
      } else {
        $$->length = 4;
      }
    }
    ;
number:
    NUMBER {$$ = $1;}
    ;

float_number:
    NUMBER {$$ = (double)$1;}
    | FLOAT_LITERAL {$$ = $1;}
    ;
type:
    INT_T      { $$ = static_cast<int>(AttrType::INTS); }
    | STRING_T { $$ = static_cast<int>(AttrType::CHARS); }
    | FLOAT_T  { $$ = static_cast<int>(AttrType::FLOATS); }
    | DATE_T   { $$ = static_cast<int>(AttrType::DATES); }
    | VECTOR_T { $$ = static_cast<int>(AttrType::VECTORS); }
    | TEXT_T   { $$ = static_cast<int>(AttrType::TEXTS); }
    ;
primary_key:
    /* empty */
    {
      $$ = nullptr;
    }
    | COMMA PRIMARY KEY LBRACE attr_list RBRACE
    {
      $$ = $5;
    }
    ;

attr_list:
    ID {
      $$ = new vector<string>();
      $$->push_back($1);
    }
    | ID COMMA attr_list {
      if ($3 != nullptr) {
        $$ = $3;
      } else {
        $$ = new vector<string>;
      }

      $$->insert($$->begin(), $1);
    }
    ;

insert_stmt:        /*insert   语句的语法解析树*/
    INSERT INTO ID VALUES LBRACE insert_value insert_value_list RBRACE
    {
      $$ = new ParsedSqlNode(SCF_INSERT);
      $$->insertion.relation_name = $3;
      if ($7 != nullptr) {
        $$->insertion.values.swap(*$7);
        delete $7;
      }
      $$->insertion.values.emplace_back(*$6);
      reverse($$->insertion.values.begin(), $$->insertion.values.end());
      delete $6;
    }
    ;

insert_value_list:
    /* empty */
    {
      $$ = nullptr;
    }
    | COMMA insert_value insert_value_list  {
      if ($3 != nullptr) {
        $$ = $3;
      } else {
        $$ = new vector<Value>;
      }
      $$->emplace_back(*$2);
      delete $2;
    }
    ;



insert_value:
    value
    | '-' NUMBER {
      $$ = new Value(-(int)$2);
      @$ = @2;
    }
    | '-' FLOAT_LITERAL {
      $$ = new Value(-(float)$2);
      @$ = @2;
    }
    // 支持 INSERT 中的常量表达式（例如 STRING_TO_VECTOR('...')）
    | expression {
      Value __v; RC __rc = $1->try_get_value(__v);
      if (__rc != RC::SUCCESS) {
        delete $1; $1 = nullptr;
        yyerror(&@$, sql_string, sql_result, scanner, "Invalid constant expression in INSERT");
        YYERROR;
      }
      $$ = new Value(__v);
      delete $1; $1 = nullptr;
      @$ = @1;
    }
    ;

value:
    NUMBER {
      $$ = new Value((int)$1);
      @$ = @1;
    }
    |FLOAT_LITERAL {
      $$ = new Value((float)$1);
      @$ = @1;
    }
    |NULL_T {
      $$ = new Value();
      $$->set_type(AttrType::UNDEFINED);
      $$->set_null(true);
      @$ = @1;
    }
    |vector_literal {
      $$ = $1;
      @$ = @1;
    }
    |SSS {
      char *tmp = common::substr($1,1,strlen($1)-2);

      // 尝试解析为向量类型
      if (tmp[0] == '[') {
        Value *vector_value = new Value();
        RC rc = DataType::type_instance(AttrType::VECTORS)->set_value_from_str(*vector_value, tmp);
        if (rc == RC::SUCCESS) {
          $$ = vector_value;
          free(tmp);
        } else {
          delete vector_value;
          // 如果向量解析失败，作为字符串处理
          $$ = new Value(tmp);
          free(tmp);
        }
      } else {
      // 检查是否为日期格式（YYYY-MM-DD或YYYY-M-D等）
      bool looks_like_date = false;
      int dash_count = 0;
      bool only_digits_and_dash = true;
      for (int i = 0; tmp[i] != '\0'; i++) {
        if (tmp[i] == '-') {
          dash_count++;
        } else if (!(tmp[i] >= '0' && tmp[i] <= '9')) {
          only_digits_and_dash = false;
        }
      }
      // 仅当恰好两个 '-' 且其余字符全为数字时，才尝试按日期解析
      if (dash_count == 2 && only_digits_and_dash) {
        looks_like_date = true;
      }

        if (looks_like_date) {
          // 尝试解析为日期类型
          Value *date_value = new Value();
          RC rc = DataType::type_instance(AttrType::DATES)->set_value_from_str(*date_value, tmp);
          if (rc == RC::SUCCESS) {
            $$ = date_value;
            free(tmp);
          } else {
            delete date_value;
            // 如果看起来像日期但解析失败，报错而不是回退到字符串
            LOG_WARN("Invalid date format: %s", tmp);
            free(tmp);
            YYERROR;
          }
        } else {
          // 不像日期格式，作为字符串处理
          $$ = new Value(tmp);
          free(tmp);
        }
      }
    }
    ;
storage_format:
    /* empty */
    {
      $$ = nullptr;
    }
    | STORAGE FORMAT EQ ID
    {
      $$ = $4;
    }
    ;

delete_stmt:    /*  delete 语句的语法解析树*/
    DELETE FROM ID where
    {
      $$ = new ParsedSqlNode(SCF_DELETE);
      $$->deletion.relation_name = $3;
      if ($4 != nullptr) {
        $$->deletion.conditions.swap(*$4);
        delete $4;
      }
    }
    ;
update_stmt:      /*  update 语句的语法解析树*/
    UPDATE ID SET update_kv_list where
    {
      $$ = new ParsedSqlNode(SCF_UPDATE);
      $$->update.relation_name = $2;
      
      if ($4 && !$4->empty()) {
        for (auto &kv : *$4) {
          $$->update.attribute_names.push_back(kv.first);
          $$->update.values.push_back(kv.second);
        }
        delete $4;
      }
      
      if ($5 != nullptr) {
        $$->update.conditions.swap(*$5);
        delete $5;
      }
    }
    | UPDATE ID SET ID EQ LBRACE sub_query RBRACE where
    {
      // 支持单字段：SET col = (subquery)
      $$ = new ParsedSqlNode(SCF_UPDATE);
      $$->update.relation_name = $2;
      $$->update.attribute_name = $4;
      $$->update.is_sub_query = true;
      $$->update.sub_query.reset($7);
      if ($9 != nullptr) {
        $$->update.conditions.swap(*$9);
        delete $9;
      }
    }
    ;

update_kv_list:
    ID EQ value
    {
      $$ = new vector<pair<string, Value>>();
      $$->push_back({$1, *$3});
      delete $3;
    }
    | update_kv_list COMMA ID EQ value
    {
      $1->push_back({$3, *$5});
      $$ = $1;
      delete $5;
    }
    | ID EQ LBRACE sub_query RBRACE
    {
      // 单字段子查询UPDATE - 回退到旧语法处理
      // 这里暂时不支持，让解析器尝试其他规则
      YYERROR;
    }
    ;
select_stmt:        /*  select 语句的语法解析树*/
    SELECT projection_list FROM rel_list join_list where group_by having order_by limit_clause
    {
      $$ = new ParsedSqlNode(SCF_SELECT);
      if ($2 != nullptr) {
        $$->selection.expressions.swap(*$2);
        delete $2;
      }

      if ($4 != nullptr) {
        for (auto &it : *$4) {
          $$->selection.relations.push_back(it.table_name);
          $$->selection.relation_aliases.push_back(it.alias);
        }
        delete $4;
      }

      if ($5 != nullptr) {
        $$->selection.joins.swap(*$5);
        delete $5;
      }

      if ($6 != nullptr) {
        $$->selection.conditions.swap(*$6);
        delete $6;
      }

      if ($7 != nullptr) {
        $$->selection.group_by.swap(*$7);
        delete $7;
      }

      if ($8 != nullptr) {
        $$->selection.having_expr.reset($8);
      }

      if ($9 != nullptr) {
        $$->selection.order_by.swap(*$9);
        delete $9;
      }
      // store optional LIMIT for later plan generation
      $$->selection.limit_count = $10;
    }
    | SELECT projection_list WHERE condition_list order_by limit_clause
    {
      $$ = new ParsedSqlNode(SCF_SELECT);
      if ($2 != nullptr) {
        $$->selection.expressions.swap(*$2);
        delete $2;
      }

      if ($4 != nullptr) {
        $$->selection.conditions.swap(*$4);
        delete $4;
      }

      if ($5 != nullptr) {
        $$->selection.order_by.swap(*$5);
        delete $5;
      }
      $$->selection.limit_count = $6;
    }
    | SELECT projection_list order_by limit_clause
    {
      $$ = new ParsedSqlNode(SCF_SELECT);
      if ($2 != nullptr) {
        $$->selection.expressions.swap(*$2);
        delete $2;
      }

      if ($3 != nullptr) {
        $$->selection.order_by.swap(*$3);
        delete $3;
      }
      $$->selection.limit_count = $4;
    }
    ;

union_stmt:        /* UNION语句的语法解析树 */
    select_stmt UNION select_stmt
    {
      $$ = new ParsedSqlNode(SCF_UNION);
      $$->union_stmt.select_statements.emplace_back(std::make_unique<SelectSqlNode>(std::move($1->selection)));
      $$->union_stmt.select_statements.emplace_back(std::make_unique<SelectSqlNode>(std::move($3->selection)));
      $$->union_stmt.distinct_flags.push_back(true);  // 第一个UNION去重
      delete $1;
      delete $3;
    }
  | select_stmt UNION_ALL select_stmt
    {
      $$ = new ParsedSqlNode(SCF_UNION);
      $$->union_stmt.select_statements.emplace_back(std::make_unique<SelectSqlNode>(std::move($1->selection)));
      $$->union_stmt.select_statements.emplace_back(std::make_unique<SelectSqlNode>(std::move($3->selection)));
      $$->union_stmt.distinct_flags.push_back(false);  // 第一个UNION ALL不去重
      delete $1;
      delete $3;
    }
  | union_stmt UNION select_stmt
    {
      $1->union_stmt.select_statements.emplace_back(std::make_unique<SelectSqlNode>(std::move($3->selection)));
      $1->union_stmt.distinct_flags.push_back(true);  // 添加新的UNION去重标志
      $$ = $1;
      delete $3;
    }
  | union_stmt UNION_ALL select_stmt
    {
      $1->union_stmt.select_statements.emplace_back(std::make_unique<SelectSqlNode>(std::move($3->selection)));
      $1->union_stmt.distinct_flags.push_back(false);  // 添加新的UNION ALL不去重标志
      $$ = $1;
      delete $3;
    }
  | LBRACE union_stmt RBRACE order_by limit_clause
    {
      $$ = $2;
      if ($4 != nullptr) {
        $$->union_stmt.order_by.swap(*$4);
        delete $4;
      }
      $$->union_stmt.limit_count = $5;
    }
    ;

calc_stmt:
    CALC expression_list
    {
      $$ = new ParsedSqlNode(SCF_CALC);
      $$->calc.expressions.swap(*$2);
      delete $2;
    }
    ;

expression_list:
    expression
    {
      $$ = new vector<unique_ptr<Expression>>;
      $$->emplace_back($1);
    }
    | expression COMMA expression_list
    {
      if ($3 != nullptr) {
        $$ = $3;
      } else {
        $$ = new vector<unique_ptr<Expression>>;
      }
      $$->emplace($$->begin(), $1);
    }
    ;

projection_list:
    expression opt_as_alias
    {
      if ($2 != nullptr) {
        $1->set_name($2);
      }
      $$ = new vector<unique_ptr<Expression>>;
      $$->emplace_back($1);
    }
    | expression opt_as_alias COMMA projection_list
    {
      if ($2 != nullptr) {
        $1->set_name($2);
      }
      if ($4 != nullptr) {
        $$ = $4;
      } else {
        $$ = new vector<unique_ptr<Expression>>;
      }
      $$->emplace($$->begin(), $1);
    }
    ;

opt_as_alias:
    /* empty */ { $$ = nullptr; }
    | ID { $$ = $1; }
    | AS ID { $$ = $2; }
    ;

expression:
    expression '+' expression {
      $$ = create_arithmetic_expression(ArithmeticExpr::Type::ADD, $1, $3, sql_string, &@$);
    }
    | expression '-' expression {
      $$ = create_arithmetic_expression(ArithmeticExpr::Type::SUB, $1, $3, sql_string, &@$);
    }
    | expression '*' expression {
      $$ = create_arithmetic_expression(ArithmeticExpr::Type::MUL, $1, $3, sql_string, &@$);
    }
    | expression '/' expression {
      $$ = create_arithmetic_expression(ArithmeticExpr::Type::DIV, $1, $3, sql_string, &@$);
    }
    | expression comp_op expression {
      $$ = new ComparisonExpr($2, unique_ptr<Expression>($1), unique_ptr<Expression>($3));
      $$->set_name(token_name(sql_string, &@$));
    }
    | LBRACE expression RBRACE {
      // Simple parentheses for grouping - just return the inner expression
      $$ = $2;
    }
    | '-' expression %prec UMINUS {
      $$ = create_arithmetic_expression(ArithmeticExpr::Type::NEGATIVE, $2, nullptr, sql_string, &@$);
    }
    | value {
      $$ = new ValueExpr(*$1);
      $$->set_name(token_name(sql_string, &@$));
      delete $1;
    }
    // function call: e.g., L2_DISTANCE(expr, expr)
    | ID LBRACE RBRACE {
      std::vector<std::unique_ptr<Expression>> args;
      $$ = new FunctionExpr($1, std::move(args));
      $$->set_name(token_name(sql_string, &@$));
    }
    | ID LBRACE expression_list RBRACE {
      std::vector<std::unique_ptr<Expression>> args;
      if ($3 != nullptr) {
        args.swap(*$3);
        delete $3;
      }
      // 解析期维度不一致快速失败：仅对向量距离类函数进行
      // 函数名：DISTANCE/L2_DISTANCE/COSINE_DISTANCE/INNER_PRODUCT
      auto __is_distance = [&](const char *fn)->bool {
        return 0==strcasecmp(fn, "DISTANCE") || 0==strcasecmp(fn, "L2_DISTANCE") || 0==strcasecmp(fn, "COSINE_DISTANCE") || 0==strcasecmp(fn, "INNER_PRODUCT");
      };
      if (__is_distance($1) && args.size() >= 2 && args[0] && args[1]) {
        Value __v0, __v1;
        RC __rc0 = args[0]->try_get_value(__v0);
        RC __rc1 = args[1]->try_get_value(__v1);
        if (__rc0 == RC::SUCCESS && __rc1 == RC::SUCCESS &&
            __v0.attr_type() == AttrType::VECTORS && __v1.attr_type() == AttrType::VECTORS &&
            __v0.length() >= (int)sizeof(int) && __v1.length() >= (int)sizeof(int)) {
          int __d0 = *(const int *)__v0.data();
          int __d1 = *(const int *)__v1.data();
          if (__d0 != __d1) {
            yyerror(&@$, sql_string, sql_result, scanner, "Vector dimensions mismatch in distance function");
            YYERROR;
          }
        }
      }
      $$ = new FunctionExpr($1, std::move(args));
      $$->set_name(token_name(sql_string, &@$));
    }
    | rel_attr {
      RelAttrSqlNode *node = $1;
      $$ = new UnboundFieldExpr(node->relation_name, node->attribute_name);
      $$->set_name(token_name(sql_string, &@$));
      delete $1;
    }
    | '*' {
      $$ = new StarExpr();
    }
    | aggregate_expr {
      $$ = $1;
    }
    | sub_query_expr {
      $$ = $1;
    }
    ;

sub_query_expr:
    LBRACE SELECT projection_list FROM rel_list join_list where group_by having order_by RBRACE
    {
      fprintf(stderr, "DEBUG: [PARSER] Parsing scalar subquery with FROM\n");
      ParsedSqlNode *parsed_node = new ParsedSqlNode(SCF_SELECT);
      if ($3 != nullptr) {
        parsed_node->selection.expressions.swap(*$3);
        delete $3;
      }
      if ($5 != nullptr) {
        for (auto &it : *$5) {
          parsed_node->selection.relations.push_back(it.table_name);
          parsed_node->selection.relation_aliases.push_back(it.alias);
        }
        delete $5;
      }

      if ($6 != nullptr) {
        parsed_node->selection.joins.swap(*$6);
        delete $6;
      }

      if ($7 != nullptr) {
        parsed_node->selection.conditions.swap(*$7);
        delete $7;
      }

      if ($8 != nullptr) {
        parsed_node->selection.group_by.swap(*$8);
        delete $8;
      }

      if ($9 != nullptr) {
        parsed_node->selection.having_expr.reset($9);
      }

      if ($10 != nullptr) {
        parsed_node->selection.order_by.swap(*$10);
        delete $10;
      }

      fprintf(stderr, "DEBUG: [PARSER] Creating SubQueryExpr\n");
      std::unique_ptr<SelectSqlNode> select_node(new SelectSqlNode(std::move(parsed_node->selection)));
      $$ = new SubQueryExpr(std::move(select_node));
      $$->set_name(token_name(sql_string, &@$));
      delete parsed_node;
      fprintf(stderr, "DEBUG: [PARSER] SubQueryExpr created successfully\n");
    }
    | LBRACE SELECT projection_list order_by RBRACE
    {
      fprintf(stderr, "DEBUG: [PARSER] Parsing scalar subquery without FROM\n");
      ParsedSqlNode *parsed_node = new ParsedSqlNode(SCF_SELECT);
      if ($3 != nullptr) {
        parsed_node->selection.expressions.swap(*$3);
        delete $3;
      }
      if ($4 != nullptr) {
        parsed_node->selection.order_by.swap(*$4);
        delete $4;
      }

      fprintf(stderr, "DEBUG: [PARSER] Creating SubQueryExpr\n");
      std::unique_ptr<SelectSqlNode> select_node(new SelectSqlNode(std::move(parsed_node->selection)));
      $$ = new SubQueryExpr(std::move(select_node));
      $$->set_name(token_name(sql_string, &@$));
      delete parsed_node;
      fprintf(stderr, "DEBUG: [PARSER] SubQueryExpr created successfully\n");
    }
    ;

rel_attr:
    ID {
      $$ = new RelAttrSqlNode;
      $$->attribute_name = $1;
    }
    | ID DOT ID {
      $$ = new RelAttrSqlNode;
      $$->relation_name  = $1;
      $$->attribute_name = $3;
    }
    | ID DOT '*' {
      $$ = new RelAttrSqlNode;
      $$->relation_name  = $1;
      $$->attribute_name = (char*)"*";
    }
    ;

relation:
    ID {
      $$ = new RelationWithAliasSqlNode();
      $$->table_name = $1;
    }
    | ID ID {
      $$ = new RelationWithAliasSqlNode();
      $$->table_name = $1;
      $$->alias = $2;
    }
    | ID AS ID {
      $$ = new RelationWithAliasSqlNode();
      $$->table_name = $1;
      $$->alias = $3;
    }
    ;
rel_list:
    relation {
      $$ = new vector<RelationWithAliasSqlNode>();
      $$->emplace_back(std::move(*$1));
      delete $1;
    }
    | relation COMMA rel_list {
      if ($3 != nullptr) {
        $$ = $3;
      } else {
        $$ = new vector<RelationWithAliasSqlNode>();
      }
      $$->emplace($$->begin(), std::move(*$1));
      delete $1;
    }
    ;



where:
    /* empty */
    {
      $$ = nullptr;
    }
    | WHERE condition_list {
      $$ = $2;
      printf("DEBUG: [YACC] WHERE condition_list: %zu conditions\n", $2 ? $2->size() : 0);
      if ($2) {
        for (size_t i = 0; i < $2->size(); i++) {
          printf("DEBUG: [YACC] Condition %zu: conjunction_op = %d\n", i, (*$2)[i].conjunction_op);
        }
      }
    }
    ;
condition_list:
    /* empty */
    {
      $$ = nullptr;
    }
    | condition {
      $$ = new vector<ConditionSqlNode>;
      $$->emplace_back(std::move(*$1));
      delete $1;
    }
    | condition AND condition_list {
      $$ = $3;
      $1->conjunction_op = AND_OP;  // 设置AND连接符
      printf("DEBUG: [YACC] Setting AND_OP for condition\n");
      $$->emplace_back(std::move(*$1));
      delete $1;
    }
    | condition OR condition_list {
      $$ = $3;
      $1->conjunction_op = OR_OP;    // 设置OR连接符
      printf("DEBUG: [YACC] Setting OR_OP for condition\n");
      $$->emplace_back(std::move(*$1));
      delete $1;
    }
    ;
condition:
    expression comp_op expression
    {
      $$ = new ConditionSqlNode;
      $$->left_expr.reset($1);
      $$->right_expr.reset($3);
      $$->comp = $2;
      $$->conjunction_op = AND_OP;  // 默认初始化为AND_OP
    }
    | expression IN LBRACE expression_list RBRACE
    {
      $$ = new ConditionSqlNode;
      $$->left_expr.reset($1);
      // For IN values operation, store values directly in right_values
      if ($4 != nullptr) {
        // Convert expressions to values
        for (auto &expr : *$4) {
          Value val;
          RC rc = expr->try_get_value(val);
          if (rc == RC::SUCCESS) {
            $$->right_values.push_back(val);
          } else {
            // If we can't get constant values, fall back to ListExpr
            $$->right_expr.reset(new ListExpr(std::move(*$4)));
            break;
          }
        }
        delete $4;
      }
      $$->comp = IN_VALUES_OP;
    }
    | expression NOT IN LBRACE expression_list RBRACE
    {
      $$ = new ConditionSqlNode;
      $$->left_expr.reset($1);
      // For NOT IN values operation, store values directly in right_values
      if ($5 != nullptr) {
        // Convert expressions to values
        for (auto &expr : *$5) {
          Value val;
          RC rc = expr->try_get_value(val);
          if (rc == RC::SUCCESS) {
            $$->right_values.push_back(val);
          } else {
            // If we can't get constant values, fall back to ListExpr
            $$->right_expr.reset(new ListExpr(std::move(*$5)));
            break;
          }
        }
        delete $5;
      }
      $$->comp = NOT_IN_VALUES_OP;
    }
    | expression IN LBRACE sub_query RBRACE
    {
      $$ = new ConditionSqlNode;
      $$->left_expr.reset($1);
      $$->sub_query.reset($4);
      $$->comp = IN_OP;
    }
    | expression NOT IN LBRACE sub_query RBRACE
    {
      $$ = new ConditionSqlNode;
      $$->left_expr.reset($1);
      $$->sub_query.reset($5);
      $$->comp = NOT_IN_OP;
    }
    | expression IS NULL_T
    {
      $$ = new ConditionSqlNode;
      $$->left_expr.reset($1);
      Value __nullv; __nullv.set_type(AttrType::UNDEFINED); __nullv.set_null(true);
      $$->right_expr.reset(new ValueExpr(__nullv));
      $$->comp = IS_NULL_OP;
    }
    | expression IS NOT NULL_T
    {
      $$ = new ConditionSqlNode;
      $$->left_expr.reset($1);
      Value __nullv2; __nullv2.set_type(AttrType::UNDEFINED); __nullv2.set_null(true);
      $$->right_expr.reset(new ValueExpr(__nullv2));
      $$->comp = IS_NOT_NULL_OP;
    }
    | EXISTS LBRACE sub_query RBRACE
    {
      $$ = new ConditionSqlNode;
      $$->sub_query.reset($3);
      $$->comp = EXISTS_OP;
    }
    | NOT EXISTS LBRACE sub_query RBRACE
    {
      $$ = new ConditionSqlNode;
      $$->sub_query.reset($4);
      $$->comp = NOT_EXISTS_OP;
    }
    ;

comp_op:
      EQ { $$ = EQUAL_TO; }
    | LT { $$ = LESS_THAN; }
    | GT { $$ = GREAT_THAN; }
    | LE { $$ = LESS_EQUAL; }
    | GE { $$ = GREAT_EQUAL; }
    | NE { $$ = NOT_EQUAL; }
    | LIKE { $$ = LIKE_OP; }
    | NOT LIKE { $$ = NOT_LIKE_OP; }
    | IN { $$ = IN_OP; }
    | NOT IN { $$ = NOT_IN_OP; }
    ;

// your code here
group_by:
    /* empty */
    {
      $$ = nullptr;
    }
    | GROUP BY expression_list
    {
      $$ = $3;
    }
    ;

having:
    /* empty */
    {
      $$ = nullptr;
    }
    | HAVING expression
    {
      $$ = $2;
    }
    ;
load_data_stmt:
    LOAD DATA INFILE SSS INTO TABLE ID
    {
      char *tmp_file_name = common::substr($4, 1, strlen($4) - 2);

      $$ = new ParsedSqlNode(SCF_LOAD_DATA);
      $$->load_data.relation_name = $7;
      $$->load_data.file_name = tmp_file_name;
      free(tmp_file_name);
    }
    ;

explain_stmt:
    EXPLAIN command_wrapper
    {
      $$ = new ParsedSqlNode(SCF_EXPLAIN);
      $$->explain.sql_node = unique_ptr<ParsedSqlNode>($2);
    }
    ;

set_variable_stmt:
    SET ID EQ value
    {
      $$ = new ParsedSqlNode(SCF_SET_VARIABLE);
      $$->set_variable.name  = $2;
      $$->set_variable.value = *$4;
      delete $4;
    }
    ;

vector_literal:
    LSQUARE number_list RSQUARE {
      $$ = new Value();

      // 将number_list转换为向量字符串格式
      std::string vector_str = "[";
      if ($2 != nullptr) {
        for (size_t i = 0; i < $2->size(); i++) {
          if (i > 0) vector_str += ",";
          // 使用stringstream来保持浮点数精度，而不是std::to_string
          std::ostringstream oss;
          oss << std::fixed << std::setprecision(6) << (*$2)[i];
          std::string float_str = oss.str();
          // 移除尾随的零和小数点（如果需要）
          size_t end = float_str.find_last_not_of('0');
          if (end != std::string::npos && float_str[end] == '.') {
            end--;
          }
          if (end != std::string::npos) {
            float_str = float_str.substr(0, end + 1);
          }
          vector_str += float_str;
        }
        delete $2;
      }
      vector_str += "]";

      // 使用VectorType解析向量字符串
      VectorType vector_type;
      RC rc = vector_type.set_value_from_str(*$$, vector_str);
      if (rc != RC::SUCCESS) {
        delete $$;
        $$ = nullptr;
        yyerror(&@$, sql_string, sql_result, scanner, "Invalid vector literal");
      }
    }
    ;

number_list:
    float_number {
      $$ = new std::vector<double>();
      $$->push_back($1);
    }
    | number_list COMMA float_number {
      $$ = $1;
      $$->push_back($3);
    }
    ;

order_by:
    /* empty */
    {
      $$ = nullptr;
    }
    | ORDER BY order_by_item_list
    {
      $$ = $3;
    }
    ;

order_by_item_list:
    order_by_item
    {
      $$ = new std::vector<OrderBySqlNode>();
      $$->emplace_back(std::move(*$1));
      delete $1;
    }
    | order_by_item_list COMMA order_by_item
    {
      $$ = $1;
      $$->emplace_back(std::move(*$3));
      delete $3;
    }
    ;

order_by_item:
    expression
    {
      $$ = new OrderBySqlNode();
      $$->expression = unique_ptr<Expression>($1);
      $$->direction = ASC_ORDER;  // 默认升序
    }
    | expression ASC
    {
      $$ = new OrderBySqlNode();
      $$->expression = unique_ptr<Expression>($1);
      $$->direction = ASC_ORDER;
    }
    | expression DESC
    {
      $$ = new OrderBySqlNode();
      $$->expression = unique_ptr<Expression>($1);
      $$->direction = DESC_ORDER;
    }
    ;

limit_clause:
    /* empty */ { $$ = 0; }
    | LIMIT number { $$ = $2; }
    ;

/* A very loose option list: accept sequences like KEY=VALUE separated by comma, ignoring contents */
option_list:
    /* empty */ { $$ = 0; }
    | option_item { $$ = 0; }
    | option_list COMMA option_item { $$ = 0; }
    ;

option_item:
    ID EQ ID { $$ = 0; }
    | ID EQ NUMBER { $$ = 0; }
    | ID EQ FLOAT_LITERAL { $$ = 0; }
    ;

count_expr:
    COUNT LBRACE expression RBRACE {
      // Check if expression is a star expression
      if (dynamic_cast<StarExpr*>($3)) {
        $$ = new UnboundAggregateExpr("count", nullptr);
        delete $3; // Clean up the star expression
      } else {
        $$ = new UnboundAggregateExpr("count", unique_ptr<Expression>($3));
      }
      $$->set_name(token_name(sql_string, &@$));
    }
    ;

aggregate_expr:
    count_expr {
      $$ = $1;
    }
    | SUM LBRACE expression RBRACE {
      $$ = new UnboundAggregateExpr("sum", unique_ptr<Expression>($3));
      $$->set_name(token_name(sql_string, &@$));
    }
    | MAX LBRACE expression RBRACE {
      $$ = new UnboundAggregateExpr("max", unique_ptr<Expression>($3));
      $$->set_name(token_name(sql_string, &@$));
    }
    | MIN LBRACE expression RBRACE {
      $$ = new UnboundAggregateExpr("min", unique_ptr<Expression>($3));
      $$->set_name(token_name(sql_string, &@$));
    }
    | AVG LBRACE expression RBRACE {
      $$ = new UnboundAggregateExpr("avg", unique_ptr<Expression>($3));
      $$->set_name(token_name(sql_string, &@$));
    }
    ;

join_list:
    /* empty */
    {
      $$ = nullptr;
    }
    | join_list join_item
    {
      if ($1 != nullptr) {
        $$ = $1;
      } else {
        $$ = new std::vector<JoinSqlNode>();
      }
      $$->emplace_back(std::move(*$2));
      delete $2;
    }
    ;

join_item:
    INNER JOIN ID ON condition_list
    {
      $$ = new JoinSqlNode();
      $$->table_name = $3;
      $$->join_type = INNER_JOIN;
      if ($5 != nullptr) {
        $$->conditions.swap(*$5);
        delete $5;
      }
    }
    | INNER JOIN ID ID ON condition_list
    {
      $$ = new JoinSqlNode();
      $$->table_name = $3;
      $$->alias = $4;
      $$->join_type = INNER_JOIN;
      if ($6 != nullptr) {
        $$->conditions.swap(*$6);
        delete $6;
      }
    }
    | INNER JOIN ID AS ID ON condition_list
    {
      $$ = new JoinSqlNode();
      $$->table_name = $3;
      $$->alias = $5;
      $$->join_type = INNER_JOIN;
      if ($7 != nullptr) {
        $$->conditions.swap(*$7);
        delete $7;
      }
    }
    | JOIN ID ON condition_list
    {
      $$ = new JoinSqlNode();
      $$->table_name = $2;
      $$->join_type = INNER_JOIN;  // 默认为INNER JOIN
      if ($4 != nullptr) {
        $$->conditions.swap(*$4);
        delete $4;
      }
    }
    | JOIN ID ID ON condition_list
    {
      $$ = new JoinSqlNode();
      $$->table_name = $2;
      $$->alias = $3;
      $$->join_type = INNER_JOIN;  // 默认为INNER JOIN
      if ($5 != nullptr) {
        $$->conditions.swap(*$5);
        delete $5;
      }
    }
    | JOIN ID AS ID ON condition_list
    {
      $$ = new JoinSqlNode();
      $$->table_name = $2;
      $$->alias = $4;
      $$->join_type = INNER_JOIN;  // 默认为INNER JOIN
      if ($6 != nullptr) {
        $$->conditions.swap(*$6);
        delete $6;
      }
    }
    ;

sub_query:
    select_stmt
    {
      $$ = $1;
    }
    ;

opt_semicolon: /*empty*/
    | SEMICOLON
    ;
%%
//_____________________________________________________________________
extern void scan_string(const char *str, yyscan_t scanner);

int sql_parse(const char *s, ParsedSqlResult *sql_result) {
  yyscan_t scanner;
  std::vector<char *> allocated_strings;
  yylex_init_extra(static_cast<void*>(&allocated_strings),&scanner);
  scan_string(s, scanner);
  int result = yyparse(s, sql_result, scanner);

  for (char *ptr : allocated_strings) {
    free(ptr);
  }
  allocated_strings.clear();

  yylex_destroy(scanner);
  return result;
}
