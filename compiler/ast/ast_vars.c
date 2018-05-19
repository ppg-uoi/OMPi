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

/* ast_vars.c -- declarations and analyses of variables in the AST.
 *
 *   The functions here (a) declare (i.e. put in the symbol table) given
 *   sets of variables and (b) discover particular variables used in a
 *   given portion of the AST. In particular, they discover:
 *
 *     (1) all global threadprivate (gtp) variables used
 *     (2) all global non-threadprivate (=> shared)
 *     (3) all non-global variables declared in previous scopes; this is
 *         used to discover all shared non-global vars in parallel regions.
 *
 *   The discovered variables are recorded in a symbol table (gtp_vars,
 *   sgl_vars and sng_vars respectively).
 *
 *   In any of those cases, each variable appearance may optionally be
 *   replaced by a pointer to this variable (X replaced by (*X))
 *   where it is expected that the pointer is properly declared and
 *   initialized elsewhere, during code transformations. Note however that
 *   in the case of (1), this option is NOT effective since this particular
 *   replacement HAS ALREADY BEEN PERFORMED BY THE parser.
 *
 *   (2) may seem (and actuall is) useless when using the thread model,
 *   since all global vars are by nature shared. However, it is useful
 *   in the process model, where nothing is shared and must be made so
 *   explicitely.
 *
 */

/*
 * 2015/07/27:
 *   re-based on ast_traverse.c
 * 2011/12/09:
 *   fixed an is_interesting_var()/mark_var() bug: now correctly recognize
 *   global firstprivate vars for tasks.
 * 2009/05/03:
 *   fixed an is_interesting_var() bug: now returns 0 if not interesting.
 *   added checks for new ompix ATNODE clause vars.
 * 2008/11/09:
 *   added support for tasks (fp/sng)
 * 2008/06/11:
 *   alive.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ast.h"
#include "symtab.h"
#include "ast_vars.h"
#include "ast_copy.h"
#include "ast_xform.h"
#include "ast_traverse.h"
#include "x_clauses.h"
#include "ompi.h"
#include "set.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     DECLARE A LIST OF VARIABLES                               *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Declare all variables appearing in a declaration list; could also be types */
void ast_declare_decllist_vars(astspec s, astdecl d)
{
	astdecl ini = NULL;

	if (d->type == DLIST && d->subtype == DECL_decllist)
	{
		ast_declare_decllist_vars(s, d->u.next);
		ast_declare_decllist_vars(s, d->decl);
		return;
	}
	if (d->type == DINIT) d = (ini = d)->decl;    /* Get rid of the initializer */
	if (d->type != DECLARATOR)
		exit_error(1, "[ast_declare_decllist_vars]: !!BUG!! not a DECLARATOR ?!\n");
	if (d->decl != NULL && d->decl->type != ABSDECLARATOR)
	{
		symbol  t;
		stentry e;
		int     kind = decl_getkind(d);

		/* Important note here:
		 *   we got rid of the initializer => e->decl has NO WAY OF KNOWING
		 *   whether it is plain or initialized (since there are no parent
		 *   relationships in non-statement nodes).
		 */
		t = decl_getidentifier_symbol(d->decl);

		/* Prevent adding function prototypes multiple times */
		if (kind == DFUNC && symtab_get(stab, t, FUNCNAME))
			return;

		e = symtab_put(stab, t, speclist_getspec(s, STCLASSSPEC, SPEC_typedef) ?
		               TYPENAME : (kind == DFUNC) ? FUNCNAME : IDNAME);
		e->spec      = s;
		e->decl      = d;
		e->idecl     = ini;
		e->isarray   = (kind == DARRAY);
		e->isthrpriv = false;
	}
}


/* Declare all variables in a varlist (i.e. in an OpenMP data clause) */
void ast_declare_varlist_vars(astdecl d, enum clausetype type,
                              enum clausesubt subtype)
{
	stentry e, orig;

	if (d->type == DLIST && d->subtype == DECL_idlist)
	{
		ast_declare_varlist_vars(d->u.next, type, subtype);
		d = d->decl;
	}
	if (d->type != DIDENT)
		exit_error(1, "[ast_declare_varlist_vars]: !!BUG!! not a DIDENT ?!\n");

