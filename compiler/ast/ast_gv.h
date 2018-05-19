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

/* ast_gviz.h */

#ifndef __AST_GV_H__
#define __AST_GV_H__

#include "ast.h"

extern void ast_expr_gviz(astexpr tree);
extern void ast_stmt_gviz(aststmt stmt);
extern void ast_decl_gviz(astdecl tree);
extern void ast_spec_gviz(astspec tree);
extern void ast_ompcon_gviz(ompcon tree);
extern void ast_ompdir_gviz(ompdir t);
extern void ast_ompclause_gviz(ompclause t);
extern void ast_oxclause_gviz(oxclause t);
extern void ast_oxdir_gviz(oxdir tree);
extern void ast_oxcon_gviz(oxcon tree);

extern void ast_gviz(aststmt tree);
extern void ast_gviz_stderr(aststmt tree);
extern void ast_spec_gviz_stderr(astspec tree);
extern void ast_decl_gviz_stderr(astdecl tree);
extern void ast_expr_gviz_stderr(astexpr tree);

/* Main interface
 */
extern void ast_expr_gviz_doc(astexpr tree, char *progstr);
extern void ast_stmt_gviz_doc(aststmt tree, char *progstr);

#endif
