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

/* AST - the Abstract Syntax Tree */

#ifndef __AST_H__
#define __AST_H__

#include <stdio.h>

/* Predefine them because they are mutually recursive & they are used in
 * our include files, too.
 */
typedef struct astexpr_   *astexpr;
typedef struct astspec_   *astspec;
typedef struct astdecl_   *astdecl;
typedef struct aststmt_   *aststmt;
typedef struct ompcon_    *ompcon;    /* OpenMP construct */
typedef struct ompdir_    *ompdir;    /* OpenMP directive */
typedef struct ompclause_ *ompclause; /* OpenMP clause */
typedef struct ompxli_    *ompxli;    /* Extended list items (array sections) */
typedef struct omparrdim_ *omparrdim;

typedef struct oxclause_ *oxclause;    /* OMPi-extension clause */
typedef struct oxcon_    *oxcon;       /* -"- construct */
typedef struct oxdir_    *oxdir;       /* -"- directive */

#include "symtab.h"

/* Create the child->parent links in the finalized AST.
 * It works, of course, only for statement nodes.
 */
extern void ast_stmt_parent(aststmt parent, aststmt t);
extern void ast_parentize(aststmt tree);

/* Add a statement before another statement in-place */
void ast_stmt_prepend(aststmt where, aststmt what);
/* Add a statement after another statement in-place */
void ast_stmt_append(aststmt where, aststmt what);
/* Given any statement, get the function node it belongs to */
extern aststmt ast_get_enclosing_function(aststmt t);
/* Inserts a statement after the declaration section in a compound */
extern void ast_compound_insert_statement(aststmt tree, aststmt t);
/* Transforms a BlockList tree to an equivallent right-ward path */
extern void ast_linearize(aststmt tree);
/* Prepends a specifier to a declaration or function definition statement */
extern void ast_declordef_addspec(aststmt orig, astspec spec);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     EXRESSION NODES                                           *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


enum exprtype { IDENT = 1, CONSTVAL, STRING, FUNCCALL, ARRAYIDX,
                DOTFIELD, PTRFIELD, CASTEXPR, CONDEXPR, UOP, BOP,
                PREOP, POSTOP, ASS, COMMALIST, SPACELIST, BRACEDINIT,
                DESIGNATED, IDXDES, DOTDES
              };

struct astexpr_
{
	enum exprtype type;
	astexpr       left, right;
	int           opid;   /* Used for operators */
	union
	{
		char    *str;   /* Used by strings and constants */
		symbol  sym;    /* Used by identifiers/fields */
		astexpr cond;   /* Used only in conditional exprs */
		astdecl dtype;  /* Used only in casts & sizeof */
	} u;
	int    l, c;                 /* Location in file (line, column) */
	symbol file;
};

/* Ids of unary, binary and assignement operators.
 * To print them out, use them as indexes to the correspoind _symbols[] array.
 */
#define UOP_addr        0
#define UOP_star        1
#define UOP_neg         2
#define UOP_bnot        3
#define UOP_lnot        4
#define UOP_sizeof      5
#define UOP_sizeoftype  6
#define UOP_inc         7
#define UOP_dec         8
#define UOP_paren       9          /* (expr) */
#define UOP_typetrick   10
extern char *UOP_symbols[11];

#define BOP_shl     0
#define BOP_shr     1
#define BOP_leq     2
#define BOP_geq     3
#define BOP_eqeq    4
#define BOP_neq     5
#define BOP_land    6
#define BOP_lor     7
#define BOP_band    8
#define BOP_bor     9
#define BOP_xor     10
#define BOP_add     11
#define BOP_sub     12
#define BOP_lt      13
#define BOP_gt      14
#define BOP_mul     15
#define BOP_div     16
#define BOP_mod     17
#define BOP_cast    18
extern char *BOP_symbols[19];

#define ASS_eq  0
#define ASS_mul 1
#define ASS_div 2
#define ASS_mod 3
#define ASS_add 4
#define ASS_sub 5
#define ASS_shl 6
#define ASS_shr 7
#define ASS_and 8
#define ASS_xor 9
#define ASS_or  10
extern char *ASS_symbols[11];

