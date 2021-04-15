/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.4"

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
#include "ompi.h"
#include "ast.h"
#include "symtab.h"
#include "ast_free.h"
#include "ast_copy.h"
#include "ast_vars.h"
#include "ast_print.h"
#include "x_arith.h"
#include "x_clauses.h"
#include "str.h"

void    check_uknown_var(char *name);
void    parse_error(int exitvalue, char *format, ...);
void    parse_warning(char *format, ...);
void    yyerror(const char *s);
void    check_for_main_and_declare(astspec s, astdecl d);
void    add_declaration_links(astspec s, astdecl d);
astdecl fix_known_typename(astspec s);
void    check_schedule(ompclsubt_e sched, ompclmod_e mod);
char    *strdupcat(char *first, char *second, int freethem);


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

#line 150 "parser.c" /* yacc.c:339  */

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
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
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
    NOELSE = 258,
    ELSE = 259,
    START_SYMBOL_EXPRESSION = 260,
    START_SYMBOL_BLOCKLIST = 261,
    START_SYMBOL_TRANSUNIT = 262,
    IDENTIFIER = 263,
    TYPE_NAME = 264,
    CONSTANT = 265,
    STRING_LITERAL = 266,
    PTR_OP = 267,
    INC_OP = 268,
    DEC_OP = 269,
    LEFT_OP = 270,
    RIGHT_OP = 271,
    LE_OP = 272,
    GE_OP = 273,
    EQ_OP = 274,
    NE_OP = 275,
    AND_OP = 276,
    OR_OP = 277,
    MUL_ASSIGN = 278,
    DIV_ASSIGN = 279,
    MOD_ASSIGN = 280,
    ADD_ASSIGN = 281,
    SUB_ASSIGN = 282,
    LEFT_ASSIGN = 283,
    RIGHT_ASSIGN = 284,
    AND_ASSIGN = 285,
    XOR_ASSIGN = 286,
    OR_ASSIGN = 287,
    SIZEOF = 288,
    TYPEDEF = 289,
    EXTERN = 290,
    STATIC = 291,
    AUTO = 292,
    REGISTER = 293,
    RESTRICT = 294,
    CHAR = 295,
    SHORT = 296,
    INT = 297,
    LONG = 298,
    SIGNED = 299,
    UNSIGNED = 300,
    FLOAT = 301,
    DOUBLE = 302,
    CONST = 303,
    VOLATILE = 304,
    VOID = 305,
    INLINE = 306,
    UBOOL = 307,
    UCOMPLEX = 308,
    UIMAGINARY = 309,
    STRUCT = 310,
    UNION = 311,
    ENUM = 312,
    ELLIPSIS = 313,
    CASE = 314,
    DEFAULT = 315,
    IF = 316,
    SWITCH = 317,
    WHILE = 318,
    DO = 319,
    FOR = 320,
    GOTO = 321,
    CONTINUE = 322,
    BREAK = 323,
    RETURN = 324,
    __BUILTIN_VA_ARG = 325,
    __BUILTIN_OFFSETOF = 326,
    __BUILTIN_TYPES_COMPATIBLE_P = 327,
    __ATTRIBUTE__ = 328,
    __ASM__ = 329,
    PRAGMA_OTHER = 330,
    PRAGMA_OMP = 331,
    PRAGMA_OMP_THREADPRIVATE = 332,
    OMP_PARALLEL = 333,
    OMP_SECTIONS = 334,
    OMP_NOWAIT = 335,
    OMP_ORDERED = 336,
    OMP_SCHEDULE = 337,
    OMP_STATIC = 338,
    OMP_DYNAMIC = 339,
    OMP_GUIDED = 340,
    OMP_RUNTIME = 341,
    OMP_AUTO = 342,
    OMP_SECTION = 343,
    OMP_AFFINITY = 344,
    OMP_SINGLE = 345,
    OMP_MASTER = 346,
    OMP_CRITICAL = 347,
    OMP_BARRIER = 348,
    OMP_ATOMIC = 349,
    OMP_FLUSH = 350,
    OMP_PRIVATE = 351,
    OMP_FIRSTPRIVATE = 352,
    OMP_LASTPRIVATE = 353,
    OMP_SHARED = 354,
    OMP_DEFAULT = 355,
    OMP_NONE = 356,
    OMP_REDUCTION = 357,
    OMP_COPYIN = 358,
    OMP_NUMTHREADS = 359,
    OMP_COPYPRIVATE = 360,
    OMP_FOR = 361,
    OMP_IF = 362,
    OMP_TASK = 363,
    OMP_UNTIED = 364,
    OMP_TASKWAIT = 365,
    OMP_COLLAPSE = 366,
    OMP_FINAL = 367,
    OMP_MERGEABLE = 368,
    OMP_TASKYIELD = 369,
    OMP_READ = 370,
    OMP_WRITE = 371,
    OMP_CAPTURE = 372,
    OMP_UPDATE = 373,
    OMP_MIN = 374,
    OMP_MAX = 375,
    OMP_PROCBIND = 376,
    OMP_CLOSE = 377,
    OMP_SPREAD = 378,
    OMP_SIMD = 379,
    OMP_INBRANCH = 380,
    OMP_NOTINBRANCH = 381,
    OMP_UNIFORM = 382,
    OMP_LINEAR = 383,
    OMP_ALIGNED = 384,
    OMP_SIMDLEN = 385,
    OMP_SAFELEN = 386,
    OMP_DECLARE = 387,
    OMP_TARGET = 388,
    OMP_DATA = 389,
    OMP_DEVICE = 390,
    OMP_MAP = 391,
    OMP_ALLOC = 392,
    OMP_TO = 393,
    OMP_FROM = 394,
    OMP_TOFROM = 395,
    OMP_END = 396,
    OMP_TEAMS = 397,
    OMP_DISTRIBUTE = 398,
    OMP_NUMTEAMS = 399,
    OMP_THREADLIMIT = 400,
    OMP_DISTSCHEDULE = 401,
    OMP_DEPEND = 402,
    OMP_IN = 403,
    OMP_OUT = 404,
    OMP_INOUT = 405,
    OMP_TASKGROUP = 406,
    OMP_SEQ_CST = 407,
    OMP_CANCEL = 408,
    OMP_INITIALIZER = 409,
    PRAGMA_OMP_CANCELLATIONPOINT = 410,
    OMP_HINT = 411,
    OMP_SOURCE = 412,
    OMP_SINK = 413,
    OMP_RELEASE = 414,
    OMP_DELETE = 415,
    OMP_ALWAYS = 416,
    OMP_ENTER = 417,
    OMP_EXIT = 418,
    OMP_IS_DEVICE_PTR = 419,
    OMP_USE_DEVICE_PTR = 420,
    OMP_PRIORITY = 421,
    OMP_TASKLOOP = 422,
    OMP_THREADS = 423,
    OMP_LINK = 424,
    OMP_DEFAULTMAP = 425,
    OMP_SCALAR = 426,
    OMP_MONOTONIC = 427,
    OMP_NONMONOTONIC = 428,
    OMP_PRIMARY = 429,
    PRAGMA_OMPIX = 430,
    OMPIX_TASKDEF = 431,
    OMPIX_TASKSYNC = 432,
    OMPIX_UPONRETURN = 433,
    OMPIX_ATNODE = 434,
    OMPIX_DETACHED = 435,
    OMPIX_ATWORKER = 436,
    OMPIX_TASKSCHEDULE = 437,
    OMPIX_STRIDE = 438,
    OMPIX_START = 439,
    OMPIX_SCOPE = 440,
    OMPIX_NODES = 441,
    OMPIX_WORKERS = 442,
    OMPIX_LOCAL = 443,
    OMPIX_GLOBAL = 444,
    OMPIX_HERE = 445,
    OMPIX_REMOTE = 446,
    OMPIX_HINTS = 447,
    OMPIX_TIED = 448
  };
#endif
/* Tokens.  */
#define NOELSE 258
#define ELSE 259
#define START_SYMBOL_EXPRESSION 260
#define START_SYMBOL_BLOCKLIST 261
#define START_SYMBOL_TRANSUNIT 262
#define IDENTIFIER 263
#define TYPE_NAME 264
#define CONSTANT 265
#define STRING_LITERAL 266
#define PTR_OP 267
#define INC_OP 268
#define DEC_OP 269
#define LEFT_OP 270
#define RIGHT_OP 271
#define LE_OP 272
#define GE_OP 273
#define EQ_OP 274
#define NE_OP 275
#define AND_OP 276
#define OR_OP 277
#define MUL_ASSIGN 278
#define DIV_ASSIGN 279
#define MOD_ASSIGN 280
#define ADD_ASSIGN 281
#define SUB_ASSIGN 282
#define LEFT_ASSIGN 283
#define RIGHT_ASSIGN 284
#define AND_ASSIGN 285
#define XOR_ASSIGN 286
#define OR_ASSIGN 287
#define SIZEOF 288
#define TYPEDEF 289
#define EXTERN 290
#define STATIC 291
#define AUTO 292
#define REGISTER 293
#define RESTRICT 294
#define CHAR 295
#define SHORT 296
#define INT 297
#define LONG 298
#define SIGNED 299
#define UNSIGNED 300
#define FLOAT 301
#define DOUBLE 302
#define CONST 303
#define VOLATILE 304
#define VOID 305
#define INLINE 306
#define UBOOL 307
#define UCOMPLEX 308
#define UIMAGINARY 309
#define STRUCT 310
#define UNION 311
#define ENUM 312
#define ELLIPSIS 313
#define CASE 314
#define DEFAULT 315
#define IF 316
#define SWITCH 317
#define WHILE 318
#define DO 319
#define FOR 320
#define GOTO 321
#define CONTINUE 322
#define BREAK 323
#define RETURN 324
#define __BUILTIN_VA_ARG 325
#define __BUILTIN_OFFSETOF 326
#define __BUILTIN_TYPES_COMPATIBLE_P 327
#define __ATTRIBUTE__ 328
#define __ASM__ 329
#define PRAGMA_OTHER 330
#define PRAGMA_OMP 331
#define PRAGMA_OMP_THREADPRIVATE 332
#define OMP_PARALLEL 333
#define OMP_SECTIONS 334
#define OMP_NOWAIT 335
#define OMP_ORDERED 336
#define OMP_SCHEDULE 337
#define OMP_STATIC 338
#define OMP_DYNAMIC 339
#define OMP_GUIDED 340
#define OMP_RUNTIME 341
#define OMP_AUTO 342
#define OMP_SECTION 343
#define OMP_AFFINITY 344
#define OMP_SINGLE 345
#define OMP_MASTER 346
#define OMP_CRITICAL 347
#define OMP_BARRIER 348
#define OMP_ATOMIC 349
#define OMP_FLUSH 350
#define OMP_PRIVATE 351
#define OMP_FIRSTPRIVATE 352
#define OMP_LASTPRIVATE 353
#define OMP_SHARED 354
#define OMP_DEFAULT 355
#define OMP_NONE 356
#define OMP_REDUCTION 357
#define OMP_COPYIN 358
#define OMP_NUMTHREADS 359
#define OMP_COPYPRIVATE 360
#define OMP_FOR 361
#define OMP_IF 362
#define OMP_TASK 363
#define OMP_UNTIED 364
#define OMP_TASKWAIT 365
#define OMP_COLLAPSE 366
#define OMP_FINAL 367
#define OMP_MERGEABLE 368
#define OMP_TASKYIELD 369
#define OMP_READ 370
#define OMP_WRITE 371
#define OMP_CAPTURE 372
#define OMP_UPDATE 373
#define OMP_MIN 374
#define OMP_MAX 375
#define OMP_PROCBIND 376
#define OMP_CLOSE 377
#define OMP_SPREAD 378
#define OMP_SIMD 379
#define OMP_INBRANCH 380
#define OMP_NOTINBRANCH 381
#define OMP_UNIFORM 382
#define OMP_LINEAR 383
#define OMP_ALIGNED 384
#define OMP_SIMDLEN 385
#define OMP_SAFELEN 386
#define OMP_DECLARE 387
#define OMP_TARGET 388
#define OMP_DATA 389
#define OMP_DEVICE 390
#define OMP_MAP 391
#define OMP_ALLOC 392
#define OMP_TO 393
#define OMP_FROM 394
#define OMP_TOFROM 395
#define OMP_END 396
#define OMP_TEAMS 397
#define OMP_DISTRIBUTE 398
#define OMP_NUMTEAMS 399
#define OMP_THREADLIMIT 400
#define OMP_DISTSCHEDULE 401
#define OMP_DEPEND 402
#define OMP_IN 403
#define OMP_OUT 404
#define OMP_INOUT 405
#define OMP_TASKGROUP 406
#define OMP_SEQ_CST 407
#define OMP_CANCEL 408
#define OMP_INITIALIZER 409
#define PRAGMA_OMP_CANCELLATIONPOINT 410
#define OMP_HINT 411
#define OMP_SOURCE 412
#define OMP_SINK 413
#define OMP_RELEASE 414
#define OMP_DELETE 415
#define OMP_ALWAYS 416
#define OMP_ENTER 417
#define OMP_EXIT 418
#define OMP_IS_DEVICE_PTR 419
#define OMP_USE_DEVICE_PTR 420
#define OMP_PRIORITY 421
#define OMP_TASKLOOP 422
#define OMP_THREADS 423
#define OMP_LINK 424
#define OMP_DEFAULTMAP 425
#define OMP_SCALAR 426
#define OMP_MONOTONIC 427
#define OMP_NONMONOTONIC 428
#define OMP_PRIMARY 429
#define PRAGMA_OMPIX 430
#define OMPIX_TASKDEF 431
#define OMPIX_TASKSYNC 432
#define OMPIX_UPONRETURN 433
#define OMPIX_ATNODE 434
#define OMPIX_DETACHED 435
#define OMPIX_ATWORKER 436
#define OMPIX_TASKSCHEDULE 437
#define OMPIX_STRIDE 438
#define OMPIX_START 439
#define OMPIX_SCOPE 440
#define OMPIX_NODES 441
#define OMPIX_WORKERS 442
#define OMPIX_LOCAL 443
#define OMPIX_GLOBAL 444
#define OMPIX_HERE 445
#define OMPIX_REMOTE 446
#define OMPIX_HINTS 447
#define OMPIX_TIED 448

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 85 "parser.y" /* yacc.c:355  */

  char      name[2048];  /* A general string */
  int       type;        /* A general integer */
  char     *string;      /* A dynamically allocated string (only for 2 rules) */
  symbol    symb;        /* A symbol */
  astexpr   expr;        /* An expression node in the AST */
  astspec   spec;        /* A declaration specifier node in the AST */
  astdecl   decl;        /* A declarator node in the AST */
  aststmt   stmt;        /* A statement node in the AST */
  asmop     asmo;        /* An asm operand */
  ompcon    ocon;        /* An OpenMP construct */
  ompdir    odir;        /* An OpenMP directive */
  ompclause ocla;        /* An OpenMP clause */
  omparrdim oasd;        /* An array section dimension/slice */
  ompxli    oxli;        /* OpenMP extended list items (ids/array secitons) */

  oxcon     xcon;        /* OMPi extensions */
  oxdir     xdir;
  oxclause  xcla;

#line 597 "parser.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 614 "parser.c" /* yacc.c:358  */

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
#define YYFINAL  228
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4883

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  219
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  367
/* YYNRULES -- Number of rules.  */
#define YYNRULES  938
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1593

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   448

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     218,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   207,     2,     2,     2,   209,   202,     2,
     194,   195,   203,   204,   201,   205,   198,   208,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   215,   217,
     210,   216,   211,   214,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   196,     2,   197,   212,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   199,   213,   200,   206,     2,     2,     2,
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
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   543,   543,   544,   545,   546,   563,   584,   588,   611,
     633,   637,   641,   649,   653,   663,   670,   674,   678,   687,
     691,   695,   699,   703,   707,   716,   719,   723,   731,   735,
     739,   743,   750,   754,   763,   768,   773,   782,   786,   790,
     794,   798,   802,   810,   814,   822,   826,   830,   834,   842,
     846,   850,   858,   862,   866,   874,   878,   882,   886,   890,
     898,   902,   906,   914,   918,   926,   930,   938,   942,   950,
     954,   962,   966,   974,   978,   986,   990,   998,  1002,  1006,
    1010,  1014,  1018,  1022,  1026,  1030,  1034,  1038,  1046,  1050,
    1058,  1072,  1087,  1094,  1099,  1103,  1107,  1115,  1119,  1123,
    1127,  1131,  1135,  1139,  1143,  1151,  1155,  1169,  1186,  1185,
    1208,  1213,  1217,  1221,  1225,  1233,  1237,  1241,  1245,  1249,
    1253,  1257,  1261,  1265,  1269,  1273,  1277,  1281,  1285,  1289,
    1298,  1302,  1306,  1321,  1328,  1335,  1346,  1350,  1358,  1362,
    1370,  1374,  1382,  1386,  1390,  1394,  1402,  1406,  1414,  1418,
    1422,  1430,  1434,  1447,  1459,  1463,  1475,  1487,  1495,  1507,
    1511,  1519,  1523,  1531,  1535,  1539,  1543,  1551,  1559,  1563,
    1576,  1580,  1588,  1592,  1596,  1600,  1604,  1608,  1612,  1616,
    1620,  1624,  1628,  1632,  1640,  1644,  1648,  1652,  1660,  1664,
    1672,  1676,  1684,  1688,  1696,  1700,  1704,  1712,  1716,  1724,
    1728,  1736,  1740,  1744,  1752,  1756,  1760,  1764,  1768,  1772,
    1776,  1780,  1784,  1788,  1792,  1800,  1808,  1812,  1816,  1824,
    1828,  1832,  1836,  1844,  1852,  1856,  1864,  1868,  1872,  1886,
    1890,  1894,  1898,  1902,  1906,  1910,  1914,  1919,  1924,  1932,
    1936,  1946,  1950,  1954,  1962,  1966,  1966,  1977,  1981,  1990,
    1994,  1998,  2003,  2012,  2016,  2025,  2029,  2033,  2042,  2046,
    2050,  2054,  2058,  2062,  2066,  2070,  2074,  2078,  2082,  2086,
    2090,  2094,  2098,  2106,  2111,  2115,  2119,  2126,  2143,  2147,
    2155,  2159,  2166,  2170,  2182,  2183,  2188,  2187,  2214,  2213,
    2249,  2248,  2277,  2276,  2314,  2318,  2338,  2342,  2349,  2353,
    2357,  2361,  2365,  2372,  2376,  2383,  2384,  2388,  2391,  2395,
    2402,  2406,  2413,  2416,  2420,  2427,  2431,  2438,  2442,  2449,
    2453,  2466,  2469,  2476,  2480,  2487,  2494,  2498,  2515,  2518,
    2522,  2538,  2539,  2540,  2559,  2563,  2571,  2580,  2584,  2591,
    2595,  2599,  2603,  2607,  2611,  2615,  2619,  2623,  2627,  2632,
    2637,  2641,  2645,  2649,  2653,  2657,  2661,  2665,  2669,  2673,
    2677,  2681,  2685,  2689,  2693,  2697,  2701,  2705,  2709,  2714,
    2730,  2734,  2739,  2744,  2749,  2754,  2759,  2764,  2769,  2776,
    2783,  2791,  2799,  2802,  2806,  2813,  2817,  2821,  2825,  2829,
    2833,  2840,  2844,  2844,  2849,  2849,  2855,  2860,  2860,  2868,
    2875,  2883,  2886,  2890,  2897,  2901,  2905,  2909,  2913,  2917,
    2924,  2928,  2940,  2946,  2945,  2955,  2954,  2966,  2973,  2977,
    2981,  2985,  2989,  2993,  2998,  3001,  3005,  3012,  3019,  3027,
    3030,  3034,  3041,  3045,  3049,  3053,  3057,  3064,  3071,  3076,
    3080,  3087,  3094,  3101,  3109,  3112,  3116,  3123,  3127,  3131,
    3135,  3142,  3142,  3151,  3159,  3167,  3170,  3174,  3182,  3186,
    3190,  3194,  3198,  3206,  3218,  3222,  3230,  3234,  3242,  3242,
    3251,  3251,  3260,  3260,  3269,  3272,  3280,  3288,  3292,  3300,
    3308,  3311,  3315,  3324,  3336,  3340,  3344,  3348,  3356,  3364,
    3372,  3375,  3379,  3386,  3390,  3398,  3406,  3414,  3417,  3421,
    3428,  3432,  3440,  3448,  3456,  3459,  3463,  3470,  3474,  3482,
    3486,  3494,  3494,  3504,  3504,  3509,  3509,  3519,  3522,  3526,
    3534,  3538,  3542,  3546,  3550,  3554,  3562,  3562,  3571,  3580,
    3580,  3589,  3589,  3600,  3608,  3611,  3615,  3622,  3626,  3630,
    3634,  3638,  3642,  3649,  3653,  3661,  3665,  3673,  3683,  3687,
    3691,  3698,  3705,  3709,  3713,  3717,  3724,  3734,  3738,  3742,
    3749,  3757,  3761,  3765,  3769,  3777,  3784,  3788,  3792,  3799,
    3803,  3807,  3811,  3815,  3822,  3822,  3827,  3827,  3837,  3842,
    3849,  3856,  3864,  3869,  3877,  3881,  3885,  3893,  3893,  3899,
    3898,  3916,  3926,  3934,  3937,  3941,  3948,  3952,  3956,  3960,
    3964,  3968,  3977,  3977,  3983,  3983,  3992,  4000,  4008,  4011,
    4015,  4022,  4026,  4030,  4034,  4041,  4045,  4045,  4054,  4063,
    4071,  4074,  4078,  4085,  4089,  4093,  4101,  4110,  4118,  4121,
    4125,  4132,  4136,  4144,  4153,  4161,  4164,  4168,  4175,  4179,
    4187,  4195,  4203,  4206,  4210,  4217,  4221,  4225,  4229,  4233,
    4241,  4249,  4257,  4260,  4264,  4271,  4275,  4279,  4287,  4295,
    4303,  4306,  4310,  4317,  4321,  4325,  4329,  4337,  4345,  4353,
    4356,  4360,  4367,  4371,  4375,  4379,  4383,  4392,  4400,  4409,
    4412,  4417,  4425,  4429,  4433,  4437,  4441,  4449,  4458,  4467,
    4470,  4475,  4483,  4487,  4495,  4504,  4512,  4515,  4519,  4526,
    4530,  4538,  4547,  4555,  4558,  4562,  4569,  4573,  4581,  4590,
    4598,  4601,  4605,  4612,  4616,  4624,  4633,  4642,  4645,  4649,
    4657,  4661,  4665,  4669,  4673,  4677,  4685,  4689,  4689,  4694,
    4698,  4703,  4711,  4711,  4720,  4724,  4728,  4735,  4743,  4751,
    4754,  4758,  4765,  4769,  4773,  4777,  4781,  4785,  4789,  4793,
    4800,  4808,  4816,  4819,  4823,  4830,  4834,  4838,  4842,  4846,
    4850,  4854,  4861,  4868,  4875,  4882,  4886,  4891,  4898,  4906,
    4913,  4921,  4929,  4937,  4945,  4952,  4963,  4971,  4974,  4978,
    4982,  4986,  4995,  4998,  5005,  5009,  5016,  5016,  5024,  5028,
    5035,  5042,  5046,  5055,  5058,  5062,  5070,  5074,  5082,  5086,
    5090,  5098,  5106,  5110,  5117,  5123,  5129,  5139,  5143,  5147,
    5154,  5158,  5162,  5166,  5174,  5181,  5181,  5181,  5189,  5189,
    5189,  5196,  5203,  5207,  5211,  5215,  5219,  5223,  5227,  5231,
    5235,  5239,  5246,  5250,  5258,  5261,  5266,  5277,  5281,  5286,
    5293,  5293,  5301,  5301,  5309,  5309,  5317,  5317,  5325,  5325,
    5334,  5334,  5340,  5340,  5348,  5349,  5350,  5351,  5352,  5353,
    5354,  5355,  5359,  5374,  5381,  5391,  5395,  5406,  5413,  5427,
    5431,  5442,  5447,  5455,  5455,  5464,  5464,  5472,  5476,  5480,
    5484,  5488,  5496,  5500,  5504,  5508,  5520,  5538,  5571,  5575,
    5583,  5591,  5590,  5603,  5606,  5610,  5617,  5621,  5625,  5629,
    5633,  5640,  5644,  5648,  5652,  5659,  5663,  5672,  5671,  5688,
    5697,  5696,  5709,  5712,  5716,  5723,  5727,  5731,  5735,  5742,
    5746,  5753,  5758,  5766,  5774,  5783,  5791,  5794,  5798,  5805,
    5809,  5813,  5817,  5821,  5825,  5829,  5833,  5837,  5841
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NOELSE", "ELSE",
  "START_SYMBOL_EXPRESSION", "START_SYMBOL_BLOCKLIST",
  "START_SYMBOL_TRANSUNIT", "IDENTIFIER", "TYPE_NAME", "CONSTANT",
  "STRING_LITERAL", "PTR_OP", "INC_OP", "DEC_OP", "LEFT_OP", "RIGHT_OP",
  "LE_OP", "GE_OP", "EQ_OP", "NE_OP", "AND_OP", "OR_OP", "MUL_ASSIGN",
  "DIV_ASSIGN", "MOD_ASSIGN", "ADD_ASSIGN", "SUB_ASSIGN", "LEFT_ASSIGN",
  "RIGHT_ASSIGN", "AND_ASSIGN", "XOR_ASSIGN", "OR_ASSIGN", "SIZEOF",
  "TYPEDEF", "EXTERN", "STATIC", "AUTO", "REGISTER", "RESTRICT", "CHAR",
  "SHORT", "INT", "LONG", "SIGNED", "UNSIGNED", "FLOAT", "DOUBLE", "CONST",
  "VOLATILE", "VOID", "INLINE", "UBOOL", "UCOMPLEX", "UIMAGINARY",
  "STRUCT", "UNION", "ENUM", "ELLIPSIS", "CASE", "DEFAULT", "IF", "SWITCH",
  "WHILE", "DO", "FOR", "GOTO", "CONTINUE", "BREAK", "RETURN",
  "__BUILTIN_VA_ARG", "__BUILTIN_OFFSETOF", "__BUILTIN_TYPES_COMPATIBLE_P",
  "__ATTRIBUTE__", "__ASM__", "PRAGMA_OTHER", "PRAGMA_OMP",
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
  "PRAGMA_OMP_CANCELLATIONPOINT", "OMP_HINT", "OMP_SOURCE", "OMP_SINK",
  "OMP_RELEASE", "OMP_DELETE", "OMP_ALWAYS", "OMP_ENTER", "OMP_EXIT",
  "OMP_IS_DEVICE_PTR", "OMP_USE_DEVICE_PTR", "OMP_PRIORITY",
  "OMP_TASKLOOP", "OMP_THREADS", "OMP_LINK", "OMP_DEFAULTMAP",
  "OMP_SCALAR", "OMP_MONOTONIC", "OMP_NONMONOTONIC", "OMP_PRIMARY",
  "PRAGMA_OMPIX", "OMPIX_TASKDEF", "OMPIX_TASKSYNC", "OMPIX_UPONRETURN",
  "OMPIX_ATNODE", "OMPIX_DETACHED", "OMPIX_ATWORKER", "OMPIX_TASKSCHEDULE",
  "OMPIX_STRIDE", "OMPIX_START", "OMPIX_SCOPE", "OMPIX_NODES",
  "OMPIX_WORKERS", "OMPIX_LOCAL", "OMPIX_GLOBAL", "OMPIX_HERE",
  "OMPIX_REMOTE", "OMPIX_HINTS", "OMPIX_TIED", "'('", "')'", "'['", "']'",
  "'.'", "'{'", "'}'", "','", "'&'", "'*'", "'+'", "'-'", "'~'", "'!'",
  "'/'", "'%'", "'<'", "'>'", "'^'", "'|'", "'?'", "':'", "'='", "';'",
  "'\\n'", "$accept", "start_trick", "enumeration_constant",
  "string_literal", "primary_expression", "postfix_expression",
  "argument_expression_list", "unary_expression", "unary_operator",
  "cast_expression", "multiplicative_expression", "additive_expression",
  "shift_expression", "relational_expression", "equality_expression",
  "AND_expression", "exclusive_OR_expression", "inclusive_OR_expression",
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
  "asm_statement", "asm_stmtrest", "asm_qualifiers", "asm_qualifier",
  "asm_output", "asm_outoperand", "asm_input", "asm_inoperand",
  "asm_clobbers", "labellist", "attribute_optseq", "attribute_seq",
  "attribute", "attribute_name_list", "attribute_name", "attr_name",
  "declaration_definition", "function_statement",
  "declarations_definitions_seq", "openmp_construct", "openmp_directive",
  "structured_block", "parallel_construct", "parallel_directive",
  "parallel_clause_optseq", "parallel_clause", "unique_parallel_clause",
  "$@7", "$@8", "$@9", "for_construct", "for_directive",
  "for_clause_optseq", "for_clause", "unique_for_clause", "$@10", "$@11",
  "schedule_kind", "schedule_mod", "sections_construct",
  "sections_directive", "sections_clause_optseq", "sections_clause",
  "section_scope", "section_sequence", "section_directive",
  "single_construct", "single_directive", "single_clause_optseq",
  "single_clause", "unique_single_clause", "$@12", "simd_construct",
  "simd_directive", "simd_clause_optseq", "simd_clause",
  "unique_simd_clause", "inbranch_clause", "uniform_clause", "$@13",
  "linear_clause", "$@14", "aligned_clause", "$@15", "optional_expression",
  "declare_simd_construct", "declare_simd_directive_seq",
  "declare_simd_directive", "declare_simd_clause_optseq",
  "declare_simd_clause", "for_simd_construct", "for_simd_directive",
  "for_simd_clause_optseq", "for_simd_clause",
  "parallel_for_simd_construct", "parallel_for_simd_directive",
  "parallel_for_simd_clause_optseq", "parallel_for_simd_clause",
  "target_data_construct", "target_data_directive",
  "target_data_clause_optseq", "target_data_clause", "device_clause",
  "$@16", "map_clause", "$@17", "$@18", "map_modifier", "map_type",
  "use_device_ptr_clause", "$@19", "defaultmap_clause",
  "is_device_ptr_clause", "$@20", "target_construct", "@21",
  "target_directive", "target_clause_optseq", "target_clause",
  "unique_target_clause", "target_enter_data_directive",
  "target_enter_data_clause_seq", "target_enter_data_clause",
  "target_exit_data_directive", "target_exit_data_clause_seq",
  "target_exit_data_clause", "target_update_directive",
  "target_update_clause_seq", "target_update_clause", "motion_clause",
  "$@22", "$@23", "declare_target_construct", "declare_target_directive",
  "end_declare_target_directive", "declare_target_directive_v45",
  "declare_target_clause_optseq", "unique_declare_target_clause", "$@24",
  "$@25", "teams_construct", "teams_directive", "teams_clause_optseq",
  "teams_clause", "unique_teams_clause", "$@26", "$@27",
  "distribute_construct", "distribute_directive",
  "distribute_clause_optseq", "distribute_clause",
  "unique_distribute_clause", "$@28", "distribute_simd_construct",
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
  "unique_task_clause", "$@29", "depend_clause", "$@30", "dependence_type",
  "parallel_for_construct", "parallel_for_directive",
  "parallel_for_clause_optseq", "parallel_for_clause",
  "parallel_sections_construct", "parallel_sections_directive",
  "parallel_sections_clause_optseq", "parallel_sections_clause",
  "master_construct", "master_directive", "critical_construct",
  "critical_directive", "region_phrase", "hint_clause",
  "barrier_directive", "taskwait_directive", "taskgroup_construct",
  "taskgroup_directive", "taskyield_directive", "atomic_construct",
  "atomic_directive", "atomic_clause_opt", "seq_cst_clause_opt",
  "flush_directive", "flush_vars", "$@31", "ordered_construct",
  "ordered_directive_full", "ordered_directive_standalone",
  "ordered_clause_optseq_full", "ordered_clause_type_full",
  "ordered_clause_optseq_standalone", "ordered_clause_depend_sink",
  "sink_vec", "sink_vec_elem", "cancel_directive", "construct_type_clause",
  "cancellation_point_directive", "threadprivate_directive", "$@32",
  "$@33", "declare_reduction_directive", "$@34", "$@35",
  "reduction_identifier", "reduction_type_list", "initializer_clause_opt",
  "data_default_clause", "data_privatization_clause", "$@36",
  "data_privatization_in_clause", "$@37", "data_privatization_out_clause",
  "$@38", "data_sharing_clause", "$@39", "data_reduction_clause", "$@40",
  "if_clause", "$@41", "$@42", "if_related_construct", "collapse_clause",
  "variable_list", "variable_array_section_list", "varid_or_array_section",
  "funcname_variable_array_section_list", "funcvarid_or_array_section",
  "array_section_slice_list", "$@43", "$@44", "array_section_slice",
  "procbind_clause", "thrprv_variable_list", "ompix_directive",
  "ox_tasksync_directive", "ox_taskschedule_directive", "$@45",
  "ox_taskschedule_clause_optseq", "ox_taskschedule_clause",
  "ox_scope_spec", "ompix_construct", "ox_taskdef_construct", "$@46",
  "ox_taskdef_directive", "$@47", "ox_taskdef_clause_optseq",
  "ox_taskdef_clause", "ox_variable_size_list", "ox_variable_size_elem",
  "ox_task_construct", "ox_task_directive", "ox_task_clause_optseq",
  "ox_task_clause", YY_NULLPTR
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
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,    40,    41,    91,    93,    46,   123,
     125,    44,    38,    42,    43,    45,   126,    33,    47,    37,
      60,    62,    94,   124,    63,    58,    61,    59,    10
};
# endif

#define YYPACT_NINF -1301

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1301)))

