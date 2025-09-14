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
    ID = 330,                      /* ID  */
    SSS = 331,                     /* SSS  */
    UMINUS = 332                   /* UMINUS  */
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

#line 279 "yacc_sql.tab.c"

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
  YYSYMBOL_FLOAT_LITERAL = 39,             /* FLOAT_LITERAL  */
  YYSYMBOL_DATE_T = 40,                    /* DATE_T  */
  YYSYMBOL_VECTOR_T = 41,                  /* VECTOR_T  */
  YYSYMBOL_HELP = 42,                      /* HELP  */
  YYSYMBOL_EXIT = 43,                      /* EXIT  */
  YYSYMBOL_DOT = 44,                       /* DOT  */
  YYSYMBOL_INTO = 45,                      /* INTO  */
  YYSYMBOL_VALUES = 46,                    /* VALUES  */
  YYSYMBOL_FROM = 47,                      /* FROM  */
  YYSYMBOL_WHERE = 48,                     /* WHERE  */
  YYSYMBOL_AND = 49,                       /* AND  */
  YYSYMBOL_JOIN = 50,                      /* JOIN  */
  YYSYMBOL_INNER = 51,                     /* INNER  */
  YYSYMBOL_ON = 52,                        /* ON  */
  YYSYMBOL_NOT = 53,                       /* NOT  */
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
  YYSYMBOL_ID = 75,                        /* ID  */
  YYSYMBOL_SSS = 76,                       /* SSS  */
  YYSYMBOL_77_ = 77,                       /* '+'  */
  YYSYMBOL_78_ = 78,                       /* '-'  */
  YYSYMBOL_79_ = 79,                       /* '*'  */
  YYSYMBOL_80_ = 80,                       /* '/'  */
  YYSYMBOL_UMINUS = 81,                    /* UMINUS  */
  YYSYMBOL_YYACCEPT = 82,                  /* $accept  */
  YYSYMBOL_commands = 83,                  /* commands  */
  YYSYMBOL_command_wrapper = 84,           /* command_wrapper  */
  YYSYMBOL_exit_stmt = 85,                 /* exit_stmt  */
  YYSYMBOL_help_stmt = 86,                 /* help_stmt  */
  YYSYMBOL_sync_stmt = 87,                 /* sync_stmt  */
  YYSYMBOL_begin_stmt = 88,                /* begin_stmt  */
  YYSYMBOL_commit_stmt = 89,               /* commit_stmt  */
  YYSYMBOL_rollback_stmt = 90,             /* rollback_stmt  */
  YYSYMBOL_drop_table_stmt = 91,           /* drop_table_stmt  */
  YYSYMBOL_analyze_table_stmt = 92,        /* analyze_table_stmt  */
  YYSYMBOL_show_tables_stmt = 93,          /* show_tables_stmt  */
  YYSYMBOL_desc_table_stmt = 94,           /* desc_table_stmt  */
  YYSYMBOL_create_index_stmt = 95,         /* create_index_stmt  */
  YYSYMBOL_drop_index_stmt = 96,           /* drop_index_stmt  */
  YYSYMBOL_create_table_stmt = 97,         /* create_table_stmt  */
  YYSYMBOL_attr_def_list = 98,             /* attr_def_list  */
  YYSYMBOL_attr_def = 99,                  /* attr_def  */
  YYSYMBOL_number = 100,                   /* number  */
  YYSYMBOL_float_number = 101,             /* float_number  */
  YYSYMBOL_type = 102,                     /* type  */
  YYSYMBOL_primary_key = 103,              /* primary_key  */
  YYSYMBOL_attr_list = 104,                /* attr_list  */
  YYSYMBOL_insert_stmt = 105,              /* insert_stmt  */
  YYSYMBOL_insert_value_list = 106,        /* insert_value_list  */
  YYSYMBOL_insert_value = 107,             /* insert_value  */
  YYSYMBOL_value = 108,                    /* value  */
  YYSYMBOL_storage_format = 109,           /* storage_format  */
  YYSYMBOL_delete_stmt = 110,              /* delete_stmt  */
  YYSYMBOL_update_stmt = 111,              /* update_stmt  */
  YYSYMBOL_select_stmt = 112,              /* select_stmt  */
  YYSYMBOL_calc_stmt = 113,                /* calc_stmt  */
  YYSYMBOL_expression_list = 114,          /* expression_list  */
  YYSYMBOL_projection_list = 115,          /* projection_list  */
  YYSYMBOL_opt_as_alias = 116,             /* opt_as_alias  */
  YYSYMBOL_expression = 117,               /* expression  */
  YYSYMBOL_rel_attr = 118,                 /* rel_attr  */
  YYSYMBOL_relation = 119,                 /* relation  */
  YYSYMBOL_rel_list = 120,                 /* rel_list  */
  YYSYMBOL_where = 121,                    /* where  */
  YYSYMBOL_condition_list = 122,           /* condition_list  */
  YYSYMBOL_condition = 123,                /* condition  */
  YYSYMBOL_comp_op = 124,                  /* comp_op  */
  YYSYMBOL_group_by = 125,                 /* group_by  */
  YYSYMBOL_having = 126,                   /* having  */
  YYSYMBOL_load_data_stmt = 127,           /* load_data_stmt  */
  YYSYMBOL_explain_stmt = 128,             /* explain_stmt  */
  YYSYMBOL_set_variable_stmt = 129,        /* set_variable_stmt  */
  YYSYMBOL_vector_literal = 130,           /* vector_literal  */
  YYSYMBOL_number_list = 131,              /* number_list  */
  YYSYMBOL_order_by = 132,                 /* order_by  */
  YYSYMBOL_order_by_item_list = 133,       /* order_by_item_list  */
  YYSYMBOL_order_by_item = 134,            /* order_by_item  */
  YYSYMBOL_count_expr = 135,               /* count_expr  */
  YYSYMBOL_aggregate_expr = 136,           /* aggregate_expr  */
  YYSYMBOL_join_list = 137,                /* join_list  */
  YYSYMBOL_join_item = 138,                /* join_item  */
  YYSYMBOL_sub_query = 139,                /* sub_query  */
  YYSYMBOL_opt_semicolon = 140             /* opt_semicolon  */
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
#define YYFINAL  79
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   313

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  82
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  59
/* YYNRULES -- Number of rules.  */
#define YYNRULES  148
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  288

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   332


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
       2,     2,    79,    77,     2,    78,     2,    80,     2,     2,
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
      75,    76,    81
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   244,   244,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   276,   282,   287,   293,   299,   305,
     311,   317,   324,   330,   337,   349,   364,   372,   400,   403,
     416,   428,   442,   446,   447,   450,   451,   452,   453,   454,
     458,   461,   468,   472,   484,   500,   503,   528,   529,   533,
     540,   544,   548,   552,   606,   609,   616,   627,   642,   682,
     697,   706,   711,   723,   731,   746,   747,   748,   752,   755,
     758,   761,   764,   779,   782,   788,   793,   802,   808,   811,
     817,   821,   829,   833,   838,   845,   850,   865,   868,   874,
     877,   882,   889,   898,   904,   913,   914,   915,   916,   917,
     918,   919,   920,   921,   922,   928,   931,   939,   942,   948,
     960,   968,   978,  1016,  1020,  1028,  1031,  1038,  1044,  1053,
    1059,  1065,  1074,  1087,  1090,  1094,  1098,  1102,  1110,  1113,
    1126,  1136,  1147,  1158,  1168,  1179,  1193,  1199,  1200
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
  "INT_T", "STRING_T", "FLOAT_T", "FLOAT_LITERAL", "DATE_T", "VECTOR_T",
  "HELP", "EXIT", "DOT", "INTO", "VALUES", "FROM", "WHERE", "AND", "JOIN",
  "INNER", "ON", "NOT", "LIKE", "SET", "LOAD", "DATA", "INFILE", "EXPLAIN",
  "STORAGE", "FORMAT", "PRIMARY", "KEY", "ANALYZE", "IN", "EXISTS",
  "UNIQUE", "EQ", "LT", "GT", "LE", "GE", "NE", "NUMBER", "ID", "SSS",
  "'+'", "'-'", "'*'", "'/'", "UMINUS", "$accept", "commands",
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

