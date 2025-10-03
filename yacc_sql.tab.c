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
#line 2 "src/observer/sql/parser/yacc_sql.y"


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


#line 126 "yacc_sql.tab.c"

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

#include "yacc_sql.tab.h"
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
  YYSYMBOL_IF = 20,                        /* IF  */
  YYSYMBOL_EXISTS = 21,                    /* EXISTS  */
  YYSYMBOL_CALC = 22,                      /* CALC  */
  YYSYMBOL_SELECT = 23,                    /* SELECT  */
  YYSYMBOL_DESC = 24,                      /* DESC  */
  YYSYMBOL_SHOW = 25,                      /* SHOW  */
  YYSYMBOL_SYNC = 26,                      /* SYNC  */
  YYSYMBOL_INSERT = 27,                    /* INSERT  */
  YYSYMBOL_DELETE = 28,                    /* DELETE  */
  YYSYMBOL_UPDATE = 29,                    /* UPDATE  */
  YYSYMBOL_LBRACE = 30,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 31,                    /* RBRACE  */
  YYSYMBOL_LSQUARE = 32,                   /* LSQUARE  */
  YYSYMBOL_RSQUARE = 33,                   /* RSQUARE  */
  YYSYMBOL_COMMA = 34,                     /* COMMA  */
  YYSYMBOL_TRX_BEGIN = 35,                 /* TRX_BEGIN  */
  YYSYMBOL_TRX_COMMIT = 36,                /* TRX_COMMIT  */
  YYSYMBOL_TRX_ROLLBACK = 37,              /* TRX_ROLLBACK  */
  YYSYMBOL_INT_T = 38,                     /* INT_T  */
  YYSYMBOL_STRING_T = 39,                  /* STRING_T  */
  YYSYMBOL_FLOAT_T = 40,                   /* FLOAT_T  */
  YYSYMBOL_DATE_T = 41,                    /* DATE_T  */
  YYSYMBOL_VECTOR_T = 42,                  /* VECTOR_T  */
  YYSYMBOL_TEXT_T = 43,                    /* TEXT_T  */
  YYSYMBOL_HELP = 44,                      /* HELP  */
  YYSYMBOL_EXIT = 45,                      /* EXIT  */
  YYSYMBOL_DOT = 46,                       /* DOT  */
  YYSYMBOL_INTO = 47,                      /* INTO  */
  YYSYMBOL_VALUES = 48,                    /* VALUES  */
  YYSYMBOL_FROM = 49,                      /* FROM  */
  YYSYMBOL_WHERE = 50,                     /* WHERE  */
  YYSYMBOL_AND = 51,                       /* AND  */
  YYSYMBOL_OR = 52,                        /* OR  */
  YYSYMBOL_JOIN = 53,                      /* JOIN  */
  YYSYMBOL_INNER = 54,                     /* INNER  */
  YYSYMBOL_ON = 55,                        /* ON  */
  YYSYMBOL_NOT = 56,                       /* NOT  */
  YYSYMBOL_NULL_T = 57,                    /* NULL_T  */
  YYSYMBOL_LIKE = 58,                      /* LIKE  */
  YYSYMBOL_SET = 59,                       /* SET  */
  YYSYMBOL_LOAD = 60,                      /* LOAD  */
  YYSYMBOL_DATA = 61,                      /* DATA  */
  YYSYMBOL_INFILE = 62,                    /* INFILE  */
  YYSYMBOL_EXPLAIN = 63,                   /* EXPLAIN  */
  YYSYMBOL_STORAGE = 64,                   /* STORAGE  */
  YYSYMBOL_FORMAT = 65,                    /* FORMAT  */
  YYSYMBOL_PRIMARY = 66,                   /* PRIMARY  */
  YYSYMBOL_KEY = 67,                       /* KEY  */
  YYSYMBOL_ANALYZE = 68,                   /* ANALYZE  */
  YYSYMBOL_IN = 69,                        /* IN  */
  YYSYMBOL_IS = 70,                        /* IS  */
  YYSYMBOL_UNIQUE = 71,                    /* UNIQUE  */
  YYSYMBOL_VIEW = 72,                      /* VIEW  */
  YYSYMBOL_EQ = 73,                        /* EQ  */
  YYSYMBOL_LT = 74,                        /* LT  */
  YYSYMBOL_GT = 75,                        /* GT  */
  YYSYMBOL_LE = 76,                        /* LE  */
  YYSYMBOL_GE = 77,                        /* GE  */
  YYSYMBOL_NE = 78,                        /* NE  */
  YYSYMBOL_NUMBER = 79,                    /* NUMBER  */
  YYSYMBOL_FLOAT_LITERAL = 80,             /* FLOAT_LITERAL  */
  YYSYMBOL_ID = 81,                        /* ID  */
  YYSYMBOL_SSS = 82,                       /* SSS  */
  YYSYMBOL_83_ = 83,                       /* '+'  */
  YYSYMBOL_84_ = 84,                       /* '-'  */
  YYSYMBOL_85_ = 85,                       /* '*'  */
  YYSYMBOL_86_ = 86,                       /* '/'  */
  YYSYMBOL_UMINUS = 87,                    /* UMINUS  */
  YYSYMBOL_YYACCEPT = 88,                  /* $accept  */
  YYSYMBOL_commands = 89,                  /* commands  */
  YYSYMBOL_command_wrapper = 90,           /* command_wrapper  */
  YYSYMBOL_exit_stmt = 91,                 /* exit_stmt  */
  YYSYMBOL_help_stmt = 92,                 /* help_stmt  */
  YYSYMBOL_sync_stmt = 93,                 /* sync_stmt  */
  YYSYMBOL_begin_stmt = 94,                /* begin_stmt  */
  YYSYMBOL_commit_stmt = 95,               /* commit_stmt  */
  YYSYMBOL_rollback_stmt = 96,             /* rollback_stmt  */
  YYSYMBOL_drop_table_stmt = 97,           /* drop_table_stmt  */
  YYSYMBOL_analyze_table_stmt = 98,        /* analyze_table_stmt  */
  YYSYMBOL_show_tables_stmt = 99,          /* show_tables_stmt  */
  YYSYMBOL_show_index_stmt = 100,          /* show_index_stmt  */
  YYSYMBOL_desc_table_stmt = 101,          /* desc_table_stmt  */
  YYSYMBOL_create_index_stmt = 102,        /* create_index_stmt  */
  YYSYMBOL_drop_index_stmt = 103,          /* drop_index_stmt  */
  YYSYMBOL_create_table_stmt = 104,        /* create_table_stmt  */
  YYSYMBOL_create_view_stmt = 105,         /* create_view_stmt  */
  YYSYMBOL_drop_view_stmt = 106,           /* drop_view_stmt  */
  YYSYMBOL_attr_def_list = 107,            /* attr_def_list  */
  YYSYMBOL_attr_def = 108,                 /* attr_def  */
  YYSYMBOL_number = 109,                   /* number  */
  YYSYMBOL_float_number = 110,             /* float_number  */
  YYSYMBOL_type = 111,                     /* type  */
  YYSYMBOL_primary_key = 112,              /* primary_key  */
  YYSYMBOL_attr_list = 113,                /* attr_list  */
  YYSYMBOL_insert_stmt = 114,              /* insert_stmt  */
  YYSYMBOL_insert_value_list = 115,        /* insert_value_list  */
  YYSYMBOL_insert_value = 116,             /* insert_value  */
  YYSYMBOL_value = 117,                    /* value  */
  YYSYMBOL_storage_format = 118,           /* storage_format  */
  YYSYMBOL_delete_stmt = 119,              /* delete_stmt  */
  YYSYMBOL_update_stmt = 120,              /* update_stmt  */
  YYSYMBOL_update_set_list = 121,          /* update_set_list  */
  YYSYMBOL_select_stmt = 122,              /* select_stmt  */
  YYSYMBOL_calc_stmt = 123,                /* calc_stmt  */
  YYSYMBOL_expression_list = 124,          /* expression_list  */
  YYSYMBOL_projection_list = 125,          /* projection_list  */
  YYSYMBOL_opt_as_alias = 126,             /* opt_as_alias  */
  YYSYMBOL_expression = 127,               /* expression  */
  YYSYMBOL_sub_query_expr = 128,           /* sub_query_expr  */
  YYSYMBOL_rel_attr = 129,                 /* rel_attr  */
  YYSYMBOL_relation = 130,                 /* relation  */
  YYSYMBOL_rel_list = 131,                 /* rel_list  */
  YYSYMBOL_where = 132,                    /* where  */
  YYSYMBOL_condition_list = 133,           /* condition_list  */
  YYSYMBOL_condition = 134,                /* condition  */
  YYSYMBOL_comp_op = 135,                  /* comp_op  */
  YYSYMBOL_group_by = 136,                 /* group_by  */
  YYSYMBOL_having = 137,                   /* having  */
  YYSYMBOL_load_data_stmt = 138,           /* load_data_stmt  */
  YYSYMBOL_explain_stmt = 139,             /* explain_stmt  */
  YYSYMBOL_set_variable_stmt = 140,        /* set_variable_stmt  */
  YYSYMBOL_vector_literal = 141,           /* vector_literal  */
  YYSYMBOL_number_list = 142,              /* number_list  */
  YYSYMBOL_order_by = 143,                 /* order_by  */
  YYSYMBOL_order_by_item_list = 144,       /* order_by_item_list  */
  YYSYMBOL_order_by_item = 145,            /* order_by_item  */
  YYSYMBOL_count_expr = 146,               /* count_expr  */
  YYSYMBOL_aggregate_expr = 147,           /* aggregate_expr  */
  YYSYMBOL_join_list = 148,                /* join_list  */
  YYSYMBOL_join_item = 149,                /* join_item  */
  YYSYMBOL_sub_query = 150,                /* sub_query  */
  YYSYMBOL_opt_semicolon = 151             /* opt_semicolon  */
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
#define YYFINAL  87
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   686

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  88
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  64
/* YYNRULES -- Number of rules.  */
#define YYNRULES  179
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  366

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   338


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
       2,     2,    85,    83,     2,    84,     2,    86,     2,     2,
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
      75,    76,    77,    78,    79,    80,    81,    82,    87
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   253,   253,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   288,   294,   299,
     305,   311,   317,   323,   328,   335,   342,   348,   356,   363,
     375,   390,   398,   422,   430,   457,   465,   478,   487,   490,
     503,   516,   533,   546,   561,   580,   597,   601,   602,   605,
     606,   607,   608,   609,   610,   614,   617,   624,   628,   640,
     656,   659,   673,   674,   678,   685,   689,   693,   699,   703,
     757,   760,   767,   778,   803,   818,   819,   831,   871,   889,
     904,   913,   918,   930,   938,   953,   954,   955,   959,   962,
     965,   968,   971,   975,   979,   982,   988,   993,  1002,  1008,
    1011,  1014,  1020,  1067,  1090,  1094,  1102,  1106,  1111,  1118,
    1123,  1138,  1141,  1153,  1156,  1161,  1168,  1177,  1185,  1207,
    1229,  1236,  1243,  1251,  1259,  1265,  1274,  1275,  1276,  1277,
    1278,  1279,  1280,  1281,  1282,  1283,  1289,  1292,  1300,  1303,
    1309,  1321,  1329,  1339,  1377,  1381,  1389,  1392,  1399,  1405,
    1414,  1420,  1426,  1435,  1448,  1451,  1455,  1459,  1463,  1471,
    1474,  1487,  1497,  1508,  1519,  1529,  1540,  1554,  1560,  1561
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
  "SUM", "MAX", "MIN", "AVG", "TABLE", "TABLES", "INDEX", "IF", "EXISTS",
  "CALC", "SELECT", "DESC", "SHOW", "SYNC", "INSERT", "DELETE", "UPDATE",
  "LBRACE", "RBRACE", "LSQUARE", "RSQUARE", "COMMA", "TRX_BEGIN",
  "TRX_COMMIT", "TRX_ROLLBACK", "INT_T", "STRING_T", "FLOAT_T", "DATE_T",
  "VECTOR_T", "TEXT_T", "HELP", "EXIT", "DOT", "INTO", "VALUES", "FROM",
  "WHERE", "AND", "OR", "JOIN", "INNER", "ON", "NOT", "NULL_T", "LIKE",
  "SET", "LOAD", "DATA", "INFILE", "EXPLAIN", "STORAGE", "FORMAT",
  "PRIMARY", "KEY", "ANALYZE", "IN", "IS", "UNIQUE", "VIEW", "EQ", "LT",
  "GT", "LE", "GE", "NE", "NUMBER", "FLOAT_LITERAL", "ID", "SSS", "'+'",
  "'-'", "'*'", "'/'", "UMINUS", "$accept", "commands", "command_wrapper",
  "exit_stmt", "help_stmt", "sync_stmt", "begin_stmt", "commit_stmt",
  "rollback_stmt", "drop_table_stmt", "analyze_table_stmt",
  "show_tables_stmt", "show_index_stmt", "desc_table_stmt",
  "create_index_stmt", "drop_index_stmt", "create_table_stmt",
  "create_view_stmt", "drop_view_stmt", "attr_def_list", "attr_def",
  "number", "float_number", "type", "primary_key", "attr_list",
  "insert_stmt", "insert_value_list", "insert_value", "value",
  "storage_format", "delete_stmt", "update_stmt", "update_set_list",
  "select_stmt", "calc_stmt", "expression_list", "projection_list",
  "opt_as_alias", "expression", "sub_query_expr", "rel_attr", "relation",
  "rel_list", "where", "condition_list", "condition", "comp_op",
  "group_by", "having", "load_data_stmt", "explain_stmt",
  "set_variable_stmt", "vector_literal", "number_list", "order_by",
  "order_by_item_list", "order_by_item", "count_expr", "aggregate_expr",
  "join_list", "join_item", "sub_query", "opt_semicolon", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-230)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-103)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     388,    32,     9,   324,   324,   -20,    19,  -230,    18,    56,
      27,  -230,  -230,  -230,  -230,  -230,    37,    67,   388,   109,
     142,   128,  -230,  -230,  -230,  -230,  -230,  -230,  -230,  -230,
    -230,  -230,  -230,  -230,  -230,  -230,  -230,  -230,  -230,  -230,
    -230,  -230,  -230,  -230,  -230,  -230,    71,    74,   127,    75,
      -1,    76,    78,   118,   123,   134,   135,   137,   214,    22,
    -230,  -230,  -230,   -14,  -230,   324,  -230,  -230,  -230,   531,
    -230,  -230,  -230,  -230,  -230,     3,   116,  -230,  -230,   119,
      88,    90,   111,   100,   115,  -230,    99,  -230,  -230,  -230,
       6,   126,   101,    28,   162,  -230,   129,  -230,   324,   324,
     324,   324,   324,   324,   289,  -230,  -230,  -230,    91,   243,
     106,  -230,   324,   -28,  -230,  -230,  -230,  -230,  -230,  -230,
    -230,  -230,   324,   324,   324,   324,   324,   191,   122,   204,
    -230,   124,  -230,   172,   133,   148,   158,   140,   -11,   130,
    -230,   186,   141,   143,   168,   186,   150,   151,   152,   396,
     419,   452,   475,   508,     5,  -230,  -230,    22,  -230,   207,
    -230,  -230,  -230,  -230,   -22,   -22,   179,   179,   600,   324,
      13,   205,  -230,   211,   221,   554,   234,    89,  -230,   324,
    -230,   215,   204,  -230,   174,  -230,   201,  -230,    72,  -230,
     219,   169,  -230,   218,   222,  -230,  -230,  -230,  -230,  -230,
    -230,  -230,   122,   223,  -230,  -230,     1,   228,  -230,   182,
    -230,   122,    85,   186,   235,    14,   236,    80,   324,  -230,
     204,   204,  -230,    50,  -230,   131,   247,  -230,  -230,  -230,
    -230,  -230,  -230,   -12,   233,   150,   238,   150,   258,  -230,
    -230,  -230,  -230,   324,  -230,  -230,   189,   224,   265,  -230,
    -230,   248,   186,   251,   303,   225,  -230,   577,  -230,  -230,
      65,   244,  -230,   186,   253,   209,   212,   240,  -230,   -31,
     261,   270,   150,  -230,   186,    85,  -230,    11,   227,   299,
     296,  -230,   274,   303,   278,   279,  -230,  -230,  -230,    50,
     280,   281,   232,   158,  -230,  -230,   283,  -230,   254,  -230,
      -8,  -230,   298,  -230,   265,   249,   204,   276,    12,   324,
     324,   234,  -230,   301,   310,  -230,  -230,   244,  -230,   158,
     269,  -230,    94,   304,   284,  -230,  -230,  -230,   296,   288,
    -230,   204,   263,   204,   291,  -230,   600,  -230,  -230,  -230,
    -230,  -230,   -11,   293,  -230,   150,   275,   234,   204,  -230,
     297,  -230,   204,   253,  -230,   320,   272,   326,  -230,   204,
    -230,  -230,  -230,  -230,  -230,  -230
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,    29,     0,     0,
       0,    30,    31,    32,    28,    27,     0,     0,     0,     0,
       0,   178,    26,    25,    16,    17,    18,    19,     9,    10,
      11,    12,    13,    14,    15,     8,    23,    24,     5,     7,
       6,     4,     3,    20,    21,    22,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      77,    75,    76,   114,    79,     0,   109,   105,    90,    91,
     111,   108,    78,   164,   110,   156,    95,    38,    36,     0,
       0,     0,     0,     0,     0,   151,     0,     1,   179,     2,
       0,     0,     0,     0,     0,    33,     0,    47,     0,     0,
       0,     0,     0,     0,     0,    57,    58,   154,     0,     0,
       0,   104,     0,     0,   142,   144,   136,   137,   138,   139,
     140,   141,     0,     0,     0,     0,     0,     0,     0,   123,
      89,     0,    96,    93,     0,     0,   121,     0,     0,     0,
      35,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   156,   103,   153,     0,   106,     0,
     115,    92,   143,   145,    98,    99,   100,   101,   102,     0,
     116,   119,   169,     0,     0,     0,   156,   124,    97,     0,
      37,     0,   123,    82,     0,   152,     0,    43,     0,    48,
       0,     0,    45,    67,     0,    34,    41,   163,   165,   166,
     167,   168,     0,     0,   155,   107,   160,   157,   158,     0,
     117,     0,   121,     0,     0,     0,   144,     0,     0,    88,
     123,   123,    94,     0,   122,     0,     0,    59,    60,    61,
      62,    63,    64,    53,    65,     0,     0,     0,     0,   169,
     113,   161,   162,     0,   118,   120,     0,     0,   146,   170,
     177,     0,     0,   145,     0,     0,   132,   127,   125,   126,
       0,    70,    72,     0,    85,     0,     0,     0,    54,     0,
       0,     0,     0,    68,     0,   121,   159,     0,     0,     0,
     148,   134,     0,     0,     0,     0,   133,    73,    74,     0,
       0,     0,     0,   121,   150,    56,     0,    55,     0,    49,
      80,    39,     0,    46,   146,     0,   123,     0,     0,     0,
       0,   156,   135,     0,     0,   128,   130,    70,    69,   121,
       0,    83,    50,     0,     0,    42,    44,    40,   148,     0,
     174,   123,     0,   123,     0,   147,   149,    87,   129,   131,
      71,    84,     0,     0,    51,     0,     0,   156,   123,   175,
       0,   171,   123,    85,    52,     0,     0,     0,   176,   123,
     172,    86,    66,    81,   112,   173
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -230,  -230,   337,  -230,  -230,  -230,  -230,  -230,  -230,  -230,
    -230,  -230,  -230,  -230,  -230,  -230,  -230,  -230,  -230,  -230,
      92,  -230,   202,  -230,  -230,  -229,  -230,    51,    81,  -135,
    -230,  -230,  -230,    16,     2,  -230,  -105,   -90,  -230,    -3,
    -230,  -230,  -230,  -169,  -202,  -173,  -230,   196,    73,    48,
    -230,  -230,  -230,  -230,  -230,  -149,  -230,   136,  -230,  -230,
     139,  -230,  -223,  -230
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,   234,
     189,   296,   107,   233,   270,   194,    38,   290,   261,    67,
     325,    39,    40,   293,   250,    42,    68,    75,   133,   175,
      70,    71,   171,   172,   183,   176,   177,   126,   280,   311,
      43,    44,    45,    72,   108,   130,   207,   208,    73,    74,
     212,   249,   251,    89
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      69,    76,    41,   185,   159,   203,   271,   161,   273,   224,
     248,   241,   127,   154,   127,     4,   109,   141,   266,    94,
      41,    59,   305,   332,   209,   242,    50,   219,    51,   282,
     162,   285,   110,   239,   113,   298,   142,    78,    79,   145,
     291,   163,   245,   302,   267,   268,    60,   258,   259,    46,
     188,    47,   128,   129,   202,   104,   324,   113,   146,   114,
     314,    77,   111,   124,   125,    80,   306,   333,    61,    62,
     115,    64,   162,   304,   116,   117,   118,   119,   120,   121,
      95,    52,    59,   253,   122,   123,   124,   125,   262,   222,
     264,   321,   307,   334,   210,   149,   150,   151,   152,   153,
      76,   105,   106,    48,    49,    81,    69,    60,    82,    69,
     227,   228,   229,   230,   231,   232,   355,   341,    83,   164,
     165,   166,   167,   168,   156,   157,    86,   131,    84,    61,
      62,    88,    64,   330,   260,   182,   255,   256,   246,   247,
     220,   221,    87,   187,   287,   288,    92,   192,    98,   284,
     343,   344,    90,    99,   262,    91,    93,    96,   349,    97,
     351,   263,   337,    59,   100,   101,   206,   102,   134,   135,
     137,   136,   113,   138,   114,   358,    76,   139,   313,   360,
     140,   143,   144,   147,   148,   115,   365,   160,    60,   116,
     117,   118,   119,   120,   121,   169,   181,   132,   357,   122,
     123,   124,   125,   170,   335,   178,   179,   353,   182,     4,
      61,    62,   186,    64,   180,   257,    53,    54,    55,    56,
      57,   184,   188,   191,   190,   173,    53,    54,    55,    56,
      57,   193,   195,   196,    58,   113,    59,   103,   205,   211,
     206,   213,   214,   127,    58,   223,    59,   225,   226,   235,
     236,    69,   237,   238,   240,    53,    54,    55,    56,    57,
     174,    60,   243,   244,   265,   252,   254,   269,   272,   274,
     277,    60,   279,    58,   158,    59,   303,   278,   289,   281,
      69,   283,   286,    61,    62,    63,    64,   292,    65,    66,
     294,   295,   300,    61,    62,    63,    64,   297,    65,    66,
      60,   301,   326,   309,   310,   312,    69,   336,   308,   315,
     316,   318,   319,   320,   322,    53,    54,    55,    56,    57,
     155,   323,    61,    62,    63,    64,     4,    65,    66,   327,
     329,   331,   338,    58,   345,    59,    53,    54,    55,    56,
      57,   339,   342,   348,   350,   113,   352,   114,   356,   346,
     354,   362,   359,   363,    58,    85,    59,   364,   115,   204,
      60,   299,   116,   117,   118,   119,   120,   121,   340,   361,
     317,   218,   122,   123,   124,   125,   347,   328,   275,   276,
       0,    60,    61,    62,    63,    64,     0,    65,    66,     0,
       0,     0,     0,     1,     2,     0,     0,     0,     0,     0,
       0,     0,     0,    61,    62,    63,    64,     0,    65,    66,
       3,     4,     5,     6,     7,     8,     9,    10,     0,     0,
       0,     0,     0,    11,    12,    13,     0,   197,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    16,    17,     0,
     198,    18,   113,     0,   114,     0,    19,     0,     0,     0,
       0,     0,     0,     0,     0,   115,     0,     0,     0,   116,
     117,   118,   119,   120,   121,   113,     0,   114,     0,   122,
     123,   124,   125,   199,     0,     0,     0,     0,   115,     0,
       0,     0,   116,   117,   118,   119,   120,   121,     0,     0,
       0,     0,   122,   123,   124,   125,   200,     0,   113,     0,
     114,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   115,     0,     0,     0,   116,   117,   118,   119,   120,
     121,   113,     0,   114,     0,   122,   123,   124,   125,   201,
       0,     0,     0,     0,   115,     0,     0,     0,   116,   117,
     118,   119,   120,   121,     0,     0,     0,     0,   122,   123,
     124,   125,     0,     0,   113,   112,   114,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   115,     0,     0,
       0,   116,   117,   118,   119,   120,   121,   113,     0,   114,
       0,   122,   123,   124,   125,     0,     0,     0,     0,     0,
     115,     0,     0,     0,   116,   117,   118,   119,   120,   121,
     215,     0,   114,     0,   122,   123,   124,   125,     0,     0,
       0,     0,     0,   216,   217,     0,     0,   116,   117,   118,
     119,   120,   121,   113,     0,   114,     0,   122,   123,   124,
     125,     0,     0,     0,     0,     0,   115,  -102,     0,     0,
     116,   117,   118,   119,   120,   121,   113,     0,   114,     0,
     122,   123,   124,   125,     0,     0,     0,     0,     0,   115,
       0,     0,     0,   116,   117,   118,   119,   120,   121,     0,
       0,     0,     0,   122,   123,   124,   125
};