	/* The identifier should be known! */
	if ((orig = symtab_get(stab, d->u.id, IDNAME)) == NULL)
		exit_error(1, "(%s, line %d) openmp error:\n\t"
		           "unknown identifier `%s'\n",
		           d->file->name, d->l, d->u.id->name);

	/* Arrays may not appear in a reduction clause. (OpenMP4.0.0.pdf:171:8) */
	if (type == OCREDUCTION && orig->isarray)
		exit_error(1, "(%s, line %d) openmp error:\n\t"
		           "reduction variable `%s' is non-scalar.\n",
		           d->file->name, d->l, d->u.id->name);

	/* Issue an error if variable is threadprivate (OpenMP4.0.0.pdf:179:13) */
	if (type == OCMAP && orig->isthrpriv)
		exit_error(1, "(%s, line %d) openmp error:\n\t"
		           "Threadprivate variable \"%s\" cannot appear in map clause\n",
		           d->file->name, d->l, d->u.id->name);

	/* Ignore variables in map clause that have appeared in a declare directive*/
	if (type == OCMAP && orig->isindevenv == 1)
		return;

	e = symtab_put(stab, d->u.id, IDNAME);  /* Declare new local var */
	e->decl      = orig->decl;                       /* Keep all infos */
	e->idecl     = orig->idecl;
	e->spec      = orig->spec;
	e->isarray   = orig->isarray;
	e->isthrpriv = orig->isthrpriv;

	e->ival      = type; /* Mark */
	e->vval      = subtype;

	/* Mark variables appearing in target data directive */
	if (type == OCMAP && target_data_scope + 1 == e->scopelevel)
		e->isindevenv = 2;
}


/* Declare function parameters */
static
void ast_declare_paramlist_vars(astdecl d)
{
	if (d->type == DLIST && d->subtype == DECL_paramlist)
	{
		ast_declare_paramlist_vars(d->u.next);
		d = d->decl;
	}
	if (d->type == DELLIPSIS)
		return;
	if (d->type != DPARAM)
		exit_error(1, "[ast_declare_paramlist_vars]: !!BUG!! not a DPARAM ?!\n");
	if (d->decl != NULL && d->decl->type != ABSDECLARATOR)
	{
		symbol  s;
		stentry e;

		s = decl_getidentifier_symbol(d->decl);
		e = symtab_put(stab, s, IDNAME);
		e->decl      = d->decl;
		e->spec      = d->spec;
		e->isarray   = (decl_getkind(d->decl) == DARRAY);
		e->isthrpriv = false;
	}
}


/* This takes all the parameters of the function declararion/definition
 * and declares them (insertes them in the symbol table). We are given
 * the declarator of the function.
 */
