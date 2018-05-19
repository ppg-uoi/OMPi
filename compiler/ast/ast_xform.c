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

/* ast_xform.c */

/*
 * 2010/10/20:
 *   fixed transformation omission of return statements.
 * 2009/12/10:
 *   thread/task functions now placed *after* the parent function so that
 *   recursion works without any warnings.
 * 2009/05/04:
 *   fixed unbelievable omission in master,ordered,atomic & critical constructs
 * 2007/09/04:
 *   brand new thread funcs handling
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ast_copy.h"
#include "ast_free.h"
#include "ast_print.h"
#include "ast_vars.h"
#include "ast_xform.h"
#include "boolean.h"
#include "dfa.h"
#include "x_clauses.h"
#include "x_parallel.h"
#include "x_arith.h"
#include "x_task.h"
#include "x_single.h"
#include "x_sections.h"
#include "x_for.h"
#include "x_thrpriv.h"
#include "x_shglob.h"
#include "x_target.h"
#include "x_types.h"
#include "x_cars.h"
#include "ox_xform.h"
#include "ompi.h"

/*
 * 2009/12/09:
 *   added forgotten type transforms in selection & iteration stmt expressions
 * 2009/05/03:
 *   fixed bug (no-xfrom) in barrier
 */


/* The scope level of the most recently encountered parallel construct
 */
int closest_parallel_scope = -1;

/* Used in ast_declare_varlist_vars for determining if a variable appears in a
 * target data clause in order to set isindevenv to true.
 */
int target_data_scope = -1;

/* Used in outline.c for setting isindevenv of functions and here to prevent
 * target pragmas inside declare target
 */
bool inDeclTarget = false;

/* Used for determining the label for the cancel parallel GOTOs
 */
static int cur_parallel_line = 0;

/* Used for determining the label for the cancel taskgroup GOTOs
 */
static int cur_taskgroup_line = 0;

static aststmt newglobals = NULL, newtail = NULL;


stentry newglobalvar(aststmt s)
{
	astdecl decl;
	stentry e;

	if (newglobals == NULL)
		newglobals = s;
	else
	{
		aststmt t = newglobals;
		newglobals = BlockList(t, s);
		t->parent = newglobals;          /* Parentize correctly */
		s->parent = newglobals;
	}

	assert(s->type == DECLARATION);    /* Declare it, too */
	decl = s->u.declaration.decl;
	e = symtab_insert_global(stab, decl_getidentifier_symbol(decl), IDNAME);
	if (decl->type == DINIT)
		decl = (e->idecl = decl)->decl;
	e->decl       = decl;
	e->spec       = s->u.declaration.spec;
	e->isarray    = (decl_getkind(decl) == DARRAY);
	e->isthrpriv  = false;
	e->scopelevel = 0;

	/* If we are in a target, add the variable to the current target tree */
	if (inTarget())
	{
		//targnewglobals = targnewglobals ? BlockList(targnewglobals, s) : s;
		declaretarget_inject_ident( decl_getidentifier_symbol(decl) );
		e->isindevenv = 3;
	}
	/* VVD--If in a declare target, add the variable to the totally global list */
	if (inDeclTarget)
	{
		declaretarget_inject_ident( decl_getidentifier_symbol(decl) );
		e->isindevenv = 1;
	}

	return (e);
}


static void aststmt_add(aststmt *to, aststmt s)
{
	if (*to == NULL)
		*to = s;
	else
	{
		aststmt t = *to;
		*to = BlockList(t, s);
		t->parent = *to;          /* Parentize correctly */
		s->parent = *to;
	}
}


void head_add(aststmt s)
{
	aststmt_add(&newglobals, s);
}