static const yytype_int16 yycheck[] =
{
       3,     4,     0,   138,   109,   154,   235,   112,   237,   182,
     212,    10,     9,   103,     9,    23,    30,    11,    30,    20,
      18,    32,    11,    11,    11,    24,    17,   176,    19,   252,
      58,   254,    46,   202,    56,    66,    30,    18,    19,    11,
     263,    69,   211,   272,    56,    57,    57,   220,   221,    17,
      81,    19,    49,    50,    49,    58,    64,    56,    30,    58,
     283,    81,    65,    85,    86,    47,    55,    55,    79,    80,
      69,    82,    58,   275,    73,    74,    75,    76,    77,    78,
      81,    72,    32,    69,    83,    84,    85,    86,   223,   179,
     225,   293,    81,    81,    81,    98,    99,   100,   101,   102,
     103,    79,    80,    71,    72,    49,   109,    57,    81,   112,
      38,    39,    40,    41,    42,    43,   345,   319,    81,   122,
     123,   124,   125,   126,    33,    34,    17,    11,    61,    79,
      80,     3,    82,   306,    84,    50,    56,    57,    53,    54,
      51,    52,     0,   141,    79,    80,    19,   145,    30,   254,
      56,    57,    81,    30,   289,    81,    81,    81,   331,    81,
     333,    30,   311,    32,    30,    30,   169,    30,    49,    81,
      59,    81,    56,    73,    58,   348,   179,    62,   283,   352,
      81,    55,    81,    21,    55,    69,   359,    81,    57,    73,
      74,    75,    76,    77,    78,     4,    48,    81,   347,    83,
      84,    85,    86,    81,   309,    81,    34,   342,    50,    23,
      79,    80,    82,    82,    81,   218,    12,    13,    14,    15,
      16,    81,    81,    55,    81,    21,    12,    13,    14,    15,
      16,    81,    81,    81,    30,    56,    32,    23,    31,    34,
     243,    30,    21,     9,    30,    30,    32,    73,    47,    30,
      81,   254,    34,    31,    31,    12,    13,    14,    15,    16,
      56,    57,    34,    81,    17,    30,    30,    34,    30,    11,
      81,    57,     7,    30,    31,    32,   274,    53,    34,    31,
     283,    30,    57,    79,    80,    81,    82,    34,    84,    85,
      81,    79,    31,    79,    80,    81,    82,    57,    84,    85,
      57,    31,   300,     4,     8,    31,   309,   310,    81,    31,
      31,    31,    31,    81,    31,    12,    13,    14,    15,    16,
      31,    67,    79,    80,    81,    82,    23,    84,    85,    31,
      81,    55,    31,    30,    30,    32,    12,    13,    14,    15,
      16,    31,    73,    55,    81,    56,    55,    58,    73,    65,
      57,    31,    55,    81,    30,    18,    32,    31,    69,   157,
      57,   269,    73,    74,    75,    76,    77,    78,   317,   353,
     289,   175,    83,    84,    85,    86,   328,   304,   239,   243,
      -1,    57,    79,    80,    81,    82,    -1,    84,    85,    -1,
      -1,    -1,    -1,     5,     6,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    85,
      22,    23,    24,    25,    26,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    35,    36,    37,    -1,    31,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    -1,
      31,    63,    56,    -1,    58,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    56,    -1,    58,    -1,    83,
      84,    85,    86,    31,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    73,    74,    75,    76,    77,    78,    -1,    -1,
      -1,    -1,    83,    84,    85,    86,    31,    -1,    56,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    56,    -1,    58,    -1,    83,    84,    85,    86,    31,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    73,    74,
      75,    76,    77,    78,    -1,    -1,    -1,    -1,    83,    84,
      85,    86,    -1,    -1,    56,    34,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    56,    -1,    58,
      -1,    83,    84,    85,    86,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    73,    74,    75,    76,    77,    78,
      56,    -1,    58,    -1,    83,    84,    85,    86,    -1,    -1,
      -1,    -1,    -1,    69,    70,    -1,    -1,    73,    74,    75,
      76,    77,    78,    56,    -1,    58,    -1,    83,    84,    85,
      86,    -1,    -1,    -1,    -1,    -1,    69,    70,    -1,    -1,
      73,    74,    75,    76,    77,    78,    56,    -1,    58,    -1,
      83,    84,    85,    86,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    -1,
      -1,    -1,    -1,    83,    84,    85,    86
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     5,     6,    22,    23,    24,    25,    26,    27,    28,
      29,    35,    36,    37,    44,    45,    59,    60,    63,    68,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   114,   119,
     120,   122,   123,   138,   139,   140,    17,    19,    71,    72,
      17,    19,    72,    12,    13,    14,    15,    16,    30,    32,
      57,    79,    80,    81,    82,    84,    85,   117,   124,   127,
     128,   129,   141,   146,   147,   125,   127,    81,    18,    19,
      47,    49,    81,    81,    61,    90,    17,     0,     3,   151,
      81,    81,    19,    81,    20,    81,    81,    81,    30,    30,
      30,    30,    30,    23,   127,    79,    80,   110,   142,    30,
      46,   127,    34,    56,    58,    69,    73,    74,    75,    76,
      77,    78,    83,    84,    85,    86,   135,     9,    49,    50,
     143,    11,    81,   126,    49,    81,    81,    59,    73,    62,
      81,    11,    30,    55,    81,    11,    30,    21,    55,   127,
     127,   127,   127,   127,   125,    31,    33,    34,    31,   124,
      81,   124,    58,    69,   127,   127,   127,   127,   127,     4,
      81,   130,   131,    21,    56,   127,   133,   134,    81,    34,
      81,    48,    50,   132,    81,   117,    82,   122,    81,   108,
      81,    55,   122,    81,   113,    81,    81,    31,    31,    31,
      31,    31,    49,   143,   110,    31,   127,   144,   145,    11,
      81,    34,   148,    30,    21,    56,    69,    70,   135,   143,
      51,    52,   125,    30,   133,    73,    47,    38,    39,    40,
      41,    42,    43,   111,   107,    30,    81,    34,    31,   131,
      31,    10,    24,    34,    81,   131,    53,    54,   132,   149,
     122,   150,    30,    69,    30,    56,    57,   127,   133,   133,
      84,   116,   117,    30,   117,    17,    30,    56,    57,    34,
     112,   113,    30,   113,    11,   148,   145,    81,    53,     7,
     136,    31,   150,    30,   124,   150,    57,    79,    80,    34,
     115,   150,    34,   121,    81,    79,   109,    57,    66,   108,
      31,    31,   113,   122,   132,    11,    55,    81,    81,     4,
       8,   137,    31,   124,   150,    31,    31,   116,    31,    31,
      81,   132,    31,    67,    64,   118,   122,    31,   136,    81,
     133,    55,    11,    55,    81,   124,   127,   143,    31,    31,
     115,   132,    73,    56,    57,    30,    65,   137,    55,   133,
      81,   133,    55,   117,    57,   113,    73,   143,   133,    55,
     133,   121,    31,    81,    31,   133
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    88,    89,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    91,    92,    93,
      94,    95,    96,    97,    97,    98,    99,   100,   101,   102,
     102,   103,   104,   104,   104,   105,   105,   106,   107,   107,
     108,   108,   108,   108,   108,   108,   109,   110,   110,   111,
     111,   111,   111,   111,   111,   112,   112,   113,   113,   114,
     115,   115,   116,   116,   116,   117,   117,   117,   117,   117,
     118,   118,   119,   120,   120,   121,   121,   122,   122,   122,
     123,   124,   124,   125,   125,   126,   126,   126,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   128,   128,   129,   129,   130,   130,   130,   131,
     131,   132,   132,   133,   133,   133,   133,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   136,   136,   137,   137,
     138,   139,   140,   141,   142,   142,   143,   143,   144,   144,
     145,   145,   145,   146,   147,   147,   147,   147,   147,   148,
     148,   149,   149,   149,   149,   149,   149,   150,   151,   151
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     5,     3,     2,     4,     2,     8,
       9,     5,     9,     5,     9,     5,     8,     3,     0,     3,
       5,     6,     7,     2,     3,     4,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     6,     1,     3,     8,
       0,     3,     1,     2,     2,     1,     1,     1,     1,     1,
       0,     4,     4,     8,     9,     0,     5,     9,     5,     3,
       2,     1,     3,     2,     4,     0,     1,     2,     3,     3,
       3,     3,     3,     3,     2,     1,     3,     4,     1,     1,
       1,     1,    11,     5,     1,     3,     1,     2,     3,     1,
       3,     0,     2,     0,     1,     3,     3,     3,     5,     6,
       5,     6,     3,     4,     4,     5,     1,     1,     1,     1,
       1,     1,     1,     2,     1,     2,     0,     3,     0,     2,
       7,     2,     4,     3,     1,     3,     0,     3,     1,     3,
       1,     2,     2,     4,     1,     4,     4,     4,     4,     0,
       2,     5,     6,     7,     4,     5,     6,     1,     0,     1
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
#line 254 "src/observer/sql/parser/yacc_sql.y"
  {
    unique_ptr<ParsedSqlNode> sql_node = unique_ptr<ParsedSqlNode>((yyvsp[-1].sql_node));
    sql_result->add_sql_node(std::move(sql_node));
  }
#line 1994 "yacc_sql.tab.c"
    break;

  case 27: /* exit_stmt: EXIT  */
#line 288 "src/observer/sql/parser/yacc_sql.y"
         {
      (void)yynerrs;  // 这么写为了消除yynerrs未使用的告警。如果你有更好的方法欢迎提PR
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXIT);
    }
