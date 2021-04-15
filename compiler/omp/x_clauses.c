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

/* x_clauses.c -- everything related to openmp clauses & their generated code */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ast_free.h"
#include "ast_vars.h"
#include "ast_xform.h"
#include "ast_copy.h"
#include "ast_types.h"
#include "x_clauses.h"
#include "x_arith.h"
#include "x_arrays.h"
#include "x_reduction.h"
#include "ompi.h"
#include "builder.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     EXTENDED LIST ITEMS STUFF                                 *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/**
 * Returns an expression with the base element of an xlitem
 * @param t  The xlitem
 * @return   The expression
 */
astexpr xc_xlitem_baseelement(ompxli t)
{
	astexpr   e = Identifier(t->id);
	omparrdim d;
	
	assert(t != NULL);
	if (t->xlitype == OXLI_ARRSEC)
		return ( arr_section_baseelement(t, NULL) );
	if (symtab_get(stab, t->id, IDNAME)->isarray)
		return ( Parenthesis(Deref(e)) );
	else     /* even in the case of a pointer.. */
		return ( Parenthesis(e) );
}


/**
 * The expression that gives the total number of elements in an xlitem
 * @param  arrsec  the xlitem
 * @return an expression with the total size of the array section (#elememts)
 */
astexpr xc_xlitem_length(ompxli arrsec)
{
	assert(arrsec != NULL);
	if (arrsec->xlitype == OXLI_ARRSEC)
		return ( arr_section_length(arrsec, ALLDIMS) );
	else     /* pointers?? */
	{
		stentry e = symtab_get(stab, arrsec->id, IDNAME);
		if (e->isarray)
			return ( arr_num_elems(e->decl, 0) );
		else
			return ( numConstant(1) );
	}
}


/**
 * Gives expressions for the basic quantities of an xlitem
 * @param xl the xlitem
 * @param itemaddr if non-NULL, it gets the starting address of the entire array
 * @param nbytes   if non-NULL, it gets the total size (in bytes)
 * @param addrlb   if non-NULL, it gets the address of the section's 1st element
 */
void xc_xlitem_copy_info(ompxli xl,
                         astexpr *itemaddr, astexpr *nbytes, astexpr *addrlb)
{
	itemaddr && ( *itemaddr = UOAddress(Identifier(xl->id)) );
	if (xl->xlitype == OXLI_IDENT)
	{
		nbytes && ( *nbytes = Sizeof(Identifier(xl->id)) );
		addrlb && ( *addrlb = ast_expr_copy(*itemaddr) );
	}
	else    /* Array section */
	{
		nbytes && ( *nbytes = arr_section_size(xl) );
		addrlb && ( *addrlb = arr_section_baseaddress(xl) );
	}
}


/* Check a list of extended-list-item OpenMP clauses and try to find if a 
 * variable is included in them; returns the xlitem.
 */
ompxli xc_xlitem_find_in_clause(ompclt_e type, ompclause t, symbol var)
{
	ompxli xl;

	if (!t) return (NULL);
	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			if ( (xl = xc_xlitem_find_in_clause(type, t->u.list.next, var)) != NULL )
				return (xl);
		t = t->u.list.elem;
		assert(t != NULL);
	}
	if (t->type == type)
		for (xl = t->u.xlist; xl; xl = xl->next)
			if (xl->id == var)
				return (xl);
	return (NULL);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     FIRSTPRIVATE RELATED STUFF                                *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* This produces a statement declaring a firstprivate variable (plus
 * another one which is need). The declaration contains an
 * initializer, only when the variable is scalar; otherwise it
 * contains a plain declaration and initialization should be done
 * through xc_ompdir_array_initializers()
 */
aststmt xc_firstprivate_declaration(symbol var)
{
	char    flvar[256];
	stentry e = symtab_get(stab, var, IDNAME);

	snprintf(flvar, 255, "_fip_%s", var->name);    /* a temp var _fip_<name> */
	return ( flr_privatize(var, Symbol(flvar), e->isarray,
	              e->isarray ? 
	                NULL :                         /* var = fip_var; */
	                IdentName(flvar)) );
}


/* Take a varlist and generate correct initialization statements
 * for lastprivate vars that are non-scalar.
 * It works for all OpenMP constructs except *parallel* which produces
 * its own declarations.
 */
static
aststmt array_initializations_from_varlist(astdecl d,
                                           ompclt_e type, ompdir ompd)
{
	aststmt list = NULL, st = NULL;

	if (d->type == DLIST && d->subtype == DECL_idlist)
	{
		list = array_initializations_from_varlist(d->u.next, type, ompd);
		d = d->decl;
	}
	assert(d->type == DIDENT);
	if (type == OCFIRSTPRIVATE)
	{
		char flvar[256];
		if ((ompd->type == DCFOR || ompd->type == DCSECTIONS || /* special case */
		     ompd->type == DCFOR_P) &&
		    symtab_get(stab, d->u.id, IDNAME)->isarray &&  /* first&last private */
		    xc_isvar_in_dataclause(d->u.id, ompd, OCLASTPRIVATE))       /* array */
			snprintf(flvar, 255, "_lap_%s", d->u.id->name);
		else
			snprintf(flvar, 255, "_fip_%s", d->u.id->name);
		if (symtab_get(stab, d->u.id, IDNAME)->isarray)
			st = xc_memcopy(Identifier(d->u.id),   /* *flvar */
			                Deref(IdentName(flvar)),
			                Sizeof(Identifier(d->u.id)));
	}
	if (st)
		list = ((list != NULL) ? BlockList(list, st) : st);
	return (list);
}


