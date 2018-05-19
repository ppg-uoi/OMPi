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


D     [0-9]
L     [a-zA-Z_]
H     [a-fA-F0-9]
E     [Ee][+-]?{D}+
FS    (f|F|l|L)
IS    (u|U|l|L)*

%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ompi.h"
#include "ast.h"            /* For the yylval / ast types */
#include "symtab.h"
#include "parser.h"
#include "scanner.h"

#define SCANNER_STR_SIZES 1024  /* All strings declared here are that long */

static int  on_omp_line = 0,    /* Scanning an OpenMP #pragma line */
            on_ompix_line = 0;  /* Scanning an OMPi-extension #pragma line */
int         __has_omp = 0,      /* True if we found at least 1 OpenMP #pragma */
            __has_ompix = 0,
            __has_affinitysched = 0;

void count(), sharp(), gobbleline(), gobblecomment();
int  checkomp(int type, int return_if_true);

#define IdentOrType() \
  ( symtab_get(stab,Symbol(yytext),TYPENAME) != NULL ? TYPE_NAME : IDENTIFIER )

/* The following variables are used for tracking the original filenames and
 * original line numbers, because our scanner/parser uses preprocessed
 * files. To do this, we parse the sharps produced by the preprocessor,
 * which mark the line number of the original file, in the of the form:
 *   # 1 "test.c"
 */
static char origfile_name[SCANNER_STR_SIZES];  // original file name
static char thisfile_name[SCANNER_STR_SIZES];  // the scanned file name
static int  thisfile_line = 1, // Current line in our file (preprocessed)
            marker_line = 0,   // The line where the last marker was found
            origfile_line = 0; // Original file line the marker was refering to
static int  thisfile_column = 0;  // Column in the currently scanned line
static int  start_token = 0;   // For starting with a particular token

%}

%%

%{
     /* Trick to get an initial token (from the bison manual);
      * This is placed in the top of the produced yylex() function.
      */
     if (start_token)
     {
       int t = start_token;
       start_token = 0;         /* Don't do it again */
       return t;
     }
%}


"//"                   { if (!on_omp_line) gobbleline(); }
"/*"                   { if (!on_omp_line) gobblecomment(); }

  /*
   * OMPi-extension tokens
   */

[ \t]*"#"[ \t]*"pragma"[ \t]+"ompix"[ \t]+  {
                         count();
                         on_ompix_line = __has_ompix = 1;
                         return (PRAGMA_OMPIX);
                       }
  /*
   * OpenMP tokens
   */

[ \t]*"#"[ \t]*"pragma"[ \t]+"omp"[ \t]+  {
                         count();
                         on_omp_line = __has_omp = 1;
                         return (PRAGMA_OMP);
                       }
[ \t]*"#"[ \t]*"pragma"[ \t]+"omp"[ \t]+"threadprivate"[ \t]*  {
                         count();
                         on_omp_line = __has_omp = 1;
                         return (PRAGMA_OMP_THREADPRIVATE);
                       }
[ \t]*"#"[ \t]*"pragma"[ \t]+"omp"[ \t]+"cancellation"[ \t]+"point"[ \t]*  {
                         count();
                         on_omp_line = __has_omp = 1;
                         return (PRAGMA_OMP_CANCELLATIONPOINT);
                       }
[ \t]*"#"[ \t]*"line"  {
                         sharp();
                       }
[ \t]*"#"              {
                         sharp();
                       }
"parallel"             { return checkomp(0, OMP_PARALLEL); }
"sections"             { return checkomp(0, OMP_SECTIONS); }
"nowait"               { return checkomp(0, OMP_NOWAIT); }
"ordered"              { return checkomp(0, OMP_ORDERED); }
"schedule"             { return checkomp(0, OMP_SCHEDULE); }
"dynamic"              { return checkomp(0, OMP_DYNAMIC); }
"guided"               { return checkomp(0, OMP_GUIDED); }
"runtime"              { return checkomp(0, OMP_RUNTIME); }
"auto"                 {
                         count();
                         if (on_omp_line)
                           return(OMP_AUTO);
                         else
                           return(AUTO);
                       }
"affinity"             { /* non-OpenMP schedule */
                         count();
                         if (on_omp_line)
                         {
                           __has_affinitysched = 1;
                           return(OMP_AFFINITY);
                         }
                         else
                           return IdentOrType();
                       }
