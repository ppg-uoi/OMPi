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

typedef struct
{
	set(dfa) autoShared;
	set(dfa) autoPrivate;
	set(dfa) autoFirstPrivate;
	set(dfa) autoUnk;
	set(dfa) autoReduction;
	set(dfa) autoAllVars;
	set(dfa) autoAuto;
	int      line;
} autoshattr_t;

extern void dfa_userfunc_add(symbol f, aststmt fBody);
extern autoshattr_t dfa_analyse(aststmt tree);
extern autoshattr_t *dfa_parreg_get_results(int line);
extern void dfa_parreg_remove(int line);

#endif /* __DFA_H__ */