#define YYTABLE_NINF -908

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    2869,  2694,  1619,  2921, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,    50,
     -43, -1301,    68, -1301,    43,    86,   396,   361, -1301,    70,
    4687,  4687, -1301,    50, -1301,  4687,  4687,   196,   627,    74,
   -1301,  2921, -1301, -1301, -1301, -1301, -1301, -1301,  3143, -1301,
   -1301,  2064, -1301, -1301, -1301, -1301,  3193,   214, -1301, -1301,
    2734,  2734,  2799,   235,   276,   286,  2122, -1301, -1301, -1301,
   -1301, -1301, -1301,   430, -1301,   412,   510,  2694, -1301,   461,
     245,   719,   141,   770,   329,   298,   314,   529,    31, -1301,
   -1301,   352,    -5,  2694,   343,   426,   429,   433,  1787,   444,
     646,   456,   460,  1022,   370, -1301,  4231,   305,   308,   482,
   -1301,  -104, -1301,    70, -1301, -1301, -1301,  1619, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,  1787, -1301,
     620, -1301,   507, -1301,  1787, -1301,   620, -1301,   620, -1301,
     620, -1301,  1787, -1301, -1301, -1301, -1301, -1301, -1301,  1787,
   -1301,   620, -1301,   620, -1301,   620, -1301,  1787, -1301,  1787,
   -1301,   620, -1301,   620, -1301,   620, -1301,   620, -1301,   620,
   -1301,   620, -1301,   620, -1301,   620, -1301,  1787, -1301,   620,
   -1301,   507, -1301,  1787, -1301,  1787, -1301, -1301, -1301,  1787,
   -1301, -1301,  1288, -1301, -1301,  1787, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301,   708,  2921,    98,    50, -1301,
     527,   458,   548, -1301,   558, -1301, -1301,   396, -1301, -1301,
     -91, -1301,   697, -1301, -1301,   144, -1301, -1301,   559,  4443,
    3549,  2172,   627, -1301,   630,    86, -1301, -1301, -1301, -1301,
   -1301, -1301,  2973,    86, -1301,   572,  2694,  2122, -1301, -1301,
    2122, -1301,  2694,  4709,  4709,   -32,  4709,   655,  4709,   587,
   -1301,   803, -1301, -1301,  2694,  2694,   826, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,  2694, -1301,
   -1301,  2694,  2694,  2694,  2694,  2694,  2694,  2694,  2694,  2694,
    2694,  2694,  2694,  2694,  2694,  2694,  2694,  2694,  2694,  2694,
    2694,    50, -1301,   571,  1717,  2694,  2694,  2694,  4323,     1,
     739,  1328,   603, -1301, -1301, -1301,   -67, -1301, -1301,   652,
     864, -1301,   370, -1301,    24, -1301,   736, -1301,   668,   -90,
     677,  1382,   -86,   776, -1301,   698,   700, -1301,   853,   787,
      14,   752,   305, -1301, -1301, -1301, -1301,   759, -1301,   778,
   -1301, -1301,  1619, -1301,   716, -1301, -1301, -1301, -1301,  1876,
   -1301, -1301, -1301, -1301, -1301, -1301,  1787, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,   773,
     789,   997,   830, -1301,   685,   827, -1301,   -21,  1033, -1301,
   -1301, -1301, -1301,    86, -1301, -1301,   559,  4443,   873,  3310,
     887, -1301, -1301,  3479, -1301, -1301,   159,   857,   856, -1301,
     205,  2216, -1301,   877,   901,  2189,   986,   918,   256, -1301,
   -1301, -1301,   559,   277, -1301,   925,   934,   930,   935,   942,
   -1301, -1301,  3261,  2354,   678, -1301,   688, -1301,  2440, -1301,
   -1301,   624,    15, -1301, -1301, -1301, -1301, -1301, -1301,   461,
     461,   245,   245,   719,   719,   719,   719,   141,   141,   770,
     329,   298,   314,   529,   197, -1301,  1787,  1717,  4297, -1301,
   -1301, -1301,   653,   684,   692,    -9,   961,  1700,   -56,  1794,
   -1301, -1301,   864,    56, -1301, -1301, -1301,  1035,  4004,   427,
     969,   275,   270, -1301,   403, -1301,  1162, -1301,   -36, -1301,
   -1301, -1301, -1301, -1301,  1020, -1301, -1301,   993, -1301,   734,
    1319, -1301, -1301,   742,   875, -1301,  1053,  1086,  1097,  1306,
      17,  1868,  1109, -1301,   810, -1301,   273, -1301,   325, -1301,
   -1301,  1549,  4318, -1301,     9,  1787, -1301,  2694,   997, -1301,
    1019,   662, -1301,   997, -1301, -1301, -1301,   704, -1301,  1059,
    1056,   449, -1301, -1301,  1247, -1301,   -47, -1301, -1301,   714,
     599, -1301,  2482, -1301,  3479,  4709, -1301,  3359, -1301,    85,
    4709, -1301, -1301,  3091, -1301,   164, -1301, -1301,  4637, -1301,
    1256,  1077,  2216, -1301, -1301,  2694, -1301,  1103,  1107,  1175,
   -1301, -1301,  2694,  1084,  1084,  4709,  1381,  4709, -1301,  1198,
    1200, -1301,  1223,  1226,   688,  3598,  2512,  2282, -1301, -1301,
   -1301,  2694, -1301, -1301,  1787,  1787,  1787,  2694,  2529,   247,
    1827,  2555,   250,    57, -1301,   167,  2575, -1301,  3978, -1301,
   -1301, -1301, -1301,  1203,  1240, -1301,  1255,  1258,  1266,  2517,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301,   929, -1301, -1301, -1301, -1301, -1301, -1301,   849,
   -1301, -1301,   -37, -1301, -1301,  1269,  1300, -1301, -1301, -1301,
   -1301,   663, -1301, -1301, -1301, -1301, -1301,  1280,  1285, -1301,
    1271, -1301,  1283,  1479, -1301,   985, -1301,  1309,  1314,  1315,
    1468, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
    1316, -1301,  1318,  1320,  3108, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301,  1321,   560, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,  1322, -1301,
   -1301, -1301,  1074, -1301, -1301, -1301, -1301,   351,    18,  3662,
     525,   561, -1301,  1323,  1325,  1327,   988, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,  1398, -1301,
    1310,  1329,  1331,  1620, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301,  1402,  2641,  1335,   520, -1301, -1301, -1301, -1301,
   -1301, -1301,  1429, -1301,  1326,  1352, -1301,  1353,  1361, -1301,
    1362, -1301,   594, -1301, -1301,   676, -1301,  1343,  1480, -1301,
    1787, -1301,   749,   946,  2694, -1301,    53,  1018,  1374,   685,
    2694, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301,  1356, -1301, -1301, -1301,  1378,  1413, -1301, -1301,
   -1301, -1301, -1301, -1301,  1379,  1380,  1384,   767, -1301,   -40,
   -1301, -1301, -1301,  1567,  1383,  1387,  1437,  1440,   312, -1301,
   -1301,  2282, -1301, -1301, -1301,  3381, -1301, -1301,  2694, -1301,
     272, -1301,  1420,  3430, -1301, -1301, -1301, -1301,  1439,  1441,
   -1301, -1301,  1443, -1301,  1442,  1444,  1445, -1301, -1301, -1301,
   -1301, -1301,  1446, -1301,  1447,  1448,  2694,  1254, -1301,  1043,
    2482,   189, -1301, -1301,  1638, -1301, -1301,   779,  1787,   780,
    2579,  2595,   364,  1787,   783,  2622,  1428,  1639,    72,   418,
   -1301,  1228, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301,  3878,  4762, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301,  1452,  1454,  1455,  1456,   696,  1056,  1457,
   -1301,   493,    33, -1301,  1483, -1301,  1504,  1487, -1301,  1547,
   -1301,  1512, -1301, -1301,  2694, -1301, -1301, -1301,   796,  4069,
   -1301, -1301, -1301, -1301,  2694,    13,  2694, -1301, -1301,   792,
    2694, -1301,  1513,  1515,  2694, -1301, -1301,  1521,  1524,   875,
   -1301, -1301,  1527,   592, -1301, -1301, -1301, -1301, -1301, -1301,
    1617, -1301,  3590, -1301,  1150, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301,   545, -1301, -1301, -1301, -1301, -1301,
   -1301,   944, -1301, -1301, -1301,   385, -1301,  1586, -1301,  1605,
    4016,  1601, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301,  3951,  1912, -1301, -1301, -1301, -1301, -1301,  1649, -1301,
    1516, -1301,  2694,  2694,  2327,  2694, -1301, -1301,  1541,  1542,
    1543, -1301,   683, -1301, -1301, -1301, -1301,  1522, -1301,    91,
   -1301, -1301, -1301, -1301,   106, -1301, -1301,   797,  3064,  1544,
    2694, -1301,  1247,  1732, -1301,  1545,  1526,  1247, -1301,  1539,
   -1301,  1056,  1734,  1734,  1734, -1301,  1071, -1301, -1301,   139,
   -1301,  2694, -1301, -1301, -1301,  1540, -1301, -1301, -1301, -1301,
   -1301, -1301,  1562, -1301, -1301, -1301,  2240, -1301, -1301, -1301,
    1787,  1546, -1301,  1787,  1787,   822,  1787,   839,  2650, -1301,
    1787,  1787,   844,   172,  1563,  2734, -1301,   167,   172, -1301,
    4688, -1301, -1301, -1301, -1301, -1301,  1479,  1479,  1479,  1479,
    1566,  1572,  1573, -1301,  1479,  2694, -1301, -1301,   636, -1301,
    2694,  1554,  1578,  1580,  1595,  1604,  1479,  1585,  1754,  1479,
     848, -1301,  1788, -1301,   855,  1608,  1591,  1596,  1194,   858,
    2694, -1301, -1301, -1301, -1301,   902,  1479,  1479,   906,  2694,
    1732,  1732, -1301, -1301, -1301,  1686,  3577, -1301,  3675, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301,   525, -1301, -1301,   561,
   -1301, -1301,  1611,  1732,  1028,  1479,  1599, -1301,  3923,  2737,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301,  2694,  2694,  2955,
    4524, -1301, -1301, -1301, -1301, -1301,   907, -1301,   911,   914,
    1621,  1622,  1624,  1633,  1634,  2694,  2694,  1122, -1301, -1301,
   -1301, -1301, -1301, -1301,   222,  1479,   924,   938,  1384,   955,
   -1301,  1945, -1301, -1301, -1301, -1301,  1600,  1635,   956, -1301,
     978,   979, -1301,  2261, -1301, -1301, -1301, -1301, -1301, -1301,
    2482, -1301, -1301, -1301, -1301,  1787, -1301,  1787,  1787,   996,
   -1301, -1301,  1787,  1822,   135,   392, -1301,   864,  1637, -1301,
     464, -1301,   998,  1005,  1006,  1009, -1301, -1301, -1301,  1618,
    1013,  1037, -1301, -1301,  1705,  1708,  1039, -1301, -1301, -1301,
   -1301, -1301,  1047, -1301,  1129,  1648,  1643,  1081, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
    1089, -1301,  1092,  1630, -1301,   514,   514, -1301,  1116,  1117,
    1121,  1479, -1301,  3800, -1301,  4057, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301,  1128, -1301, -1301, -1301,
   -1301, -1301, -1301,  1652,  1151,  1663,  3827,  4497, -1301, -1301,
   -1301, -1301, -1301,  1152,  1156,  4461, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
    1674,  1680, -1301,  1675,  1682,  3064, -1301,  1193, -1301, -1301,
    1545, -1301,  1732,  2694,   551,  1681,  1945,  1734,  1860, -1301,
    1734, -1301, -1301, -1301, -1301, -1301, -1301, -1301,  1787, -1301,
    1683,  2694,   172,   864,   180, -1301, -1301,   864, -1301, -1301,
   -1301, -1301,  1732, -1301, -1301, -1301, -1301, -1301,  2694, -1301,
    2694,  2694, -1301,  1754, -1301,  2694, -1301, -1301, -1301,  1732,
    2694,  1684,  1687, -1301, -1301, -1301,  1195,  3731,  4184, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301,  1688,  4425, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301,  2694, -1301, -1301,
    1155, -1301, -1301,  2694, -1301, -1301,   352,  2694, -1301,  1691,
    1213,  1873,  1694, -1301, -1301,   864,  1216, -1301,   430,   579,
    2734,  1232,  1234,  1235,   461,   461, -1301,  1242,  2694,  1243,
     352, -1301, -1301, -1301,  4133, -1301, -1301, -1301, -1301, -1301,
    1732, -1301, -1301,  1244, -1301, -1301,  1264,   352, -1301, -1301,
    1698, -1301,   181, -1301,   864,  1877,  1701, -1301, -1301, -1301,
   -1301,  1267, -1301, -1301,  1272, -1301, -1301, -1301,  2694,   430,
   -1301,  1276, -1301, -1301, -1301,  1746,  1277, -1301,  1893,  1714,
    1692, -1301, -1301,  1904, -1301,   -79,  2694,  2694,  1279,  1718,
    1719, -1301, -1301
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,     0,     0,   170,   215,   110,   111,   112,   113,
     114,   164,   116,   117,   118,   119,   122,   123,   120,   121,
     163,   165,   115,   167,   124,   125,   126,   136,   137,   321,
       0,   283,     0,   815,     0,     0,   184,     0,   281,     0,
      97,    99,   127,   321,   128,   101,   103,   292,   168,     0,
     129,     2,   278,   280,   284,   285,   166,    94,     0,   477,
      95,     0,   579,    93,    96,   282,     0,     9,    10,     7,
       0,     0,     0,     0,     0,     0,     0,    37,    38,    39,
      40,    41,    42,    11,    13,    28,    43,     0,    45,    49,
      52,    55,    60,    63,    65,    67,    69,    71,    73,    75,
      88,     3,     9,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   238,     0,     0,     0,   245,
     253,     0,   249,     0,   250,   229,   230,     4,   247,   231,
     232,   233,   260,   234,   235,   236,   251,   339,     0,   340,
       0,   341,     0,   342,     0,   350,     0,   351,     0,   352,
       0,   353,     0,   354,   531,   377,   378,   376,   355,     0,
     356,     0,   357,     0,   358,     0,   359,     0,   360,     0,
     361,     0,   362,     0,   363,     0,   364,     0,   365,     0,
     366,     0,   367,     0,   368,     0,   349,     0,   343,     0,
     344,     0,   345,     0,   346,     0,   370,   372,   369,     0,
     373,   347,     0,   371,   348,     0,   789,   374,   375,   252,
     888,   889,   237,   905,   906,     0,     5,     0,   322,   323,
       0,     0,     0,   910,     0,   188,   186,   185,     1,    91,
       0,   105,   290,    98,   100,     0,   102,   104,     0,     0,
       0,     0,   169,   279,     0,     0,   336,   476,   478,   335,
     334,   337,     0,     0,   288,   909,    25,     0,    29,    30,
       0,    32,     0,     0,     0,     0,   142,   199,   144,     0,
       8,     0,    21,    22,    25,     0,     0,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    77,     0,    43,
      31,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   321,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   274,   275,   276,     0,   305,   306,     0,
       0,   296,     0,   303,   382,   429,   793,   444,     0,     0,
       0,   777,     0,   401,   717,     0,     0,   455,   534,   593,
     608,     0,     0,   810,   811,   812,   813,     0,   926,     0,
     891,   244,     0,   254,   107,   248,   379,   380,   399,     0,
     427,   442,   453,   488,   495,   502,     0,   591,   606,   618,
     626,   633,   640,   650,   658,   667,   677,   687,   694,   701,
     708,   715,   737,   750,   762,   764,   772,   775,   788,     0,
     157,     0,   158,   324,   328,     0,   480,     0,     0,   912,
     171,   189,   187,     0,    92,   108,     0,     0,   134,     0,
     135,   289,   294,     0,   197,   182,   195,     0,   190,   192,
       0,     0,   172,    38,     0,     0,     0,   290,     0,   338,
     578,   286,     0,     0,    26,     0,     0,     0,     0,     0,
      12,   143,     0,     0,   201,   200,   202,   145,     0,    18,
      20,     0,     0,    17,    19,    76,    46,    47,    48,    50,
      51,    53,    54,    58,    59,    56,    57,    61,    62,    64,
      66,    68,    70,    72,     0,    89,     0,     0,     0,   239,
     243,   240,     0,     0,     0,   534,     0,     0,     0,     0,
     273,   277,     0,     0,   297,   304,   752,   739,     0,     0,
       0,     0,     0,   798,     0,   763,     0,   765,     0,   770,
     778,   779,   781,   780,   782,   786,   784,     0,   490,     0,
       0,   771,   774,     0,     0,   504,   642,     0,     0,     0,
     652,     0,     0,   620,     0,   773,     0,   814,     0,   890,
     893,     0,     0,   438,     0,     0,   532,    25,     0,     6,
     161,     0,   159,     0,   331,   332,   333,     0,   326,   329,
       0,     0,   587,   589,     0,   580,     0,   584,   886,     0,
       0,   106,     0,   287,     0,     0,   131,     0,   138,     0,
       0,   295,   293,     0,   194,   201,   196,   181,     0,   183,
       0,     0,     0,   179,   174,     0,   173,    38,     0,     0,
     908,    15,     0,     0,    33,     0,     0,     0,   211,     0,
       0,   205,    38,     0,   203,     0,     0,     0,    44,    16,
      14,     0,   241,   242,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   298,   307,     0,   497,     0,   397,
     840,   842,   846,     0,     0,   394,     0,     0,     0,     0,
     381,   383,   385,   386,   387,   388,   389,   390,   391,   396,
     436,   844,     0,   428,   430,   432,   433,   434,   435,     0,
     797,   796,     0,   790,   794,     0,     0,   792,   799,   450,
     451,     0,   443,   445,   447,   448,   449,     0,     0,   766,
       0,   783,     0,     0,   785,     0,   409,   410,     0,     0,
       0,   400,   402,   404,   405,   406,   407,   408,   417,   729,
       0,   730,     0,     0,     0,   716,   718,   720,   725,   721,
     722,   723,   724,   726,   470,   472,     0,     0,   454,   456,
     458,   464,   465,   459,   460,   461,   462,   573,     0,   574,
     576,   570,     0,   566,   569,   572,   571,     0,   669,     0,
       0,     0,   540,     0,     0,     0,     0,   533,   543,   544,
     545,   546,   535,   537,   539,   541,   542,   538,     0,   660,
       0,     0,     0,     0,   592,   594,   596,   597,   598,   599,
     600,   601,   628,     0,     0,     0,   607,   609,   614,   611,
     612,   613,     0,   807,     0,     0,   935,     0,     0,   936,
       0,   934,     0,   925,   927,     0,   246,     0,     0,   437,
       0,   439,     0,     0,     0,   151,     0,     0,     0,   328,
      25,   821,   828,   829,   830,   831,   825,   823,   822,   824,
     826,   827,     0,   466,   467,   468,     0,     0,   479,   487,
     486,   484,   485,   481,     0,     0,   871,     0,   869,     0,
     583,   585,   816,     0,     0,     0,     0,     0,     0,   911,
     913,     0,   216,   109,   291,     0,   130,   139,     0,   141,
       0,   146,   148,     0,   191,   193,   198,   176,     0,     0,
     180,   175,     0,    27,     0,     0,     0,   213,   204,   209,
     207,   212,     0,   206,    38,     0,     0,     0,   219,     0,
       0,     0,   224,    74,   255,   257,   258,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     308,     0,   751,   755,   753,   756,   757,   758,   759,   760,
     761,     0,     0,   738,   742,   743,   740,   744,   745,   746,
     747,   748,   749,     0,     0,     0,     0,     0,     0,     0,
     392,   850,     0,   384,     0,   431,     0,     0,   795,     0,
     800,     0,   446,   768,     0,   767,   776,   863,     0,     0,
     489,   493,   494,   491,     0,   424,     0,   403,   727,     0,
       0,   719,     0,     0,     0,   457,   511,     0,     0,     0,
     565,   567,     0,     0,   503,   505,   507,   508,   510,   509,
       0,   679,     0,   649,     0,   641,   646,   645,   643,   648,
     647,   555,   552,   551,     0,   548,   554,   553,   564,   561,
     560,     0,   557,   563,   562,   517,   529,     0,   536,   689,
       0,     0,   651,   655,   656,   653,   657,   602,   604,   595,
     635,     0,     0,   619,   625,   623,   621,   624,     0,   610,
       0,   808,     0,     0,     0,     0,   928,   900,     0,     0,
       0,   899,     0,   892,   894,   441,   440,     0,   152,     0,
     162,   154,   160,   153,     0,   325,   327,     0,     0,     0,
       0,   482,     0,     0,   875,   872,     0,     0,   586,     0,
     887,     0,     0,     0,     0,   914,     0,   132,   150,     0,
     140,     0,   133,   177,   178,     0,    34,    35,    36,   214,
     210,   208,     0,   227,   228,    23,     0,   220,   223,   225,
       0,     0,   261,     0,     0,     0,     0,     0,     0,   269,
       0,     0,     0,   312,     0,     0,   299,     0,   312,   754,
       0,   496,   501,   498,   500,   741,     0,     0,     0,     0,
       0,     0,     0,   848,     0,     0,   854,   855,   856,   861,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   787,     0,   492,     0,     0,     0,     0,     0,     0,
       0,   734,   735,   736,   732,     0,     0,     0,     0,     0,
       0,     0,   568,   526,   506,   696,     0,   676,     0,   668,
     673,   672,   670,   675,   674,   644,     0,   547,   549,     0,
     556,   558,   518,     0,     0,     0,     0,   703,     0,     0,
     659,   663,   666,   661,   664,   665,   654,     0,     0,     0,
       0,   627,   631,   629,   632,   622,     0,   809,     0,     0,
       0,     0,    38,     0,     0,     0,     0,     0,   895,   924,
     155,   156,   330,   832,     0,     0,     0,     0,   867,     0,
     865,     0,   873,   582,   870,   817,     0,   921,     0,   919,
       0,     0,   217,     0,   147,   149,   581,   226,    24,   221,
       0,   256,   259,   264,   263,     0,   262,     0,     0,     0,
     271,   270,     0,     0,     0,     0,   313,     0,     0,   309,
       0,   499,     0,     0,     0,     0,   839,   837,   838,     0,
       0,     0,   860,   857,     0,     0,     0,   852,   882,   884,
     885,   883,     0,   791,   804,     0,   802,     0,   769,   864,
     411,   415,   425,   426,   423,   418,   419,   420,   421,   422,
       0,   862,     0,     0,   731,   474,   474,   463,     0,     0,
       0,     0,   710,     0,   686,     0,   678,   683,   682,   680,
     685,   684,   671,   550,   559,   519,     0,   520,   521,   522,
     523,   524,   525,     0,     0,     0,     0,     0,   688,   692,
     693,   690,   662,     0,     0,     0,   634,   639,   638,   636,
     630,   615,   616,   938,   937,   931,   932,   929,   930,   933,
       0,     0,   901,   902,     0,     0,   818,     0,   483,   588,
     868,   590,     0,   881,   879,     0,     0,     0,     0,   915,
       0,   916,   917,   218,   222,   267,   266,   265,     0,   272,
       0,     0,     0,     0,     0,   311,   300,     0,   398,   841,
     843,   847,     0,   395,   393,   858,   859,   851,     0,   845,
       0,     0,   801,     0,   452,     0,   412,   413,   728,     0,
       0,     0,     0,   512,   575,   577,     0,     0,     0,   695,
     699,   700,   697,   681,   516,   513,   530,     0,     0,   702,
     706,   707,   704,   691,   603,   605,   637,     0,   896,   897,
       0,   898,   833,     0,   469,   866,   880,   878,   876,     0,
       0,     0,     0,   920,   268,     0,     0,   314,   317,     0,
       0,     0,     0,     0,   805,   806,   803,     0,     0,     0,
     475,   471,   473,   527,     0,   709,   713,   714,   711,   698,
       0,   528,   705,     0,   904,   903,     0,   877,   874,   918,
       0,   923,     0,   316,     0,     0,     0,   301,   849,   853,
     416,     0,   733,   712,     0,   617,   819,   922,     0,   318,
     319,     0,   310,   414,   514,   834,     0,   302,     0,     0,
       0,   315,   320,     0,   820,     0,    25,     0,     0,     0,
       0,   835,   836
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1301, -1301, -1301,  -321, -1301, -1301,  -267,   -69, -1301,   -31,
    -283,  1058,  1253,  1181,  1614,  1610,  1623,  1626,  1641, -1301,
     -99,  -236, -1301,    -1,  -788,   240,   888, -1301,  1514, -1301,
   -1301,   -70, -1301, -1301,  -370,  -547,  -253, -1301,   816, -1301,
     -61,  -749,   473, -1301,    -4,   -41,   -13,   -81,  -218, -1301,
    1330, -1301,  -213,  -179,  -384,  -187,  -539,  1055,  -905, -1301,
    1046,     8,  1467, -1301,  -138, -1301,  1598,  -111,  1759, -1301,
   -1301,  4657, -1301,  1960,   157,    11,  1900, -1301, -1301, -1301,
   -1301, -1301,  1552, -1301,  1640, -1301,  1642, -1301,   824,   825,
     533,   530, -1301,     2, -1301,    30, -1301,  1147, -1301,  1727,
   -1301, -1301, -1301,  -130,   758, -1301, -1301, -1301,  1324,  -489,
   -1301, -1301, -1301, -1301, -1301, -1301,  -458,  -504, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301,  1308,  1791, -1301,  1430, -1301,
   -1301, -1301,  1294, -1301, -1301, -1301, -1301, -1301,  -453,  -684,
   -1301, -1301, -1301,  -544, -1301,  -542, -1301,   631, -1301, -1301,
    1937, -1301,  1158, -1301, -1301, -1301,  1023, -1301, -1301, -1301,
    -892, -1301, -1301, -1301,  1000,  -413, -1301,  -621, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301,  1241,  -555, -1301, -1301,  -897, -1301, -1301,  -952, -1301,
   -1301,  -713, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
    -521, -1301, -1301, -1301, -1301, -1301,  -513,  -317, -1301, -1301,
   -1301, -1301, -1301,  1211,  -530, -1301, -1301, -1301, -1301,  -735,
   -1301, -1301, -1301,  -997, -1301, -1301, -1301,  -847, -1301, -1301,
   -1301,  1004, -1301, -1301, -1301,  -743, -1301, -1301, -1301,  -974,
   -1301, -1301, -1301,   811, -1301, -1301, -1301,   657, -1301, -1301,
   -1301, -1023, -1301, -1301, -1301,   542, -1301, -1301, -1301, -1300,
   -1301, -1301, -1301,   490, -1301, -1301, -1301,  1301, -1301, -1301,
    -247, -1301, -1301, -1301, -1301, -1301,  -631, -1301, -1301, -1301,
    1095, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301,  1345, -1301,  -309,   565, -1301, -1301,  1685,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301,  -579, -1301, -1301,
    -484,  -365, -1301,   -76, -1301,  -428, -1301,  -328, -1301,    59,
   -1301,   242, -1301, -1301, -1301,  -468,    26, -1159,   613,   947,
     939,   774, -1301, -1301,   612, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301,   973, -1301, -1301,    60, -1301, -1301, -1301, -1301,
    1178, -1071,   617, -1301, -1301, -1301,  1236
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    37,   560,    83,    84,    85,   443,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   288,   121,   313,    38,   123,   230,   231,   582,
      40,    41,    42,    43,   587,   588,   267,   880,   881,    44,
     561,   562,    45,    46,    47,    48,    49,   227,   619,   428,
     429,   430,   269,   620,   456,    50,   908,   909,   910,   911,
     912,   366,   490,   125,   126,   362,   127,   128,   129,   130,
     131,   132,   133,    51,    52,    53,    54,   416,   238,    55,
     417,   239,   423,   134,   331,   332,   333,   929,   930,  1305,
    1306,  1519,  1571,   217,   218,    56,   567,   568,   569,   251,
     247,   252,   135,   136,   367,   137,   138,   508,   661,   944,
    1165,   959,   953,   139,   140,   529,   981,   945,  1528,  1465,
    1350,  1188,   141,   142,   509,   674,   370,   554,   555,   143,
     144,   514,   693,   694,   971,   145,   146,   533,  1054,   740,
     849,   850,  1089,   741,   992,   742,   993,  1471,    57,    58,
      59,   571,   853,   147,   148,   705,   983,   149,   150,   941,
    1397,   151,   152,   757,  1005,   768,  1199,   769,  1540,  1223,
    1224,  1383,  1008,  1361,   770,   771,  1225,   153,   376,   154,
     539,   772,   773,   155,  1024,  1025,   156,  1031,  1032,   157,
     752,   753,   754,   997,   998,    60,    61,   440,    62,   576,
     577,   854,   855,   158,   159,   541,  1043,   786,  1237,  1238,
     160,   161,   544,   797,  1055,  1497,   162,   163,   793,  1232,
     164,   165,  1051,  1390,   166,   167,  1239,  1491,   168,   169,
     759,  1018,   170,   171,   780,  1211,   172,   173,  1040,  1368,
     174,   175,  1012,  1212,   176,   177,  1206,  1369,   178,   179,
    1228,  1481,   180,   181,  1363,  1482,   182,   183,  1386,  1537,
     184,   185,  1477,  1538,   186,   187,   530,   726,   727,  1190,
     755,  1353,  1194,   188,   189,   648,  1154,   190,   191,   646,
     934,   192,   193,   194,   195,   518,   700,   196,   197,   198,
     199,   200,   201,   202,   524,   702,   203,   527,   703,   204,
     205,   206,   511,   684,   512,   513,  1335,  1336,   207,   357,
     208,    63,   222,  1099,    64,  1503,  1575,   842,  1264,  1580,
     947,   948,   954,   949,   955,   950,   964,   951,   956,   952,
    1319,   668,  1170,  1458,  1171,   718,   978,  1269,  1270,   857,
     858,  1095,  1426,  1271,  1425,   669,   579,   209,   210,   211,
     550,   815,  1074,  1414,   212,   213,   442,    66,   409,   580,
     870,  1278,  1279,   214,   215,   548,   814
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     101,   258,   259,   261,   312,   434,   266,   461,   242,   503,
     124,   469,   470,   451,   798,   457,   365,   946,   289,   662,
     444,   982,   427,   226,   663,   713,   447,   851,   785,   852,
     402,   224,  1280,  1281,   289,   232,  1080,  1045,   444,  1001,
     877,  1359,  1360,   873,   445,   235,   729,   446,   420,  1153,
     448,   449,   465,   308,  1243,   861,   290,   787,  1056,   219,
      65,   559,   254,    65,  1376,   746,  1233,   270,   270,   246,
     624,   712,   250,   219,   485,   265,   801,  1082,     4,  1221,
     739,   677,     4,   270,   460,   818,  1492,   680,   455,   464,
    1108,   572,   542,     4,     4,   778,  1010,   310,   572,   559,
     421,   716,  1185,   506,   516,   744,   400,     5,   525,   358,
     413,    65,   326,   363,   559,  1586,   320,   572,  1122,   364,
     698,   751,   573,    30,  1172,   535,   414,  1218,   517,   573,
     507,   681,   526,   536,   310,   124,  1007,  1587,   543,  1023,
    1030,   779,  1011,   664,   675,   310,   270,     4,   573,   695,
     501,   220,   418,     5,   859,  1173,  1174,   933,   298,   299,
     435,   640,   935,   450,   714,   730,   589,     4,   743,   310,
     662,   860,     4,   574,   775,   663,   788,   223,    69,   799,
     666,   643,   699,    69,   491,  1186,  1187,   266,  1542,   256,
     266,   270,   270,   266,   266,   601,   266,   575,   266,   608,
     221,   713,   732,   688,  1016,  1391,   713,  1175,   243,   819,
     311,   624,   630,   790,   412,   875,   310,   623,   938,   223,
     883,  1290,   289,   289,   289,   289,   289,   289,   289,   289,
     289,   289,   289,   289,   289,   289,   289,   289,   289,   289,
     729,   437,   122,  1400,   677,   309,  1017,   596,   403,   441,
    1044,   644,   987,  1081,   454,  1392,   265,  1152,  1311,   265,
     466,   467,   468,   250,    35,   798,  1145,  1374,    35,   746,
    1049,   645,   926,    36,   462,   787,    65,   716,   583,    35,
      35,   936,   716,   728,   995,   592,  1202,   229,    36,    36,
     822,  1260,   774,  1522,   664,   982,   787,   401,  1236,   787,
     878,   249,   879,   851,   610,   852,  1261,   675,   484,   744,
    1529,  1155,  1046,   486,   492,   493,   494,  1245,   939,  1373,
     498,   444,   489,  1285,   928,   746,   695,   801,   877,  1441,
    1082,   666,   589,    35,   589,  1082,   877,   589,  1098,   751,
     714,   219,    36,   419,  1006,   714,   872,  1022,  1029,   266,
     602,   300,   301,   593,   878,   453,  1510,   491,   593,   730,
     453,   228,    36,   927,  1493,   744,   888,   122,  1303,   889,
     124,  1127,   743,   243,  1520,  1568,   893,   970,  1290,  1163,
     657,  1564,  1007,   353,   354,   906,   805,   907,   221,   289,
     905,   872,  1399,  -515,   788,  -288,   732,   609,   310,   680,
     599,   775,   894,  1023,   896,  1128,   600,   902,   256,   364,
    1030,   355,   631,   595,   864,   788,   358,   685,   788,   327,
    1244,   328,   594,  1415,   271,   272,   273,   628,   743,   262,
     799,   790,   665,   676,   806,    11,   329,  1416,   696,   454,
     365,   270,   933,   681,    20,    21,   874,   935,   310,   294,
     295,   310,   790,   715,   731,   790,   356,  1210,   657,  1016,
     865,   866,   867,   776,   920,   789,  1152,   925,   800,    30,
     263,   686,   611,  1109,   802,   713,   682,   728,   612,   422,
     264,   807,  1044,   689,   223,   359,   748,   763,   687,  1110,
     360,   803,   249,   683,   632,   489,   639,   823,   642,   650,
     651,  1017,   827,   938,   808,   809,   810,   670,   690,   225,
     305,  1044,  1019,  1026,  1033,   266,  1002,   266,   811,   774,
     266,  1242,  1276,   650,   651,   671,   812,   306,   787,   654,
     787,   304,   913,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   813,  1046,   266,  1222,   266,  1496,   268,
     307,   716,  1003,   310,   242,  1152,  1234,   787,   314,   124,
     405,   499,   289,  1087,   330,   310,   936,   667,   678,  1004,
     937,  1166,   746,  1046,   843,   844,   845,   734,   735,   846,
     595,  1138,   406,   665,   746,   882,   751,  1289,   717,   224,
    1006,   407,   745,  1442,   444,  1023,   676,  1244,  1030,    36,
     791,  1167,   122,   939,   691,  1021,   274,  1443,   275,  1244,
     276,  1022,   744,  1146,   714,   696,   650,   651,  1029,  1147,
     315,   692,   589,   316,   744,  1021,  1168,   317,   672,   715,
     589,   709,   657,  1148,   715,   872,   917,   919,   321,   922,
     924,  1028,   914,   915,   916,   673,  1169,   788,   731,   788,
     847,  1367,   657,  1210,   322,   805,   650,   422,   671,  1446,
     748,   763,   654,   591,   291,  1442,   794,   848,   657,   292,
     293,   709,   722,   323,   872,   743,   788,   324,  1044,  1447,
     748,   763,   361,   789,   790,   109,   790,   743,   734,   735,
     776,   736,   722,   564,   565,   566,   748,   763,  1242,   657,
     411,   864,  1152,   806,   789,   940,   369,   789,   722,  1398,
    1242,  1152,  1235,   790,   225,  1182,   399,  1057,   667,   800,
    1124,   404,  1234,  1231,   787,   312,   287,   748,   763,  1470,
     268,   678,  1244,   268,   296,   297,   268,   268,   746,   268,
    1046,   268,   408,   689,  1289,  1234,  1216,   865,   866,   867,
     807,  1434,   310,   410,  1322,   289,  1244,  1002,   119,   650,
     651,   746,   436,  1217,   717,  1213,  1507,  1019,   690,   717,
    1323,  -907,   733,   808,   809,   810,   756,  1026,   744,   312,
    1554,   777,   458,  1160,  1033,  1067,   487,   811,   804,   302,
     303,   122,  1067,  1152,  1555,  1161,   745,  1162,  1324,  1325,
     868,   744,   496,  1022,  1152,   266,  1029,   312,  1480,   289,
    1367,   459,     5,   266,   706,   707,   708,   869,   791,   629,
     500,   240,  1304,   241,   591,   612,   928,  1304,  1253,  1254,
     650,   651,   671,  1242,   463,     5,   654,   289,   650,   791,
     671,   743,   791,   788,   654,   709,   502,  1244,   634,   452,
    1152,   453,   745,   709,   310,   937,  1398,  1242,    36,  1068,
    1069,  1070,   825,   826,   743,  1398,  1068,  1069,  1070,  1071,
     734,   735,   452,   736,   453,    69,  1071,  1072,  1235,   635,
     790,  1234,   625,   510,   626,   310,   515,   636,    39,  1231,
     872,    39,   268,   310,  1073,   519,  -286,   746,  -107,   828,
     528,  1235,   371,   715,   225,   829,   650,   651,   411,   862,
     375,  1389,  1231,   415,  -107,   863,   531,   377,   532,  1135,
    1137,   709,  1536,  1480,  1142,   381,  1132,   382,   233,   234,
     540,  1139,   415,   236,   237,   710,   789,   744,   789,    39,
    1191,  1192,  1193,   737,  1077,   391,   245,  1398,  1242,    39,
     612,   394,   711,   395,   253,   747,   794,   396,  1398,  1370,
     738,  1213,  1096,   398,  1057,   789,   733,   557,  1097,  1026,
     545,   534,  1033,  1180,  1131,  1133,  1057,   547,  1140,  1536,
     310,   310,   657,  1184,   310,  1189,  1444,   535,   558,  1195,
     940,  1181,  1262,  1198,   756,   536,   549,  1182,   612,  1009,
     743,  1020,  1027,  1034,  1398,   559,   966,   967,   777,   670,
     748,   795,   312,   749,   750,   537,   538,  1295,  1263,  1410,
    1411,   570,   722,   310,  1028,   650,   651,   671,   796,   563,
      67,   654,    68,    69,  1297,    70,    71,  1235,   717,  1302,
     310,   578,   289,  1338,  1060,   310,  1389,   872,  1231,   310,
    1340,   657,   597,  1351,   872,    72,   310,   598,   268,   310,
     268,  1248,  1249,   268,   831,   706,   707,   708,   762,  1490,
    1389,   791,   585,   791,   603,   411,  1308,   832,   833,   748,
     763,   650,   651,   671,   650,   651,   590,   654,   268,  1266,
     268,   722,    73,    74,    75,   657,   709,  1354,   604,   745,
     791,  1357,  1401,   310,    39,   882,  1403,   310,  1402,  1404,
     406,   745,   310,   734,   735,   310,   736,  -286,  1370,  1418,
     613,  1304,  1518,   748,   763,   310,  1518,   553,   426,   614,
    1057,   615,   789,  1419,   556,   722,   616,  1299,  1291,  1097,
      39,  1293,  1294,   617,  1296,  1219,  1078,  1079,  1300,  1301,
    1421,  1429,   764,  1057,   747,   637,  1422,  1430,   765,   647,
    1490,  1389,  1220,   679,  1321,  1377,  1378,  1379,  1380,  1326,
     697,  1490,   701,  1431,  1432,   834,   835,  1524,  1525,  1430,
    1430,   657,  1312,  1313,  1314,  1315,   979,  1381,  1382,  1352,
    1320,  1438,  1512,  1448,  1552,  1344,   758,   310,  1358,  1182,
    1449,  1450,  1332,   980,  1451,  1337,  1182,  1182,  1453,   748,
    1182,   704,   749,   750,  1182,   792,    76,  1490,  1083,  1084,
     760,   722,  1355,  1356,    77,    78,    79,    80,    81,    82,
    1013,   761,  1454,  1569,  1457,   824,  1393,  1394,   310,   325,
     310,   756,  1459,  1125,  1126,  1009,   650,   651,  1182,   652,
     653,  1384,   654,   830,  1214,   856,  1020,   657,   836,   837,
     838,   839,  1123,     5,   886,   745,  1027,   791,   840,   841,
    1424,  1282,  1283,  1034,   887,   999,  1464,  1345,  1346,  1347,
    1348,  1349,  1182,   627,  1466,   748,   763,  1468,   745,  1057,
    1467,  1417,  1000,   310,   781,   782,    67,   722,    68,    69,
     890,    70,    71,  1435,   891,  1436,  1437,   892,  1412,  1413,
    1439,  1473,  1474,   821,   764,   649,  1475,   310,  1422,  1588,
     765,    72,  1422,  1484,   650,   651,   671,   652,   653,  1422,
     654,   655,   656,  1460,  1461,   657,    67,     5,    68,    69,
     426,    70,    71,  1544,  1545,  1502,  1486,  1494,   268,   658,
     444,  1495,  1182,   310,   471,   472,   268,   310,    73,    74,
      75,    72,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,   762,  1476,  1504,   895,
    1533,   289,   289,   897,  1182,   898,  1182,   957,    73,    74,
      75,    30,   650,   651,    32,    33,   650,   651,  1549,   652,
     653,  1553,   654,   657,  1430,   650,   651,   310,   652,   653,
     899,   709,  1506,   900,   745,  1424,   657,  1557,   719,  1558,
    1559,   720,   721,  1554,   958,  1422,   310,  1560,  1562,  1565,
    1516,   748,   763,   310,  1422,   310,  1514,   685,  1371,   960,
    1214,  1556,   961,   722,   781,   782,   794,  1523,  1027,  1566,
     962,  1034,  1573,   969,  1527,   310,   722,  1574,   310,  1530,
     764,  1577,  1581,  1422,  1590,   973,   765,  1578,   310,   974,
     612,   426,    76,   477,   478,   723,   426,   977,  1589,   975,
      77,    78,    79,    80,    81,    82,  1543,   520,   521,   522,
     523,   976,  1546,   984,  1039,   120,  1547,   766,   985,   986,
     988,  1041,   989,   426,   990,   994,   996,  1035,   289,  1036,
     724,  1037,    76,  1047,   767,  1048,  1050,  1561,  1042,  1058,
      77,    78,    79,    80,    81,    82,   657,   725,   843,   844,
     845,   734,   735,   846,  1061,   497,  1062,  1063,   706,   707,
     708,   473,   474,   475,   476,  1064,  1065,   102,     5,    68,
      69,  1075,    70,    71,   650,   651,   671,  1576,   817,  1085,
     654,  1088,  1090,  1092,  1093,  1100,  1115,  1101,  1076,   709,
    1094,  1102,    72,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,  1371,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,    73,
      74,    75,    30,   114,   115,   116,    33,   102,     5,    68,
      69,  1103,    70,    71,  1104,  1111,  1113,  1116,  1114,  1117,
    1118,  1119,  1130,  1143,  1120,  1121,  1156,  1144,  1157,  1158,
    1159,  1164,    72,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,  1176,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,    73,
      74,    75,    30,   114,   115,   116,    33,   650,   651,  1177,
     652,   653,  1178,   654,   117,   967,  1179,  1196,    67,  1197,
      68,    69,   709,    70,    71,  1200,   650,   651,  1201,   652,
     653,  1203,   654,  1205,   118,   102,  1226,    68,    69,  1227,
      70,    71,  1246,    72,  1247,  1255,  1256,  1257,  1265,  1259,
    1268,  1272,  1277,    76,  1273,   781,   782,   794,   119,   816,
      72,    77,    78,    79,    80,    81,    82,  1275,  1286,  1287,
    1307,  1316,  1334,  1292,   781,   782,   120,  1317,  1318,  1327,
      73,    74,    75,  1328,   117,  1329,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,    73,    74,    75,
    1330,   114,   115,   488,   118,   102,  1339,    68,    69,  1331,
      70,    71,    67,  1333,    68,    69,  1342,    70,    71,  1341,
    1362,  1343,  1375,    76,  1385,  1427,  1405,  1406,   119,  1407,
      72,    77,    78,    79,    80,    81,    82,    72,  1408,  1409,
    1440,  1428,  1445,  1452,  1487,    67,   120,    68,    69,  1455,
      70,    71,  1456,  1462,  1463,  1469,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,    73,    74,    75,
      72,   114,   115,   318,    73,    74,    75,  1485,    67,  1498,
      68,    69,   117,    70,    71,  1499,  1500,  1501,  1508,  1531,
    1515,  1550,  1532,  1541,   102,  1570,    68,    69,  1548,    70,
      71,  1551,   319,    72,    76,  1567,  1572,    73,    74,    75,
    1579,  1582,    77,    78,    79,    80,    81,    82,  1583,    72,
    1584,    76,  1585,  1591,  1592,   480,   119,   638,   479,    77,
      78,    79,    80,    81,    82,  1284,  1106,   581,   885,   481,
      73,    74,    75,   482,   120,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,    73,    74,    75,   483,
     114,   115,   552,    67,   633,    68,    69,  1129,    70,    71,
     551,   397,   319,   216,   650,   651,   255,   652,   653,   584,
     654,  1309,   504,  1310,   505,  1517,  1086,  1521,    72,   439,
     965,    76,   393,   963,   820,   972,   119,  1472,    76,    77,
      78,    79,    80,    81,    82,   248,    77,    78,    79,    80,
      81,    82,  1183,  1204,   120,  1091,  1059,  1038,   650,   651,
     671,   641,   781,   782,   654,    73,    74,    75,  1215,  1372,
    1539,    76,  1483,   709,  1563,   991,  1149,   968,  1526,    77,
      78,    79,    80,    81,    82,  1505,  1274,   546,  1509,  1267,
     734,   735,  1420,   736,   921,  1258,  1105,  1513,  1066,     0,
       0,   319,     0,     0,    76,     0,     0,     0,   794,     0,
       0,     0,    77,    78,    79,    80,    81,    82,     0,   783,
      76,     0,     4,     5,  1511,   119,     0,     0,    77,    78,
      79,    80,    81,    82,     0,     0,   784,     0,     0,     0,
       0,     0,     0,   120,     0,     0,     0,     0,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,     0,     0,     0,     0,
      67,     5,    68,    69,     0,    70,    71,    30,     0,    76,
      32,    33,     0,     0,     0,     0,     0,    77,    78,    79,
      80,    81,    82,     0,     0,    72,     0,     0,     0,     0,
    1423,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,     0,    24,    25,    26,    27,    28,    29,
      67,     0,    68,    69,     0,    70,    71,     0,     0,     0,
       0,     0,    73,    74,    75,    30,     0,    67,     0,    68,
      69,     0,    70,    71,     0,    72,     0,     0,   431,     0,
       0,    11,     0,     0,     0,     0,     0,     0,     0,     0,
      20,    21,    72,     0,    67,   605,    68,    69,    11,    70,
      71,     0,     0,     0,     0,     0,     0,    20,    21,     0,
       0,     0,    73,    74,    75,    30,     0,     0,    67,    72,
      68,    69,     0,    70,    71,    11,     0,     0,    35,    73,
      74,    75,    30,     0,    20,    21,     0,    36,     0,    67,
       0,    68,    69,    72,    70,    71,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    73,    74,    75,    30,
      67,     0,    68,    69,    72,    70,    71,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      73,    74,    75,     0,     0,    72,    76,     0,     0,     0,
       0,     0,     0,     0,    77,    78,    79,    80,    81,    82,
       0,    73,    74,    75,     0,    67,     0,    68,    69,     0,
      70,    71,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    73,    74,    75,     0,     0,     0,     0,     0,
      72,     0,    67,     0,    68,    69,    76,    70,    71,   432,
       0,     0,     0,     0,    77,   433,    79,    80,    81,    82,
       0,     0,     0,    76,     0,     0,   606,    72,     0,     0,
       0,    77,   607,    79,    80,    81,    82,    73,    74,    75,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      76,     0,     0,     0,     0,     0,     0,     0,    77,    78,
      79,    80,    81,    82,    73,    74,    75,     0,     0,     0,
       0,     0,     0,     0,    76,     0,   906,     0,   907,   871,
    1288,     0,    77,    78,    79,    80,    81,    82,    67,     0,
      68,    69,     0,    70,    71,    76,     0,   906,     0,   907,
     871,  1433,     0,    77,    78,    79,    80,    81,    82,     0,
       0,     0,     0,    72,     0,     0,    76,     0,   906,     0,
     907,   871,     0,     0,    77,    78,    79,    80,    81,    82,
      67,     0,    68,    69,     0,    70,    71,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      73,    74,    75,     0,     0,    72,     0,  1250,  1251,     0,
      67,    76,    68,    69,     0,    70,    71,     0,     0,    77,
    1252,    79,    80,    81,    82,     0,     0,    67,     0,    68,
      69,     0,    70,    71,     0,    72,     0,     0,    76,     0,
       0,   621,    73,    74,    75,     0,    77,   622,    79,    80,
      81,    82,    72,    67,     0,    68,    69,     0,    70,    71,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    73,    74,    75,     0,     0,    67,    72,    68,
      69,     0,    70,    71,     0,     0,     0,     0,     0,    73,
      74,    75,     0,    67,   649,    68,    69,     0,    70,    71,
       0,     0,    72,   650,   651,     0,   652,   653,     0,   654,
     655,   656,     0,     0,   657,    73,    74,    75,    72,     0,
      67,     0,    68,    69,    76,    70,    71,     0,   658,   627,
       0,     0,    77,    78,    79,    80,    81,    82,     0,    73,
      74,    75,     0,     0,     0,    72,     0,     0,    67,     0,
      68,    69,   649,    70,    71,    73,    74,    75,     0,     0,
       0,   650,   651,   671,   652,   653,    76,   654,   655,   656,
       0,   871,   657,    72,    77,    78,    79,    80,    81,    82,
       0,     0,    73,    74,    75,     0,   658,     0,     0,     0,
       0,     0,    67,     0,    68,    69,    76,    70,    71,   903,
       0,     0,     0,     0,    77,   904,    79,    80,    81,    82,
      73,    74,    75,    76,   918,     0,     0,    72,     0,     0,
       0,    77,    78,    79,    80,    81,    82,   650,   651,   671,
       0,     0,    67,   654,    68,    69,     0,    70,    71,    76,
     923,     0,   709,     0,     0,     0,     0,    77,    78,    79,
      80,    81,    82,     0,    73,    74,    75,    72,     0,   734,
     735,     0,   736,    76,  1134,     0,   931,     0,     0,     0,
       0,    77,    78,    79,    80,    81,    82,   794,     0,    76,
    1136,     0,     0,   932,     0,     0,     0,    77,    78,    79,
      80,    81,    82,     0,    73,    74,    75,    67,     0,    68,
      69,     0,    70,    71,     0,     0,    76,  1141,     0,     0,
       0,     0,     0,     0,    77,    78,    79,    80,    81,    82,
       0,     0,    72,   650,   651,   671,   652,   653,     0,   654,
       0,     0,  1052,     0,    76,  1298,     0,     0,   709,     0,
       0,     0,    77,    78,    79,    80,    81,    82,     0,  1053,
       0,     0,     0,     0,     0,   734,   735,     0,   736,    73,
      74,    75,     0,     0,     1,     2,     3,     4,     5,     0,
       0,   781,   782,   794,     0,     0,     0,     0,    76,     0,
       0,     0,     0,     0,     0,     0,    77,    78,    79,    80,
      81,    82,     0,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,   257,     4,
       5,     0,     0,     0,     0,     0,    77,    78,    79,    80,
      81,    82,    30,     0,    31,    32,    33,     0,     0,     0,
       0,     0,     0,     0,     0,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     4,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   260,    30,     0,    31,    32,    33,     0,
       0,    77,    78,    79,    80,    81,    82,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,     0,   707,   708,     0,     0,
       0,     0,   649,     0,    34,     0,    30,     0,     0,   438,
      33,   650,   651,   671,   652,   653,     0,   654,   655,   656,
       0,     0,   657,    35,     0,     0,   709,     0,     0,     0,
       0,     0,    36,     5,     0,     0,   658,     0,     0,     0,
       0,     0,     0,   734,   735,     0,   736,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    34,     0,     0,     4,
       5,   794,     0,     0,    12,    13,    14,    15,    16,    17,
      18,    19,     0,     0,    22,    35,    24,    25,    26,    27,
      28,    29,     0,     0,    36,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     4,     5,     0,     0,     0,  1395,     0,     0,     0,
       0,     0,     0,     0,    30,     0,     0,    35,     0,     0,
       0,     0,     0,  1396,     0,     0,    36,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     4,     5,     0,   650,   651,     0,   652,   653,     0,
       0,     0,     0,     0,     0,   657,    30,   719,     0,   244,
     720,   721,     0,     0,     0,     0,     0,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,   722,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    30,     0,     0,     0,
       5,     0,     0,     0,   723,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   593,   618,   453,     0,     0,
       0,     0,     0,     0,    36,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     5,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    30,     0,     0,    35,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,     0,    24,    25,    26,    27,    28,    29,     5,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    30,     0,     0,     0,    35,     0,     0,
       5,     0,     0,     0,     0,     0,    36,     0,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
       0,    24,    25,    26,    27,    28,    29,     0,     0,     0,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    30,    24,    25,    26,    27,    28,    29,     5,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    30,   452,   618,   453,     0,     0,
       0,     0,     0,     0,    36,     0,     0,     0,     0,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,     0,    24,    25,    26,    27,    28,    29,     5,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    30,     0,     0,     0,     0,     0,     0,
     586,     0,     0,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    30,     0,     0,    32,    33,   424,     5,   876,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1107,     0,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     5,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    30,     0,     0,     0,     0,     0,     0,     0,
    1112,     0,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,  1364,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1207,    30,     0,   650,   651,   671,   652,   653,   119,   654,
       0,     0,     0,     0,   657,     0,   650,   651,   709,   652,
     653,     0,   654,     0,     0,     0,     0,   657,     0,     0,
       0,   709,     0,     0,     0,   734,   735,     0,   736,     0,
       0,     0,   748,   763,     0,     0,     0,     0,     0,     0,
       0,   781,   782,   794,   722,   748,   763,     0,     0,     0,
       0,     0,     0,     0,   781,   782,   794,   722,     0,     0,
       0,   764,  1013,     0,   425,     0,     0,   765,     0,     0,
       0,     0,     0,     0,   764,  1207,     0,     0,   650,   651,
     765,   652,   653,     0,   654,     0,     0,     0,     0,   657,
       0,   650,   651,     0,   652,   653,     0,   654,  1365,     0,
       0,     0,   657,     0,     0,     0,   709,     0,     0,     0,
       0,  1208,     0,   901,     0,  1366,     0,   748,   763,     0,
       0,     0,     0,     0,     0,     0,   781,   782,  1209,   722,
     748,   763,   707,   708,     0,     0,     0,     0,   649,   781,
     782,   794,   722,     0,     0,     0,   764,   650,   651,   671,
     652,   653,   765,   654,   655,   656,     0,     0,   657,   764,
       0,     0,   709,     0,     0,   765,     0,     0,     0,     0,
       0,     0,   658,     0,     0,     0,     0,     0,     0,   734,
     735,     0,   736,  1014,     0,     0,   748,   763,     0,     0,
       0,     0,     0,     0,     0,   781,   782,   794,     0,     0,
    1015,   707,   708,     0,     0,     0,     0,   649,     0,     0,
       0,     0,     0,     0,     0,   764,   650,   651,   671,   652,
     653,   765,   654,   655,   656,     0,     0,   657,   707,   708,
       0,   709,     0,     0,   649,     0,     0,     0,     0,     0,
       0,   658,     0,   650,   651,   671,   652,   653,     0,   654,
     655,   656,  1534,     0,   657,   748,   763,     0,   709,     0,
       0,     0,     0,     0,   781,   782,   794,     0,   658,  1535,
       0,     0,     0,     0,     0,   734,   735,     0,   736,   707,
     708,     0,     0,     0,   764,   649,     0,     0,     0,     0,
     765,   781,   782,   794,   650,   651,   671,   652,   653,     0,
     654,   655,   656,     0,     0,   657,     0,     0,     0,   709,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   658,
       0,  1478,     0,     0,   707,   708,   734,   735,     0,   736,
     649,     0,     0,     0,     0,     0,     0,     0,  1479,   650,
     651,   671,   652,   653,     0,   654,   655,   656,  1488,     0,
     657,     0,   707,   708,   709,     0,     0,     0,   649,     0,
       0,     0,     0,     0,   658,  1489,     0,   650,   651,   671,
     652,   653,     0,   654,   655,   656,     0,     0,   657,   707,
     708,     0,   709,     0,     0,   649,     0,   781,   782,   794,
       0,     0,   658,     0,   650,   651,   671,   652,   653,  1150,
     654,   655,   656,     0,     0,   657,     0,     0,     0,   709,
       0,   649,     0,     0,     0,     0,  1151,   794,     0,   658,
     650,   651,     0,   652,   653,     0,   654,   655,   656,     0,
       0,   657,   650,   651,   671,   652,   653,     0,   654,     0,
       0,     0,     0,     0,  1387,   658,     0,   709,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1364,     0,     0,
       0,  1388,     0,     0,   734,   735,     0,   736,     0,   706,
     707,   708,  1240,   650,   651,   671,   652,   653,     0,   654,
     781,   782,   794,     0,   657,   650,   651,   671,   709,  1241,
       0,   654,     0,     0,     0,     0,     0,     0,     0,   942,
     709,     0,     0,     0,     0,   734,   735,     0,   736,     0,
       0,     0,   748,   763,     0,     0,   943,   734,   735,     0,
     736,   781,   782,   794,   722,   659,     0,     0,     0,     0,
       0,     0,     0,     0,   707,   708,     0,  1229,     0,     0,
     649,   764,   660,     0,     0,     0,     0,   765,     0,   650,
     651,   671,   652,   653,  1230,   654,   655,   656,     0,     0,
     657,     0,     0,     0,   709,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   658,     0,     0,     0,     0,     0,
       0,   734,   735,     0,   736,   707,   708,     0,   748,   763,
       0,   649,     0,     0,     0,     0,     0,   781,   782,   794,
     650,   651,   671,   652,   653,     0,   654,   655,   656,     0,
       0,   657,     0,     0,     0,   709,     0,   764,     0,     0,
       0,     0,     0,   765,     0,   658,     0,     0,     0,   334,
     335,     0,   336,     0,     0,     0,     0,     0,     0,   748,
     763,   337,   338,   339,   340,   341,   342,     0,   781,   782,
     794,     0,     0,     0,     0,     0,     0,   343,     0,   344,
       0,   345,     0,     0,     0,   346,     0,     0,   764,     0,
       0,     0,     0,     0,   765,   347,     0,     0,     0,     0,
       0,     0,     0,   221,   348,     0,     0,     0,     0,     0,
       0,     0,     0,   349,   350,   334,   335,     0,   336,     0,
       0,     0,   351,     0,   352,     0,     0,   337,   338,   339,
     340,   341,   342,     0,     0,     0,   334,   335,     0,   336,
       0,   334,   335,   343,   336,   344,   817,   345,   337,   338,
     339,   346,   341,   337,   338,   339,     0,   341,     0,     0,
       0,   347,     0,     0,   343,     0,   344,     0,     0,   343,
     348,   344,     0,     0,     0,     0,     0,     0,     0,   349,
     350,     0,   347,     0,     0,     0,     0,   347,   351,     0,
     352,   495,     5,     0,     0,     0,   495,     0,     0,     0,
     349,   350,     0,     0,     0,   349,   350,     0,     0,   351,
       0,     0,     0,     0,   351,     0,     0,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,     0,   707,   708,     0,     0,
       0,     0,   649,     0,     0,     0,    30,     0,     0,    32,
      33,   650,   651,   671,   652,   653,     0,   654,   655,   656,
       0,     0,   657,     0,     0,     0,   709,     0,     0,     0,
       0,     0,   707,   708,     0,     0,   658,     0,   649,     0,
       0,     0,     0,   734,   735,     0,   736,   650,   651,   671,
     652,   653,     0,   654,   655,   656,     0,     0,   657,   781,
     782,   794,   709,     0,     0,     0,     0,     0,   707,   708,
       0,     0,   658,     0,   649,     0,     0,     0,     0,   734,
     735,     0,   736,   650,   651,   671,   652,   653,     0,   654,
     655,   656,     0,     0,   657,   707,   708,   794,   709,     0,
       0,   649,     0,     0,     0,     0,     0,     0,   658,     0,
     650,   651,   671,   652,   653,     0,   654,   655,   656,     0,
       0,   657,     0,     0,     0,   709,     0,     0,     0,     0,
       0,   781,   782,   794,     0,   658,     5,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     794,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   884,     5,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      30,     0,     0,     0,     0,     0,     0,     0,     5,     0,
       0,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      30,    24,    25,    26,    27,    28,    29,     0,     0,   707,
     708,     0,     0,     0,     0,   649,     0,     0,     0,     0,
       0,     0,    30,     0,   650,   651,   671,   652,   653,     0,
     654,   655,   656,     0,     0,   657,     0,   368,     0,   709,
       0,     0,     0,   372,     0,   373,     0,   374,     0,   658,
       0,     0,     0,     0,     0,     0,   734,   735,   378,   736,
     379,     0,   380,     0,     0,     0,     0,     0,   383,     0,
     384,     0,   385,     0,   386,     0,   387,     0,   388,     0,
     389,     0,   390,   707,   708,     0,   392,     0,     0,   649,
       0,     0,     0,     0,     0,     0,     0,     0,   650,   651,
     671,   652,   653,     0,   654,   655,   656,     0,     0,   657,
       0,     0,     0,   709,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   658
};

