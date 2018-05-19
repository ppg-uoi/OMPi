%{
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
%}

%union {
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
}

%error-verbose

/* expect 3 shift/reduce & 1 reduce/reduce */
/* %expect 3 */

/* Start-symbol tokens (trick from bison manual) */
%token START_SYMBOL_EXPRESSION START_SYMBOL_BLOCKLIST START_SYMBOL_TRANSUNIT
%type <type> start_trick

/* C tokens */
%token <name> IDENTIFIER TYPE_NAME CONSTANT STRING_LITERAL
%token <name> PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token <name> AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token <name> SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token <name> XOR_ASSIGN OR_ASSIGN SIZEOF

%token <name> TYPEDEF EXTERN STATIC AUTO REGISTER RESTRICT
%token <name> CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE
%token <name> CONST VOLATILE VOID INLINE
%token <name> UBOOL UCOMPLEX UIMAGINARY
%token <name> STRUCT UNION ENUM ELLIPSIS

%token <name> CASE DEFAULT IF ELSE SWITCH WHILE DO FOR
%token <name> GOTO CONTINUE BREAK RETURN

/* Hacks */
%token <name> __BUILTIN_VA_ARG __BUILTIN_OFFSETOF __BUILTIN_TYPES_COMPATIBLE_P
              __ATTRIBUTE__

/* OpenMP tokens */
%token <name> PRAGMA_OMP PRAGMA_OMP_THREADPRIVATE OMP_PARALLEL OMP_SECTIONS
%token <name> OMP_NOWAIT OMP_ORDERED OMP_SCHEDULE OMP_STATIC OMP_DYNAMIC
%token <name> OMP_GUIDED OMP_RUNTIME OMP_AUTO OMP_SECTION OMP_AFFINITY
%token <name> OMP_SINGLE OMP_MASTER OMP_CRITICAL OMP_BARRIER OMP_ATOMIC
%token <name> OMP_FLUSH OMP_PRIVATE OMP_FIRSTPRIVATE
%token <name> OMP_LASTPRIVATE OMP_SHARED OMP_DEFAULT OMP_NONE OMP_REDUCTION
%token <name> OMP_COPYIN OMP_NUMTHREADS OMP_COPYPRIVATE OMP_FOR OMP_IF
       /* added @ OpenMP 3.0 */
%token <name> OMP_TASK OMP_UNTIED OMP_TASKWAIT OMP_COLLAPSE
       /* added @ OpenMP 3.1 */
%token <name> OMP_FINAL OMP_MERGEABLE OMP_TASKYIELD OMP_READ OMP_WRITE
%token <name> OMP_CAPTURE OMP_UPDATE OMP_MIN OMP_MAX
       /* added @ OpenMP 4.0 */
%token <name> OMP_PROCBIND OMP_CLOSE OMP_SPREAD OMP_SIMD OMP_INBRANCH
%token <name> OMP_NOTINBRANCH OMP_UNIFORM OMP_LINEAR OMP_ALIGNED OMP_SIMDLEN
%token <name> OMP_SAFELEN OMP_DECLARE OMP_TARGET OMP_DATA OMP_DEVICE OMP_MAP
%token <name> OMP_ALLOC OMP_TO OMP_FROM OMP_TOFROM OMP_END OMP_TEAMS
%token <name> OMP_DISTRIBUTE OMP_NUMTEAMS OMP_THREADLIMIT OMP_DISTSCHEDULE
%token <name> OMP_DEPEND OMP_IN OMP_OUT OMP_INOUT OMP_TASKGROUP OMP_SEQ_CST
%token <name> OMP_CANCEL OMP_INITIALIZER PRAGMA_OMP_CANCELLATIONPOINT

/* C non-terminals */
%type <symb>   enumeration_constant
%type <string> string_literal
%type <expr>   primary_expression
%type <expr>   postfix_expression
%type <expr>   argument_expression_list
%type <expr>   unary_expression
%type <type>   unary_operator
%type <expr>   cast_expression
%type <expr>   multiplicative_expression
%type <expr>   additive_expression
%type <expr>   shift_expression
%type <expr>   relational_expression
%type <expr>   equality_expression
%type <expr>   AND_expression
%type <expr>   exclusive_OR_expression
%type <expr>   inclusive_OR_expression
%type <expr>   logical_AND_expression
%type <expr>   logical_OR_expression
%type <expr>   conditional_expression
%type <expr>   assignment_expression
%type <type>   assignment_operator
%type <expr>   expression
%type <expr>   constant_expression
%type <stmt>   declaration
%type <spec>   declaration_specifiers
%type <decl>   init_declarator_list
%type <decl>   init_declarator
%type <spec>   storage_class_specifier
%type <spec>   type_specifier
%type <spec>   struct_or_union_specifier
%type <type>   struct_or_union
%type <decl>   struct_declaration_list
%type <decl>   struct_declaration
%type <spec>   specifier_qualifier_list
%type <decl>   struct_declarator_list
%type <decl>   struct_declarator
%type <spec>   enum_specifier
%type <spec>   enumerator_list
%type <spec>   enumerator
%type <spec>   type_qualifier
%type <spec>   function_specifier
%type <decl>   declarator
%type <decl>   direct_declarator
%type <spec>   pointer
%type <spec>   type_qualifier_list
%type <decl>   parameter_type_list
%type <decl>   parameter_list
%type <decl>   parameter_declaration
%type <decl>   identifier_list
%type <decl>   type_name
%type <decl>   abstract_declarator
%type <decl>   direct_abstract_declarator
%type <symb>   typedef_name
%type <expr>   initializer
%type <expr>   initializer_list
%type <expr>   designation
%type <expr>   designator_list
%type <expr>   designator
%type <stmt>   statement
%type <stmt>   statement_for_labeled
%type <stmt>   labeled_statement
%type <stmt>   compound_statement
%type <stmt>   block_item_list
%type <stmt>   block_item
%type <stmt>   expression_statement
%type <stmt>   selection_statement
%type <stmt>   iteration_statement
%type <stmt>   iteration_statement_for
%type <stmt>   jump_statement
%type <stmt>   translation_unit
%type <stmt>   external_declaration
%type <stmt>   function_definition
%type <stmt>   normal_function_definition
%type <stmt>   oldstyle_function_definition
%type <stmt>   declaration_list

/* OpenMP non-terminals */
%type <ocon>   openmp_construct
%type <ocon>   openmp_directive
%type <stmt>   structured_block
%type <ocon>   parallel_construct
%type <odir>   parallel_directive
%type <ocla>   parallel_clause_optseq
%type <ocla>   parallel_clause
%type <ocla>   unique_parallel_clause
%type <ocon>   for_construct
%type <ocla>   for_clause_optseq
%type <odir>   for_directive
%type <ocla>   for_clause
%type <ocla>   unique_for_clause
%type <type>   schedule_kind
%type <ocon>   sections_construct
%type <ocla>   sections_clause_optseq
%type <odir>   sections_directive
%type <ocla>   sections_clause
%type <stmt>   section_scope
%type <stmt>   section_sequence
%type <odir>   section_directive
%type <ocon>   single_construct
%type <ocla>   single_clause_optseq
%type <odir>   single_directive
%type <ocla>   single_clause
%type <ocon>   parallel_for_construct
%type <ocla>   parallel_for_clause_optseq
%type <odir>   parallel_for_directive
%type <ocla>   parallel_for_clause
%type <ocon>   parallel_sections_construct
%type <ocla>   parallel_sections_clause_optseq
%type <odir>   parallel_sections_directive
%type <ocla>   parallel_sections_clause
%type <ocon>   master_construct
%type <odir>   master_directive
%type <ocon>   critical_construct
%type <odir>   critical_directive
%type <symb>   region_phrase
%type <odir>   barrier_directive
%type <ocon>   atomic_construct
%type <odir>   atomic_directive
%type <ocla>   atomic_clause_opt
%type <odir>   flush_directive
%type <decl>   flush_vars
%type <ocon>   ordered_construct
%type <odir>   ordered_directive
%type <odir>   threadprivate_directive
%type <ocla>   procbind_clause
%type <decl>   variable_list
%type <decl>   thrprv_variable_list
    /* added @ OpenMP V3.0 */
%type <ocon>   task_construct
%type <odir>   task_directive
%type <ocla>   task_clause_optseq
%type <ocla>   task_clause
%type <ocla>   unique_task_clause
%type <odir>   taskwait_directive
%type <odir>   taskyield_directive
%type <ocla>   data_default_clause
%type <ocla>   data_privatization_clause
%type <ocla>   data_privatization_in_clause
%type <ocla>   data_privatization_out_clause
%type <ocla>   data_sharing_clause
%type <ocla>   data_reduction_clause
    /* added @ OpenMP V4.0 */
%type <stmt>   declaration_definition
%type <stmt>   function_statement
%type <stmt>   declarations_definitions_seq
%type <ocon>   simd_construct
%type <odir>   simd_directive
%type <ocla>   simd_clause_optseq
%type <ocla>   simd_clause
%type <ocla>   unique_simd_clause
%type <ocon>   declare_simd_construct
//TODO declare_simd_directive_seq
%type <odir>   declare_simd_directive
%type <ocla>   declare_simd_clause_optseq
%type <ocla>   declare_simd_clause
%type <ocon>   for_simd_construct
%type <odir>   for_simd_directive
%type <ocla>   for_simd_clause_optseq
%type <ocla>   for_simd_clause
%type <ocon>   parallel_for_simd_construct
%type <odir>   parallel_for_simd_directive
%type <ocla>   parallel_for_simd_clause_optseq
%type <ocla>   parallel_for_simd_clause
%type <ocon>   target_data_construct
%type <odir>   target_data_directive
%type <ocla>   target_data_clause_optseq
%type <ocla>   target_data_clause
%type <ocla>   device_clause
%type <ocla>   map_clause
%type <type>   map_type
%type <ocon>   target_construct
%type <odir>   target_directive
%type <ocla>   target_clause_optseq
%type <ocla>   target_clause
%type <ocla>   unique_target_clause
%type <ocon>   target_update_construct
%type <odir>   target_update_directive
%type <ocla>   target_update_clause_seq
%type <ocla>   target_update_clause
%type <ocla>   motion_clause
%type <ocon>   teams_construct
%type <odir>   teams_directive
%type <ocla>   teams_clause_optseq
%type <ocla>   teams_clause
%type <ocla>   unique_teams_clause
%type <ocon>   distribute_construct
%type <odir>   distribute_directive
%type <ocla>   distribute_clause_optseq
%type <ocla>   distribute_clause
%type <ocla>   unique_distribute_clause
%type <ocon>   distribute_simd_construct
%type <odir>   distribute_simd_directive
%type <ocla>   distribute_simd_clause_optseq
%type <ocla>   distribute_simd_clause
%type <ocon>   distribute_parallel_for_construct
%type <odir>   distribute_parallel_for_directive
%type <ocla>   distribute_parallel_for_clause_optseq
%type <ocla>   distribute_parallel_for_clause
%type <ocon>   distribute_parallel_for_simd_construct
%type <odir>   distribute_parallel_for_simd_directive
%type <ocla>   distribute_parallel_for_simd_clause_optseq
%type <ocla>   distribute_parallel_for_simd_clause
%type <ocon>   target_teams_construct
%type <odir>   target_teams_directive
%type <ocla>   target_teams_clause_optseq
%type <ocla>   target_teams_clause
%type <ocon>   teams_distribute_construct
%type <odir>   teams_distribute_directive
%type <ocla>   teams_distribute_clause_optseq
%type <ocla>   teams_distribute_clause
%type <ocon>   teams_distribute_simd_construct
%type <odir>   teams_distribute_simd_directive
%type <ocla>   teams_distribute_simd_clause_optseq
%type <ocla>   teams_distribute_simd_clause
%type <ocon>   target_teams_distribute_construct
%type <odir>   target_teams_distribute_directive
%type <ocla>   target_teams_distribute_clause_optseq
%type <ocla>   target_teams_distribute_clause
%type <ocon>   target_teams_distribute_simd_construct
%type <odir>   target_teams_distribute_simd_directive
%type <ocla>   target_teams_distribute_simd_clause_optseq
%type <ocla>   target_teams_distribute_simd_clause
%type <ocon>   teams_distribute_parallel_for_construct
%type <odir>   teams_distribute_parallel_for_directive
%type <ocla>   teams_distribute_parallel_for_clause_optseq
%type <ocla>   teams_distribute_parallel_for_clause
%type <ocon>   target_teams_distribute_parallel_for_construct
%type <odir>   target_teams_distribute_parallel_for_directive
%type <ocla>   target_teams_distribute_parallel_for_clause_optseq
%type <ocla>   target_teams_distribute_parallel_for_clause
%type <ocon>   teams_distribute_parallel_for_simd_construct
%type <odir>   teams_distribute_parallel_for_simd_directive
%type <ocla>   teams_distribute_parallel_for_simd_clause_optseq
%type <ocla>   teams_distribute_parallel_for_simd_clause
%type <ocon>   target_teams_distribute_parallel_for_simd_construct
%type <odir>   target_teams_distribute_parallel_for_simd_directive
%type <ocla>   target_teams_distribute_parallel_for_simd_clause_optseq
%type <ocla>   target_teams_distribute_parallel_for_simd_clause
%type <ocla>   unique_single_clause
%type <ocla>   aligned_clause
%type <ocla>   linear_clause
%type <expr>   optional_expression
%type <ocla>   uniform_clause
%type <ocla>   inbranch_clause
%type <ocon>   declare_target_construct
%type <odir>   declare_target_directive
%type <odir>   end_declare_target_directive
%type <type>   dependence_type
%type <ocon>   taskgroup_construct
%type <odir>   taskgroup_directive
%type <ocla>   seq_cst_clause_opt
%type <odir>   cancel_directive
%type <odir>   cancellation_point_directive
%type <ocla>   construct_type_clause
%type <type>   reduction_identifier
%type <type>   reduction_type_list // TODO
%type <ocla>   initializer_clause_opt
%type <ocla>   if_clause
%type <ocla>   collapse_clause
%type <decl>   array_section
%type <decl>   variable_array_section_list
%type <decl>   array_section_plain

/*
 * OMPi-extensions
 */

/* Tokens */
%token <name> PRAGMA_OMPIX OMPIX_TASKDEF OMPIX_IN OMPIX_OUT OMPIX_INOUT
%token <name> OMPIX_TASKSYNC OMPIX_UPONRETURN OMPIX_ATNODE OMPIX_DETACHED
%token <name> OMPIX_ATWORKER OMPIX_TASKSCHEDULE OMPIX_STRIDE OMPIX_START
%token <name> OMPIX_SCOPE OMPIX_NODES OMPIX_WORKERS OMPIX_LOCAL OMPIX_GLOBAL
%token <name> OMPIX_TIED

