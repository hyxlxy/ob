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

#ifndef YY_YY_YACC_SQL_TAB_H_INCLUDED
# define YY_YY_YACC_SQL_TAB_H_INCLUDED
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
    CREATE = 260,                  /* CREATE  */
    DROP = 261,                    /* DROP  */
    GROUP = 262,                   /* GROUP  */
    HAVING = 263,                  /* HAVING  */
    ORDER = 264,                   /* ORDER  */
    ASC = 265,                     /* ASC  */
    AS = 266,                      /* AS  */
    COUNT = 267,                   /* COUNT  */
    SUM = 268,                     /* SUM  */
    MAX = 269,                     /* MAX  */
    MIN = 270,                     /* MIN  */
    AVG = 271,                     /* AVG  */
    TABLE = 272,                   /* TABLE  */
    TABLES = 273,                  /* TABLES  */
    INDEX = 274,                   /* INDEX  */
    CALC = 275,                    /* CALC  */
    SELECT = 276,                  /* SELECT  */
    DESC = 277,                    /* DESC  */
    SHOW = 278,                    /* SHOW  */
    SYNC = 279,                    /* SYNC  */
    INSERT = 280,                  /* INSERT  */
    DELETE = 281,                  /* DELETE  */
    UPDATE = 282,                  /* UPDATE  */
    LBRACE = 283,                  /* LBRACE  */
    RBRACE = 284,                  /* RBRACE  */
    LSQUARE = 285,                 /* LSQUARE  */
    RSQUARE = 286,                 /* RSQUARE  */
    COMMA = 287,                   /* COMMA  */
    TRX_BEGIN = 288,               /* TRX_BEGIN  */
    TRX_COMMIT = 289,              /* TRX_COMMIT  */
    TRX_ROLLBACK = 290,            /* TRX_ROLLBACK  */
    INT_T = 291,                   /* INT_T  */
    STRING_T = 292,                /* STRING_T  */
    FLOAT_T = 293,                 /* FLOAT_T  */
    FLOAT_LITERAL = 294,           /* FLOAT_LITERAL  */
    DATE_T = 295,                  /* DATE_T  */
    VECTOR_T = 296,                /* VECTOR_T  */
    HELP = 297,                    /* HELP  */
    EXIT = 298,                    /* EXIT  */
    DOT = 299,                     /* DOT  */
    INTO = 300,                    /* INTO  */
    VALUES = 301,                  /* VALUES  */
    FROM = 302,                    /* FROM  */
    WHERE = 303,                   /* WHERE  */
    AND = 304,                     /* AND  */
    JOIN = 305,                    /* JOIN  */
    INNER = 306,                   /* INNER  */
    ON = 307,                      /* ON  */
    NOT = 308,                     /* NOT  */
    LIKE = 309,                    /* LIKE  */
    SET = 310,                     /* SET  */
    LOAD = 311,                    /* LOAD  */
    DATA = 312,                    /* DATA  */
    INFILE = 313,                  /* INFILE  */
    EXPLAIN = 314,                 /* EXPLAIN  */
    STORAGE = 315,                 /* STORAGE  */
    FORMAT = 316,                  /* FORMAT  */
    PRIMARY = 317,                 /* PRIMARY  */
    KEY = 318,                     /* KEY  */
    ANALYZE = 319,                 /* ANALYZE  */
    EQ = 320,                      /* EQ  */
    LT = 321,                      /* LT  */
    GT = 322,                      /* GT  */
    LE = 323,                      /* LE  */
    GE = 324,                      /* GE  */
    NE = 325,                      /* NE  */
    NUMBER = 326,                  /* NUMBER  */
    ID = 327,                      /* ID  */
    SSS = 328,                     /* SSS  */
    UMINUS = 329                   /* UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 140 "yacc_sql.y"

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
  float                                      floats;
  vector<float> *                            float_list;
  vector<OrderBySqlNode> *                   order_by_list;
  OrderBySqlNode *                           order_by_item;
  vector<JoinSqlNode> *                      join_list;
  vector<RelationWithAliasSqlNode> *         relation_with_alias_list;
  RelationWithAliasSqlNode *                 relation_with_alias;
  JoinSqlNode *                              join_item;

#line 165 "yacc_sql.tab.h"

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


#endif /* !YY_YY_YACC_SQL_TAB_H_INCLUDED  */