static
aststmt array_initializations_from_clauses(ompclause t, ompdir d)
{
	aststmt list = NULL, st = NULL;

	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			list = array_initializations_from_clauses(t->u.list.next, d);
		t = t->u.list.elem;
		assert(t != NULL);
	}

	if (t->type == OCFIRSTPRIVATE)
		(st = array_initializations_from_varlist(t->u.varlist, t->type, d))
		&& (list = ((list != NULL) ? BlockList(list, st) : st));
	return (list);
}


/* Memory copying statements for fip vars (firstprivate) */
aststmt xc_ompdir_fiparray_initializers(ompdir t)
{
	return (t->clauses ? array_initializations_from_clauses(t->clauses, t)
	        : NULL);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     LASTPRIVATE RELATED STUFF                                 *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* This produces a statement declaring a lastprivate variable (plus
 * another one which is needed), exactly as in firstprivate; the only
 * difference is that there is no initializer.
 */
aststmt xc_lastprivate_declaration(symbol var)
{
	char flvar[256];
	
	snprintf(flvar, 255, "_lap_%s", var->name);    /* a temp var _lap_<name> */
	return ( flr_privatize(var, Symbol(flvar), 1, NULL) );
}


/* This produces a statement declaring a lastprivate variable (plus
 * another one which is need), exactly as in firstprivate; the only
 * difference is that there is no initializer.
 */
aststmt xc_firstlastprivate_declaration(symbol var)
{
	char    flvar[256];

	/* Declare and intialize a temp var _flp_<name> */
	/* We call it _lap_<name> because xc_lastprivate_assignments()
	 * has no way of telling whether the var is also firstprivate
	 */ 
	snprintf(flvar, 255, "_lap_%s", var->name);
	return ( flr_privatize(var, Symbol(flvar), 1,
	              symtab_get(stab, var, IDNAME)->isarray ? 
	                NULL :        /* var = *flp_var; */
	                Deref(IdentName(flvar))) );
}


/* Take a lastprivate varlist and generate correct assignement statements */
static
aststmt lastprivate_assignments_from_varlist(astdecl d)
{
	char    flvar[256];
	aststmt list = NULL, st = NULL;

	if (d->type == DLIST && d->subtype == DECL_idlist)
	{
		list = lastprivate_assignments_from_varlist(d->u.next);
		d = d->decl;
	}
	assert(d->type == DIDENT);
	snprintf(flvar, 255, "_lap_%s", d->u.id->name);
	if (symtab_get(stab, d->u.id, IDNAME)->isarray)
		//st = xc_array_assigner(d->u.id,   /* *flvar */
		//                       Deref(IdentName(flvar)));
		st = xc_memcopy(IdentName(flvar), Identifier(d->u.id),
		                Sizeof(Identifier(d->u.id)));
	else  /* Scalar */
		st = AssignStmt( Deref(IdentName(flvar)), Identifier(d->u.id) );
	list = ((list != NULL) ? BlockList(list, st) : st);
	return (list);
}


static
aststmt lastprivate_assignments_from_clauses(ompclause t)
{
	aststmt list = NULL, st = NULL;

	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			list = lastprivate_assignments_from_clauses(t->u.list.next);
		t = t->u.list.elem;
		assert(t != NULL);
	}
	if (t->type == OCLASTPRIVATE)
		(st = lastprivate_assignments_from_varlist(t->u.varlist))
		&& (list = ((list != NULL) ? BlockList(list, st) : st));
	return (list);
}


/* Assignement / memory copying statements for lap vars (lastprivate) */
aststmt xc_ompdir_lastprivate_assignments(ompdir t)
{
	return (t->clauses ? lastprivate_assignments_from_clauses(t->clauses)
	                   : NULL);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     DECLARATIONS FROM THE DATA CLAUSE VARS                    *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Take a varlist and generate a correct declaration for each of the
 * vars. It takes care of private/firstprivate/lastprivate/etc types
 * of varlists.
 *
 * It works for all OpenMP constructs except *parallel* which produces
 * its own declarations.
 */
static
aststmt declarations_from_varlist(astdecl d, ompclt_e type, int operator)
{
	aststmt list = NULL, st = NULL;

	if (d->type == DLIST && d->subtype == DECL_idlist)
	{
		list = declarations_from_varlist(d->u.next, type, operator);
		d = d->decl;
	}
	assert(d->type == DIDENT);
	switch (type)
	{
		case OCFIRSTPRIVATE:
			st = xc_firstprivate_declaration(d->u.id);
			break;
		case OCLASTPRIVATE:
			st = xc_lastprivate_declaration(d->u.id);
			break;
		case OCPRIVATE:
			st = xform_clone_declaration(d->u.id, NULL, false, NULL);
			break;
		case OCCOPYIN:        /* This is handled entirely in x_parallel.c */
		case OCSHARED:        /* These do not produce any declarations */
		case OCCOPYPRIVATE:   /* This is handled entirely in x_single.c */
			break;
	}
	if (st) list = ((list != NULL) ? BlockList(list, st) : st);
	return (list);
}


static
aststmt declarations_from_xlist(ompxli xl, ompclt_e type, int operator)
{
	aststmt list = NULL, st;

	for (; xl; xl = xl->next)
		if (type == OCREDUCTION)
		{
			st = red_generate_declaration(xl->id, operator, xl);
			list = ((list != NULL) ? BlockList(list, st) : st);
		};
	return (list);
}


/* It is assumed that a validity check for clauses and variables
 * appearing in them has already taken place.
 * ompdir is only needed for error messages & validity checks
 */
static
aststmt declarations_from_clauses(enum dircontype dirtype, ompclause t)
{
	aststmt list = NULL, st = NULL;

	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			list = declarations_from_clauses(dirtype, t->u.list.next);
		t = t->u.list.elem;
		assert(t != NULL);
	}
	switch (t->type)
	{
		case OCSHARED:          /* Only data clauses matter */
		case OCCOPYIN:
		case OCPRIVATE:
		case OCFIRSTPRIVATE:
		case OCCOPYPRIVATE:
		case OCLASTPRIVATE:
			(st = declarations_from_varlist(t->u.varlist, t->type, t->subtype))
			&& (list = ((list != NULL) ? BlockList(list, st) : st));
			break;
		case OCREDUCTION:
			(st = declarations_from_xlist(t->u.xlist, t->type, t->subtype))
			&& (list = ((list != NULL) ? BlockList(list, st) : st));
			break;
	}
	return (list);
}