#line 2003 "yacc_sql.tab.c"
    break;

  case 28: /* help_stmt: HELP  */
#line 294 "src/observer/sql/parser/yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_HELP);
    }
#line 2011 "yacc_sql.tab.c"
    break;

  case 29: /* sync_stmt: SYNC  */
#line 299 "src/observer/sql/parser/yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SYNC);
    }
#line 2019 "yacc_sql.tab.c"
    break;

  case 30: /* begin_stmt: TRX_BEGIN  */
#line 305 "src/observer/sql/parser/yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_BEGIN);
    }
#line 2027 "yacc_sql.tab.c"
    break;

  case 31: /* commit_stmt: TRX_COMMIT  */
#line 311 "src/observer/sql/parser/yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_COMMIT);
    }
#line 2035 "yacc_sql.tab.c"
    break;

  case 32: /* rollback_stmt: TRX_ROLLBACK  */
#line 317 "src/observer/sql/parser/yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ROLLBACK);
    }
#line 2043 "yacc_sql.tab.c"
    break;

  case 33: /* drop_table_stmt: DROP TABLE ID  */
#line 323 "src/observer/sql/parser/yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_TABLE);
      (yyval.sql_node)->drop_table.relation_name = (yyvsp[0].cstring);
      (yyval.sql_node)->drop_table.if_exists = false;
    }
