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

/* dfa.h -- Data flow analysis module for OMPi */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "symtab.h"
#include "ompi.h"
#include "dfa.h"
#include "ast_traverse.h"

#define mySym tree->u.sym
#define reading (!d->func&& !d->writing && !d->readWrite )
#define test fprintf(stderr,"This is a test\n")

void show_autoscope_results(autoScopeSets as);

int inFunc = 0;
int inPriv = 0, inFp = 0, inLp = 0, inRd = 0;
int justPassingBy = 0;

parNode *parListStart;

static int dr[NCASES][NCASES] =
{
	{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1},
	{1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0}
};


void force(int type, symbol s, int clauseType, dfa_t d);
int isScoped(astexpr t, dfa_t d);

dfa_func_t *dfa_funcListStart;


void addToFuncList(symbol f, aststmt fBody, dfa_func_t **listStart)
{
	dfa_func_t *temp = (dfa_func_t *) smalloc(sizeof(dfa_func_t));

	temp->funcName = f;
	temp->funcBody = fBody;
	temp->next     = *listStart;
	*listStart = temp;
}


void freeFuncList()
{
	dfa_func_t *tmp = dfa_funcListStart;

	while (tmp != NULL)
	{
		dfa_funcListStart = tmp;
		tmp = tmp->next;
		free(dfa_funcListStart);
	}
}

aststmt find_func(symbol f, dfa_func_t *list)
{
	for (; list; list = list->next)
		if (list->funcName == f)
			return list->funcBody;
	return NULL;
}


void symListEraseCurrent(dfa_t d)
{
	autoSym *temp;
	int i;
	for (temp = d->symListStart; temp; temp = temp->next)
	  for (i = 0; i < NCASES; i++)
			temp->current[i] = 0;
}


void symListEraseActive(dfa_t d)
{
	autoSym *temp;
	int i;
	for (temp = d->symListStart; temp; temp = temp->next)
	  for (i = 0; i < NCASES; i++)
			temp->active[i] = 0;
}


void transferInfo(dfa_t d)
{
	autoSym *temp;
	int i;
	
	for(temp = d->symListStart; temp; temp = temp->next)
		for (i = 0; i < NCASES; i++)
			temp->active[i] = (temp->current[i] || temp->active[i]);
}

void transferPFCI(dfa_t d)
{

	autoSym *temp;
	temp = d->symListStart;
	while (temp)
	{
		temp->active[DFA_FREE] = (temp->active[DFA_FREE] || temp->active[DFA_PFCI]);
		temp->active[DFA_FREE + 1] = (temp->active[DFA_FREE + 1]
		                              || temp->active[DFA_PFCI + 1]);
		temp->active[DFA_PFCI] = 0;
		temp->active[DFA_PFCI + 1] = 0;
		temp = temp->next;
	}
}

parNode *insertToAutoList(autoScopeSets aSs, int l)
{
	parNode *p = (parNode *) smalloc(sizeof(parNode));
	p->a = aSs;
	p->line = l;
	p->next = NULL;

	if (parListStart != NULL)
		p->next = parListStart;
	return ( parListStart = p );
}

parNode *isInAutoList(int l)
{
	parNode *p = parListStart;

	for (; p; p = p->next)
		if (p->line == l)
			return p;
	return NULL;
}

void dfa_removeFromAutoList(int l)
{
	parNode *p = parListStart, *temp;

	if (parListStart == NULL)
		return;
	if (p->line == l)
	{
		parListStart = p->next;
		free(p);
	}
	else
		for ( ; p->next != NULL; p = p->next)
			if (p->next->line == l)
			{
				p->next = (temp = p->next)->next;
				free(temp);
				break;
			};
}

void addToSymList(symbol s1, dfa_t d)
{
	autoSym *a = (autoSym *) smalloc(sizeof(struct _autoSym));
	int i;
	a->s = s1;
	for (i = 0; i < NCASES; i++)
	{
		a->active[i] = 0;
		a->current[i] = 0;
	}

	a->next = d->symListStart;
	d->symListStart = a;
}

