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

/* callgraph.c */

#include <assert.h>
#include "callgraph.h"
#include "set.h"
#include "ompi.h"

SET_TYPE_DEFINE(cg_stag, symbol, struct { int visited; symbol *froms; }, 97)
SET_TYPE_IMPLEMENT(cg_stag)
static set(cg_stag) cg;     /* The call graph as a set of edges */

SET_TYPE_DEFINE(defuncs_stag, symbol, struct { int id; aststmt def; }, 97)
SET_TYPE_IMPLEMENT(defuncs_stag)
set(defuncs_stag) defuncs;  /* The set of defined functions */
static int nfd;             /* Their number */


static setelem(cg_stag) cg_add_and_init(symbol new)
{
	setelem(cg_stag) e;
	int i;

	if ((e = set_get(cg, new)) == NULL)
	{
		e = set_put(cg, new);
		e->value.visited = 0;
		e->value.froms = (symbol *) smalloc(nfd * sizeof(symbol));
		for (i = 0; i < nfd; i++)
			e->value.froms[i] = NULL;
	}
	return (e);
}


void cg_add_edge(symbol from, symbol to)
{
	setelem(cg_stag) e = cg_add_and_init(to);
	int i;

	for (i = 0; i < nfd; i++)
		if (e->value.froms[i] == from)
			return;
		else
			if (e->value.froms[i] == NULL)
			{
				e->value.froms[i] = from;
				return;
			};
	exit_error(1, "[cg_add_edge]: more functions than expected!\n");
}


typedef struct { 
	symbol currfunc;   /* The current function we traverse */
	int    hasfexpr;   /* True if there is a function call we cannot evaluate */
} cgstate_t;

travopts_t cgtrops;


static void call_graph_do(aststmt t)
{
	ast_stmt_traverse(t, &cgtrops);
}


static void cg_funccall(astexpr t, void *state, int ignore)
{
	cgstate_t        *s = (cgstate_t *) state;
	setelem(cg_stag) e;
	
	if (t->left->type != IDENT)
		s->hasfexpr = 1;  /* impossible function call (expression) */
	else
	{
		cg_add_edge(s->currfunc, t->left->u.sym);
		if (set_get(defuncs, t->left->u.sym))            /* internal function */
			if ( !(set_get(cg, t->left->u.sym)->value.visited) )
			{
				symbol curr = s->currfunc;
				call_graph_do( set_get(defuncs, t->left->u.sym)->value.def );
				s->currfunc = curr;
			};
	}
}


static void cg_funcdef(aststmt t, void *state, int ignore)
{
	cgstate_t *s = (cgstate_t *) state;
	
	s->currfunc = t->u.declaration.decl->decl->decl->u.id;
	cg_add_and_init(s->currfunc)->value.visited = 1;
}


/* Finds all defined functions */
void build_funcdef_set(aststmt t, set(defuncs_stag) set)
{
	if (!t) return;
	if (t->type == STATEMENTLIST)
	{
		build_funcdef_set(t->u.next, set);
		build_funcdef_set(t->body, set);
	}
	else
		if (t->type == FUNCDEF)
		{
			setelem(defuncs_stag) e;
			e = set_put(set, decl_getidentifier(t->u.declaration.decl->decl)->u.id);
			e->value.id  = set_size(set) - 1;
			e->value.def = t;
		};
}


cg_t *cgset_to_cgmat()
{
	setelem(defuncs_stag) f;
	setelem(cg_stag) e;
	int  i, to, fid = nfd;  /* fid = first id of external functions */
	cg_t *g = (cg_t *) smalloc(sizeof(cg_t));

	/* Check how many of the internally defined functions are in the cg set */
	for (i = 0, f = defuncs->first; f; f = f->next)
		if (set_get(cg, f->key))
			i++;
	g->next = set_size(cg) - i;  /* This is the number of external functions */
	g->nint = nfd;
	
	g->funcs = (symbol *) smalloc((nfd + g->next)*sizeof(symbol));
	for (i = 0, f = defuncs->first; f; f = f->next, i++)
		g->funcs[i] = f->key;      /* Store all internal function symbols */
	
	g->cgmat = (char **) smalloc(nfd*sizeof(char *));
	for (i = 0; i < nfd; i++)
	{
		g->cgmat[i] = (char *) calloc((nfd + g->next), sizeof(char));
		if (!g->cgmat[i])
			exit_error(1, "[cgset_to_cgmat]: calloc() out of memory\n");
	}
	
	for (e = cg->first; e; e = e->next)
	{
	  if ((f = set_get(defuncs, e->key)) != NULL)
		  to = f->value.id;
		else
		{
			to = fid++;
			g->funcs[to] = e->key;     /* Store the external function symbol */
		}
		for (i = 0; i < nfd && e->value.froms[i] != NULL; i++)
			g->cgmat[ set_get(defuncs, e->value.froms[i])->value.id ][to] = 1;
	}
	
	return (g);
}


cg_t *call_graph(aststmt wholetree, aststmt t)
{
	aststmt    funcdef = ast_get_enclosing_function(t);
	cgstate_t  st = { NULL, 0 }; 
	
	/* Get all defined functions first */
	if (!defuncs)
	{
		defuncs = set_new(defuncs_stag);
		build_funcdef_set(wholetree, defuncs);
		nfd = set_size(defuncs);
	}

	if (funcdef)
		st.currfunc = funcdef->u.declaration.decl->decl->u.id;
	set_init(cg_stag, &cg);
	
	/* Traverse the AST */
	travopts_init_noop(&cgtrops);
	cgtrops.exprc.funccall_c = cg_funccall;
	cgtrops.stmtc.funcdef_c = cg_funcdef;
	cgtrops.starg = &st;
	call_graph_do(t);
	if (st.hasfexpr)
		warning("[call_graph]: possibly incomplete analysis\n");
	
	/* Get the struct */
	return ( cgset_to_cgmat() );
}


void call_graph_debug(cg_t *g)
{
	int i, j; 
	
	fprintf(stderr, "CALL GRAPH:\n===========\n");
	for (i = 0, fprintf(stderr, " internal funcs:\n"); i < g->nint; i++)
		fprintf(stderr, "   (%d) %s\n", i, g->funcs[i]->name);
	for (i = 0, fprintf(stderr, " external funcs:\n"); i < g->next; i++)
		fprintf(stderr, "   (%d) %s\n", i+g->nint, g->funcs[i + g->nint]->name);
	for (i = 0, fprintf(stderr, " calls:\n"); i < g->nint; i++)
		for (j = 0; j < g->next+g->nint; j++)
			if (g->cgmat[i][j])
				fprintf(stderr, "   from (%d) to (%d)\n", i, j);
}


void call_graph_test(aststmt tree)
{
	int  i, j; 
	cg_t *g;
	
	g = call_graph(tree, tree);
	if (!g)
		return;
	
	printf("digraph G {\n");
	printf("  node [shape=box,style=filled,fillcolor=white];\n");
	for (i = 0; i < g->next; i++)
		printf("\"%s\" [fillcolor=lightgray]\n", g->funcs[i + g->nint]->name);
	for (i = 0; i < g->nint; i++)
		for (j = 0; j < g->next+g->nint; j++)
			if (g->cgmat[i][j])
				printf("\"%s\" -> \"%s\"\n", g->funcs[i]->name, g->funcs[j]->name);
	printf("}\n");
}
