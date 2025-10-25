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
#line 2 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"


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


#line 126 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"

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

#include "yacc_sql.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_SEMICOLON = 3,                  /* SEMICOLON  */
  YYSYMBOL_BY = 4,                         /* BY  */
  YYSYMBOL_LIMIT = 5,                      /* LIMIT  */
  YYSYMBOL_WITH = 6,                       /* WITH  */
  YYSYMBOL_CREATE = 7,                     /* CREATE  */
  YYSYMBOL_DROP = 8,                       /* DROP  */
  YYSYMBOL_GROUP = 9,                      /* GROUP  */
  YYSYMBOL_HAVING = 10,                    /* HAVING  */
  YYSYMBOL_ORDER = 11,                     /* ORDER  */
  YYSYMBOL_ASC = 12,                       /* ASC  */
  YYSYMBOL_AS = 13,                        /* AS  */
  YYSYMBOL_COUNT = 14,                     /* COUNT  */
  YYSYMBOL_SUM = 15,                       /* SUM  */
  YYSYMBOL_MAX = 16,                       /* MAX  */
  YYSYMBOL_MIN = 17,                       /* MIN  */
  YYSYMBOL_AVG = 18,                       /* AVG  */
  YYSYMBOL_TABLE = 19,                     /* TABLE  */
  YYSYMBOL_TABLES = 20,                    /* TABLES  */
  YYSYMBOL_INDEX = 21,                     /* INDEX  */
  YYSYMBOL_ALTER = 22,                     /* ALTER  */
  YYSYMBOL_ADD = 23,                       /* ADD  */
  YYSYMBOL_CHANGE = 24,                    /* CHANGE  */
  YYSYMBOL_COLUMN = 25,                    /* COLUMN  */
  YYSYMBOL_RENAME = 26,                    /* RENAME  */
  YYSYMBOL_TO = 27,                        /* TO  */
  YYSYMBOL_IF = 28,                        /* IF  */
  YYSYMBOL_EXISTS = 29,                    /* EXISTS  */
  YYSYMBOL_CALC = 30,                      /* CALC  */
  YYSYMBOL_SELECT = 31,                    /* SELECT  */
  YYSYMBOL_UNION = 32,                     /* UNION  */
  YYSYMBOL_UNION_ALL = 33,                 /* UNION_ALL  */
  YYSYMBOL_DESC = 34,                      /* DESC  */
  YYSYMBOL_SHOW = 35,                      /* SHOW  */
  YYSYMBOL_SYNC = 36,                      /* SYNC  */
  YYSYMBOL_INSERT = 37,                    /* INSERT  */
  YYSYMBOL_DELETE = 38,                    /* DELETE  */
  YYSYMBOL_UPDATE = 39,                    /* UPDATE  */
  YYSYMBOL_LBRACE = 40,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 41,                    /* RBRACE  */
  YYSYMBOL_LSQUARE = 42,                   /* LSQUARE  */
  YYSYMBOL_RSQUARE = 43,                   /* RSQUARE  */
  YYSYMBOL_COMMA = 44,                     /* COMMA  */
  YYSYMBOL_TRX_BEGIN = 45,                 /* TRX_BEGIN  */
  YYSYMBOL_TRX_COMMIT = 46,                /* TRX_COMMIT  */
  YYSYMBOL_TRX_ROLLBACK = 47,              /* TRX_ROLLBACK  */
  YYSYMBOL_INT_T = 48,                     /* INT_T  */
  YYSYMBOL_STRING_T = 49,                  /* STRING_T  */
  YYSYMBOL_FLOAT_T = 50,                   /* FLOAT_T  */
  YYSYMBOL_DATE_T = 51,                    /* DATE_T  */
  YYSYMBOL_VECTOR_T = 52,                  /* VECTOR_T  */
  YYSYMBOL_TEXT_T = 53,                    /* TEXT_T  */
  YYSYMBOL_HELP = 54,                      /* HELP  */
  YYSYMBOL_EXIT = 55,                      /* EXIT  */
  YYSYMBOL_DOT = 56,                       /* DOT  */
  YYSYMBOL_INTO = 57,                      /* INTO  */
  YYSYMBOL_VALUES = 58,                    /* VALUES  */
  YYSYMBOL_FROM = 59,                      /* FROM  */
  YYSYMBOL_WHERE = 60,                     /* WHERE  */
  YYSYMBOL_AND = 61,                       /* AND  */
  YYSYMBOL_OR = 62,                        /* OR  */
  YYSYMBOL_JOIN = 63,                      /* JOIN  */
  YYSYMBOL_INNER = 64,                     /* INNER  */
  YYSYMBOL_ON = 65,                        /* ON  */
  YYSYMBOL_NOT = 66,                       /* NOT  */
  YYSYMBOL_NULL_T = 67,                    /* NULL_T  */
  YYSYMBOL_LIKE = 68,                      /* LIKE  */
  YYSYMBOL_SET = 69,                       /* SET  */
  YYSYMBOL_LOAD = 70,                      /* LOAD  */
  YYSYMBOL_DATA = 71,                      /* DATA  */
  YYSYMBOL_INFILE = 72,                    /* INFILE  */
  YYSYMBOL_EXPLAIN = 73,                   /* EXPLAIN  */
  YYSYMBOL_STORAGE = 74,                   /* STORAGE  */
  YYSYMBOL_FORMAT = 75,                    /* FORMAT  */
  YYSYMBOL_PRIMARY = 76,                   /* PRIMARY  */
  YYSYMBOL_KEY = 77,                       /* KEY  */
  YYSYMBOL_ANALYZE = 78,                   /* ANALYZE  */
  YYSYMBOL_IN = 79,                        /* IN  */
  YYSYMBOL_IS = 80,                        /* IS  */
  YYSYMBOL_UNIQUE = 81,                    /* UNIQUE  */
  YYSYMBOL_VIEW = 82,                      /* VIEW  */
  YYSYMBOL_EQ = 83,                        /* EQ  */
  YYSYMBOL_LT = 84,                        /* LT  */
  YYSYMBOL_GT = 85,                        /* GT  */
  YYSYMBOL_LE = 86,                        /* LE  */
  YYSYMBOL_GE = 87,                        /* GE  */
  YYSYMBOL_NE = 88,                        /* NE  */
  YYSYMBOL_NUMBER = 89,                    /* NUMBER  */
  YYSYMBOL_FLOAT_LITERAL = 90,             /* FLOAT_LITERAL  */
  YYSYMBOL_ID = 91,                        /* ID  */
  YYSYMBOL_SSS = 92,                       /* SSS  */
  YYSYMBOL_93_ = 93,                       /* '+'  */
  YYSYMBOL_94_ = 94,                       /* '-'  */
  YYSYMBOL_95_ = 95,                       /* '*'  */
  YYSYMBOL_96_ = 96,                       /* '/'  */
  YYSYMBOL_UMINUS = 97,                    /* UMINUS  */
  YYSYMBOL_YYACCEPT = 98,                  /* $accept  */
  YYSYMBOL_commands = 99,                  /* commands  */
  YYSYMBOL_command_wrapper = 100,          /* command_wrapper  */
  YYSYMBOL_alter_table_stmt = 101,         /* alter_table_stmt  */
  YYSYMBOL_exit_stmt = 102,                /* exit_stmt  */
  YYSYMBOL_help_stmt = 103,                /* help_stmt  */
  YYSYMBOL_sync_stmt = 104,                /* sync_stmt  */
  YYSYMBOL_begin_stmt = 105,               /* begin_stmt  */
  YYSYMBOL_commit_stmt = 106,              /* commit_stmt  */
  YYSYMBOL_rollback_stmt = 107,            /* rollback_stmt  */
  YYSYMBOL_drop_table_stmt = 108,          /* drop_table_stmt  */
  YYSYMBOL_analyze_table_stmt = 109,       /* analyze_table_stmt  */
  YYSYMBOL_show_tables_stmt = 110,         /* show_tables_stmt  */
  YYSYMBOL_show_index_stmt = 111,          /* show_index_stmt  */
  YYSYMBOL_desc_table_stmt = 112,          /* desc_table_stmt  */
  YYSYMBOL_create_index_stmt = 113,        /* create_index_stmt  */
  YYSYMBOL_drop_index_stmt = 114,          /* drop_index_stmt  */
  YYSYMBOL_create_table_stmt = 115,        /* create_table_stmt  */
  YYSYMBOL_create_view_stmt = 116,         /* create_view_stmt  */
  YYSYMBOL_drop_view_stmt = 117,           /* drop_view_stmt  */
  YYSYMBOL_attr_def_list = 118,            /* attr_def_list  */
  YYSYMBOL_attr_def = 119,                 /* attr_def  */
  YYSYMBOL_number = 120,                   /* number  */
  YYSYMBOL_float_number = 121,             /* float_number  */
  YYSYMBOL_type = 122,                     /* type  */
  YYSYMBOL_primary_key = 123,              /* primary_key  */
  YYSYMBOL_attr_list = 124,                /* attr_list  */
  YYSYMBOL_insert_stmt = 125,              /* insert_stmt  */
  YYSYMBOL_insert_value_list = 126,        /* insert_value_list  */
  YYSYMBOL_insert_value = 127,             /* insert_value  */
  YYSYMBOL_value = 128,                    /* value  */
  YYSYMBOL_storage_format = 129,           /* storage_format  */
  YYSYMBOL_delete_stmt = 130,              /* delete_stmt  */
  YYSYMBOL_update_stmt = 131,              /* update_stmt  */
  YYSYMBOL_update_kv_list = 132,           /* update_kv_list  */
  YYSYMBOL_select_stmt = 133,              /* select_stmt  */
  YYSYMBOL_union_stmt = 134,               /* union_stmt  */
  YYSYMBOL_calc_stmt = 135,                /* calc_stmt  */
  YYSYMBOL_expression_list = 136,          /* expression_list  */
  YYSYMBOL_projection_list = 137,          /* projection_list  */
  YYSYMBOL_opt_as_alias = 138,             /* opt_as_alias  */
  YYSYMBOL_expression = 139,               /* expression  */
  YYSYMBOL_sub_query_expr = 140,           /* sub_query_expr  */
  YYSYMBOL_rel_attr = 141,                 /* rel_attr  */
  YYSYMBOL_relation = 142,                 /* relation  */
  YYSYMBOL_rel_list = 143,                 /* rel_list  */
  YYSYMBOL_where = 144,                    /* where  */
  YYSYMBOL_condition_list = 145,           /* condition_list  */
  YYSYMBOL_condition = 146,                /* condition  */
  YYSYMBOL_comp_op = 147,                  /* comp_op  */
  YYSYMBOL_group_by = 148,                 /* group_by  */
  YYSYMBOL_having = 149,                   /* having  */
  YYSYMBOL_load_data_stmt = 150,           /* load_data_stmt  */
  YYSYMBOL_explain_stmt = 151,             /* explain_stmt  */
  YYSYMBOL_set_variable_stmt = 152,        /* set_variable_stmt  */
  YYSYMBOL_vector_literal = 153,           /* vector_literal  */
  YYSYMBOL_number_list = 154,              /* number_list  */
  YYSYMBOL_order_by = 155,                 /* order_by  */
  YYSYMBOL_order_by_item_list = 156,       /* order_by_item_list  */
  YYSYMBOL_order_by_item = 157,            /* order_by_item  */
  YYSYMBOL_limit_clause = 158,             /* limit_clause  */
  YYSYMBOL_option_list = 159,              /* option_list  */
  YYSYMBOL_option_item = 160,              /* option_item  */
  YYSYMBOL_count_expr = 161,               /* count_expr  */
  YYSYMBOL_aggregate_expr = 162,           /* aggregate_expr  */
  YYSYMBOL_join_list = 163,                /* join_list  */
  YYSYMBOL_join_item = 164,                /* join_item  */
  YYSYMBOL_sub_query = 165,                /* sub_query  */
  YYSYMBOL_opt_semicolon = 166             /* opt_semicolon  */
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
#define YYFINAL  95
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   743

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  98
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  69
/* YYNRULES -- Number of rules.  */
#define YYNRULES  204
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  432

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   348


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
       2,     2,    95,    93,     2,    94,     2,    96,     2,     2,
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
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    97
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   271,   271,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   308,
     316,   324,   332,   341,   347,   352,   358,   364,   370,   376,
     381,   388,   395,   401,   409,   416,   428,   440,   452,   467,
     475,   499,   507,   534,   542,   555,   564,   567,   580,   596,
     616,   631,   646,   665,   682,   686,   687,   690,   691,   692,
     693,   694,   695,   699,   702,   709,   713,   725,   737,   755,
     758,   772,   773,   777,   782,   796,   800,   804,   810,   814,
     871,   874,   881,   893,   931,   937,   944,   973,  1002,  1044,
    1063,  1080,  1089,  1098,  1105,  1112,  1124,  1133,  1138,  1150,
    1159,  1175,  1176,  1177,  1181,  1184,  1187,  1190,  1193,  1197,
    1201,  1204,  1210,  1215,  1244,  1250,  1253,  1256,  1263,  1310,
    1333,  1337,  1342,  1350,  1354,  1359,  1366,  1371,  1386,  1389,
    1401,  1404,  1409,  1416,  1425,  1433,  1455,  1477,  1486,  1495,
    1503,  1511,  1520,  1533,  1534,  1535,  1536,  1537,  1538,  1539,
    1540,  1541,  1542,  1548,  1551,  1559,  1562,  1568,  1580,  1588,
    1598,  1636,  1640,  1648,  1651,  1658,  1664,  1673,  1679,  1685,
    1694,  1695,  1700,  1701,  1702,  1706,  1707,  1708,  1712,  1725,
    1728,  1732,  1736,  1740,  1748,  1751,  1764,  1774,  1785,  1796,
    1806,  1817,  1831,  1837,  1838
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
  "LIMIT", "WITH", "CREATE", "DROP", "GROUP", "HAVING", "ORDER", "ASC",
  "AS", "COUNT", "SUM", "MAX", "MIN", "AVG", "TABLE", "TABLES", "INDEX",
  "ALTER", "ADD", "CHANGE", "COLUMN", "RENAME", "TO", "IF", "EXISTS",
  "CALC", "SELECT", "UNION", "UNION_ALL", "DESC", "SHOW", "SYNC", "INSERT",
  "DELETE", "UPDATE", "LBRACE", "RBRACE", "LSQUARE", "RSQUARE", "COMMA",
  "TRX_BEGIN", "TRX_COMMIT", "TRX_ROLLBACK", "INT_T", "STRING_T",
  "FLOAT_T", "DATE_T", "VECTOR_T", "TEXT_T", "HELP", "EXIT", "DOT", "INTO",
  "VALUES", "FROM", "WHERE", "AND", "OR", "JOIN", "INNER", "ON", "NOT",
  "NULL_T", "LIKE", "SET", "LOAD", "DATA", "INFILE", "EXPLAIN", "STORAGE",
  "FORMAT", "PRIMARY", "KEY", "ANALYZE", "IN", "IS", "UNIQUE", "VIEW",
  "EQ", "LT", "GT", "LE", "GE", "NE", "NUMBER", "FLOAT_LITERAL", "ID",
  "SSS", "'+'", "'-'", "'*'", "'/'", "UMINUS", "$accept", "commands",
  "command_wrapper", "alter_table_stmt", "exit_stmt", "help_stmt",
  "sync_stmt", "begin_stmt", "commit_stmt", "rollback_stmt",
  "drop_table_stmt", "analyze_table_stmt", "show_tables_stmt",
  "show_index_stmt", "desc_table_stmt", "create_index_stmt",
  "drop_index_stmt", "create_table_stmt", "create_view_stmt",
  "drop_view_stmt", "attr_def_list", "attr_def", "number", "float_number",
  "type", "primary_key", "attr_list", "insert_stmt", "insert_value_list",
  "insert_value", "value", "storage_format", "delete_stmt", "update_stmt",
  "update_kv_list", "select_stmt", "union_stmt", "calc_stmt",
  "expression_list", "projection_list", "opt_as_alias", "expression",
  "sub_query_expr", "rel_attr", "relation", "rel_list", "where",
  "condition_list", "condition", "comp_op", "group_by", "having",
  "load_data_stmt", "explain_stmt", "set_variable_stmt", "vector_literal",
  "number_list", "order_by", "order_by_item_list", "order_by_item",
  "limit_clause", "option_list", "option_item", "count_expr",
  "aggregate_expr", "join_list", "join_item", "sub_query", "opt_semicolon", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-339)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-119)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     435,     9,    17,    23,   328,   328,   -34,    86,  -339,    70,
      15,    83,    98,  -339,  -339,  -339,  -339,  -339,    87,   113,
     435,   181,   209,   213,  -339,  -339,  -339,  -339,  -339,  -339,
    -339,  -339,  -339,  -339,  -339,  -339,  -339,  -339,  -339,  -339,
    -339,  -339,  -339,  -339,    49,   132,  -339,  -339,  -339,  -339,
     121,   135,   211,   212,   139,   -15,   149,   150,   151,   199,
     210,   214,   218,   221,   220,   124,  -339,  -339,  -339,    -1,
    -339,   328,  -339,  -339,  -339,   601,  -339,  -339,  -339,  -339,
    -339,     5,    62,  -339,  -339,   194,   172,   173,   196,    49,
     111,   183,   195,  -339,   177,  -339,  -339,  -339,   238,   238,
     238,   238,    19,   217,   185,   186,    30,   245,  -339,   223,
    -339,   153,   328,   328,   328,   328,   328,   328,   443,  -339,
    -339,  -339,   176,   230,   -46,  -339,   328,    52,  -339,  -339,
    -339,  -339,  -339,  -339,  -339,  -339,   328,   328,   328,   328,
     328,   285,   208,   189,   293,   215,  -339,   261,   216,   -17,
     248,   222,   305,    50,   225,  -339,  -339,  -339,  -339,  -339,
     238,   227,   232,   263,   264,   238,   235,   239,   240,   308,
     315,   322,   321,   466,   499,   522,   555,   578,    13,  -339,
    -339,   124,  -339,   309,  -339,  -339,  -339,  -339,  -339,   -49,
     -49,   283,   283,   207,   328,     7,   310,  -339,   312,   326,
     624,   305,   160,   267,  -339,  -339,   328,  -339,   235,   317,
     189,  -339,   278,    61,   293,  -339,   306,  -339,   120,  -339,
     324,   274,   276,  -339,   325,   335,  -339,  -339,   286,   292,
     294,   295,  -339,  -339,  -339,  -339,  -339,   208,   343,  -339,
    -339,     0,   344,  -339,   298,  -339,   208,   123,   238,   350,
      58,   351,   158,   328,   293,   189,   189,  -339,  -339,  -339,
     352,   357,  -339,   328,   301,  -339,  -339,   375,  -339,  -339,
    -339,  -339,  -339,  -339,   -13,   354,   235,   356,   360,   235,
     388,  -339,   355,   311,  -339,  -339,  -339,  -339,  -339,   328,
    -339,  -339,   314,   345,   398,  -339,  -339,   372,   238,   376,
     320,   358,  -339,   647,  -339,  -339,  -339,   363,   364,   382,
     122,   207,    18,   207,   346,   336,   267,   361,  -339,   -51,
     389,   391,   235,   235,  -339,   238,  -339,   385,   123,  -339,
      -2,   347,   430,   425,  -339,   395,   320,   396,   399,  -339,
     401,   147,   148,   357,   403,   328,  -339,   404,  -339,   362,
    -339,    -5,  -339,   409,   419,  -339,  -339,   398,   370,   189,
     397,     6,   328,   328,   305,  -339,   422,   423,  -339,  -339,
     357,   382,  -339,   115,   207,   161,   427,   393,  -339,  -339,
     470,  -339,   425,   412,  -339,   189,   387,   189,   414,  -339,
     207,   293,  -339,  -339,   382,  -339,   416,  -339,   235,   402,
     446,   305,   189,  -339,   426,  -339,   189,  -339,   447,  -339,
     451,   405,   406,   452,  -339,   189,  -339,  -339,  -339,  -339,
     411,   167,  -339,  -339,  -339,   104,  -339,   406,  -339,  -339,
    -339,  -339
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,    35,     0,
       0,     0,     0,    36,    37,    38,    34,    33,     0,     0,
       0,     0,     0,   203,    26,    28,    27,    17,    18,    19,
      20,    10,    11,    12,    13,    14,    15,    16,     9,    24,
      25,     6,     8,     7,     4,     5,     3,    21,    22,    23,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    87,    85,    86,   130,
      89,     0,   125,   121,   106,   107,   127,   124,    88,   189,
     126,   173,   111,    44,    42,     0,     0,     0,     0,     0,
       0,     0,     0,   168,     0,     1,   204,     2,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    39,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,    65,
      66,   171,     0,     0,     0,   120,     0,     0,   159,   161,
     153,   154,   155,   156,   157,   158,     0,     0,     0,     0,
       0,     0,     0,   140,   180,     0,   112,   109,     0,     0,
     138,     0,   173,     0,     0,    41,   101,   102,   103,   104,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   173,   119,
     170,     0,   122,     0,   131,   132,   108,   160,   162,   114,
     115,   116,   117,   118,     0,   133,   136,   194,     0,     0,
       0,   173,   141,     0,   100,   113,     0,    43,     0,     0,
     140,    92,     0,   138,   180,   169,     0,    51,     0,    56,
       0,     0,     0,    53,    75,     0,    40,    49,     0,     0,
       0,     0,   188,   190,   191,   192,   193,     0,     0,   172,
     123,   177,   174,   175,     0,   134,     0,   138,     0,     0,
       0,   161,     0,     0,   180,   140,   140,    64,   181,   110,
       0,     0,   139,     0,     0,    93,   105,     0,    67,    68,
      69,    70,    71,    72,    61,    73,     0,     0,     0,     0,
       0,    30,     0,     0,    32,   194,   129,   178,   179,     0,
     135,   137,     0,     0,   163,   195,   202,     0,     0,   162,
       0,     0,   149,   144,    99,   142,   143,     0,     0,    79,
     121,    84,   121,    96,     0,     0,     0,     0,    62,     0,
       0,     0,     0,     0,    76,     0,    29,     0,   138,   176,
       0,     0,     0,   165,   151,     0,     0,     0,     0,   150,
       0,    85,    86,     0,     0,     0,   167,     0,    63,     0,
      57,    90,    45,     0,     0,    54,    31,   163,     0,   140,
       0,     0,     0,     0,   173,   152,     0,     0,   145,   147,
       0,    79,    77,   121,    97,    58,     0,     0,    50,    52,
      47,    46,   165,     0,   199,   140,     0,   140,     0,   164,
     166,   180,   146,   148,    79,    80,     0,    59,     0,     0,
       0,   173,   140,   200,     0,   196,   140,    98,     0,    60,
       0,     0,   182,     0,   201,   140,   197,    78,    74,    91,
       0,     0,   183,   128,   198,     0,    48,     0,   186,   187,
     185,   184
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -339,  -339,   467,  -339,  -339,  -339,  -339,  -339,  -339,  -339,
    -339,  -339,  -339,  -339,  -339,  -339,  -339,  -339,  -339,  -339,
    -339,   179,   184,   318,  -339,  -339,  -199,  -339,  -338,  -312,
    -147,  -339,  -339,  -339,  -339,     2,   483,  -339,  -119,  -102,
    -339,    -4,  -339,  -339,  -339,   -86,  -203,  -205,  -339,   302,
     144,   128,  -339,  -339,  -339,  -339,  -339,  -149,  -339,   226,
    -206,  -339,    76,  -339,  -339,   229,  -339,  -263,  -339
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
     275,   219,   258,   121,   274,   320,   225,    41,   344,   309,
      73,   378,    42,    43,   213,   296,    45,    46,    74,    81,
     147,   200,    76,    77,   196,   197,   211,   201,   202,   140,
     333,   364,    47,    48,    49,    78,   122,   144,   242,   243,
     204,   421,   422,    79,    80,   247,   295,   297,    97
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      75,    82,    44,   214,   183,   262,   215,   186,   266,   260,
     265,   358,   287,   107,    89,   178,   141,   127,   -94,   386,
     244,   -94,    44,   208,   141,   349,     5,   316,    50,   238,
      51,   371,   160,   395,   288,   335,    55,   338,    56,   123,
     218,   209,    58,   165,   294,   184,   138,   139,   304,   185,
     305,   306,   254,   317,   318,   124,   408,    83,   394,   161,
     118,    52,   -94,   359,   142,   143,   127,   125,   128,   377,
     166,   387,   237,   367,    87,   145,   108,   321,   -94,   129,
     324,    98,    99,   130,   131,   132,   133,   134,   135,   360,
      53,    54,    65,   136,   137,   138,   139,   388,   245,    57,
     156,   157,   158,   159,   259,   264,    84,    85,   173,   174,
     175,   176,   177,    82,   310,   -95,   312,    66,   -95,    75,
     187,   210,    75,   353,   354,   357,   187,    86,   127,     5,
     128,   188,   189,   190,   191,   192,   193,   299,    12,    67,
      68,   129,    70,   100,   101,   130,   131,   132,   133,   134,
     135,   285,   152,   146,   384,   136,   137,   138,   139,   -95,
     291,   169,   217,   -81,   100,   101,   -81,   223,   268,   269,
     270,   271,   272,   273,    88,   -95,   170,   171,    91,   172,
     403,   337,   405,   210,    92,   407,   292,   293,   -82,   -83,
     241,   -82,   -83,   428,   429,   430,   310,   414,   373,   410,
      94,   416,    82,    59,    60,    61,    62,    63,   426,    95,
     424,   427,   102,   119,   120,   391,    96,   366,   198,   180,
     181,   255,   256,   310,   301,   302,   103,   396,   397,    64,
     106,    65,   104,   105,    59,    60,    61,    62,    63,   112,
     109,   110,   111,   389,    59,    60,    61,    62,    63,   303,
     113,   117,   413,   148,   114,   199,    66,   311,   115,   313,
      64,   116,    65,   149,   150,   151,   153,   154,   155,     5,
      64,   182,    65,   127,   167,   128,   163,   164,    67,    68,
      69,    70,   162,    71,    72,   241,   129,    66,   168,   194,
     130,   131,   132,   133,   134,   135,    75,    66,   203,   195,
     136,   137,   138,   139,   125,   206,   205,   207,   210,    67,
      68,    69,    70,   212,    71,    72,   141,   216,   218,    67,
      68,    69,    70,   220,    71,    72,   224,   355,   221,   222,
     226,   227,    75,   228,    59,    60,    61,    62,    63,   311,
     229,   374,    59,    60,    61,    62,    63,   230,   231,   127,
     240,     5,   248,   379,   246,   249,   257,   261,    75,   390,
      64,   263,    65,   267,   276,   277,   311,   278,    64,   279,
      65,    59,    60,    61,    62,    63,   280,   281,    59,    60,
      61,    62,    63,   282,   286,   283,   284,    66,   289,   290,
     298,   300,   314,   307,   315,    66,   322,    64,   319,    65,
     323,   325,   327,   326,    64,   330,    65,   332,   331,    67,
      68,    69,    70,   334,    71,    72,   336,    67,    68,    69,
      70,   340,    71,    72,    66,   339,   343,   346,   348,   345,
     351,    66,   352,   356,   362,   363,   365,   368,   361,   376,
     369,   370,     1,     2,   372,   375,    67,    68,    69,    70,
     380,   308,    72,   341,   342,    69,    70,     3,    71,    72,
     381,   383,   385,   392,   393,     4,     5,   398,   399,     6,
       7,     8,     9,    10,    11,    12,   400,   402,   404,   406,
      13,    14,    15,   409,   179,   411,   412,    93,   417,    16,
      17,   415,   418,   423,   425,    90,   419,   420,   350,   239,
     347,   382,   253,   431,    18,    19,     0,   232,    20,   127,
     401,   128,     0,    21,   328,   329,     0,     0,     0,     0,
       0,     0,   129,     0,     0,     0,   130,   131,   132,   133,
     134,   135,   127,     0,   128,     0,   136,   137,   138,   139,
     233,     0,     0,     0,     0,   129,     0,     0,     0,   130,
     131,   132,   133,   134,   135,     0,     0,     0,     0,   136,
     137,   138,   139,   234,     0,   127,     0,   128,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   129,     0,
       0,     0,   130,   131,   132,   133,   134,   135,   127,     0,
     128,     0,   136,   137,   138,   139,   235,     0,     0,     0,
       0,   129,     0,     0,     0,   130,   131,   132,   133,   134,
     135,     0,     0,     0,     0,   136,   137,   138,   139,   236,
       0,   127,     0,   128,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   129,     0,     0,     0,   130,   131,
     132,   133,   134,   135,   127,   126,   128,     0,   136,   137,
     138,   139,     0,     0,     0,     0,     0,   129,     0,     0,
       0,   130,   131,   132,   133,   134,   135,   127,     0,   128,
       0,   136,   137,   138,   139,     0,     0,     0,     0,     0,
     129,     0,     0,     0,   130,   131,   132,   133,   134,   135,
     250,     0,   128,     0,   136,   137,   138,   139,     0,     0,
       0,     0,     0,   251,   252,     0,     0,   130,   131,   132,
     133,   134,   135,   127,     0,   128,     0,   136,   137,   138,
     139,     0,     0,     0,     0,     0,   129,  -118,     0,     0,
     130,   131,   132,   133,   134,   135,     0,     0,     0,     0,
     136,   137,   138,   139
};

