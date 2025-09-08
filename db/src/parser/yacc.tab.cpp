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
#line 1 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"

#include "ast.h"
#include "yacc.tab.h"
#include <iostream>
#include <memory>

int yylex(YYSTYPE *yylval, YYLTYPE *yylloc);

void yyerror(YYLTYPE *locp, const char* s) {
    std::cerr << "Parser Error at line " << locp->first_line << " column " << locp->first_column << ": " << s << std::endl;
}

using namespace ast;

#line 86 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"

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

#include "yacc.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_SHOW = 3,                       /* SHOW  */
  YYSYMBOL_TABLES = 4,                     /* TABLES  */
  YYSYMBOL_CREATE = 5,                     /* CREATE  */
  YYSYMBOL_TABLE = 6,                      /* TABLE  */
  YYSYMBOL_DROP = 7,                       /* DROP  */
  YYSYMBOL_DESC = 8,                       /* DESC  */
  YYSYMBOL_INSERT = 9,                     /* INSERT  */
  YYSYMBOL_INTO = 10,                      /* INTO  */
  YYSYMBOL_VALUES = 11,                    /* VALUES  */
  YYSYMBOL_DELETE = 12,                    /* DELETE  */
  YYSYMBOL_FROM = 13,                      /* FROM  */
  YYSYMBOL_ASC = 14,                       /* ASC  */
  YYSYMBOL_ORDER = 15,                     /* ORDER  */
  YYSYMBOL_BY = 16,                        /* BY  */
  YYSYMBOL_SUM = 17,                       /* SUM  */
  YYSYMBOL_HAVING = 18,                    /* HAVING  */
  YYSYMBOL_ON = 19,                        /* ON  */
  YYSYMBOL_COUNT = 20,                     /* COUNT  */
  YYSYMBOL_MIN = 21,                       /* MIN  */
  YYSYMBOL_MAX = 22,                       /* MAX  */
  YYSYMBOL_AVG = 23,                       /* AVG  */
  YYSYMBOL_LOAD = 24,                      /* LOAD  */
  YYSYMBOL_OUTPUT_FILE = 25,               /* OUTPUT_FILE  */
  YYSYMBOL_OFF = 26,                       /* OFF  */
  YYSYMBOL_EXPLAIN = 27,                   /* EXPLAIN  */
  YYSYMBOL_WHERE = 28,                     /* WHERE  */
  YYSYMBOL_UPDATE = 29,                    /* UPDATE  */
  YYSYMBOL_SET = 30,                       /* SET  */
  YYSYMBOL_SELECT = 31,                    /* SELECT  */
  YYSYMBOL_INT = 32,                       /* INT  */
  YYSYMBOL_CHAR = 33,                      /* CHAR  */
  YYSYMBOL_FLOAT = 34,                     /* FLOAT  */
  YYSYMBOL_INDEX = 35,                     /* INDEX  */
  YYSYMBOL_AND = 36,                       /* AND  */
  YYSYMBOL_JOIN = 37,                      /* JOIN  */
  YYSYMBOL_EXIT = 38,                      /* EXIT  */
  YYSYMBOL_HELP = 39,                      /* HELP  */
  YYSYMBOL_TXN_BEGIN = 40,                 /* TXN_BEGIN  */
  YYSYMBOL_TXN_COMMIT = 41,                /* TXN_COMMIT  */
  YYSYMBOL_TXN_ABORT = 42,                 /* TXN_ABORT  */
  YYSYMBOL_TXN_ROLLBACK = 43,              /* TXN_ROLLBACK  */
  YYSYMBOL_ORDER_BY = 44,                  /* ORDER_BY  */
  YYSYMBOL_ENABLE_NESTLOOP = 45,           /* ENABLE_NESTLOOP  */
  YYSYMBOL_ENABLE_SORTMERGE = 46,          /* ENABLE_SORTMERGE  */
  YYSYMBOL_SEMI = 47,                      /* SEMI  */
  YYSYMBOL_GROUP = 48,                     /* GROUP  */
  YYSYMBOL_AS = 49,                        /* AS  */
  YYSYMBOL_LIMIT = 50,                     /* LIMIT  */
  YYSYMBOL_STATIC_CHECKPOINT = 51,         /* STATIC_CHECKPOINT  */
  YYSYMBOL_LEQ = 52,                       /* LEQ  */
  YYSYMBOL_NEQ = 53,                       /* NEQ  */
  YYSYMBOL_GEQ = 54,                       /* GEQ  */
  YYSYMBOL_T_EOF = 55,                     /* T_EOF  */
  YYSYMBOL_56_ = 56,                       /* '='  */
  YYSYMBOL_57_ = 57,                       /* '<'  */
  YYSYMBOL_58_ = 58,                       /* '>'  */
  YYSYMBOL_59_ = 59,                       /* '+'  */
  YYSYMBOL_60_ = 60,                       /* '-'  */
  YYSYMBOL_61_ = 61,                       /* '*'  */
  YYSYMBOL_62_ = 62,                       /* '/'  */
  YYSYMBOL_63_ = 63,                       /* '('  */
  YYSYMBOL_64_ = 64,                       /* ')'  */
  YYSYMBOL_UMINUS = 65,                    /* UMINUS  */
  YYSYMBOL_IDENTIFIER = 66,                /* IDENTIFIER  */
  YYSYMBOL_VALUE_STRING = 67,              /* VALUE_STRING  */
  YYSYMBOL_VALUE_INT = 68,                 /* VALUE_INT  */
  YYSYMBOL_VALUE_FLOAT = 69,               /* VALUE_FLOAT  */
  YYSYMBOL_VALUE_BOOL = 70,                /* VALUE_BOOL  */
  YYSYMBOL_FILE_NAME = 71,                 /* FILE_NAME  */
  YYSYMBOL_72_ = 72,                       /* ';'  */
  YYSYMBOL_73_ = 73,                       /* ','  */
  YYSYMBOL_74_ = 74,                       /* '.'  */
  YYSYMBOL_YYACCEPT = 75,                  /* $accept  */
  YYSYMBOL_start = 76,                     /* start  */
  YYSYMBOL_stmt = 77,                      /* stmt  */
  YYSYMBOL_txnStmt = 78,                   /* txnStmt  */
  YYSYMBOL_dbStmt = 79,                    /* dbStmt  */
  YYSYMBOL_setStmt = 80,                   /* setStmt  */
  YYSYMBOL_ddl = 81,                       /* ddl  */
  YYSYMBOL_dml = 82,                       /* dml  */
  YYSYMBOL_groupby_clause = 83,            /* groupby_clause  */
  YYSYMBOL_having_clause = 84,             /* having_clause  */
  YYSYMBOL_logical_expr = 85,              /* logical_expr  */
  YYSYMBOL_compare_expr = 86,              /* compare_expr  */
  YYSYMBOL_opt_order_clause = 87,          /* opt_order_clause  */
  YYSYMBOL_order_clause_list = 88,         /* order_clause_list  */
  YYSYMBOL_order_clause = 89,              /* order_clause  */
  YYSYMBOL_fieldList = 90,                 /* fieldList  */
  YYSYMBOL_colNameList = 91,               /* colNameList  */
  YYSYMBOL_field = 92,                     /* field  */
  YYSYMBOL_type = 93,                      /* type  */
  YYSYMBOL_valueList = 94,                 /* valueList  */
  YYSYMBOL_value = 95,                     /* value  */
  YYSYMBOL_condition = 96,                 /* condition  */
  YYSYMBOL_optWhereClause = 97,            /* optWhereClause  */
  YYSYMBOL_whereClause = 98,               /* whereClause  */
  YYSYMBOL_col = 99,                       /* col  */
  YYSYMBOL_colList = 100,                  /* colList  */
  YYSYMBOL_colItem = 101,                  /* colItem  */
  YYSYMBOL_agg_expr = 102,                 /* agg_expr  */
  YYSYMBOL_op = 103,                       /* op  */
  YYSYMBOL_expr = 104,                     /* expr  */
  YYSYMBOL_setClauses = 105,               /* setClauses  */
  YYSYMBOL_setClause = 106,                /* setClause  */
  YYSYMBOL_selector = 107,                 /* selector  */
  YYSYMBOL_tableList = 108,                /* tableList  */
  YYSYMBOL_opt_asc_desc = 109,             /* opt_asc_desc  */
  YYSYMBOL_opt_limit_clause = 110,         /* opt_limit_clause  */
  YYSYMBOL_set_knob_type = 111,            /* set_knob_type  */
  YYSYMBOL_file_path = 112,                /* file_path  */
  YYSYMBOL_firsts = 113,                   /* firsts  */
  YYSYMBOL_first = 114,                    /* first  */
  YYSYMBOL_last = 115,                     /* last  */
  YYSYMBOL_tbName = 116,                   /* tbName  */
  YYSYMBOL_file_name = 117,                /* file_name  */
  YYSYMBOL_colName = 118                   /* colName  */
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
typedef yytype_uint8 yy_state_t;

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
#define YYFINAL  62
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   279

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  75
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  44
/* YYNRULES -- Number of rules.  */
#define YYNRULES  125
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  245

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   317


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
      63,    64,    61,    59,    73,    60,    74,    62,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    72,
      57,    56,    58,     2,     2,     2,     2,     2,     2,     2,
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
      55,    65,    66,    67,    68,    69,    70,    71
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    72,    72,    77,    82,    87,    92,    97,   105,   106,
     107,   108,   109,   114,   118,   122,   126,   133,   137,   144,
     150,   154,   158,   162,   166,   170,   177,   181,   185,   189,
     193,   200,   201,   205,   206,   210,   214,   221,   226,   231,
     236,   245,   249,   253,   257,   264,   271,   275,   282,   286,
     293,   300,   304,   308,   315,   319,   326,   330,   334,   338,
     342,   346,   350,   354,   361,   368,   369,   376,   380,   387,
     391,   398,   402,   410,   414,   418,   422,   430,   434,   438,
     442,   446,   450,   457,   461,   465,   469,   473,   477,   484,
     488,   492,   496,   501,   514,   518,   522,   526,   530,   537,
     541,   548,   555,   559,   563,   567,   571,   575,   579,   586,
     587,   588,   592,   596,   600,   601,   605,   609,   616,   620,
     627,   631,   638,   644,   645,   646
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
  "\"end of file\"", "error", "\"invalid token\"", "SHOW", "TABLES",
  "CREATE", "TABLE", "DROP", "DESC", "INSERT", "INTO", "VALUES", "DELETE",
  "FROM", "ASC", "ORDER", "BY", "SUM", "HAVING", "ON", "COUNT", "MIN",
  "MAX", "AVG", "LOAD", "OUTPUT_FILE", "OFF", "EXPLAIN", "WHERE", "UPDATE",
  "SET", "SELECT", "INT", "CHAR", "FLOAT", "INDEX", "AND", "JOIN", "EXIT",
  "HELP", "TXN_BEGIN", "TXN_COMMIT", "TXN_ABORT", "TXN_ROLLBACK",
  "ORDER_BY", "ENABLE_NESTLOOP", "ENABLE_SORTMERGE", "SEMI", "GROUP", "AS",
  "LIMIT", "STATIC_CHECKPOINT", "LEQ", "NEQ", "GEQ", "T_EOF", "'='", "'<'",
  "'>'", "'+'", "'-'", "'*'", "'/'", "'('", "')'", "UMINUS", "IDENTIFIER",
  "VALUE_STRING", "VALUE_INT", "VALUE_FLOAT", "VALUE_BOOL", "FILE_NAME",
  "';'", "','", "'.'", "$accept", "start", "stmt", "txnStmt", "dbStmt",
  "setStmt", "ddl", "dml", "groupby_clause", "having_clause",
  "logical_expr", "compare_expr", "opt_order_clause", "order_clause_list",
  "order_clause", "fieldList", "colNameList", "field", "type", "valueList",
  "value", "condition", "optWhereClause", "whereClause", "col", "colList",
  "colItem", "agg_expr", "op", "expr", "setClauses", "setClause",
  "selector", "tableList", "opt_asc_desc", "opt_limit_clause",
  "set_knob_type", "file_path", "firsts", "first", "last", "tbName",
  "file_name", "colName", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-136)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-124)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     204,     8,     1,     7,   -50,    20,    33,   107,   -50,   -11,
       0,  -136,  -136,  -136,  -136,  -136,  -136,  -136,    49,    -8,
    -136,  -136,  -136,  -136,  -136,  -136,    55,   -50,   -50,  -136,
     -50,   -50,  -136,  -136,   -50,   -50,  -136,  -136,     4,   103,
     -47,  -136,    61,   105,    19,  -136,  -136,    87,   106,   114,
     117,   119,   130,  -136,    76,   113,   118,  -136,   147,   166,
     123,  -136,  -136,  -136,   -50,   141,   143,  -136,   145,   199,
     186,   155,   -50,  -136,  -136,    27,  -136,   156,  -136,  -136,
     153,   158,    73,   158,   158,   158,   159,    71,   160,   -50,
     156,  -136,   156,   156,   156,   164,   158,  -136,  -136,  -136,
     163,  -136,   -13,  -136,   175,  -136,   168,   172,   173,   174,
     176,   177,  -136,  -136,  -136,    22,  -136,  -136,   -20,  -136,
      65,    36,  -136,    51,   116,  -136,   203,   196,  -136,   156,
    -136,   -12,  -136,  -136,  -136,  -136,  -136,  -136,   -50,   214,
     -50,   207,  -136,   156,  -136,   193,  -136,  -136,  -136,   156,
    -136,    43,    58,  -136,  -136,  -136,  -136,    69,  -136,   158,
    -136,  -136,  -136,  -136,  -136,  -136,   -12,  -136,    62,    85,
     -12,  -136,  -136,  -136,   128,   238,   -50,  -136,   242,   243,
    -136,   192,  -136,  -136,  -136,  -136,  -136,  -136,   116,  -136,
     128,  -136,  -136,  -136,  -136,  -136,  -136,    99,   -12,   -12,
     -12,   -12,   158,   244,    71,    71,   247,   200,  -136,  -136,
      79,    79,  -136,  -136,   203,   158,   118,   229,  -136,   196,
     196,   250,   217,  -136,   203,    71,    97,    97,   158,   201,
    -136,  -136,  -136,  -136,  -136,  -136,   195,  -136,   102,  -136,
     158,  -136,  -136,  -136,  -136
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     4,     3,    13,    14,    15,    16,     7,     0,     0,
      11,     8,    12,     9,    10,    17,     0,     0,     0,    25,
       0,     0,   123,    22,     0,     0,   124,   116,     0,     0,
       0,   118,     0,     0,     0,   114,   115,     0,     0,     0,
       0,     0,     0,   102,   125,    73,   103,    71,    74,     0,
       0,    70,     1,     2,     0,     0,     0,    21,     0,     0,
      65,     0,     0,   119,   117,     0,   121,     0,     6,     5,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    18,     0,     0,     0,     0,     0,    27,   120,    30,
       0,   125,    65,    99,     0,    19,     0,     0,     0,     0,
       0,     0,    75,    72,    76,    65,   104,    69,     0,    46,
       0,     0,    48,     0,     0,    67,    66,     0,   122,     0,
      28,     0,    79,    77,    78,    80,    81,    82,     0,     0,
       0,    31,    20,     0,    51,     0,    53,    50,    23,     0,
      24,     0,     0,    62,    56,    59,    63,     0,    54,     0,
      87,    86,    88,    83,    84,    85,     0,   100,     0,     0,
       0,    89,    90,    91,   101,   106,     0,   108,     0,    33,
      47,     0,    49,    57,    60,    58,    61,    26,     0,    68,
      64,    56,    59,    92,    56,    59,    93,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    42,     0,    55,    98,
      94,    95,    96,    97,   105,     0,    32,    34,    35,     0,
       0,     0,   113,    52,   107,     0,     0,     0,     0,     0,
      29,    36,    39,    40,    37,    38,    41,    43,   111,   112,
       0,   110,   109,    45,    44
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -136,  -136,  -136,  -136,  -136,  -136,  -136,  -136,  -136,  -136,
      45,  -136,  -136,  -136,    31,  -136,   178,   131,  -136,  -136,
    -123,   120,   -84,  -135,   -10,    72,   188,    -6,   -73,     2,
    -136,   144,  -136,  -136,  -136,  -136,  -136,  -136,  -136,   237,
    -136,    -2,   -37,   -53
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    18,    19,    20,    21,    22,    23,    24,   179,   206,
     217,   218,   222,   236,   237,   118,   121,   119,   147,   157,
     171,   125,    97,   126,   172,    56,    57,   173,   166,   174,
     102,   103,    59,   115,   243,   230,    47,    39,    40,    41,
      74,    60,    42,    61
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      55,   158,    33,    75,    58,    48,    43,    27,    49,    50,
      51,    52,    25,    30,    44,    96,    32,    48,   130,    36,
      49,    50,    51,    52,   104,    65,    66,    38,    67,    68,
      34,   141,    69,    70,    45,    46,    28,   117,    78,   120,
     122,   122,    31,    26,   142,    79,    35,   168,   169,    62,
      96,   170,    29,   143,    54,   153,   154,   155,   156,   138,
     129,    53,    91,   128,    63,   208,    54,   214,    64,   139,
      99,   106,   108,   109,   110,   111,   104,    55,    71,    48,
     224,    58,    49,    50,    51,    52,   127,   116,    48,    76,
     120,    49,    50,    51,    52,   140,   182,   144,   145,   146,
     148,   100,    48,   232,   234,    49,    50,    51,    52,   149,
     241,   183,   184,    72,    48,   150,   242,    49,    50,    51,
      52,   168,   169,    76,   149,   170,   185,   186,    54,   153,
     191,   192,   156,   187,   107,    77,   175,    54,   177,    54,
     200,   201,   188,    80,   168,   169,   226,   227,   170,   127,
    -123,    54,   153,   194,   195,   156,   151,   152,   198,   199,
     200,   201,    86,   209,   153,   154,   155,   156,   190,    81,
     193,   196,   197,    36,   203,   151,   152,    82,    37,    89,
      83,    38,    84,   153,   154,   155,   156,   198,   199,   200,
     201,    87,   127,    85,    55,   219,    88,    90,    58,   220,
     210,   211,   212,   213,    92,   127,    93,     1,    94,     2,
      95,     3,     4,     5,    96,   219,     6,    98,   238,   220,
     233,   235,   101,   105,    54,   112,   114,   124,     7,    36,
     238,   131,   132,     8,     9,    10,   133,   134,   135,   159,
     136,   137,    11,    12,    13,    14,    15,    16,   160,   161,
     162,   176,   163,   164,   165,   178,   181,   202,   204,    17,
     207,   205,   221,   215,   223,   225,   228,   229,   240,   239,
     231,   244,   123,   167,   180,   113,   216,    73,     0,   189
};