#line 2053 "yacc_sql.tab.c"
    break;

  case 34: /* drop_table_stmt: DROP TABLE IF EXISTS ID  */
#line 328 "src/observer/sql/parser/yacc_sql.y"
                              {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_TABLE);
      (yyval.sql_node)->drop_table.relation_name = (yyvsp[0].cstring);
      (yyval.sql_node)->drop_table.if_exists = true;
    }
#line 2063 "yacc_sql.tab.c"
    break;

  case 35: /* analyze_table_stmt: ANALYZE TABLE ID  */
#line 335 "src/observer/sql/parser/yacc_sql.y"
                     {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ANALYZE_TABLE);
      (yyval.sql_node)->analyze_table.relation_name = (yyvsp[0].cstring);
    }
#line 2072 "yacc_sql.tab.c"
    break;

  case 36: /* show_tables_stmt: SHOW TABLES  */
#line 342 "src/observer/sql/parser/yacc_sql.y"
                {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SHOW_TABLES);
    }
#line 2080 "yacc_sql.tab.c"
    break;

  case 37: /* show_index_stmt: SHOW INDEX FROM ID  */
#line 349 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SHOW_INDEX);
      (yyval.sql_node)->show_index.relation_name = (yyvsp[0].cstring);
    }
#line 2089 "yacc_sql.tab.c"
    break;

  case 38: /* desc_table_stmt: DESC ID  */
#line 356 "src/observer/sql/parser/yacc_sql.y"
             {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DESC_TABLE);
      (yyval.sql_node)->desc_table.relation_name = (yyvsp[0].cstring);
    }
#line 2098 "yacc_sql.tab.c"
    break;

  case 39: /* create_index_stmt: CREATE INDEX ID ON ID LBRACE attr_list RBRACE  */
#line 364 "src/observer/sql/parser/yacc_sql.y"
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
#line 2114 "yacc_sql.tab.c"
    break;

  case 40: /* create_index_stmt: CREATE UNIQUE INDEX ID ON ID LBRACE attr_list RBRACE  */
#line 376 "src/observer/sql/parser/yacc_sql.y"
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
#line 2130 "yacc_sql.tab.c"
    break;

  case 41: /* drop_index_stmt: DROP INDEX ID ON ID  */
#line 391 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_INDEX);
      (yyval.sql_node)->drop_index.index_name = (yyvsp[-2].cstring);
      (yyval.sql_node)->drop_index.relation_name = (yyvsp[0].cstring);
    }
#line 2140 "yacc_sql.tab.c"
    break;

  case 42: /* create_table_stmt: CREATE TABLE ID LBRACE attr_def attr_def_list primary_key RBRACE storage_format  */
#line 399 "src/observer/sql/parser/yacc_sql.y"
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
#line 2168 "yacc_sql.tab.c"
    break;

  case 43: /* create_table_stmt: CREATE TABLE ID AS select_stmt  */
#line 423 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_TABLE);
      CreateTableSqlNode &create_table = (yyval.sql_node)->create_table;
      create_table.relation_name = (yyvsp[-2].cstring);
      create_table.as_select = std::make_unique<SelectSqlNode>(std::move((yyvsp[0].sql_node)->selection));
      delete (yyvsp[0].sql_node);
    }
#line 2180 "yacc_sql.tab.c"
    break;

  case 44: /* create_table_stmt: CREATE TABLE ID LBRACE attr_def attr_def_list primary_key RBRACE select_stmt  */
#line 431 "src/observer/sql/parser/yacc_sql.y"
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
#line 2208 "yacc_sql.tab.c"
    break;

  case 45: /* create_view_stmt: CREATE VIEW ID AS select_stmt  */
#line 458 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_VIEW);
      CreateViewSqlNode &create_view = (yyval.sql_node)->create_view;
      create_view.relation_name = (yyvsp[-2].cstring);
      create_view.create_view_select = std::make_unique<SelectSqlNode>(std::move((yyvsp[0].sql_node)->selection));
      delete (yyvsp[0].sql_node);
    }