static const yytype_int16 yycheck[] =
{
       1,    70,    71,    72,   103,   241,    76,   274,    49,   330,
       2,   294,   295,   266,   544,   268,   127,   648,    87,   508,
     256,   705,   240,    36,   508,   529,   262,   571,   541,   571,
     217,    35,  1103,  1104,   103,    39,   824,   780,   274,   752,
     587,  1200,  1201,   582,   257,    43,   530,   260,   235,   941,
     263,   264,   288,    22,  1051,   576,    87,   541,   793,    29,
       0,     8,    66,     3,  1223,   533,  1040,    11,    11,    58,
     454,   529,    61,    43,   310,    76,   544,   826,     8,  1031,
     533,   509,     8,    11,   271,    76,  1386,   124,   267,   276,
     878,   138,    78,     8,     8,    78,    78,   201,   138,     8,
     238,   529,    89,    79,   194,   533,     8,     9,   194,   108,
     201,    51,   113,   217,     8,   194,   108,   138,   906,   123,
     156,   534,   169,    73,    91,   134,   217,  1024,   218,   169,
     106,   168,   218,   142,   201,   127,   757,   216,   124,   760,
     761,   124,   124,   508,   509,   201,    11,     8,   169,   514,
     217,   194,     8,     9,   201,   122,   123,   646,    17,    18,
     241,   217,   646,   195,   529,   530,   419,     8,   533,   201,
     659,   218,     8,   194,   539,   659,   541,   176,    11,   544,
     508,   502,   218,    11,   314,   172,   173,   257,  1488,   194,
     260,    11,    11,   263,   264,   431,   266,   218,   268,   435,
     132,   705,   530,   512,   759,  1228,   710,   174,    51,   200,
     215,   595,   197,   541,   227,   585,   201,   453,   646,   176,
     590,  1126,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     724,   245,     2,  1240,   672,   214,   759,   426,   218,   253,
     780,   195,   710,   200,   267,  1229,   257,   941,  1150,   260,
     291,   292,   293,   252,   194,   795,   194,  1219,   194,   737,
     783,   215,   215,   203,   275,   759,   216,   705,   416,   194,
     194,   646,   710,   530,   737,   423,   999,   217,   203,   203,
     557,   200,   539,  1452,   659,   979,   780,   199,  1041,   783,
     215,    61,   217,   847,   442,   847,   200,   672,   309,   737,
    1469,   942,   780,   311,   315,   316,   317,  1052,   646,  1216,
     321,   557,   314,  1111,   645,   793,   691,   795,   875,   194,
    1079,   659,   585,   194,   587,  1084,   883,   590,   859,   752,
     705,   311,   203,   199,   757,   710,   582,   760,   761,   419,
     431,   210,   211,   194,   215,   196,  1427,   487,   194,   724,
     196,     0,   203,   196,  1387,   793,   602,   127,   196,   605,
     362,   910,   737,   216,   194,   194,   612,   686,  1283,   958,
     107,  1540,  1003,    78,    79,   196,    61,   198,   132,   458,
     626,   627,  1239,     8,   759,   199,   724,   141,   201,   124,
     195,   766,   615,  1024,   617,   216,   201,   625,   194,   413,
    1031,   106,   215,   426,   102,   780,   108,   147,   783,    49,
    1051,    51,   426,   201,    12,    13,    14,   458,   793,   194,
     795,   759,   508,   509,   109,    39,    66,   215,   514,   452,
     551,    11,   931,   168,    48,    49,   584,   931,   201,   204,
     205,   201,   780,   529,   530,   783,   151,  1012,   107,  1014,
     148,   149,   150,   539,   217,   541,  1150,   217,   544,    73,
     194,   201,   195,   201,   201,   979,   201,   724,   201,   239,
     194,   156,  1012,    80,   176,   177,   135,   136,   218,   217,
     182,   218,   252,   218,   486,   487,   497,   558,   499,    96,
      97,  1014,   563,   931,   179,   180,   181,    80,   105,    36,
     212,  1041,   759,   760,   761,   585,   165,   587,   193,   766,
     590,  1051,  1101,    96,    97,    98,   201,   213,  1012,   102,
    1014,   202,   631,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,   218,  1012,   615,   161,   617,  1395,    76,
      21,   979,   201,   201,   595,  1239,  1040,  1041,   215,   551,
     102,   321,   631,   830,   194,   201,   931,   508,   509,   218,
     646,    78,  1040,  1041,   125,   126,   127,   128,   129,   130,
     593,   217,   124,   659,  1052,   589,   999,  1126,   529,   593,
    1003,   133,   533,   201,   830,  1216,   672,  1228,  1219,   203,
     541,   108,   362,   931,   201,    80,   194,   215,   196,  1240,
     198,  1024,  1040,   195,   979,   691,    96,    97,  1031,   201,
     194,   218,   875,   194,  1052,    80,   133,   194,   201,   705,
     883,   111,   107,   215,   710,   871,   637,   638,   194,   640,
     641,    80,   634,   635,   636,   218,   153,  1012,   724,  1014,
     201,  1206,   107,  1208,     8,    61,    96,   417,    98,   195,
     135,   136,   102,   423,   203,   201,   146,   218,   107,   208,
     209,   111,   147,   217,   910,  1040,  1041,   217,  1208,   215,
     135,   136,   200,   759,  1012,    65,  1014,  1052,   128,   129,
     766,   131,   147,     8,     9,    10,   135,   136,  1228,   107,
     227,   102,  1386,   109,   780,   646,   199,   783,   147,  1239,
    1240,  1395,  1040,  1041,   241,   201,     8,   793,   659,   795,
     907,   194,  1206,  1040,  1208,   824,   216,   135,   136,   215,
     257,   672,  1363,   260,    15,    16,   263,   264,  1206,   266,
    1208,   268,   194,    80,  1283,  1229,   201,   148,   149,   150,
     156,  1290,   201,   195,   118,   824,  1387,   165,   199,    96,
      97,  1229,   132,   218,   705,  1012,   215,  1014,   105,   710,
     134,   199,   530,   179,   180,   181,   534,  1024,  1206,   878,
     201,   539,   195,    87,  1031,   109,   215,   193,   546,    19,
      20,   551,   109,  1477,   215,    99,   737,   101,   162,   163,
     201,  1229,    63,  1216,  1488,   875,  1219,   906,  1363,   878,
    1365,     8,     9,   883,    80,    81,    82,   218,   759,   195,
     217,   194,  1143,   196,   584,   201,  1147,  1148,  1064,  1065,
      96,    97,    98,  1363,     8,     9,   102,   906,    96,   780,
      98,  1206,   783,  1208,   102,   111,   194,  1478,   195,   194,
    1534,   196,   793,   111,   201,   931,  1386,  1387,   203,   183,
     184,   185,   200,   201,  1229,  1395,   183,   184,   185,   193,
     128,   129,   194,   131,   196,    11,   193,   201,  1206,   195,
    1208,  1365,   194,   147,   196,   201,   218,   195,     0,  1206,
    1126,     3,   419,   201,   218,   218,   199,  1365,   201,   195,
     124,  1229,   144,   979,   431,   201,    96,    97,   435,   195,
     152,  1228,  1229,   216,   217,   201,   218,   159,   218,   920,
     921,   111,  1477,  1478,   925,   167,   918,   169,    40,    41,
     143,   923,   216,    45,    46,   201,  1012,  1365,  1014,    51,
     148,   149,   150,   201,   195,   187,    58,  1477,  1478,    61,
     201,   193,   218,   195,    66,    80,   146,   199,  1488,  1206,
     218,  1208,   195,   205,  1040,  1041,   724,   194,   201,  1216,
     218,   118,  1219,   974,   195,   195,  1052,   218,   195,  1534,
     201,   201,   107,   984,   201,   986,  1307,   134,   199,   990,
     931,   195,   195,   994,   752,   142,   218,   201,   201,   757,
    1365,   759,   760,   761,  1534,     8,   157,   158,   766,    80,
     135,   201,  1111,   138,   139,   162,   163,   195,  1088,  1255,
    1256,   194,   147,   201,    80,    96,    97,    98,   218,   199,
       8,   102,    10,    11,   195,    13,    14,  1365,   979,   195,
     201,     8,  1111,   195,   802,   201,  1363,  1283,  1365,   201,
     195,   107,   195,   195,  1290,    33,   201,   201,   585,   201,
     587,  1062,  1063,   590,     8,    80,    81,    82,    80,  1386,
    1387,  1012,   199,  1014,   197,   602,  1145,    21,    22,   135,
     136,    96,    97,    98,    96,    97,   199,   102,   615,  1090,
     617,   147,    70,    71,    72,   107,   111,   195,   197,  1040,
    1041,   195,   195,   201,   216,  1109,   195,   201,   201,   195,
     124,  1052,   201,   128,   129,   201,   131,   199,  1365,   195,
     195,  1442,  1443,   135,   136,   201,  1447,   369,   240,   195,
    1206,   201,  1208,   195,   376,   147,   201,  1138,  1130,   201,
     252,  1133,  1134,   201,  1136,   201,   200,   201,  1140,  1141,
     195,   195,   164,  1229,    80,   194,   201,   201,   170,   124,
    1477,  1478,   218,   194,  1165,   137,   138,   139,   140,  1170,
       8,  1488,   152,   195,   195,   119,   120,  1460,  1461,   201,
     201,   107,  1156,  1157,  1158,  1159,   201,   159,   160,  1190,
    1164,   195,  1428,   195,  1515,     1,   143,   201,  1199,   201,
     195,   195,  1176,   218,   195,  1179,   201,   201,   195,   135,
     201,   218,   138,   139,   201,   106,   194,  1534,   200,   201,
     134,   147,  1196,  1197,   202,   203,   204,   205,   206,   207,
      80,   134,   195,  1554,   195,   216,  1237,  1238,   201,   217,
     201,   999,   195,   200,   201,  1003,    96,    97,   201,    99,
     100,  1225,   102,   194,  1012,     8,  1014,   107,   202,   203,
     204,   205,     8,     9,     8,  1206,  1024,  1208,   212,   213,
    1271,   200,   201,  1031,   197,   201,   195,    83,    84,    85,
      86,    87,   201,   199,   195,   135,   136,   195,  1229,  1365,
     201,  1265,   218,   201,   144,   145,     8,   147,    10,    11,
     197,    13,    14,  1295,   197,  1297,  1298,   132,   186,   187,
    1302,   195,   195,   555,   164,    87,   195,   201,   201,  1586,
     170,    33,   201,   195,    96,    97,    98,    99,   100,   201,
     102,   103,   104,   204,   205,   107,     8,     9,    10,    11,
     452,    13,    14,   188,   189,  1415,   195,   195,   875,   121,
    1586,   195,   201,   201,   296,   297,   883,   201,    70,    71,
      72,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    80,  1361,   195,     8,
     195,  1460,  1461,   195,   201,   195,   201,   194,    70,    71,
      72,    73,    96,    97,    76,    77,    96,    97,   195,    99,
     100,   195,   102,   107,   201,    96,    97,   201,    99,   100,
     197,   111,  1423,   197,  1365,  1426,   107,   195,   109,   195,
     195,   112,   113,   201,   194,   201,   201,   195,   195,   195,
    1441,   135,   136,   201,   201,   201,  1438,   147,  1206,   194,
    1208,  1520,   194,   147,   144,   145,   146,  1458,  1216,   195,
     194,  1219,   195,   194,  1465,   201,   147,   195,   201,  1470,
     164,   195,   195,   201,   195,   195,   170,   201,   201,   194,
     201,   593,   194,   302,   303,   166,   598,     8,  1587,   218,
     202,   203,   204,   205,   206,   207,  1497,   115,   116,   117,
     118,   218,  1503,   194,   106,   217,  1507,   201,   194,   194,
     194,   201,   194,   625,   194,   194,   194,   194,  1587,   194,
     201,   194,   194,   194,   218,   194,   124,  1528,   218,   194,
     202,   203,   204,   205,   206,   207,   107,   218,   125,   126,
     127,   128,   129,   130,   218,   217,   194,   194,    80,    81,
      82,   298,   299,   300,   301,   194,   194,     8,     9,    10,
      11,   218,    13,    14,    96,    97,    98,  1568,    88,   195,
     102,   215,   194,   194,   194,     8,   133,   194,   820,   111,
     196,   194,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,  1365,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,     8,     9,    10,
      11,   194,    13,    14,   194,   215,   197,   195,   197,   195,
     195,   195,     4,   215,   197,   197,   194,     8,   194,   194,
     194,   194,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,   194,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    96,    97,   195,
      99,   100,   215,   102,   155,   158,   194,   194,     8,   194,
      10,    11,   111,    13,    14,   194,    96,    97,   194,    99,
     100,   194,   102,   106,   175,     8,   140,    10,    11,   124,
      13,    14,    83,    33,   218,   194,   194,   194,   194,   217,
       8,   196,     8,   194,   218,   144,   145,   146,   199,   200,
      33,   202,   203,   204,   205,   206,   207,   218,   218,   197,
     197,   195,     8,   217,   144,   145,   217,   195,   195,   215,
      70,    71,    72,   195,   155,   195,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
     195,    74,    75,    76,   175,     8,     8,    10,    11,   195,
      13,    14,     8,   218,    10,    11,   215,    13,    14,   201,
     124,   215,   201,   194,   215,   215,   195,   195,   199,   195,
      33,   202,   203,   204,   205,   206,   207,    33,   195,   195,
       8,   196,   195,   215,   171,     8,   217,    10,    11,   134,
      13,    14,   134,   195,   201,   215,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      33,    74,    75,    76,    70,    71,    72,   215,     8,   195,
      10,    11,   155,    13,    14,   195,   201,   195,   197,   195,
     197,     8,   195,   195,     8,     8,    10,    11,   197,    13,
      14,   197,   175,    33,   194,   197,   195,    70,    71,    72,
     154,     8,   202,   203,   204,   205,   206,   207,   194,    33,
     218,   194,     8,   195,   195,   305,   199,   217,   304,   202,
     203,   204,   205,   206,   207,  1109,   871,   413,   598,   306,
      70,    71,    72,   307,   217,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,   308,
      74,    75,    76,     8,   487,    10,    11,   911,    13,    14,
     362,   202,   175,     3,    96,    97,    66,    99,   100,   417,
     102,  1147,   332,  1148,   332,  1442,   829,  1447,    33,   252,
     672,   194,   191,   659,   554,   691,   199,  1356,   194,   202,
     203,   204,   205,   206,   207,    58,   202,   203,   204,   205,
     206,   207,   979,  1003,   217,   847,   795,   766,    96,    97,
      98,   217,   144,   145,   102,    70,    71,    72,  1014,  1208,
    1478,   194,  1365,   111,  1534,   724,   931,   682,  1463,   202,
     203,   204,   205,   206,   207,  1422,  1097,   352,  1426,  1092,
     128,   129,  1268,   131,   217,  1072,   868,  1430,   812,    -1,
      -1,   175,    -1,    -1,   194,    -1,    -1,    -1,   146,    -1,
      -1,    -1,   202,   203,   204,   205,   206,   207,    -1,   201,
     194,    -1,     8,     9,   214,   199,    -1,    -1,   202,   203,
     204,   205,   206,   207,    -1,    -1,   218,    -1,    -1,    -1,
      -1,    -1,    -1,   217,    -1,    -1,    -1,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       8,     9,    10,    11,    -1,    13,    14,    73,    -1,   194,
      76,    77,    -1,    -1,    -1,    -1,    -1,   202,   203,   204,
     205,   206,   207,    -1,    -1,    33,    -1,    -1,    -1,    -1,
     215,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    -1,    52,    53,    54,    55,    56,    57,
       8,    -1,    10,    11,    -1,    13,    14,    -1,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    -1,     8,    -1,    10,
      11,    -1,    13,    14,    -1,    33,    -1,    -1,    36,    -1,
      -1,    39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    49,    33,    -1,     8,    36,    10,    11,    39,    13,
      14,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,
      -1,    -1,    70,    71,    72,    73,    -1,    -1,     8,    33,
      10,    11,    -1,    13,    14,    39,    -1,    -1,   194,    70,
      71,    72,    73,    -1,    48,    49,    -1,   203,    -1,     8,
      -1,    10,    11,    33,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    70,    71,    72,    73,
       8,    -1,    10,    11,    33,    13,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      70,    71,    72,    -1,    -1,    33,   194,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   202,   203,   204,   205,   206,   207,
      -1,    70,    71,    72,    -1,     8,    -1,    10,    11,    -1,
      13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,    -1,
      33,    -1,     8,    -1,    10,    11,   194,    13,    14,   197,
      -1,    -1,    -1,    -1,   202,   203,   204,   205,   206,   207,
      -1,    -1,    -1,   194,    -1,    -1,   197,    33,    -1,    -1,
      -1,   202,   203,   204,   205,   206,   207,    70,    71,    72,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,   203,
     204,   205,   206,   207,    70,    71,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   194,    -1,   196,    -1,   198,   199,
     200,    -1,   202,   203,   204,   205,   206,   207,     8,    -1,
      10,    11,    -1,    13,    14,   194,    -1,   196,    -1,   198,
     199,   200,    -1,   202,   203,   204,   205,   206,   207,    -1,
      -1,    -1,    -1,    33,    -1,    -1,   194,    -1,   196,    -1,
     198,   199,    -1,    -1,   202,   203,   204,   205,   206,   207,
       8,    -1,    10,    11,    -1,    13,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      70,    71,    72,    -1,    -1,    33,    -1,   190,   191,    -1,
       8,   194,    10,    11,    -1,    13,    14,    -1,    -1,   202,
     203,   204,   205,   206,   207,    -1,    -1,     8,    -1,    10,
      11,    -1,    13,    14,    -1,    33,    -1,    -1,   194,    -1,
      -1,   197,    70,    71,    72,    -1,   202,   203,   204,   205,
     206,   207,    33,     8,    -1,    10,    11,    -1,    13,    14,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    71,    72,    -1,    -1,     8,    33,    10,
      11,    -1,    13,    14,    -1,    -1,    -1,    -1,    -1,    70,
      71,    72,    -1,     8,    87,    10,    11,    -1,    13,    14,
      -1,    -1,    33,    96,    97,    -1,    99,   100,    -1,   102,
     103,   104,    -1,    -1,   107,    70,    71,    72,    33,    -1,
       8,    -1,    10,    11,   194,    13,    14,    -1,   121,   199,
      -1,    -1,   202,   203,   204,   205,   206,   207,    -1,    70,
      71,    72,    -1,    -1,    -1,    33,    -1,    -1,     8,    -1,
      10,    11,    87,    13,    14,    70,    71,    72,    -1,    -1,
      -1,    96,    97,    98,    99,   100,   194,   102,   103,   104,
      -1,   199,   107,    33,   202,   203,   204,   205,   206,   207,
      -1,    -1,    70,    71,    72,    -1,   121,    -1,    -1,    -1,
      -1,    -1,     8,    -1,    10,    11,   194,    13,    14,   197,
      -1,    -1,    -1,    -1,   202,   203,   204,   205,   206,   207,
      70,    71,    72,   194,   195,    -1,    -1,    33,    -1,    -1,
      -1,   202,   203,   204,   205,   206,   207,    96,    97,    98,
      -1,    -1,     8,   102,    10,    11,    -1,    13,    14,   194,
     195,    -1,   111,    -1,    -1,    -1,    -1,   202,   203,   204,
     205,   206,   207,    -1,    70,    71,    72,    33,    -1,   128,
     129,    -1,   131,   194,   195,    -1,   201,    -1,    -1,    -1,
      -1,   202,   203,   204,   205,   206,   207,   146,    -1,   194,
     195,    -1,    -1,   218,    -1,    -1,    -1,   202,   203,   204,
     205,   206,   207,    -1,    70,    71,    72,     8,    -1,    10,
      11,    -1,    13,    14,    -1,    -1,   194,   195,    -1,    -1,
      -1,    -1,    -1,    -1,   202,   203,   204,   205,   206,   207,
      -1,    -1,    33,    96,    97,    98,    99,   100,    -1,   102,
      -1,    -1,   201,    -1,   194,   195,    -1,    -1,   111,    -1,
      -1,    -1,   202,   203,   204,   205,   206,   207,    -1,   218,
      -1,    -1,    -1,    -1,    -1,   128,   129,    -1,   131,    70,
      71,    72,    -1,    -1,     5,     6,     7,     8,     9,    -1,
      -1,   144,   145,   146,    -1,    -1,    -1,    -1,   194,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   202,   203,   204,   205,
     206,   207,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    -1,   194,     8,
       9,    -1,    -1,    -1,    -1,    -1,   202,   203,   204,   205,
     206,   207,    73,    -1,    75,    76,    77,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    -1,
      -1,     8,     9,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   194,    73,    -1,    75,    76,    77,    -1,
      -1,   202,   203,   204,   205,   206,   207,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    -1,    -1,    -1,    -1,    -1,    81,    82,    -1,    -1,
      -1,    -1,    87,    -1,   175,    -1,    73,    -1,    -1,    76,
      77,    96,    97,    98,    99,   100,    -1,   102,   103,   104,
      -1,    -1,   107,   194,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,   203,     9,    -1,    -1,   121,    -1,    -1,    -1,
      -1,    -1,    -1,   128,   129,    -1,   131,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,     8,
       9,   146,    -1,    -1,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    -1,    50,   194,    52,    53,    54,    55,
      56,    57,    -1,    -1,   203,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    -1,
      -1,     8,     9,    -1,    -1,    -1,   201,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    73,    -1,    -1,   194,    -1,    -1,
      -1,    -1,    -1,   218,    -1,    -1,   203,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,     8,     9,    -1,    96,    97,    -1,    99,   100,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    73,   109,    -1,    76,
     112,   113,    -1,    -1,    -1,    -1,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,
       9,    -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   194,   195,   196,    -1,    -1,
      -1,    -1,    -1,    -1,   203,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,     9,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    73,    -1,    -1,   194,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    -1,    52,    53,    54,    55,    56,    57,     9,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    73,    -1,    -1,    -1,   194,    -1,    -1,
       9,    -1,    -1,    -1,    -1,    -1,   203,    -1,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      -1,    52,    53,    54,    55,    56,    57,    -1,    -1,    -1,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    73,    52,    53,    54,    55,    56,    57,     9,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    73,   194,   195,   196,    -1,    -1,
      -1,    -1,    -1,    -1,   203,    -1,    -1,    -1,    -1,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    -1,    52,    53,    54,    55,    56,    57,     9,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,
     200,    -1,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    73,    -1,    -1,    76,    77,     8,     9,   200,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   200,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,     9,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     200,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      80,    73,    -1,    96,    97,    98,    99,   100,   199,   102,
      -1,    -1,    -1,    -1,   107,    -1,    96,    97,   111,    99,
     100,    -1,   102,    -1,    -1,    -1,    -1,   107,    -1,    -1,
      -1,   111,    -1,    -1,    -1,   128,   129,    -1,   131,    -1,
      -1,    -1,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   144,   145,   146,   147,    -1,    -1,
      -1,   164,    80,    -1,   195,    -1,    -1,   170,    -1,    -1,
      -1,    -1,    -1,    -1,   164,    80,    -1,    -1,    96,    97,
     170,    99,   100,    -1,   102,    -1,    -1,    -1,    -1,   107,
      -1,    96,    97,    -1,    99,   100,    -1,   102,   201,    -1,
      -1,    -1,   107,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,   201,    -1,   195,    -1,   218,    -1,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   144,   145,   218,   147,
     135,   136,    81,    82,    -1,    -1,    -1,    -1,    87,   144,
     145,   146,   147,    -1,    -1,    -1,   164,    96,    97,    98,
      99,   100,   170,   102,   103,   104,    -1,    -1,   107,   164,
      -1,    -1,   111,    -1,    -1,   170,    -1,    -1,    -1,    -1,
      -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,   128,
     129,    -1,   131,   201,    -1,    -1,   135,   136,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   144,   145,   146,    -1,    -1,
     218,    81,    82,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   164,    96,    97,    98,    99,
     100,   170,   102,   103,   104,    -1,    -1,   107,    81,    82,
      -1,   111,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
      -1,   121,    -1,    96,    97,    98,    99,   100,    -1,   102,
     103,   104,   201,    -1,   107,   135,   136,    -1,   111,    -1,
      -1,    -1,    -1,    -1,   144,   145,   146,    -1,   121,   218,
      -1,    -1,    -1,    -1,    -1,   128,   129,    -1,   131,    81,
      82,    -1,    -1,    -1,   164,    87,    -1,    -1,    -1,    -1,
     170,   144,   145,   146,    96,    97,    98,    99,   100,    -1,
     102,   103,   104,    -1,    -1,   107,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
      -1,   201,    -1,    -1,    81,    82,   128,   129,    -1,   131,
      87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   218,    96,
      97,    98,    99,   100,    -1,   102,   103,   104,   201,    -1,
     107,    -1,    81,    82,   111,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,   121,   218,    -1,    96,    97,    98,
      99,   100,    -1,   102,   103,   104,    -1,    -1,   107,    81,
      82,    -1,   111,    -1,    -1,    87,    -1,   144,   145,   146,
      -1,    -1,   121,    -1,    96,    97,    98,    99,   100,   201,
     102,   103,   104,    -1,    -1,   107,    -1,    -1,    -1,   111,
      -1,    87,    -1,    -1,    -1,    -1,   218,   146,    -1,   121,
      96,    97,    -1,    99,   100,    -1,   102,   103,   104,    -1,
      -1,   107,    96,    97,    98,    99,   100,    -1,   102,    -1,
      -1,    -1,    -1,    -1,   201,   121,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,
      -1,   218,    -1,    -1,   128,   129,    -1,   131,    -1,    80,
      81,    82,   201,    96,    97,    98,    99,   100,    -1,   102,
     144,   145,   146,    -1,   107,    96,    97,    98,   111,   218,
      -1,   102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   201,
     111,    -1,    -1,    -1,    -1,   128,   129,    -1,   131,    -1,
      -1,    -1,   135,   136,    -1,    -1,   218,   128,   129,    -1,
     131,   144,   145,   146,   147,   201,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    82,    -1,   201,    -1,    -1,
      87,   164,   218,    -1,    -1,    -1,    -1,   170,    -1,    96,
      97,    98,    99,   100,   218,   102,   103,   104,    -1,    -1,
     107,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,
      -1,   128,   129,    -1,   131,    81,    82,    -1,   135,   136,
      -1,    87,    -1,    -1,    -1,    -1,    -1,   144,   145,   146,
      96,    97,    98,    99,   100,    -1,   102,   103,   104,    -1,
      -1,   107,    -1,    -1,    -1,   111,    -1,   164,    -1,    -1,
      -1,    -1,    -1,   170,    -1,   121,    -1,    -1,    -1,    78,
      79,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    90,    91,    92,    93,    94,    95,    -1,   144,   145,
     146,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,   108,
      -1,   110,    -1,    -1,    -1,   114,    -1,    -1,   164,    -1,
      -1,    -1,    -1,    -1,   170,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   142,   143,    78,    79,    -1,    81,    -1,
      -1,    -1,   151,    -1,   153,    -1,    -1,    90,    91,    92,
      93,    94,    95,    -1,    -1,    -1,    78,    79,    -1,    81,
      -1,    78,    79,   106,    81,   108,    88,   110,    90,    91,
      92,   114,    94,    90,    91,    92,    -1,    94,    -1,    -1,
      -1,   124,    -1,    -1,   106,    -1,   108,    -1,    -1,   106,
     133,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   142,
     143,    -1,   124,    -1,    -1,    -1,    -1,   124,   151,    -1,
     153,   133,     9,    -1,    -1,    -1,   133,    -1,    -1,    -1,
     142,   143,    -1,    -1,    -1,   142,   143,    -1,    -1,   151,
      -1,    -1,    -1,    -1,   151,    -1,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    -1,    -1,    -1,    -1,    -1,    81,    82,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    73,    -1,    -1,    76,
      77,    96,    97,    98,    99,   100,    -1,   102,   103,   104,
      -1,    -1,   107,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    81,    82,    -1,    -1,   121,    -1,    87,    -1,
      -1,    -1,    -1,   128,   129,    -1,   131,    96,    97,    98,
      99,   100,    -1,   102,   103,   104,    -1,    -1,   107,   144,
     145,   146,   111,    -1,    -1,    -1,    -1,    -1,    81,    82,
      -1,    -1,   121,    -1,    87,    -1,    -1,    -1,    -1,   128,
     129,    -1,   131,    96,    97,    98,    99,   100,    -1,   102,
     103,   104,    -1,    -1,   107,    81,    82,   146,   111,    -1,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,
      96,    97,    98,    99,   100,    -1,   102,   103,   104,    -1,
      -1,   107,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,   144,   145,   146,    -1,   121,     9,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,     9,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,    -1,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    -1,    -1,    -1,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      73,    52,    53,    54,    55,    56,    57,    -1,    -1,    81,
      82,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    73,    -1,    96,    97,    98,    99,   100,    -1,
     102,   103,   104,    -1,    -1,   107,    -1,   140,    -1,   111,
      -1,    -1,    -1,   146,    -1,   148,    -1,   150,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,   128,   129,   161,   131,
     163,    -1,   165,    -1,    -1,    -1,    -1,    -1,   171,    -1,
     173,    -1,   175,    -1,   177,    -1,   179,    -1,   181,    -1,
     183,    -1,   185,    81,    82,    -1,   189,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    96,    97,
      98,    99,   100,    -1,   102,   103,   104,    -1,    -1,   107,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   121
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     5,     6,     7,     8,     9,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      73,    75,    76,    77,   175,   194,   203,   220,   244,   245,
     249,   250,   251,   252,   258,   261,   262,   263,   264,   265,
     274,   292,   293,   294,   295,   298,   314,   367,   368,   369,
     414,   415,   417,   530,   533,   574,   576,     8,    10,    11,
      13,    14,    33,    70,    71,    72,   194,   202,   203,   204,
     205,   206,   207,   222,   223,   224,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   242,     8,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    74,    75,    76,   155,   175,   199,
     217,   242,   244,   245,   280,   282,   283,   285,   286,   287,
     288,   289,   290,   291,   302,   321,   322,   324,   325,   332,
     333,   341,   342,   348,   349,   354,   355,   372,   373,   376,
     377,   380,   381,   396,   398,   402,   405,   408,   422,   423,
     429,   430,   435,   436,   439,   440,   443,   444,   447,   448,
     451,   452,   455,   456,   459,   460,   463,   464,   467,   468,
     471,   472,   475,   476,   479,   480,   483,   484,   492,   493,
     496,   497,   500,   501,   502,   503,   506,   507,   508,   509,
     510,   511,   512,   515,   518,   519,   520,   527,   529,   566,
     567,   568,   573,   574,   582,   583,   292,   312,   313,   314,
     194,   132,   531,   176,   263,   261,   265,   266,     0,   217,
     246,   247,   263,   245,   245,   312,   245,   245,   297,   300,
     194,   196,   264,   293,    76,   245,   294,   319,   369,   244,
     294,   318,   320,   245,   263,   295,   194,   194,   226,   226,
     194,   226,   194,   194,   194,   242,   250,   255,   261,   271,
      11,    12,    13,    14,   194,   196,   198,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,   216,   241,   226,
     228,   203,   208,   209,   204,   205,    15,    16,    17,    18,
     210,   211,    19,    20,   202,   212,   213,    21,    22,   214,
     201,   215,   239,   243,   215,   194,   194,   194,    76,   175,
     280,   194,     8,   217,   217,   217,   242,    49,    51,    66,
     194,   303,   304,   305,    78,    79,    81,    90,    91,    92,
      93,    94,    95,   106,   108,   110,   114,   124,   133,   142,
     143,   151,   153,    78,    79,   106,   151,   528,   108,   177,
     182,   200,   284,   217,   263,   286,   280,   323,   290,   199,
     345,   323,   290,   290,   290,   323,   397,   323,   290,   290,
     290,   323,   323,   290,   290,   290,   290,   290,   290,   290,
     290,   323,   290,   345,   323,   323,   323,   287,   323,     8,
       8,   199,   274,   314,   194,   102,   124,   133,   194,   577,
     195,   261,   265,   201,   217,   216,   296,   299,     8,   199,
     274,   283,   244,   301,     8,   195,   245,   267,   268,   269,
     270,    36,   197,   203,   240,   266,   132,   263,    76,   318,
     416,   263,   575,   225,   240,   271,   271,   240,   271,   271,
     195,   255,   194,   196,   265,   272,   273,   255,   195,     8,
     274,   225,   242,     8,   274,   240,   228,   228,   228,   229,
     229,   230,   230,   231,   231,   231,   231,   232,   232,   233,
     234,   235,   236,   237,   242,   240,   312,   215,    76,   280,
     281,   322,   242,   242,   242,   133,    63,   217,   242,   244,
     217,   217,   194,   222,   303,   305,    79,   106,   326,   343,
     147,   521,   523,   524,   350,   218,   194,   218,   504,   218,
     115,   116,   117,   118,   513,   194,   218,   516,   124,   334,
     485,   218,   218,   356,   118,   134,   142,   162,   163,   399,
     143,   424,    78,   124,   431,   218,   528,   218,   584,   218,
     569,   285,    76,   323,   346,   347,   323,   194,   199,     8,
     221,   259,   260,   199,     8,     9,    10,   315,   316,   317,
     194,   370,   138,   169,   194,   218,   418,   419,     8,   565,
     578,   247,   248,   283,   301,   199,   200,   253,   254,   255,
     199,   244,   283,   194,   263,   265,   272,   195,   201,   195,
     201,   240,   266,   197,   197,    36,   197,   203,   240,   141,
     283,   195,   201,   195,   195,   201,   201,   201,   195,   267,
     272,   197,   203,   240,   273,   194,   196,   199,   228,   195,
     197,   215,   280,   281,   195,   195,   195,   194,   217,   242,
     217,   217,   242,   222,   195,   215,   498,   124,   494,    87,
      96,    97,    99,   100,   102,   103,   104,   107,   121,   201,
     218,   327,   328,   539,   540,   542,   546,   548,   550,   564,
      80,    98,   201,   218,   344,   540,   542,   544,   548,   194,
     124,   168,   201,   218,   522,   147,   201,   218,   524,    80,
     105,   201,   218,   351,   352,   540,   542,     8,   156,   218,
     505,   152,   514,   517,   218,   374,    80,    81,    82,   111,
     201,   218,   335,   336,   540,   542,   544,   548,   554,   109,
     112,   113,   147,   166,   201,   218,   486,   487,   489,   539,
     540,   542,   546,   550,   128,   129,   131,   201,   218,   357,
     358,   362,   364,   540,   544,   548,   554,    80,   135,   138,
     139,   384,   409,   410,   411,   489,   550,   382,   143,   449,
     134,   134,    80,   136,   164,   170,   201,   218,   384,   386,
     393,   394,   400,   401,   489,   540,   542,   550,    78,   124,
     453,   144,   145,   201,   218,   425,   426,   539,   540,   542,
     546,   548,   106,   437,   146,   201,   218,   432,   433,   540,
     542,   554,   201,   218,   550,    61,   109,   156,   179,   180,
     181,   193,   201,   218,   585,   570,   200,    88,    76,   200,
     347,   323,   225,   259,   216,   200,   201,   259,   195,   201,
     194,     8,    21,    22,   119,   120,   202,   203,   204,   205,
     212,   213,   536,   125,   126,   127,   130,   201,   218,   359,
     360,   362,   364,   371,   420,   421,     8,   558,   559,   201,
     218,   419,   195,   201,   102,   148,   149,   150,   201,   218,
     579,   199,   240,   275,   283,   253,   200,   254,   215,   217,
     256,   257,   263,   253,    58,   269,     8,   197,   240,   240,
     197,   197,   132,   240,   271,     8,   271,   195,   195,   197,
     197,   195,   267,   197,   203,   240,   196,   198,   275,   276,
     277,   278,   279,   239,   280,   280,   280,   242,   195,   242,
     217,   217,   242,   195,   242,   217,   215,   196,   222,   306,
     307,   201,   218,   328,   499,   539,   540,   542,   544,   546,
     548,   378,   201,   218,   328,   336,   495,   539,   540,   542,
     544,   546,   548,   331,   541,   543,   547,   194,   194,   330,
     194,   194,   194,   327,   545,   344,   157,   158,   522,   194,
     524,   353,   351,   195,   194,   218,   218,     8,   555,   201,
     218,   335,   358,   375,   194,   194,   194,   335,   194,   194,
     194,   486,   363,   365,   194,   357,   194,   412,   413,   201,
     218,   410,   165,   201,   218,   383,   384,   386,   391,   550,
      78,   124,   461,    80,   201,   218,   401,   425,   450,   489,
     550,    80,   384,   386,   403,   404,   489,   550,    80,   384,
     386,   406,   407,   489,   550,   194,   194,   194,   400,   106,
     457,   201,   218,   425,   433,   454,   554,   194,   194,   425,
     124,   441,   201,   218,   357,   433,   438,   542,   194,   432,
     550,   218,   194,   194,   194,   194,   585,   109,   183,   184,
     185,   193,   201,   218,   571,   218,   323,   195,   200,   201,
     243,   200,   260,   200,   201,   195,   316,   225,   215,   361,
     194,   371,   194,   194,   196,   560,   195,   201,   419,   532,
       8,   194,   194,   194,   194,   579,   276,   200,   243,   201,
     217,   215,   200,   197,   197,   133,   195,   195,   195,   195,
     197,   197,   243,     8,   274,   200,   201,   275,   216,   279,
       4,   195,   280,   195,   195,   242,   195,   242,   217,   280,
     195,   195,   242,   215,     8,   194,   195,   201,   215,   499,
     201,   218,   358,   379,   495,   495,   194,   194,   194,   194,
      87,    99,   101,   536,   194,   329,    78,   108,   133,   153,
     551,   553,    91,   122,   123,   174,   194,   195,   215,   194,
     242,   195,   201,   375,   242,    89,   172,   173,   340,   242,
     488,   148,   149,   150,   491,   242,   194,   194,   242,   385,
     194,   194,   410,   194,   383,   106,   465,    80,   201,   218,
     401,   454,   462,   489,   550,   450,   201,   218,   404,   201,
     218,   407,   161,   388,   389,   395,   140,   124,   469,   201,
     218,   426,   438,   458,   539,   546,   454,   427,   428,   445,
     201,   218,   433,   442,   495,   438,    83,   218,   242,   242,
     190,   191,   203,   240,   240,   194,   194,   194,   571,   217,
     200,   200,   195,   250,   537,   194,   242,   558,     8,   556,
     557,   562,   196,   218,   559,   218,   536,     8,   580,   581,
     580,   580,   200,   201,   257,   243,   218,   197,   200,   275,
     277,   280,   217,   280,   280,   195,   280,   195,   195,   242,
     280,   280,   195,   196,   222,   308,   309,   197,   226,   307,
     308,   379,   555,   555,   555,   555,   195,   195,   195,   549,
     555,   242,   118,   134,   162,   163,   242,   215,   195,   195,
     195,   195,   555,   218,     8,   525,   526,   555,   195,     8,
     195,   201,   215,   215,     1,    83,    84,    85,    86,    87,
     339,   195,   242,   490,   195,   555,   555,   195,   242,   556,
     556,   392,   124,   473,    80,   201,   218,   401,   458,   466,
     489,   550,   462,   404,   407,   201,   556,   137,   138,   139,
     140,   159,   160,   390,   555,   215,   477,   201,   218,   426,
     442,   470,   458,   242,   242,   201,   218,   379,   433,   446,
     442,   195,   201,   195,   195,   195,   195,   195,   195,   195,
     240,   240,   186,   187,   572,   201,   215,   555,   195,   195,
     560,   195,   201,   215,   242,   563,   561,   215,   196,   195,
     201,   195,   195,   200,   275,   280,   280,   280,   195,   280,
       8,   194,   201,   215,   222,   195,   195,   215,   195,   195,
     195,   195,   215,   195,   195,   134,   134,   195,   552,   195,
     204,   205,   195,   201,   195,   338,   195,   201,   195,   215,
     215,   366,   366,   195,   195,   195,   555,   481,   201,   218,
     401,   470,   474,   466,   195,   215,   195,   171,   201,   218,
     426,   446,   478,   470,   195,   195,   446,   434,   195,   195,
     201,   195,   250,   534,   195,   557,   242,   215,   197,   563,
     580,   214,   240,   581,   280,   197,   242,   309,   222,   310,
     194,   310,   556,   242,   229,   229,   525,   242,   337,   556,
     242,   195,   195,   195,   201,   218,   401,   478,   482,   474,
     387,   195,   478,   242,   188,   189,   242,   242,   197,   195,
       8,   197,   222,   195,   201,   215,   226,   195,   195,   195,
     195,   242,   195,   482,   556,   195,   195,   197,   194,   222,
       8,   311,   195,   195,   195,   535,   242,   195,   201,   154,
     538,   195,     8,   194,   218,     8,   194,   216,   225,   239,
     195,   195,   195
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   219,   220,   220,   220,   220,   221,   222,   222,   223,
     223,   223,   223,   224,   224,   224,   224,   224,   224,   224,
     224,   224,   224,   224,   224,   225,   225,   225,   226,   226,
     226,   226,   226,   226,   226,   226,   226,   227,   227,   227,
     227,   227,   227,   228,   228,   229,   229,   229,   229,   230,
     230,   230,   231,   231,   231,   232,   232,   232,   232,   232,
     233,   233,   233,   234,   234,   235,   235,   236,   236,   237,
     237,   238,   238,   239,   239,   240,   240,   241,   241,   241,
     241,   241,   241,   241,   241,   241,   241,   241,   242,   242,
     243,   244,   244,   244,   244,   244,   244,   245,   245,   245,
     245,   245,   245,   245,   245,   246,   246,   247,   248,   247,
     249,   249,   249,   249,   249,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,   250,
     251,   251,   251,   251,   251,   251,   252,   252,   253,   253,
     254,   254,   255,   255,   255,   255,   256,   256,   257,   257,
     257,   258,   258,   258,   258,   258,   258,   258,   258,   259,
     259,   260,   260,   261,   261,   261,   261,   262,   263,   263,
     264,   264,   264,   264,   264,   264,   264,   264,   264,   264,
     264,   264,   264,   264,   265,   265,   265,   265,   266,   266,
     267,   267,   268,   268,   269,   269,   269,   270,   270,   271,
     271,   272,   272,   272,   273,   273,   273,   273,   273,   273,
     273,   273,   273,   273,   273,   274,   275,   275,   275,   276,
     276,   276,   276,   277,   278,   278,   279,   279,   279,   280,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   281,
     281,   282,   282,   282,   283,   284,   283,   285,   285,   286,
     286,   286,   286,   287,   287,   288,   288,   288,   289,   289,
     289,   290,   290,   290,   290,   290,   290,   290,   290,   290,
     290,   290,   290,   291,   291,   291,   291,   291,   292,   292,
     293,   293,   293,   293,   294,   294,   296,   295,   297,   295,
     299,   298,   300,   298,   301,   301,   302,   302,   303,   303,
     303,   303,   303,   304,   304,   305,   305,   306,   306,   306,
     307,   307,   308,   308,   308,   309,   309,   310,   310,   311,
     311,   312,   312,   313,   313,   314,   315,   315,   316,   316,
     316,   317,   317,   317,   318,   318,   319,   320,   320,   321,
     321,   321,   321,   321,   321,   321,   321,   321,   321,   321,
     321,   321,   321,   321,   321,   321,   321,   321,   321,   321,
     321,   321,   321,   321,   321,   321,   321,   321,   321,   321,
     322,   322,   322,   322,   322,   322,   322,   322,   322,   323,
     324,   325,   326,   326,   326,   327,   327,   327,   327,   327,
     327,   328,   329,   328,   330,   328,   328,   331,   328,   332,
     333,   334,   334,   334,   335,   335,   335,   335,   335,   335,
     336,   336,   336,   337,   336,   338,   336,   336,   339,   339,
     339,   339,   339,   339,   340,   340,   340,   341,   342,   343,
     343,   343,   344,   344,   344,   344,   344,   345,   346,   346,
     346,   347,   348,   349,   350,   350,   350,   351,   351,   351,
     351,   353,   352,   354,   355,   356,   356,   356,   357,   357,
     357,   357,   357,   358,   358,   358,   359,   359,   361,   360,
     363,   362,   365,   364,   366,   366,   367,   368,   368,   369,
     370,   370,   370,   371,   371,   371,   371,   371,   372,   373,
     374,   374,   374,   375,   375,   376,   377,   378,   378,   378,
     379,   379,   380,   381,   382,   382,   382,   383,   383,   383,
     383,   385,   384,   387,   386,   388,   386,   389,   389,   389,
     390,   390,   390,   390,   390,   390,   392,   391,   393,   395,
     394,   397,   396,   398,   399,   399,   399,   400,   400,   400,
     400,   400,   400,   401,   401,   401,   401,   402,   403,   403,
     403,   404,   404,   404,   404,   404,   405,   406,   406,   406,
     407,   407,   407,   407,   407,   408,   409,   409,   409,   410,
     410,   410,   410,   410,   412,   411,   413,   411,   414,   414,
     415,   416,   417,   417,   418,   418,   418,   420,   419,   421,
     419,   422,   423,   424,   424,   424,   425,   425,   425,   425,
     425,   425,   427,   426,   428,   426,   429,   430,   431,   431,
     431,   432,   432,   432,   432,   433,   434,   433,   435,   436,
     437,   437,   437,   438,   438,   438,   439,   440,   441,   441,
     441,   442,   442,   443,   444,   445,   445,   445,   446,   446,
     447,   448,   449,   449,   449,   450,   450,   450,   450,   450,
     451,   452,   453,   453,   453,   454,   454,   454,   455,   456,
     457,   457,   457,   458,   458,   458,   458,   459,   460,   461,
     461,   461,   462,   462,   462,   462,   462,   463,   464,   465,
     465,   465,   466,   466,   466,   466,   466,   467,   468,   469,
     469,   469,   470,   470,   471,   472,   473,   473,   473,   474,
     474,   475,   476,   477,   477,   477,   478,   478,   479,   480,
     481,   481,   481,   482,   482,   483,   484,   485,   485,   485,
     486,   486,   486,   486,   486,   486,   487,   488,   487,   487,
     487,   487,   490,   489,   491,   491,   491,   492,   493,   494,
     494,   494,   495,   495,   495,   495,   495,   495,   495,   495,
     496,   497,   498,   498,   498,   499,   499,   499,   499,   499,
     499,   499,   500,   501,   502,   503,   503,   503,   504,   505,
     506,   507,   508,   509,   510,   511,   512,   513,   513,   513,
     513,   513,   514,   514,   515,   515,   517,   516,   518,   518,
     519,   520,   520,   521,   521,   521,   522,   522,   523,   523,
     523,   524,   525,   525,   526,   526,   526,   527,   527,   527,
     528,   528,   528,   528,   529,   531,   532,   530,   534,   535,
     533,   536,   536,   536,   536,   536,   536,   536,   536,   536,
     536,   536,   537,   537,   538,   538,   538,   539,   539,   539,
     541,   540,   543,   542,   545,   544,   547,   546,   549,   548,
     551,   550,   552,   550,   553,   553,   553,   553,   553,   553,
     553,   553,   554,   555,   555,   556,   556,   557,   557,   558,
     558,   559,   559,   561,   560,   562,   560,   563,   563,   563,
     563,   563,   564,   564,   564,   564,   565,   565,   566,   566,
     567,   569,   568,   570,   570,   570,   571,   571,   571,   571,
     571,   572,   572,   572,   572,   573,   573,   575,   574,   574,
     577,   576,   578,   578,   578,   579,   579,   579,   579,   580,
     580,   581,   581,   581,   582,   583,   584,   584,   584,   585,
     585,   585,   585,   585,   585,   585,   585,   585,   585
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     2,     2,     1,     1,     2,     1,
       1,     1,     3,     1,     4,     4,     4,     3,     3,     3,
       3,     2,     2,     6,     7,     0,     1,     3,     1,     2,
       2,     2,     2,     4,     6,     6,     6,     1,     1,     1,
       1,     1,     1,     1,     4,     1,     3,     3,     3,     1,
       3,     3,     1,     3,     3,     1,     3,     3,     3,     3,
       1,     3,     3,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     5,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     2,     3,     1,     1,     1,     1,     1,     2,     1,
       2,     1,     2,     1,     2,     1,     3,     1,     0,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       5,     4,     6,     6,     3,     3,     1,     1,     1,     2,
       3,     2,     1,     2,     1,     2,     1,     3,     1,     3,
       2,     5,     6,     6,     6,     7,     7,     3,     3,     1,
       3,     1,     3,     1,     1,     1,     1,     1,     1,     2,
       1,     3,     3,     4,     4,     5,     5,     6,     6,     4,
       5,     4,     3,     4,     1,     2,     2,     3,     1,     2,
       1,     3,     1,     3,     2,     1,     2,     1,     3,     1,
       2,     1,     1,     2,     3,     2,     3,     3,     4,     3,
       4,     2,     3,     3,     4,     1,     1,     3,     4,     1,
       2,     3,     4,     2,     1,     2,     3,     2,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     4,     3,     2,     0,     4,     1,     2,     1,
       1,     1,     1,     1,     2,     5,     7,     5,     5,     7,
       1,     6,     7,     7,     7,     8,     8,     8,     9,     6,
       7,     7,     8,     3,     2,     2,     2,     3,     1,     2,
       1,     1,     1,     1,     1,     1,     0,     4,     0,     3,
       0,     5,     0,     4,     1,     2,     2,     3,     3,     5,
       7,     9,    11,     1,     2,     1,     1,     0,     1,     3,
       7,     4,     0,     1,     3,     7,     4,     1,     3,     1,
       3,     0,     1,     1,     2,     6,     1,     3,     0,     1,
       4,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     4,     0,     2,     3,     1,     1,     1,     1,     1,
       1,     1,     0,     5,     0,     5,     1,     0,     5,     2,
       4,     0,     2,     3,     1,     1,     1,     1,     1,     1,
       1,     4,     5,     0,     8,     0,     7,     1,     1,     1,
       1,     1,     1,     1,     0,     2,     2,     2,     4,     0,
       2,     3,     1,     1,     1,     1,     1,     3,     1,     2,
       3,     3,     2,     4,     0,     2,     3,     1,     1,     1,
       1,     0,     5,     2,     4,     0,     2,     3,     1,     1,
       1,     1,     1,     4,     1,     1,     1,     1,     0,     5,
       0,     6,     0,     6,     0,     2,     2,     1,     2,     5,
       0,     2,     3,     4,     1,     1,     1,     1,     2,     5,
       0,     2,     3,     1,     1,     2,     6,     0,     2,     3,
       1,     1,     2,     5,     0,     2,     3,     1,     1,     1,
       1,     0,     5,     0,     8,     0,     5,     0,     1,     2,
       1,     1,     1,     1,     1,     1,     0,     5,     6,     0,
       5,     0,     3,     4,     0,     2,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     6,     1,     2,
       3,     1,     1,     1,     1,     1,     6,     1,     2,     3,
       1,     1,     1,     1,     1,     5,     1,     2,     3,     1,
       1,     1,     1,     1,     0,     5,     0,     5,     3,     1,
       4,     5,     7,     5,     1,     2,     3,     0,     5,     0,
       5,     2,     4,     0,     2,     3,     1,     1,     1,     1,
       1,     1,     0,     5,     0,     5,     2,     4,     0,     2,
       3,     1,     1,     1,     1,     4,     0,     7,     2,     5,
       0,     2,     3,     1,     1,     1,     2,     6,     0,     2,
       3,     1,     1,     2,     7,     0,     2,     3,     1,     1,
       2,     5,     0,     2,     3,     1,     1,     1,     1,     1,
       2,     5,     0,     2,     3,     1,     1,     1,     2,     6,
       0,     2,     3,     1,     1,     1,     1,     2,     6,     0,
       2,     3,     1,     1,     1,     1,     1,     2,     7,     0,
       2,     3,     1,     1,     1,     1,     1,     2,     7,     0,
       2,     3,     1,     1,     2,     8,     0,     2,     3,     1,
       1,     2,     8,     0,     2,     3,     1,     1,     2,     9,
       0,     2,     3,     1,     1,     2,     4,     0,     2,     3,
       1,     1,     1,     1,     1,     1,     1,     0,     5,     1,
       1,     4,     0,     7,     1,     1,     1,     2,     5,     0,
       2,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     5,     0,     2,     3,     1,     1,     1,     1,     1,
       1,     1,     2,     3,     2,     3,     4,     5,     3,     4,
       3,     3,     2,     3,     3,     2,     5,     0,     1,     1,
       1,     1,     0,     1,     3,     4,     0,     4,     2,     1,
       4,     7,     4,     0,     2,     3,     1,     1,     1,     2,
       3,     6,     1,     3,     1,     3,     3,     4,     5,     6,
       1,     1,     1,     1,     3,     0,     0,     7,     0,     0,
      14,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     0,     6,     7,     4,     4,     4,
       0,     5,     0,     5,     0,     5,     0,     5,     0,     7,
       0,     5,     0,     7,     1,     1,     1,     2,     3,     3,
       2,     1,     4,     1,     3,     1,     3,     1,     2,     1,
       3,     1,     2,     0,     5,     0,     4,     3,     2,     1,
       2,     1,     4,     4,     4,     4,     1,     3,     1,     1,
       3,     0,     5,     0,     2,     3,     4,     4,     4,     1,
       1,     1,     1,     3,     3,     1,     1,     0,     4,     2,
       0,     5,     0,     2,     3,     4,     4,     4,     6,     1,
       3,     1,     5,     4,     6,     4,     0,     2,     3,     4,
       4,     4,     4,     4,     1,     1,     1,     4,     4
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
#line 543 "parser.y" /* yacc.c:1646  */
    { /* to avoid warnings */ }
#line 3707 "parser.c" /* yacc.c:1646  */
    break;

  case 3:
#line 544 "parser.y" /* yacc.c:1646  */
    { pastree_expr = (yyvsp[0].expr); }
#line 3713 "parser.c" /* yacc.c:1646  */
    break;

  case 4:
#line 545 "parser.y" /* yacc.c:1646  */
    { pastree_stmt = (yyvsp[0].stmt); }
#line 3719 "parser.c" /* yacc.c:1646  */
    break;

  case 5:
#line 546 "parser.y" /* yacc.c:1646  */
    { pastree_stmt = (yyvsp[0].stmt); }
#line 3725 "parser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 564 "parser.y" /* yacc.c:1646  */
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
#line 3740 "parser.c" /* yacc.c:1646  */
    break;

  case 7:
#line 585 "parser.y" /* yacc.c:1646  */
    {
      (yyval.string) = strdup((yyvsp[0].name));
    }
#line 3748 "parser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 589 "parser.y" /* yacc.c:1646  */
    {
      /* Or we could leave it as is (as a SpaceList) */
      if (((yyvsp[-1].string) = realloc((yyvsp[-1].string), strlen((yyvsp[-1].string)) + strlen((yyvsp[0].name)))) == NULL)
        parse_error(-1, "string out of memory\n");
      strcpy(((yyvsp[-1].string))+(strlen((yyvsp[-1].string))-1),((yyvsp[0].name))+1);  /* Catenate on the '"' */
      (yyval.string) = (yyvsp[-1].string);
    }
#line 3760 "parser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 612 "parser.y" /* yacc.c:1646  */
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
      (yyval.expr) = chflag ? Parenthesis(Deref(Identifier(id)))
                  : Identifier(id);
    }
