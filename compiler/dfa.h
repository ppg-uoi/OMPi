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

/* dfa.h */

#ifndef __DFA_H__
#define __DFA_H__

#include "ast.h"
#include "set.h"

/* Set type for DFA (key: symbol, value: dontcare) */
SET_TYPE_DEFINE(dfa, symbol, char, 389)

#define DFA_CRITICAL 0
#define DFA_MASTER   2
#define DFA_SINGLE   4
#define DFA_ATOMIC   6
#define DFA_FREE     8
#define DFA_PFCI     10

#define DFA_PRIVATE      0
#define DFA_FIRSTPRIVATE 1
#define DFA_SHARED       2
#define DFA_REDUCTION    3
#define DFA_LASTPRIVATE  4
#define DFA_DEFAUTO      5

#define NCASES 12

typedef struct _dfa_params *dfa_t;

typedef struct _autoScopeSets
{
	set(dfa) autoShared;
	set(dfa) autoPrivate;
	set(dfa) autoFirstPrivate;
	set(dfa) autoUnk;
	set(dfa) autoReduction;
	set(dfa) autoAllVars;
	set(dfa) autoAuto;
	int      line;
} autoScopeSets;

typedef struct _dfa_func
{
	aststmt funcBody;    /* For non-external functions (NULL otherwise) */
	symbol  funcName;
	struct _dfa_func *next;
} dfa_func_t;

extern dfa_func_t *dfa_funcListStart;
extern void addToFuncList(symbol f, aststmt fBody, dfa_func_t **);

typedef struct _parNode
{
	struct _parNode *next;
	int line;
	autoScopeSets a;
} parNode;

extern parNode *parListStart;

typedef struct _autoSym
{
	symbol s;
	int active[NCASES];
	int current[NCASES];
	struct _autoSym *next;
} autoSym;

extern void freeFuncList();

struct _dfa_params
{
	int      writing, func, readWrite, isReference,
	         noWait, isAutoVar, isInParallelFor, foundNested,
	         clauses;
	symbol   parForIndex;
	set(dfa) R, unkSet, reductionSet, dataRaceSet, ignoreSet, parIndexSet, local,
	         funcSet;
	set(dfa) wSolo, wSoloOld, rSolo, rSoloOld, allVarsSet, W, RW, WR, autoSet,
	         notAutoSet;
	autoSym *symListStart;
	int      dirCase;
};

extern aststmt find_func(symbol f, dfa_func_t *list);
extern parNode *isInAutoList(int l);
extern autoScopeSets dfa_analyse(aststmt tree);
extern void dfa_removeFromAutoList(int l);

#endif /* __DFA_H__ */
