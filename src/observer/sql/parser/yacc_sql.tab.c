/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 2 "yacc_sql.y"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iomanip>

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


#line 128 "yacc_sql.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif


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
    DATE_T = 294,                  /* DATE_T  */
    VECTOR_T = 295,                /* VECTOR_T  */
    HELP = 296,                    /* HELP  */
    EXIT = 297,                    /* EXIT  */
    DOT = 298,                     /* DOT  */
    INTO = 299,                    /* INTO  */
    VALUES = 300,                  /* VALUES  */
    FROM = 301,                    /* FROM  */
    WHERE = 302,                   /* WHERE  */
    AND = 303,                     /* AND  */
    JOIN = 304,                    /* JOIN  */
    INNER = 305,                   /* INNER  */
    ON = 306,                      /* ON  */
    NOT = 307,                     /* NOT  */
    NULL_T = 308,                  /* NULL_T  */
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
    IN = 320,                      /* IN  */
    EXISTS = 321,                  /* EXISTS  */
    UNIQUE = 322,                  /* UNIQUE  */
    EQ = 323,                      /* EQ  */
    LT = 324,                      /* LT  */
    GT = 325,                      /* GT  */
    LE = 326,                      /* LE  */
    GE = 327,                      /* GE  */
    NE = 328,                      /* NE  */
    NUMBER = 329,                  /* NUMBER  */
    FLOAT_LITERAL = 330,           /* FLOAT_LITERAL  */
    ID = 331,                      /* ID  */
    SSS = 332,                     /* SSS  */
    UMINUS = 333                   /* UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 143 "yacc_sql.y"

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

#line 280 "yacc_sql.tab.c"

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



/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_SEMICOLON = 3,                  /* SEMICOLON  */
  YYSYMBOL_BY = 4,                         /* BY  */
  YYSYMBOL_CREATE = 5,                     /* CREATE  */
  YYSYMBOL_DROP = 6,                       /* DROP  */
  YYSYMBOL_GROUP = 7,                      /* GROUP  */
  YYSYMBOL_HAVING = 8,                     /* HAVING  */
  YYSYMBOL_ORDER = 9,                      /* ORDER  */
  YYSYMBOL_ASC = 10,                       /* ASC  */
  YYSYMBOL_AS = 11,                        /* AS  */
  YYSYMBOL_COUNT = 12,                     /* COUNT  */
  YYSYMBOL_SUM = 13,                       /* SUM  */
  YYSYMBOL_MAX = 14,                       /* MAX  */
  YYSYMBOL_MIN = 15,                       /* MIN  */
  YYSYMBOL_AVG = 16,                       /* AVG  */
  YYSYMBOL_TABLE = 17,                     /* TABLE  */
  YYSYMBOL_TABLES = 18,                    /* TABLES  */
  YYSYMBOL_INDEX = 19,                     /* INDEX  */
  YYSYMBOL_CALC = 20,                      /* CALC  */
  YYSYMBOL_SELECT = 21,                    /* SELECT  */
  YYSYMBOL_DESC = 22,                      /* DESC  */
  YYSYMBOL_SHOW = 23,                      /* SHOW  */
  YYSYMBOL_SYNC = 24,                      /* SYNC  */
  YYSYMBOL_INSERT = 25,                    /* INSERT  */
  YYSYMBOL_DELETE = 26,                    /* DELETE  */
  YYSYMBOL_UPDATE = 27,                    /* UPDATE  */
  YYSYMBOL_LBRACE = 28,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 29,                    /* RBRACE  */
  YYSYMBOL_LSQUARE = 30,                   /* LSQUARE  */
  YYSYMBOL_RSQUARE = 31,                   /* RSQUARE  */
  YYSYMBOL_COMMA = 32,                     /* COMMA  */
  YYSYMBOL_TRX_BEGIN = 33,                 /* TRX_BEGIN  */
  YYSYMBOL_TRX_COMMIT = 34,                /* TRX_COMMIT  */
  YYSYMBOL_TRX_ROLLBACK = 35,              /* TRX_ROLLBACK  */
  YYSYMBOL_INT_T = 36,                     /* INT_T  */
  YYSYMBOL_STRING_T = 37,                  /* STRING_T  */
  YYSYMBOL_FLOAT_T = 38,                   /* FLOAT_T  */
  YYSYMBOL_DATE_T = 39,                    /* DATE_T  */
  YYSYMBOL_VECTOR_T = 40,                  /* VECTOR_T  */
  YYSYMBOL_HELP = 41,                      /* HELP  */
  YYSYMBOL_EXIT = 42,                      /* EXIT  */
  YYSYMBOL_DOT = 43,                       /* DOT  */
  YYSYMBOL_INTO = 44,                      /* INTO  */
  YYSYMBOL_VALUES = 45,                    /* VALUES  */
  YYSYMBOL_FROM = 46,                      /* FROM  */
  YYSYMBOL_WHERE = 47,                     /* WHERE  */
  YYSYMBOL_AND = 48,                       /* AND  */
  YYSYMBOL_JOIN = 49,                      /* JOIN  */
  YYSYMBOL_INNER = 50,                     /* INNER  */
  YYSYMBOL_ON = 51,                        /* ON  */
  YYSYMBOL_NOT = 52,                       /* NOT  */
  YYSYMBOL_NULL_T = 53,                    /* NULL_T  */
  YYSYMBOL_LIKE = 54,                      /* LIKE  */
  YYSYMBOL_SET = 55,                       /* SET  */
  YYSYMBOL_LOAD = 56,                      /* LOAD  */
  YYSYMBOL_DATA = 57,                      /* DATA  */
  YYSYMBOL_INFILE = 58,                    /* INFILE  */
  YYSYMBOL_EXPLAIN = 59,                   /* EXPLAIN  */
  YYSYMBOL_STORAGE = 60,                   /* STORAGE  */
  YYSYMBOL_FORMAT = 61,                    /* FORMAT  */
  YYSYMBOL_PRIMARY = 62,                   /* PRIMARY  */
  YYSYMBOL_KEY = 63,                       /* KEY  */
  YYSYMBOL_ANALYZE = 64,                   /* ANALYZE  */
  YYSYMBOL_IN = 65,                        /* IN  */
  YYSYMBOL_EXISTS = 66,                    /* EXISTS  */
  YYSYMBOL_UNIQUE = 67,                    /* UNIQUE  */
  YYSYMBOL_EQ = 68,                        /* EQ  */
  YYSYMBOL_LT = 69,                        /* LT  */
  YYSYMBOL_GT = 70,                        /* GT  */
  YYSYMBOL_LE = 71,                        /* LE  */
  YYSYMBOL_GE = 72,                        /* GE  */
  YYSYMBOL_NE = 73,                        /* NE  */
  YYSYMBOL_NUMBER = 74,                    /* NUMBER  */
  YYSYMBOL_FLOAT_LITERAL = 75,             /* FLOAT_LITERAL  */
  YYSYMBOL_ID = 76,                        /* ID  */
  YYSYMBOL_SSS = 77,                       /* SSS  */
  YYSYMBOL_78_ = 78,                       /* '+'  */
  YYSYMBOL_79_ = 79,                       /* '-'  */
  YYSYMBOL_80_ = 80,                       /* '*'  */
  YYSYMBOL_81_ = 81,                       /* '/'  */
  YYSYMBOL_UMINUS = 82,                    /* UMINUS  */
  YYSYMBOL_YYACCEPT = 83,                  /* $accept  */
  YYSYMBOL_commands = 84,                  /* commands  */
  YYSYMBOL_command_wrapper = 85,           /* command_wrapper  */
  YYSYMBOL_exit_stmt = 86,                 /* exit_stmt  */
  YYSYMBOL_help_stmt = 87,                 /* help_stmt  */
  YYSYMBOL_sync_stmt = 88,                 /* sync_stmt  */
  YYSYMBOL_begin_stmt = 89,                /* begin_stmt  */
  YYSYMBOL_commit_stmt = 90,               /* commit_stmt  */
  YYSYMBOL_rollback_stmt = 91,             /* rollback_stmt  */
  YYSYMBOL_drop_table_stmt = 92,           /* drop_table_stmt  */
  YYSYMBOL_analyze_table_stmt = 93,        /* analyze_table_stmt  */
  YYSYMBOL_show_tables_stmt = 94,          /* show_tables_stmt  */
  YYSYMBOL_desc_table_stmt = 95,           /* desc_table_stmt  */
  YYSYMBOL_create_index_stmt = 96,         /* create_index_stmt  */
  YYSYMBOL_drop_index_stmt = 97,           /* drop_index_stmt  */
  YYSYMBOL_create_table_stmt = 98,         /* create_table_stmt  */
  YYSYMBOL_attr_def_list = 99,             /* attr_def_list  */
  YYSYMBOL_attr_def = 100,                 /* attr_def  */
  YYSYMBOL_number = 101,                   /* number  */
  YYSYMBOL_float_number = 102,             /* float_number  */
  YYSYMBOL_type = 103,                     /* type  */
  YYSYMBOL_primary_key = 104,              /* primary_key  */
  YYSYMBOL_attr_list = 105,                /* attr_list  */
  YYSYMBOL_insert_stmt = 106,              /* insert_stmt  */
  YYSYMBOL_insert_value_list = 107,        /* insert_value_list  */
  YYSYMBOL_insert_value = 108,             /* insert_value  */
  YYSYMBOL_value = 109,                    /* value  */
  YYSYMBOL_storage_format = 110,           /* storage_format  */
  YYSYMBOL_delete_stmt = 111,              /* delete_stmt  */
  YYSYMBOL_update_stmt = 112,              /* update_stmt  */
  YYSYMBOL_select_stmt = 113,              /* select_stmt  */
  YYSYMBOL_calc_stmt = 114,                /* calc_stmt  */
  YYSYMBOL_expression_list = 115,          /* expression_list  */
  YYSYMBOL_projection_list = 116,          /* projection_list  */
  YYSYMBOL_opt_as_alias = 117,             /* opt_as_alias  */
  YYSYMBOL_expression = 118,               /* expression  */
  YYSYMBOL_rel_attr = 119,                 /* rel_attr  */
  YYSYMBOL_relation = 120,                 /* relation  */
  YYSYMBOL_rel_list = 121,                 /* rel_list  */
  YYSYMBOL_where = 122,                    /* where  */
  YYSYMBOL_condition_list = 123,           /* condition_list  */
  YYSYMBOL_condition = 124,                /* condition  */
  YYSYMBOL_comp_op = 125,                  /* comp_op  */
  YYSYMBOL_group_by = 126,                 /* group_by  */
  YYSYMBOL_having = 127,                   /* having  */
  YYSYMBOL_load_data_stmt = 128,           /* load_data_stmt  */
  YYSYMBOL_explain_stmt = 129,             /* explain_stmt  */
  YYSYMBOL_set_variable_stmt = 130,        /* set_variable_stmt  */
  YYSYMBOL_vector_literal = 131,           /* vector_literal  */
  YYSYMBOL_number_list = 132,              /* number_list  */
  YYSYMBOL_order_by = 133,                 /* order_by  */
  YYSYMBOL_order_by_item_list = 134,       /* order_by_item_list  */
  YYSYMBOL_order_by_item = 135,            /* order_by_item  */
  YYSYMBOL_count_expr = 136,               /* count_expr  */
  YYSYMBOL_aggregate_expr = 137,           /* aggregate_expr  */
  YYSYMBOL_join_list = 138,                /* join_list  */
  YYSYMBOL_join_item = 139,                /* join_item  */
  YYSYMBOL_sub_query = 140,                /* sub_query  */
  YYSYMBOL_opt_semicolon = 141             /* opt_semicolon  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  80
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   354

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  83
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  59
/* YYNRULES -- Number of rules.  */
#define YYNRULES  157
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  306

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   333


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,    80,    78,     2,    79,     2,    81,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    82
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   243,   243,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   275,   281,   286,   292,   298,   304,
     310,   316,   323,   329,   336,   348,   363,   371,   399,   402,
     415,   428,   441,   454,   467,   480,   495,   499,   500,   503,
     504,   505,   506,   507,   511,   514,   521,   525,   537,   553,
     556,   570,   571,   575,   582,   586,   590,   595,   599,   653,
     656,   663,   674,   704,   744,   762,   777,   786,   791,   803,
     811,   826,   827,   828,   832,   835,   838,   841,   844,   848,
     851,   857,   862,   871,   877,   880,   883,   903,   907,   915,
     919,   924,   931,   936,   951,   954,   960,   963,   968,   975,
     982,  1004,  1042,  1048,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1072,  1075,  1083,  1086,  1092,  1104,
    1112,  1122,  1160,  1164,  1172,  1175,  1182,  1188,  1197,  1203,
    1209,  1218,  1231,  1234,  1238,  1242,  1246,  1254,  1257,  1270,
    1280,  1291,  1302,  1312,  1323,  1337,  1343,  1344
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "SEMICOLON", "BY",
  "CREATE", "DROP", "GROUP", "HAVING", "ORDER", "ASC", "AS", "COUNT",
  "SUM", "MAX", "MIN", "AVG", "TABLE", "TABLES", "INDEX", "CALC", "SELECT",
  "DESC", "SHOW", "SYNC", "INSERT", "DELETE", "UPDATE", "LBRACE", "RBRACE",
  "LSQUARE", "RSQUARE", "COMMA", "TRX_BEGIN", "TRX_COMMIT", "TRX_ROLLBACK",
  "INT_T", "STRING_T", "FLOAT_T", "DATE_T", "VECTOR_T", "HELP", "EXIT",
  "DOT", "INTO", "VALUES", "FROM", "WHERE", "AND", "JOIN", "INNER", "ON",
  "NOT", "NULL_T", "LIKE", "SET", "LOAD", "DATA", "INFILE", "EXPLAIN",
  "STORAGE", "FORMAT", "PRIMARY", "KEY", "ANALYZE", "IN", "EXISTS",
  "UNIQUE", "EQ", "LT", "GT", "LE", "GE", "NE", "NUMBER", "FLOAT_LITERAL",
  "ID", "SSS", "'+'", "'-'", "'*'", "'/'", "UMINUS", "$accept", "commands",
  "command_wrapper", "exit_stmt", "help_stmt", "sync_stmt", "begin_stmt",
  "commit_stmt", "rollback_stmt", "drop_table_stmt", "analyze_table_stmt",
  "show_tables_stmt", "desc_table_stmt", "create_index_stmt",
  "drop_index_stmt", "create_table_stmt", "attr_def_list", "attr_def",
  "number", "float_number", "type", "primary_key", "attr_list",
  "insert_stmt", "insert_value_list", "insert_value", "value",
  "storage_format", "delete_stmt", "update_stmt", "select_stmt",
  "calc_stmt", "expression_list", "projection_list", "opt_as_alias",
  "expression", "rel_attr", "relation", "rel_list", "where",
  "condition_list", "condition", "comp_op", "group_by", "having",
  "load_data_stmt", "explain_stmt", "set_variable_stmt", "vector_literal",
  "number_list", "order_by", "order_by_item_list", "order_by_item",
  "count_expr", "aggregate_expr", "join_list", "join_item", "sub_query",
  "opt_semicolon", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-227)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     242,    -7,    20,   176,   176,   -55,    33,  -227,    14,    17,
     -20,  -227,  -227,  -227,  -227,  -227,     7,    16,   242,    54,
      94,   115,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,
    -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,
    -227,  -227,  -227,    43,    44,   121,    62,    66,   118,   123,
     124,   138,   140,    31,   -50,  -227,  -227,  -227,    -5,  -227,
     176,  -227,  -227,  -227,   147,  -227,  -227,  -227,  -227,    23,
      -4,  -227,  -227,    67,    71,   114,   102,   116,  -227,   111,
    -227,  -227,  -227,   166,   144,   125,  -227,   151,   176,   176,
     176,   176,   176,  -227,   -13,   174,  -227,  -227,  -227,    10,
     143,   132,  -227,   176,   176,   176,   176,   176,   203,   137,
     101,  -227,   139,  -227,   182,   179,   183,   145,   219,   154,
    -227,   156,   157,   184,   158,    83,   104,   119,   131,   164,
    -227,  -227,  -227,   -50,  -227,   207,  -227,  -227,   -32,   -32,
    -227,  -227,   176,     6,   205,  -227,   172,   212,   256,   232,
     198,  -227,   176,   226,   101,  -227,   189,  -227,   215,    87,
    -227,   243,   194,  -227,  -227,  -227,  -227,  -227,  -227,  -227,
    -227,    12,   241,  -227,   202,  -227,   137,   -21,   246,   258,
     -34,  -227,   252,  -227,  -227,  -227,  -227,  -227,  -227,   176,
    -227,   101,  -227,   228,  -227,   219,   265,  -227,  -227,  -227,
    -227,  -227,   -17,   253,   210,   259,  -227,  -227,   176,  -227,
    -227,   213,   239,   283,  -227,   258,   262,  -227,   264,   176,
      56,  -227,    21,   263,  -227,   183,   223,   230,   247,  -227,
     -43,   280,   279,   284,   210,  -227,     2,   236,   310,   307,
     287,  -227,   176,   288,  -227,  -227,   228,   289,  -227,  -227,
    -227,   290,  -227,   257,  -227,   270,   210,  -227,   293,   255,
     101,   272,     4,   176,   176,   232,  -227,   303,  -227,   263,
    -227,    46,   305,   277,  -227,  -227,  -227,   291,  -227,   101,
     267,   101,   294,  -227,    56,  -227,  -227,  -227,   286,  -227,
     210,   273,   101,  -227,   295,  -227,   101,  -227,   311,   268,
    -227,   101,  -227,  -227,  -227,  -227
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,    26,     0,     0,
       0,    27,    28,    29,    25,    24,     0,     0,     0,     0,
       0,   156,    23,    22,    15,    16,    17,    18,     9,    10,
      11,    12,    13,    14,     8,     5,     7,     6,     4,     3,
      19,    20,    21,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    66,    64,    65,    97,    68,
       0,    94,    90,    76,    77,    93,    67,   142,    95,   134,
      81,    33,    32,     0,     0,     0,     0,     0,   129,     0,
       1,   157,     2,     0,     0,     0,    30,     0,     0,     0,
       0,     0,     0,   155,     0,     0,    47,    48,   132,     0,
       0,     0,    89,     0,     0,     0,     0,     0,     0,     0,
     106,    75,     0,    82,    79,     0,   104,     0,     0,     0,
      31,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      88,    96,   131,     0,    91,     0,    98,    78,    84,    85,
      86,    87,     0,    99,   102,   147,     0,     0,     0,   134,
     107,    83,     0,     0,   106,    71,     0,   130,     0,     0,
      38,     0,     0,    36,   141,   143,   144,   145,   146,   133,
      92,   138,   135,   136,     0,   100,     0,   104,     0,     0,
       0,   120,   122,   114,   115,   116,   117,   118,   119,     0,
      74,   106,    80,     0,   105,     0,     0,    49,    50,    51,
      52,    53,    43,    54,     0,     0,   139,   140,     0,   101,
     103,     0,     0,   124,   148,     0,     0,   121,   123,     0,
     109,   108,     0,    59,    61,   104,     0,     0,     0,    44,
       0,     0,    56,     0,     0,   137,     0,     0,     0,   126,
       0,   112,     0,     0,    62,    63,     0,     0,    72,   128,
      46,     0,    45,     0,    39,    69,     0,    34,     0,     0,
     106,     0,     0,     0,     0,   134,   113,     0,   110,    59,
      58,    40,     0,     0,    37,    57,    35,     0,   152,   106,
       0,   106,     0,   125,   127,    73,   111,    60,     0,    41,
       0,     0,   106,   153,     0,   149,   106,    42,     0,     0,
     154,   106,   150,    55,    70,   151
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -227,  -227,   329,  -227,  -227,  -227,  -227,  -227,  -227,  -227,
    -227,  -227,  -227,  -227,  -227,  -227,  -227,   120,  -227,   216,
    -227,  -227,  -226,  -227,    79,   105,  -114,  -227,  -227,  -227,
       9,  -227,   -98,   200,  -227,    -3,  -227,  -227,   177,  -163,
    -151,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,
    -143,  -227,   146,  -227,  -227,  -227,  -227,  -161,  -227
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,   203,   160,   251,    98,
     202,   231,   233,    35,   247,   223,    62,   274,    36,    37,
      93,    39,    63,    69,   114,   148,    65,   144,   145,   155,
     149,   150,   189,   239,   265,    40,    41,    42,    66,    99,
     111,   172,   173,    67,    68,   177,   214,    95,    82
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      64,    70,   135,   194,   157,   137,   190,   112,   258,    38,
      43,   227,    44,   259,   213,   280,   130,   174,   216,   253,
     217,    71,   206,   100,    96,    97,   154,    38,   211,   212,
     275,   218,   108,   159,   207,   228,   229,    46,   101,    47,
     221,   132,   133,    48,    49,    50,    51,    52,   106,   107,
      94,    72,     4,   260,   240,   281,    75,   102,    73,    53,
      45,    54,   248,    74,   298,   104,   105,   106,   107,   109,
     110,    79,   113,    77,   104,   105,   106,   107,   261,   224,
     282,   225,   175,    76,    55,   125,   126,   127,   128,   129,
     104,   105,   106,   107,    80,   244,   245,    64,   288,   289,
      64,   138,   139,   140,   141,    56,    57,    58,    59,   278,
      60,    61,   164,    48,    49,    50,    51,    52,    81,    83,
      84,   243,   285,   197,   198,   199,   200,   201,   293,    53,
     295,    54,   224,   165,   104,   105,   106,   107,    86,   171,
      85,   300,    87,   115,   267,   302,    88,   116,   166,    70,
     305,    89,    90,   146,    55,    48,    49,    50,    51,    52,
     167,   104,   105,   106,   107,   283,    91,   147,    92,   117,
     118,    53,   134,    54,   119,    56,    57,    58,    59,   103,
      60,    61,   104,   105,   106,   107,   220,   120,    48,    49,
      50,    51,    52,   168,   121,   122,    55,   104,   105,   106,
     107,   123,   124,   131,    53,   171,    54,   142,   136,   104,
     105,   106,   107,   143,   152,   151,    64,    56,    57,    58,
      59,   156,    60,    61,   153,   104,   105,   106,   107,    55,
     154,   158,   159,   161,   163,   162,   170,   176,   178,    64,
     179,   108,   104,   105,   106,   107,   191,     1,     2,    54,
      56,    57,    58,    59,   193,    60,    61,   195,    54,   196,
      64,   284,     3,     4,     5,     6,     7,     8,     9,    10,
     205,   204,    55,   208,   215,    11,    12,    13,   209,     4,
     219,    55,   226,    14,    15,   230,   232,   234,   237,   236,
     238,   241,   242,    56,    57,   246,    59,    16,    17,   249,
     252,    18,    56,    57,   250,    59,    19,   222,   180,   255,
     181,   256,   262,   257,   263,   264,   266,   268,   270,   271,
     272,   182,   276,   279,   183,   184,   185,   186,   187,   188,
     273,   277,   286,   290,   104,   105,   106,   107,   291,   297,
     303,   299,   292,   294,   304,   296,   301,    78,   287,   169,
     254,   269,   192,   210,   235
};

