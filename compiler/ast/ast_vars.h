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

/* A set used in analyze functions */
SET_TYPE_DEFINE(vars, symbol, int, 1031)

/* Declare all variables appearing in a declaration list */
extern void ast_declare_decllist_vars(astspec s, astdecl d);
/* Declare all vars in a varlist (i.e. in an OpenMP data clause) */
extern void ast_declare_varlist_vars(astdecl d,  enum clausetype type,
                                     enum clausesubt subtype);
/* Declare function parameters;
 * d must be the declarator of FUNCDEF */
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
	DCT_BYVALRES, DCT_REDUCTION, DCT_DDENV, DCT_IGNORE,
	DCT_SIZE /* SIZE has to be last */
} vartype_t;

/* Functions that find/change variables in the code
 */
extern set(vars)  analyze_find_gtp_vars(aststmt t);
extern void       analyze_pointerize_sgl(aststmt t);
extern void       analyze_pointerize_vars(aststmt t, set(vars) vars);
extern void       analyze_pointerize_declared_vars(aststmt t);
extern set(vars) *analyze_used_vars(aststmt t);
#endif