static const yytype_int16 yycheck[] =
{
       4,     5,     0,   152,   123,   210,   153,   126,   214,   208,
     213,    13,    12,    28,    12,   117,    11,    66,     0,    13,
      13,     3,    20,    40,    11,    76,    31,    40,    19,   178,
      21,   343,    13,   371,    34,   298,    19,   300,    21,    40,
      91,    58,    19,    13,   247,    91,    95,    96,   254,    95,
     255,   256,   201,    66,    67,    56,   394,    91,   370,    40,
      64,    52,    44,    65,    59,    60,    66,    71,    68,    74,
      40,    65,    59,   336,    59,    13,    91,   276,    60,    79,
     279,    32,    33,    83,    84,    85,    86,    87,    88,    91,
      81,    82,    42,    93,    94,    95,    96,    91,    91,    82,
      98,    99,   100,   101,   206,    44,    20,    21,   112,   113,
     114,   115,   116,   117,   261,     0,   263,    67,     3,   123,
      68,    60,   126,   322,   323,   328,    68,    57,    66,    31,
      68,    79,   136,   137,   138,   139,   140,    79,    40,    89,
      90,    79,    92,    32,    33,    83,    84,    85,    86,    87,
      88,   237,    41,    91,   359,    93,    94,    95,    96,    44,
     246,     8,   160,    41,    32,    33,    44,   165,    48,    49,
      50,    51,    52,    53,    91,    60,    23,    24,    91,    26,
     385,   300,   387,    60,    71,   391,    63,    64,    41,    41,
     194,    44,    44,    89,    90,    91,   343,   402,   345,   398,
      19,   406,   206,    14,    15,    16,    17,    18,    41,     0,
     415,    44,    91,    89,    90,   364,     3,   336,    29,    43,
      44,    61,    62,   370,    66,    67,    91,    66,    67,    40,
      91,    42,    21,    21,    14,    15,    16,    17,    18,    40,
      91,    91,    91,   362,    14,    15,    16,    17,    18,   253,
      40,    31,   401,    59,    40,    66,    67,   261,    40,   263,
      40,    40,    42,    91,    91,    69,    83,    72,    91,    31,
      40,    41,    42,    66,    29,    68,    91,    91,    89,    90,
      91,    92,    65,    94,    95,   289,    79,    67,    65,     4,
      83,    84,    85,    86,    87,    88,   300,    67,     5,    91,
      93,    94,    95,    96,   308,    44,    91,    91,    60,    89,
      90,    91,    92,    91,    94,    95,    11,    92,    91,    89,
      90,    91,    92,    91,    94,    95,    91,   325,    65,    65,
      91,    91,   336,    25,    14,    15,    16,    17,    18,   343,
      25,   345,    14,    15,    16,    17,    18,    25,    27,    66,
      41,    31,    40,   351,    44,    29,    89,    40,   362,   363,
      40,    83,    42,    57,    40,    91,   370,    91,    40,    44,
      42,    14,    15,    16,    17,    18,    41,    91,    14,    15,
      16,    17,    18,    91,    41,    91,    91,    67,    44,    91,
      40,    40,    91,    41,    19,    67,    40,    40,    44,    42,
      40,    13,    91,    48,    40,    91,    42,     9,    63,    89,
      90,    91,    92,    41,    94,    95,    40,    89,    90,    91,
      92,    58,    94,    95,    67,    67,    44,    91,    67,    83,
      41,    67,    41,    48,     4,    10,    41,    41,    91,    77,
      41,    40,     7,     8,    41,    41,    89,    90,    91,    92,
      41,    94,    95,    89,    90,    91,    92,    22,    94,    95,
      41,    91,    65,    41,    41,    30,    31,    40,    75,    34,
      35,    36,    37,    38,    39,    40,     6,    65,    91,    65,
      45,    46,    47,    67,    41,    83,    40,    20,    41,    54,
      55,    65,    41,    41,    83,    12,    91,    91,   319,   181,
     316,   357,   200,   427,    69,    70,    -1,    41,    73,    66,
     382,    68,    -1,    78,   285,   289,    -1,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    83,    84,    85,    86,
      87,    88,    66,    -1,    68,    -1,    93,    94,    95,    96,
      41,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    83,
      84,    85,    86,    87,    88,    -1,    -1,    -1,    -1,    93,
      94,    95,    96,    41,    -1,    66,    -1,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    83,    84,    85,    86,    87,    88,    66,    -1,
      68,    -1,    93,    94,    95,    96,    41,    -1,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    83,    84,    85,    86,    87,
      88,    -1,    -1,    -1,    -1,    93,    94,    95,    96,    41,
      -1,    66,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    83,    84,
      85,    86,    87,    88,    66,    44,    68,    -1,    93,    94,
      95,    96,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    83,    84,    85,    86,    87,    88,    66,    -1,    68,
      -1,    93,    94,    95,    96,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    83,    84,    85,    86,    87,    88,
      66,    -1,    68,    -1,    93,    94,    95,    96,    -1,    -1,
      -1,    -1,    -1,    79,    80,    -1,    -1,    83,    84,    85,
      86,    87,    88,    66,    -1,    68,    -1,    93,    94,    95,
      96,    -1,    -1,    -1,    -1,    -1,    79,    80,    -1,    -1,
      83,    84,    85,    86,    87,    88,    -1,    -1,    -1,    -1,
      93,    94,    95,    96
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     7,     8,    22,    30,    31,    34,    35,    36,    37,
      38,    39,    40,    45,    46,    47,    54,    55,    69,    70,
      73,    78,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   125,   130,   131,   133,   134,   135,   150,   151,   152,
      19,    21,    52,    81,    82,    19,    21,    82,    19,    14,
      15,    16,    17,    18,    40,    42,    67,    89,    90,    91,
      92,    94,    95,   128,   136,   139,   140,   141,   153,   161,
     162,   137,   139,    91,    20,    21,    57,    59,    91,   133,
     134,    91,    71,   100,    19,     0,     3,   166,    32,    33,
      32,    33,    91,    91,    21,    21,    91,    28,    91,    91,
      91,    91,    40,    40,    40,    40,    40,    31,   139,    89,
      90,   121,   154,    40,    56,   139,    44,    66,    68,    79,
      83,    84,    85,    86,    87,    88,    93,    94,    95,    96,
     147,    11,    59,    60,   155,    13,    91,   138,    59,    91,
      91,    69,    41,    83,    72,    91,   133,   133,   133,   133,
      13,    40,    65,    91,    91,    13,    40,    29,    65,     8,
      23,    24,    26,   139,   139,   139,   139,   139,   137,    41,
      43,    44,    41,   136,    91,    95,   136,    68,    79,   139,
     139,   139,   139,   139,     4,    91,   142,   143,    29,    66,
     139,   145,   146,     5,   158,    91,    44,    91,    40,    58,
      60,   144,    91,   132,   155,   128,    92,   133,    91,   119,
      91,    65,    65,   133,    91,   124,    91,    91,    25,    25,
      25,    27,    41,    41,    41,    41,    41,    59,   155,   121,
      41,   139,   156,   157,    13,    91,    44,   163,    40,    29,
      66,    79,    80,   147,   155,    61,    62,    89,   120,   137,
     124,    40,   145,    83,    44,   144,   158,    57,    48,    49,
      50,    51,    52,    53,   122,   118,    40,    91,    91,    44,
      41,    91,    91,    91,    91,   143,    41,    12,    34,    44,
      91,   143,    63,    64,   144,   164,   133,   165,    40,    79,
      40,    66,    67,   139,   158,   145,   145,    41,    94,   127,
     128,   139,   128,   139,    91,    19,    40,    66,    67,    44,
     123,   124,    40,    40,   124,    13,    48,    91,   163,   157,
      91,    63,     9,   148,    41,   165,    40,   136,   165,    67,
      58,    89,    90,    44,   126,    83,    91,   120,    67,    76,
     119,    41,    41,   124,   124,   133,    48,   144,    13,    65,
      91,    91,     4,    10,   149,    41,   136,   165,    41,    41,
      40,   127,    41,   128,   139,    41,    77,    74,   129,   133,
      41,    41,   148,    91,   145,    65,    13,    65,    91,   136,
     139,   155,    41,    41,   127,   126,    66,    67,    40,    75,
       6,   149,    65,   145,    91,   145,    65,   158,   126,    67,
     124,    83,    40,   155,   145,    65,   145,    41,    41,    91,
      91,   159,   160,    41,   145,    83,    41,    44,    89,    90,
      91,   160
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    98,    99,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   101,
     101,   101,   101,   102,   103,   104,   105,   106,   107,   108,
     108,   109,   110,   111,   112,   113,   113,   113,   113,   114,
     115,   115,   115,   116,   116,   117,   118,   118,   119,   119,
     119,   119,   119,   119,   120,   121,   121,   122,   122,   122,
     122,   122,   122,   123,   123,   124,   124,   125,   125,   126,
     126,   127,   127,   127,   127,   128,   128,   128,   128,   128,
     129,   129,   130,   131,   132,   132,   132,   132,   133,   133,
     133,   134,   134,   134,   134,   134,   135,   136,   136,   137,
     137,   138,   138,   138,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   140,   140,
     141,   141,   141,   142,   142,   142,   143,   143,   144,   144,
     145,   145,   145,   145,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   148,   148,   149,   149,   150,   151,   152,
     153,   154,   154,   155,   155,   156,   156,   157,   157,   157,
     158,   158,   159,   159,   159,   160,   160,   160,   161,   162,
     162,   162,   162,   162,   163,   163,   164,   164,   164,   164,
     164,   164,   165,   166,   166
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     7,
       6,     8,     6,     1,     1,     1,     1,     1,     1,     3,
       5,     3,     2,     4,     2,     8,     9,     9,    13,     5,
       9,     5,     9,     5,     8,     3,     0,     3,     5,     6,
       7,     2,     3,     4,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     6,     1,     3,     8,    11,     0,
       3,     1,     2,     2,     1,     1,     1,     1,     1,     1,
       0,     4,     4,     5,     3,     5,     3,     5,    10,     6,
       4,     3,     3,     3,     3,     5,     2,     1,     3,     2,
       4,     0,     1,     2,     3,     3,     3,     3,     3,     3,
       2,     1,     3,     4,     1,     1,     1,     1,    11,     5,
       1,     3,     3,     1,     2,     3,     1,     3,     0,     2,
       0,     1,     3,     3,     3,     5,     6,     5,     6,     3,
       4,     4,     5,     1,     1,     1,     1,     1,     1,     1,
       2,     1,     2,     0,     3,     0,     2,     7,     2,     4,
       3,     1,     3,     0,     3,     1,     3,     1,     2,     2,
       0,     2,     0,     1,     3,     3,     3,     3,     4,     1,
       4,     4,     4,     4,     0,     2,     5,     6,     7,     4,
       5,     6,     1,     0,     1
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
#line 272 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
  {
    unique_ptr<ParsedSqlNode> sql_node = unique_ptr<ParsedSqlNode>((yyvsp[-1].sql_node));
    sql_result->add_sql_node(std::move(sql_node));
  }
#line 2054 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 29: /* alter_table_stmt: ALTER TABLE ID ADD COLUMN ID INT_T  */
#line 308 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                       {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ALTER_TABLE);
      (yyval.sql_node)->alter_table.op_type = ParsedSqlNode::AlterOpType::ADD_COLUMN;
      (yyval.sql_node)->alter_table.table_name = (yyvsp[-4].cstring);
      (yyval.sql_node)->alter_table.column_name = (yyvsp[-1].cstring);
      (yyval.sql_node)->alter_table.new_column_name.clear();
      (yyval.sql_node)->alter_table.data_type = AttrType::INTS;
    }
