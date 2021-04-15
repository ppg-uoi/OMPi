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

/* x_arith.h */

#ifndef __X_ARITH_H__
#define __X_ARITH_H__

#include "ast.h"

extern int xar_expr_is_zero(astexpr t);
extern int xar_expr_is_constant(astexpr t);
extern int xar_expr_has_constant_value(astexpr t);
extern int xar_calc_int_expr(astexpr t, int *error);  /* error is 1 */

#endif