/* This is actually called only from x_single.c (!!) */
aststmt xc_ompdir_declarations(ompdir t)
{
	return (t->clauses ? declarations_from_clauses(t->type, t->clauses) : NULL);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     DATA CLAUSE VARIABLES                                     *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* The following 4 functions are used to gather all variables appearing
 * in a directive's data clauses. They also detect duplicates.
 * They use a local symbol table to do it.
 *
 * For each var in the table, the ival field stores the clause type
 * and the "value" field encodes the operator, in case of reduction vars.
 */
static symtab    dc_vars = NULL;    /* Store the list here */
static ompclause dc_vars_clause;    /* Only needed for error messages */


static
void checkNstore_varlist_vars(astdecl d, int clausetype, int opid)
{
	stentry e;

	if (d->type == DLIST && d->subtype == DECL_idlist)
	{
		checkNstore_varlist_vars(d->u.next, clausetype, opid);
		d = d->decl;
	}
	if (d->type != DIDENT)
		exit_error(1, "[checkNstore_varlist_vars]: !!BUG!! not a DIDENT ?!\n");
	if ((e = symtab_get(dc_vars, d->u.id, IDNAME)) != NULL)
	{
		if ((clausetype == OCFIRSTPRIVATE && e->ival == OCLASTPRIVATE) ||
		    (clausetype == OCLASTPRIVATE && e->ival == OCFIRSTPRIVATE))
		{
			e->ival = OCFIRSTLASTPRIVATE;       /* Change it -- special case! */
			return;                             /* Don't put it in again */
		}
		else
			exit_error(1, "(%s, line %d) openmp error:\n\t"
			           "variable `%s' appears more than once in "
			           "the directive's clause(s)\n",
			           dc_vars_clause->file->name, dc_vars_clause->l,
			           d->u.id->name);
	}
	switch (clausetype)
	{
		case OCCOPYIN:         /* Copyin is only for threadprivate vars */
			if (!symtab_get(stab, d->u.id, IDNAME)->isthrpriv)
				exit_error(1, "(%s, line %d) openmp error:\n\t"
				           "copyin clause variable '%s' is not threadprivate.\n",
				           dc_vars_clause->file->name, dc_vars_clause->l, d->u.id->name);
			break;
		case OCCOPYPRIVATE:   /* Nothing here */
			break;
		default:              /* All others */
			if (symtab_get(stab, d->u.id, IDNAME)->isthrpriv)
				exit_error(1, "(%s, line %d) openmp error:\n\t"
				           "threadprivate variable '%s' cannot appear in a %s clause.\n",
				           dc_vars_clause->file->name, dc_vars_clause->l,
				           d->u.id->name, clausenames[clausetype]);
			break;
	}
	/* Put the opid in the "value" field */
	e = symtab_put(dc_vars, d->u.id, IDNAME);
	e->vval = opid;
	e->ival = clausetype;
	e->pval = NULL;        /* No xlitem */
}


static
void checkNstore_xlist_vars(ompxli xl, int clausetype, int opid)
{
	stentry e;

	for (; xl; xl = xl->next)
	{
		if ((e = symtab_get(dc_vars, xl->id, IDNAME)) != NULL)
			exit_error(1, "(%s, line %d) openmp error:\n\t"
								"variable `%s' appears more than once in "
								"the directive's clause(s)\n",
								dc_vars_clause->file->name, dc_vars_clause->l,
								xl->id->name);
		if (symtab_get(stab, xl->id, IDNAME)->isthrpriv)
			exit_error(1, "(%s, line %d) openmp error:\n\t"
								"threadprivate variable '%s' cannot appear in a %s clause.\n",
								dc_vars_clause->file->name, dc_vars_clause->l,
								xl->id->name, clausenames[clausetype]);
		/* Put the opid in the "value" field */
		e = symtab_put(dc_vars, xl->id, IDNAME);
		e->vval = opid;
		e->ival = clausetype;
		e->pval = xl;          /* Remeber the xlitem */
	}
}


static
void checkNstore_dcclause_vars(ompclause t)
{
	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			checkNstore_dcclause_vars(t->u.list.next);
		assert((t = t->u.list.elem) != NULL);
	}
	dc_vars_clause = t;
	switch (t->type)
	{
		case OCPRIVATE:
		case OCFIRSTPRIVATE:
		case OCLASTPRIVATE:
		case OCCOPYPRIVATE:
		case OCCOPYIN:
		case OCSHARED:
		case OCAUTO:
			if (t->u.varlist)   /* t->subtype is the opid in case of reduction */
				checkNstore_varlist_vars(t->u.varlist, t->type, t->subtype);
			break;
		case OCREDUCTION:
		case OCMAP:
			if (t->u.xlist)
				checkNstore_xlist_vars(t->u.xlist, t->type, t->subtype);
			break;
	}
}


/* Checks for duplicates AND keeps the list of vars; returns the table */
symtab xc_validate_store_dataclause_vars(ompdir d)
{
	if (dc_vars == NULL)
		dc_vars = Symtab();
	else
		symtab_drain(dc_vars);
	if (d->clauses)
		checkNstore_dcclause_vars(d->clauses);
	return (dc_vars);
}


/* Only checks for duplicates */
void xc_validate_only_dataclause_vars(ompdir d)
{
	xc_validate_store_dataclause_vars(d);
	symtab_drain(dc_vars);                   /* Not needed any more */
}