static const yytype_int16 yycheck[] =
{
       3,     4,   100,   154,   118,   103,   149,    11,   234,     0,
      17,    28,    19,    11,   177,    11,    29,    11,   179,    62,
      54,    76,    10,    28,    74,    75,    47,    18,    49,    50,
     256,    65,     9,    76,    22,    52,    53,    17,    43,    19,
     191,    31,    32,    12,    13,    14,    15,    16,    80,    81,
      53,    18,    21,    51,   215,    51,    76,    60,    44,    28,
      67,    30,   225,    46,   290,    78,    79,    80,    81,    46,
      47,    17,    76,    57,    78,    79,    80,    81,    76,   193,
      76,   195,    76,    76,    53,    88,    89,    90,    91,    92,
      78,    79,    80,    81,     0,    74,    75,   100,    52,    53,
     103,   104,   105,   106,   107,    74,    75,    76,    77,   260,
      79,    80,    29,    12,    13,    14,    15,    16,     3,    76,
      76,   219,   265,    36,    37,    38,    39,    40,   279,    28,
     281,    30,   246,    29,    78,    79,    80,    81,    76,   142,
      19,   292,    76,    76,   242,   296,    28,    76,    29,   152,
     301,    28,    28,    52,    53,    12,    13,    14,    15,    16,
      29,    78,    79,    80,    81,   263,    28,    66,    28,    55,
      68,    28,    29,    30,    58,    74,    75,    76,    77,    32,
      79,    80,    78,    79,    80,    81,   189,    76,    12,    13,
      14,    15,    16,    29,    28,    51,    53,    78,    79,    80,
      81,    76,    51,    29,    28,   208,    30,     4,    76,    78,
      79,    80,    81,    76,    32,    76,   219,    74,    75,    76,
      77,    76,    79,    80,    45,    78,    79,    80,    81,    53,
      47,    77,    76,    76,    76,    51,    29,    32,    66,   242,
      28,     9,    78,    79,    80,    81,    48,     5,     6,    30,
      74,    75,    76,    77,    28,    79,    80,    68,    30,    44,
     263,   264,    20,    21,    22,    23,    24,    25,    26,    27,
      76,    28,    53,    32,    28,    33,    34,    35,    76,    21,
      28,    53,    17,    41,    42,    32,    76,    28,    49,    76,
       7,    29,    28,    74,    75,    32,    77,    55,    56,    76,
      53,    59,    74,    75,    74,    77,    64,    79,    52,    29,
      54,    32,    76,    29,     4,     8,    29,    29,    29,    29,
      63,    65,    29,    51,    68,    69,    70,    71,    72,    73,
      60,    76,    29,    28,    78,    79,    80,    81,    61,    53,
      29,    68,    51,    76,    76,    51,    51,    18,   269,   133,
     230,   246,   152,   176,   208
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     5,     6,    20,    21,    22,    23,    24,    25,    26,
      27,    33,    34,    35,    41,    42,    55,    56,    59,    64,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,   106,   111,   112,   113,   114,
     128,   129,   130,    17,    19,    67,    17,    19,    12,    13,
      14,    15,    16,    28,    30,    53,    74,    75,    76,    77,
      79,    80,   109,   115,   118,   119,   131,   136,   137,   116,
     118,    76,    18,    44,    46,    76,    76,    57,    85,    17,
       0,     3,   141,    76,    76,    19,    76,    76,    28,    28,
      28,    28,    28,   113,   118,   140,    74,    75,   102,   132,
      28,    43,   118,    32,    78,    79,    80,    81,     9,    46,
      47,   133,    11,    76,   117,    76,    76,    55,    68,    58,
      76,    28,    51,    76,    51,   118,   118,   118,   118,   118,
      29,    29,    31,    32,    29,   115,    76,   115,   118,   118,
     118,   118,     4,    76,   120,   121,    52,    66,   118,   123,
     124,    76,    32,    45,    47,   122,    76,   109,    77,    76,
     100,    76,    51,    76,    29,    29,    29,    29,    29,   102,
      29,   118,   134,   135,    11,    76,    32,   138,    66,    28,
      52,    54,    65,    68,    69,    70,    71,    72,    73,   125,
     133,    48,   116,    28,   123,    68,    44,    36,    37,    38,
      39,    40,   103,    99,    28,    76,    10,    22,    32,    76,
     121,    49,    50,   122,   139,    28,   140,    54,    65,    28,
     118,   123,    79,   108,   109,   109,    17,    28,    52,    53,
      32,   104,    76,   105,    28,   135,    76,    49,     7,   126,
     140,    29,    28,   115,    74,    75,    32,   107,   122,    76,
      74,   101,    53,    62,   100,    29,    32,    29,   105,    11,
      51,    76,    76,     4,     8,   127,    29,   115,    29,   108,
      29,    29,    63,    60,   110,   105,    29,    76,   123,    51,
      11,    51,    76,   115,   118,   133,    29,   107,    52,    53,
      28,    61,    51,   123,    76,   123,    51,    53,   105,    68,
     123,    51,   123,    29,    76,   123
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    83,    84,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    96,    97,    98,    99,    99,
     100,   100,   100,   100,   100,   100,   101,   102,   102,   103,
     103,   103,   103,   103,   104,   104,   105,   105,   106,   107,
     107,   108,   108,   108,   109,   109,   109,   109,   109,   110,
     110,   111,   112,   113,   113,   113,   114,   115,   115,   116,
     116,   117,   117,   117,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   119,   119,   120,
     120,   120,   121,   121,   122,   122,   123,   123,   123,   124,
     124,   124,   124,   124,   125,   125,   125,   125,   125,   125,
     125,   125,   125,   125,   126,   126,   127,   127,   128,   129,
     130,   131,   132,   132,   133,   133,   134,   134,   135,   135,
     135,   136,   137,   137,   137,   137,   137,   138,   138,   139,
     139,   139,   139,   139,   139,   140,   141,   141
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     2,     2,     8,     9,     5,     9,     0,     3,
       5,     6,     7,     2,     3,     4,     1,     1,     1,     1,
       1,     1,     1,     1,     0,     6,     1,     3,     8,     0,
       3,     1,     2,     2,     1,     1,     1,     1,     1,     0,
       4,     4,     7,     9,     5,     3,     2,     1,     3,     2,
       4,     0,     1,     2,     3,     3,     3,     3,     3,     2,
       1,     3,     4,     1,     1,     1,     3,     1,     3,     1,
       2,     3,     1,     3,     0,     2,     0,     1,     3,     3,
       5,     6,     4,     5,     1,     1,     1,     1,     1,     1,
       1,     2,     1,     2,     0,     3,     0,     2,     7,     2,
       4,     3,     1,     3,     0,     3,     1,     3,     1,     2,
       2,     4,     1,     4,     4,     4,     4,     0,     2,     5,
       6,     7,     4,     5,     6,     1,     0,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (&yylloc, sql_string, sql_result, scanner, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location, sql_string, sql_result, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  YY_USE (sql_string);
  YY_USE (sql_result);
  YY_USE (scanner);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp, sql_string, sql_result, scanner);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]), sql_string, sql_result, scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, sql_string, sql_result, scanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  YY_USE (sql_string);
  YY_USE (sql_result);
  YY_USE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, &yylloc, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* commands: command_wrapper opt_semicolon  */