/* Non-terminals */
%type <xcon>   ompix_construct
%type <xcon>   ompix_directive
%type <xcon>   ox_taskdef_construct
%type <xdir>   ox_taskdef_directive
%type <xcla>   ox_taskdef_clause_optseq
%type <xcla>   ox_taskdef_clause
%type <decl>   ox_variable_size_list
%type <decl>   ox_variable_size_elem
%type <xcon>   ox_task_construct
%type <xdir>   ox_task_directive
%type <xcla>   ox_task_clause_optseq
%type <xcla>   ox_task_clause
%type <expr>   ox_funccall_expression
%type <xdir>   ox_tasksync_directive
%type <xdir>   ox_taskschedule_directive
%type <xcla>   ox_taskschedule_clause_optseq
%type <xcla>   ox_taskschedule_clause
%type <type>   ox_scope_spec

%%

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     THE RULES                                                 *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

start_trick:
    translation_unit                        { /* to avoid warnings */ }
  | START_SYMBOL_EXPRESSION expression      { pastree_expr = $2; }
  | START_SYMBOL_BLOCKLIST block_item_list  { pastree_stmt = $2; }
  | START_SYMBOL_TRANSUNIT translation_unit { pastree_stmt = $2; }
;



/* -------------------------------------------------------------------------
 * ---------- ISO/IEC 9899:1999 A.1 Lexical grammar ------------------------
 * -------------------------------------------------------------------------
 */

/* -------------------------------------------------------------------------
 * ---------- ISO/IEC 9899:1999 A.1.5 Constants ----------------------------
 * -------------------------------------------------------------------------
 */

/* ISO/IEC 9899:1999 6.4.4.3 */
enumeration_constant:
    IDENTIFIER
    {
      symbol s = Symbol($1);
      if (checkDecls)
      {
        if ( symtab_get(stab, s, LABELNAME) )  /* NOT a type name */
          parse_error(-1, "enum symbol '%s' is already in use.", $1);
        symtab_put(stab, s, LABELNAME);
      }
      $$ = s;
    }
;


/* -------------------------------------------------------------------------
 * ---------- ISO/IEC 9899:1999 A.1.6 String literals ----------------------
 * -------------------------------------------------------------------------
 */

/* ISO/IEC 9899:1999 6.4.5 */
string_literal:
    STRING_LITERAL
    {
      $$ = strdup($1);
    }
  | string_literal STRING_LITERAL
    {
      /* Or we could leave it as is (as a SpaceList) */
      if (($1 = realloc($1, strlen($1) + strlen($2))) == NULL)
        parse_error(-1, "string out of memory\n");
      strcpy(($1)+(strlen($1)-1),($2)+1);  /* Catenate on the '"' */
      $$ = $1;
    }
;


/* -------------------------------------------------------------------------
 * ------ ISO/IEC 9899:1999 A.2 Phrase structure grammar -------------------
 * -------------------------------------------------------------------------
 */

/* -------------------------------------------------------------------------
 * ------- ISO/IEC 9899:1999 A.2.1 Expressions -----------------------------
 * -------------------------------------------------------------------------
 */

