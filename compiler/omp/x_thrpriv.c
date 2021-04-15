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

/* x_thrpriv.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ast_free.h"
#include "ast_xform.h"
#include "ast_copy.h"
#include "ast_vars.h"
#include "x_thrpriv.h"
#include "x_clauses.h"
#include "symtab.h"
#include "ompi.h"
#include "builder.h"


static int  tpngnum = -1;
static char _tpnew[128], _tpkey[128];


/* The new name given to the original threadprivate var */
symbol tp_new_name(symbol var)
{
	snprintf(_tpnew, 127, "tp_%s_", var->name);
	return (Symbol(_tpnew));
}


/* The key for the given var (requires the stentry) */
symbol tp_key_name(stentry e)
{
	if (e->ival == -1)
		snprintf(_tpkey, 127, "tp_%s_key_", e->key->name);
	else
		snprintf(_tpkey, 127, "tpng%d_%s_key_", e->ival, e->key->name);
	return (Symbol(_tpkey));
}


/* Stuff:
 * ======
 * The parser has taken care of threadprivate variables up to a certain
 * point; specifically it has REPLACED all identifiers that refer to
 * threadprivate variables, in all expressions, with POINTERS to those
 * variables.
 * That's it; nothing else. Note that the AST is NOT semantically correct
 * at this point. What we have to do here is actually DECLARE those
 * pointers and, of course, change the names of the original variables
 * since those names are now used for the pointers.
 * Consequently, for every variable X in a threadprivate clause we must
 *   (1) Rename the original variable to something like _tp_X.
 *   (2) Declare a pointer X, which will be initialized to the address of
 *       the thread-specific copy of the original variable
 *   (3) Declare a global key like tp_key_X to make the thread-specific
 *       storage possible (see the pthreads manual).
 * (2) is easy for static block-scope variables; the declaration is made
 * in-situ. It is tuffer for global threadprivate vars: we must do (2) in
 * every function referencing the variable.
 */


/* Non-global */
static
aststmt tp_ng_new(symbol var)
{
	stentry e = symtab_get(stab, var, IDNAME);
	astdecl decl = e->decl;
	symbol  s;

	tpngnum++;
	e->isthrpriv = true;         /* Mark */
	e->ival = tpngnum;        /* Remember so as to recreate this key name */

	/* Add a global key; we add a numeric id since there may exist multiple
	 * non-global threadprivate vars with the same name
	 */
	bld_globalvar_add(Declaration(Speclist_right(
	                           StClassSpec(SPEC_static),
	                           Declspec(SPEC_void)
	                         ),
	                         Declarator(
	                           Pointer(),
	                           IdentifierDecl(tp_key_name(e))
	                         )));
	/* Change the name in the original declaration (not the symbol table) */
	s = tp_new_name(var);
	decl_rename(decl, s);

	/* Create a new declaration locally */
	return (tp_declaration(e, var, Identifier(s), false));
}


/* For global ones */
static
aststmt tp_g(symbol var)
{
	stentry e = symtab_get(stab, var, IDNAME);
	aststmt new;

	e->isthrpriv = true;
	e->ival      = -1;     /* Don't mix with non-global ones */

	/* Add a global key  */
	new = Declaration(Speclist_right(
	                    StClassSpec(SPEC_static),
	                    Declspec(SPEC_void)
	                  ),
	                  Declarator(
	                    Pointer(),
	                    IdentifierDecl(tp_key_name(e))
	                  ));

	/* Change the name in the original declaration.
	 * NOTE:
	 *   We don't change the symbol table -- i.e. the variable is still
	 *   known by its original name (=> its appearances will not produce
	 *   "unknown identifier"s). BUT, if one duplicates the declaration,
	 *   the new name will be included.
	 */
	decl_rename(e->decl, tp_new_name(var));

	return (new);
}


/* When in pure process mode just mark the var as threadprivate; nothing else */
static
aststmt tp_procmode(symbol var)
{
	symtab_get(stab, var, IDNAME)->isthrpriv = true;
	return (NULL);
}


