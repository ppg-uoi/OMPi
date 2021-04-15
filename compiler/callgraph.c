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

/* callgraph.c */

#include <assert.h>
#include "callgraph.h"
#include "ompi.h"

/* Uncomment the following to activate debugging */
//#define DBG 

#ifdef DBG
	#define DBGPRN(s) fprintf s;
#else
	#define DBGPRN(s) 
#endif
#undef DBG

SET_TYPE_DEFINE(cg_stag, symbol, struct { int visited; symbol *froms; }, 97)
SET_TYPE_IMPLEMENT(cg_stag)
static set(cg_stag) cg;     /* The call graph as a set of edges */

SET_TYPE_DEFINE(defuncs_stag, symbol, struct { int id; aststmt def; }, 97)
SET_TYPE_IMPLEMENT(defuncs_stag)
set(defuncs_stag) defuncs;  /* The set of defined functions */
static int nfd;             /* Their number */

SET_TYPE_IMPLEMENT(cgfun)


static setelem(cg_stag) cg_add_and_init(symbol new)
{
	setelem(cg_stag) e;
	int i;

	if ((e = set_get(cg, new)) == NULL)
	{
		DBGPRN((stderr, "[cg] adding new node %s\n", new->name));
		e = set_put(cg, new);
		e->value.visited = 0;
		/* nfd+1 to accomodate for initial (possibly fake) function */
		e->value.froms = (symbol *) smalloc((nfd+1) * sizeof(symbol));
		for (i = 0; i <= nfd; i++)
			e->value.froms[i] = NULL;
	}
	return (e);
}