#line 3786 "parser.c" /* yacc.c:1646  */
    break;

  case 10:
#line 634 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Constant( strdup((yyvsp[0].name)) );
    }
#line 3794 "parser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 638 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = String((yyvsp[0].string));
    }
#line 3802 "parser.c" /* yacc.c:1646  */
    break;

  case 12:
#line 642 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Parenthesis((yyvsp[-1].expr));
    }
#line 3810 "parser.c" /* yacc.c:1646  */
    break;

  case 13:
#line 650 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 3818 "parser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 654 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = ArrayIndex((yyvsp[-3].expr), (yyvsp[-1].expr));
    }
#line 3826 "parser.c" /* yacc.c:1646  */
    break;

  case 15:
#line 664 "parser.y" /* yacc.c:1646  */
    {
      /* Catch calls to "main()" (unlikely but possible) */
      (yyval.expr) = strcmp((yyvsp[-3].name), "main") ?
             FunctionCall(IdentName((yyvsp[-3].name)), (yyvsp[-1].expr)) :
             FunctionCall(IdentName(MAIN_NEWNAME), (yyvsp[-1].expr));
    }
#line 3837 "parser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 671 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = FunctionCall((yyvsp[-3].expr), (yyvsp[-1].expr));
    }
#line 3845 "parser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 675 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = DotField((yyvsp[-2].expr), Symbol((yyvsp[0].name)));
    }
