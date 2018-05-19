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

/* ast_traverse.h -- generic traversal of the AST using callbacks @ each node */

#ifndef __AST_TRAVERSE_H__
#define __AST_TRAVERSE_H__

#include "ast.h"

typedef struct {
	void (*break_c)(aststmt t, void *starg, int vistime);
	void (*continue_c)(aststmt t, void *starg, int vistime);
	void (*return_c)(aststmt t, void *starg, int vistime);
	void (*goto_c)(aststmt t, void *starg, int vistime);
	void (*for_c)(aststmt t, void *starg, int vistime);
	void (*while_c)(aststmt t, void *starg, int vistime);
	void (*do_c)(aststmt t, void *starg, int vistime);
	void (*switch_c)(aststmt t, void *starg, int vistime);
	void (*if_c)(aststmt t, void *starg, int vistime);
	void (*label_c)(aststmt t, void *starg, int vistime);
	void (*case_c)(aststmt t, void *starg, int vistime);
	void (*default_c)(aststmt t, void *starg, int vistime);
	void (*expression_c)(aststmt t, void *starg, int vistime);
	void (*compound_c)(aststmt t, void *starg, int vistime);
	void (*declaration_c)(aststmt t, void *starg, int vistime);
	void (*funcdef_c)(aststmt t, void *starg, int vistime);
	void (*stmtlist_c)(aststmt t, void *starg, int vistime);
	void (*verbatim_c)(aststmt t, void *starg, int vistime);
} stmtcbs_t;

typedef struct {
	void (*ident_c)(astexpr e, void *starg, int vistime);
	void (*constval_c)(astexpr e, void *starg, int vistime);
	void (*string_c)(astexpr e, void *starg, int vistime);
	void (*funccall_c)(astexpr e, void *starg, int vistime);
	void (*arrayidx_c)(astexpr e, void *starg, int vistime);
	void (*dotfield_c)(astexpr e, void *starg, int vistime);
	void (*ptrfield_c)(astexpr e, void *starg, int vistime);
	void (*bracedinit_c)(astexpr e, void *starg, int vistime);
	void (*castexpr_c)(astexpr e, void *starg, int vistime);
	void (*condexpr_c)(astexpr e, void *starg, int vistime);
	void (*uop_c)(astexpr e, void *starg, int vistime);
	void (*bop_c)(astexpr e, void *starg, int vistime);
	void (*preop_c)(astexpr e, void *starg, int vistime);
	void (*postop_c)(astexpr e, void *starg, int vistime);
	void (*ass_c)(astexpr e, void *starg, int vistime);
	void (*designated_c)(astexpr e, void *starg, int vistime);
	void (*idxdes_c)(astexpr e, void *starg, int vistime);
	void (*dotdes_c)(astexpr e, void *starg, int vistime);
	void (*list_c)(astexpr e, void *starg, int vistime);
} exprcbs_t;

typedef struct {
	void (*spec_c)(astspec s, void *starg, int vistime);
	void (*usertype_c)(astspec s, void *starg, int vistime);
	void (*specenum_c)(astspec s, void *starg, int vistime);
	void (*specstruct_c)(astspec s, void *starg, int vistime);
	void (*specunion_c)(astspec s, void *starg, int vistime);
	void (*enumerator_c)(astspec s, void *starg, int vistime);
	void (*speclist_c)(astspec s, void *starg, int vistime);
} speccbs_t;

typedef struct {
	void (*dident_c)(astdecl d, void *starg, int vistime);
	void (*dparen_c)(astdecl d, void *starg, int vistime);
	void (*darray_c)(astdecl d, void *starg, int vistime);
	void (*dfunc_c)(astdecl d, void *starg, int vistime);
	void (*dinit_c)(astdecl d, void *starg, int vistime);
	void (*declarator_c)(astdecl d, void *starg, int vistime);
	void (*absdeclarator_c)(astdecl d, void *starg, int vistime);
	void (*dparam_c)(astdecl d, void *starg, int vistime);
	void (*dellipsis_c)(astdecl d, void *starg, int vistime);
	void (*dbit_c)(astdecl d, void *starg, int vistime);
	void (*dstructfield_c)(astdecl d, void *starg, int vistime);
	void (*dcasttype_c)(astdecl d, void *starg, int vistime);
	void (*dlist_c)(astdecl d, void *starg, int vistime);
} declcbs_t;