autoSym *isInSymList(symbol s1, dfa_t d)
{
	autoSym *temp;
	temp = d->symListStart;
	int i = 0;
	while (temp != NULL)
	{
		i++;
		if (temp->s == s1)
			return temp;

		temp = temp->next;

	}
	return NULL;
}

void setActive(symbol s, int pos, int val, dfa_t d)
{
	autoSym *p = isInSymList(s, d);
	if (p != NULL)
		p->active[pos] = val;
}

void setCurrent(symbol s, int pos, int val, dfa_t d)
{
	autoSym *p = isInSymList(s, d);
	if (p != NULL)
		p->current[pos] = val;
}

int getActive(symbol s, int pos, dfa_t d)
{
	autoSym *p = isInSymList(s, d);
	if (p != NULL)
		return (p->active[pos]);
	else
		return -1;
}

int getCurrent(symbol s, int pos, dfa_t d)
{
	autoSym *p = isInSymList(s, d);
	if (p != NULL)
		return (p->current[pos]);
	else
		return -1;
}

void clearSymList(dfa_t d)
{
	autoSym *temp;
	if (d->symListStart == NULL)
		return;

	temp = d->symListStart->next;
	while (temp)
	{
		free(d->symListStart);
		d->symListStart = temp;
		temp = d->symListStart->next;
	}
	free(d->symListStart);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     SET IMPLEMENTATION & FUNCTIONALITY                        *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


SET_TYPE_IMPLEMENT(dfa)


/* Maybe they are useful in other contexts as well (put in set.h?) */
/* This takes 0(n) (assuming search is O(1)), where n=min{size(a),size(b)} */
set(dfa) set_intersection(set(dfa) a, set(dfa) b)
{
	setelem(dfa) e;
	set(dfa)     res;

	if (set_size(a) > set_size(b))     /* Determine smallest set */
	{
		res = a;
		a = b;
		b = res;
	}
	res = set_new(dfa);
	for (e = a->first; e; e = e->next)
		if (set_get(b, e->key))          /* Include only if e is also in b */
			set_put(res, e->key);
	return res;
}

/* This takes 0(n+m), where n = size(a), m = size(b) */
set(dfa) set_union(set(dfa) a, set(dfa) b)
{
	setelem(dfa) e;
	set(dfa)     res, intersection;

	if (set_size(a) < set_size(b))     /* Determine largest set */
	{
		res = a;
		a = b;
		b = res;
	}
	res = set_new(dfa);
	set_copy(res, a);                  /* Add a as is */
	for (e = b->first; e; e = e->next) /* Check elements of set b */
		set_put_unique(res, e->key);
	return res;
}

/* This takes 0(n), where n = size(a) */
set(dfa) set_difference(set(dfa) a, set(dfa) b)
{
	setelem(dfa) e;
	set(dfa)     res = set_new(dfa);
	
	for (e = a->first; e; e = e->next)
		if (!set_get(b, e->key))         /* Add only if it does not belong to b */
			set_put(res, e->key);
	return res;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Main part of the data race detection algorithm,
 *  using the data race transition table
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void checkForDataRace(symbol s, dfa_t d)
{
	/* First we adjust the directive case: Reading cases are defined as [d->writing cases]+1 */
	int idCase = d->dirCase;
	int i;
	if (reading)
		idCase++;

	if (idCase == DFA_FREE)
		set_put_unique(d->dataRaceSet,
		         s); // If we are in the parallel region, a write causes datarace, no need for further check
	else
	{
		/* We use the transition table to detect the data race:
		 * We check if any active data access causes data race when colliding with the current state */
		for (i = 0; i < NCASES; i++)
		{
			if (getActive(s, i, d) > 0 && dr[ idCase ][ i ])
			{
				set_put_unique(d->dataRaceSet, s);
				break;
			}
		}
	}
	/* The current case is stored, and will be active at the end of the directive */
	if (d->dirCase != DFA_PFCI && d->dirCase != DFA_FREE)
		setCurrent(s, idCase, 1, d);
	else
		setActive(s, idCase, 1, d);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Check to decide if we have an array occurance
 *  who's index is the parallel for counter
 *  PFCI: Parallel For Counter Index
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void makePFCIchecks(symbol s, dfa_t d)
{
	/* This is in case it's a counter of a parallel for: we need to store it and scope it as private*/
	if (d->isInParallelFor && d->parForIndex == NULL && !d->clauses)
	{
		d->parForIndex = s;
		set_put_unique(d->parIndexSet, s);
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void adjustRwOrder(symbol s, dfa_t d)
{
	int idCase = d->dirCase;
	
	if (reading)
		idCase++;

	switch (idCase)
	{
		case DFA_CRITICAL:
		case DFA_ATOMIC:
		case DFA_FREE:
		case DFA_PFCI:
			if (d->readWrite && !set_get(d->W, s))
			{
				if (set_get(d->wSoloOld, s))
					set_put_unique(d->WR, s);
				else
					set_put_unique(d->R, s);
			}
			else
				if (d->writing && !set_get(d->R, s))
				{
					if (set_get(d->wSoloOld, s))
						set_put_unique(d->RW, s);
					else
						set_put_unique(d->W, s);
				}
			break;

		case DFA_CRITICAL + 1:
		case DFA_ATOMIC + 1:
		case DFA_FREE + 1:
		case DFA_PFCI + 1:
			if (!set_get(d->W, s))
			{
				if (set_get(d->wSoloOld, s))
					set_put_unique(d->WR, s);
				else
					set_put_unique(d->R, s);
			}
			break;

		case DFA_MASTER + 1:
		case DFA_SINGLE + 1:
			if (!set_get(d->W, s) && !set_get(d->R, s) && !set_get(d->wSolo, s)
			    && !set_get(d->wSolo, s))
			{
				if (set_get(d->wSoloOld, s))
					set_put_unique(d->WR, s);
				else
					set_put_unique(d->wSolo, s);
			}
			break;
		case DFA_MASTER:
		case DFA_SINGLE:
			if (!set_get(d->W, s) && !set_get(d->R, s) && !set_get(d->wSolo, s)
			    && !set_get(d->wSolo, s))
			{
				if (set_get(d->wSoloOld, s))
					set_put_unique(d->RW, s);
				else
					set_put_unique(d->wSolo, s);
			}
			break;
	}
}


void dfa_handleBarrier(dfa_t d)
{
	symListEraseActive(d);
}

void force(int type, symbol s, int clauseType, dfa_t d)
{
	int     isProtected = (clauseType==DFA_CRITICAL || clauseType==DFA_ATOMIC);
	void    ident_c(astexpr tree, void *state, int vistime);
	astexpr fEx;
	
	type = d->dirCase;
	d->func = 0;

	switch (type)
	{
		case DFA_PRIVATE:
			d->writing = 1;
			d->readWrite = 0;
			return;
		case DFA_FIRSTPRIVATE:
			d->writing = 0;
			d->readWrite = 0;
			break;
		case DFA_REDUCTION:
			d->writing = 0;
			d->readWrite = 1;
			break;
	}

	if (isProtected)
		d->dirCase = clauseType;
	else
		d->dirCase = DFA_FREE;

	d->foundNested--;
	ident_c(fEx = Identifier(s), d, PREVISIT);
	free(fEx);
	d->foundNested++;

	d->dirCase = type;
}


void accessSet(set(dfa) a, int setType, dfa_t d)
{
	stentry e;

	switch (setType)
	{
		case DFA_FIRSTPRIVATE:
		case DFA_PRIVATE:
		case DFA_REDUCTION:
			for (e = ((symtab) a)->top; e; e = e->stacknext)
			{
				force(setType, e->key,  d->dirCase, d);
				set_put_unique(d->ignoreSet, e->key);
			}
		case DFA_SHARED:
			break;
		case DFA_DEFAUTO:
			for (e = ((symtab) a)->top; e; e = e->stacknext)
				addToSymList(e->key, d);
			break;
	}
}

void dfa_handleSoloEnd(dfa_t d)
{
	set(dfa) temp;
	temp = d->wSoloOld;
	d->wSoloOld = set_union(d->wSolo, d->wSoloOld);
	set_drain(d->wSolo);
	set_free(temp);

	/* FIXME: why is this double???? */
// 	temp = d->wSoloOld;
// 	d->wSoloOld = set_union(d->wSolo, d->wSoloOld);
// 	set_drain(d->wSolo);
// 	set_free(temp);
}

void recursiveDFA(aststmt t, dfa_t d)
{
	autoScopeSets as2;
	parNode *p = isInAutoList(t->l);

	if (p != NULL)
		as2 = p->a;
	else
	{
		as2 = dfa_analyse(t);
		//insertToAutoList( as2, t->l );
	}

	accessSet(as2.autoPrivate, DFA_PRIVATE, d);
	accessSet(as2.autoFirstPrivate, DFA_FIRSTPRIVATE, d);
	accessSet(as2.autoReduction, DFA_REDUCTION, d);
	accessSet(as2.autoShared, DFA_SHARED, d);
}

void dfa_handleEndOfConstruct(dfa_t d)
{
	transferInfo(d);
	symListEraseCurrent(d);
	d->dirCase = DFA_FREE;

}

void dfa_handeEndOfFor(dfa_t d)
{
	transferPFCI(d);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     AST TRAVERSAL                                             *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void ident_c(astexpr tree, void *state, int vistime)
{
	dfa_t d = (dfa_t) state;
	
	if (vistime != PREVISIT) return;
	
//	if (d->func)
//	{
//		int tempId = d->fId;
//		d->fId = find_func_id(mySym);
//		if (d->fId >= 0 && !set_get(d->funcSet, mySym))
//		{
//			set_put_unique(d->funcSet, mySym);
//			d->func = 0;
//			inFunc = 1;
//			ast_stmt_dfa(find_func(mySym, dfa_funcListStart), d);
//			inFunc = 0;
//			set_drain(d->local);
//		}
//		d->fId = tempId;
//	}
// 	else
		if (d->isReference)
		{
			set_put_unique(d->unkSet, mySym);
			d->isReference = 0;
		}
		else
		{
			if (!justPassingBy)
			{
				makePFCIchecks(mySym, d);

				/* If it's allready scoped, or user didn't ask for an autoscope,
				 * there's no reason to analyse
				 */
				if (!isScoped(tree, d) && !set_get(d->notAutoSet, mySym)
				    && !set_get(d->ignoreSet, mySym) && !set_get(d->local, mySym))
				{
					if (d->foundNested > 0)
					{
						int type = d->dirCase;
						d->dirCase = DFA_FREE;

						if (d->readWrite)
							force(DFA_REDUCTION, mySym, type, d);
						else
							if (d->writing)
								force(DFA_PRIVATE, mySym, type, d);
							else
								force(DFA_FIRSTPRIVATE, mySym, type, d);

						d->dirCase = type;
					}
					else
					{
						/* In order for something to be scoped as "Reduction",
						 * it can't appear in a simple read or a write
						 */
						if (set_get(d->reductionSet, mySym) && !d->readWrite)
						{
							set_remove(d->reductionSet, mySym);
							set_put_unique(d->R, mySym);
						}

						checkForDataRace(mySym, d);
						adjustRwOrder(mySym, d);
						set_put_unique(d->allVarsSet, mySym);
					}
				}
				d->readWrite = 0;
				d->writing = 0;
			}
			else
				if (!d->func && !inFunc)
					set_put_unique(d->allVarsSet, mySym);
		}
		if (d->func)   /* VVD - the only function call case */
			d->func = 0;
}


static void constval_c(astexpr t, void *state, int vistime)
{
	if (vistime == PREVISIT)
	{
		dfa_t d = (dfa_t) state;
		d->readWrite = 0;
		d->writing = 0;
	}
}

static void funccall_c(astexpr t, void *state, int vistime)
{
	if (vistime == PREVISIT)
		if (t->left->type == IDENT) /* We do not handle more complex stuff */
			((dfa_t) state)->func = 1;   /* d->func = 0; will occur at INDENT time */
}


static void arrayidx_c(astexpr t, void *state, int vistime)
{
	dfa_t d = (dfa_t) state;
	
	if (vistime == PREVISIT)
		if (t->right->type == IDENT && d->isInParallelFor)
			if (set_get(d->parIndexSet, t->right->u.sym))
				d->dirCase = DFA_PFCI;
}


static void uop_c(astexpr t, void *state, int vistime)
{
	if (vistime == PREVISIT)
		if (t->opid == UOP_addr)
			((dfa_t) state)->isReference = 1;
}

static void preop_c(astexpr t, void *state, int vistime)
{
	dfa_t d = (dfa_t) state;

	if (vistime == PREVISIT)
		d->readWrite = 1;
	if (vistime == POSTVISIT)
		d->readWrite = 0;
}

static void postop_c(astexpr t, void *state, int vistime)
{
	dfa_t d = (dfa_t) state;

	if (vistime == PREVISIT)
		d->readWrite = 1;
	if (vistime == POSTVISIT)
		d->readWrite = 0;
}

static void ass_c(astexpr t, void *state, int vistime)
{
	dfa_t d = (dfa_t) state;

	if (vistime == PREVISIT)
	{
		if (t->opid == ASS_eq)
			d->writing = 1;
		else
			d->readWrite = 1;
	}
	if (vistime == MIDVISIT)
		d->readWrite = 0;
}


static void dident_c(astdecl t, void *state, int vistime)
{
	dfa_t d = (dfa_t) state;

	if (vistime != PREVISIT) return;
	if (d->isAutoVar && d->foundNested == 0)
	{
		set_put_unique(d->autoSet, t->u.id);
		addToSymList(t->u.id, d);
	}
	else
		if (inPriv)
		{
			force(DFA_PRIVATE, t->u.id, d->dirCase, d);
			set_put_unique(d->ignoreSet, t->u.id);
		}
		else
			if (inFp)
			{
				force(DFA_FIRSTPRIVATE, t->u.id, d->dirCase, d);
				set_put_unique(d->ignoreSet, t->u.id);
			}
			else
				if (inLp)
				{
					force(DFA_LASTPRIVATE, t->u.id, d->dirCase, d);
					set_put_unique(d->ignoreSet, t->u.id);
				}
				else
					if (inRd)
					{
						force(DFA_REDUCTION, t->u.id, d->dirCase, d);
						set_put_unique(d->ignoreSet, t->u.id);
					};

	if (!d->isAutoVar && d->foundNested == 0)
	{
		if (inFunc)
			set_put_unique(d->local, t->u.id);
		else
		{
			set_put_unique(d->notAutoSet, t->u.id);
			set_remove(d->autoSet, t->u.id);
		}
	}
}


static void ompclvars_c(ompclause t, void *state, int vistime)
{
	dfa_t d = (dfa_t) state;
	
	if (vistime == PREVISIT)
	{
		if (d->foundNested > 0)
		{
			if (t->type == OCCOPYIN || t->type == OCPRIVATE)
				inPriv = 1;
			if (t->type == OCFIRSTPRIVATE)
				inFp = 1;
			if (t->type == OCLASTPRIVATE)
				inLp = 1;
			if (t->type == OCREDUCTION)
				inRd = 1;
		}
		if (t->type == OCAUTO)
			d->isAutoVar = 1;
	}
	if (vistime == POSTVISIT)
	{
		if (t->type == OCCOPYIN || t->type == OCPRIVATE)
			inPriv = 0;
		if (t->type == OCFIRSTPRIVATE)
			inFp = 0;
		if (t->type == OCLASTPRIVATE)
			inLp = 0;
		if (t->type == OCREDUCTION)
			inRd = 0;
		if (t->type == OCAUTO)
			d->isAutoVar = 0;
	}
}

static void ompclplain_c(ompclause t, void *state, int vistime)
{
	dfa_t d = (dfa_t) state;

	if (vistime != PREVISIT) return;

	if (t->type == OCNOWAIT)
		d->noWait = 1;
	if (t->type == OCDEFAULT)
		if (t->subtype == OC_auto && !justPassingBy && d->foundNested == 0)
		{
			autoScopeSets as2;
			justPassingBy = 1;
			as2 = dfa_analyse(t->parent->parent->parent);
			justPassingBy = 0;
			d->autoSet = set_difference(set_union(d->autoSet, as2.autoAllVars),
																	d->notAutoSet);
			accessSet(d->autoSet, DFA_DEFAUTO, d);
		};
}


static void ompdircrit_c(ompdir t, void *state, int vistime)
{
	if (vistime == PREVISIT)
		((dfa_t) state)->dirCase = DFA_CRITICAL;
}

static void ompdirrest_c(ompdir t, void *state, int vistime)
{
	dfa_t d = (dfa_t) state;
	
	if (vistime == PREVISIT)
	{
		switch (t->type)
		{
			case DCSINGLE:
				d->dirCase = DFA_SINGLE;
				break;
			case DCFOR:
				d->dirCase = DFA_FREE;
				d->parForIndex = NULL;
				d->isInParallelFor = 1;
				break;
			// TODO: where is DCSECTIONS?????
			case DCMASTER:
				d->dirCase = DFA_MASTER;
				break;
			case DCATOMIC:
				d->dirCase = DFA_ATOMIC;
				break;
			case DCBARRIER:
				dfa_handleBarrier(d);
				break;
			case DCPARALLEL:
				d->foundNested++;
				if (d->foundNested > 0 && !justPassingBy)
					recursiveDFA(t->parent->parent, d);
				break;
		}
		if (t->clauses)
			d->clauses = 1;
	}
	if (vistime == POSTVISIT)
		d->clauses = 0;
}


static void ompconall_c(ompcon t, void *state, int vistime)
{
	dfa_t d = (dfa_t) state;

	if (vistime == POSTVISIT && t->body)
		switch (t->directive->type)
		{
			case DCSINGLE:
				dfa_handleSoloEnd(d);
				dfa_handleEndOfConstruct(d);
				if (!d->noWait)
					dfa_handleBarrier(d);
				else
					d->noWait = 0;
				break;
			case DCCRITICAL:
			case DCATOMIC:
				dfa_handleEndOfConstruct(d);
				break;
			case DCMASTER:
				dfa_handleSoloEnd(d);
				dfa_handleEndOfConstruct(d);
				break;
			case DCFOR:
				dfa_handleEndOfConstruct(d);
				dfa_handeEndOfFor(d);
				set_remove(d->parIndexSet, d->parForIndex);
				if (!d->noWait)
					dfa_handleBarrier(d);
				else
					d->noWait = 0;
				d->isInParallelFor = 0;
				break;
			// TODO: where is DCSECTIONS?????
			case DCPARALLEL:
				d->foundNested--;
				set_drain(d->ignoreSet);
		};
}


int isScoped(astexpr tree, dfa_t d)
{
	return ((set_get(d->R, mySym) || set_get(d->W, mySym) || set_get(d->WR, mySym)
	         || set_get(d->RW, mySym)) && set_get(d->dataRaceSet, mySym));
}

void initialiseDfaSets(dfa_t d)
{
	d->unkSet       = set_new(dfa);
	d->dataRaceSet  = set_new(dfa);
	d->reductionSet = set_new(dfa);
	d->wSolo        = set_new(dfa);
	d->wSoloOld     = set_new(dfa);
	d->allVarsSet   = set_new(dfa);
	d->R            = set_new(dfa);
	d->W            = set_new(dfa);
	d->RW           = set_new(dfa);
	d->WR           = set_new(dfa);
	d->autoSet      = set_new(dfa);
	d->ignoreSet    = set_new(dfa);
	d->parIndexSet  = set_new(dfa);
	d->notAutoSet   = set_new(dfa);
	d->local        = set_new(dfa);
	d->funcSet      = set_new(dfa);
}


void dfa_traversal(aststmt tree, dfa_t d)
{
	travopts_t dfaopts;
	
	travopts_init_noop(&dfaopts);
	dfaopts.when = PREPOSTVISIT | MIDVISIT;
	dfaopts.starg = d;
	
	dfaopts.exprc.ident_c = ident_c;
	dfaopts.exprc.constval_c = constval_c;
	dfaopts.exprc.funccall_c = funccall_c;
	dfaopts.exprc.arrayidx_c = arrayidx_c;
	dfaopts.exprc.uop_c = uop_c;
	dfaopts.exprc.preop_c = preop_c;
	dfaopts.exprc.postop_c = postop_c;
	dfaopts.exprc.ass_c = ass_c;

	dfaopts.declc.dident_c = dident_c;

	//TODO There is probably some kind of bug here; Aggelos did not
	// recurse on IF, NUMTHREADS, but he DID recurse on SCHEDULE's expr.
	// I believe the latter is correct, so, for the moment, I do not prune.
	dfaopts.ompclausec.ompclvars_c = ompclvars_c;
	dfaopts.ompclausec.ompclplain_c = ompclplain_c;

	dfaopts.ompdcc.ompdircrit_c = ompdircrit_c;
	dfaopts.ompdcc.ompdirrest_c = ompdirrest_c;
	dfaopts.ompdcc.ompconall_c = ompconall_c;

	ast_stmt_traverse(tree, &dfaopts);
}


autoScopeSets dfa_analyse(aststmt tree)
{
	static int    isMain = 0;
	autoScopeSets as;
	struct _dfa_params d1 = {0, 0, 0, 0, 0, 0, 0, -1, 0,
		       NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		       NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		       NULL, DFA_FREE
	};
	dfa_t d = &d1;

	if (isMain == 0)                // VVD {
	{
		parListStart = NULL;
		isMain = 1;
	}                               // VVD }
	isMain++;
//	if (isMain == 1)              // ORIGINAL
//		parListStart = NULL;

	initialiseDfaSets(d);

	dfa_traversal(tree, d);

	d->unkSet = set_union(d->unkSet, set_union(set_intersection(d->WR, d->dataRaceSet),
	                                         set_intersection(d->RW, d->dataRaceSet)));
	as.autoUnk = d->unkSet;
	as.autoPrivate = (set_difference(set_intersection(d->dataRaceSet, set_union(d->W,
	                                                 d->wSoloOld)), d->unkSet));
	as.autoFirstPrivate = set_difference(set_intersection(d->dataRaceSet,
	                                                     set_union(d->R, d->wSoloOld)), d->unkSet);
	as.autoReduction = d->reductionSet;
	as.autoAllVars = d->allVarsSet;
	as.line = tree->l;
	as.autoShared = set_difference(d->autoSet, d->dataRaceSet);
	as.autoAuto = d->autoSet;

	if (!justPassingBy)
	{
		insertToAutoList(as, tree->l);
		if (!set_isempty(d->autoSet) && showdbginfo)
			show_autoscope_results(as);
	}

	isMain--;

	insertToAutoList(as, tree->l);
	return as;
}


static void show_set(char *msg, set(dfa) a)
{
	setelem(dfa) e;

	if (set_isempty(a)) 
		return;

	fprintf(stderr, "\t%s\t  { ", msg);
	for (e = a->first; e; e = e->next)
		fprintf(stderr, "%s%s", e->key->name, (e->next) ? ", " : " }\n");
}


void show_autoscope_results(autoScopeSets as)
{
	fprintf(stderr, "=== Autoscoping #parallel (@line %d) { \n", as.line);
	show_set("Scoped as shared:\n", as.autoShared);
	show_set("Scoped as private:\n", as.autoPrivate);
  show_set("Scoped as firstprivate:\n", as.autoFirstPrivate);
	show_set("Scoped as reduction:\n", as.autoReduction);
	show_set("\tUnable to scope:\n", as.autoUnk);
	if (!set_isempty(as.autoAuto))
		fprintf(stderr, "=== } (%d autoscoped) ==========\n", set_size(as.autoAuto));
	fprintf(stderr, "\n");
}
