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

/* x_reduction.h -- everything related to openmp reduction clauses */

#ifndef __X_REDUCTION_H__
#define __X_REDUCTION_H__

#include "symtab.h"
#include "ast.h"


/* Correct initializer depending on the reduction operator */
extern astexpr red_scalar_initializer(ompclsubt_e op, symbol var);
/* Generaes correct array initializers for array/pointer based reductions */
extern aststmt red_array_initializer(int op, stentry e, ompxli xl);
/* Generates reduction code for an item */
extern aststmt red_generate_code(ompclsubt_e op, ompxli orivar, astexpr redvar);
/* Code for all reduction clauses of the directive */
extern aststmt red_generate_code_from_ompdir(ompdir t);
/* Memory filling list of statements for all reduction arrays */
extern aststmt red_array_initializers_from_ompdir(ompdir t);
/* Privatize a pointer-based array section */
extern aststmt red_privatize_ptr2arr(symbol var, ompxli xlitem, symbol st);
/* Produces a decl/init statement for a reduction var and its auxiliary vars */
extern aststmt red_generate_declaration(symbol var, int redop, ompxli xl);
/* Produces (if applicable) a statement that frees  1 reduction var (PBASs) */
extern aststmt red_generate_deallocation(ompxli var);
/* Statements for possible deallocation related to reductions */
extern aststmt red_generate_deallocations_from_ompdir(ompdir t);
/* Replaces any non-constant parameters by variables (destructive) and returns
 * a list of declaration statements for those variables.
 */
extern aststmt red_arrayexpr_simplify(ompdir t);

#endif /* __X_REDUCTION_H__ */