#define YYPACT_NINF (-211)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     215,    -1,    58,   153,   153,   -61,     1,  -211,   -15,   -10,
     -35,  -211,  -211,  -211,  -211,  -211,   -28,    15,   215,    59,
      83,    86,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,    21,    28,    76,    38,    39,    88,    93,
      94,    95,    97,   153,   -14,  -211,  -211,   -13,  -211,   153,
    -211,  -211,  -211,   -26,  -211,  -211,  -211,  -211,    20,    27,
    -211,  -211,    61,    62,    78,    70,    82,  -211,    67,  -211,
    -211,  -211,   118,    98,    72,  -211,   100,   153,   153,   153,
     153,   153,   119,  -211,  -211,  -211,    -9,   115,    80,  -211,
     153,   153,   153,   153,   153,   149,    89,  -211,   104,  -211,
     141,   131,   140,   107,   -19,   121,  -211,   128,   129,   143,
     142,    13,    40,   122,   130,   134,  -211,  -211,   -14,  -211,
     167,  -211,  -211,   -36,   -36,  -211,  -211,   153,    -2,   173,
    -211,  -211,   153,   187,    96,  -211,   148,  -211,   174,   120,
    -211,   190,   147,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,     2,   191,  -211,   150,  -211,    89,   136,  -211,   102,
     158,   198,   212,  -211,   181,   -19,   216,  -211,  -211,  -211,
    -211,  -211,   206,   211,   169,   217,  -211,  -211,   153,  -211,
    -211,   176,   202,   246,  -211,   -12,   222,  -211,   227,   235,
     -37,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,   153,
      96,   140,   184,   186,   -49,   232,   230,   234,   169,  -211,
      -7,   189,   263,   260,  -211,  -211,   102,   240,   235,  -211,
     243,  -211,  -211,   -44,  -211,  -211,  -211,  -211,   244,   213,
    -211,   218,   169,  -211,   257,   200,    96,   236,    -4,   153,
     153,   278,   222,  -211,   264,  -211,  -211,   266,   237,  -211,
    -211,  -211,   245,  -211,    96,   220,    96,   247,  -211,   -44,
    -211,  -211,  -211,   169,   228,    96,  -211,   248,  -211,    96,
     272,   229,  -211,    96,  -211,  -211,  -211,  -211
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,    26,     0,     0,
       0,    27,    28,    29,    25,    24,     0,     0,     0,     0,
       0,   147,    23,    22,    15,    16,    17,    18,     9,    10,
      11,    12,    13,    14,     8,     5,     7,     6,     4,     3,
      19,    20,    21,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    61,    60,    90,    63,     0,
      88,    84,    70,    71,    87,    62,   133,    89,   125,    75,
      33,    32,     0,     0,     0,     0,     0,   120,     0,     1,
     148,     2,     0,     0,     0,    30,     0,     0,     0,     0,
       0,     0,     0,    44,    43,   123,     0,     0,     0,    83,
       0,     0,     0,     0,     0,     0,     0,    69,     0,    76,
      73,     0,    97,     0,     0,     0,    31,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    82,   122,     0,    85,
       0,    91,    72,    78,    79,    80,    81,     0,    92,    95,
     138,    77,     0,     0,    99,    66,     0,   121,     0,     0,
      38,     0,     0,    36,   132,   134,   135,   136,   137,   124,
      86,   129,   126,   127,     0,    93,     0,    97,    74,     0,
       0,     0,     0,    98,   100,     0,     0,    45,    46,    47,
      48,    49,    41,    50,     0,     0,   130,   131,     0,    94,
      96,     0,     0,   115,   139,     0,    55,    57,     0,     0,
       0,   111,   113,   105,   106,   107,   108,   109,   110,     0,
      99,    97,     0,     0,     0,     0,    52,     0,     0,   128,
       0,     0,     0,   117,    59,    58,     0,     0,     0,   146,
       0,   112,   114,   102,   101,    67,   119,    42,     0,     0,
      39,    64,     0,    34,     0,     0,    99,     0,     0,     0,
       0,   125,    55,    54,     0,   103,    40,     0,     0,    37,
      53,    35,     0,   143,    99,     0,    99,     0,   116,   118,
      68,    56,   104,     0,     0,    99,   144,     0,   140,    99,
       0,     0,   145,    99,   141,    51,    65,   142
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -211,  -211,   284,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,  -211,  -211,  -211,    91,  -211,   175,
    -211,  -211,  -210,  -211,    54,    81,  -111,  -211,  -211,  -211,
    -189,  -211,   -51,   166,  -211,    -3,  -211,  -211,   144,  -146,
    -205,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
      60,  -211,   124,  -211,  -211,  -211,  -211,    85,  -211
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,   183,   150,   238,    95,
     182,   215,   217,    35,   227,   196,    61,   259,    36,    37,
      38,    39,    62,    68,   110,   172,    64,   139,   140,   145,
     173,   174,   209,   223,   251,    40,    41,    42,    65,    96,
     107,   162,   163,    66,    67,   167,   194,   230,    81
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      63,    69,    92,   147,   245,   234,   100,   265,   244,   164,
     229,    54,   186,   239,    70,    97,    43,   231,    44,    71,
      55,   193,   127,   128,   187,    93,   149,   224,   232,   105,
      72,    98,   260,   101,   102,   103,   104,    73,   108,   229,
      74,   263,   154,   103,   104,   246,   130,    75,   266,   132,
      63,   101,   102,   103,   104,    56,    99,    58,   197,   276,
      94,   278,   225,   280,   211,   235,    45,   106,   247,   155,
     282,   267,    76,   165,   284,    46,    78,    47,   287,   101,
     102,   103,   104,    79,   121,   122,   123,   124,   125,    80,
     101,   102,   103,   104,    63,    84,    82,    63,   133,   134,
     135,   136,   109,    83,   101,   102,   103,   104,    48,    49,
      50,    51,    52,    85,    86,   197,    87,   101,   102,   103,
     104,    88,    89,    90,    53,    91,    54,    48,    49,    50,
      51,    52,    54,   113,   161,    55,   111,   112,   114,    69,
     115,    55,   116,    53,   129,    54,   117,   119,   126,   170,
     118,   156,   120,   137,    55,   131,   177,   178,   179,   157,
     180,   181,   171,   158,   138,    48,    49,    50,    51,    52,
      56,    57,    58,   142,    59,    60,    56,   143,    58,   141,
     195,    53,   146,    54,   144,   161,   191,   192,   144,    56,
      57,    58,    55,    59,    60,   152,   160,   148,   268,   101,
     102,   103,   104,   149,   151,   166,   233,   101,   102,   103,
     104,   101,   102,   103,   104,   169,   175,   153,   184,   176,
       1,     2,   185,   188,   198,   189,   199,    56,    57,    58,
     210,    59,    60,   212,   213,     3,     4,     5,     6,     7,
       8,     9,    10,   214,   216,   218,    63,   269,    11,    12,
      13,   220,   221,   222,   226,   228,     4,    14,    15,   236,
     237,   241,   242,   243,   248,   200,   201,   249,   250,   253,
      16,    17,   255,   256,    18,   262,   257,   202,   258,    19,
     203,   204,   205,   206,   207,   208,   261,   105,   264,   101,
     102,   103,   104,   272,   273,   277,   281,   275,   274,   279,
     283,   285,    77,   159,   286,   240,   271,   252,   168,     0,
     190,   270,   219,   254
};