#line 2067 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 30: /* alter_table_stmt: ALTER TABLE ID DROP COLUMN ID  */
#line 316 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ALTER_TABLE);
      (yyval.sql_node)->alter_table.op_type = ParsedSqlNode::AlterOpType::DROP_COLUMN;
      (yyval.sql_node)->alter_table.table_name = (yyvsp[-3].cstring);
      (yyval.sql_node)->alter_table.column_name = (yyvsp[0].cstring);
      (yyval.sql_node)->alter_table.new_column_name.clear();
      (yyval.sql_node)->alter_table.data_type = AttrType::UNDEFINED;
    }
#line 2080 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 31: /* alter_table_stmt: ALTER TABLE ID CHANGE COLUMN ID ID INT_T  */
#line 324 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                             {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ALTER_TABLE);
      (yyval.sql_node)->alter_table.op_type = ParsedSqlNode::AlterOpType::CHANGE_COLUMN_RENAME;
      (yyval.sql_node)->alter_table.table_name = (yyvsp[-5].cstring);
      (yyval.sql_node)->alter_table.column_name = (yyvsp[-2].cstring);
      (yyval.sql_node)->alter_table.new_column_name = (yyvsp[-1].cstring);
      (yyval.sql_node)->alter_table.data_type = AttrType::INTS; /* type not used, but present */
    }