typedef struct {
	void (*ompclexpr_c)(ompclause d, void *starg, int vistime);
	void (*ompclvars_c)(ompclause d, void *starg, int vistime);
	void (*ompclplain_c)(ompclause d, void *starg, int vistime);
	void (*ompcllist_c)(ompclause d, void *starg, int vistime);
	/* prune_* fields disallow recursing when visiting the corresponding nodes */
	int  prune_ompclexpr_c;
	int  prune_ompclvars_c;
} ompclcbs_t;

typedef struct {
	void (*ompdircrit_c)(ompdir d, void *starg, int vistime);
	void (*ompdirflush_c)(ompdir d, void *starg, int vistime);
	void (*ompdirthrpriv_c)(ompdir d, void *starg, int vistime);
	void (*ompdirrest_c)(ompdir d, void *starg, int vistime);
	void (*ompconall_c)(ompcon c, void *starg, int vistime);
} ompdccbs_t;

typedef struct {
	void (*oxclvars_c)(oxclause d, void *starg, int vistime);
	void (*oxclexpr_c)(oxclause d, void *starg, int vistime);
	void (*oxclrest_c)(oxclause d, void *starg, int vistime);
	void (*oxcllist_c)(oxclause d, void *starg, int vistime);
	void (*oxdirall_c)(oxdir d, void *starg, int vistime);
	void (*oxconall_c)(oxcon d, void *starg, int vistime);
} oxcbs_t;



#define PREVISIT  1  /* Also used for simple visit; this is the default. */
#define POSTVISIT 2
#define PREPOSTVISIT (PREVISIT | POSTVISIT)
#define MIDVISIT  4  /* Only used in ASS_OP, DECLARATION */

typedef struct {
	int        when;        /* PRE/POST/MID visit */

	int        doexpr;      /* Set to false to avoid visiting such nodes */
	int        dospec;
	int        dodecl;
	int        doomp;       /* Even if false, construct body is always visited */
	int        doox;        /* Even if false, construct body is always visited */
	
	void      *starg;       /* State (argument) passed to callback functions */

	stmtcbs_t  stmtc;
	exprcbs_t  exprc;
	speccbs_t  specc;
	declcbs_t  declc;
	ompclcbs_t ompclausec;
	ompdccbs_t ompdcc;
	oxcbs_t    oxc;
} travopts_t;

/* While setting manualy every field of the structs may be fulfilling,
 * it is not the easiest (or shortest) thing to do. The following create
 * a batch-filled struct; fields can then be differentiated seperately.
 */
extern void travopts_init_noop(travopts_t *trop);       /* Everything NULL */
extern void travopts_init_batch(travopts_t *trop, 
                void (*stmtcb)(aststmt t,        void *starg, int vistime), 
                void (*exprcb)(astexpr t,        void *starg, int vistime),
                void (*speccb)(astspec t,        void *starg, int vistime), 
                void (*declcb)(astdecl t,        void *starg, int vistime),
                void (*ompclausecb)(ompclause t, void *starg, int vistime), 
                void (*ompdircb)(ompdir t,       void *starg, int vistime),
                void (*ompconcb)(ompcon t,       void *starg, int vistime), 
                void (*oxclausecb)(oxclause t,   void *starg, int vistime),
                void (*oxdircb)(oxdir t,         void *starg, int vistime), 
                void (*oxconcb)(oxcon t,         void *starg, int vistime));

extern void ast_stmt_traverse(aststmt tree,        travopts_t *trop);
extern void ast_expr_traverse(astexpr tree,        travopts_t *trop);
extern void ast_spec_traverse(astspec tree,        travopts_t *trop);
extern void ast_decl_traverse(astdecl tree,        travopts_t *trop);
extern void ast_ompclause_traverse(ompclause tree, travopts_t *trop);
extern void ast_ompdir_traverse(ompdir tree,       travopts_t *trop);
extern void ast_ompcon_traverse(ompcon tree,       travopts_t *trop);
extern void ast_oxclause_traverse(oxclause tree,   travopts_t *trop);
extern void ast_oxdir_traverse(oxdir tree,         travopts_t *trop);
extern void ast_oxcon_traverse(oxcon tree,         travopts_t *trop);

#endif