/* Node creation calls
 */
extern astexpr Astexpr(enum exprtype type, astexpr left, astexpr right);
extern astexpr Identifier(symbol s);
extern astexpr Constant(char *s);
extern astexpr numConstant(int n);
extern astexpr String(char *s);
extern astexpr DotField(astexpr e, symbol s);
extern astexpr PtrField(astexpr e, symbol s);
extern astexpr Operator(enum exprtype type, int opid, astexpr left,
                        astexpr right);
extern astexpr ConditionalExpr(astexpr cond, astexpr t, astexpr f);
extern astexpr DotDesignator(symbol s);
extern astexpr CastedExpr(astdecl d, astexpr e);
extern astexpr Sizeoftype(astdecl d);
extern astexpr TypeTrick(astdecl d);
#define        ArrayIndex(a,b)           Astexpr(ARRAYIDX,a,b)
#define        FunctionCall(a,b)         Astexpr(FUNCCALL,a,b)
#define        PostOperator(e,opid)      Operator(POSTOP,opid,e,NULL)
#define        PreOperator(e,opid)       Operator(PREOP,opid,e,NULL)
#define        UnaryOperator(opid,e)     Operator(UOP,opid,e,NULL)
#define        BinaryOperator(opid,a,b)  Operator(BOP,opid,a,b)
#define        Assignment(lhs,opid,rhs)  Operator(ASS,opid,lhs,rhs)
#define        CommaList(a,b)            Astexpr(COMMALIST,a,b)
#define        SpaceList(a,b)            Astexpr(SPACELIST,a,b)
#define        Sizeof(e)                 UnaryOperator(UOP_sizeof,e)
#define        Parenthesis(e)            UnaryOperator(UOP_paren,e)
#define        UOAddress(e)              UnaryOperator(UOP_addr,e)
#define        BracedInitializer(a)      Astexpr(BRACEDINIT,a,NULL)
#define        Designated(a,b)           Astexpr(DESIGNATED,a,b)
#define        IdxDesignator(a)          Astexpr(IDXDES,a,NULL)
#define        IdentName(a)              Identifier(Symbol(a))
#define        CastVoidStar(a)           CastedExpr(Casttypename( \
                                           Declspec(SPEC_void), \
                                           AbstractDeclarator( \
                                             Declspec(SPEC_star), NULL) \
                                           ), \
                                           a \
                                         )
#define        NullExpr()                CastVoidStar(numConstant(0))
#define        Deref(a)                  UnaryOperator(UOP_star,a)
#define        DerefParen(a)             Deref(Parenthesis(a))

#define Comma2(a,b)         CommaList((a),(b))
#define Comma3(a,b,c)       Comma2((a),Comma2((b),(c)))
#define Comma4(a,b,c,d)     Comma2(Comma2((a),(b)),Comma2((c),(d)))
#define Comma5(a,b,c,d,e)   Comma2(Comma2((a),(b)),Comma3((c),(d),(e)))
#define Comma6(a,b,c,d,e,f) Comma2(Comma3((a),(b),(c)),Comma3((d),(e),(f)))
#define Comma7(a,b,c,d,e,f,g) \
                            Comma2(Comma3((a),(b),(c)),Comma4((d),(e),(f),(g)))
#define Comma8(a,b,c,d,e,f,g,h) \
                        Comma2(Comma4((a),(b),(c),(d)),Comma4((e),(f),(g),(h)))


/* Find the number of elements in a list (1 if not a COMMA/SPACE LIST) */
extern int expr_list_cardinality(astexpr expr);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     DECLARATION NODES                                         *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * We have 2 types of declaration nodes:
 *   (1) specifier nodes and
 *   (2) declarator nodes
 * The first one contains vaguely the "type" and the second one
 * the actual "name" that is being declared, or the "left" part
 * and the "right" part of the declaration, correspondingly.
 * Of course, each part is a quite complicated tree.
 */

/*
 * SPECIFIERS
 */