"section"              { return checkomp(0, OMP_SECTION); }
"single"               { return checkomp(0, OMP_SINGLE); }
"master"               { return checkomp(0, OMP_MASTER); }
"critical"             { return checkomp(0, OMP_CRITICAL); }
"barrier"              { return checkomp(0, OMP_BARRIER); }
"atomic"               { return checkomp(0, OMP_ATOMIC); }
"flush"                { return checkomp(0, OMP_FLUSH); }
"private"              { return checkomp(0, OMP_PRIVATE); }
"firstprivate"         { return checkomp(0, OMP_FIRSTPRIVATE); }
"lastprivate"          { return checkomp(0, OMP_LASTPRIVATE); }
"shared"               { return checkomp(0, OMP_SHARED); }
"none"                 { return checkomp(0, OMP_NONE); }
"reduction"            { return checkomp(2, OMP_REDUCTION); }
"copyin"               { return checkomp(0, OMP_COPYIN); }
"num_threads"          { return checkomp(0, OMP_NUMTHREADS); }
"copyprivate"          { return checkomp(0, OMP_COPYPRIVATE); }

   /* OpenMP 3.0 */

"task"                 { return checkomp(2, OMP_TASK); }
"untied"               { return checkomp(2, OMP_UNTIED); }
"taskwait"             { return checkomp(0, OMP_TASKWAIT); }
"collapse"             { return checkomp(0, OMP_COLLAPSE); }

   /* OpenMP 3.1 */

"final"                { return checkomp(0, OMP_FINAL); }
"mergeable"            { return checkomp(0, OMP_MERGEABLE); }
"taskyield"            { return checkomp(0, OMP_TASKYIELD); }
"read"                 { return checkomp(0, OMP_READ); }
"write"                { return checkomp(0, OMP_WRITE); }
"capture"              { return checkomp(0, OMP_CAPTURE); }
"update"               { return checkomp(0, OMP_UPDATE); }
"min"                  { return checkomp(0, OMP_MIN); }
"max"                  { return checkomp(0, OMP_MAX); }

   /* OpenMP 4.0 */

"proc_bind"            { return checkomp(0, OMP_PROCBIND); }
"close"                { return checkomp(0, OMP_CLOSE); }
"spread"               { return checkomp(0, OMP_SPREAD); }
"simd"                 { return checkomp(0, OMP_SIMD); }
"safelen"              { return checkomp(0, OMP_SAFELEN); }
"simdlen"              { return checkomp(0, OMP_SIMDLEN); }
"declare"              { return checkomp(0, OMP_DECLARE); }
"inbranch"             { return checkomp(0, OMP_INBRANCH); }
"notinbranch"          { return checkomp(0, OMP_NOTINBRANCH); }
"uniform"              { return checkomp(0, OMP_UNIFORM); }
"linear"               { return checkomp(0, OMP_LINEAR); }
"aligned"              { return checkomp(0, OMP_ALIGNED); }
"target"               { return checkomp(0, OMP_TARGET); }
"data"                 { return checkomp(0, OMP_DATA); }
"device"               { return checkomp(0, OMP_DEVICE); }
"map"                  { return checkomp(0, OMP_MAP); }
"alloc"                { return checkomp(0, OMP_ALLOC); }
"to"                   { return checkomp(0, OMP_TO); }
"from"                 { return checkomp(0, OMP_FROM); }
"tofrom"               { return checkomp(0, OMP_TOFROM); }
"end"                  { return checkomp(0, OMP_END); }
"teams"                { return checkomp(0, OMP_TEAMS); }
"distribute"           { return checkomp(0, OMP_DISTRIBUTE); }
"num_teams"            { return checkomp(0, OMP_NUMTEAMS); }
"thread_limit"         { return checkomp(0, OMP_THREADLIMIT); }
"dist_schedule"        { return checkomp(0, OMP_DISTSCHEDULE); }
"depend"               { return checkomp(0, OMP_DEPEND); }
"in"                   { return checkomp(0, OMP_IN); }
"out"                  { return checkomp(0, OMP_OUT); }
"inout"                { return checkomp(0, OMP_INOUT); }
"taskgroup"            { return checkomp(0, OMP_TASKGROUP); }
"seq_cst"              { return checkomp(0, OMP_SEQ_CST); }
"cancel"               { return checkomp(0, OMP_CANCEL); }
"initializer"          { return checkomp(0, OMP_INITIALIZER); }

  /*
   * OMPi-extension tokens
   */

