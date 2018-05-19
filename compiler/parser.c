/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "parser.y" /* yacc.c:339  */

/*
  OMPi OpenMP Compiler
  == Copyright since 2001 the OMPi Team
  == Dept. of Computer Science & Engineering, University of Ioannina

  This file is part of OMPi.

  OMPi is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  OMPi is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OMPi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* parser.y */

/*
 * 2010/11/10:
 *   dropped OpenMP-specific for parsing; fewer rules, less code
 * 2009/05/11:
 *   added AUTO schedule type
 * 2009/05/03:
 *   added ATNODE ompix clause
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include "scanner.h"
#include "boolean.h"
#include "ompi.h"
#include "ast.h"
#include "symtab.h"
#include "ast_free.h"
#include "ast_copy.h"
#include "ast_vars.h"
#include "x_arith.h"

void    check_uknown_var(char *name);
void    parse_error(int exitvalue, char *format, ...);
void    parse_warning(char *format, ...);
void    yyerror(const char *s);
void    check_for_main_and_declare(astspec s, astdecl d);
void    add_declaration_links(astspec s, astdecl d);
astdecl fix_known_typename(astspec s);

aststmt pastree = NULL;       /* The generated AST */
aststmt pastree_stmt = NULL;  /* For when parsing statment strings */
astexpr pastree_expr = NULL;  /* For when parsing expression strings */
int     checkDecls = 1;       /* 0 when scanning strings (no context check) */
int     tempsave;
int     isTypedef  = 0;       /* To keep track of typedefs */

char    *parsingstring;       /* For error reporting when parsing string */

int     __has_target = 0;

//TODO this is not 100% correct (e.g. if returns lies in a dead code area).
/* Return and goto statements that lead outside the outlined region constitute
 * programmer errors and so we should stop the compilation. When simply 
 * analyzing vars (and not outlining) we ignore this rule.
 */
int     errorOnReturn = 0;

#line 145 "parser.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    START_SYMBOL_EXPRESSION = 258,
    START_SYMBOL_BLOCKLIST = 259,
    START_SYMBOL_TRANSUNIT = 260,
    IDENTIFIER = 261,
    TYPE_NAME = 262,
    CONSTANT = 263,
    STRING_LITERAL = 264,
    PTR_OP = 265,
    INC_OP = 266,
    DEC_OP = 267,
    LEFT_OP = 268,
    RIGHT_OP = 269,
    LE_OP = 270,
    GE_OP = 271,
    EQ_OP = 272,
    NE_OP = 273,
    AND_OP = 274,
    OR_OP = 275,
    MUL_ASSIGN = 276,
    DIV_ASSIGN = 277,
    MOD_ASSIGN = 278,
    ADD_ASSIGN = 279,
    SUB_ASSIGN = 280,
    LEFT_ASSIGN = 281,
    RIGHT_ASSIGN = 282,
    AND_ASSIGN = 283,
    XOR_ASSIGN = 284,
    OR_ASSIGN = 285,
    SIZEOF = 286,
    TYPEDEF = 287,
    EXTERN = 288,
    STATIC = 289,
    AUTO = 290,
    REGISTER = 291,
    RESTRICT = 292,
    CHAR = 293,
    SHORT = 294,
    INT = 295,
    LONG = 296,
    SIGNED = 297,
    UNSIGNED = 298,
    FLOAT = 299,
    DOUBLE = 300,
    CONST = 301,
    VOLATILE = 302,
    VOID = 303,
    INLINE = 304,
    UBOOL = 305,
    UCOMPLEX = 306,
    UIMAGINARY = 307,
    STRUCT = 308,
    UNION = 309,
    ENUM = 310,
    ELLIPSIS = 311,
    CASE = 312,
    DEFAULT = 313,
    IF = 314,
    ELSE = 315,
    SWITCH = 316,
    WHILE = 317,
    DO = 318,
    FOR = 319,
    GOTO = 320,
    CONTINUE = 321,
    BREAK = 322,
    RETURN = 323,
    __BUILTIN_VA_ARG = 324,
    __BUILTIN_OFFSETOF = 325,
    __BUILTIN_TYPES_COMPATIBLE_P = 326,
    __ATTRIBUTE__ = 327,
    PRAGMA_OMP = 328,
    PRAGMA_OMP_THREADPRIVATE = 329,
    OMP_PARALLEL = 330,
    OMP_SECTIONS = 331,
    OMP_NOWAIT = 332,
    OMP_ORDERED = 333,
    OMP_SCHEDULE = 334,
    OMP_STATIC = 335,
    OMP_DYNAMIC = 336,
    OMP_GUIDED = 337,
    OMP_RUNTIME = 338,
    OMP_AUTO = 339,
    OMP_SECTION = 340,
    OMP_AFFINITY = 341,
    OMP_SINGLE = 342,
    OMP_MASTER = 343,
    OMP_CRITICAL = 344,
    OMP_BARRIER = 345,
    OMP_ATOMIC = 346,
    OMP_FLUSH = 347,
    OMP_PRIVATE = 348,
    OMP_FIRSTPRIVATE = 349,
    OMP_LASTPRIVATE = 350,
    OMP_SHARED = 351,
    OMP_DEFAULT = 352,
    OMP_NONE = 353,
    OMP_REDUCTION = 354,
    OMP_COPYIN = 355,
    OMP_NUMTHREADS = 356,
    OMP_COPYPRIVATE = 357,
    OMP_FOR = 358,
    OMP_IF = 359,
    OMP_TASK = 360,
    OMP_UNTIED = 361,
    OMP_TASKWAIT = 362,
    OMP_COLLAPSE = 363,
    OMP_FINAL = 364,
    OMP_MERGEABLE = 365,
    OMP_TASKYIELD = 366,
    OMP_READ = 367,
    OMP_WRITE = 368,
    OMP_CAPTURE = 369,
    OMP_UPDATE = 370,
    OMP_MIN = 371,
    OMP_MAX = 372,
    OMP_PROCBIND = 373,
    OMP_CLOSE = 374,
    OMP_SPREAD = 375,
    OMP_SIMD = 376,
    OMP_INBRANCH = 377,
    OMP_NOTINBRANCH = 378,
    OMP_UNIFORM = 379,
    OMP_LINEAR = 380,
    OMP_ALIGNED = 381,
    OMP_SIMDLEN = 382,
    OMP_SAFELEN = 383,
    OMP_DECLARE = 384,
    OMP_TARGET = 385,
    OMP_DATA = 386,
    OMP_DEVICE = 387,
    OMP_MAP = 388,
    OMP_ALLOC = 389,
    OMP_TO = 390,
    OMP_FROM = 391,
    OMP_TOFROM = 392,
    OMP_END = 393,
    OMP_TEAMS = 394,
    OMP_DISTRIBUTE = 395,
    OMP_NUMTEAMS = 396,
    OMP_THREADLIMIT = 397,
    OMP_DISTSCHEDULE = 398,
    OMP_DEPEND = 399,
    OMP_IN = 400,
    OMP_OUT = 401,
    OMP_INOUT = 402,
    OMP_TASKGROUP = 403,
    OMP_SEQ_CST = 404,
    OMP_CANCEL = 405,
    OMP_INITIALIZER = 406,
    PRAGMA_OMP_CANCELLATIONPOINT = 407,
    PRAGMA_OMPIX = 408,
    OMPIX_TASKDEF = 409,
    OMPIX_IN = 410,
    OMPIX_OUT = 411,
    OMPIX_INOUT = 412,
    OMPIX_TASKSYNC = 413,
    OMPIX_UPONRETURN = 414,
    OMPIX_ATNODE = 415,
    OMPIX_DETACHED = 416,
    OMPIX_ATWORKER = 417,
    OMPIX_TASKSCHEDULE = 418,
    OMPIX_STRIDE = 419,
    OMPIX_START = 420,
    OMPIX_SCOPE = 421,
    OMPIX_NODES = 422,
    OMPIX_WORKERS = 423,
    OMPIX_LOCAL = 424,
    OMPIX_GLOBAL = 425,
    OMPIX_TIED = 426
  };
#endif
/* Tokens.  */
#define START_SYMBOL_EXPRESSION 258
#define START_SYMBOL_BLOCKLIST 259
#define START_SYMBOL_TRANSUNIT 260
#define IDENTIFIER 261
#define TYPE_NAME 262
#define CONSTANT 263
#define STRING_LITERAL 264
#define PTR_OP 265
#define INC_OP 266
#define DEC_OP 267
#define LEFT_OP 268
#define RIGHT_OP 269
#define LE_OP 270
#define GE_OP 271
#define EQ_OP 272
#define NE_OP 273
#define AND_OP 274
#define OR_OP 275
#define MUL_ASSIGN 276
#define DIV_ASSIGN 277
#define MOD_ASSIGN 278
#define ADD_ASSIGN 279
#define SUB_ASSIGN 280
#define LEFT_ASSIGN 281
#define RIGHT_ASSIGN 282
#define AND_ASSIGN 283
#define XOR_ASSIGN 284
#define OR_ASSIGN 285
#define SIZEOF 286
#define TYPEDEF 287
#define EXTERN 288
#define STATIC 289
#define AUTO 290
#define REGISTER 291
#define RESTRICT 292
#define CHAR 293
#define SHORT 294
#define INT 295
#define LONG 296
#define SIGNED 297
#define UNSIGNED 298
#define FLOAT 299
#define DOUBLE 300
#define CONST 301
#define VOLATILE 302
#define VOID 303
#define INLINE 304
#define UBOOL 305
#define UCOMPLEX 306
#define UIMAGINARY 307
#define STRUCT 308
#define UNION 309
#define ENUM 310
#define ELLIPSIS 311
#define CASE 312
#define DEFAULT 313
#define IF 314
#define ELSE 315
#define SWITCH 316
#define WHILE 317
#define DO 318
#define FOR 319
#define GOTO 320
#define CONTINUE 321
#define BREAK 322
#define RETURN 323
#define __BUILTIN_VA_ARG 324
#define __BUILTIN_OFFSETOF 325
#define __BUILTIN_TYPES_COMPATIBLE_P 326
#define __ATTRIBUTE__ 327
#define PRAGMA_OMP 328
#define PRAGMA_OMP_THREADPRIVATE 329
#define OMP_PARALLEL 330
#define OMP_SECTIONS 331
#define OMP_NOWAIT 332
#define OMP_ORDERED 333
#define OMP_SCHEDULE 334
#define OMP_STATIC 335
#define OMP_DYNAMIC 336
#define OMP_GUIDED 337
#define OMP_RUNTIME 338
#define OMP_AUTO 339
#define OMP_SECTION 340
#define OMP_AFFINITY 341
#define OMP_SINGLE 342
#define OMP_MASTER 343
#define OMP_CRITICAL 344
#define OMP_BARRIER 345
#define OMP_ATOMIC 346
#define OMP_FLUSH 347
#define OMP_PRIVATE 348
#define OMP_FIRSTPRIVATE 349
#define OMP_LASTPRIVATE 350
#define OMP_SHARED 351
#define OMP_DEFAULT 352
#define OMP_NONE 353
#define OMP_REDUCTION 354
#define OMP_COPYIN 355
#define OMP_NUMTHREADS 356
#define OMP_COPYPRIVATE 357
#define OMP_FOR 358
#define OMP_IF 359
#define OMP_TASK 360
#define OMP_UNTIED 361
#define OMP_TASKWAIT 362
#define OMP_COLLAPSE 363
#define OMP_FINAL 364
#define OMP_MERGEABLE 365
#define OMP_TASKYIELD 366
#define OMP_READ 367
#define OMP_WRITE 368
#define OMP_CAPTURE 369
#define OMP_UPDATE 370
#define OMP_MIN 371
#define OMP_MAX 372
#define OMP_PROCBIND 373
#define OMP_CLOSE 374
#define OMP_SPREAD 375
#define OMP_SIMD 376
#define OMP_INBRANCH 377
#define OMP_NOTINBRANCH 378
#define OMP_UNIFORM 379
#define OMP_LINEAR 380
#define OMP_ALIGNED 381
#define OMP_SIMDLEN 382
#define OMP_SAFELEN 383
#define OMP_DECLARE 384
#define OMP_TARGET 385
#define OMP_DATA 386
#define OMP_DEVICE 387
#define OMP_MAP 388
#define OMP_ALLOC 389
#define OMP_TO 390
#define OMP_FROM 391
#define OMP_TOFROM 392
#define OMP_END 393
#define OMP_TEAMS 394
#define OMP_DISTRIBUTE 395
#define OMP_NUMTEAMS 396
#define OMP_THREADLIMIT 397
#define OMP_DISTSCHEDULE 398
#define OMP_DEPEND 399
#define OMP_IN 400
#define OMP_OUT 401
#define OMP_INOUT 402
#define OMP_TASKGROUP 403
#define OMP_SEQ_CST 404
#define OMP_CANCEL 405
#define OMP_INITIALIZER 406
#define PRAGMA_OMP_CANCELLATIONPOINT 407
#define PRAGMA_OMPIX 408
#define OMPIX_TASKDEF 409
#define OMPIX_IN 410
#define OMPIX_OUT 411
#define OMPIX_INOUT 412
#define OMPIX_TASKSYNC 413
#define OMPIX_UPONRETURN 414
#define OMPIX_ATNODE 415
#define OMPIX_DETACHED 416
#define OMPIX_ATWORKER 417
#define OMPIX_TASKSCHEDULE 418
#define OMPIX_STRIDE 419
#define OMPIX_START 420
#define OMPIX_SCOPE 421
#define OMPIX_NODES 422
#define OMPIX_WORKERS 423
#define OMPIX_LOCAL 424
#define OMPIX_GLOBAL 425
#define OMPIX_TIED 426

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 80 "parser.y" /* yacc.c:355  */

  char      name[2048];  /* A general string */
  int       type;        /* A general integer */
  char     *string;      /* A dynamically allocated string (only for 2 rules) */
  symbol    symb;        /* A symbol */
  astexpr   expr;        /* An expression node in the AST */
  astspec   spec;        /* A declaration specifier node in the AST */
  astdecl   decl;        /* A declarator node in the AST */
  aststmt   stmt;        /* A statement node in the AST */
  ompcon    ocon;        /* An OpenMP construct */
  ompdir    odir;        /* An OpenMP directive */
  ompclause ocla;        /* An OpenMP clause */

  oxcon     xcon;        /* OMPi extensions */
  oxdir     xdir;
  oxclause  xcla;

#line 545 "parser.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 560 "parser.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
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


#if ! defined yyoverflow || YYERROR_VERBOSE

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
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  217
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4535

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  197
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  321
/* YYNRULES -- Number of rules.  */
#define YYNRULES  792
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1325

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   426

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     196,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   185,     2,     2,     2,   187,   180,     2,
     172,   173,   181,   182,   179,   183,   176,   186,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   193,   195,
     188,   194,   189,   192,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   174,     2,   175,   190,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   177,   191,   178,   184,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   453,   453,   454,   455,   456,   473,   494,   498,   521,
     543,   547,   551,   559,   563,   573,   580,   587,   591,   595,
     599,   608,   612,   616,   620,   624,   628,   636,   640,   648,
     652,   656,   660,   667,   671,   680,   685,   690,   699,   703,
     707,   711,   715,   719,   727,   731,   739,   743,   747,   751,
     759,   763,   767,   775,   779,   783,   791,   795,   799,   803,
     807,   815,   819,   823,   831,   835,   843,   847,   855,   859,
     867,   871,   879,   883,   891,   895,   903,   907,   915,   919,
     923,   927,   931,   935,   939,   943,   947,   951,   955,   963,
     967,   975,   989,  1004,  1011,  1016,  1020,  1024,  1032,  1036,
    1040,  1044,  1048,  1052,  1056,  1060,  1068,  1072,  1086,  1103,
    1102,  1125,  1130,  1134,  1138,  1142,  1150,  1154,  1158,  1162,
    1166,  1170,  1174,  1178,  1182,  1186,  1190,  1194,  1198,  1202,
    1206,  1214,  1218,  1222,  1237,  1244,  1251,  1262,  1266,  1274,
    1278,  1286,  1290,  1298,  1302,  1306,  1310,  1318,  1322,  1330,
    1334,  1338,  1346,  1350,  1362,  1366,  1378,  1390,  1394,  1402,
    1406,  1414,  1418,  1422,  1430,  1438,  1442,  1455,  1459,  1467,
    1471,  1475,  1479,  1483,  1487,  1491,  1495,  1499,  1503,  1507,
    1511,  1519,  1523,  1527,  1531,  1539,  1543,  1551,  1555,  1563,
    1567,  1575,  1579,  1583,  1591,  1595,  1603,  1607,  1615,  1619,
    1623,  1631,  1635,  1639,  1643,  1647,  1651,  1655,  1659,  1663,
    1667,  1671,  1679,  1687,  1691,  1695,  1703,  1707,  1711,  1715,
    1723,  1731,  1735,  1743,  1747,  1751,  1765,  1769,  1773,  1777,
    1781,  1785,  1789,  1794,  1803,  1807,  1817,  1821,  1825,  1833,
    1837,  1837,  1848,  1852,  1861,  1865,  1869,  1874,  1883,  1887,
    1896,  1900,  1904,  1913,  1917,  1921,  1925,  1929,  1933,  1937,
    1941,  1945,  1949,  1953,  1957,  1961,  1965,  1969,  1977,  1982,
    1986,  1990,  1997,  2014,  2018,  2026,  2030,  2037,  2049,  2050,
    2055,  2054,  2071,  2070,  2092,  2091,  2110,  2109,  2133,  2137,
    2152,  2156,  2164,  2173,  2177,  2184,  2188,  2192,  2196,  2200,
    2204,  2208,  2212,  2216,  2220,  2225,  2230,  2234,  2238,  2242,
    2246,  2250,  2254,  2258,  2262,  2266,  2270,  2274,  2278,  2282,
    2286,  2290,  2294,  2298,  2302,  2306,  2311,  2327,  2331,  2336,
    2341,  2346,  2351,  2358,  2365,  2373,  2381,  2384,  2388,  2395,
    2399,  2403,  2407,  2411,  2415,  2422,  2426,  2426,  2431,  2431,
    2437,  2442,  2442,  2450,  2457,  2465,  2468,  2472,  2479,  2483,
    2487,  2491,  2495,  2499,  2506,  2510,  2514,  2514,  2522,  2521,
    2533,  2540,  2544,  2548,  2552,  2556,  2560,  2565,  2572,  2580,
    2583,  2587,  2594,  2598,  2602,  2606,  2610,  2617,  2624,  2629,
    2633,  2640,  2647,  2654,  2662,  2665,  2669,  2676,  2680,  2684,
    2688,  2695,  2695,  2704,  2712,  2720,  2723,  2727,  2735,  2739,
    2743,  2747,  2751,  2759,  2771,  2775,  2783,  2787,  2795,  2795,
    2804,  2804,  2813,  2813,  2822,  2825,  2833,  2841,  2845,  2853,
    2861,  2864,  2868,  2877,  2889,  2893,  2897,  2901,  2909,  2917,
    2925,  2928,  2932,  2939,  2943,  2951,  2959,  2967,  2970,  2974,
    2981,  2985,  2993,  3001,  3009,  3012,  3016,  3023,  3027,  3031,
    3039,  3039,  3048,  3048,  3053,  3053,  3062,  3066,  3070,  3074,
    3082,  3082,  3093,  3101,  3104,  3108,  3115,  3119,  3126,  3130,
    3138,  3146,  3153,  3157,  3161,  3168,  3172,  3176,  3183,  3183,
    3188,  3188,  3197,  3204,  3211,  3219,  3227,  3235,  3238,  3242,
    3249,  3253,  3257,  3261,  3265,  3269,  3277,  3277,  3283,  3283,
    3292,  3300,  3308,  3311,  3315,  3322,  3326,  3330,  3334,  3341,
    3345,  3345,  3354,  3363,  3371,  3374,  3378,  3385,  3389,  3393,
    3401,  3410,  3418,  3421,  3425,  3432,  3436,  3444,  3453,  3461,
    3464,  3468,  3475,  3479,  3487,  3495,  3503,  3506,  3510,  3517,
    3521,  3529,  3537,  3545,  3548,  3552,  3559,  3563,  3567,  3575,
    3583,  3591,  3594,  3598,  3605,  3609,  3613,  3617,  3625,  3633,
    3641,  3644,  3648,  3655,  3659,  3667,  3675,  3684,  3687,  3692,
    3700,  3704,  3712,  3721,  3730,  3733,  3738,  3746,  3750,  3758,
    3767,  3775,  3778,  3782,  3789,  3793,  3801,  3810,  3818,  3821,
    3825,  3832,  3836,  3844,  3853,  3861,  3864,  3868,  3875,  3879,
    3887,  3896,  3905,  3908,  3912,  3920,  3924,  3928,  3932,  3936,
    3944,  3948,  3948,  3953,  3957,  3962,  3970,  3974,  3978,  3985,
    3993,  4001,  4004,  4008,  4015,  4019,  4023,  4027,  4031,  4035,
    4039,  4043,  4050,  4058,  4066,  4069,  4073,  4080,  4084,  4088,
    4092,  4096,  4100,  4104,  4111,  4118,  4125,  4132,  4136,  4143,
    4150,  4158,  4166,  4174,  4182,  4189,  4200,  4208,  4211,  4215,
    4219,  4223,  4232,  4235,  4242,  4246,  4253,  4253,  4261,  4268,
    4276,  4280,  4284,  4291,  4295,  4299,  4303,  4311,  4318,  4318,
    4318,  4326,  4326,  4326,  4333,  4338,  4342,  4346,  4350,  4354,
    4358,  4362,  4366,  4370,  4374,  4381,  4385,  4393,  4396,  4401,
    4412,  4416,  4421,  4428,  4428,  4436,  4436,  4444,  4444,  4452,
    4452,  4460,  4460,  4468,  4468,  4476,  4491,  4502,  4509,  4519,
    4526,  4530,  4537,  4544,  4544,  4549,  4549,  4557,  4561,  4569,
    4573,  4577,  4589,  4607,  4633,  4637,  4645,  4653,  4652,  4665,
    4668,  4672,  4679,  4683,  4687,  4691,  4695,  4702,  4706,  4710,
    4714,  4721,  4725,  4734,  4733,  4750,  4759,  4758,  4771,  4774,
    4778,  4785,  4789,  4793,  4797,  4804,  4808,  4815,  4820,  4828,
    4836,  4844,  4852,  4855,  4859,  4866,  4870,  4874,  4878,  4882,
    4886,  4893,  4899
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "START_SYMBOL_EXPRESSION",
  "START_SYMBOL_BLOCKLIST", "START_SYMBOL_TRANSUNIT", "IDENTIFIER",
  "TYPE_NAME", "CONSTANT", "STRING_LITERAL", "PTR_OP", "INC_OP", "DEC_OP",
  "LEFT_OP", "RIGHT_OP", "LE_OP", "GE_OP", "EQ_OP", "NE_OP", "AND_OP",
  "OR_OP", "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN", "ADD_ASSIGN",
  "SUB_ASSIGN", "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN", "XOR_ASSIGN",
  "OR_ASSIGN", "SIZEOF", "TYPEDEF", "EXTERN", "STATIC", "AUTO", "REGISTER",
  "RESTRICT", "CHAR", "SHORT", "INT", "LONG", "SIGNED", "UNSIGNED",
  "FLOAT", "DOUBLE", "CONST", "VOLATILE", "VOID", "INLINE", "UBOOL",
  "UCOMPLEX", "UIMAGINARY", "STRUCT", "UNION", "ENUM", "ELLIPSIS", "CASE",
  "DEFAULT", "IF", "ELSE", "SWITCH", "WHILE", "DO", "FOR", "GOTO",
  "CONTINUE", "BREAK", "RETURN", "__BUILTIN_VA_ARG", "__BUILTIN_OFFSETOF",
  "__BUILTIN_TYPES_COMPATIBLE_P", "__ATTRIBUTE__", "PRAGMA_OMP",
  "PRAGMA_OMP_THREADPRIVATE", "OMP_PARALLEL", "OMP_SECTIONS", "OMP_NOWAIT",
  "OMP_ORDERED", "OMP_SCHEDULE", "OMP_STATIC", "OMP_DYNAMIC", "OMP_GUIDED",
  "OMP_RUNTIME", "OMP_AUTO", "OMP_SECTION", "OMP_AFFINITY", "OMP_SINGLE",
  "OMP_MASTER", "OMP_CRITICAL", "OMP_BARRIER", "OMP_ATOMIC", "OMP_FLUSH",
  "OMP_PRIVATE", "OMP_FIRSTPRIVATE", "OMP_LASTPRIVATE", "OMP_SHARED",
  "OMP_DEFAULT", "OMP_NONE", "OMP_REDUCTION", "OMP_COPYIN",
  "OMP_NUMTHREADS", "OMP_COPYPRIVATE", "OMP_FOR", "OMP_IF", "OMP_TASK",
  "OMP_UNTIED", "OMP_TASKWAIT", "OMP_COLLAPSE", "OMP_FINAL",
  "OMP_MERGEABLE", "OMP_TASKYIELD", "OMP_READ", "OMP_WRITE", "OMP_CAPTURE",
  "OMP_UPDATE", "OMP_MIN", "OMP_MAX", "OMP_PROCBIND", "OMP_CLOSE",
  "OMP_SPREAD", "OMP_SIMD", "OMP_INBRANCH", "OMP_NOTINBRANCH",
  "OMP_UNIFORM", "OMP_LINEAR", "OMP_ALIGNED", "OMP_SIMDLEN", "OMP_SAFELEN",
  "OMP_DECLARE", "OMP_TARGET", "OMP_DATA", "OMP_DEVICE", "OMP_MAP",
  "OMP_ALLOC", "OMP_TO", "OMP_FROM", "OMP_TOFROM", "OMP_END", "OMP_TEAMS",
  "OMP_DISTRIBUTE", "OMP_NUMTEAMS", "OMP_THREADLIMIT", "OMP_DISTSCHEDULE",
  "OMP_DEPEND", "OMP_IN", "OMP_OUT", "OMP_INOUT", "OMP_TASKGROUP",
  "OMP_SEQ_CST", "OMP_CANCEL", "OMP_INITIALIZER",
  "PRAGMA_OMP_CANCELLATIONPOINT", "PRAGMA_OMPIX", "OMPIX_TASKDEF",
  "OMPIX_IN", "OMPIX_OUT", "OMPIX_INOUT", "OMPIX_TASKSYNC",
  "OMPIX_UPONRETURN", "OMPIX_ATNODE", "OMPIX_DETACHED", "OMPIX_ATWORKER",
  "OMPIX_TASKSCHEDULE", "OMPIX_STRIDE", "OMPIX_START", "OMPIX_SCOPE",
  "OMPIX_NODES", "OMPIX_WORKERS", "OMPIX_LOCAL", "OMPIX_GLOBAL",
  "OMPIX_TIED", "'('", "')'", "'['", "']'", "'.'", "'{'", "'}'", "','",
  "'&'", "'*'", "'+'", "'-'", "'~'", "'!'", "'/'", "'%'", "'<'", "'>'",
  "'^'", "'|'", "'?'", "':'", "'='", "';'", "'\\n'", "$accept",
  "start_trick", "enumeration_constant", "string_literal",
  "primary_expression", "postfix_expression", "argument_expression_list",
  "unary_expression", "unary_operator", "cast_expression",
  "multiplicative_expression", "additive_expression", "shift_expression",
  "relational_expression", "equality_expression", "AND_expression",
  "exclusive_OR_expression", "inclusive_OR_expression",
  "logical_AND_expression", "logical_OR_expression",
  "conditional_expression", "assignment_expression", "assignment_operator",
  "expression", "constant_expression", "declaration",
  "declaration_specifiers", "init_declarator_list", "init_declarator",
  "$@1", "storage_class_specifier", "type_specifier",
  "struct_or_union_specifier", "struct_or_union",
  "struct_declaration_list", "struct_declaration",
  "specifier_qualifier_list", "struct_declarator_list",
  "struct_declarator", "enum_specifier", "enumerator_list", "enumerator",
  "type_qualifier", "function_specifier", "declarator",
  "direct_declarator", "pointer", "type_qualifier_list",
  "parameter_type_list", "parameter_list", "parameter_declaration",
  "identifier_list", "type_name", "abstract_declarator",
  "direct_abstract_declarator", "typedef_name", "initializer",
  "initializer_list", "designation", "designator_list", "designator",
  "statement", "statement_for_labeled", "labeled_statement",
  "compound_statement", "@2", "block_item_list", "block_item",
  "expression_statement", "selection_statement", "iteration_statement",
  "iteration_statement_for", "jump_statement", "translation_unit",
  "external_declaration", "function_definition",
  "normal_function_definition", "$@3", "$@4",
  "oldstyle_function_definition", "$@5", "$@6", "declaration_list",
  "declaration_definition", "function_statement",
  "declarations_definitions_seq", "openmp_construct", "openmp_directive",
  "structured_block", "parallel_construct", "parallel_directive",
  "parallel_clause_optseq", "parallel_clause", "unique_parallel_clause",
  "$@7", "$@8", "$@9", "for_construct", "for_directive",
  "for_clause_optseq", "for_clause", "unique_for_clause", "$@10", "$@11",
  "schedule_kind", "sections_construct", "sections_directive",
  "sections_clause_optseq", "sections_clause", "section_scope",
  "section_sequence", "section_directive", "single_construct",
  "single_directive", "single_clause_optseq", "single_clause",
  "unique_single_clause", "$@12", "simd_construct", "simd_directive",
  "simd_clause_optseq", "simd_clause", "unique_simd_clause",
  "inbranch_clause", "uniform_clause", "$@13", "linear_clause", "$@14",
  "aligned_clause", "$@15", "optional_expression",
  "declare_simd_construct", "declare_simd_directive_seq",
  "declare_simd_directive", "declare_simd_clause_optseq",
  "declare_simd_clause", "for_simd_construct", "for_simd_directive",
  "for_simd_clause_optseq", "for_simd_clause",
  "parallel_for_simd_construct", "parallel_for_simd_directive",
  "parallel_for_simd_clause_optseq", "parallel_for_simd_clause",
  "target_data_construct", "target_data_directive",
  "target_data_clause_optseq", "target_data_clause", "device_clause",
  "$@16", "map_clause", "$@17", "$@18", "map_type", "target_construct",
  "@19", "target_directive", "target_clause_optseq", "target_clause",
  "unique_target_clause", "target_update_construct",
  "target_update_directive", "target_update_clause_seq",
  "target_update_clause", "motion_clause", "$@20", "$@21",
  "declare_target_construct", "declare_target_directive",
  "end_declare_target_directive", "teams_construct", "teams_directive",
  "teams_clause_optseq", "teams_clause", "unique_teams_clause", "$@22",
  "$@23", "distribute_construct", "distribute_directive",
  "distribute_clause_optseq", "distribute_clause",
  "unique_distribute_clause", "$@24", "distribute_simd_construct",
  "distribute_simd_directive", "distribute_simd_clause_optseq",
  "distribute_simd_clause", "distribute_parallel_for_construct",
  "distribute_parallel_for_directive",
  "distribute_parallel_for_clause_optseq",
  "distribute_parallel_for_clause",
  "distribute_parallel_for_simd_construct",
  "distribute_parallel_for_simd_directive",
  "distribute_parallel_for_simd_clause_optseq",
  "distribute_parallel_for_simd_clause", "target_teams_construct",
  "target_teams_directive", "target_teams_clause_optseq",
  "target_teams_clause", "teams_distribute_construct",
  "teams_distribute_directive", "teams_distribute_clause_optseq",
  "teams_distribute_clause", "teams_distribute_simd_construct",
  "teams_distribute_simd_directive", "teams_distribute_simd_clause_optseq",
  "teams_distribute_simd_clause", "target_teams_distribute_construct",
  "target_teams_distribute_directive",
  "target_teams_distribute_clause_optseq",
  "target_teams_distribute_clause",
  "target_teams_distribute_simd_construct",
  "target_teams_distribute_simd_directive",
  "target_teams_distribute_simd_clause_optseq",
  "target_teams_distribute_simd_clause",
  "teams_distribute_parallel_for_construct",
  "teams_distribute_parallel_for_directive",
  "teams_distribute_parallel_for_clause_optseq",
  "teams_distribute_parallel_for_clause",
  "target_teams_distribute_parallel_for_construct",
  "target_teams_distribute_parallel_for_directive",
  "target_teams_distribute_parallel_for_clause_optseq",
  "target_teams_distribute_parallel_for_clause",
  "teams_distribute_parallel_for_simd_construct",
  "teams_distribute_parallel_for_simd_directive",
  "teams_distribute_parallel_for_simd_clause_optseq",
  "teams_distribute_parallel_for_simd_clause",
  "target_teams_distribute_parallel_for_simd_construct",
  "target_teams_distribute_parallel_for_simd_directive",
  "target_teams_distribute_parallel_for_simd_clause_optseq",
  "target_teams_distribute_parallel_for_simd_clause", "task_construct",
  "task_directive", "task_clause_optseq", "task_clause",
  "unique_task_clause", "$@25", "dependence_type",
  "parallel_for_construct", "parallel_for_directive",
  "parallel_for_clause_optseq", "parallel_for_clause",
  "parallel_sections_construct", "parallel_sections_directive",
  "parallel_sections_clause_optseq", "parallel_sections_clause",
  "master_construct", "master_directive", "critical_construct",
  "critical_directive", "region_phrase", "barrier_directive",
  "taskwait_directive", "taskgroup_construct", "taskgroup_directive",
  "taskyield_directive", "atomic_construct", "atomic_directive",
  "atomic_clause_opt", "seq_cst_clause_opt", "flush_directive",
  "flush_vars", "$@26", "ordered_construct", "ordered_directive",
  "cancel_directive", "construct_type_clause",
  "cancellation_point_directive", "threadprivate_directive", "$@27",
  "$@28", "declare_reduction_directive", "$@29", "$@30",
  "reduction_identifier", "reduction_type_list", "initializer_clause_opt",
  "data_default_clause", "data_privatization_clause", "$@31",
  "data_privatization_in_clause", "$@32", "data_privatization_out_clause",
  "$@33", "data_sharing_clause", "$@34", "data_reduction_clause", "$@35",
  "if_clause", "$@36", "collapse_clause", "array_section", "variable_list",
  "variable_array_section_list", "array_section_subscript", "$@37", "$@38",
  "array_section_plain", "procbind_clause", "thrprv_variable_list",
  "ompix_directive", "ox_tasksync_directive", "ox_taskschedule_directive",
  "$@39", "ox_taskschedule_clause_optseq", "ox_taskschedule_clause",
  "ox_scope_spec", "ompix_construct", "ox_taskdef_construct", "$@40",
  "ox_taskdef_directive", "$@41", "ox_taskdef_clause_optseq",
  "ox_taskdef_clause", "ox_variable_size_list", "ox_variable_size_elem",
  "ox_task_construct", "ox_task_directive", "ox_task_clause_optseq",
  "ox_task_clause", "ox_funccall_expression", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,    40,    41,    91,    93,    46,   123,   125,    44,
      38,    42,    43,    45,   126,    33,    47,    37,    60,    62,
      94,   124,    63,    58,    61,    59,    10
};
# endif

