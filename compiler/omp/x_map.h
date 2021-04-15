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

/* x_map.h -- mapping-related code */

#ifndef XMAP_H
#define XMAP_H

#include "ast.h"
#include "set.h"
#include "ast_vars.h"

#define UPDATE_DISABLE 0
#define UPDATE_NORMAL  1
#define UPDATE_FORCED  2
#define REFER_NORMAL   4
#define REFER_DELETE   8 


typedef aststmt (*muper_t)(setelem(vars) origvar, astexpr device, int update);
typedef aststmt (*muperx_t)(setelem(xlitems) origvar, astexpr device, int upd);


/* Produces a statement to map a #target or a #target data variable */
extern aststmt xm_map_structured(setelem(vars) e, astexpr ignore, int update);
/* Produces a statement to map a #target declare link or exit data variable */
extern aststmt xm_map_unstructured(setelem(vars) e, astexpr devexpr, int update);

/* Produces a statement to unmap a #target or a #target data variable. */
extern aststmt xm_unmap_structured(setelem(vars) e, astexpr ignore, int how);
/* Produces a statement to unmap a #target declare link or exit data variable */
extern aststmt xm_unmap_unstructured(setelem(vars) e, astexpr devexpr, int how);

/* Iterates over a set of variables "s" and creates (un)mapping calls through
 * function "func". The generated code is returned through "stmt".
 */
extern int xm_mup_set(set(vars) s, aststmt *stmt, 
             astexpr devexpr, muper_t func, int updatehow, char *comment);

/* Produces a statement to map a #target declare link or enter data variable */
extern aststmt xm_map_xlitem(setelem(xlitems) e, astexpr devexpr, int how);

/* Produces a statement to unmap a #target declare link or enter data var. */
extern aststmt xm_unmap_xlitem(setelem(xlitems) e, astexpr devexpr, int how);

/* Iterates over a set of xlitems "s" and creates mapping calls through
 * function "func". The generated code is returned through "stmt".
 */
extern int xm_mup_xliset(set(xlitems) s, aststmt *stmt, 
             astexpr devexpr, muperx_t func, int updatehow, char *comment);

/* Generates code for mapping and unmapping any used declare link variables.
 * Called when transforming #target and #targetdata.
 */
extern void xm_linkvars_mappings(astexpr devxpr, ompcon con, set(vars) implicit,
              aststmt *maps, aststmt *unmaps);

/* Generates code for mapping and unmapping any used variables.
 * Called when transforming #target and #targetdata.
 */
extern int xm_usedvars_mappings(set(vars) *usedvars, 
              aststmt *maps, aststmt *unmaps);

#endif  /* XMAP_H */