static const yytype_int16 yycheck[] =
{
       3,     4,    53,   114,    11,   210,    32,    11,   218,    11,
     199,    30,    10,    62,    75,    28,    17,    54,    19,    18,
      39,   167,    31,    32,    22,    39,    75,    39,    65,     9,
      45,    44,   242,    77,    78,    79,    80,    47,    11,   228,
      75,   246,    29,    79,    80,    52,    97,    75,    52,   100,
      53,    77,    78,    79,    80,    74,    59,    76,   169,   264,
      74,   266,    74,   273,   175,   211,    67,    47,    75,    29,
     275,    75,    57,    75,   279,    17,    17,    19,   283,    77,
      78,    79,    80,     0,    87,    88,    89,    90,    91,     3,
      77,    78,    79,    80,    97,    19,    75,   100,   101,   102,
     103,   104,    75,    75,    77,    78,    79,    80,    12,    13,
      14,    15,    16,    75,    75,   226,    28,    77,    78,    79,
      80,    28,    28,    28,    28,    28,    30,    12,    13,    14,
      15,    16,    30,    55,   137,    39,    75,    75,    68,   142,
      58,    39,    75,    28,    29,    30,    28,    75,    29,    53,
      52,    29,    52,     4,    39,    75,    36,    37,    38,    29,
      40,    41,    66,    29,    75,    12,    13,    14,    15,    16,
      74,    75,    76,    32,    78,    79,    74,    46,    76,    75,
      78,    28,    75,    30,    48,   188,    50,    51,    48,    74,
      75,    76,    39,    78,    79,    52,    29,    76,   249,    77,
      78,    79,    80,    75,    75,    32,   209,    77,    78,    79,
      80,    77,    78,    79,    80,    28,    68,    75,    28,    45,
       5,     6,    75,    32,    66,    75,    28,    74,    75,    76,
      49,    78,    79,    17,    28,    20,    21,    22,    23,    24,
      25,    26,    27,    32,    75,    28,   249,   250,    33,    34,
      35,    75,    50,     7,    32,    28,    21,    42,    43,    75,
      74,    29,    32,    29,    75,    53,    54,     4,     8,    29,
      55,    56,    29,    29,    59,    75,    63,    65,    60,    64,
      68,    69,    70,    71,    72,    73,    29,     9,    52,    77,
      78,    79,    80,    29,    28,    75,    68,    52,    61,    52,
      52,    29,    18,   128,    75,   214,   252,   226,   142,    -1,
     166,   251,   188,   228
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     5,     6,    20,    21,    22,    23,    24,    25,    26,
      27,    33,    34,    35,    42,    43,    55,    56,    59,    64,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,   105,   110,   111,   112,   113,
     127,   128,   129,    17,    19,    67,    17,    19,    12,    13,
      14,    15,    16,    28,    30,    39,    74,    75,    76,    78,
      79,   108,   114,   117,   118,   130,   135,   136,   115,   117,
      75,    18,    45,    47,    75,    75,    57,    84,    17,     0,
       3,   140,    75,    75,    19,    75,    75,    28,    28,    28,
      28,    28,   114,    39,    74,   101,   131,    28,    44,   117,
      32,    77,    78,    79,    80,     9,    47,   132,    11,    75,
     116,    75,    75,    55,    68,    58,    75,    28,    52,    75,
      52,   117,   117,   117,   117,   117,    29,    31,    32,    29,
     114,    75,   114,   117,   117,   117,   117,     4,    75,   119,
     120,    75,    32,    46,    48,   121,    75,   108,    76,    75,
      99,    75,    52,    75,    29,    29,    29,    29,    29,   101,
      29,   117,   133,   134,    11,    75,    32,   137,   115,    28,
      53,    66,   117,   122,   123,    68,    45,    36,    37,    38,
      40,    41,   102,    98,    28,    75,    10,    22,    32,    75,
     120,    50,    51,   121,   138,    78,   107,   108,    66,    28,
      53,    54,    65,    68,    69,    70,    71,    72,    73,   124,
      49,   108,    17,    28,    32,   103,    75,   104,    28,   134,
      75,    50,     7,   125,    39,    74,    32,   106,    28,   112,
     139,    54,    65,   117,   122,   121,    75,    74,   100,    62,
      99,    29,    32,    29,   104,    11,    52,    75,    75,     4,
       8,   126,   107,    29,   139,    29,    29,    63,    60,   109,
     104,    29,    75,   122,    52,    11,    52,    75,   114,   117,
     132,   106,    29,    28,    61,    52,   122,    75,   122,    52,
     104,    68,   122,    52,   122,    29,    75,   122
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    82,    83,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    95,    96,    97,    98,    98,
      99,    99,   100,   101,   101,   102,   102,   102,   102,   102,
     103,   103,   104,   104,   105,   106,   106,   107,   107,   107,
     108,   108,   108,   108,   109,   109,   110,   111,   112,   112,
     113,   114,   114,   115,   115,   116,   116,   116,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     118,   118,   119,   119,   119,   120,   120,   121,   121,   122,
     122,   122,   123,   123,   123,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   125,   125,   126,   126,   127,
     128,   129,   130,   131,   131,   132,   132,   133,   133,   134,
     134,   134,   135,   136,   136,   136,   136,   136,   137,   137,
     138,   138,   138,   138,   138,   138,   139,   140,   140
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     2,     2,     8,     9,     5,     9,     0,     3,
       5,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     6,     1,     3,     8,     0,     3,     1,     2,     2,
       1,     1,     1,     1,     0,     4,     4,     7,     9,     3,
       2,     1,     3,     2,     4,     0,     1,     2,     3,     3,
       3,     3,     3,     2,     1,     3,     4,     1,     1,     1,
       1,     3,     1,     2,     3,     1,     3,     0,     2,     0,
       1,     3,     3,     4,     5,     1,     1,     1,     1,     1,
       1,     1,     2,     1,     2,     0,     3,     0,     2,     7,
       2,     4,     3,     1,     3,     0,     3,     1,     3,     1,
       2,     2,     4,     1,     4,     4,     4,     4,     0,     2,
       5,     6,     7,     4,     5,     6,     1,     0,     1
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
#line 245 "yacc_sql.y"
  {
    unique_ptr<ParsedSqlNode> sql_node = unique_ptr<ParsedSqlNode>((yyvsp[-1].sql_node));
    sql_result->add_sql_node(std::move(sql_node));
  }
#line 2030 "yacc_sql.tab.c"
    break;

  case 24: /* exit_stmt: EXIT  */
#line 276 "yacc_sql.y"
         {
      (void)yynerrs;  // 这么写为了消除yynerrs未使用的告警。如果你有更好的方法欢迎提PR
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXIT);
    }