/*
 * The following 3 functions are used to search whether a particular
 * variable appears in any (or a specific) data clause.
 */


static
int findvar_varlist(symbol var, astdecl d)
{
	if (d->type == DLIST)
	{
		if (findvar_varlist(var, d->u.next))
			return (1);
		d = d->decl;
	}
	if (d->type != DIDENT)
		exit_error(1, "[findvar_varlist]: !!BUG!! not a DIDENT ?!\n");
	return (d->u.id == var);
}


static
int findvar_xlist(symbol var, ompxli xl)
{
	for (; xl; xl = xl->next)
		if (xl->id == var)
			return (1);
	return (0);
}


static
int findvar_dataclause(symbol var, ompclause t, ompclt_e c)
{
	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			if (findvar_dataclause(var, t->u.list.next, c))
				return (1);
		assert((t = t->u.list.elem) != NULL);
	}
	if (t->type == c && t->u.varlist)
		return ( (c != OCREDUCTION) ? 
		             findvar_varlist(var, t->u.varlist) :
		             findvar_xlist(var, t->u.xlist) );
	return (0);
}


static
ompclt_e findvar_any_dataclause(symbol var, ompclause t)
{
	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			if (findvar_any_dataclause(var, t->u.list.next))
				return (OCPRIVATE);  /* anything would do */
		assert((t = t->u.list.elem) != NULL);
	}
	switch (t->type)
	{
		case OCPRIVATE:
		case OCFIRSTPRIVATE:
		case OCLASTPRIVATE:
		case OCCOPYPRIVATE:
		case OCCOPYIN:
		case OCSHARED:
			if (t->u.varlist && findvar_varlist(var, t->u.varlist))
				return (t->type);
			break;
		case OCREDUCTION:
			if (t->u.varlist && findvar_xlist(var, t->u.xlist))
				return (t->type);
			break;
	}
	return (OCNOCLAUSE);
}


/* This one searches to see whether var appears in a type c
 * data clauses of d; returns 1 if so, 0 if not.
 */
int xc_isvar_in_dataclause(symbol var, ompdir d, ompclt_e c)
{
	return (findvar_dataclause(var, d->clauses, c) != OCNOCLAUSE);
}


/* Find whether var appears in any of the data clauses of d.
 * If found, it returns the clause type, otherwise 0.
 */
