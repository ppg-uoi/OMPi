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

/* ast_vars.h -- declare & analyze vars
 *
 *   gtp = global threadprivate
 *   sgl = shared (non-threadprivate) global
 *   sng = shared non-global
 */

#ifndef __AST_VARS_H__
#define __AST_VARS_H__

#include "ast.h"
#include "set.h"
#include "symtab.h"

/* A set used in _analyze functions to analyze vars used in a block.
 * 
 * Explanation of the struct fields:
 * clause: The OpenMP clause that set the attributes of the variable. A value 
 *         of OCNOCLAUSE forces the variable to be implicitly determined.
 * clsubt: The clause subtype (e.g. map type, reduction operator etc)
 * clmod:  The clause modifier (v45, e.g. always)
 * ptr:    Unfortunately, it is unavoidable to use the plain set "vars" in some 
 *         cases where the "xlitems" set (below) should be utilized. For such 
 *         cases, ptr stores a pointer to the actual xlitem.
 */
struct varstruct {
	ompclt_e    clause;    /* If OCNOCLAUSE => implicit variable */
	ompclsubt_e clsubt;    /* E.g. map type, reduction operator etc */
	ompclmod_e  clmod;     /* E.g. always (v45) */
	void       *ptr;       /* General pointer; see explanation above */
};
SET_TYPE_DEFINE(vars, symbol, struct varstruct, 1031)

/* A set used in _analyze functions for extended list items (array sections) 
 * 
 * Explanation of the struct fields:
 * clause: The OpenMP clause it came from (or OCNOCLAUSE).
 * clsubt: The clause subtype (e.g. map type, reduction operator etc)
 * clmod:  The clause modifier (v45, e.g. always)
 * xl:     The actual xlitem
 */
struct xlistruct {
	ompclt_e    clause;    /* E.g. map. reduction etc. (or OCNOCLAUSE) */
	ompclsubt_e clsubt;    /* E.g. map type, reduction operator etc */
	ompclmod_e  clmod;     /* E.g. always (v45) */
	ompxli      xl;        /* The actual xlitemt */
};
SET_TYPE_DEFINE(xlitems, symbol, struct xlistruct, 1031)

/* Explanation of the values of "vars" set:
 * ival: Used to remember variable attributes (e.g. the subtype of the clause
 *       a variable was found in).
 * ptr:  Unfortunately, it is unavoidable to use the plain set "vars" in some 
 *       cases where the "xlitems" set should be utilized. For such cases, 
 *       ptr stores a pointer to the actual xlitem.
 */

/* Declare all variables appearing in a declaration list */
extern void ast_declare_decllist_vars(astspec s, astdecl d);
/* Declare all vars in a varlist (i.e. in an OpenMP data clause) */
extern void ast_declare_varlist_vars(astdecl d, ompclt_e type,ompclsubt_e subtp);
/* Declare all variables in an OpenMP extended list (REDUCTION/MAP clause) */
extern void ast_declare_xlist_vars(ompclause t);
/* Declare function parameters; d must be the declarator of FUNCDEF */
extern void ast_declare_function_params(astdecl d);

/* Given a stentry: */
#define istp(e) ((e)->isthrpriv)
#define isgtp(e) (istp(e) && (e)->scopelevel == 0)
#define issgl(e) (!istp(e) && (e)->scopelevel == 0)
#define isextern(e) (speclist_getspec((e)->spec, STCLASSSPEC, SPEC_extern))

/* Different variable types encountered by outline */
typedef enum
{
	DCT_UNSPECIFIED = 0, DCT_BYREF, DCT_PRIVATE, DCT_BYVALUE, DCT_BYRESULT,
	DCT_BYVALRES, DCT_REDUCTION, 
	DCT_MAPALLOC, DCT_MAPTO, DCT_MAPFROM, DCT_MAPTOFROM, DCT_ZLAS,
	DCT_DDENV, DCT_IGNORE,
	DCT_SIZE /* SIZE has to be last */
} vartype_t;

/* How are the discovered variables found */
#define FOUND_INCLAUSE   1
#define FOUND_IMPLICITLY 2

/* Utilities */
extern astdecl ast_set2varlist(set(vars) s);
extern void    ast_varlist2set(astdecl d, set(vars) s);

/* Functions that find/change variables in the code
 */
extern set(vars)  analyze_find_gtp_vars(aststmt t);
extern void       analyze_pointerize_sgl(aststmt t);
extern void       analyze_pointerize_vars(aststmt t, set(vars) vars);
extern void       analyze_pointerize_decltarg_varsonly(aststmt t);
extern void       analyze_pointerize_decltarg_varsfuncs(aststmt t);
extern void       analyze_rename_vars(aststmt, set(vars), char *);
extern set(vars) *analyze_used_vars(aststmt t);
#endif