#line 2093 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 32: /* alter_table_stmt: ALTER TABLE ID RENAME TO ID  */
#line 332 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ALTER_TABLE);
      (yyval.sql_node)->alter_table.op_type = ParsedSqlNode::AlterOpType::RENAME_TABLE;
      (yyval.sql_node)->alter_table.table_name = (yyvsp[-3].cstring);
      (yyval.sql_node)->alter_table.new_table_name = (yyvsp[0].cstring);
    }
#line 2104 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 33: /* exit_stmt: EXIT  */
#line 341 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
         {
      (void)yynerrs;  // 这么写为了消除yynerrs未使用的告警。如果你有更好的方法欢迎提PR
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXIT);
    }
#line 2113 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 34: /* help_stmt: HELP  */
#line 347 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_HELP);
    }
#line 2121 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 35: /* sync_stmt: SYNC  */
#line 352 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SYNC);
    }
#line 2129 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 36: /* begin_stmt: TRX_BEGIN  */
#line 358 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_BEGIN);
    }
#line 2137 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 37: /* commit_stmt: TRX_COMMIT  */
#line 364 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_COMMIT);
    }
#line 2145 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 38: /* rollback_stmt: TRX_ROLLBACK  */
#line 370 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ROLLBACK);
    }
#line 2153 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 39: /* drop_table_stmt: DROP TABLE ID  */
#line 376 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_TABLE);
      (yyval.sql_node)->drop_table.relation_name = (yyvsp[0].cstring);
      (yyval.sql_node)->drop_table.if_exists = false;
    }
#line 2163 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 40: /* drop_table_stmt: DROP TABLE IF EXISTS ID  */
#line 381 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                              {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_TABLE);
      (yyval.sql_node)->drop_table.relation_name = (yyvsp[0].cstring);
      (yyval.sql_node)->drop_table.if_exists = true;
    }
#line 2173 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 41: /* analyze_table_stmt: ANALYZE TABLE ID  */
#line 388 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                     {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ANALYZE_TABLE);
      (yyval.sql_node)->analyze_table.relation_name = (yyvsp[0].cstring);
    }
#line 2182 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 42: /* show_tables_stmt: SHOW TABLES  */
#line 395 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SHOW_TABLES);
    }
#line 2190 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 43: /* show_index_stmt: SHOW INDEX FROM ID  */
#line 402 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SHOW_INDEX);
      (yyval.sql_node)->show_index.relation_name = (yyvsp[0].cstring);
    }
#line 2199 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 44: /* desc_table_stmt: DESC ID  */
#line 409 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
             {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DESC_TABLE);
      (yyval.sql_node)->desc_table.relation_name = (yyvsp[0].cstring);
    }
#line 2208 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 45: /* create_index_stmt: CREATE INDEX ID ON ID LBRACE attr_list RBRACE  */
#line 417 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
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
#line 2224 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 46: /* create_index_stmt: CREATE UNIQUE INDEX ID ON ID LBRACE attr_list RBRACE  */
#line 429 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
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
#line 2240 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 47: /* create_index_stmt: CREATE VECTOR_T INDEX ID ON ID LBRACE attr_list RBRACE  */
#line 441 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
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
#line 2256 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 48: /* create_index_stmt: CREATE VECTOR_T INDEX ID ON ID LBRACE attr_list RBRACE WITH LBRACE option_list RBRACE  */
#line 453 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_INDEX);
      CreateIndexSqlNode &create_index = (yyval.sql_node)->create_index;
      create_index.index_name = (yyvsp[-9].cstring);
      create_index.relation_name = (yyvsp[-7].cstring);
      if ((yyvsp[-5].key_list) != nullptr) {
        create_index.attribute_names.swap(*(yyvsp[-5].key_list));
        delete (yyvsp[-5].key_list);
      }
      create_index.is_unique = false;
    }
#line 2272 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 49: /* drop_index_stmt: DROP INDEX ID ON ID  */
#line 468 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_INDEX);
      (yyval.sql_node)->drop_index.index_name = (yyvsp[-2].cstring);
      (yyval.sql_node)->drop_index.relation_name = (yyvsp[0].cstring);
    }
#line 2282 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 50: /* create_table_stmt: CREATE TABLE ID LBRACE attr_def attr_def_list primary_key RBRACE storage_format  */
#line 476 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
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
#line 2310 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 51: /* create_table_stmt: CREATE TABLE ID AS select_stmt  */
#line 500 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_TABLE);
      CreateTableSqlNode &create_table = (yyval.sql_node)->create_table;
      create_table.relation_name = (yyvsp[-2].cstring);
      create_table.as_select = std::make_unique<SelectSqlNode>(std::move((yyvsp[0].sql_node)->selection));
      delete (yyvsp[0].sql_node);
    }
#line 2322 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 52: /* create_table_stmt: CREATE TABLE ID LBRACE attr_def attr_def_list primary_key RBRACE select_stmt  */
#line 508 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_TABLE);
      CreateTableSqlNode &create_table = (yyval.sql_node)->create_table;
      create_table.relation_name = (yyvsp[-6].cstring);
      
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
      
      // Store the SELECT statement for data insertion
      create_table.as_select = std::make_unique<SelectSqlNode>(std::move((yyvsp[0].sql_node)->selection));
      delete (yyvsp[0].sql_node);
    }
#line 2350 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 53: /* create_view_stmt: CREATE VIEW ID AS select_stmt  */
#line 535 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_VIEW);
      CreateViewSqlNode &create_view = (yyval.sql_node)->create_view;
      create_view.relation_name = (yyvsp[-2].cstring);
      create_view.create_view_select = std::make_unique<SelectSqlNode>(std::move((yyvsp[0].sql_node)->selection));
      delete (yyvsp[0].sql_node);
    }
