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

/* builder.c -- bits and pieces needed for building the final code */

#ifndef __BUILDER_H__
#define __BUILDER_H__

#include "ompi.h"

/* Inject code @ top and bottom */
extern void    bld_head_add(aststmt s);
extern void    bld_tail_add(aststmt s);
extern stentry bld_globalvar_add(aststmt s);

extern void bld_outfuncs_add(symbol name, aststmt fd, aststmt curfunc);
extern void bld_outfuncs_xform();
extern void bld_outfuncs_place();

extern void bld_headtail_place(aststmt *tree);

extern void bld_ortinits_add(aststmt st);
extern void bld_autoinits_add(aststmt st);
extern void bld_ctors_build();

#endif  /* __BUILDER_H__ */