#line 2220 "yacc_sql.tab.c"
    break;

  case 46: /* create_view_stmt: CREATE VIEW ID LBRACE attr_list RBRACE AS select_stmt  */
#line 466 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_VIEW);
      CreateViewSqlNode &create_view = (yyval.sql_node)->create_view;
      create_view.relation_name = (yyvsp[-5].cstring);
      create_view.attribute_names = std::move(*(yyvsp[-3].key_list));
      create_view.create_view_select = std::make_unique<SelectSqlNode>(std::move((yyvsp[0].sql_node)->selection));
      delete (yyvsp[-3].key_list);
      delete (yyvsp[0].sql_node);
    }
#line 2234 "yacc_sql.tab.c"
    break;

  case 47: /* drop_view_stmt: DROP VIEW ID  */
#line 479 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_VIEW);
      (yyval.sql_node)->drop_view.relation_name = (yyvsp[0].cstring);
    }
#line 2243 "yacc_sql.tab.c"
    break;

  case 48: /* attr_def_list: %empty  */
#line 487 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.attr_infos) = nullptr;
    }
#line 2251 "yacc_sql.tab.c"
    break;

  case 49: /* attr_def_list: attr_def_list COMMA attr_def  */
#line 491 "src/observer/sql/parser/yacc_sql.y"
    {
      if ((yyvsp[-2].attr_infos) != nullptr) {
        (yyval.attr_infos) = (yyvsp[-2].attr_infos);
      } else {
        (yyval.attr_infos) = new vector<AttrInfoSqlNode>;
      }
      (yyval.attr_infos)->insert((yyval.attr_infos)->begin(), *(yyvsp[0].attr_info));
      delete (yyvsp[0].attr_info);
    }
#line 2265 "yacc_sql.tab.c"
    break;

  case 50: /* attr_def: ID type LBRACE number RBRACE  */
#line 504 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-3].number);
      (yyval.attr_info)->name = (yyvsp[-4].cstring);
      (yyval.attr_info)->nullable = false;  // default NOT NULL
      // For VECTOR(n), store bytes for [dimension(int32) + n float32 values]
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int) + ((yyvsp[-1].number) * sizeof(float)));
      } else {
        (yyval.attr_info)->length = (yyvsp[-1].number);
      }
    }
#line 2282 "yacc_sql.tab.c"
    break;

  case 51: /* attr_def: ID type LBRACE number RBRACE NULL_T  */
#line 517 "src/observer/sql/parser/yacc_sql.y"
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
      // Add extra byte for NULL flag for nullable fields
      if ((yyval.attr_info)->nullable) {
        (yyval.attr_info)->length++;
      }
    }
#line 2303 "yacc_sql.tab.c"
    break;

  case 52: /* attr_def: ID type LBRACE number RBRACE NOT NULL_T  */
#line 534 "src/observer/sql/parser/yacc_sql.y"
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
#line 2320 "yacc_sql.tab.c"
    break;

  case 53: /* attr_def: ID type  */
#line 547 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[0].number);
      (yyval.attr_info)->name = (yyvsp[-1].cstring);
      (yyval.attr_info)->nullable = false;  // default NOT NULL
      // Default length: INT/FLOAT/DATE/STRING fallback 4; VECTOR default header size (dimension); TEXT default 65535
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int));
      } else if ((yyval.attr_info)->type == AttrType::TEXTS) {
        (yyval.attr_info)->length = 255;  // TEXT default length (reduced for testing)
      } else {
        (yyval.attr_info)->length = 4;
      }
    }
#line 2339 "yacc_sql.tab.c"
    break;

  case 54: /* attr_def: ID type NULL_T  */
#line 562 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-1].number);
      (yyval.attr_info)->name = (yyvsp[-2].cstring);
      (yyval.attr_info)->nullable = true;  // explicitly nullable
      // Default length: INT/FLOAT/DATE/STRING fallback 4; VECTOR default header size (dimension); TEXT default 65535
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int));
      } else if ((yyval.attr_info)->type == AttrType::TEXTS) {
        (yyval.attr_info)->length = 255;  // TEXT default length (reduced for testing)
      } else {
        (yyval.attr_info)->length = 4;
      }
      // Add extra byte for NULL flag for nullable fields
      if ((yyval.attr_info)->nullable) {
        (yyval.attr_info)->length++;
      }
    }
#line 2362 "yacc_sql.tab.c"
    break;

  case 55: /* attr_def: ID type NOT NULL_T  */
#line 581 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-2].number);
      (yyval.attr_info)->name = (yyvsp[-3].cstring);
      (yyval.attr_info)->nullable = false;  // explicitly not nullable
      // Default length: INT/FLOAT/DATE/STRING fallback 4; VECTOR default header size (dimension); TEXT default 65535
      if ((yyval.attr_info)->type == AttrType::VECTORS) {
        (yyval.attr_info)->length = static_cast<int>(sizeof(int));
      } else if ((yyval.attr_info)->type == AttrType::TEXTS) {
        (yyval.attr_info)->length = 255;  // TEXT default length (reduced for testing)
      } else {
        (yyval.attr_info)->length = 4;
      }
    }
#line 2381 "yacc_sql.tab.c"
    break;

  case 56: /* number: NUMBER  */
#line 597 "src/observer/sql/parser/yacc_sql.y"
           {(yyval.number) = (yyvsp[0].number);}
#line 2387 "yacc_sql.tab.c"
    break;

  case 57: /* float_number: NUMBER  */
#line 601 "src/observer/sql/parser/yacc_sql.y"
           {(yyval.floats) = (double)(yyvsp[0].number);}
#line 2393 "yacc_sql.tab.c"
    break;

  case 58: /* float_number: FLOAT_LITERAL  */
#line 602 "src/observer/sql/parser/yacc_sql.y"
                    {(yyval.floats) = (yyvsp[0].floats);}
#line 2399 "yacc_sql.tab.c"
    break;

  case 59: /* type: INT_T  */
#line 605 "src/observer/sql/parser/yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::INTS); }
#line 2405 "yacc_sql.tab.c"
    break;

  case 60: /* type: STRING_T  */
#line 606 "src/observer/sql/parser/yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::CHARS); }
#line 2411 "yacc_sql.tab.c"
    break;

  case 61: /* type: FLOAT_T  */
#line 607 "src/observer/sql/parser/yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::FLOATS); }
#line 2417 "yacc_sql.tab.c"
    break;

  case 62: /* type: DATE_T  */
#line 608 "src/observer/sql/parser/yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::DATES); }
#line 2423 "yacc_sql.tab.c"
    break;

  case 63: /* type: VECTOR_T  */
#line 609 "src/observer/sql/parser/yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::VECTORS); }
#line 2429 "yacc_sql.tab.c"
    break;

  case 64: /* type: TEXT_T  */
#line 610 "src/observer/sql/parser/yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::TEXTS); }
#line 2435 "yacc_sql.tab.c"
    break;

  case 65: /* primary_key: %empty  */
#line 614 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.key_list) = nullptr;
    }
#line 2443 "yacc_sql.tab.c"
    break;

  case 66: /* primary_key: COMMA PRIMARY KEY LBRACE attr_list RBRACE  */
#line 618 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.key_list) = (yyvsp[-1].key_list);
    }
#line 2451 "yacc_sql.tab.c"
    break;

  case 67: /* attr_list: ID  */
#line 624 "src/observer/sql/parser/yacc_sql.y"
       {
      (yyval.key_list) = new vector<string>();
      (yyval.key_list)->push_back((yyvsp[0].cstring));
    }
#line 2460 "yacc_sql.tab.c"
    break;

  case 68: /* attr_list: ID COMMA attr_list  */
#line 628 "src/observer/sql/parser/yacc_sql.y"
                         {
      if ((yyvsp[0].key_list) != nullptr) {
        (yyval.key_list) = (yyvsp[0].key_list);
      } else {
        (yyval.key_list) = new vector<string>;
      }

      (yyval.key_list)->insert((yyval.key_list)->begin(), (yyvsp[-2].cstring));
    }
#line 2474 "yacc_sql.tab.c"
    break;

  case 69: /* insert_stmt: INSERT INTO ID VALUES LBRACE insert_value insert_value_list RBRACE  */
#line 641 "src/observer/sql/parser/yacc_sql.y"
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
#line 2490 "yacc_sql.tab.c"
    break;

  case 70: /* insert_value_list: %empty  */
#line 656 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.value_list) = nullptr;
    }
#line 2498 "yacc_sql.tab.c"
    break;

  case 71: /* insert_value_list: COMMA insert_value insert_value_list  */
#line 659 "src/observer/sql/parser/yacc_sql.y"
                                            {
      if ((yyvsp[0].value_list) != nullptr) {
        (yyval.value_list) = (yyvsp[0].value_list);
      } else {
        (yyval.value_list) = new vector<Value>;
      }
      (yyval.value_list)->emplace_back(*(yyvsp[-1].value));
      delete (yyvsp[-1].value);
    }
#line 2512 "yacc_sql.tab.c"
    break;

  case 73: /* insert_value: '-' NUMBER  */