static const yytype_int16 yycheck[] =
{
      10,   124,     4,    40,    10,    17,     8,     6,    20,    21,
      22,    23,     4,     6,    25,    28,    66,    17,   102,    66,
      20,    21,    22,    23,    77,    27,    28,    74,    30,    31,
      10,   115,    34,    35,    45,    46,    35,    90,    19,    92,
      93,    94,    35,    35,    64,    26,    13,    59,    60,     0,
      28,    63,    51,    73,    66,    67,    68,    69,    70,    37,
      73,    61,    64,   100,    72,   188,    66,   202,    13,    47,
      72,    81,    82,    83,    84,    85,   129,    87,    74,    17,
     215,    87,    20,    21,    22,    23,    96,    89,    17,    62,
     143,    20,    21,    22,    23,    73,   149,    32,    33,    34,
      64,    74,    17,   226,   227,    20,    21,    22,    23,    73,
       8,    68,    69,    10,    17,    64,    14,    20,    21,    22,
      23,    59,    60,    62,    73,    63,    68,    69,    66,    67,
      68,    69,    70,    64,    61,    30,   138,    66,   140,    66,
      61,    62,    73,    56,    59,    60,   219,   220,    63,   159,
      74,    66,    67,    68,    69,    70,    59,    60,    59,    60,
      61,    62,    49,    64,    67,    68,    69,    70,   166,    63,
     168,   169,   170,    66,   176,    59,    60,    63,    71,    13,
      63,    74,    63,    67,    68,    69,    70,    59,    60,    61,
      62,    73,   202,    63,   204,   205,    49,    74,   204,   205,
     198,   199,   200,   201,    63,   215,    63,     3,    63,     5,
      11,     7,     8,     9,    28,   225,    12,    62,   228,   225,
     226,   227,    66,    70,    66,    66,    66,    63,    24,    66,
     240,    56,    64,    29,    30,    31,    64,    64,    64,    36,
      64,    64,    38,    39,    40,    41,    42,    43,    52,    53,
      54,    37,    56,    57,    58,    48,    63,    19,    16,    55,
      68,    18,    15,    19,    64,    36,    16,    50,    73,    68,
     225,   240,    94,   129,   143,    87,   204,    40,    -1,   159
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     5,     7,     8,     9,    12,    24,    29,    30,
      31,    38,    39,    40,    41,    42,    43,    55,    76,    77,
      78,    79,    80,    81,    82,     4,    35,     6,    35,    51,
       6,    35,    66,   116,    10,    13,    66,    71,    74,   112,
     113,   114,   117,   116,    25,    45,    46,   111,    17,    20,
      21,    22,    23,    61,    66,    99,   100,   101,   102,   107,
     116,   118,     0,    72,    13,   116,   116,   116,   116,   116,
     116,    74,    10,   114,   115,   117,    62,    30,    19,    26,
      56,    63,    63,    63,    63,    63,    49,    73,    49,    13,
      74,   116,    63,    63,    63,    11,    28,    97,    62,   116,
      74,    66,   105,   106,   118,    70,    99,    61,    99,    99,
      99,    99,    66,   101,    66,   108,   116,   118,    90,    92,
     118,    91,   118,    91,    63,    96,    98,    99,   117,    73,
      97,    56,    64,    64,    64,    64,    64,    64,    37,    47,
      73,    97,    64,    73,    32,    33,    34,    93,    64,    73,
      64,    59,    60,    67,    68,    69,    70,    94,    95,    36,
      52,    53,    54,    56,    57,    58,   103,   106,    59,    60,
      63,    95,    99,   102,   104,   116,    37,   116,    48,    83,
      92,    63,   118,    68,    69,    68,    69,    64,    73,    96,
     104,    68,    69,   104,    68,    69,   104,   104,    59,    60,
      61,    62,    19,   116,    16,    18,    84,    68,    95,    64,
     104,   104,   104,   104,    98,    19,   100,    85,    86,    99,
     102,    15,    87,    64,    98,    36,   103,   103,    16,    50,
     110,    85,    95,   102,    95,   102,    88,    89,    99,    68,
      73,     8,    14,   109,    89
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    75,    76,    76,    76,    76,    76,    76,    77,    77,
      77,    77,    77,    78,    78,    78,    78,    79,    79,    80,
      81,    81,    81,    81,    81,    81,    82,    82,    82,    82,
      82,    83,    83,    84,    84,    85,    85,    86,    86,    86,
      86,    87,    87,    88,    88,    89,    90,    90,    91,    91,
      92,    93,    93,    93,    94,    94,    95,    95,    95,    95,
      95,    95,    95,    95,    96,    97,    97,    98,    98,    99,
      99,   100,   100,   101,   101,   101,   101,   102,   102,   102,
     102,   102,   102,   103,   103,   103,   103,   103,   103,   104,
     104,   104,   104,   104,   104,   104,   104,   104,   104,   105,
     105,   106,   107,   107,   108,   108,   108,   108,   108,   109,
     109,   109,   110,   110,   111,   111,   112,   112,   113,   113,
     114,   114,   115,   116,   117,   118
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     4,     4,
       6,     3,     2,     6,     6,     2,     7,     4,     5,     9,
       4,     0,     3,     0,     2,     1,     3,     3,     3,     3,
       3,     3,     0,     1,     3,     2,     1,     3,     1,     3,
       2,     1,     4,     1,     1,     3,     1,     2,     2,     1,
       2,     2,     1,     1,     3,     0,     2,     1,     3,     3,
       1,     1,     3,     1,     1,     3,     3,     4,     4,     4,
       4,     4,     4,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     3,     3,     3,     3,     3,     1,
       3,     3,     1,     1,     1,     5,     3,     6,     3,     1,
       1,     0,     2,     0,     1,     1,     1,     2,     1,     2,
       3,     2,     3,     1,     1,     1
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
        yyerror (&yylloc, YY_("syntax error: cannot back up")); \
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
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
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
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
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
                 int yyrule)
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
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
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
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
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
yyparse (void)
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
      yychar = yylex (&yylval, &yylloc);
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
  case 2: /* start: stmt ';'  */