static
aststmt thrpriv_handlelist(astdecl d)
{
	aststmt l = NULL, st = NULL;

	if (d->type == DLIST && d->subtype == DECL_idlist)
	{
		l = thrpriv_handlelist(d->u.next);
		d = d->decl;
	}
	assert(d->type == DIDENT);

	if (!threadmode)               /* Just mark as threadprivate */
		return (tp_procmode(d->u.id));

	/* The identifier is known for sure (the parser guarantees it) */
	if (stab->scopelevel > 0)      /* In non-global scope */
		st = tp_ng_new(d->u.id);
	else
		st = tp_g(d->u.id);
	if (l) st = BlockList(l, st);
	return (st);
}


void xform_threadprivate(aststmt *t)
{
	aststmt parent = (*t)->parent, v, decls;

	v = ompdir_commented((*t)->u.omp->directive); /* Put directive in comments */
	decls = thrpriv_handlelist((*t)->u.omp->directive->u.varlist);
	ast_ompcon_free((*t)->u.omp);
	*t = (decls != NULL) ? BlockList(decls, v) : v;
	ast_stmt_parent(parent, *t);
}


/* Takes a compound (specifically the body of a function, but any other
 * compound can do), discovers all global threadprivate (gtp) vars used
 * and declares pointers to those @ the top of the compound.
 */
void tp_fix_funcbody_gtpvars(aststmt t)
{
	stentry       v;
	setelem(vars) e;
	aststmt       p, st, l = NULL;
	symbol        s;

	if (!threadmode)               /* Just ignore in pure process mode */
		return;

	if (t == NULL || t->body == NULL) return;


	for (e = analyze_find_gtp_vars(t)->first; e; e = e->next)
	{
		/* Get actual var */
		v = symtab_get(stab, e->key, IDNAME);
		/* Add a new pointer declaration */
		sprintf(_tpkey, "tp_%s_key_", v->key->name);
		s = tp_new_name(v->key);
		st = Declaration(
		       ast_spec_copy_nosc(v->spec),
		       InitDecl(
		         decl_topointer(decl_rename(ast_decl_copy(v->decl), e->key)),
		         FunctionCall(
		           IdentName("ort_get_thrpriv"),
		           Comma3(
		             UOAddress(IdentName(_tpkey)),
		             Sizeof(Identifier(s)),
		             UOAddress(Identifier(s))
		           )
		         )
		       )
		     );
		if (!l)
			l = st;
		else
		{
			l = BlockList(p = l, st);
			p->parent = st->parent = l;          /* parentize correctly */
		}
	}
	if (l)
	{
		t->body = BlockList(l, p = t->body);
		p->parent = l->parent = t->body;
		t->body->parent = t;
	}
}


/**
 * Declares and initializes a pointer to a threadprivate var.
 * 
 * @param e       the original tp var
 * @param newvar  the name of the new pointer var
 * @param base    where we intialize from (normally, Identifier(e->key))
 * @param baseisptr is a flag to denote that base is a pointer or not
 *                in the non-pointer case we get:
 *                   newvar = ort_get_thrpriv(<orig_key>, sizeof(base), &base)
 *                else:
 *                   newvar = ort_get_thrpriv(<orig_key>, sizeof(*base), base)
 * 
 * @return A declaration statement.
 */
aststmt tp_declaration(stentry e, symbol newvar, astexpr base, bool baseisptr)
{
	return
	  Declaration(
	    ast_spec_copy_nosc(e->spec),
	    InitDecl(
	      decl_topointer(decl_rename(ast_decl_copy(e->decl), newvar)),
	      FunctionCall(
	        IdentName("ort_get_thrpriv"),
	        Comma3(
	          UOAddress(Identifier(tp_key_name(e))),
	          Sizeof(baseisptr ? Deref(Parenthesis(base)) : base),
	          /* **MUST** copy the "base", not use it for a 2nd time! */
	          baseisptr ? ast_expr_copy(base) : UOAddress(ast_expr_copy(base))
	        )
	      )
	    )
	  );
}