#line 2362 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 54: /* create_view_stmt: CREATE VIEW ID LBRACE attr_list RBRACE AS select_stmt  */
#line 543 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_VIEW);
      CreateViewSqlNode &create_view = (yyval.sql_node)->create_view;
      create_view.relation_name = (yyvsp[-5].cstring);
      create_view.attribute_names = std::move(*(yyvsp[-3].key_list));
      create_view.create_view_select = std::make_unique<SelectSqlNode>(std::move((yyvsp[0].sql_node)->selection));
      delete (yyvsp[-3].key_list);
      delete (yyvsp[0].sql_node);
    }
#line 2376 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 55: /* drop_view_stmt: DROP VIEW ID  */
#line 556 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_VIEW);
      (yyval.sql_node)->drop_view.relation_name = (yyvsp[0].cstring);
    }
#line 2385 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 56: /* attr_def_list: %empty  */
#line 564 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.attr_infos) = nullptr;
    }
#line 2393 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 57: /* attr_def_list: attr_def_list COMMA attr_def  */
#line 568 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      if ((yyvsp[-2].attr_infos) != nullptr) {
        (yyval.attr_infos) = (yyvsp[-2].attr_infos);
      } else {
        (yyval.attr_infos) = new vector<AttrInfoSqlNode>;
      }
      (yyval.attr_infos)->insert((yyval.attr_infos)->begin(), *(yyvsp[0].attr_info));
      delete (yyvsp[0].attr_info);
    }
#line 2407 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 58: /* attr_def: ID type LBRACE number RBRACE  */
#line 581 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-3].number);
      (yyval.attr_info)->name = (yyvsp[-4].cstring);
      (yyval.attr_info)->nullable = false;  // default NOT NULL
      // For VECTOR(n), store bytes for [dimension(int32) + n float32 values]
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int) + ((yyvsp[-1].number) * sizeof(float)));
      } else if ((yyval.attr_info)->type == AttrType::TEXTS) {
        // TEXT stores offset(8)+length(4) in-row
        (yyval.attr_info)->length = static_cast<int>(8 + 4);
      } else {
        (yyval.attr_info)->length = (yyvsp[-1].number);
      }
    }
#line 2427 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 59: /* attr_def: ID type LBRACE number RBRACE NULL_T  */
#line 597 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-4].number);
      (yyval.attr_info)->name = (yyvsp[-5].cstring);
      (yyval.attr_info)->nullable = true;  // explicitly nullable
      // For VECTOR(n), store bytes for [dimension(int32) + n float32 values]
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int) + ((yyvsp[-2].number) * sizeof(float)));
      } else if ((yyval.attr_info)->type == AttrType::TEXTS) {
        // TEXT in-row header size
        (yyval.attr_info)->length = static_cast<int>(8 + 4);
      } else {
        (yyval.attr_info)->length = (yyvsp[-2].number);
      }
      // Add extra byte for NULL flag for nullable fields
      if ((yyval.attr_info)->nullable) {
        (yyval.attr_info)->length++;
      }
    }
#line 2451 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 60: /* attr_def: ID type LBRACE number RBRACE NOT NULL_T  */
#line 617 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-5].number);
      (yyval.attr_info)->name = (yyvsp[-6].cstring);
      (yyval.attr_info)->nullable = false;  // explicitly not nullable
      // For VECTOR(n), store bytes for [dimension(int32) + n float32 values]
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int) + ((yyvsp[-3].number) * sizeof(float)));
      } else if ((yyval.attr_info)->type == AttrType::TEXTS) {
        (yyval.attr_info)->length = static_cast<int>(8 + 4);
      } else {
        (yyval.attr_info)->length = (yyvsp[-3].number);
      }
    }
#line 2470 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 61: /* attr_def: ID type  */
#line 632 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[0].number);
      (yyval.attr_info)->name = (yyvsp[-1].cstring);
      (yyval.attr_info)->nullable = false;  // default NOT NULL
      // Default length: INT/FLOAT/DATE/STRING fallback 4; VECTOR default header size (dimension); TEXT header 12
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int));
      } else if ((yyval.attr_info)->type == AttrType::TEXTS) {
        (yyval.attr_info)->length = static_cast<int>(8 + 4);
      } else {
        (yyval.attr_info)->length = 4;
      }
    }
#line 2489 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 62: /* attr_def: ID type NULL_T  */
#line 647 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-1].number);
      (yyval.attr_info)->name = (yyvsp[-2].cstring);
      (yyval.attr_info)->nullable = true;  // explicitly nullable
      // Default length: INT/FLOAT/DATE/STRING fallback 4; VECTOR default header size (dimension); TEXT header 12
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int));
      } else if ((yyval.attr_info)->type == AttrType::TEXTS) {
        (yyval.attr_info)->length = static_cast<int>(8 + 4);
      } else {
        (yyval.attr_info)->length = 4;
      }
      // Add extra byte for NULL flag for nullable fields
      if ((yyval.attr_info)->nullable) {
        (yyval.attr_info)->length++;
      }
    }
#line 2512 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 63: /* attr_def: ID type NOT NULL_T  */
#line 666 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-2].number);
      (yyval.attr_info)->name = (yyvsp[-3].cstring);
      (yyval.attr_info)->nullable = false;  // explicitly not nullable
      // Default length: INT/FLOAT/DATE/STRING fallback 4; VECTOR default header size (dimension); TEXT header 12
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int));
      } else if ((yyval.attr_info)->type == AttrType::TEXTS) {
        (yyval.attr_info)->length = static_cast<int>(8 + 4);
      } else {
        (yyval.attr_info)->length = 4;
      }
    }
#line 2531 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 64: /* number: NUMBER  */
#line 682 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
           {(yyval.number) = (yyvsp[0].number);}
#line 2537 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 65: /* float_number: NUMBER  */
#line 686 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
           {(yyval.floats) = (double)(yyvsp[0].number);}
#line 2543 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 66: /* float_number: FLOAT_LITERAL  */
#line 687 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                    {(yyval.floats) = (yyvsp[0].floats);}
#line 2549 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 67: /* type: INT_T  */
#line 690 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::INTS); }
#line 2555 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 68: /* type: STRING_T  */
#line 691 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::CHARS); }
#line 2561 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 69: /* type: FLOAT_T  */
#line 692 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::FLOATS); }
#line 2567 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 70: /* type: DATE_T  */
#line 693 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::DATES); }
#line 2573 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 71: /* type: VECTOR_T  */
#line 694 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::VECTORS); }
#line 2579 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 72: /* type: TEXT_T  */
#line 695 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::TEXTS); }
#line 2585 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 73: /* primary_key: %empty  */
#line 699 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.key_list) = nullptr;
    }
#line 2593 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 74: /* primary_key: COMMA PRIMARY KEY LBRACE attr_list RBRACE  */
#line 703 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.key_list) = (yyvsp[-1].key_list);
    }
#line 2601 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 75: /* attr_list: ID  */
#line 709 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
       {
      (yyval.key_list) = new vector<string>();
      (yyval.key_list)->push_back((yyvsp[0].cstring));
    }
#line 2610 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 76: /* attr_list: ID COMMA attr_list  */
#line 713 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                         {
      if ((yyvsp[0].key_list) != nullptr) {
        (yyval.key_list) = (yyvsp[0].key_list);
      } else {
        (yyval.key_list) = new vector<string>;
      }

      (yyval.key_list)->insert((yyval.key_list)->begin(), (yyvsp[-2].cstring));
    }
#line 2624 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 77: /* insert_stmt: INSERT INTO ID VALUES LBRACE insert_value insert_value_list RBRACE  */
#line 726 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
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
#line 2640 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 78: /* insert_stmt: INSERT INTO ID LBRACE attr_list RBRACE VALUES LBRACE insert_value insert_value_list RBRACE  */
#line 738 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_INSERT);
      (yyval.sql_node)->insertion.relation_name = (yyvsp[-8].cstring);
      (yyval.sql_node)->insertion.attribute_names = std::move(*(yyvsp[-6].key_list));
      delete (yyvsp[-6].key_list);
      if ((yyvsp[-1].value_list) != nullptr) {
        (yyval.sql_node)->insertion.values.swap(*(yyvsp[-1].value_list));
        delete (yyvsp[-1].value_list);
      }
      (yyval.sql_node)->insertion.values.emplace_back(*(yyvsp[-2].value));
      reverse((yyval.sql_node)->insertion.values.begin(), (yyval.sql_node)->insertion.values.end());
      delete (yyvsp[-2].value);
    }
#line 2658 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 79: /* insert_value_list: %empty  */
#line 755 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.value_list) = nullptr;
    }
#line 2666 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 80: /* insert_value_list: COMMA insert_value insert_value_list  */
#line 758 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                            {
      if ((yyvsp[0].value_list) != nullptr) {
        (yyval.value_list) = (yyvsp[0].value_list);
      } else {
        (yyval.value_list) = new vector<Value>;
      }
      (yyval.value_list)->emplace_back(*(yyvsp[-1].value));
      delete (yyvsp[-1].value);
    }
#line 2680 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 82: /* insert_value: '-' NUMBER  */
#line 773 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                 {
      (yyval.value) = new Value(-(int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2689 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 83: /* insert_value: '-' FLOAT_LITERAL  */
#line 777 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                        {
      (yyval.value) = new Value(-(float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2698 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 84: /* insert_value: expression  */
#line 782 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                 {
      Value __v; RC __rc = (yyvsp[0].expression)->try_get_value(__v);
      if (__rc != RC::SUCCESS) {
        delete (yyvsp[0].expression); (yyvsp[0].expression) = nullptr;
        yyerror(&(yyloc), sql_string, sql_result, scanner, "Invalid constant expression in INSERT");
        YYERROR;
      }
      (yyval.value) = new Value(__v);
      delete (yyvsp[0].expression); (yyvsp[0].expression) = nullptr;
      (yyloc) = (yylsp[0]);
    }
#line 2714 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 85: /* value: NUMBER  */
#line 796 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
           {
      (yyval.value) = new Value((int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2723 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 86: /* value: FLOAT_LITERAL  */
#line 800 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                   {
      (yyval.value) = new Value((float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2732 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 87: /* value: NULL_T  */
#line 804 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
            {
      (yyval.value) = new Value();
      (yyval.value)->set_type(AttrType::UNDEFINED);
      (yyval.value)->set_null(true);
      (yyloc) = (yylsp[0]);
    }
#line 2743 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 88: /* value: vector_literal  */
#line 810 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                    {
      (yyval.value) = (yyvsp[0].value);
      (yyloc) = (yylsp[0]);
    }
#line 2752 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 89: /* value: SSS  */
#line 814 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
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
#line 2811 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 90: /* storage_format: %empty  */
#line 871 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.cstring) = nullptr;
    }
#line 2819 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 91: /* storage_format: STORAGE FORMAT EQ ID  */
#line 875 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.cstring) = (yyvsp[0].cstring);
    }
#line 2827 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 92: /* delete_stmt: DELETE FROM ID where  */
#line 882 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DELETE);
      (yyval.sql_node)->deletion.relation_name = (yyvsp[-1].cstring);
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.sql_node)->deletion.conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 2840 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 93: /* update_stmt: UPDATE ID SET update_kv_list where  */
#line 894 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_UPDATE);
      (yyval.sql_node)->update.relation_name = (yyvsp[-3].cstring);
      
      if ((yyvsp[-1].update_kv_list) && !(yyvsp[-1].update_kv_list)->empty()) {
        for (auto &kv : *(yyvsp[-1].update_kv_list)) {
          (yyval.sql_node)->update.attribute_names.push_back(kv.attribute_names);
          (yyval.sql_node)->update.values.push_back(kv.values);
          (yyval.sql_node)->update.value_is_sub_query.push_back(kv.value_is_sub_query);
          (yyval.sql_node)->update.value_sub_queries.push_back(std::move(kv.value_sub_queries));
        }
        delete (yyvsp[-1].update_kv_list);
      }
      
      if((yyval.sql_node)->update.value_is_sub_query.size() == 1){
        (yyval.sql_node)->update.relation_name = (yyvsp[-3].cstring);
        (yyval.sql_node)->update.attribute_name = (yyval.sql_node)->update.attribute_names.front();
        (yyval.sql_node)->update.value = (yyval.sql_node)->update.values.front();
        if((yyval.sql_node)->update.value_is_sub_query.front() == true){
          (yyval.sql_node)->update.is_sub_query = true;
          (yyval.sql_node)->update.sub_query = std::move((yyval.sql_node)->update.value_sub_queries.front());
        }
        else{
          (yyval.sql_node)->update.is_sub_query = false;
          (yyval.sql_node)->update.sub_query.reset(nullptr);
        }
        (yyval.sql_node)->update.attribute_names.clear();
      }

      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.sql_node)->update.conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 2879 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 94: /* update_kv_list: ID EQ value  */
#line 932 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.update_kv_list) = new vector<UpdateKvSqlNode>();
      (yyval.update_kv_list)->push_back({(yyvsp[-2].cstring), *(yyvsp[0].value),false,nullptr});
      delete (yyvsp[0].value);
    }
#line 2889 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 95: /* update_kv_list: update_kv_list COMMA ID EQ value  */
#line 938 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyvsp[-4].update_kv_list)->push_back({(yyvsp[-2].cstring), *(yyvsp[0].value),false,nullptr});
      (yyval.update_kv_list) = (yyvsp[-4].update_kv_list);
      delete (yyvsp[0].value);
    }
#line 2899 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 96: /* update_kv_list: ID EQ expression  */
#line 945 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
        cout<<"push subqry11"<<endl;

      (yyval.update_kv_list) = new vector<UpdateKvSqlNode>();

      if(auto && sub_qry = dynamic_cast<SubQueryExpr*>((yyvsp[0].expression))){
        auto && one = std::make_unique<ParsedSqlNode>(SCF_SELECT);
        one->selection = std::move(sub_qry->sub_query_node());
        cout<<"push subqry"<<endl;
        Value placeholder; placeholder.set_type(AttrType::UNDEFINED); placeholder.set_null(true);
        (yyval.update_kv_list)->push_back({(yyvsp[-2].cstring), placeholder,true,std::move(one)});
      }
      else{

        // 尝试在解析期将表达式折叠为常量；失败则报错以保持简化实现
        Value __v; RC __rc = (yyvsp[0].expression)->try_get_value(__v);
        if (__rc != RC::SUCCESS) {
          yyerror(&(yyloc), sql_string, sql_result, scanner, "Only constant or foldable expressions are allowed in UPDATE SET");
          delete (yyvsp[0].expression);
          YYERROR;
        }
        cout<<"push subqry22"<<endl;

        (yyval.update_kv_list)->push_back({(yyvsp[-2].cstring), __v,false,nullptr}); 
      }
      
      delete (yyvsp[0].expression);
    }