#line 674 "src/observer/sql/parser/yacc_sql.y"
                 {
      (yyval.value) = new Value(-(int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2521 "yacc_sql.tab.c"
    break;

  case 74: /* insert_value: '-' FLOAT_LITERAL  */
#line 678 "src/observer/sql/parser/yacc_sql.y"
                        {
      (yyval.value) = new Value(-(float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2530 "yacc_sql.tab.c"
    break;

  case 75: /* value: NUMBER  */
#line 685 "src/observer/sql/parser/yacc_sql.y"
           {
      (yyval.value) = new Value((int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2539 "yacc_sql.tab.c"
    break;

  case 76: /* value: FLOAT_LITERAL  */
#line 689 "src/observer/sql/parser/yacc_sql.y"
                   {
      (yyval.value) = new Value((float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2548 "yacc_sql.tab.c"
    break;

  case 77: /* value: NULL_T  */
#line 693 "src/observer/sql/parser/yacc_sql.y"
            {
      (yyval.value) = new Value();
      (yyval.value)->set_type(AttrType::UNDEFINED);
      (yyval.value)->set_null(true);
      (yyloc) = (yylsp[0]);
    }
#line 2559 "yacc_sql.tab.c"
    break;

  case 78: /* value: vector_literal  */
#line 699 "src/observer/sql/parser/yacc_sql.y"
                    {
      (yyval.value) = (yyvsp[0].value);
      (yyloc) = (yylsp[0]);
    }
#line 2568 "yacc_sql.tab.c"
    break;

  case 79: /* value: SSS  */
#line 703 "src/observer/sql/parser/yacc_sql.y"
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
#line 2624 "yacc_sql.tab.c"
    break;

  case 80: /* storage_format: %empty  */
#line 757 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.cstring) = nullptr;
    }
#line 2632 "yacc_sql.tab.c"
    break;

  case 81: /* storage_format: STORAGE FORMAT EQ ID  */
#line 761 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.cstring) = (yyvsp[0].cstring);
    }
#line 2640 "yacc_sql.tab.c"
    break;

  case 82: /* delete_stmt: DELETE FROM ID where  */
#line 768 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DELETE);
      (yyval.sql_node)->deletion.relation_name = (yyvsp[-1].cstring);
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.sql_node)->deletion.conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 2653 "yacc_sql.tab.c"
    break;

  case 83: /* update_stmt: UPDATE ID SET ID EQ value update_set_list where  */
#line 779 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_UPDATE);
      (yyval.sql_node)->update.relation_name = (yyvsp[-6].cstring);
      
      // 第一个字段
      (yyval.sql_node)->update.attribute_names.push_back((yyvsp[-4].cstring));
      (yyval.sql_node)->update.values.push_back(*(yyvsp[-2].value));
      delete (yyvsp[-2].value);
      
      // 其他字段（如果有）
      if ((yyvsp[-1].update_set_list) != nullptr) {
        for (auto &p : *(yyvsp[-1].update_set_list)) {
          (yyval.sql_node)->update.attribute_names.push_back(p.first);
          (yyval.sql_node)->update.values.push_back(p.second);
        }
        delete (yyvsp[-1].update_set_list);
      }
      
      (yyval.sql_node)->update.is_sub_query = false;
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.sql_node)->update.conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 2682 "yacc_sql.tab.c"
    break;

  case 84: /* update_stmt: UPDATE ID SET ID EQ LBRACE sub_query RBRACE where  */
#line 804 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_UPDATE);
      (yyval.sql_node)->update.relation_name = (yyvsp[-7].cstring);
      (yyval.sql_node)->update.attribute_name = (yyvsp[-5].cstring);
      (yyval.sql_node)->update.sub_query.reset((yyvsp[-2].sql_node));
      (yyval.sql_node)->update.is_sub_query = true;
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.sql_node)->update.conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 2698 "yacc_sql.tab.c"
    break;

  case 85: /* update_set_list: %empty  */
#line 818 "src/observer/sql/parser/yacc_sql.y"
                { (yyval.update_set_list) = nullptr; }
#line 2704 "yacc_sql.tab.c"
    break;

  case 86: /* update_set_list: COMMA ID EQ value update_set_list  */
#line 820 "src/observer/sql/parser/yacc_sql.y"
    {
      if ((yyvsp[0].update_set_list) != nullptr) {
        (yyval.update_set_list) = (yyvsp[0].update_set_list);
      } else {
        (yyval.update_set_list) = new vector<pair<string, Value>>();
      }
      (yyval.update_set_list)->emplace_back((yyvsp[-3].cstring), *(yyvsp[-1].value));
      delete (yyvsp[-1].value);
    }
#line 2718 "yacc_sql.tab.c"
    break;

  case 87: /* select_stmt: SELECT projection_list FROM rel_list join_list where group_by having order_by  */
#line 832 "src/observer/sql/parser/yacc_sql.y"
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
#line 2762 "yacc_sql.tab.c"
    break;

  case 88: /* select_stmt: SELECT projection_list WHERE condition_list order_by  */
#line 872 "src/observer/sql/parser/yacc_sql.y"
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
#line 2784 "yacc_sql.tab.c"
    break;

  case 89: /* select_stmt: SELECT projection_list order_by  */
#line 890 "src/observer/sql/parser/yacc_sql.y"
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
#line 2801 "yacc_sql.tab.c"
    break;

  case 90: /* calc_stmt: CALC expression_list  */
#line 905 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CALC);
      (yyval.sql_node)->calc.expressions.swap(*(yyvsp[0].expression_list));
      delete (yyvsp[0].expression_list);
    }
#line 2811 "yacc_sql.tab.c"
    break;

  case 91: /* expression_list: expression  */
#line 914 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      (yyval.expression_list)->emplace_back((yyvsp[0].expression));
    }
#line 2820 "yacc_sql.tab.c"
    break;

  case 92: /* expression_list: expression COMMA expression_list  */
#line 919 "src/observer/sql/parser/yacc_sql.y"
    {
      if ((yyvsp[0].expression_list) != nullptr) {
        (yyval.expression_list) = (yyvsp[0].expression_list);
      } else {
        (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      }
      (yyval.expression_list)->emplace((yyval.expression_list)->begin(), (yyvsp[-2].expression));
    }
#line 2833 "yacc_sql.tab.c"
    break;

  case 93: /* projection_list: expression opt_as_alias  */
#line 931 "src/observer/sql/parser/yacc_sql.y"
    {
      if ((yyvsp[0].cstring) != nullptr) {
        (yyvsp[-1].expression)->set_name((yyvsp[0].cstring));
      }
      (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      (yyval.expression_list)->emplace_back((yyvsp[-1].expression));
    }
#line 2845 "yacc_sql.tab.c"
    break;

  case 94: /* projection_list: expression opt_as_alias COMMA projection_list  */
#line 939 "src/observer/sql/parser/yacc_sql.y"
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
#line 2861 "yacc_sql.tab.c"
    break;

  case 95: /* opt_as_alias: %empty  */
#line 953 "src/observer/sql/parser/yacc_sql.y"
                { (yyval.cstring) = nullptr; }
#line 2867 "yacc_sql.tab.c"
    break;

  case 96: /* opt_as_alias: ID  */
#line 954 "src/observer/sql/parser/yacc_sql.y"
         { (yyval.cstring) = (yyvsp[0].cstring); }
#line 2873 "yacc_sql.tab.c"
    break;

  case 97: /* opt_as_alias: AS ID  */
#line 955 "src/observer/sql/parser/yacc_sql.y"
            { (yyval.cstring) = (yyvsp[0].cstring); }
#line 2879 "yacc_sql.tab.c"
    break;

  case 98: /* expression: expression '+' expression  */
#line 959 "src/observer/sql/parser/yacc_sql.y"
                              {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::ADD, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2887 "yacc_sql.tab.c"
    break;

  case 99: /* expression: expression '-' expression  */
#line 962 "src/observer/sql/parser/yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::SUB, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2895 "yacc_sql.tab.c"
    break;

  case 100: /* expression: expression '*' expression  */
#line 965 "src/observer/sql/parser/yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::MUL, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2903 "yacc_sql.tab.c"
    break;

  case 101: /* expression: expression '/' expression  */
#line 968 "src/observer/sql/parser/yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::DIV, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2911 "yacc_sql.tab.c"
    break;

  case 102: /* expression: expression comp_op expression  */
#line 971 "src/observer/sql/parser/yacc_sql.y"
                                    {
      (yyval.expression) = new ComparisonExpr((yyvsp[-1].comp), unique_ptr<Expression>((yyvsp[-2].expression)), unique_ptr<Expression>((yyvsp[0].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2920 "yacc_sql.tab.c"
    break;

  case 103: /* expression: LBRACE expression RBRACE  */
#line 975 "src/observer/sql/parser/yacc_sql.y"
                               {
      // Simple parentheses for grouping - just return the inner expression
      (yyval.expression) = (yyvsp[-1].expression);
    }
#line 2929 "yacc_sql.tab.c"
    break;

  case 104: /* expression: '-' expression  */
#line 979 "src/observer/sql/parser/yacc_sql.y"
                                  {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::NEGATIVE, (yyvsp[0].expression), nullptr, sql_string, &(yyloc));
    }
#line 2937 "yacc_sql.tab.c"
    break;

  case 105: /* expression: value  */
#line 982 "src/observer/sql/parser/yacc_sql.y"
            {
      (yyval.expression) = new ValueExpr(*(yyvsp[0].value));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete (yyvsp[0].value);
    }
#line 2947 "yacc_sql.tab.c"
    break;

  case 106: /* expression: ID LBRACE RBRACE  */
#line 988 "src/observer/sql/parser/yacc_sql.y"
                       {
      std::vector<std::unique_ptr<Expression>> args;
      (yyval.expression) = new FunctionExpr((yyvsp[-2].cstring), std::move(args));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2957 "yacc_sql.tab.c"
    break;

  case 107: /* expression: ID LBRACE expression_list RBRACE  */
#line 993 "src/observer/sql/parser/yacc_sql.y"
                                       {
      std::vector<std::unique_ptr<Expression>> args;
      if ((yyvsp[-1].expression_list) != nullptr) {
        args.swap(*(yyvsp[-1].expression_list));
        delete (yyvsp[-1].expression_list);
      }
      (yyval.expression) = new FunctionExpr((yyvsp[-3].cstring), std::move(args));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2971 "yacc_sql.tab.c"
    break;

  case 108: /* expression: rel_attr  */
#line 1002 "src/observer/sql/parser/yacc_sql.y"
               {
      RelAttrSqlNode *node = (yyvsp[0].rel_attr);
      (yyval.expression) = new UnboundFieldExpr(node->relation_name, node->attribute_name);
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete (yyvsp[0].rel_attr);
    }
#line 2982 "yacc_sql.tab.c"
    break;

  case 109: /* expression: '*'  */
#line 1008 "src/observer/sql/parser/yacc_sql.y"
          {
      (yyval.expression) = new StarExpr();
    }
#line 2990 "yacc_sql.tab.c"
    break;

  case 110: /* expression: aggregate_expr  */
#line 1011 "src/observer/sql/parser/yacc_sql.y"
                     {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 2998 "yacc_sql.tab.c"
    break;

  case 111: /* expression: sub_query_expr  */
#line 1014 "src/observer/sql/parser/yacc_sql.y"
                     {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 3006 "yacc_sql.tab.c"
    break;

  case 112: /* sub_query_expr: LBRACE SELECT projection_list FROM rel_list join_list where group_by having order_by RBRACE  */
#line 1021 "src/observer/sql/parser/yacc_sql.y"
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
#line 3057 "yacc_sql.tab.c"
    break;

  case 113: /* sub_query_expr: LBRACE SELECT projection_list order_by RBRACE  */
#line 1068 "src/observer/sql/parser/yacc_sql.y"
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
#line 3081 "yacc_sql.tab.c"
    break;

  case 114: /* rel_attr: ID  */
#line 1090 "src/observer/sql/parser/yacc_sql.y"
       {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->attribute_name = (yyvsp[0].cstring);
    }
#line 3090 "yacc_sql.tab.c"
    break;

  case 115: /* rel_attr: ID DOT ID  */
#line 1094 "src/observer/sql/parser/yacc_sql.y"
                {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->relation_name  = (yyvsp[-2].cstring);
      (yyval.rel_attr)->attribute_name = (yyvsp[0].cstring);
    }
#line 3100 "yacc_sql.tab.c"
    break;

  case 116: /* relation: ID  */
#line 1102 "src/observer/sql/parser/yacc_sql.y"
       {
      (yyval.relation_with_alias) = new RelationWithAliasSqlNode();
      (yyval.relation_with_alias)->table_name = (yyvsp[0].cstring);
    }
#line 3109 "yacc_sql.tab.c"
    break;

  case 117: /* relation: ID ID  */
#line 1106 "src/observer/sql/parser/yacc_sql.y"
            {
      (yyval.relation_with_alias) = new RelationWithAliasSqlNode();
      (yyval.relation_with_alias)->table_name = (yyvsp[-1].cstring);
      (yyval.relation_with_alias)->alias = (yyvsp[0].cstring);
    }
#line 3119 "yacc_sql.tab.c"
    break;

  case 118: /* relation: ID AS ID  */
#line 1111 "src/observer/sql/parser/yacc_sql.y"
               {
      (yyval.relation_with_alias) = new RelationWithAliasSqlNode();
      (yyval.relation_with_alias)->table_name = (yyvsp[-2].cstring);
      (yyval.relation_with_alias)->alias = (yyvsp[0].cstring);
    }
#line 3129 "yacc_sql.tab.c"
    break;

  case 119: /* rel_list: relation  */
#line 1118 "src/observer/sql/parser/yacc_sql.y"
             {
      (yyval.relation_with_alias_list) = new vector<RelationWithAliasSqlNode>();
      (yyval.relation_with_alias_list)->emplace_back(std::move(*(yyvsp[0].relation_with_alias)));
      delete (yyvsp[0].relation_with_alias);
    }
#line 3139 "yacc_sql.tab.c"
    break;

  case 120: /* rel_list: relation COMMA rel_list  */
#line 1123 "src/observer/sql/parser/yacc_sql.y"
                              {
      if ((yyvsp[0].relation_with_alias_list) != nullptr) {
        (yyval.relation_with_alias_list) = (yyvsp[0].relation_with_alias_list);
      } else {
        (yyval.relation_with_alias_list) = new vector<RelationWithAliasSqlNode>();
      }
      (yyval.relation_with_alias_list)->emplace((yyval.relation_with_alias_list)->begin(), std::move(*(yyvsp[-2].relation_with_alias)));
      delete (yyvsp[-2].relation_with_alias);
    }
#line 3153 "yacc_sql.tab.c"
    break;

  case 121: /* where: %empty  */
#line 1138 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 3161 "yacc_sql.tab.c"
    break;

  case 122: /* where: WHERE condition_list  */
#line 1141 "src/observer/sql/parser/yacc_sql.y"
                           {
      (yyval.condition_list) = (yyvsp[0].condition_list);
      printf("DEBUG: [YACC] WHERE condition_list: %zu conditions\n", (yyvsp[0].condition_list) ? (yyvsp[0].condition_list)->size() : 0);
      if ((yyvsp[0].condition_list)) {
        for (size_t i = 0; i < (yyvsp[0].condition_list)->size(); i++) {
          printf("DEBUG: [YACC] Condition %zu: conjunction_op = %d\n", i, (*(yyvsp[0].condition_list))[i].conjunction_op);
        }
      }
    }
#line 3175 "yacc_sql.tab.c"
    break;

  case 123: /* condition_list: %empty  */
#line 1153 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 3183 "yacc_sql.tab.c"
    break;

  case 124: /* condition_list: condition  */
#line 1156 "src/observer/sql/parser/yacc_sql.y"
                {
      (yyval.condition_list) = new vector<ConditionSqlNode>;
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[0].condition)));
      delete (yyvsp[0].condition);
    }
#line 3193 "yacc_sql.tab.c"
    break;

  case 125: /* condition_list: condition AND condition_list  */
#line 1161 "src/observer/sql/parser/yacc_sql.y"
                                   {
      (yyval.condition_list) = (yyvsp[0].condition_list);
      (yyvsp[-2].condition)->conjunction_op = AND_OP;  // 设置AND连接符
      printf("DEBUG: [YACC] Setting AND_OP for condition\n");
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[-2].condition)));
      delete (yyvsp[-2].condition);
    }
#line 3205 "yacc_sql.tab.c"
    break;

  case 126: /* condition_list: condition OR condition_list  */
#line 1168 "src/observer/sql/parser/yacc_sql.y"
                                  {
      (yyval.condition_list) = (yyvsp[0].condition_list);
      (yyvsp[-2].condition)->conjunction_op = OR_OP;    // 设置OR连接符
      printf("DEBUG: [YACC] Setting OR_OP for condition\n");
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[-2].condition)));
      delete (yyvsp[-2].condition);
    }
#line 3217 "yacc_sql.tab.c"
    break;

  case 127: /* condition: expression comp_op expression  */
#line 1178 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr.reset((yyvsp[-2].expression));
      (yyval.condition)->right_expr.reset((yyvsp[0].expression));
      (yyval.condition)->comp = (yyvsp[-1].comp);
      (yyval.condition)->conjunction_op = AND_OP;  // 默认初始化为AND_OP
    }
#line 3229 "yacc_sql.tab.c"
    break;

  case 128: /* condition: expression IN LBRACE expression_list RBRACE  */
#line 1186 "src/observer/sql/parser/yacc_sql.y"
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
#line 3255 "yacc_sql.tab.c"
    break;

  case 129: /* condition: expression NOT IN LBRACE expression_list RBRACE  */
#line 1208 "src/observer/sql/parser/yacc_sql.y"
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
#line 3281 "yacc_sql.tab.c"
    break;

  case 130: /* condition: expression IN LBRACE sub_query RBRACE  */
#line 1230 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr.reset((yyvsp[-4].expression));
      (yyval.condition)->sub_query.reset((yyvsp[-1].sql_node));
      (yyval.condition)->comp = IN_OP;
    }
#line 3292 "yacc_sql.tab.c"
    break;

  case 131: /* condition: expression NOT IN LBRACE sub_query RBRACE  */
#line 1237 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr.reset((yyvsp[-5].expression));
      (yyval.condition)->sub_query.reset((yyvsp[-1].sql_node));
      (yyval.condition)->comp = NOT_IN_OP;
    }
#line 3303 "yacc_sql.tab.c"
    break;

  case 132: /* condition: expression IS NULL_T  */
#line 1244 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr.reset((yyvsp[-2].expression));
      Value __nullv; __nullv.set_type(AttrType::UNDEFINED); __nullv.set_null(true);
      (yyval.condition)->right_expr.reset(new ValueExpr(__nullv));
      (yyval.condition)->comp = IS_NULL_OP;
    }
#line 3315 "yacc_sql.tab.c"
    break;

  case 133: /* condition: expression IS NOT NULL_T  */
#line 1252 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr.reset((yyvsp[-3].expression));
      Value __nullv2; __nullv2.set_type(AttrType::UNDEFINED); __nullv2.set_null(true);
      (yyval.condition)->right_expr.reset(new ValueExpr(__nullv2));
      (yyval.condition)->comp = IS_NOT_NULL_OP;
    }
#line 3327 "yacc_sql.tab.c"
    break;

  case 134: /* condition: EXISTS LBRACE sub_query RBRACE  */
#line 1260 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->sub_query.reset((yyvsp[-1].sql_node));
      (yyval.condition)->comp = EXISTS_OP;
    }
