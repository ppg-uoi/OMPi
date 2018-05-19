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

/* x_target.h */

#ifndef __X_TARGET_H__
#define __X_TARGET_H__

#include "ast.h"
#include "boolean.h"

/* A tree holding the current target code
 */
extern aststmt targtree, targnewglobals;
extern int     targetnum;

extern void declaretarget_inject_ident(symbol s);
extern void produce_target_files();
extern void produce_decl_var_code();
extern void xform_target(aststmt *t);
extern void xform_targetdata(aststmt *t);
extern void xform_targetupdate(aststmt *t);
extern void xform_declaretarget(aststmt *t);
extern aststmt get_denv_var_decl(bool createNew);

#define inTarget() (targtree != NULL)
#endif