#line 2039 "yacc_sql.tab.c"
    break;

  case 25: /* help_stmt: HELP  */
#line 282 "yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_HELP);
    }
#line 2047 "yacc_sql.tab.c"
    break;

  case 26: /* sync_stmt: SYNC  */
#line 287 "yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SYNC);
    }
#line 2055 "yacc_sql.tab.c"
    break;

  case 27: /* begin_stmt: TRX_BEGIN  */
#line 293 "yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_BEGIN);
    }
#line 2063 "yacc_sql.tab.c"
    break;

  case 28: /* commit_stmt: TRX_COMMIT  */
#line 299 "yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_COMMIT);
    }
#line 2071 "yacc_sql.tab.c"
    break;

  case 29: /* rollback_stmt: TRX_ROLLBACK  */
#line 305 "yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ROLLBACK);
    }
#line 2079 "yacc_sql.tab.c"
    break;

  case 30: /* drop_table_stmt: DROP TABLE ID  */
#line 311 "yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_TABLE);
      (yyval.sql_node)->drop_table.relation_name = (yyvsp[0].cstring);
    }
#line 2088 "yacc_sql.tab.c"
    break;

  case 31: /* analyze_table_stmt: ANALYZE TABLE ID  */
#line 317 "yacc_sql.y"
                     {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ANALYZE_TABLE);
      (yyval.sql_node)->analyze_table.relation_name = (yyvsp[0].cstring);
    }
#line 2097 "yacc_sql.tab.c"
    break;

  case 32: /* show_tables_stmt: SHOW TABLES  */
#line 324 "yacc_sql.y"
                {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SHOW_TABLES);
    }
#line 2105 "yacc_sql.tab.c"
    break;

  case 33: /* desc_table_stmt: DESC ID  */
#line 330 "yacc_sql.y"
             {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DESC_TABLE);
      (yyval.sql_node)->desc_table.relation_name = (yyvsp[0].cstring);
    }
#line 2114 "yacc_sql.tab.c"
    break;

  case 34: /* create_index_stmt: CREATE INDEX ID ON ID LBRACE attr_list RBRACE  */
#line 338 "yacc_sql.y"
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
#line 2130 "yacc_sql.tab.c"
    break;

  case 35: /* create_index_stmt: CREATE UNIQUE INDEX ID ON ID LBRACE attr_list RBRACE  */
#line 350 "yacc_sql.y"
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
#line 2146 "yacc_sql.tab.c"
    break;

  case 36: /* drop_index_stmt: DROP INDEX ID ON ID  */
#line 365 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_INDEX);
      (yyval.sql_node)->drop_index.index_name = (yyvsp[-2].cstring);
      (yyval.sql_node)->drop_index.relation_name = (yyvsp[0].cstring);
    }
#line 2156 "yacc_sql.tab.c"
    break;

  case 37: /* create_table_stmt: CREATE TABLE ID LBRACE attr_def attr_def_list primary_key RBRACE storage_format  */
#line 373 "yacc_sql.y"
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
#line 2184 "yacc_sql.tab.c"
    break;

  case 38: /* attr_def_list: %empty  */
#line 400 "yacc_sql.y"
    {
      (yyval.attr_infos) = nullptr;
    }
#line 2192 "yacc_sql.tab.c"
    break;

  case 39: /* attr_def_list: attr_def_list COMMA attr_def  */
#line 404 "yacc_sql.y"
    {
      if ((yyvsp[-2].attr_infos) != nullptr) {
        (yyval.attr_infos) = (yyvsp[-2].attr_infos);
      } else {
        (yyval.attr_infos) = new vector<AttrInfoSqlNode>;
      }
      (yyval.attr_infos)->insert((yyval.attr_infos)->begin(), *(yyvsp[0].attr_info));
      delete (yyvsp[0].attr_info);
    }
#line 2206 "yacc_sql.tab.c"
    break;

  case 40: /* attr_def: ID type LBRACE number RBRACE  */
#line 417 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-3].number);
      (yyval.attr_info)->name = (yyvsp[-4].cstring);
      // For VECTOR(n), store bytes for [dimension(int32) + n float32 values]
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int) + ((yyvsp[-1].number) * sizeof(float)));
      } else {
        (yyval.attr_info)->length = (yyvsp[-1].number);
      }
    }
#line 2222 "yacc_sql.tab.c"
    break;

  case 41: /* attr_def: ID type  */
#line 429 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[0].number);
      (yyval.attr_info)->name = (yyvsp[-1].cstring);
      // Default length: INT/FLOAT/DATE/STRING fallback 4; VECTOR default header size (dimension)
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int));
      } else {
        (yyval.attr_info)->length = 4;
      }
    }
#line 2238 "yacc_sql.tab.c"
    break;

  case 42: /* number: NUMBER  */
#line 442 "yacc_sql.y"
           {(yyval.number) = (yyvsp[0].number);}
#line 2244 "yacc_sql.tab.c"
    break;

  case 43: /* float_number: NUMBER  */
#line 446 "yacc_sql.y"
           {(yyval.floats) = (double)(yyvsp[0].number);}