#line 73 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        parse_tree = (yyvsp[-1].sv_node);
        YYACCEPT;
    }
#line 1768 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 3: /* start: HELP  */
#line 78 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        parse_tree = std::make_shared<Help>();
        YYACCEPT;
    }
#line 1777 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 4: /* start: EXIT  */
#line 83 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
#line 1786 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 5: /* start: SET OUTPUT_FILE OFF  */
#line 88 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        parse_tree = std::make_shared<SetKnobStmt>(ast::SetKnobType::OutputFile, false);
        YYACCEPT;
    }
#line 1795 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 6: /* start: SET OUTPUT_FILE ON  */
#line 93 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        parse_tree = std::make_shared<SetKnobStmt>(ast::SetKnobType::OutputFile, true);
        YYACCEPT;
    }
#line 1804 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 7: /* start: T_EOF  */
#line 98 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
#line 1813 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 13: /* txnStmt: TXN_BEGIN  */
#line 115 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnBegin>();
    }
#line 1821 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 14: /* txnStmt: TXN_COMMIT  */
#line 119 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnCommit>();
    }
#line 1829 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 15: /* txnStmt: TXN_ABORT  */
#line 123 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnAbort>();
    }
#line 1837 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 16: /* txnStmt: TXN_ROLLBACK  */
#line 127 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnRollback>();
    }
