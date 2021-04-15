/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

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
#line 85 "parser.y" /* yacc.c:1909  */

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

#line 461 "parser.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_H_INCLUDED  */