#line 2250 "yacc_sql.tab.c"
    break;

  case 44: /* float_number: FLOAT_LITERAL  */
#line 447 "yacc_sql.y"
                    {(yyval.floats) = (yyvsp[0].floats);}
#line 2256 "yacc_sql.tab.c"
    break;

  case 45: /* type: INT_T  */
#line 450 "yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::INTS); }
#line 2262 "yacc_sql.tab.c"
    break;

  case 46: /* type: STRING_T  */
#line 451 "yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::CHARS); }
#line 2268 "yacc_sql.tab.c"
    break;

  case 47: /* type: FLOAT_T  */
#line 452 "yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::FLOATS); }
#line 2274 "yacc_sql.tab.c"
    break;

  case 48: /* type: DATE_T  */
#line 453 "yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::DATES); }
#line 2280 "yacc_sql.tab.c"
    break;

  case 49: /* type: VECTOR_T  */
#line 454 "yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::VECTORS); }
#line 2286 "yacc_sql.tab.c"
    break;

  case 50: /* primary_key: %empty  */
#line 458 "yacc_sql.y"
    {
      (yyval.key_list) = nullptr;
    }
#line 2294 "yacc_sql.tab.c"
    break;

  case 51: /* primary_key: COMMA PRIMARY KEY LBRACE attr_list RBRACE  */
#line 462 "yacc_sql.y"
    {
      (yyval.key_list) = (yyvsp[-1].key_list);
    }
#line 2302 "yacc_sql.tab.c"
    break;

  case 52: /* attr_list: ID  */
#line 468 "yacc_sql.y"
       {
      (yyval.key_list) = new vector<string>();
      (yyval.key_list)->push_back((yyvsp[0].cstring));
    }
#line 2311 "yacc_sql.tab.c"
    break;

  case 53: /* attr_list: ID COMMA attr_list  */
#line 472 "yacc_sql.y"
                         {
      if ((yyvsp[0].key_list) != nullptr) {
        (yyval.key_list) = (yyvsp[0].key_list);
      } else {
        (yyval.key_list) = new vector<string>;
      }

      (yyval.key_list)->insert((yyval.key_list)->begin(), (yyvsp[-2].cstring));
    }
#line 2325 "yacc_sql.tab.c"
    break;

  case 54: /* insert_stmt: INSERT INTO ID VALUES LBRACE insert_value insert_value_list RBRACE  */
#line 485 "yacc_sql.y"
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
#line 2341 "yacc_sql.tab.c"
    break;

  case 55: /* insert_value_list: %empty  */
#line 500 "yacc_sql.y"
    {
      (yyval.value_list) = nullptr;
    }
#line 2349 "yacc_sql.tab.c"
    break;

  case 56: /* insert_value_list: COMMA insert_value insert_value_list  */
#line 503 "yacc_sql.y"
                                            {
      if ((yyvsp[0].value_list) != nullptr) {
        (yyval.value_list) = (yyvsp[0].value_list);
      } else {
        (yyval.value_list) = new vector<Value>;
      }
      (yyval.value_list)->emplace_back(*(yyvsp[-1].value));
      delete (yyvsp[-1].value);
    }
#line 2363 "yacc_sql.tab.c"
    break;

  case 58: /* insert_value: '-' NUMBER  */