"taskdef"              { return checkomp(1, OMPIX_TASKDEF); }
"uponreturn"           { return checkomp(1, OMPIX_UPONRETURN); }
"tasksync"             { return checkomp(1, OMPIX_TASKSYNC); }
"in"|"IN"|"In"         { return checkomp(1, OMPIX_IN); }
"out"|"OUT"|"Out"      { return checkomp(1, OMPIX_OUT); }
"inout"|"INOUT"|"Inout"|"InOut" {
                         return checkomp(1, OMPIX_INOUT);
                       }
"atnode"|"ATNODE"|"Atnode"|"AtNode" {
                         return checkomp(1, OMPIX_ATNODE);
                       }
"atworker"|"ATWORKER"|"Atworker"|"AtWorker" {
                         return checkomp(1, OMPIX_ATWORKER);
                       }
"taskschedule"         { return checkomp(1, OMPIX_TASKSCHEDULE); }
"stride"               { return checkomp(1, OMPIX_STRIDE); }
"start"                { return checkomp(1, OMPIX_START); }
"scope"                { return checkomp(1, OMPIX_SCOPE); }
"nodes"                { return checkomp(1, OMPIX_NODES); }
"workers"              { return checkomp(1, OMPIX_WORKERS); }
"local"                { return checkomp(1, OMPIX_LOCAL); }
"global"               { return checkomp(1, OMPIX_GLOBAL); }
"tied"                 { return checkomp(1, OMPIX_TIED); }
"detached"|"DETACHED"|"Detached" {
                         return checkomp(1, OMPIX_DETACHED);
                       }

  /*
   * C tokens

"auto"                 { count(); return(AUTO); }

   */
"break"                { count(); return(BREAK); }
"case"                 { count(); return(CASE); }
"char"                 { count(); return(CHAR); }
"const"                { count(); return(CONST); }
"continue"             { count(); return(CONTINUE); }
"default"              {
                         count();
                         if (on_omp_line)
                           return(OMP_DEFAULT);
                         else
                           return (DEFAULT);
                       }
"do"                   { count(); return(DO); }
"double"               { count(); return(DOUBLE); }
"else"                 { count(); return(ELSE); }
"enum"                 { count(); return(ENUM); }
"extern"               { count(); return(EXTERN); }
"float"                { count(); return(FLOAT); }
"for"                  {
                         count();
                         if (on_omp_line)
                           return (OMP_FOR);
                         else
                           return(FOR);
                       }
"goto"                 { count(); return(GOTO); }
"if"                   {
                         count();
                         if (on_omp_line)
                           return(OMP_IF);
                         else
                           return(IF);
                       }
"int"                  { count(); return(INT); }
"long"                 { count(); return(LONG); }
"register"             { count(); return(REGISTER); }
"return"               { count(); return(RETURN); }
"short"                { count(); return(SHORT); }
"signed"               { count(); return(SIGNED); }
"sizeof"               { count(); return(SIZEOF); }
"static"               {
                         count();
                         if (on_omp_line)
                           return(OMP_STATIC);
                         else
                           return (STATIC);
                       }
"struct"               { count(); return(STRUCT); }
"switch"               { count(); return(SWITCH); }
"typedef"              { count(); return(TYPEDEF); }
"union"                { count(); return(UNION); }
"unsigned"             { count(); return(UNSIGNED); }
"_Bool"                { count(); return(UBOOL); }
"_Complex"             { count(); return(UCOMPLEX); }
"_Imaginary"           { count(); return(UIMAGINARY); }
"void"                 { count(); return(VOID); }
"restrict"             { count(); return(RESTRICT); }
"volatile"             { count(); return(VOLATILE); }
"inline"               { count(); return(INLINE); }
"while"                { count(); return(WHILE); }

  /*
   * Hacks
   */
"__builtin_va_arg"     { count(); return(__BUILTIN_VA_ARG); }
"__builtin_offsetof"   { count(); return(__BUILTIN_OFFSETOF); }
"__builtin_types_compatible_p" { count();return(__BUILTIN_TYPES_COMPATIBLE_P); }
"__attribute__"        { count(); return(__ATTRIBUTE__); }

  /*
   * Identifiers, constants and strings
   */

{L}({L}|{D})*          { count(); return( IdentOrType() ); }
0[xX]{H}+{IS}?         { count(); return(CONSTANT); }
0{D}+{IS}?             { count(); return(CONSTANT); }
{D}+{IS}?              { count(); return(CONSTANT); }
'(\\.|[^\\'])+'        { count(); return(CONSTANT); }

