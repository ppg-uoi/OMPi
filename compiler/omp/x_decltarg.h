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

/* x_decltarg.h */

#ifndef __X_DECLTARG_H__
#define __X_DECLTARG_H__

#include "ast.h"
#include "stddefs.h"
#include "x_cars.h"

#define declvars_exist() (declare_variables && !set_isempty(declare_variables))
extern set(vars) declare_variables; /* Declared variables */
extern set(vars) declare_funcproto; /* Declared function prototypes */

extern aststmt structdecl;

extern symbol declstructVar, declstructArg, declstructType;

extern void decltarg_inject_newglobal(symbol s);
extern void xform_declaretarget(aststmt *t);
extern void xform_declaretarget_v45(aststmt *t);

extern void decltarg_find_all_directives(aststmt t);
extern void decltarg_add_calledfunc(symbol s);

extern bool decltarg_id_isknown(symbol s);
extern void decltarg_bind_id(stentry e);
extern ompclt_e decltarg_id_clause(symbol s);

extern void decltarg_struct_code(aststmt *initvars, aststmt *regstmts,
                                 aststmt *structinit);
extern aststmt decltarg_kernel_globals();
extern aststmt decltarg_kernel_struct_code();
extern aststmt decltarg_gpu_kernel_varinits();
extern astexpr decltarg_offload_arguments();
extern astdecl decltarg_gpu_kernel_parameters();

#endif /* __X_DECLTARG_H__ */