#line 244 "yacc_sql.y"
  {
    unique_ptr<ParsedSqlNode> sql_node = unique_ptr<ParsedSqlNode>((yyvsp[-1].sql_node));
    sql_result->add_sql_node(std::move(sql_node));
  }
#line 2049 "yacc_sql.tab.c"
    break;

  case 24: /* exit_stmt: EXIT  */
#line 275 "yacc_sql.y"
         {
      (void)yynerrs;  // 这么写为了消除yynerrs未使用的告警。如果你有更好的方法欢迎提PR
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXIT);
    }
#line 2058 "yacc_sql.tab.c"
    break;

  case 25: /* help_stmt: HELP  */
#line 281 "yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_HELP);
    }
#line 2066 "yacc_sql.tab.c"
    break;

  case 26: /* sync_stmt: SYNC  */
#line 286 "yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SYNC);
    }
#line 2074 "yacc_sql.tab.c"
    break;

  case 27: /* begin_stmt: TRX_BEGIN  */
#line 292 "yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_BEGIN);
    }
#line 2082 "yacc_sql.tab.c"
    break;

  case 28: /* commit_stmt: TRX_COMMIT  */
#line 298 "yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_COMMIT);
    }
#line 2090 "yacc_sql.tab.c"
    break;

  case 29: /* rollback_stmt: TRX_ROLLBACK  */