void ast_declare_function_params(astdecl d)
{
	while (d->type == DECLARATOR || d->type == ABSDECLARATOR || d->type == DPAREN)
		d = d->decl;    /* The "while" was added 2009/12/04 - thnx to sagathos */
	if (d == NULL || d->type != DFUNC)
		exit_error(1, "[ast_declare_function_params]: !!BUG!! not a FuncDef ?!\n");
	if ((d = d->u.params) != NULL)
		if (d->type == DPARAM || (d->type == DLIST && d->subtype == DECL_paramlist))
			ast_declare_paramlist_vars(d);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     TRAVERSE THE AST                                          *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static travopts_t *vartrops;

static void vars_compound(aststmt t, void *ignore, int vistime)
{
	if (vistime == PREVISIT)
		if (t->body)
			scope_start(stab);
	if (vistime == POSTVISIT)
		if (t->body)
			scope_end(stab);
}

static void vars_declaration(aststmt t, void *ignore, int vistime)
{
	if (vistime == POSTVISIT) /* MIDVISIT */
		if (t->u.declaration.decl)
			ast_declare_decllist_vars(t->u.declaration.spec, t->u.declaration.decl);
}

static void vars_funcdef(aststmt t, void *ignore, int vistime)
{
	if (vistime == PREVISIT)
	{
		scope_start(stab);
		if (!t->u.declaration.dlist)    /* new style */
			ast_declare_function_params(t->u.declaration.decl);/* declare manualy */
	}
	if (vistime == POSTVISIT)
		scope_end(stab);
}

static void vars_ident(astexpr t, void *f, int vistime)
{
	if (vistime == PREVISIT)
		(*((void (*)(astexpr)) f))(t);
}

static void vars_ompclvars(ompclause t, void *ignore, int vistime)
{
	if (vistime == PREVISIT)
		switch (t->type)
		{
			case OCPRIVATE:
			case OCFIRSTPRIVATE:
			case OCSHARED:
			case OCCOPYIN:
			case OCREDUCTION:
			case OCMAP:
			case OCAUTO:
				ast_declare_varlist_vars(t->u.varlist, t->type, t->subtype);
				break;
		};
}

static void vars_ompconall(ompcon t, void *ignore, int vistime)
{
	if (vistime == PREVISIT)
		scope_start(stab);
	if (vistime == POSTVISIT)
		scope_end(stab);
}

static void vars_oxconall(oxcon t, void *ignore, int vistime)
{
	if (vistime == PREVISIT)
		scope_start(stab);
	if (vistime == POSTVISIT)
		scope_end(stab);
}


void ast_stmt_vars(aststmt t, void (*idh)(astexpr))
{
	if (vartrops == NULL)
	{
		travopts_init_noop(vartrops = (travopts_t *) smalloc(sizeof(travopts_t)));
		vartrops->stmtc.compound_c = vars_compound;
		vartrops->stmtc.declaration_c = vars_declaration;
		vartrops->stmtc.funcdef_c = vars_funcdef;
		vartrops->exprc.ident_c = vars_ident;
		vartrops->ompclausec.ompclvars_c = vars_ompclvars;
		vartrops->ompclausec.prune_ompclexpr_c = 1;         /* Important! */
		vartrops->ompdcc.ompconall_c = vars_ompconall;
		vartrops->oxc.oxconall_c = vars_oxconall;
		vartrops->when = PREPOSTVISIT | MIDVISIT;
	}
	vartrops->starg = idh;
	ast_stmt_traverse(t, vartrops);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     ANALYZE VARIABLES USAGE                                   *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* A set used in analyze functions */
SET_TYPE_IMPLEMENT(vars)

/* Used by analyze_pointerize_vars and analyze_used_vars*/
static int thislevel;


static set(vars) gtpvars;  /* Used in analyze_find_gtp_vars */

static
void idh_find_gtp_vars(astexpr t)
{
	stentry e = symtab_get(stab, t->u.sym, IDNAME);
	if (e && isgtp(e))
		set_put_unique(gtpvars, t->u.sym);
}

/**
 * Finds all global thread private variables
 * Used in x_thrpriv.c
 *
 * @param t The statement to search in
 * return   A set containing the thread private variables
 */
set(vars) analyze_find_gtp_vars(aststmt t)
{
	set_init(vars, &gtpvars);
	ast_stmt_vars(t, idh_find_gtp_vars);
	return gtpvars;
}


static
void idh_sgl_makeptr(astexpr t)
{
	stentry e = symtab_get(stab, t->u.sym, IDNAME);

	if (e && issgl(e) && !isextern(e))
	{
		astexpr sub = Parenthesis(UnaryOperator(UOP_star, Identifier(t->u.sym)));
		*t = *sub;
	}
}

/**
 * Turns all global (non thread private) variables into pointers.
 * Used in ast_xform when in process mode
 */
void analyze_pointerize_sgl(aststmt t)
{
	ast_stmt_vars(t, idh_sgl_makeptr);
}


static set(vars) ptrvars;  /* Used in analyze_pointerize_vars */

static
void idh_makeptr(astexpr t)
{
	stentry e = symtab_get(stab, t->u.sym, IDNAME);
	if (e && e->scopelevel <= thislevel && set_get(ptrvars, t->u.sym))
		*t = *Parenthesis(UnaryOperator(UOP_star, Identifier(t->u.sym)));
}

/**
 * Turns all variables listed in the given set into pointers
 *
 * @param t    The statement containing variables that we want to pointerize
 * @param vars The variables to turn into pointers
 */
void analyze_pointerize_vars(aststmt t, set(vars) vars)
{
	ptrvars = vars;
	thislevel = stab->scopelevel;
	ast_stmt_vars(t, idh_makeptr);
}


static
void idh_makedeclptr(astexpr t)
{
	stentry e = symtab_get(stab, t->u.sym, IDNAME);

	if (!e)
	{
		/* Check if it is a function and is in a declare target */
		e = symtab_get(stab, t->u.sym, FUNCNAME);
		if (e && !e->isindevenv)
			exit_error(1, "(%s, line %d) openmp error:\n\t"
			           "Function \"%s\" needs to be declared in a \"declare "
			           "target\" directive.\n",
			           t->file->name, t->l, t->u.sym->name);

		return;
	}

	/* Ignore non global variables */
	if (e->scopelevel != 0)
		return;

	/* Make sure all global variables where declared */
	if (!e->isindevenv)
		exit_error(1, "(%s, line %d) openmp error:\n\t"
		           "Global variable \"%s\" needs to be declared in a \"declare "
		           "target\" directive.\n",
		           t->file->name, t->l, t->u.sym->name);

	//TODO test
	if (e->isindevenv == 2)
		exit_error(1, "analyze_pointerize_declared_vars %s:%d\n", __FILE__, __LINE__);

	*t = *Parenthesis(UnaryOperator(UOP_star, Identifier(t->u.sym)));
}

/**
 * Turns global into pointers making sure they appeared in declare target
 *
 * @param t    The statement containing variables that we want to pointerize
 */
void analyze_pointerize_declared_vars(aststmt t)
{
	ast_stmt_vars(t, idh_makedeclptr);
}


static set(vars) usedvars[DCT_SIZE]; /* Analysis-produced set */

vartype_t OC2DCT(stentry e)
{
	switch (e->ival)
	{
		case OCCOPYIN:
		case OCSHARED:
			return DCT_BYREF;
		case OCPRIVATE:
			return DCT_PRIVATE;
		case OCFIRSTPRIVATE:
			return DCT_BYVALUE;
		case OCREDUCTION:
			return DCT_REDUCTION;
		case OCMAP:
			if (e->isindevenv == 1) //Ignore declared variables
				return DCT_IGNORE;

			switch (e->vval)
			{
				case OC_alloc:
					return DCT_PRIVATE;
				case OC_to:
					return DCT_BYVALUE;
				case OC_from:
					return DCT_BYRESULT;
				case OC_tofrom:
					return DCT_BYVALRES;
				default:
					fprintf(stderr, "OC2DCT map type %s\n", clausesubs[e->vval]);
					exit(1);
			}
		case OCAUTO:
			return DCT_UNSPECIFIED;
		default:
			fprintf(stderr, "ast_vars.c:idh_usevar:Clause type of \"%s\" is %s[%d]\n",
			        e->key->name, clausenames[e->ival], e->ival);
			exit(1);
	}
}

static
void idh_usevar(astexpr t)
{
	stentry       e = symtab_get(stab, t->u.sym, IDNAME);
	setelem(vars) s;

	if (!e)
		return;

	/* Check if the variable was in a clause. +1 is for the scope that we open in
	 * the OpenMP directive
	 */
	if (e->scopelevel == thislevel + 1)
	{
		s = set_put_unique(usedvars[OC2DCT(e)], t->u.sym);
		/* We treat copyin variables just like thread privates (as byref) but we
		 * also mark them using ival to write the required code in x_parallel.
		 * We treat auto variables as unspecified and mark that they are auto.
		 */
		if (e->ival == OCCOPYIN || e->ival == OCAUTO)
			s->value = e->ival;
		else
			/* For reduction we also need the reduction type which is in vval*/
			if (e->ival == OCREDUCTION)
				s->value = e->vval;

	}
	/* Else if the variable was declared in a previous block we put it in the
	 * unspecified set. They are later moved in the other sets by outline using
	 * the implicitDefault() function.
	 */
	else
		if (e->scopelevel <= thislevel)
			set_put_unique(usedvars[DCT_UNSPECIFIED], t->u.sym)->value = e->ival;
}

/**
 * Returns a set with all variables (which were defined in a previous scope)
 * that appear inside a statement
 *
 * @param t The statement to search
 * @return  A set with all the variables found
 */
set(vars) *analyze_used_vars(aststmt t)
{
	int i;

	for (i = 0; i < DCT_SIZE; i++)
		set_init(vars, &usedvars[i]);
	thislevel = stab->scopelevel;
	ast_stmt_vars(t, idh_usevar);
	return usedvars;
}