#line 2932 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 97: /* update_kv_list: update_kv_list COMMA ID EQ expression  */
#line 974 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {

      if(auto && sub_qry = dynamic_cast<SubQueryExpr*>((yyvsp[0].expression))){
        auto && one = std::make_unique<ParsedSqlNode>(SCF_SELECT);
        one->selection = std::move(sub_qry->sub_query_node());
        Value placeholder; placeholder.set_type(AttrType::UNDEFINED); placeholder.set_null(true);

        (yyvsp[-4].update_kv_list)->push_back({(yyvsp[-2].cstring), placeholder,true,std::move(one)});
      }
      else{

        // 叠加一个可折叠表达式
        Value __v; RC __rc = (yyvsp[0].expression)->try_get_value(__v);
        if (__rc != RC::SUCCESS) {
          yyerror(&(yyloc), sql_string, sql_result, scanner, "Only constant or foldable expressions are allowed in UPDATE SET");
          delete (yyvsp[0].expression);
          YYERROR;
        }
        
        (yyvsp[-4].update_kv_list)->push_back({(yyvsp[-2].cstring), __v,false,nullptr});
                
      }
      (yyval.update_kv_list) = (yyvsp[-4].update_kv_list);
      delete (yyvsp[0].expression);
    }
#line 2962 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 98: /* select_stmt: SELECT projection_list FROM rel_list join_list where group_by having order_by limit_clause  */
#line 1003 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SELECT);
      if ((yyvsp[-8].expression_list) != nullptr) {
        (yyval.sql_node)->selection.expressions.swap(*(yyvsp[-8].expression_list));
        delete (yyvsp[-8].expression_list);
      }

      if ((yyvsp[-6].relation_with_alias_list) != nullptr) {
        for (auto &it : *(yyvsp[-6].relation_with_alias_list)) {
          (yyval.sql_node)->selection.relations.push_back(it.table_name);
          (yyval.sql_node)->selection.relation_aliases.push_back(it.alias);
        }
        delete (yyvsp[-6].relation_with_alias_list);
      }

      if ((yyvsp[-5].join_list) != nullptr) {
        (yyval.sql_node)->selection.joins.swap(*(yyvsp[-5].join_list));
        delete (yyvsp[-5].join_list);
      }

      if ((yyvsp[-4].condition_list) != nullptr) {
        (yyval.sql_node)->selection.conditions.swap(*(yyvsp[-4].condition_list));
        delete (yyvsp[-4].condition_list);
      }

      if ((yyvsp[-3].expression_list) != nullptr) {
        (yyval.sql_node)->selection.group_by.swap(*(yyvsp[-3].expression_list));
        delete (yyvsp[-3].expression_list);
      }

      if ((yyvsp[-2].expression) != nullptr) {
        (yyval.sql_node)->selection.having_expr.reset((yyvsp[-2].expression));
      }

      if ((yyvsp[-1].order_by_list) != nullptr) {
        (yyval.sql_node)->selection.order_by.swap(*(yyvsp[-1].order_by_list));
        delete (yyvsp[-1].order_by_list);
      }
      // store optional LIMIT for later plan generation
      (yyval.sql_node)->selection.limit_count = (yyvsp[0].number);
    }
#line 3008 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 99: /* select_stmt: SELECT projection_list WHERE condition_list order_by limit_clause  */
#line 1045 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SELECT);
      if ((yyvsp[-4].expression_list) != nullptr) {
        (yyval.sql_node)->selection.expressions.swap(*(yyvsp[-4].expression_list));
        delete (yyvsp[-4].expression_list);
      }

      if ((yyvsp[-2].condition_list) != nullptr) {
        (yyval.sql_node)->selection.conditions.swap(*(yyvsp[-2].condition_list));
        delete (yyvsp[-2].condition_list);
      }

      if ((yyvsp[-1].order_by_list) != nullptr) {
        (yyval.sql_node)->selection.order_by.swap(*(yyvsp[-1].order_by_list));
        delete (yyvsp[-1].order_by_list);
      }
      (yyval.sql_node)->selection.limit_count = (yyvsp[0].number);
    }
#line 3031 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 100: /* select_stmt: SELECT projection_list order_by limit_clause  */
#line 1064 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SELECT);
      if ((yyvsp[-2].expression_list) != nullptr) {
        (yyval.sql_node)->selection.expressions.swap(*(yyvsp[-2].expression_list));
        delete (yyvsp[-2].expression_list);
      }

      if ((yyvsp[-1].order_by_list) != nullptr) {
        (yyval.sql_node)->selection.order_by.swap(*(yyvsp[-1].order_by_list));
        delete (yyvsp[-1].order_by_list);
      }
      (yyval.sql_node)->selection.limit_count = (yyvsp[0].number);
    }
#line 3049 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 101: /* union_stmt: select_stmt UNION select_stmt  */
#line 1081 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_UNION);
      (yyval.sql_node)->union_stmt.select_statements.emplace_back(std::make_unique<SelectSqlNode>(std::move((yyvsp[-2].sql_node)->selection)));
      (yyval.sql_node)->union_stmt.select_statements.emplace_back(std::make_unique<SelectSqlNode>(std::move((yyvsp[0].sql_node)->selection)));
      (yyval.sql_node)->union_stmt.distinct_flags.push_back(true);  // 第一个UNION去重
      delete (yyvsp[-2].sql_node);
      delete (yyvsp[0].sql_node);
    }
#line 3062 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 102: /* union_stmt: select_stmt UNION_ALL select_stmt  */
#line 1090 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_UNION);
      (yyval.sql_node)->union_stmt.select_statements.emplace_back(std::make_unique<SelectSqlNode>(std::move((yyvsp[-2].sql_node)->selection)));
      (yyval.sql_node)->union_stmt.select_statements.emplace_back(std::make_unique<SelectSqlNode>(std::move((yyvsp[0].sql_node)->selection)));
      (yyval.sql_node)->union_stmt.distinct_flags.push_back(false);  // 第一个UNION ALL不去重
      delete (yyvsp[-2].sql_node);
      delete (yyvsp[0].sql_node);
    }
#line 3075 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 103: /* union_stmt: union_stmt UNION select_stmt  */
#line 1099 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyvsp[-2].sql_node)->union_stmt.select_statements.emplace_back(std::make_unique<SelectSqlNode>(std::move((yyvsp[0].sql_node)->selection)));
      (yyvsp[-2].sql_node)->union_stmt.distinct_flags.push_back(true);  // 添加新的UNION去重标志
      (yyval.sql_node) = (yyvsp[-2].sql_node);
      delete (yyvsp[0].sql_node);
    }
#line 3086 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 104: /* union_stmt: union_stmt UNION_ALL select_stmt  */
#line 1106 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyvsp[-2].sql_node)->union_stmt.select_statements.emplace_back(std::make_unique<SelectSqlNode>(std::move((yyvsp[0].sql_node)->selection)));
      (yyvsp[-2].sql_node)->union_stmt.distinct_flags.push_back(false);  // 添加新的UNION ALL不去重标志
      (yyval.sql_node) = (yyvsp[-2].sql_node);
      delete (yyvsp[0].sql_node);
    }
#line 3097 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 105: /* union_stmt: LBRACE union_stmt RBRACE order_by limit_clause  */
#line 1113 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = (yyvsp[-3].sql_node);
      if ((yyvsp[-1].order_by_list) != nullptr) {
        (yyval.sql_node)->union_stmt.order_by.swap(*(yyvsp[-1].order_by_list));
        delete (yyvsp[-1].order_by_list);
      }
      (yyval.sql_node)->union_stmt.limit_count = (yyvsp[0].number);
    }
#line 3110 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 106: /* calc_stmt: CALC expression_list  */
#line 1125 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CALC);
      (yyval.sql_node)->calc.expressions.swap(*(yyvsp[0].expression_list));
      delete (yyvsp[0].expression_list);
    }
#line 3120 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 107: /* expression_list: expression  */
#line 1134 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      (yyval.expression_list)->emplace_back((yyvsp[0].expression));
    }
#line 3129 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 108: /* expression_list: expression COMMA expression_list  */
#line 1139 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      if ((yyvsp[0].expression_list) != nullptr) {
        (yyval.expression_list) = (yyvsp[0].expression_list);
      } else {
        (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      }
      (yyval.expression_list)->emplace((yyval.expression_list)->begin(), (yyvsp[-2].expression));
    }
#line 3142 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 109: /* projection_list: expression opt_as_alias  */
#line 1151 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      if ((yyvsp[0].cstring) != nullptr) {
        (yyvsp[-1].expression)->set_name((yyvsp[0].cstring));
        (yyvsp[-1].expression)->set_alias((yyvsp[0].cstring));
      }
      (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      (yyval.expression_list)->emplace_back((yyvsp[-1].expression));
    }
#line 3155 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 110: /* projection_list: expression opt_as_alias COMMA projection_list  */
#line 1160 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      if ((yyvsp[-2].cstring) != nullptr) {
        (yyvsp[-3].expression)->set_name((yyvsp[-2].cstring));
        (yyvsp[-3].expression)->set_alias((yyvsp[-2].cstring));
      }
      if ((yyvsp[0].expression_list) != nullptr) {
        (yyval.expression_list) = (yyvsp[0].expression_list);
      } else {
        (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      }
      (yyval.expression_list)->emplace((yyval.expression_list)->begin(), (yyvsp[-3].expression));
    }