/* Specifier keywords/subtypes */
#define SPEC_typedef     1
#define SPEC_extern      2
#define SPEC_static      3
#define SPEC_auto        4
#define SPEC_register    5
#define SPEC_void        6
#define SPEC_char        7
#define SPEC_short       8
#define SPEC_int         9
#define SPEC_long        10
#define SPEC_float       11
#define SPEC_double      12
#define SPEC_signed      13
#define SPEC_unsigned    14
#define SPEC_ubool       15
#define SPEC_ucomplex    16
#define SPEC_uimaginary  17
#define SPEC_struct      18
#define SPEC_union       19
#define SPEC_enum        20
#define SPEC_const       21
#define SPEC_restrict    22
#define SPEC_volatile    23
#define SPEC_inline      24
#define SPEC_star        25       /* for pointer declaration specifiers */
#define SPEC_Rlist       26       /* It is only a subtype */
#define SPEC_Llist       27       /* It is only a subtype */
extern char *SPEC_symbols[28];

/* All declaration specifier node types */
enum spectype { SPEC = 1, STCLASSSPEC, USERTYPE, SUE, ENUMERATOR, SPECLIST,
                ATTRSPEC } ;

struct astspec_
{
	enum spectype type;
	int           subtype;
	symbol        name;        /* For SUE/enumlist name/user types */
	astspec       body;        /* E.g. for SUE fields, lists */
	astspec       sueattr;     /* For SUE attributes */
	union
	{
		astexpr     expr;        /* For enum list */
		astspec     next;        /* For Lists */
		astdecl     decl;        /* For structure specifiers (the fields) */
		char        *txt;        /* For attributes (verbatim text) */
	} u;
	int    l, c;               /* Location in file (line, column) */
	symbol file;
};

extern astspec Specifier(enum spectype type, int subtp, symbol name, astspec d);
extern astspec Enumerator(symbol name, astexpr expr);
extern astspec Specifierlist(int type, astspec e, astspec l);
extern astspec SUdecl(int type, symbol sym, astdecl decl, astspec attr);
extern astspec Enumdecl(symbol sym, astspec body, astspec attr);
extern astspec AttrSpec(char *s);
#define        Declspec(type)          Specifier(SPEC,type,NULL,NULL)
#define        StClassSpec(type)       Specifier(STCLASSSPEC, type, NULL, NULL)
#define        Usertype(sym)           Specifier(USERTYPE,0,sym,NULL)
#define        Speclist_right(e,l)     Specifierlist(SPEC_Rlist,e,l)
#define        Speclist_left(l,e)      Specifierlist(SPEC_Llist,e,l)
#define        Enumbodylist(l,e)       Specifierlist(SPEC_enum,e,l)

/*
 * DECLARATORS
 */

/* Declarator lists / subtypes */
#define DECL_decllist  1
#define DECL_idlist    2
#define DECL_paramlist 3
#define DECL_fieldlist 4

/* All declarator node types */
enum decltype { DIDENT = 1, DPAREN, DARRAY, DFUNC, DINIT,
                DECLARATOR, ABSDECLARATOR, DPARAM, DELLIPSIS, DBIT,
                DSTRUCTFIELD, DCASTTYPE, DLIST
              } ;

struct astdecl_
{
	enum decltype type;
	int           subtype;
	astdecl       decl;     /* For initlist,initializer,declarator */
	astspec       spec;     /* For pointer declarator */
	union
	{
		symbol      id;       /* Identifiers */
		astexpr     expr;     /* For initializer/bitdeclarator */
		astdecl     next;     /* For lists */
		astdecl     params;   /* For funcs */
	} u;
	int    l, c;            /* Location in file (line, column) */
	symbol file;
};