#line 3337 "yacc_sql.tab.c"
    break;

  case 135: /* condition: NOT EXISTS LBRACE sub_query RBRACE  */
#line 1266 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->sub_query.reset((yyvsp[-1].sql_node));
      (yyval.condition)->comp = NOT_EXISTS_OP;
    }
#line 3347 "yacc_sql.tab.c"
    break;

  case 136: /* comp_op: EQ  */
#line 1274 "src/observer/sql/parser/yacc_sql.y"
         { (yyval.comp) = EQUAL_TO; }
#line 3353 "yacc_sql.tab.c"
    break;

  case 137: /* comp_op: LT  */
#line 1275 "src/observer/sql/parser/yacc_sql.y"
         { (yyval.comp) = LESS_THAN; }
#line 3359 "yacc_sql.tab.c"
    break;

  case 138: /* comp_op: GT  */
#line 1276 "src/observer/sql/parser/yacc_sql.y"
         { (yyval.comp) = GREAT_THAN; }
#line 3365 "yacc_sql.tab.c"
    break;

  case 139: /* comp_op: LE  */
#line 1277 "src/observer/sql/parser/yacc_sql.y"
         { (yyval.comp) = LESS_EQUAL; }
#line 3371 "yacc_sql.tab.c"
    break;

  case 140: /* comp_op: GE  */
