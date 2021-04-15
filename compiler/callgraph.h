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

/* callgraph.h */

#ifndef __CALLGRAPH_H__
#define __CALLGRAPH_H__

#include "ast.h"
#include "ast_traverse.h"
#include "set.h"

/* In the following struct:
 * - nint is the number of internal functions defined in the user code
 * - next is the number of external functions called from the user code
 * - cgmat[][] is an [nint][nint+next] matrix; if cgmat[i][j] is 1, then 
 *   function i calls function j
 * - funcs[] contains the function symbols  
 */
typedef struct {
	int    nint, next;   /* # internal and external functions */
	symbol *funcs;       /* their symbols */
	char   **cgmat;      /* the callgraph matrix */
} cg_t;

/* Set type for (key: symbol, value: dontcare) */
SET_TYPE_DEFINE(cgfun, symbol, char, 93)

/* This is the first function that should be called; it is a prerequisite
 * for the others to work. 
 */
extern void cg_find_defined_funcs(aststmt tree);

/* t can be any statement */
extern cg_t *cg_build_call_graph(aststmt t);
extern set(cgfun) cg_find_called_funcs(aststmt t);

extern void cg_call_graph_test(aststmt wholetree);
extern void cg_defuncs_debug();

#endif