extern astdecl Decl(enum decltype type, int subtype, astdecl d, astspec s);
extern astdecl IdentifierDecl(symbol s);
extern astdecl ArrayDecl(astdecl decl, astspec s, astexpr e);
extern astdecl FuncDecl(astdecl decl, astdecl p);
extern astdecl InitDecl(astdecl decl, astexpr e);
extern astdecl BitDecl(astdecl decl, astexpr e);
extern astdecl Declanylist(int subtype, astdecl l, astdecl e);
#define        ParenDecl(d)              Decl(DPAREN,0,d,NULL)
#define        Pointer()                 Declspec(SPEC_star)
#define        Declarator(ptr,direct)    Decl(DECLARATOR,0,direct,ptr)
#define        AbstractDeclarator(ptr,d) Decl(ABSDECLARATOR,0,d,ptr)
#define        Casttypename(s,d)         Decl(DCASTTYPE,0,d,s)
#define        ParamDecl(s,d)            Decl(DPARAM,0,d,s)
#define        Ellipsis()                Decl(DELLIPSIS,0,NULL,NULL)
#define        DeclList(l,e)             Declanylist(DECL_decllist,l,e)
#define        IdList(l,e)               Declanylist(DECL_idlist,l,e)
#define        ParamList(l,e)            Declanylist(DECL_paramlist,l,e)
#define        StructfieldDecl(s,d)      Decl(DSTRUCTFIELD,0,d,s)
#define        StructfieldList(l,e)      Declanylist(DECL_fieldlist,l,e)

/*
 * Stuff related to declarated symbols
 */

#define        decl_getidentifier_symbol(d) ( decl_getidentifier(d)->u.id )
extern astdecl decl_getidentifier(astdecl d);
extern astdecl decl_topointer(astdecl d);
extern astdecl decl_rename(astdecl d, symbol newname);
extern int     decl_getkind(astdecl d);         /* DFUNC/DARRAY/DIDENT */
extern int     decl_ispointer(astdecl d);
extern int     func_returnspointer(astdecl d);
extern void    decl_ptr2arr(astdecl d, astexpr size);
extern void    decl_arr2ptr(astdecl d);
extern astspec speclist_getspec(astspec s, int type, int subtype);
/* Find the number of top-level elements in an initializer (DINIT) */
extern int     decl_initializer_cardinality(astdecl decl);
/* Gets/sets the size of a given array dimension from the array declaration */
extern astexpr decl_array_dimension_size(astdecl arr,int dimidx,astexpr newxpr);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     STATEMENT NODES                                           *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* All statement types */
enum stmttype { JUMP = 1, ITERATION, SELECTION, LABELED, EXPRESSION,
                DECLARATION, COMPOUND, STATEMENTLIST, FUNCDEF, OMPSTMT,
                VERBATIM, ASMSTMT, OX_STMT
              };

/* Structs for handling asm nodes */
typedef struct asmop_ *asmop;
struct asmop_ 
{
	astexpr symbolicname, var;
	char    *constraint;      /* if NULL, it is a LIST */
	asmop   next, op;           /* for lists */
};
typedef struct asmnode_ *asmnode;
struct asmnode_ 
{
	char    *template;
	astspec qualifiers;
	astexpr clobbers, labels;
	asmop   ins, outs;
};

struct aststmt_
{
	enum stmttype type;
	int           subtype;
	aststmt       parent;   /* Set *after* AST construction */
	aststmt       body;     /* Most have a body (COMPOUND has ONLY body) */
	union
	{
		astexpr expr;                 /* For expression & return statements */
		struct
		{
			aststmt init;
			astexpr cond, incr;
		} iteration;
		struct
		{
			astexpr cond;
			aststmt elsebody;
		} selection;
		struct
		{
			astspec spec;
			astdecl decl;       /* dlist is for FuncDef */
			aststmt dlist;
		} declaration;
		symbol  label;                /* For GOTO and labeled statements */
		aststmt next;                 /* For StatementList */
		ompcon  omp;                  /* OpenMP construct node */
		oxcon   ox;                   /* OMPi-extension node */
		char    *code;                /* For verbatim nodes */
		asmnode assem;                /* For asm statements */
	} u;
	int    l, c;                    /* Location in file (line, column) */
	symbol file;
};