#line 1845 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 17: /* dbStmt: SHOW TABLES  */
#line 134 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<ShowTables>();
    }
#line 1853 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 18: /* dbStmt: SHOW INDEX FROM tbName  */
#line 138 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<ShowIndex>((yyvsp[0].sv_str));
    }
#line 1861 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 19: /* setStmt: SET set_knob_type '=' VALUE_BOOL  */
#line 145 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<SetStmt>((yyvsp[-2].sv_setKnobType), (yyvsp[0].sv_bool));
    }
#line 1869 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 20: /* ddl: CREATE TABLE tbName '(' fieldList ')'  */
#line 151 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<CreateTable>((yyvsp[-3].sv_str), (yyvsp[-1].sv_fields));
    }
#line 1877 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 21: /* ddl: DROP TABLE tbName  */
#line 155 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DropTable>((yyvsp[0].sv_str));
    }
#line 1885 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 22: /* ddl: DESC tbName  */
#line 159 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DescTable>((yyvsp[0].sv_str));
    }
#line 1893 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 23: /* ddl: CREATE INDEX tbName '(' colNameList ')'  */
#line 163 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<CreateIndex>((yyvsp[-3].sv_str), (yyvsp[-1].sv_strs));
    }
#line 1901 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 24: /* ddl: DROP INDEX tbName '(' colNameList ')'  */
#line 167 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DropIndex>((yyvsp[-3].sv_str), (yyvsp[-1].sv_strs));
    }
