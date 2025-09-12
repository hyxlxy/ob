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
#include <vector>
#include <string>

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
  YYSYMBOL_EQ = 65,                        /* EQ  */
  YYSYMBOL_LT = 66,                        /* LT  */
  YYSYMBOL_GT = 67,                        /* GT  */
  YYSYMBOL_LE = 68,                        /* LE  */
  YYSYMBOL_GE = 69,                        /* GE  */
  YYSYMBOL_NE = 70,                        /* NE  */
  YYSYMBOL_NUMBER = 71,                    /* NUMBER  */
  YYSYMBOL_ID = 72,                        /* ID  */
  YYSYMBOL_SSS = 73,                       /* SSS  */
  YYSYMBOL_74_ = 74,                       /* '+'  */
  YYSYMBOL_75_ = 75,                       /* '-'  */
  YYSYMBOL_76_ = 76,                       /* '*'  */
  YYSYMBOL_77_ = 77,                       /* '/'  */
  YYSYMBOL_UMINUS = 78,                    /* UMINUS  */
  YYSYMBOL_YYACCEPT = 79,                  /* $accept  */
  YYSYMBOL_commands = 80,                  /* commands  */
  YYSYMBOL_command_wrapper = 81,           /* command_wrapper  */
  YYSYMBOL_exit_stmt = 82,                 /* exit_stmt  */
  YYSYMBOL_help_stmt = 83,                 /* help_stmt  */
  YYSYMBOL_sync_stmt = 84,                 /* sync_stmt  */
  YYSYMBOL_begin_stmt = 85,                /* begin_stmt  */
  YYSYMBOL_commit_stmt = 86,               /* commit_stmt  */
  YYSYMBOL_rollback_stmt = 87,             /* rollback_stmt  */
  YYSYMBOL_drop_table_stmt = 88,           /* drop_table_stmt  */
  YYSYMBOL_analyze_table_stmt = 89,        /* analyze_table_stmt  */
  YYSYMBOL_show_tables_stmt = 90,          /* show_tables_stmt  */
  YYSYMBOL_desc_table_stmt = 91,           /* desc_table_stmt  */
  YYSYMBOL_create_index_stmt = 92,         /* create_index_stmt  */
  YYSYMBOL_drop_index_stmt = 93,           /* drop_index_stmt  */
  YYSYMBOL_create_table_stmt = 94,         /* create_table_stmt  */
  YYSYMBOL_attr_def_list = 95,             /* attr_def_list  */
  YYSYMBOL_attr_def = 96,                  /* attr_def  */
  YYSYMBOL_number = 97,                    /* number  */
  YYSYMBOL_type = 98,                      /* type  */
  YYSYMBOL_primary_key = 99,               /* primary_key  */
  YYSYMBOL_attr_list = 100,                /* attr_list  */
  YYSYMBOL_insert_stmt = 101,              /* insert_stmt  */
  YYSYMBOL_insert_value_list = 102,        /* insert_value_list  */
  YYSYMBOL_insert_value = 103,             /* insert_value  */
  YYSYMBOL_value = 104,                    /* value  */
  YYSYMBOL_storage_format = 105,           /* storage_format  */
  YYSYMBOL_delete_stmt = 106,              /* delete_stmt  */
  YYSYMBOL_update_stmt = 107,              /* update_stmt  */
  YYSYMBOL_select_stmt = 108,              /* select_stmt  */
  YYSYMBOL_calc_stmt = 109,                /* calc_stmt  */
  YYSYMBOL_expression_list = 110,          /* expression_list  */
  YYSYMBOL_projection_list = 111,          /* projection_list  */
  YYSYMBOL_opt_as_alias = 112,             /* opt_as_alias  */
  YYSYMBOL_expression = 113,               /* expression  */
  YYSYMBOL_rel_attr = 114,                 /* rel_attr  */
  YYSYMBOL_relation = 115,                 /* relation  */
  YYSYMBOL_rel_list = 116,                 /* rel_list  */
  YYSYMBOL_where = 117,                    /* where  */
  YYSYMBOL_condition_list = 118,           /* condition_list  */
  YYSYMBOL_condition = 119,                /* condition  */
  YYSYMBOL_comp_op = 120,                  /* comp_op  */
  YYSYMBOL_group_by = 121,                 /* group_by  */
  YYSYMBOL_having = 122,                   /* having  */
  YYSYMBOL_load_data_stmt = 123,           /* load_data_stmt  */
  YYSYMBOL_explain_stmt = 124,             /* explain_stmt  */
  YYSYMBOL_set_variable_stmt = 125,        /* set_variable_stmt  */
  YYSYMBOL_vector_literal = 126,           /* vector_literal  */
  YYSYMBOL_number_list = 127,              /* number_list  */
  YYSYMBOL_order_by = 128,                 /* order_by  */
  YYSYMBOL_order_by_item_list = 129,       /* order_by_item_list  */
  YYSYMBOL_order_by_item = 130,            /* order_by_item  */
  YYSYMBOL_aggregate_expr = 131,           /* aggregate_expr  */
  YYSYMBOL_join_list = 132,                /* join_list  */
  YYSYMBOL_join_item = 133,                /* join_item  */
  YYSYMBOL_opt_semicolon = 134             /* opt_semicolon  */
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
#define YYFINAL  77
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   296

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  79
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  56
/* YYNRULES -- Number of rules.  */
#define YYNRULES  141
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  268

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   329


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
       2,     2,    76,    74,     2,    75,     2,    77,     2,     2,
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
      65,    66,    67,    68,    69,    70,    71,    72,    73,    78
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   236,   236,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   268,   274,   279,   285,   291,   297,
     303,   309,   316,   322,   329,   340,   348,   376,   379,   392,
     404,   418,   419,   422,   423,   424,   425,   426,   430,   433,
     440,   444,   456,   472,   475,   487,   488,   492,   499,   503,
     507,   511,   565,   568,   575,   586,   599,   639,   654,   663,
     668,   680,   688,   703,   704,   705,   709,   712,   715,   718,
     721,   725,   728,   734,   739,   748,   754,   757,   763,   767,
     775,   779,   784,   791,   796,   811,   814,   820,   823,   828,
     835,   845,   846,   847,   848,   849,   850,   851,   852,   858,
     861,   869,   872,   878,   890,   898,   908,   934,   938,   946,
     949,   956,   962,   971,   977,   983,   992,   996,  1000,  1004,
    1008,  1012,  1020,  1023,  1036,  1046,  1057,  1068,  1078,  1089,
    1102,  1103
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
  "STORAGE", "FORMAT", "PRIMARY", "KEY", "ANALYZE", "EQ", "LT", "GT", "LE",
  "GE", "NE", "NUMBER", "ID", "SSS", "'+'", "'-'", "'*'", "'/'", "UMINUS",
  "$accept", "commands", "command_wrapper", "exit_stmt", "help_stmt",
  "sync_stmt", "begin_stmt", "commit_stmt", "rollback_stmt",
  "drop_table_stmt", "analyze_table_stmt", "show_tables_stmt",
  "desc_table_stmt", "create_index_stmt", "drop_index_stmt",
  "create_table_stmt", "attr_def_list", "attr_def", "number", "type",
  "primary_key", "attr_list", "insert_stmt", "insert_value_list",
  "insert_value", "value", "storage_format", "delete_stmt", "update_stmt",
  "select_stmt", "calc_stmt", "expression_list", "projection_list",
  "opt_as_alias", "expression", "rel_attr", "relation", "rel_list",
  "where", "condition_list", "condition", "comp_op", "group_by", "having",
  "load_data_stmt", "explain_stmt", "set_variable_stmt", "vector_literal",
  "number_list", "order_by", "order_by_item_list", "order_by_item",
  "aggregate_expr", "join_list", "join_item", "opt_semicolon", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-155)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     197,    -2,    25,    87,    87,   -52,    16,  -155,     5,    10,
     -29,  -155,  -155,  -155,  -155,  -155,   -20,     8,   197,    37,
      64,    63,  -155,  -155,  -155,  -155,  -155,  -155,  -155,  -155,
    -155,  -155,  -155,  -155,  -155,  -155,  -155,  -155,  -155,  -155,
    -155,  -155,  -155,    15,    42,    44,    46,    52,    78,    92,
      95,    96,    87,   -31,  -155,  -155,    -9,  -155,    87,  -155,
    -155,  -155,    80,  -155,  -155,  -155,     4,     0,  -155,  -155,
      58,    60,    79,    72,    83,  -155,    70,  -155,  -155,  -155,
     118,    91,  -155,   109,   136,    87,    87,    87,    87,    34,
    -155,  -155,  -155,    -4,    17,    99,  -155,    87,    87,    87,
      87,    87,   161,   100,  -155,   101,  -155,   142,   130,   129,
     112,    74,   105,  -155,   121,   122,   123,   167,    93,   106,
     111,   115,   124,  -155,  -155,   -31,  -155,   176,  -155,  -155,
     -16,   -16,  -155,  -155,    87,    -5,   178,  -155,  -155,    87,
     186,    87,  -155,   141,  -155,   170,   -15,  -155,   188,  -155,
    -155,  -155,  -155,  -155,  -155,  -155,  -155,  -155,    -6,   193,
    -155,   154,  -155,   100,    77,  -155,   174,   201,  -155,   184,
      74,   210,  -155,  -155,  -155,  -155,  -155,   206,   203,   164,
    -155,  -155,    87,  -155,  -155,   165,   191,   231,  -155,   -30,
     211,  -155,   190,  -155,  -155,  -155,  -155,  -155,  -155,  -155,
      87,    87,   129,   179,   -31,   -48,   213,   217,  -155,     1,
     185,   244,   242,  -155,  -155,   174,   229,  -155,   -38,  -155,
    -155,  -155,   230,   199,  -155,   200,  -155,   192,    87,   220,
       7,    87,    87,   254,   211,  -155,  -155,   237,   212,  -155,
     222,  -155,    87,   207,    87,   228,  -155,   -38,  -155,  -155,
     209,   218,    87,  -155,   232,  -155,    87,   250,   256,   214,
    -155,    87,  -155,   209,  -155,  -155,  -155,  -155
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,    26,     0,     0,
       0,    27,    28,    29,    25,    24,     0,     0,     0,     0,
       0,   140,    23,    22,    15,    16,    17,    18,     9,    10,
      11,    12,    13,    14,     8,     5,     7,     6,     4,     3,
      19,    20,    21,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    59,    58,    88,    61,     0,    86,
      82,    68,    69,    85,    60,    87,   119,    73,    33,    32,
       0,     0,     0,     0,     0,   114,     0,     1,   141,     2,
       0,     0,    30,     0,     0,     0,     0,     0,     0,     0,
      42,    41,   117,     0,     0,     0,    81,     0,     0,     0,
       0,     0,     0,     0,    67,     0,    74,    71,     0,    95,
       0,     0,     0,    31,     0,     0,     0,    86,     0,     0,
       0,     0,     0,    80,   116,     0,    83,     0,    89,    70,
      76,    77,    78,    79,     0,    90,    93,   132,    75,     0,
       0,    97,    64,     0,   115,     0,     0,    37,     0,    35,
     126,   127,   128,   129,   130,   131,   118,    84,   123,   120,
     121,     0,    91,     0,    95,    72,     0,     0,    96,    98,
       0,     0,    43,    44,    45,    46,    47,    40,    48,     0,
     124,   125,     0,    92,    94,     0,     0,   109,   133,     0,
      53,    55,     0,   107,   101,   102,   103,   104,   105,   106,
       0,    97,    95,     0,     0,     0,     0,     0,   122,     0,
       0,     0,   111,    57,    56,     0,     0,   108,   100,    99,
      65,   113,     0,     0,    38,    62,    34,     0,    97,     0,
       0,     0,     0,   119,    53,    52,    39,     0,     0,    36,
       0,   137,    97,     0,    97,     0,   110,   112,    66,    54,
       0,     0,    97,   138,     0,   134,    97,    50,     0,     0,
     139,    97,   135,     0,    49,    63,   136,    51
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -155,  -155,   269,  -155,  -155,  -155,  -155,  -155,  -155,  -155,
    -155,  -155,  -155,  -155,  -155,  -155,  -155,    84,  -118,  -155,
    -155,    27,  -155,    54,    76,  -108,  -155,  -155,  -155,  -155,
    -155,   -92,   153,  -155,    -3,  -155,  -155,   131,  -154,  -123,
    -155,  -155,  -155,  -155,  -155,  -155,  -155,  -155,  -155,    62,
    -155,   114,  -155,  -155,  -155,  -155
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,   178,   147,    92,   177,
     206,   258,    35,   216,   190,    60,   239,    36,    37,    38,
      39,    61,    66,   107,   167,    63,   136,   137,   142,   168,
     169,   200,   212,   233,    40,    41,    42,    64,    93,   104,
     159,   160,    65,   164,   188,    79
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      62,    67,   127,   144,   180,   129,   161,   156,    90,   213,
     187,   105,   227,   102,   223,    43,   181,    44,   243,    94,
      68,   172,   173,   174,   146,   175,   176,   124,   125,    47,
      48,    49,    50,    51,    69,    95,    98,    99,   100,   101,
      91,   214,    45,    72,    46,    52,   126,    53,   220,    89,
      70,   103,    73,   228,    76,    96,    54,    71,   191,   244,
     100,   101,   202,   123,    77,    74,    78,   162,    98,    99,
     100,   101,   106,   229,    98,    99,   100,   101,   219,   245,
      84,   118,   119,   120,   121,   122,   222,    80,    55,    56,
      57,    62,    58,    59,    62,   130,   131,   132,   133,    47,
      48,    49,    50,    51,    53,   241,    85,   191,    98,    99,
     100,   101,    97,    54,    81,    52,    82,    53,    83,   253,
      86,   255,   151,    87,    88,   141,    54,   185,   186,   260,
     108,   158,   109,   262,   110,   152,    67,   111,   266,   246,
     153,   112,   113,   115,   154,    55,   114,    57,    47,    48,
      49,    50,    51,   155,    98,    99,   100,   101,    55,    56,
      57,   116,    58,    59,    52,   134,    53,    98,    99,   100,
     101,   128,   135,   138,   139,    54,   140,   141,   145,   158,
      98,    99,   100,   101,   143,    98,    99,   100,   101,    98,
      99,   100,   101,   146,   148,   149,   150,   218,    98,    99,
     100,   101,     1,     2,    53,   157,   170,    55,    56,    57,
     163,    58,   117,    54,   166,   171,   179,     3,     4,     5,
       6,     7,     8,     9,    10,   182,   183,   203,    62,   247,
      11,    12,    13,   201,   204,   205,   207,   209,   211,    14,
      15,   210,   225,   215,   217,    55,   226,    57,   231,   189,
     232,   221,    16,    17,   192,   193,    18,   230,   235,   236,
     238,    19,   237,   102,   240,   250,   194,   195,   196,   197,
     198,   199,   242,   251,   252,    98,    99,   100,   101,   254,
     256,   257,   263,   259,   261,   264,   265,    75,   249,   224,
     267,   234,   165,     0,   184,   248,   208
};