#line 3172 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 111: /* opt_as_alias: %empty  */
#line 1175 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                { (yyval.cstring) = nullptr; }
#line 3178 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 112: /* opt_as_alias: ID  */
#line 1176 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
         { (yyval.cstring) = (yyvsp[0].cstring); }
#line 3184 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 113: /* opt_as_alias: AS ID  */
#line 1177 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
            { (yyval.cstring) = (yyvsp[0].cstring); }
#line 3190 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 114: /* expression: expression '+' expression  */
#line 1181 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                              {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::ADD, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 3198 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 115: /* expression: expression '-' expression  */
#line 1184 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::SUB, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 3206 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 116: /* expression: expression '*' expression  */
#line 1187 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::MUL, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 3214 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 117: /* expression: expression '/' expression  */
#line 1190 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::DIV, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 3222 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 118: /* expression: expression comp_op expression  */
#line 1193 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                    {
      (yyval.expression) = new ComparisonExpr((yyvsp[-1].comp), unique_ptr<Expression>((yyvsp[-2].expression)), unique_ptr<Expression>((yyvsp[0].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3231 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 119: /* expression: LBRACE expression RBRACE  */
#line 1197 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                               {
      // Simple parentheses for grouping - just return the inner expression
      (yyval.expression) = (yyvsp[-1].expression);
    }
#line 3240 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 120: /* expression: '-' expression  */
#line 1201 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                  {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::NEGATIVE, (yyvsp[0].expression), nullptr, sql_string, &(yyloc));
    }
#line 3248 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 121: /* expression: value  */
#line 1204 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
            {
      (yyval.expression) = new ValueExpr(*(yyvsp[0].value));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete (yyvsp[0].value);
    }
#line 3258 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 122: /* expression: ID LBRACE RBRACE  */
#line 1210 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                       {
      std::vector<std::unique_ptr<Expression>> args;
      (yyval.expression) = new FunctionExpr((yyvsp[-2].cstring), std::move(args));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3268 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 123: /* expression: ID LBRACE expression_list RBRACE  */
#line 1215 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                       {
      std::vector<std::unique_ptr<Expression>> args;
      if ((yyvsp[-1].expression_list) != nullptr) {
        args.swap(*(yyvsp[-1].expression_list));
        delete (yyvsp[-1].expression_list);
      }
      // 解析期维度不一致快速失败：仅对向量距离类函数进行
      // 函数名：DISTANCE/L2_DISTANCE/COSINE_DISTANCE/INNER_PRODUCT
      auto __is_distance = [&](const char *fn)->bool {
        return 0==strcasecmp(fn, "DISTANCE") || 0==strcasecmp(fn, "L2_DISTANCE") || 0==strcasecmp(fn, "COSINE_DISTANCE") || 0==strcasecmp(fn, "INNER_PRODUCT");
      };
      if (__is_distance((yyvsp[-3].cstring)) && args.size() >= 2 && args[0] && args[1]) {
        Value __v0, __v1;
        RC __rc0 = args[0]->try_get_value(__v0);
        RC __rc1 = args[1]->try_get_value(__v1);
        if (__rc0 == RC::SUCCESS && __rc1 == RC::SUCCESS &&
            __v0.attr_type() == AttrType::VECTORS && __v1.attr_type() == AttrType::VECTORS &&
            __v0.length() >= (int)sizeof(int) && __v1.length() >= (int)sizeof(int)) {
          int __d0 = *(const int *)__v0.data();
          int __d1 = *(const int *)__v1.data();
          if (__d0 != __d1) {
            yyerror(&(yyloc), sql_string, sql_result, scanner, "Vector dimensions mismatch in distance function");
            YYERROR;
          }
        }
      }
      (yyval.expression) = new FunctionExpr((yyvsp[-3].cstring), std::move(args));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3302 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 124: /* expression: rel_attr  */
#line 1244 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
               {
      RelAttrSqlNode *node = (yyvsp[0].rel_attr);
      (yyval.expression) = new UnboundFieldExpr(node->relation_name, node->attribute_name);
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete (yyvsp[0].rel_attr);
    }
#line 3313 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 125: /* expression: '*'  */
#line 1250 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
          {
      (yyval.expression) = new StarExpr();
    }
#line 3321 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 126: /* expression: aggregate_expr  */
#line 1253 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                     {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 3329 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 127: /* expression: sub_query_expr  */
#line 1256 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                     {
      (yyval.expression) = (yyvsp[0].expression);
      cout<<"sub_expr"<<endl;
    }
#line 3338 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 128: /* sub_query_expr: LBRACE SELECT projection_list FROM rel_list join_list where group_by having order_by RBRACE  */
#line 1264 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      fprintf(stderr, "DEBUG: [PARSER] Parsing scalar subquery with FROM\n");
      ParsedSqlNode *parsed_node = new ParsedSqlNode(SCF_SELECT);
      if ((yyvsp[-8].expression_list) != nullptr) {
        parsed_node->selection.expressions.swap(*(yyvsp[-8].expression_list));
        delete (yyvsp[-8].expression_list);
      }
      if ((yyvsp[-6].relation_with_alias_list) != nullptr) {
        for (auto &it : *(yyvsp[-6].relation_with_alias_list)) {
          parsed_node->selection.relations.push_back(it.table_name);
          parsed_node->selection.relation_aliases.push_back(it.alias);
        }
        delete (yyvsp[-6].relation_with_alias_list);
      }

      if ((yyvsp[-5].join_list) != nullptr) {
        parsed_node->selection.joins.swap(*(yyvsp[-5].join_list));
        delete (yyvsp[-5].join_list);
      }

      if ((yyvsp[-4].condition_list) != nullptr) {
        parsed_node->selection.conditions.swap(*(yyvsp[-4].condition_list));
        delete (yyvsp[-4].condition_list);
      }

      if ((yyvsp[-3].expression_list) != nullptr) {
        parsed_node->selection.group_by.swap(*(yyvsp[-3].expression_list));
        delete (yyvsp[-3].expression_list);
      }

      if ((yyvsp[-2].expression) != nullptr) {
        parsed_node->selection.having_expr.reset((yyvsp[-2].expression));
      }

      if ((yyvsp[-1].order_by_list) != nullptr) {
        parsed_node->selection.order_by.swap(*(yyvsp[-1].order_by_list));
        delete (yyvsp[-1].order_by_list);
      }

      fprintf(stderr, "DEBUG: [PARSER] Creating SubQueryExpr\n");
      std::unique_ptr<SelectSqlNode> select_node(new SelectSqlNode(std::move(parsed_node->selection)));
      (yyval.expression) = new SubQueryExpr(std::move(select_node));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete parsed_node;
      fprintf(stderr, "DEBUG: [PARSER] SubQueryExpr created successfully\n");
    }
#line 3389 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 129: /* sub_query_expr: LBRACE SELECT projection_list order_by RBRACE  */
#line 1311 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      fprintf(stderr, "DEBUG: [PARSER] Parsing scalar subquery without FROM\n");
      ParsedSqlNode *parsed_node = new ParsedSqlNode(SCF_SELECT);
      if ((yyvsp[-2].expression_list) != nullptr) {
        parsed_node->selection.expressions.swap(*(yyvsp[-2].expression_list));
        delete (yyvsp[-2].expression_list);
      }
      if ((yyvsp[-1].order_by_list) != nullptr) {
        parsed_node->selection.order_by.swap(*(yyvsp[-1].order_by_list));
        delete (yyvsp[-1].order_by_list);
      }

      fprintf(stderr, "DEBUG: [PARSER] Creating SubQueryExpr\n");
      std::unique_ptr<SelectSqlNode> select_node(new SelectSqlNode(std::move(parsed_node->selection)));
      (yyval.expression) = new SubQueryExpr(std::move(select_node));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete parsed_node;
      fprintf(stderr, "DEBUG: [PARSER] SubQueryExpr created successfully\n");
    }
#line 3413 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 130: /* rel_attr: ID  */
#line 1333 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
       {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->attribute_name = (yyvsp[0].cstring);
    }
#line 3422 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 131: /* rel_attr: ID DOT ID  */
#line 1337 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->relation_name  = (yyvsp[-2].cstring);
      (yyval.rel_attr)->attribute_name = (yyvsp[0].cstring);
    }
#line 3432 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 132: /* rel_attr: ID DOT '*'  */
#line 1342 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                 {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->relation_name  = (yyvsp[-2].cstring);
      (yyval.rel_attr)->attribute_name = (char*)"*";
    }
#line 3442 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 133: /* relation: ID  */
#line 1350 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
       {
      (yyval.relation_with_alias) = new RelationWithAliasSqlNode();
      (yyval.relation_with_alias)->table_name = (yyvsp[0].cstring);
    }
#line 3451 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 134: /* relation: ID ID  */
#line 1354 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
            {
      (yyval.relation_with_alias) = new RelationWithAliasSqlNode();
      (yyval.relation_with_alias)->table_name = (yyvsp[-1].cstring);
      (yyval.relation_with_alias)->alias = (yyvsp[0].cstring);
    }
#line 3461 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 135: /* relation: ID AS ID  */
#line 1359 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
               {
      (yyval.relation_with_alias) = new RelationWithAliasSqlNode();
      (yyval.relation_with_alias)->table_name = (yyvsp[-2].cstring);
      (yyval.relation_with_alias)->alias = (yyvsp[0].cstring);
    }
#line 3471 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 136: /* rel_list: relation  */
#line 1366 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
             {
      (yyval.relation_with_alias_list) = new vector<RelationWithAliasSqlNode>();
      (yyval.relation_with_alias_list)->emplace_back(std::move(*(yyvsp[0].relation_with_alias)));
      delete (yyvsp[0].relation_with_alias);
    }
#line 3481 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 137: /* rel_list: relation COMMA rel_list  */
#line 1371 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                              {
      if ((yyvsp[0].relation_with_alias_list) != nullptr) {
        (yyval.relation_with_alias_list) = (yyvsp[0].relation_with_alias_list);
      } else {
        (yyval.relation_with_alias_list) = new vector<RelationWithAliasSqlNode>();
      }
      (yyval.relation_with_alias_list)->emplace((yyval.relation_with_alias_list)->begin(), std::move(*(yyvsp[-2].relation_with_alias)));
      delete (yyvsp[-2].relation_with_alias);
    }
#line 3495 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 138: /* where: %empty  */
#line 1386 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 3503 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 139: /* where: WHERE condition_list  */
#line 1389 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                           {
      (yyval.condition_list) = (yyvsp[0].condition_list);
      printf("DEBUG: [YACC] WHERE condition_list: %zu conditions\n", (yyvsp[0].condition_list) ? (yyvsp[0].condition_list)->size() : 0);
      if ((yyvsp[0].condition_list)) {
        for (size_t i = 0; i < (yyvsp[0].condition_list)->size(); i++) {
          printf("DEBUG: [YACC] Condition %zu: conjunction_op = %d\n", i, (*(yyvsp[0].condition_list))[i].conjunction_op);
        }
      }
    }
#line 3517 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 140: /* condition_list: %empty  */
#line 1401 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 3525 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 141: /* condition_list: condition  */
#line 1404 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                {
      (yyval.condition_list) = new vector<ConditionSqlNode>;
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[0].condition)));
      delete (yyvsp[0].condition);
    }
#line 3535 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 142: /* condition_list: condition AND condition_list  */
#line 1409 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                   {
      (yyval.condition_list) = (yyvsp[0].condition_list);
      (yyvsp[-2].condition)->conjunction_op = AND_OP;  // 设置AND连接符
      printf("DEBUG: [YACC] Setting AND_OP for condition\n");
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[-2].condition)));
      delete (yyvsp[-2].condition);
    }
#line 3547 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 143: /* condition_list: condition OR condition_list  */
#line 1416 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                  {
      (yyval.condition_list) = (yyvsp[0].condition_list);
      (yyvsp[-2].condition)->conjunction_op = OR_OP;    // 设置OR连接符
      printf("DEBUG: [YACC] Setting OR_OP for condition\n");
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[-2].condition)));
      delete (yyvsp[-2].condition);
    }