#line 1909 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 25: /* ddl: CREATE STATIC_CHECKPOINT  */
#line 171 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<CreateStaticCheckpoint>();
    }
#line 1917 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 26: /* dml: INSERT INTO tbName VALUES '(' valueList ')'  */
#line 178 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<InsertStmt>((yyvsp[-4].sv_str), (yyvsp[-1].sv_vals));
    }
#line 1925 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 27: /* dml: DELETE FROM tbName optWhereClause  */
#line 182 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DeleteStmt>((yyvsp[-1].sv_str), (yyvsp[0].sv_conds));
    }
#line 1933 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 28: /* dml: UPDATE tbName SET setClauses optWhereClause  */
#line 186 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<UpdateStmt>((yyvsp[-3].sv_str), (yyvsp[-1].sv_set_clauses), (yyvsp[0].sv_conds));
    }
#line 1941 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 29: /* dml: SELECT selector FROM tableList optWhereClause groupby_clause having_clause opt_order_clause opt_limit_clause  */
#line 190 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<SelectStmt>((yyvsp[-7].sv_exprs), (yyvsp[-5].sv_node), (yyvsp[-4].sv_conds), (yyvsp[-1].sv_orderbys), (yyvsp[-3].sv_exprs), (yyvsp[-2].sv_expr), (yyvsp[0].sv_int));
    }