static const yytype_int16 yycheck[] =
{
       3,     4,    94,   111,    10,    97,    11,   125,    39,    39,
     164,    11,    11,     9,    62,    17,    22,    19,    11,    28,
      72,    36,    37,    38,    72,    40,    41,    31,    32,    12,
      13,    14,    15,    16,    18,    44,    74,    75,    76,    77,
      71,    71,    17,    72,    19,    28,    29,    30,   202,    52,
      45,    47,    72,    52,    17,    58,    39,    47,   166,    52,
      76,    77,   170,    29,     0,    57,     3,    72,    74,    75,
      76,    77,    72,    72,    74,    75,    76,    77,   201,    72,
      28,    84,    85,    86,    87,    88,   204,    72,    71,    72,
      73,    94,    75,    76,    97,    98,    99,   100,   101,    12,
      13,    14,    15,    16,    30,   228,    28,   215,    74,    75,
      76,    77,    32,    39,    72,    28,    72,    30,    72,   242,
      28,   244,    29,    28,    28,    48,    39,    50,    51,   252,
      72,   134,    72,   256,    55,    29,   139,    65,   261,   231,
      29,    58,    72,    52,    29,    71,    28,    73,    12,    13,
      14,    15,    16,    29,    74,    75,    76,    77,    71,    72,
      73,    52,    75,    76,    28,     4,    30,    74,    75,    76,
      77,    72,    72,    72,    32,    39,    46,    48,    73,   182,
      74,    75,    76,    77,    72,    74,    75,    76,    77,    74,
      75,    76,    77,    72,    72,    72,    29,   200,    74,    75,
      76,    77,     5,     6,    30,    29,    65,    71,    72,    73,
      32,    75,    76,    39,    28,    45,    28,    20,    21,    22,
      23,    24,    25,    26,    27,    32,    72,    17,   231,   232,
      33,    34,    35,    49,    28,    32,    72,    72,     7,    42,
      43,    50,    29,    32,    54,    71,    29,    73,     4,    75,
       8,    72,    55,    56,    53,    54,    59,    72,    29,    29,
      60,    64,    63,     9,    72,    28,    65,    66,    67,    68,
      69,    70,    52,    61,    52,    74,    75,    76,    77,    72,
      52,    72,    32,    65,    52,    29,    72,    18,   234,   205,
     263,   215,   139,    -1,   163,   233,   182
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     5,     6,    20,    21,    22,    23,    24,    25,    26,
      27,    33,    34,    35,    42,    43,    55,    56,    59,    64,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,   101,   106,   107,   108,   109,
     123,   124,   125,    17,    19,    17,    19,    12,    13,    14,
      15,    16,    28,    30,    39,    71,    72,    73,    75,    76,
     104,   110,   113,   114,   126,   131,   111,   113,    72,    18,
      45,    47,    72,    72,    57,    81,    17,     0,     3,   134,
      72,    72,    72,    72,    28,    28,    28,    28,    28,   113,
      39,    71,    97,   127,    28,    44,   113,    32,    74,    75,
      76,    77,     9,    47,   128,    11,    72,   112,    72,    72,
      55,    65,    58,    72,    28,    52,    52,    76,   113,   113,
     113,   113,   113,    29,    31,    32,    29,   110,    72,   110,
     113,   113,   113,   113,     4,    72,   115,   116,    72,    32,
      46,    48,   117,    72,   104,    73,    72,    96,    72,    72,
      29,    29,    29,    29,    29,    29,    97,    29,   113,   129,
     130,    11,    72,    32,   132,   111,    28,   113,   118,   119,
      65,    45,    36,    37,    38,    40,    41,    98,    95,    28,
      10,    22,    32,    72,   116,    50,    51,   117,   133,    75,
     103,   104,    53,    54,    65,    66,    67,    68,    69,    70,
     120,    49,   104,    17,    28,    32,    99,    72,   130,    72,
      50,     7,   121,    39,    71,    32,   102,    54,   113,   118,
     117,    72,    97,    62,    96,    29,    29,    11,    52,    72,
      72,     4,     8,   122,   103,    29,    29,    63,    60,   105,
      72,   118,    52,    11,    52,    72,   110,   113,   128,   102,
      28,    61,    52,   118,    72,   118,    52,    72,   100,    65,
     118,    52,   118,    32,    29,    72,   118,   100
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    79,    80,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    95,    96,
      96,    97,    97,    98,    98,    98,    98,    98,    99,    99,
     100,   100,   101,   102,   102,   103,   103,   103,   104,   104,
     104,   104,   105,   105,   106,   107,   108,   108,   109,   110,
     110,   111,   111,   112,   112,   112,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   114,   114,
     115,   115,   115,   116,   116,   117,   117,   118,   118,   118,
     119,   120,   120,   120,   120,   120,   120,   120,   120,   121,
     121,   122,   122,   123,   124,   125,   126,   127,   127,   128,
     128,   129,   129,   130,   130,   130,   131,   131,   131,   131,
     131,   131,   132,   132,   133,   133,   133,   133,   133,   133,
     134,   134
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     2,     2,     8,     5,     9,     0,     3,     5,
       2,     1,     1,     1,     1,     1,     1,     1,     0,     6,
       1,     3,     8,     0,     3,     1,     2,     2,     1,     1,
       1,     1,     0,     4,     4,     7,     9,     3,     2,     1,
       3,     2,     4,     0,     1,     2,     3,     3,     3,     3,
       3,     2,     1,     3,     4,     1,     1,     1,     1,     3,
       1,     2,     3,     1,     3,     0,     2,     0,     1,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     2,     0,
       3,     0,     2,     7,     2,     4,     3,     1,     3,     0,
       3,     1,     3,     1,     2,     2,     4,     4,     4,     4,
       4,     4,     0,     2,     5,     6,     7,     4,     5,     6,
       0,     1
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
#line 237 "yacc_sql.y"
  {
    unique_ptr<ParsedSqlNode> sql_node = unique_ptr<ParsedSqlNode>((yyvsp[-1].sql_node));
    sql_result->add_sql_node(std::move(sql_node));
  }
#line 1855 "yacc_sql.tab.c"
    break;

  case 24: /* exit_stmt: EXIT  */
#line 268 "yacc_sql.y"
         {
      (void)yynerrs;  // 这么写为了消除yynerrs未使用的告警。如果你有更好的方法欢迎提PR
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXIT);
    }