#line 1278 "src/observer/sql/parser/yacc_sql.y"
         { (yyval.comp) = GREAT_EQUAL; }
#line 3377 "yacc_sql.tab.c"
    break;

  case 141: /* comp_op: NE  */
#line 1279 "src/observer/sql/parser/yacc_sql.y"
         { (yyval.comp) = NOT_EQUAL; }
#line 3383 "yacc_sql.tab.c"
    break;

  case 142: /* comp_op: LIKE  */
#line 1280 "src/observer/sql/parser/yacc_sql.y"
           { (yyval.comp) = LIKE_OP; }
#line 3389 "yacc_sql.tab.c"
    break;

  case 143: /* comp_op: NOT LIKE  */
#line 1281 "src/observer/sql/parser/yacc_sql.y"
               { (yyval.comp) = NOT_LIKE_OP; }
#line 3395 "yacc_sql.tab.c"
    break;

  case 144: /* comp_op: IN  */
#line 1282 "src/observer/sql/parser/yacc_sql.y"
         { (yyval.comp) = IN_OP; }
#line 3401 "yacc_sql.tab.c"
    break;

  case 145: /* comp_op: NOT IN  */
#line 1283 "src/observer/sql/parser/yacc_sql.y"
             { (yyval.comp) = NOT_IN_OP; }
#line 3407 "yacc_sql.tab.c"
    break;

  case 146: /* group_by: %empty  */
#line 1289 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.expression_list) = nullptr;
    }
#line 3415 "yacc_sql.tab.c"
    break;

  case 147: /* group_by: GROUP BY expression_list  */
#line 1293 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.expression_list) = (yyvsp[0].expression_list);
    }
#line 3423 "yacc_sql.tab.c"
    break;

  case 148: /* having: %empty  */
#line 1300 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.expression) = nullptr;
    }
#line 3431 "yacc_sql.tab.c"
    break;

  case 149: /* having: HAVING expression  */
#line 1304 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 3439 "yacc_sql.tab.c"
    break;

  case 150: /* load_data_stmt: LOAD DATA INFILE SSS INTO TABLE ID  */
#line 1310 "src/observer/sql/parser/yacc_sql.y"
    {
      char *tmp_file_name = common::substr((yyvsp[-3].cstring), 1, strlen((yyvsp[-3].cstring)) - 2);

      (yyval.sql_node) = new ParsedSqlNode(SCF_LOAD_DATA);
      (yyval.sql_node)->load_data.relation_name = (yyvsp[0].cstring);
      (yyval.sql_node)->load_data.file_name = tmp_file_name;
      free(tmp_file_name);
    }
#line 3452 "yacc_sql.tab.c"
    break;

  case 151: /* explain_stmt: EXPLAIN command_wrapper  */
#line 1322 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXPLAIN);
      (yyval.sql_node)->explain.sql_node = unique_ptr<ParsedSqlNode>((yyvsp[0].sql_node));
    }
#line 3461 "yacc_sql.tab.c"
    break;

  case 152: /* set_variable_stmt: SET ID EQ value  */
#line 1330 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SET_VARIABLE);
      (yyval.sql_node)->set_variable.name  = (yyvsp[-2].cstring);
      (yyval.sql_node)->set_variable.value = *(yyvsp[0].value);
      delete (yyvsp[0].value);
    }
#line 3472 "yacc_sql.tab.c"
    break;

  case 153: /* vector_literal: LSQUARE number_list RSQUARE  */
#line 1339 "src/observer/sql/parser/yacc_sql.y"
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
#line 3512 "yacc_sql.tab.c"
    break;

  case 154: /* number_list: float_number  */
#line 1377 "src/observer/sql/parser/yacc_sql.y"
                 {
      (yyval.float_list) = new std::vector<double>();
      (yyval.float_list)->push_back((yyvsp[0].floats));
    }
#line 3521 "yacc_sql.tab.c"
    break;

  case 155: /* number_list: number_list COMMA float_number  */
#line 1381 "src/observer/sql/parser/yacc_sql.y"
                                     {
      (yyval.float_list) = (yyvsp[-2].float_list);
      (yyval.float_list)->push_back((yyvsp[0].floats));
    }
#line 3530 "yacc_sql.tab.c"
    break;

  case 156: /* order_by: %empty  */
#line 1389 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.order_by_list) = nullptr;
    }
#line 3538 "yacc_sql.tab.c"
    break;

  case 157: /* order_by: ORDER BY order_by_item_list  */
#line 1393 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.order_by_list) = (yyvsp[0].order_by_list);
    }
#line 3546 "yacc_sql.tab.c"
    break;

  case 158: /* order_by_item_list: order_by_item  */
#line 1400 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.order_by_list) = new std::vector<OrderBySqlNode>();
      (yyval.order_by_list)->emplace_back(std::move(*(yyvsp[0].order_by_item)));
      delete (yyvsp[0].order_by_item);
    }
#line 3556 "yacc_sql.tab.c"
    break;

  case 159: /* order_by_item_list: order_by_item_list COMMA order_by_item  */
#line 1406 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.order_by_list) = (yyvsp[-2].order_by_list);
      (yyval.order_by_list)->emplace_back(std::move(*(yyvsp[0].order_by_item)));
      delete (yyvsp[0].order_by_item);
    }
#line 3566 "yacc_sql.tab.c"
    break;

  case 160: /* order_by_item: expression  */
#line 1415 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.order_by_item) = new OrderBySqlNode();
      (yyval.order_by_item)->expression = unique_ptr<Expression>((yyvsp[0].expression));
      (yyval.order_by_item)->direction = ASC_ORDER;  // 默认升序
    }
#line 3576 "yacc_sql.tab.c"
    break;

  case 161: /* order_by_item: expression ASC  */
#line 1421 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.order_by_item) = new OrderBySqlNode();
      (yyval.order_by_item)->expression = unique_ptr<Expression>((yyvsp[-1].expression));
      (yyval.order_by_item)->direction = ASC_ORDER;
    }
#line 3586 "yacc_sql.tab.c"
    break;

  case 162: /* order_by_item: expression DESC  */
#line 1427 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.order_by_item) = new OrderBySqlNode();
      (yyval.order_by_item)->expression = unique_ptr<Expression>((yyvsp[-1].expression));
      (yyval.order_by_item)->direction = DESC_ORDER;
    }
#line 3596 "yacc_sql.tab.c"
    break;

  case 163: /* count_expr: COUNT LBRACE expression RBRACE  */
#line 1435 "src/observer/sql/parser/yacc_sql.y"
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
#line 3611 "yacc_sql.tab.c"
    break;

  case 164: /* aggregate_expr: count_expr  */
#line 1448 "src/observer/sql/parser/yacc_sql.y"
               {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 3619 "yacc_sql.tab.c"
    break;

  case 165: /* aggregate_expr: SUM LBRACE expression RBRACE  */
#line 1451 "src/observer/sql/parser/yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("sum", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3628 "yacc_sql.tab.c"
    break;

  case 166: /* aggregate_expr: MAX LBRACE expression RBRACE  */
#line 1455 "src/observer/sql/parser/yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("max", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3637 "yacc_sql.tab.c"
    break;

  case 167: /* aggregate_expr: MIN LBRACE expression RBRACE  */
#line 1459 "src/observer/sql/parser/yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("min", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3646 "yacc_sql.tab.c"
    break;

  case 168: /* aggregate_expr: AVG LBRACE expression RBRACE  */
#line 1463 "src/observer/sql/parser/yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("avg", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 3655 "yacc_sql.tab.c"
    break;

  case 169: /* join_list: %empty  */
#line 1471 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.join_list) = nullptr;
    }
#line 3663 "yacc_sql.tab.c"
    break;

  case 170: /* join_list: join_list join_item  */
#line 1475 "src/observer/sql/parser/yacc_sql.y"
    {
      if ((yyvsp[-1].join_list) != nullptr) {
        (yyval.join_list) = (yyvsp[-1].join_list);
      } else {
        (yyval.join_list) = new std::vector<JoinSqlNode>();
      }
      (yyval.join_list)->emplace_back(std::move(*(yyvsp[0].join_item)));
      delete (yyvsp[0].join_item);
    }
#line 3677 "yacc_sql.tab.c"
    break;

  case 171: /* join_item: INNER JOIN ID ON condition_list  */
#line 1488 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.join_item) = new JoinSqlNode();
      (yyval.join_item)->table_name = (yyvsp[-2].cstring);
      (yyval.join_item)->join_type = INNER_JOIN;
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.join_item)->conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 3691 "yacc_sql.tab.c"
    break;

  case 172: /* join_item: INNER JOIN ID ID ON condition_list  */
#line 1498 "src/observer/sql/parser/yacc_sql.y"
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
#line 3706 "yacc_sql.tab.c"
    break;

  case 173: /* join_item: INNER JOIN ID AS ID ON condition_list  */
#line 1509 "src/observer/sql/parser/yacc_sql.y"
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
#line 3721 "yacc_sql.tab.c"
    break;

  case 174: /* join_item: JOIN ID ON condition_list  */
#line 1520 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.join_item) = new JoinSqlNode();
      (yyval.join_item)->table_name = (yyvsp[-2].cstring);
      (yyval.join_item)->join_type = INNER_JOIN;  // 默认为INNER JOIN
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.join_item)->conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 3735 "yacc_sql.tab.c"
    break;

  case 175: /* join_item: JOIN ID ID ON condition_list  */
#line 1530 "src/observer/sql/parser/yacc_sql.y"
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
#line 3750 "yacc_sql.tab.c"
    break;

  case 176: /* join_item: JOIN ID AS ID ON condition_list  */
#line 1541 "src/observer/sql/parser/yacc_sql.y"
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
#line 3765 "yacc_sql.tab.c"
    break;

  case 177: /* sub_query: select_stmt  */
#line 1555 "src/observer/sql/parser/yacc_sql.y"
    {
      (yyval.sql_node) = (yyvsp[0].sql_node);
    }
#line 3773 "yacc_sql.tab.c"
    break;


#line 3777 "yacc_sql.tab.c"

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

#line 1563 "src/observer/sql/parser/yacc_sql.y"

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