#line 1949 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 30: /* dml: LOAD file_path INTO tbName  */
#line 194 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<LoadStmt>((yyvsp[-2].sv_str), (yyvsp[0].sv_str));
    }
#line 1957 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 31: /* groupby_clause: %empty  */
#line 200 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
                { (yyval.sv_exprs) = {}; }
#line 1963 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 32: /* groupby_clause: GROUP BY colList  */
#line 201 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
                       { (yyval.sv_exprs) = (yyvsp[0].sv_exprs); }
#line 1969 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 33: /* having_clause: %empty  */
#line 205 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
                { (yyval.sv_expr) = nullptr; }
#line 1975 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 34: /* having_clause: HAVING logical_expr  */
#line 206 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
                          { (yyval.sv_expr) = (yyvsp[0].sv_expr); }
#line 1981 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 35: /* logical_expr: compare_expr  */
#line 211 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = (yyvsp[0].sv_expr);
    }
#line 1989 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 36: /* logical_expr: logical_expr AND logical_expr  */
#line 215 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::static_pointer_cast<Expr>(std::make_shared<LogicalExpr>((yyvsp[-2].sv_expr), LogicalExpr::AND, (yyvsp[0].sv_expr)));
    }
#line 1997 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 37: /* compare_expr: agg_expr op value  */
#line 222 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        // 聚合函数与常量比较，如 COUNT(*) > 1
        (yyval.sv_expr) = std::static_pointer_cast<Expr>(std::make_shared<CompareExpr>((yyvsp[-2].sv_expr), (yyvsp[-1].sv_comp_op), std::static_pointer_cast<Expr>((yyvsp[0].sv_val))));
    }
#line 2006 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 38: /* compare_expr: agg_expr op agg_expr  */
#line 227 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        // 聚合函数之间比较，如 MIN(score) > MAX(other)
        (yyval.sv_expr) = std::static_pointer_cast<Expr>(std::make_shared<CompareExpr>((yyvsp[-2].sv_expr), (yyvsp[-1].sv_comp_op), (yyvsp[0].sv_expr)));
    }
#line 2015 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 39: /* compare_expr: col op value  */
#line 232 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        // 列与常量比较
        (yyval.sv_expr) = std::static_pointer_cast<Expr>(std::make_shared<CompareExpr>(std::static_pointer_cast<Expr>((yyvsp[-2].sv_col)), (yyvsp[-1].sv_comp_op), std::static_pointer_cast<Expr>((yyvsp[0].sv_val))));
    }
#line 2024 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 40: /* compare_expr: col op agg_expr  */
#line 237 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        // 列与聚合函数比较
        (yyval.sv_expr) = std::static_pointer_cast<Expr>(std::make_shared<CompareExpr>(std::static_pointer_cast<Expr>((yyvsp[-2].sv_col)), (yyvsp[-1].sv_comp_op), (yyvsp[0].sv_expr)));
    }
#line 2033 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 41: /* opt_order_clause: ORDER BY order_clause_list  */
#line 246 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_orderbys) = (yyvsp[0].sv_orderbys);
    }
#line 2041 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 42: /* opt_order_clause: %empty  */
#line 249 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
                { (yyval.sv_orderbys) = {}; }
#line 2047 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 43: /* order_clause_list: order_clause  */
#line 254 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_orderbys) = std::vector<std::shared_ptr<OrderBy>>{(yyvsp[0].sv_orderby)};
    }
#line 2055 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 44: /* order_clause_list: order_clause_list ',' order_clause  */
#line 258 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_orderbys).push_back((yyvsp[0].sv_orderby));
    }
#line 2063 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 45: /* order_clause: col opt_asc_desc  */
#line 265 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_orderby) = std::make_shared<OrderBy>((yyvsp[-1].sv_col), (yyvsp[0].sv_orderby_dir));
    }
#line 2071 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 46: /* fieldList: field  */
#line 272 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_fields) = std::vector<std::shared_ptr<Field>>{(yyvsp[0].sv_field)};
    }
#line 2079 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 47: /* fieldList: fieldList ',' field  */
#line 276 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_fields).push_back((yyvsp[0].sv_field));
    }
#line 2087 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 48: /* colNameList: colName  */
#line 283 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_strs) = std::vector<std::string>{(yyvsp[0].sv_str)};
    }
#line 2095 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 49: /* colNameList: colNameList ',' colName  */
#line 287 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_strs).push_back((yyvsp[0].sv_str));
    }
#line 2103 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 50: /* field: colName type  */
#line 294 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_field) = std::make_shared<ColDef>((yyvsp[-1].sv_str), (yyvsp[0].sv_type_len));
    }
#line 2111 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 51: /* type: INT  */
#line 301 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_INT, sizeof(int));
    }
#line 2119 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 52: /* type: CHAR '(' VALUE_INT ')'  */
#line 305 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_STRING, (yyvsp[-1].sv_int));
    }
#line 2127 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 53: /* type: FLOAT  */
#line 309 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_FLOAT, sizeof(float));
    }
#line 2135 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 54: /* valueList: value  */
#line 316 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_vals) = std::vector<std::shared_ptr<Value>>{(yyvsp[0].sv_val)};
    }
#line 2143 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 55: /* valueList: valueList ',' value  */
#line 320 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_vals).push_back((yyvsp[0].sv_val));
    }
#line 2151 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 56: /* value: VALUE_INT  */
#line 327 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<IntLit>((yyvsp[0].sv_int));
    }