void tail_add(aststmt s)
{
	aststmt_add(&newtail, s);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     AST TRAVERSAL OF NON-OPENMP NODES                         *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* The only thing we do here is correctly start/end scopes and
 * declare all identifiers. Whenever we insert a symbol into the
 * symbol table, we record its specifiers/declarator so as to
 * find it later if needed.
 */


void ast_stmt_iteration_xform(aststmt *t)
{
	switch ((*t)->subtype)
	{
		case SFOR:
			if ((*t)->u.iteration.init != NULL)
				ast_stmt_xform(&((*t)->u.iteration.init));
			if ((*t)->u.iteration.cond != NULL)
				ast_expr_xform(&((*t)->u.iteration.cond));
			if ((*t)->u.iteration.incr != NULL)
				ast_expr_xform(&((*t)->u.iteration.incr));
			ast_stmt_xform(&((*t)->body));
			break;
		case SWHILE:
			if ((*t)->u.iteration.cond != NULL)
				ast_expr_xform(&((*t)->u.iteration.cond));
			ast_stmt_xform(&((*t)->body));
			break;
		case SDO:
			if ((*t)->u.iteration.cond != NULL)
				ast_expr_xform(&((*t)->u.iteration.cond));
			ast_stmt_xform(&((*t)->body));
			break;
	}
}

void ast_stmt_selection_xform(aststmt *t)
{
	switch ((*t)->subtype)
	{
		case SSWITCH:
			ast_expr_xform(&((*t)->u.selection.cond));
			ast_stmt_xform(&((*t)->body));
			break;
		case SIF:
			ast_expr_xform(&((*t)->u.selection.cond));
			ast_stmt_xform(&((*t)->body));
			if ((*t)->u.selection.elsebody)
				ast_stmt_xform(&((*t)->u.selection.elsebody));
			break;
	}
}

void ast_stmt_labeled_xform(aststmt *t)
{
	ast_stmt_xform(&((*t)->body));
}


/* Only statement types that matter are considered. */
void ast_stmt_xform(aststmt *t)
{
	if (t == NULL || *t == NULL) return;

	switch ((*t)->type)
	{
		case EXPRESSION:
			ast_expr_xform(&((*t)->u.expr));
			break;
		case ITERATION:
			ast_stmt_iteration_xform(t);
			break;
		case JUMP:   /* 2010-10-20 (PEH) */
			if ((*t)->subtype == SRETURN && (*t)->u.expr != NULL)
				ast_expr_xform(&((*t)->u.expr));
			break;
		case SELECTION:
			ast_stmt_selection_xform(t);
			break;
		case LABELED:
			ast_stmt_labeled_xform(t);
			break;
		case COMPOUND:
			if ((*t)->body)
			{
				scope_start(stab);
				ast_stmt_xform(&((*t)->body));
				scope_end(stab);
			}
			break;
		case STATEMENTLIST:
			ast_stmt_xform(&((*t)->u.next));
			ast_stmt_xform(&((*t)->body));
			break;
		case DECLARATION:
			xt_declaration_xform(t);   /* transform & declare */
			break;
		case FUNCDEF:
			/* First declare the function */
			if (symtab_get(stab, decl_getidentifier_symbol((*t)->u.declaration.decl),
			               FUNCNAME) == NULL)
				symtab_put(stab, decl_getidentifier_symbol((*t)->u.declaration.decl),
				           FUNCNAME);

			scope_start(stab);         /* New scope here */

			if ((*t)->u.declaration.dlist) /* Old style */
			{
				ast_stmt_xform(&((*t)->u.declaration.dlist));  /* declared themselves */
				xt_dlist_array2pointer((*t)->u.declaration.dlist);  /* !!array params */
			}
			else                           /* Normal; has paramtypelist */
			{
				/* Used to do xt_barebones_decl((*t)->u.declaration.decl);
				 * Bug detected by sagathos, 2009/12/04.
				 */
				xt_barebones_substitute(&((*t)->u.declaration.spec),
				                        &((*t)->u.declaration.decl));
				ast_declare_function_params((*t)->u.declaration.decl);/* decl manualy */
				/* take care of array params */
				xt_decl_array2pointer((*t)->u.declaration.decl->decl->u.params);
			}
			ast_stmt_xform(&((*t)->body));
			tp_fix_funcbody_gtpvars((*t)->body);    /* take care of gtp vars */

			if (processmode)
				analyze_pointerize_sgl(*t);        /* just replace them with pointers */

			scope_end(stab);           /* Scope done! */
			break;
		case OMPSTMT:
			if (enableOpenMP || testingmode) ast_omp_xform(t);
			break;
		case OX_STMT:
			if (enableOmpix || testingmode) ast_ompix_xform(t);
			break;
	}
}


/* Expressions are only examined for type casts and sizeof(type) */
void ast_expr_xform(astexpr *t)
{
	if (t == NULL || *t == NULL) return;
	switch ((*t)->type)
	{
		case CONDEXPR:
			ast_expr_xform(&((*t)->u.cond));
		/* Then continue below */
		case FUNCCALL:
		case BOP:
		case ASS:
		case DESIGNATED:
		case COMMALIST:
		case SPACELIST:
		case ARRAYIDX:
			if ((*t)->right)
				ast_expr_xform(&((*t)->right));
		/* Then continue below */
		case DOTFIELD:
		case PTRFIELD:
		case BRACEDINIT:
		case PREOP:
		case POSTOP:
		case IDXDES:
			ast_expr_xform(&((*t)->left));
			break;
		case CASTEXPR:
			xt_barebones_substitute(&((*t)->u.dtype->spec), &((*t)->u.dtype->decl));
			ast_expr_xform(&((*t)->left));
			break;
		case UOP:
			if ((*t)->opid == UOP_sizeoftype || (*t)->opid == UOP_typetrick)
				xt_barebones_substitute(&((*t)->u.dtype->spec), &((*t)->u.dtype->decl));
			else
				ast_expr_xform(&((*t)->left));
			break;
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OPENMP NODES TO BE TRANSFORMED                            *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void ast_ompcon_xform(ompcon *t)
{
	if ((*t)->body)     /* barrier & flush don't have a body! */
		ast_stmt_xform(&((*t)->body));
}


static
aststmt linepragma(int line, symbol file)
{
	return ((cppLineNo) ? verbit("# %d \"%s\"", line, file->name) : verbit(""));
}


/* This simply prints the directive into a string and encloses it
 * in comments, adding also the source line number.
 * It returns a verbatim node with the comment.
 */
aststmt ompdir_commented(ompdir d)
{
	static str bf = NULL;
	aststmt    st;

	if (bf == NULL) bf = Strnew();
	str_printf(bf, "/* (l%d) ", d->l);
	ast_ompdir_print(bf, d);
	str_printf(bf, " */");

	st = BlockList(Verbatim(strdup(str_string(bf))),
	               linepragma(d->l, d->file));
	str_truncate(bf);
	return (st);
}

/**
 * Reproduces the original declaration which may optionally have an initializer
 * or convert the variable into a pointer
 *
 * @todo "const" is not reproduced giving a warning
 *
 * @param s         The variable whose declaration we want to reproduce
 * @param initer    An optional initializer
 * @param mkpointer If true the variable will be declared as a pointer
 * @return          A statement with the declaration
 */
inline aststmt xform_clone_declaration(symbol s, astexpr initer, bool mkpointer)
{
	stentry tmp  = symtab_get(stab, s, IDNAME);
	astdecl decl = mkpointer ? xc_decl_topointer(ast_decl_copy(tmp->decl)) :
	               ast_decl_copy(tmp->decl);
	return (Declaration(
	          ast_spec_copy_nosc(tmp->spec),
	          initer ?
	          InitDecl(
	            decl,
	            initer
	          ) :
	          decl
	        )
	       );
}


/**
 * Reproduces the original declaration of a function
 *
 * @param funcname  The function whose declaration we want to reproduce
 * @return          A statement with the declaration
 */
inline aststmt xform_clone_funcdecl(symbol funcname)
{
	stentry func  = symtab_get(stab, funcname, FUNCNAME);
	assert(func != NULL);
	return (Declaration(ast_spec_copy(func->spec), ast_decl_copy(func->decl)));
}


/* This one checks whether an openmp construct should include an
 * implicit barrier. This is needed only if the nowait is NOT present.
 * It tries to avoid the barrier by checking if it is redundant:
 *    if the construct is the last statement in the body of another
 *    construct which ends there and does have a barrier, then we don't
 *    need to output one more here.
 */
int xform_implicit_barrier_is_needed(ompcon t)
{
	aststmt p = t->parent;    /* the openmp statement */

	/* We must go up the tree to find the closest BlockList ancestor who has
	 * us as the rightmost leaf (i.e. last statement).
	 */
	for (; p->parent->type == STATEMENTLIST && p->parent->body == p; p = p->parent)
		;
	/* Now, climb up the compounds */
	for (; p->parent->type == COMPOUND; p = p->parent)
		;
	/* We may be in luck only if p is an openmp statement */
	if ((p = p->parent)->type == OMPSTMT)
	{
		/* Now this statement must be a convenient & blocking one; loops won't do */
		t = p->u.omp;
		if (t->type == DCPARSECTIONS || t->type == DCSECTIONS ||
		    t->type == DCPARALLEL || t->type == DCSINGLE)
			if (xc_ompcon_get_clause(t, OCNOWAIT) == NULL)
				return (0);  /* Not needed after all! */
	}
	return (1);        /* Needed */
}


void xform_master(aststmt *t)
{
	aststmt s = (*t)->u.omp->body, parent = (*t)->parent, v;
	bool    parisif;

	v = ompdir_commented((*t)->u.omp->directive); /* Put directive in comments */

	/* Check if our parent is an IF statement */
	parisif = (*t)->parent->type == SELECTION && (*t)->parent->subtype == SIF;

	(*t)->u.omp->body = NULL;     /* Make it NULL so as to free t easily */
	ast_free(*t);                 /* Get rid of the OmpStmt */

	/* Here we play a bit risky: this might be a single statement, and we
	 * convert it to a block list; it seems that it won't create problems;
	 * should we better turn it into a compound?
	 */
	/* Explanation of the following code:
	 * We output an "if { ... }" thing. If our parent is an IF statement,
	 * we should enclose the whole thing in curly braces { } (i.e. create a
	 * compound) so as not to cause syntactic problems with a possibly
	 * following "else" statement.
	 */
	*t = BlockList(
	       BlockList(
	         v,
	         If( /* omp_get_thread_num() == 0 */
	           BinaryOperator(
	             BOP_eqeq,
	             Call0_expr("omp_get_thread_num"),
	             numConstant(0)
	           ),
	           s,
	           NULL
	         )
	       ),
	       linepragma(s->l + 1 - parisif, s->file)
	     );
	if (parisif)
		*t = Compound(*t);
	ast_stmt_parent(parent, *t);
}


/* In many transforms we use an "stlist" flag. This is to
 * check if our parent is a statement list etc so as to
 * avoid enclosing the produced code into a compound statement.
 * If one does not care for the looks, the produced code
 * should *always* be enlosed into a compound statement.
 */


void xform_ordered(aststmt *t)
{
	aststmt s = (*t)->u.omp->body, parent = (*t)->parent, v;
	bool    stlist;   /* See comment above */

	v = ompdir_commented((*t)->u.omp->directive); /* Put directive in comments */
	stlist = ((*t)->parent->type == STATEMENTLIST ||
	          (*t)->parent->type == COMPOUND);

	(*t)->u.omp->body = NULL;     /* Make it NULL so as to free t easily */
	ast_free(*t);                 /* Get rid of the OmpStmt */

	*t = BlockList(
	       BlockList(
	         BlockList(
	           BlockList(v, Call0_stmt("ort_ordered_begin")),
	           s
	         ),
	         Call0_stmt("ort_ordered_end")
	       ),
	       linepragma(s->l + 1 - (!stlist), s->file)
	     );
	if (!stlist)
		*t = Compound(*t);
	ast_stmt_parent(parent, *t);
}


void xform_critical(aststmt *t)
{
	aststmt s = (*t)->u.omp->body, parent = (*t)->parent, v;
	char    lock[128];
	bool    stlist;   /* See comment above */

	v = ompdir_commented((*t)->u.omp->directive); /* Put directive in comments */
	stlist = ((*t)->parent->type == STATEMENTLIST ||
	          (*t)->parent->type == COMPOUND);

	/* A lock named after the region name */
	if ((*t)->u.omp->directive->u.region)
		snprintf(lock, 127, "_ompi_crity_%s", (*t)->u.omp->directive->u.region->name);
	else
		strcpy(lock, "_ompi_crity");

	(*t)->u.omp->body = NULL;    /* Make it NULL so as to free it easily */
	ast_free(*t);                /* Get rid of the OmpStmt */

	if (!symbol_exists(lock))    /* Check for the lock */
		newglobalvar(Declaration(Declspec(SPEC_void),   /* Don't use omp_lock_t */
		                         Declarator(
		                           Pointer(),
		                           IdentifierDecl(Symbol(lock))
		                         )));
	*t = BlockList(
	       BlockList(
	         BlockList(
	           BlockList(
	             v,
	             Expression(   /* ort_critical_begin(&lock); */
	               FunctionCall(
	                 IdentName("ort_critical_begin"),
	                 UOAddress(IdentName(lock))
	               )
	             )
	           ),
	           s
	         ),
	         Expression(   /* ort_critical_end(&lock); */
	           FunctionCall(
	             IdentName("ort_critical_end"),
	             UOAddress(IdentName(lock))
	           )
	         )
	       ),
	       linepragma(s->l + 1 - (!stlist), s->file)
	     );

	if (!stlist)
		*t = Compound(*t);
	ast_stmt_parent(parent, *t);
}


void xform_taskgroup(aststmt *t)
{
	aststmt s = (*t)->u.omp->body, parent = (*t)->parent, v;
	bool    stlist;   /* See comment above */
	char    clabel[23];

	snprintf(clabel, 23, "CANCEL_taskgroup_%d", cur_taskgroup_line);
	
	v = ompdir_commented((*t)->u.omp->directive); /* Put directive in comments */
	stlist = ((*t)->parent->type == STATEMENTLIST ||
	          (*t)->parent->type == COMPOUND);

	(*t)->u.omp->body = NULL;    /* Make it NULL so as to free it easily */
	ast_free(*t);                /* Get rid of the OmpStmt */

	*t = BlockList(
	       BlockList(
	         BlockList(
	           BlockList(
	             v,
	             Call0_stmt("ort_entering_taskgroup")
	           ),
	           s
	         ),
	         Labeled(
	           Symbol(clabel), /* label used for cancel */
	           Call0_stmt("ort_leaving_taskgroup")
	         )
	       ),
	       linepragma(s->l + 1 - (!stlist), s->file)
	     );

	if (!stlist)
		*t = Compound(*t);
	ast_stmt_parent(parent, *t);
}


void xform_atomic(aststmt *t)
{
	aststmt s = (*t)->u.omp->body, parent = (*t)->parent, v;
	astexpr ex = s->u.expr;
	bool    stlist;   /* See comment above */

	if ((s->type != EXPRESSION) ||
	    (ex->type != POSTOP && ex->type != PREOP && ex->type != ASS))
		exit_error(1, "(%s, line %d) openmp error:\n\t"
		           "non-compliant ATOMIC expression.\n",
		           (*t)->u.omp->directive->file->name, (*t)->u.omp->directive->l);

	v = ompdir_commented((*t)->u.omp->directive); /* Put directive in comments */
	stlist = ((*t)->parent->type == STATEMENTLIST ||
	          (*t)->parent->type == COMPOUND);

	(*t)->u.omp->body = NULL;     /* Make it NULL so as to free t easily */
	ast_free(*t);                 /* Get rid of the OmpStmt */

	if (ex->type == ASS &&
	    (ex->right->type != IDENT && !xar_expr_is_constant(ex->right)))
	{
		aststmt tmp;

		tmp = Declaration(
		        (ex->left->type != IDENT ?
		         Declspec(SPEC_long) :
		         ast_spec_copy_nosc(
		           symtab_get(stab, ex->left->u.sym, IDNAME)->spec)
		        ),
		        InitDecl(
		          Declarator(NULL, IdentifierDecl(Symbol("__tmp"))),
		          ex->right
		        )
		      );
		ex->right = IdentName("__tmp");
		*t = Compound(
		       BlockList(
		         BlockList(
		           BlockList(
		             BlockList(BlockList(v, tmp), Call0_stmt("ort_atomic_begin")),
		             s
		           ),
		           Call0_stmt("ort_atomic_end")
		         ),
		         linepragma(s->l + 1 - (!stlist), s->file)
		       )
		     );
	}
	else
	{
		*t = BlockList(
		       BlockList(
		         BlockList(
		           BlockList(v, Call0_stmt("ort_atomic_begin")),
		           s
		         ),
		         Call0_stmt("ort_atomic_end")
		       ),
		       linepragma(s->l + 1 - (!stlist), s->file)
		     );
		if (!stlist)
			*t = Compound(*t);
	}
	ast_stmt_parent(parent, *t);
}


void xform_flush(aststmt *t)
{
	aststmt parent = (*t)->parent, v;

	v = ompdir_commented((*t)->u.omp->directive);  /* Put directive in comments */
	ast_free(*t);                                  /* Cut the tree here */
	/* flush occurs ONLY as a blocklist */
	*t = BlockList(v, Call0_stmt("ort_fence"));
	ast_stmt_parent(parent, *t);                   /* This MUST BE DONE!! */
}

/* Used to be a macro in ast_xform.h. Was turned into a function to incorporate
 * cancel logic.
 */
aststmt BarrierCall()
{
	char label[23], mabel[23];
	
	if (!cur_parallel_line && !cur_taskgroup_line)
		return Call0_stmt("ort_barrier_me");
	if (cur_parallel_line && !cur_taskgroup_line)
	{
		snprintf(label, 23, "CANCEL_parallel_%d", cur_parallel_line);
		/* if(ort_barrier_me()) goto <label>; */
		return If(BinaryOperator(BOP_eqeq, 
	                             Call0_expr("ort_barrier_me"), numConstant(1)), 
	              Goto(Symbol(label)), 
	              NULL);
	}
	if (!cur_parallel_line && cur_taskgroup_line)
	{
		snprintf(label, 23, "CANCEL_taskgroup_%d", cur_taskgroup_line);
		/* if(ort_barrier_me()) goto <label>; */
		return If(BinaryOperator(BOP_eqeq, 
	                             Call0_expr("ort_barrier_me"), numConstant(2)), 
	              Goto(Symbol(label)), 
	              NULL);
	}
	/* 4th case: both parallel and taskgroup constructs active */
	snprintf(label, 23, "CANCEL_parallel_%d", cur_parallel_line);
	snprintf(mabel, 23, "CANCEL_taskgroup_%d", cur_taskgroup_line);
	Switch(Call0_expr("ort_barrier_me"),
	  Compound(
	    BlockList(
	      BlockList(
	        Case(numConstant(0), Break()), 
	        Case(numConstant(1), Goto(Symbol(label)))
	      ),
	      Case(numConstant(2), Goto(Symbol(mabel)))
	    )
	  )
	);
}

void xform_barrier(aststmt *t)
{
	aststmt parent = (*t)->parent, v;

	v = ompdir_commented((*t)->u.omp->directive);
	ast_free(*t);
	*t = BlockList(v, BarrierCall());
	ast_stmt_parent(parent, *t);
}


void xform_taskwait(aststmt *t)
{
	aststmt parent = (*t)->parent, v;

	v = ompdir_commented((*t)->u.omp->directive);
	ast_free(*t);
	*t = BlockList(v, FuncCallStmt(IdentName("ort_taskwait"),
	                               numConstant(0)));
	ast_stmt_parent(parent, *t);
}


void xform_taskyield(aststmt *t)
{
	aststmt parent = (*t)->parent, v;

	v = ompdir_commented((*t)->u.omp->directive);
	ast_free(*t);
	*t = v;            /* Just ignore it for now */

	ast_stmt_parent(parent, *t);
}


static void cancel_error(aststmt *t, char *ctype, char *etype)
{
	exit_error(1, "(%s, line %d) openmp error:\n\t"
	           "\"cancel %s\" must be closely nested inside a \"%s\" construct\n",
	           (*t)->u.omp->directive->file->name, (*t)->u.omp->directive->l,
	           ctype, etype);
}


void xform_cancel_type(aststmt *t, int *type, char label[22])
{
	ompclause c;
	ompcon    enclosing_con = ast_get_enclosing_ompcon((*t)->parent, 0);

	if ((c = xc_ompcon_get_clause((*t)->u.omp, OCPARALLEL)) != NULL)
	{
		if (enclosing_con->type != DCPARALLEL)
			cancel_error(t, "parallel", "parallel");

		*type = 0;
	}
	else
		if ((c = xc_ompcon_get_clause((*t)->u.omp, OCTASKGROUP)) != NULL)
		{
			if (enclosing_con->type != DCTASK)
				cancel_error(t, "taskgroup", "task");

			*type = 1;
		}
		else
			if ((c = xc_ompcon_get_clause((*t)->u.omp, OCFOR)) != NULL)
			{
				if (enclosing_con->type != DCFOR && enclosing_con->type != DCFOR_P)
					cancel_error(t, "for", "for");

				*type = 2;
			}
			else
			{
				c = xc_ompcon_get_clause((*t)->u.omp, OCSECTIONS);
				if (enclosing_con->type != DCSECTIONS)
					cancel_error(t, "sections", "sections");

				*type = 3;
			}
	assert(c != NULL);

	snprintf(label, 22, "CANCEL_%s_%d", ompdirnames[enclosing_con->type],
	         enclosing_con->l);
}


void xform_cancel(aststmt *t)
{
	astexpr   ifexpr = NULL;
	aststmt   parent = (*t)->parent, v;
	int       type = -1;
	char      label[22];
	ompclause c;

	/* Find and store "if" clause */
	if ((c = xc_ompcon_get_clause((*t)->u.omp, OCIF)) != NULL)
		ifexpr = ast_expr_copy(c->u.expr);

	/* Get label and type */
	xform_cancel_type(t, &type, label);

	v = ompdir_commented((*t)->u.omp->directive);
	ast_free(*t);

	if (ifexpr == NULL)
		/* if (ort_enable_cancel(<type>))
		 *   goto <label>;
		 */
		*t = BlockList(v,
		               If(
		                 FunctionCall(
		                   IdentName("ort_enable_cancel"), numConstant(type)
		                 ),
		                 Goto(Symbol(label)),
		                 NULL
		               )
		              );
	else
		/* if (<ifexpr>)
		 *   if (ort_enable_cancel(<type>))
		 *     goto <label>;
		 * else
		 *   if (ort_check_cancel(<type>))
		 *     goto <label>;
		 */
		*t = BlockList(v,
		               If(ifexpr,
		                  If(
		                    FunctionCall(
		                      IdentName("ort_enable_cancel"), numConstant(type)
		                    ),
		                    Goto(Symbol(label)),
		                    NULL
		                  ),
		                  If(
		                    FunctionCall(
		                      IdentName("ort_check_cancel"), numConstant(type)
		                    ),
		                    Goto(Symbol(label)),
		                    NULL
		                  )
		                 )
		              );
	ast_stmt_parent(parent, *t);
}


void xform_cancellationpoint(aststmt *t)
{
	aststmt   parent = (*t)->parent, v;
	int       type = -1;
	char      label[22];

	/* Get label and type */
	xform_cancel_type(t, &type, label);

	v = ompdir_commented((*t)->u.omp->directive);
	ast_free(*t);

	/* if (ort_check_cancel(<type>))
	 *   goto <label>;
	 */
	*t = BlockList(v,
	               If(
	                 FunctionCall(
	                   IdentName("ort_check_cancel"), numConstant(type)
	                 ),
	                 Goto(Symbol(label)),
	                 NULL
	               )
	              );

	ast_stmt_parent(parent, *t);
}


/* See below for comments regarding the following func,
 * which declares fully all private-scope vars
 */
static
void declare_private_dataclause_vars(ompclause t)
{
	if (t == NULL) return;
	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			declare_private_dataclause_vars(t->u.list.next);
		assert((t = t->u.list.elem) != NULL);
	}
	switch (t->type)
	{
		case OCPRIVATE:
		case OCFIRSTPRIVATE:
		case OCLASTPRIVATE:
		case OCCOPYPRIVATE:
		case OCREDUCTION:
		case OCCOPYIN:
		case OCMAP:
			ast_declare_varlist_vars(t->u.varlist, t->type, t->subtype);
			break;
	}
}


/* We must first transform the body of the construct and then
 * do the transformation of the construct itself (i.e. create new code
 * based on the clauses).
 * BUT, FOR CONSTRUCT WITH DATA CLAUSES:
 *   before transforming the body, we have to *declare* (fully)
 *   all the identifiers in the clauses that have *private* scope,
 *   so that in the body those variables have the correct visibility.
 *   (the others don't need any treatment as they have already been
 *    declared in previous scopes).
 *
 * Thus, we begin our new scope, declare them, transform the body
 * and finaly close the scope (ditching the declarations).
 * Note that the declarations, DO NOT PRODUCE ANY NEW CODE; they
 * are simply inserted in the symbol table for the correct visibility
 * of variables used in the body. The declarations are produced when
 * we transform the construct itself.
 */
static
void xform_ompcon_body(ompcon t)
{
	if (t->body->type == COMPOUND && t->body->body == NULL)
		return;      /* degenerate case */
	
	xc_validate_only_dataclause_vars(t->directive);
	scope_start(stab);

	/* If we are in a target data put __ort_denv in the symbol table, so that if
	 * there is any clause that uses outline in the body it will put __ort_denv
	 * in the struct it creates.
	 */
	if (t->type == DCTARGETDATA || t->type == DCTARGET)
	{
		aststmt tmp = get_denv_var_decl(true);
		ast_declare_decllist_vars(tmp->u.declaration.spec, tmp->u.declaration.decl);
	}

	declare_private_dataclause_vars(t->directive->clauses);
	ast_stmt_xform(&(t->body));     /* First transform the body */
	scope_end(stab);
}


/* Degenerate case: check whether an empty-bodied omp construct is given.
 * If so, replace it by a comment.
 */
int empty_bodied_omp(aststmt *t, char *conname)
{
	if ((*t)->u.omp->body->type == COMPOUND && (*t)->u.omp->body->body == NULL)
	{
		ast_stmt_free(*t);
		*t = verbit("/* (l%d) empty %s construct removed */", (*t)->l, conname);
		return (1);
	}
	return (0);
}


void ast_omp_xform(aststmt *t)
{
	int combined = 0;   /* for combined parallel for/sections */

	/* First do a check on the clauses */
	xc_validate_clauses((*t)->u.omp->type, (*t)->u.omp->directive->clauses);

	switch ((*t)->u.omp->type)
	{
		case DCTHREADPRIVATE:
			xform_threadprivate(t);
			break;
		case DCATOMIC:
			ast_stmt_xform(&((*t)->u.omp->body));     /* First transform the body */
			xform_atomic(t);
			break;
		case DCBARRIER:
			xform_barrier(t);
			break;
		case DCTASKWAIT:
			xform_taskwait(t);
			break;
		case DCTASKYIELD:
			xform_taskyield(t);
			break;
		case DCCRITICAL:
			ast_stmt_xform(&((*t)->u.omp->body));     /* First transform the body */
			xform_critical(t);
			break;
		case DCFLUSH:
			xform_flush(t);
			break;
		case DCMASTER:
			ast_stmt_xform(&((*t)->u.omp->body));     /* First transform the body */
			xform_master(t);
			break;
		case DCORDERED:
			ast_stmt_xform(&((*t)->u.omp->body));     /* First transform the body */
			xform_ordered(t);
			break;
		case DCTASK:
			if (!empty_bodied_omp(t, "task"))
			{
				xform_ompcon_body((*t)->u.omp);
				xform_task(t);
			}
			break;
		case DCTASKGROUP:
			if (!empty_bodied_omp(t, "taskgroup"))
			{
				int save_tgl = cur_taskgroup_line;
				
				cur_taskgroup_line = (*t)->l;    /* save */
				xform_ompcon_body((*t)->u.omp);
				xform_taskgroup(t);
				cur_taskgroup_line = save_tgl;   /* restore */
			}
			break;
		case DCSINGLE:
			xform_ompcon_body((*t)->u.omp);
			xform_single(t);
			break;
		case DCSECTIONS:
			xform_ompcon_body((*t)->u.omp);
			xform_sections(t);
			break;
		case DCSECTION:
			/* To avoid falling into default case; Handled within
			 * xform_ompcon_body of DCSECTIONS case.
			 */
			ast_ompcon_xform(&((*t)->u.omp));
			break;
		case DCFOR:
		case DCFOR_P:       /* Need optimized transformation */
			xform_ompcon_body((*t)->u.omp);
			xform_for(t);
			break;
		case DCPARSECTIONS:
		case DCPARFOR:
		{
			/* Here, we just create 2 separate statements; an openmp parallel and
			 * and an openmp sections/for; the clauses are split and assigned to
			 * the appropriate statement. The new sections/for statement assumes
			 * a little different node in the tree.
			 * We conclude by transforming the parallel statement.
			 */
			ompclause       pc = NULL, wc = NULL;
			enum dircontype type = ((*t)->u.omp->type == DCPARFOR ?
			                        DCFOR_P : DCSECTIONS);  /* Special node */

			if (empty_bodied_omp(t, "combined parallel"))
				break;

			combined = 1;
			if ((*t)->u.omp->directive->clauses)
				xc_split_combined_clauses((*t)->u.omp->directive->clauses, &pc, &wc);
			ast_ompclause_free((*t)->u.omp->directive->clauses);   /* Not needed */
			(*t)->u.omp->type = DCPARALLEL;                   /* Change the node */
			(*t)->u.omp->directive->type = DCPARALLEL;
			(*t)->u.omp->directive->clauses = pc;
			(*t)->u.omp->body = OmpStmt(                      /* New body */
			                      OmpConstruct(
			                        type,
			                        OmpDirective(type, wc),
			                        (*t)->u.omp->body
			                      )
			                    );
			(*t)->u.omp->body->file =                    /* Fix infos */
			  (*t)->u.omp->body->u.omp->file =
			    (*t)->u.omp->body->u.omp->directive->file =
			      (*t)->u.omp->directive->file;
			(*t)->u.omp->body->l =
			  (*t)->u.omp->body->u.omp->l =
			    (*t)->u.omp->body->u.omp->directive->l =
			      (*t)->u.omp->directive->l;
			(*t)->u.omp->body->c =
			  (*t)->u.omp->body->u.omp->c =
			    (*t)->u.omp->body->u.omp->directive->c =
			      (*t)->u.omp->directive->c;
			ast_stmt_parent((*t)->parent, (*t));   /* Reparentize correctly */
		}
		/* continues directly to DCPARALLEL */
		case DCPARALLEL:
		{
			int savescope = closest_parallel_scope;
			int savecpl   = cur_parallel_line;
			int savectgl  = cur_taskgroup_line;

			if (empty_bodied_omp(t, "parallel"))
				break;

			cur_parallel_line = (*t)->l;
			cur_taskgroup_line = 0; /* New team => forget old taskgroups */
			/*******************************/
			/*          Agelos             */
			/*******************************/
			/* If it's already analyzed, do nothing.
			 * Else perform the analysis */
			/* TODO: add when bug fixed
			if (isInAutoList(cur_parallel_line) == NULL)
				dfa_analyse(*t);
			*/
			/*******************************/
			/*                             */
			/*******************************/

			closest_parallel_scope = stab->scopelevel;
			xform_ompcon_body((*t)->u.omp);
			xform_parallel(t, combined);     /* Now do the actual transformation */

			dfa_removeFromAutoList(cur_parallel_line);

			closest_parallel_scope = savescope;
			cur_parallel_line = savecpl;
			cur_taskgroup_line = savectgl;
			break;
		}
		case DCTARGET:
		{
			int savecpl   = cur_parallel_line;
			int savectgl  = cur_taskgroup_line;

			if (empty_bodied_omp(t, "target"))
				break;

			cur_parallel_line = 0;  /* Reset */
			cur_taskgroup_line = 0; /* Reset */
			
			/* Turn the check into a macro to avoid repeating it 3 times */
			#define CHECKTARGET(TYPE) do {\
			if (inTarget())\
				exit_error(1, "(%s, line %d) openmp error:\n\t"\
				           TYPE " in target leads to undefined behavior\n",\
				           (*t)->u.omp->directive->file->name,\
				           (*t)->u.omp->directive->l);\
			\
			if (inDeclTarget)\
				exit_error(1, "(%s, line %d) openmp error:\n\t"\
				           TYPE " in declare target leads to undefined behavior\n",\
				           (*t)->u.omp->directive->file->name,\
				           (*t)->u.omp->directive->l);\
			} while(0)

			CHECKTARGET("Target");

			/* We add the comment to mark that we are in a "target".
			 * If outline is called while we transform the body it's code will
			 * be added to the target code.
			 */
			targtree = verbit("");    /* Dummy */

			cars_analyze_target((*t)->u.omp->body);  /* Before any transformations */  

			xform_ompcon_body((*t)->u.omp);
			xform_target(t);

			cur_parallel_line = savecpl;
			cur_taskgroup_line = savectgl;
			break;
		}
		case DCTARGETDATA:
		{
			if (empty_bodied_omp(t, "target data"))
				break;

			CHECKTARGET("Target data");

			/* We store the scope level of the target data and use it later in
			 * xform_ompcon_body -> declare_private_dataclause_vars to set
			 * isindevenv
			 */
			target_data_scope = stab->scopelevel;

			xform_ompcon_body((*t)->u.omp);
			xform_targetdata(t);

			target_data_scope = -1;
			break;
		}
		case DCTARGETUPD:
		{
			CHECKTARGET("Target update");
			#undef CHECKTARGET
			xform_targetupdate(t);
			break;
		}
		case DCDECLTARGET:
		{
			inDeclTarget = true;
			ast_stmt_xform(&((*t)->u.omp->body));     /* First transform the body */
			xform_declaretarget(t);
			inDeclTarget = false;
			break;
		}
		case DCCANCEL:
			xform_cancel(t);
			break;
		case DCCANCELLATIONPOINT:
			xform_cancellationpoint(t);
			break;
		default:
			fprintf(stderr, "WARNING: %s directive not implemented\n",
			        ompdirnames[(*t)->u.omp->type]);
			ast_ompcon_xform(&((*t)->u.omp));
			break;
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     THE LIST OF THREAD FUNCTIONS                              *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* All produced thread functions are inserted in this LIFO list.
 * They are transformed seperately and each one gets inserted in the AST,
 * just before the function that created it. The LIFO way guarantees
 * correct placement of nested threads.
 * The whole code here is based on the assumption that the FUNCDEF nodes
 * in the original AST won't change (which does hold since the
 * transformation code---see above---only transforms the body of the FUNCDEF,
 * not the FUNCDEF code itself). Otherwise the ->fromfunc pointers might
 * point to invalid nodes.
 */
typedef struct funclist_ *funclist;
struct funclist_
{
	aststmt  funcdef;        /* The thread/task function */
	aststmt  fromfunc;       /* Must be placed after this function */
	symbol   fname;          /* The name of the thread/task function */
	funclist next;
};
static funclist thrfuncs = NULL;


/* New funcs are inserted @ front */
void xform_add_threadfunc(symbol name, aststmt fd, aststmt curfunc)
{
	funclist e   = (funclist) smalloc(sizeof(struct funclist_));
	e->fname     = name;
	e->funcdef   = fd;
	e->fromfunc  = curfunc;
	e->next      = thrfuncs;
	thrfuncs     = e;
}


/* Takes the list with the produced thread functions (from xform_parallel()),
 * and transforms them. Notice that no new thread functions can be added here
 * since before a parallel construct is transformed, all nested constructs
 * have already been transformed (so there will be no #parallel in any
 * of the functions here).
 */
static
void xform_thread_functions(funclist l)
{
	if (l != NULL)
	{
		ast_stmt_xform(&(l->funcdef));
		if (l->next != NULL)
			xform_thread_functions(l->next);
	}
}


static
void place_thread_functions(funclist l)
{
	aststmt neu, bl;

	if (l == NULL) return;

	/* Replace l->fromfunc by a small BlockList; notice that anything pointing
	 * to the same func will now "see" this new blocklist.
	 *   ----> (pfunc) now becomes
	 *
	 *   ---> (BL) -----------> (BL2) --> (thrfunc)
	 *         \                   \
	 *          -->(thrfunc decl)   -->(pfunc)
	 */
	neu = (aststmt) smalloc(sizeof(struct aststmt_));
	*neu = *(bl = l->fromfunc);                     /* new node for pfunc */
	*(bl) = *BlockList(                             /* BL */
	          Declaration(                          /* thrfunc decl */
	            Speclist_right(StClassSpec(SPEC_static), Declspec(SPEC_void)),
	            Declarator(
	              Pointer(),
	              FuncDecl(
	                IdentifierDecl(l->fname) ,
	                ParamDecl(
	                  Declspec(SPEC_void),
	                  AbstractDeclarator(
	                    Pointer(),
	                    NULL
	                  )
	                )
	              )
	            )
	          ),
	          BlockList(neu, l->funcdef)  /* BL2 */
	        );
	bl->parent         = neu->parent;   /* Parentize (BL) */
	neu->parent        = bl->body;      /* pfuncs's parent */
	l->funcdef->parent = bl->body;      /* thrfunc's parent */
	bl->u.next->parent = bl;            /* Declarations's parent */
	bl->body->parent   = bl;            /* BL2's parent */
	bl->file           = NULL;
	bl->u.next->file   = NULL;
	bl->body->file     = NULL;
	l->funcdef->file   = NULL;

	if (l->next != NULL)
		place_thread_functions(l->next);
	free(l);    /* No longer needed */
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     TRANSFORM THE AST (ITS OPENMP NODES)                      *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


aststmt place_globals(aststmt tree)
{
	aststmt p;

	if (testingmode)
	{
		/* Just place them at the very beginning;
		 * it is guaranteed that the tree begins with a statementlist.
		 */
		for (p = tree; p->type == STATEMENTLIST; p = p->u.next)
			;               /* Go down to the leftmost leaf */
		p = p->parent;    /* Up to parent */
		p->u.next = BlockList(p->u.next, newglobals);
		p->u.next->parent       = p;           /* Parentize correctly */
		p->u.next->body->parent = p->u.next;
		newglobals->parent      = p->u.next;
		return (tree);
	}

	/* We put all globals right after __ompi_defs__'s declaration
	 * (see bottom of ort.defs).
	 */
	if (tree->type == STATEMENTLIST)
	{
		if ((p = place_globals(tree->u.next)) != NULL)
			return (p);
		else
			return (place_globals(tree->body));
	}

	/* Try to find where __ompi_defs__ is declared. */
	if (tree->type == DECLARATION &&
	    tree->u.declaration.decl != NULL &&
	    decl_getidentifier(tree->u.declaration.decl)->u.id ==
	    Symbol("__ompi_defs__"))
	{
		p = smalloc(sizeof(struct aststmt_));
		*p = *tree;
		if (cppLineNo)
			*tree = *BlockList(
			          p,
			          BlockList(
			            verbit("# 1 \"%s-newglobals\"", filename),
			            BlockList(
			              newglobals,
			              verbit("# 1 \"%s\"", filename)
			            )
			          )
			        );
		else
			*tree = *BlockList(p, newglobals);
		tree->parent = p->parent;
		p->parent = tree;
		newglobals->parent = tree;
		return (tree);
	}
	return (NULL);
}


void ast_xform(aststmt *tree)
{
	newglobals = NULL;
	thrfuncs   = NULL;
	newtail    = NULL;

	if (__has_target)
		cars_analyze_declared_funcs(*tree);   /* CARS analysis */
	
	ast_stmt_xform(tree);

	xform_thread_functions(thrfuncs);  /* xform & add the new thread/task funcs */
	place_thread_functions(thrfuncs);  /* place them wisely */
	thrfuncs = NULL;

	sgl_fix_sglvars(); /* Called before adding the tail, since it adds
                        something to it */

	produce_decl_var_code(); /* Adds to globals and tail */

	if (tree != NULL && newglobals != NULL) /* Add the new globals */
	{
		//    ast_stmt_xform(&newglobals);          /* Must do it ! (why??) */
		place_globals(*tree);
	}

	if (tree != NULL && newtail != NULL)    /* Append @ bottom */
		/* Cannot do ast_stmt_xform(&newtail). See x_shglob.c */
		*tree = BlockList(*tree, newtail);

	if (newglobals != NULL)
		newglobals = NULL;
	if (newtail != NULL);
	newtail = NULL;
}