#define YYPACT_NINF -1109

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1109)))

#define YYTABLE_NINF -764

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    2715,  2510,  1409,  2765, -1109, -1109, -1109, -1109, -1109, -1109,
   -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109,
   -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109,    36,
      28, -1109,    39,   136,   351,   114, -1109,   140,  4394,  4394,
   -1109,   100, -1109,  4394,  4394,   163,    34,   130, -1109,  2765,
   -1109, -1109, -1109, -1109, -1109,  3065, -1109, -1109,  2915, -1109,
   -1109, -1109,  3115,   180, -1109, -1109,  2531,  2531,  2545,   224,
     242,   257,  1855, -1109, -1109, -1109, -1109, -1109, -1109,   479,
   -1109,   153,   693,  2510, -1109,   361,   276,   164,    94,   619,
     323,   330,   378,   538,    12, -1109, -1109,   410,   -21,  2510,
     415,   431,   441,   444,  1479,   451,   653,   486,   497,   868,
    3786,   305,    -9,   496, -1109,   124, -1109,   140, -1109, -1109,
   -1109,  1409, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109,
   -1109,  1479, -1109,   605, -1109,   506, -1109,  1479, -1109,   605,
   -1109,   605, -1109,   605, -1109,  1479, -1109, -1109, -1109, -1109,
   -1109,  1479, -1109,   605, -1109,   605, -1109,   605, -1109,  1479,
   -1109,  1479, -1109,   605, -1109,   605, -1109,   605, -1109,   605,
   -1109,   605, -1109,   605, -1109,   605, -1109,   605, -1109,  1479,
   -1109,   605, -1109,   506, -1109,  1479, -1109,  1479, -1109, -1109,
   -1109,  1479, -1109, -1109,  1089, -1109, -1109,  1479, -1109, -1109,
   -1109, -1109, -1109, -1109, -1109, -1109,   701,  2765,   522,   723,
     310,   564, -1109,   591, -1109, -1109,   351, -1109, -1109,   154,
   -1109,   364, -1109, -1109,   598,  2335,   600, -1109, -1109,   602,
    2592,  3336,  1789,    34, -1109,   612,   136, -1109, -1109, -1109,
   -1109, -1109, -1109,  2965,   136, -1109,   613,  2040,  1855, -1109,
   -1109,  1855, -1109,  2510,  3038,  3038,   155,  3038,   398,  3038,
     595, -1109,   665, -1109, -1109,  2071,  2510,   704, -1109, -1109,
   -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109,  2510,
   -1109, -1109,  2510,  2510,  2510,  2510,  2510,  2510,  2510,  2510,
    2510,  2510,  2510,  2510,  2510,  2510,  2510,  2510,  2510,  2510,
    2510,  2510,  1479, -1109,   604,  1060,  2510,  2510,  2510,  3874,
     -24,   739,  1576,   597, -1109, -1109, -1109,   248,    40, -1109,
     617, -1109,   622,   -17,   626,  1000,    -5,   732, -1109,   629,
     660, -1109,   358,   728,     3,   686,   305, -1109, -1109, -1109,
   -1109,   688, -1109,   690, -1109, -1109,  1409, -1109,   699, -1109,
   -1109, -1109, -1109,  1627, -1109, -1109, -1109, -1109, -1109, -1109,
    1479, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109,
   -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109,
   -1109, -1109, -1109,   736,   705,   723, -1109,   721,   582, -1109,
     748, -1109,   758,   950, -1109, -1109, -1109, -1109,   136, -1109,
   -1109,   602,  2592,  3038, -1109,  2862, -1109,    14,  3038, -1109,
   -1109,  3267, -1109, -1109,   183,   791,   801, -1109,   403,  1972,
   -1109,   817,   835,  1906,   863,   857,   296, -1109, -1109, -1109,
     602, -1109,   447, -1109,   851,   874,   881,   886,   891, -1109,
   -1109,  3165,  2137,   420, -1109,   488, -1109,  2178, -1109, -1109,
   -1109,   448,   377, -1109, -1109, -1109, -1109, -1109, -1109,   361,
     361,   276,   276,   164,   164,   164,   164,    94,    94,   619,
     323,   330,   378,   538,   184, -1109, -1109,  1060,  3830, -1109,
   -1109, -1109,   473,   474,   518,   906,  1145,   259,  1670, -1109,
   -1109, -1109,   964,  3676,   556, -1109,   416, -1109,  1082, -1109,
     893, -1109, -1109, -1109, -1109, -1109,   941, -1109, -1109,   896,
   -1109,   866,  1113, -1109, -1109,  1401,   243, -1109,   970,   228,
       8,   825,   999, -1109,   491, -1109,   -40, -1109,   735, -1109,
   -1109,  1340,  3868, -1109,   -28,  1479, -1109,  2226, -1109,   673,
    2510, -1109,   106,   568,   578, -1109, -1109,   533,   321, -1109,
    2254, -1109,  3267,  3188, -1109, -1109,  2510, -1109,   269, -1109,
     910,  3214, -1109, -1109,  3015, -1109,   120, -1109, -1109,  4344,
   -1109,  1128,   968,  1972, -1109, -1109,  2510, -1109,   975,   987,
    1016, -1109, -1109,  2510,   961,   961,  3038,  1164,  3038, -1109,
    1006,  1018, -1109,  1019,  1022,   488,  3386,  2268,  1993, -1109,
   -1109, -1109,  2510, -1109,  1479,  1479,  1479,  2510,  2282,   313,
    1696,  2296,   327,  1650, -1109,  3650, -1109, -1109, -1109, -1109,
    1021,  1027, -1109,  1028,  1030,  1031,  4248, -1109, -1109, -1109,
   -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109,   795,
   -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109,   422, -1109,
   -1109, -1109, -1109, -1109,  1035, -1109, -1109,  1008,  1205, -1109,
    1070, -1109, -1109,  1046,  1048,  1420, -1109, -1109, -1109, -1109,
   -1109, -1109, -1109, -1109, -1109,  1049, -1109,  1056,  1142, -1109,
   -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109,  1058,
     275, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109,
    1059, -1109, -1109, -1109,   366, -1109, -1109, -1109,   482,    20,
    1692,  1061,   212, -1109, -1109, -1109, -1109, -1109, -1109,  1131,
   -1109,   706,  1075,  1078,  1572, -1109, -1109, -1109, -1109, -1109,
   -1109, -1109, -1109,  1132,  2102,  1086,    -6, -1109, -1109, -1109,
   -1109, -1109, -1109,  1155, -1109,  1064, -1109,  1095, -1109,  1096,
   -1109,   698, -1109, -1109,   807, -1109,  1079,  1192, -1109,  1479,
   -1109, -1109,   553, -1109,   109, -1109, -1109, -1109, -1109, -1109,
   -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109,  1085,
   -1109, -1109, -1109,  1110,   713, -1109, -1109, -1109, -1109, -1109,
   -1109, -1109,  1277,  1118,  1123,  1125,  1126,   501, -1109, -1109,
    1993, -1109, -1109, -1109, -1109, -1109,    22, -1109,  2510, -1109,
   -1109, -1109, -1109, -1109,  1124,  1129, -1109, -1109,  1171, -1109,
    1130,  1134,  1135, -1109, -1109, -1109, -1109, -1109,  1137, -1109,
    1138,  1140,  2510,   921, -1109,   797,  2254,   357, -1109, -1109,
    1245, -1109, -1109,   565,  1479,   583,  2323,  2413,   334,  1479,
     599,  2479,  2605, -1109, -1109, -1109, -1109, -1109, -1109, -1109,
   -1109, -1109,  3509,  4417, -1109, -1109, -1109, -1109, -1109, -1109,
   -1109, -1109, -1109, -1109,  1144,  1146,  1149,  1152,   469,   568,
    1166, -1109, -1109,    50, -1109,  1167, -1109,  1169, -1109, -1109,
   -1109, -1109,   614,  1186, -1109, -1109, -1109, -1109,   559,  2510,
   -1109, -1109,   -61, -1109,  1170,  1172,  2510, -1109, -1109,  1173,
    1178,   243, -1109, -1109,   212, -1109, -1109, -1109, -1109, -1109,
    1233, -1109,   687,  3263, -1109, -1109, -1109, -1109,  1197, -1109,
    1232,  3708,  1460, -1109, -1109, -1109, -1109, -1109, -1109, -1109,
   -1109, -1109,  3613,  1194, -1109, -1109, -1109, -1109, -1109,  1274,
   -1109,  1165, -1109,  2552,  2510, -1109, -1109,  1188,  1190,  1191,
   -1109,   911, -1109, -1109, -1109, -1109, -1109, -1109,  3238,  1193,
    2510, -1109,  1168, -1109,   568,  1360,  1360,  1360, -1109,   809,
   -1109, -1109, -1109, -1109,  1200, -1109, -1109, -1109, -1109, -1109,
   -1109,  1225, -1109, -1109, -1109,  1923, -1109, -1109, -1109,  1479,
    1174, -1109,  1479,  1479,   636,  1479,   638,  2503, -1109,  1479,
    1479,   682, -1109,  4159, -1109, -1109, -1109, -1109, -1109,  1205,
    1205,  1205,  1205,  1239,  1246,  1249, -1109,  1205,  2510,  2510,
    1250,  1251,  1252,  1205,  1205, -1109,  1361, -1109, -1109, -1109,
   -1109, -1109, -1109, -1109,  1247,   692,   694,  2510, -1109, -1109,
   -1109,  1234,  1205,  1205,   702,  2510,  1422,  1422, -1109, -1109,
    1308,  3689,  4232, -1109, -1109, -1109, -1109, -1109, -1109, -1109,
   -1109, -1109,  1422,  1237, -1109,  3561,  4196, -1109, -1109, -1109,
   -1109, -1109, -1109, -1109,  2510,  2510,  3472,  4132, -1109, -1109,
   -1109, -1109, -1109,   730, -1109,  1258,  1259,  1260,  2510,  2510,
     833, -1109, -1109,   213,  1205,   731, -1109,  1241,  1261,   750,
   -1109,   769,   773, -1109,  1951, -1109, -1109, -1109, -1109,  2254,
   -1109, -1109, -1109, -1109,  1479, -1109,  1479,  1479,   774, -1109,
   -1109,  1479, -1109,   778,   806,   810,   823, -1109, -1109, -1109,
    1243,   832,   840,   847, -1109, -1109, -1109,   852,   856, -1109,
   -1109, -1109, -1109, -1109,   865,  1422,   240,   240, -1109,   882,
    1263, -1109,   885,   908, -1109,  3420,  4175, -1109, -1109, -1109,
   -1109, -1109,   920, -1109,  3364,  4105, -1109, -1109, -1109, -1109,
   -1109,   962,   963,  4069, -1109, -1109, -1109, -1109, -1109, -1109,
   -1109, -1109, -1109, -1109,  1265,  1266, -1109,  1286,  1296,  3238,
   -1109,   973, -1109,  1360,   924, -1109,  1360, -1109, -1109, -1109,
   -1109, -1109, -1109, -1109,  1479, -1109, -1109, -1109, -1109, -1109,
    1205, -1109, -1109, -1109, -1109, -1109,  2510,  2510, -1109,   982,
    2510,  1311,  1313, -1109, -1109,  1307, -1109,  1483, -1109,  2747,
    4018, -1109, -1109, -1109, -1109, -1109, -1109,  1422,  3982, -1109,
   -1109, -1109, -1109, -1109, -1109, -1109, -1109,  2510, -1109, -1109,
     839, -1109, -1109,  2510, -1109,   993,  1489,  1326, -1109, -1109,
     994,   998,  1001, -1109,   410, -1109, -1109,  2510, -1109,  1263,
   -1109,  3931, -1109, -1109, -1109, -1109, -1109,  1002, -1109,  1009,
   -1109, -1109,  1010, -1109,  1327, -1109, -1109, -1109, -1109,   289,
    1328,  2510, -1109, -1109, -1109, -1109, -1109,  2510, -1109,  1329,
    1354,   410, -1109,  1334,  1312,  1501, -1109,   128,  2510,  2510,
    1013,  1338,  1343, -1109, -1109
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,     0,     0,   167,   212,   111,   112,   113,   114,
     115,   162,   117,   118,   119,   120,   123,   124,   121,   122,
     161,   163,   116,   164,   125,   126,   127,   137,   138,     0,
       0,   688,     0,     0,   181,     0,   276,     0,    98,   100,
     128,     0,   129,   102,   104,   286,   165,     0,   130,     2,
     273,   275,   278,   279,    95,     0,   427,    96,     0,    94,
      97,   277,     0,     9,    10,     7,     0,     0,     0,     0,
       0,     0,     0,    38,    39,    40,    41,    42,    43,    11,
      13,    29,    44,     0,    46,    50,    53,    56,    61,    64,
      66,    68,    70,    72,    74,    76,    89,     3,     9,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   240,   248,     0,   244,     0,   245,   226,
     227,     4,   242,   228,   229,   230,   255,   231,   232,   246,
     295,     0,   296,     0,   297,     0,   298,     0,   306,     0,
     307,     0,   308,     0,   309,     0,   310,   470,   311,   480,
     312,     0,   313,     0,   314,     0,   315,     0,   316,     0,
     317,     0,   318,     0,   319,     0,   320,     0,   321,     0,
     322,     0,   323,     0,   324,     0,   325,     0,   305,     0,
     299,     0,   300,     0,   301,     0,   302,     0,   327,   329,
     326,     0,   330,   303,     0,   328,   304,     0,   331,   332,
     247,   744,   745,   233,   761,   762,     0,     5,   156,     0,
       0,     0,   766,     0,   185,   183,   182,     1,    92,     0,
     106,   284,    99,   101,   135,     0,   136,   103,   105,     0,
       0,     0,     0,   166,   274,     0,     0,   292,   426,   428,
     291,   290,   293,     0,     0,   282,   765,     0,     0,    30,
      31,     0,    33,     0,     0,     0,     0,   143,   196,   145,
       0,     8,     0,    23,    24,     0,     0,     0,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    78,     0,
      44,    32,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    91,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   269,   270,   271,     0,   336,   379,
       0,   394,     0,     0,     0,   667,     0,   355,   612,     0,
       0,   405,   473,   497,   512,     0,     0,   683,   684,   685,
     686,     0,   782,     0,   747,   239,     0,   249,   108,   243,
     333,   334,   353,     0,   377,   392,   403,   438,   445,   452,
       0,   495,   510,   522,   530,   537,   544,   551,   559,   568,
     575,   582,   589,   596,   603,   610,   629,   642,   654,   656,
     662,   665,   678,     0,     0,     0,     6,   159,     0,   157,
       0,   430,     0,     0,   768,   168,   186,   184,     0,    93,
     109,     0,     0,     0,   132,     0,   139,     0,     0,   283,
     288,     0,   194,   179,   192,     0,   187,   189,     0,     0,
     169,    39,     0,     0,     0,   284,     0,   294,   492,   280,
       0,    15,     0,    27,     0,     0,     0,     0,     0,    12,
     144,     0,     0,   198,   197,   199,   146,     0,    20,    22,
      17,     0,     0,    19,    21,    77,    47,    48,    49,    51,
      52,    54,    55,    59,    60,    57,    58,    62,    63,    65,
      67,    69,    71,    73,     0,    90,   236,     0,     0,   234,
     238,   235,     0,     0,     0,     0,     0,     0,     0,   268,
     272,   644,   631,     0,     0,   679,     0,   655,     0,   657,
       0,   660,   668,   669,   671,   670,   672,   676,   674,     0,
     440,     0,     0,   661,   664,     0,     0,   454,   546,     0,
     553,     0,     0,   524,     0,   663,     0,   687,     0,   746,
     749,     0,     0,   388,     0,     0,   471,     0,   780,     0,
       0,   152,     0,     0,     0,   493,   742,     0,     0,   107,
       0,   281,     0,     0,   131,   140,     0,   142,     0,   147,
     149,     0,   289,   287,     0,   191,   198,   193,   178,     0,
     180,     0,     0,     0,   176,   171,     0,   170,    39,     0,
       0,   764,    16,     0,     0,    34,     0,     0,     0,   208,
       0,     0,   202,    39,     0,   200,     0,     0,     0,    45,
      18,    14,     0,   237,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   447,     0,   351,   713,   715,   719,
       0,     0,   348,     0,     0,     0,     0,   335,   337,   339,
     340,   341,   342,   343,   344,   345,   350,   386,   717,     0,
     378,   380,   382,   383,   384,   385,   400,   401,     0,   393,
     395,   397,   398,   399,     0,   658,   673,     0,     0,   675,
       0,   363,   364,     0,     0,     0,   354,   356,   358,   359,
     360,   361,   362,   370,   623,     0,   624,     0,     0,   611,
     613,   615,   616,   617,   618,   619,   620,   420,   422,     0,
       0,   404,   406,   408,   414,   415,   409,   410,   411,   412,
       0,   488,   490,   486,     0,   482,   485,   487,     0,   570,
       0,     0,     0,   472,   478,   479,   474,   476,   477,     0,
     561,     0,     0,     0,     0,   496,   498,   500,   501,   502,
     503,   504,   505,   532,     0,     0,     0,   511,   513,   518,
     515,   516,   517,     0,   680,     0,   789,     0,   790,     0,
     788,     0,   781,   783,     0,   241,     0,     0,   387,     0,
     389,   791,     0,   153,     0,   160,   154,   158,   694,   701,
     702,   703,   704,   698,   696,   695,   697,   699,   700,     0,
     416,   417,   418,     0,     0,   429,   437,   436,   434,   435,
     431,   689,     0,     0,     0,     0,     0,     0,   767,   769,
       0,   213,   110,   285,   133,   151,     0,   141,     0,   134,
     188,   190,   195,   173,     0,     0,   177,   172,     0,    28,
       0,     0,     0,   210,   201,   206,   204,   209,     0,   203,
      39,     0,     0,     0,   216,     0,     0,     0,   221,    75,
     250,   252,   253,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   643,   647,   645,   648,   649,   650,   651,
     652,   653,     0,     0,   630,   634,   635,   632,   636,   637,
     638,   639,   640,   641,     0,     0,     0,     0,     0,     0,
       0,   346,   723,     0,   338,     0,   381,     0,   396,   659,
     666,   727,     0,     0,   439,   443,   444,   441,     0,     0,
     357,   621,     0,   614,     0,     0,     0,   407,   460,     0,
       0,     0,   481,   483,     0,   453,   455,   457,   458,   459,
       0,   577,     0,     0,   545,   549,   550,   547,   464,   475,
     584,     0,     0,   552,   556,   557,   554,   558,   506,   508,
     499,   539,     0,     0,   523,   529,   527,   525,   528,     0,
     514,     0,   681,     0,     0,   784,   756,     0,     0,     0,
     755,     0,   748,   750,   391,   390,   792,   155,     0,     0,
       0,   432,     0,   743,     0,     0,     0,     0,   770,     0,
     148,   150,   174,   175,     0,    35,    36,    37,   211,   207,
     205,     0,   224,   225,    25,     0,   217,   220,   222,     0,
       0,   256,     0,     0,     0,     0,     0,     0,   264,     0,
       0,     0,   646,     0,   446,   451,   448,   450,   633,     0,
       0,     0,     0,     0,     0,     0,   721,     0,     0,     0,
       0,     0,     0,     0,     0,   677,     0,   442,   376,   371,
     372,   373,   374,   375,     0,     0,     0,     0,   626,   627,
     628,     0,     0,     0,     0,     0,     0,     0,   484,   456,
     591,     0,     0,   569,   573,   574,   571,   548,   466,   467,
     468,   469,     0,     0,   598,     0,     0,   560,   564,   567,
     562,   565,   566,   555,     0,     0,     0,     0,   531,   535,
     533,   536,   526,     0,   682,    39,     0,     0,     0,     0,
       0,   751,   705,     0,     0,     0,   690,     0,   777,     0,
     775,     0,     0,   214,     0,   494,   223,    26,   218,     0,
     251,   254,   259,   258,     0,   257,     0,     0,     0,   266,
     265,     0,   449,     0,     0,     0,     0,   712,   710,   711,
       0,     0,     0,     0,   739,   740,   741,     0,     0,   728,
     368,   365,   366,   725,     0,     0,   424,   424,   413,     0,
     729,   730,     0,     0,   605,     0,     0,   576,   580,   581,
     578,   572,     0,   462,     0,     0,   583,   587,   588,   585,
     563,     0,     0,     0,   538,   543,   542,   540,   534,   519,
     520,   785,   786,   787,     0,     0,   757,   758,     0,     0,
     691,     0,   433,     0,     0,   771,     0,   772,   773,   215,
     219,   262,   261,   260,     0,   267,   352,   714,   716,   720,
       0,   349,   347,   724,   718,   402,     0,     0,   622,     0,
       0,     0,     0,   461,   735,   726,   489,     0,   491,     0,
       0,   590,   594,   595,   592,   579,   465,     0,     0,   597,
     601,   602,   599,   586,   507,   509,   541,     0,   752,   753,
       0,   754,   706,     0,   419,     0,     0,     0,   776,   263,
       0,     0,     0,   625,   425,   421,   423,     0,   733,   731,
     732,     0,   604,   608,   609,   606,   593,     0,   600,     0,
     760,   759,     0,   774,     0,   779,   722,   369,   367,   738,
       0,     0,   607,   463,   521,   692,   778,     0,   736,     0,
     707,   737,   734,     0,     0,     0,   693,     0,     0,     0,
       0,     0,     0,   708,   709
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1109, -1109, -1109, -1109, -1109, -1109,  -259,   -65, -1109,    -3,
     757,   786,  1067,   763,  1235,  1236,  1242,  1253,  1256, -1109,
     -95,  -207, -1109,    -1,  -523,    19,   727, -1109,  1133, -1109,
   -1109,   -67, -1109, -1109,   -41,  -101,   -57, -1109,   752, -1109,
    1148,  -518,   782, -1109,     4,   -38,   -15,  -222,  -223, -1109,
     965, -1109,  -199,  -138,  -383,   -25,  -539,   760,  -932, -1109,
     726,    13,  1087, -1109,  -110, -1109,  1219,  -114,  1372, -1109,
   -1109,  4317, -1109,  1564,    -2,   -29,  1507, -1109, -1109, -1109,
   -1109, -1109,  1175,  1330, -1109, -1109, -1109,  -205,   -86, -1109,
   -1109, -1109,   944,  -466, -1109, -1109, -1109, -1109, -1109, -1109,
    -376,  -489, -1109, -1109, -1109, -1109, -1109, -1109,   932,  1389,
   -1109,  1040, -1109, -1109, -1109,   927, -1109, -1109, -1109, -1109,
   -1109,  -479,  -508, -1109, -1109, -1109,  -532, -1109,  -531, -1109,
     419, -1109, -1109,  1523, -1109,   811, -1109, -1109, -1109,   703,
   -1109, -1109, -1109,  -814, -1109, -1109, -1109,   684,  -440, -1109,
    -670, -1109, -1109, -1109, -1109, -1109, -1109, -1109,  -457, -1108,
   -1109, -1109, -1109,  -636, -1109, -1109, -1109, -1109, -1109, -1109,
   -1109, -1109, -1109,  -451,  -249, -1109, -1109, -1109, -1109, -1109,
     843,  -485, -1109, -1109, -1109, -1109,  -680, -1109, -1109, -1109,
    -912, -1109, -1109, -1109, -1017, -1109, -1109, -1109,   676, -1109,
   -1109, -1109,  -671, -1109, -1109, -1109,  -878, -1109, -1109, -1109,
     543, -1109, -1109, -1109,   434, -1109, -1109, -1109, -1014, -1109,
   -1109, -1109,   394, -1109, -1109, -1109, -1100, -1109, -1109, -1109,
     325, -1109, -1109, -1109,   959, -1109, -1109, -1109, -1109, -1109,
   -1109,  -592, -1109, -1109, -1109,   788, -1109, -1109, -1109, -1109,
   -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109, -1109,
   -1109, -1109, -1109, -1109, -1109, -1109,  1305, -1109, -1109, -1109,
   -1109, -1109, -1109, -1109,  -812, -1109, -1109,  -368,  -255, -1109,
    -198, -1109,  -480, -1109,    18, -1109,   -99, -1109,   -80, -1109,
    -397,   405,  -930,  -959, -1109, -1109, -1109,   342, -1109, -1109,
   -1109, -1109, -1109, -1109, -1109,   683, -1109, -1109,    79, -1109,
   -1109, -1109, -1109,   855,  -933,   442, -1109, -1109, -1109,   902,
   -1109
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    35,   387,    79,    80,    81,   432,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,   279,   115,   304,    36,   117,   219,   220,   550,
      38,    39,    40,    41,   405,   406,   258,   558,   559,    42,
     388,   389,    43,    44,    45,    46,    47,   216,   590,   416,
     417,   418,   260,   591,   445,    48,   834,   835,   836,   837,
     838,   350,   480,   119,   120,   346,   121,   122,   123,   124,
     125,   126,   127,    49,    50,    51,    52,   401,   229,    53,
     402,   230,   411,   242,   238,   243,   128,   129,   351,   130,
     131,   493,   628,   865,  1028,   880,   874,   132,   133,   511,
     895,   866,  1227,  1226,  1045,   134,   135,   494,   641,   354,
     534,   535,   136,   137,   496,   650,   651,   887,   138,   139,
     515,   945,   693,   786,   787,   969,   694,   904,   695,   905,
    1231,    54,    55,    56,   544,   790,   140,   141,   660,   897,
     142,   143,   862,  1185,   144,   145,   708,   916,   714,  1055,
     715,  1247,  1072,  1073,   146,   360,   147,   519,   925,   717,
     148,   149,   704,   705,   706,   909,   910,    57,    58,   428,
     150,   151,   521,   934,   727,  1084,  1085,   152,   153,   524,
     738,   946,  1257,   154,   155,   734,  1079,   156,   157,   942,
    1178,   158,   159,  1086,  1251,   160,   161,   710,   927,   162,
     163,   721,  1065,   164,   165,   931,  1169,   166,   167,   922,
    1066,   168,   169,  1061,  1170,   170,   171,  1075,  1243,   172,
     173,  1165,  1244,   174,   175,  1174,  1284,   176,   177,  1239,
    1285,   178,   179,   512,   680,   681,  1047,  1051,   180,   181,
     615,  1017,   182,   183,   613,   855,   184,   185,   186,   187,
     500,   188,   189,   190,   191,   192,   193,   194,   506,   657,
     195,   509,   658,   196,   197,   198,   341,   199,    59,   211,
     972,    60,  1263,  1310,   779,  1103,  1314,   868,   869,   875,
     870,   876,   871,   885,   872,   877,   873,  1140,   635,  1029,
     673,  1161,   892,  1162,  1235,  1301,  1277,  1300,   636,   547,
     200,   201,   202,   530,   754,   963,  1198,   203,   204,   430,
      62,   394,   548,   799,  1109,  1110,   205,   206,   528,   753,
     384
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      97,   249,   250,   252,   303,   257,   451,   349,   415,   233,
     423,   802,   788,   789,   644,   118,   226,   765,   280,   215,
       4,   116,   668,   867,   767,   422,   237,   629,     4,   241,
    1090,   671,   299,   805,   280,   697,   692,   213,   918,   739,
     433,   221,   208,  1111,  1112,   757,   436,   234,  1016,   434,
     936,   355,   435,  1080,   947,   437,   438,  1242,   433,   359,
     595,  1179,   716,  1119,   624,   361,   245,  1026,   913,  1187,
     726,   256,   455,   365,  1252,   366,   703,   240,   522,    61,
     281,   342,    61,   719,  1048,  1049,  1050,   617,   618,  1133,
    1134,  1135,  1136,   375,   475,   920,   342,  1141,  1163,   378,
     481,   379,   664,  1147,  1148,   380,   224,     5,   317,   289,
     290,   382,   386,  1172,   217,   386,   491,   311,   699,   409,
     444,   348,  1156,  1157,   523,   630,     4,   742,    61,   720,
     212,  1283,  1242,   859,   118,   667,     4,   735,  1030,   743,
     116,   921,     4,   492,   682,   212,     4,   854,  1288,   343,
     758,   247,   896,   728,   344,   498,   744,   210,   257,   644,
     629,  1253,  1107,   262,   263,   264,  1256,   507,   407,  1031,
    1032,   668,   302,  1283,  1201,  1188,   668,   287,   288,   499,
     671,   257,  1119,   595,   257,   671,    33,   257,   257,     4,
     257,   508,   257,   212,    33,    34,  1229,   573,  1180,  1132,
     440,   397,   446,    34,   300,   234,   231,   556,   232,   557,
     697,   907,   572,   209,   241,   556,   579,   280,   280,   280,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   280,   280,   594,   935,   449,   631,   642,
     425,   652,   454,   443,   918,   856,   767,   256,   429,   410,
     256,   739,   788,   789,   697,   929,   669,   683,   630,   926,
     696,  1083,   240,  1092,   703,   452,   729,   533,   917,   740,
    1265,  1018,   481,   940,   536,  1058,   567,   225,   762,   456,
     457,   458,   291,   292,   766,   981,    61,   967,  1287,   900,
    1270,   551,   564,   699,   442,   632,   643,   996,   653,   474,
    1318,   563,    33,   301,   555,   482,   483,   484,    33,   991,
     682,   487,    33,   670,   684,   476,   624,    34,   479,   347,
     581,    34,  1319,   730,   937,   265,   741,   266,   439,   267,
     433,   488,   624,   398,   301,   218,   257,   699,   257,   742,
    -282,   257,   728,   801,   700,   711,   407,   624,   407,   399,
    1091,   407,   247,   728,  1015,   564,   728,   442,   857,   118,
     700,   711,   553,   301,    34,   116,   814,   561,   617,   815,
     638,   631,   859,   828,   621,   700,   819,   602,   701,   702,
     337,   338,   280,   664,   642,   896,   854,   820,    11,   822,
     831,   801,  1199,   652,   634,   645,   253,    20,    21,   566,
     687,   688,   348,   689,   668,   669,  1200,   712,   339,   390,
     669,   560,   672,   671,   254,   858,   698,   349,   565,  1036,
     793,   410,   732,   683,   713,   210,   443,   301,   632,   255,
     562,   391,   686,  1230,   580,   696,   707,   935,   301,   718,
     392,   643,   803,   490,   599,   303,   745,   935,   806,   760,
     653,   697,   555,   340,   610,   729,  1118,  1089,   285,   286,
     555,   303,   670,   697,   807,  1064,   729,   670,   301,   729,
     624,   703,   926,   516,   917,   280,   794,   795,   796,   696,
     684,   740,  1307,  1091,   856,   609,   257,   612,   261,   517,
     479,   280,   301,   646,   257,  1091,   407,   518,   700,   646,
     797,   701,   702,   295,   407,  1015,   301,   839,   846,   617,
     618,   633,   730,   301,   861,   617,   618,   798,   647,   257,
     296,   257,   851,   730,   647,   937,   730,   634,   233,  1007,
     685,   832,    34,   833,   699,   937,   948,   280,   741,   731,
     645,  -280,   282,  -108,   118,   911,   699,   283,   284,   566,
     116,   997,   601,  1023,   728,   728,   301,   298,   400,  -108,
    1038,   672,   912,  1081,   728,  1024,   672,  1025,   213,   297,
     441,   562,   442,  1091,   768,  1118,   570,   935,  1015,    34,
    1210,   697,   571,  1091,   617,   618,   624,   769,   770,   301,
    1089,   698,   441,   801,   442,   648,   697,   857,   686,   664,
     793,  1186,  1089,   306,  1168,  1064,   843,   845,   305,   848,
     850,   732,   649,   307,   700,   711,   308,   840,   841,   842,
     582,   600,   732,   312,   707,   732,   583,   583,   919,   801,
     718,   860,   718,   637,   735,   698,   293,   294,   669,  1039,
    1040,  1041,  1042,  1043,   633,  1044,   604,   605,  1091,   617,
     618,   638,   301,   301,   858,   621,   794,   795,   796,   313,
     596,   914,   597,   951,   699,   937,  1015,   729,   729,   105,
     736,   448,     5,   965,   345,  1015,   696,   729,   915,   699,
    1089,   314,  1078,   353,   771,   772,   697,   737,   696,  1186,
    1089,   606,   315,  1081,   728,   670,   685,   301,  1186,   385,
     780,   781,   782,   687,   688,   783,   791,   383,  1081,  1168,
     453,     5,   792,   303,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   730,   730,   966,    37,   731,   386,
      37,  1015,   583,   948,   730,   639,   393,   303,  1000,   731,
    1015,   424,   731,   280,   301,   948,  1096,  1097,   773,   774,
     775,   776,   640,   861,  1186,  1089,  1002,   784,   777,   778,
     541,   542,   301,  1186,   395,   222,   223,   280,   447,   699,
     227,   228,  1009,  1015,   785,   403,    37,   408,   301,   113,
     617,   618,   236,   619,   620,    37,   621,  1035,   801,   244,
    -763,   624,   489,  1036,   672,   664,  1186,   477,  1081,   617,
     618,   485,   619,   620,   746,   621,   696,   729,   993,  1124,
     560,  1126,  1078,   495,   664,   301,   214,   301,   497,   700,
     711,   696,   501,   732,   732,   513,  1177,  1078,   722,   723,
     735,   707,   698,   732,   919,   780,   781,   782,   687,   688,
     783,   746,   718,   718,   698,  1004,  1006,   722,   723,   735,
    1011,   763,   764,   510,   259,  1131,   514,  1001,   747,   748,
     749,   301,  1008,   948,   730,  1151,  1062,  1153,   520,   750,
     860,  1152,   637,   301,    63,  1158,    64,    65,   948,    66,
      67,   301,   525,  1063,   527,   932,   529,   278,   617,   618,
     638,  1194,  1195,   400,   621,   747,   748,   749,  1046,    68,
     538,  1102,   933,  1189,  1202,  1054,   750,   801,   537,  1190,
     301,   696,   801,   956,   751,   540,  1177,  1078,   617,   618,
     543,   619,   620,  1205,   621,  1250,  1177,   992,     5,  1206,
      63,   752,    64,    65,    37,    66,    67,    69,    70,    71,
     731,   731,  1207,   661,   662,   663,  1208,  1214,  1206,  1082,
     731,  1216,  1206,   301,   545,    68,   546,  1036,   414,   617,
     618,   638,   698,   732,   568,   621,   722,   723,   948,  1105,
      37,   957,   958,   959,   664,   994,   995,   698,   960,  1217,
     569,   718,   718,  1218,   391,  1036,   961,  1113,  1114,  1036,
    1250,  1177,   574,    69,    70,    71,  1219,  1267,   396,  1250,
    1196,  1197,  1036,   962,   724,  1221,  1128,   259,  1290,  1291,
     575,  1036,  1120,  1222,   214,  1122,  1123,   956,  1125,   301,
    1223,   725,  1129,  1130,   584,  1224,   301,  1142,  1143,  1225,
     259,  1036,  1250,   259,  -280,  1036,   259,   259,  1228,   259,
      72,   259,   459,   460,   301,   665,  1154,   585,    73,    74,
      75,    76,    77,    78,  1159,  1233,   467,   468,  1236,  1320,
     586,   301,   666,   316,  1237,   587,    98,   698,    64,    65,
     588,    66,    67,   461,   462,   957,   958,   959,   607,  1082,
     731,  1238,   960,  1181,  1182,   614,   718,  1237,   654,   655,
     656,    68,   659,  1246,  1082,    63,    72,    64,    65,  1237,
      66,    67,   733,   808,    73,    74,    75,    76,    77,    78,
     709,   433,   502,   503,   504,   505,  1266,    99,   100,   101,
      68,   102,   103,   104,   105,   106,   107,   108,   109,    69,
      70,    71,  1262,   478,   812,  1254,  1255,  1211,   598,  1212,
    1213,   301,   301,   813,  1215,   818,  1264,   661,   662,   663,
     816,    63,  1036,    64,    65,  1273,    66,    67,    69,    70,
      71,  1237,   817,   617,   618,   638,  1293,  1296,   414,   621,
     821,  1297,  1206,  1036,  1298,  1303,    68,   301,   664,   823,
     301,  1237,  1304,  1305,  1082,   259,  1322,   259,   301,   301,
     259,   824,   583,   878,   825,   687,   688,   826,   689,   879,
     881,   214,   882,   883,   890,   396,   617,   618,   889,   619,
     620,   891,   111,   310,    69,    70,    71,   624,   898,   674,
     899,   901,   675,   676,  1321,  1271,  1272,  1269,   902,  1274,
     906,   908,    72,   928,   930,   617,   618,   113,   619,   620,
      73,    74,    75,    76,    77,    78,   624,   938,   674,   893,
     939,   675,   676,   941,   280,   114,  1289,   677,   949,   624,
     952,    72,  1292,   661,   662,   663,   894,   953,   954,    73,
      74,    75,    76,    77,    78,   964,  1299,   756,   968,   617,
     618,   638,   970,   973,   114,   621,   677,   617,   618,   638,
     974,   414,   678,   621,   664,   975,   414,   976,   977,   982,
    1299,   984,   664,   985,   983,   999,  1311,   986,   987,   679,
     988,   687,   688,   989,   689,   990,  1019,    72,  1020,   687,
     688,  1021,   689,   414,  1022,    73,    74,    75,    76,    77,
      78,  1068,  1069,  1070,  1071,   259,  1060,   735,  1027,  1033,
     608,  1034,  1052,   259,  1053,  1056,    98,     5,    64,    65,
    1057,    66,    67,  1074,  1093,   396,   463,   464,   465,   466,
    1098,  1094,  1099,  1100,  1106,  1104,  1108,  1149,   259,  1121,
     259,    68,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,  1115,    99,   100,   101,
    1116,   102,   103,   104,   105,   106,   107,   108,   109,    69,
      70,    71,  1137,   110,    31,    98,     5,    64,    65,  1138,
      66,    67,  1139,  1144,  1145,  1146,  1150,  1155,  1160,  1164,
    1173,  1191,  1192,  1193,  1203,  1204,  1220,  1234,  1258,  1259,
      68,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,  1260,    99,   100,   101,  1261,
     102,   103,   104,   105,   106,   107,   108,   109,    69,    70,
      71,  1278,   110,    31,  1275,    98,  1276,    64,    65,  1279,
      66,    67,   111,   112,   617,  1294,   638,   661,   662,   663,
     621,  1295,  1306,  1308,  1312,  1313,  1315,  1317,  1316,   664,
      68,  1323,    72,   617,   618,   638,  1324,   113,   755,   621,
      73,    74,    75,    76,    77,    78,   687,   688,   664,   689,
     469,   549,   470,   539,   811,   114,    99,   100,   101,   471,
     102,   103,   104,   105,   106,   107,   108,   109,    69,    70,
      71,   472,   309,   617,   618,   473,   619,   620,   980,   621,
     979,   111,   112,   998,   603,   531,   381,   207,   664,   246,
     884,   886,   377,   427,   759,   888,  1232,   552,   239,   950,
     690,    72,    63,     5,    64,    65,   113,    66,    67,    73,
      74,    75,    76,    77,    78,   971,  1037,   691,  1059,  1067,
    1245,   722,   723,   735,   114,  1171,  1302,    68,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,   310,    98,  1286,    64,    65,   903,    66,    67,
    1012,   526,  1280,  1309,  1101,    69,    70,    71,  1268,    30,
      31,    72,   978,   955,     0,     0,   113,     0,    68,    73,
      74,    75,    76,    77,    78,   617,   618,     0,   619,   620,
       0,   621,     0,     0,   114,     0,    63,     0,    64,    65,
       0,    66,    67,     0,    99,   100,   101,     0,   102,   103,
     104,   105,   106,   107,   108,   109,    69,    70,    71,     0,
     532,    68,    63,     0,    64,    65,     0,    66,    67,     0,
       0,     0,     0,   722,   723,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    68,     0,     0,
       0,     0,     0,     0,   616,     0,     0,     0,     0,    69,
      70,    71,     0,   617,   618,   638,   619,   620,    72,   621,
     622,   623,     0,     0,   624,     0,    73,    74,    75,    76,
      77,    78,     0,     0,     0,    69,    70,    71,   625,     0,
       0,   486,     0,     0,     0,     0,     0,     0,     0,     0,
     310,     0,     0,     0,     0,   617,   618,     0,   619,   620,
       0,   621,     0,     0,     0,    63,   624,    64,    65,    72,
      66,    67,     0,     0,   113,     0,     0,    73,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,     0,
      68,     0,   114,   419,   700,   711,    11,     0,     0,   852,
       0,     0,     0,   722,   723,    20,    21,     0,     0,     0,
       0,     0,    72,     0,     0,     0,   853,     0,     0,     0,
      73,    74,    75,    76,    77,    78,     0,     0,    69,    70,
      71,    63,     5,    64,    65,   611,    66,    67,    72,     0,
       0,   923,     0,     0,     0,     0,    73,    74,    75,    76,
      77,    78,     0,     0,     0,     0,    68,     0,   924,     0,
       0,   847,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,     0,    24,    25,    26,    27,    28,
      29,     0,    63,     0,    64,    65,     0,    66,    67,     0,
       0,     0,     0,     0,    69,    70,    71,     0,     0,    63,
       0,    64,    65,     0,    66,    67,     0,    68,     0,     0,
     576,     0,     0,    11,     0,     0,     0,     0,     0,     0,
       0,     0,    20,    21,    68,     0,     0,    63,     0,    64,
      65,    72,    66,    67,   420,     0,     0,     0,     0,    73,
     421,    75,    76,    77,    78,    69,    70,    71,    63,     0,
      64,    65,    68,    66,    67,     0,     0,     0,     0,     0,
       0,     0,    69,    70,    71,     0,     0,     0,     0,    63,
       0,    64,    65,    68,    66,    67,     0,     0,     0,    11,
       0,     0,     0,     0,     0,     0,     0,     0,    20,    21,
      69,    70,    71,     0,    68,     0,     0,    72,     0,     0,
       0,     0,     0,     0,     0,    73,    74,    75,    76,    77,
      78,    69,    70,    71,     0,     0,    63,     0,    64,    65,
       0,    66,    67,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,    70,    71,     0,     0,     0,     0,     0,
       0,    68,     0,     0,     0,     0,     0,    63,    72,    64,
      65,   577,    66,    67,     0,     0,    73,   578,    75,    76,
      77,    78,     0,     0,     0,    72,     0,   832,     0,   833,
     800,  1117,    68,    73,    74,    75,    76,    77,    78,    69,
      70,    71,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    72,     0,   832,     0,   833,   800,  1209,
       0,    73,    74,    75,    76,    77,    78,     0,     0,     0,
      69,    70,    71,    63,    72,    64,    65,     0,    66,    67,
       0,     0,    73,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,     0,    72,     0,   832,    68,   833,
     800,     0,     0,    73,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,    63,     0,    64,    65,     0,    66,
      67,     0,     0,     0,     0,   617,   618,   638,     0,     0,
       0,   621,     0,     0,     0,     0,    69,    70,    71,    68,
     664,     0,    72,   431,     0,     0,     0,     0,     0,     0,
      73,    74,    75,    76,    77,    78,     0,   687,   688,     0,
     689,     0,    63,     0,    64,    65,     0,    66,    67,     0,
       0,     0,     0,    72,   450,   735,     0,    69,    70,    71,
       0,    73,    74,    75,    76,    77,    78,    68,     0,     0,
      63,     0,    64,    65,     0,    66,    67,     0,     0,     0,
       0,     0,     0,     0,    63,     0,    64,    65,     0,    66,
      67,   943,     0,     0,     0,    68,     0,     0,    63,     0,
      64,    65,     0,    66,    67,    69,    70,    71,   944,    68,
       0,     0,    63,     0,    64,    65,     0,    66,    67,    72,
       0,     0,   592,    68,     0,     0,     0,    73,   593,    75,
      76,    77,    78,    69,    70,    71,     0,    68,     0,    63,
       0,    64,    65,     0,    66,    67,     0,    69,    70,    71,
       0,     0,     5,     0,     0,     0,     0,     0,     0,     0,
      72,    69,    70,    71,    68,   598,     0,     0,    73,    74,
      75,    76,    77,    78,     0,    69,    70,    71,     0,     0,
       0,     0,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,     0,    24,    25,    26,    27,    28,
      29,     0,    69,    70,    71,     0,     0,     0,    72,   761,
       0,     0,     0,     0,     0,     0,    73,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,     0,    63,
       0,    64,    65,     0,    66,    67,    72,     0,     0,     0,
       0,   800,     0,     0,    73,    74,    75,    76,    77,    78,
      72,     0,     0,   829,    68,     0,     0,     0,    73,   830,
      75,    76,    77,    78,    72,   844,     0,     0,     0,     0,
       0,     0,    73,    74,    75,    76,    77,    78,    72,   849,
       0,     0,     0,     0,     0,     0,    73,    74,    75,    76,
      77,    78,    69,    70,    71,    63,     0,    64,    65,     0,
      66,    67,     0,     0,     0,    72,  1003,     0,     0,     0,
       0,     0,     0,    73,    74,    75,    76,    77,    78,    63,
      68,    64,    65,   404,    66,    67,    63,     0,    64,    65,
       0,    66,    67,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    68,     0,     0,    63,     0,    64,
      65,    68,    66,    67,     0,     0,     0,     0,    69,    70,
      71,    63,     0,    64,    65,     0,    66,    67,    63,     0,
      64,    65,    68,    66,    67,     0,     0,     0,     0,     0,
       0,     0,    69,    70,    71,     0,    68,     0,     0,    69,
      70,    71,     0,    68,     0,    72,  1005,     0,     0,     0,
       0,     0,     0,    73,    74,    75,    76,    77,    78,     5,
      69,    70,    71,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    69,    70,    71,     0,     0,     0,
       0,    69,    70,    71,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,    72,  1010,     0,     0,     0,     0,     0,     0,    73,
      74,    75,    76,    77,    78,    30,    31,     0,     0,     0,
       0,     0,     0,     0,     0,    72,  1127,     0,     0,     0,
       0,     0,    72,    73,    74,    75,    76,    77,    78,   616,
      73,    74,    75,    76,    77,    78,     0,     0,   617,   618,
     638,   619,   620,   248,   621,   622,   623,     0,     0,   624,
       0,    73,    74,    75,    76,    77,    78,   251,     1,     2,
       3,     4,     5,   625,    72,    73,    74,    75,    76,    77,
      78,     0,    73,  1095,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     4,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    30,    31,
       0,     0,     0,     0,     0,     0,     0,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,   662,   663,     0,     0,     0,
       0,   616,     0,     0,     0,     0,     0,     0,    30,    31,
     617,   618,   638,   619,   620,     0,   621,   622,   623,     0,
       0,   624,     0,     0,     0,   664,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   625,     0,     0,    32,     5,
       0,     0,   687,   688,     0,   689,     0,     0,     0,   700,
     711,     0,     0,     0,     0,     0,     0,    33,   722,   723,
     735,     0,     0,     0,     0,     0,    34,     0,     0,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,     0,    24,    25,    26,    27,    28,    29,    32,     0,
       0,     4,     5,     0,     0,     0,  1281,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    33,     0,     0,
       0,     0,     0,  1282,     0,     0,    34,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     4,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    30,    31,
       0,     0,     0,     0,     0,     0,     0,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     4,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   426,    31,
     554,     0,     0,     0,     0,     5,     0,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     4,     5,     0,     0,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    33,    24,    25,
      26,    27,    28,    29,     0,     0,    34,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     4,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    33,   235,     0,
       0,     0,     0,     0,     0,     0,    34,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   564,   589,   442,
       0,     0,     0,     0,     0,     5,    34,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     5,     0,     0,     0,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    33,    24,    25,
      26,    27,    28,    29,     0,     5,    34,     0,     0,     0,
       0,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,     0,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,     5,     0,    12,    13,    14,    15,
      16,    17,    18,    19,     0,     0,    22,    33,    24,    25,
      26,    27,    28,    29,     0,     0,    34,     0,     0,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   441,   589,   442,
      30,    31,   412,     5,     0,     0,    34,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   617,   618,     0,   619,
     620,     0,   621,     0,     0,     0,   804,   624,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,   809,     5,     0,   700,   711,     0,     0,     0,
       0,     0,     0,     0,   722,   723,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,   662,   663,   113,     0,     0,     0,   616,     0,
       0,     0,     0,     0,     0,     0,     0,   617,   618,   638,
     619,   620,     0,   621,   622,   623,     0,     0,   624,     0,
       0,     0,   664,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   625,     0,     0,     0,     0,     0,     0,   687,
     688,     0,   689,     0,     0,     0,     0,     0,   662,   663,
       0,     0,     0,     0,   616,   722,   723,   735,     0,   413,
       0,     0,     0,   617,   618,   638,   619,   620,     0,   621,
     622,   623,     0,     0,   624,     0,     0,     0,   664,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   625,     0,
       0,     0,     0,  1248,     0,     0,     0,     0,     0,     0,
     662,   663,   700,   711,     0,     0,   616,     0,     0,   827,
    1249,   722,   723,   735,     0,   617,   618,   638,   619,   620,
       0,   621,   622,   623,     0,     0,   624,     0,     0,     0,
     664,     0,     0,     0,     0,     0,     0,   662,   663,     0,
     625,     0,     0,   616,     0,     0,     0,   687,   688,  1240,
     689,     0,   617,   618,   638,   619,   620,     0,   621,   622,
     623,     0,     0,   624,     0,   735,  1241,   664,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   625,     0,     0,
       0,     0,     0,     0,   687,   688,     0,   689,     0,   662,
     663,     0,     0,     0,     0,   616,     0,     0,     0,     0,
       0,  1183,     0,     0,   617,   618,   638,   619,   620,     0,
     621,   622,   623,     0,     0,   624,     0,     0,  1184,   664,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   625,
       0,     0,     0,     0,     0,     0,     0,     0,  1013,     0,
       0,   662,   663,     0,     0,     0,     0,   616,     0,     0,
       0,     0,   722,   723,   735,  1014,   617,   618,   638,   619,
     620,     0,   621,   622,   623,     0,     0,   624,     0,     0,
       0,   664,     0,     0,     0,     0,     0,     0,   662,   663,
       0,   625,     0,     0,   616,     0,     0,     0,     0,     0,
    1175,     0,     0,   617,   618,   638,   619,   620,     0,   621,
     622,   623,     0,     0,   624,     0,   735,  1176,   664,     0,
     616,     0,     0,     0,     0,     0,     0,     0,   625,   617,
     618,     0,   619,   620,     0,   621,   622,   623,     0,     0,
     624,     0,   617,   618,   638,   619,   620,     0,   621,     0,
       0,     0,  1087,   624,   625,     0,     0,   664,     0,     0,
       0,   617,   618,   638,   619,   620,     0,   621,     0,  1088,
       0,     0,     0,     0,   687,   688,   664,   689,     0,     0,
       0,   700,   711,     0,     0,     0,     0,     0,     0,   863,
     722,   723,   735,   687,   688,     0,   689,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   864,     0,     0,   722,
     723,   735,     0,     0,     0,   626,     0,     0,     0,     0,
       0,   318,   319,     0,   320,     0,     0,     0,  1166,     0,
       0,     0,   627,   321,   322,   323,   324,   325,   326,     0,
       0,     0,     0,     0,     0,  1167,     0,  1076,     0,   327,
       0,   328,     0,   329,     0,     0,     0,   330,     0,     0,
       0,     0,     0,     0,  1077,   318,   319,   331,   320,     0,
       0,     0,     0,     0,     0,   210,   332,   321,   322,   323,
     324,   325,   326,     0,     0,   333,   334,     0,     0,     0,
       0,     0,     0,   327,   335,   328,   336,   329,     0,     0,
       0,   330,     0,   318,   319,     0,   320,     0,     0,   318,
     319,   331,   320,   756,     0,   321,   322,   323,     0,   325,
     332,   321,   322,   323,     0,   325,     0,     0,     0,   333,
     334,   327,     0,   328,     0,     0,     0,   327,   335,   328,
     336,     0,     0,     0,     0,     0,     0,     0,     0,   331,
       0,     0,     0,     0,     0,   331,     0,     0,   332,     0,
       0,     0,     0,     0,   332,     0,     0,   333,   334,   662,
     663,     0,     0,   333,   334,   616,   335,     0,     0,     0,
       0,     0,   335,     0,   617,   618,   638,   619,   620,     0,
     621,   622,   623,     0,     0,   624,     0,     0,     0,   664,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   625,
       0,     0,     0,     0,     0,     0,   687,   688,     0,   689,
     662,   663,     0,   700,   711,     0,   616,     0,     0,     0,
       0,     0,   722,   723,   735,   617,   618,   638,   619,   620,
       0,   621,   622,   623,     0,     0,   624,     0,     0,     0,
     664,     0,     0,     0,     0,     0,   662,   663,     0,     0,
     625,     0,   616,     0,     0,     0,     0,   687,   688,     0,
     689,   617,   618,   638,   619,   620,     0,   621,   622,   623,
       0,     0,   624,   722,   723,   735,   664,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   625,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   662,   663,     0,
     700,   711,     0,   616,     0,     0,     0,     0,     0,   722,
     723,   735,   617,   618,   638,   619,   620,     0,   621,   622,
     623,     0,     0,   624,     0,     0,     0,   664,     0,     0,
       0,     0,     0,   662,   663,     0,     0,   625,     0,   616,
       0,     0,     0,     0,   687,   688,     0,   689,   617,   618,
     638,   619,   620,     0,   621,   622,   623,     0,     0,   624,
     662,   663,   735,   664,     0,     0,   616,     0,     0,     0,
       0,     0,     0,   625,     0,   617,   618,   638,   619,   620,
       0,   621,   622,   623,     0,     0,   624,   662,   663,     0,
     664,     0,     0,   616,     0,     0,   722,   723,   735,     0,
     625,     0,   617,   618,   638,   619,   620,     0,   621,   622,
     623,     0,     0,   624,     0,     0,     0,   664,   617,   618,
     638,   619,   620,     0,   621,   735,     0,   625,     0,   624,
       0,     0,     0,   664,   687,   688,     0,   689,     0,   617,
     618,   638,   619,   620,     0,   621,     0,     0,     0,     0,
     687,   688,     0,   689,   664,     0,     0,   700,   711,     0,
       0,     0,     0,     0,     0,     0,   722,   723,   735,     0,
       0,   687,   688,     0,   689,   617,   618,     0,   619,   620,
       0,   621,   616,     0,     0,     0,   624,   722,   723,   735,
     664,   617,   618,     0,   619,   620,     0,   621,   622,   623,
       0,     5,   624,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   700,   711,   625,     0,     0,     0,
       0,     0,     0,   722,   723,   735,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
     810,     5,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
     352,     0,     0,     0,     0,     0,   356,     0,   357,     0,
     358,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     362,     0,   363,     0,   364,     0,     0,     0,     0,     0,
     367,     0,   368,     0,   369,     0,   370,     0,   371,     0,
     372,     0,   373,     0,   374,   662,   663,     0,   376,     0,
       0,   616,     0,     0,     0,     0,     0,     0,     0,     0,
     617,   618,   638,   619,   620,     0,   621,   622,   623,     0,
       0,   624,     0,     0,     0,   664,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   625
};