#line 529 "yacc_sql.y"
                 {
      (yyval.value) = new Value(-(int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2372 "yacc_sql.tab.c"
    break;

  case 59: /* insert_value: '-' FLOAT_LITERAL  */
#line 533 "yacc_sql.y"
                        {
      (yyval.value) = new Value(-(float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2381 "yacc_sql.tab.c"
    break;

  case 60: /* value: NUMBER  */
#line 540 "yacc_sql.y"
           {
      (yyval.value) = new Value((int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2390 "yacc_sql.tab.c"
    break;

  case 61: /* value: FLOAT_LITERAL  */
#line 544 "yacc_sql.y"
                   {
      (yyval.value) = new Value((float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2399 "yacc_sql.tab.c"
    break;

  case 62: /* value: vector_literal  */
#line 548 "yacc_sql.y"
                    {
      (yyval.value) = (yyvsp[0].value);
      (yyloc) = (yylsp[0]);
    }
#line 2408 "yacc_sql.tab.c"
    break;

  case 63: /* value: SSS  */
#line 552 "yacc_sql.y"
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
#line 2464 "yacc_sql.tab.c"
    break;

  case 64: /* storage_format: %empty  */
#line 606 "yacc_sql.y"
    {
      (yyval.cstring) = nullptr;
    }
#line 2472 "yacc_sql.tab.c"
    break;

  case 65: /* storage_format: STORAGE FORMAT EQ ID  */
#line 610 "yacc_sql.y"
    {
      (yyval.cstring) = (yyvsp[0].cstring);
    }
#line 2480 "yacc_sql.tab.c"
    break;

  case 66: /* delete_stmt: DELETE FROM ID where  */
#line 617 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DELETE);
      (yyval.sql_node)->deletion.relation_name = (yyvsp[-1].cstring);
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.sql_node)->deletion.conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 2493 "yacc_sql.tab.c"
    break;

  case 67: /* update_stmt: UPDATE ID SET ID EQ value where  */
#line 628 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_UPDATE);
      (yyval.sql_node)->update.relation_name = (yyvsp[-5].cstring);
      (yyval.sql_node)->update.attribute_name = (yyvsp[-3].cstring);
      (yyval.sql_node)->update.value = *(yyvsp[-1].value);
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.sql_node)->update.conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
      delete (yyvsp[-1].value);
    }
#line 2509 "yacc_sql.tab.c"
    break;

  case 68: /* select_stmt: SELECT projection_list FROM rel_list join_list where group_by having order_by  */
#line 643 "yacc_sql.y"
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
#line 2553 "yacc_sql.tab.c"
    break;

  case 69: /* select_stmt: SELECT projection_list order_by  */
#line 683 "yacc_sql.y"
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
#line 2570 "yacc_sql.tab.c"
    break;

  case 70: /* calc_stmt: CALC expression_list  */
#line 698 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CALC);
      (yyval.sql_node)->calc.expressions.swap(*(yyvsp[0].expression_list));
      delete (yyvsp[0].expression_list);
    }
#line 2580 "yacc_sql.tab.c"
    break;

  case 71: /* expression_list: expression  */
#line 707 "yacc_sql.y"
    {
      (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      (yyval.expression_list)->emplace_back((yyvsp[0].expression));
    }
#line 2589 "yacc_sql.tab.c"
    break;

  case 72: /* expression_list: expression COMMA expression_list  */
#line 712 "yacc_sql.y"
    {
      if ((yyvsp[0].expression_list) != nullptr) {
        (yyval.expression_list) = (yyvsp[0].expression_list);
      } else {
        (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      }
      (yyval.expression_list)->emplace((yyval.expression_list)->begin(), (yyvsp[-2].expression));
    }
#line 2602 "yacc_sql.tab.c"
    break;

  case 73: /* projection_list: expression opt_as_alias  */
#line 724 "yacc_sql.y"
    {
      if ((yyvsp[0].cstring) != nullptr) {
        (yyvsp[-1].expression)->set_name((yyvsp[0].cstring));
      }
      (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      (yyval.expression_list)->emplace_back((yyvsp[-1].expression));
    }
#line 2614 "yacc_sql.tab.c"
    break;

  case 74: /* projection_list: expression opt_as_alias COMMA projection_list  */
#line 732 "yacc_sql.y"
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
#line 2630 "yacc_sql.tab.c"
    break;

  case 75: /* opt_as_alias: %empty  */
#line 746 "yacc_sql.y"
                { (yyval.cstring) = nullptr; }
#line 2636 "yacc_sql.tab.c"
    break;

  case 76: /* opt_as_alias: ID  */
#line 747 "yacc_sql.y"
         { (yyval.cstring) = (yyvsp[0].cstring); }
#line 2642 "yacc_sql.tab.c"
    break;

  case 77: /* opt_as_alias: AS ID  */
#line 748 "yacc_sql.y"
            { (yyval.cstring) = (yyvsp[0].cstring); }
#line 2648 "yacc_sql.tab.c"
    break;

  case 78: /* expression: expression '+' expression  */
#line 752 "yacc_sql.y"
                              {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::ADD, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2656 "yacc_sql.tab.c"
    break;

  case 79: /* expression: expression '-' expression  */
#line 755 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::SUB, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2664 "yacc_sql.tab.c"
    break;

  case 80: /* expression: expression '*' expression  */
#line 758 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::MUL, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2672 "yacc_sql.tab.c"
    break;

  case 81: /* expression: expression '/' expression  */
#line 761 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::DIV, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2680 "yacc_sql.tab.c"
    break;

  case 82: /* expression: LBRACE expression_list RBRACE  */
#line 764 "yacc_sql.y"
                                    {
      if ((yyvsp[-1].expression_list) != nullptr && (yyvsp[-1].expression_list)->size() > 1) {
        (yyval.expression) = new ListExpr(std::move(*(yyvsp[-1].expression_list)));
        delete (yyvsp[-1].expression_list);
      } else if ((yyvsp[-1].expression_list) != nullptr && (yyvsp[-1].expression_list)->size() == 1) {
        (yyval.expression) = (yyvsp[-1].expression_list)->front().release();
        delete (yyvsp[-1].expression_list);
      } else {
        // Empty list case
        std::vector<std::unique_ptr<Expression>> empty_exprs;
        (yyval.expression) = new ListExpr(std::move(empty_exprs));
        delete (yyvsp[-1].expression_list);
      }
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2700 "yacc_sql.tab.c"
    break;

  case 83: /* expression: '-' expression  */
#line 779 "yacc_sql.y"
                                  {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::NEGATIVE, (yyvsp[0].expression), nullptr, sql_string, &(yyloc));
    }
#line 2708 "yacc_sql.tab.c"
    break;

  case 84: /* expression: value  */
#line 782 "yacc_sql.y"
            {
      (yyval.expression) = new ValueExpr(*(yyvsp[0].value));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete (yyvsp[0].value);
    }
#line 2718 "yacc_sql.tab.c"
    break;

  case 85: /* expression: ID LBRACE RBRACE  */
#line 788 "yacc_sql.y"
                       {
      std::vector<std::unique_ptr<Expression>> args;
      (yyval.expression) = new FunctionExpr((yyvsp[-2].cstring), std::move(args));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2728 "yacc_sql.tab.c"
    break;

  case 86: /* expression: ID LBRACE expression_list RBRACE  */
#line 793 "yacc_sql.y"
                                       {
      std::vector<std::unique_ptr<Expression>> args;
      if ((yyvsp[-1].expression_list) != nullptr) {
        args.swap(*(yyvsp[-1].expression_list));
        delete (yyvsp[-1].expression_list);
      }
      (yyval.expression) = new FunctionExpr((yyvsp[-3].cstring), std::move(args));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2742 "yacc_sql.tab.c"
    break;

  case 87: /* expression: rel_attr  */
#line 802 "yacc_sql.y"
               {
      RelAttrSqlNode *node = (yyvsp[0].rel_attr);
      (yyval.expression) = new UnboundFieldExpr(node->relation_name, node->attribute_name);
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete (yyvsp[0].rel_attr);
    }
#line 2753 "yacc_sql.tab.c"
    break;

  case 88: /* expression: '*'  */
#line 808 "yacc_sql.y"
          {
      (yyval.expression) = new StarExpr();
    }
#line 2761 "yacc_sql.tab.c"
    break;

  case 89: /* expression: aggregate_expr  */
#line 811 "yacc_sql.y"
                     {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 2769 "yacc_sql.tab.c"
    break;

  case 90: /* rel_attr: ID  */
#line 817 "yacc_sql.y"
       {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->attribute_name = (yyvsp[0].cstring);
    }
#line 2778 "yacc_sql.tab.c"
    break;

  case 91: /* rel_attr: ID DOT ID  */
#line 821 "yacc_sql.y"
                {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->relation_name  = (yyvsp[-2].cstring);
      (yyval.rel_attr)->attribute_name = (yyvsp[0].cstring);
    }
#line 2788 "yacc_sql.tab.c"
    break;

  case 92: /* relation: ID  */
#line 829 "yacc_sql.y"
       {
      (yyval.relation_with_alias) = new RelationWithAliasSqlNode();
      (yyval.relation_with_alias)->table_name = (yyvsp[0].cstring);
    }
#line 2797 "yacc_sql.tab.c"
    break;

  case 93: /* relation: ID ID  */
#line 833 "yacc_sql.y"
            {
      (yyval.relation_with_alias) = new RelationWithAliasSqlNode();
      (yyval.relation_with_alias)->table_name = (yyvsp[-1].cstring);
      (yyval.relation_with_alias)->alias = (yyvsp[0].cstring);
    }
#line 2807 "yacc_sql.tab.c"
    break;

  case 94: /* relation: ID AS ID  */
#line 838 "yacc_sql.y"
               {
      (yyval.relation_with_alias) = new RelationWithAliasSqlNode();
      (yyval.relation_with_alias)->table_name = (yyvsp[-2].cstring);
      (yyval.relation_with_alias)->alias = (yyvsp[0].cstring);
    }
#line 2817 "yacc_sql.tab.c"
    break;

  case 95: /* rel_list: relation  */
#line 845 "yacc_sql.y"
             {
      (yyval.relation_with_alias_list) = new vector<RelationWithAliasSqlNode>();
      (yyval.relation_with_alias_list)->emplace_back(std::move(*(yyvsp[0].relation_with_alias)));
      delete (yyvsp[0].relation_with_alias);
    }
#line 2827 "yacc_sql.tab.c"
    break;

  case 96: /* rel_list: relation COMMA rel_list  */
#line 850 "yacc_sql.y"
                              {
      if ((yyvsp[0].relation_with_alias_list) != nullptr) {
        (yyval.relation_with_alias_list) = (yyvsp[0].relation_with_alias_list);
      } else {
        (yyval.relation_with_alias_list) = new vector<RelationWithAliasSqlNode>();
      }
      (yyval.relation_with_alias_list)->emplace((yyval.relation_with_alias_list)->begin(), std::move(*(yyvsp[-2].relation_with_alias)));
      delete (yyvsp[-2].relation_with_alias);
    }
#line 2841 "yacc_sql.tab.c"
    break;

  case 97: /* where: %empty  */
#line 865 "yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 2849 "yacc_sql.tab.c"
    break;

  case 98: /* where: WHERE condition_list  */
#line 868 "yacc_sql.y"
                           {
      (yyval.condition_list) = (yyvsp[0].condition_list);
    }
#line 2857 "yacc_sql.tab.c"
    break;

  case 99: /* condition_list: %empty  */
#line 874 "yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 2865 "yacc_sql.tab.c"
    break;

  case 100: /* condition_list: condition  */
#line 877 "yacc_sql.y"
                {
      (yyval.condition_list) = new vector<ConditionSqlNode>;
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[0].condition)));
      delete (yyvsp[0].condition);
    }
#line 2875 "yacc_sql.tab.c"
    break;

  case 101: /* condition_list: condition AND condition_list  */
#line 882 "yacc_sql.y"
                                   {
      (yyval.condition_list) = (yyvsp[0].condition_list);
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[-2].condition)));
      delete (yyvsp[-2].condition);
    }
#line 2885 "yacc_sql.tab.c"
    break;

  case 102: /* condition: expression comp_op expression  */
#line 890 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr.reset((yyvsp[-2].expression));
      (yyval.condition)->right_expr.reset((yyvsp[0].expression));
      (yyval.condition)->comp = (yyvsp[-1].comp);
    }
#line 2896 "yacc_sql.tab.c"
    break;

  case 103: /* condition: EXISTS LBRACE sub_query RBRACE  */
#line 899 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->sub_query.reset((yyvsp[-1].sql_node));
      (yyval.condition)->comp = EXISTS_OP;
    }
#line 2906 "yacc_sql.tab.c"
    break;

  case 104: /* condition: NOT EXISTS LBRACE sub_query RBRACE  */
#line 905 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->sub_query.reset((yyvsp[-1].sql_node));
      (yyval.condition)->comp = NOT_EXISTS_OP;
    }
#line 2916 "yacc_sql.tab.c"
    break;

  case 105: /* comp_op: EQ  */
#line 913 "yacc_sql.y"
         { (yyval.comp) = EQUAL_TO; }
#line 2922 "yacc_sql.tab.c"
    break;

  case 106: /* comp_op: LT  */
#line 914 "yacc_sql.y"
         { (yyval.comp) = LESS_THAN; }
#line 2928 "yacc_sql.tab.c"
    break;

  case 107: /* comp_op: GT  */
#line 915 "yacc_sql.y"
         { (yyval.comp) = GREAT_THAN; }
#line 2934 "yacc_sql.tab.c"
    break;

  case 108: /* comp_op: LE  */
#line 916 "yacc_sql.y"
         { (yyval.comp) = LESS_EQUAL; }
#line 2940 "yacc_sql.tab.c"
    break;

  case 109: /* comp_op: GE  */
#line 917 "yacc_sql.y"
         { (yyval.comp) = GREAT_EQUAL; }
#line 2946 "yacc_sql.tab.c"
    break;

  case 110: /* comp_op: NE  */
#line 918 "yacc_sql.y"
         { (yyval.comp) = NOT_EQUAL; }
#line 2952 "yacc_sql.tab.c"
    break;

  case 111: /* comp_op: LIKE  */
#line 919 "yacc_sql.y"
           { (yyval.comp) = LIKE_OP; }
#line 2958 "yacc_sql.tab.c"
    break;

  case 112: /* comp_op: NOT LIKE  */
#line 920 "yacc_sql.y"
               { (yyval.comp) = NOT_LIKE_OP; }
#line 2964 "yacc_sql.tab.c"
    break;

  case 113: /* comp_op: IN  */
#line 921 "yacc_sql.y"
         { (yyval.comp) = IN_OP; }
#line 2970 "yacc_sql.tab.c"
    break;

  case 114: /* comp_op: NOT IN  */
#line 922 "yacc_sql.y"
             { (yyval.comp) = NOT_IN_OP; }
#line 2976 "yacc_sql.tab.c"
    break;

  case 115: /* group_by: %empty  */
#line 928 "yacc_sql.y"
    {
      (yyval.expression_list) = nullptr;
    }
#line 2984 "yacc_sql.tab.c"
    break;

  case 116: /* group_by: GROUP BY expression_list  */
#line 932 "yacc_sql.y"
    {
      (yyval.expression_list) = (yyvsp[0].expression_list);
    }
#line 2992 "yacc_sql.tab.c"
    break;

  case 117: /* having: %empty  */
#line 939 "yacc_sql.y"
    {
      (yyval.expression) = nullptr;
    }
#line 3000 "yacc_sql.tab.c"
    break;

  case 118: /* having: HAVING expression  */
#line 943 "yacc_sql.y"
    {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 3008 "yacc_sql.tab.c"
    break;

  case 119: /* load_data_stmt: LOAD DATA INFILE SSS INTO TABLE ID  */
#line 949 "yacc_sql.y"
    {
      char *tmp_file_name = common::substr((yyvsp[-3].cstring), 1, strlen((yyvsp[-3].cstring)) - 2);

      (yyval.sql_node) = new ParsedSqlNode(SCF_LOAD_DATA);
      (yyval.sql_node)->load_data.relation_name = (yyvsp[0].cstring);
      (yyval.sql_node)->load_data.file_name = tmp_file_name;
      free(tmp_file_name);
    }
#line 3021 "yacc_sql.tab.c"
    break;

  case 120: /* explain_stmt: EXPLAIN command_wrapper  */
#line 961 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXPLAIN);
      (yyval.sql_node)->explain.sql_node = unique_ptr<ParsedSqlNode>((yyvsp[0].sql_node));
    }
#line 3030 "yacc_sql.tab.c"
    break;

  case 121: /* set_variable_stmt: SET ID EQ value  */
#line 969 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SET_VARIABLE);
      (yyval.sql_node)->set_variable.name  = (yyvsp[-2].cstring);
      (yyval.sql_node)->set_variable.value = *(yyvsp[0].value);
      delete (yyvsp[0].value);
    }
#line 3041 "yacc_sql.tab.c"
    break;

  case 122: /* vector_literal: LSQUARE number_list RSQUARE  */
#line 978 "yacc_sql.y"
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
#line 3081 "yacc_sql.tab.c"
    break;

  case 123: /* number_list: float_number  */
#line 1016 "yacc_sql.y"
                 {
      (yyval.float_list) = new std::vector<double>();
      (yyval.float_list)->push_back((yyvsp[0].floats));
    }
#line 3090 "yacc_sql.tab.c"
    break;

  case 124: /* number_list: number_list COMMA float_number  */
#line 1020 "yacc_sql.y"
                                     {
      (yyval.float_list) = (yyvsp[-2].float_list);
      (yyval.float_list)->push_back((yyvsp[0].floats));
    }
#line 3099 "yacc_sql.tab.c"
    break;

  case 125: /* order_by: %empty  */
#line 1028 "yacc_sql.y"
    {
      (yyval.order_by_list) = nullptr;
    }
#line 3107 "yacc_sql.tab.c"
    break;

  case 126: /* order_by: ORDER BY order_by_item_list  */
#line 1032 "yacc_sql.y"
    {
      (yyval.order_by_list) = (yyvsp[0].order_by_list);
    }
#line 3115 "yacc_sql.tab.c"
    break;

  case 127: /* order_by_item_list: order_by_item  */
#line 1039 "yacc_sql.y"
    {
      (yyval.order_by_list) = new std::vector<OrderBySqlNode>();
      (yyval.order_by_list)->emplace_back(std::move(*(yyvsp[0].order_by_item)));
      delete (yyvsp[0].order_by_item);
    }
#line 3125 "yacc_sql.tab.c"
    break;

  case 128: /* order_by_item_list: order_by_item_list COMMA order_by_item  */
#line 1045 "yacc_sql.y"
    {
      (yyval.order_by_list) = (yyvsp[-2].order_by_list);
      (yyval.order_by_list)->emplace_back(std::move(*(yyvsp[0].order_by_item)));
      delete (yyvsp[0].order_by_item);
    }
#line 3135 "yacc_sql.tab.c"
    break;

  case 129: /* order_by_item: expression  */
#line 1054 "yacc_sql.y"
    {
      (yyval.order_by_item) = new OrderBySqlNode();
      (yyval.order_by_item)->expression = unique_ptr<Expression>((yyvsp[0].expression));
      (yyval.order_by_item)->direction = ASC_ORDER;  // 默认升序
    }
#line 3145 "yacc_sql.tab.c"
    break;

  case 130: /* order_by_item: expression ASC  */
#line 1060 "yacc_sql.y"
    {
      (yyval.order_by_item) = new OrderBySqlNode();
      (yyval.order_by_item)->expression = unique_ptr<Expression>((yyvsp[-1].expression));
      (yyval.order_by_item)->direction = ASC_ORDER;
    }
#line 3155 "yacc_sql.tab.c"
    break;

  case 131: /* order_by_item: expression DESC  */
#line 1066 "yacc_sql.y"
    {
      (yyval.order_by_item) = new OrderBySqlNode();
      (yyval.order_by_item)->expression = unique_ptr<Expression>((yyvsp[-1].expression));
      (yyval.order_by_item)->direction = DESC_ORDER;
    }
#line 3165 "yacc_sql.tab.c"
    break;

  case 132: /* count_expr: COUNT LBRACE expression RBRACE  */
#line 1074 "yacc_sql.y"
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
#line 3180 "yacc_sql.tab.c"
    break;

  case 133: /* aggregate_expr: count_expr  */
#line 1087 "yacc_sql.y"
               {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 3188 "yacc_sql.tab.c"
    break;

  case 134: /* aggregate_expr: SUM LBRACE expression RBRACE  */
#line 1090 "yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("sum", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3197 "yacc_sql.tab.c"
    break;

  case 135: /* aggregate_expr: MAX LBRACE expression RBRACE  */
#line 1094 "yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("max", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3206 "yacc_sql.tab.c"
    break;

  case 136: /* aggregate_expr: MIN LBRACE expression RBRACE  */
#line 1098 "yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("min", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3215 "yacc_sql.tab.c"
    break;

  case 137: /* aggregate_expr: AVG LBRACE expression RBRACE  */
#line 1102 "yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("avg", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3224 "yacc_sql.tab.c"
    break;

  case 138: /* join_list: %empty  */
#line 1110 "yacc_sql.y"
    {
      (yyval.join_list) = nullptr;
    }
#line 3232 "yacc_sql.tab.c"
    break;

  case 139: /* join_list: join_list join_item  */
#line 1114 "yacc_sql.y"
    {
      if ((yyvsp[-1].join_list) != nullptr) {
        (yyval.join_list) = (yyvsp[-1].join_list);
      } else {
        (yyval.join_list) = new std::vector<JoinSqlNode>();
      }
      (yyval.join_list)->emplace_back(std::move(*(yyvsp[0].join_item)));
      delete (yyvsp[0].join_item);
    }
#line 3246 "yacc_sql.tab.c"
    break;

  case 140: /* join_item: INNER JOIN ID ON condition_list  */
#line 1127 "yacc_sql.y"
    {
      (yyval.join_item) = new JoinSqlNode();
      (yyval.join_item)->table_name = (yyvsp[-2].cstring);
      (yyval.join_item)->join_type = INNER_JOIN;
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.join_item)->conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 3260 "yacc_sql.tab.c"
    break;

  case 141: /* join_item: INNER JOIN ID ID ON condition_list  */
#line 1137 "yacc_sql.y"
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
#line 3275 "yacc_sql.tab.c"
    break;

  case 142: /* join_item: INNER JOIN ID AS ID ON condition_list  */
#line 1148 "yacc_sql.y"
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
#line 3290 "yacc_sql.tab.c"
    break;

  case 143: /* join_item: JOIN ID ON condition_list  */
#line 1159 "yacc_sql.y"
    {
      (yyval.join_item) = new JoinSqlNode();
      (yyval.join_item)->table_name = (yyvsp[-2].cstring);
      (yyval.join_item)->join_type = INNER_JOIN;  // 默认为INNER JOIN
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.join_item)->conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 3304 "yacc_sql.tab.c"
    break;

  case 144: /* join_item: JOIN ID ID ON condition_list  */
#line 1169 "yacc_sql.y"
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
#line 3319 "yacc_sql.tab.c"
    break;

  case 145: /* join_item: JOIN ID AS ID ON condition_list  */
#line 1180 "yacc_sql.y"
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
#line 3334 "yacc_sql.tab.c"
    break;

  case 146: /* sub_query: select_stmt  */
#line 1194 "yacc_sql.y"
    {
      (yyval.sql_node) = (yyvsp[0].sql_node);
    }
#line 3342 "yacc_sql.tab.c"
    break;


#line 3346 "yacc_sql.tab.c"

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

#line 1202 "yacc_sql.y"

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
