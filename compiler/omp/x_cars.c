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

#include <stdio.h>
#include <string.h>
#include "x_cars.h"
#include "x_clauses.h"
#include "ompi.h"
#include "ast_traverse.h"
#include "symtab.h"
#include "ast_vars.h"
#include "ast_copy.h"
#include "ast_xform.h"
#include "set.h"
#include "ast.h"


/* Initially, this set contains all the DECLAREd functions (and their
 * oroginal definitions, before any OMP transformations).
 * Then, after the CARS pass, each used function gets its own stats;
 * max_par_lev is an interprocedural metric.
 */
typedef struct {
	aststmt     definition; /* Pointer to definition in AST (NULL if decl only) */
	bool        visited;    /* True if actually called from somewhere */
	int         finished;   /* True if passed completely (recursion detection) */
	targstats_t stats;      /* The collected statistics */
} tdstruct_t;

int nowaitneeded = 0;
int locksneeded = 0;
static int indlev = 0;    /* Indentation level */

SET_TYPE_DEFINE(tdfuncs_st, symbol, tdstruct_t, 53)
SET_TYPE_IMPLEMENT(tdfuncs_st)
static set(tdfuncs_st) tdfuncs;

SET_TYPE_IMPLEMENT(critnames_st)

static travopts_t *tstrops;   /* Initialized by cars_analyze_declared_funcs */
targstats_t *analyze_block(aststmt t, targstats_t *ts);



void indent()
{
	int i;

	if (indlev > 0)
		for (i = 2 * indlev; i > 0; i--)
			putchar(' ');
}


static void showstats(targstats_t *stats)
{
  setelem(critnames_st) e;
  
  if (!stats) return;
  fprintf(stderr, "   parallel      = %d\n", carsmtr(stats,nparallel));
  fprintf(stderr, "   max_par_lev   = %d\n", carsmtr(stats,max_par_lev));
  fprintf(stderr, "   single        = %d\n", carsmtr(stats,nsingle));
  fprintf(stderr, "   for           = %d\n", carsmtr(stats,nfor));
  fprintf(stderr, "   schedstatic   = %d\n", carsmtr(stats,nschedstatic));
  fprintf(stderr, "   scheddynamic  = %d\n", carsmtr(stats,nscheddynamic));
  fprintf(stderr, "   schedguided   = %d\n", carsmtr(stats,nschedguided));
  fprintf(stderr, "   reduction     = %d\n", carsmtr(stats,nreduction));
  fprintf(stderr, "   ordered       = %d\n", carsmtr(stats,nordered));
  fprintf(stderr, "   atomic        = %d\n", carsmtr(stats,natomic));
  fprintf(stderr, "   forordered    = %d\n", carsmtr(stats,nforordered));
  fprintf(stderr, "   uncritical    = %d\n", carsmtr(stats,nuncritical));
  fprintf(stderr, "   nowait        = %d\n", carsmtr(stats,nnowait));
  fprintf(stderr, "   criticals     : ");
  if (!stats->critnames)
    fprintf(stderr, "none");
  else
  {
    fprintf(stderr, "%d, ", set_size(stats->critnames));
    for (e = stats->critnames->first; e; e = e->next)
      fprintf(stderr, "%s, ", e->key->name);
  }
  fprintf(stderr, "\n");
  fprintf(stderr, "   sections      = %d\n", carsmtr(stats,nsections));
  fprintf(stderr, "   tasks         = %d\n", carsmtr(stats,ntask));
  fprintf(stderr, "   hasfuncptr    = %d\n", carsmtr(stats,hasfuncptr));
  fprintf(stderr, "   hasrecursion  = %d\n", carsmtr(stats,hasrecursion));
  fprintf(stderr, "   hasextraicvs  = %d\n", carsmtr(stats,hasextraicvs));
  fprintf(stderr, "   haslocks      = %d\n", carsmtr(stats,haslocks));
}