#line 304 "yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ROLLBACK);
    }
#line 2098 "yacc_sql.tab.c"
    break;

  case 30: /* drop_table_stmt: DROP TABLE ID  */
#line 310 "yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_TABLE);
      (yyval.sql_node)->drop_table.relation_name = (yyvsp[0].cstring);
    }
#line 2107 "yacc_sql.tab.c"
    break;

  case 31: /* analyze_table_stmt: ANALYZE TABLE ID  */
#line 316 "yacc_sql.y"
                     {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ANALYZE_TABLE);
      (yyval.sql_node)->analyze_table.relation_name = (yyvsp[0].cstring);
    }
#line 2116 "yacc_sql.tab.c"
    break;

  case 32: /* show_tables_stmt: SHOW TABLES  */
#line 323 "yacc_sql.y"
                {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SHOW_TABLES);
    }
#line 2124 "yacc_sql.tab.c"
    break;

  case 33: /* desc_table_stmt: DESC ID  */
#line 329 "yacc_sql.y"
             {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DESC_TABLE);
      (yyval.sql_node)->desc_table.relation_name = (yyvsp[0].cstring);
    }
#line 2133 "yacc_sql.tab.c"
    break;

  case 34: /* create_index_stmt: CREATE INDEX ID ON ID LBRACE attr_list RBRACE  */
#line 337 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_INDEX);
      CreateIndexSqlNode &create_index = (yyval.sql_node)->create_index;
      create_index.index_name = (yyvsp[-5].cstring);
      create_index.relation_name = (yyvsp[-3].cstring);
      if ((yyvsp[-1].key_list) != nullptr) {
        create_index.attribute_names.swap(*(yyvsp[-1].key_list));
        delete (yyvsp[-1].key_list);
      }
      create_index.is_unique = false;
    }
#line 2149 "yacc_sql.tab.c"
    break;

  case 35: /* create_index_stmt: CREATE UNIQUE INDEX ID ON ID LBRACE attr_list RBRACE  */
#line 349 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_INDEX);
      CreateIndexSqlNode &create_index = (yyval.sql_node)->create_index;
      create_index.index_name = (yyvsp[-5].cstring);
      create_index.relation_name = (yyvsp[-3].cstring);
      if ((yyvsp[-1].key_list) != nullptr) {
        create_index.attribute_names.swap(*(yyvsp[-1].key_list));
        delete (yyvsp[-1].key_list);
      }
      create_index.is_unique = true;
    }
#line 2165 "yacc_sql.tab.c"
    break;

  case 36: /* drop_index_stmt: DROP INDEX ID ON ID  */
#line 364 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_INDEX);
      (yyval.sql_node)->drop_index.index_name = (yyvsp[-2].cstring);
      (yyval.sql_node)->drop_index.relation_name = (yyvsp[0].cstring);
    }
#line 2175 "yacc_sql.tab.c"
    break;

  case 37: /* create_table_stmt: CREATE TABLE ID LBRACE attr_def attr_def_list primary_key RBRACE storage_format  */
#line 372 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_TABLE);
      CreateTableSqlNode &create_table = (yyval.sql_node)->create_table;
      create_table.relation_name = (yyvsp[-6].cstring);
      //free($3);

      vector<AttrInfoSqlNode> *src_attrs = (yyvsp[-3].attr_infos);

      if (src_attrs != nullptr) {
        create_table.attr_infos.swap(*src_attrs);
        delete src_attrs;
      }
      create_table.attr_infos.emplace_back(*(yyvsp[-4].attr_info));
      reverse(create_table.attr_infos.begin(), create_table.attr_infos.end());
      delete (yyvsp[-4].attr_info);
      if ((yyvsp[-2].key_list) != nullptr) {
        create_table.primary_keys.swap(*(yyvsp[-2].key_list));
        delete (yyvsp[-2].key_list);
      }
      if ((yyvsp[0].cstring) != nullptr) {
        create_table.storage_format = (yyvsp[0].cstring);
      }
    }
#line 2203 "yacc_sql.tab.c"
    break;

  case 38: /* attr_def_list: %empty  */
#line 399 "yacc_sql.y"
    {
      (yyval.attr_infos) = nullptr;
    }
#line 2211 "yacc_sql.tab.c"
    break;

  case 39: /* attr_def_list: attr_def_list COMMA attr_def  */
#line 403 "yacc_sql.y"
    {
      if ((yyvsp[-2].attr_infos) != nullptr) {
        (yyval.attr_infos) = (yyvsp[-2].attr_infos);
      } else {
        (yyval.attr_infos) = new vector<AttrInfoSqlNode>;
      }
      (yyval.attr_infos)->insert((yyval.attr_infos)->begin(), *(yyvsp[0].attr_info));
      delete (yyvsp[0].attr_info);
    }
#line 2225 "yacc_sql.tab.c"
    break;

  case 40: /* attr_def: ID type LBRACE number RBRACE  */
#line 416 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-3].number);
      (yyval.attr_info)->name = (yyvsp[-4].cstring);
      (yyval.attr_info)->nullable = true;  // default nullable
      // For VECTOR(n), store bytes for [dimension(int32) + n float32 values]
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int) + ((yyvsp[-1].number) * sizeof(float)));
      } else {
        (yyval.attr_info)->length = (yyvsp[-1].number);
      }
    }
#line 2242 "yacc_sql.tab.c"
    break;

  case 41: /* attr_def: ID type LBRACE number RBRACE NULL_T  */
#line 429 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-4].number);
      (yyval.attr_info)->name = (yyvsp[-5].cstring);
      (yyval.attr_info)->nullable = true;  // explicitly nullable
      // For VECTOR(n), store bytes for [dimension(int32) + n float32 values]
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int) + ((yyvsp[-2].number) * sizeof(float)));
      } else {
        (yyval.attr_info)->length = (yyvsp[-2].number);
      }
    }
#line 2259 "yacc_sql.tab.c"
    break;

  case 42: /* attr_def: ID type LBRACE number RBRACE NOT NULL_T  */
#line 442 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-5].number);
      (yyval.attr_info)->name = (yyvsp[-6].cstring);
      (yyval.attr_info)->nullable = false;  // explicitly not nullable
      // For VECTOR(n), store bytes for [dimension(int32) + n float32 values]
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int) + ((yyvsp[-3].number) * sizeof(float)));
      } else {
        (yyval.attr_info)->length = (yyvsp[-3].number);
      }
    }
#line 2276 "yacc_sql.tab.c"
    break;

  case 43: /* attr_def: ID type  */
#line 455 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[0].number);
      (yyval.attr_info)->name = (yyvsp[-1].cstring);
      (yyval.attr_info)->nullable = true;  // default nullable
      // Default length: INT/FLOAT/DATE/STRING fallback 4; VECTOR default header size (dimension)
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int));
      } else {
        (yyval.attr_info)->length = 4;
      }
    }
#line 2293 "yacc_sql.tab.c"
    break;

  case 44: /* attr_def: ID type NULL_T  */