static const yytype_int16 yycheck[] =
{
       1,    66,    67,    68,    99,    72,   265,   121,   231,    47,
     232,   550,   544,   544,   494,     2,    41,   540,    83,    34,
       6,     2,   511,   615,   542,   232,    55,   493,     6,    58,
     942,   511,    20,   556,    99,   515,   515,    33,   708,   524,
     247,    37,     6,   976,   977,    73,   253,    49,   862,   248,
     721,   137,   251,   931,   734,   254,   255,  1165,   265,   145,
     443,  1075,   519,   995,   104,   151,    62,   879,   704,  1086,
     521,    72,   279,   159,  1174,   161,   516,    58,    75,     0,
      83,   105,     3,    75,   145,   146,   147,    93,    94,  1019,
    1020,  1021,  1022,   179,   301,    75,   105,  1027,  1057,   185,
     305,   187,   108,  1033,  1034,   191,     6,     7,   109,    15,
      16,   197,     6,  1072,     0,     6,    76,   104,   515,   229,
     258,   117,  1052,  1053,   121,   493,     6,   524,    49,   121,
     154,  1239,  1240,   613,   121,   511,     6,   143,    88,   179,
     121,   121,     6,   103,   512,   154,     6,   613,  1248,   158,
     178,   172,   660,   521,   163,   172,   196,   129,   225,   639,
     626,  1175,   974,    10,    11,    12,  1183,   172,   225,   119,
     120,   660,   193,  1281,  1104,  1087,   665,    13,    14,   196,
     660,   248,  1114,   566,   251,   665,   172,   254,   255,     6,
     257,   196,   259,   154,   172,   181,  1155,   419,  1076,  1013,
     257,   216,   259,   181,   192,   207,   172,   193,   174,   195,
     690,   690,   419,   177,   243,   193,   423,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   442,   721,   262,   493,   494,
     236,   496,   267,   258,   914,   613,   764,   248,   244,   230,
     251,   736,   784,   784,   734,   712,   511,   512,   626,   710,
     515,   932,   243,   943,   704,   266,   521,   353,   708,   524,
    1203,   863,   477,   724,   360,   911,   414,   177,   537,   282,
     283,   284,   188,   189,   178,   808,   207,   178,  1247,   665,
    1220,   401,   172,   690,   174,   493,   494,   836,   496,   300,
     172,   411,   172,   179,   405,   306,   307,   308,   172,   832,
     678,   312,   172,   511,   512,   302,   104,   181,   305,   195,
     430,   181,   194,   521,   721,   172,   524,   174,   173,   176,
     537,   312,   104,   179,   179,   195,   403,   734,   405,   736,
     177,   408,   710,   550,   132,   133,   403,   104,   405,   195,
     942,   408,   172,   721,   862,   172,   724,   174,   613,   346,
     132,   133,   403,   179,   181,   346,   573,   408,    93,   576,
      95,   626,   852,   596,    99,   132,   583,   193,   135,   136,
      75,    76,   447,   108,   639,   893,   852,   586,    37,   588,
     597,   598,   179,   648,   493,   494,   172,    46,    47,   414,
     125,   126,   398,   128,   893,   660,   193,   179,   103,    99,
     665,   407,   511,   893,   172,   613,   515,   531,   414,   179,
      99,   402,   521,   678,   196,   129,   441,   179,   626,   172,
     411,   121,   512,   193,   138,   690,   516,   922,   179,   519,
     130,   639,   552,   195,   447,   540,   526,   932,   179,   535,
     648,   931,   553,   148,   195,   710,   995,   942,   182,   183,
     561,   556,   660,   943,   195,   922,   721,   665,   179,   724,
     104,   911,   923,   115,   914,   540,   155,   156,   157,   734,
     678,   736,   193,  1075,   852,   486,   553,   488,     9,   131,
     477,   556,   179,    77,   561,  1087,   553,   139,   132,    77,
     179,   135,   136,   180,   561,  1013,   179,   602,   195,    93,
      94,   493,   710,   179,   613,    93,    94,   196,   102,   586,
     190,   588,   195,   721,   102,   922,   724,   626,   566,   195,
     512,   174,   181,   176,   931,   932,   734,   602,   736,   521,
     639,   177,   181,   179,   531,   179,   943,   186,   187,   564,
     531,   194,   175,    84,   922,   923,   179,    19,   194,   195,
       1,   660,   196,   931,   932,    96,   665,    98,   564,   191,
     172,   552,   174,  1165,     6,  1114,   173,  1062,  1086,   181,
    1119,  1061,   179,  1175,    93,    94,   104,    19,    20,   179,
    1075,   690,   172,   800,   174,   179,  1076,   852,   678,   108,
      99,  1086,  1087,   172,  1061,  1062,   607,   608,   193,   610,
     611,   710,   196,   172,   132,   133,   172,   604,   605,   606,
     173,   173,   721,   172,   704,   724,   179,   179,   708,   836,
     710,   613,   712,    77,   143,   734,    17,    18,   893,    80,
      81,    82,    83,    84,   626,    86,   173,   173,  1240,    93,
      94,    95,   179,   179,   852,    99,   155,   156,   157,     6,
     172,   179,   174,   743,  1061,  1062,  1174,   922,   923,    64,
     179,     6,     7,   759,   178,  1183,   931,   932,   196,  1076,
    1165,   195,   931,   177,   116,   117,  1166,   196,   943,  1174,
    1175,   173,   195,  1061,  1062,   893,   678,   179,  1183,   177,
     122,   123,   124,   125,   126,   127,   173,     6,  1076,  1166,
       6,     7,   179,   808,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,   922,   923,   173,     0,   710,     6,
       3,  1239,   179,   931,   932,   179,   172,   832,   173,   721,
    1248,   129,   724,   808,   179,   943,   953,   954,   180,   181,
     182,   183,   196,   852,  1239,  1240,   173,   179,   190,   191,
     178,   179,   179,  1248,   173,    38,    39,   832,   173,  1166,
      43,    44,   173,  1281,   196,   177,    49,   177,   179,   177,
      93,    94,    55,    96,    97,    58,    99,   173,   995,    62,
     177,   104,   195,   179,   893,   108,  1281,   193,  1166,    93,
      94,    62,    96,    97,   106,    99,  1061,  1062,   833,   173,
     806,   173,  1061,   196,   108,   179,    34,   179,   196,   132,
     133,  1076,   196,   922,   923,   196,  1075,  1076,   141,   142,
     143,   911,   931,   932,   914,   122,   123,   124,   125,   126,
     127,   106,   922,   923,   943,   846,   847,   141,   142,   143,
     851,   178,   179,   121,    72,   173,   196,   844,   160,   161,
     162,   179,   849,  1061,  1062,   173,   179,   173,   140,   171,
     852,   179,    77,   179,     6,   173,     8,     9,  1076,    11,
      12,   179,   196,   196,   196,   179,   196,   194,    93,    94,
      95,  1098,  1099,   194,    99,   160,   161,   162,   899,    31,
     195,   968,   196,   173,   173,   906,   171,  1114,   172,   179,
     179,  1166,  1119,   106,   179,   194,  1165,  1166,    93,    94,
     172,    96,    97,   173,    99,  1174,  1175,     6,     7,   179,
       6,   196,     8,     9,   207,    11,    12,    69,    70,    71,
     922,   923,   173,    77,    78,    79,   173,   173,   179,   931,
     932,   173,   179,   179,   196,    31,     6,   179,   231,    93,
      94,    95,  1061,  1062,   173,    99,   141,   142,  1166,   970,
     243,   164,   165,   166,   108,   178,   179,  1076,   171,   173,
     179,  1061,  1062,   173,   121,   179,   179,   178,   179,   179,
    1239,  1240,   175,    69,    70,    71,   173,  1204,   216,  1248,
     167,   168,   179,   196,   179,   173,  1007,   225,   169,   170,
     175,   179,   999,   173,   232,  1002,  1003,   106,  1005,   179,
     173,   196,  1009,  1010,   173,   173,   179,  1028,  1029,   173,
     248,   179,  1281,   251,   177,   179,   254,   255,   173,   257,
     172,   259,   285,   286,   179,   179,  1047,   173,   180,   181,
     182,   183,   184,   185,  1055,   173,   293,   294,   173,  1318,
     179,   179,   196,   195,   179,   179,     6,  1166,     8,     9,
     179,    11,    12,   287,   288,   164,   165,   166,   172,  1061,
    1062,   173,   171,  1084,  1085,   121,  1166,   179,     6,   196,
     149,    31,   196,   173,  1076,     6,   172,     8,     9,   179,
      11,    12,   103,   193,   180,   181,   182,   183,   184,   185,
     140,  1318,   112,   113,   114,   115,   192,    57,    58,    59,
      31,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,  1199,    73,     6,   173,   173,  1124,   177,  1126,
    1127,   179,   179,   175,  1131,   129,   173,    77,    78,    79,
     175,     6,   179,     8,     9,   173,    11,    12,    69,    70,
      71,   179,   175,    93,    94,    95,   173,   173,   441,    99,
       6,   173,   179,   179,   173,   173,    31,   179,   108,   173,
     179,   179,   173,   173,  1166,   403,   173,   405,   179,   179,
     408,   173,   179,   172,   175,   125,   126,   175,   128,   172,
     172,   419,   172,   172,   196,   423,    93,    94,   173,    96,
      97,     6,   152,   153,    69,    70,    71,   104,   172,   106,
     172,   172,   109,   110,  1319,  1226,  1227,  1214,   172,  1230,
     172,   172,   172,   172,   103,    93,    94,   177,    96,    97,
     180,   181,   182,   183,   184,   185,   104,   172,   106,   179,
     172,   109,   110,   121,  1319,   195,  1257,   144,   172,   104,
     196,   172,  1263,    77,    78,    79,   196,   172,   172,   180,
     181,   182,   183,   184,   185,   196,  1277,    85,   193,    93,
      94,    95,   172,     6,   195,    99,   144,    93,    94,    95,
     172,   564,   179,    99,   108,   172,   569,   172,   172,   175,
    1301,   130,   108,   173,   175,    60,  1307,   173,   173,   196,
     173,   125,   126,   175,   128,   175,   172,   172,   172,   125,
     126,   172,   128,   596,   172,   180,   181,   182,   183,   184,
     185,   134,   135,   136,   137,   553,   103,   143,   172,   172,
     195,   172,   172,   561,   172,   172,     6,     7,     8,     9,
     172,    11,    12,   121,    80,   573,   289,   290,   291,   292,
     172,   196,   172,   172,   196,   172,     6,     6,   586,   195,
     588,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   196,    57,    58,    59,
     175,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,   173,    73,    74,     6,     7,     8,     9,   173,
      11,    12,   173,   173,   173,   173,   179,   193,     6,   121,
     193,   173,   173,   173,   193,   174,   193,   174,   173,   173,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,   179,    57,    58,    59,   173,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,   174,    73,    74,   173,     6,   173,     8,     9,     6,
      11,    12,   152,   153,    93,     6,    95,    77,    78,    79,
      99,   175,   175,   175,   175,   151,   172,     6,   196,   108,
      31,   173,   172,    93,    94,    95,   173,   177,   178,    99,
     180,   181,   182,   183,   184,   185,   125,   126,   108,   128,
     295,   398,   296,   385,   569,   195,    57,    58,    59,   297,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,   298,    73,    93,    94,   299,    96,    97,   806,    99,
     800,   152,   153,   837,   477,   346,   194,     3,   108,    62,
     626,   639,   183,   243,   534,   648,  1157,   402,    55,   736,
     179,   172,     6,     7,     8,     9,   177,    11,    12,   180,
     181,   182,   183,   184,   185,   784,   893,   196,   914,   923,
    1166,   141,   142,   143,   195,  1062,  1281,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   153,     6,  1240,     8,     9,   678,    11,    12,
     852,   336,  1237,  1301,   961,    69,    70,    71,  1206,    73,
      74,   172,   797,   751,    -1,    -1,   177,    -1,    31,   180,
     181,   182,   183,   184,   185,    93,    94,    -1,    96,    97,
      -1,    99,    -1,    -1,   195,    -1,     6,    -1,     8,     9,
      -1,    11,    12,    -1,    57,    58,    59,    -1,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    -1,
      73,    31,     6,    -1,     8,     9,    -1,    11,    12,    -1,
      -1,    -1,    -1,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    84,    -1,    -1,    -1,    -1,    69,
      70,    71,    -1,    93,    94,    95,    96,    97,   172,    99,
     100,   101,    -1,    -1,   104,    -1,   180,   181,   182,   183,
     184,   185,    -1,    -1,    -1,    69,    70,    71,   118,    -1,
      -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     153,    -1,    -1,    -1,    -1,    93,    94,    -1,    96,    97,
      -1,    99,    -1,    -1,    -1,     6,   104,     8,     9,   172,
      11,    12,    -1,    -1,   177,    -1,    -1,   180,   181,   182,
     183,   184,   185,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,   195,    34,   132,   133,    37,    -1,    -1,   179,
      -1,    -1,    -1,   141,   142,    46,    47,    -1,    -1,    -1,
      -1,    -1,   172,    -1,    -1,    -1,   196,    -1,    -1,    -1,
     180,   181,   182,   183,   184,   185,    -1,    -1,    69,    70,
      71,     6,     7,     8,     9,   195,    11,    12,   172,    -1,
      -1,   179,    -1,    -1,    -1,    -1,   180,   181,   182,   183,
     184,   185,    -1,    -1,    -1,    -1,    31,    -1,   196,    -1,
      -1,   195,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    51,    52,    53,    54,
      55,    -1,     6,    -1,     8,     9,    -1,    11,    12,    -1,
      -1,    -1,    -1,    -1,    69,    70,    71,    -1,    -1,     6,
      -1,     8,     9,    -1,    11,    12,    -1,    31,    -1,    -1,
      34,    -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    31,    -1,    -1,     6,    -1,     8,
       9,   172,    11,    12,   175,    -1,    -1,    -1,    -1,   180,
     181,   182,   183,   184,   185,    69,    70,    71,     6,    -1,
       8,     9,    31,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    70,    71,    -1,    -1,    -1,    -1,     6,
      -1,     8,     9,    31,    11,    12,    -1,    -1,    -1,    37,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      69,    70,    71,    -1,    31,    -1,    -1,   172,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   180,   181,   182,   183,   184,
     185,    69,    70,    71,    -1,    -1,     6,    -1,     8,     9,
      -1,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    70,    71,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    -1,    -1,     6,   172,     8,
       9,   175,    11,    12,    -1,    -1,   180,   181,   182,   183,
     184,   185,    -1,    -1,    -1,   172,    -1,   174,    -1,   176,
     177,   178,    31,   180,   181,   182,   183,   184,   185,    69,
      70,    71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   172,    -1,   174,    -1,   176,   177,   178,
      -1,   180,   181,   182,   183,   184,   185,    -1,    -1,    -1,
      69,    70,    71,     6,   172,     8,     9,    -1,    11,    12,
      -1,    -1,   180,   181,   182,   183,   184,   185,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   172,    -1,   174,    31,   176,
     177,    -1,    -1,   180,   181,   182,   183,   184,   185,    -1,
      -1,    -1,    -1,    -1,     6,    -1,     8,     9,    -1,    11,
      12,    -1,    -1,    -1,    -1,    93,    94,    95,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    69,    70,    71,    31,
     108,    -1,   172,   173,    -1,    -1,    -1,    -1,    -1,    -1,
     180,   181,   182,   183,   184,   185,    -1,   125,   126,    -1,
     128,    -1,     6,    -1,     8,     9,    -1,    11,    12,    -1,
      -1,    -1,    -1,   172,   173,   143,    -1,    69,    70,    71,
      -1,   180,   181,   182,   183,   184,   185,    31,    -1,    -1,
       6,    -1,     8,     9,    -1,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     6,    -1,     8,     9,    -1,    11,
      12,   179,    -1,    -1,    -1,    31,    -1,    -1,     6,    -1,
       8,     9,    -1,    11,    12,    69,    70,    71,   196,    31,
      -1,    -1,     6,    -1,     8,     9,    -1,    11,    12,   172,
      -1,    -1,   175,    31,    -1,    -1,    -1,   180,   181,   182,
     183,   184,   185,    69,    70,    71,    -1,    31,    -1,     6,
      -1,     8,     9,    -1,    11,    12,    -1,    69,    70,    71,
      -1,    -1,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     172,    69,    70,    71,    31,   177,    -1,    -1,   180,   181,
     182,   183,   184,   185,    -1,    69,    70,    71,    -1,    -1,
      -1,    -1,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    51,    52,    53,    54,
      55,    -1,    69,    70,    71,    -1,    -1,    -1,   172,   173,
      -1,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,   183,
     184,   185,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     6,
      -1,     8,     9,    -1,    11,    12,   172,    -1,    -1,    -1,
      -1,   177,    -1,    -1,   180,   181,   182,   183,   184,   185,
     172,    -1,    -1,   175,    31,    -1,    -1,    -1,   180,   181,
     182,   183,   184,   185,   172,   173,    -1,    -1,    -1,    -1,
      -1,    -1,   180,   181,   182,   183,   184,   185,   172,   173,
      -1,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,   183,
     184,   185,    69,    70,    71,     6,    -1,     8,     9,    -1,
      11,    12,    -1,    -1,    -1,   172,   173,    -1,    -1,    -1,
      -1,    -1,    -1,   180,   181,   182,   183,   184,   185,     6,
      31,     8,     9,   178,    11,    12,     6,    -1,     8,     9,
      -1,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,     6,    -1,     8,
       9,    31,    11,    12,    -1,    -1,    -1,    -1,    69,    70,
      71,     6,    -1,     8,     9,    -1,    11,    12,     6,    -1,
       8,     9,    31,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    70,    71,    -1,    31,    -1,    -1,    69,
      70,    71,    -1,    31,    -1,   172,   173,    -1,    -1,    -1,
      -1,    -1,    -1,   180,   181,   182,   183,   184,   185,     7,
      69,    70,    71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    70,    71,    -1,    -1,    -1,
      -1,    69,    70,    71,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,   172,   173,    -1,    -1,    -1,    -1,    -1,    -1,   180,
     181,   182,   183,   184,   185,    73,    74,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   172,   173,    -1,    -1,    -1,
      -1,    -1,   172,   180,   181,   182,   183,   184,   185,    84,
     180,   181,   182,   183,   184,   185,    -1,    -1,    93,    94,
      95,    96,    97,   172,    99,   100,   101,    -1,    -1,   104,
      -1,   180,   181,   182,   183,   184,   185,   172,     3,     4,
       5,     6,     7,   118,   172,   180,   181,   182,   183,   184,
     185,    -1,   180,   181,   182,   183,   184,   185,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    74,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    78,    79,    -1,    -1,    -1,
      -1,    84,    -1,    -1,    -1,    -1,    -1,    -1,    73,    74,
      93,    94,    95,    96,    97,    -1,    99,   100,   101,    -1,
      -1,   104,    -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   118,    -1,    -1,   153,     7,
      -1,    -1,   125,   126,    -1,   128,    -1,    -1,    -1,   132,
     133,    -1,    -1,    -1,    -1,    -1,    -1,   172,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    51,    52,    53,    54,    55,   153,    -1,
      -1,     6,     7,    -1,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,
      -1,    -1,    -1,   196,    -1,    -1,   181,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    74,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    74,
     178,    -1,    -1,    -1,    -1,     7,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,     6,     7,    -1,    -1,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,   172,    50,    51,
      52,    53,    54,    55,    -1,    -1,   181,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,    73,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   181,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,
      -1,    -1,    -1,    -1,    -1,     7,   181,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,     7,    -1,    -1,    -1,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,   172,    50,    51,
      52,    53,    54,    55,    -1,     7,   181,    -1,    -1,    -1,
      -1,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,     7,    -1,    38,    39,    40,    41,
      42,    43,    44,    45,    -1,    -1,    48,   172,    50,    51,
      52,    53,    54,    55,    -1,    -1,   181,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,
      73,    74,     6,     7,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    -1,    96,
      97,    -1,    99,    -1,    -1,    -1,   178,   104,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   178,     7,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    78,    79,   177,    -1,    -1,    -1,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,
      96,    97,    -1,    99,   100,   101,    -1,    -1,   104,    -1,
      -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,    -1,    -1,    -1,    -1,    -1,    78,    79,
      -1,    -1,    -1,    -1,    84,   141,   142,   143,    -1,   173,
      -1,    -1,    -1,    93,    94,    95,    96,    97,    -1,    99,
     100,   101,    -1,    -1,   104,    -1,    -1,    -1,   108,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,
      -1,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,   132,   133,    -1,    -1,    84,    -1,    -1,   173,
     196,   141,   142,   143,    -1,    93,    94,    95,    96,    97,
      -1,    99,   100,   101,    -1,    -1,   104,    -1,    -1,    -1,
     108,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    -1,
     118,    -1,    -1,    84,    -1,    -1,    -1,   125,   126,   179,
     128,    -1,    93,    94,    95,    96,    97,    -1,    99,   100,
     101,    -1,    -1,   104,    -1,   143,   196,   108,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,    -1,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,    -1,    78,
      79,    -1,    -1,    -1,    -1,    84,    -1,    -1,    -1,    -1,
      -1,   179,    -1,    -1,    93,    94,    95,    96,    97,    -1,
      99,   100,   101,    -1,    -1,   104,    -1,    -1,   196,   108,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   179,    -1,
      -1,    78,    79,    -1,    -1,    -1,    -1,    84,    -1,    -1,
      -1,    -1,   141,   142,   143,   196,    93,    94,    95,    96,
      97,    -1,    99,   100,   101,    -1,    -1,   104,    -1,    -1,
      -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,
      -1,   118,    -1,    -1,    84,    -1,    -1,    -1,    -1,    -1,
     179,    -1,    -1,    93,    94,    95,    96,    97,    -1,    99,
     100,   101,    -1,    -1,   104,    -1,   143,   196,   108,    -1,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    93,
      94,    -1,    96,    97,    -1,    99,   100,   101,    -1,    -1,
     104,    -1,    93,    94,    95,    96,    97,    -1,    99,    -1,
      -1,    -1,   179,   104,   118,    -1,    -1,   108,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    -1,    99,    -1,   196,
      -1,    -1,    -1,    -1,   125,   126,   108,   128,    -1,    -1,
      -1,   132,   133,    -1,    -1,    -1,    -1,    -1,    -1,   179,
     141,   142,   143,   125,   126,    -1,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,    -1,   141,
     142,   143,    -1,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,    75,    76,    -1,    78,    -1,    -1,    -1,   179,    -1,
      -1,    -1,   196,    87,    88,    89,    90,    91,    92,    -1,
      -1,    -1,    -1,    -1,    -1,   196,    -1,   179,    -1,   103,
      -1,   105,    -1,   107,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,   196,    75,    76,   121,    78,    -1,
      -1,    -1,    -1,    -1,    -1,   129,   130,    87,    88,    89,
      90,    91,    92,    -1,    -1,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,   103,   148,   105,   150,   107,    -1,    -1,
      -1,   111,    -1,    75,    76,    -1,    78,    -1,    -1,    75,
      76,   121,    78,    85,    -1,    87,    88,    89,    -1,    91,
     130,    87,    88,    89,    -1,    91,    -1,    -1,    -1,   139,
     140,   103,    -1,   105,    -1,    -1,    -1,   103,   148,   105,
     150,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,   130,    -1,
      -1,    -1,    -1,    -1,   130,    -1,    -1,   139,   140,    78,
      79,    -1,    -1,   139,   140,    84,   148,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    93,    94,    95,    96,    97,    -1,
      99,   100,   101,    -1,    -1,   104,    -1,    -1,    -1,   108,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,
      -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
      78,    79,    -1,   132,   133,    -1,    84,    -1,    -1,    -1,
      -1,    -1,   141,   142,   143,    93,    94,    95,    96,    97,
      -1,    99,   100,   101,    -1,    -1,   104,    -1,    -1,    -1,
     108,    -1,    -1,    -1,    -1,    -1,    78,    79,    -1,    -1,
     118,    -1,    84,    -1,    -1,    -1,    -1,   125,   126,    -1,
     128,    93,    94,    95,    96,    97,    -1,    99,   100,   101,
      -1,    -1,   104,   141,   142,   143,   108,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    -1,
     132,   133,    -1,    84,    -1,    -1,    -1,    -1,    -1,   141,
     142,   143,    93,    94,    95,    96,    97,    -1,    99,   100,
     101,    -1,    -1,   104,    -1,    -1,    -1,   108,    -1,    -1,
      -1,    -1,    -1,    78,    79,    -1,    -1,   118,    -1,    84,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,    93,    94,
      95,    96,    97,    -1,    99,   100,   101,    -1,    -1,   104,
      78,    79,   143,   108,    -1,    -1,    84,    -1,    -1,    -1,
      -1,    -1,    -1,   118,    -1,    93,    94,    95,    96,    97,
      -1,    99,   100,   101,    -1,    -1,   104,    78,    79,    -1,
     108,    -1,    -1,    84,    -1,    -1,   141,   142,   143,    -1,
     118,    -1,    93,    94,    95,    96,    97,    -1,    99,   100,
     101,    -1,    -1,   104,    -1,    -1,    -1,   108,    93,    94,
      95,    96,    97,    -1,    99,   143,    -1,   118,    -1,   104,
      -1,    -1,    -1,   108,   125,   126,    -1,   128,    -1,    93,
      94,    95,    96,    97,    -1,    99,    -1,    -1,    -1,    -1,
     125,   126,    -1,   128,   108,    -1,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,    -1,
      -1,   125,   126,    -1,   128,    93,    94,    -1,    96,    97,
      -1,    99,    84,    -1,    -1,    -1,   104,   141,   142,   143,
     108,    93,    94,    -1,    96,    97,    -1,    99,   100,   101,
      -1,     7,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,   118,    -1,    -1,    -1,
      -1,    -1,    -1,   141,   142,   143,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
     133,    -1,    -1,    -1,    -1,    -1,   139,    -1,   141,    -1,
     143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     153,    -1,   155,    -1,   157,    -1,    -1,    -1,    -1,    -1,
     163,    -1,   165,    -1,   167,    -1,   169,    -1,   171,    -1,
     173,    -1,   175,    -1,   177,    78,    79,    -1,   181,    -1,
      -1,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      93,    94,    95,    96,    97,    -1,    99,   100,   101,    -1,
      -1,   104,    -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   118
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     3,     4,     5,     6,     7,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      73,    74,   153,   172,   181,   198,   222,   223,   227,   228,
     229,   230,   236,   239,   240,   241,   242,   243,   252,   270,
     271,   272,   273,   276,   328,   329,   330,   364,   365,   465,
     468,   505,   507,     6,     8,     9,    11,    12,    31,    69,
      70,    71,   172,   180,   181,   182,   183,   184,   185,   200,
     201,   202,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   220,     6,    57,
      58,    59,    61,    62,    63,    64,    65,    66,    67,    68,
      73,   152,   153,   177,   195,   220,   222,   223,   258,   260,
     261,   263,   264,   265,   266,   267,   268,   269,   283,   284,
     286,   287,   294,   295,   302,   303,   309,   310,   315,   316,
     333,   334,   337,   338,   341,   342,   351,   353,   357,   358,
     367,   368,   374,   375,   380,   381,   384,   385,   388,   389,
     392,   393,   396,   397,   400,   401,   404,   405,   408,   409,
     412,   413,   416,   417,   420,   421,   424,   425,   428,   429,
     435,   436,   439,   440,   443,   444,   445,   446,   448,   449,
     450,   451,   452,   453,   454,   457,   460,   461,   462,   464,
     497,   498,   499,   504,   505,   513,   514,   270,     6,   177,
     129,   466,   154,   241,   239,   243,   244,     0,   195,   224,
     225,   241,   223,   223,     6,   177,   252,   223,   223,   275,
     278,   172,   174,   242,   271,    73,   223,   272,   281,   330,
     222,   272,   280,   282,   223,   241,   273,   172,   172,   204,
     204,   172,   204,   172,   172,   172,   220,   228,   233,   239,
     249,     9,    10,    11,    12,   172,   174,   176,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,   194,   219,
     204,   206,   181,   186,   187,   182,   183,    13,    14,    15,
      16,   188,   189,    17,    18,   180,   190,   191,    19,    20,
     192,   179,   193,   217,   221,   193,   172,   172,   172,    73,
     153,   258,   172,     6,   195,   195,   195,   220,    75,    76,
      78,    87,    88,    89,    90,    91,    92,   103,   105,   107,
     111,   121,   130,   139,   140,   148,   150,    75,    76,   103,
     148,   463,   105,   158,   163,   178,   262,   195,   241,   264,
     258,   285,   268,   177,   306,   285,   268,   268,   268,   285,
     352,   285,   268,   268,   268,   285,   285,   268,   268,   268,
     268,   268,   268,   268,   268,   285,   268,   306,   285,   285,
     285,   265,   285,     6,   517,   177,     6,   199,   237,   238,
      99,   121,   130,   172,   508,   173,   239,   243,   179,   195,
     194,   274,   277,   177,   178,   231,   232,   233,   177,   261,
     222,   279,     6,   173,   223,   245,   246,   247,   248,    34,
     175,   181,   218,   244,   129,   241,    73,   280,   366,   241,
     506,   173,   203,   218,   249,   249,   218,   249,   249,   173,
     233,   172,   174,   243,   250,   251,   233,   173,     6,   252,
     173,   203,   220,     6,   252,   218,   206,   206,   206,   207,
     207,   208,   208,   209,   209,   209,   209,   210,   210,   211,
     212,   213,   214,   215,   220,   218,   258,   193,    73,   258,
     259,   284,   220,   220,   220,    62,   195,   220,   222,   195,
     195,    76,   103,   288,   304,   196,   311,   196,   172,   196,
     447,   196,   112,   113,   114,   115,   455,   172,   196,   458,
     121,   296,   430,   196,   196,   317,   115,   131,   139,   354,
     140,   369,    75,   121,   376,   196,   463,   196,   515,   196,
     500,   263,    73,   285,   307,   308,   285,   172,   195,   237,
     194,   178,   179,   172,   331,   196,     6,   496,   509,   225,
     226,   261,   279,   231,   178,   232,   193,   195,   234,   235,
     241,   231,   222,   261,   172,   241,   243,   250,   173,   179,
     173,   179,   218,   244,   175,   175,    34,   175,   181,   218,
     138,   261,   173,   179,   173,   173,   179,   179,   179,   173,
     245,   250,   175,   181,   218,   251,   172,   174,   177,   206,
     173,   175,   193,   259,   173,   173,   173,   172,   195,   220,
     195,   195,   220,   441,   121,   437,    84,    93,    94,    96,
      97,    99,   100,   101,   104,   118,   179,   196,   289,   290,
     474,   475,   477,   481,   483,   485,   495,    77,    95,   179,
     196,   305,   475,   477,   479,   483,    77,   102,   179,   196,
     312,   313,   475,   477,     6,   196,   149,   456,   459,   196,
     335,    77,    78,    79,   108,   179,   196,   297,   298,   475,
     477,   479,   483,   487,   106,   109,   110,   144,   179,   196,
     431,   432,   474,   475,   477,   481,   485,   125,   126,   128,
     179,   196,   318,   319,   323,   325,   475,   479,   483,   487,
     132,   135,   136,   345,   359,   360,   361,   485,   343,   140,
     394,   133,   179,   196,   345,   347,   355,   356,   485,    75,
     121,   398,   141,   142,   179,   196,   370,   371,   474,   475,
     477,   481,   483,   103,   382,   143,   179,   196,   377,   378,
     475,   477,   487,   179,   196,   485,   106,   160,   161,   162,
     171,   179,   196,   516,   501,   178,    85,    73,   178,   308,
     285,   173,   203,   178,   179,   221,   178,   238,     6,    19,
      20,   116,   117,   180,   181,   182,   183,   190,   191,   471,
     122,   123,   124,   127,   179,   196,   320,   321,   323,   325,
     332,   173,   179,    99,   155,   156,   157,   179,   196,   510,
     177,   218,   253,   261,   178,   221,   179,   195,   193,   178,
      56,   247,     6,   175,   218,   218,   175,   175,   129,   218,
     249,     6,   249,   173,   173,   175,   175,   173,   245,   175,
     181,   218,   174,   176,   253,   254,   255,   256,   257,   217,
     258,   258,   258,   220,   173,   220,   195,   195,   220,   173,
     220,   195,   179,   196,   290,   442,   474,   475,   477,   479,
     481,   483,   339,   179,   196,   290,   298,   438,   474,   475,
     477,   479,   481,   483,   293,   476,   478,   482,   172,   172,
     292,   172,   172,   172,   289,   480,   305,   314,   312,   173,
     196,     6,   489,   179,   196,   297,   319,   336,   172,   172,
     297,   172,   172,   431,   324,   326,   172,   318,   172,   362,
     363,   179,   196,   360,   179,   196,   344,   345,   347,   485,
      75,   121,   406,   179,   196,   355,   370,   395,   172,   355,
     103,   402,   179,   196,   370,   378,   399,   487,   172,   172,
     370,   121,   386,   179,   196,   318,   378,   383,   477,   172,
     377,   485,   196,   172,   172,   516,   106,   164,   165,   166,
     171,   179,   196,   502,   196,   285,   173,   178,   193,   322,
     172,   332,   467,     6,   172,   172,   172,   172,   510,   254,
     235,   221,   175,   175,   130,   173,   173,   173,   173,   175,
     175,   221,     6,   252,   178,   179,   253,   194,   257,    60,
     173,   258,   173,   173,   220,   173,   220,   195,   258,   173,
     173,   220,   442,   179,   196,   319,   340,   438,   438,   172,
     172,   172,   172,    84,    96,    98,   471,   172,   291,   486,
      88,   119,   120,   172,   172,   173,   179,   336,     1,    80,
      81,    82,    83,    84,    86,   301,   220,   433,   145,   146,
     147,   434,   172,   172,   220,   346,   172,   172,   360,   344,
     103,   410,   179,   196,   355,   399,   407,   395,   134,   135,
     136,   137,   349,   350,   121,   414,   179,   196,   371,   383,
     403,   474,   481,   399,   372,   373,   390,   179,   196,   378,
     387,   438,   383,    80,   196,   181,   218,   218,   172,   172,
     172,   502,   228,   472,   172,   220,   196,   471,     6,   511,
     512,   511,   511,   178,   179,   196,   175,   178,   253,   255,
     258,   195,   258,   258,   173,   258,   173,   173,   220,   258,
     258,   173,   340,   489,   489,   489,   489,   173,   173,   173,
     484,   489,   220,   220,   173,   173,   173,   489,   489,     6,
     179,   173,   179,   173,   220,   193,   489,   489,   173,   220,
       6,   488,   490,   490,   121,   418,   179,   196,   355,   403,
     411,   407,   490,   193,   422,   179,   196,   371,   387,   415,
     403,   220,   220,   179,   196,   340,   378,   391,   387,   173,
     179,   173,   173,   173,   218,   218,   167,   168,   503,   179,
     193,   489,   173,   193,   174,   173,   179,   173,   173,   178,
     253,   258,   258,   258,   173,   258,   173,   173,   173,   173,
     193,   173,   173,   173,   173,   173,   300,   299,   173,   490,
     193,   327,   327,   173,   174,   491,   173,   179,   173,   426,
     179,   196,   356,   415,   419,   411,   173,   348,   179,   196,
     371,   391,   423,   415,   173,   173,   391,   379,   173,   173,
     179,   173,   228,   469,   173,   511,   192,   218,   512,   258,
     489,   220,   220,   173,   220,   173,   173,   493,   174,     6,
     488,   179,   196,   356,   423,   427,   419,   490,   423,   220,
     169,   170,   220,   173,     6,   175,   173,   173,   173,   220,
     494,   492,   427,   173,   173,   173,   175,   193,   175,   494,
     470,   220,   175,   151,   473,   172,   196,     6,   172,   194,
     203,   217,   173,   173,   173
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   197,   198,   198,   198,   198,   199,   200,   200,   201,
     201,   201,   201,   202,   202,   202,   202,   202,   202,   202,
     202,   202,   202,   202,   202,   202,   202,   203,   203,   204,
     204,   204,   204,   204,   204,   204,   204,   204,   205,   205,
     205,   205,   205,   205,   206,   206,   207,   207,   207,   207,
     208,   208,   208,   209,   209,   209,   210,   210,   210,   210,
     210,   211,   211,   211,   212,   212,   213,   213,   214,   214,
     215,   215,   216,   216,   217,   217,   218,   218,   219,   219,
     219,   219,   219,   219,   219,   219,   219,   219,   219,   220,
     220,   221,   222,   222,   222,   222,   222,   222,   223,   223,
     223,   223,   223,   223,   223,   223,   224,   224,   225,   226,
     225,   227,   227,   227,   227,   227,   228,   228,   228,   228,
     228,   228,   228,   228,   228,   228,   228,   228,   228,   228,
     228,   229,   229,   229,   229,   229,   229,   230,   230,   231,
     231,   232,   232,   233,   233,   233,   233,   234,   234,   235,
     235,   235,   236,   236,   236,   236,   236,   237,   237,   238,
     238,   239,   239,   239,   240,   241,   241,   242,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     242,   243,   243,   243,   243,   244,   244,   245,   245,   246,
     246,   247,   247,   247,   248,   248,   249,   249,   250,   250,
     250,   251,   251,   251,   251,   251,   251,   251,   251,   251,
     251,   251,   252,   253,   253,   253,   254,   254,   254,   254,
     255,   256,   256,   257,   257,   257,   258,   258,   258,   258,
     258,   258,   258,   258,   259,   259,   260,   260,   260,   261,
     262,   261,   263,   263,   264,   264,   264,   264,   265,   265,
     266,   266,   266,   267,   267,   267,   268,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   268,   269,   269,
     269,   269,   269,   270,   270,   271,   271,   271,   272,   272,
     274,   273,   275,   273,   277,   276,   278,   276,   279,   279,
     280,   280,   281,   282,   282,   283,   283,   283,   283,   283,
     283,   283,   283,   283,   283,   283,   283,   283,   283,   283,
     283,   283,   283,   283,   283,   283,   283,   283,   283,   283,
     283,   283,   283,   283,   283,   283,   283,   284,   284,   284,
     284,   284,   284,   285,   286,   287,   288,   288,   288,   289,
     289,   289,   289,   289,   289,   290,   291,   290,   292,   290,
     290,   293,   290,   294,   295,   296,   296,   296,   297,   297,
     297,   297,   297,   297,   298,   298,   299,   298,   300,   298,
     298,   301,   301,   301,   301,   301,   301,   302,   303,   304,
     304,   304,   305,   305,   305,   305,   305,   306,   307,   307,
     307,   308,   309,   310,   311,   311,   311,   312,   312,   312,
     312,   314,   313,   315,   316,   317,   317,   317,   318,   318,
     318,   318,   318,   319,   319,   319,   320,   320,   322,   321,
     324,   323,   326,   325,   327,   327,   328,   329,   329,   330,
     331,   331,   331,   332,   332,   332,   332,   332,   333,   334,
     335,   335,   335,   336,   336,   337,   338,   339,   339,   339,
     340,   340,   341,   342,   343,   343,   343,   344,   344,   344,
     346,   345,   348,   347,   349,   347,   350,   350,   350,   350,
     352,   351,   353,   354,   354,   354,   355,   355,   356,   356,
     357,   358,   359,   359,   359,   360,   360,   360,   362,   361,
     363,   361,   364,   365,   366,   367,   368,   369,   369,   369,
     370,   370,   370,   370,   370,   370,   372,   371,   373,   371,
     374,   375,   376,   376,   376,   377,   377,   377,   377,   378,
     379,   378,   380,   381,   382,   382,   382,   383,   383,   383,
     384,   385,   386,   386,   386,   387,   387,   388,   389,   390,
     390,   390,   391,   391,   392,   393,   394,   394,   394,   395,
     395,   396,   397,   398,   398,   398,   399,   399,   399,   400,
     401,   402,   402,   402,   403,   403,   403,   403,   404,   405,
     406,   406,   406,   407,   407,   408,   409,   410,   410,   410,
     411,   411,   412,   413,   414,   414,   414,   415,   415,   416,
     417,   418,   418,   418,   419,   419,   420,   421,   422,   422,
     422,   423,   423,   424,   425,   426,   426,   426,   427,   427,
     428,   429,   430,   430,   430,   431,   431,   431,   431,   431,
     432,   433,   432,   432,   432,   432,   434,   434,   434,   435,
     436,   437,   437,   437,   438,   438,   438,   438,   438,   438,
     438,   438,   439,   440,   441,   441,   441,   442,   442,   442,
     442,   442,   442,   442,   443,   444,   445,   446,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   455,   455,
     455,   455,   456,   456,   457,   457,   459,   458,   460,   461,
     462,   462,   462,   463,   463,   463,   463,   464,   466,   467,
     465,   469,   470,   468,   471,   471,   471,   471,   471,   471,
     471,   471,   471,   471,   471,   472,   472,   473,   473,   473,
     474,   474,   474,   476,   475,   478,   477,   480,   479,   482,
     481,   484,   483,   486,   485,   487,   488,   489,   489,   490,
     490,   490,   490,   492,   491,   493,   491,   494,   494,   495,
     495,   495,   496,   496,   497,   497,   498,   500,   499,   501,
     501,   501,   502,   502,   502,   502,   502,   503,   503,   503,
     503,   504,   504,   506,   505,   505,   508,   507,   509,   509,
     509,   510,   510,   510,   510,   511,   511,   512,   512,   512,
     513,   514,   515,   515,   515,   516,   516,   516,   516,   516,
     516,   517,   517
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     2,     2,     1,     1,     2,     1,
       1,     1,     3,     1,     4,     3,     4,     3,     4,     3,
       3,     3,     3,     2,     2,     6,     7,     1,     3,     1,
       2,     2,     2,     2,     4,     6,     6,     6,     1,     1,
       1,     1,     1,     1,     1,     4,     1,     3,     3,     3,
       1,     3,     3,     1,     3,     3,     1,     3,     3,     3,
       3,     1,     3,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     5,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     2,     3,     1,     1,     1,     1,     1,     2,
       1,     2,     1,     2,     1,     2,     1,     3,     1,     0,
       4,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     3,     5,     5,     2,     2,     1,     1,     1,
       2,     3,     2,     1,     2,     1,     2,     1,     3,     1,
       3,     2,     4,     5,     5,     6,     2,     1,     3,     1,
       3,     1,     1,     1,     1,     1,     2,     1,     3,     3,
       4,     4,     5,     5,     6,     6,     4,     5,     4,     3,
       4,     1,     2,     2,     3,     1,     2,     1,     3,     1,
       3,     2,     1,     2,     1,     3,     1,     2,     1,     1,
       2,     3,     2,     3,     3,     4,     3,     4,     2,     3,
       3,     4,     1,     1,     3,     4,     1,     2,     3,     4,
       2,     1,     2,     3,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     4,     3,     2,
       0,     4,     1,     2,     1,     1,     1,     1,     1,     2,
       5,     7,     5,     5,     7,     1,     6,     7,     7,     7,
       8,     8,     8,     9,     6,     7,     7,     8,     3,     2,
       2,     2,     3,     1,     2,     1,     1,     1,     1,     1,
       0,     4,     0,     3,     0,     5,     0,     4,     1,     2,
       1,     1,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     4,     0,     2,     3,     1,
       1,     1,     1,     1,     1,     1,     0,     5,     0,     5,
       1,     0,     5,     2,     4,     0,     2,     3,     1,     1,
       1,     1,     1,     1,     1,     4,     0,     7,     0,     7,
       1,     1,     1,     1,     1,     1,     1,     2,     4,     0,
       2,     3,     1,     1,     1,     1,     1,     3,     1,     2,
       3,     3,     2,     4,     0,     2,     3,     1,     1,     1,
       1,     0,     5,     2,     4,     0,     2,     3,     1,     1,
       1,     1,     1,     4,     1,     1,     1,     1,     0,     5,
       0,     6,     0,     6,     0,     2,     2,     1,     2,     5,
       0,     2,     3,     4,     1,     1,     1,     1,     2,     5,
       0,     2,     3,     1,     1,     2,     6,     0,     2,     3,
       1,     1,     2,     5,     0,     2,     3,     1,     1,     1,
       0,     5,     0,     7,     0,     5,     1,     1,     1,     1,
       0,     3,     4,     0,     2,     3,     1,     1,     1,     1,
       1,     5,     1,     2,     3,     1,     1,     1,     0,     5,
       0,     5,     3,     4,     5,     2,     4,     0,     2,     3,
       1,     1,     1,     1,     1,     1,     0,     5,     0,     5,
       2,     4,     0,     2,     3,     1,     1,     1,     1,     4,
       0,     7,     2,     5,     0,     2,     3,     1,     1,     1,
       2,     6,     0,     2,     3,     1,     1,     2,     7,     0,
       2,     3,     1,     1,     2,     5,     0,     2,     3,     1,
       1,     2,     5,     0,     2,     3,     1,     1,     1,     2,
       6,     0,     2,     3,     1,     1,     1,     1,     2,     6,
       0,     2,     3,     1,     1,     2,     7,     0,     2,     3,
       1,     1,     2,     7,     0,     2,     3,     1,     1,     2,
       8,     0,     2,     3,     1,     1,     2,     8,     0,     2,
       3,     1,     1,     2,     9,     0,     2,     3,     1,     1,
       2,     4,     0,     2,     3,     1,     1,     1,     1,     1,
       1,     0,     5,     1,     1,     6,     1,     1,     1,     2,
       5,     0,     2,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     5,     0,     2,     3,     1,     1,     1,
       1,     1,     1,     1,     2,     3,     2,     3,     4,     3,
       3,     3,     2,     3,     3,     2,     5,     0,     1,     1,
       1,     1,     0,     1,     3,     4,     0,     4,     2,     3,
       4,     5,     6,     1,     1,     1,     1,     3,     0,     0,
       7,     0,     0,    14,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     0,     6,     7,
       4,     4,     4,     0,     5,     0,     5,     0,     5,     0,
       5,     0,     7,     0,     5,     4,     2,     1,     3,     1,
       1,     3,     3,     0,     5,     0,     4,     3,     1,     4,
       4,     4,     1,     3,     1,     1,     3,     0,     5,     0,
       2,     3,     4,     4,     4,     1,     1,     1,     1,     3,
       3,     1,     1,     0,     4,     2,     0,     5,     0,     2,
       3,     4,     4,     4,     6,     1,     3,     1,     5,     4,
       3,     4,     0,     2,     3,     4,     4,     4,     1,     1,
       1,     3,     4
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
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
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



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

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
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
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
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
            /* Fall through.  */
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

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
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
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
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
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
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
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
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

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
| yyreduce -- Do a reduction.  |
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


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 453 "parser.y" /* yacc.c:1646  */
    { /* to avoid warnings */ }
#line 3428 "parser.c" /* yacc.c:1646  */
    break;

  case 3:
#line 454 "parser.y" /* yacc.c:1646  */
    { pastree_expr = (yyvsp[0].expr); }
#line 3434 "parser.c" /* yacc.c:1646  */
    break;

  case 4:
#line 455 "parser.y" /* yacc.c:1646  */
    { pastree_stmt = (yyvsp[0].stmt); }
#line 3440 "parser.c" /* yacc.c:1646  */
    break;

  case 5:
#line 456 "parser.y" /* yacc.c:1646  */
    { pastree_stmt = (yyvsp[0].stmt); }
#line 3446 "parser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 474 "parser.y" /* yacc.c:1646  */
    {
      symbol s = Symbol((yyvsp[0].name));
      if (checkDecls)
      {
        if ( symtab_get(stab, s, LABELNAME) )  /* NOT a type name */
          parse_error(-1, "enum symbol '%s' is already in use.", (yyvsp[0].name));
        symtab_put(stab, s, LABELNAME);
      }
      (yyval.symb) = s;
    }
#line 3461 "parser.c" /* yacc.c:1646  */
    break;

  case 7:
#line 495 "parser.y" /* yacc.c:1646  */
    {
      (yyval.string) = strdup((yyvsp[0].name));
    }
#line 3469 "parser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 499 "parser.y" /* yacc.c:1646  */
    {
      /* Or we could leave it as is (as a SpaceList) */
      if (((yyvsp[-1].string) = realloc((yyvsp[-1].string), strlen((yyvsp[-1].string)) + strlen((yyvsp[0].name)))) == NULL)
        parse_error(-1, "string out of memory\n");
      strcpy(((yyvsp[-1].string))+(strlen((yyvsp[-1].string))-1),((yyvsp[0].name))+1);  /* Catenate on the '"' */
      (yyval.string) = (yyvsp[-1].string);
    }
#line 3481 "parser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 522 "parser.y" /* yacc.c:1646  */
    {
      symbol  id = Symbol((yyvsp[0].name));
      stentry e;
      bool    chflag = false;

      if (checkDecls)
      {
        check_uknown_var((yyvsp[0].name));
        /* The parser constructs the original AST; this is the only
         * place it doesn't (actually there is one more place, when replacing
         * the "main" function): threadprivate variables are replaced on the
         * fly by pointers. This makes the job of later stages much smoother,
         * but the produced AST is semantically incorrect.
         */
        if ((e = symtab_get(stab, id, IDNAME)) != NULL) /* could be enum name */
          if (istp(e) && threadmode)
            chflag = true;
      }
      (yyval.expr) = chflag ? Parenthesis(UnaryOperator(UOP_star, Identifier(id)))
                  : Identifier(id);
    }
#line 3507 "parser.c" /* yacc.c:1646  */
    break;

  case 10:
#line 544 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Constant( strdup((yyvsp[0].name)) );
    }