/* All statement subtypes */
#define SGOTO        1    /* Jumps */
#define SBREAK       2
#define SCONTINUE    3
#define SRETURN      4
#define SWHILE       5    /* Iterations */
#define SDO          6
#define SFOR         7
#define SIF          8    /* Selections */
#define SSWITCH      9
#define SLABEL       10   /* Labeled */
#define SCASE        11
#define SDEFAULT     12
#define SXTENDED     13   /* For non-goto extended asm */

extern aststmt Statement(enum stmttype type, int subtype, aststmt body);
extern aststmt Jumpstatement(int subtype, astexpr expr);
extern aststmt Iterationstatement(int subtype,
                                  aststmt init, astexpr cond, astexpr incr, aststmt body);
extern aststmt Selectionstatement(int subtype,
                                  astexpr cond, aststmt body, aststmt elsebody);
extern aststmt LabeledStatement(int subtype, symbol l, astexpr e, aststmt st);
extern aststmt Goto(symbol s);
extern aststmt Expression(astexpr e);     /* Maybe NULL */
extern aststmt Declaration(astspec spec, astdecl decl);
extern aststmt BlockList(aststmt l, aststmt st);
extern aststmt FuncDef(astspec spec, astdecl decl, aststmt dlist, aststmt body);
extern aststmt OmpStmt(ompcon omp);
extern aststmt OmpixStmt(oxcon ox);
extern aststmt Verbatim(char *code);       /* Uses it as is */
extern aststmt verbit(char *format, ...);  /* Flexy way; includes strdup() */
extern asmop   AsmOp(astexpr id, char *con, astexpr var, asmop op, asmop nxt);
extern aststmt AsmStmt(int subtype, astspec qual, char *tpl, 
                       asmop out, asmop in, astexpr clob, astexpr labs);
#define Break()                  Jumpstatement(SBREAK, NULL)
#define Continue()               Jumpstatement(SCONTINUE, NULL)
#define Return(n)                Jumpstatement(SRETURN, n)
#define While(cond,body)         Iterationstatement(SWHILE,NULL,cond,NULL,body)
#define Do(body,cond)            Iterationstatement(SDO,NULL,cond,NULL,body)
#define For(init,cond,incr,body) Iterationstatement(SFOR,init,cond,incr,body)
#define If(c,t,e)                Selectionstatement(SIF,c,t,e)
#define Switch(c,b)              Selectionstatement(SSWITCH,c,b,NULL)
#define Labeled(l,a)             LabeledStatement(SLABEL,l,NULL,a)
#define Default(a)               LabeledStatement(SDEFAULT,NULL,NULL,a)
#define Case(e,a)                LabeledStatement(SCASE,NULL,e,a)
#define Compound(a)              Statement(COMPOUND,0,a)
/* Common cases */
#define AssignStmt(to, from)     Expression(Assignment(to, ASS_eq, from))
#define FuncCallStmt(a,b)        Expression(FunctionCall(a,b))

#define Block2(a,b)         BlockList((a),(b))
#define Block3(a,b,c)       Block2((a),Block2((b),(c)))
#define Block4(a,b,c,d)     Block2(Block2((a),(b)),Block2((c),(d)))
#define Block5(a,b,c,d,e)   Block2(Block2((a),(b)),Block3((c),(d),(e)))
#define Block6(a,b,c,d,e,f) Block2(Block3((a),(b),(c)),Block3((d),(e),(f)))

#define BasicAsm(q,t)           AsmStmt(SDEFAULT,q,t,NULL,NULL,NULL,NULL)
#define XtendAsm(q,t,o,i,c)     AsmStmt(SXTENDED,q,t,o,i,c,NULL)
#define XtendAsmGoto(t,i,c,l)   AsmStmt(SGOTO,NULL,t,NULL,i,c,l)
#define XAsmOperand(i,c,v)      AsmOp(i,c,v,NULL,NULL)
#define XAsmOpList(l,n)         AsmOp(NULL,NULL,NULL,n,l)


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OPENMP NODES                                              *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* All OpenMP clause types */
/* OCFIRSTLASTPRIVATE is NOT an OpenMP clause per se,
 * but is very helpful when analyzing variables that are both
 * first- and last-private.
 */