#line 3559 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 144: /* condition: expression comp_op expression  */
#line 1426 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr.reset((yyvsp[-2].expression));
      (yyval.condition)->right_expr.reset((yyvsp[0].expression));
      (yyval.condition)->comp = (yyvsp[-1].comp);
      (yyval.condition)->conjunction_op = AND_OP;  // 默认初始化为AND_OP
    }
#line 3571 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 145: /* condition: expression IN LBRACE expression_list RBRACE  */
#line 1434 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
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
#line 3597 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 146: /* condition: expression NOT IN LBRACE expression_list RBRACE  */
#line 1456 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
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
#line 3623 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 147: /* condition: expression IN LBRACE sub_query RBRACE  */
#line 1478 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr.reset((yyvsp[-4].expression));
      //$$->sub_query.reset($4);
      unique_ptr<Expression> one = std::make_unique<SubQueryExpr>(std::move((yyvsp[-1].sql_node)->selection)); 
      (yyval.condition)->right_expr = std::move(one);
      (yyval.condition)->comp = IN_OP;
    }
#line 3636 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 148: /* condition: expression NOT IN LBRACE sub_query RBRACE  */
#line 1487 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr.reset((yyvsp[-5].expression));
      //$$->sub_query.reset($5);
      unique_ptr<Expression> one = std::make_unique<SubQueryExpr>(std::move((yyvsp[-1].sql_node)->selection)); 
      (yyval.condition)->right_expr = std::move(one);
      (yyval.condition)->comp = NOT_IN_OP;
    }
#line 3649 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 149: /* condition: expression IS NULL_T  */
#line 1496 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr.reset((yyvsp[-2].expression));
      Value __nullv; __nullv.set_type(AttrType::UNDEFINED); __nullv.set_null(true);
      (yyval.condition)->right_expr.reset(new ValueExpr(__nullv));
      (yyval.condition)->comp = IS_NULL_OP;
    }
#line 3661 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 150: /* condition: expression IS NOT NULL_T  */
#line 1504 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr.reset((yyvsp[-3].expression));
      Value __nullv2; __nullv2.set_type(AttrType::UNDEFINED); __nullv2.set_null(true);
      (yyval.condition)->right_expr.reset(new ValueExpr(__nullv2));
      (yyval.condition)->comp = IS_NOT_NULL_OP;
    }
#line 3673 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 151: /* condition: EXISTS LBRACE sub_query RBRACE  */
#line 1512 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      // $$->sub_query.reset($3);
      unique_ptr<Expression> one = std::make_unique<SubQueryExpr>(std::move((yyvsp[-1].sql_node)->selection)); 
      (yyval.condition)->right_expr = std::move(one);
      (yyval.condition)->left_expr = std::make_unique<NothingExpr>();
      (yyval.condition)->comp = EXISTS_OP;
    }
#line 3686 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 152: /* condition: NOT EXISTS LBRACE sub_query RBRACE  */
#line 1521 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      //$$->sub_query.reset($4);
      unique_ptr<Expression> one = std::make_unique<SubQueryExpr>(std::move((yyvsp[-1].sql_node)->selection)); 
      (yyval.condition)->right_expr = std::move(one);
      (yyval.condition)->left_expr = std::make_unique<NothingExpr>();

      (yyval.condition)->comp = NOT_EXISTS_OP;
    }
#line 3700 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 153: /* comp_op: EQ  */
#line 1533 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
         { (yyval.comp) = EQUAL_TO; }
#line 3706 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 154: /* comp_op: LT  */
#line 1534 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
         { (yyval.comp) = LESS_THAN; }
#line 3712 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 155: /* comp_op: GT  */
#line 1535 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
         { (yyval.comp) = GREAT_THAN; }
#line 3718 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 156: /* comp_op: LE  */
#line 1536 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
         { (yyval.comp) = LESS_EQUAL; }
#line 3724 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 157: /* comp_op: GE  */
#line 1537 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
         { (yyval.comp) = GREAT_EQUAL; }
#line 3730 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 158: /* comp_op: NE  */
#line 1538 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
         { (yyval.comp) = NOT_EQUAL; }
#line 3736 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 159: /* comp_op: LIKE  */
#line 1539 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
           { (yyval.comp) = LIKE_OP; }
#line 3742 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 160: /* comp_op: NOT LIKE  */
#line 1540 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
               { (yyval.comp) = NOT_LIKE_OP; }
#line 3748 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 161: /* comp_op: IN  */
#line 1541 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
         { (yyval.comp) = IN_OP; }
#line 3754 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 162: /* comp_op: NOT IN  */
#line 1542 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
             { (yyval.comp) = NOT_IN_OP; }
#line 3760 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 163: /* group_by: %empty  */
#line 1548 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.expression_list) = nullptr;
    }
#line 3768 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 164: /* group_by: GROUP BY expression_list  */
#line 1552 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.expression_list) = (yyvsp[0].expression_list);
    }
#line 3776 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 165: /* having: %empty  */
#line 1559 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.expression) = nullptr;
    }
#line 3784 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 166: /* having: HAVING expression  */
#line 1563 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 3792 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 167: /* load_data_stmt: LOAD DATA INFILE SSS INTO TABLE ID  */
#line 1569 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      char *tmp_file_name = common::substr((yyvsp[-3].cstring), 1, strlen((yyvsp[-3].cstring)) - 2);

      (yyval.sql_node) = new ParsedSqlNode(SCF_LOAD_DATA);
      (yyval.sql_node)->load_data.relation_name = (yyvsp[0].cstring);
      (yyval.sql_node)->load_data.file_name = tmp_file_name;
      free(tmp_file_name);
    }
#line 3805 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 168: /* explain_stmt: EXPLAIN command_wrapper  */
#line 1581 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXPLAIN);
      (yyval.sql_node)->explain.sql_node = unique_ptr<ParsedSqlNode>((yyvsp[0].sql_node));
    }
#line 3814 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 169: /* set_variable_stmt: SET ID EQ value  */
#line 1589 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SET_VARIABLE);
      (yyval.sql_node)->set_variable.name  = (yyvsp[-2].cstring);
      (yyval.sql_node)->set_variable.value = *(yyvsp[0].value);
      delete (yyvsp[0].value);
    }
#line 3825 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 170: /* vector_literal: LSQUARE number_list RSQUARE  */
#line 1598 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
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
#line 3865 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 171: /* number_list: float_number  */
#line 1636 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                 {
      (yyval.float_list) = new std::vector<double>();
      (yyval.float_list)->push_back((yyvsp[0].floats));
    }
#line 3874 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 172: /* number_list: number_list COMMA float_number  */
#line 1640 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                     {
      (yyval.float_list) = (yyvsp[-2].float_list);
      (yyval.float_list)->push_back((yyvsp[0].floats));
    }
#line 3883 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 173: /* order_by: %empty  */
#line 1648 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.order_by_list) = nullptr;
    }
#line 3891 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 174: /* order_by: ORDER BY order_by_item_list  */
#line 1652 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.order_by_list) = (yyvsp[0].order_by_list);
    }
#line 3899 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 175: /* order_by_item_list: order_by_item  */
#line 1659 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.order_by_list) = new std::vector<OrderBySqlNode>();
      (yyval.order_by_list)->emplace_back(std::move(*(yyvsp[0].order_by_item)));
      delete (yyvsp[0].order_by_item);
    }
#line 3909 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 176: /* order_by_item_list: order_by_item_list COMMA order_by_item  */
#line 1665 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.order_by_list) = (yyvsp[-2].order_by_list);
      (yyval.order_by_list)->emplace_back(std::move(*(yyvsp[0].order_by_item)));
      delete (yyvsp[0].order_by_item);
    }
#line 3919 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 177: /* order_by_item: expression  */
#line 1674 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.order_by_item) = new OrderBySqlNode();
      (yyval.order_by_item)->expression = unique_ptr<Expression>((yyvsp[0].expression));
      (yyval.order_by_item)->direction = ASC_ORDER;  // 默认升序
    }
#line 3929 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 178: /* order_by_item: expression ASC  */
#line 1680 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.order_by_item) = new OrderBySqlNode();
      (yyval.order_by_item)->expression = unique_ptr<Expression>((yyvsp[-1].expression));
      (yyval.order_by_item)->direction = ASC_ORDER;
    }
#line 3939 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 179: /* order_by_item: expression DESC  */
#line 1686 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.order_by_item) = new OrderBySqlNode();
      (yyval.order_by_item)->expression = unique_ptr<Expression>((yyvsp[-1].expression));
      (yyval.order_by_item)->direction = DESC_ORDER;
    }
#line 3949 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 180: /* limit_clause: %empty  */
#line 1694 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                { (yyval.number) = 0; }
#line 3955 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 181: /* limit_clause: LIMIT number  */
#line 1695 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                   { (yyval.number) = (yyvsp[0].number); }
#line 3961 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 182: /* option_list: %empty  */
#line 1700 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                { (yyval.number) = 0; }
#line 3967 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 183: /* option_list: option_item  */
#line 1701 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                  { (yyval.number) = 0; }
#line 3973 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 184: /* option_list: option_list COMMA option_item  */
#line 1702 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                    { (yyval.number) = 0; }
#line 3979 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 185: /* option_item: ID EQ ID  */
#line 1706 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
             { (yyval.number) = 0; }
#line 3985 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 186: /* option_item: ID EQ NUMBER  */
#line 1707 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                   { (yyval.number) = 0; }
#line 3991 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 187: /* option_item: ID EQ FLOAT_LITERAL  */
#line 1708 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                          { (yyval.number) = 0; }
#line 3997 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 188: /* count_expr: COUNT LBRACE expression RBRACE  */
#line 1712 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
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
#line 4012 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 189: /* aggregate_expr: count_expr  */
#line 1725 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
               {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 4020 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 190: /* aggregate_expr: SUM LBRACE expression RBRACE  */
#line 1728 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("sum", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 4029 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 191: /* aggregate_expr: MAX LBRACE expression RBRACE  */
#line 1732 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("max", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 4038 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 192: /* aggregate_expr: MIN LBRACE expression RBRACE  */
#line 1736 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("min", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 4047 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 193: /* aggregate_expr: AVG LBRACE expression RBRACE  */
#line 1740 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("avg", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 4056 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 194: /* join_list: %empty  */
#line 1748 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.join_list) = nullptr;
    }
#line 4064 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 195: /* join_list: join_list join_item  */
#line 1752 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      if ((yyvsp[-1].join_list) != nullptr) {
        (yyval.join_list) = (yyvsp[-1].join_list);
      } else {
        (yyval.join_list) = new std::vector<JoinSqlNode>();
      }
      (yyval.join_list)->emplace_back(std::move(*(yyvsp[0].join_item)));
      delete (yyvsp[0].join_item);
    }
#line 4078 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 196: /* join_item: INNER JOIN ID ON condition_list  */
#line 1765 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.join_item) = new JoinSqlNode();
      (yyval.join_item)->table_name = (yyvsp[-2].cstring);
      (yyval.join_item)->join_type = INNER_JOIN;
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.join_item)->conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 4092 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 197: /* join_item: INNER JOIN ID ID ON condition_list  */
#line 1775 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
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
#line 4107 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 198: /* join_item: INNER JOIN ID AS ID ON condition_list  */
#line 1786 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
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
#line 4122 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 199: /* join_item: JOIN ID ON condition_list  */
#line 1797 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.join_item) = new JoinSqlNode();
      (yyval.join_item)->table_name = (yyvsp[-2].cstring);
      (yyval.join_item)->join_type = INNER_JOIN;  // 默认为INNER JOIN
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.join_item)->conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 4136 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 200: /* join_item: JOIN ID ID ON condition_list  */
#line 1807 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
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
#line 4151 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 201: /* join_item: JOIN ID AS ID ON condition_list  */
#line 1818 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
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
#line 4166 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;

  case 202: /* sub_query: select_stmt  */
#line 1832 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = (yyvsp[0].sql_node);
    }
#line 4174 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"
    break;


#line 4178 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.cpp"

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

#line 1840 "/home/hxl/Desktop/oceambase/miniob-competition-2025/src/observer/sql/parser/yacc_sql.y"

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
