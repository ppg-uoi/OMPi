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

/* x_target.h */

#ifndef __X_TARGET_H__
#define __X_TARGET_H__

#include "ast.h"
#include "stddefs.h"
#include "x_cars.h"

/* Values of the ->isindevenv stentry field (when non-FALSE).
 * They represent the reason the symbol is in the device environments
 */
#define due2DECLTARG 1  /* symbol appeared in a declare target didrective */
#define due2TARGDATA 2  /* symbol appeared in a target data directive */

/* The variable name holding the current device ("__ompi_devID") */
extern char *currdevvarName;

/* For GPU kernels */
#define DEVQUAL "__DEVQLFR"
#define DEVSPEC "__DEVSPEC"
#define DEVSPECQUAL "__DEVQLFR __DEVSPEC"

/* A tree holding the current target code */
#define inTarget() (targtree != NULL)
extern aststmt targtree, targnewglobals;
extern int     targetnum;

extern void xform_target(aststmt *t, targstats_t *ts);
extern void xform_targetdata(aststmt *t);
extern void xform_targetupdate(aststmt *t);
extern void xform_targetenterdata(aststmt *t);
extern void xform_targetexitdata(aststmt *t);
extern aststmt get_denv_var_decl(bool createNew);
extern symbol targstruct_offsetname(symbol var);

extern void produce_target_files();
extern void produce_decl_var_code();

#endif