#line 468 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-1].number);
      (yyval.attr_info)->name = (yyvsp[-2].cstring);
      (yyval.attr_info)->nullable = true;  // explicitly nullable
      // Default length: INT/FLOAT/DATE/STRING fallback 4; VECTOR default header size (dimension)
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int));
      } else {
        (yyval.attr_info)->length = 4;
      }
    }
#line 2310 "yacc_sql.tab.c"
    break;

  case 45: /* attr_def: ID type NOT NULL_T  */
#line 481 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-2].number);
      (yyval.attr_info)->name = (yyvsp[-3].cstring);
      (yyval.attr_info)->nullable = false;  // explicitly not nullable
      // Default length: INT/FLOAT/DATE/STRING fallback 4; VECTOR default header size (dimension)
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int));
      } else {
        (yyval.attr_info)->length = 4;
      }
    }
#line 2327 "yacc_sql.tab.c"
    break;

  case 46: /* number: NUMBER  */
#line 495 "yacc_sql.y"
           {(yyval.number) = (yyvsp[0].number);}
#line 2333 "yacc_sql.tab.c"
    break;

  case 47: /* float_number: NUMBER  */
#line 499 "yacc_sql.y"
           {(yyval.floats) = (double)(yyvsp[0].number);}
#line 2339 "yacc_sql.tab.c"
    break;

  case 48: /* float_number: FLOAT_LITERAL  */
#line 500 "yacc_sql.y"
                    {(yyval.floats) = (yyvsp[0].floats);}
#line 2345 "yacc_sql.tab.c"
    break;

  case 49: /* type: INT_T  */
#line 503 "yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::INTS); }
#line 2351 "yacc_sql.tab.c"
    break;

  case 50: /* type: STRING_T  */
#line 504 "yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::CHARS); }
#line 2357 "yacc_sql.tab.c"
    break;

  case 51: /* type: FLOAT_T  */
#line 505 "yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::FLOATS); }
#line 2363 "yacc_sql.tab.c"
    break;

  case 52: /* type: DATE_T  */
#line 506 "yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::DATES); }
#line 2369 "yacc_sql.tab.c"
    break;

  case 53: /* type: VECTOR_T  */
#line 507 "yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::VECTORS); }
#line 2375 "yacc_sql.tab.c"
    break;

  case 54: /* primary_key: %empty  */
#line 511 "yacc_sql.y"
    {
      (yyval.key_list) = nullptr;
    }
#line 2383 "yacc_sql.tab.c"
    break;

  case 55: /* primary_key: COMMA PRIMARY KEY LBRACE attr_list RBRACE  */
#line 515 "yacc_sql.y"
    {
      (yyval.key_list) = (yyvsp[-1].key_list);
    }
#line 2391 "yacc_sql.tab.c"
    break;

  case 56: /* attr_list: ID  */
#line 521 "yacc_sql.y"
       {
      (yyval.key_list) = new vector<string>();
      (yyval.key_list)->push_back((yyvsp[0].cstring));
    }
#line 2400 "yacc_sql.tab.c"
    break;

  case 57: /* attr_list: ID COMMA attr_list  */
#line 525 "yacc_sql.y"
                         {
      if ((yyvsp[0].key_list) != nullptr) {
        (yyval.key_list) = (yyvsp[0].key_list);
      } else {
        (yyval.key_list) = new vector<string>;
      }

      (yyval.key_list)->insert((yyval.key_list)->begin(), (yyvsp[-2].cstring));
    }
#line 2414 "yacc_sql.tab.c"
    break;

  case 58: /* insert_stmt: INSERT INTO ID VALUES LBRACE insert_value insert_value_list RBRACE  */
#line 538 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_INSERT);
      (yyval.sql_node)->insertion.relation_name = (yyvsp[-5].cstring);
      if ((yyvsp[-1].value_list) != nullptr) {
        (yyval.sql_node)->insertion.values.swap(*(yyvsp[-1].value_list));
        delete (yyvsp[-1].value_list);
      }
      (yyval.sql_node)->insertion.values.emplace_back(*(yyvsp[-2].value));
      reverse((yyval.sql_node)->insertion.values.begin(), (yyval.sql_node)->insertion.values.end());
      delete (yyvsp[-2].value);
    }
#line 2430 "yacc_sql.tab.c"
    break;

  case 59: /* insert_value_list: %empty  */
#line 553 "yacc_sql.y"
    {
      (yyval.value_list) = nullptr;
    }
#line 2438 "yacc_sql.tab.c"
    break;

  case 60: /* insert_value_list: COMMA insert_value insert_value_list  */
#line 556 "yacc_sql.y"
                                            {
      if ((yyvsp[0].value_list) != nullptr) {
        (yyval.value_list) = (yyvsp[0].value_list);
      } else {
        (yyval.value_list) = new vector<Value>;
      }
      (yyval.value_list)->emplace_back(*(yyvsp[-1].value));
      delete (yyvsp[-1].value);
    }
#line 2452 "yacc_sql.tab.c"
    break;

  case 62: /* insert_value: '-' NUMBER  */