ompclt_e xc_dataclause_of_var(symbol var, ompdir d)
{
	return (findvar_any_dataclause(var, d->clauses));
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     DECLARATIONS FROM A SET OF VARS                           *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* We have 2 ways of declaring the variables in the data clauses.
 *   (1) by scanning all the data clauses and declaring from each clause
 *   (2) by collecting all data clause variables in a set, along with
 *       the dataclause type of each one and declaring them from there.
 * Both have some advantages. The second one does not need recursion,
 * plus it makes it trivial to discover vars that are both firstprivate
 * and lastprivate.
 */


/* Produces a list of declarations for all variables in the dc_vars table
 */
aststmt xc_stored_vars_declarations(bool *has_last, bool *has_both,
                                    bool *has_red)
{
	stentry e;
	aststmt st = NULL, list = NULL;

	*has_last = *has_both = *has_red = false;
	for (e = dc_vars->top; e; e = e->stacknext)
	{
		switch (e->ival)
		{
			case OCFIRSTPRIVATE:
				st = xc_firstprivate_declaration(e->key);
				break;
			case OCREDUCTION:
				*has_red = true;
				st = red_generate_declaration(e->key, e->vval, e->pval);
				break;
			case OCLASTPRIVATE:
				*has_last = true;
				st = xc_lastprivate_declaration(e->key);
				break;
			case OCFIRSTLASTPRIVATE:
				*has_both = true;
				*has_last = true;
				st = xc_firstlastprivate_declaration(e->key);
				break;
			case OCPRIVATE:
				st = xform_clone_declaration(e->key, NULL, false, NULL);
				break;
			case OCCOPYIN:        /* Taken care in x_parallel.c */
			case OCSHARED:        /* These do not produce any declarations */
			case OCCOPYPRIVATE:   /* This is handled entirely in x_single.c */
				break;
		}

		if (st)
			list = ((list == NULL) ? st : BlockList(list, st));
	}
	return (list);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     UTILITY FUNCTIONS                                         *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/**
 * Creates a local variable with the same name (with an optional initializer), 
 * plus a copy of/pointer to the original one.  I.e. the following:
 *   <spec> varbak = var, var [ = initer ]; or
 *   <spec> *varbak = &var, var [ = initer ]; or
 *
 * @param var       The original variable whose declaration we want to reproduce
 * @param varbak    The copy of/pointer to the original variable
 * @param isptr     A flag for making varbak a pointer or not
 * @param initer    An optional initializer for the new local variable
 * @return          A statement with the declaration
 */
aststmt flr_privatize(symbol var, symbol varbak, int isptr, astexpr initer)
{
	astdecl bakdecl, id, localvar;
	stentry e = symtab_get(stab, var, IDNAME);
	
	bakdecl = ast_decl_copy(e->decl);      /* Make the varbak */
	id = IdentifierDecl(varbak);
	*(decl_getidentifier(bakdecl)) = *id;
	free(id);
	if (isptr)
		bakdecl = InitDecl( decl_topointer(bakdecl),UOAddress(Identifier(var)) );
	else
		bakdecl = InitDecl( bakdecl, Identifier(var) );

	localvar = xform_clone_declonly(e);    /* Make the local var declarator */ 
	if (initer)
		localvar = InitDecl(localvar, initer);
	
	/* <spec> *varbak = &var, var = <initializer>; */
	return Declaration(ast_spec_copy_nosc(e->spec), DeclList(bakdecl, localvar));
}


/* Applies rules to ensure validity & OpenMP compliance of all clauses.
 * It won't check the variables inside the clauses.
 */
void xc_validate_clauses(enum dircontype dirtype, ompclause cl)
{
	ompclause t;
	int hasnw = 0, hasord = 0, hasif = 0, hasnth = 0, hasdef = 0, hassch = 0,
	    hasunt = 0, hascollapse = 0, hasfinal = 0, hasmerge = 0, hasprocbind = 0,
	    hasdevice = 0, hasauto = 0, hasdefauto = 0;

	while (cl)
	{
		t = (cl->type == OCLIST) ? cl->u.list.elem : cl;
		assert(t != NULL);
		
		switch (t->type)
		{
			/* Data sharing clauses */

			//TODO the checks here are obsolete except the one for auto and for
			//default(auto)
			case OCREDUCTION:
				if (dirtype != DCPARALLEL && dirtype != DCFOR && dirtype != DCSECTIONS &&
						dirtype != DCFOR_P && dirtype != DCPARFOR && dirtype != DCPARSECTIONS &&
					  dirtype != DCTEAMS)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"reduction clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCSHARED:
				if (dirtype != DCPARALLEL && dirtype != DCPARFOR &&
						dirtype != DCPARSECTIONS && dirtype != DCTASK && dirtype != DCTEAMS)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"shared clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCCOPYIN:
				if (dirtype != DCPARALLEL && dirtype != DCPARFOR &&
						dirtype != DCPARSECTIONS)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"copyin clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCPRIVATE:
			case OCFIRSTPRIVATE:
				if (dirtype != DCPARALLEL && dirtype != DCFOR && dirtype != DCSECTIONS &&
						dirtype != DCFOR_P && dirtype != DCPARFOR &&
						dirtype != DCPARSECTIONS && dirtype != DCSINGLE && 
						dirtype != DCTASK && dirtype != DCTARGET && dirtype != DCTEAMS)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"(first)private clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCCOPYPRIVATE:
				if (dirtype != DCSINGLE)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"copyprivate clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCLASTPRIVATE:
				if (dirtype != DCFOR && dirtype != DCSECTIONS &&
						dirtype != DCFOR_P && dirtype != DCPARFOR && dirtype != DCPARSECTIONS)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"lastprivate clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;

			/* Data mapping clauses */

			case OCMAP:
				if (dirtype != DCTARGENTERDATA && dirtype != DCTARGEXITDATA &&
						dirtype != DCTARGET && dirtype != DCTARGETDATA)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"map clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCFROM:
				if (dirtype != DCTARGETUPD)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"from clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCTO:
				if (dirtype != DCTARGETUPD && dirtype != DCDECLTARGET)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"to clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCLINK:
				if (dirtype != DCDECLTARGET)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"link clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCISDEVPTR:
				if (dirtype != DCTARGET)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"is_device_ptr clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCUSEDEVPTR:
				if (dirtype != DCTARGETDATA)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"use_device_ptr clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCDEFAULTMAP:
				if (dirtype != DCTARGET)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"defaultmap clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;

			/* Non-data clauses */

			case OCIF:
				if (hasif)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"multiple if clauses are not allowed in a directive\n",
										t->file->name, t->l);
				hasif = 1;
				if (dirtype != DCPARALLEL && dirtype != DCPARFOR &&
						dirtype != DCPARSECTIONS && dirtype != DCTASK &&
						dirtype != DCTARGET && dirtype != DCTARGETDATA &&
						dirtype != DCTARGENTERDATA && dirtype != DCTARGEXITDATA &&
						dirtype != DCTARGETUPD && dirtype != DCCANCEL)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"if clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				if (t->modifier != OCM_none)
				{
					if ((dirtype == DCPARALLEL && t->modifier != OCM_parallel) ||
							(dirtype == DCTARGET && t->modifier != OCM_target) ||
							(dirtype == DCTARGETDATA && t->modifier != OCM_targetdata) ||
							(dirtype == DCTARGENTERDATA && t->modifier != OCM_targetenterdata) ||
							(dirtype == DCTARGEXITDATA && t->modifier != OCM_targetexitdata) ||
							(dirtype == DCTARGETUPD && t->modifier != OCM_targetupdate) ||
							(dirtype == DCTASK && t->modifier != OCM_task) ||
							(dirtype == DCCANCEL && t->modifier != OCM_cancel) ||
							(dirtype == DCPARFOR && t->modifier != OCM_parallel) ||
							(dirtype == DCPARSECTIONS && t->modifier != OCM_parallel))
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"if clause modifier (%s) refers to illegal construct (%s)\n",
										t->file->name, t->l, clausemods[t->modifier], 
										ompdirnames[dirtype]);
				}
				break;
			case OCFINAL:     /* OpenMP 3.1 */
				if (hasfinal)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"multiple final clauses are not allowed in a directive\n",
										t->file->name, t->l);
				hasfinal = 1;
				if (dirtype != DCTASK)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"final clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCMERGEABLE: /* OpenMP 3.1 */
				if (hasmerge)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"multiple mergeable clauses are not allowed in a directive\n",
										t->file->name, t->l);
				hasmerge = 1;
				if (dirtype != DCTASK)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"mergeable clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCNUMTHREADS:
				if (hasnth)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"multiple num_threads clauses are not allowed in a directive\n",
										t->file->name, t->l);
				hasnth = 1;
				if (dirtype != DCPARALLEL && dirtype != DCPARFOR &&
						dirtype != DCPARSECTIONS)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"num_threads clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCDEVICE:
				if (hasdevice)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"multiple device clauses are not allowed in a directive\n",
										t->file->name, t->l);
				hasnth = 1;
				if (dirtype != DCTARGET && dirtype != DCTARGETDATA &&
						dirtype != DCTARGENTERDATA && dirtype != DCTARGEXITDATA &&
						dirtype != DCTARGETUPD)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"device clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCDEFAULT:
				if (hasdef)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"multiple default clauses are not allowed in a directive\n",
										t->file->name, t->l);
				hasdef = 1;
				if (dirtype != DCPARALLEL && dirtype != DCPARFOR &&
						dirtype != DCPARSECTIONS && dirtype != DCTASK && dirtype != DCTEAMS)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"default clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				/* This one is needed for Aggelo's auto scoping */
				if (t->subtype == OC_auto)
				{
					hasdefauto = 1;
					if (!enableAutoscope)
						exit_error(1, "(%s, line %d) error:\n\t"
											"autoscoping has not been enabled (use --autoscope)\n",
											t->file->name, t->l);
					if (dirtype != DCPARALLEL && dirtype != DCPARFOR && 
							dirtype != DCPARSECTIONS)
						exit_error(1, "(%s, line %d) openmp error:\n\t"
											"default(auto) clause isn't allowed in `%s' directives\n",
											t->file->name, t->l, ompdirnames[dirtype]);
				}
				else
					if (hasauto)
						exit_error(1, "(%s, line %d) openmp error:\n\t"
											"auto and default clauses collide\n",
											t->file->name, t->l);
				break;
			case OCORDERED:
			case OCORDEREDNUM:
				if (hasord)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"multiple order clauses are not allowed in a directive\n",
										t->file->name, t->l);
				hasord = 1;
				if (dirtype != DCFOR && dirtype != DCFOR_P && dirtype != DCPARFOR)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"ordered clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCSCHEDULE:
				if (hassch)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"multiple schedule clauses are not allowed in a directive\n",
										t->file->name, t->l);
				hassch = 1;
				if (dirtype != DCFOR && dirtype != DCFOR_P && dirtype != DCPARFOR)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"schedule clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCNOWAIT:
				if (hasnw)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"multiple nowait clauses are not allowed in a directive\n",
										t->file->name, t->l);
				hasnw = 1;
				if (dirtype != DCFOR && dirtype != DCSECTIONS && dirtype != DCSINGLE && 
						dirtype != DCTARGET && dirtype != DCTARGENTERDATA && 
						dirtype != DCTARGEXITDATA && dirtype != DCTARGETUPD)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"nowait clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCUNTIED:
				if (hasunt)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"multiple untied clauses are not allowed in a directive\n",
										t->file->name, t->l);
				hasunt = 1;
				if (dirtype != DCTASK)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"untied clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCCOLLAPSE:
				if (hascollapse)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"multiple collapse clauses are not allowed in a directive\n",
										t->file->name, t->l);
				hascollapse = 1;
				if (dirtype != DCFOR && dirtype != DCFOR_P && dirtype != DCPARFOR)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"collapse clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCPROCBIND:
				if (hasprocbind)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"multiple proc_bind clauses are not allowed in a directive\n",
										t->file->name, t->l);
				hasprocbind = 1;
				if (dirtype != DCPARALLEL && dirtype != DCPARFOR &&
						dirtype != DCPARSECTIONS)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"proc_bind clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCAUTO: /* This one is needed for Aggelo's auto scoping */
				if (!enableAutoscope)
					exit_error(1, "(%s, line %d) error:\n\t"
										"autoscoping has not been enabled (use --autoscope)\n",
										t->file->name, t->l);
				hasauto = 1;
				if (dirtype != DCPARALLEL && dirtype != DCPARFOR &&
						dirtype != DCPARSECTIONS)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"auto clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				if (hasdef && !hasdefauto)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
											"auto and default clauses collide\n",
											t->file->name, t->l);
				break;
			case OCPARALLEL:
			case OCSECTIONS:
			case OCFOR:
			case OCTASKGROUP:
				if (dirtype != DCCANCEL && dirtype != DCCANCELLATIONPOINT)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"%s clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, clausenames[t->type],
										ompdirnames[dirtype]);
				break;
			case OCDEPEND:  /* OpenMP 4.0 / 4.5 */
				if (dirtype != DCTASK &&  dirtype != DCORDERED && dirtype != DCTARGET &&
						dirtype != DCTARGENTERDATA && dirtype != DCTARGEXITDATA &&
						dirtype != DCTARGETUPD)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"depend clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCNUMTEAMS:  /* OpenMP 4.0 */
				if (dirtype != DCTEAMS && dirtype != DCTARGETTEAMS)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"num_teams clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCTHREADLIMIT:  /* OpenMP 4.0 */
				if (dirtype != DCTEAMS && dirtype != DCTARGETTEAMS)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"thread_limit clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCHINT:  /* OpenMP 4.5 */
				if (dirtype != DCCRITICAL)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"hint clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCPRIORITY:  /* OpenMP 4.5 */
				if (dirtype != DCTASK)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"priority clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;
			case OCTHREADS:  /* OpenMP 4.5 */
				if (dirtype != DCORDERED)
					exit_error(1, "(%s, line %d) openmp error:\n\t"
										"threads clauses aren't allowed in `%s' directives\n",
										t->file->name, t->l, ompdirnames[dirtype]);
				break;

			default:
				exit_error(1, "(%s, line %d) openmp error:\n\t"
									"unknown clause type (%d) in `%s' directive\n",
									t->file->name, t->l, t->type, ompdirnames[dirtype]);
				break;
		}
		
		cl = (cl->type == OCLIST)? cl->u.list.next : NULL;
	}
	

}