#line 2159 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 57: /* value: '+' VALUE_INT  */
#line 331 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<IntLit>((yyvsp[0].sv_int));
    }
#line 2167 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 58: /* value: '-' VALUE_INT  */
#line 335 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<IntLit>(-(yyvsp[0].sv_int));
    }
#line 2175 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 59: /* value: VALUE_FLOAT  */
#line 339 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<FloatLit>((yyvsp[0].sv_float));
    }
#line 2183 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 60: /* value: '+' VALUE_FLOAT  */
#line 343 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<FloatLit>((yyvsp[0].sv_float));
    }
#line 2191 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 61: /* value: '-' VALUE_FLOAT  */
#line 347 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<FloatLit>(-(yyvsp[0].sv_float));
    }
#line 2199 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 62: /* value: VALUE_STRING  */
#line 351 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<StringLit>((yyvsp[0].sv_str));
    }
#line 2207 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 63: /* value: VALUE_BOOL  */
#line 355 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<BoolLit>((yyvsp[0].sv_bool));
    }
#line 2215 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 64: /* condition: col op expr  */
#line 362 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_cond) = std::make_shared<BinaryExpr>((yyvsp[-2].sv_col), (yyvsp[-1].sv_comp_op), (yyvsp[0].sv_expr));
    }
#line 2223 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 65: /* optWhereClause: %empty  */
#line 368 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
                      { /* ignore*/ }
#line 2229 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 66: /* optWhereClause: WHERE whereClause  */
#line 370 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_conds) = (yyvsp[0].sv_conds);
    }
#line 2237 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 67: /* whereClause: condition  */
#line 377 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_conds) = std::vector<std::shared_ptr<BinaryExpr>>{(yyvsp[0].sv_cond)};
    }
#line 2245 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 68: /* whereClause: whereClause AND condition  */
#line 381 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_conds).push_back((yyvsp[0].sv_cond));
    }
#line 2253 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 69: /* col: tbName '.' colName  */
#line 388 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>((yyvsp[-2].sv_str), (yyvsp[0].sv_str));
    }
#line 2261 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 70: /* col: colName  */
#line 392 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>("", (yyvsp[0].sv_str));
    }
#line 2269 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 71: /* colList: colItem  */
#line 399 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_exprs).push_back((yyvsp[0].sv_expr));
    }
#line 2277 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 72: /* colList: colList ',' colItem  */
#line 403 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_exprs).push_back((yyvsp[0].sv_expr));
    }
#line 2285 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 73: /* colItem: col  */
#line 411 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = (yyvsp[0].sv_col);
    }
#line 2293 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 74: /* colItem: agg_expr  */
#line 415 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = (yyvsp[0].sv_expr);
    }
#line 2301 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 75: /* colItem: col AS IDENTIFIER  */
#line 419 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<AliasExpr>((yyvsp[-2].sv_col), (yyvsp[0].sv_str));
    }
#line 2309 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 76: /* colItem: agg_expr AS IDENTIFIER  */
#line 423 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<AliasExpr>((yyvsp[-2].sv_expr), (yyvsp[0].sv_str));
    }
#line 2317 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 77: /* agg_expr: COUNT '(' '*' ')'  */
#line 431 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<AggExpr>(AGG_COUNT, nullptr);
    }
#line 2325 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 78: /* agg_expr: COUNT '(' col ')'  */
#line 435 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<AggExpr>(AGG_COUNT, (yyvsp[-1].sv_col));
    }
#line 2333 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 79: /* agg_expr: SUM '(' col ')'  */
#line 439 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<AggExpr>(AGG_SUM, (yyvsp[-1].sv_col));
    }
#line 2341 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 80: /* agg_expr: MIN '(' col ')'  */
#line 443 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<AggExpr>(AGG_MIN, (yyvsp[-1].sv_col));
    }
#line 2349 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 81: /* agg_expr: MAX '(' col ')'  */
#line 447 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<AggExpr>(AGG_MAX, (yyvsp[-1].sv_col));
    }
#line 2357 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 82: /* agg_expr: AVG '(' col ')'  */
#line 451 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<AggExpr>(AGG_AVG, (yyvsp[-1].sv_col));
    }
#line 2365 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 83: /* op: '='  */
#line 458 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_EQ;
    }
#line 2373 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 84: /* op: '<'  */
#line 462 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_LT;
    }
#line 2381 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 85: /* op: '>'  */
#line 466 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_GT;
    }
#line 2389 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 86: /* op: NEQ  */
#line 470 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_NE;
    }
#line 2397 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 87: /* op: LEQ  */
#line 474 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_LE;
    }
#line 2405 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 88: /* op: GEQ  */
#line 478 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_GE;
    }
#line 2413 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 89: /* expr: value  */
#line 485 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::static_pointer_cast<Expr>((yyvsp[0].sv_val));
    }
#line 2421 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 90: /* expr: col  */
#line 489 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::static_pointer_cast<Expr>((yyvsp[0].sv_col));
    }
#line 2429 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 91: /* expr: agg_expr  */
#line 493 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::static_pointer_cast<Expr>((yyvsp[0].sv_expr));
    }
#line 2437 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 92: /* expr: '+' expr  */
#line 497 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        /* 一元+操作符，值不变 */
        (yyval.sv_expr) = (yyvsp[0].sv_expr);
    }
#line 2446 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 93: /* expr: '-' expr  */
#line 502 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        /* 一元-操作符，对数值取反 */
        if (auto intLit = std::dynamic_pointer_cast<IntLit>((yyvsp[0].sv_expr))) {
            (yyval.sv_expr) = std::make_shared<IntLit>(-intLit->val);
        } else if (auto floatLit = std::dynamic_pointer_cast<FloatLit>((yyvsp[0].sv_expr))) {
            (yyval.sv_expr) = std::make_shared<FloatLit>(-floatLit->val);
        } else {
            /* 如果不是简单数值，创建一个取反的算术表达式 */
            auto zero = std::make_shared<IntLit>(0);
            (yyval.sv_expr) = std::make_shared<ArithExpr>(std::static_pointer_cast<Expr>(zero), ArithOp::SUB, (yyvsp[0].sv_expr));
        }
    }