#line 571 "yacc_sql.y"
                 {
      (yyval.value) = new Value(-(int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2461 "yacc_sql.tab.c"
    break;

  case 63: /* insert_value: '-' FLOAT_LITERAL  */
#line 575 "yacc_sql.y"
                        {
      (yyval.value) = new Value(-(float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2470 "yacc_sql.tab.c"
    break;

  case 64: /* value: NUMBER  */
#line 582 "yacc_sql.y"
           {
      (yyval.value) = new Value((int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2479 "yacc_sql.tab.c"
    break;

  case 65: /* value: FLOAT_LITERAL  */
#line 586 "yacc_sql.y"
                   {
      (yyval.value) = new Value((float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2488 "yacc_sql.tab.c"
    break;

  case 66: /* value: NULL_T  */
#line 590 "yacc_sql.y"
            {
      (yyval.value) = new Value();
      (yyval.value)->set_type(AttrType::UNDEFINED);
      (yyloc) = (yylsp[0]);
    }
#line 2498 "yacc_sql.tab.c"
    break;

  case 67: /* value: vector_literal  */
#line 595 "yacc_sql.y"
                    {
      (yyval.value) = (yyvsp[0].value);
      (yyloc) = (yylsp[0]);
    }
#line 2507 "yacc_sql.tab.c"
    break;

  case 68: /* value: SSS  */
#line 599 "yacc_sql.y"
         {
      char *tmp = common::substr((yyvsp[0].cstring),1,strlen((yyvsp[0].cstring))-2);

      // 尝试解析为向量类型
      if (tmp[0] == '[') {
        Value *vector_value = new Value();
        RC rc = DataType::type_instance(AttrType::VECTORS)->set_value_from_str(*vector_value, tmp);
        if (rc == RC::SUCCESS) {
          (yyval.value) = vector_value;
          free(tmp);
        } else {
          delete vector_value;
          // 如果向量解析失败，作为字符串处理
          (yyval.value) = new Value(tmp);
          free(tmp);
        }
      } else {
        // 检查是否为日期格式（YYYY-MM-DD或YYYY-M-D等）
        bool looks_like_date = false;
        int dash_count = 0;
        for (int i = 0; tmp[i] != '\0'; i++) {
          if (tmp[i] == '-') {
            dash_count++;
          }
        }
        // 如果有两个破折号，可能是日期格式
        if (dash_count == 2) {
          looks_like_date = true;
        }

        if (looks_like_date) {
          // 尝试解析为日期类型
          Value *date_value = new Value();
          RC rc = DataType::type_instance(AttrType::DATES)->set_value_from_str(*date_value, tmp);
          if (rc == RC::SUCCESS) {
            (yyval.value) = date_value;
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
          (yyval.value) = new Value(tmp);
          free(tmp);
        }
      }
    }
#line 2563 "yacc_sql.tab.c"
    break;

  case 69: /* storage_format: %empty  */
#line 653 "yacc_sql.y"
    {
      (yyval.cstring) = nullptr;
    }
#line 2571 "yacc_sql.tab.c"
    break;

  case 70: /* storage_format: STORAGE FORMAT EQ ID  */
#line 657 "yacc_sql.y"
    {
      (yyval.cstring) = (yyvsp[0].cstring);
    }
#line 2579 "yacc_sql.tab.c"
    break;

  case 71: /* delete_stmt: DELETE FROM ID where  */
#line 664 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DELETE);
      (yyval.sql_node)->deletion.relation_name = (yyvsp[-1].cstring);
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.sql_node)->deletion.conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 2592 "yacc_sql.tab.c"
    break;

  case 72: /* update_stmt: UPDATE ID SET ID EQ value where  */
#line 675 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_UPDATE);
      (yyval.sql_node)->update.relation_name = (yyvsp[-5].cstring);
      (yyval.sql_node)->update.attribute_name = (yyvsp[-3].cstring);
      (yyval.sql_node)->update.value = *(yyvsp[-1].value);
      (yyval.sql_node)->update.is_sub_query = false;
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.sql_node)->update.conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
      delete (yyvsp[-1].value);
    }
#line 2609 "yacc_sql.tab.c"
    break;

  case 73: /* select_stmt: SELECT projection_list FROM rel_list join_list where group_by having order_by  */
#line 705 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SELECT);
      if ((yyvsp[-7].expression_list) != nullptr) {
        (yyval.sql_node)->selection.expressions.swap(*(yyvsp[-7].expression_list));
        delete (yyvsp[-7].expression_list);
      }

      if ((yyvsp[-5].relation_with_alias_list) != nullptr) {
        for (auto &it : *(yyvsp[-5].relation_with_alias_list)) {
          (yyval.sql_node)->selection.relations.push_back(it.table_name);
          (yyval.sql_node)->selection.relation_aliases.push_back(it.alias);
        }
        delete (yyvsp[-5].relation_with_alias_list);
      }

      if ((yyvsp[-4].join_list) != nullptr) {
        (yyval.sql_node)->selection.joins.swap(*(yyvsp[-4].join_list));
        delete (yyvsp[-4].join_list);
      }

      if ((yyvsp[-3].condition_list) != nullptr) {
        (yyval.sql_node)->selection.conditions.swap(*(yyvsp[-3].condition_list));
        delete (yyvsp[-3].condition_list);
      }

      if ((yyvsp[-2].expression_list) != nullptr) {
        (yyval.sql_node)->selection.group_by.swap(*(yyvsp[-2].expression_list));
        delete (yyvsp[-2].expression_list);
      }

      if ((yyvsp[-1].expression) != nullptr) {
        (yyval.sql_node)->selection.having_expr.reset((yyvsp[-1].expression));
      }

      if ((yyvsp[0].order_by_list) != nullptr) {
        (yyval.sql_node)->selection.order_by.swap(*(yyvsp[0].order_by_list));
        delete (yyvsp[0].order_by_list);
      }
    }
#line 2653 "yacc_sql.tab.c"
    break;

  case 74: /* select_stmt: SELECT projection_list WHERE condition_list order_by  */
#line 745 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SELECT);
      if ((yyvsp[-3].expression_list) != nullptr) {
        (yyval.sql_node)->selection.expressions.swap(*(yyvsp[-3].expression_list));
        delete (yyvsp[-3].expression_list);
      }

      if ((yyvsp[-1].condition_list) != nullptr) {
        (yyval.sql_node)->selection.conditions.swap(*(yyvsp[-1].condition_list));
        delete (yyvsp[-1].condition_list);
      }

      if ((yyvsp[0].order_by_list) != nullptr) {
        (yyval.sql_node)->selection.order_by.swap(*(yyvsp[0].order_by_list));
        delete (yyvsp[0].order_by_list);
      }
    }
#line 2675 "yacc_sql.tab.c"
    break;

  case 75: /* select_stmt: SELECT projection_list order_by  */
#line 763 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SELECT);
      if ((yyvsp[-1].expression_list) != nullptr) {
        (yyval.sql_node)->selection.expressions.swap(*(yyvsp[-1].expression_list));
        delete (yyvsp[-1].expression_list);
      }

      if ((yyvsp[0].order_by_list) != nullptr) {
        (yyval.sql_node)->selection.order_by.swap(*(yyvsp[0].order_by_list));
        delete (yyvsp[0].order_by_list);
      }
    }
#line 2692 "yacc_sql.tab.c"
    break;

  case 76: /* calc_stmt: CALC expression_list  */
#line 778 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CALC);
      (yyval.sql_node)->calc.expressions.swap(*(yyvsp[0].expression_list));
      delete (yyvsp[0].expression_list);
    }
#line 2702 "yacc_sql.tab.c"
    break;

  case 77: /* expression_list: expression  */
#line 787 "yacc_sql.y"
    {
      (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      (yyval.expression_list)->emplace_back((yyvsp[0].expression));
    }
#line 2711 "yacc_sql.tab.c"
    break;

  case 78: /* expression_list: expression COMMA expression_list  */
#line 792 "yacc_sql.y"
    {
      if ((yyvsp[0].expression_list) != nullptr) {
        (yyval.expression_list) = (yyvsp[0].expression_list);
      } else {
        (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      }
      (yyval.expression_list)->emplace((yyval.expression_list)->begin(), (yyvsp[-2].expression));
    }
#line 2724 "yacc_sql.tab.c"
    break;

  case 79: /* projection_list: expression opt_as_alias  */
#line 804 "yacc_sql.y"
    {
      if ((yyvsp[0].cstring) != nullptr) {
        (yyvsp[-1].expression)->set_name((yyvsp[0].cstring));
      }
      (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      (yyval.expression_list)->emplace_back((yyvsp[-1].expression));
    }
#line 2736 "yacc_sql.tab.c"
    break;

  case 80: /* projection_list: expression opt_as_alias COMMA projection_list  */
#line 812 "yacc_sql.y"
    {
      if ((yyvsp[-2].cstring) != nullptr) {
        (yyvsp[-3].expression)->set_name((yyvsp[-2].cstring));
      }
      if ((yyvsp[0].expression_list) != nullptr) {
        (yyval.expression_list) = (yyvsp[0].expression_list);
      } else {
        (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      }
      (yyval.expression_list)->emplace((yyval.expression_list)->begin(), (yyvsp[-3].expression));
    }
#line 2752 "yacc_sql.tab.c"
    break;

  case 81: /* opt_as_alias: %empty  */
#line 826 "yacc_sql.y"
                { (yyval.cstring) = nullptr; }
#line 2758 "yacc_sql.tab.c"
    break;

  case 82: /* opt_as_alias: ID  */
#line 827 "yacc_sql.y"
         { (yyval.cstring) = (yyvsp[0].cstring); }
#line 2764 "yacc_sql.tab.c"
    break;

  case 83: /* opt_as_alias: AS ID  */
#line 828 "yacc_sql.y"
            { (yyval.cstring) = (yyvsp[0].cstring); }
#line 2770 "yacc_sql.tab.c"
    break;

  case 84: /* expression: expression '+' expression  */
#line 832 "yacc_sql.y"
                              {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::ADD, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2778 "yacc_sql.tab.c"
    break;

  case 85: /* expression: expression '-' expression  */
#line 835 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::SUB, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2786 "yacc_sql.tab.c"
    break;

  case 86: /* expression: expression '*' expression  */
#line 838 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::MUL, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2794 "yacc_sql.tab.c"
    break;

  case 87: /* expression: expression '/' expression  */
#line 841 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::DIV, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2802 "yacc_sql.tab.c"
    break;

  case 88: /* expression: LBRACE expression RBRACE  */
#line 844 "yacc_sql.y"
                               {
      // Simple parentheses for grouping - just return the inner expression
      (yyval.expression) = (yyvsp[-1].expression);
    }
#line 2811 "yacc_sql.tab.c"
    break;

  case 89: /* expression: '-' expression  */
#line 848 "yacc_sql.y"
                                  {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::NEGATIVE, (yyvsp[0].expression), nullptr, sql_string, &(yyloc));
    }
#line 2819 "yacc_sql.tab.c"
    break;

  case 90: /* expression: value  */
#line 851 "yacc_sql.y"
            {
      (yyval.expression) = new ValueExpr(*(yyvsp[0].value));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete (yyvsp[0].value);
    }
#line 2829 "yacc_sql.tab.c"
    break;

  case 91: /* expression: ID LBRACE RBRACE  */
#line 857 "yacc_sql.y"
                       {
      std::vector<std::unique_ptr<Expression>> args;
      (yyval.expression) = new FunctionExpr((yyvsp[-2].cstring), std::move(args));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2839 "yacc_sql.tab.c"
    break;

  case 92: /* expression: ID LBRACE expression_list RBRACE  */
#line 862 "yacc_sql.y"
                                       {
      std::vector<std::unique_ptr<Expression>> args;
      if ((yyvsp[-1].expression_list) != nullptr) {
        args.swap(*(yyvsp[-1].expression_list));
        delete (yyvsp[-1].expression_list);
      }
      (yyval.expression) = new FunctionExpr((yyvsp[-3].cstring), std::move(args));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2853 "yacc_sql.tab.c"
    break;

  case 93: /* expression: rel_attr  */
#line 871 "yacc_sql.y"
               {
      RelAttrSqlNode *node = (yyvsp[0].rel_attr);
      (yyval.expression) = new UnboundFieldExpr(node->relation_name, node->attribute_name);
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete (yyvsp[0].rel_attr);
    }
#line 2864 "yacc_sql.tab.c"
    break;

  case 94: /* expression: '*'  */
#line 877 "yacc_sql.y"
          {
      (yyval.expression) = new StarExpr();
    }
#line 2872 "yacc_sql.tab.c"
    break;

  case 95: /* expression: aggregate_expr  */
#line 880 "yacc_sql.y"
                     {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 2880 "yacc_sql.tab.c"
    break;

  case 96: /* expression: LBRACE sub_query RBRACE  */
#line 883 "yacc_sql.y"
                              {
      printf("DEBUG: [PARSER] Parsing scalar subquery\n");
      ParsedSqlNode *parsed_node = (yyvsp[-1].sql_node);
      if (parsed_node && parsed_node->flag == SCF_SELECT) {
        printf("DEBUG: [PARSER] Creating SubQueryExpr\n");
        // Transfer ownership of the SelectSqlNode
        std::unique_ptr<SelectSqlNode> select_node(new SelectSqlNode(std::move(parsed_node->selection)));
        (yyval.expression) = new SubQueryExpr(std::move(select_node));
        (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
        delete parsed_node;
        printf("DEBUG: [PARSER] SubQueryExpr created successfully, ptr=%p, type=%d\n", (yyval.expression), static_cast<int>((yyval.expression)->type()));
      } else {
        printf("ERROR: [PARSER] Invalid subquery node\n");
        (yyval.expression) = nullptr;
        delete parsed_node;
      }
    }
#line 2902 "yacc_sql.tab.c"
    break;

  case 97: /* rel_attr: ID  */
#line 903 "yacc_sql.y"
       {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->attribute_name = (yyvsp[0].cstring);
    }
#line 2911 "yacc_sql.tab.c"
    break;

  case 98: /* rel_attr: ID DOT ID  */
#line 907 "yacc_sql.y"
                {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->relation_name  = (yyvsp[-2].cstring);
      (yyval.rel_attr)->attribute_name = (yyvsp[0].cstring);
    }
#line 2921 "yacc_sql.tab.c"
    break;

  case 99: /* relation: ID  */
#line 915 "yacc_sql.y"
       {
      (yyval.relation_with_alias) = new RelationWithAliasSqlNode();
      (yyval.relation_with_alias)->table_name = (yyvsp[0].cstring);
    }
#line 2930 "yacc_sql.tab.c"
    break;

  case 100: /* relation: ID ID  */
#line 919 "yacc_sql.y"
            {
      (yyval.relation_with_alias) = new RelationWithAliasSqlNode();
      (yyval.relation_with_alias)->table_name = (yyvsp[-1].cstring);
      (yyval.relation_with_alias)->alias = (yyvsp[0].cstring);
    }
#line 2940 "yacc_sql.tab.c"
    break;

  case 101: /* relation: ID AS ID  */
#line 924 "yacc_sql.y"
               {
      (yyval.relation_with_alias) = new RelationWithAliasSqlNode();
      (yyval.relation_with_alias)->table_name = (yyvsp[-2].cstring);
      (yyval.relation_with_alias)->alias = (yyvsp[0].cstring);
    }
#line 2950 "yacc_sql.tab.c"
    break;

  case 102: /* rel_list: relation  */
#line 931 "yacc_sql.y"
             {
      (yyval.relation_with_alias_list) = new vector<RelationWithAliasSqlNode>();
      (yyval.relation_with_alias_list)->emplace_back(std::move(*(yyvsp[0].relation_with_alias)));
      delete (yyvsp[0].relation_with_alias);
    }
#line 2960 "yacc_sql.tab.c"
    break;

  case 103: /* rel_list: relation COMMA rel_list  */
#line 936 "yacc_sql.y"
                              {
      if ((yyvsp[0].relation_with_alias_list) != nullptr) {
        (yyval.relation_with_alias_list) = (yyvsp[0].relation_with_alias_list);
      } else {
        (yyval.relation_with_alias_list) = new vector<RelationWithAliasSqlNode>();
      }
      (yyval.relation_with_alias_list)->emplace((yyval.relation_with_alias_list)->begin(), std::move(*(yyvsp[-2].relation_with_alias)));
      delete (yyvsp[-2].relation_with_alias);
    }
#line 2974 "yacc_sql.tab.c"
    break;

  case 104: /* where: %empty  */
#line 951 "yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 2982 "yacc_sql.tab.c"
    break;

  case 105: /* where: WHERE condition_list  */
#line 954 "yacc_sql.y"
                           {
      (yyval.condition_list) = (yyvsp[0].condition_list);
    }
#line 2990 "yacc_sql.tab.c"
    break;

  case 106: /* condition_list: %empty  */
#line 960 "yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 2998 "yacc_sql.tab.c"
    break;

  case 107: /* condition_list: condition  */
#line 963 "yacc_sql.y"
                {
      (yyval.condition_list) = new vector<ConditionSqlNode>;
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[0].condition)));
      delete (yyvsp[0].condition);
    }
#line 3008 "yacc_sql.tab.c"
    break;

  case 108: /* condition_list: condition AND condition_list  */
#line 968 "yacc_sql.y"
                                   {
      (yyval.condition_list) = (yyvsp[0].condition_list);
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[-2].condition)));
      delete (yyvsp[-2].condition);
    }
#line 3018 "yacc_sql.tab.c"
    break;

  case 109: /* condition: expression comp_op expression  */
#line 976 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr.reset((yyvsp[-2].expression));
      (yyval.condition)->right_expr.reset((yyvsp[0].expression));
      (yyval.condition)->comp = (yyvsp[-1].comp);
    }
#line 3029 "yacc_sql.tab.c"
    break;

  case 110: /* condition: expression IN LBRACE expression_list RBRACE  */
#line 983 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr.reset((yyvsp[-4].expression));
      // For IN values operation, store values directly in right_values
      if ((yyvsp[-1].expression_list) != nullptr) {
        // Convert expressions to values
        for (auto &expr : *(yyvsp[-1].expression_list)) {
          Value val;
          RC rc = expr->try_get_value(val);
          if (rc == RC::SUCCESS) {
            (yyval.condition)->right_values.push_back(val);
          } else {
            // If we can't get constant values, fall back to ListExpr
            (yyval.condition)->right_expr.reset(new ListExpr(std::move(*(yyvsp[-1].expression_list))));
            break;
          }
        }
        delete (yyvsp[-1].expression_list);
      }
      (yyval.condition)->comp = IN_VALUES_OP;
    }
#line 3055 "yacc_sql.tab.c"
    break;

  case 111: /* condition: expression NOT IN LBRACE expression_list RBRACE  */
#line 1005 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr.reset((yyvsp[-5].expression));
      // For NOT IN values operation, store values directly in right_values
      if ((yyvsp[-1].expression_list) != nullptr) {
        // Convert expressions to values
        for (auto &expr : *(yyvsp[-1].expression_list)) {
          Value val;
          RC rc = expr->try_get_value(val);
          if (rc == RC::SUCCESS) {
            (yyval.condition)->right_values.push_back(val);
          } else {
            // If we can't get constant values, fall back to ListExpr
            (yyval.condition)->right_expr.reset(new ListExpr(std::move(*(yyvsp[-1].expression_list))));
            break;
          }
        }
        delete (yyvsp[-1].expression_list);
      }
      (yyval.condition)->comp = NOT_IN_VALUES_OP;
    }
#line 3081 "yacc_sql.tab.c"
    break;

  case 112: /* condition: EXISTS LBRACE sub_query RBRACE  */
#line 1043 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->sub_query.reset((yyvsp[-1].sql_node));
      (yyval.condition)->comp = EXISTS_OP;
    }
#line 3091 "yacc_sql.tab.c"
    break;

  case 113: /* condition: NOT EXISTS LBRACE sub_query RBRACE  */
#line 1049 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->sub_query.reset((yyvsp[-1].sql_node));
      (yyval.condition)->comp = NOT_EXISTS_OP;
    }
#line 3101 "yacc_sql.tab.c"
    break;

  case 114: /* comp_op: EQ  */
#line 1057 "yacc_sql.y"
         { (yyval.comp) = EQUAL_TO; }
#line 3107 "yacc_sql.tab.c"
    break;

  case 115: /* comp_op: LT  */
#line 1058 "yacc_sql.y"
         { (yyval.comp) = LESS_THAN; }
#line 3113 "yacc_sql.tab.c"
    break;

  case 116: /* comp_op: GT  */
#line 1059 "yacc_sql.y"
         { (yyval.comp) = GREAT_THAN; }
#line 3119 "yacc_sql.tab.c"
    break;

  case 117: /* comp_op: LE  */
#line 1060 "yacc_sql.y"
         { (yyval.comp) = LESS_EQUAL; }
#line 3125 "yacc_sql.tab.c"
    break;

  case 118: /* comp_op: GE  */
#line 1061 "yacc_sql.y"
         { (yyval.comp) = GREAT_EQUAL; }
#line 3131 "yacc_sql.tab.c"
    break;

  case 119: /* comp_op: NE  */
#line 1062 "yacc_sql.y"
         { (yyval.comp) = NOT_EQUAL; }
#line 3137 "yacc_sql.tab.c"
    break;

  case 120: /* comp_op: LIKE  */
#line 1063 "yacc_sql.y"
           { (yyval.comp) = LIKE_OP; }
#line 3143 "yacc_sql.tab.c"
    break;

  case 121: /* comp_op: NOT LIKE  */
#line 1064 "yacc_sql.y"
               { (yyval.comp) = NOT_LIKE_OP; }
#line 3149 "yacc_sql.tab.c"
    break;

  case 122: /* comp_op: IN  */
#line 1065 "yacc_sql.y"
         { (yyval.comp) = IN_OP; }
#line 3155 "yacc_sql.tab.c"
    break;

  case 123: /* comp_op: NOT IN  */
#line 1066 "yacc_sql.y"
             { (yyval.comp) = NOT_IN_OP; }
#line 3161 "yacc_sql.tab.c"
    break;

  case 124: /* group_by: %empty  */
#line 1072 "yacc_sql.y"
    {
      (yyval.expression_list) = nullptr;
    }
#line 3169 "yacc_sql.tab.c"
    break;

  case 125: /* group_by: GROUP BY expression_list  */
#line 1076 "yacc_sql.y"
    {
      (yyval.expression_list) = (yyvsp[0].expression_list);
    }
#line 3177 "yacc_sql.tab.c"
    break;

  case 126: /* having: %empty  */
#line 1083 "yacc_sql.y"
    {
      (yyval.expression) = nullptr;
    }
#line 3185 "yacc_sql.tab.c"
    break;

  case 127: /* having: HAVING expression  */
#line 1087 "yacc_sql.y"
    {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 3193 "yacc_sql.tab.c"
    break;

  case 128: /* load_data_stmt: LOAD DATA INFILE SSS INTO TABLE ID  */
#line 1093 "yacc_sql.y"
    {
      char *tmp_file_name = common::substr((yyvsp[-3].cstring), 1, strlen((yyvsp[-3].cstring)) - 2);

      (yyval.sql_node) = new ParsedSqlNode(SCF_LOAD_DATA);
      (yyval.sql_node)->load_data.relation_name = (yyvsp[0].cstring);
      (yyval.sql_node)->load_data.file_name = tmp_file_name;
      free(tmp_file_name);
    }
#line 3206 "yacc_sql.tab.c"
    break;

  case 129: /* explain_stmt: EXPLAIN command_wrapper  */
#line 1105 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXPLAIN);
      (yyval.sql_node)->explain.sql_node = unique_ptr<ParsedSqlNode>((yyvsp[0].sql_node));
    }
#line 3215 "yacc_sql.tab.c"
    break;

  case 130: /* set_variable_stmt: SET ID EQ value  */
#line 1113 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SET_VARIABLE);
      (yyval.sql_node)->set_variable.name  = (yyvsp[-2].cstring);
      (yyval.sql_node)->set_variable.value = *(yyvsp[0].value);
      delete (yyvsp[0].value);
    }
#line 3226 "yacc_sql.tab.c"
    break;

  case 131: /* vector_literal: LSQUARE number_list RSQUARE  */
#line 1122 "yacc_sql.y"
                                {
      (yyval.value) = new Value();

      // 将number_list转换为向量字符串格式
      std::string vector_str = "[";
      if ((yyvsp[-1].float_list) != nullptr) {
        for (size_t i = 0; i < (yyvsp[-1].float_list)->size(); i++) {
          if (i > 0) vector_str += ",";
          // 使用stringstream来保持浮点数精度，而不是std::to_string
          std::ostringstream oss;
          oss << std::fixed << std::setprecision(6) << (*(yyvsp[-1].float_list))[i];
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
        delete (yyvsp[-1].float_list);
      }
      vector_str += "]";

      // 使用VectorType解析向量字符串
      VectorType vector_type;
      RC rc = vector_type.set_value_from_str(*(yyval.value), vector_str);
      if (rc != RC::SUCCESS) {
        delete (yyval.value);
        (yyval.value) = nullptr;
        yyerror(&(yyloc), sql_string, sql_result, scanner, "Invalid vector literal");
      }
    }
#line 3266 "yacc_sql.tab.c"
    break;

  case 132: /* number_list: float_number  */
#line 1160 "yacc_sql.y"
                 {
      (yyval.float_list) = new std::vector<double>();
      (yyval.float_list)->push_back((yyvsp[0].floats));
    }
#line 3275 "yacc_sql.tab.c"
    break;

  case 133: /* number_list: number_list COMMA float_number  */
#line 1164 "yacc_sql.y"
                                     {
      (yyval.float_list) = (yyvsp[-2].float_list);
      (yyval.float_list)->push_back((yyvsp[0].floats));
    }
#line 3284 "yacc_sql.tab.c"
    break;

  case 134: /* order_by: %empty  */
#line 1172 "yacc_sql.y"
    {
      (yyval.order_by_list) = nullptr;
    }
#line 3292 "yacc_sql.tab.c"
    break;

  case 135: /* order_by: ORDER BY order_by_item_list  */
#line 1176 "yacc_sql.y"
    {
      (yyval.order_by_list) = (yyvsp[0].order_by_list);
    }
#line 3300 "yacc_sql.tab.c"
    break;

  case 136: /* order_by_item_list: order_by_item  */
#line 1183 "yacc_sql.y"
    {
      (yyval.order_by_list) = new std::vector<OrderBySqlNode>();
      (yyval.order_by_list)->emplace_back(std::move(*(yyvsp[0].order_by_item)));
      delete (yyvsp[0].order_by_item);
    }
#line 3310 "yacc_sql.tab.c"
    break;

  case 137: /* order_by_item_list: order_by_item_list COMMA order_by_item  */
#line 1189 "yacc_sql.y"
    {
      (yyval.order_by_list) = (yyvsp[-2].order_by_list);
      (yyval.order_by_list)->emplace_back(std::move(*(yyvsp[0].order_by_item)));
      delete (yyvsp[0].order_by_item);
    }
#line 3320 "yacc_sql.tab.c"
    break;

  case 138: /* order_by_item: expression  */
#line 1198 "yacc_sql.y"
    {
      (yyval.order_by_item) = new OrderBySqlNode();
      (yyval.order_by_item)->expression = unique_ptr<Expression>((yyvsp[0].expression));
      (yyval.order_by_item)->direction = ASC_ORDER;  // 默认升序
    }
#line 3330 "yacc_sql.tab.c"
    break;

  case 139: /* order_by_item: expression ASC  */
#line 1204 "yacc_sql.y"
    {
      (yyval.order_by_item) = new OrderBySqlNode();
      (yyval.order_by_item)->expression = unique_ptr<Expression>((yyvsp[-1].expression));
      (yyval.order_by_item)->direction = ASC_ORDER;
    }
#line 3340 "yacc_sql.tab.c"
    break;

  case 140: /* order_by_item: expression DESC  */
#line 1210 "yacc_sql.y"
    {
      (yyval.order_by_item) = new OrderBySqlNode();
      (yyval.order_by_item)->expression = unique_ptr<Expression>((yyvsp[-1].expression));
      (yyval.order_by_item)->direction = DESC_ORDER;
    }
#line 3350 "yacc_sql.tab.c"
    break;

  case 141: /* count_expr: COUNT LBRACE expression RBRACE  */
#line 1218 "yacc_sql.y"
                                   {
      // Check if expression is a star expression
      if (dynamic_cast<StarExpr*>((yyvsp[-1].expression))) {
        (yyval.expression) = new UnboundAggregateExpr("count", nullptr);
        delete (yyvsp[-1].expression); // Clean up the star expression
      } else {
        (yyval.expression) = new UnboundAggregateExpr("count", unique_ptr<Expression>((yyvsp[-1].expression)));
      }
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3365 "yacc_sql.tab.c"
    break;

  case 142: /* aggregate_expr: count_expr  */
#line 1231 "yacc_sql.y"
               {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 3373 "yacc_sql.tab.c"
    break;

  case 143: /* aggregate_expr: SUM LBRACE expression RBRACE  */
#line 1234 "yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("sum", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3382 "yacc_sql.tab.c"
    break;

  case 144: /* aggregate_expr: MAX LBRACE expression RBRACE  */
#line 1238 "yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("max", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3391 "yacc_sql.tab.c"
    break;

  case 145: /* aggregate_expr: MIN LBRACE expression RBRACE  */
#line 1242 "yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("min", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3400 "yacc_sql.tab.c"
    break;

  case 146: /* aggregate_expr: AVG LBRACE expression RBRACE  */
#line 1246 "yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("avg", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3409 "yacc_sql.tab.c"
    break;

  case 147: /* join_list: %empty  */
#line 1254 "yacc_sql.y"
    {
      (yyval.join_list) = nullptr;
    }
#line 3417 "yacc_sql.tab.c"
    break;

  case 148: /* join_list: join_list join_item  */
#line 1258 "yacc_sql.y"
    {
      if ((yyvsp[-1].join_list) != nullptr) {
        (yyval.join_list) = (yyvsp[-1].join_list);
      } else {
        (yyval.join_list) = new std::vector<JoinSqlNode>();
      }
      (yyval.join_list)->emplace_back(std::move(*(yyvsp[0].join_item)));
      delete (yyvsp[0].join_item);
    }
#line 3431 "yacc_sql.tab.c"
    break;

  case 149: /* join_item: INNER JOIN ID ON condition_list  */
#line 1271 "yacc_sql.y"
    {
      (yyval.join_item) = new JoinSqlNode();
      (yyval.join_item)->table_name = (yyvsp[-2].cstring);
      (yyval.join_item)->join_type = INNER_JOIN;
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.join_item)->conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 3445 "yacc_sql.tab.c"
    break;

  case 150: /* join_item: INNER JOIN ID ID ON condition_list  */
#line 1281 "yacc_sql.y"
    {
      (yyval.join_item) = new JoinSqlNode();
      (yyval.join_item)->table_name = (yyvsp[-3].cstring);
      (yyval.join_item)->alias = (yyvsp[-2].cstring);
      (yyval.join_item)->join_type = INNER_JOIN;
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.join_item)->conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 3460 "yacc_sql.tab.c"
    break;

  case 151: /* join_item: INNER JOIN ID AS ID ON condition_list  */
#line 1292 "yacc_sql.y"
    {
      (yyval.join_item) = new JoinSqlNode();
      (yyval.join_item)->table_name = (yyvsp[-4].cstring);
      (yyval.join_item)->alias = (yyvsp[-2].cstring);
      (yyval.join_item)->join_type = INNER_JOIN;
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.join_item)->conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 3475 "yacc_sql.tab.c"
    break;

  case 152: /* join_item: JOIN ID ON condition_list  */
#line 1303 "yacc_sql.y"
    {
      (yyval.join_item) = new JoinSqlNode();
      (yyval.join_item)->table_name = (yyvsp[-2].cstring);
      (yyval.join_item)->join_type = INNER_JOIN;  // 默认为INNER JOIN
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.join_item)->conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 3489 "yacc_sql.tab.c"
    break;

  case 153: /* join_item: JOIN ID ID ON condition_list  */
#line 1313 "yacc_sql.y"
    {
      (yyval.join_item) = new JoinSqlNode();
      (yyval.join_item)->table_name = (yyvsp[-3].cstring);
      (yyval.join_item)->alias = (yyvsp[-2].cstring);
      (yyval.join_item)->join_type = INNER_JOIN;  // 默认为INNER JOIN
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.join_item)->conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 3504 "yacc_sql.tab.c"
    break;

  case 154: /* join_item: JOIN ID AS ID ON condition_list  */
#line 1324 "yacc_sql.y"
    {
      (yyval.join_item) = new JoinSqlNode();
      (yyval.join_item)->table_name = (yyvsp[-4].cstring);
      (yyval.join_item)->alias = (yyvsp[-2].cstring);
      (yyval.join_item)->join_type = INNER_JOIN;  // 默认为INNER JOIN
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.join_item)->conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 3519 "yacc_sql.tab.c"
    break;

  case 155: /* sub_query: select_stmt  */
#line 1338 "yacc_sql.y"
    {
      (yyval.sql_node) = (yyvsp[0].sql_node);
    }
#line 3527 "yacc_sql.tab.c"
    break;


#line 3531 "yacc_sql.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (&yylloc, sql_string, sql_result, scanner, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc, sql_string, sql_result, scanner);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp, sql_string, sql_result, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, sql_string, sql_result, scanner, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, sql_string, sql_result, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp, sql_string, sql_result, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 1346 "yacc_sql.y"

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