/**
 * Get clause(s) of a given type from a clauselist.
 *
 * @param t    The OpenMP clauses we are searching in
 * @param type The type of clause we want to search for
 * @param what 0 if searching any, 1 if searching unique, 2 if searching all
 * @return     The clause(s) of the given type, or NULL if none exists
 */
ompclause xc_clauselist_get_clause(ompclause t, ompclt_e type, char what)
{
	ompclause c = NULL;

	if (t == NULL) return (NULL);
	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			if ((c = xc_clauselist_get_clause(t->u.list.next, type, what)) != NULL
			    && what == 0)
				return (c);        /* Found one, stop here */
		assert((t = t->u.list.elem) != NULL);
	}
	if (t->type == type)
	{
		if (what == 2)
			t = ast_ompclause_copy(t);   /* get a copy */
		if (!c)
			c = t;
		else
		{
			if (what == 1)
				exit_error(1, "(%s, line %d) openmp error:\n\t"
				           "multiple %s() clauses in directive.\n",
				           t->file->name, t->l, clausenames[type]);
			c = OmpClauseList(c, t);
		}
	}
	return (c);
}


/**
 * Find a clause of a given type in a construct.
 * It only returns the leftmost clause found.
 *
 * @param t    The OpenMP construct we are searching in
 * @param type The type of clause we want to search for
 * @return     The leftmost clause of the given type (or NULL)
 */