#line 2463 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 94: /* expr: expr '+' expr  */
#line 515 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<ArithExpr>((yyvsp[-2].sv_expr), ArithOp::ADD, (yyvsp[0].sv_expr));
    }
#line 2471 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 95: /* expr: expr '-' expr  */
#line 519 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<ArithExpr>((yyvsp[-2].sv_expr), ArithOp::SUB, (yyvsp[0].sv_expr));
    }
#line 2479 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 96: /* expr: expr '*' expr  */
#line 523 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<ArithExpr>((yyvsp[-2].sv_expr), ArithOp::MUL, (yyvsp[0].sv_expr));
    }
#line 2487 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 97: /* expr: expr '/' expr  */
#line 527 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<ArithExpr>((yyvsp[-2].sv_expr), ArithOp::DIV, (yyvsp[0].sv_expr));
    }
#line 2495 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 98: /* expr: '(' expr ')'  */
#line 531 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = (yyvsp[-1].sv_expr);
    }
#line 2503 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 99: /* setClauses: setClause  */
#line 538 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_set_clauses) = std::vector<std::shared_ptr<SetClause>>{(yyvsp[0].sv_set_clause)};
    }
#line 2511 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 100: /* setClauses: setClauses ',' setClause  */
#line 542 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_set_clauses).push_back((yyvsp[0].sv_set_clause));
    }
#line 2519 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 101: /* setClause: colName '=' expr  */
#line 549 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_set_clause) = std::make_shared<SetClause>((yyvsp[-2].sv_str), (yyvsp[0].sv_expr));
    }
#line 2527 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 102: /* selector: '*'  */
#line 556 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_exprs) = {};
    }
#line 2535 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 104: /* tableList: tbName  */
#line 564 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TableRef>((yyvsp[0].sv_str));
    }
#line 2543 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 105: /* tableList: tableList JOIN tbName ON whereClause  */
#line 568 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<JoinExpr>((yyvsp[-4].sv_node),std::make_shared<TableRef>((yyvsp[-2].sv_str)),(yyvsp[0].sv_conds),INNER_JOIN);
    }
#line 2551 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 106: /* tableList: tableList JOIN tbName  */
#line 572 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<JoinExpr>((yyvsp[-2].sv_node), std::make_shared<TableRef>((yyvsp[0].sv_str)), std::vector<std::shared_ptr<BinaryExpr>>{}, INNER_JOIN);
    }
#line 2559 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 107: /* tableList: tableList SEMI JOIN tbName ON whereClause  */
#line 576 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<JoinExpr>((yyvsp[-5].sv_node),std::make_shared<TableRef>((yyvsp[-2].sv_str)),(yyvsp[0].sv_conds),SEMI_JOIN);
    }
#line 2567 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 108: /* tableList: tableList ',' tbName  */
#line 580 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<JoinExpr>((yyvsp[-2].sv_node), std::make_shared<TableRef>((yyvsp[0].sv_str)), std::vector<std::shared_ptr<BinaryExpr>>{}, INNER_JOIN);
    }
#line 2575 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 109: /* opt_asc_desc: ASC  */
#line 586 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
                 { (yyval.sv_orderby_dir) = OrderBy_ASC;     }
#line 2581 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 110: /* opt_asc_desc: DESC  */
#line 587 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
                 { (yyval.sv_orderby_dir) = OrderBy_DESC;    }
#line 2587 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 111: /* opt_asc_desc: %empty  */
#line 588 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
            { (yyval.sv_orderby_dir) = OrderBy_DEFAULT; }
#line 2593 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 112: /* opt_limit_clause: LIMIT VALUE_INT  */
#line 593 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_int) = (yyvsp[0].sv_int);
    }
#line 2601 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 113: /* opt_limit_clause: %empty  */
#line 596 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
                { (yyval.sv_int) = -1; }
#line 2607 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 114: /* set_knob_type: ENABLE_NESTLOOP  */
#line 600 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
                    { (yyval.sv_setKnobType) = EnableNestLoop; }
#line 2613 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 115: /* set_knob_type: ENABLE_SORTMERGE  */
#line 601 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
                         { (yyval.sv_setKnobType) = EnableSortMerge; }
#line 2619 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 116: /* file_path: FILE_NAME  */
#line 606 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_str) = (yyvsp[0].sv_str);
    }
#line 2627 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 117: /* file_path: firsts last  */
#line 610 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_str) = (yyvsp[-1].sv_str) + (yyvsp[0].sv_str);
    }
#line 2635 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 118: /* firsts: first  */
#line 617 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_str) = (yyvsp[0].sv_str);
    }
#line 2643 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 119: /* firsts: firsts first  */
#line 621 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_str) += (yyvsp[0].sv_str);
    }
#line 2651 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 120: /* first: '.' '.' '/'  */
#line 628 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_str) = "../";
    }
#line 2659 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 121: /* first: file_name '/'  */
#line 632 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_str) = (yyvsp[-1].sv_str) + "/";
    }
#line 2667 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;

  case 122: /* last: file_name '.' file_name  */
#line 639 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"
    {
        (yyval.sv_str) = (yyvsp[-2].sv_str) + "." + (yyvsp[0].sv_str);
    }
#line 2675 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"
    break;


#line 2679 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.tab.cpp"

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
        yyerror (&yylloc, yymsgp);
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
                      yytoken, &yylval, &yylloc);
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
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
  yyerror (&yylloc, YY_("memory exhausted"));
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
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
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

#line 647 "/home/hxl/Desktop/work/rmdb/src/parser/yacc.y"

