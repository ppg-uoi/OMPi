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

/* ast_types.h */

#ifndef __AST_TYPES_H__
#define __AST_TYPES_H__

#include "ast.h"


typedef enum { NORMAL = 0, SHORT, LONG, LONGLONG } nt_size;
typedef enum { SIGNED = 0, UNSIGNED } nt_sign;
typedef enum { UBOOL = 0, CHAR, INT, FLOAT, DOUBLE } nt_basetype;

nt_size     speclist_size(astspec s);
nt_sign     speclist_sign(astspec s);
nt_basetype speclist_basetype(astspec s);

#endif