#line 3515 "parser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 548 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = String((yyvsp[0].string));
    }
#line 3523 "parser.c" /* yacc.c:1646  */
    break;

  case 12:
#line 552 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Parenthesis((yyvsp[-1].expr));
    }
#line 3531 "parser.c" /* yacc.c:1646  */
    break;

  case 13:
#line 560 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 3539 "parser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 564 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = ArrayIndex((yyvsp[-3].expr), (yyvsp[-1].expr));
    }
#line 3547 "parser.c" /* yacc.c:1646  */
    break;

  case 15:
#line 574 "parser.y" /* yacc.c:1646  */
    {
      /* Catch calls to "main()" (unlikely but possible) */
      (yyval.expr) = strcmp((yyvsp[-2].name), "main") ?
             FunctionCall(IdentName((yyvsp[-2].name)), NULL) :
             FunctionCall(IdentName(MAIN_NEWNAME), NULL);
    }
#line 3558 "parser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 581 "parser.y" /* yacc.c:1646  */
    {
      /* Catch calls to "main()" (unlikely but possible) */
      (yyval.expr) = strcmp((yyvsp[-3].name), "main") ?
             FunctionCall(IdentName((yyvsp[-3].name)), (yyvsp[-1].expr)) :
             FunctionCall(IdentName(MAIN_NEWNAME), (yyvsp[-1].expr));
    }