void cg_add_edge(symbol from, symbol to)
{
	setelem(cg_stag) e = cg_add_and_init(to);
	int i;

	for (i = 0; i <= nfd; i++)
		if (e->value.froms[i] == from)
			return;
		else
			if (e->value.froms[i] == NULL)
			{
				DBGPRN((stderr, "[cg] adding new edge %s -> %s\n",from->name,to->name));
				e->value.froms[i] = from;
				return;
			};
	exit_error(1, "[cg_add_edge]: more functions than expected (%s -> %s)!\n",
	              from->name, to->name);
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


static void cg_ident(astexpr t, void *state, int ignore)
{
	cgstate_t *s = (cgstate_t *) state;
	stentry   f = symtab_get(stab, t->u.sym, FUNCNAME);
	
	if (f)   /* Pointer to a known function; we assume we call it */
	{
		DBGPRN((stderr, "[cg] found funcname identifier (%s)\n", t->u.sym->name));
		cg_add_edge(s->currfunc, t->u.sym);
		if (set_get(defuncs, t->u.sym))            /* internal function */
			if ( !(set_get(cg, t->u.sym)->value.visited) )
			{
				DBGPRN((stderr, "[cg]   going into %s\n", t->u.sym->name));
				symbol curr = s->currfunc;
				call_graph_do( set_get(defuncs, t->u.sym)->value.def );
				s->currfunc = curr;
			};
	}
}


static void cg_funccall(astexpr t, void *state, int ignore)
{
	cgstate_t  *s = (cgstate_t *) state;
	
	if (t->left->type != IDENT)
		s->hasfexpr = 1;  /* impossible function call (expression) */
	else
	{
		DBGPRN((stderr, "[cg] found funccall (%s)\n", t->left->u.sym->name));
		cg_add_edge(s->currfunc, t->left->u.sym);
		if (set_get(defuncs, t->left->u.sym))            /* internal function */
			if ( !(set_get(cg, t->left->u.sym)->value.visited) )
			{
				DBGPRN((stderr, "[cg]   going into %s\n", t->left->u.sym->name));
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
	DBGPRN((stderr, "[cg] found funcdef (%s)\n", s->currfunc->name));
	cg_add_and_init(s->currfunc)->value.visited = 1;
}


/* Finds all defined functions */
static void build_funcdef_set(aststmt t, set(defuncs_stag) set)
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


/**
 * This one discovers all the functions defined in the given AST.
 *
 * Any new injected functions could become known through some
 * cg_add_injected_func() function (although such functions are not
 * directly called by any other function).
 * 
 * It is better to call this function when the AST has been finalized.
 * s
 * 
 * @param tree The whole program tress
 */
void cg_find_defined_funcs(aststmt tree)
{
	set_init(defuncs_stag, &defuncs);
	nfd = 0;
	build_funcdef_set(tree, defuncs);
	nfd = set_size(defuncs);
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


/**
 * Build the call graph starting from a given statement (if the statement is not
 * at the translation unit level, we discover the function that encloses it).
 * TODO: maybe we should have an option to include a function if its named is
 *       mentioned in an expression, other than funccall (e.g. passed as an
 *       argument to some function).
 * @param  wholetree the whole AST
 * @param  t         the statement to start from
 * @return A 2D array representing the whole graph (see callgraph.h)
 */
cg_t *cg_build_call_graph(aststmt t)
{
	aststmt   funcdef = ast_get_enclosing_function(t);
	cgstate_t st = { NULL, 0 }; 
	
	if (!defuncs) return NULL;
	if (funcdef)
		st.currfunc = funcdef->u.declaration.decl->decl->u.id;
	set_init(cg_stag, &cg);
	
	/* Traverse the statement */
	travopts_init_noop(&cgtrops);
	cgtrops.exprc.ident_c = cg_ident;
	cgtrops.exprc.funccall_c = cg_funccall;
	cgtrops.stmtc.funcdef_c = cg_funcdef;
	cgtrops.starg = &st;
	call_graph_do(t);
	if (st.hasfexpr)
		warning("[call_graph]: possibly incomplete analysis\n");
	
	/* Get the struct */
	return ( cgset_to_cgmat() );
}


/**
 * Builds the call graph starting from a given statement and from that
 * it returns a set of all the functions called.
 * @param  t         the statement to start from
 * @return A set with the called functions
 */
set(cgfun) cg_find_called_funcs(aststmt t)
{
	static set(cgfun) funcs = NULL;
	setelem(cg_stag) e;
	cgstate_t st = { NULL, 0 }; 
	symbol startfunc;
	bool   selfcall = false;       /* Whether this func calls itself */
	int    i;
	
	if (!defuncs) return NULL;
	
	/* If t is not a function definition, pretend we start from a fake func */
	startfunc = (t->type != FUNCDEF) ? Symbol("__fakefunc__") : 
	                             decl_getidentifier_symbol(t->u.declaration.decl);

	st.currfunc = startfunc;
	set_init(cg_stag, &cg);
	cg_add_and_init(startfunc);
	
	/* Traverse the statement */
	travopts_init_noop(&cgtrops);
	cgtrops.exprc.ident_c = cg_ident;
	cgtrops.exprc.funccall_c = cg_funccall;
	cgtrops.stmtc.funcdef_c = cg_funcdef;
	cgtrops.starg = &st;
	call_graph_do(t);

	/* Check whether we call ourselves */
	if (startfunc != Symbol("__fakefunc__"))
		for (e = cg->first; e; e = e->next)
			if (e->key == startfunc)
				for (i = 0; i <= nfd; i++)
					if (e->value.froms[i] == startfunc)
						selfcall = true;

	/* Copy the functions, skipping startfunc (unless it recurses) */
	set_init(cgfun, &funcs);
	for (e = cg->first; e; e = e->next)
		if (e->key != startfunc || selfcall)
			set_put(funcs, e->key);
	return (funcs);
}


static void call_graph_debug(cg_t *g)
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


void cg_defuncs_debug()
{
	setelem(defuncs_stag) f;
	
	fprintf(stderr, "-- All defined functions:\n");
	for (f = defuncs->first; f; f = f->next)
		fprintf(stderr, "\t%s\n", f->key->name);
	fprintf(stderr, "---\n");
}


/** 
 * Should be called with the whole AST as its parameter
 */
void cg_call_graph_test(aststmt tree)
{
	int  i, j; 
	cg_t *g;
	
	if (!defuncs) /* We assume here that tree = whole AST */
	{
		cg_find_defined_funcs(tree);
		if (!defuncs)
			return;
	}
	
	g = cg_build_call_graph(tree);
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