void cars_stringify_stats(str s, targstats_t *stats)
{
  setelem(critnames_st) e;

  if (!stats) return;
  str_printf(s, "   parallel      = %d\n", carsmtr(stats,nparallel));
  str_printf(s, "   max_par_lev   = %d\n", carsmtr(stats,max_par_lev));
  str_printf(s, "   single        = %d\n", carsmtr(stats,nsingle));
  str_printf(s, "   for           = %d\n", carsmtr(stats,nfor));
  str_printf(s, "   schedstatic   = %d\n", carsmtr(stats,nschedstatic));
  str_printf(s, "   scheddynamic  = %d\n", carsmtr(stats,nscheddynamic));
  str_printf(s, "   schedguided   = %d\n", carsmtr(stats,nschedguided));
  str_printf(s, "   reduction     = %d\n", carsmtr(stats,nreduction));
  str_printf(s, "   ordered       = %d\n", carsmtr(stats,nordered));
  str_printf(s, "   atomic        = %d\n", carsmtr(stats,natomic));
  str_printf(s, "   forordered    = %d\n", carsmtr(stats,nforordered));
  str_printf(s, "   uncritical    = %d\n", carsmtr(stats,nuncritical));
  str_printf(s, "   nowait        = %d\n", carsmtr(stats,nnowait));
  str_printf(s, "   criticals     = ");
  if (!stats->critnames)
    str_printf(s, "0");
  else
  {
    str_printf(s, "%d, ", set_size(stats->critnames));
    for (e = stats->critnames->first; e; e = e->next)
      str_printf(s, "%s, ", e->key->name);
  }
  str_printf(s, "\n");
  str_printf(s, "   sections      = %d\n", carsmtr(stats,nsections));
  str_printf(s, "   tasks         = %d\n", carsmtr(stats,ntask));
  str_printf(s, "   hasfuncptr    = %d\n", carsmtr(stats,hasfuncptr));
  str_printf(s, "   hasrecursion  = %d\n", carsmtr(stats,hasrecursion));
  str_printf(s, "   hasextraicvs  = %d\n", carsmtr(stats,hasextraicvs));
  str_printf(s, "   haslocks      = %d\n", carsmtr(stats,haslocks));
}


static targstats_t *targstats_init(targstats_t *t)
{
	if (t == NULL)
	{
		t = (targstats_t *) smalloc(sizeof(targstats_t));
		memset(t, 0, sizeof(targstats_t));
	}
	else
	{
		set(critnames_st) s = t->critnames;

		memset(t, 0, sizeof(targstats_t));
		if (s)
		{
			set_drain(s);
			t->critnames = s;
		}
	}
	return (t);
}


static void visit_td_function(setelem(tdfuncs_st) e)
{
	e->value.visited  = 1;
	e->value.finished = 0;
	targstats_init(&(e->value.stats));
	analyze_block(e->value.definition->body, &(e->value.stats));
  showstats(&(e->value.stats));
	e->value.finished = 1;
}



static void funccall_c(astexpr t, void *state, int vistime)
{
	targstats_t           *s = (targstats_t *) state, *fs;
	setelem(tdfuncs_st)   e;
	setelem(critnames_st) cre;
	char *func_name;

	if (vistime != PREVISIT) return;

	if (t->left->type != IDENT)
	{
		carsmtr(s,hasfuncptr) = 1;  /* impossible function call (expression) */
		return;
	}

	if ((e = set_get(tdfuncs, t->left->u.sym)) == NULL)  /* Illegal */
	{
		warning("illegal function call to '%s'.\n", t->left->u.sym->name);
		return;
	}

	func_name = t->left->u.sym->name;

	/* Check if we have a lock function */
	if ((locksneeded == 0) && (strcmp(func_name, "omp_set_lock") == 0))
	{
		carsmtr(s,haslocks) = locksneeded = 1;
	}

	/* Check if we have task-related ICV functions */
	if ((!strcmp(func_name, "omp_set_dynamic"))
		|| (!strcmp(func_name, "omp_get_dynamic"))
		|| (!strcmp(func_name, "omp_set_nested"))
		|| (!strcmp(func_name, "omp_get_nested"))
		|| (!strcmp(func_name, "omp_get_schedule"))
		|| (!strcmp(func_name, "omp_set_schedule"))
		)
	{
		carsmtr(s,hasextraicvs) = 1;
	}

  if (!e->value.definition)     /* Only a prototype */
		return;

	if (!e->value.visited)        /* 1st time called */
		visit_td_function(e);

	/* Update our stats */
	fs = &e->value.stats;
	if (!e->value.finished || carsmtr(fs,hasrecursion))    /* Recursion */
		carsmtr(s,hasrecursion) = 1;
	else
	{
		if (carsmtr(s,cur_par_lev) + carsmtr(fs,max_par_lev) > carsmtr(s,max_par_lev))
			carsmtr(s,max_par_lev) = carsmtr(s,cur_par_lev) + carsmtr(fs,max_par_lev);
		if (fs->critnames)
		{
			if (s->critnames == NULL)
				s->critnames = set_new(critnames_st);
			for (cre = fs->critnames->first; cre; cre = cre->next)
				set_put_unique(s->critnames, cre->key);
		} 
    /* Update every other target region metric 
    except critnames */
    int i;
    for (i=0; i<NUM_METRICS-1; i++)
      s->mtr[i] += fs->mtr[i];
	}

  /* If we have recursion, we need to just note
  which metrics are being used by adding 1 */
  if (carsmtr(fs,hasrecursion))
  {
    int i;
    for (i=0; i<NUM_METRICS-7; i++)
      s->mtr[i] += (fs->mtr[i] > 0);
  }
}