#line 1864 "yacc_sql.tab.c"
    break;

  case 25: /* help_stmt: HELP  */
#line 274 "yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_HELP);
    }
#line 1872 "yacc_sql.tab.c"
    break;

  case 26: /* sync_stmt: SYNC  */
#line 279 "yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SYNC);
    }
#line 1880 "yacc_sql.tab.c"
    break;

  case 27: /* begin_stmt: TRX_BEGIN  */
#line 285 "yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_BEGIN);
    }
#line 1888 "yacc_sql.tab.c"
    break;

  case 28: /* commit_stmt: TRX_COMMIT  */
#line 291 "yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_COMMIT);
    }
#line 1896 "yacc_sql.tab.c"
    break;

  case 29: /* rollback_stmt: TRX_ROLLBACK  */
#line 297 "yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ROLLBACK);
    }
#line 1904 "yacc_sql.tab.c"
    break;

  case 30: /* drop_table_stmt: DROP TABLE ID  */
#line 303 "yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_TABLE);
      (yyval.sql_node)->drop_table.relation_name = (yyvsp[0].cstring);
    }
#line 1913 "yacc_sql.tab.c"
    break;

  case 31: /* analyze_table_stmt: ANALYZE TABLE ID  */
#line 309 "yacc_sql.y"
                     {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ANALYZE_TABLE);
      (yyval.sql_node)->analyze_table.relation_name = (yyvsp[0].cstring);
    }