#line 3853 "parser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 679 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = PtrField((yyvsp[-2].expr), Symbol((yyvsp[0].name)));
    }
#line 3861 "parser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 688 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = DotField((yyvsp[-2].expr), (yyvsp[0].symb));
    }
#line 3869 "parser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 692 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = PtrField((yyvsp[-2].expr), (yyvsp[0].symb));
    }
#line 3877 "parser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 696 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = PostOperator((yyvsp[-1].expr), UOP_inc);
    }
#line 3885 "parser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 700 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = PostOperator((yyvsp[-1].expr), UOP_dec);
    }
#line 3893 "parser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 704 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = CastedExpr((yyvsp[-4].decl), BracedInitializer((yyvsp[-1].expr)));
    }
#line 3901 "parser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 708 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = CastedExpr((yyvsp[-5].decl), BracedInitializer((yyvsp[-2].expr)));
    }
#line 3909 "parser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 716 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = NULL;
    }
#line 3917 "parser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 720 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 3925 "parser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 724 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = CommaList((yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 3933 "parser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 732 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 3941 "parser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 736 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = PreOperator((yyvsp[0].expr), UOP_inc);
    }
#line 3949 "parser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 740 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = PreOperator((yyvsp[0].expr), UOP_dec);
    }
#line 3957 "parser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 744 "parser.y" /* yacc.c:1646  */
    {
      if ((yyvsp[-1].type) == -1)
        (yyval.expr) = (yyvsp[0].expr);                    /* simplify */
      else
        (yyval.expr) = UnaryOperator((yyvsp[-1].type), (yyvsp[0].expr));
    }
#line 3968 "parser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 751 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Sizeof((yyvsp[0].expr));
    }
#line 3976 "parser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 755 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Sizeoftype((yyvsp[-1].decl));
    }
#line 3984 "parser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 764 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = FunctionCall(IdentName("__builtin_va_arg"),
                        CommaList((yyvsp[-3].expr), TypeTrick((yyvsp[-1].decl))));
    }
#line 3993 "parser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 769 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = FunctionCall(IdentName("__builtin_offsetof"),
                        CommaList(TypeTrick((yyvsp[-3].decl)), IdentName((yyvsp[-1].name))));
    }
#line 4002 "parser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 774 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = FunctionCall(IdentName("__builtin_types_compatible_p"),
                        CommaList(TypeTrick((yyvsp[-3].decl)), TypeTrick((yyvsp[-1].decl))));
    }
#line 4011 "parser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 783 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = UOP_addr;
    }
#line 4019 "parser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 787 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = UOP_star;
    }
#line 4027 "parser.c" /* yacc.c:1646  */
    break;

  case 39:
#line 791 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = -1;         /* Ingore this one */
    }
#line 4035 "parser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 795 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = UOP_neg;
    }
#line 4043 "parser.c" /* yacc.c:1646  */
    break;

  case 41:
#line 799 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = UOP_bnot;
    }
#line 4051 "parser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 803 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = UOP_lnot;
    }
#line 4059 "parser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 811 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4067 "parser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 815 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = CastedExpr((yyvsp[-2].decl), (yyvsp[0].expr));
    }
#line 4075 "parser.c" /* yacc.c:1646  */
    break;

  case 45:
#line 823 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4083 "parser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 827 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_mul, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4091 "parser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 831 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_div, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4099 "parser.c" /* yacc.c:1646  */
    break;

  case 48:
#line 835 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_mod, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4107 "parser.c" /* yacc.c:1646  */
    break;

  case 49:
#line 843 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4115 "parser.c" /* yacc.c:1646  */
    break;

  case 50:
#line 847 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_add, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4123 "parser.c" /* yacc.c:1646  */
    break;

  case 51:
#line 851 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_sub, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4131 "parser.c" /* yacc.c:1646  */
    break;

  case 52:
#line 859 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4139 "parser.c" /* yacc.c:1646  */
    break;

  case 53:
#line 863 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_shl, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4147 "parser.c" /* yacc.c:1646  */
    break;

  case 54:
#line 867 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_shr, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4155 "parser.c" /* yacc.c:1646  */
    break;

  case 55:
#line 875 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4163 "parser.c" /* yacc.c:1646  */
    break;

  case 56:
#line 879 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_lt, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4171 "parser.c" /* yacc.c:1646  */
    break;

  case 57:
#line 883 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_gt, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4179 "parser.c" /* yacc.c:1646  */
    break;

  case 58:
#line 887 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_leq, (yyvsp[-2].expr), (yyvsp[0].expr));
     }
#line 4187 "parser.c" /* yacc.c:1646  */
    break;

  case 59:
#line 891 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_geq, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4195 "parser.c" /* yacc.c:1646  */
    break;

  case 60:
#line 899 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4203 "parser.c" /* yacc.c:1646  */
    break;

  case 61:
#line 903 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_eqeq, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4211 "parser.c" /* yacc.c:1646  */
    break;

  case 62:
#line 907 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_neq, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4219 "parser.c" /* yacc.c:1646  */
    break;

  case 63:
#line 915 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4227 "parser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 919 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_band, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4235 "parser.c" /* yacc.c:1646  */
    break;

  case 65:
#line 927 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4243 "parser.c" /* yacc.c:1646  */
    break;

  case 66:
#line 931 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_xor, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4251 "parser.c" /* yacc.c:1646  */
    break;

  case 67:
#line 939 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4259 "parser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 943 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_bor, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4267 "parser.c" /* yacc.c:1646  */
    break;

  case 69:
#line 951 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4275 "parser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 955 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_land, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4283 "parser.c" /* yacc.c:1646  */
    break;

  case 71:
#line 963 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4291 "parser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 967 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BinaryOperator(BOP_lor, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4299 "parser.c" /* yacc.c:1646  */
    break;

  case 73:
#line 975 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4307 "parser.c" /* yacc.c:1646  */
    break;

  case 74:
#line 979 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = ConditionalExpr((yyvsp[-4].expr), (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4315 "parser.c" /* yacc.c:1646  */
    break;

  case 75:
#line 987 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4323 "parser.c" /* yacc.c:1646  */
    break;

  case 76:
#line 991 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Assignment((yyvsp[-2].expr), (yyvsp[-1].type), (yyvsp[0].expr));
    }
#line 4331 "parser.c" /* yacc.c:1646  */
    break;

  case 77:
#line 999 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_eq;  /* Need fix here! */
    }
#line 4339 "parser.c" /* yacc.c:1646  */
    break;

  case 78:
#line 1003 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_mul;
    }
#line 4347 "parser.c" /* yacc.c:1646  */
    break;

  case 79:
#line 1007 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_div;
    }
#line 4355 "parser.c" /* yacc.c:1646  */
    break;

  case 80:
#line 1011 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_mod;
    }
#line 4363 "parser.c" /* yacc.c:1646  */
    break;

  case 81:
#line 1015 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_add;
    }
#line 4371 "parser.c" /* yacc.c:1646  */
    break;

  case 82:
#line 1019 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_sub;
    }
#line 4379 "parser.c" /* yacc.c:1646  */
    break;

  case 83:
#line 1023 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_shl;
    }
#line 4387 "parser.c" /* yacc.c:1646  */
    break;

  case 84:
#line 1027 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_shr;
    }
#line 4395 "parser.c" /* yacc.c:1646  */
    break;

  case 85:
#line 1031 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_and;
    }
#line 4403 "parser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 1035 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_xor;
    }
#line 4411 "parser.c" /* yacc.c:1646  */
    break;

  case 87:
#line 1039 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = ASS_or;
    }
#line 4419 "parser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 1047 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4427 "parser.c" /* yacc.c:1646  */
    break;

  case 89:
#line 1051 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = CommaList((yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 4435 "parser.c" /* yacc.c:1646  */
    break;

  case 90:
#line 1059 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 4443 "parser.c" /* yacc.c:1646  */
    break;

  case 91:
#line 1073 "parser.y" /* yacc.c:1646  */
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
#line 4462 "parser.c" /* yacc.c:1646  */
    break;

  case 92:
#line 1088 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Declaration((yyvsp[-2].spec), (yyvsp[-1].decl));
      if (checkDecls) add_declaration_links((yyvsp[-2].spec), (yyvsp[-1].decl));
      isTypedef = 0;

    }
#line 4473 "parser.c" /* yacc.c:1646  */
    break;

  case 93:
#line 1095 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpStmt(OmpConstruct(DCTHREADPRIVATE, (yyvsp[0].odir), NULL));
    }
#line 4481 "parser.c" /* yacc.c:1646  */
    break;

  case 94:
#line 1100 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpStmt(OmpConstruct(DCTHREADPRIVATE, $1, NULL)); TODO
    }
#line 4489 "parser.c" /* yacc.c:1646  */
    break;

  case 95:
#line 1104 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpStmt((yyvsp[0].ocon));
    }
#line 4497 "parser.c" /* yacc.c:1646  */
    break;

  case 96:
#line 1108 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpStmt(OmpConstruct(DCTHREADPRIVATE, $1, NULL)); TODO
    }
#line 4505 "parser.c" /* yacc.c:1646  */
    break;

  case 97:
#line 1116 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4513 "parser.c" /* yacc.c:1646  */
    break;

  case 98:
#line 1120 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right((yyvsp[-1].spec), (yyvsp[0].spec));
    }
#line 4521 "parser.c" /* yacc.c:1646  */
    break;

  case 99:
#line 1124 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4529 "parser.c" /* yacc.c:1646  */
    break;

  case 100:
#line 1128 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right((yyvsp[-1].spec), (yyvsp[0].spec));
    }
#line 4537 "parser.c" /* yacc.c:1646  */
    break;

  case 101:
#line 1132 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4545 "parser.c" /* yacc.c:1646  */
    break;

  case 102:
#line 1136 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right((yyvsp[-1].spec), (yyvsp[0].spec));
    }
#line 4553 "parser.c" /* yacc.c:1646  */
    break;

  case 103:
#line 1140 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4561 "parser.c" /* yacc.c:1646  */
    break;

  case 104:
#line 1144 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right((yyvsp[-1].spec), (yyvsp[0].spec));
    }
#line 4569 "parser.c" /* yacc.c:1646  */
    break;

  case 105:
#line 1152 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 4577 "parser.c" /* yacc.c:1646  */
    break;

  case 106:
#line 1156 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = DeclList((yyvsp[-2].decl), (yyvsp[0].decl));
    }
#line 4585 "parser.c" /* yacc.c:1646  */
    break;

  case 107:
#line 1170 "parser.y" /* yacc.c:1646  */
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
#line 4605 "parser.c" /* yacc.c:1646  */
    break;

  case 108:
#line 1186 "parser.y" /* yacc.c:1646  */
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
#line 4624 "parser.c" /* yacc.c:1646  */
    break;

  case 109:
#line 1201 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = InitDecl((yyvsp[-3].decl), (yyvsp[0].expr));
    }
#line 4632 "parser.c" /* yacc.c:1646  */
    break;

  case 110:
#line 1209 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = StClassSpec(SPEC_typedef);    /* Just a string */
      isTypedef = 1;
    }
#line 4641 "parser.c" /* yacc.c:1646  */
    break;

  case 111:
#line 1214 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = StClassSpec(SPEC_extern);
    }
#line 4649 "parser.c" /* yacc.c:1646  */
    break;

  case 112:
#line 1218 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = StClassSpec(SPEC_static);
    }
#line 4657 "parser.c" /* yacc.c:1646  */
    break;

  case 113:
#line 1222 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = StClassSpec(SPEC_auto);
    }
#line 4665 "parser.c" /* yacc.c:1646  */
    break;

  case 114:
#line 1226 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = StClassSpec(SPEC_register);
    }
#line 4673 "parser.c" /* yacc.c:1646  */
    break;

  case 115:
#line 1234 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_void);
    }
#line 4681 "parser.c" /* yacc.c:1646  */
    break;

  case 116:
#line 1238 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_char);
    }
#line 4689 "parser.c" /* yacc.c:1646  */
    break;

  case 117:
#line 1242 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_short);
    }
#line 4697 "parser.c" /* yacc.c:1646  */
    break;

  case 118:
#line 1246 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_int);
    }
#line 4705 "parser.c" /* yacc.c:1646  */
    break;

  case 119:
#line 1250 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_long);
    }
#line 4713 "parser.c" /* yacc.c:1646  */
    break;

  case 120:
#line 1254 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_float);
    }
#line 4721 "parser.c" /* yacc.c:1646  */
    break;

  case 121:
#line 1258 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_double);
    }
#line 4729 "parser.c" /* yacc.c:1646  */
    break;

  case 122:
#line 1262 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_signed);
    }
#line 4737 "parser.c" /* yacc.c:1646  */
    break;

  case 123:
#line 1266 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_unsigned);
    }
#line 4745 "parser.c" /* yacc.c:1646  */
    break;

  case 124:
#line 1270 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_ubool);
    }
#line 4753 "parser.c" /* yacc.c:1646  */
    break;

  case 125:
#line 1274 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_ucomplex);
    }
#line 4761 "parser.c" /* yacc.c:1646  */
    break;

  case 126:
#line 1278 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_uimaginary);
    }
#line 4769 "parser.c" /* yacc.c:1646  */
    break;

  case 127:
#line 1282 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4777 "parser.c" /* yacc.c:1646  */
    break;

  case 128:
#line 1286 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4785 "parser.c" /* yacc.c:1646  */
    break;

  case 129:
#line 1290 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Usertype((yyvsp[0].symb));
    }
#line 4793 "parser.c" /* yacc.c:1646  */
    break;

  case 130:
#line 1299 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = SUdecl((yyvsp[-4].type), NULL, (yyvsp[-1].decl), (yyvsp[-3].spec));
    }
#line 4801 "parser.c" /* yacc.c:1646  */
    break;

  case 131:
#line 1303 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = SUdecl((yyvsp[-3].type), NULL, NULL, (yyvsp[-2].spec));
    }
#line 4809 "parser.c" /* yacc.c:1646  */
    break;

  case 132:
#line 1307 "parser.y" /* yacc.c:1646  */
    {
      symbol s = Symbol((yyvsp[-3].name));
      /* Well, struct & union names have their own name space, and
       * their own scopes. I.e. they can be re-declare in nested
       * scopes. We don't do any kind of duplicate checks.
       */
      if (checkDecls)
        symtab_put(stab, s, SUNAME);
      (yyval.spec) = SUdecl((yyvsp[-5].type), s, (yyvsp[-1].decl), (yyvsp[-4].spec));
    }
#line 4824 "parser.c" /* yacc.c:1646  */
    break;

  case 133:
#line 1322 "parser.y" /* yacc.c:1646  */
    {
      symbol s = (yyvsp[-3].symb);
      if (checkDecls)
        symtab_put(stab, s, SUNAME);
      (yyval.spec) = SUdecl((yyvsp[-5].type), s, (yyvsp[-1].decl), (yyvsp[-4].spec));
    }
#line 4835 "parser.c" /* yacc.c:1646  */
    break;

  case 134:
#line 1329 "parser.y" /* yacc.c:1646  */
    {
      symbol s = Symbol((yyvsp[0].name));
      if (checkDecls)
        symtab_put(stab, s, SUNAME);
      (yyval.spec) = SUdecl((yyvsp[-2].type), s, NULL, (yyvsp[-1].spec));
    }
#line 4846 "parser.c" /* yacc.c:1646  */
    break;

  case 135:
#line 1336 "parser.y" /* yacc.c:1646  */
    {
      symbol s = (yyvsp[0].symb);
      if (checkDecls)
        symtab_put(stab, s, SUNAME);
      (yyval.spec) = SUdecl((yyvsp[-2].type), s, NULL, (yyvsp[-1].spec));
    }
#line 4857 "parser.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1347 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = SPEC_struct;
    }
#line 4865 "parser.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1351 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = SPEC_union;
    }
#line 4873 "parser.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1359 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 4881 "parser.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1363 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = StructfieldList((yyvsp[-1].decl), (yyvsp[0].decl));
    }
#line 4889 "parser.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1371 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = StructfieldDecl((yyvsp[-2].spec), (yyvsp[-1].decl));
    }
#line 4897 "parser.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1375 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = StructfieldDecl((yyvsp[-1].spec), NULL);
    }
#line 4905 "parser.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1383 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4913 "parser.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1387 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right((yyvsp[-1].spec), (yyvsp[0].spec));
    }
#line 4921 "parser.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1391 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 4929 "parser.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1395 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right((yyvsp[-1].spec), (yyvsp[0].spec));
    }
#line 4937 "parser.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1403 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 4945 "parser.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1407 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = DeclList((yyvsp[-2].decl), (yyvsp[0].decl));
    }
#line 4953 "parser.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1415 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 4961 "parser.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1419 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = BitDecl((yyvsp[-2].decl), (yyvsp[0].expr));
    }
#line 4969 "parser.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1423 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = BitDecl(NULL, (yyvsp[0].expr));
    }
#line 4977 "parser.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1431 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Enumdecl(NULL, (yyvsp[-1].spec), (yyvsp[-3].spec));
    }
#line 4985 "parser.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1435 "parser.y" /* yacc.c:1646  */
    {
      symbol s = Symbol((yyvsp[-3].name));

      if (checkDecls)
      {
        if (symtab_get(stab, s, ENUMNAME))
          parse_error(-1, "enum name '%s' is already in use.", (yyvsp[-3].name));
        symtab_put(stab, s, ENUMNAME);
      }
      (yyval.spec) = Enumdecl(s, (yyvsp[-1].spec), (yyvsp[-4].spec));
    }
#line 5001 "parser.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1448 "parser.y" /* yacc.c:1646  */
    {
      symbol s = (yyvsp[-3].symb);

      if (checkDecls)
      {
        if (symtab_get(stab, s, ENUMNAME))
          parse_error(-1, "enum name '%s' is already in use.", s->name);
        symtab_put(stab, s, ENUMNAME);
      }
      (yyval.spec) = Enumdecl(s, (yyvsp[-1].spec), (yyvsp[-4].spec));
    }
#line 5017 "parser.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1460 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Enumdecl(NULL, (yyvsp[-2].spec), (yyvsp[-4].spec));
    }
#line 5025 "parser.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1464 "parser.y" /* yacc.c:1646  */
    {
      symbol s = Symbol((yyvsp[-4].name));

      if (checkDecls)
      {
        if (symtab_get(stab, s, ENUMNAME))
          parse_error(-1, "enum name '%s' is already in use.", (yyvsp[-4].name));
        symtab_put(stab, s, ENUMNAME);
      }
      (yyval.spec) = Enumdecl(s, (yyvsp[-2].spec), (yyvsp[-5].spec));
    }
#line 5041 "parser.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1476 "parser.y" /* yacc.c:1646  */
    {
      symbol s = (yyvsp[-4].symb);

      if (checkDecls)
      {
        if (symtab_get(stab, s, ENUMNAME))
          parse_error(-1, "enum name '%s' is already in use.", s->name);
        symtab_put(stab, s, ENUMNAME);
      }
      (yyval.spec) = Enumdecl(s, (yyvsp[-2].spec), (yyvsp[-5].spec));
    }
#line 5057 "parser.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1488 "parser.y" /* yacc.c:1646  */
    {
      /*
      if (symtab_get(stab, s, ENUMNAME))
        parse_error(-1, "enum name '%s' is unknown.", $2);
      */
      (yyval.spec) = Enumdecl(Symbol((yyvsp[0].name)), NULL, (yyvsp[-1].spec));
    }
#line 5069 "parser.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1496 "parser.y" /* yacc.c:1646  */
    {
      /*
      if (symtab_get(stab, s, ENUMNAME))
        parse_error(-1, "enum name '%s' is unknown.", $2);
      */
      (yyval.spec) = Enumdecl((yyvsp[0].symb), NULL, (yyvsp[-1].spec));
    }
#line 5081 "parser.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1508 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 5089 "parser.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1512 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Enumbodylist((yyvsp[-2].spec), (yyvsp[0].spec));
    }
#line 5097 "parser.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1520 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Enumerator((yyvsp[0].symb), NULL);
    }
#line 5105 "parser.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1524 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Enumerator((yyvsp[-2].symb), (yyvsp[0].expr));
    }
#line 5113 "parser.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1532 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_const);
    }
#line 5121 "parser.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1536 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_restrict);
    }
#line 5129 "parser.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1540 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_volatile);
    }
#line 5137 "parser.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1544 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 5145 "parser.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1552 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Declspec(SPEC_inline);
    }
#line 5153 "parser.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1560 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = Declarator(NULL, (yyvsp[0].decl));
    }
#line 5161 "parser.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1564 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = Declarator((yyvsp[-1].spec), (yyvsp[0].decl));
    }
#line 5169 "parser.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1577 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = IdentifierDecl( Symbol((yyvsp[0].name)) );
    }
#line 5177 "parser.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1581 "parser.y" /* yacc.c:1646  */
    {
      /* Try to simplify a bit: (ident) -> ident */
      if ((yyvsp[-1].decl)->spec == NULL && (yyvsp[-1].decl)->decl->type == DIDENT)
        (yyval.decl) = (yyvsp[-1].decl)->decl;
      else
        (yyval.decl) = ParenDecl((yyvsp[-1].decl));
    }
#line 5189 "parser.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1589 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-2].decl), NULL, NULL);
    }
#line 5197 "parser.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1593 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-3].decl), (yyvsp[-1].spec), NULL);
    }
#line 5205 "parser.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1597 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-3].decl), NULL, (yyvsp[-1].expr));
    }
#line 5213 "parser.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1601 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-4].decl), (yyvsp[-2].spec), (yyvsp[-1].expr));
    }
#line 5221 "parser.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1605 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-4].decl), StClassSpec(SPEC_static), (yyvsp[-1].expr));
    }
#line 5229 "parser.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1609 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-5].decl), Speclist_right( StClassSpec(SPEC_static), (yyvsp[-2].spec) ), (yyvsp[-1].expr));
    }
#line 5237 "parser.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1613 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-5].decl), Speclist_left( (yyvsp[-3].spec), StClassSpec(SPEC_static) ), (yyvsp[-1].expr));
    }
#line 5245 "parser.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1617 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-3].decl), Declspec(SPEC_star), NULL);
    }
#line 5253 "parser.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1621 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-4].decl), Speclist_left( (yyvsp[-2].spec), Declspec(SPEC_star) ), NULL);
    }
#line 5261 "parser.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1625 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = FuncDecl((yyvsp[-3].decl), (yyvsp[-1].decl));
    }
#line 5269 "parser.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1629 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = FuncDecl((yyvsp[-2].decl), NULL);
    }
#line 5277 "parser.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1633 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = FuncDecl((yyvsp[-3].decl), (yyvsp[-1].decl));
    }
#line 5285 "parser.c" /* yacc.c:1646  */
    break;

  case 184:
#line 1641 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Pointer();
    }
#line 5293 "parser.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1645 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right(Pointer(), (yyvsp[0].spec));
    }
#line 5301 "parser.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1649 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right(Pointer(), (yyvsp[0].spec));
    }
#line 5309 "parser.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1653 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right( Pointer(), Speclist_left((yyvsp[-1].spec), (yyvsp[0].spec)) );
    }
#line 5317 "parser.c" /* yacc.c:1646  */
    break;

  case 188:
#line 1661 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 5325 "parser.c" /* yacc.c:1646  */
    break;

  case 189:
#line 1665 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_left((yyvsp[-1].spec), (yyvsp[0].spec));
    }
#line 5333 "parser.c" /* yacc.c:1646  */
    break;

  case 190:
#line 1673 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 5341 "parser.c" /* yacc.c:1646  */
    break;

  case 191:
#line 1677 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ParamList((yyvsp[-2].decl), Ellipsis());
    }
#line 5349 "parser.c" /* yacc.c:1646  */
    break;

  case 192:
#line 1685 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 5357 "parser.c" /* yacc.c:1646  */
    break;

  case 193:
#line 1689 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ParamList((yyvsp[-2].decl), (yyvsp[0].decl));
    }
#line 5365 "parser.c" /* yacc.c:1646  */
    break;

  case 194:
#line 1697 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ParamDecl((yyvsp[-1].spec), (yyvsp[0].decl));
    }
#line 5373 "parser.c" /* yacc.c:1646  */
    break;

  case 195:
#line 1701 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ParamDecl((yyvsp[0].spec), NULL);
    }
#line 5381 "parser.c" /* yacc.c:1646  */
    break;

  case 196:
#line 1705 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ParamDecl((yyvsp[-1].spec), (yyvsp[0].decl));
    }
#line 5389 "parser.c" /* yacc.c:1646  */
    break;

  case 197:
#line 1713 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = IdentifierDecl( Symbol((yyvsp[0].name)) );
    }
#line 5397 "parser.c" /* yacc.c:1646  */
    break;

  case 198:
#line 1717 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = IdList((yyvsp[-2].decl), IdentifierDecl( Symbol((yyvsp[0].name)) ));
    }
#line 5405 "parser.c" /* yacc.c:1646  */
    break;

  case 199:
#line 1725 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = Casttypename((yyvsp[0].spec), NULL);
    }
#line 5413 "parser.c" /* yacc.c:1646  */
    break;

  case 200:
#line 1729 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = Casttypename((yyvsp[-1].spec), (yyvsp[0].decl));
    }
#line 5421 "parser.c" /* yacc.c:1646  */
    break;

  case 201:
#line 1737 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = AbstractDeclarator((yyvsp[0].spec), NULL);
    }
#line 5429 "parser.c" /* yacc.c:1646  */
    break;

  case 202:
#line 1741 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = AbstractDeclarator(NULL, (yyvsp[0].decl));
    }
#line 5437 "parser.c" /* yacc.c:1646  */
    break;

  case 203:
#line 1745 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = AbstractDeclarator((yyvsp[-1].spec), (yyvsp[0].decl));
    }
#line 5445 "parser.c" /* yacc.c:1646  */
    break;

  case 204:
#line 1753 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ParenDecl((yyvsp[-1].decl));
    }
#line 5453 "parser.c" /* yacc.c:1646  */
    break;

  case 205:
#line 1757 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl(NULL, NULL, NULL);
    }
#line 5461 "parser.c" /* yacc.c:1646  */
    break;

  case 206:
#line 1761 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-2].decl), NULL, NULL);
    }
#line 5469 "parser.c" /* yacc.c:1646  */
    break;

  case 207:
#line 1765 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl(NULL, NULL, (yyvsp[-1].expr));
    }
#line 5477 "parser.c" /* yacc.c:1646  */
    break;

  case 208:
#line 1769 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-3].decl), NULL, (yyvsp[-1].expr));
    }
#line 5485 "parser.c" /* yacc.c:1646  */
    break;

  case 209:
#line 1773 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl(NULL, Declspec(SPEC_star), NULL);
    }
#line 5493 "parser.c" /* yacc.c:1646  */
    break;

  case 210:
#line 1777 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl((yyvsp[-3].decl), Declspec(SPEC_star), NULL);
    }
#line 5501 "parser.c" /* yacc.c:1646  */
    break;

  case 211:
#line 1781 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = FuncDecl(NULL, NULL);
    }
#line 5509 "parser.c" /* yacc.c:1646  */
    break;

  case 212:
#line 1785 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = FuncDecl((yyvsp[-2].decl), NULL);
    }
#line 5517 "parser.c" /* yacc.c:1646  */
    break;

  case 213:
#line 1789 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = FuncDecl(NULL, (yyvsp[-1].decl));
    }
#line 5525 "parser.c" /* yacc.c:1646  */
    break;

  case 214:
#line 1793 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = FuncDecl((yyvsp[-3].decl), (yyvsp[-1].decl));
    }
#line 5533 "parser.c" /* yacc.c:1646  */
    break;

  case 215:
#line 1801 "parser.y" /* yacc.c:1646  */
    {
      (yyval.symb) = Symbol((yyvsp[0].name));
    }
#line 5541 "parser.c" /* yacc.c:1646  */
    break;

  case 216:
#line 1809 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 5549 "parser.c" /* yacc.c:1646  */
    break;

  case 217:
#line 1813 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BracedInitializer((yyvsp[-1].expr));
    }
#line 5557 "parser.c" /* yacc.c:1646  */
    break;

  case 218:
#line 1817 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = BracedInitializer((yyvsp[-2].expr));
    }
#line 5565 "parser.c" /* yacc.c:1646  */
    break;

  case 219:
#line 1825 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 5573 "parser.c" /* yacc.c:1646  */
    break;

  case 220:
#line 1829 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Designated((yyvsp[-1].expr), (yyvsp[0].expr));
    }
#line 5581 "parser.c" /* yacc.c:1646  */
    break;

  case 221:
#line 1833 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = CommaList((yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 5589 "parser.c" /* yacc.c:1646  */
    break;

  case 222:
#line 1837 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = CommaList((yyvsp[-3].expr), Designated((yyvsp[-1].expr), (yyvsp[0].expr)));
    }
#line 5597 "parser.c" /* yacc.c:1646  */
    break;

  case 223:
#line 1845 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[-1].expr);
    }
#line 5605 "parser.c" /* yacc.c:1646  */
    break;

  case 224:
#line 1853 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 5613 "parser.c" /* yacc.c:1646  */
    break;

  case 225:
#line 1857 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = SpaceList((yyvsp[-1].expr), (yyvsp[0].expr));
    }
#line 5621 "parser.c" /* yacc.c:1646  */
    break;

  case 226:
#line 1865 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = IdxDesignator((yyvsp[-1].expr));
    }
#line 5629 "parser.c" /* yacc.c:1646  */
    break;

  case 227:
#line 1869 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = DotDesignator( Symbol((yyvsp[0].name)) );
    }
#line 5637 "parser.c" /* yacc.c:1646  */
    break;

  case 228:
#line 1873 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = DotDesignator((yyvsp[0].symb));
    }
#line 5645 "parser.c" /* yacc.c:1646  */
    break;

  case 229:
#line 1887 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5653 "parser.c" /* yacc.c:1646  */
    break;

  case 230:
#line 1891 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5661 "parser.c" /* yacc.c:1646  */
    break;

  case 231:
#line 1895 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5669 "parser.c" /* yacc.c:1646  */
    break;

  case 232:
#line 1899 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5677 "parser.c" /* yacc.c:1646  */
    break;

  case 233:
#line 1903 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5685 "parser.c" /* yacc.c:1646  */
    break;

  case 234:
#line 1907 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5693 "parser.c" /* yacc.c:1646  */
    break;

  case 235:
#line 1911 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5701 "parser.c" /* yacc.c:1646  */
    break;

  case 236:
#line 1915 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpStmt((yyvsp[0].ocon));
      (yyval.stmt)->l = (yyvsp[0].ocon)->l;
    }
#line 5710 "parser.c" /* yacc.c:1646  */
    break;

  case 237:
#line 1920 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpixStmt((yyvsp[0].xcon));
      (yyval.stmt)->l = (yyvsp[0].xcon)->l;
    }
#line 5719 "parser.c" /* yacc.c:1646  */
    break;

  case 238:
#line 1925 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Verbatim(strdup((yyvsp[0].name)));
    }
#line 5727 "parser.c" /* yacc.c:1646  */
    break;

  case 239:
#line 1933 "parser.y" /* yacc.c:1646  */
    { 
      (yyval.stmt) = (yyvsp[0].stmt); 
    }
#line 5735 "parser.c" /* yacc.c:1646  */
    break;

  case 240:
#line 1937 "parser.y" /* yacc.c:1646  */
    {       
      (yyval.stmt) = OmpStmt((yyvsp[0].ocon));
      (yyval.stmt)->l = (yyvsp[0].ocon)->l;
    }
#line 5744 "parser.c" /* yacc.c:1646  */
    break;

  case 241:
#line 1947 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Labeled( Symbol((yyvsp[-3].name)), (yyvsp[0].stmt) );
    }
#line 5752 "parser.c" /* yacc.c:1646  */
    break;

  case 242:
#line 1951 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Case((yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5760 "parser.c" /* yacc.c:1646  */
    break;

  case 243:
#line 1955 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Default((yyvsp[0].stmt));
    }
#line 5768 "parser.c" /* yacc.c:1646  */
    break;

  case 244:
#line 1963 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Compound(NULL);
    }
#line 5776 "parser.c" /* yacc.c:1646  */
    break;

  case 245:
#line 1966 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = sc_original_line()-1; scope_start(stab); }
#line 5782 "parser.c" /* yacc.c:1646  */
    break;

  case 246:
#line 1968 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Compound((yyvsp[-1].stmt));
      scope_end(stab);
      (yyval.stmt)->l = (yyvsp[-2].type);     /* Remember 1st line */
    }
#line 5792 "parser.c" /* yacc.c:1646  */
    break;

  case 247:
#line 1978 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5800 "parser.c" /* yacc.c:1646  */
    break;

  case 248:
#line 1982 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = BlockList((yyvsp[-1].stmt), (yyvsp[0].stmt));
      (yyval.stmt)->l = (yyvsp[-1].stmt)->l;
    }