typedef enum clausetype
{
	OCNOCLAUSE = 0, OCNOWAIT, OCIF, OCNUMTHREADS, OCORDERED, OCSCHEDULE,
	OCCOPYIN, OCPRIVATE, OCCOPYPRIVATE, OCFIRSTPRIVATE, OCLASTPRIVATE, OCSHARED,
	OCDEFAULT, OCREDUCTION, OCLIST, OCFIRSTLASTPRIVATE,
	/* OpenMP 3.0 */
	OCUNTIED, OCCOLLAPSE,
	/* OpenMP 3.1 */
	OCFINAL, OCMERGEABLE,
	/* OpenMP 4.0 */
	OCPROCBIND, OCMAP, OCDEVICE, OCTO, OCFROM, OCPARALLEL, OCSECTIONS, OCFOR,
	OCTASKGROUP, OCDEPEND, OCNUMTEAMS, OCTHREADLIMIT,
	/* OpenMP 4.5 */
	OCHINT, OCPRIORITY, OCISDEVPTR, OCUSEDEVPTR, OCTHREADS, OCLINK,
	OCDEFAULTMAP, OCORDEREDNUM,
	/* Clause added for Aggelo's auto scoping */
	OCAUTO 
} ompclt_e;
extern char *clausenames[];

/* Clause subtypes */
typedef enum clausesubt
{
	OC_DontCare = -1,  /* no subtype given */ 
	OC_static = 0, OC_dynamic, OC_guided, OC_runtime, OC_defshared, OC_defnone,
	OC_plus, OC_times, OC_minus, OC_band, OC_bor, OC_xor, OC_land, OC_lor,
	OC_affinity, OC_auto, OC_min, OC_max,
	/* OpenMP 4.0 */
	OC_bindmaster, OC_bindclose, OC_bindspread, OC_alloc, OC_to, OC_from,
	OC_tofrom, OC_in, OC_out, OC_inout,
	/* OpenMP 4.5 */
	OC_source, OC_sink, OC_release, OC_delete,
	/* OpenMP 5.1 */
	OC_bindprimary
} ompclsubt_e;
extern char *clausesubs[];

/* Clause modifiers (>= v45) */
typedef enum clausemodt
{
	OCM_none = 0,  /* never change this */
	/* OpenMP 4.5 modifiers */
	OCM_always, OCM_monotonic, OCM_nonmonotonic, OCM_simd,
	OCM_parallel, OCM_task, OCM_target, OCM_targetdata,
	OCM_targetenterdata, OCM_targetexitdata, OCM_targetupdate, OCM_cancel
} ompclmod_e;
extern char *clausemods[];

/* Extended list items (plain identifier or array section) */
enum ompxli_type { OXLI_IDENT, OXLI_ARRSEC };

struct omparrdim_
{
	astexpr   lb;
	astexpr   len;
	omparrdim next;
};

struct ompxli_
{
	enum ompxli_type xlitype;
	symbol           id;        /* The identifier */
	omparrdim        dim;       /* List of dimensions */
	ompxli           next;      /* Lists of such items */
	int    l, c;                /* Location in file (line, column) */
	symbol file;
};
	
struct ompclause_
{
	ompclt_e    type;
	ompclsubt_e subtype;
	ompclmod_e  modifier;        /* Added in OpenMP4.5 */
	ompdir      parent;          /* The directive the clause belongs to */
	union
	{
		astexpr expr;
		astdecl varlist;
		ompxli  xlist;
		struct
		{
			ompclause elem;
			ompclause next;
		} list;
	} u;
	int    l, c;                 /* Location in file (line, column) */
	symbol file;
};

ompclause OmpClause(ompclt_e typ, ompclsubt_e subtype, ompclmod_e mod,
                    astexpr expr, astdecl vlist);
ompclause OmpClauseList(ompclause next, ompclause elem);
ompclause OmpXlistClause(ompclt_e typ, ompclsubt_e subt, ompclmod_e mod, 
                         ompxli xlist);