#line 1922 "yacc_sql.tab.c"
    break;

  case 32: /* show_tables_stmt: SHOW TABLES  */
#line 316 "yacc_sql.y"
                {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SHOW_TABLES);
    }
#line 1930 "yacc_sql.tab.c"
    break;

  case 33: /* desc_table_stmt: DESC ID  */
#line 322 "yacc_sql.y"
             {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DESC_TABLE);
      (yyval.sql_node)->desc_table.relation_name = (yyvsp[0].cstring);
    }
#line 1939 "yacc_sql.tab.c"
    break;

  case 34: /* create_index_stmt: CREATE INDEX ID ON ID LBRACE ID RBRACE  */
#line 330 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_INDEX);
      CreateIndexSqlNode &create_index = (yyval.sql_node)->create_index;
      create_index.index_name = (yyvsp[-5].cstring);
      create_index.relation_name = (yyvsp[-3].cstring);
      create_index.attribute_name = (yyvsp[-1].cstring);
    }
#line 1951 "yacc_sql.tab.c"
    break;

  case 35: /* drop_index_stmt: DROP INDEX ID ON ID  */
#line 341 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_INDEX);
      (yyval.sql_node)->drop_index.index_name = (yyvsp[-2].cstring);
      (yyval.sql_node)->drop_index.relation_name = (yyvsp[0].cstring);
    }
#line 1961 "yacc_sql.tab.c"
    break;

  case 36: /* create_table_stmt: CREATE TABLE ID LBRACE attr_def attr_def_list primary_key RBRACE storage_format  */
#line 349 "yacc_sql.y"
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
#line 1989 "yacc_sql.tab.c"
    break;

  case 37: /* attr_def_list: %empty  */
#line 376 "yacc_sql.y"
    {
      (yyval.attr_infos) = nullptr;
    }
#line 1997 "yacc_sql.tab.c"
    break;

  case 38: /* attr_def_list: attr_def_list COMMA attr_def  */
#line 380 "yacc_sql.y"
    {
      if ((yyvsp[-2].attr_infos) != nullptr) {
        (yyval.attr_infos) = (yyvsp[-2].attr_infos);
      } else {
        (yyval.attr_infos) = new vector<AttrInfoSqlNode>;
      }
      (yyval.attr_infos)->insert((yyval.attr_infos)->begin(), *(yyvsp[0].attr_info));
      delete (yyvsp[0].attr_info);
    }
#line 2011 "yacc_sql.tab.c"
    break;

  case 39: /* attr_def: ID type LBRACE number RBRACE  */
#line 393 "yacc_sql.y"
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
#line 2027 "yacc_sql.tab.c"
    break;

  case 40: /* attr_def: ID type  */
#line 405 "yacc_sql.y"
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
#line 2043 "yacc_sql.tab.c"
    break;

  case 41: /* number: NUMBER  */
#line 418 "yacc_sql.y"
           {(yyval.number) = (yyvsp[0].number);}
#line 2049 "yacc_sql.tab.c"
    break;

  case 42: /* number: FLOAT_LITERAL  */