ompclause xc_ompcon_get_clause(ompcon t, ompclt_e type)
{
	return (xc_clauselist_get_clause(t->directive->clauses, type, 0));
}


/**
 * Find a clause of a given type in a construct.
 * If the clause is not unique it generates an error.
 *
 * @param t    The OpenMP construct we are searching in
 * @param type The type of clause we want to search for
 * @return     The leftmost clause of the given type (or NULL)
 */
ompclause xc_ompcon_get_unique_clause(ompcon t, ompclt_e type)
{
	return (xc_clauselist_get_clause(t->directive->clauses, type, 1));
}


/**
 * Get all clauses of a given type in a construct.
 * The returned list is a copy (so it can be freed).
 *
 * @param t    The OpenMP construct we are searching in
 * @param type The type of clause we want to search for
 * @return     A list of all clauses of the given type (or NULL)
 */
ompclause xc_ompcon_get_every_clause(ompcon t, ompclt_e type)
{
	return (xc_clauselist_get_clause(t->directive->clauses, type, 2));
}


static set(vars) split_comb_defnone;
static void 
split_combined_clauses(ompclause all, ompclause *parc, ompclause *wshc, int def)
{
	if (all == NULL) { *parc = *wshc = NULL; return; }
	if (all->type == OCLIST)
	{
		split_combined_clauses(all->u.list.next, parc, wshc, def);
		all = all->u.list.elem;
		assert(all != NULL);
	}
	all = ast_ompclause_copy(all);
	switch (all->type)
	{
		case OCIF:
		case OCSHARED:
		case OCCOPYIN:
		case OCREDUCTION:
		case OCNUMTHREADS:
		case OCPRIVATE:
		case OCDEFAULT:
		case OCAUTO:
			*parc = (*parc) ? OmpClauseList(*parc, all) : all;
			break;
		case OCFIRSTPRIVATE:
		case OCLASTPRIVATE:
			if (def)            /* Remember them */
				ast_varlist2set(all->u.varlist, split_comb_defnone);
		default:
			*wshc = (*wshc) ? OmpClauseList(*wshc, all) : all;
			break;
	}
}


/* Take the clauses of a combined parallel-workshare statement and split them; 
 * some are given to parallel and some to the workshare. Parallel actually 
 * takes all the clauses it can take. A problem exists if there is a 
 * default(none) and a variable listed in a firstprivate and/or lastprivate 
 * clause; this is because these clauses are given to the worksharing
 * construct, resulting in implicit sharing resolution on the parallel 
 * construct (which is however disallowed due to default(none)).
 * Solution: we explicitely tag them as shared.
 */
void xc_split_combined_clauses(ompclause all, ompclause *parc, ompclause *wshc)
{
	ompclause ocl = xc_clauselist_get_clause(all, OCDEFAULT, 0);
	int       hasdef = (ocl != NULL && ocl->subtype == OC_defnone);
	
	if (hasdef)
		set_init(vars, &split_comb_defnone);
	
	split_combined_clauses(all, parc, wshc, hasdef);
	
	/* Create a SHARED clause to give to the #parallel construct */
	if (hasdef && !set_isempty(split_comb_defnone))
	{
		ocl = VarlistClause(OCSHARED, ast_set2varlist(split_comb_defnone));
		*parc = (*parc) ? OmpClauseList(*parc, ocl) : ocl;
	}
}


/* Take the clauses of a V4.5 target construct and split them in two:
 * the ones that refer to the actual (V4.0) target and the ones that
 * refer to the implied target task. The nowait one gets nowhere.
 */
void xc_split_target_clauses(ompclause all, ompclause *targc, ompclause *taskc)
{
	if (all == NULL) { *targc = *taskc = NULL; return; }
	if (all->type == OCLIST)
	{
		xc_split_target_clauses(all->u.list.next, targc, taskc);
		all = all->u.list.elem;
		assert(all != NULL);
	}
	all = ast_ompclause_copy(all);
	switch (all->type)
	{
		case OCDEPEND:
			*taskc = (*taskc) ? OmpClauseList(*taskc, all) : all;
			break;
		default:
			*targc = (*targc) ? OmpClauseList(*targc, all) : all;
			break;
	}
}


/* Next 4 functions return a set containing all the variables in clauses of
 * a given type.
 */
static
void varlist_get_vars(astdecl d, set(vars) s, 
                      ompclt_e type, ompclsubt_e subt, ompclmod_e mod)
{
	setelem(vars) e;

	if (d->type == DLIST && d->subtype == DECL_idlist)
	{
		varlist_get_vars(d->u.next, s, type, subt, mod);
		d = d->decl;
	}
	assert(d->type == DIDENT);
	e = set_put_unique(s, d->u.id);
	e->value.clause = type;
	e->value.clsubt = subt;
	e->value.clmod  = mod;
}