omparrdim OmpArrDim(astexpr lb, astexpr len);
ompxli    OmpXLItem(enum ompxli_type type, symbol id, omparrdim dim);
#define VarlistClause(type,varlist) OmpClause(type,0,OCM_none,NULL,varlist)
#define DefaultClause(what)         OmpClause(OCDEFAULT,what,OCM_none,NULL,NULL)
#define PlainClause(type)           OmpClause(type,0,OCM_none,NULL,NULL)
#define IfClause(expr,mod)          OmpClause(OCIF,0,mod,expr,NULL)
#define NumthreadsClause(expr)      OmpClause(OCNUMTHREADS,0,OCM_none,expr,NULL)
#define ScheduleClause(kind,mod,xp) OmpClause(OCSCHEDULE,kind,mod,xp,NULL)
#define CollapseClause(num)         OmpClause(OCCOLLAPSE,num,OCM_none,NULL,NULL)
#define FinalClause(expr)           OmpClause(OCFINAL,0,OCM_none,expr,NULL)
/* OpenMP V4.0 */
#define ReductionClause(op,xlist)   OmpXlistClause(OCREDUCTION,op,0,xlist)
#define ProcBindClause(what)        OmpClause(OCPROCBIND,what,0,NULL,NULL)
#define DeviceClause(expr)          OmpClause(OCDEVICE,0,OCM_none,expr,NULL)
#define ArraySection(id,dim)        OmpXLItem(OXLI_ARRSEC,id,dim)
#define PlainXLI(id)                OmpXLItem(OXLI_IDENT,id,NULL)
#define DependClause(way,xlist)     OmpXlistClause(OCDEPEND,way,OCM_none,xlist)
#define MapClause(type,mod,xlist)   OmpXlistClause(OCMAP,type,mod,xlist)
#define UpdateClause(type,xlist)    OmpXlistClause(type,OC_DontCare,0,xlist)
#define NumteamsClause(expr)        OmpClause(OCNUMTEAMS,0,OCM_none,expr,NULL)
#define ThreadlimitClause(expr)     OmpClause(OCTHREADLIMIT,0,0,expr,NULL)
/* OpenMP V4.5 */
#define HintClause(expr)            OmpClause(OCHINT,0,0,expr,NULL)
#define PriorityClause(expr)        OmpClause(OCPRIORITY,0,OCM_none,expr,NULL)
#define OrderedNumClause(num)       OmpClause(OCORDEREDNUM,num,OCM_none,NULL,NULL)

/* directive/construct types */
enum dircontype { DCPARALLEL = 1, DCFOR, DCSECTIONS, DCSECTION,
                  DCSINGLE, DCPARFOR, DCPARSECTIONS, DCFOR_P,
                  DCMASTER, DCCRITICAL, DCATOMIC, DCORDERED,
                  DCBARRIER, DCFLUSH, DCTHREADPRIVATE,
                  DCTASK, DCTASKWAIT, /* OpenMP 3.0 */
                  DCTASKYIELD,        /* OpenMP 3.1 */
                  /* OpenMP 4.0 */
                  DCTARGET, DCTARGETDATA, DCTARGETUPD, DCDECLTARGET, 
                  DCCANCEL, DCCANCELLATIONPOINT, DCTASKGROUP,
                  DCTEAMS, DCTARGETTEAMS,
                  /* OpenMP 4.5 */
                  DCTARGENTERDATA, DCTARGEXITDATA
                };
extern char *ompdirnames[];

struct ompdir_
{
	enum dircontype type;
	ompclause       clauses;      /* actually a clause list */
	ompcon          parent;       /* The construct the directive belongs to */
	union
	{
		symbol  region;
		astdecl varlist;     /* For flush(), threadprivate() */
	} u;
	int    l, c;                 /* Location in file (line, column) */
	symbol file;
};

extern ompdir OmpDirective(enum dircontype type, ompclause cla);
extern ompdir OmpCriticalDirective(symbol r, ompclause cla);
extern ompdir OmpFlushDirective(astdecl a);
extern ompdir OmpThreadprivateDirective(astdecl a);

