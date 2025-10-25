/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_HOME_HXL_DESKTOP_OCEAMBASE_MINIOB_COMPETITION_2025_SRC_OBSERVER_SQL_PARSER_YACC_SQL_HPP_INCLUDED
# define YY_YY_HOME_HXL_DESKTOP_OCEAMBASE_MINIOB_COMPETITION_2025_SRC_OBSERVER_SQL_PARSER_YACC_SQL_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    SEMICOLON = 258,               /* SEMICOLON  */
    BY = 259,                      /* BY  */
    LIMIT = 260,                   /* LIMIT  */
    WITH = 261,                    /* WITH  */
    CREATE = 262,                  /* CREATE  */
    DROP = 263,                    /* DROP  */
    GROUP = 264,                   /* GROUP  */
    HAVING = 265,                  /* HAVING  */
    ORDER = 266,                   /* ORDER  */
    ASC = 267,                     /* ASC  */
    AS = 268,                      /* AS  */
    COUNT = 269,                   /* COUNT  */
    SUM = 270,                     /* SUM  */
    MAX = 271,                     /* MAX  */
    MIN = 272,                     /* MIN  */
    AVG = 273,                     /* AVG  */
    TABLE = 274,                   /* TABLE  */
    TABLES = 275,                  /* TABLES  */
    INDEX = 276,                   /* INDEX  */
    ALTER = 277,                   /* ALTER  */
    ADD = 278,                     /* ADD  */
    CHANGE = 279,                  /* CHANGE  */
    COLUMN = 280,                  /* COLUMN  */
    RENAME = 281,                  /* RENAME  */
    TO = 282,                      /* TO  */
    IF = 283,                      /* IF  */
    EXISTS = 284,                  /* EXISTS  */
    CALC = 285,                    /* CALC  */
    SELECT = 286,                  /* SELECT  */
    UNION = 287,                   /* UNION  */
    UNION_ALL = 288,               /* UNION_ALL  */
    DESC = 289,                    /* DESC  */
    SHOW = 290,                    /* SHOW  */
    SYNC = 291,                    /* SYNC  */
    INSERT = 292,                  /* INSERT  */
    DELETE = 293,                  /* DELETE  */
    UPDATE = 294,                  /* UPDATE  */
    LBRACE = 295,                  /* LBRACE  */
    RBRACE = 296,                  /* RBRACE  */
    LSQUARE = 297,                 /* LSQUARE  */
    RSQUARE = 298,                 /* RSQUARE  */
    COMMA = 299,                   /* COMMA  */
    TRX_BEGIN = 300,               /* TRX_BEGIN  */
    TRX_COMMIT = 301,              /* TRX_COMMIT  */
    TRX_ROLLBACK = 302,            /* TRX_ROLLBACK  */
    INT_T = 303,                   /* INT_T  */
    STRING_T = 304,                /* STRING_T  */
    FLOAT_T = 305,                 /* FLOAT_T  */
    DATE_T = 306,                  /* DATE_T  */
    VECTOR_T = 307,                /* VECTOR_T  */
    TEXT_T = 308,                  /* TEXT_T  */
    HELP = 309,                    /* HELP  */
    EXIT = 310,                    /* EXIT  */
    DOT = 311,                     /* DOT  */
    INTO = 312,                    /* INTO  */
    VALUES = 313,                  /* VALUES  */
    FROM = 314,                    /* FROM  */
    WHERE = 315,                   /* WHERE  */
    AND = 316,                     /* AND  */
    OR = 317,                      /* OR  */
    JOIN = 318,                    /* JOIN  */
    INNER = 319,                   /* INNER  */
    ON = 320,                      /* ON  */
    NOT = 321,                     /* NOT  */
    NULL_T = 322,                  /* NULL_T  */
    LIKE = 323,                    /* LIKE  */
    SET = 324,                     /* SET  */
    LOAD = 325,                    /* LOAD  */
    DATA = 326,                    /* DATA  */
    INFILE = 327,                  /* INFILE  */
    EXPLAIN = 328,                 /* EXPLAIN  */
    STORAGE = 329,                 /* STORAGE  */
    FORMAT = 330,                  /* FORMAT  */
    PRIMARY = 331,                 /* PRIMARY  */
    KEY = 332,                     /* KEY  */
    ANALYZE = 333,                 /* ANALYZE  */
    IN = 334,                      /* IN  */
    IS = 335,                      /* IS  */
    UNIQUE = 336,                  /* UNIQUE  */
    VIEW = 337,                    /* VIEW  */
    EQ = 338,                      /* EQ  */
    LT = 339,                      /* LT  */
    GT = 340,                      /* GT  */
    LE = 341,                      /* LE  */
    GE = 342,                      /* GE  */
    NE = 343,                      /* NE  */
    NUMBER = 344,                  /* NUMBER  */
    FLOAT_LITERAL = 345,           /* FLOAT_LITERAL  */
    ID = 346,                      /* ID  */
    SSS = 347,                     /* SSS  */
    UMINUS = 348                   /* UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 159 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"

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
  vector<UpdateKvSqlNode> *                  update_kv_list;

#line 185 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif




int yyparse (const char * sql_string, ParsedSqlResult * sql_result, void * scanner);


#endif /* !YY_YY_HOME_HXL_DESKTOP_OCEAMBASE_MINIOB_COMPETITION_2025_SRC_OBSERVER_SQL_PARSER_YACC_SQL_HPP_INCLUDED  */