static
void xlist_get_vars(ompclause t, set(vars) s)
{
	setelem(vars) e;
	ompxli        xl;

	for (xl = t->u.xlist; xl; xl = xl->next)
	{
		e = set_put_unique(s, xl->id);      // TODO: array sections 
		e->value.ptr    = xl;
		e->value.clause = t->type;
		e->value.clsubt = t->subtype;
		e->value.clmod  = t->modifier;
	}
}


static
void clauselist_get_vars(ompclause t, 
         ompclt_e type, ompclsubt_e subt, set(vars) s)
{
	if (t == NULL) return;

	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			clauselist_get_vars(t->u.list.next, type, subt, s);
		t = t->u.list.elem;
		assert(t != NULL);
	}
	if (t->type == type && (subt == OC_DontCare || t->subtype == subt))
	{
		if (type == OCDEPEND || type == OCMAP || type == OCREDUCTION)
			xlist_get_vars(t, s);
		else
			varlist_get_vars(t->u.varlist, s, type, t->subtype, t->modifier);
	}
}


/**
 * Find varlist clauses of a given type/subtype and return a set with the 
 * variables that appear in those clauses.
 *
 * @param t    The OpenMP construct we are searching in
 * @param type The type of clause we want to search for
 * @param subt The clause subtype (pass OC_DontCare to ignore)
 * @param s    An initiliazed set (must not be NULL) that after execution will
 * contain all the variables that appear in a "type" clause
 */
void xc_ompcon_get_vars(ompcon t, 
         ompclt_e type, ompclsubt_e subt, set(vars) s)
{
	clauselist_get_vars(t->directive->clauses, type, subt, s);
}


/* Next 3 functions return a set containing all the extended list items
 * in clauses of a given type.
 */
static
void xlist_get_items(ompxli xl, set(xlitems) s, 
                     ompclt_e type, ompclsubt_e subt, ompclmod_e mod)
{
	setelem(xlitems) e;
	
	for (; xl; xl = xl->next)
	{
		e = set_put(s, xl->id);
		e->value.clause = type;
		e->value.clsubt = subt;
		e->value.clmod  = mod;
		e->value.xl     = xl;
	}
}


static
void clauselist_get_xlitems(ompclause t, 
         ompclt_e type, ompclsubt_e subt, set(xlitems) s)
{
	if (t == NULL) return;

	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			clauselist_get_xlitems(t->u.list.next, type, subt, s);
		t = t->u.list.elem;
		assert(t != NULL);
	}
	if (t->type == type && (subt == OC_DontCare || t->subtype == subt))
		xlist_get_items(t->u.xlist, s, type, t->subtype, t->modifier);
}

/**
 * Find clauses of a given type/subtype and return a set with the extended
 * list items that appear in those clauses. This only refers to
 * REDUCTION/MAP/DEPEND/TO/FROM clauses (the only with extended list items).
 *
 * @param t    The OpenMP construct we are searching in
 * @param type The type of clause we want to search for
 * @param subt The clause subtype (pass OC_DontCare to ignore)
 * @param s    An initiliazed set (must not be NULL) that after execution will
 * contain all the extended list items that appear in a "type" clause
 */
void xc_ompcon_get_xlitems(ompcon t, 
         ompclt_e type, ompclsubt_e subt, set(xlitems) s)
{
	clauselist_get_xlitems(t->directive->clauses, type, subt, s);
}


/**
 * Produces a memory copy statement
 *
 *    memcpy( (void *) to, (void *) from, size );
 *
 * @param to   The identifier we are copyin to
 * @param from The identifier we are copyin from
 * @param size The size we will copy (usually either SizeOf(to) or SizeOf(from)
 *             depending on which one is not a pointer
 * @return     The new statement
 */
inline aststmt xc_memcopy(astexpr to, astexpr from, astexpr size)
{
	needMemcpy = true;
	return (
	         FuncCallStmt(
	           IdentName("memcpy"),
	           Comma3(CastVoidStar(to), CastVoidStar(from), size)
	         )
	       );
}


/**
 * Produces memory fill code (either a memset() call if filling with 0s 
 * or an explicit loop that sets each element to the given value).
 *
 * @param addr     The start of the area we are filling
 * @param nelems   Number of elements
 * @param nbytes   Size in bytes
 * @param what     The filler value
 * @return         The new statement
 */
aststmt xc_memfill(astexpr addr, astexpr nelems, astexpr nbytes, astexpr what)
{
	symbol loopidx = Symbol("_tmp_i_");
	
	if (what->type == CONSTVAL && strcmp(what->u.str, "0") == 0)
	{
		needMemset = true;                     /* Fill with zeroes */
		return FuncCallStmt(
		         IdentName("memset"),
		         Comma3(CastVoidStar(addr), what, nbytes));
	}
	
	/* { int i; for (i = (<nelems>)-1; i >= 0; i--)  (<addr>)[i] = <what>; } */
	return
		Compound(  
			BlockList(
				Declaration(
					Declspec(SPEC_int), Declarator(NULL, IdentifierDecl(loopidx))), 
				For(
					AssignStmt(
						Identifier(loopidx),
						BinaryOperator( BOP_sub, Parenthesis(nelems), numConstant(1) )
					), 
					BinaryOperator(BOP_geq, Identifier(loopidx), numConstant(0)), 
					PostOperator(Identifier(loopidx), UOP_dec),    
					Expression(
						Assignment(
							ArrayIndex(UnaryOperator(UOP_paren, addr), Identifier(loopidx)), 
							ASS_eq, 
							what
						)
					)
				)
			)
		);
}
