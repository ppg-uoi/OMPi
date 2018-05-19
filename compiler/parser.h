/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

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
#line 80 "parser.y" /* yacc.c:1909  */

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

#line 414 "parser.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_H_INCLUDED  */