struct ompcon_
{
	enum dircontype type;
	ompdir          directive;
	aststmt         body;
	aststmt         parent;   /* The OmpStmt node the construct belongs to */
	int    l, c;              /* Location in file (line, column) */
	symbol file;
};

extern ompcon OmpConstruct(enum dircontype type, ompdir dir, aststmt body);

/* Given a statement, return the closest enclosing OpenMP construct of the
 * given type (or of any type if type is 0); returns NULL if none found.
 */
extern ompcon ast_get_enclosing_ompcon(aststmt t, enum dircontype type);

#define OMPCON_IS_STANDALONE(c) ((c)->body == NULL)


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OMPi-EXTENSION NODES                                      *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* all OMPi-extension clause types */
enum oxclausetype { OX_OCIN = 1, OX_OCOUT, OX_OCINOUT, OX_OCLIST,
                    OX_OCREDUCE, OX_OCATNODE, OX_OCATALL, OX_OCDETACHED,
                    OX_OCTIED, OX_OCUNTIED, OX_OCSTRIDE, OX_OCSTART,
                    OX_OCSCOPE, OX_OCATWORKER, OX_OCIF,
					OX_OCLOCAL, OX_OCREMOTE, OX_OCHINTS
                  };
extern char *oxclausenames[19];

struct oxclause_
{
	enum oxclausetype type;
	int               operator;     /* reduction operator */
	oxdir             parent;       /* The directive the clause belongs to */
	union
	{
		int      value;         /* scope type */
		astexpr  expr;
		astdecl  varlist;
		struct
		{
			oxclause elem;
			oxclause next;
		} list;
	} u;
	int    l, c;                 /* Location in file (line, column) */
	symbol file;
};

/* Scope type for taskschedule */
#define OX_SCOPE_NODES     0
#define OX_SCOPE_WLOCAL    1
#define OX_SCOPE_WGLOBAL   2

extern oxclause OmpixClause(enum oxclausetype type, astdecl vlist, astexpr e);
extern oxclause OmpixClauseList(oxclause next, oxclause elem);
#define OmpixVarlistClause(type,varlist) OmpixClause(type,varlist,NULL)
extern oxclause OmpixReductionClause(int op, astdecl varlist);
extern oxclause OmpixScopeClause(int scope);
#define OmpixAtnodeClause(expr) OmpixClause(OX_OCATNODE,NULL,expr)
#define OmpixAtworkerClause(expr) OmpixClause(OX_OCATWORKER,NULL,expr)
#define OmpixHintsClause(expr) OmpixClause(OX_OCHINTS,NULL,expr)
#define OmpixPlainClause(type) OmpixClause(type, NULL, NULL)
#define OmpixStrideClause(expr) OmpixClause(OX_OCSTRIDE,NULL,expr)
#define OmpixStartClause(expr) OmpixClause(OX_OCSTART,NULL,expr)
#define OmpixIfClause(expr) OmpixClause(OX_OCIF,NULL,expr)

/* directive/construct types */
enum oxdircontype { OX_DCTASKDEF = 1, OX_DCTASK, OX_DCTASKSYNC,
                    OX_DCTASKSCHEDULE
                  };
extern char *oxdirnames[5];

struct oxdir_
{
	enum oxdircontype type;
	oxclause          clauses;      /* actually a clause list */
	oxcon             parent;       /* The construct the directive belongs to */
	int    l, c;                 /* Location in file (line, column) */
	symbol file;
};

extern oxdir OmpixDirective(enum oxdircontype type, oxclause cla);

struct oxcon_
{
	enum oxdircontype type;
	oxdir             directive;
	aststmt           body;
	aststmt           callback; /* Only for taskdef with uponreturn code */
	aststmt           parent;   /* The OmpStmt node the construct belongs to */
	int    l, c;                /* Location in file (line, column) */
	symbol file;
};

extern oxcon OmpixConstruct(enum oxdircontype type, oxdir dir, aststmt body);
extern oxcon OmpixTaskdef(oxdir dir, aststmt body, aststmt callbackblock);

#endif /* __AST_H__ */