{D}+{E}{FS}?           { count(); return(CONSTANT); }
{D}*"."{D}+({E})?{FS}? { count(); return(CONSTANT); }
{D}+"."{D}*({E})?{FS}? { count(); return(CONSTANT); }
\"(\\.|[^\\"])*\"      { count(); return(STRING_LITERAL); }

  /*
   * Operators
   */

">>="                  { count(); return(RIGHT_ASSIGN); }
"<<="                  { count(); return(LEFT_ASSIGN); }
"+="                   { count(); return(ADD_ASSIGN); }
"-="                   { count(); return(SUB_ASSIGN); }
"*="                   { count(); return(MUL_ASSIGN); }
"/="                   { count(); return(DIV_ASSIGN); }
"%="                   { count(); return(MOD_ASSIGN); }
"&="                   { count(); return(AND_ASSIGN); }
"^="                   { count(); return(XOR_ASSIGN); }
"|="                   { count(); return(OR_ASSIGN); }
">>"                   { count(); return(RIGHT_OP); }
"<<"                   { count(); return(LEFT_OP); }
"++"                   { count(); return(INC_OP); }
"--"                   { count(); return(DEC_OP); }
"->"                   { count(); return(PTR_OP); }
"&&"                   { count(); return(AND_OP); }
"||"                   { count(); return(OR_OP); }
"<="                   { count(); return(LE_OP); }
">="                   { count(); return(GE_OP); }
"=="                   { count(); return(EQ_OP); }
"!="                   { count(); return(NE_OP); }
";"                    { count(); return(';'); }
"{"                    { count(); return('{'); }
"}"                    { count(); return('}'); }
","                    { count(); return(','); }
":"                    { count(); return(':'); }
"="                    { count(); return('='); }
"("                    { count(); return('('); }
")"                    { count(); return(')'); }
"["                    { count(); return('['); }
"]"                    { count(); return(']'); }
"."                    { count(); return('.'); }
"&"                    { count(); return('&'); }
"!"                    { count(); return('!'); }
"~"                    { count(); return('~'); }
"-"                    { count(); return('-'); }
"+"                    { count(); return('+'); }
"*"                    { count(); return('*'); }
"/"                    { count(); return('/'); }
"%"                    { count(); return('%'); }
"<"                    { count(); return('<'); }
">"                    { count(); return('>'); }
"^"                    { count(); return('^'); }
"|"                    { count(); return('|'); }
"?"                    { count(); return('?'); }
"..."                  { count(); return(ELLIPSIS); }

  /*
   * Spaces, newlines etc.
   */

[ \t\v\f]              { count(); }
\n                     {
                         count();
                         if (on_omp_line || on_ompix_line)
                         {
                           on_omp_line = on_ompix_line = 0;  /* Line finished */
                           return('\n');
                         }
                       }
.                      { /* ignore bad characters */ }
%%


/* Notice that in the following we make use of input() and unput().
 * Those destroy yytext[] but we don't care what happens to it
 * at those points in the code.
 */


int yywrap()
{
  return(1);
}


/* Called upon encountering a line starting with '#' which does not
 * contain an OpenMP pragma.
 * We actually throw this line away but before doing so we check if
 * it conveys line number information from the original file. This
 * must be of the form:
 *    # <number> "<filename>"
 */
void sharp()
{
  char c, line[SCANNER_STR_SIZES] = {0}, *s, *t;
  int  lineno = -1;

  for (s = line; (c = input()) != '\n' && c != 0; )   /* Read in the line */
    if (s-line < SCANNER_STR_SIZES-2)
      *s++ = c;

  if (c == 0) return;          /* End of file - ignore anyways */
  *s = 0;                      /* Ignore \n and force end of string */
  thisfile_line++;             /* Update counters */
  thisfile_column = 0;

  if ( sscanf(line, "%d", &lineno) < 0 ) return;   /* Nope -- no line number */

  for (s = line; *s != '\"' && *s != 0; s++)       /* Find the " */
    ;
  if (*s == 0) return;                             /* Nope -- no file name */

  for (t = (++s); *s != '\"' && *s != 0; s++)      /* Find the next " */
    ;
  if (t == s) return;                              /* Nope -- empty file name */
  *s = 0;

  strncpy(origfile_name, t, SCANNER_STR_SIZES-1);
  origfile_line = lineno;
  marker_line   = thisfile_line;
}


/* Gobble till end of line (i.e. ignore C++ // comments)
 */
