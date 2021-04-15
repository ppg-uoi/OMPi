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

/* ast_free.c -- free the nodes on an AST */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ast_traverse.h"
#include "ast_free.h"

static travopts_t *freetrops;


static void free_stmt(aststmt t, void *ignore, int ignore2)
{
	if (t) free(t);
}

static void free_verbatim(aststmt t, void *ignore, int ignore2)
{
	if (t)
	{
		if (t->u.code)
			free(t->u.code);
		free(t);
	}
}

static void _free_asmop(asmop t)
{
	if (!t) return; 
	if (t->symbolicname) ast_expr_free(t->symbolicname);
	free(t->constraint);
	_free_asmop(t->next);
	_free_asmop(t->op);
	free(t);
}

static void free_asmstmt(aststmt t, void *ignore, int ignore2)
{
	if (t)
	{
		free(t->u.assem->template);
		if (t->u.assem->ins)
			_free_asmop(t->u.assem->ins);
		if (t->u.assem->outs)
			_free_asmop(t->u.assem->outs);
		free(t);
	}
}

static void free_expr(astexpr t, void *ignore, int ignore2)
{
	if (t) free(t);
}

static void free_expr_fixed(astexpr t, void *ignore, int ignore2)
{
	if (t)
	{
		if (t->u.str)
			free(t->u.str);
		free(t);
	}
}

static void free_spec(astspec t, void *ignore, int ignore2)
{
	if (t) 
	{
		if (t->type == ATTRSPEC && t->u.txt != NULL)
			free(t->u.txt);
		free(t);
	}
}

static void free_decl(astdecl t, void *ignore, int ignore2)
{
	if (t) free(t);
}


/* export */
void ast_ompxli_free(ompxli xl)
{
	if (!xl) return;
	if (xl->xlitype != OXLI_IDENT)
	{
		omparrdim s, next;
		for (s = xl->dim; s; s = next)
		{
			next = s->next;
			if (s->lb) ast_expr_free(s->lb);
			if (s->len) ast_expr_free(s->len);
			free(s);
		}
	}
	free(xl);
}

static void free_ompclause(ompclause t, void *ignore, int ignore2)
{
	// TODO: fix memory leak for extended list items...
	if (t) free(t);
}


static void free_ompdir(ompdir t, void *ignore, int ignore2)
{
	if (t) free(t);
}


static void free_ompcon(ompcon t, void *ignore, int ignore2)
{
	if (t) free(t);
}


static void free_oxclause(oxclause t, void *ignore, int ignore2)
{
	if (t) free(t);
}

static void free_oxdir(oxdir t, void *ignore, int ignore2)
{
	if (t) free(t);
}

static void free_oxcon(oxcon t, void *ignore, int ignore2)
{
	if (t) free(t);
}


static void init_free_trops()
{
	if (freetrops) return;
	
	travopts_init_batch(freetrops = (travopts_t *) smalloc(sizeof(travopts_t)), 
	                    free_stmt, free_expr, free_spec, free_decl, 
	                    free_ompclause, free_ompdir, free_ompcon, 
	                    free_oxclause, free_oxdir, free_oxcon);
	freetrops->stmtc.verbatim_c = free_verbatim;
	freetrops->stmtc.asmstmt_c = free_asmstmt;;
	freetrops->exprc.constval_c = free_expr_fixed;
	freetrops->exprc.string_c = free_expr_fixed;
	freetrops->when = POSTVISIT;
}


/* Too lazy to type them all... */
#define FREE_FUNC(TYPE,ARGTYPE) \
	void ast_##TYPE##_free(ARGTYPE t) { \
		if (!freetrops) init_free_trops(); \
		ast_##TYPE##_traverse(t, freetrops); \
	}

FREE_FUNC(expr,astexpr)
FREE_FUNC(stmt,aststmt)
FREE_FUNC(decl,astdecl)
FREE_FUNC(spec,astspec)
FREE_FUNC(ompclause,ompclause)
FREE_FUNC(ompdir,ompdir)
FREE_FUNC(ompcon,ompcon)
FREE_FUNC(oxclause,oxclause)
FREE_FUNC(oxdir,oxdir)
FREE_FUNC(oxcon,oxcon)