#line 5809 "parser.c" /* yacc.c:1646  */
    break;

  case 249:
#line 1991 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5817 "parser.c" /* yacc.c:1646  */
    break;

  case 250:
#line 1995 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 5825 "parser.c" /* yacc.c:1646  */
    break;

  case 251:
#line 1999 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpStmt((yyvsp[0].ocon));
      (yyval.stmt)->l = (yyvsp[0].ocon)->l;
    }
#line 5834 "parser.c" /* yacc.c:1646  */
    break;

  case 252:
#line 2004 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpixStmt((yyvsp[0].xcon));
      (yyval.stmt)->l = (yyvsp[0].xcon)->l;
    }
#line 5843 "parser.c" /* yacc.c:1646  */
    break;

  case 253:
#line 2013 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Expression(NULL);
    }
#line 5851 "parser.c" /* yacc.c:1646  */
    break;

  case 254:
#line 2017 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Expression((yyvsp[-1].expr));
      (yyval.stmt)->l = (yyvsp[-1].expr)->l;
    }
#line 5860 "parser.c" /* yacc.c:1646  */
    break;

  case 255:
#line 2026 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = If((yyvsp[-2].expr), (yyvsp[0].stmt), NULL);
    }
#line 5868 "parser.c" /* yacc.c:1646  */
    break;

  case 256:
#line 2030 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = If((yyvsp[-4].expr), (yyvsp[-2].stmt), (yyvsp[0].stmt));
    }
#line 5876 "parser.c" /* yacc.c:1646  */
    break;

  case 257:
#line 2034 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Switch((yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5884 "parser.c" /* yacc.c:1646  */
    break;

  case 258:
#line 2043 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = While((yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5892 "parser.c" /* yacc.c:1646  */
    break;

  case 259:
#line 2047 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Do((yyvsp[-5].stmt), (yyvsp[-2].expr));
    }
#line 5900 "parser.c" /* yacc.c:1646  */
    break;

  case 261:
#line 2055 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(NULL, NULL, NULL, (yyvsp[0].stmt));
    }
#line 5908 "parser.c" /* yacc.c:1646  */
    break;

  case 262:
#line 2059 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(Expression((yyvsp[-4].expr)), NULL, NULL, (yyvsp[0].stmt));
    }
#line 5916 "parser.c" /* yacc.c:1646  */
    break;

  case 263:
#line 2063 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(NULL, (yyvsp[-3].expr), NULL, (yyvsp[0].stmt));
    }
#line 5924 "parser.c" /* yacc.c:1646  */
    break;

  case 264:
#line 2067 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(NULL, NULL, (yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5932 "parser.c" /* yacc.c:1646  */
    break;

  case 265:
#line 2071 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(Expression((yyvsp[-5].expr)), (yyvsp[-3].expr), NULL, (yyvsp[0].stmt));
    }
#line 5940 "parser.c" /* yacc.c:1646  */
    break;

  case 266:
#line 2075 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(Expression((yyvsp[-5].expr)), NULL, (yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5948 "parser.c" /* yacc.c:1646  */
    break;

  case 267:
#line 2079 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(NULL, (yyvsp[-4].expr), (yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5956 "parser.c" /* yacc.c:1646  */
    break;

  case 268:
#line 2083 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For(Expression((yyvsp[-6].expr)), (yyvsp[-4].expr), (yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5964 "parser.c" /* yacc.c:1646  */
    break;

  case 269:
#line 2087 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For((yyvsp[-3].stmt), NULL, NULL, (yyvsp[0].stmt));
    }
#line 5972 "parser.c" /* yacc.c:1646  */
    break;

  case 270:
#line 2091 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For((yyvsp[-4].stmt), (yyvsp[-3].expr), NULL, (yyvsp[0].stmt));
    }
#line 5980 "parser.c" /* yacc.c:1646  */
    break;

  case 271:
#line 2095 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For((yyvsp[-4].stmt), NULL, (yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5988 "parser.c" /* yacc.c:1646  */
    break;

  case 272:
#line 2099 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = For((yyvsp[-5].stmt), (yyvsp[-4].expr), (yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 5996 "parser.c" /* yacc.c:1646  */
    break;

  case 273:
#line 2107 "parser.y" /* yacc.c:1646  */
    {
      /* We don't keep track of labels -- we leave it to the native compiler */
      (yyval.stmt) = Goto( Symbol((yyvsp[-1].name)) );
    }
#line 6005 "parser.c" /* yacc.c:1646  */
    break;

  case 274:
#line 2112 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Continue();
    }
#line 6013 "parser.c" /* yacc.c:1646  */
    break;

  case 275:
#line 2116 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Break();
    }
#line 6021 "parser.c" /* yacc.c:1646  */
    break;

  case 276:
#line 2120 "parser.y" /* yacc.c:1646  */
    {
      //TODO: simple hack, not 100% correct, does not cover goto
      if (errorOnReturn)
        parse_error(1, "return statement not allowed in an outlined region\n");
      (yyval.stmt) = Return(NULL);
    }
#line 6032 "parser.c" /* yacc.c:1646  */
    break;

  case 277:
#line 2127 "parser.y" /* yacc.c:1646  */
    {
      //TODO: simple hack, not 100% correct, does not cover goto
      if (errorOnReturn)
        parse_error(1, "return statement not allowed in an outlined region\n");
      (yyval.stmt) = Return((yyvsp[-1].expr));
    }
#line 6043 "parser.c" /* yacc.c:1646  */
    break;

  case 278:
#line 2144 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = pastree = (yyvsp[0].stmt);
    }
#line 6051 "parser.c" /* yacc.c:1646  */
    break;

  case 279:
#line 2148 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = pastree = BlockList((yyvsp[-1].stmt), (yyvsp[0].stmt));
    }
#line 6059 "parser.c" /* yacc.c:1646  */
    break;

  case 280:
#line 2156 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 6067 "parser.c" /* yacc.c:1646  */
    break;

  case 281:
#line 2160 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 6075 "parser.c" /* yacc.c:1646  */
    break;

  case 282:
#line 2167 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpixStmt((yyvsp[0].xcon));
    }
#line 6083 "parser.c" /* yacc.c:1646  */
    break;

  case 283:
#line 2171 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Verbatim(strdup((yyvsp[0].name)));
    }
#line 6091 "parser.c" /* yacc.c:1646  */
    break;

  case 284:
#line 2182 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt) = (yyvsp[0].stmt); }
#line 6097 "parser.c" /* yacc.c:1646  */
    break;

  case 285:
#line 2183 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt) = (yyvsp[0].stmt); }
#line 6103 "parser.c" /* yacc.c:1646  */
    break;

  case 286:
#line 2188 "parser.y" /* yacc.c:1646  */
    {
      stentry f;
      
      if (isTypedef || (yyvsp[0].decl)->decl->type != DFUNC)
        parse_error(1, "function definition cannot be parsed.\n");
      f = symtab_get(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME);
      if (f && f->funcdef)
        parse_error(1, "function %s is multiply defined.\n", f->key->name);
      if (f == NULL)
      {
        f = symtab_put(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME);
        f->spec = (yyvsp[-1].spec);
        f->decl = (yyvsp[0].decl);
      }

      scope_start(stab);
      ast_declare_function_params((yyvsp[0].decl));
    }
#line 6126 "parser.c" /* yacc.c:1646  */
    break;

  case 287:
#line 2207 "parser.y" /* yacc.c:1646  */
    {
      scope_end(stab);
      check_for_main_and_declare((yyvsp[-3].spec), (yyvsp[-2].decl));
      (yyval.stmt) = FuncDef((yyvsp[-3].spec), (yyvsp[-2].decl), NULL, (yyvsp[0].stmt));
      symtab_get(stab, decl_getidentifier_symbol((yyvsp[-2].decl)), FUNCNAME)->funcdef = (yyval.stmt);
    }
#line 6137 "parser.c" /* yacc.c:1646  */
    break;

  case 288:
#line 2214 "parser.y" /* yacc.c:1646  */
    {
      stentry f;
      
      if (isTypedef || (yyvsp[0].decl)->decl->type != DFUNC)
        parse_error(1, "function definition cannot be parsed.\n");
      f = symtab_get(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME);
      if (f && f->funcdef)
        parse_error(1, "function %s is multiply defined.\n", f->key->name);
      if (f == NULL)
      {
        f = symtab_put(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME);
        f->spec = NULL;
        f->decl = (yyvsp[0].decl);
      }

      scope_start(stab);
      ast_declare_function_params((yyvsp[0].decl));
    }
#line 6160 "parser.c" /* yacc.c:1646  */
    break;

  case 289:
#line 2233 "parser.y" /* yacc.c:1646  */
    {
      astspec s = Declspec(SPEC_int);  /* return type defaults to "int" */
      stentry f;
      
      scope_end(stab);
      check_for_main_and_declare(s, (yyvsp[-2].decl));
      (yyval.stmt) = FuncDef(s, (yyvsp[-2].decl), NULL, (yyvsp[0].stmt));
      
      f = symtab_get(stab, decl_getidentifier_symbol((yyvsp[-2].decl)), FUNCNAME);
      if (!f->spec) f->spec = s;
      f->funcdef = (yyval.stmt);
    }
#line 6177 "parser.c" /* yacc.c:1646  */
    break;

  case 290:
#line 2249 "parser.y" /* yacc.c:1646  */
    {
      stentry f;
      
      if (isTypedef || (yyvsp[0].decl)->decl->type != DFUNC)
        parse_error(1, "function definition cannot be parsed.\n");
      f = symtab_get(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME);
      if (f && f->funcdef)
        parse_error(1, "function %s is multiply defined.\n", f->key->name);
      if (f == NULL)
      {
        f = symtab_put(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME);
        f->spec = (yyvsp[-1].spec);
        f->decl = (yyvsp[0].decl);
      }

      scope_start(stab);
      /* Notice here that the function parameters are declared through
       * the declaration_list and we need to do nothing else!
       */
    }
#line 6202 "parser.c" /* yacc.c:1646  */
    break;

  case 291:
#line 2270 "parser.y" /* yacc.c:1646  */
    {
      scope_end(stab);
      check_for_main_and_declare((yyvsp[-4].spec), (yyvsp[-3].decl));
      (yyval.stmt) = FuncDef((yyvsp[-4].spec), (yyvsp[-3].decl), (yyvsp[-1].stmt), (yyvsp[0].stmt));
      symtab_get(stab, decl_getidentifier_symbol((yyvsp[-3].decl)), FUNCNAME)->funcdef = (yyval.stmt);
    }
#line 6213 "parser.c" /* yacc.c:1646  */
    break;

  case 292:
#line 2277 "parser.y" /* yacc.c:1646  */
    {
      stentry f;
      
      if (isTypedef || (yyvsp[0].decl)->decl->type != DFUNC)
        parse_error(1, "function definition cannot be parsed.\n");
      f = symtab_get(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME);
      if (f && f->funcdef)
        parse_error(1, "function %s is multiply defined.\n", f->key->name);
      if (f == NULL)
      {
        f = symtab_put(stab, decl_getidentifier_symbol((yyvsp[0].decl)), FUNCNAME);
        f->spec = NULL;
        f->decl = (yyvsp[0].decl);
      }

      scope_start(stab);
      /* Notice here that the function parameters are declared through
       * the declaration_list and we need to do nothing else!
       */
    }
#line 6238 "parser.c" /* yacc.c:1646  */
    break;

  case 293:
#line 2298 "parser.y" /* yacc.c:1646  */
    {
      astspec s = Declspec(SPEC_int);  /* return type defaults to "int" */
      stentry f;

      scope_end(stab);
      check_for_main_and_declare(s, (yyvsp[-3].decl));
      (yyval.stmt) = FuncDef(s, (yyvsp[-3].decl), (yyvsp[-1].stmt), (yyvsp[0].stmt));
      
      f = symtab_get(stab, decl_getidentifier_symbol((yyvsp[-3].decl)), FUNCNAME);
      if (!f->spec) f->spec = s;
      f->funcdef = (yyval.stmt);
    }
#line 6255 "parser.c" /* yacc.c:1646  */
    break;

  case 294:
#line 2315 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 6263 "parser.c" /* yacc.c:1646  */
    break;

  case 295:
#line 2319 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = BlockList((yyvsp[-1].stmt), (yyvsp[0].stmt));         /* Same as block list */
    }
#line 6271 "parser.c" /* yacc.c:1646  */
    break;

  case 296:
#line 2339 "parser.y" /* yacc.c:1646  */
    {
      ((yyval.stmt) = (yyvsp[0].stmt))->u.assem->qualifiers = NULL;
    }
#line 6279 "parser.c" /* yacc.c:1646  */
    break;

  case 297:
#line 2343 "parser.y" /* yacc.c:1646  */
    {
      ((yyval.stmt) = (yyvsp[0].stmt))->u.assem->qualifiers = (yyvsp[-1].spec);
    }
#line 6287 "parser.c" /* yacc.c:1646  */
    break;

  case 298:
#line 2350 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = BasicAsm(NULL, (yyvsp[-1].string));
    }
#line 6295 "parser.c" /* yacc.c:1646  */
    break;

  case 299:
#line 2354 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = XtendAsm(NULL, (yyvsp[-3].string), (yyvsp[-1].asmo), NULL, NULL);
    }
#line 6303 "parser.c" /* yacc.c:1646  */
    break;

  case 300:
#line 2358 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = XtendAsm(NULL, (yyvsp[-5].string), (yyvsp[-3].asmo), (yyvsp[-1].asmo), NULL);
    }
#line 6311 "parser.c" /* yacc.c:1646  */
    break;

  case 301:
#line 2362 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = XtendAsm(NULL, (yyvsp[-7].string), (yyvsp[-5].asmo), (yyvsp[-3].asmo), (yyvsp[-1].expr));
    }
#line 6319 "parser.c" /* yacc.c:1646  */
    break;

  case 302:
#line 2366 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = XtendAsmGoto((yyvsp[-8].string), (yyvsp[-5].asmo), (yyvsp[-3].expr), (yyvsp[-1].expr));
    }
#line 6327 "parser.c" /* yacc.c:1646  */
    break;

  case 303:
#line 2373 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = (yyvsp[0].spec);
    }
#line 6335 "parser.c" /* yacc.c:1646  */
    break;

  case 304:
#line 2377 "parser.y" /* yacc.c:1646  */
    {
      (yyval.spec) = Speclist_right((yyvsp[-1].spec), (yyvsp[0].spec));
    }
#line 6343 "parser.c" /* yacc.c:1646  */
    break;

  case 305:
#line 2383 "parser.y" /* yacc.c:1646  */
    { (yyval.spec) = Declspec(SPEC_volatile); }
#line 6349 "parser.c" /* yacc.c:1646  */
    break;

  case 306:
#line 2384 "parser.y" /* yacc.c:1646  */
    { (yyval.spec) = Declspec(SPEC_inline); }
#line 6355 "parser.c" /* yacc.c:1646  */
    break;

  case 307:
#line 2388 "parser.y" /* yacc.c:1646  */
    {
      (yyval.asmo) = NULL;
    }
#line 6363 "parser.c" /* yacc.c:1646  */
    break;

  case 308:
#line 2392 "parser.y" /* yacc.c:1646  */
    {
      (yyval.asmo) = (yyvsp[0].asmo);
    }
#line 6371 "parser.c" /* yacc.c:1646  */
    break;

  case 309:
#line 2396 "parser.y" /* yacc.c:1646  */
    {
      (yyval.asmo) = XAsmOpList((yyvsp[-2].asmo), (yyvsp[0].asmo));
    }
#line 6379 "parser.c" /* yacc.c:1646  */
    break;

  case 310:
#line 2403 "parser.y" /* yacc.c:1646  */
    {
      (yyval.asmo) = XAsmOperand(IdentName((yyvsp[-5].name)), (yyvsp[-3].string), (yyvsp[-1].expr));
    }
#line 6387 "parser.c" /* yacc.c:1646  */
    break;

  case 311:
#line 2407 "parser.y" /* yacc.c:1646  */
    {
      (yyval.asmo) = XAsmOperand(NULL, (yyvsp[-3].string), (yyvsp[-1].expr));
    }
#line 6395 "parser.c" /* yacc.c:1646  */
    break;

  case 312:
#line 2413 "parser.y" /* yacc.c:1646  */
    {
      (yyval.asmo) = NULL;
    }
#line 6403 "parser.c" /* yacc.c:1646  */
    break;

  case 313:
#line 2417 "parser.y" /* yacc.c:1646  */
    {
      (yyval.asmo) = (yyvsp[0].asmo);
    }
#line 6411 "parser.c" /* yacc.c:1646  */
    break;

  case 314:
#line 2421 "parser.y" /* yacc.c:1646  */
    {
      (yyval.asmo) = XAsmOpList((yyvsp[-2].asmo), (yyvsp[0].asmo));
    }
#line 6419 "parser.c" /* yacc.c:1646  */
    break;

  case 315:
#line 2428 "parser.y" /* yacc.c:1646  */
    {
      (yyval.asmo) = XAsmOperand(IdentName((yyvsp[-5].name)), (yyvsp[-3].string), (yyvsp[-1].expr));
    }
#line 6427 "parser.c" /* yacc.c:1646  */
    break;

  case 316:
#line 2432 "parser.y" /* yacc.c:1646  */
    {
      (yyval.asmo) = XAsmOperand(NULL, (yyvsp[-3].string), (yyvsp[-1].expr));
    }
#line 6435 "parser.c" /* yacc.c:1646  */
    break;

  case 317:
#line 2439 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = String((yyvsp[0].string));
    }
#line 6443 "parser.c" /* yacc.c:1646  */
    break;

  case 318:
#line 2443 "parser.y" /* yacc.c:1646  */
    {
       (yyval.expr) = CommaList((yyvsp[-2].expr), String((yyvsp[0].string)));
    }
#line 6451 "parser.c" /* yacc.c:1646  */
    break;

  case 319:
#line 2450 "parser.y" /* yacc.c:1646  */
    { 
      (yyval.expr) = IdentName((yyvsp[0].name));
    }
#line 6459 "parser.c" /* yacc.c:1646  */
    break;

  case 320:
#line 2454 "parser.y" /* yacc.c:1646  */
    {
       (yyval.expr) = CommaList((yyvsp[-2].expr), IdentName((yyvsp[0].name)));
    }
#line 6467 "parser.c" /* yacc.c:1646  */
    break;

  case 321:
#line 2466 "parser.y" /* yacc.c:1646  */
    {
			(yyval.spec) = NULL;
		}
#line 6475 "parser.c" /* yacc.c:1646  */
    break;

  case 322:
#line 2470 "parser.y" /* yacc.c:1646  */
    {
			(yyval.spec) = (yyvsp[0].spec);
		}
#line 6483 "parser.c" /* yacc.c:1646  */
    break;

  case 323:
#line 2477 "parser.y" /* yacc.c:1646  */
    {
			(yyval.spec) = (yyvsp[0].spec);
		}
#line 6491 "parser.c" /* yacc.c:1646  */
    break;

  case 324:
#line 2481 "parser.y" /* yacc.c:1646  */
    {
			(yyval.spec) = Speclist_left((yyvsp[-1].spec), (yyvsp[0].spec));
		}
#line 6499 "parser.c" /* yacc.c:1646  */
    break;

  case 325:
#line 2488 "parser.y" /* yacc.c:1646  */
    {
			(yyval.spec) = AttrSpec((yyvsp[-2].string));
		}
#line 6507 "parser.c" /* yacc.c:1646  */
    break;

  case 326:
#line 2495 "parser.y" /* yacc.c:1646  */
    {
			(yyval.string) = (yyvsp[0].string);
		}
#line 6515 "parser.c" /* yacc.c:1646  */
    break;

  case 327:
#line 2499 "parser.y" /* yacc.c:1646  */
    {
			if ((yyvsp[-2].string) == NULL && (yyvsp[0].string) == NULL)
				(yyval.string) = strdup(",");
			else
			  if ((yyvsp[-2].string) == NULL)
					(yyval.string) = strdupcat(strdup(", "), (yyvsp[0].string), 1);
				else
					if ((yyvsp[0].string) == NULL)
						(yyval.string) = strdupcat((yyvsp[-2].string), strdup(", "), 1);
					else
						(yyval.string) = strdupcat((yyvsp[-2].string), strdupcat(strdup(", "), (yyvsp[0].string), 1), 1);
		}
#line 6532 "parser.c" /* yacc.c:1646  */
    break;

  case 328:
#line 2515 "parser.y" /* yacc.c:1646  */
    {
			(yyval.string) = NULL;
		}
#line 6540 "parser.c" /* yacc.c:1646  */
    break;

  case 329:
#line 2519 "parser.y" /* yacc.c:1646  */
    {
			(yyval.string) = (yyvsp[0].string);
		}
#line 6548 "parser.c" /* yacc.c:1646  */
    break;

  case 330:
#line 2523 "parser.y" /* yacc.c:1646  */
    {
			static str xp = NULL;
			
			if (xp == NULL) xp = Strnew();
			str_printf(xp, "%s(", (yyvsp[-3].string));
			if ((yyvsp[-1].expr))
				ast_expr_print(xp, (yyvsp[-1].expr));
			str_printf(xp, ")");
		  (yyval.string) = strdup(str_string(xp));
		  str_truncate(xp);
		  free((yyvsp[-3].string));
		}
#line 6565 "parser.c" /* yacc.c:1646  */
    break;

  case 331:
#line 2538 "parser.y" /* yacc.c:1646  */
    { (yyval.string) = strdup((yyvsp[0].name)); }
#line 6571 "parser.c" /* yacc.c:1646  */
    break;

  case 332:
#line 2539 "parser.y" /* yacc.c:1646  */
    { (yyval.string) = strdup((yyvsp[0].name)); }
#line 6577 "parser.c" /* yacc.c:1646  */
    break;

  case 333:
#line 2540 "parser.y" /* yacc.c:1646  */
    { (yyval.string) = strdup((yyvsp[0].name)); }
#line 6583 "parser.c" /* yacc.c:1646  */
    break;

  case 334:
#line 2560 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 6591 "parser.c" /* yacc.c:1646  */
    break;

  case 335:
#line 2564 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 6599 "parser.c" /* yacc.c:1646  */
    break;

  case 336:
#line 2572 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 6607 "parser.c" /* yacc.c:1646  */
    break;

  case 337:
#line 2581 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 6615 "parser.c" /* yacc.c:1646  */
    break;

  case 338:
#line 2585 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = pastree = BlockList((yyvsp[-1].stmt), (yyvsp[0].stmt));
    }
#line 6623 "parser.c" /* yacc.c:1646  */
    break;

  case 339:
#line 2592 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6631 "parser.c" /* yacc.c:1646  */
    break;

  case 340:
#line 2596 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6639 "parser.c" /* yacc.c:1646  */
    break;

  case 341:
#line 2600 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6647 "parser.c" /* yacc.c:1646  */
    break;

  case 342:
#line 2604 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6655 "parser.c" /* yacc.c:1646  */
    break;

  case 343:
#line 2608 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6663 "parser.c" /* yacc.c:1646  */
    break;

  case 344:
#line 2612 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6671 "parser.c" /* yacc.c:1646  */
    break;

  case 345:
#line 2616 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6679 "parser.c" /* yacc.c:1646  */
    break;

  case 346:
#line 2620 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6687 "parser.c" /* yacc.c:1646  */
    break;

  case 347:
#line 2624 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6695 "parser.c" /* yacc.c:1646  */
    break;

  case 348:
#line 2628 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6703 "parser.c" /* yacc.c:1646  */
    break;

  case 349:
#line 2633 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6711 "parser.c" /* yacc.c:1646  */
    break;

  case 350:
#line 2638 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6719 "parser.c" /* yacc.c:1646  */
    break;

  case 351:
#line 2642 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6727 "parser.c" /* yacc.c:1646  */
    break;

  case 352:
#line 2646 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6735 "parser.c" /* yacc.c:1646  */
    break;

  case 353:
#line 2650 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6743 "parser.c" /* yacc.c:1646  */
    break;

  case 354:
#line 2654 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6751 "parser.c" /* yacc.c:1646  */
    break;

  case 355:
#line 2658 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6759 "parser.c" /* yacc.c:1646  */
    break;

  case 356:
#line 2662 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6767 "parser.c" /* yacc.c:1646  */
    break;

  case 357:
#line 2666 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6775 "parser.c" /* yacc.c:1646  */
    break;

  case 358:
#line 2670 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6783 "parser.c" /* yacc.c:1646  */
    break;

  case 359:
#line 2674 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6791 "parser.c" /* yacc.c:1646  */
    break;

  case 360:
#line 2678 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6799 "parser.c" /* yacc.c:1646  */
    break;

  case 361:
#line 2682 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6807 "parser.c" /* yacc.c:1646  */
    break;

  case 362:
#line 2686 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6815 "parser.c" /* yacc.c:1646  */
    break;

  case 363:
#line 2690 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6823 "parser.c" /* yacc.c:1646  */
    break;

  case 364:
#line 2694 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6831 "parser.c" /* yacc.c:1646  */
    break;

  case 365:
#line 2698 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6839 "parser.c" /* yacc.c:1646  */
    break;

  case 366:
#line 2702 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6847 "parser.c" /* yacc.c:1646  */
    break;

  case 367:
#line 2706 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6855 "parser.c" /* yacc.c:1646  */
    break;

  case 368:
#line 2710 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6863 "parser.c" /* yacc.c:1646  */
    break;

  case 369:
#line 2715 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = (yyvsp[0].ocon);
    }
#line 6871 "parser.c" /* yacc.c:1646  */
    break;

  case 370:
#line 2731 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCBARRIER, (yyvsp[0].odir), NULL);
    }
#line 6879 "parser.c" /* yacc.c:1646  */
    break;

  case 371:
#line 2735 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCFLUSH, (yyvsp[0].odir), NULL);
    }
#line 6887 "parser.c" /* yacc.c:1646  */
    break;

  case 372:
#line 2740 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTASKWAIT, (yyvsp[0].odir), NULL);
    }
#line 6895 "parser.c" /* yacc.c:1646  */
    break;

  case 373:
#line 2745 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTASKYIELD, (yyvsp[0].odir), NULL);
    }
#line 6903 "parser.c" /* yacc.c:1646  */
    break;

  case 374:
#line 2750 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCCANCEL, (yyvsp[0].odir), NULL);
    }
#line 6911 "parser.c" /* yacc.c:1646  */
    break;

  case 375:
#line 2755 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCCANCELLATIONPOINT, (yyvsp[0].odir), NULL);
    }
#line 6919 "parser.c" /* yacc.c:1646  */
    break;

  case 376:
#line 2760 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTARGETUPD, (yyvsp[0].odir), NULL);
    }
#line 6927 "parser.c" /* yacc.c:1646  */
    break;

  case 377:
#line 2765 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTARGENTERDATA, (yyvsp[0].odir), NULL);
    }
#line 6935 "parser.c" /* yacc.c:1646  */
    break;

  case 378:
#line 2770 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTARGEXITDATA, (yyvsp[0].odir), NULL);
    }
#line 6943 "parser.c" /* yacc.c:1646  */
    break;

  case 379:
#line 2777 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = (yyvsp[0].stmt);
    }
#line 6951 "parser.c" /* yacc.c:1646  */
    break;

  case 380:
#line 2784 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCPARALLEL, (yyvsp[-1].odir), (yyvsp[0].stmt));
      (yyval.ocon)->l = (yyvsp[-1].odir)->l;
    }
#line 6960 "parser.c" /* yacc.c:1646  */
    break;

  case 381:
#line 2792 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCPARALLEL, (yyvsp[-1].ocla));
    }
#line 6968 "parser.c" /* yacc.c:1646  */
    break;

  case 382:
#line 2799 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 6976 "parser.c" /* yacc.c:1646  */
    break;

  case 383:
#line 2803 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 6984 "parser.c" /* yacc.c:1646  */
    break;

  case 384:
#line 2807 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 6992 "parser.c" /* yacc.c:1646  */
    break;

  case 385:
#line 2814 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7000 "parser.c" /* yacc.c:1646  */
    break;

  case 386:
#line 2818 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7008 "parser.c" /* yacc.c:1646  */
    break;

  case 387:
#line 2822 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7016 "parser.c" /* yacc.c:1646  */
    break;

  case 388:
#line 2826 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7024 "parser.c" /* yacc.c:1646  */
    break;

  case 389:
#line 2830 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7032 "parser.c" /* yacc.c:1646  */
    break;

  case 390:
#line 2834 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7040 "parser.c" /* yacc.c:1646  */
    break;

  case 391:
#line 2841 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7048 "parser.c" /* yacc.c:1646  */
    break;

  case 392:
#line 2844 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7054 "parser.c" /* yacc.c:1646  */
    break;

  case 393:
#line 2845 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = NumthreadsClause((yyvsp[-1].expr));
    }
#line 7063 "parser.c" /* yacc.c:1646  */
    break;

  case 394:
#line 2849 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7069 "parser.c" /* yacc.c:1646  */
    break;

  case 395:
#line 2850 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCCOPYIN, (yyvsp[-1].decl));
    }
#line 7078 "parser.c" /* yacc.c:1646  */
    break;

  case 396:
#line 2856 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7086 "parser.c" /* yacc.c:1646  */
    break;

  case 397:
#line 2860 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7092 "parser.c" /* yacc.c:1646  */
    break;

  case 398:
#line 2861 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCAUTO, (yyvsp[-1].decl));
    }
#line 7101 "parser.c" /* yacc.c:1646  */
    break;

  case 399:
#line 2869 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCFOR, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 7109 "parser.c" /* yacc.c:1646  */
    break;

  case 400:
#line 2876 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCFOR, (yyvsp[-1].ocla));
    }
#line 7117 "parser.c" /* yacc.c:1646  */
    break;

  case 401:
#line 2883 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7125 "parser.c" /* yacc.c:1646  */
    break;

  case 402:
#line 2887 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7133 "parser.c" /* yacc.c:1646  */
    break;

  case 403:
#line 2891 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7141 "parser.c" /* yacc.c:1646  */
    break;

  case 404:
#line 2898 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7149 "parser.c" /* yacc.c:1646  */
    break;

  case 405:
#line 2902 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7157 "parser.c" /* yacc.c:1646  */
    break;

  case 406:
#line 2906 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7165 "parser.c" /* yacc.c:1646  */
    break;

  case 407:
#line 2910 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7173 "parser.c" /* yacc.c:1646  */
    break;

  case 408:
#line 2914 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7181 "parser.c" /* yacc.c:1646  */
    break;

  case 409:
#line 2918 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCNOWAIT);
    }
#line 7189 "parser.c" /* yacc.c:1646  */
    break;

  case 410:
#line 2925 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCORDERED);
    }
#line 7197 "parser.c" /* yacc.c:1646  */
    break;

  case 411:
#line 2929 "parser.y" /* yacc.c:1646  */
    {
      int n = 0, er = 0;
      if (xar_expr_is_constant((yyvsp[-1].expr)))
      {
        n = xar_calc_int_expr((yyvsp[-1].expr), &er);
        if (er) n = 0;
      }
      if (n <= 0)
        parse_error(1, "invalid number in ordered() clause.\n");
      (yyval.ocla) = OrderedNumClause(n);
    }
#line 7213 "parser.c" /* yacc.c:1646  */
    break;

  case 412:
#line 2941 "parser.y" /* yacc.c:1646  */
    {
      check_schedule((yyvsp[-1].type), (yyvsp[-2].type));
      (yyval.ocla) = ScheduleClause((yyvsp[-1].type), (yyvsp[-2].type), NULL);
    }
#line 7222 "parser.c" /* yacc.c:1646  */
    break;

  case 413:
#line 2946 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7228 "parser.c" /* yacc.c:1646  */
    break;

  case 414:
#line 2947 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      if ((yyvsp[-4].type) == OC_runtime)
        parse_error(1, "\"runtime\" schedules may not have a chunksize.\n");
      check_schedule((yyvsp[-4].type), (yyvsp[-5].type));
      (yyval.ocla) = ScheduleClause((yyvsp[-4].type), (yyvsp[-5].type), (yyvsp[-1].expr));
    }
#line 7240 "parser.c" /* yacc.c:1646  */
    break;

  case 415:
#line 2955 "parser.y" /* yacc.c:1646  */
    {  /* non-OpenMP schedule */
      tempsave = checkDecls;
      checkDecls = 0;   /* Because the index of the loop is usualy involved */
      sc_pause_openmp();
    }
#line 7250 "parser.c" /* yacc.c:1646  */
    break;

  case 416:
#line 2961 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      checkDecls = tempsave;
      (yyval.ocla) = ScheduleClause(OC_affinity, OCM_none, (yyvsp[-1].expr));
    }
#line 7260 "parser.c" /* yacc.c:1646  */
    break;

  case 417:
#line 2967 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7268 "parser.c" /* yacc.c:1646  */
    break;

  case 418:
#line 2974 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_static;
    }
#line 7276 "parser.c" /* yacc.c:1646  */
    break;

  case 419:
#line 2978 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_dynamic;
    }
#line 7284 "parser.c" /* yacc.c:1646  */
    break;

  case 420:
#line 2982 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_guided;
    }
#line 7292 "parser.c" /* yacc.c:1646  */
    break;

  case 421:
#line 2986 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_runtime;
    }
#line 7300 "parser.c" /* yacc.c:1646  */
    break;

  case 422:
#line 2990 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_auto;
    }
#line 7308 "parser.c" /* yacc.c:1646  */
    break;

  case 423:
#line 2993 "parser.y" /* yacc.c:1646  */
    { parse_error(1, "invalid openmp schedule type.\n"); }
#line 7314 "parser.c" /* yacc.c:1646  */
    break;

  case 424:
#line 2998 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OCM_none;
    }
#line 7322 "parser.c" /* yacc.c:1646  */
    break;

  case 425:
#line 3002 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OCM_monotonic;
    }
#line 7330 "parser.c" /* yacc.c:1646  */
    break;

  case 426:
#line 3006 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OCM_nonmonotonic;
    }
#line 7338 "parser.c" /* yacc.c:1646  */
    break;

  case 427:
#line 3013 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCSECTIONS, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 7346 "parser.c" /* yacc.c:1646  */
    break;

  case 428:
#line 3020 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCSECTIONS, (yyvsp[-1].ocla));
    }
#line 7354 "parser.c" /* yacc.c:1646  */
    break;

  case 429:
#line 3027 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7362 "parser.c" /* yacc.c:1646  */
    break;

  case 430:
#line 3031 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7370 "parser.c" /* yacc.c:1646  */
    break;

  case 431:
#line 3035 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7378 "parser.c" /* yacc.c:1646  */
    break;

  case 432:
#line 3042 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7386 "parser.c" /* yacc.c:1646  */
    break;

  case 433:
#line 3046 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7394 "parser.c" /* yacc.c:1646  */
    break;

  case 434:
#line 3050 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7402 "parser.c" /* yacc.c:1646  */
    break;

  case 435:
#line 3054 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7410 "parser.c" /* yacc.c:1646  */
    break;

  case 436:
#line 3058 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCNOWAIT);
    }
#line 7418 "parser.c" /* yacc.c:1646  */
    break;

  case 437:
#line 3065 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = Compound((yyvsp[-1].stmt));
    }
#line 7426 "parser.c" /* yacc.c:1646  */
    break;

  case 438:
#line 3072 "parser.y" /* yacc.c:1646  */
    {
      /* Make it look like it had a section pragma */
      (yyval.stmt) = OmpStmt( OmpConstruct(DCSECTION, OmpDirective(DCSECTION,NULL), (yyvsp[0].stmt)) );
    }
#line 7435 "parser.c" /* yacc.c:1646  */
    break;

  case 439:
#line 3077 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = OmpStmt( OmpConstruct(DCSECTION, (yyvsp[-1].odir), (yyvsp[0].stmt)) );
    }
#line 7443 "parser.c" /* yacc.c:1646  */
    break;

  case 440:
#line 3081 "parser.y" /* yacc.c:1646  */
    {
      (yyval.stmt) = BlockList((yyvsp[-2].stmt), OmpStmt( OmpConstruct(DCSECTION, (yyvsp[-1].odir), (yyvsp[0].stmt)) ));
    }
#line 7451 "parser.c" /* yacc.c:1646  */
    break;

  case 441:
#line 3088 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCSECTION, NULL);
    }
#line 7459 "parser.c" /* yacc.c:1646  */
    break;

  case 442:
#line 3095 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCSINGLE, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 7467 "parser.c" /* yacc.c:1646  */
    break;

  case 443:
#line 3102 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCSINGLE, (yyvsp[-1].ocla));
    }
#line 7475 "parser.c" /* yacc.c:1646  */
    break;

  case 444:
#line 3109 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7483 "parser.c" /* yacc.c:1646  */
    break;

  case 445:
#line 3113 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7491 "parser.c" /* yacc.c:1646  */
    break;

  case 446:
#line 3117 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7499 "parser.c" /* yacc.c:1646  */
    break;

  case 447:
#line 3124 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7507 "parser.c" /* yacc.c:1646  */
    break;

  case 448:
#line 3128 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7515 "parser.c" /* yacc.c:1646  */
    break;

  case 449:
#line 3132 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7523 "parser.c" /* yacc.c:1646  */
    break;

  case 450:
#line 3136 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCNOWAIT);
    }
#line 7531 "parser.c" /* yacc.c:1646  */
    break;

  case 451:
#line 3142 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7537 "parser.c" /* yacc.c:1646  */
    break;

  case 452:
#line 3143 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCCOPYPRIVATE, (yyvsp[-1].decl));
    }
#line 7546 "parser.c" /* yacc.c:1646  */
    break;

  case 453:
#line 3152 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCSIMD, $1, $2); TODO DCSIMD
    }
#line 7554 "parser.c" /* yacc.c:1646  */
    break;

  case 454:
#line 3160 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCSIMD, $3); TODO DCSIMD
    }
#line 7562 "parser.c" /* yacc.c:1646  */
    break;

  case 455:
#line 3167 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7570 "parser.c" /* yacc.c:1646  */
    break;

  case 456:
#line 3171 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7578 "parser.c" /* yacc.c:1646  */
    break;

  case 457:
#line 3175 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7586 "parser.c" /* yacc.c:1646  */
    break;

  case 458:
#line 3183 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7594 "parser.c" /* yacc.c:1646  */
    break;

  case 459:
#line 3187 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7602 "parser.c" /* yacc.c:1646  */
    break;

  case 460:
#line 3191 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7610 "parser.c" /* yacc.c:1646  */
    break;

  case 461:
#line 3195 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7618 "parser.c" /* yacc.c:1646  */
    break;

  case 462:
#line 3199 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7626 "parser.c" /* yacc.c:1646  */
    break;

  case 463:
#line 3207 "parser.y" /* yacc.c:1646  */
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
#line 7642 "parser.c" /* yacc.c:1646  */
    break;

  case 464:
#line 3219 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7650 "parser.c" /* yacc.c:1646  */
    break;

  case 465:
#line 3223 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7658 "parser.c" /* yacc.c:1646  */
    break;

  case 466:
#line 3231 "parser.y" /* yacc.c:1646  */
    {
      //$$ = PlainClause(OCINBRANCH); TODO ast
    }
#line 7666 "parser.c" /* yacc.c:1646  */
    break;

  case 467:
#line 3235 "parser.y" /* yacc.c:1646  */
    {
      //$$ = PlainClause(OCNOTINBRANCH); TODO ast
    }
#line 7674 "parser.c" /* yacc.c:1646  */
    break;

  case 468:
#line 3242 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7680 "parser.c" /* yacc.c:1646  */
    break;

  case 469:
#line 3243 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      //$$ = VarlistClause(OCUNIFORM, $4); TODO ast
    }
#line 7689 "parser.c" /* yacc.c:1646  */
    break;

  case 470:
#line 3251 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7695 "parser.c" /* yacc.c:1646  */
    break;

  case 471:
#line 3252 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      // TODO ast
    }
#line 7704 "parser.c" /* yacc.c:1646  */
    break;

  case 472:
#line 3260 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 7710 "parser.c" /* yacc.c:1646  */
    break;

  case 473:
#line 3261 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      // TODO ast
    }
#line 7719 "parser.c" /* yacc.c:1646  */
    break;

  case 474:
#line 3269 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = NULL;
    }
#line 7727 "parser.c" /* yacc.c:1646  */
    break;

  case 475:
#line 3273 "parser.y" /* yacc.c:1646  */
    {
      // TODO ast
    }
#line 7735 "parser.c" /* yacc.c:1646  */
    break;

  case 476:
#line 3281 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCDECLSIMD, $1, $2); TODO DCDECLSIMD or change it to stmt
    }
#line 7743 "parser.c" /* yacc.c:1646  */
    break;

  case 477:
#line 3289 "parser.y" /* yacc.c:1646  */
    {
      //TODO
    }
#line 7751 "parser.c" /* yacc.c:1646  */
    break;

  case 478:
#line 3293 "parser.y" /* yacc.c:1646  */
    {
        //TODO
    }
#line 7759 "parser.c" /* yacc.c:1646  */
    break;

  case 479:
#line 3301 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCDECLSIMD, $4); TODO DCDECLSIMD
    }
#line 7767 "parser.c" /* yacc.c:1646  */
    break;

  case 480:
#line 3308 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7775 "parser.c" /* yacc.c:1646  */
    break;

  case 481:
#line 3312 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7783 "parser.c" /* yacc.c:1646  */
    break;

  case 482:
#line 3316 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7791 "parser.c" /* yacc.c:1646  */
    break;

  case 483:
#line 3325 "parser.y" /* yacc.c:1646  */
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
#line 7807 "parser.c" /* yacc.c:1646  */
    break;

  case 484:
#line 3337 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7815 "parser.c" /* yacc.c:1646  */
    break;

  case 485:
#line 3341 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7823 "parser.c" /* yacc.c:1646  */
    break;

  case 486:
#line 3345 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7831 "parser.c" /* yacc.c:1646  */
    break;

  case 487:
#line 3349 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7839 "parser.c" /* yacc.c:1646  */
    break;

  case 488:
#line 3357 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCFORSIMD, $1, $2); TODO DCFORSIMD
    }
#line 7847 "parser.c" /* yacc.c:1646  */
    break;

  case 489:
#line 3365 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCFORSIMD, $4); TODO DCFORSIMD
    }
#line 7855 "parser.c" /* yacc.c:1646  */
    break;

  case 490:
#line 3372 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7863 "parser.c" /* yacc.c:1646  */
    break;

  case 491:
#line 3376 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7871 "parser.c" /* yacc.c:1646  */
    break;

  case 492:
#line 3380 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7879 "parser.c" /* yacc.c:1646  */
    break;

  case 493:
#line 3387 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7887 "parser.c" /* yacc.c:1646  */
    break;

  case 494:
#line 3391 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7895 "parser.c" /* yacc.c:1646  */
    break;

  case 495:
#line 3399 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCPARFORSIMD, $1, $2); TODO DCPARFORSIMD
    }
#line 7903 "parser.c" /* yacc.c:1646  */
    break;

  case 496:
#line 3407 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCPARFORSIMD, $5); TODO DCFORSIMD
    }
#line 7911 "parser.c" /* yacc.c:1646  */
    break;

  case 497:
#line 3414 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7919 "parser.c" /* yacc.c:1646  */
    break;

  case 498:
#line 3418 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7927 "parser.c" /* yacc.c:1646  */
    break;

  case 499:
#line 3422 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7935 "parser.c" /* yacc.c:1646  */
    break;

  case 500:
#line 3429 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7943 "parser.c" /* yacc.c:1646  */
    break;

  case 501:
#line 3433 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7951 "parser.c" /* yacc.c:1646  */
    break;

  case 502:
#line 3441 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTARGETDATA, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 7959 "parser.c" /* yacc.c:1646  */
    break;

  case 503:
#line 3449 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTARGETDATA, (yyvsp[-1].ocla));
    }
#line 7967 "parser.c" /* yacc.c:1646  */
    break;

  case 504:
#line 3456 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 7975 "parser.c" /* yacc.c:1646  */
    break;

  case 505:
#line 3460 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 7983 "parser.c" /* yacc.c:1646  */
    break;

  case 506:
#line 3464 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 7991 "parser.c" /* yacc.c:1646  */
    break;

  case 507:
#line 3471 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 7999 "parser.c" /* yacc.c:1646  */
    break;

  case 508:
#line 3475 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
      /* OpenMP v4.5, 2.10.1, page 97: must be to/from/tofrom/alloc */
      if ((yyval.ocla)->subtype != OC_tofrom && (yyval.ocla)->subtype != OC_to && 
          (yyval.ocla)->subtype != OC_from   && (yyval.ocla)->subtype != OC_alloc)
        parse_error(1, "expected a map type of 'to', 'from', 'tofrom' or 'alloc'\n");
    }
#line 8011 "parser.c" /* yacc.c:1646  */
    break;

  case 509:
#line 3483 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8019 "parser.c" /* yacc.c:1646  */
    break;

  case 510:
#line 3487 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8027 "parser.c" /* yacc.c:1646  */
    break;

  case 511:
#line 3494 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 8033 "parser.c" /* yacc.c:1646  */
    break;

  case 512:
#line 3495 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = DeviceClause((yyvsp[-1].expr));
    }
#line 8042 "parser.c" /* yacc.c:1646  */
    break;

  case 513:
#line 3504 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 8048 "parser.c" /* yacc.c:1646  */
    break;

  case 514:
#line 3505 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = MapClause((yyvsp[-4].type), (yyvsp[-5].type), (yyvsp[-1].oxli));
    }
#line 8057 "parser.c" /* yacc.c:1646  */
    break;

  case 515:
#line 3509 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 8063 "parser.c" /* yacc.c:1646  */
    break;

  case 516:
#line 3510 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = MapClause(OC_tofrom, OCM_none, (yyvsp[-1].oxli));
    }
#line 8072 "parser.c" /* yacc.c:1646  */
    break;

  case 517:
#line 3519 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OCM_none;
    }
#line 8080 "parser.c" /* yacc.c:1646  */
    break;

  case 518:
#line 3523 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OCM_always;
    }
#line 8088 "parser.c" /* yacc.c:1646  */
    break;

  case 519:
#line 3527 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OCM_always;
    }
#line 8096 "parser.c" /* yacc.c:1646  */
    break;

  case 520:
#line 3535 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_alloc;
    }
#line 8104 "parser.c" /* yacc.c:1646  */
    break;

  case 521:
#line 3539 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_to;
    }
#line 8112 "parser.c" /* yacc.c:1646  */
    break;

  case 522:
#line 3543 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_from;
    }
#line 8120 "parser.c" /* yacc.c:1646  */
    break;

  case 523:
#line 3547 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_tofrom;
    }
#line 8128 "parser.c" /* yacc.c:1646  */
    break;

  case 524:
#line 3551 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_release; 
    }
#line 8136 "parser.c" /* yacc.c:1646  */
    break;

  case 525:
#line 3555 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_delete;
    }
#line 8144 "parser.c" /* yacc.c:1646  */
    break;

  case 526:
#line 3562 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 8150 "parser.c" /* yacc.c:1646  */
    break;

  case 527:
#line 3563 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCUSEDEVPTR, (yyvsp[-1].decl));
    }
#line 8159 "parser.c" /* yacc.c:1646  */
    break;

  case 528:
#line 3572 "parser.y" /* yacc.c:1646  */
    {
      /* No parameters needed */
      (yyval.ocla) = PlainClause(OCDEFAULTMAP);
    }
#line 8168 "parser.c" /* yacc.c:1646  */
    break;

  case 529:
#line 3580 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 8174 "parser.c" /* yacc.c:1646  */
    break;

  case 530:
#line 3581 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCISDEVPTR, (yyvsp[-1].decl));
    }
#line 8183 "parser.c" /* yacc.c:1646  */
    break;

  case 531:
#line 3589 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = errorOnReturn;  errorOnReturn = 1; }
#line 8189 "parser.c" /* yacc.c:1646  */
    break;

  case 532:
#line 3591 "parser.y" /* yacc.c:1646  */
    {
      errorOnReturn = (yyvsp[-1].type);
      (yyval.ocon) = OmpConstruct(DCTARGET, (yyvsp[-2].odir), (yyvsp[0].stmt));
      __has_target = 1;
    }
#line 8199 "parser.c" /* yacc.c:1646  */
    break;

  case 533:
#line 3601 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTARGET, (yyvsp[-1].ocla));
    }
#line 8207 "parser.c" /* yacc.c:1646  */
    break;

  case 534:
#line 3608 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8215 "parser.c" /* yacc.c:1646  */
    break;

  case 535:
#line 3612 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8223 "parser.c" /* yacc.c:1646  */
    break;

  case 536:
#line 3616 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8231 "parser.c" /* yacc.c:1646  */
    break;

  case 537:
#line 3623 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8239 "parser.c" /* yacc.c:1646  */
    break;

  case 538:
#line 3627 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8247 "parser.c" /* yacc.c:1646  */
    break;

  case 539:
#line 3631 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8255 "parser.c" /* yacc.c:1646  */
    break;

  case 540:
#line 3635 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCNOWAIT);
    }
#line 8263 "parser.c" /* yacc.c:1646  */
    break;

  case 541:
#line 3639 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8271 "parser.c" /* yacc.c:1646  */
    break;

  case 542:
#line 3643 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8279 "parser.c" /* yacc.c:1646  */
    break;

  case 543:
#line 3650 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8287 "parser.c" /* yacc.c:1646  */
    break;

  case 544:
#line 3654 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
      /* OpenMP v4.5, 2.10.4, page 106: must be to/from/tofrom/alloc */
      if ((yyval.ocla)->subtype != OC_tofrom && (yyval.ocla)->subtype != OC_to && 
          (yyval.ocla)->subtype != OC_from   && (yyval.ocla)->subtype != OC_alloc)
        parse_error(1, "expected a map type of 'to', 'from', 'tofrom' or 'alloc'\n");
    }
#line 8299 "parser.c" /* yacc.c:1646  */
    break;

  case 545:
#line 3662 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8307 "parser.c" /* yacc.c:1646  */
    break;

  case 546:
#line 3666 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8315 "parser.c" /* yacc.c:1646  */
    break;

  case 547:
#line 3674 "parser.y" /* yacc.c:1646  */
    {
      if (xc_clauselist_get_clause((yyvsp[-1].ocla), OCMAP, 0) == NULL)
        parse_error(1, "target enter data directives must contain at least 1 "
                       "map() clause");
      (yyval.odir) = OmpDirective(DCTARGENTERDATA, (yyvsp[-1].ocla));
    }
#line 8326 "parser.c" /* yacc.c:1646  */
    break;

  case 548:
#line 3684 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8334 "parser.c" /* yacc.c:1646  */
    break;

  case 549:
#line 3688 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8342 "parser.c" /* yacc.c:1646  */
    break;

  case 550:
#line 3692 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8350 "parser.c" /* yacc.c:1646  */
    break;

  case 551:
#line 3699 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
      /* OpenMP v4.5, 2.10.2, page 99: must be to/alloc */
      if ((yyval.ocla)->subtype != OC_to && (yyval.ocla)->subtype != OC_alloc)
        parse_error(1, "expected a map type of 'to' or 'alloc'\n");
    }
#line 8361 "parser.c" /* yacc.c:1646  */
    break;

  case 552:
#line 3706 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8369 "parser.c" /* yacc.c:1646  */
    break;

  case 553:
#line 3710 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8377 "parser.c" /* yacc.c:1646  */
    break;

  case 554:
#line 3714 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8385 "parser.c" /* yacc.c:1646  */
    break;

  case 555:
#line 3718 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCNOWAIT);
    }
#line 8393 "parser.c" /* yacc.c:1646  */
    break;

  case 556:
#line 3725 "parser.y" /* yacc.c:1646  */
    {
      if (xc_clauselist_get_clause((yyvsp[-1].ocla), OCMAP, 0) == NULL)
        parse_error(1, "target exit data directives must contain at least 1 "
                       "map() clause");
      (yyval.odir) = OmpDirective(DCTARGEXITDATA, (yyvsp[-1].ocla));
    }
#line 8404 "parser.c" /* yacc.c:1646  */
    break;

  case 557:
#line 3735 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8412 "parser.c" /* yacc.c:1646  */
    break;

  case 558:
#line 3739 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8420 "parser.c" /* yacc.c:1646  */
    break;

  case 559:
#line 3743 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8428 "parser.c" /* yacc.c:1646  */
    break;

  case 560:
#line 3750 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
      /* OpenMP v4.5, 2.10.3, page 102: must be from/release/delete */
      if ((yyval.ocla)->subtype != OC_from && (yyval.ocla)->subtype != OC_release &&
          (yyval.ocla)->subtype != OC_delete)
        parse_error(1, "expected a map type of 'from', 'release' or 'delete'\n");
    }
#line 8440 "parser.c" /* yacc.c:1646  */
    break;

  case 561:
#line 3758 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8448 "parser.c" /* yacc.c:1646  */
    break;

  case 562:
#line 3762 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8456 "parser.c" /* yacc.c:1646  */
    break;

  case 563:
#line 3766 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8464 "parser.c" /* yacc.c:1646  */
    break;

  case 564:
#line 3770 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCNOWAIT);
    }
#line 8472 "parser.c" /* yacc.c:1646  */
    break;

  case 565:
#line 3778 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTARGETUPD, (yyvsp[-1].ocla));
    }
#line 8480 "parser.c" /* yacc.c:1646  */
    break;

  case 566:
#line 3785 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8488 "parser.c" /* yacc.c:1646  */
    break;

  case 567:
#line 3789 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8496 "parser.c" /* yacc.c:1646  */
    break;

  case 568:
#line 3793 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8504 "parser.c" /* yacc.c:1646  */
    break;

  case 569:
#line 3800 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8512 "parser.c" /* yacc.c:1646  */
    break;

  case 570:
#line 3804 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8520 "parser.c" /* yacc.c:1646  */
    break;

  case 571:
#line 3808 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8528 "parser.c" /* yacc.c:1646  */
    break;

  case 572:
#line 3812 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8536 "parser.c" /* yacc.c:1646  */
    break;

  case 573:
#line 3816 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCNOWAIT);
    }
#line 8544 "parser.c" /* yacc.c:1646  */
    break;

  case 574:
#line 3822 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 8550 "parser.c" /* yacc.c:1646  */
    break;

  case 575:
#line 3823 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = UpdateClause(OCTO, (yyvsp[-1].oxli));
    }
#line 8559 "parser.c" /* yacc.c:1646  */
    break;

  case 576:
#line 3827 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 8565 "parser.c" /* yacc.c:1646  */
    break;

  case 577:
#line 3828 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = UpdateClause(OCFROM, (yyvsp[-1].oxli));
    }
#line 8574 "parser.c" /* yacc.c:1646  */
    break;

  case 578:
#line 3839 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCDECLTARGET, (yyvsp[-2].odir), (yyvsp[-1].stmt));
    }
#line 8582 "parser.c" /* yacc.c:1646  */
    break;

  case 579:
#line 3843 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCDECLTARGET, (yyvsp[0].odir), NULL);
    }
#line 8590 "parser.c" /* yacc.c:1646  */
    break;

  case 580:
#line 3850 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCDECLTARGET, NULL);
    }
#line 8598 "parser.c" /* yacc.c:1646  */
    break;

  case 581:
#line 3857 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCENDDECLTARGET, NULL); TODO DCENDDECLTARGET
    }
#line 8606 "parser.c" /* yacc.c:1646  */
    break;

  case 582:
#line 3866 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCDECLTARGET, UpdateClause(OCTO, (yyvsp[-2].oxli)));
    }
#line 8614 "parser.c" /* yacc.c:1646  */
    break;

  case 583:
#line 3870 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCDECLTARGET, (yyvsp[-1].ocla));
    }
#line 8622 "parser.c" /* yacc.c:1646  */
    break;

  case 584:
#line 3878 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8630 "parser.c" /* yacc.c:1646  */
    break;

  case 585:
#line 3882 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8638 "parser.c" /* yacc.c:1646  */
    break;

  case 586:
#line 3886 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8646 "parser.c" /* yacc.c:1646  */
    break;

  case 587:
#line 3893 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 8652 "parser.c" /* yacc.c:1646  */
    break;

  case 588:
#line 3894 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = UpdateClause(OCTO, (yyvsp[-1].oxli));
    }
#line 8661 "parser.c" /* yacc.c:1646  */
    break;

  case 589:
#line 3899 "parser.y" /* yacc.c:1646  */
    { 
      tempsave = checkDecls;   /* No check--the directive can appear anywhere */
      checkDecls = 0; 
      sc_pause_openmp(); 
    }
#line 8671 "parser.c" /* yacc.c:1646  */
    break;

  case 590:
#line 3905 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      checkDecls = tempsave;
      (yyval.ocla) = UpdateClause(OCLINK, (yyvsp[-1].oxli));
    }
#line 8681 "parser.c" /* yacc.c:1646  */
    break;

  case 591:
#line 3917 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTEAMS, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 8689 "parser.c" /* yacc.c:1646  */
    break;

  case 592:
#line 3927 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTEAMS, (yyvsp[-1].ocla));
    }
#line 8697 "parser.c" /* yacc.c:1646  */
    break;

  case 593:
#line 3934 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8705 "parser.c" /* yacc.c:1646  */
    break;

  case 594:
#line 3938 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8713 "parser.c" /* yacc.c:1646  */
    break;

  case 595:
#line 3942 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8721 "parser.c" /* yacc.c:1646  */
    break;

  case 596:
#line 3949 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8729 "parser.c" /* yacc.c:1646  */
    break;

  case 597:
#line 3953 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8737 "parser.c" /* yacc.c:1646  */
    break;

  case 598:
#line 3957 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8745 "parser.c" /* yacc.c:1646  */
    break;

  case 599:
#line 3961 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8753 "parser.c" /* yacc.c:1646  */
    break;

  case 600:
#line 3965 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8761 "parser.c" /* yacc.c:1646  */
    break;

  case 601:
#line 3969 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8769 "parser.c" /* yacc.c:1646  */
    break;

  case 602:
#line 3977 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 8775 "parser.c" /* yacc.c:1646  */
    break;

  case 603:
#line 3978 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = NumteamsClause((yyvsp[-1].expr));
    }
#line 8784 "parser.c" /* yacc.c:1646  */
    break;

  case 604:
#line 3983 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 8790 "parser.c" /* yacc.c:1646  */
    break;

  case 605:
#line 3984 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = ThreadlimitClause((yyvsp[-1].expr));
    }
#line 8799 "parser.c" /* yacc.c:1646  */
    break;

  case 606:
#line 3993 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCDISTRIBUTE, $1, $2); TODO DCDISTRIBUTE
    }
#line 8807 "parser.c" /* yacc.c:1646  */
    break;

  case 607:
#line 4001 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCDISTRIBUTE, $3); TODO DCDISTRIBUTE
    }
#line 8815 "parser.c" /* yacc.c:1646  */
    break;

  case 608:
#line 4008 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8823 "parser.c" /* yacc.c:1646  */
    break;

  case 609:
#line 4012 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8831 "parser.c" /* yacc.c:1646  */
    break;

  case 610:
#line 4016 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8839 "parser.c" /* yacc.c:1646  */
    break;

  case 611:
#line 4023 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8847 "parser.c" /* yacc.c:1646  */
    break;

  case 612:
#line 4027 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8855 "parser.c" /* yacc.c:1646  */
    break;

  case 613:
#line 4031 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8863 "parser.c" /* yacc.c:1646  */
    break;

  case 614:
#line 4035 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8871 "parser.c" /* yacc.c:1646  */
    break;

  case 615:
#line 4042 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = ScheduleClause(OC_static, OCM_none, NULL);
    }
#line 8879 "parser.c" /* yacc.c:1646  */
    break;

  case 616:
#line 4045 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 8885 "parser.c" /* yacc.c:1646  */
    break;

  case 617:
#line 4046 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = ScheduleClause(OC_static,OCM_none,  (yyvsp[-1].expr));
    }
#line 8894 "parser.c" /* yacc.c:1646  */
    break;

  case 618:
#line 4055 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCDISTSIMD, $1, $2); TODO DCDISTSIMD
      //$$->l = $1->l;
    }
#line 8903 "parser.c" /* yacc.c:1646  */
    break;

  case 619:
#line 4064 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCDISTSIMD, $4); TODO DCDISTSIMD
    }
#line 8911 "parser.c" /* yacc.c:1646  */
    break;

  case 620:
#line 4071 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8919 "parser.c" /* yacc.c:1646  */
    break;

  case 621:
#line 4075 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8927 "parser.c" /* yacc.c:1646  */
    break;

  case 622:
#line 4079 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 8935 "parser.c" /* yacc.c:1646  */
    break;

  case 623:
#line 4086 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8943 "parser.c" /* yacc.c:1646  */
    break;

  case 624:
#line 4090 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8951 "parser.c" /* yacc.c:1646  */
    break;

  case 625:
#line 4094 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 8959 "parser.c" /* yacc.c:1646  */
    break;

  case 626:
#line 4102 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCDISTPARFOR, $1, $2); TODO DCDISTPARFOR
      //$$->l = $1->l;
    }
#line 8968 "parser.c" /* yacc.c:1646  */
    break;

  case 627:
#line 4111 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCDISTPARFOR, $5); TODO DCDISTPARFOR
    }
#line 8976 "parser.c" /* yacc.c:1646  */
    break;

  case 628:
#line 4118 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 8984 "parser.c" /* yacc.c:1646  */
    break;

  case 629:
#line 4122 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 8992 "parser.c" /* yacc.c:1646  */
    break;

  case 630:
#line 4126 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 9000 "parser.c" /* yacc.c:1646  */
    break;

  case 631:
#line 4133 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9008 "parser.c" /* yacc.c:1646  */
    break;

  case 632:
#line 4137 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9016 "parser.c" /* yacc.c:1646  */
    break;

  case 633:
#line 4145 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCDISTPARFORSIMD, $1, $2); TODO DCDISTPARFORSIMD
      //$$->l = $1->l;
    }
#line 9025 "parser.c" /* yacc.c:1646  */
    break;

  case 634:
#line 4154 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCDISTPARFORSIMD, $6); TODO DCDISTPARFORSIMD
    }
#line 9033 "parser.c" /* yacc.c:1646  */
    break;

  case 635:
#line 4161 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 9041 "parser.c" /* yacc.c:1646  */
    break;

  case 636:
#line 4165 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 9049 "parser.c" /* yacc.c:1646  */
    break;

  case 637:
#line 4169 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 9057 "parser.c" /* yacc.c:1646  */
    break;

  case 638:
#line 4176 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9065 "parser.c" /* yacc.c:1646  */
    break;

  case 639:
#line 4180 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9073 "parser.c" /* yacc.c:1646  */
    break;

  case 640:
#line 4188 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTARGETTEAMS, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 9081 "parser.c" /* yacc.c:1646  */
    break;

  case 641:
#line 4196 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTARGETTEAMS, (yyvsp[-1].ocla));
    }
#line 9089 "parser.c" /* yacc.c:1646  */
    break;

  case 642:
#line 4203 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 9097 "parser.c" /* yacc.c:1646  */
    break;

  case 643:
#line 4207 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 9105 "parser.c" /* yacc.c:1646  */
    break;

  case 644:
#line 4211 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 9113 "parser.c" /* yacc.c:1646  */
    break;

  case 645:
#line 4218 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9121 "parser.c" /* yacc.c:1646  */
    break;

  case 646:
#line 4222 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9129 "parser.c" /* yacc.c:1646  */
    break;

  case 647:
#line 4226 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9137 "parser.c" /* yacc.c:1646  */
    break;

  case 648:
#line 4230 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9145 "parser.c" /* yacc.c:1646  */
    break;

  case 649:
#line 4234 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCNOWAIT);
    }
#line 9153 "parser.c" /* yacc.c:1646  */
    break;

  case 650:
#line 4242 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTEAMSDIST, $1, $2); TODO DCTEAMSDIST
    }
#line 9161 "parser.c" /* yacc.c:1646  */
    break;

  case 651:
#line 4250 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTEAMSDIST, $4); TODO DCTEAMSDIST
    }
#line 9169 "parser.c" /* yacc.c:1646  */
    break;

  case 652:
#line 4257 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 9177 "parser.c" /* yacc.c:1646  */
    break;

  case 653:
#line 4261 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 9185 "parser.c" /* yacc.c:1646  */
    break;

  case 654:
#line 4265 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 9193 "parser.c" /* yacc.c:1646  */
    break;

  case 655:
#line 4272 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9201 "parser.c" /* yacc.c:1646  */
    break;

  case 656:
#line 4276 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9209 "parser.c" /* yacc.c:1646  */
    break;

  case 657:
#line 4280 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9217 "parser.c" /* yacc.c:1646  */
    break;

  case 658:
#line 4288 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTEAMSDISTSIMD, $1, $2); TODO DCTEAMSDISTSIMD
    }
#line 9225 "parser.c" /* yacc.c:1646  */
    break;

  case 659:
#line 4296 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTEAMSDISTSIMD, $5); TODO DCTEAMSDISTSIMD
    }
#line 9233 "parser.c" /* yacc.c:1646  */
    break;

  case 660:
#line 4303 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 9241 "parser.c" /* yacc.c:1646  */
    break;

  case 661:
#line 4307 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 9249 "parser.c" /* yacc.c:1646  */
    break;

  case 662:
#line 4311 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 9257 "parser.c" /* yacc.c:1646  */
    break;

  case 663:
#line 4318 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9265 "parser.c" /* yacc.c:1646  */
    break;

  case 664:
#line 4322 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9273 "parser.c" /* yacc.c:1646  */
    break;

  case 665:
#line 4326 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9281 "parser.c" /* yacc.c:1646  */
    break;

  case 666:
#line 4330 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9289 "parser.c" /* yacc.c:1646  */
    break;

  case 667:
#line 4338 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTARGETTEAMSDIST, $1, $2); TODO DCTARGETTEAMSDIST
    }
#line 9297 "parser.c" /* yacc.c:1646  */
    break;

  case 668:
#line 4346 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTARGETTEAMSDIST, $5); TODO DCTARGETTEAMSDIST
    }
#line 9305 "parser.c" /* yacc.c:1646  */
    break;

  case 669:
#line 4353 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 9313 "parser.c" /* yacc.c:1646  */
    break;

  case 670:
#line 4357 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 9321 "parser.c" /* yacc.c:1646  */
    break;

  case 671:
#line 4361 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 9329 "parser.c" /* yacc.c:1646  */
    break;

  case 672:
#line 4368 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9337 "parser.c" /* yacc.c:1646  */
    break;

  case 673:
#line 4372 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9345 "parser.c" /* yacc.c:1646  */
    break;

  case 674:
#line 4376 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9353 "parser.c" /* yacc.c:1646  */
    break;

  case 675:
#line 4380 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9361 "parser.c" /* yacc.c:1646  */
    break;

  case 676:
#line 4384 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCNOWAIT);
    }
#line 9369 "parser.c" /* yacc.c:1646  */
    break;

  case 677:
#line 4393 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTARGETTEAMSDISTSIMD, $1, $2); TODO DCTARGETTEAMSDISTSIMD
    }
#line 9377 "parser.c" /* yacc.c:1646  */
    break;

  case 678:
#line 4402 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTARGETTEAMSDISTSIMD, $6); TODO DCTARGETTEAMSDISTSIMD
    }
#line 9385 "parser.c" /* yacc.c:1646  */
    break;

  case 679:
#line 4409 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 9393 "parser.c" /* yacc.c:1646  */
    break;

  case 680:
#line 4414 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 9401 "parser.c" /* yacc.c:1646  */
    break;

  case 681:
#line 4419 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 9409 "parser.c" /* yacc.c:1646  */
    break;

  case 682:
#line 4426 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9417 "parser.c" /* yacc.c:1646  */
    break;

  case 683:
#line 4430 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9425 "parser.c" /* yacc.c:1646  */
    break;

  case 684:
#line 4434 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9433 "parser.c" /* yacc.c:1646  */
    break;

  case 685:
#line 4438 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9441 "parser.c" /* yacc.c:1646  */
    break;

  case 686:
#line 4442 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCNOWAIT);
    }
#line 9449 "parser.c" /* yacc.c:1646  */
    break;

  case 687:
#line 4450 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTEAMSDISTPARFOR, $1, $2); TODO DCTEAMSDISTPARFOR
      //$$->l = $1->l;
    }
#line 9458 "parser.c" /* yacc.c:1646  */
    break;

  case 688:
#line 4460 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTEAMSDISTPARFOR, $3); TODO DCTEAMSDISTPARFOR
    }
#line 9466 "parser.c" /* yacc.c:1646  */
    break;

  case 689:
#line 4467 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 9474 "parser.c" /* yacc.c:1646  */
    break;

  case 690:
#line 4472 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 9482 "parser.c" /* yacc.c:1646  */
    break;

  case 691:
#line 4477 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 9490 "parser.c" /* yacc.c:1646  */
    break;

  case 692:
#line 4484 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9498 "parser.c" /* yacc.c:1646  */
    break;

  case 693:
#line 4488 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9506 "parser.c" /* yacc.c:1646  */
    break;

  case 694:
#line 4496 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTARGETTEAMSDISTPARFOR, $1, $2); TODO DCTARGETTEAMSDISTPARFOR
      //$$->l = $1->l;
    }
#line 9515 "parser.c" /* yacc.c:1646  */
    break;

  case 695:
#line 4505 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTARGETTEAMSDISTPARFOR, $7); TODO DCTARGETTEAMSDISTPARFOR
    }
#line 9523 "parser.c" /* yacc.c:1646  */
    break;

  case 696:
#line 4512 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 9531 "parser.c" /* yacc.c:1646  */
    break;

  case 697:
#line 4516 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 9539 "parser.c" /* yacc.c:1646  */
    break;

  case 698:
#line 4520 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 9547 "parser.c" /* yacc.c:1646  */
    break;

  case 699:
#line 4527 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9555 "parser.c" /* yacc.c:1646  */
    break;

  case 700:
#line 4531 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9563 "parser.c" /* yacc.c:1646  */
    break;

  case 701:
#line 4539 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTEAMSDISTPARFORSIMD, $1, $2); TODO DCTEAMSDISTPARFORSIMD
      //$$->l = $1->l;
    }
#line 9572 "parser.c" /* yacc.c:1646  */
    break;

  case 702:
#line 4548 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTEAMSDISTPARFORSIMD, $7); TODO DCTEAMSDISTPARFORSIMD
    }
#line 9580 "parser.c" /* yacc.c:1646  */
    break;

  case 703:
#line 4555 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 9588 "parser.c" /* yacc.c:1646  */
    break;

  case 704:
#line 4559 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 9596 "parser.c" /* yacc.c:1646  */
    break;

  case 705:
#line 4563 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 9604 "parser.c" /* yacc.c:1646  */
    break;

  case 706:
#line 4570 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9612 "parser.c" /* yacc.c:1646  */
    break;

  case 707:
#line 4574 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9620 "parser.c" /* yacc.c:1646  */
    break;

  case 708:
#line 4582 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpConstruct(DCTARGETTEAMSDISTPARFORSIMD, $1, $2); TODO DCTARGETTEAMSDISTPARFORSIMD
      //$$->l = $1->l;
    }
#line 9629 "parser.c" /* yacc.c:1646  */
    break;

  case 709:
#line 4591 "parser.y" /* yacc.c:1646  */
    {
      //$$ = OmpDirective(DCTARGETTEAMSDISTPARFORSIMD, $8); TODO DCTARGETTEAMSDISTPARFORSIMD
    }
#line 9637 "parser.c" /* yacc.c:1646  */
    break;

  case 710:
#line 4598 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 9645 "parser.c" /* yacc.c:1646  */
    break;

  case 711:
#line 4602 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 9653 "parser.c" /* yacc.c:1646  */
    break;

  case 712:
#line 4606 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 9661 "parser.c" /* yacc.c:1646  */
    break;

  case 713:
#line 4613 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9669 "parser.c" /* yacc.c:1646  */
    break;

  case 714:
#line 4617 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9677 "parser.c" /* yacc.c:1646  */
    break;

  case 715:
#line 4625 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTASK, (yyvsp[-1].odir), (yyvsp[0].stmt));
      (yyval.ocon)->l = (yyvsp[-1].odir)->l;
    }
#line 9686 "parser.c" /* yacc.c:1646  */
    break;

  case 716:
#line 4634 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTASK, (yyvsp[-1].ocla));
    }
#line 9694 "parser.c" /* yacc.c:1646  */
    break;

  case 717:
#line 4642 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 9702 "parser.c" /* yacc.c:1646  */
    break;

  case 718:
#line 4646 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 9710 "parser.c" /* yacc.c:1646  */
    break;

  case 719:
#line 4650 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 9718 "parser.c" /* yacc.c:1646  */
    break;

  case 720:
#line 4658 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9726 "parser.c" /* yacc.c:1646  */
    break;

  case 721:
#line 4662 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9734 "parser.c" /* yacc.c:1646  */
    break;

  case 722:
#line 4666 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9742 "parser.c" /* yacc.c:1646  */
    break;

  case 723:
#line 4670 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9750 "parser.c" /* yacc.c:1646  */
    break;

  case 724:
#line 4674 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9758 "parser.c" /* yacc.c:1646  */
    break;

  case 725:
#line 4678 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9766 "parser.c" /* yacc.c:1646  */
    break;

  case 726:
#line 4686 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9774 "parser.c" /* yacc.c:1646  */
    break;

  case 727:
#line 4689 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 9780 "parser.c" /* yacc.c:1646  */
    break;

  case 728:
#line 4690 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = FinalClause((yyvsp[-1].expr));
    }
#line 9789 "parser.c" /* yacc.c:1646  */
    break;

  case 729:
#line 4695 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCUNTIED);
    }
#line 9797 "parser.c" /* yacc.c:1646  */
    break;

  case 730:
#line 4699 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCMERGEABLE);
    }
#line 9805 "parser.c" /* yacc.c:1646  */
    break;

  case 731:
#line 4704 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PriorityClause((yyvsp[-1].expr));
    }
#line 9813 "parser.c" /* yacc.c:1646  */
    break;

  case 732:
#line 4711 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 9819 "parser.c" /* yacc.c:1646  */
    break;

  case 733:
#line 4712 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = DependClause((yyvsp[-4].type), (yyvsp[-1].oxli));
    }
#line 9828 "parser.c" /* yacc.c:1646  */
    break;

  case 734:
#line 4721 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_in;
    }
#line 9836 "parser.c" /* yacc.c:1646  */
    break;

  case 735:
#line 4725 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_out;
    }
#line 9844 "parser.c" /* yacc.c:1646  */
    break;

  case 736:
#line 4729 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_inout;
    }
#line 9852 "parser.c" /* yacc.c:1646  */
    break;

  case 737:
#line 4736 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCPARFOR, (yyvsp[-1].odir), (yyvsp[0].stmt));
      (yyval.ocon)->l = (yyvsp[-1].odir)->l;
    }
#line 9861 "parser.c" /* yacc.c:1646  */
    break;

  case 738:
#line 4744 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCPARFOR, (yyvsp[-1].ocla));
    }
#line 9869 "parser.c" /* yacc.c:1646  */
    break;

  case 739:
#line 4751 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 9877 "parser.c" /* yacc.c:1646  */
    break;

  case 740:
#line 4755 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 9885 "parser.c" /* yacc.c:1646  */
    break;

  case 741:
#line 4759 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 9893 "parser.c" /* yacc.c:1646  */
    break;

  case 742:
#line 4766 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9901 "parser.c" /* yacc.c:1646  */
    break;

  case 743:
#line 4770 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9909 "parser.c" /* yacc.c:1646  */
    break;

  case 744:
#line 4774 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9917 "parser.c" /* yacc.c:1646  */
    break;

  case 745:
#line 4778 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9925 "parser.c" /* yacc.c:1646  */
    break;

  case 746:
#line 4782 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9933 "parser.c" /* yacc.c:1646  */
    break;

  case 747:
#line 4786 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9941 "parser.c" /* yacc.c:1646  */
    break;

  case 748:
#line 4790 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9949 "parser.c" /* yacc.c:1646  */
    break;

  case 749:
#line 4794 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 9957 "parser.c" /* yacc.c:1646  */
    break;

  case 750:
#line 4801 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCPARSECTIONS, (yyvsp[-1].odir), (yyvsp[0].stmt));
      (yyval.ocon)->l = (yyvsp[-1].odir)->l;
    }
#line 9966 "parser.c" /* yacc.c:1646  */
    break;

  case 751:
#line 4809 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCPARSECTIONS, (yyvsp[-1].ocla));
    }
#line 9974 "parser.c" /* yacc.c:1646  */
    break;

  case 752:
#line 4816 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 9982 "parser.c" /* yacc.c:1646  */
    break;

  case 753:
#line 4820 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 9990 "parser.c" /* yacc.c:1646  */
    break;

  case 754:
#line 4824 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 9998 "parser.c" /* yacc.c:1646  */
    break;

  case 755:
#line 4831 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 10006 "parser.c" /* yacc.c:1646  */
    break;

  case 756:
#line 4835 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 10014 "parser.c" /* yacc.c:1646  */
    break;

  case 757:
#line 4839 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 10022 "parser.c" /* yacc.c:1646  */
    break;

  case 758:
#line 4843 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 10030 "parser.c" /* yacc.c:1646  */
    break;

  case 759:
#line 4847 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 10038 "parser.c" /* yacc.c:1646  */
    break;

  case 760:
#line 4851 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 10046 "parser.c" /* yacc.c:1646  */
    break;

  case 761:
#line 4855 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 10054 "parser.c" /* yacc.c:1646  */
    break;

  case 762:
#line 4862 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCMASTER, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 10062 "parser.c" /* yacc.c:1646  */
    break;

  case 763:
#line 4869 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCMASTER, NULL);
    }
#line 10070 "parser.c" /* yacc.c:1646  */
    break;

  case 764:
#line 4876 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCCRITICAL, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 10078 "parser.c" /* yacc.c:1646  */
    break;

  case 765:
#line 4883 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpCriticalDirective(NULL, NULL);
    }
#line 10086 "parser.c" /* yacc.c:1646  */
    break;

  case 766:
#line 4887 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpCriticalDirective((yyvsp[-1].symb), NULL);
    }
#line 10094 "parser.c" /* yacc.c:1646  */
    break;

  case 767:
#line 4892 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpCriticalDirective((yyvsp[-2].symb), (yyvsp[-1].ocla));
    }
#line 10102 "parser.c" /* yacc.c:1646  */
    break;

  case 768:
#line 4899 "parser.y" /* yacc.c:1646  */
    {
      (yyval.symb) = Symbol((yyvsp[-1].name));
    }
#line 10110 "parser.c" /* yacc.c:1646  */
    break;

  case 769:
#line 4907 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = HintClause((yyvsp[-1].expr));
    }
#line 10118 "parser.c" /* yacc.c:1646  */
    break;

  case 770:
#line 4914 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCBARRIER, NULL);
    }
#line 10126 "parser.c" /* yacc.c:1646  */
    break;

  case 771:
#line 4922 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTASKWAIT, NULL);
    }
#line 10134 "parser.c" /* yacc.c:1646  */
    break;

  case 772:
#line 4930 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCTASKGROUP, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 10142 "parser.c" /* yacc.c:1646  */
    break;

  case 773:
#line 4938 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTASKGROUP, NULL);
    }
#line 10150 "parser.c" /* yacc.c:1646  */
    break;

  case 774:
#line 4946 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCTASKYIELD, NULL);
    }
#line 10158 "parser.c" /* yacc.c:1646  */
    break;

  case 775:
#line 4953 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCATOMIC, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 10166 "parser.c" /* yacc.c:1646  */
    break;

  case 776:
#line 4964 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCATOMIC, NULL);  //TODO Check how to do it since it now has 2 clauses
    }
#line 10174 "parser.c" /* yacc.c:1646  */
    break;

  case 777:
#line 4971 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 10182 "parser.c" /* yacc.c:1646  */
    break;

  case 778:
#line 4975 "parser.y" /* yacc.c:1646  */
    {
      //$$ = TODO
    }
#line 10190 "parser.c" /* yacc.c:1646  */
    break;

  case 779:
#line 4979 "parser.y" /* yacc.c:1646  */
    {
      //$$ = TODO
    }
#line 10198 "parser.c" /* yacc.c:1646  */
    break;

  case 780:
#line 4983 "parser.y" /* yacc.c:1646  */
    {
      //$$ = TODO
    }
#line 10206 "parser.c" /* yacc.c:1646  */
    break;

  case 781:
#line 4987 "parser.y" /* yacc.c:1646  */
    {
      //$$ = TODO
    }
#line 10214 "parser.c" /* yacc.c:1646  */
    break;

  case 782:
#line 4995 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 10222 "parser.c" /* yacc.c:1646  */
    break;

  case 783:
#line 4999 "parser.y" /* yacc.c:1646  */
    {
      //$$ = TODO
    }
#line 10230 "parser.c" /* yacc.c:1646  */
    break;

  case 784:
#line 5006 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpFlushDirective(NULL);
    }
#line 10238 "parser.c" /* yacc.c:1646  */
    break;

  case 785:
#line 5010 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpFlushDirective((yyvsp[-1].decl));
    }
#line 10246 "parser.c" /* yacc.c:1646  */
    break;

  case 786:
#line 5016 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 10252 "parser.c" /* yacc.c:1646  */
    break;

  case 787:
#line 5017 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.decl) = (yyvsp[-1].decl);
    }
#line 10261 "parser.c" /* yacc.c:1646  */
    break;

  case 788:
#line 5025 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCORDERED, (yyvsp[-1].odir), (yyvsp[0].stmt));
    }
#line 10269 "parser.c" /* yacc.c:1646  */
    break;

  case 789:
#line 5029 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocon) = OmpConstruct(DCORDERED, (yyvsp[0].odir), NULL);
    }
#line 10277 "parser.c" /* yacc.c:1646  */
    break;

  case 790:
#line 5036 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCORDERED, (yyvsp[-1].ocla));
    }
#line 10285 "parser.c" /* yacc.c:1646  */
    break;

  case 791:
#line 5043 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCORDERED, DependClause(OC_source,NULL));
    }
#line 10293 "parser.c" /* yacc.c:1646  */
    break;

  case 792:
#line 5047 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCORDERED, (yyvsp[-1].ocla));
    }
#line 10301 "parser.c" /* yacc.c:1646  */
    break;

  case 793:
#line 5055 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 10309 "parser.c" /* yacc.c:1646  */
    break;

  case 794:
#line 5059 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 10317 "parser.c" /* yacc.c:1646  */
    break;

  case 795:
#line 5063 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 10325 "parser.c" /* yacc.c:1646  */
    break;

  case 796:
#line 5071 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCTHREADS);
    }
#line 10333 "parser.c" /* yacc.c:1646  */
    break;

  case 797:
#line 5075 "parser.y" /* yacc.c:1646  */
    {
      //TODO
    }
#line 10341 "parser.c" /* yacc.c:1646  */
    break;

  case 798:
#line 5083 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = (yyvsp[0].ocla);
    }
#line 10349 "parser.c" /* yacc.c:1646  */
    break;

  case 799:
#line 5087 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-1].ocla), (yyvsp[0].ocla));
    }
#line 10357 "parser.c" /* yacc.c:1646  */
    break;

  case 800:
#line 5091 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = OmpClauseList((yyvsp[-2].ocla), (yyvsp[0].ocla));
    }
#line 10365 "parser.c" /* yacc.c:1646  */
    break;

  case 801:
#line 5099 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = DependClause(OC_sink, NULL);
      (yyval.ocla)->u.expr = (yyvsp[-1].expr);
    }
#line 10374 "parser.c" /* yacc.c:1646  */
    break;

  case 802:
#line 5107 "parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
    }
#line 10382 "parser.c" /* yacc.c:1646  */
    break;

  case 803:
#line 5111 "parser.y" /* yacc.c:1646  */
    { 
      (yyval.expr) = CommaList((yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 10390 "parser.c" /* yacc.c:1646  */
    break;

  case 804:
#line 5118 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls)
        check_uknown_var((yyvsp[0].name));
      (yyval.expr) = BinaryOperator(BOP_add, IdentName((yyvsp[0].name)), numConstant(0));
    }
#line 10400 "parser.c" /* yacc.c:1646  */
    break;

  case 805:
#line 5124 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls)
        check_uknown_var((yyvsp[-2].name));
      (yyval.expr) = BinaryOperator(BOP_add, IdentName((yyvsp[-2].name)), (yyvsp[0].expr));
    }
#line 10410 "parser.c" /* yacc.c:1646  */
    break;

  case 806:
#line 5130 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls)
        check_uknown_var((yyvsp[-2].name));
      (yyval.expr) = BinaryOperator(BOP_sub, IdentName((yyvsp[-2].name)), (yyvsp[0].expr));
    }
#line 10420 "parser.c" /* yacc.c:1646  */
    break;

  case 807:
#line 5140 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCCANCEL, (yyvsp[-1].ocla));
    }
#line 10428 "parser.c" /* yacc.c:1646  */
    break;

  case 808:
#line 5144 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCCANCEL, OmpClauseList((yyvsp[-2].ocla), (yyvsp[-1].ocla)));
    }
#line 10436 "parser.c" /* yacc.c:1646  */
    break;

  case 809:
#line 5148 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCCANCEL, OmpClauseList((yyvsp[-3].ocla), (yyvsp[-1].ocla)));
    }
#line 10444 "parser.c" /* yacc.c:1646  */
    break;

  case 810:
#line 5155 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCPARALLEL);
    }
#line 10452 "parser.c" /* yacc.c:1646  */
    break;

  case 811:
#line 5159 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCSECTIONS);
    }
#line 10460 "parser.c" /* yacc.c:1646  */
    break;

  case 812:
#line 5163 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCFOR);
    }
#line 10468 "parser.c" /* yacc.c:1646  */
    break;

  case 813:
#line 5167 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = PlainClause(OCTASKGROUP);
    }
#line 10476 "parser.c" /* yacc.c:1646  */
    break;

  case 814:
#line 5175 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpDirective(DCCANCELLATIONPOINT, (yyvsp[-1].ocla));
    }
#line 10484 "parser.c" /* yacc.c:1646  */
    break;

  case 815:
#line 5181 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 10490 "parser.c" /* yacc.c:1646  */
    break;

  case 816:
#line 5181 "parser.y" /* yacc.c:1646  */
    { sc_start_openmp(); }
#line 10496 "parser.c" /* yacc.c:1646  */
    break;

  case 817:
#line 5182 "parser.y" /* yacc.c:1646  */
    {
      (yyval.odir) = OmpThreadprivateDirective((yyvsp[-3].decl));
    }
#line 10504 "parser.c" /* yacc.c:1646  */
    break;

  case 818:
#line 5189 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 10510 "parser.c" /* yacc.c:1646  */
    break;

  case 819:
#line 5189 "parser.y" /* yacc.c:1646  */
    { sc_start_openmp(); }
#line 10516 "parser.c" /* yacc.c:1646  */
    break;

  case 820:
#line 5190 "parser.y" /* yacc.c:1646  */
    {
      //$$ = TODO
    }
#line 10524 "parser.c" /* yacc.c:1646  */
    break;

  case 821:
#line 5197 "parser.y" /* yacc.c:1646  */
    {
      parse_error(1, "user-defined reductions are not implemented yet.\n");
      //$$ = OC_identifier TODO
      //Symbol($2);  TODO
    }
#line 10534 "parser.c" /* yacc.c:1646  */
    break;

  case 822:
#line 5204 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_plus;
    }
#line 10542 "parser.c" /* yacc.c:1646  */
    break;

  case 823:
#line 5208 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_times;
    }
#line 10550 "parser.c" /* yacc.c:1646  */
    break;

  case 824:
#line 5212 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_minus;
    }
#line 10558 "parser.c" /* yacc.c:1646  */
    break;

  case 825:
#line 5216 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_band;
    }
#line 10566 "parser.c" /* yacc.c:1646  */
    break;

  case 826:
#line 5220 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_xor;
    }
#line 10574 "parser.c" /* yacc.c:1646  */
    break;

  case 827:
#line 5224 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_bor;
    }
#line 10582 "parser.c" /* yacc.c:1646  */
    break;

  case 828:
#line 5228 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_land;
    }
#line 10590 "parser.c" /* yacc.c:1646  */
    break;

  case 829:
#line 5232 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_lor;
    }
#line 10598 "parser.c" /* yacc.c:1646  */
    break;

  case 830:
#line 5236 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_min;
    }
#line 10606 "parser.c" /* yacc.c:1646  */
    break;

  case 831:
#line 5240 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OC_max;
    }
#line 10614 "parser.c" /* yacc.c:1646  */
    break;

  case 832:
#line 5247 "parser.y" /* yacc.c:1646  */
    {
      //TODO
    }
#line 10622 "parser.c" /* yacc.c:1646  */
    break;

  case 833:
#line 5251 "parser.y" /* yacc.c:1646  */
    {
      //TODO
    }
#line 10630 "parser.c" /* yacc.c:1646  */
    break;

  case 834:
#line 5258 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = NULL;
    }
#line 10638 "parser.c" /* yacc.c:1646  */
    break;

  case 835:
#line 5262 "parser.y" /* yacc.c:1646  */
    {
        //TODO must check if identifier is omp_priv and that conditional
        //expression contains only omp_priv and omp_orig variables
    }
#line 10647 "parser.c" /* yacc.c:1646  */
    break;

  case 836:
#line 5267 "parser.y" /* yacc.c:1646  */
    {
      //TODO in argument_expression_list one of the variables must be &omp_priv
      // TODO check ox_funccall_expression
      //$$ = strcmp($1, "main") ?
      //       FunctionCall(IdentName($1), $3) :
      //       FunctionCall(IdentName(MAIN_NEWNAME), $3);
    }
#line 10659 "parser.c" /* yacc.c:1646  */
    break;

  case 837:
#line 5278 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = DefaultClause(OC_defshared);
    }
#line 10667 "parser.c" /* yacc.c:1646  */
    break;

  case 838:
#line 5282 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = DefaultClause(OC_defnone);
    }
#line 10675 "parser.c" /* yacc.c:1646  */
    break;

  case 839:
#line 5287 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = DefaultClause(OC_auto); //I'm using the existing subtype (Alexandros)
    }
#line 10683 "parser.c" /* yacc.c:1646  */
    break;

  case 840:
#line 5293 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 10689 "parser.c" /* yacc.c:1646  */
    break;

  case 841:
#line 5294 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCPRIVATE, (yyvsp[-1].decl));
    }
#line 10698 "parser.c" /* yacc.c:1646  */
    break;

  case 842:
#line 5301 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 10704 "parser.c" /* yacc.c:1646  */
    break;

  case 843:
#line 5302 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCFIRSTPRIVATE, (yyvsp[-1].decl));
    }
#line 10713 "parser.c" /* yacc.c:1646  */
    break;

  case 844:
#line 5309 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 10719 "parser.c" /* yacc.c:1646  */
    break;

  case 845:
#line 5310 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCLASTPRIVATE, (yyvsp[-1].decl));
    }
#line 10728 "parser.c" /* yacc.c:1646  */
    break;

  case 846:
#line 5317 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 10734 "parser.c" /* yacc.c:1646  */
    break;

  case 847:
#line 5318 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = VarlistClause(OCSHARED, (yyvsp[-1].decl));
    }
#line 10743 "parser.c" /* yacc.c:1646  */
    break;

  case 848:
#line 5325 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 10749 "parser.c" /* yacc.c:1646  */
    break;

  case 849:
#line 5327 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = ReductionClause((yyvsp[-4].type), (yyvsp[-1].oxli));
    }
#line 10758 "parser.c" /* yacc.c:1646  */
    break;

  case 850:
#line 5334 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 10764 "parser.c" /* yacc.c:1646  */
    break;

  case 851:
#line 5335 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = IfClause((yyvsp[-1].expr), OCM_none);
    }
#line 10773 "parser.c" /* yacc.c:1646  */
    break;

  case 852:
#line 5340 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 10779 "parser.c" /* yacc.c:1646  */
    break;

  case 853:
#line 5341 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.ocla) = IfClause((yyvsp[-1].expr), (yyvsp[-4].type));
    }
#line 10788 "parser.c" /* yacc.c:1646  */
    break;

  case 854:
#line 5348 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = OCM_parallel; }
#line 10794 "parser.c" /* yacc.c:1646  */
    break;

  case 855:
#line 5349 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = OCM_task; }
#line 10800 "parser.c" /* yacc.c:1646  */
    break;

  case 856:
#line 5350 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = OCM_target; }
#line 10806 "parser.c" /* yacc.c:1646  */
    break;

  case 857:
#line 5351 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = OCM_targetdata; }
#line 10812 "parser.c" /* yacc.c:1646  */
    break;

  case 858:
#line 5352 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = OCM_targetenterdata; }
#line 10818 "parser.c" /* yacc.c:1646  */
    break;

  case 859:
#line 5353 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = OCM_targetexitdata; }
#line 10824 "parser.c" /* yacc.c:1646  */
    break;

  case 860:
#line 5354 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = OCM_targetupdate; }
#line 10830 "parser.c" /* yacc.c:1646  */
    break;

  case 861:
#line 5355 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = OCM_cancel; }
#line 10836 "parser.c" /* yacc.c:1646  */
    break;

  case 862:
#line 5360 "parser.y" /* yacc.c:1646  */
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
#line 10852 "parser.c" /* yacc.c:1646  */
    break;

  case 863:
#line 5375 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls)
        if (symtab_get(stab, Symbol((yyvsp[0].name)), IDNAME) == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[0].name));
      (yyval.decl) = IdentifierDecl( Symbol((yyvsp[0].name)) );
    }
#line 10863 "parser.c" /* yacc.c:1646  */
    break;

  case 864:
#line 5382 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls)
        if (symtab_get(stab, Symbol((yyvsp[0].name)), IDNAME) == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[0].name));
      (yyval.decl) = IdList((yyvsp[-2].decl), IdentifierDecl( Symbol((yyvsp[0].name)) ));
    }
#line 10874 "parser.c" /* yacc.c:1646  */
    break;

  case 865:
#line 5392 "parser.y" /* yacc.c:1646  */
    {
      (yyval.oxli) = (yyvsp[0].oxli);
    }
#line 10882 "parser.c" /* yacc.c:1646  */
    break;

  case 866:
#line 5396 "parser.y" /* yacc.c:1646  */
    {
      ompxli l = (yyvsp[-2].oxli);
      
      for (; l->next; l = l->next) ;  /* Till the end */
      l->next = (yyvsp[0].oxli);
      (yyval.oxli) = (yyvsp[-2].oxli);
    }
#line 10894 "parser.c" /* yacc.c:1646  */
    break;

  case 867:
#line 5407 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls)
        if (symtab_get(stab, Symbol((yyvsp[0].name)), IDNAME) == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[0].name));
      (yyval.oxli) = PlainXLI( Symbol((yyvsp[0].name)) );
    }
#line 10905 "parser.c" /* yacc.c:1646  */
    break;

  case 868:
#line 5414 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls)
        if (symtab_get(stab, Symbol((yyvsp[-1].name)), IDNAME) == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[-1].name));
      (yyval.oxli) = ArraySection( Symbol((yyvsp[-1].name)), (yyvsp[0].oasd) );
    }
#line 10916 "parser.c" /* yacc.c:1646  */
    break;

  case 869:
#line 5428 "parser.y" /* yacc.c:1646  */
    {
      (yyval.oxli) = (yyvsp[0].oxli);
    }
#line 10924 "parser.c" /* yacc.c:1646  */
    break;

  case 870:
#line 5432 "parser.y" /* yacc.c:1646  */
    {
      ompxli l = (yyvsp[-2].oxli);
      
      for (; l->next; l = l->next) ;  /* Till the end */
      l->next = (yyvsp[0].oxli);
      (yyval.oxli) = (yyvsp[-2].oxli);
    }
#line 10936 "parser.c" /* yacc.c:1646  */
    break;

  case 871:
#line 5443 "parser.y" /* yacc.c:1646  */
    {
      /* No check for known identifiers since constuct can be anywhere */
      (yyval.oxli) = PlainXLI( Symbol((yyvsp[0].name)) );
    }
#line 10945 "parser.c" /* yacc.c:1646  */
    break;

  case 872:
#line 5448 "parser.y" /* yacc.c:1646  */
    {
      /* No check for known identifiers since constuct can be anywhere */
      (yyval.oxli) = ArraySection( Symbol((yyvsp[-1].name)), (yyvsp[0].oasd) );
    }
#line 10954 "parser.c" /* yacc.c:1646  */
    break;

  case 873:
#line 5455 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 10960 "parser.c" /* yacc.c:1646  */
    break;

  case 874:
#line 5456 "parser.y" /* yacc.c:1646  */
    {
      omparrdim d = (yyvsp[-4].oasd);
      
      sc_start_openmp();
      for (; d->next; d = d->next) ;  /* Till the end */
      d->next = (yyvsp[-1].oasd);
      (yyval.oasd) = (yyvsp[-4].oasd);
    }
#line 10973 "parser.c" /* yacc.c:1646  */
    break;

  case 875:
#line 5464 "parser.y" /* yacc.c:1646  */
    { sc_pause_openmp(); }
#line 10979 "parser.c" /* yacc.c:1646  */
    break;

  case 876:
#line 5465 "parser.y" /* yacc.c:1646  */
    {
      sc_start_openmp();
      (yyval.oasd) = (yyvsp[-1].oasd);
    }
#line 10988 "parser.c" /* yacc.c:1646  */
    break;

  case 877:
#line 5473 "parser.y" /* yacc.c:1646  */
    {
      (yyval.oasd) = OmpArrDim((yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 10996 "parser.c" /* yacc.c:1646  */
    break;

  case 878:
#line 5477 "parser.y" /* yacc.c:1646  */
    {
      (yyval.oasd) = OmpArrDim((yyvsp[-1].expr), NULL);
    }
#line 11004 "parser.c" /* yacc.c:1646  */
    break;

  case 879:
#line 5481 "parser.y" /* yacc.c:1646  */
    {
      (yyval.oasd) = OmpArrDim((yyvsp[0].expr), numConstant(1));
    }
#line 11012 "parser.c" /* yacc.c:1646  */
    break;

  case 880:
#line 5485 "parser.y" /* yacc.c:1646  */
    {
      (yyval.oasd) = OmpArrDim(numConstant(0), (yyvsp[0].expr));
    }
#line 11020 "parser.c" /* yacc.c:1646  */
    break;

  case 881:
#line 5489 "parser.y" /* yacc.c:1646  */
    {
      (yyval.oasd) = OmpArrDim(numConstant(0), NULL);
    }
#line 11028 "parser.c" /* yacc.c:1646  */
    break;

  case 882:
#line 5497 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = ProcBindClause(OC_bindmaster);
    }
#line 11036 "parser.c" /* yacc.c:1646  */
    break;

  case 883:
#line 5501 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = ProcBindClause(OC_bindprimary);
    }
#line 11044 "parser.c" /* yacc.c:1646  */
    break;

  case 884:
#line 5505 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = ProcBindClause(OC_bindclose);
    }
#line 11052 "parser.c" /* yacc.c:1646  */
    break;

  case 885:
#line 5509 "parser.y" /* yacc.c:1646  */
    {
      (yyval.ocla) = ProcBindClause(OC_bindspread);
    }
#line 11060 "parser.c" /* yacc.c:1646  */
    break;

  case 886:
#line 5521 "parser.y" /* yacc.c:1646  */
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
#line 11082 "parser.c" /* yacc.c:1646  */
    break;

  case 887:
#line 5539 "parser.y" /* yacc.c:1646  */
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
#line 11104 "parser.c" /* yacc.c:1646  */
    break;

  case 888:
#line 5572 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcon) = OmpixConstruct(OX_DCTASKSYNC, (yyvsp[0].xdir), NULL);
    }
#line 11112 "parser.c" /* yacc.c:1646  */
    break;

  case 889:
#line 5576 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcon) = OmpixConstruct(OX_DCTASKSCHEDULE, (yyvsp[0].xdir), NULL);
    }
#line 11120 "parser.c" /* yacc.c:1646  */
    break;

  case 890:
#line 5584 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xdir) = OmpixDirective(OX_DCTASKSYNC, NULL);
    }
#line 11128 "parser.c" /* yacc.c:1646  */
    break;

  case 891:
#line 5591 "parser.y" /* yacc.c:1646  */
    {
      scope_start(stab);
    }
#line 11136 "parser.c" /* yacc.c:1646  */
    break;

  case 892:
#line 5595 "parser.y" /* yacc.c:1646  */
    {
      scope_end(stab);
      (yyval.xdir) = OmpixDirective(OX_DCTASKSCHEDULE, (yyvsp[-1].xcla));
    }
#line 11145 "parser.c" /* yacc.c:1646  */
    break;

  case 893:
#line 5603 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = NULL;
    }
#line 11153 "parser.c" /* yacc.c:1646  */
    break;

  case 894:
#line 5607 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixClauseList((yyvsp[-1].xcla), (yyvsp[0].xcla));
    }
#line 11161 "parser.c" /* yacc.c:1646  */
    break;

  case 895:
#line 5611 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixClauseList((yyvsp[-2].xcla), (yyvsp[0].xcla));
    }
#line 11169 "parser.c" /* yacc.c:1646  */
    break;

  case 896:
#line 5618 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixStrideClause((yyvsp[-1].expr));
    }
#line 11177 "parser.c" /* yacc.c:1646  */
    break;

  case 897:
#line 5622 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixStartClause((yyvsp[-1].expr));
    }
#line 11185 "parser.c" /* yacc.c:1646  */
    break;

  case 898:
#line 5626 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixScopeClause((yyvsp[-1].type));
    }
#line 11193 "parser.c" /* yacc.c:1646  */
    break;

  case 899:
#line 5630 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixPlainClause(OX_OCTIED);
    }
#line 11201 "parser.c" /* yacc.c:1646  */
    break;

  case 900:
#line 5634 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixPlainClause(OX_OCUNTIED);
    }
#line 11209 "parser.c" /* yacc.c:1646  */
    break;

  case 901:
#line 5641 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OX_SCOPE_NODES;
    }
#line 11217 "parser.c" /* yacc.c:1646  */
    break;

  case 902:
#line 5645 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OX_SCOPE_WGLOBAL;
    }
#line 11225 "parser.c" /* yacc.c:1646  */
    break;

  case 903:
#line 5649 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OX_SCOPE_WGLOBAL;
    }
#line 11233 "parser.c" /* yacc.c:1646  */
    break;

  case 904:
#line 5653 "parser.y" /* yacc.c:1646  */
    {
      (yyval.type) = OX_SCOPE_WLOCAL;
    }
#line 11241 "parser.c" /* yacc.c:1646  */
    break;

  case 905:
#line 5660 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcon) = (yyvsp[0].xcon);
    }
#line 11249 "parser.c" /* yacc.c:1646  */
    break;

  case 906:
#line 5664 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcon) = (yyvsp[0].xcon);
    }
#line 11257 "parser.c" /* yacc.c:1646  */
    break;

  case 907:
#line 5672 "parser.y" /* yacc.c:1646  */
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
#line 11272 "parser.c" /* yacc.c:1646  */
    break;

  case 908:
#line 5683 "parser.y" /* yacc.c:1646  */
    {
      scope_end(stab);
      (yyval.xcon) = OmpixTaskdef((yyvsp[-3].xdir), (yyvsp[-2].stmt), (yyvsp[0].stmt));
      (yyval.xcon)->l = (yyvsp[-3].xdir)->l;
    }
#line 11282 "parser.c" /* yacc.c:1646  */
    break;

  case 909:
#line 5689 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcon) = OmpixTaskdef((yyvsp[-1].xdir), (yyvsp[0].stmt), NULL);
      (yyval.xcon)->l = (yyvsp[-1].xdir)->l;
    }
#line 11291 "parser.c" /* yacc.c:1646  */
    break;

  case 910:
#line 5697 "parser.y" /* yacc.c:1646  */
    {
      scope_start(stab);
    }
#line 11299 "parser.c" /* yacc.c:1646  */
    break;

  case 911:
#line 5701 "parser.y" /* yacc.c:1646  */
    {
      scope_end(stab);
      (yyval.xdir) = OmpixDirective(OX_DCTASKDEF, (yyvsp[-1].xcla));
    }
#line 11308 "parser.c" /* yacc.c:1646  */
    break;

  case 912:
#line 5709 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = NULL;
    }
#line 11316 "parser.c" /* yacc.c:1646  */
    break;

  case 913:
#line 5713 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixClauseList((yyvsp[-1].xcla), (yyvsp[0].xcla));
    }
#line 11324 "parser.c" /* yacc.c:1646  */
    break;

  case 914:
#line 5717 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixClauseList((yyvsp[-2].xcla), (yyvsp[0].xcla));
    }
#line 11332 "parser.c" /* yacc.c:1646  */
    break;

  case 915:
#line 5724 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixVarlistClause(OX_OCIN, (yyvsp[-1].decl));
    }
#line 11340 "parser.c" /* yacc.c:1646  */
    break;

  case 916:
#line 5728 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixVarlistClause(OX_OCOUT, (yyvsp[-1].decl));
    }
#line 11348 "parser.c" /* yacc.c:1646  */
    break;

  case 917:
#line 5732 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixVarlistClause(OX_OCINOUT, (yyvsp[-1].decl));
    }
#line 11356 "parser.c" /* yacc.c:1646  */
    break;

  case 918:
#line 5736 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixReductionClause((yyvsp[-3].type), (yyvsp[-1].decl));
    }
#line 11364 "parser.c" /* yacc.c:1646  */
    break;

  case 919:
#line 5743 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = (yyvsp[0].decl);
    }
#line 11372 "parser.c" /* yacc.c:1646  */
    break;

  case 920:
#line 5747 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = IdList((yyvsp[-2].decl), (yyvsp[0].decl));
    }
#line 11380 "parser.c" /* yacc.c:1646  */
    break;

  case 921:
#line 5754 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = IdentifierDecl( Symbol((yyvsp[0].name)) );
      symtab_put(stab, Symbol((yyvsp[0].name)), IDNAME);
    }
#line 11389 "parser.c" /* yacc.c:1646  */
    break;

  case 922:
#line 5759 "parser.y" /* yacc.c:1646  */
    {
      if (checkDecls) check_uknown_var((yyvsp[-1].name));
      /* Use extern to differentiate */
      (yyval.decl) = ArrayDecl(IdentifierDecl( Symbol((yyvsp[-4].name)) ), StClassSpec(SPEC_extern),
                     IdentName((yyvsp[-1].name)));
      symtab_put(stab, Symbol((yyvsp[-4].name)), IDNAME);
    }
#line 11401 "parser.c" /* yacc.c:1646  */
    break;

  case 923:
#line 5767 "parser.y" /* yacc.c:1646  */
    {
      (yyval.decl) = ArrayDecl(IdentifierDecl( Symbol((yyvsp[-3].name)) ), NULL, (yyvsp[-1].expr));
      symtab_put(stab, Symbol((yyvsp[-3].name)), IDNAME);
    }
#line 11410 "parser.c" /* yacc.c:1646  */
    break;

  case 924:
#line 5775 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcon) = OmpixConstruct(OX_DCTASK, (yyvsp[-5].xdir), 
             FuncCallStmt(IdentName(strcmp((yyvsp[-4].name),"main") ? (yyvsp[-4].name) : MAIN_NEWNAME),(yyvsp[-2].expr)));
      (yyval.xcon)->l = (yyvsp[-5].xdir)->l;
    }
#line 11420 "parser.c" /* yacc.c:1646  */
    break;

  case 925:
#line 5784 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xdir) = OmpixDirective(OX_DCTASK, (yyvsp[-1].xcla));
    }
#line 11428 "parser.c" /* yacc.c:1646  */
    break;

  case 926:
#line 5791 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = NULL;
    }
#line 11436 "parser.c" /* yacc.c:1646  */
    break;

  case 927:
#line 5795 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixClauseList((yyvsp[-1].xcla), (yyvsp[0].xcla));
    }
#line 11444 "parser.c" /* yacc.c:1646  */
    break;

  case 928:
#line 5799 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixClauseList((yyvsp[-2].xcla), (yyvsp[0].xcla));
    }
#line 11452 "parser.c" /* yacc.c:1646  */
    break;

  case 929:
#line 5806 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixPlainClause(OX_OCATALL);
    }
#line 11460 "parser.c" /* yacc.c:1646  */
    break;

  case 930:
#line 5810 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixAtnodeClause((yyvsp[-1].expr));
    }
#line 11468 "parser.c" /* yacc.c:1646  */
    break;

  case 931:
#line 5814 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixPlainClause(OX_OCLOCAL);
    }
#line 11476 "parser.c" /* yacc.c:1646  */
    break;

  case 932:
#line 5818 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixPlainClause(OX_OCREMOTE);
    }
#line 11484 "parser.c" /* yacc.c:1646  */
    break;

  case 933:
#line 5822 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixAtworkerClause((yyvsp[-1].expr));
    }
#line 11492 "parser.c" /* yacc.c:1646  */
    break;

  case 934:
#line 5826 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixPlainClause(OX_OCTIED);
    }
#line 11500 "parser.c" /* yacc.c:1646  */
    break;

  case 935:
#line 5830 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixPlainClause(OX_OCUNTIED);
    }
#line 11508 "parser.c" /* yacc.c:1646  */
    break;

  case 936:
#line 5834 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixPlainClause(OX_OCDETACHED);
    }
#line 11516 "parser.c" /* yacc.c:1646  */
    break;

  case 937:
#line 5838 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixHintsClause((yyvsp[-1].expr));
    }
#line 11524 "parser.c" /* yacc.c:1646  */
    break;

  case 938:
#line 5842 "parser.y" /* yacc.c:1646  */
    {
      (yyval.xcla) = OmpixIfClause((yyvsp[-1].expr));
    }
#line 11532 "parser.c" /* yacc.c:1646  */
    break;


#line 11536 "parser.c" /* yacc.c:1646  */
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
#line 5848 "parser.y" /* yacc.c:1906  */



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


/* Utility function 
 */
char *strdupcat(char *first, char *second, int freethem)
{
	char *res;
	
	if (first == NULL && second == NULL)
		return NULL;
	if (first == NULL) 
		return (freethem) ? second : strdup(second);
	if (second == NULL) 
		return (freethem) ? first : strdup(first);
	if ((res = malloc(strlen(first)+strlen(second)+1)) == NULL)
		parse_error(1, "strdupcat ran out of memory\n");
	sprintf(res, "%s%s", first, second);
	if (freethem)
	{
		free(first);
		free(second);
	}
	return res;
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


void  check_schedule(ompclsubt_e sched, ompclmod_e mod)
{
	if (mod == OCM_none) return;
	if (mod == OCM_nonmonotonic && sched != OC_dynamic && sched != OC_guided)
		parse_error(1, "nonmonotonic modifier is only allowed in dynamic or "
		                 "guided schedules\n");
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


#define PARSE_STRING_SIZE 16384


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