#line 3569 "parser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 588 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = FunctionCall((yyvsp[-2].expr), NULL);
    }
#line 3577 "parser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 592 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = FunctionCall((yyvsp[-3].expr), (yyvsp[-1].expr));
    }
#line 3585 "parser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 596 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = DotField((yyvsp[-2].expr), Symbol((yyvsp[0].name)));
    }
#line 3593 "parser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 600 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = PtrField((yyvsp[-2].expr), Symbol((yyvsp[0].name)));
    }
#line 3601 "parser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 609 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = DotField((yyvsp[-2].expr), (yyvsp[0].symb));
    }
#line 3609 "parser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 613 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = PtrField((yyvsp[-2].expr), (yyvsp[0].symb));
    }
#line 3617 "parser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 617 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = PostOperator((yyvsp[-1].expr), UOP_inc);
    }
#line 3625 "parser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 621 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = PostOperator((yyvsp[-1].expr), UOP_dec);
    }
#line 3633 "parser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 625 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = CastedExpr((yyvsp[-4].decl), BracedInitializer((yyvsp[-1].expr)));
    }
#line 3641 "parser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 629 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = CastedExpr((yyvsp[-5].decl), BracedInitializer((yyvsp[-2].expr)));
    }
#line 3649 "parser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 637 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 3657 "parser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 641 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = CommaList((yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3665 "parser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 649 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 3673 "parser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 653 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = PreOperator((yyvsp[0].expr), UOP_inc);
    }
#line 3681 "parser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 657 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = PreOperator((yyvsp[0].expr), UOP_dec);
    }
#line 3689 "parser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 661 "parser.y" /* yacc.c:1646  */
    {
      if ((yyvsp[-1].type) == -1)
        (yyval.expr) = (yyvsp[0].expr);                    /* simplify */
      else
        (yyval.expr) = UnaryOperator((yyvsp[-1].type), (yyvsp[0].expr));
    }
#line 3700 "parser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 668 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Sizeof((yyvsp[0].expr));
    }
#line 3708 "parser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 672 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Sizeoftype((yyvsp[-1].decl));
    }
#line 3716 "parser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 681 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = FunctionCall(IdentName("__builtin_va_arg"),
                        CommaList((yyvsp[-3].expr), TypeTrick((yyvsp[-1].decl))));
    }
#line 3725 "parser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 686 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = FunctionCall(IdentName("__builtin_offsetof"),
                        CommaList(TypeTrick((yyvsp[-3].decl)), IdentName((yyvsp[-1].name))));
    }
#line 3734 "parser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 691 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = FunctionCall(IdentName("__builtin_types_compatible_p"),
                        CommaList(TypeTrick((yyvsp[-3].decl)), TypeTrick((yyvsp[-1].decl))));
    }
#line 3743 "parser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 700 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = UOP_addr;
    }
#line 3751 "parser.c" /* yacc.c:1646  */
    break;

  case 39:
#line 704 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = UOP_star;
    }
#line 3759 "parser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 708 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = -1;         /* Ingore this one */
    }
#line 3767 "parser.c" /* yacc.c:1646  */
    break;

  case 41:
#line 712 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = UOP_neg;
    }
#line 3775 "parser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 716 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = UOP_bnot;
    }
#line 3783 "parser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 720 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = UOP_lnot;
    }
#line 3791 "parser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 728 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 3799 "parser.c" /* yacc.c:1646  */
    break;

  case 45:
#line 732 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = CastedExpr((yyvsp[-2].decl), (yyvsp[0].expr));
    }
#line 3807 "parser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 740 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 3815 "parser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 744 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_mul, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3823 "parser.c" /* yacc.c:1646  */
    break;

  case 48:
#line 748 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_div, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3831 "parser.c" /* yacc.c:1646  */
    break;

  case 49:
#line 752 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_mod, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3839 "parser.c" /* yacc.c:1646  */
    break;

  case 50:
#line 760 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 3847 "parser.c" /* yacc.c:1646  */
    break;

  case 51:
#line 764 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_add, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3855 "parser.c" /* yacc.c:1646  */
    break;

  case 52:
#line 768 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_sub, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3863 "parser.c" /* yacc.c:1646  */
    break;

  case 53:
#line 776 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 3871 "parser.c" /* yacc.c:1646  */
    break;

  case 54:
#line 780 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_shl, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3879 "parser.c" /* yacc.c:1646  */
    break;

  case 55:
#line 784 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_shr, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3887 "parser.c" /* yacc.c:1646  */
    break;

  case 56:
#line 792 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 3895 "parser.c" /* yacc.c:1646  */
    break;

  case 57:
#line 796 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_lt, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3903 "parser.c" /* yacc.c:1646  */
    break;

  case 58:
#line 800 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_gt, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3911 "parser.c" /* yacc.c:1646  */
    break;

  case 59:
#line 804 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_leq, (yyvsp[-2].expr), (yyvsp[0].expr));
     }
#line 3919 "parser.c" /* yacc.c:1646  */
    break;

  case 60:
#line 808 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_geq, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3927 "parser.c" /* yacc.c:1646  */
    break;

  case 61:
#line 816 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 3935 "parser.c" /* yacc.c:1646  */
    break;

  case 62:
#line 820 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_eqeq, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3943 "parser.c" /* yacc.c:1646  */
    break;

  case 63:
#line 824 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_neq, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3951 "parser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 832 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 3959 "parser.c" /* yacc.c:1646  */
    break;

  case 65:
#line 836 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_band, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3967 "parser.c" /* yacc.c:1646  */
    break;

  case 66:
#line 844 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 3975 "parser.c" /* yacc.c:1646  */
    break;

  case 67:
#line 848 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_xor, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3983 "parser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 856 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 3991 "parser.c" /* yacc.c:1646  */
    break;

  case 69:
#line 860 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_bor, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3999 "parser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 868 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4007 "parser.c" /* yacc.c:1646  */
    break;

  case 71:
#line 872 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_land, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4015 "parser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 880 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4023 "parser.c" /* yacc.c:1646  */
    break;

  case 73:
#line 884 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_lor, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4031 "parser.c" /* yacc.c:1646  */
    break;

  case 74:
#line 892 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4039 "parser.c" /* yacc.c:1646  */
    break;

  case 75:
#line 896 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = ConditionalExpr((yyvsp[-4].expr), (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4047 "parser.c" /* yacc.c:1646  */
    break;

  case 76:
#line 904 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4055 "parser.c" /* yacc.c:1646  */
    break;

  case 77:
#line 908 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Assignment((yyvsp[-2].expr), (yyvsp[-1].type), (yyvsp[0].expr));
    }
#line 4063 "parser.c" /* yacc.c:1646  */
    break;

  case 78:
#line 916 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_eq;  /* Need fix here! */
    }
#line 4071 "parser.c" /* yacc.c:1646  */
    break;

  case 79:
#line 920 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_mul;
    }
#line 4079 "parser.c" /* yacc.c:1646  */
    break;

  case 80:
#line 924 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_div;
    }
#line 4087 "parser.c" /* yacc.c:1646  */
    break;

  case 81:
#line 928 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_mod;
    }
#line 4095 "parser.c" /* yacc.c:1646  */
    break;

  case 82:
#line 932 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_add;
    }
#line 4103 "parser.c" /* yacc.c:1646  */
    break;

  case 83:
#line 936 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_sub;
    }
#line 4111 "parser.c" /* yacc.c:1646  */
    break;

  case 84:
#line 940 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_shl;
    }
#line 4119 "parser.c" /* yacc.c:1646  */
    break;

  case 85:
#line 944 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_shr;
    }
#line 4127 "parser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 948 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_and;
    }
#line 4135 "parser.c" /* yacc.c:1646  */
    break;

  case 87:
#line 952 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_xor;
    }
#line 4143 "parser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 956 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_or;
    }
#line 4151 "parser.c" /* yacc.c:1646  */
    break;

  case 89:
#line 964 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4159 "parser.c" /* yacc.c:1646  */
    break;

  case 90:
#line 968 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = CommaList((yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4167 "parser.c" /* yacc.c:1646  */
    break;

  case 91:
#line 976 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4175 "parser.c" /* yacc.c:1646  */
    break;

  case 92:
#line 990 "parser.y" /* yacc.c:1646  */
    {
      /* There is a special case which wrongly uses this rule:
       *   typedef xxx already_known_user_type.
       * In this case the already_known_user_type (T) is re-defined,
       * and because T is known, it is not considered as a declarator,
       * but a "typedef_name", and is part of the specifier.
       * We fix it here.
       */
      if (isTypedef && (yyvsp[-1].spec)->type == SPECLIST)
        (yyval.stmt) = Declaration((yyvsp[-1].spec), fix_known_typename((yyvsp[-1].spec)));
      else
        (yyval.stmt) = Declaration((yyvsp[-1].spec), NULL);
      isTypedef = 0;
    }
#line 4194 "parser.c" /* yacc.c:1646  */
    break;

  case 93:
#line 1005 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Declaration((yyvsp[-2].spec), (yyvsp[-1].decl));
      if (checkDecls) add_declaration_links((yyvsp[-2].spec), (yyvsp[-1].decl));
      isTypedef = 0;

    }
#line 4205 "parser.c" /* yacc.c:1646  */
    break;

  case 94:
#line 1012 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpStmt(OmpConstruct(DCTHREADPRIVATE, (yyvsp[0].odir), NULL));
    }
#line 4213 "parser.c" /* yacc.c:1646  */
    break;

  case 95:
#line 1017 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpStmt(OmpConstruct(DCTHREADPRIVATE, $1, NULL)); TODO
    }
#line 4221 "parser.c" /* yacc.c:1646  */
    break;

  case 96:
#line 1021 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpStmt((yyvsp[0].ocon));
    }
#line 4229 "parser.c" /* yacc.c:1646  */
    break;

  case 97:
#line 1025 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpStmt(OmpConstruct(DCTHREADPRIVATE, $1, NULL)); TODO
    }
#line 4237 "parser.c" /* yacc.c:1646  */
    break;

  case 98:
#line 1033 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4245 "parser.c" /* yacc.c:1646  */
    break;

  case 99:
#line 1037 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right((yyvsp[-1].spec), (yyvsp[0].spec));
    }
#line 4253 "parser.c" /* yacc.c:1646  */
    break;

  case 100:
#line 1041 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4261 "parser.c" /* yacc.c:1646  */
    break;

  case 101:
#line 1045 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right((yyvsp[-1].spec), (yyvsp[0].spec));
    }
#line 4269 "parser.c" /* yacc.c:1646  */
    break;

  case 102:
#line 1049 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4277 "parser.c" /* yacc.c:1646  */
    break;

  case 103:
#line 1053 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right((yyvsp[-1].spec), (yyvsp[0].spec));
    }
#line 4285 "parser.c" /* yacc.c:1646  */
    break;

  case 104:
#line 1057 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4293 "parser.c" /* yacc.c:1646  */
    break;

  case 105:
#line 1061 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right((yyvsp[-1].spec), (yyvsp[0].spec));
    }
#line 4301 "parser.c" /* yacc.c:1646  */
    break;

  case 106:
#line 1069 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 4309 "parser.c" /* yacc.c:1646  */
    break;

  case 107:
#line 1073 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = DeclList((yyvsp[-2].decl), (yyvsp[0].decl));
    }
#line 4317 "parser.c" /* yacc.c:1646  */
    break;

  case 108:
#line 1087 "parser.y" /* yacc.c:1646  */
    {
      astdecl s = decl_getidentifier((yyvsp[0].decl));
      int     declkind = decl_getkind((yyvsp[0].decl));
      stentry e;

      if (!isTypedef && declkind == DFUNC && strcmp(s->u.id->name, "main") == 0)
        s->u.id = Symbol(MAIN_NEWNAME);       /* Catch main()'s declaration */
      if (checkDecls)
      {
        e = symtab_put(stab, s->u.id, (isTypedef) ? TYPENAME :
                                       (declkind == DFUNC) ? FUNCNAME : IDNAME);
        e->isarray = (declkind == DARRAY);
      }
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 4337 "parser.c" /* yacc.c:1646  */
    break;

  case 109:
#line 1103 "parser.y" /* yacc.c:1646  */
    {
      astdecl s = decl_getidentifier((yyvsp[-1].decl));
      int     declkind = decl_getkind((yyvsp[-1].decl));
      stentry e;

      if (!isTypedef && declkind == DFUNC && strcmp(s->u.id->name, "main") == 0)
        s->u.id = Symbol(MAIN_NEWNAME);         /* Catch main()'s declaration */
      if (checkDecls)
      {
        e = symtab_put(stab, s->u.id, (isTypedef) ? TYPENAME :
                                       (declkind == DFUNC) ? FUNCNAME : IDNAME);
        e->isarray = (declkind == DARRAY);
      }
    }
#line 4356 "parser.c" /* yacc.c:1646  */
    break;

  case 110:
#line 1118 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = InitDecl((yyvsp[-3].decl), (yyvsp[0].expr));
    }
#line 4364 "parser.c" /* yacc.c:1646  */
    break;

  case 111:
#line 1126 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = StClassSpec(SPEC_typedef);    /* Just a string */
      isTypedef = 1;
    }
#line 4373 "parser.c" /* yacc.c:1646  */
    break;

  case 112:
#line 1131 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = StClassSpec(SPEC_extern);
    }
#line 4381 "parser.c" /* yacc.c:1646  */
    break;

  case 113:
#line 1135 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = StClassSpec(SPEC_static);
    }
#line 4389 "parser.c" /* yacc.c:1646  */
    break;

  case 114:
#line 1139 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = StClassSpec(SPEC_auto);
    }
#line 4397 "parser.c" /* yacc.c:1646  */
    break;

  case 115:
#line 1143 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = StClassSpec(SPEC_register);
    }
#line 4405 "parser.c" /* yacc.c:1646  */
    break;

  case 116:
#line 1151 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_void);
    }
#line 4413 "parser.c" /* yacc.c:1646  */
    break;

  case 117:
#line 1155 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_char);
    }
#line 4421 "parser.c" /* yacc.c:1646  */
    break;

  case 118:
#line 1159 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_short);
    }
#line 4429 "parser.c" /* yacc.c:1646  */
    break;

  case 119:
#line 1163 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_int);
    }
#line 4437 "parser.c" /* yacc.c:1646  */
    break;

  case 120:
#line 1167 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_long);
    }
#line 4445 "parser.c" /* yacc.c:1646  */
    break;

  case 121:
#line 1171 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_float);
    }
#line 4453 "parser.c" /* yacc.c:1646  */
    break;

  case 122:
#line 1175 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_double);
    }
#line 4461 "parser.c" /* yacc.c:1646  */
    break;

  case 123:
#line 1179 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_signed);
    }
#line 4469 "parser.c" /* yacc.c:1646  */
    break;

  case 124:
#line 1183 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_unsigned);
    }
#line 4477 "parser.c" /* yacc.c:1646  */
    break;

  case 125:
#line 1187 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_ubool);
    }
#line 4485 "parser.c" /* yacc.c:1646  */
    break;

  case 126:
#line 1191 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_ucomplex);
    }
#line 4493 "parser.c" /* yacc.c:1646  */
    break;

  case 127:
#line 1195 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_uimaginary);
    }
#line 4501 "parser.c" /* yacc.c:1646  */
    break;

  case 128:
#line 1199 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4509 "parser.c" /* yacc.c:1646  */
    break;

  case 129:
#line 1203 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4517 "parser.c" /* yacc.c:1646  */
    break;

  case 130:
#line 1207 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Usertype((yyvsp[0].symb));
    }
#line 4525 "parser.c" /* yacc.c:1646  */
    break;

  case 131:
#line 1215 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = SUdecl((yyvsp[-3].type), NULL, (yyvsp[-1].decl));
    }
#line 4533 "parser.c" /* yacc.c:1646  */
    break;

  case 132:
#line 1219 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = SUdecl((yyvsp[-2].type), NULL, NULL);
    }
#line 4541 "parser.c" /* yacc.c:1646  */
    break;

  case 133:
#line 1223 "parser.y" /* yacc.c:1646  */
    {
      symbol s = Symbol((yyvsp[-3].name));
      /* Well, struct & union names have their own name space, and
       * their own scopes. I.e. they can be re-declare in nested
       * scopes. We don't do any kind of duplicate checks.
       */
      if (checkDecls)
        symtab_put(stab, s, SUNAME);
      (yyval.spec) = SUdecl((yyvsp[-4].type), s, (yyvsp[-1].decl));
    }
#line 4556 "parser.c" /* yacc.c:1646  */
    break;

  case 134:
#line 1238 "parser.y" /* yacc.c:1646  */
    {
      symbol s = (yyvsp[-3].symb);
      if (checkDecls)
        symtab_put(stab, s, SUNAME);
      (yyval.spec) = SUdecl((yyvsp[-4].type), s, (yyvsp[-1].decl));
    }
#line 4567 "parser.c" /* yacc.c:1646  */
    break;

  case 135:
#line 1245 "parser.y" /* yacc.c:1646  */
    {
      symbol s = Symbol((yyvsp[0].name));
      if (checkDecls)
        symtab_put(stab, s, SUNAME);
      (yyval.spec) = SUdecl((yyvsp[-1].type), s, NULL);
    }
#line 4578 "parser.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1252 "parser.y" /* yacc.c:1646  */
    {
      symbol s = (yyvsp[0].symb);
      if (checkDecls)
        symtab_put(stab, s, SUNAME);
      (yyval.spec) = SUdecl((yyvsp[-1].type), s, NULL);
    }
#line 4589 "parser.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1263 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = SPEC_struct;
    }
#line 4597 "parser.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1267 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = SPEC_union;
    }
#line 4605 "parser.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1275 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 4613 "parser.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1279 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = StructfieldList((yyvsp[-1].decl), (yyvsp[0].decl));
    }
#line 4621 "parser.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1287 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = StructfieldDecl((yyvsp[-2].spec), (yyvsp[-1].decl));
    }
#line 4629 "parser.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1291 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = StructfieldDecl((yyvsp[-1].spec), NULL);
    }
#line 4637 "parser.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1299 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4645 "parser.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1303 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right((yyvsp[-1].spec), (yyvsp[0].spec));
    }
#line 4653 "parser.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1307 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4661 "parser.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1311 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right((yyvsp[-1].spec), (yyvsp[0].spec));
    }
#line 4669 "parser.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1319 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 4677 "parser.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1323 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = DeclList((yyvsp[-2].decl), (yyvsp[0].decl));
    }
#line 4685 "parser.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1331 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 4693 "parser.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1335 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = BitDecl((yyvsp[-2].decl), (yyvsp[0].expr));
    }
#line 4701 "parser.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1339 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = BitDecl(NULL, (yyvsp[0].expr));
    }
#line 4709 "parser.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1347 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Enumdecl(NULL, (yyvsp[-1].spec));
    }
#line 4717 "parser.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1351 "parser.y" /* yacc.c:1646  */
    {
      symbol s = Symbol((yyvsp[-3].name));

      if (checkDecls)
      {
        if (symtab_get(stab, s, ENUMNAME))
          parse_error(-1, "enum name '%s' is already in use.", (yyvsp[-3].name));
        symtab_put(stab, s, ENUMNAME);
      }
      (yyval.spec) = Enumdecl(s, (yyvsp[-1].spec));
    }
#line 4733 "parser.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1363 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Enumdecl(NULL, (yyvsp[-2].spec));
    }
#line 4741 "parser.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1367 "parser.y" /* yacc.c:1646  */
    {
      symbol s = Symbol((yyvsp[-4].name));

      if (checkDecls)
      {
        if (symtab_get(stab, s, ENUMNAME))
          parse_error(-1, "enum name '%s' is already in use.", (yyvsp[-4].name));
        symtab_put(stab, s, ENUMNAME);
      }
      (yyval.spec) = Enumdecl(s, (yyvsp[-2].spec));
    }
#line 4757 "parser.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1379 "parser.y" /* yacc.c:1646  */
    {
      /*
      if (symtab_get(stab, s, ENUMNAME))
        parse_error(-1, "enum name '%s' is unknown.", $2);
      */
      (yyval.spec) = Enumdecl(Symbol((yyvsp[0].name)), NULL);
    }
#line 4769 "parser.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1391 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4777 "parser.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1395 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Enumbodylist((yyvsp[-2].spec), (yyvsp[0].spec));
    }
#line 4785 "parser.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1403 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Enumerator((yyvsp[0].symb), NULL);
    }
#line 4793 "parser.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1407 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Enumerator((yyvsp[-2].symb), (yyvsp[0].expr));
    }
#line 4801 "parser.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1415 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_const);
    }
#line 4809 "parser.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1419 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_restrict);
    }
#line 4817 "parser.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1423 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_volatile);
    }
#line 4825 "parser.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1431 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_inline);
    }
#line 4833 "parser.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1439 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = Declarator(NULL, (yyvsp[0].decl));
    }
#line 4841 "parser.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1443 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = Declarator((yyvsp[-1].spec), (yyvsp[0].decl));
    }
#line 4849 "parser.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1456 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = IdentifierDecl( Symbol((yyvsp[0].name)) );
    }
#line 4857 "parser.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1460 "parser.y" /* yacc.c:1646  */
    {
      /* Try to simplify a bit: (ident) -> ident */
      if ((yyvsp[-1].decl)->spec == NULL && (yyvsp[-1].decl)->decl->type == DIDENT)
        (yyval.decl) = (yyvsp[-1].decl)->decl;
      else
        (yyval.decl) = ParenDecl((yyvsp[-1].decl));
    }
#line 4869 "parser.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1468 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-2].decl), NULL, NULL);
    }
#line 4877 "parser.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1472 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-3].decl), (yyvsp[-1].spec), NULL);
    }
#line 4885 "parser.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1476 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-3].decl), NULL, (yyvsp[-1].expr));
    }
#line 4893 "parser.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1480 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-4].decl), (yyvsp[-2].spec), (yyvsp[-1].expr));
    }
#line 4901 "parser.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1484 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-4].decl), StClassSpec(SPEC_static), (yyvsp[-1].expr));
    }
#line 4909 "parser.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1488 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-5].decl), Speclist_right( StClassSpec(SPEC_static), (yyvsp[-2].spec) ), (yyvsp[-1].expr));
    }
#line 4917 "parser.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1492 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-5].decl), Speclist_left( (yyvsp[-3].spec), StClassSpec(SPEC_static) ), (yyvsp[-1].expr));
    }
#line 4925 "parser.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1496 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-3].decl), Declspec(SPEC_star), NULL);
    }
#line 4933 "parser.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1500 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-4].decl), Speclist_left( (yyvsp[-2].spec), Declspec(SPEC_star) ), NULL);
    }
#line 4941 "parser.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1504 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = FuncDecl((yyvsp[-3].decl), (yyvsp[-1].decl));
    }
#line 4949 "parser.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1508 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = FuncDecl((yyvsp[-2].decl), NULL);
    }
#line 4957 "parser.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1512 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = FuncDecl((yyvsp[-3].decl), (yyvsp[-1].decl));
    }
#line 4965 "parser.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1520 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Pointer();
    }
#line 4973 "parser.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1524 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right(Pointer(), (yyvsp[0].spec));
    }
#line 4981 "parser.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1528 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right(Pointer(), (yyvsp[0].spec));
    }
#line 4989 "parser.c" /* yacc.c:1646  */
    break;

  case 184:
#line 1532 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right( Pointer(), Speclist_left((yyvsp[-1].spec), (yyvsp[0].spec)) );
    }
#line 4997 "parser.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1540 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 5005 "parser.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1544 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_left((yyvsp[-1].spec), (yyvsp[0].spec));
    }
#line 5013 "parser.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1552 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 5021 "parser.c" /* yacc.c:1646  */
    break;

  case 188:
#line 1556 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ParamList((yyvsp[-2].decl), Ellipsis());
    }
#line 5029 "parser.c" /* yacc.c:1646  */
    break;

  case 189:
#line 1564 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 5037 "parser.c" /* yacc.c:1646  */
    break;

  case 190:
#line 1568 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ParamList((yyvsp[-2].decl), (yyvsp[0].decl));
    }
#line 5045 "parser.c" /* yacc.c:1646  */
    break;

  case 191:
#line 1576 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ParamDecl((yyvsp[-1].spec), (yyvsp[0].decl));
    }
#line 5053 "parser.c" /* yacc.c:1646  */
    break;

  case 192:
#line 1580 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ParamDecl((yyvsp[0].spec), NULL);
    }
#line 5061 "parser.c" /* yacc.c:1646  */
    break;

  case 193:
#line 1584 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ParamDecl((yyvsp[-1].spec), (yyvsp[0].decl));
    }
#line 5069 "parser.c" /* yacc.c:1646  */
    break;

  case 194:
#line 1592 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = IdentifierDecl( Symbol((yyvsp[0].name)) );
    }
#line 5077 "parser.c" /* yacc.c:1646  */
    break;

  case 195:
#line 1596 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = IdList((yyvsp[-2].decl), IdentifierDecl( Symbol((yyvsp[0].name)) ));
    }
#line 5085 "parser.c" /* yacc.c:1646  */
    break;

  case 196:
#line 1604 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = Casttypename((yyvsp[0].spec), NULL);
    }
#line 5093 "parser.c" /* yacc.c:1646  */
    break;

  case 197:
#line 1608 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = Casttypename((yyvsp[-1].spec), (yyvsp[0].decl));
    }
#line 5101 "parser.c" /* yacc.c:1646  */
    break;

  case 198:
#line 1616 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = AbstractDeclarator((yyvsp[0].spec), NULL);
    }
#line 5109 "parser.c" /* yacc.c:1646  */
    break;

  case 199:
#line 1620 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = AbstractDeclarator(NULL, (yyvsp[0].decl));
    }
#line 5117 "parser.c" /* yacc.c:1646  */
    break;

  case 200:
#line 1624 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = AbstractDeclarator((yyvsp[-1].spec), (yyvsp[0].decl));
    }
#line 5125 "parser.c" /* yacc.c:1646  */
    break;

  case 201:
#line 1632 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ParenDecl((yyvsp[-1].decl));
    }
#line 5133 "parser.c" /* yacc.c:1646  */
    break;

  case 202:
#line 1636 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl(NULL, NULL, NULL);
    }
#line 5141 "parser.c" /* yacc.c:1646  */
    break;

  case 203:
#line 1640 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-2].decl), NULL, NULL);
    }
#line 5149 "parser.c" /* yacc.c:1646  */
    break;

  case 204:
#line 1644 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl(NULL, NULL, (yyvsp[-1].expr));
    }
#line 5157 "parser.c" /* yacc.c:1646  */
    break;

  case 205:
#line 1648 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-3].decl), NULL, (yyvsp[-1].expr));
    }
#line 5165 "parser.c" /* yacc.c:1646  */
    break;

  case 206:
#line 1652 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl(NULL, Declspec(SPEC_star), NULL);
    }
#line 5173 "parser.c" /* yacc.c:1646  */
    break;

  case 207:
#line 1656 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-3].decl), Declspec(SPEC_star), NULL);
    }
#line 5181 "parser.c" /* yacc.c:1646  */
    break;

  case 208:
#line 1660 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = FuncDecl(NULL, NULL);
    }
#line 5189 "parser.c" /* yacc.c:1646  */
    break;

  case 209:
#line 1664 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = FuncDecl((yyvsp[-2].decl), NULL);
    }
#line 5197 "parser.c" /* yacc.c:1646  */
    break;

  case 210:
#line 1668 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = FuncDecl(NULL, (yyvsp[-1].decl));
    }
#line 5205 "parser.c" /* yacc.c:1646  */
    break;

  case 211:
#line 1672 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = FuncDecl((yyvsp[-3].decl), (yyvsp[-1].decl));
    }
#line 5213 "parser.c" /* yacc.c:1646  */
    break;

  case 212:
#line 1680 "parser.y" /* yacc.c:1646  */
    {
      (yyval.symb) = Symbol((yyvsp[0].name));
    }
#line 5221 "parser.c" /* yacc.c:1646  */
    break;

  case 213:
#line 1688 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 5229 "parser.c" /* yacc.c:1646  */
    break;

  case 214:
#line 1692 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BracedInitializer((yyvsp[-1].expr));
    }
#line 5237 "parser.c" /* yacc.c:1646  */
    break;

  case 215:
#line 1696 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BracedInitializer((yyvsp[-2].expr));
    }
#line 5245 "parser.c" /* yacc.c:1646  */
    break;

  case 216:
#line 1704 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 5253 "parser.c" /* yacc.c:1646  */
    break;

  case 217:
#line 1708 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Designated((yyvsp[-1].expr), (yyvsp[0].expr));
    }
#line 5261 "parser.c" /* yacc.c:1646  */
    break;

  case 218:
#line 1712 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = CommaList((yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 5269 "parser.c" /* yacc.c:1646  */
    break;

  case 219:
#line 1716 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = CommaList((yyvsp[-3].expr), Designated((yyvsp[-1].expr), (yyvsp[0].expr)));
    }
#line 5277 "parser.c" /* yacc.c:1646  */
    break;

  case 220:
#line 1724 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[-1].expr);
    }
#line 5285 "parser.c" /* yacc.c:1646  */
    break;

  case 221:
#line 1732 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 5293 "parser.c" /* yacc.c:1646  */
    break;

  case 222:
#line 1736 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = SpaceList((yyvsp[-1].expr), (yyvsp[0].expr));
    }
#line 5301 "parser.c" /* yacc.c:1646  */
    break;

  case 223:
#line 1744 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = IdxDesignator((yyvsp[-1].expr));
    }
#line 5309 "parser.c" /* yacc.c:1646  */
    break;

  case 224:
#line 1748 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = DotDesignator( Symbol((yyvsp[0].name)) );
    }
#line 5317 "parser.c" /* yacc.c:1646  */
    break;

  case 225:
#line 1752 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = DotDesignator((yyvsp[0].symb));
    }
#line 5325 "parser.c" /* yacc.c:1646  */
    break;

  case 226:
#line 1766 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5333 "parser.c" /* yacc.c:1646  */
    break;

  case 227:
#line 1770 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5341 "parser.c" /* yacc.c:1646  */
    break;

  case 228:
#line 1774 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5349 "parser.c" /* yacc.c:1646  */
    break;

  case 229:
#line 1778 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5357 "parser.c" /* yacc.c:1646  */
    break;

  case 230:
#line 1782 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5365 "parser.c" /* yacc.c:1646  */
    break;

  case 231:
#line 1786 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5373 "parser.c" /* yacc.c:1646  */
    break;

  case 232:
#line 1790 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpStmt((yyvsp[0].ocon));
      (yyval.stmt)->l = (yyvsp[0].ocon)->l;
    }
#line 5382 "parser.c" /* yacc.c:1646  */
    break;

  case 233:
#line 1795 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpixStmt((yyvsp[0].xcon));
      (yyval.stmt)->l = (yyvsp[0].xcon)->l;
    }
#line 5391 "parser.c" /* yacc.c:1646  */
    break;

  case 234:
#line 1804 "parser.y" /* yacc.c:1646  */
    { 
      (yyval.stmt) = (yyvsp[0].stmt); 
    }
#line 5399 "parser.c" /* yacc.c:1646  */
    break;

  case 235:
#line 1808 "parser.y" /* yacc.c:1646  */
    {       
      (yyval.stmt) = OmpStmt((yyvsp[0].ocon));
      (yyval.stmt)->l = (yyvsp[0].ocon)->l;
    }
#line 5408 "parser.c" /* yacc.c:1646  */
    break;

  case 236:
#line 1818 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Labeled( Symbol((yyvsp[-2].name)), (yyvsp[0].stmt) );
    }
#line 5416 "parser.c" /* yacc.c:1646  */
    break;

  case 237:
#line 1822 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Case((yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5424 "parser.c" /* yacc.c:1646  */
    break;

  case 238:
#line 1826 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Default((yyvsp[0].stmt));
    }
#line 5432 "parser.c" /* yacc.c:1646  */
    break;

  case 239:
#line 1834 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Compound(NULL);
    }
#line 5440 "parser.c" /* yacc.c:1646  */
    break;

  case 240:
#line 1837 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = sc_original_line()-1; scope_start(stab); }
#line 5446 "parser.c" /* yacc.c:1646  */
    break;

  case 241:
#line 1839 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Compound((yyvsp[-1].stmt));
      scope_end(stab);
      (yyval.stmt)->l = (yyvsp[-2].type);     /* Remember 1st line */
    }
#line 5456 "parser.c" /* yacc.c:1646  */
    break;

  case 242:
#line 1849 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5464 "parser.c" /* yacc.c:1646  */
    break;

  case 243:
#line 1853 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = BlockList((yyvsp[-1].stmt), (yyvsp[0].stmt));
      (yyval.stmt)->l = (yyvsp[-1].stmt)->l;
    }
#line 5473 "parser.c" /* yacc.c:1646  */
    break;

  case 244:
#line 1862 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5481 "parser.c" /* yacc.c:1646  */
    break;

  case 245:
#line 1866 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5489 "parser.c" /* yacc.c:1646  */
    break;

  case 246:
#line 1870 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpStmt((yyvsp[0].ocon));
      (yyval.stmt)->l = (yyvsp[0].ocon)->l;
    }
#line 5498 "parser.c" /* yacc.c:1646  */
    break;

  case 247:
#line 1875 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpixStmt((yyvsp[0].xcon));
      (yyval.stmt)->l = (yyvsp[0].xcon)->l;
    }
#line 5507 "parser.c" /* yacc.c:1646  */
    break;

  case 248:
#line 1884 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Expression(NULL);
    }
#line 5515 "parser.c" /* yacc.c:1646  */
    break;

  case 249:
#line 1888 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Expression((yyvsp[-1].expr));
      (yyval.stmt)->l = (yyvsp[-1].expr)->l;
    }
#line 5524 "parser.c" /* yacc.c:1646  */
    break;

  case 250:
#line 1897 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = If((yyvsp[-2].expr), (yyvsp[0].stmt), NULL);
    }
#line 5532 "parser.c" /* yacc.c:1646  */
    break;

  case 251:
#line 1901 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = If((yyvsp[-4].expr), (yyvsp[-2].stmt), (yyvsp[0].stmt));
    }
#line 5540 "parser.c" /* yacc.c:1646  */
    break;

  case 252:
#line 1905 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Switch((yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5548 "parser.c" /* yacc.c:1646  */
    break;

  case 253:
#line 1914 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = While((yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5556 "parser.c" /* yacc.c:1646  */
    break;

  case 254:
#line 1918 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Do((yyvsp[-5].stmt), (yyvsp[-2].expr));
    }
#line 5564 "parser.c" /* yacc.c:1646  */
    break;

  case 256:
#line 1926 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(NULL, NULL, NULL, (yyvsp[0].stmt));
    }
#line 5572 "parser.c" /* yacc.c:1646  */
    break;

  case 257:
#line 1930 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(Expression((yyvsp[-4].expr)), NULL, NULL, (yyvsp[0].stmt));
    }
#line 5580 "parser.c" /* yacc.c:1646  */
    break;

  case 258:
#line 1934 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(NULL, (yyvsp[-3].expr), NULL, (yyvsp[0].stmt));
    }
#line 5588 "parser.c" /* yacc.c:1646  */
    break;

  case 259:
#line 1938 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(NULL, NULL, (yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5596 "parser.c" /* yacc.c:1646  */
    break;

  case 260:
#line 1942 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(Expression((yyvsp[-5].expr)), (yyvsp[-3].expr), NULL, (yyvsp[0].stmt));
    }
#line 5604 "parser.c" /* yacc.c:1646  */
    break;

  case 261:
#line 1946 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(Expression((yyvsp[-5].expr)), NULL, (yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5612 "parser.c" /* yacc.c:1646  */
    break;

  case 262:
#line 1950 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(NULL, (yyvsp[-4].expr), (yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5620 "parser.c" /* yacc.c:1646  */
    break;

  case 263:
#line 1954 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(Expression((yyvsp[-6].expr)), (yyvsp[-4].expr), (yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5628 "parser.c" /* yacc.c:1646  */
    break;

  case 264:
#line 1958 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For((yyvsp[-3].stmt), NULL, NULL, (yyvsp[0].stmt));
    }
#line 5636 "parser.c" /* yacc.c:1646  */
    break;

  case 265:
#line 1962 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For((yyvsp[-4].stmt), (yyvsp[-3].expr), NULL, (yyvsp[0].stmt));
    }
#line 5644 "parser.c" /* yacc.c:1646  */
    break;

  case 266:
#line 1966 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For((yyvsp[-4].stmt), NULL, (yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5652 "parser.c" /* yacc.c:1646  */
    break;

  case 267:
#line 1970 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For((yyvsp[-5].stmt), (yyvsp[-4].expr), (yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5660 "parser.c" /* yacc.c:1646  */
    break;

  case 268:
#line 1978 "parser.y" /* yacc.c:1646  */
    {
      /* We don't keep track of labels -- we leave it to the native compiler */
      (yyval.stmt) = Goto( Symbol((yyvsp[-1].name)) );
    }
#line 5669 "parser.c" /* yacc.c:1646  */
    break;

  case 269:
#line 1983 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Continue();
    }
#line 5677 "parser.c" /* yacc.c:1646  */
    break;

  case 270:
#line 1987 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Break();
    }
#line 5685 "parser.c" /* yacc.c:1646  */
    break;

  case 271:
#line 1991 "parser.y" /* yacc.c:1646  */
    {
      //TODO: simple hack, not 100% correct, does not cover goto
      if (errorOnReturn)
        parse_error(1, "return statement not allowed in an outlined region\n");
      (yyval.stmt) = Return(NULL);
    }
#line 5696 "parser.c" /* yacc.c:1646  */
    break;

  case 272:
#line 1998 "parser.y" /* yacc.c:1646  */
    {
      //TODO: simple hack, not 100% correct, does not cover goto
      if (errorOnReturn)
        parse_error(1, "return statement not allowed in an outlined region\n");
      (yyval.stmt) = Return((yyvsp[-1].expr));
    }
#line 5707 "parser.c" /* yacc.c:1646  */
    break;

  case 273:
#line 2015 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = pastree = (yyvsp[0].stmt);
    }
#line 5715 "parser.c" /* yacc.c:1646  */
    break;

  case 274:
#line 2019 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = pastree = BlockList((yyvsp[-1].stmt), (yyvsp[0].stmt));
    }
#line 5723 "parser.c" /* yacc.c:1646  */
    break;

  case 275:
#line 2027 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5731 "parser.c" /* yacc.c:1646  */
    break;

  case 276:
#line 2031 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5739 "parser.c" /* yacc.c:1646  */
    break;

  case 277:
#line 2038 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpixStmt((yyvsp[0].xcon));
    }
#line 5747 "parser.c" /* yacc.c:1646  */
    break;

  case 278:
#line 2049 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt) = (yyvsp[0].stmt); }
#line 5753 "parser.c" /* yacc.c:1646  */
    break;

  case 279:
#line 2050 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt) = (yyvsp[0].stmt); }
#line 5759 "parser.c" /* yacc.c:1646  */
    break;

  case 280:
#line 2055 "parser.y" /* yacc.c:1646  */
    {
      if (isTypedef || (yyvsp[0].decl)->decl->type != DFUNC)
        parse_error(1, "function definition cannot be parsed.\n");
      if (symtab_get(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME) == NULL)
        symtab_put(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME);

      scope_start(stab);
      ast_declare_function_params((yyvsp[0].decl));
    }
#line 5773 "parser.c" /* yacc.c:1646  */
    break;

  case 281:
#line 2065 "parser.y" /* yacc.c:1646  */
    {
      scope_end(stab);
      check_for_main_and_declare((yyvsp[-3].spec), (yyvsp[-2].decl));
      (yyval.stmt) = FuncDef((yyvsp[-3].spec), (yyvsp[-2].decl), NULL, (yyvsp[0].stmt));
    }
#line 5783 "parser.c" /* yacc.c:1646  */
    break;

  case 282:
#line 2071 "parser.y" /* yacc.c:1646  */
    {
      if (isTypedef || (yyvsp[0].decl)->decl->type != DFUNC)
        parse_error(1, "function definition cannot be parsed.\n");
      if (symtab_get(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME) == NULL)
        symtab_put(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME);

      scope_start(stab);
      ast_declare_function_params((yyvsp[0].decl));
    }
#line 5797 "parser.c" /* yacc.c:1646  */
    break;

  case 283:
#line 2081 "parser.y" /* yacc.c:1646  */
    {
      astspec s = Declspec(SPEC_int);  /* return type defaults to "int" */

      scope_end(stab);
      check_for_main_and_declare(s, (yyvsp[-2].decl));
      (yyval.stmt) = FuncDef(s, (yyvsp[-2].decl), NULL, (yyvsp[0].stmt));
    }
#line 5809 "parser.c" /* yacc.c:1646  */
    break;

  case 284:
#line 2092 "parser.y" /* yacc.c:1646  */
    {
      if (isTypedef || (yyvsp[0].decl)->decl->type != DFUNC)
        parse_error(1, "function definition cannot be parsed.\n");
      if (symtab_get(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME) == NULL)
        symtab_put(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME);

      scope_start(stab);
      /* Notice here that the function parameters are declared through
       * the declaration_list and we need to do nothing else!
       */
    }
#line 5825 "parser.c" /* yacc.c:1646  */
    break;

  case 285:
#line 2104 "parser.y" /* yacc.c:1646  */
    {
      scope_end(stab);
      check_for_main_and_declare((yyvsp[-4].spec), (yyvsp[-3].decl));
      (yyval.stmt) = FuncDef((yyvsp[-4].spec), (yyvsp[-3].decl), (yyvsp[-1].stmt), (yyvsp[0].stmt));
    }
#line 5835 "parser.c" /* yacc.c:1646  */
    break;

  case 286:
#line 2110 "parser.y" /* yacc.c:1646  */
    {
      if (isTypedef || (yyvsp[0].decl)->decl->type != DFUNC)
        parse_error(1, "function definition cannot be parsed.\n");
      if (symtab_get(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME) == NULL)
        symtab_put(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME);

      scope_start(stab);
      /* Notice here that the function parameters are declared through
       * the declaration_list and we need to do nothing else!
       */
    }
#line 5851 "parser.c" /* yacc.c:1646  */
    break;

  case 287:
#line 2122 "parser.y" /* yacc.c:1646  */
    {
      astspec s = Declspec(SPEC_int);  /* return type defaults to "int" */

      scope_end(stab);
      check_for_main_and_declare(s, (yyvsp[-3].decl));
      (yyval.stmt) = FuncDef(s, (yyvsp[-3].decl), (yyvsp[-1].stmt), (yyvsp[0].stmt));
    }
#line 5863 "parser.c" /* yacc.c:1646  */
    break;

  case 288:
#line 2134 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5871 "parser.c" /* yacc.c:1646  */
    break;

  case 289:
#line 2138 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = BlockList((yyvsp[-1].stmt), (yyvsp[0].stmt));         /* Same as block list */
    }
#line 5879 "parser.c" /* yacc.c:1646  */
    break;

  case 290:
#line 2153 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5887 "parser.c" /* yacc.c:1646  */
    break;

  case 291:
#line 2157 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5895 "parser.c" /* yacc.c:1646  */
    break;

  case 292:
#line 2165 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5903 "parser.c" /* yacc.c:1646  */
    break;

  case 293:
#line 2174 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5911 "parser.c" /* yacc.c:1646  */
    break;

  case 294:
#line 2178 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = pastree = BlockList((yyvsp[-1].stmt), (yyvsp[0].stmt));
    }
#line 5919 "parser.c" /* yacc.c:1646  */
    break;

  case 295:
#line 2185 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 5927 "parser.c" /* yacc.c:1646  */
    break;

  case 296:
#line 2189 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 5935 "parser.c" /* yacc.c:1646  */
    break;

  case 297:
#line 2193 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 5943 "parser.c" /* yacc.c:1646  */
    break;

  case 298:
#line 2197 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 5951 "parser.c" /* yacc.c:1646  */
    break;

  case 299:
#line 2201 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 5959 "parser.c" /* yacc.c:1646  */
    break;

  case 300:
#line 2205 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 5967 "parser.c" /* yacc.c:1646  */
    break;

  case 301:
#line 2209 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 5975 "parser.c" /* yacc.c:1646  */
    break;

  case 302:
#line 2213 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 5983 "parser.c" /* yacc.c:1646  */
    break;

  case 303:
#line 2217 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 5991 "parser.c" /* yacc.c:1646  */
    break;

  case 304:
#line 2221 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 5999 "parser.c" /* yacc.c:1646  */
    break;

  case 305:
#line 2226 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6007 "parser.c" /* yacc.c:1646  */
    break;

  case 306:
#line 2231 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6015 "parser.c" /* yacc.c:1646  */
    break;

  case 307:
#line 2235 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6023 "parser.c" /* yacc.c:1646  */
    break;

  case 308:
#line 2239 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6031 "parser.c" /* yacc.c:1646  */
    break;

  case 309:
#line 2243 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6039 "parser.c" /* yacc.c:1646  */
    break;

  case 310:
#line 2247 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6047 "parser.c" /* yacc.c:1646  */
    break;

  case 311:
#line 2251 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6055 "parser.c" /* yacc.c:1646  */
    break;

  case 312:
#line 2255 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6063 "parser.c" /* yacc.c:1646  */
    break;

  case 313:
#line 2259 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6071 "parser.c" /* yacc.c:1646  */
    break;

  case 314:
#line 2263 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6079 "parser.c" /* yacc.c:1646  */
    break;

  case 315:
#line 2267 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6087 "parser.c" /* yacc.c:1646  */
    break;

  case 316:
#line 2271 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6095 "parser.c" /* yacc.c:1646  */
    break;

  case 317:
#line 2275 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6103 "parser.c" /* yacc.c:1646  */
    break;

  case 318:
#line 2279 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6111 "parser.c" /* yacc.c:1646  */
    break;

  case 319:
#line 2283 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6119 "parser.c" /* yacc.c:1646  */
    break;

  case 320:
#line 2287 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6127 "parser.c" /* yacc.c:1646  */
    break;

  case 321:
#line 2291 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6135 "parser.c" /* yacc.c:1646  */
    break;

  case 322:
#line 2295 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6143 "parser.c" /* yacc.c:1646  */
    break;

  case 323:
#line 2299 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6151 "parser.c" /* yacc.c:1646  */
    break;

  case 324:
#line 2303 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6159 "parser.c" /* yacc.c:1646  */
    break;

  case 325:
#line 2307 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6167 "parser.c" /* yacc.c:1646  */
    break;

  case 326:
#line 2312 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6175 "parser.c" /* yacc.c:1646  */
    break;

  case 327:
#line 2328 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCBARRIER, (yyvsp[0].odir), NULL);
    }
#line 6183 "parser.c" /* yacc.c:1646  */
    break;

  case 328:
#line 2332 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCFLUSH, (yyvsp[0].odir), NULL);
    }
#line 6191 "parser.c" /* yacc.c:1646  */
    break;

  case 329:
#line 2337 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTASKWAIT, (yyvsp[0].odir), NULL);
    }
#line 6199 "parser.c" /* yacc.c:1646  */
    break;

  case 330:
#line 2342 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTASKYIELD, (yyvsp[0].odir), NULL);
    }
#line 6207 "parser.c" /* yacc.c:1646  */
    break;

  case 331:
#line 2347 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCCANCEL, (yyvsp[0].odir), NULL);
    }
#line 6215 "parser.c" /* yacc.c:1646  */
    break;

  case 332:
#line 2352 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCCANCELLATIONPOINT, (yyvsp[0].odir), NULL);
    }
#line 6223 "parser.c" /* yacc.c:1646  */
    break;

  case 333:
#line 2359 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 6231 "parser.c" /* yacc.c:1646  */
    break;

  case 334:
#line 2366 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCPARALLEL, (yyvsp[-1].odir), (yyvsp[0].stmt));
      (yyval.ocon)->l = (yyvsp[-1].odir)->l;
    }
#line 6240 "parser.c" /* yacc.c:1646  */
    break;

  case 335:
#line 2374 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCPARALLEL, (yyvsp[-1].ocla));
    }
#line 6248 "parser.c" /* yacc.c:1646  */
    break;

  case 336:
#line 2381 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 6256 "parser.c" /* yacc.c:1646  */
    break;

  case 337:
#line 2385 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 6264 "parser.c" /* yacc.c:1646  */
    break;

  case 338:
#line 2389 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 6272 "parser.c" /* yacc.c:1646  */
    break;

  case 339:
#line 2396 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6280 "parser.c" /* yacc.c:1646  */
    break;

  case 340:
#line 2400 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6288 "parser.c" /* yacc.c:1646  */
    break;

  case 341:
#line 2404 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6296 "parser.c" /* yacc.c:1646  */
    break;

  case 342:
#line 2408 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6304 "parser.c" /* yacc.c:1646  */
    break;

  case 343:
#line 2412 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6312 "parser.c" /* yacc.c:1646  */
    break;

  case 344:
#line 2416 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6320 "parser.c" /* yacc.c:1646  */
    break;

  case 345:
#line 2423 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6328 "parser.c" /* yacc.c:1646  */
    break;

  case 346:
#line 2426 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 6334 "parser.c" /* yacc.c:1646  */
    break;

  case 347:
#line 2427 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = NumthreadsClause((yyvsp[-1].expr));
    }
#line 6343 "parser.c" /* yacc.c:1646  */
    break;

  case 348:
#line 2431 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 6349 "parser.c" /* yacc.c:1646  */
    break;

  case 349:
#line 2432 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCCOPYIN, (yyvsp[-1].decl));
    }
#line 6358 "parser.c" /* yacc.c:1646  */
    break;

  case 350:
#line 2438 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6366 "parser.c" /* yacc.c:1646  */
    break;

  case 351:
#line 2442 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 6372 "parser.c" /* yacc.c:1646  */
    break;

  case 352:
#line 2443 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCAUTO, (yyvsp[-1].decl));
    }
#line 6381 "parser.c" /* yacc.c:1646  */
    break;

  case 353:
#line 2451 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCFOR, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 6389 "parser.c" /* yacc.c:1646  */
    break;

  case 354:
#line 2458 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCFOR, (yyvsp[-1].ocla));
    }
#line 6397 "parser.c" /* yacc.c:1646  */
    break;

  case 355:
#line 2465 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 6405 "parser.c" /* yacc.c:1646  */
    break;

  case 356:
#line 2469 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 6413 "parser.c" /* yacc.c:1646  */
    break;

  case 357:
#line 2473 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 6421 "parser.c" /* yacc.c:1646  */
    break;

  case 358:
#line 2480 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6429 "parser.c" /* yacc.c:1646  */
    break;

  case 359:
#line 2484 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6437 "parser.c" /* yacc.c:1646  */
    break;

  case 360:
#line 2488 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6445 "parser.c" /* yacc.c:1646  */
    break;

  case 361:
#line 2492 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6453 "parser.c" /* yacc.c:1646  */
    break;

  case 362:
#line 2496 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6461 "parser.c" /* yacc.c:1646  */
    break;

  case 363:
#line 2500 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCNOWAIT);
    }
#line 6469 "parser.c" /* yacc.c:1646  */
    break;

  case 364:
#line 2507 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCORDERED);
    }
#line 6477 "parser.c" /* yacc.c:1646  */
    break;

  case 365:
#line 2511 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = ScheduleClause((yyvsp[-1].type), NULL);
    }
#line 6485 "parser.c" /* yacc.c:1646  */
    break;

  case 366:
#line 2514 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 6491 "parser.c" /* yacc.c:1646  */
    break;

  case 367:
#line 2515 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      if ((yyvsp[-4].type) == OC_runtime)
        parse_error(1, "\"runtime\" schedules may not have a chunksize.\n");
      (yyval.ocla) = ScheduleClause((yyvsp[-4].type), (yyvsp[-1].expr));
    }
#line 6502 "parser.c" /* yacc.c:1646  */
    break;

  case 368:
#line 2522 "parser.y" /* yacc.c:1646  */
    {  /* non-OpenMP schedule */
      tempsave = checkDecls;
      checkDecls = 0;   /* Because the index of the loop is usualy involved */
      sc_pause_openmp();
    }
#line 6512 "parser.c" /* yacc.c:1646  */
    break;

  case 369:
#line 2528 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      checkDecls = tempsave;
      (yyval.ocla) = ScheduleClause(OC_affinity, (yyvsp[-1].expr));
    }
#line 6522 "parser.c" /* yacc.c:1646  */
    break;

  case 370:
#line 2534 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6530 "parser.c" /* yacc.c:1646  */
    break;

  case 371:
#line 2541 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_static;
    }
#line 6538 "parser.c" /* yacc.c:1646  */
    break;

  case 372:
#line 2545 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_dynamic;
    }
#line 6546 "parser.c" /* yacc.c:1646  */
    break;

  case 373:
#line 2549 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_guided;
    }
#line 6554 "parser.c" /* yacc.c:1646  */
    break;

  case 374:
#line 2553 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_runtime;
    }
#line 6562 "parser.c" /* yacc.c:1646  */
    break;

  case 375:
#line 2557 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_auto;
    }
#line 6570 "parser.c" /* yacc.c:1646  */
    break;

  case 376:
#line 2560 "parser.y" /* yacc.c:1646  */
    { parse_error(1, "invalid openmp schedule type.\n"); }
#line 6576 "parser.c" /* yacc.c:1646  */
    break;

  case 377:
#line 2566 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCSECTIONS, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 6584 "parser.c" /* yacc.c:1646  */
    break;

  case 378:
#line 2573 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCSECTIONS, (yyvsp[-1].ocla));
    }
#line 6592 "parser.c" /* yacc.c:1646  */
    break;

  case 379:
#line 2580 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 6600 "parser.c" /* yacc.c:1646  */
    break;

  case 380:
#line 2584 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 6608 "parser.c" /* yacc.c:1646  */
    break;

  case 381:
#line 2588 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 6616 "parser.c" /* yacc.c:1646  */
    break;

  case 382:
#line 2595 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6624 "parser.c" /* yacc.c:1646  */
    break;

  case 383:
#line 2599 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6632 "parser.c" /* yacc.c:1646  */
    break;

  case 384:
#line 2603 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6640 "parser.c" /* yacc.c:1646  */
    break;

  case 385:
#line 2607 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6648 "parser.c" /* yacc.c:1646  */
    break;

  case 386:
#line 2611 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCNOWAIT);
    }
#line 6656 "parser.c" /* yacc.c:1646  */
    break;

  case 387:
#line 2618 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Compound((yyvsp[-1].stmt));
    }
#line 6664 "parser.c" /* yacc.c:1646  */
    break;

  case 388:
#line 2625 "parser.y" /* yacc.c:1646  */
    {
      /* Make it look like it had a section pragma */
      (yyval.stmt) = OmpStmt( OmpConstruct(DCSECTION, OmpDirective(DCSECTION,NULL), (yyvsp[0].stmt)) );
    }
#line 6673 "parser.c" /* yacc.c:1646  */
    break;

  case 389:
#line 2630 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpStmt( OmpConstruct(DCSECTION, (yyvsp[-1].odir), (yyvsp[0].stmt)) );
    }
#line 6681 "parser.c" /* yacc.c:1646  */
    break;

  case 390:
#line 2634 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = BlockList((yyvsp[-2].stmt), OmpStmt( OmpConstruct(DCSECTION, (yyvsp[-1].odir), (yyvsp[0].stmt)) ));
    }
#line 6689 "parser.c" /* yacc.c:1646  */
    break;

  case 391:
#line 2641 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCSECTION, NULL);
    }
#line 6697 "parser.c" /* yacc.c:1646  */
    break;

  case 392:
#line 2648 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCSINGLE, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 6705 "parser.c" /* yacc.c:1646  */
    break;

  case 393:
#line 2655 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCSINGLE, (yyvsp[-1].ocla));
    }
#line 6713 "parser.c" /* yacc.c:1646  */
    break;

  case 394:
#line 2662 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 6721 "parser.c" /* yacc.c:1646  */
    break;

  case 395:
#line 2666 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 6729 "parser.c" /* yacc.c:1646  */
    break;

  case 396:
#line 2670 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 6737 "parser.c" /* yacc.c:1646  */
    break;

  case 397:
#line 2677 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6745 "parser.c" /* yacc.c:1646  */
    break;

  case 398:
#line 2681 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6753 "parser.c" /* yacc.c:1646  */
    break;

  case 399:
#line 2685 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6761 "parser.c" /* yacc.c:1646  */
    break;

  case 400:
#line 2689 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCNOWAIT);
    }
#line 6769 "parser.c" /* yacc.c:1646  */
    break;

  case 401:
#line 2695 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 6775 "parser.c" /* yacc.c:1646  */
    break;

  case 402:
#line 2696 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCCOPYPRIVATE, (yyvsp[-1].decl));
    }
#line 6784 "parser.c" /* yacc.c:1646  */
    break;

  case 403:
#line 2705 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCSIMD, $1, $2); TODO DCSIMD
    }
#line 6792 "parser.c" /* yacc.c:1646  */
    break;

  case 404:
#line 2713 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCSIMD, $3); TODO DCSIMD
    }
#line 6800 "parser.c" /* yacc.c:1646  */
    break;

  case 405:
#line 2720 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 6808 "parser.c" /* yacc.c:1646  */
    break;

  case 406:
#line 2724 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 6816 "parser.c" /* yacc.c:1646  */
    break;

  case 407:
#line 2728 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 6824 "parser.c" /* yacc.c:1646  */
    break;

  case 408:
#line 2736 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6832 "parser.c" /* yacc.c:1646  */
    break;

  case 409:
#line 2740 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6840 "parser.c" /* yacc.c:1646  */
    break;

  case 410:
#line 2744 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6848 "parser.c" /* yacc.c:1646  */
    break;

  case 411:
#line 2748 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6856 "parser.c" /* yacc.c:1646  */
    break;

  case 412:
#line 2752 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6864 "parser.c" /* yacc.c:1646  */
    break;

  case 413:
#line 2760 "parser.y" /* yacc.c:1646  */
    {
      int n = 0, er = 0;
      if (xar_expr_is_constant((yyvsp[-1].expr)))
      {
        n = xar_calc_int_expr((yyvsp[-1].expr), &er);
        if (er) n = 0;
      }
      if (n <= 0)
        parse_error(1, "invalid number in simdlen() clause.\n");
      //$$ = CollapseClause(n); //TODO SAFELEN
    }
#line 6880 "parser.c" /* yacc.c:1646  */
    break;

  case 414:
#line 2772 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6888 "parser.c" /* yacc.c:1646  */
    break;

  case 415:
#line 2776 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 6896 "parser.c" /* yacc.c:1646  */
    break;

  case 416:
#line 2784 "parser.y" /* yacc.c:1646  */
    {
      //$$ = PlainClause(OCINBRANCH); TODO ast
    }
#line 6904 "parser.c" /* yacc.c:1646  */
    break;

  case 417:
#line 2788 "parser.y" /* yacc.c:1646  */
    {
      //$$ = PlainClause(OCNOTINBRANCH); TODO ast
    }
#line 6912 "parser.c" /* yacc.c:1646  */
    break;

  case 418:
#line 2795 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 6918 "parser.c" /* yacc.c:1646  */
    break;

  case 419:
#line 2796 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      //$$ = VarlistClause(OCUNIFORM, $4); TODO ast
    }
#line 6927 "parser.c" /* yacc.c:1646  */
    break;

  case 420:
#line 2804 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 6933 "parser.c" /* yacc.c:1646  */
    break;

  case 421:
#line 2805 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      // TODO ast
    }
#line 6942 "parser.c" /* yacc.c:1646  */
    break;

  case 422:
#line 2813 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 6948 "parser.c" /* yacc.c:1646  */
    break;

  case 423:
#line 2814 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      // TODO ast
    }
#line 6957 "parser.c" /* yacc.c:1646  */
    break;

  case 424:
#line 2822 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = NULL;
    }
#line 6965 "parser.c" /* yacc.c:1646  */
    break;

  case 425:
#line 2826 "parser.y" /* yacc.c:1646  */
    {
      // TODO ast
    }
#line 6973 "parser.c" /* yacc.c:1646  */
    break;

  case 426:
#line 2834 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCDECLSIMD, $1, $2); TODO DCDECLSIMD or change it to stmt
    }
#line 6981 "parser.c" /* yacc.c:1646  */
    break;

  case 427:
#line 2842 "parser.y" /* yacc.c:1646  */
    {
      //TODO
    }
#line 6989 "parser.c" /* yacc.c:1646  */
    break;

  case 428:
#line 2846 "parser.y" /* yacc.c:1646  */
    {
        //TODO
    }
#line 6997 "parser.c" /* yacc.c:1646  */
    break;

  case 429:
#line 2854 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCDECLSIMD, $4); TODO DCDECLSIMD
    }
#line 7005 "parser.c" /* yacc.c:1646  */
    break;

  case 430:
#line 2861 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7013 "parser.c" /* yacc.c:1646  */
    break;

  case 431:
#line 2865 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7021 "parser.c" /* yacc.c:1646  */
    break;

  case 432:
#line 2869 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7029 "parser.c" /* yacc.c:1646  */
    break;

  case 433:
#line 2878 "parser.y" /* yacc.c:1646  */
    {
      int n = 0, er = 0;
      if (xar_expr_is_constant((yyvsp[-1].expr)))
      {
        n = xar_calc_int_expr((yyvsp[-1].expr), &er);
        if (er) n = 0;
      }
      if (n <= 0)
        parse_error(1, "invalid number in simdlen() clause.\n");
      //$$ = CollapseClause(n); //TODO SIMDLEN
    }
#line 7045 "parser.c" /* yacc.c:1646  */
    break;

  case 434:
#line 2890 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7053 "parser.c" /* yacc.c:1646  */
    break;

  case 435:
#line 2894 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7061 "parser.c" /* yacc.c:1646  */
    break;

  case 436:
#line 2898 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7069 "parser.c" /* yacc.c:1646  */
    break;

  case 437:
#line 2902 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7077 "parser.c" /* yacc.c:1646  */
    break;

  case 438:
#line 2910 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCFORSIMD, $1, $2); TODO DCFORSIMD
    }
#line 7085 "parser.c" /* yacc.c:1646  */
    break;

  case 439:
#line 2918 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCFORSIMD, $4); TODO DCFORSIMD
    }
#line 7093 "parser.c" /* yacc.c:1646  */
    break;

  case 440:
#line 2925 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7101 "parser.c" /* yacc.c:1646  */
    break;

  case 441:
#line 2929 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7109 "parser.c" /* yacc.c:1646  */
    break;

  case 442:
#line 2933 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7117 "parser.c" /* yacc.c:1646  */
    break;

  case 443:
#line 2940 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7125 "parser.c" /* yacc.c:1646  */
    break;

  case 444:
#line 2944 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7133 "parser.c" /* yacc.c:1646  */
    break;

  case 445:
#line 2952 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCPARFORSIMD, $1, $2); TODO DCPARFORSIMD
    }
#line 7141 "parser.c" /* yacc.c:1646  */
    break;

  case 446:
#line 2960 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCPARFORSIMD, $5); TODO DCFORSIMD
    }
#line 7149 "parser.c" /* yacc.c:1646  */
    break;

  case 447:
#line 2967 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7157 "parser.c" /* yacc.c:1646  */
    break;

  case 448:
#line 2971 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7165 "parser.c" /* yacc.c:1646  */
    break;

  case 449:
#line 2975 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7173 "parser.c" /* yacc.c:1646  */
    break;

  case 450:
#line 2982 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7181 "parser.c" /* yacc.c:1646  */
    break;

  case 451:
#line 2986 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7189 "parser.c" /* yacc.c:1646  */
    break;

  case 452:
#line 2994 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTARGETDATA, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 7197 "parser.c" /* yacc.c:1646  */
    break;

  case 453:
#line 3002 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTARGETDATA, (yyvsp[-1].ocla));
    }
#line 7205 "parser.c" /* yacc.c:1646  */
    break;

  case 454:
#line 3009 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7213 "parser.c" /* yacc.c:1646  */
    break;

  case 455:
#line 3013 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7221 "parser.c" /* yacc.c:1646  */
    break;

  case 456:
#line 3017 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7229 "parser.c" /* yacc.c:1646  */
    break;

  case 457:
#line 3024 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7237 "parser.c" /* yacc.c:1646  */
    break;

  case 458:
#line 3028 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7245 "parser.c" /* yacc.c:1646  */
    break;

  case 459:
#line 3032 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7253 "parser.c" /* yacc.c:1646  */
    break;

  case 460:
#line 3039 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7259 "parser.c" /* yacc.c:1646  */
    break;

  case 461:
#line 3040 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = DeviceClause((yyvsp[-1].expr));
    }
#line 7268 "parser.c" /* yacc.c:1646  */
    break;

  case 462:
#line 3048 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7274 "parser.c" /* yacc.c:1646  */
    break;

  case 463:
#line 3049 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = MapClause((yyvsp[-4].type), (yyvsp[-1].decl));
    }
#line 7283 "parser.c" /* yacc.c:1646  */
    break;

  case 464:
#line 3053 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7289 "parser.c" /* yacc.c:1646  */
    break;

  case 465:
#line 3054 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
	  (yyval.ocla) = MapClause(OC_tofrom, (yyvsp[-1].decl));
    }
#line 7298 "parser.c" /* yacc.c:1646  */
    break;

  case 466:
#line 3063 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_alloc;
    }
#line 7306 "parser.c" /* yacc.c:1646  */
    break;

  case 467:
#line 3067 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_to;
    }
#line 7314 "parser.c" /* yacc.c:1646  */
    break;

  case 468:
#line 3071 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_from;
    }
#line 7322 "parser.c" /* yacc.c:1646  */
    break;

  case 469:
#line 3075 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_tofrom;
    }
#line 7330 "parser.c" /* yacc.c:1646  */
    break;

  case 470:
#line 3082 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = errorOnReturn;  errorOnReturn = 1; }
#line 7336 "parser.c" /* yacc.c:1646  */
    break;

  case 471:
#line 3084 "parser.y" /* yacc.c:1646  */
    {
      errorOnReturn = (yyvsp[-1].type);
      (yyval.ocon) = OmpConstruct(DCTARGET, (yyvsp[-2].odir), (yyvsp[0].stmt));
      __has_target = 1;
    }
#line 7346 "parser.c" /* yacc.c:1646  */
    break;

  case 472:
#line 3094 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTARGET, (yyvsp[-1].ocla));
    }
#line 7354 "parser.c" /* yacc.c:1646  */
    break;

  case 473:
#line 3101 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7362 "parser.c" /* yacc.c:1646  */
    break;

  case 474:
#line 3105 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7370 "parser.c" /* yacc.c:1646  */
    break;

  case 475:
#line 3109 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7378 "parser.c" /* yacc.c:1646  */
    break;

  case 476:
#line 3116 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7386 "parser.c" /* yacc.c:1646  */
    break;

  case 477:
#line 3120 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7394 "parser.c" /* yacc.c:1646  */
    break;

  case 478:
#line 3127 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7402 "parser.c" /* yacc.c:1646  */
    break;

  case 479:
#line 3131 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7410 "parser.c" /* yacc.c:1646  */
    break;

  case 480:
#line 3139 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTARGETUPD, (yyvsp[0].odir), NULL);
    }
#line 7418 "parser.c" /* yacc.c:1646  */
    break;

  case 481:
#line 3147 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTARGETUPD, (yyvsp[-1].ocla));
    }
#line 7426 "parser.c" /* yacc.c:1646  */
    break;

  case 482:
#line 3154 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7434 "parser.c" /* yacc.c:1646  */
    break;

  case 483:
#line 3158 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7442 "parser.c" /* yacc.c:1646  */
    break;

  case 484:
#line 3162 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7450 "parser.c" /* yacc.c:1646  */
    break;

  case 485:
#line 3169 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7458 "parser.c" /* yacc.c:1646  */
    break;

  case 486:
#line 3173 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7466 "parser.c" /* yacc.c:1646  */
    break;

  case 487:
#line 3177 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7474 "parser.c" /* yacc.c:1646  */
    break;

  case 488:
#line 3183 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7480 "parser.c" /* yacc.c:1646  */
    break;

  case 489:
#line 3184 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCTO, (yyvsp[-1].decl));
    }
#line 7489 "parser.c" /* yacc.c:1646  */
    break;

  case 490:
#line 3188 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7495 "parser.c" /* yacc.c:1646  */
    break;

  case 491:
#line 3189 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCFROM, (yyvsp[-1].decl));
    }
#line 7504 "parser.c" /* yacc.c:1646  */
    break;

  case 492:
#line 3198 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCDECLTARGET, (yyvsp[-2].odir), (yyvsp[-1].stmt));
    }
#line 7512 "parser.c" /* yacc.c:1646  */
    break;

  case 493:
#line 3205 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCDECLTARGET, NULL);
    }
#line 7520 "parser.c" /* yacc.c:1646  */
    break;

  case 494:
#line 3212 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCENDDECLTARGET, NULL); TODO DCENDDECLTARGET
    }
#line 7528 "parser.c" /* yacc.c:1646  */
    break;

  case 495:
#line 3220 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTEAMS, $1, $2); TODO DCTEAMS
    }
#line 7536 "parser.c" /* yacc.c:1646  */
    break;

  case 496:
#line 3228 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTEAMS, $3); TODO DCTEAMS
    }
#line 7544 "parser.c" /* yacc.c:1646  */
    break;

  case 497:
#line 3235 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7552 "parser.c" /* yacc.c:1646  */
    break;

  case 498:
#line 3239 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7560 "parser.c" /* yacc.c:1646  */
    break;

  case 499:
#line 3243 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7568 "parser.c" /* yacc.c:1646  */
    break;

  case 500:
#line 3250 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7576 "parser.c" /* yacc.c:1646  */
    break;

  case 501:
#line 3254 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7584 "parser.c" /* yacc.c:1646  */
    break;

  case 502:
#line 3258 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7592 "parser.c" /* yacc.c:1646  */
    break;

  case 503:
#line 3262 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7600 "parser.c" /* yacc.c:1646  */
    break;

  case 504:
#line 3266 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7608 "parser.c" /* yacc.c:1646  */
    break;

  case 505:
#line 3270 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7616 "parser.c" /* yacc.c:1646  */
    break;

  case 506:
#line 3277 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7622 "parser.c" /* yacc.c:1646  */
    break;

  case 507:
#line 3278 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      //$$ = NumthreadsClause($4); //TODO check if numthreads is good or if I should make something new
    }
#line 7631 "parser.c" /* yacc.c:1646  */
    break;

  case 508:
#line 3283 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7637 "parser.c" /* yacc.c:1646  */
    break;

  case 509:
#line 3284 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      //$$ = NumthreadsClause($4); //TODO check if numthreads is good or if I should make something new
    }
#line 7646 "parser.c" /* yacc.c:1646  */
    break;

  case 510:
#line 3293 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCDISTRIBUTE, $1, $2); TODO DCDISTRIBUTE
    }
#line 7654 "parser.c" /* yacc.c:1646  */
    break;

  case 511:
#line 3301 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCDISTRIBUTE, $3); TODO DCDISTRIBUTE
    }
#line 7662 "parser.c" /* yacc.c:1646  */
    break;

  case 512:
#line 3308 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7670 "parser.c" /* yacc.c:1646  */
    break;

  case 513:
#line 3312 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7678 "parser.c" /* yacc.c:1646  */
    break;

  case 514:
#line 3316 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7686 "parser.c" /* yacc.c:1646  */
    break;

  case 515:
#line 3323 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7694 "parser.c" /* yacc.c:1646  */
    break;

  case 516:
#line 3327 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7702 "parser.c" /* yacc.c:1646  */
    break;

  case 517:
#line 3331 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7710 "parser.c" /* yacc.c:1646  */
    break;

  case 518:
#line 3335 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7718 "parser.c" /* yacc.c:1646  */
    break;

  case 519:
#line 3342 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = ScheduleClause(OC_static, NULL);
    }
#line 7726 "parser.c" /* yacc.c:1646  */
    break;

  case 520:
#line 3345 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7732 "parser.c" /* yacc.c:1646  */
    break;

  case 521:
#line 3346 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = ScheduleClause(OC_static, (yyvsp[-1].expr));
    }
#line 7741 "parser.c" /* yacc.c:1646  */
    break;

  case 522:
#line 3355 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCDISTSIMD, $1, $2); TODO DCDISTSIMD
      //$$->l = $1->l;
    }
#line 7750 "parser.c" /* yacc.c:1646  */
    break;

  case 523:
#line 3364 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCDISTSIMD, $4); TODO DCDISTSIMD
    }
#line 7758 "parser.c" /* yacc.c:1646  */
    break;

  case 524:
#line 3371 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7766 "parser.c" /* yacc.c:1646  */
    break;

  case 525:
#line 3375 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7774 "parser.c" /* yacc.c:1646  */
    break;

  case 526:
#line 3379 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7782 "parser.c" /* yacc.c:1646  */
    break;

  case 527:
#line 3386 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7790 "parser.c" /* yacc.c:1646  */
    break;

  case 528:
#line 3390 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7798 "parser.c" /* yacc.c:1646  */
    break;

  case 529:
#line 3394 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7806 "parser.c" /* yacc.c:1646  */
    break;

  case 530:
#line 3402 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCDISTPARFOR, $1, $2); TODO DCDISTPARFOR
      //$$->l = $1->l;
    }
#line 7815 "parser.c" /* yacc.c:1646  */
    break;

  case 531:
#line 3411 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCDISTPARFOR, $5); TODO DCDISTPARFOR
    }
#line 7823 "parser.c" /* yacc.c:1646  */
    break;

  case 532:
#line 3418 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7831 "parser.c" /* yacc.c:1646  */
    break;

  case 533:
#line 3422 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7839 "parser.c" /* yacc.c:1646  */
    break;

  case 534:
#line 3426 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7847 "parser.c" /* yacc.c:1646  */
    break;

  case 535:
#line 3433 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7855 "parser.c" /* yacc.c:1646  */
    break;

  case 536:
#line 3437 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7863 "parser.c" /* yacc.c:1646  */
    break;

  case 537:
#line 3445 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCDISTPARFORSIMD, $1, $2); TODO DCDISTPARFORSIMD
      //$$->l = $1->l;
    }
#line 7872 "parser.c" /* yacc.c:1646  */
    break;

  case 538:
#line 3454 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCDISTPARFORSIMD, $6); TODO DCDISTPARFORSIMD
    }
#line 7880 "parser.c" /* yacc.c:1646  */
    break;

  case 539:
#line 3461 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7888 "parser.c" /* yacc.c:1646  */
    break;

  case 540:
#line 3465 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7896 "parser.c" /* yacc.c:1646  */
    break;

  case 541:
#line 3469 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7904 "parser.c" /* yacc.c:1646  */
    break;

  case 542:
#line 3476 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7912 "parser.c" /* yacc.c:1646  */
    break;

  case 543:
#line 3480 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7920 "parser.c" /* yacc.c:1646  */
    break;

  case 544:
#line 3488 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTARGETTEAMS, $1, $2); TODO DCTARGETTEAMS
    }
#line 7928 "parser.c" /* yacc.c:1646  */
    break;

  case 545:
#line 3496 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTARGETTEAMS, $4); TODO DCTARGETTEAMS
    }
#line 7936 "parser.c" /* yacc.c:1646  */
    break;

  case 546:
#line 3503 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7944 "parser.c" /* yacc.c:1646  */
    break;

  case 547:
#line 3507 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7952 "parser.c" /* yacc.c:1646  */
    break;

  case 548:
#line 3511 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7960 "parser.c" /* yacc.c:1646  */
    break;

  case 549:
#line 3518 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7968 "parser.c" /* yacc.c:1646  */
    break;

  case 550:
#line 3522 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7976 "parser.c" /* yacc.c:1646  */
    break;

  case 551:
#line 3530 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTEAMSDIST, $1, $2); TODO DCTEAMSDIST
    }
#line 7984 "parser.c" /* yacc.c:1646  */
    break;

  case 552:
#line 3538 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTEAMSDIST, $4); TODO DCTEAMSDIST
    }
#line 7992 "parser.c" /* yacc.c:1646  */
    break;

  case 553:
#line 3545 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8000 "parser.c" /* yacc.c:1646  */
    break;

  case 554:
#line 3549 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8008 "parser.c" /* yacc.c:1646  */
    break;

  case 555:
#line 3553 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8016 "parser.c" /* yacc.c:1646  */
    break;

  case 556:
#line 3560 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8024 "parser.c" /* yacc.c:1646  */
    break;

  case 557:
#line 3564 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8032 "parser.c" /* yacc.c:1646  */
    break;

  case 558:
#line 3568 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8040 "parser.c" /* yacc.c:1646  */
    break;

  case 559:
#line 3576 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTEAMSDISTSIMD, $1, $2); TODO DCTEAMSDISTSIMD
    }
#line 8048 "parser.c" /* yacc.c:1646  */
    break;

  case 560:
#line 3584 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTEAMSDISTSIMD, $5); TODO DCTEAMSDISTSIMD
    }
#line 8056 "parser.c" /* yacc.c:1646  */
    break;

  case 561:
#line 3591 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8064 "parser.c" /* yacc.c:1646  */
    break;

  case 562:
#line 3595 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8072 "parser.c" /* yacc.c:1646  */
    break;

  case 563:
#line 3599 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8080 "parser.c" /* yacc.c:1646  */
    break;

  case 564:
#line 3606 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8088 "parser.c" /* yacc.c:1646  */
    break;

  case 565:
#line 3610 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8096 "parser.c" /* yacc.c:1646  */
    break;

  case 566:
#line 3614 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8104 "parser.c" /* yacc.c:1646  */
    break;

  case 567:
#line 3618 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8112 "parser.c" /* yacc.c:1646  */
    break;

  case 568:
#line 3626 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTARGETTEAMSDIST, $1, $2); TODO DCTARGETTEAMSDIST
    }
#line 8120 "parser.c" /* yacc.c:1646  */
    break;

  case 569:
#line 3634 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTARGETTEAMSDIST, $5); TODO DCTARGETTEAMSDIST
    }
#line 8128 "parser.c" /* yacc.c:1646  */
    break;

  case 570:
#line 3641 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8136 "parser.c" /* yacc.c:1646  */
    break;

  case 571:
#line 3645 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8144 "parser.c" /* yacc.c:1646  */
    break;

  case 572:
#line 3649 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8152 "parser.c" /* yacc.c:1646  */
    break;

  case 573:
#line 3656 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8160 "parser.c" /* yacc.c:1646  */
    break;

  case 574:
#line 3660 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8168 "parser.c" /* yacc.c:1646  */
    break;

  case 575:
#line 3668 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTARGETTEAMSDISTSIMD, $1, $2); TODO DCTARGETTEAMSDISTSIMD
    }
#line 8176 "parser.c" /* yacc.c:1646  */
    break;

  case 576:
#line 3677 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTARGETTEAMSDISTSIMD, $6); TODO DCTARGETTEAMSDISTSIMD
    }
#line 8184 "parser.c" /* yacc.c:1646  */
    break;

  case 577:
#line 3684 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8192 "parser.c" /* yacc.c:1646  */
    break;

  case 578:
#line 3689 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8200 "parser.c" /* yacc.c:1646  */
    break;

  case 579:
#line 3694 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8208 "parser.c" /* yacc.c:1646  */
    break;

  case 580:
#line 3701 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8216 "parser.c" /* yacc.c:1646  */
    break;

  case 581:
#line 3705 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8224 "parser.c" /* yacc.c:1646  */
    break;

  case 582:
#line 3713 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTEAMSDISTPARFOR, $1, $2); TODO DCTEAMSDISTPARFOR
      //$$->l = $1->l;
    }
#line 8233 "parser.c" /* yacc.c:1646  */
    break;

  case 583:
#line 3723 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTEAMSDISTPARFOR, $3); TODO DCTEAMSDISTPARFOR
    }
#line 8241 "parser.c" /* yacc.c:1646  */
    break;

  case 584:
#line 3730 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8249 "parser.c" /* yacc.c:1646  */
    break;

  case 585:
#line 3735 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8257 "parser.c" /* yacc.c:1646  */
    break;

  case 586:
#line 3740 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8265 "parser.c" /* yacc.c:1646  */
    break;

  case 587:
#line 3747 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8273 "parser.c" /* yacc.c:1646  */
    break;

  case 588:
#line 3751 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8281 "parser.c" /* yacc.c:1646  */
    break;

  case 589:
#line 3759 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTARGETTEAMSDISTPARFOR, $1, $2); TODO DCTARGETTEAMSDISTPARFOR
      //$$->l = $1->l;
    }
#line 8290 "parser.c" /* yacc.c:1646  */
    break;

  case 590:
#line 3768 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTARGETTEAMSDISTPARFOR, $7); TODO DCTARGETTEAMSDISTPARFOR
    }
#line 8298 "parser.c" /* yacc.c:1646  */
    break;

  case 591:
#line 3775 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8306 "parser.c" /* yacc.c:1646  */
    break;

  case 592:
#line 3779 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8314 "parser.c" /* yacc.c:1646  */
    break;

  case 593:
#line 3783 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8322 "parser.c" /* yacc.c:1646  */
    break;

  case 594:
#line 3790 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8330 "parser.c" /* yacc.c:1646  */
    break;

  case 595:
#line 3794 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8338 "parser.c" /* yacc.c:1646  */
    break;

  case 596:
#line 3802 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTEAMSDISTPARFORSIMD, $1, $2); TODO DCTEAMSDISTPARFORSIMD
      //$$->l = $1->l;
    }
#line 8347 "parser.c" /* yacc.c:1646  */
    break;

  case 597:
#line 3811 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTEAMSDISTPARFORSIMD, $7); TODO DCTEAMSDISTPARFORSIMD
    }
#line 8355 "parser.c" /* yacc.c:1646  */
    break;

  case 598:
#line 3818 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8363 "parser.c" /* yacc.c:1646  */
    break;

  case 599:
#line 3822 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8371 "parser.c" /* yacc.c:1646  */
    break;

  case 600:
#line 3826 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8379 "parser.c" /* yacc.c:1646  */
    break;

  case 601:
#line 3833 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8387 "parser.c" /* yacc.c:1646  */
    break;

  case 602:
#line 3837 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8395 "parser.c" /* yacc.c:1646  */
    break;

  case 603:
#line 3845 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTARGETTEAMSDISTPARFORSIMD, $1, $2); TODO DCTARGETTEAMSDISTPARFORSIMD
      //$$->l = $1->l;
    }
#line 8404 "parser.c" /* yacc.c:1646  */
    break;

  case 604:
#line 3854 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTARGETTEAMSDISTPARFORSIMD, $8); TODO DCTARGETTEAMSDISTPARFORSIMD
    }
#line 8412 "parser.c" /* yacc.c:1646  */
    break;

  case 605:
#line 3861 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8420 "parser.c" /* yacc.c:1646  */
    break;

  case 606:
#line 3865 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8428 "parser.c" /* yacc.c:1646  */
    break;

  case 607:
#line 3869 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8436 "parser.c" /* yacc.c:1646  */
    break;

  case 608:
#line 3876 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8444 "parser.c" /* yacc.c:1646  */
    break;

  case 609:
#line 3880 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8452 "parser.c" /* yacc.c:1646  */
    break;

  case 610:
#line 3888 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTASK, (yyvsp[-1].odir), (yyvsp[0].stmt));
      (yyval.ocon)->l = (yyvsp[-1].odir)->l;
    }
#line 8461 "parser.c" /* yacc.c:1646  */
    break;

  case 611:
#line 3897 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTASK, (yyvsp[-1].ocla));
    }
#line 8469 "parser.c" /* yacc.c:1646  */
    break;

  case 612:
#line 3905 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8477 "parser.c" /* yacc.c:1646  */
    break;

  case 613:
#line 3909 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8485 "parser.c" /* yacc.c:1646  */
    break;

  case 614:
#line 3913 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8493 "parser.c" /* yacc.c:1646  */
    break;

  case 615:
#line 3921 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8501 "parser.c" /* yacc.c:1646  */
    break;

  case 616:
#line 3925 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8509 "parser.c" /* yacc.c:1646  */
    break;

  case 617:
#line 3929 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8517 "parser.c" /* yacc.c:1646  */
    break;

  case 618:
#line 3933 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8525 "parser.c" /* yacc.c:1646  */
    break;

  case 619:
#line 3937 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8533 "parser.c" /* yacc.c:1646  */
    break;

  case 620:
#line 3945 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8541 "parser.c" /* yacc.c:1646  */
    break;

  case 621:
#line 3948 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 8547 "parser.c" /* yacc.c:1646  */
    break;

  case 622:
#line 3949 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = FinalClause((yyvsp[-1].expr));
    }
#line 8556 "parser.c" /* yacc.c:1646  */
    break;

  case 623:
#line 3954 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCUNTIED);
    }
#line 8564 "parser.c" /* yacc.c:1646  */
    break;

  case 624:
#line 3958 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCMERGEABLE);
    }
#line 8572 "parser.c" /* yacc.c:1646  */
    break;

  case 625:
#line 3963 "parser.y" /* yacc.c:1646  */
    {
      //$$ = VarlistClause(OCPRIVATE, $6); TODO find out how to do this. It needs type OCDEPEND subtype from $3 and a list from $6
    }
#line 8580 "parser.c" /* yacc.c:1646  */
    break;

  case 626:
#line 3971 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OC_in; TODO OC_in
    }
#line 8588 "parser.c" /* yacc.c:1646  */
    break;

  case 627:
#line 3975 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OC_out; TODO OC_out
    }
#line 8596 "parser.c" /* yacc.c:1646  */
    break;

  case 628:
#line 3979 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OC_inout; TODO OC_inout
    }
#line 8604 "parser.c" /* yacc.c:1646  */
    break;

  case 629:
#line 3986 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCPARFOR, (yyvsp[-1].odir), (yyvsp[0].stmt));
      (yyval.ocon)->l = (yyvsp[-1].odir)->l;
    }
#line 8613 "parser.c" /* yacc.c:1646  */
    break;

  case 630:
#line 3994 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCPARFOR, (yyvsp[-1].ocla));
    }
#line 8621 "parser.c" /* yacc.c:1646  */
    break;

  case 631:
#line 4001 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8629 "parser.c" /* yacc.c:1646  */
    break;

  case 632:
#line 4005 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8637 "parser.c" /* yacc.c:1646  */
    break;

  case 633:
#line 4009 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8645 "parser.c" /* yacc.c:1646  */
    break;

  case 634:
#line 4016 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8653 "parser.c" /* yacc.c:1646  */
    break;

  case 635:
#line 4020 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8661 "parser.c" /* yacc.c:1646  */
    break;

  case 636:
#line 4024 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8669 "parser.c" /* yacc.c:1646  */
    break;

  case 637:
#line 4028 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8677 "parser.c" /* yacc.c:1646  */
    break;

  case 638:
#line 4032 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8685 "parser.c" /* yacc.c:1646  */
    break;

  case 639:
#line 4036 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8693 "parser.c" /* yacc.c:1646  */
    break;

  case 640:
#line 4040 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8701 "parser.c" /* yacc.c:1646  */
    break;

  case 641:
#line 4044 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8709 "parser.c" /* yacc.c:1646  */
    break;

  case 642:
#line 4051 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCPARSECTIONS, (yyvsp[-1].odir), (yyvsp[0].stmt));
      (yyval.ocon)->l = (yyvsp[-1].odir)->l;
    }
#line 8718 "parser.c" /* yacc.c:1646  */
    break;

  case 643:
#line 4059 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCPARSECTIONS, (yyvsp[-1].ocla));
    }
#line 8726 "parser.c" /* yacc.c:1646  */
    break;

  case 644:
#line 4066 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8734 "parser.c" /* yacc.c:1646  */
    break;

  case 645:
#line 4070 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8742 "parser.c" /* yacc.c:1646  */
    break;

  case 646:
#line 4074 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8750 "parser.c" /* yacc.c:1646  */
    break;

  case 647:
#line 4081 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8758 "parser.c" /* yacc.c:1646  */
    break;

  case 648:
#line 4085 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8766 "parser.c" /* yacc.c:1646  */
    break;

  case 649:
#line 4089 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8774 "parser.c" /* yacc.c:1646  */
    break;

  case 650:
#line 4093 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8782 "parser.c" /* yacc.c:1646  */
    break;

  case 651:
#line 4097 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8790 "parser.c" /* yacc.c:1646  */
    break;

  case 652:
#line 4101 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8798 "parser.c" /* yacc.c:1646  */
    break;

  case 653:
#line 4105 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8806 "parser.c" /* yacc.c:1646  */
    break;

  case 654:
#line 4112 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCMASTER, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 8814 "parser.c" /* yacc.c:1646  */
    break;

  case 655:
#line 4119 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCMASTER, NULL);
    }
#line 8822 "parser.c" /* yacc.c:1646  */
    break;

  case 656:
#line 4126 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCCRITICAL, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 8830 "parser.c" /* yacc.c:1646  */
    break;

  case 657:
#line 4133 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpCriticalDirective(NULL);
    }
#line 8838 "parser.c" /* yacc.c:1646  */
    break;

  case 658:
#line 4137 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpCriticalDirective((yyvsp[-1].symb));
    }
#line 8846 "parser.c" /* yacc.c:1646  */
    break;

  case 659:
#line 4144 "parser.y" /* yacc.c:1646  */
    {
      (yyval.symb) = Symbol((yyvsp[-1].name));
    }
#line 8854 "parser.c" /* yacc.c:1646  */
    break;

  case 660:
#line 4151 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCBARRIER, NULL);
    }
#line 8862 "parser.c" /* yacc.c:1646  */
    break;

  case 661:
#line 4159 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTASKWAIT, NULL);
    }
#line 8870 "parser.c" /* yacc.c:1646  */
    break;

  case 662:
#line 4167 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTASKGROUP, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 8878 "parser.c" /* yacc.c:1646  */
    break;

  case 663:
#line 4175 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTASKGROUP, NULL);
    }
#line 8886 "parser.c" /* yacc.c:1646  */
    break;

  case 664:
#line 4183 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTASKYIELD, NULL);
    }
#line 8894 "parser.c" /* yacc.c:1646  */
    break;

  case 665:
#line 4190 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCATOMIC, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 8902 "parser.c" /* yacc.c:1646  */
    break;

  case 666:
#line 4201 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCATOMIC, NULL);  //TODO Check how to do it since it now has 2 clauses
    }
#line 8910 "parser.c" /* yacc.c:1646  */
    break;

  case 667:
#line 4208 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8918 "parser.c" /* yacc.c:1646  */
    break;

  case 668:
#line 4212 "parser.y" /* yacc.c:1646  */
    {
      //$$ = TODO
    }
#line 8926 "parser.c" /* yacc.c:1646  */
    break;

  case 669:
#line 4216 "parser.y" /* yacc.c:1646  */
    {
      //$$ = TODO
    }
#line 8934 "parser.c" /* yacc.c:1646  */
    break;

  case 670:
#line 4220 "parser.y" /* yacc.c:1646  */
    {
      //$$ = TODO
    }
#line 8942 "parser.c" /* yacc.c:1646  */
    break;

  case 671:
#line 4224 "parser.y" /* yacc.c:1646  */
    {
      //$$ = TODO
    }
#line 8950 "parser.c" /* yacc.c:1646  */
    break;

  case 672:
#line 4232 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8958 "parser.c" /* yacc.c:1646  */
    break;

  case 673:
#line 4236 "parser.y" /* yacc.c:1646  */
    {
      //$$ = TODO
    }
#line 8966 "parser.c" /* yacc.c:1646  */
    break;

  case 674:
#line 4243 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpFlushDirective(NULL);
    }
#line 8974 "parser.c" /* yacc.c:1646  */
    break;

  case 675:
#line 4247 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpFlushDirective((yyvsp[-1].decl));
    }
#line 8982 "parser.c" /* yacc.c:1646  */
    break;

  case 676:
#line 4253 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 8988 "parser.c" /* yacc.c:1646  */
    break;

  case 677:
#line 4254 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.decl) = (yyvsp[-1].decl);
    }
#line 8997 "parser.c" /* yacc.c:1646  */
    break;

  case 678:
#line 4262 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCORDERED, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 9005 "parser.c" /* yacc.c:1646  */
    break;

  case 679:
#line 4269 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCORDERED, NULL);
    }
#line 9013 "parser.c" /* yacc.c:1646  */
    break;

  case 680:
#line 4277 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCCANCEL, (yyvsp[-1].ocla));
    }
#line 9021 "parser.c" /* yacc.c:1646  */
    break;

  case 681:
#line 4281 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCCANCEL, OmpClauseList((yyvsp[-2].ocla), (yyvsp[-1].ocla)));
    }
#line 9029 "parser.c" /* yacc.c:1646  */
    break;

  case 682:
#line 4285 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCCANCEL, OmpClauseList((yyvsp[-3].ocla), (yyvsp[-1].ocla)));
    }
#line 9037 "parser.c" /* yacc.c:1646  */
    break;

  case 683:
#line 4292 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCPARALLEL);
    }
#line 9045 "parser.c" /* yacc.c:1646  */
    break;

  case 684:
#line 4296 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCSECTIONS);
    }
#line 9053 "parser.c" /* yacc.c:1646  */
    break;

  case 685:
#line 4300 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCFOR);
    }
#line 9061 "parser.c" /* yacc.c:1646  */
    break;

  case 686:
#line 4304 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCTASKGROUP);
    }
#line 9069 "parser.c" /* yacc.c:1646  */
    break;

  case 687:
#line 4312 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCCANCELLATIONPOINT, (yyvsp[-1].ocla));
    }
#line 9077 "parser.c" /* yacc.c:1646  */
    break;

  case 688:
#line 4318 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 9083 "parser.c" /* yacc.c:1646  */
    break;

  case 689:
#line 4318 "parser.y" /* yacc.c:1646  */
    { sc_start_openmp(); }
#line 9089 "parser.c" /* yacc.c:1646  */
    break;

  case 690:
#line 4319 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpThreadprivateDirective((yyvsp[-3].decl));
    }
#line 9097 "parser.c" /* yacc.c:1646  */
    break;

  case 691:
#line 4326 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 9103 "parser.c" /* yacc.c:1646  */
    break;

  case 692:
#line 4326 "parser.y" /* yacc.c:1646  */
    { sc_start_openmp(); }
#line 9109 "parser.c" /* yacc.c:1646  */
    break;

  case 693:
#line 4327 "parser.y" /* yacc.c:1646  */
    {
      //$$ = TODO
    }
#line 9117 "parser.c" /* yacc.c:1646  */
    break;

  case 694:
#line 4334 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OC_identifier TODO
      //Symbol($2);  TODO
    }
#line 9126 "parser.c" /* yacc.c:1646  */
    break;

  case 695:
#line 4339 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_plus;
    }
#line 9134 "parser.c" /* yacc.c:1646  */
    break;

  case 696:
#line 4343 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_times;
    }
#line 9142 "parser.c" /* yacc.c:1646  */
    break;

  case 697:
#line 4347 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_minus;
    }
#line 9150 "parser.c" /* yacc.c:1646  */
    break;

  case 698:
#line 4351 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_band;
    }
#line 9158 "parser.c" /* yacc.c:1646  */
    break;

  case 699:
#line 4355 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_xor;
    }
#line 9166 "parser.c" /* yacc.c:1646  */
    break;

  case 700:
#line 4359 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_bor;
    }
#line 9174 "parser.c" /* yacc.c:1646  */
    break;

  case 701:
#line 4363 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_land;
    }
#line 9182 "parser.c" /* yacc.c:1646  */
    break;

  case 702:
#line 4367 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_lor;
    }
#line 9190 "parser.c" /* yacc.c:1646  */
    break;

  case 703:
#line 4371 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_min;
    }
#line 9198 "parser.c" /* yacc.c:1646  */
    break;

  case 704:
#line 4375 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_max;
    }
#line 9206 "parser.c" /* yacc.c:1646  */
    break;

  case 705:
#line 4382 "parser.y" /* yacc.c:1646  */
    {
      //TODO
    }
#line 9214 "parser.c" /* yacc.c:1646  */
    break;

  case 706:
#line 4386 "parser.y" /* yacc.c:1646  */
    {
      //TODO
    }
#line 9222 "parser.c" /* yacc.c:1646  */
    break;

  case 707:
#line 4393 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 9230 "parser.c" /* yacc.c:1646  */
    break;

  case 708:
#line 4397 "parser.y" /* yacc.c:1646  */
    {
        //TODO must check if identifier is omp_priv and that conditional
        //expression contains only omp_priv and omp_orig variables
    }
#line 9239 "parser.c" /* yacc.c:1646  */
    break;

  case 709:
#line 4402 "parser.y" /* yacc.c:1646  */
    {
      //TODO in argument_expression_list one of the variables must be &omp_priv
      // TODO check ox_funccall_expression
      //$$ = strcmp($1, "main") ?
      //       FunctionCall(IdentName($1), $3) :
      //       FunctionCall(IdentName(MAIN_NEWNAME), $3);
    }
#line 9251 "parser.c" /* yacc.c:1646  */
    break;

  case 710:
#line 4413 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = DefaultClause(OC_defshared);
    }
#line 9259 "parser.c" /* yacc.c:1646  */
    break;

  case 711:
#line 4417 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = DefaultClause(OC_defnone);
    }
#line 9267 "parser.c" /* yacc.c:1646  */
    break;

  case 712:
#line 4422 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = DefaultClause(OC_auto); //I'm using the existing subtype (Alexandros)
    }
#line 9275 "parser.c" /* yacc.c:1646  */
    break;

  case 713:
#line 4428 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 9281 "parser.c" /* yacc.c:1646  */
    break;

  case 714:
#line 4429 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCPRIVATE, (yyvsp[-1].decl));
    }
#line 9290 "parser.c" /* yacc.c:1646  */
    break;

  case 715:
#line 4436 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 9296 "parser.c" /* yacc.c:1646  */
    break;

  case 716:
#line 4437 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCFIRSTPRIVATE, (yyvsp[-1].decl));
    }
#line 9305 "parser.c" /* yacc.c:1646  */
    break;

  case 717:
#line 4444 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 9311 "parser.c" /* yacc.c:1646  */
    break;

  case 718:
#line 4445 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCLASTPRIVATE, (yyvsp[-1].decl));
    }
#line 9320 "parser.c" /* yacc.c:1646  */
    break;

  case 719:
#line 4452 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 9326 "parser.c" /* yacc.c:1646  */
    break;

  case 720:
#line 4453 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCSHARED, (yyvsp[-1].decl));
    }
#line 9335 "parser.c" /* yacc.c:1646  */
    break;

  case 721:
#line 4460 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 9341 "parser.c" /* yacc.c:1646  */
    break;

  case 722:
#line 4461 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = ReductionClause((yyvsp[-4].type), (yyvsp[-1].decl));
    }
#line 9350 "parser.c" /* yacc.c:1646  */
    break;

  case 723:
#line 4468 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 9356 "parser.c" /* yacc.c:1646  */
    break;

  case 724:
#line 4469 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = IfClause((yyvsp[-1].expr));
    }
#line 9365 "parser.c" /* yacc.c:1646  */
    break;

  case 725:
#line 4477 "parser.y" /* yacc.c:1646  */
    {
      int n = 0, er = 0;
      if (xar_expr_is_constant((yyvsp[-1].expr)))
      {
        n = xar_calc_int_expr((yyvsp[-1].expr), &er);
        if (er) n = 0;
      }
      if (n <= 0)
        parse_error(1, "invalid number in collapse() clause.\n");
      (yyval.ocla) = CollapseClause(n);
    }
#line 9381 "parser.c" /* yacc.c:1646  */
    break;

  case 726:
#line 4492 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls)
        if (symtab_get(stab, Symbol((yyvsp[-1].name)), IDNAME) == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[-1].name));
      (yyval.decl) = IdentifierDecl( Symbol((yyvsp[-1].name)) );
      parse_warning("Array section not supported yet. Ignored.\n");
    }
#line 9393 "parser.c" /* yacc.c:1646  */
    break;

  case 727:
#line 4503 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls)
        if (symtab_get(stab, Symbol((yyvsp[0].name)), IDNAME) == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[0].name));
      (yyval.decl) = IdentifierDecl( Symbol((yyvsp[0].name)) );
    }
#line 9404 "parser.c" /* yacc.c:1646  */
    break;

  case 728:
#line 4510 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls)
        if (symtab_get(stab, Symbol((yyvsp[0].name)), IDNAME) == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[0].name));
      (yyval.decl) = IdList((yyvsp[-2].decl), IdentifierDecl( Symbol((yyvsp[0].name)) ));
    }
#line 9415 "parser.c" /* yacc.c:1646  */
    break;

  case 729:
#line 4520 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls)
        if (symtab_get(stab, Symbol((yyvsp[0].name)), IDNAME) == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[0].name));
      (yyval.decl) = IdentifierDecl( Symbol((yyvsp[0].name)) );
    }
#line 9426 "parser.c" /* yacc.c:1646  */
    break;

  case 730:
#line 4527 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 9434 "parser.c" /* yacc.c:1646  */
    break;

  case 731:
#line 4531 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls)
        if (symtab_get(stab, Symbol((yyvsp[0].name)), IDNAME) == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[0].name));
      (yyval.decl) = IdList((yyvsp[-2].decl), IdentifierDecl( Symbol((yyvsp[0].name)) ));
    }
#line 9445 "parser.c" /* yacc.c:1646  */
    break;

  case 732:
#line 4538 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = IdList((yyvsp[-2].decl), (yyvsp[0].decl));
    }
#line 9453 "parser.c" /* yacc.c:1646  */
    break;

  case 733:
#line 4544 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 9459 "parser.c" /* yacc.c:1646  */
    break;

  case 734:
#line 4545 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      //TODO
    }
#line 9468 "parser.c" /* yacc.c:1646  */
    break;

  case 735:
#line 4549 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 9474 "parser.c" /* yacc.c:1646  */
    break;

  case 736:
#line 4550 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      //TODO
    }
#line 9483 "parser.c" /* yacc.c:1646  */
    break;

  case 737:
#line 4558 "parser.y" /* yacc.c:1646  */
    {
      //TODO
    }
#line 9491 "parser.c" /* yacc.c:1646  */
    break;

  case 738:
#line 4562 "parser.y" /* yacc.c:1646  */
    {
      //TODO
    }
#line 9499 "parser.c" /* yacc.c:1646  */
    break;

  case 739:
#line 4570 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = ProcBindClause(OC_bindmaster);
    }
#line 9507 "parser.c" /* yacc.c:1646  */
    break;

  case 740:
#line 4574 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = ProcBindClause(OC_bindclose);
    }
#line 9515 "parser.c" /* yacc.c:1646  */
    break;

  case 741:
#line 4578 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = ProcBindClause(OC_bindspread);
    }
#line 9523 "parser.c" /* yacc.c:1646  */
    break;

  case 742:
#line 4590 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls)
      {
        stentry e = symtab_get(stab, Symbol((yyvsp[0].name)), IDNAME);
        if (e == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[0].name));
        if (e->scopelevel != stab->scopelevel)
          parse_error(-1, "threadprivate directive appears at different "
                          "scope level\nfrom the one `%s' was declared.\n", (yyvsp[0].name));
        if (stab->scopelevel > 0)    /* Don't care for globals */
          if (speclist_getspec(e->spec, STCLASSSPEC, SPEC_static) == NULL)
            parse_error(-1, "threadprivate variable `%s' does not have static "
                            "storage type.\n", (yyvsp[0].name));
        e->isthrpriv = true;   /* Mark */
      }
      (yyval.decl) = IdentifierDecl( Symbol((yyvsp[0].name)) );
    }
#line 9545 "parser.c" /* yacc.c:1646  */
    break;

  case 743:
#line 4608 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls)
      {
        stentry e = symtab_get(stab, Symbol((yyvsp[0].name)), IDNAME);
        if (e == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[0].name));
        if (e->scopelevel != stab->scopelevel)
          parse_error(-1, "threadprivate directive appears at different "
                          "scope level\nfrom the one `%s' was declared.\n", (yyvsp[0].name));
        if (stab->scopelevel > 0)    /* Don't care for globals */
          if (speclist_getspec(e->spec, STCLASSSPEC, SPEC_static) == NULL)
            parse_error(-1, "threadprivate variable `%s' does not have static "
                            "storage type.\n", (yyvsp[0].name));
        e->isthrpriv = true;   /* Mark */
      }
      (yyval.decl) = IdList((yyvsp[-2].decl), IdentifierDecl( Symbol((yyvsp[0].name)) ));
    }
#line 9567 "parser.c" /* yacc.c:1646  */
    break;

  case 744:
#line 4634 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcon) = OmpixConstruct(OX_DCTASKSYNC, (yyvsp[0].xdir), NULL);
    }
#line 9575 "parser.c" /* yacc.c:1646  */
    break;

  case 745:
#line 4638 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcon) = OmpixConstruct(OX_DCTASKSCHEDULE, (yyvsp[0].xdir), NULL);
    }
#line 9583 "parser.c" /* yacc.c:1646  */
    break;

  case 746:
#line 4646 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xdir) = OmpixDirective(OX_DCTASKSYNC, NULL);
    }
#line 9591 "parser.c" /* yacc.c:1646  */
    break;

  case 747:
#line 4653 "parser.y" /* yacc.c:1646  */
    {
      scope_start(stab);
    }
#line 9599 "parser.c" /* yacc.c:1646  */
    break;

  case 748:
#line 4657 "parser.y" /* yacc.c:1646  */
    {
      scope_end(stab);
      (yyval.xdir) = OmpixDirective(OX_DCTASKSCHEDULE, (yyvsp[-1].xcla));
    }
#line 9608 "parser.c" /* yacc.c:1646  */
    break;

  case 749:
#line 4665 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = NULL;
    }
#line 9616 "parser.c" /* yacc.c:1646  */
    break;

  case 750:
#line 4669 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixClauseList((yyvsp[-1].xcla), (yyvsp[0].xcla));
    }
#line 9624 "parser.c" /* yacc.c:1646  */
    break;

  case 751:
#line 4673 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixClauseList((yyvsp[-2].xcla), (yyvsp[0].xcla));
    }
#line 9632 "parser.c" /* yacc.c:1646  */
    break;

  case 752:
#line 4680 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixStrideClause((yyvsp[-1].expr));
    }
#line 9640 "parser.c" /* yacc.c:1646  */
    break;

  case 753:
#line 4684 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixStartClause((yyvsp[-1].expr));
    }
#line 9648 "parser.c" /* yacc.c:1646  */
    break;

  case 754:
#line 4688 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixScopeClause((yyvsp[-1].type));
    }
#line 9656 "parser.c" /* yacc.c:1646  */
    break;

  case 755:
#line 4692 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixPlainClause(OX_OCTIED);
    }
#line 9664 "parser.c" /* yacc.c:1646  */
    break;

  case 756:
#line 4696 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixPlainClause(OX_OCUNTIED);
    }
#line 9672 "parser.c" /* yacc.c:1646  */
    break;

  case 757:
#line 4703 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OX_SCOPE_NODES;
    }
#line 9680 "parser.c" /* yacc.c:1646  */
    break;

  case 758:
#line 4707 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OX_SCOPE_WGLOBAL;
    }
#line 9688 "parser.c" /* yacc.c:1646  */
    break;

  case 759:
#line 4711 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OX_SCOPE_WGLOBAL;
    }
#line 9696 "parser.c" /* yacc.c:1646  */
    break;

  case 760:
#line 4715 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OX_SCOPE_WLOCAL;
    }
#line 9704 "parser.c" /* yacc.c:1646  */
    break;

  case 761:
#line 4722 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcon) = (yyvsp[0].xcon);
    }
#line 9712 "parser.c" /* yacc.c:1646  */
    break;

  case 762:
#line 4726 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcon) = (yyvsp[0].xcon);
    }
#line 9720 "parser.c" /* yacc.c:1646  */
    break;

  case 763:
#line 4734 "parser.y" /* yacc.c:1646  */
    {
      /* Should put the name of the callback function in the stab, too
      if (symtab_get(stab, decl_getidentifier_symbol($2->u.declaration.decl),
            FUNCNAME) == NULL)
        symtab_put(stab, decl_getidentifier_symbol($2->u.declaration.spec),
            FUNCNAME);
      */
      scope_start(stab);   /* re-declare the arguments of the task function */
      ast_declare_function_params((yyvsp[0].stmt)->u.declaration.decl);
    }
#line 9735 "parser.c" /* yacc.c:1646  */
    break;

  case 764:
#line 4745 "parser.y" /* yacc.c:1646  */
    {
      scope_end(stab);
      (yyval.xcon) = OmpixTaskdef((yyvsp[-3].xdir), (yyvsp[-2].stmt), (yyvsp[0].stmt));
      (yyval.xcon)->l = (yyvsp[-3].xdir)->l;
    }
#line 9745 "parser.c" /* yacc.c:1646  */
    break;

  case 765:
#line 4751 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcon) = OmpixTaskdef((yyvsp[-1].xdir), (yyvsp[0].stmt), NULL);
      (yyval.xcon)->l = (yyvsp[-1].xdir)->l;
    }
#line 9754 "parser.c" /* yacc.c:1646  */
    break;

  case 766:
#line 4759 "parser.y" /* yacc.c:1646  */
    {
      scope_start(stab);
    }
#line 9762 "parser.c" /* yacc.c:1646  */
    break;

  case 767:
#line 4763 "parser.y" /* yacc.c:1646  */
    {
      scope_end(stab);
      (yyval.xdir) = OmpixDirective(OX_DCTASKDEF, (yyvsp[-1].xcla));
    }
#line 9771 "parser.c" /* yacc.c:1646  */
    break;

  case 768:
#line 4771 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = NULL;
    }
#line 9779 "parser.c" /* yacc.c:1646  */
    break;

  case 769:
#line 4775 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixClauseList((yyvsp[-1].xcla), (yyvsp[0].xcla));
    }
#line 9787 "parser.c" /* yacc.c:1646  */
    break;

  case 770:
#line 4779 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixClauseList((yyvsp[-2].xcla), (yyvsp[0].xcla));
    }
#line 9795 "parser.c" /* yacc.c:1646  */
    break;

  case 771:
#line 4786 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixVarlistClause(OX_OCIN, (yyvsp[-1].decl));
    }
#line 9803 "parser.c" /* yacc.c:1646  */
    break;

  case 772:
#line 4790 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixVarlistClause(OX_OCOUT, (yyvsp[-1].decl));
    }
#line 9811 "parser.c" /* yacc.c:1646  */
    break;

  case 773:
#line 4794 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixVarlistClause(OX_OCINOUT, (yyvsp[-1].decl));
    }
#line 9819 "parser.c" /* yacc.c:1646  */
    break;

  case 774:
#line 4798 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixReductionClause((yyvsp[-3].type), (yyvsp[-1].decl));
    }
#line 9827 "parser.c" /* yacc.c:1646  */
    break;

  case 775:
#line 4805 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 9835 "parser.c" /* yacc.c:1646  */
    break;

  case 776:
#line 4809 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = IdList((yyvsp[-2].decl), (yyvsp[0].decl));
    }
#line 9843 "parser.c" /* yacc.c:1646  */
    break;

  case 777:
#line 4816 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = IdentifierDecl( Symbol((yyvsp[0].name)) );
      symtab_put(stab, Symbol((yyvsp[0].name)), IDNAME);
    }
#line 9852 "parser.c" /* yacc.c:1646  */
    break;

  case 778:
#line 4821 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls) check_uknown_var((yyvsp[-1].name));
      /* Use extern to differentiate */
      (yyval.decl) = ArrayDecl(IdentifierDecl( Symbol((yyvsp[-4].name)) ), StClassSpec(SPEC_extern),
                     IdentName((yyvsp[-1].name)));
      symtab_put(stab, Symbol((yyvsp[-4].name)), IDNAME);
    }
#line 9864 "parser.c" /* yacc.c:1646  */
    break;

  case 779:
#line 4829 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl(IdentifierDecl( Symbol((yyvsp[-3].name)) ), NULL, (yyvsp[-1].expr));
      symtab_put(stab, Symbol((yyvsp[-3].name)), IDNAME);
    }
#line 9873 "parser.c" /* yacc.c:1646  */
    break;

  case 780:
#line 4837 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcon) = OmpixConstruct(OX_DCTASK, (yyvsp[-2].xdir), Expression((yyvsp[-1].expr)));
      (yyval.xcon)->l = (yyvsp[-2].xdir)->l;
    }
#line 9882 "parser.c" /* yacc.c:1646  */
    break;

  case 781:
#line 4845 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xdir) = OmpixDirective(OX_DCTASK, (yyvsp[-1].xcla));
    }
#line 9890 "parser.c" /* yacc.c:1646  */
    break;

  case 782:
#line 4852 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = NULL;
    }
#line 9898 "parser.c" /* yacc.c:1646  */
    break;

  case 783:
#line 4856 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixClauseList((yyvsp[-1].xcla), (yyvsp[0].xcla));
    }
#line 9906 "parser.c" /* yacc.c:1646  */
    break;

  case 784:
#line 4860 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixClauseList((yyvsp[-2].xcla), (yyvsp[0].xcla));
    }
#line 9914 "parser.c" /* yacc.c:1646  */
    break;

  case 785:
#line 4867 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixPlainClause(OX_OCATALL);
    }
#line 9922 "parser.c" /* yacc.c:1646  */
    break;

  case 786:
#line 4871 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixAtnodeClause((yyvsp[-1].expr));
    }
#line 9930 "parser.c" /* yacc.c:1646  */
    break;

  case 787:
#line 4875 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixAtworkerClause((yyvsp[-1].expr));
    }
#line 9938 "parser.c" /* yacc.c:1646  */
    break;

  case 788:
#line 4879 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixPlainClause(OX_OCTIED);
    }
#line 9946 "parser.c" /* yacc.c:1646  */
    break;

  case 789:
#line 4883 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixPlainClause(OX_OCUNTIED);
    }
#line 9954 "parser.c" /* yacc.c:1646  */
    break;

  case 790:
#line 4887 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixPlainClause(OX_OCDETACHED);
    }
#line 9962 "parser.c" /* yacc.c:1646  */
    break;

  case 791:
#line 4894 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = strcmp((yyvsp[-2].name), "main") ?
             FunctionCall(IdentName((yyvsp[-2].name)), NULL) :
             FunctionCall(IdentName(MAIN_NEWNAME), NULL);
    }
#line 9972 "parser.c" /* yacc.c:1646  */
    break;

  case 792:
#line 4900 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = strcmp((yyvsp[-3].name), "main") ?
             FunctionCall(IdentName((yyvsp[-3].name)), (yyvsp[-1].expr)) :
             FunctionCall(IdentName(MAIN_NEWNAME), (yyvsp[-1].expr));
    }
#line 9982 "parser.c" /* yacc.c:1646  */
    break;


#line 9986 "parser.c" /* yacc.c:1646  */
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
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



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
                      yytoken, &yylval);
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

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

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 4908 "parser.y" /* yacc.c:1906  */



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     CODE                                                      *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void yyerror(const char *s)
{
  fprintf(stderr, "(file %s, line %d, column %d):\n\t%s\n",
                  sc_original_file(), sc_original_line(), sc_column(), s);
}


/* Check whether the identifier is known or not
 */
void check_uknown_var(char *name)
{
  symbol s = Symbol(name);
  if (symtab_get(stab, s, IDNAME) == NULL &&
      symtab_get(stab, s, LABELNAME) == NULL &&
      symtab_get(stab, s, FUNCNAME) == NULL)
    parse_error(-1, "unknown identifier `%s'.\n", name);
}



/* See the "declaration" rule: if the last element of the list
 * is a user typename, we remove it, and we return it as an
 * identifier declarator.
 * The list should have 3 elements (typedef xxx type).
 */
astdecl fix_known_typename(astspec s)
{
  astspec prev;
  astdecl d;

  if (s->type != SPECLIST || s->u.next->type != SPECLIST) return (NULL);

  for (; s->u.next->type == SPECLIST; prev = s, s = s->u.next)
    ;   /* goto last list node */
  if (s->u.next->type != USERTYPE)         /* nope */
    return (NULL);

  prev->u.next = s->body;

  d = Declarator(NULL, IdentifierDecl(s->u.next->name));
  if (checkDecls)
    symtab_put(stab, s->u.next->name, TYPENAME);
  free(s);
  return (d);
}


void check_for_main_and_declare(astspec s, astdecl d)
{
  astdecl n = decl_getidentifier(d);

  assert(d->type == DECLARATOR);
  assert(d->decl->type == DFUNC);

  if (strcmp(n->u.id->name, "main") == 0)
  {
    n->u.id = Symbol(MAIN_NEWNAME);         /* Catch main()'s definition */
    hasMainfunc = 1;

    /* Now check for return type and # parameters */
    /* It d != NULL then its parameters is either (id or idlist) or
     * (paramtype or parmatypelist). If it is a list, assume the
     * standard 2 params, otherwise, we guess the single argument
     * must be the type "void" which means no params.
     * In any case, we always force main have (argc, argv[]).
     */
    if (d->decl->u.params == NULL || d->decl->u.params->type != DLIST)
      d->decl->u.params =
          ParamList(
            ParamDecl(
              Declspec(SPEC_int),
              Declarator( NULL, IdentifierDecl( Symbol("_argc_ignored") ) )
            ),
            ParamDecl(
              Declspec(SPEC_char),
              Declarator(Speclist_right( Pointer(), Pointer() ),
                         IdentifierDecl( Symbol("_argv_ignored") ))
            )
          );

    mainfuncRettype = 0; /* int */
    if (s != NULL)
    {
      for (; s->type == SPECLIST && s->subtype == SPEC_Rlist; s = s->u.next)
        if (s->body->type == SPEC && s->body->subtype == SPEC_void)
        {
          s = s->body;
          break;
        };
      if (s->type == SPEC && s->subtype == SPEC_void)
        mainfuncRettype = 1; /* void */
    }
  }
  if (symtab_get(stab, n->u.id, FUNCNAME) == NULL)/* From earlier declaration */
    symtab_put(stab, n->u.id, FUNCNAME);
}


/* For each variable/typename in the given declaration, add pointers in the
 * symbol table entries back to the declaration nodes.
 */
void add_declaration_links(astspec s, astdecl d)
{
  astdecl ini = NULL;

  if (d->type == DLIST && d->subtype == DECL_decllist)
  {
    add_declaration_links(s, d->u.next);
    d = d->decl;
  }
  if (d->type == DINIT) d = (ini = d)->decl;   /* Skip the initializer */
  assert(d->type == DECLARATOR);
  if (d->decl != NULL && d->decl->type != ABSDECLARATOR)
  {
    symbol  t = decl_getidentifier_symbol(d->decl);
    stentry e = isTypedef ?
                symtab_get(stab,t,TYPENAME) :
                symtab_get(stab,t,(decl_getkind(d)==DFUNC) ? FUNCNAME : IDNAME);
    e->spec  = s;
    e->decl  = d;
    e->idecl = ini;
  }
}


void parse_error(int exitvalue, char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  fprintf(stderr, "(%s, line %d)\n\t", sc_original_file(), sc_original_line());
  vfprintf(stderr, format, ap);
  va_end(ap);
  if (strcmp(sc_original_file(), "injected_code") == 0)
    fprintf(stderr, "\n>>>>>>>\n%s\n>>>>>>>\n", parsingstring);
  _exit(exitvalue);
}


void parse_warning(char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  fprintf(stderr, "[warning] ");
  vfprintf(stderr, format, ap);
  va_end(ap);
}


aststmt parse_file(char *fname, int *error)
{
  *error = 0;
  if ( (yyin = fopen(fname, "r")) == NULL )
    return (NULL);
  sc_set_filename(fname);      /* Inform the scanner */
  *error = yyparse();
  fclose(yyin);                /* No longer needed */
  return (pastree);
}


#define PARSE_STRING_SIZE 8192


astexpr parse_expression_string(char *format, ...)
{
  static char s[PARSE_STRING_SIZE];
  int    savecD;

  va_list ap;
  va_start(ap, format);
  vsnprintf(s, PARSE_STRING_SIZE-1, format, ap);
  va_end(ap);
  parsingstring = s;
  sc_scan_string(s);
  sc_set_start_token(START_SYMBOL_EXPRESSION);

  savecD = checkDecls;
  checkDecls = 0;         /* Don't check identifiers & don't declare them */
  yyparse();
  checkDecls = savecD;    /* Reset it */
  return ( pastree_expr );
}


aststmt parse_blocklist_string(char *format, ...)
{
  static char s[PARSE_STRING_SIZE];
  int    savecD;

  va_list ap;
  va_start(ap, format);
  vsnprintf(s, PARSE_STRING_SIZE-1, format, ap);
  va_end(ap);
  parsingstring = s;
  sc_scan_string(s);
  sc_set_start_token(START_SYMBOL_BLOCKLIST);

  savecD = checkDecls;
  checkDecls = 0;         /* Don't check identifiers & don't declare them */
  yyparse();
  checkDecls = savecD;    /* Reset it */
  return ( pastree_stmt );
}


aststmt parse_and_declare_blocklist_string(char *format, ...)
{
  static char s[PARSE_STRING_SIZE];
  int    savecD;

  va_list ap;
  va_start(ap, format);
  vsnprintf(s, PARSE_STRING_SIZE-1, format, ap);
  va_end(ap);
  parsingstring = s;
  sc_scan_string(s);
  sc_set_start_token(START_SYMBOL_BLOCKLIST);

  savecD = checkDecls;
  checkDecls = 1;         /* Do check identifiers & do declare them */
  yyparse();
  checkDecls = savecD;    /* Reset it */
  return ( pastree_stmt );
}


aststmt parse_transunit_string(char *format, ...)
{
  static char s[PARSE_STRING_SIZE];
  int    savecD;

  va_list ap;
  va_start(ap, format);
  vsnprintf(s, PARSE_STRING_SIZE-1, format, ap);
  va_end(ap);
  parsingstring = s;
  sc_scan_string(s);
  sc_set_start_token(START_SYMBOL_TRANSUNIT);

  savecD = checkDecls;
  checkDecls = 0;         /* Don't check identifiers & don't declare them */
  yyparse();
  checkDecls = savecD;    /* Reset it */
  return ( pastree_stmt );
}