static void ompdircrit_c(ompdir t, void *state, int vistime)
{
	targstats_t *s = (targstats_t *) state;

	if (vistime != PREVISIT) return;

	if (t->u.region)
	{
		if (s->critnames == NULL)
			s->critnames = set_new(critnames_st);
		set_put_unique(s->critnames, t->u.region);
	}
	else
		carsmtr(s,nuncritical)++;
}

static void oxconall_c(oxcon t, void *ignore, int vistime)
{
	if (vistime == PREVISIT)
		scope_start(stab);
	if (vistime == POSTVISIT)
		scope_end(stab);
}

static void ompclall_c(ompclause t, void *state, int vistime)
{
	targstats_t *s = (targstats_t *) state;
	if (vistime == PREVISIT) {
		switch (t->type)
		{
			case OCNOWAIT:
				carsmtr(s,nnowait)++;
				break;
			case OCREDUCTION:
				carsmtr(s,nreduction)++;
				break;
			default:
				break;
		}
	}
}

void analyze_dcfor(ompcon t, void *state)
{
  targstats_t *s = (targstats_t *) state;

  carsmtr(s,nfor)++;

  ompclause ordered = xc_ompcon_get_clause(t, OCORDERED);
  if (ordered != NULL)
  {
    if (carsmtr(s,nordered)==0) carsmtr(s,nordered)++; /* or forordered */
  }
  ompclause sch = xc_ompcon_get_clause(t, OCSCHEDULE);
  if (sch != NULL)
  {
    switch ( sch->subtype )
    {
      case OC_static:  carsmtr(s,nschedstatic)++; break;
      case OC_dynamic: carsmtr(s,nscheddynamic)++; break;
      case OC_guided:  carsmtr(s,nschedguided)++; break;
      default: 
        break;
    }
  }
}


static void ompconall_c(ompcon t, void *state, int vistime)
{
	targstats_t *s = (targstats_t *) state;

	switch (t->type)
	{
		case DCPARSECTIONS:
		case DCPARFOR:
		case DCPARALLEL:
			if (vistime == PREVISIT)
			{
				if (carsmtr(s,cur_par_lev) == 0)
				{
					if (t->type == DCPARSECTIONS)
						carsmtr(s,nsections)++;
					else if (t->type == DCPARFOR)
          {
						analyze_dcfor(t, s);
          }

					carsmtr(s,nparallel)++;         /* count # 1st-level parallel regions */
				}
				carsmtr(s,cur_par_lev)++;
				if (carsmtr(s,max_par_lev) < carsmtr(s,cur_par_lev))
					carsmtr(s,max_par_lev) = carsmtr(s,cur_par_lev);
			}
			if (vistime == POSTVISIT)
				carsmtr(s,cur_par_lev)--;
      
			break;
		case DCSINGLE:
			if (vistime == PREVISIT)
      {
				carsmtr(s,nsingle)++;
        if (xc_ompcon_get_clause(t, OCNOWAIT))
        	carsmtr(s,nnowait)++;
      }
      
			break;
		case DCSECTIONS:
			if (vistime == PREVISIT)
      {
				carsmtr(s,nsections)++;
				if (xc_ompcon_get_clause(t, OCNOWAIT))
        	carsmtr(s,nnowait)++;
      }
			break;
		case DCFOR:
			if (vistime == PREVISIT)
      {
				if (xc_ompcon_get_clause(t, OCNOWAIT))
        	carsmtr(s,nnowait)++;

        analyze_dcfor(t, s);
      }
			break;
		case DCTASK:
			if (vistime == PREVISIT)
				carsmtr(s,ntask)++;
			break;
		case DCATOMIC:
			if (vistime == PREVISIT)
				carsmtr(s,natomic)++;
			break;
		case DCORDERED:
			if (vistime == PREVISIT)
				carsmtr(s,nordered)++;
			break;
	}
}