#line 419 "yacc_sql.y"
                    {(yyval.number) = (yyvsp[0].floats);}
#line 2055 "yacc_sql.tab.c"
    break;

  case 43: /* type: INT_T  */
#line 422 "yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::INTS); }
#line 2061 "yacc_sql.tab.c"
    break;

  case 44: /* type: STRING_T  */
#line 423 "yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::CHARS); }
#line 2067 "yacc_sql.tab.c"
    break;

  case 45: /* type: FLOAT_T  */
#line 424 "yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::FLOATS); }
#line 2073 "yacc_sql.tab.c"
    break;

  case 46: /* type: DATE_T  */
#line 425 "yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::DATES); }
#line 2079 "yacc_sql.tab.c"
    break;

  case 47: /* type: VECTOR_T  */
#line 426 "yacc_sql.y"
               { (yyval.number) = static_cast<int>(AttrType::VECTORS); }
#line 2085 "yacc_sql.tab.c"
    break;

  case 48: /* primary_key: %empty  */
#line 430 "yacc_sql.y"
    {
      (yyval.key_list) = nullptr;
    }
#line 2093 "yacc_sql.tab.c"
    break;

  case 49: /* primary_key: COMMA PRIMARY KEY LBRACE attr_list RBRACE  */
#line 434 "yacc_sql.y"
    {
      (yyval.key_list) = (yyvsp[-1].key_list);
    }
#line 2101 "yacc_sql.tab.c"
    break;

  case 50: /* attr_list: ID  */
#line 440 "yacc_sql.y"
       {
      (yyval.key_list) = new vector<string>();
      (yyval.key_list)->push_back((yyvsp[0].cstring));
    }
#line 2110 "yacc_sql.tab.c"
    break;

  case 51: /* attr_list: ID COMMA attr_list  */
#line 444 "yacc_sql.y"
                         {
      if ((yyvsp[0].key_list) != nullptr) {
        (yyval.key_list) = (yyvsp[0].key_list);
      } else {
        (yyval.key_list) = new vector<string>;
      }

      (yyval.key_list)->insert((yyval.key_list)->begin(), (yyvsp[-2].cstring));
    }
#line 2124 "yacc_sql.tab.c"
    break;

  case 52: /* insert_stmt: INSERT INTO ID VALUES LBRACE insert_value insert_value_list RBRACE  */
#line 457 "yacc_sql.y"
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
#line 2140 "yacc_sql.tab.c"
    break;

  case 53: /* insert_value_list: %empty  */
#line 472 "yacc_sql.y"
    {
      (yyval.value_list) = nullptr;
    }
#line 2148 "yacc_sql.tab.c"
    break;

  case 54: /* insert_value_list: COMMA insert_value insert_value_list  */
#line 475 "yacc_sql.y"
                                            {
      if ((yyvsp[0].value_list) != nullptr) {
        (yyval.value_list) = (yyvsp[0].value_list);
      } else {
        (yyval.value_list) = new vector<Value>;
      }
      (yyval.value_list)->emplace_back(*(yyvsp[-1].value));
      delete (yyvsp[-1].value);
    }
#line 2162 "yacc_sql.tab.c"
    break;

  case 56: /* insert_value: '-' NUMBER  */