/*  ISO/IEC 9899:1999 6.5.1 */
primary_expression:
    IDENTIFIER
    {
      symbol  id = Symbol($1);
      stentry e;
      bool    chflag = false;

      if (checkDecls)
      {
        check_uknown_var($1);
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
      $$ = chflag ? Parenthesis(UnaryOperator(UOP_star, Identifier(id)))
                  : Identifier(id);
    }
  | CONSTANT
    {
      $$ = Constant( strdup($1) );
    }
  | string_literal
    {
      $$ = String($1);
    }
  | '(' expression ')'
    {
      $$ = Parenthesis($2);
    }
;

/*  ISO/IEC 9899:1999 6.5.2 */
postfix_expression:
    primary_expression
    {
      $$ = $1;
    }
  | postfix_expression '[' expression ']'
    {
      $$ = ArrayIndex($1, $3);
    }
  /* The following 2 rules were added so that calling undeclared functions
   * does not result in "unknown identifier" messages (it was matched by
   * the IDENTIFIER rule in primary_expression.
   * They account for 2 shift/reduce conflicts.
   * (VVD)
   */
  | IDENTIFIER '(' ')'
    {
      /* Catch calls to "main()" (unlikely but possible) */
      $$ = strcmp($1, "main") ?
             FunctionCall(IdentName($1), NULL) :
             FunctionCall(IdentName(MAIN_NEWNAME), NULL);
    }
  | IDENTIFIER '(' argument_expression_list ')'
    {
      /* Catch calls to "main()" (unlikely but possible) */
      $$ = strcmp($1, "main") ?
             FunctionCall(IdentName($1), $3) :
             FunctionCall(IdentName(MAIN_NEWNAME), $3);
    }
  | postfix_expression '(' ')'
    {
      $$ = FunctionCall($1, NULL);
    }
  | postfix_expression '(' argument_expression_list ')'
    {
      $$ = FunctionCall($1, $3);
    }
  | postfix_expression '.' IDENTIFIER
    {
      $$ = DotField($1, Symbol($3));
    }
  | postfix_expression PTR_OP IDENTIFIER
    {
      $$ = PtrField($1, Symbol($3));
    }
    /* The next two are artificial rules, to cover the cases where
     * a struct field name is identical to one of user type names;
     * the scanner returns TYPE_NAME in this case, which would causes
     * a syntax error.
     */
  | postfix_expression '.' typedef_name
    {
      $$ = DotField($1, $3);
    }
  | postfix_expression PTR_OP typedef_name
    {
      $$ = PtrField($1, $3);
    }
  | postfix_expression INC_OP
    {
      $$ = PostOperator($1, UOP_inc);
    }
  | postfix_expression DEC_OP
    {
      $$ = PostOperator($1, UOP_dec);
    }
  | '(' type_name ')' '{' initializer_list '}'
    {
      $$ = CastedExpr($2, BracedInitializer($5));
    }
  | '(' type_name ')' '{' initializer_list ',' '}'
    {
      $$ = CastedExpr($2, BracedInitializer($5));
    }
;

/*  ISO/IEC 9899:1999 6.5.2 */
argument_expression_list:
    assignment_expression
    {
      $$ = $1;
    }
  | argument_expression_list ',' assignment_expression
    {
      $$ = CommaList($1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.5.3 */
unary_expression:
    postfix_expression
    {
      $$ = $1;
    }
  | INC_OP unary_expression
    {
      $$ = PreOperator($2, UOP_inc);
    }
  | DEC_OP unary_expression
    {
      $$ = PreOperator($2, UOP_dec);
    }
  | unary_operator cast_expression
    {
      if ($1 == -1)
        $$ = $2;                    /* simplify */
      else
        $$ = UnaryOperator($1, $2);
    }
  | SIZEOF unary_expression
    {
      $$ = Sizeof($2);
    }
  | SIZEOF '(' type_name ')'
    {
      $$ = Sizeoftype($3);
    }
  /* The following are hacks to let some functions accept an
   * argument that is a type, not an expression. The "TypeTrick"
   * makes it behave like a "Sizeof", which however when the
   * tree is printed is nothing ("").
   */
  | __BUILTIN_VA_ARG '(' assignment_expression ',' type_name ')'
    {
      $$ = FunctionCall(IdentName("__builtin_va_arg"),
                        CommaList($3, TypeTrick($5)));
    }
  | __BUILTIN_OFFSETOF '(' type_name ',' IDENTIFIER ')'
    {
      $$ = FunctionCall(IdentName("__builtin_offsetof"),
                        CommaList(TypeTrick($3), IdentName($5)));
    }
  | __BUILTIN_TYPES_COMPATIBLE_P '(' type_name ',' type_name ')'
    {
      $$ = FunctionCall(IdentName("__builtin_types_compatible_p"),
                        CommaList(TypeTrick($3), TypeTrick($5)));
    }
;

/*  ISO/IEC 9899:1999 6.5.3 */
unary_operator:
    '&'
    {
      $$ = UOP_addr;
    }
  | '*'
    {
      $$ = UOP_star;
    }
  | '+'
    {
      $$ = -1;         /* Ingore this one */
    }
  | '-'
    {
      $$ = UOP_neg;
    }
  | '~'
    {
      $$ = UOP_bnot;
    }
  | '!'
    {
      $$ = UOP_lnot;
    }
;

/*  ISO/IEC 9899:1999 6.5.4 */
cast_expression:
    unary_expression
    {
      $$ = $1;
    }
  | '(' type_name ')' cast_expression
    {
      $$ = CastedExpr($2, $4);
    }
;

/*  ISO/IEC 9899:1999 6.5.5 */
multiplicative_expression:
    cast_expression
    {
      $$ = $1;
    }
  | multiplicative_expression '*' cast_expression
    {
      $$ = BinaryOperator(BOP_mul, $1, $3);
    }
  | multiplicative_expression '/' cast_expression
    {
      $$ = BinaryOperator(BOP_div, $1, $3);
    }
  | multiplicative_expression '%' cast_expression
    {
      $$ = BinaryOperator(BOP_mod, $1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.5.6 */
additive_expression:
    multiplicative_expression
    {
      $$ = $1;
    }
  | additive_expression '+' multiplicative_expression
    {
      $$ = BinaryOperator(BOP_add, $1, $3);
    }
  | additive_expression '-' multiplicative_expression
    {
      $$ = BinaryOperator(BOP_sub, $1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.5.7 */
shift_expression:
    additive_expression
    {
      $$ = $1;
    }
  | shift_expression LEFT_OP additive_expression
    {
      $$ = BinaryOperator(BOP_shl, $1, $3);
    }
  | shift_expression RIGHT_OP additive_expression
    {
      $$ = BinaryOperator(BOP_shr, $1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.5.8 */
relational_expression:
    shift_expression
    {
      $$ = $1;
    }
  | relational_expression '<' shift_expression
    {
      $$ = BinaryOperator(BOP_lt, $1, $3);
    }
  | relational_expression '>' shift_expression
    {
      $$ = BinaryOperator(BOP_gt, $1, $3);
    }
  | relational_expression LE_OP shift_expression
    {
      $$ = BinaryOperator(BOP_leq, $1, $3);
     }
  | relational_expression GE_OP shift_expression
    {
      $$ = BinaryOperator(BOP_geq, $1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.5.9 */
equality_expression:
    relational_expression
    {
      $$ = $1;
    }
  | equality_expression EQ_OP relational_expression
    {
      $$ = BinaryOperator(BOP_eqeq, $1, $3);
    }
  | equality_expression NE_OP relational_expression
    {
      $$ = BinaryOperator(BOP_neq, $1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.5.10 */
AND_expression:
    equality_expression
    {
      $$ = $1;
    }
  | AND_expression '&' equality_expression
    {
      $$ = BinaryOperator(BOP_band, $1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.5.11 */
exclusive_OR_expression:
    AND_expression
    {
      $$ = $1;
    }
  | exclusive_OR_expression '^' AND_expression
    {
      $$ = BinaryOperator(BOP_xor, $1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.5.12 */
inclusive_OR_expression:
    exclusive_OR_expression
    {
      $$ = $1;
    }
  | inclusive_OR_expression '|' exclusive_OR_expression
    {
      $$ = BinaryOperator(BOP_bor, $1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.5.13 */
logical_AND_expression:
    inclusive_OR_expression
    {
      $$ = $1;
    }
  | logical_AND_expression AND_OP inclusive_OR_expression
    {
      $$ = BinaryOperator(BOP_land, $1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.5.14 */
logical_OR_expression:
    logical_AND_expression
    {
      $$ = $1;
    }
  | logical_OR_expression OR_OP logical_AND_expression
    {
      $$ = BinaryOperator(BOP_lor, $1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.5.15 */
conditional_expression:
    logical_OR_expression
    {
      $$ = $1;
    }
  | logical_OR_expression '?' expression ':' conditional_expression
    {
      $$ = ConditionalExpr($1, $3, $5);
    }
;

/*  ISO/IEC 9899:1999 6.5.16 */
assignment_expression:
    conditional_expression
    {
      $$ = $1;
    }
  | unary_expression assignment_operator assignment_expression
    {
      $$ = Assignment($1, $2, $3);
    }
;

/*  ISO/IEC 9899:1999 6.5.16 */
assignment_operator:
    '='
    {
      $$ = ASS_eq;  /* Need fix here! */
    }
  | MUL_ASSIGN
    {
      $$ = ASS_mul;
    }
  | DIV_ASSIGN
    {
      $$ = ASS_div;
    }
  | MOD_ASSIGN
    {
      $$ = ASS_mod;
    }
  | ADD_ASSIGN
    {
      $$ = ASS_add;
    }
  | SUB_ASSIGN
    {
      $$ = ASS_sub;
    }
  | LEFT_ASSIGN
    {
      $$ = ASS_shl;
    }
  | RIGHT_ASSIGN
    {
      $$ = ASS_shr;
    }
  | AND_ASSIGN
    {
      $$ = ASS_and;
    }
  | XOR_ASSIGN
    {
      $$ = ASS_xor;
    }
  | OR_ASSIGN
    {
      $$ = ASS_or;
    }
;

/*  ISO/IEC 9899:1999 6.5.17 */
expression:
    assignment_expression
    {
      $$ = $1;
    }
  | expression ',' assignment_expression
    {
      $$ = CommaList($1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.6 */
constant_expression:
    conditional_expression
    {
      $$ = $1;
    }
;


/* -------------------------------------------------------------------------
 * ------------ ISO/IEC 9899:1999 A.2.2 Declarations -----------------------
 * -------------------------------------------------------------------------
 */

/*  ISO/IEC 9899:1999 6.7 */
declaration:
    declaration_specifiers ';'
    {
      /* There is a special case which wrongly uses this rule:
       *   typedef xxx already_known_user_type.
       * In this case the already_known_user_type (T) is re-defined,
       * and because T is known, it is not considered as a declarator,
       * but a "typedef_name", and is part of the specifier.
       * We fix it here.
       */
      if (isTypedef && $1->type == SPECLIST)
        $$ = Declaration($1, fix_known_typename($1));
      else
        $$ = Declaration($1, NULL);
      isTypedef = 0;
    }
  | declaration_specifiers init_declarator_list ';'
    {
      $$ = Declaration($1, $2);
      if (checkDecls) add_declaration_links($1, $2);
      isTypedef = 0;

    }
  | threadprivate_directive // OpenMP Version 2.5 ISO/IEC 9899:1999 addition
    {
      $$ = OmpStmt(OmpConstruct(DCTHREADPRIVATE, $1, NULL));
    }
  | /* OpenMP V4.0 */
    declare_simd_construct
    {
      //$$ = OmpStmt(OmpConstruct(DCTHREADPRIVATE, $1, NULL)); TODO
    }
  | declare_target_construct
    {
      $$ = OmpStmt($1);
    }
  | declare_reduction_directive
    {
      //$$ = OmpStmt(OmpConstruct(DCTHREADPRIVATE, $1, NULL)); TODO
    }
;

/* ISO/IEC 9899:1999 6.7 */
declaration_specifiers:
    storage_class_specifier
    {
      $$ = $1;
    }
  | storage_class_specifier declaration_specifiers
    {
      $$ = Speclist_right($1, $2);
    }
  | type_specifier
    {
      $$ = $1;
    }
  | type_specifier declaration_specifiers
    {
      $$ = Speclist_right($1, $2);
    }
  | type_qualifier
    {
      $$ = $1;
    }
  | type_qualifier declaration_specifiers
    {
      $$ = Speclist_right($1, $2);
    }
  | function_specifier
    {
      $$ = $1;
    }
  | function_specifier declaration_specifiers
    {
      $$ = Speclist_right($1, $2);
    }
;

/*  ISO/IEC 9899:1999 6.7 */
init_declarator_list:
    init_declarator
    {
      $$ = $1;
    }
  | init_declarator_list ',' init_declarator
    {
      $$ = DeclList($1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.7 */
/* This is the only place to come for a full declaration.
 * Other declarator calls are not of particular interest.
 * Also, note that we cannot do it in a parent rule (e.g. in
 * "declaration" since initializers may use variables defined
 * previously, in the same declarator list.
 */
init_declarator:
    declarator
    {
      astdecl s = decl_getidentifier($1);
      int     declkind = decl_getkind($1);
      stentry e;

      if (!isTypedef && declkind == DFUNC && strcmp(s->u.id->name, "main") == 0)
        s->u.id = Symbol(MAIN_NEWNAME);       /* Catch main()'s declaration */
      if (checkDecls)
      {
        e = symtab_put(stab, s->u.id, (isTypedef) ? TYPENAME :
                                       (declkind == DFUNC) ? FUNCNAME : IDNAME);
        e->isarray = (declkind == DARRAY);
      }
      $$ = $1;
    }
  | declarator '='
    {
      astdecl s = decl_getidentifier($1);
      int     declkind = decl_getkind($1);
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
    initializer
    {
      $$ = InitDecl($1, $4);
    }
;

/*  ISO/IEC 9899:1999 6.7.1 */
storage_class_specifier:
    TYPEDEF
    {
      $$ = StClassSpec(SPEC_typedef);    /* Just a string */
      isTypedef = 1;
    }
  | EXTERN
    {
      $$ = StClassSpec(SPEC_extern);
    }
  | STATIC
    {
      $$ = StClassSpec(SPEC_static);
    }
  | AUTO
    {
      $$ = StClassSpec(SPEC_auto);
    }
  | REGISTER
    {
      $$ = StClassSpec(SPEC_register);
    }
;

/*  ISO/IEC 9899:1999 6.7.2 */
type_specifier:
    VOID
    {
      $$ = Declspec(SPEC_void);
    }
  | CHAR
    {
      $$ = Declspec(SPEC_char);
    }
  | SHORT
    {
      $$ = Declspec(SPEC_short);
    }
  | INT
    {
      $$ = Declspec(SPEC_int);
    }
  | LONG
    {
      $$ = Declspec(SPEC_long);
    }
  | FLOAT
    {
      $$ = Declspec(SPEC_float);
    }
  | DOUBLE
    {
      $$ = Declspec(SPEC_double);
    }
  | SIGNED
    {
      $$ = Declspec(SPEC_signed);
    }
  | UNSIGNED
    {
      $$ = Declspec(SPEC_unsigned);
    }
  | UBOOL
    {
      $$ = Declspec(SPEC_ubool);
    }
  | UCOMPLEX
    {
      $$ = Declspec(SPEC_ucomplex);
    }
  | UIMAGINARY
    {
      $$ = Declspec(SPEC_uimaginary);
    }
  | struct_or_union_specifier
    {
      $$ = $1;
    }
  | enum_specifier
    {
      $$ = $1;
    }
  | typedef_name
    {
      $$ = Usertype($1);
    }
;

/*  ISO/IEC 9899:1999 6.7.2.1 */
struct_or_union_specifier:
    struct_or_union '{' struct_declaration_list '}'
    {
      $$ = SUdecl($1, NULL, $3);
    }
  | struct_or_union '{' '}' /* NON-ISO empty declaration (added by SM) */
    {
      $$ = SUdecl($1, NULL, NULL);
    }
  | struct_or_union IDENTIFIER '{' struct_declaration_list '}'
    {
      symbol s = Symbol($2);
      /* Well, struct & union names have their own name space, and
       * their own scopes. I.e. they can be re-declare in nested
       * scopes. We don't do any kind of duplicate checks.
       */
      if (checkDecls)
        symtab_put(stab, s, SUNAME);
      $$ = SUdecl($1, s, $4);
    }
  /* If we have "typedef struct X X;" then X will become a TYPE_NAME
   * from now on, altough it is also a SUNAME. Thus it won't be matched
   * by the previous rule -- this explains the following one!
   */
  | struct_or_union typedef_name '{' struct_declaration_list '}'
    {
      symbol s = $2;
      if (checkDecls)
        symtab_put(stab, s, SUNAME);
      $$ = SUdecl($1, s, $4);
    }
  | struct_or_union IDENTIFIER
    {
      symbol s = Symbol($2);
      if (checkDecls)
        symtab_put(stab, s, SUNAME);
      $$ = SUdecl($1, s, NULL);
    }
  | struct_or_union typedef_name       /* As above! */
    {
      symbol s = $2;
      if (checkDecls)
        symtab_put(stab, s, SUNAME);
      $$ = SUdecl($1, s, NULL);
    }
;

/*  ISO/IEC 9899:1999 6.7.2.1 */
struct_or_union:
    STRUCT
    {
      $$ = SPEC_struct;
    }
  | UNION
    {
      $$ = SPEC_union;
    }
;

/*  ISO/IEC 9899:1999 6.7.2.1 */
struct_declaration_list:
    struct_declaration
    {
      $$ = $1;
    }
  | struct_declaration_list struct_declaration
    {
      $$ = StructfieldList($1, $2);
    }
;

/*  ISO/IEC 9899:1999 6.7.2.1 */
struct_declaration:
    specifier_qualifier_list struct_declarator_list ';'
    {
      $$ = StructfieldDecl($1, $2);
    }
  | specifier_qualifier_list ';'        /* added rule; just for GCC's shake */
    {
      $$ = StructfieldDecl($1, NULL);
    }
;

/*  ISO/IEC 9899:1999 6.7.2.1 */
specifier_qualifier_list:
    type_specifier
    {
      $$ = $1;
    }
  | type_specifier specifier_qualifier_list
    {
      $$ = Speclist_right($1, $2);
    }
  | type_qualifier
    {
      $$ = $1;
    }
  | type_qualifier specifier_qualifier_list
    {
      $$ = Speclist_right($1, $2);
    }
;

/*  ISO/IEC 9899:1999 6.7.2.1 */
struct_declarator_list:
    struct_declarator
    {
      $$ = $1;
    }
  | struct_declarator_list ',' struct_declarator
    {
      $$ = DeclList($1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.7.2.1 */
struct_declarator:
    declarator
    {
      $$ = $1;
    }
  | declarator ':' constant_expression
    {
      $$ = BitDecl($1, $3);
    }
  | ':' constant_expression
    {
      $$ = BitDecl(NULL, $2);
    }
;

/*  ISO/IEC 9899:1999 6.7.2.2 */
enum_specifier:
    ENUM '{' enumerator_list '}'
    {
      $$ = Enumdecl(NULL, $3);
    }
  | ENUM IDENTIFIER '{' enumerator_list '}'
    {
      symbol s = Symbol($2);

      if (checkDecls)
      {
        if (symtab_get(stab, s, ENUMNAME))
          parse_error(-1, "enum name '%s' is already in use.", $2);
        symtab_put(stab, s, ENUMNAME);
      }
      $$ = Enumdecl(s, $4);
    }
  | ENUM '{' enumerator_list ',' '}'
    {
      $$ = Enumdecl(NULL, $3);
    }
  | ENUM IDENTIFIER '{' enumerator_list ',' '}'
    {
      symbol s = Symbol($2);

      if (checkDecls)
      {
        if (symtab_get(stab, s, ENUMNAME))
          parse_error(-1, "enum name '%s' is already in use.", $2);
        symtab_put(stab, s, ENUMNAME);
      }
      $$ = Enumdecl(s, $4);
    }
  | ENUM IDENTIFIER
    {
      /*
      if (symtab_get(stab, s, ENUMNAME))
        parse_error(-1, "enum name '%s' is unknown.", $2);
      */
      $$ = Enumdecl(Symbol($2), NULL);
    }
;

/*  ISO/IEC 9899:1999 6.7.2.2 */
enumerator_list:
    enumerator
    {
      $$ = $1;
    }
  | enumerator_list ',' enumerator
    {
      $$ = Enumbodylist($1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.7.2.2 */
enumerator:
    enumeration_constant
    {
      $$ = Enumerator($1, NULL);
    }
  |  enumeration_constant '=' constant_expression
    {
      $$ = Enumerator($1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.7.3 */
type_qualifier:
    CONST
    {
      $$ = Declspec(SPEC_const);
    }
  | RESTRICT
    {
      $$ = Declspec(SPEC_restrict);
    }
  | VOLATILE
    {
      $$ = Declspec(SPEC_volatile);
    }
;

/*  ISO/IEC 9899:1999 6.7.4 */
function_specifier:
    INLINE
    {
      $$ = Declspec(SPEC_inline);
    }
;

/*  ISO/IEC 9899:1999 6.7.5 */
declarator:
    direct_declarator
    {
      $$ = Declarator(NULL, $1);
    }
  | pointer direct_declarator
    {
      $$ = Declarator($1, $2);
    }
/*  | error
    {
      parse_error(1, "was expecting a declarator here (%s).\n", yylval.name);
    }*/
;


/*  ISO/IEC 9899:1999 6.7.5 */
direct_declarator:
    IDENTIFIER
    {
      $$ = IdentifierDecl( Symbol($1) );
    }
  | '(' declarator ')'
    {
      /* Try to simplify a bit: (ident) -> ident */
      if ($2->spec == NULL && $2->decl->type == DIDENT)
        $$ = $2->decl;
      else
        $$ = ParenDecl($2);
    }
  | direct_declarator '[' ']'
    {
      $$ = ArrayDecl($1, NULL, NULL);
    }
  | direct_declarator '[' type_qualifier_list ']'
    {
      $$ = ArrayDecl($1, $3, NULL);
    }
  | direct_declarator '[' assignment_expression ']'
    {
      $$ = ArrayDecl($1, NULL, $3);
    }
  | direct_declarator '[' type_qualifier_list assignment_expression ']'
    {
      $$ = ArrayDecl($1, $3, $4);
    }
  | direct_declarator '[' STATIC assignment_expression ']'
    {
      $$ = ArrayDecl($1, StClassSpec(SPEC_static), $4);
    }
  | direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'
    {
      $$ = ArrayDecl($1, Speclist_right( StClassSpec(SPEC_static), $4 ), $5);
    }
  | direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'
    {
      $$ = ArrayDecl($1, Speclist_left( $3, StClassSpec(SPEC_static) ), $5);
    }
  | direct_declarator '[' '*' ']'
    {
      $$ = ArrayDecl($1, Declspec(SPEC_star), NULL);
    }
  | direct_declarator '[' type_qualifier_list '*' ']'
    {
      $$ = ArrayDecl($1, Speclist_left( $3, Declspec(SPEC_star) ), NULL);
    }
  | direct_declarator '(' parameter_type_list ')'
    {
      $$ = FuncDecl($1, $3);
    }
  | direct_declarator '(' ')'
    {
      $$ = FuncDecl($1, NULL);
    }
  | direct_declarator '(' identifier_list ')'
    {
      $$ = FuncDecl($1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.7.5 */
pointer:
    '*'
    {
      $$ = Pointer();
    }
  | '*' type_qualifier_list
    {
      $$ = Speclist_right(Pointer(), $2);
    }
  | '*' pointer
    {
      $$ = Speclist_right(Pointer(), $2);
    }
  | '*' type_qualifier_list pointer
    {
      $$ = Speclist_right( Pointer(), Speclist_left($2, $3) );
    }
;

/*  ISO/IEC 9899:1999 6.7.5 */
type_qualifier_list:
    type_qualifier
    {
      $$ = $1;
    }
  | type_qualifier_list type_qualifier
    {
      $$ = Speclist_left($1, $2);
    }
;

/*  ISO/IEC 9899:1999 6.7.5 */
parameter_type_list:
    parameter_list
    {
      $$ = $1;
    }
  | parameter_list ',' ELLIPSIS
    {
      $$ = ParamList($1, Ellipsis());
    }
;

/*  ISO/IEC 9899:1999 6.7.5 */
parameter_list:
    parameter_declaration
    {
      $$ = $1;
    }
  | parameter_list ',' parameter_declaration
    {
      $$ = ParamList($1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.7.5 */
parameter_declaration:
    declaration_specifiers declarator
    {
      $$ = ParamDecl($1, $2);
    }
  | declaration_specifiers
    {
      $$ = ParamDecl($1, NULL);
    }
  | declaration_specifiers abstract_declarator
    {
      $$ = ParamDecl($1, $2);
    }
;

/*  ISO/IEC 9899:1999 6.7.5 */
identifier_list:
    IDENTIFIER
    {
      $$ = IdentifierDecl( Symbol($1) );
    }
  | identifier_list ',' IDENTIFIER
    {
      $$ = IdList($1, IdentifierDecl( Symbol($3) ));
    }
;

/*  ISO/IEC 9899:1999 6.7.6 */
type_name:
    specifier_qualifier_list
    {
      $$ = Casttypename($1, NULL);
    }
  | specifier_qualifier_list abstract_declarator
    {
      $$ = Casttypename($1, $2);
    }
;

/*  ISO/IEC 9899:1999 6.7.6 */
abstract_declarator:
    pointer
    {
      $$ = AbstractDeclarator($1, NULL);
    }
  | direct_abstract_declarator
    {
      $$ = AbstractDeclarator(NULL, $1);
    }
  | pointer direct_abstract_declarator
    {
      $$ = AbstractDeclarator($1, $2);
    }
;

/*  ISO/IEC 9899:1999 6.7.6 */
direct_abstract_declarator:
    '(' abstract_declarator ')'
    {
      $$ = ParenDecl($2);
    }
  | '[' ']'
    {
      $$ = ArrayDecl(NULL, NULL, NULL);
    }
  | direct_abstract_declarator '[' ']'
    {
      $$ = ArrayDecl($1, NULL, NULL);
    }
  | '[' assignment_expression ']'
    {
      $$ = ArrayDecl(NULL, NULL, $2);
    }
  | direct_abstract_declarator '[' assignment_expression ']'
    {
      $$ = ArrayDecl($1, NULL, $3);
    }
  | '[' '*' ']'
    {
      $$ = ArrayDecl(NULL, Declspec(SPEC_star), NULL);
    }
  | direct_abstract_declarator '[' '*' ']'
    {
      $$ = ArrayDecl($1, Declspec(SPEC_star), NULL);
    }
  | '(' ')'
    {
      $$ = FuncDecl(NULL, NULL);
    }
  | direct_abstract_declarator '(' ')'
    {
      $$ = FuncDecl($1, NULL);
    }
  | '(' parameter_type_list ')'
    {
      $$ = FuncDecl(NULL, $2);
    }
  | direct_abstract_declarator '(' parameter_type_list ')'
    {
      $$ = FuncDecl($1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.7.7 */
typedef_name:
    TYPE_NAME
    {
      $$ = Symbol($1);
    }
;

/*  ISO/IEC 9899:1999 6.7.8 */
initializer:
    assignment_expression
    {
      $$ = $1;
    }
  | '{' initializer_list '}'
    {
      $$ = BracedInitializer($2);
    }
  | '{' initializer_list ',' '}'
    {
      $$ = BracedInitializer($2);
    }
;

/*  ISO/IEC 9899:1999 6.7.8 */
initializer_list:
    initializer
    {
      $$ = $1;
    }
  | designation initializer
    {
      $$ = Designated($1, $2);
    }
  | initializer_list ',' initializer
    {
      $$ = CommaList($1, $3);
    }
  | initializer_list ',' designation initializer
    {
      $$ = CommaList($1, Designated($3, $4));
    }
;

/*  ISO/IEC 9899:1999 6.7.8 */
designation:
    designator_list '='
    {
      $$ = $1;
    }
;

/*  ISO/IEC 9899:1999 6.7.8 */
designator_list:
    designator
    {
      $$ = $1;
    }
  | designator_list designator
    {
      $$ = SpaceList($1, $2);
    }
;

/*  ISO/IEC 9899:1999 6.7.8 */
designator:
    '[' constant_expression ']'
    {
      $$ = IdxDesignator($2);
    }
  | '.' IDENTIFIER
    {
      $$ = DotDesignator( Symbol($2) );
    }
  | '.' typedef_name     /* artificial rule */
    {
      $$ = DotDesignator($2);
    }
;


/* -------------------------------------------------------------------------
 * ------------- ISO/IEC 9899:1999 A.2.3 Statements ------------------------
 * -------------------------------------------------------------------------
 */

/*  ISO/IEC 9899:1999 6.8 */
statement:
    labeled_statement
    {
      $$ = $1;
    }
  | compound_statement
    {
      $$ = $1;
    }
  | expression_statement
    {
      $$ = $1;
    }
  | selection_statement
    {
      $$ = $1;
    }
  | iteration_statement
    {
      $$ = $1;
    }
  | jump_statement
    {
      $$ = $1;
    }
  | openmp_construct // OpenMP Version 2.5 ISO/IEC 9899:1999 addition
    {
      $$ = OmpStmt($1);
      $$->l = $1->l;
    }
  | ompix_construct // OMPi extensions
    {
      $$ = OmpixStmt($1);
      $$->l = $1->l;
    }
;


statement_for_labeled:
    statement
    { 
      $$ = $1; 
    }
  | openmp_directive 
    {       
      $$ = OmpStmt($1);
      $$->l = $1->l;
    }
;


/*  ISO/IEC 9899:1999 6.8.1 */
labeled_statement:             /* Allow openmp_directive for CASE and DEFAULT */
    IDENTIFIER ':' statement
    {
      $$ = Labeled( Symbol($1), $3 );
    }
  | CASE constant_expression ':' statement_for_labeled
    {
      $$ = Case($2, $4);
    }
  | DEFAULT ':' statement_for_labeled
    {
      $$ = Default($3);
    }
;

/*  ISO/IEC 9899:1999 6.8.2 */
compound_statement:
    '{' '}'
    {
      $$ = Compound(NULL);
    }
  | '{'  { $<type>$ = sc_original_line()-1; scope_start(stab); }
       block_item_list '}'
    {
      $$ = Compound($3);
      scope_end(stab);
      $$->l = $<type>2;     /* Remember 1st line */
    }
;

/*  ISO/IEC 9899:1999 6.8.2 */
block_item_list:
    block_item
    {
      $$ = $1;
    }
  | block_item_list block_item
    {
      $$ = BlockList($1, $2);
      $$->l = $1->l;
    }
;

/*  ISO/IEC 9899:1999 6.8.2 */
block_item:
    declaration
    {
      $$ = $1;
    }
  | statement
    {
      $$ = $1;
    }
  | openmp_directive // OpenMP Version 2.5 ISvO/IEC 9899:1999 addition
    {
      $$ = OmpStmt($1);
      $$->l = $1->l;
    }
  | ompix_directive // ompi extensions
    {
      $$ = OmpixStmt($1);
      $$->l = $1->l;
    }
;

/*  ISO/IEC 9899:1999 6.8.3 */
expression_statement:
    ';'
    {
      $$ = Expression(NULL);
    }
  | expression ';'
    {
      $$ = Expression($1);
      $$->l = $1->l;
    }
;

/*  ISO/IEC 9899:1999 6.8.4 */
selection_statement:
    IF '(' expression ')' statement
    {
      $$ = If($3, $5, NULL);
    }
  | IF '(' expression ')' statement ELSE statement
    {
      $$ = If($3, $5, $7);
    }
  | SWITCH '(' expression ')' statement
    {
      $$ = Switch($3, $5);
    }
;

/*  ISO/IEC 9899:1999 6.8.5 */
/* (VVD) broke off the FOR part */
iteration_statement:
    WHILE '(' expression ')' statement
    {
      $$ = While($3, $5);
    }
  | DO statement WHILE '(' expression ')' ';'
    {
      $$ = Do($2, $5);
    }
  | iteration_statement_for
;

iteration_statement_for:
    FOR '(' ';' ';' ')' statement
    {
      $$ = For(NULL, NULL, NULL, $6);
    }
  | FOR '(' expression ';' ';' ')' statement
    {
      $$ = For(Expression($3), NULL, NULL, $7);
    }
  | FOR '(' ';' expression ';' ')' statement
    {
      $$ = For(NULL, $4, NULL, $7);
    }
  | FOR '(' ';' ';' expression ')' statement
    {
      $$ = For(NULL, NULL, $5, $7);
    }
  | FOR '(' expression ';' expression ';' ')' statement
    {
      $$ = For(Expression($3), $5, NULL, $8);
    }
  | FOR '(' expression ';' ';' expression ')' statement
    {
      $$ = For(Expression($3), NULL, $6, $8);
    }
  | FOR '(' ';' expression ';' expression ')' statement
    {
      $$ = For(NULL, $4, $6, $8);
    }
  | FOR '(' expression ';' expression ';' expression ')' statement
    {
      $$ = For(Expression($3), $5, $7, $9);
    }
  | FOR '(' declaration ';' ')' statement
    {
      $$ = For($3, NULL, NULL, $6);
    }
  | FOR '(' declaration expression ';' ')' statement
    {
      $$ = For($3, $4, NULL, $7);
    }
  | FOR '(' declaration ';' expression ')' statement
    {
      $$ = For($3, NULL, $5, $7);
    }
  | FOR '(' declaration expression ';' expression ')' statement
    {
      $$ = For($3, $4, $6, $8);
    }
;

/*  ISO/IEC 9899:1999 6.8.6 */
jump_statement:
    GOTO IDENTIFIER ';'
    {
      /* We don't keep track of labels -- we leave it to the native compiler */
      $$ = Goto( Symbol($2) );
    }
  | CONTINUE ';'
    {
      $$ = Continue();
    }
  | BREAK ';'
    {
      $$ = Break();
    }
  | RETURN ';'
    {
      //TODO: simple hack, not 100% correct, does not cover goto
      if (errorOnReturn)
        parse_error(1, "return statement not allowed in an outlined region\n");
      $$ = Return(NULL);
    }
  | RETURN expression ';'
    {
      //TODO: simple hack, not 100% correct, does not cover goto
      if (errorOnReturn)
        parse_error(1, "return statement not allowed in an outlined region\n");
      $$ = Return($2);
    }
;


/* -------------------------------------------------------------------------
 * ------ ISO/IEC 9899:1999 A.2.4 External definitions ---------------------
 * -------------------------------------------------------------------------
 */

/*  ISO/IEC 9899:1999 6.9 */
translation_unit:
    external_declaration
    {
      $$ = pastree = $1;
    }
  | translation_unit external_declaration
    {
      $$ = pastree = BlockList($1, $2);
    }
;

/*  ISO/IEC 9899:1999 6.9 */
external_declaration:
    function_definition
    {
      $$ = $1;
    }
  | declaration
    {
      $$ = $1;
    }
    /* Actually, although not in the grammar, we support 1 more option
     * here:  Verbatim
     */
  | ox_taskdef_construct
    {
      $$ = OmpixStmt($1);
    }
;

/*  ISO/IEC 9899:1999 6.9.1 */
/* We open a new scope which encloses the compound statement.
 * In there we will only declare the function parameters.
 * We break the rule into two subrules, for modularity.
 */
function_definition:
    normal_function_definition   { $$ = $1; }
  | oldstyle_function_definition { $$ = $1; }
;

normal_function_definition:
    declaration_specifiers declarator
    {
      if (isTypedef || $2->decl->type != DFUNC)
        parse_error(1, "function definition cannot be parsed.\n");
      if (symtab_get(stab, decl_getidentifier_symbol($2), FUNCNAME) == NULL)
        symtab_put(stab, decl_getidentifier_symbol($2), FUNCNAME);

      scope_start(stab);
      ast_declare_function_params($2);
    }
    compound_statement
    {
      scope_end(stab);
      check_for_main_and_declare($1, $2);
      $$ = FuncDef($1, $2, NULL, $4);
    }
  | declarator /* no return type */
    {
      if (isTypedef || $1->decl->type != DFUNC)
        parse_error(1, "function definition cannot be parsed.\n");
      if (symtab_get(stab, decl_getidentifier_symbol($1), FUNCNAME) == NULL)
        symtab_put(stab, decl_getidentifier_symbol($1), FUNCNAME);

      scope_start(stab);
      ast_declare_function_params($1);
    }
    compound_statement
    {
      astspec s = Declspec(SPEC_int);  /* return type defaults to "int" */

      scope_end(stab);
      check_for_main_and_declare(s, $1);
      $$ = FuncDef(s, $1, NULL, $3);
    }
;

oldstyle_function_definition:
    declaration_specifiers declarator /* oldstyle: params declared seperately */
    {
      if (isTypedef || $2->decl->type != DFUNC)
        parse_error(1, "function definition cannot be parsed.\n");
      if (symtab_get(stab, decl_getidentifier_symbol($2), FUNCNAME) == NULL)
        symtab_put(stab, decl_getidentifier_symbol($2), FUNCNAME);

      scope_start(stab);
      /* Notice here that the function parameters are declared through
       * the declaration_list and we need to do nothing else!
       */
    }
    declaration_list compound_statement
    {
      scope_end(stab);
      check_for_main_and_declare($1, $2);
      $$ = FuncDef($1, $2, $4, $5);
    }
  | declarator /* no return type & oldstyle: params declared seperately */
    {
      if (isTypedef || $1->decl->type != DFUNC)
        parse_error(1, "function definition cannot be parsed.\n");
      if (symtab_get(stab, decl_getidentifier_symbol($1), FUNCNAME) == NULL)
        symtab_put(stab, decl_getidentifier_symbol($1), FUNCNAME);

      scope_start(stab);
      /* Notice here that the function parameters are declared through
       * the declaration_list and we need to do nothing else!
       */
    }
    declaration_list compound_statement
    {
      astspec s = Declspec(SPEC_int);  /* return type defaults to "int" */

      scope_end(stab);
      check_for_main_and_declare(s, $1);
      $$ = FuncDef(s, $1, $3, $4);
    }
;

/*  ISO/IEC 9899:1999 6.9.1 */
declaration_list:
    declaration
    {
      $$ = $1;
    }
  | declaration_list declaration
    {
      $$ = BlockList($1, $2);         /* Same as block list */
    }
;


/* -------------------------------------------------------------------------
 * --- OpenMP Version 2.5 ISO/IEC 9899:1999 additions begin ----------------
 * -------------------------------------------------------------------------
 */


declaration_definition:
  //From external_declaration
    function_definition
    {
      $$ = $1;
    }
  | declaration
    {
      $$ = $1;
    }
;

// OpenMP V4.0
function_statement: // TODO
    function_definition
    {
      $$ = $1;
    }
    //TODO function declaration
;

// OpenMP V4.0
declarations_definitions_seq:
    declaration_definition
    {
      $$ = $1;
    }
  | declarations_definitions_seq declaration_definition
    {
      $$ = pastree = BlockList($1, $2);
    }
;

openmp_construct:
    parallel_construct
    {
      $$ = $1;
    }
  | for_construct
    {
      $$ = $1;
    }
  | sections_construct
    {
      $$ = $1;
    }
  | single_construct
    {
      $$ = $1;
    }
  | parallel_for_construct
    {
      $$ = $1;
    }
  | parallel_sections_construct
    {
      $$ = $1;
    }
  | master_construct
    {
      $$ = $1;
    }
  | critical_construct
    {
      $$ = $1;
    }
  | atomic_construct
    {
      $$ = $1;
    }
  | ordered_construct
    {
      $$ = $1;
    }
  | /* OpenMP V3.0 */
    task_construct
    {
      $$ = $1;
    }
  | /* OpenMP V4.0 */
    simd_construct
    {
      $$ = $1;
    }
  | for_simd_construct
    {
      $$ = $1;
    }
  | parallel_for_simd_construct
    {
      $$ = $1;
    }
  | target_data_construct
    {
      $$ = $1;
    }
  | target_construct
    {
      $$ = $1;
    }
  | target_update_construct
    {
      $$ = $1;
    }
  | teams_construct
    {
      $$ = $1;
    }
  | distribute_construct
    {
      $$ = $1;
    }
  | distribute_simd_construct
    {
      $$ = $1;
    }
  | distribute_parallel_for_construct
    {
      $$ = $1;
    }
  | distribute_parallel_for_simd_construct
    {
      $$ = $1;
    }
  | target_teams_construct
    {
      $$ = $1;
    }
  | teams_distribute_construct
    {
      $$ = $1;
    }
  | teams_distribute_simd_construct
    {
      $$ = $1;
    }
  | target_teams_distribute_construct
    {
      $$ = $1;
    }
  | target_teams_distribute_simd_construct
    {
      $$ = $1;
    }
  | teams_distribute_parallel_for_construct
    {
      $$ = $1;
    }
  | target_teams_distribute_parallel_for_construct
    {
      $$ = $1;
    }
  | teams_distribute_parallel_for_simd_construct
    {
      $$ = $1;
    }
  | target_teams_distribute_parallel_for_simd_construct
    {
      $$ = $1;
    }
  | /* OpenMP V4.0 2.12.5 page 126 */
    taskgroup_construct //TODO not sure where it is supposed to be
    {
      $$ = $1;
    }
;

openmp_directive:
        /*
    pomp_construct
    {
      $$ = $1;
    }
  | */
    /* We create constructs out of the next directive-only rules,
     * for uniformity.
     */
    barrier_directive
    {
      $$ = OmpConstruct(DCBARRIER, $1, NULL);
    }
  | flush_directive
    {
      $$ = OmpConstruct(DCFLUSH, $1, NULL);
    }
  | /* OpenMP V3.0 */
    taskwait_directive
    {
      $$ = OmpConstruct(DCTASKWAIT, $1, NULL);
    }
  | /* OpenMP V3.1 */
    taskyield_directive
    {
      $$ = OmpConstruct(DCTASKYIELD, $1, NULL);
    }
  | /* OpenMP V4.0 2.13.1 page 140 */
    cancel_directive
    {
      $$ = OmpConstruct(DCCANCEL, $1, NULL);
    }
  | /* OpenMP V4.0 2.13.2 page 143 */
    cancellation_point_directive
    {
      $$ = OmpConstruct(DCCANCELLATIONPOINT, $1, NULL);
    }
;

structured_block:
    statement
    {
      $$ = $1;
    }
;

parallel_construct:
    parallel_directive structured_block
    {
      $$ = OmpConstruct(DCPARALLEL, $1, $2);
      $$->l = $1->l;
    }
;

parallel_directive:
    PRAGMA_OMP OMP_PARALLEL parallel_clause_optseq '\n'
    {
      $$ = OmpDirective(DCPARALLEL, $3);
    }
;

parallel_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | parallel_clause_optseq parallel_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | parallel_clause_optseq ',' parallel_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

parallel_clause:
    unique_parallel_clause
    {
      $$ = $1;
    }
  | data_default_clause
    {
      $$ = $1;
    }
  | data_privatization_clause
    {
      $$ = $1;
    }
  | data_privatization_in_clause
    {
      $$ = $1;
    }
  | data_sharing_clause
    {
      $$ = $1;
    }
  | data_reduction_clause
    {
      $$ = $1;
    }
;

unique_parallel_clause:
    if_clause
    {
      $$ = $1;
    }
  | OMP_NUMTHREADS '(' { sc_pause_openmp(); } expression ')'
    {
      sc_start_openmp();
      $$ = NumthreadsClause($4);
    }
  | OMP_COPYIN { sc_pause_openmp(); } '(' variable_list ')'
    {
      sc_start_openmp();
      $$ = VarlistClause(OCCOPYIN, $4);
    }
  |  /* OpenMP V4.0 */
    procbind_clause  //TODO it's not mentioned anywhere in the OpenMP v4 grammmar
    {
      $$ = $1;
    }
  |  /* Clause added for Aggelo's auto scoping */
    OMP_AUTO { sc_pause_openmp(); } '(' variable_list ')'
    {
      sc_start_openmp();
      $$ = VarlistClause(OCAUTO, $4);
    }
;

for_construct:
    for_directive iteration_statement_for
    {
      $$ = OmpConstruct(DCFOR, $1, $2);
    }
;

for_directive:
    PRAGMA_OMP OMP_FOR for_clause_optseq '\n'
    {
      $$ = OmpDirective(DCFOR, $3);
    }
;

for_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | for_clause_optseq for_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | for_clause_optseq ',' for_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

for_clause:
    unique_for_clause
    {
      $$ = $1;
    }
  | data_privatization_clause
    {
      $$ = $1;
    }
  | data_privatization_in_clause
    {
      $$ = $1;
    }
  | data_privatization_out_clause
    {
      $$ = $1;
    }
  | data_reduction_clause
    {
      $$ = $1;
    }
  | OMP_NOWAIT
    {
      $$ = PlainClause(OCNOWAIT);
    }
;

unique_for_clause:
    OMP_ORDERED
    {
      $$ = PlainClause(OCORDERED);
    }
  | OMP_SCHEDULE '(' schedule_kind ')'
    {
      $$ = ScheduleClause($3, NULL);
    }
  | OMP_SCHEDULE '(' schedule_kind ',' { sc_pause_openmp(); } expression ')'
    {
      sc_start_openmp();
      if ($3 == OC_runtime)
        parse_error(1, "\"runtime\" schedules may not have a chunksize.\n");
      $$ = ScheduleClause($3, $6);
    }
  | OMP_SCHEDULE '(' OMP_AFFINITY ','
    {  /* non-OpenMP schedule */
      tempsave = checkDecls;
      checkDecls = 0;   /* Because the index of the loop is usualy involved */
      sc_pause_openmp();
    }
    expression ')'
    {
      sc_start_openmp();
      checkDecls = tempsave;
      $$ = ScheduleClause(OC_affinity, $6);
    }
  | collapse_clause
    {
      $$ = $1;
    }
;

schedule_kind:
    OMP_STATIC
    {
      $$ = OC_static;
    }
  | OMP_DYNAMIC
    {
      $$ = OC_dynamic;
    }
  | OMP_GUIDED
    {
      $$ = OC_guided;
    }
  | OMP_RUNTIME
    {
      $$ = OC_runtime;
    }
  | OMP_AUTO      /* OpenMP 3.0 */
    {
      $$ = OC_auto;
    }
  | error { parse_error(1, "invalid openmp schedule type.\n"); }

;

sections_construct:
    sections_directive section_scope
    {
      $$ = OmpConstruct(DCSECTIONS, $1, $2);
    }
;

sections_directive:
    PRAGMA_OMP OMP_SECTIONS sections_clause_optseq '\n'
    {
      $$ = OmpDirective(DCSECTIONS, $3);
    }
;

sections_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | sections_clause_optseq sections_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | sections_clause_optseq ',' sections_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

sections_clause:
    data_privatization_clause
    {
      $$ = $1;
    }
  | data_privatization_in_clause
    {
      $$ = $1;
    }
  | data_privatization_out_clause
    {
      $$ = $1;
    }
  | data_reduction_clause
    {
      $$ = $1;
    }
  | OMP_NOWAIT
    {
      $$ = PlainClause(OCNOWAIT);
    }
;

section_scope:
    '{' section_sequence '}'
    {
      $$ = Compound($2);
    }
;

section_sequence:
    structured_block  // 1 shift/reduce conflict here
    {
      /* Make it look like it had a section pragma */
      $$ = OmpStmt( OmpConstruct(DCSECTION, OmpDirective(DCSECTION,NULL), $1) );
    }
  | section_directive structured_block
    {
      $$ = OmpStmt( OmpConstruct(DCSECTION, $1, $2) );
    }
  | section_sequence section_directive structured_block
    {
      $$ = BlockList($1, OmpStmt( OmpConstruct(DCSECTION, $2, $3) ));
    }
;

section_directive:
    PRAGMA_OMP OMP_SECTION '\n'
    {
      $$ = OmpDirective(DCSECTION, NULL);
    }
;

single_construct:
    single_directive structured_block
    {
      $$ = OmpConstruct(DCSINGLE, $1, $2);
    }
;

single_directive:
    PRAGMA_OMP OMP_SINGLE single_clause_optseq '\n'
    {
      $$ = OmpDirective(DCSINGLE, $3);
    }
;

single_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | single_clause_optseq single_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | single_clause_optseq ',' single_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

single_clause:
    unique_single_clause
    {
      $$ = $1;
    }
  | data_privatization_clause
    {
      $$ = $1;
    }
  | data_privatization_in_clause
    {
      $$ = $1;
    }
  | OMP_NOWAIT
    {
      $$ = PlainClause(OCNOWAIT);
    }
;

unique_single_clause:
    OMP_COPYPRIVATE  { sc_pause_openmp(); } '(' variable_list ')'
    {
      sc_start_openmp();
      $$ = VarlistClause(OCCOPYPRIVATE, $4);
    }
;

/* OpenMP V4.0 2.8.1 page 68 */
simd_construct:
    simd_directive iteration_statement_for
    {
      //$$ = OmpConstruct(DCSIMD, $1, $2); TODO DCSIMD
    }
;

/* OpenMP V4.0 2.8.1 page 68 */
simd_directive:
    PRAGMA_OMP OMP_SIMD simd_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCSIMD, $3); TODO DCSIMD
    }
;

simd_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | simd_clause_optseq simd_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | simd_clause_optseq ',' simd_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

/* OpenMP V4.0 2.8.1 page 68 */
simd_clause:
    unique_simd_clause
    {
      $$ = $1;
    }
  | data_privatization_clause
    {
      $$ = $1;
    }
  | data_privatization_out_clause
    {
      $$ = $1;
    }
  | data_reduction_clause
    {
      $$ = $1;
    }
  | collapse_clause
    {
      $$ = $1;
    }
;

unique_simd_clause:
    /* OpenMP V4.0 2.8.1 page 70 */
    OMP_SAFELEN '(' expression /* CONSTANT */ ')'
    {
      int n = 0, er = 0;
      if (xar_expr_is_constant($3))
      {
        n = xar_calc_int_expr($3, &er);
        if (er) n = 0;
      }
      if (n <= 0)
        parse_error(1, "invalid number in simdlen() clause.\n");
      //$$ = CollapseClause(n); //TODO SAFELEN
    }
  | linear_clause
    {
      $$ = $1;
    }
  | aligned_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.8.2 page 74 */
inbranch_clause:
    OMP_INBRANCH
    {
      //$$ = PlainClause(OCINBRANCH); TODO ast
    }
  | OMP_NOTINBRANCH
    {
      //$$ = PlainClause(OCNOTINBRANCH); TODO ast
    }
;

/* OpenMP V4.0 2.8.2 page 73 */
uniform_clause:
    OMP_UNIFORM { sc_pause_openmp(); } '(' variable_list ')'
    {
      sc_start_openmp();
      //$$ = VarlistClause(OCUNIFORM, $4); TODO ast
    }
;

/* OpenMP V4.0 2.14.3.7 page 172 */
linear_clause:
    OMP_LINEAR { sc_pause_openmp(); } '(' variable_list optional_expression ')'
    {
      sc_start_openmp();
      // TODO ast
    }
;

/* OpenMP V4.0 2.8.1 page 70 and 2.8.2 page 74 */
aligned_clause: //The type of list items appearing in the aligned clause must be array or pointer.
    OMP_ALIGNED { sc_pause_openmp(); } '(' variable_list optional_expression ')'
    {
      sc_start_openmp();
      // TODO ast
    }
;

optional_expression:
    // empty
    {
      $$ = NULL;
    }
  |  ':' expression
    {
      // TODO ast
    }
;

/* OpenMP V4.0 2.8.2 page 72 */
declare_simd_construct: // TODO doesn't seem to be called from anywhere
    declare_simd_directive_seq function_statement
    {
      //$$ = OmpConstruct(DCDECLSIMD, $1, $2); TODO DCDECLSIMD or change it to stmt
    }
;

/* OpenMP V4.0 2.8.2 page 72 */
declare_simd_directive_seq:
    declare_simd_directive
    {
      //TODO
    }
  | declare_simd_directive_seq declare_simd_directive
    {
        //TODO
    }
;

/* OpenMP V4.0 2.8.2 page 72 */
declare_simd_directive:
    PRAGMA_OMP OMP_DECLARE OMP_SIMD declare_simd_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCDECLSIMD, $4); TODO DCDECLSIMD
    }
;

declare_simd_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | declare_simd_clause_optseq declare_simd_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | declare_simd_clause_optseq ',' declare_simd_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

/* OpenMP V4.0 2.8.2 page 72 */
declare_simd_clause:
    /* OpenMP V4.0 2.8.2 page 73 */
    OMP_SIMDLEN '(' expression /* CONSTANT */ ')'
    {
      int n = 0, er = 0;
      if (xar_expr_is_constant($3))
      {
        n = xar_calc_int_expr($3, &er);
        if (er) n = 0;
      }
      if (n <= 0)
        parse_error(1, "invalid number in simdlen() clause.\n");
      //$$ = CollapseClause(n); //TODO SIMDLEN
    }
  | linear_clause
    {
      $$ = $1;
    }
  | aligned_clause
    {
      $$ = $1;
    }
  | uniform_clause
    {
      $$ = $1;
    }
  | inbranch_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.8.3 page 76 */
for_simd_construct:
    for_simd_directive iteration_statement_for
    {
      //$$ = OmpConstruct(DCFORSIMD, $1, $2); TODO DCFORSIMD
    }
;

/* OpenMP V4.0 2.8.3 page 76 */
for_simd_directive:
    PRAGMA_OMP OMP_FOR OMP_SIMD for_simd_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCFORSIMD, $4); TODO DCFORSIMD
    }
;

for_simd_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | for_simd_clause_optseq for_simd_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | for_simd_clause_optseq ',' for_simd_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

for_simd_clause:
    for_clause
    {
      $$ = $1;
    }
  | unique_simd_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.10.4 page 100 */
parallel_for_simd_construct:
    parallel_for_simd_directive iteration_statement_for
    {
      //$$ = OmpConstruct(DCPARFORSIMD, $1, $2); TODO DCPARFORSIMD
    }
;

/* OpenMP V4.0 2.10.4 page 100 */
parallel_for_simd_directive:
    PRAGMA_OMP OMP_PARALLEL OMP_FOR OMP_SIMD parallel_for_simd_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCPARFORSIMD, $5); TODO DCFORSIMD
    }
;

parallel_for_simd_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | parallel_for_simd_clause_optseq parallel_for_simd_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | parallel_for_simd_clause_optseq ',' parallel_for_simd_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

parallel_for_simd_clause:
    parallel_for_clause
    {
      $$ = $1;
    }
  | unique_simd_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.9.1 page 77 */
target_data_construct:
    target_data_directive structured_block
    {
      $$ = OmpConstruct(DCTARGETDATA, $1, $2);
    }
;

/* OpenMP V4.0 2.9.1 page 77 */
target_data_directive:
    PRAGMA_OMP OMP_TARGET OMP_DATA target_data_clause_optseq '\n'
    {
      $$ = OmpDirective(DCTARGETDATA, $4);
    }
;

target_data_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | target_data_clause_optseq target_data_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | target_data_clause_optseq ',' target_data_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

target_data_clause:
    device_clause
    {
      $$ = $1;
    }
  | map_clause
    {
      $$ = $1;
    }
  | if_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.9.1-3 77-83 */
device_clause:
    OMP_DEVICE '(' { sc_pause_openmp(); } expression ')'
    {
      sc_start_openmp();
      $$ = DeviceClause($4);
    }
;

/* OpenMP V4.0 2.14.5 page 177 */
map_clause:
    OMP_MAP '(' map_type ':' { sc_pause_openmp(); } variable_array_section_list ')'
    {
      sc_start_openmp();
      $$ = MapClause($3, $6);
    }
  | OMP_MAP '(' { sc_pause_openmp(); } variable_array_section_list ')'
    {
      sc_start_openmp();
	  $$ = MapClause(OC_tofrom, $4);
    }
;

/* OpenMP V4.0 2.14.5 page 178 */
map_type:
    OMP_ALLOC
    {
      $$ = OC_alloc;
    }
  | OMP_TO
    {
      $$ = OC_to;
    }
  | OMP_FROM
    {
      $$ = OC_from;
    }
  | OMP_TOFROM
    {
      $$ = OC_tofrom;
    }
;

/* OpenMP V4.0 2.9.2 page 79 */
target_construct:
    target_directive { $<type>$ = errorOnReturn;  errorOnReturn = 1; } 
    structured_block
    {
      errorOnReturn = $<type>2;
      $$ = OmpConstruct(DCTARGET, $1, $3);
      __has_target = 1;
    }
;

/* OpenMP V4.0 2.9.2 page 79 */
target_directive:
    PRAGMA_OMP OMP_TARGET target_clause_optseq '\n'
    {
      $$ = OmpDirective(DCTARGET, $3);
    }
;

target_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | target_clause_optseq target_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | target_clause_optseq ',' target_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

target_clause: //TODO same as target_data_clause???
    unique_target_clause
    {
      $$ = $1;
    }
  | if_clause
    {
      $$ = $1;
    }
;

unique_target_clause:
    device_clause
    {
      $$ = $1;
    }
  | map_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.9.3 page 81 */
target_update_construct:
    target_update_directive
    {
      $$ = OmpConstruct(DCTARGETUPD, $1, NULL);
    }
;

/* OpenMP V4.0 2.9.3 page 81 */
target_update_directive:
    PRAGMA_OMP OMP_TARGET OMP_UPDATE target_update_clause_seq '\n'
    {
      $$ = OmpDirective(DCTARGETUPD, $4);
    }
;

target_update_clause_seq:
    target_update_clause
    {
      $$ = $1;
    }
  | target_update_clause_seq target_update_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | target_update_clause_seq ',' target_update_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

target_update_clause:
    motion_clause
    {
      $$ = $1;
    }
  | device_clause
    {
      $$ = $1;
    }
  | if_clause
    {
      $$ = $1;
    }
;

motion_clause:
    OMP_TO { sc_pause_openmp(); } '(' variable_array_section_list ')'
    {
      sc_start_openmp();
      $$ = VarlistClause(OCTO, $4);
    }
  | OMP_FROM { sc_pause_openmp(); } '(' variable_array_section_list ')'
    {
      sc_start_openmp();
      $$ = VarlistClause(OCFROM, $4);
    }
;

/* OpenMP V4.0 2.9.4 page 83 */
declare_target_construct:
    declare_target_directive declarations_definitions_seq end_declare_target_directive
    {
      $$ = OmpConstruct(DCDECLTARGET, $1, $2);
    }
;

declare_target_directive:
    PRAGMA_OMP OMP_DECLARE OMP_TARGET'\n'
    {
      $$ = OmpDirective(DCDECLTARGET, NULL);
    }
;

end_declare_target_directive:
    PRAGMA_OMP OMP_END OMP_DECLARE OMP_TARGET'\n'
    {
      //$$ = OmpDirective(DCENDDECLTARGET, NULL); TODO DCENDDECLTARGET
    }
;

/* OpenMP V4.0 2.9.5 page 86 */
teams_construct:
    teams_directive structured_block
    {
      //$$ = OmpConstruct(DCTEAMS, $1, $2); TODO DCTEAMS
    }
;

/* OpenMP V4.0 2.9.5 page 86 */
teams_directive:
    PRAGMA_OMP OMP_TEAMS teams_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCTEAMS, $3); TODO DCTEAMS
    }
;

teams_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | teams_clause_optseq teams_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | teams_clause_optseq ',' teams_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

teams_clause:
    unique_teams_clause
    {
      $$ = $1;
    }
  | data_default_clause
    {
      $$ = $1;
    }
  | data_privatization_clause
    {
      $$ = $1;
    }
  | data_privatization_in_clause
    {
      $$ = $1;
    }
  | data_sharing_clause
    {
      $$ = $1;
    }
  | data_reduction_clause
    {
      $$ = $1;
    }
;

unique_teams_clause:
    /* OpenMP V4.0 2.9.5 page 87 */
    OMP_NUMTEAMS '(' { sc_pause_openmp(); } expression ')'
    {
      sc_start_openmp();
      //$$ = NumthreadsClause($4); //TODO check if numthreads is good or if I should make something new
    }
  | /* OpenMP V4.0 2.9.5 page 87 */
    OMP_THREADLIMIT '(' { sc_pause_openmp(); } expression ')'
    {
      sc_start_openmp();
      //$$ = NumthreadsClause($4); //TODO check if numthreads is good or if I should make something new
    }
;

/* OpenMP V4.0 OpenMP V4.0 2.9.6 page 88 */
distribute_construct:
    distribute_directive iteration_statement_for
    {
      //$$ = OmpConstruct(DCDISTRIBUTE, $1, $2); TODO DCDISTRIBUTE
    }
;

/* OpenMP V4.0 OpenMP V4.0 2.9.6 page 88 */
distribute_directive:
    PRAGMA_OMP OMP_DISTRIBUTE distribute_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCDISTRIBUTE, $3); TODO DCDISTRIBUTE
    }
;

distribute_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | distribute_clause_optseq distribute_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | distribute_clause_optseq ',' distribute_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

distribute_clause:
    data_privatization_clause
    {
      $$ = $1;
    }
  | data_privatization_in_clause
    {
      $$ = $1;
    }
  | collapse_clause
    {
      $$ = $1;
    }
  | unique_distribute_clause
    {
      $$ = $1;
    }
;

unique_distribute_clause:
    OMP_DISTSCHEDULE '(' OMP_STATIC ')'
    {
      $$ = ScheduleClause(OC_static, NULL);
    }
  | OMP_DISTSCHEDULE '(' OMP_STATIC ',' { sc_pause_openmp(); } expression ')'
    {
      sc_start_openmp();
      $$ = ScheduleClause(OC_static, $6);
    }
;

/* OpenMP V4.0 2.9.7 page 91 */
distribute_simd_construct:
    distribute_simd_directive iteration_statement_for
    {
      //$$ = OmpConstruct(DCDISTSIMD, $1, $2); TODO DCDISTSIMD
      //$$->l = $1->l;
    }
;

/* OpenMP V4.0 2.9.7 page 91 */
distribute_simd_directive:
    PRAGMA_OMP OMP_DISTRIBUTE OMP_SIMD distribute_simd_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCDISTSIMD, $4); TODO DCDISTSIMD
    }
;

distribute_simd_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | distribute_simd_clause_optseq distribute_simd_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | distribute_simd_clause_optseq ',' distribute_simd_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

distribute_simd_clause:
    unique_distribute_clause
    {
      $$ = $1;
    }
  | data_privatization_in_clause
    {
      $$ = $1;
    }
  | simd_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.9.8 page 92 */
distribute_parallel_for_construct:
    distribute_parallel_for_directive iteration_statement_for
    {
      //$$ = OmpConstruct(DCDISTPARFOR, $1, $2); TODO DCDISTPARFOR
      //$$->l = $1->l;
    }
;

/* OpenMP V4.0 2.9.8 page 92 */
distribute_parallel_for_directive:
    PRAGMA_OMP OMP_DISTRIBUTE OMP_PARALLEL OMP_FOR distribute_parallel_for_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCDISTPARFOR, $5); TODO DCDISTPARFOR
    }
;

distribute_parallel_for_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | distribute_parallel_for_clause_optseq distribute_parallel_for_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | distribute_parallel_for_clause_optseq ',' distribute_parallel_for_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

distribute_parallel_for_clause:
    unique_distribute_clause
    {
      $$ = $1;
    }
  | parallel_for_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.9.9 page 94 */
distribute_parallel_for_simd_construct:
    distribute_parallel_for_simd_directive structured_block
    {
      //$$ = OmpConstruct(DCDISTPARFORSIMD, $1, $2); TODO DCDISTPARFORSIMD
      //$$->l = $1->l;
    }
;

/* OpenMP V4.0 2.9.9 page 94  */
distribute_parallel_for_simd_directive:
    PRAGMA_OMP OMP_DISTRIBUTE OMP_PARALLEL OMP_FOR OMP_SIMD distribute_parallel_for_simd_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCDISTPARFORSIMD, $6); TODO DCDISTPARFORSIMD
    }
;

distribute_parallel_for_simd_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | distribute_parallel_for_simd_clause_optseq distribute_parallel_for_simd_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | distribute_parallel_for_simd_clause_optseq ',' distribute_parallel_for_simd_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

distribute_parallel_for_simd_clause:
    unique_distribute_clause
    {
      $$ = $1;
    }
  | parallel_for_simd_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.10.5 page 101 */
target_teams_construct:
    target_teams_directive structured_block //TODO find out if structured-block or iteration-statement
    {
      //$$ = OmpConstruct(DCTARGETTEAMS, $1, $2); TODO DCTARGETTEAMS
    }
;

/* OpenMP V4.0 2.10.5 page 101 */
target_teams_directive:
    PRAGMA_OMP OMP_TARGET OMP_TEAMS target_teams_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCTARGETTEAMS, $4); TODO DCTARGETTEAMS
    }
;

target_teams_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | target_teams_clause_optseq target_teams_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | target_teams_clause_optseq ',' target_teams_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

target_teams_clause:
    target_clause
    {
      $$ = $1;
    }
  | teams_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.10.6 page 102 */
teams_distribute_construct:
    teams_distribute_directive iteration_statement_for
    {
      //$$ = OmpConstruct(DCTEAMSDIST, $1, $2); TODO DCTEAMSDIST
    }
;

/* OpenMP V4.0 2.10.6 page 102 */
teams_distribute_directive:
    PRAGMA_OMP OMP_TEAMS OMP_DISTRIBUTE teams_distribute_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCTEAMSDIST, $4); TODO DCTEAMSDIST
    }
;

teams_distribute_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | teams_distribute_clause_optseq teams_distribute_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | teams_distribute_clause_optseq ',' teams_distribute_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

teams_distribute_clause:
    teams_clause
    {
      $$ = $1;
    }
  | unique_distribute_clause
    {
      $$ = $1;
    }
  | collapse_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.10.7 page 104 */
teams_distribute_simd_construct:
    teams_distribute_simd_directive iteration_statement_for
    {
      //$$ = OmpConstruct(DCTEAMSDISTSIMD, $1, $2); TODO DCTEAMSDISTSIMD
    }
;

/* OpenMP V4.0 2.10.7 page 104 */
teams_distribute_simd_directive:
    PRAGMA_OMP OMP_TEAMS OMP_DISTRIBUTE OMP_SIMD teams_distribute_simd_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCTEAMSDISTSIMD, $5); TODO DCTEAMSDISTSIMD
    }
;

teams_distribute_simd_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | teams_distribute_simd_clause_optseq teams_distribute_simd_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | teams_distribute_simd_clause_optseq ',' teams_distribute_simd_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

teams_distribute_simd_clause:
    unique_teams_clause
    {
      $$ = $1;
    }
  | data_default_clause
    {
      $$ = $1;
    }
  | data_sharing_clause
    {
      $$ = $1;
    }
  | distribute_simd_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.10.8 page 105 */
target_teams_distribute_construct:
    target_teams_distribute_directive iteration_statement_for
    {
      //$$ = OmpConstruct(DCTARGETTEAMSDIST, $1, $2); TODO DCTARGETTEAMSDIST
    }
;

/* OpenMP V4.0 2.10.8 page 105 */
target_teams_distribute_directive:
    PRAGMA_OMP OMP_TARGET OMP_TEAMS OMP_DISTRIBUTE target_teams_distribute_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCTARGETTEAMSDIST, $5); TODO DCTARGETTEAMSDIST
    }
;

target_teams_distribute_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | target_teams_distribute_clause_optseq target_teams_distribute_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | target_teams_distribute_clause_optseq ',' target_teams_distribute_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

target_teams_distribute_clause:
    target_clause
    {
      $$ = $1;
    }
  | teams_distribute_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.10.9 page 106 */
target_teams_distribute_simd_construct:
    target_teams_distribute_simd_directive iteration_statement_for
    {
      //$$ = OmpConstruct(DCTARGETTEAMSDISTSIMD, $1, $2); TODO DCTARGETTEAMSDISTSIMD
    }
;

/* OpenMP V4.0 2.10.9 page 106 */
target_teams_distribute_simd_directive:
    PRAGMA_OMP OMP_TARGET OMP_TEAMS OMP_DISTRIBUTE OMP_SIMD
    target_teams_distribute_simd_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCTARGETTEAMSDISTSIMD, $6); TODO DCTARGETTEAMSDISTSIMD
    }
;

target_teams_distribute_simd_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | target_teams_distribute_simd_clause_optseq
    target_teams_distribute_simd_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | target_teams_distribute_simd_clause_optseq ','
    target_teams_distribute_simd_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

target_teams_distribute_simd_clause:
    target_clause
    {
      $$ = $1;
    }
  | teams_distribute_simd_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.10.10 page 107 */
teams_distribute_parallel_for_construct:
    teams_distribute_parallel_for_directive iteration_statement_for
    {
      //$$ = OmpConstruct(DCTEAMSDISTPARFOR, $1, $2); TODO DCTEAMSDISTPARFOR
      //$$->l = $1->l;
    }
;

/* OpenMP V4.0 2.10.10 page 107 */
teams_distribute_parallel_for_directive:
    PRAGMA_OMP OMP_TEAMS OMP_DISTRIBUTE OMP_PARALLEL OMP_FOR
    teams_distribute_parallel_for_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCTEAMSDISTPARFOR, $3); TODO DCTEAMSDISTPARFOR
    }
;

teams_distribute_parallel_for_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | teams_distribute_parallel_for_clause_optseq
    teams_distribute_parallel_for_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | teams_distribute_parallel_for_clause_optseq ','
    teams_distribute_parallel_for_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

teams_distribute_parallel_for_clause:
    unique_teams_clause
    {
      $$ = $1;
    }
  | distribute_parallel_for_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.10.11 page 109 */
target_teams_distribute_parallel_for_construct:
    target_teams_distribute_parallel_for_directive iteration_statement_for
    {
      //$$ = OmpConstruct(DCTARGETTEAMSDISTPARFOR, $1, $2); TODO DCTARGETTEAMSDISTPARFOR
      //$$->l = $1->l;
    }
;

/* OpenMP V4.0 2.10.11 page 109 */
target_teams_distribute_parallel_for_directive:
    PRAGMA_OMP OMP_TARGET OMP_TEAMS OMP_DISTRIBUTE OMP_PARALLEL OMP_FOR target_teams_distribute_parallel_for_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCTARGETTEAMSDISTPARFOR, $7); TODO DCTARGETTEAMSDISTPARFOR
    }
;

target_teams_distribute_parallel_for_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | target_teams_distribute_parallel_for_clause_optseq target_teams_distribute_parallel_for_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | target_teams_distribute_parallel_for_clause_optseq ',' target_teams_distribute_parallel_for_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

target_teams_distribute_parallel_for_clause:
    unique_target_clause
    {
      $$ = $1;
    }
  | teams_distribute_parallel_for_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.10.12 page 110 */
teams_distribute_parallel_for_simd_construct:
    teams_distribute_parallel_for_simd_directive iteration_statement_for
    {
      //$$ = OmpConstruct(DCTEAMSDISTPARFORSIMD, $1, $2); TODO DCTEAMSDISTPARFORSIMD
      //$$->l = $1->l;
    }
;

/* OpenMP V4.0 2.10.12 page 110 */
teams_distribute_parallel_for_simd_directive:
    PRAGMA_OMP OMP_TEAMS OMP_DISTRIBUTE OMP_PARALLEL OMP_FOR OMP_SIMD teams_distribute_parallel_for_simd_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCTEAMSDISTPARFORSIMD, $7); TODO DCTEAMSDISTPARFORSIMD
    }
;

teams_distribute_parallel_for_simd_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | teams_distribute_parallel_for_simd_clause_optseq teams_distribute_parallel_for_simd_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | teams_distribute_parallel_for_simd_clause_optseq ',' teams_distribute_parallel_for_simd_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

teams_distribute_parallel_for_simd_clause:
    unique_teams_clause
    {
      $$ = $1;
    }
  | distribute_parallel_for_simd_clause
    {
      $$ = $1;
    }
;

/* OpenMP V4.0 2.10.13 page 111 */
target_teams_distribute_parallel_for_simd_construct:
    target_teams_distribute_parallel_for_simd_directive iteration_statement_for
    {
      //$$ = OmpConstruct(DCTARGETTEAMSDISTPARFORSIMD, $1, $2); TODO DCTARGETTEAMSDISTPARFORSIMD
      //$$->l = $1->l;
    }
;

/* OpenMP V4.0 2.10.13 page 111 */
target_teams_distribute_parallel_for_simd_directive:
    PRAGMA_OMP OMP_TARGET OMP_TEAMS OMP_DISTRIBUTE OMP_PARALLEL OMP_FOR OMP_SIMD target_teams_distribute_parallel_for_simd_clause_optseq '\n'
    {
      //$$ = OmpDirective(DCTARGETTEAMSDISTPARFORSIMD, $8); TODO DCTARGETTEAMSDISTPARFORSIMD
    }
;

target_teams_distribute_parallel_for_simd_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | target_teams_distribute_parallel_for_simd_clause_optseq target_teams_distribute_parallel_for_simd_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | target_teams_distribute_parallel_for_simd_clause_optseq ',' target_teams_distribute_parallel_for_simd_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

target_teams_distribute_parallel_for_simd_clause:
    unique_target_clause
    {
      $$ = $1;
    }
  | teams_distribute_parallel_for_simd_clause
    {
      $$ = $1;
    }
;

/* OpenMP V3.0 */
task_construct:
    task_directive structured_block
    {
      $$ = OmpConstruct(DCTASK, $1, $2);
      $$->l = $1->l;
    }
;

/* OpenMP V3.0 */
task_directive:
    PRAGMA_OMP OMP_TASK task_clause_optseq '\n'
    {
      $$ = OmpDirective(DCTASK, $3);
    }
;

/* OpenMP V3.0 */
task_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | task_clause_optseq task_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | task_clause_optseq ',' task_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

/* OpenMP V3.0 */
task_clause:
    unique_task_clause
    {
      $$ = $1;
    }
  | data_default_clause
    {
      $$ = $1;
    }
  | data_privatization_clause
    {
      $$ = $1;
    }
  | data_privatization_in_clause
    {
      $$ = $1;
    }
  | data_sharing_clause
    {
      $$ = $1;
    }
;

/* OpenMP V3.0 */
unique_task_clause:
    if_clause
    {
      $$ = $1;
    }
  | OMP_FINAL '(' { sc_pause_openmp(); } expression ')'
    {
      sc_start_openmp();
      $$ = FinalClause($4);
    }
  | OMP_UNTIED
    {
      $$ = PlainClause(OCUNTIED);
    }
  | OMP_MERGEABLE
    {
      $$ = PlainClause(OCMERGEABLE);
    }
  | /* OpenMP V4.0 2.11.1.1 page 116 */
    OMP_DEPEND '(' dependence_type ':' variable_array_section_list ')'
    {
      //$$ = VarlistClause(OCPRIVATE, $6); TODO find out how to do this. It needs type OCDEPEND subtype from $3 and a list from $6
    }
;

/* OpenMP V4.0 2.11.1.1 page 116 */
dependence_type:
    OMP_IN
    {
      //$$ = OC_in; TODO OC_in
    }
  | OMP_OUT
    {
      //$$ = OC_out; TODO OC_out
    }
  | OMP_INOUT
    {
      //$$ = OC_inout; TODO OC_inout
    }
;

parallel_for_construct:
    parallel_for_directive iteration_statement_for
    {
      $$ = OmpConstruct(DCPARFOR, $1, $2);
      $$->l = $1->l;
    }
;

parallel_for_directive:
    PRAGMA_OMP OMP_PARALLEL OMP_FOR parallel_for_clause_optseq '\n'
    {
      $$ = OmpDirective(DCPARFOR, $4);
    }
;

parallel_for_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | parallel_for_clause_optseq parallel_for_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | parallel_for_clause_optseq ',' parallel_for_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

parallel_for_clause:
    unique_parallel_clause
    {
      $$ = $1;
    }
  | unique_for_clause
    {
      $$ = $1;
    }
  | data_default_clause
    {
      $$ = $1;
    }
  | data_privatization_clause
    {
      $$ = $1;
    }
  | data_privatization_in_clause
    {
      $$ = $1;
    }
  | data_privatization_out_clause
    {
      $$ = $1;
    }
  | data_sharing_clause
    {
      $$ = $1;
    }
  | data_reduction_clause
    {
      $$ = $1;
    }
;

parallel_sections_construct:
    parallel_sections_directive section_scope
    {
      $$ = OmpConstruct(DCPARSECTIONS, $1, $2);
      $$->l = $1->l;
    }
;

parallel_sections_directive:
    PRAGMA_OMP OMP_PARALLEL OMP_SECTIONS parallel_sections_clause_optseq '\n'
    {
      $$ = OmpDirective(DCPARSECTIONS, $4);
    }
;

parallel_sections_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | parallel_sections_clause_optseq parallel_sections_clause
    {
      $$ = OmpClauseList($1, $2);
    }
  | parallel_sections_clause_optseq ',' parallel_sections_clause
    {
      $$ = OmpClauseList($1, $3);
    }
;

parallel_sections_clause:
    unique_parallel_clause
    {
      $$ = $1;
    }
  | data_default_clause
    {
      $$ = $1;
    }
  | data_privatization_clause
    {
      $$ = $1;
    }
  | data_privatization_in_clause
    {
      $$ = $1;
    }
  | data_privatization_out_clause
    {
      $$ = $1;
    }
  | data_sharing_clause
    {
      $$ = $1;
    }
  | data_reduction_clause
    {
      $$ = $1;
    }
;

master_construct:
    master_directive structured_block
    {
      $$ = OmpConstruct(DCMASTER, $1, $2);
    }
;

master_directive:
    PRAGMA_OMP OMP_MASTER '\n'
    {
      $$ = OmpDirective(DCMASTER, NULL);
    }
;

critical_construct:
    critical_directive structured_block
    {
      $$ = OmpConstruct(DCCRITICAL, $1, $2);
    }
;

critical_directive:
    PRAGMA_OMP OMP_CRITICAL '\n'
    {
      $$ = OmpCriticalDirective(NULL);
    }
  | PRAGMA_OMP OMP_CRITICAL region_phrase '\n'
    {
      $$ = OmpCriticalDirective($3);
    }
;

region_phrase:
    '(' IDENTIFIER ')'
    {
      $$ = Symbol($2);
    }
;

barrier_directive:
    PRAGMA_OMP OMP_BARRIER '\n'
    {
      $$ = OmpDirective(DCBARRIER, NULL);
    }
;

/* OpenMP V3.0 */
taskwait_directive:
    PRAGMA_OMP OMP_TASKWAIT '\n'
    {
      $$ = OmpDirective(DCTASKWAIT, NULL);
    }
;

/* OpenMP V4.0 2.12.5 page 126 */
taskgroup_construct: //TODO where is it called from??? Put it on the TODO list
    taskgroup_directive structured_block
    {
      $$ = OmpConstruct(DCTASKGROUP, $1, $2);
    }
;

/* OpenMP V4.0 2.12.5 page 126 */
taskgroup_directive:
    PRAGMA_OMP OMP_TASKGROUP'\n'
    {
      $$ = OmpDirective(DCTASKGROUP, NULL);
    }
;

/* OpenMP V3.1 */
taskyield_directive:
    PRAGMA_OMP OMP_TASKYIELD '\n'
    {
      $$ = OmpDirective(DCTASKYIELD, NULL);
    }
;

atomic_construct:
    atomic_directive expression_statement
    {
      $$ = OmpConstruct(DCATOMIC, $1, $2);
    }
//  | atomic_directive structured_block //TODO check
//    {
//      $$ = OmpConstruct(DCATOMIC, $1, $2); //TODO check that capture was provided?
//    }
;

atomic_directive:
    PRAGMA_OMP OMP_ATOMIC atomic_clause_opt seq_cst_clause_opt '\n' //TODO change the clauses according to page 131 line 13
    {
      $$ = OmpDirective(DCATOMIC, NULL);  //TODO Check how to do it since it now has 2 clauses
    }
;

atomic_clause_opt:
    // empty
    {
      $$ = NULL;
    }
  | OMP_READ
    {
      //$$ = TODO
    }
  | OMP_WRITE
    {
      //$$ = TODO
    }
  | OMP_UPDATE
    {
      //$$ = TODO
    }
  | OMP_CAPTURE
    {
      //$$ = TODO
    }
;

/* OpenMP V4.0 2.12.6 page 132 */
seq_cst_clause_opt:
    // empty
    {
      $$ = NULL;
    }
  | OMP_SEQ_CST
    {
      //$$ = TODO
    }
;

flush_directive:
    PRAGMA_OMP OMP_FLUSH '\n'
    {
      $$ = OmpFlushDirective(NULL);
    }
  | PRAGMA_OMP OMP_FLUSH flush_vars '\n'
    {
      $$ = OmpFlushDirective($3);
    }
;

flush_vars:
    '(' { sc_pause_openmp(); } variable_list ')'
    {
      sc_start_openmp();
      $$ = $3;
    }
;

ordered_construct:
    ordered_directive structured_block
    {
      $$ = OmpConstruct(DCORDERED, $1, $2);
    }
;

ordered_directive:
    PRAGMA_OMP OMP_ORDERED '\n'
    {
      $$ = OmpDirective(DCORDERED, NULL);
    }
;

/* OpenMP V4.0 2.13.1 page 140 */
cancel_directive: //TODO not called from anywhere
    PRAGMA_OMP OMP_CANCEL construct_type_clause '\n'
    {
      $$ = OmpDirective(DCCANCEL, $3);
    }
  | PRAGMA_OMP OMP_CANCEL construct_type_clause if_clause '\n'
    {
      $$ = OmpDirective(DCCANCEL, OmpClauseList($3, $4));
    }
  | PRAGMA_OMP OMP_CANCEL construct_type_clause ',' if_clause '\n'
    {
      $$ = OmpDirective(DCCANCEL, OmpClauseList($3, $5));
    }
;

construct_type_clause:
    OMP_PARALLEL
    {
      $$ = PlainClause(OCPARALLEL);
    }
  | OMP_SECTIONS
    {
      $$ = PlainClause(OCSECTIONS);
    }
  | OMP_FOR
    {
      $$ = PlainClause(OCFOR);
    }
  | OMP_TASKGROUP
    {
      $$ = PlainClause(OCTASKGROUP);
    }
;

/* OpenMP V4.0 2.13.2 page 143 */
cancellation_point_directive: //TODO not called from anywhere
    PRAGMA_OMP_CANCELLATIONPOINT construct_type_clause '\n'
    {
      $$ = OmpDirective(DCCANCELLATIONPOINT, $2);
    }
;

threadprivate_directive:
    PRAGMA_OMP_THREADPRIVATE { sc_pause_openmp(); } '(' thrprv_variable_list ')' { sc_start_openmp(); } '\n'
    {
      $$ = OmpThreadprivateDirective($4);
    }
;

/* OpenMP V4.0 2.15 page 180 */
declare_reduction_directive:
    PRAGMA_OMP OMP_DECLARE OMP_REDUCTION '(' reduction_identifier ':' reduction_type_list ':' { sc_pause_openmp(); } expression ')' { sc_start_openmp(); } initializer_clause_opt '\n'
    {
      //$$ = TODO
    }
;

reduction_identifier:
    IDENTIFIER
    {
      //$$ = OC_identifier TODO
      //Symbol($2);  TODO
    }
  | '+'
    {
      $$ = OC_plus;
    }
  | '*'
    {
      $$ = OC_times;
    }
  | '-'
    {
      $$ = OC_minus;
    }
  | '&'
    {
      $$ = OC_band;
    }
  | '^'
    {
      $$ = OC_xor;
    }
  | '|'
    {
      $$ = OC_bor;
    }
  | AND_OP
    {
      $$ = OC_land;
    }
  | OR_OP
    {
      $$ = OC_lor;
    }
  | OMP_MIN
    {
      $$ = OC_min;
    }
  | OMP_MAX
    {
      $$ = OC_max;
    }
;

reduction_type_list:
    type_specifier
    {
      //TODO
    }
  | reduction_type_list ',' type_specifier
    {
      //TODO
    }
;

initializer_clause_opt:
    // empty
    {
      $$ = NULL;
    }
  | OMP_INITIALIZER '(' IDENTIFIER '=' conditional_expression ')'
    {
        //TODO must check if identifier is omp_priv and that conditional
        //expression contains only omp_priv and omp_orig variables
    }
  | OMP_INITIALIZER '(' IDENTIFIER '(' argument_expression_list ')' ')'
    {
      //TODO in argument_expression_list one of the variables must be &omp_priv
      // TODO check ox_funccall_expression
      //$$ = strcmp($1, "main") ?
      //       FunctionCall(IdentName($1), $3) :
      //       FunctionCall(IdentName(MAIN_NEWNAME), $3);
    }
;

data_default_clause:
    OMP_DEFAULT '(' OMP_SHARED ')'
    {
      $$ = DefaultClause(OC_defshared);
    }
  | OMP_DEFAULT '(' OMP_NONE ')'
    {
      $$ = DefaultClause(OC_defnone);
    }
  | /* Clause added for Aggelo's auto scoping */
    OMP_DEFAULT '(' OMP_AUTO ')'
    {
      $$ = DefaultClause(OC_auto); //I'm using the existing subtype (Alexandros)
    }
;

data_privatization_clause:
    OMP_PRIVATE { sc_pause_openmp(); } '(' variable_list ')'
    {
      sc_start_openmp();
      $$ = VarlistClause(OCPRIVATE, $4);
    }
;

data_privatization_in_clause:
    OMP_FIRSTPRIVATE { sc_pause_openmp(); } '(' variable_list ')'
    {
      sc_start_openmp();
      $$ = VarlistClause(OCFIRSTPRIVATE, $4);
    }
;

data_privatization_out_clause:
    OMP_LASTPRIVATE { sc_pause_openmp(); } '(' variable_list ')'
    {
      sc_start_openmp();
      $$ = VarlistClause(OCLASTPRIVATE, $4);
    }
;

data_sharing_clause:
    OMP_SHARED { sc_pause_openmp(); } '(' variable_list ')'
    {
      sc_start_openmp();
      $$ = VarlistClause(OCSHARED, $4);
    }
;

data_reduction_clause:
    OMP_REDUCTION '(' reduction_identifier { sc_pause_openmp(); } ':' variable_list ')'
    {
      sc_start_openmp();
      $$ = ReductionClause($3, $6);
    }
;

if_clause:
    OMP_IF '(' { sc_pause_openmp(); } expression ')'
    {
      sc_start_openmp();
      $$ = IfClause($4);
    }
;

collapse_clause:
    OMP_COLLAPSE '(' expression /* CONSTANT */ ')'   /* OpenMP V3.0 */
    {
      int n = 0, er = 0;
      if (xar_expr_is_constant($3))
      {
        n = xar_calc_int_expr($3, &er);
        if (er) n = 0;
      }
      if (n <= 0)
        parse_error(1, "invalid number in collapse() clause.\n");
      $$ = CollapseClause(n);
    }
;

array_section:
    IDENTIFIER array_section_subscript
    {
      if (checkDecls)
        if (symtab_get(stab, Symbol($1), IDNAME) == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", $1);
      $$ = IdentifierDecl( Symbol($1) );
      parse_warning("Array section not supported yet. Ignored.\n");
    }
;

variable_list:
    IDENTIFIER
    {
      if (checkDecls)
        if (symtab_get(stab, Symbol($1), IDNAME) == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", $1);
      $$ = IdentifierDecl( Symbol($1) );
    }
  | variable_list ',' IDENTIFIER
    {
      if (checkDecls)
        if (symtab_get(stab, Symbol($3), IDNAME) == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", $3);
      $$ = IdList($1, IdentifierDecl( Symbol($3) ));
    }
;

variable_array_section_list:
    IDENTIFIER //TODO
    {
      if (checkDecls)
        if (symtab_get(stab, Symbol($1), IDNAME) == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", $1);
      $$ = IdentifierDecl( Symbol($1) );
    }
  | array_section
    {
      $$ = $1;
    }
  | variable_array_section_list ',' IDENTIFIER
    {
      if (checkDecls)
        if (symtab_get(stab, Symbol($3), IDNAME) == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", $3);
      $$ = IdList($1, IdentifierDecl( Symbol($3) ));
    }
  | variable_array_section_list ',' array_section
    {
      $$ = IdList($1, $3);
    }
;

array_section_subscript:
    array_section_subscript '[' { sc_pause_openmp(); } array_section_plain ']'
    {
      sc_start_openmp();
      //TODO
    }
  | '[' { sc_pause_openmp(); } array_section_plain ']'
    {
      sc_start_openmp();
      //TODO
    }
;

array_section_plain:
    expression ':' expression
    {
      //TODO
    }
  | expression
    {
      //TODO
    }
 ;


procbind_clause:
    OMP_PROCBIND '(' OMP_MASTER ')'
    {
      $$ = ProcBindClause(OC_bindmaster);
    }
  | OMP_PROCBIND '(' OMP_CLOSE ')'
    {
      $$ = ProcBindClause(OC_bindclose);
    }
  | OMP_PROCBIND '(' OMP_SPREAD ')'
    {
      $$ = ProcBindClause(OC_bindspread);
    }
;

/* The same as "variable_list" only it checks if the variables
 * are declared @ the *same* scope level and whether they include the
 * "static" specifier. The original variable is also marked as
 * threadprivate.
 */
thrprv_variable_list:
    IDENTIFIER
    {
      if (checkDecls)
      {
        stentry e = symtab_get(stab, Symbol($1), IDNAME);
        if (e == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", $1);
        if (e->scopelevel != stab->scopelevel)
          parse_error(-1, "threadprivate directive appears at different "
                          "scope level\nfrom the one `%s' was declared.\n", $1);
        if (stab->scopelevel > 0)    /* Don't care for globals */
          if (speclist_getspec(e->spec, STCLASSSPEC, SPEC_static) == NULL)
            parse_error(-1, "threadprivate variable `%s' does not have static "
                            "storage type.\n", $1);
        e->isthrpriv = true;   /* Mark */
      }
      $$ = IdentifierDecl( Symbol($1) );
    }
  | thrprv_variable_list ',' IDENTIFIER
    {
      if (checkDecls)
      {
        stentry e = symtab_get(stab, Symbol($3), IDNAME);
        if (e == NULL)
          parse_error(-1, "unknown identifier `%s'.\n", $3);
        if (e->scopelevel != stab->scopelevel)
          parse_error(-1, "threadprivate directive appears at different "
                          "scope level\nfrom the one `%s' was declared.\n", $3);
        if (stab->scopelevel > 0)    /* Don't care for globals */
          if (speclist_getspec(e->spec, STCLASSSPEC, SPEC_static) == NULL)
            parse_error(-1, "threadprivate variable `%s' does not have static "
                            "storage type.\n", $3);
        e->isthrpriv = true;   /* Mark */
      }
      $$ = IdList($1, IdentifierDecl( Symbol($3) ));
    }
;

/* -------------------------------------------------------------------------
 * --- OMPi extensions -----------------------------------------------------
 * -------------------------------------------------------------------------
 */

ompix_directive:
    ox_tasksync_directive
    {
      $$ = OmpixConstruct(OX_DCTASKSYNC, $1, NULL);
    }
  | ox_taskschedule_directive
    {
      $$ = OmpixConstruct(OX_DCTASKSCHEDULE, $1, NULL);
    }

;

ox_tasksync_directive:
    PRAGMA_OMPIX OMPIX_TASKSYNC '\n'
    {
      $$ = OmpixDirective(OX_DCTASKSYNC, NULL);
    }
;

ox_taskschedule_directive:
    PRAGMA_OMPIX OMPIX_TASKSCHEDULE
    {
      scope_start(stab);
    }
    ox_taskschedule_clause_optseq '\n'
    {
      scope_end(stab);
      $$ = OmpixDirective(OX_DCTASKSCHEDULE, $4);
    }
;

ox_taskschedule_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | ox_taskschedule_clause_optseq ox_taskschedule_clause
    {
      $$ = OmpixClauseList($1, $2);
    }
  | ox_taskschedule_clause_optseq ',' ox_taskschedule_clause
    {
      $$ = OmpixClauseList($1, $3);
    }
;

ox_taskschedule_clause:
    OMPIX_STRIDE '(' assignment_expression')'
    {
      $$ = OmpixStrideClause($3);
    }
  | OMPIX_START '(' assignment_expression ')'
    {
      $$ = OmpixStartClause($3);
    }
  | OMPIX_SCOPE '(' ox_scope_spec ')'
    {
      $$ = OmpixScopeClause($3);
    }
  | OMPIX_TIED
    {
      $$ = OmpixPlainClause(OX_OCTIED);
    }
  | OMP_UNTIED
    {
      $$ = OmpixPlainClause(OX_OCUNTIED);
    }
;

ox_scope_spec:
    OMPIX_NODES
    {
      $$ = OX_SCOPE_NODES;
    }
  | OMPIX_WORKERS
    {
      $$ = OX_SCOPE_WGLOBAL;
    }
  | OMPIX_WORKERS ',' OMPIX_GLOBAL
    {
      $$ = OX_SCOPE_WGLOBAL;
    }
  | OMPIX_WORKERS ',' OMPIX_LOCAL
    {
      $$ = OX_SCOPE_WLOCAL;
    }
;

ompix_construct:
    ox_taskdef_construct
    {
      $$ = $1;
    }
  | ox_task_construct
    {
      $$ = $1;
    }
;

/* 1 reduce-reduce here; it's benign but we'll improve it some day.. */
ox_taskdef_construct:
   ox_taskdef_directive normal_function_definition
    {
      /* Should put the name of the callback function in the stab, too
      if (symtab_get(stab, decl_getidentifier_symbol($2->u.declaration.decl),
            FUNCNAME) == NULL)
        symtab_put(stab, decl_getidentifier_symbol($2->u.declaration.spec),
            FUNCNAME);
      */
      scope_start(stab);   /* re-declare the arguments of the task function */
      ast_declare_function_params($2->u.declaration.decl);
    }
    compound_statement
    {
      scope_end(stab);
      $$ = OmpixTaskdef($1, $2, $4);
      $$->l = $1->l;
    }
  | ox_taskdef_directive normal_function_definition
    {
      $$ = OmpixTaskdef($1, $2, NULL);
      $$->l = $1->l;
    }
;

ox_taskdef_directive:
    PRAGMA_OMPIX OMPIX_TASKDEF
    {
      scope_start(stab);
    }
    ox_taskdef_clause_optseq '\n'
    {
      scope_end(stab);
      $$ = OmpixDirective(OX_DCTASKDEF, $4);
    }
;

ox_taskdef_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | ox_taskdef_clause_optseq ox_taskdef_clause
    {
      $$ = OmpixClauseList($1, $2);
    }
  | ox_taskdef_clause_optseq ',' ox_taskdef_clause
    {
      $$ = OmpixClauseList($1, $3);
    }
;

ox_taskdef_clause:
    OMPIX_IN '(' ox_variable_size_list')'
    {
      $$ = OmpixVarlistClause(OX_OCIN, $3);
    }
  | OMPIX_OUT '(' ox_variable_size_list')'
    {
      $$ = OmpixVarlistClause(OX_OCOUT, $3);
    }
  | OMPIX_INOUT '(' ox_variable_size_list')'
    {
      $$ = OmpixVarlistClause(OX_OCINOUT, $3);
    }
    | OMP_REDUCTION '(' reduction_identifier ':' ox_variable_size_list ')'
    {
      $$ = OmpixReductionClause($3, $5);
    }
;

ox_variable_size_list:
    ox_variable_size_elem
    {
      $$ = $1;
    }
  | ox_variable_size_list ',' ox_variable_size_elem
    {
      $$ = IdList($1, $3);
    }
;

ox_variable_size_elem:
    IDENTIFIER
    {
      $$ = IdentifierDecl( Symbol($1) );
      symtab_put(stab, Symbol($1), IDNAME);
    }
  | IDENTIFIER '[' '?' IDENTIFIER ']'
    {
      if (checkDecls) check_uknown_var($4);
      /* Use extern to differentiate */
      $$ = ArrayDecl(IdentifierDecl( Symbol($1) ), StClassSpec(SPEC_extern),
                     IdentName($4));
      symtab_put(stab, Symbol($1), IDNAME);
    }
  | IDENTIFIER '[' assignment_expression ']'
    {
      $$ = ArrayDecl(IdentifierDecl( Symbol($1) ), NULL, $3);
      symtab_put(stab, Symbol($1), IDNAME);
    }
;

ox_task_construct:
    ox_task_directive ox_funccall_expression ';'
    {
      $$ = OmpixConstruct(OX_DCTASK, $1, Expression($2));
      $$->l = $1->l;
    }
;

ox_task_directive:
    PRAGMA_OMPIX OMP_TASK ox_task_clause_optseq '\n'
    {
      $$ = OmpixDirective(OX_DCTASK, $3);
    }
;

ox_task_clause_optseq:
    // empty
    {
      $$ = NULL;
    }
  | ox_task_clause_optseq ox_task_clause
    {
      $$ = OmpixClauseList($1, $2);
    }
  | ox_task_clause_optseq ',' ox_task_clause
    {
      $$ = OmpixClauseList($1, $3);
    }
;

ox_task_clause:
    OMPIX_ATNODE '(' '*' ')'
    {
      $$ = OmpixPlainClause(OX_OCATALL);
    }
  | OMPIX_ATNODE '(' assignment_expression ')'
    {
      $$ = OmpixAtnodeClause($3);
    }
  | OMPIX_ATWORKER '(' assignment_expression ')'
    {
      $$ = OmpixAtworkerClause($3);
    }
  | OMPIX_TIED
    {
      $$ = OmpixPlainClause(OX_OCTIED);
    }
  | OMP_UNTIED
    {
      $$ = OmpixPlainClause(OX_OCUNTIED);
    }
  | OMPIX_DETACHED
    {
      $$ = OmpixPlainClause(OX_OCDETACHED);
    }
;

ox_funccall_expression:
    IDENTIFIER '(' ')'
    {
      $$ = strcmp($1, "main") ?
             FunctionCall(IdentName($1), NULL) :
             FunctionCall(IdentName(MAIN_NEWNAME), NULL);
    }
  | IDENTIFIER '(' argument_expression_list ')'
    {
      $$ = strcmp($1, "main") ?
             FunctionCall(IdentName($1), $3) :
             FunctionCall(IdentName(MAIN_NEWNAME), $3);
    }
;


%%


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