void gobbleline()
{
  char c;
  for ( ; (c = input()) != '\n' && c != 0; )   /* Read in the line */
    ;
  thisfile_line++;             /* Update counters */
  thisfile_column = 0;
}


/* Gobble spaces (returns the non-space char found)
 */
char gobblespaces()
{
  char c;
  for ( ; (c = input()) != 0 && isspace(c); )
    if (c == '\n')
    {
      thisfile_line++;
      thisfile_column = 0;
    }
    else
      thisfile_column++;
  if (c)
    thisfile_column++;
  return (c);
}


/* Gobble till the end of comment (i.e. ignore C comments)
 * We actually don't need to recognize comments, since we scan a
 * file that has already been preprocessed. However, we need this
 * functionality for testing purposes.
 * The code is a bit dump, i.e. it won't handle nested comments,
 * and it won't check for strings.
 */
void gobblecomment()
{
  char c, done = 0;

  /* Never put the !done *after* the (c = input()) != 0. Order matters!! */
  for ( ; !done && (c = input()) != 0; )
  {
    if (c == '\n')
    {
      thisfile_line++;               /* Update counters */
      thisfile_column = 0;
      continue;
    }
    thisfile_column++;
    if (c == '*')
    {
      while ((c = input()) == '*')
        thisfile_column++;
      if (c == '\n')
      {
        thisfile_line++;             /* Update counters */
        thisfile_column = 0;
      }
      else
      {
        thisfile_column++;
        if (c == '/')
          done = 1;
      }
    }
  }
}


/* Gobble (but store) till the end of an attribute phrase ((...)).
 * The code is a bit dump, i.e. it won't handle strings with "))" in them.
 */
int sc_scan_attribute(char **string)
{
  char c, done = 0, text[SCANNER_STR_SIZES];
  int  n;

  c = gobblespaces();
  if (c != '(' || ((c = input()) != '(')) return (-1);
  thisfile_column++;
  text[0] = text[1] = '(';

  /* Never put the !done *after* the (c = input()) != 0. Order matters!! */
  for (n = 2; !done && (c = input()) != 0 && n < SCANNER_STR_SIZES; )
  {
    text[n++] = c;
    if (c == '\n')
    {
      thisfile_line++;               /* Update counters */
      thisfile_column = 0;
      continue;
    }
    thisfile_column++;
    if (c == ')')
    {
      if ((c = input()) == ')')
        done = 1;
      text[n++] = c;
      if (done)
      {
        text[n++] = 0;
        *string = strdup(text);
      }
      thisfile_column++;
    }
  }
  return ( (c == 0 || n >= SCANNER_STR_SIZES) ? -1 : 0 );
}


/* Update line/column counters & generate the thingy to return to the parser */
void count()
{
  int i, nonempty = 0;

  for (i = 0; yytext[i] != 0; i++)
    if (yytext[i] == '\n')
    {
      thisfile_column = 0;
      thisfile_line++;
    }
    else
      if (yytext[i] == '\t')
        thisfile_column += ( 8 - (thisfile_column % 8) );
      else
      {
        thisfile_column++;
        if (!isspace(yytext[i]))
          nonempty = 1;
      };

  if (nonempty)
    strcpy(yylval.name, yytext);
}

/* Returns "return_if_true" if on omp/ompi line else returns IdentOrType
 * type 0 checks for OpenMp line, 1 checks for ompi line, 2 checks both
 */
int checkomp(int type, int return_if_true) {
  count();
  if ((type == 0 && on_omp_line) ||
      (type == 1 && on_ompix_line) ||
      (type == 2 && (on_omp_line || on_ompix_line)))
    return(return_if_true);
  else
    return IdentOrType();
}

/* Set everything up to scan from a string */
void sc_scan_string(char *s)
{
  yy_scan_string(s);
  *origfile_name = 0;
  sc_set_filename("injected_code");
}


/* Utilities */

void sc_set_start_token(int t)
       { start_token = t; thisfile_line = 1; thisfile_column = 0;
         marker_line = origfile_line = 0; }
void sc_set_filename(char *fn)
       { strncpy(thisfile_name, fn, 255); }
char *sc_original_file()
       { return ( (*origfile_name) ?  origfile_name : thisfile_name ); }
int  sc_original_line()  { return(thisfile_line - marker_line + origfile_line);}
int  sc_line()           { return(thisfile_line); }
int  sc_column()         { return(thisfile_column); }
void sc_pause_openmp()   { on_omp_line = 0; }
void sc_start_openmp()   { on_omp_line = 1; }