#line 488 "yacc_sql.y"
                 {
      (yyval.value) = new Value(-(int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2171 "yacc_sql.tab.c"
    break;

  case 57: /* insert_value: '-' FLOAT_LITERAL  */
#line 492 "yacc_sql.y"
                        {
      (yyval.value) = new Value(-(float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2180 "yacc_sql.tab.c"
    break;

  case 58: /* value: NUMBER  */
#line 499 "yacc_sql.y"
           {
      (yyval.value) = new Value((int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2189 "yacc_sql.tab.c"
    break;

  case 59: /* value: FLOAT_LITERAL  */
#line 503 "yacc_sql.y"
                   {
      (yyval.value) = new Value((float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2198 "yacc_sql.tab.c"
    break;

  case 60: /* value: vector_literal  */
#line 507 "yacc_sql.y"
                    {
      (yyval.value) = (yyvsp[0].value);
      (yyloc) = (yylsp[0]);
    }
#line 2207 "yacc_sql.tab.c"
    break;

  case 61: /* value: SSS  */
#line 511 "yacc_sql.y"
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
#line 2263 "yacc_sql.tab.c"
    break;

  case 62: /* storage_format: %empty  */
#line 565 "yacc_sql.y"
    {
      (yyval.cstring) = nullptr;
    }
#line 2271 "yacc_sql.tab.c"
    break;

  case 63: /* storage_format: STORAGE FORMAT EQ ID  */
#line 569 "yacc_sql.y"
    {
      (yyval.cstring) = (yyvsp[0].cstring);
    }
#line 2279 "yacc_sql.tab.c"
    break;

  case 64: /* delete_stmt: DELETE FROM ID where  */
#line 576 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DELETE);
      (yyval.sql_node)->deletion.relation_name = (yyvsp[-1].cstring);
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.sql_node)->deletion.conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 2292 "yacc_sql.tab.c"
    break;

  case 65: /* update_stmt: UPDATE ID SET ID EQ value where  */
#line 587 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_UPDATE);
      (yyval.sql_node)->update.relation_name = (yyvsp[-5].cstring);
      (yyval.sql_node)->update.attribute_name = (yyvsp[-3].cstring);
      (yyval.sql_node)->update.value = *(yyvsp[-1].value);
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.sql_node)->update.conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 2307 "yacc_sql.tab.c"
    break;

  case 66: /* select_stmt: SELECT projection_list FROM rel_list join_list where group_by having order_by  */
#line 600 "yacc_sql.y"
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
#line 2351 "yacc_sql.tab.c"
    break;

  case 67: /* select_stmt: SELECT projection_list order_by  */
#line 640 "yacc_sql.y"
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
#line 2368 "yacc_sql.tab.c"
    break;

  case 68: /* calc_stmt: CALC expression_list  */
#line 655 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CALC);
      (yyval.sql_node)->calc.expressions.swap(*(yyvsp[0].expression_list));
      delete (yyvsp[0].expression_list);
    }
#line 2378 "yacc_sql.tab.c"
    break;

  case 69: /* expression_list: expression  */
#line 664 "yacc_sql.y"
    {
      (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      (yyval.expression_list)->emplace_back((yyvsp[0].expression));
    }
#line 2387 "yacc_sql.tab.c"
    break;

  case 70: /* expression_list: expression COMMA expression_list  */
#line 669 "yacc_sql.y"
    {
      if ((yyvsp[0].expression_list) != nullptr) {
        (yyval.expression_list) = (yyvsp[0].expression_list);
      } else {
        (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      }
      (yyval.expression_list)->emplace((yyval.expression_list)->begin(), (yyvsp[-2].expression));
    }
#line 2400 "yacc_sql.tab.c"
    break;

  case 71: /* projection_list: expression opt_as_alias  */
#line 681 "yacc_sql.y"
    {
      if ((yyvsp[0].cstring) != nullptr) {
        (yyvsp[-1].expression)->set_name((yyvsp[0].cstring));
      }
      (yyval.expression_list) = new vector<unique_ptr<Expression>>;
      (yyval.expression_list)->emplace_back((yyvsp[-1].expression));
    }
#line 2412 "yacc_sql.tab.c"
    break;

  case 72: /* projection_list: expression opt_as_alias COMMA projection_list  */
#line 689 "yacc_sql.y"
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
#line 2428 "yacc_sql.tab.c"
    break;

  case 73: /* opt_as_alias: %empty  */
#line 703 "yacc_sql.y"
                { (yyval.cstring) = nullptr; }
#line 2434 "yacc_sql.tab.c"
    break;

  case 74: /* opt_as_alias: ID  */
#line 704 "yacc_sql.y"
         { (yyval.cstring) = (yyvsp[0].cstring); }
#line 2440 "yacc_sql.tab.c"
    break;

  case 75: /* opt_as_alias: AS ID  */
#line 705 "yacc_sql.y"
            { (yyval.cstring) = (yyvsp[0].cstring); }
#line 2446 "yacc_sql.tab.c"
    break;

  case 76: /* expression: expression '+' expression  */
#line 709 "yacc_sql.y"
                              {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::ADD, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2454 "yacc_sql.tab.c"
    break;

  case 77: /* expression: expression '-' expression  */
#line 712 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::SUB, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2462 "yacc_sql.tab.c"
    break;

  case 78: /* expression: expression '*' expression  */
#line 715 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::MUL, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2470 "yacc_sql.tab.c"
    break;

  case 79: /* expression: expression '/' expression  */
#line 718 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::DIV, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2478 "yacc_sql.tab.c"
    break;

  case 80: /* expression: LBRACE expression RBRACE  */
#line 721 "yacc_sql.y"
                               {
      (yyval.expression) = (yyvsp[-1].expression);
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2487 "yacc_sql.tab.c"
    break;

  case 81: /* expression: '-' expression  */
#line 725 "yacc_sql.y"
                                  {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::NEGATIVE, (yyvsp[0].expression), nullptr, sql_string, &(yyloc));
    }
#line 2495 "yacc_sql.tab.c"
    break;

  case 82: /* expression: value  */
#line 728 "yacc_sql.y"
            {
      (yyval.expression) = new ValueExpr(*(yyvsp[0].value));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete (yyvsp[0].value);
    }
#line 2505 "yacc_sql.tab.c"
    break;

  case 83: /* expression: ID LBRACE RBRACE  */
#line 734 "yacc_sql.y"
                       {
      std::vector<std::unique_ptr<Expression>> args;
      (yyval.expression) = new FunctionExpr((yyvsp[-2].cstring), std::move(args));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2515 "yacc_sql.tab.c"
    break;

  case 84: /* expression: ID LBRACE expression_list RBRACE  */
#line 739 "yacc_sql.y"
                                       {
      std::vector<std::unique_ptr<Expression>> args;
      if ((yyvsp[-1].expression_list) != nullptr) {
        args.swap(*(yyvsp[-1].expression_list));
        delete (yyvsp[-1].expression_list);
      }
      (yyval.expression) = new FunctionExpr((yyvsp[-3].cstring), std::move(args));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2529 "yacc_sql.tab.c"
    break;

  case 85: /* expression: rel_attr  */
#line 748 "yacc_sql.y"
               {
      RelAttrSqlNode *node = (yyvsp[0].rel_attr);
      (yyval.expression) = new UnboundFieldExpr(node->relation_name, node->attribute_name);
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete (yyvsp[0].rel_attr);
    }
#line 2540 "yacc_sql.tab.c"
    break;

  case 86: /* expression: '*'  */
#line 754 "yacc_sql.y"
          {
      (yyval.expression) = new StarExpr();
    }
#line 2548 "yacc_sql.tab.c"
    break;

  case 87: /* expression: aggregate_expr  */
#line 757 "yacc_sql.y"
                     {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 2556 "yacc_sql.tab.c"
    break;

  case 88: /* rel_attr: ID  */
#line 763 "yacc_sql.y"
       {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->attribute_name = (yyvsp[0].cstring);
    }
#line 2565 "yacc_sql.tab.c"
    break;

  case 89: /* rel_attr: ID DOT ID  */
#line 767 "yacc_sql.y"
                {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->relation_name  = (yyvsp[-2].cstring);
      (yyval.rel_attr)->attribute_name = (yyvsp[0].cstring);
    }
#line 2575 "yacc_sql.tab.c"
    break;

  case 90: /* relation: ID  */
#line 775 "yacc_sql.y"
       {
      (yyval.relation_with_alias) = new RelationWithAliasSqlNode();
      (yyval.relation_with_alias)->table_name = (yyvsp[0].cstring);
    }
#line 2584 "yacc_sql.tab.c"
    break;

  case 91: /* relation: ID ID  */
#line 779 "yacc_sql.y"
            {
      (yyval.relation_with_alias) = new RelationWithAliasSqlNode();
      (yyval.relation_with_alias)->table_name = (yyvsp[-1].cstring);
      (yyval.relation_with_alias)->alias = (yyvsp[0].cstring);
    }
#line 2594 "yacc_sql.tab.c"
    break;

  case 92: /* relation: ID AS ID  */
#line 784 "yacc_sql.y"
               {
      (yyval.relation_with_alias) = new RelationWithAliasSqlNode();
      (yyval.relation_with_alias)->table_name = (yyvsp[-2].cstring);
      (yyval.relation_with_alias)->alias = (yyvsp[0].cstring);
    }
#line 2604 "yacc_sql.tab.c"
    break;

  case 93: /* rel_list: relation  */
#line 791 "yacc_sql.y"
             {
      (yyval.relation_with_alias_list) = new vector<RelationWithAliasSqlNode>();
      (yyval.relation_with_alias_list)->emplace_back(std::move(*(yyvsp[0].relation_with_alias)));
      delete (yyvsp[0].relation_with_alias);
    }
#line 2614 "yacc_sql.tab.c"
    break;

  case 94: /* rel_list: relation COMMA rel_list  */
#line 796 "yacc_sql.y"
                              {
      if ((yyvsp[0].relation_with_alias_list) != nullptr) {
        (yyval.relation_with_alias_list) = (yyvsp[0].relation_with_alias_list);
      } else {
        (yyval.relation_with_alias_list) = new vector<RelationWithAliasSqlNode>();
      }
      (yyval.relation_with_alias_list)->emplace((yyval.relation_with_alias_list)->begin(), std::move(*(yyvsp[-2].relation_with_alias)));
      delete (yyvsp[-2].relation_with_alias);
    }
#line 2628 "yacc_sql.tab.c"
    break;

  case 95: /* where: %empty  */
#line 811 "yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 2636 "yacc_sql.tab.c"
    break;

  case 96: /* where: WHERE condition_list  */
#line 814 "yacc_sql.y"
                           {
      (yyval.condition_list) = (yyvsp[0].condition_list);
    }
#line 2644 "yacc_sql.tab.c"
    break;

  case 97: /* condition_list: %empty  */
#line 820 "yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 2652 "yacc_sql.tab.c"
    break;

  case 98: /* condition_list: condition  */
#line 823 "yacc_sql.y"
                {
      (yyval.condition_list) = new vector<ConditionSqlNode>;
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[0].condition)));
      delete (yyvsp[0].condition);
    }
#line 2662 "yacc_sql.tab.c"
    break;

  case 99: /* condition_list: condition AND condition_list  */
#line 828 "yacc_sql.y"
                                   {
      (yyval.condition_list) = (yyvsp[0].condition_list);
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[-2].condition)));
      delete (yyvsp[-2].condition);
    }
#line 2672 "yacc_sql.tab.c"
    break;

  case 100: /* condition: expression comp_op expression  */
#line 836 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr.reset((yyvsp[-2].expression));
      (yyval.condition)->right_expr.reset((yyvsp[0].expression));
      (yyval.condition)->comp = (yyvsp[-1].comp);
    }
#line 2683 "yacc_sql.tab.c"
    break;

  case 101: /* comp_op: EQ  */
#line 845 "yacc_sql.y"
         { (yyval.comp) = EQUAL_TO; }
#line 2689 "yacc_sql.tab.c"
    break;

  case 102: /* comp_op: LT  */
#line 846 "yacc_sql.y"
         { (yyval.comp) = LESS_THAN; }
#line 2695 "yacc_sql.tab.c"
    break;

  case 103: /* comp_op: GT  */
#line 847 "yacc_sql.y"
         { (yyval.comp) = GREAT_THAN; }
#line 2701 "yacc_sql.tab.c"
    break;

  case 104: /* comp_op: LE  */
#line 848 "yacc_sql.y"
         { (yyval.comp) = LESS_EQUAL; }
#line 2707 "yacc_sql.tab.c"
    break;

  case 105: /* comp_op: GE  */
#line 849 "yacc_sql.y"
         { (yyval.comp) = GREAT_EQUAL; }
#line 2713 "yacc_sql.tab.c"
    break;

  case 106: /* comp_op: NE  */
#line 850 "yacc_sql.y"
         { (yyval.comp) = NOT_EQUAL; }
#line 2719 "yacc_sql.tab.c"
    break;

  case 107: /* comp_op: LIKE  */
#line 851 "yacc_sql.y"
           { (yyval.comp) = LIKE_OP; }
#line 2725 "yacc_sql.tab.c"
    break;

  case 108: /* comp_op: NOT LIKE  */
#line 852 "yacc_sql.y"
               { (yyval.comp) = NOT_LIKE_OP; }
#line 2731 "yacc_sql.tab.c"
    break;

  case 109: /* group_by: %empty  */
#line 858 "yacc_sql.y"
    {
      (yyval.expression_list) = nullptr;
    }
#line 2739 "yacc_sql.tab.c"
    break;

  case 110: /* group_by: GROUP BY expression_list  */
#line 862 "yacc_sql.y"
    {
      (yyval.expression_list) = (yyvsp[0].expression_list);
    }
#line 2747 "yacc_sql.tab.c"
    break;

  case 111: /* having: %empty  */
#line 869 "yacc_sql.y"
    {
      (yyval.expression) = nullptr;
    }
#line 2755 "yacc_sql.tab.c"
    break;

  case 112: /* having: HAVING expression  */
#line 873 "yacc_sql.y"
    {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 2763 "yacc_sql.tab.c"
    break;

  case 113: /* load_data_stmt: LOAD DATA INFILE SSS INTO TABLE ID  */
#line 879 "yacc_sql.y"
    {
      char *tmp_file_name = common::substr((yyvsp[-3].cstring), 1, strlen((yyvsp[-3].cstring)) - 2);

      (yyval.sql_node) = new ParsedSqlNode(SCF_LOAD_DATA);
      (yyval.sql_node)->load_data.relation_name = (yyvsp[0].cstring);
      (yyval.sql_node)->load_data.file_name = tmp_file_name;
      free(tmp_file_name);
    }
#line 2776 "yacc_sql.tab.c"
    break;

  case 114: /* explain_stmt: EXPLAIN command_wrapper  */
#line 891 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXPLAIN);
      (yyval.sql_node)->explain.sql_node = unique_ptr<ParsedSqlNode>((yyvsp[0].sql_node));
    }
#line 2785 "yacc_sql.tab.c"
    break;

  case 115: /* set_variable_stmt: SET ID EQ value  */
#line 899 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SET_VARIABLE);
      (yyval.sql_node)->set_variable.name  = (yyvsp[-2].cstring);
      (yyval.sql_node)->set_variable.value = *(yyvsp[0].value);
      delete (yyvsp[0].value);
    }
#line 2796 "yacc_sql.tab.c"
    break;

  case 116: /* vector_literal: LSQUARE number_list RSQUARE  */
#line 908 "yacc_sql.y"
                                {
      (yyval.value) = new Value();

      // 将number_list转换为向量字符串格式
      std::string vector_str = "[";
      if ((yyvsp[-1].float_list) != nullptr) {
        for (size_t i = 0; i < (yyvsp[-1].float_list)->size(); i++) {
          if (i > 0) vector_str += ",";
          vector_str += std::to_string((*(yyvsp[-1].float_list))[i]);
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
#line 2824 "yacc_sql.tab.c"
    break;

  case 117: /* number_list: number  */
#line 934 "yacc_sql.y"
           {
      (yyval.float_list) = new std::vector<float>();
      (yyval.float_list)->push_back((yyvsp[0].number));
    }
#line 2833 "yacc_sql.tab.c"
    break;

  case 118: /* number_list: number_list COMMA number  */
#line 938 "yacc_sql.y"
                               {
      (yyval.float_list) = (yyvsp[-2].float_list);
      (yyval.float_list)->push_back((yyvsp[0].number));
    }
#line 2842 "yacc_sql.tab.c"
    break;

  case 119: /* order_by: %empty  */
#line 946 "yacc_sql.y"
    {
      (yyval.order_by_list) = nullptr;
    }
#line 2850 "yacc_sql.tab.c"
    break;

  case 120: /* order_by: ORDER BY order_by_item_list  */
#line 950 "yacc_sql.y"
    {
      (yyval.order_by_list) = (yyvsp[0].order_by_list);
    }
#line 2858 "yacc_sql.tab.c"
    break;

  case 121: /* order_by_item_list: order_by_item  */
#line 957 "yacc_sql.y"
    {
      (yyval.order_by_list) = new std::vector<OrderBySqlNode>();
      (yyval.order_by_list)->emplace_back(std::move(*(yyvsp[0].order_by_item)));
      delete (yyvsp[0].order_by_item);
    }
#line 2868 "yacc_sql.tab.c"
    break;

  case 122: /* order_by_item_list: order_by_item_list COMMA order_by_item  */
#line 963 "yacc_sql.y"
    {
      (yyval.order_by_list) = (yyvsp[-2].order_by_list);
      (yyval.order_by_list)->emplace_back(std::move(*(yyvsp[0].order_by_item)));
      delete (yyvsp[0].order_by_item);
    }
#line 2878 "yacc_sql.tab.c"
    break;

  case 123: /* order_by_item: expression  */
#line 972 "yacc_sql.y"
    {
      (yyval.order_by_item) = new OrderBySqlNode();
      (yyval.order_by_item)->expression = unique_ptr<Expression>((yyvsp[0].expression));
      (yyval.order_by_item)->direction = ASC_ORDER;  // 默认升序
    }
#line 2888 "yacc_sql.tab.c"
    break;

  case 124: /* order_by_item: expression ASC  */
#line 978 "yacc_sql.y"
    {
      (yyval.order_by_item) = new OrderBySqlNode();
      (yyval.order_by_item)->expression = unique_ptr<Expression>((yyvsp[-1].expression));
      (yyval.order_by_item)->direction = ASC_ORDER;
    }
#line 2898 "yacc_sql.tab.c"
    break;

  case 125: /* order_by_item: expression DESC  */
#line 984 "yacc_sql.y"
    {
      (yyval.order_by_item) = new OrderBySqlNode();
      (yyval.order_by_item)->expression = unique_ptr<Expression>((yyvsp[-1].expression));
      (yyval.order_by_item)->direction = DESC_ORDER;
    }
#line 2908 "yacc_sql.tab.c"
    break;

  case 126: /* aggregate_expr: COUNT LBRACE '*' RBRACE  */
#line 992 "yacc_sql.y"
                            {
      (yyval.expression) = new UnboundAggregateExpr("count", nullptr);
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2917 "yacc_sql.tab.c"
    break;

  case 127: /* aggregate_expr: COUNT LBRACE expression RBRACE  */
#line 996 "yacc_sql.y"
                                     {
      (yyval.expression) = new UnboundAggregateExpr("count", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2926 "yacc_sql.tab.c"
    break;

  case 128: /* aggregate_expr: SUM LBRACE expression RBRACE  */
#line 1000 "yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("sum", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2935 "yacc_sql.tab.c"
    break;

  case 129: /* aggregate_expr: MAX LBRACE expression RBRACE  */
#line 1004 "yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("max", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2944 "yacc_sql.tab.c"
    break;

  case 130: /* aggregate_expr: MIN LBRACE expression RBRACE  */
#line 1008 "yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("min", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2953 "yacc_sql.tab.c"
    break;

  case 131: /* aggregate_expr: AVG LBRACE expression RBRACE  */
#line 1012 "yacc_sql.y"
                                   {
      (yyval.expression) = new UnboundAggregateExpr("avg", unique_ptr<Expression>((yyvsp[-1].expression)));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2962 "yacc_sql.tab.c"
    break;

  case 132: /* join_list: %empty  */
#line 1020 "yacc_sql.y"
    {
      (yyval.join_list) = nullptr;
    }
#line 2970 "yacc_sql.tab.c"
    break;

  case 133: /* join_list: join_list join_item  */
#line 1024 "yacc_sql.y"
    {
      if ((yyvsp[-1].join_list) != nullptr) {
        (yyval.join_list) = (yyvsp[-1].join_list);
      } else {
        (yyval.join_list) = new std::vector<JoinSqlNode>();
      }
      (yyval.join_list)->emplace_back(std::move(*(yyvsp[0].join_item)));
      delete (yyvsp[0].join_item);
    }
#line 2984 "yacc_sql.tab.c"
    break;

  case 134: /* join_item: INNER JOIN ID ON condition_list  */
#line 1037 "yacc_sql.y"
    {
      (yyval.join_item) = new JoinSqlNode();
      (yyval.join_item)->table_name = (yyvsp[-2].cstring);
      (yyval.join_item)->join_type = INNER_JOIN;
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.join_item)->conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 2998 "yacc_sql.tab.c"
    break;

  case 135: /* join_item: INNER JOIN ID ID ON condition_list  */
#line 1047 "yacc_sql.y"
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
#line 3013 "yacc_sql.tab.c"
    break;

  case 136: /* join_item: INNER JOIN ID AS ID ON condition_list  */
#line 1058 "yacc_sql.y"
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
#line 3028 "yacc_sql.tab.c"
    break;

  case 137: /* join_item: JOIN ID ON condition_list  */
#line 1069 "yacc_sql.y"
    {
      (yyval.join_item) = new JoinSqlNode();
      (yyval.join_item)->table_name = (yyvsp[-2].cstring);
      (yyval.join_item)->join_type = INNER_JOIN;  // 默认为INNER JOIN
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.join_item)->conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
    }
#line 3042 "yacc_sql.tab.c"
    break;

  case 138: /* join_item: JOIN ID ID ON condition_list  */
#line 1079 "yacc_sql.y"
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
#line 3057 "yacc_sql.tab.c"
    break;

  case 139: /* join_item: JOIN ID AS ID ON condition_list  */
#line 1090 "yacc_sql.y"
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
#line 3072 "yacc_sql.tab.c"
    break;


#line 3076 "yacc_sql.tab.c"

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

#line 1105 "yacc_sql.y"

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