/* This assumes that the statement is a block; any block actually.
 * If ts is NULL, it creates a new one.
 */
targstats_t *analyze_block(aststmt t, targstats_t *ts)
{
	void *tsbak;

	if (ts == NULL)
		ts = targstats_init(NULL);
	tsbak = tstrops->starg;   /* backup */
	tstrops->starg = ts;
	ast_stmt_traverse(t, tstrops);
	tstrops->starg = tsbak;   /* restore */
	return (ts);
}



targstats_t *cars_analyze_target(aststmt t)
{
	targstats_t *stats;

	stats = analyze_block(t, NULL);
	if (showdbginfo && stats)
	{
		fprintf(stderr, "TARGET analysis:\n--------------\n");
		showstats(stats);
	}
	return (stats);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     GATHER AND PASS ALL TARGET-DECLARED FUNCTIONS             *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void do_td_funcdecls(astdecl t)
{
	if(!t) return;
	switch (t->type)
	{
		case DFUNC:
			set_put_unique(tdfuncs, t->decl->u.id);  /* All fields 0 if new elem */
			break;
		case DLIST:
			do_td_funcdecls(t->u.next);
		default:
			do_td_funcdecls(t->decl);
	}
}


/* Gather all function definitions within a target declare region */
static void do_td_funcdef_nodes(aststmt t)
{
	if (!t) return;
	switch (t->type)
	{
		case DECLARATION:
			if (t->u.declaration.decl)
				do_td_funcdecls(t->u.declaration.decl);
			break;
		case FUNCDEF:
		{
			setelem(tdfuncs_st) e;
			e = set_put_unique(tdfuncs,
			            decl_getidentifier(t->u.declaration.decl->decl)->u.id);
			e->value.definition = t;
			e->value.visited = 0;     /* All other fields were initialized to 0 */
			break;
		}
		case STATEMENTLIST:
			do_td_funcdef_nodes(t->u.next);
			do_td_funcdef_nodes(t->body);
			break;
	}
}


/* Visit all target declare tree nodes */
static void do_target_declare_nodes(aststmt t)
{
	if (!t) return;
	if (t->type == STATEMENTLIST)
	{
		do_target_declare_nodes(t->u.next);
		do_target_declare_nodes(t->body);
	}
	else
		if (t->type == OMPSTMT && t->u.omp->type == DCDECLTARGET)
			do_td_funcdef_nodes(t->u.omp->body);
}


/* This searches the tree for function definitions inside TARGET DECLARE
 * regions and analyzes them
 */
void cars_analyze_declared_funcs(aststmt wholetree)
{
	setelem(tdfuncs_st) e;

	if (!tdfuncs)
		tdfuncs = set_new(tdfuncs_st);
	do_target_declare_nodes(wholetree);

	travopts_init_noop(tstrops = (travopts_t *) smalloc(sizeof(travopts_t))); /* Traverse options */

	tstrops->exprc.funccall_c = funccall_c;
	tstrops->ompclausec.ompclvars_c = ompclall_c;
	tstrops->ompclausec.prune_ompclexpr_c = 1; 
	tstrops->ompdcc.ompconall_c = ompconall_c;
	tstrops->ompdcc.ompdircrit_c = ompdircrit_c;
  tstrops->dodecl = 1;
	tstrops->when = PREPOSTVISIT;

	/* Now pass through all of them... */
	for (e = tdfuncs->first; e; e = e->next)
		if (e->value.definition && !e->value.visited)
			visit_td_function(e);

	/* Show stats */
  if (showdbginfo)
		for (e = tdfuncs->first; e; e = e->next)
		{
			fprintf(stderr, "FUNCTION: %s\n", (e->key)? e->key->name : "< no key >");
		};
}
