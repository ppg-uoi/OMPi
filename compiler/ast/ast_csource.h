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

/* ast_csource.h */

#ifndef __AST_CSOURCE_H__
#define __AST_CSOURCE_H__

#include "ast.h"

extern void ast_expr_csource(astexpr tree);
extern void ast_stmt_csource(aststmt stmt);
extern void ast_decl_csource(astdecl tree);
extern void ast_spec_csource(astspec tree);
extern void ast_ompcon_csource(ompcon tree);
extern void ast_ompdir_csource(ompdir t);
extern void ast_ompclause_csource(ompclause t);

extern void ast_csource(aststmt tree);
extern void ast_csource_stderr(aststmt tree);
extern void ast_spec_csource_stderr(astspec tree);
extern void ast_decl_csource_stderr(astdecl tree);
extern void ast_expr_csource_stderr(astexpr tree);

/* OMPi-extensions
 */
extern void ast_oxclause_csource(oxclause t);
extern void ast_oxdir_csource(oxdir tree);
extern void ast_oxcon_csource(oxcon tree);

#endif
