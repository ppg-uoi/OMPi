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

/* ast_xform.h  */

#ifndef __AST_XFORM_H__
#define __AST_XFORM_H__

#include "ast.h"
#include "stddefs.h"
#include "symtab.h"

extern void ast_xform(aststmt *tree);
extern void ast_expr_xform(astexpr *t);
extern void ast_stmt_xform(aststmt *t);
extern void ast_decl_xform(astdecl *t);
extern void ast_spec_xform(astspec *t);
extern void ast_ompclause_xform(ompclause *t);
extern void ast_ompdir_xform(ompdir *t);
extern void ast_ompcon_xform(ompcon *t);
extern void ast_omp_xform(aststmt *t);

/**
 * Reproduces only the Declarator part of the original declaration; the
 * special thing about this is that it takes care of arrays with missing
 * size for the 1st dimension.
 * 
 * @param e  The symtab entry for the var whose declaratοr we want to reproduce
 * @return   A declaration node (astdecl)
 */
extern astdecl xform_clone_declonly(stentry e);

/* Produces an identical declaration which may optionally have a new var name,
 * an initializer and/or convert the variable into a pointer
 */
extern aststmt xform_clone_declaration(symbol s, astexpr initer,
                                       bool mkpointer, symbol snew);

/* Reproduces the original declaration of a function */
extern aststmt xform_clone_funcdecl(symbol funcname);

/* Call a function with no args */
#define Call0_expr(s) FunctionCall(Identifier(Symbol(s)),NULL)
#define Call0_stmt(s) Expression(Call0_expr(s))

/* Was a macro before, we turned it into function to incorporate cancel logic */
extern aststmt BarrierCall();

/* Holds the scope level of the most recently encountered
 * parallel construct
 */
extern int closest_parallel_scope;

/* Used in ast_declare_varlist_vars for determining if a variable appears in a
 * target data clause in order to set isindevenv to true.
 */
extern int target_data_scope;

/* Used in outline.c for setting isindevenv of functions */
extern int inDeclTarget;

/* This one checks whether an openmp construct should include an
 * implicit barrier.
 */
extern int xform_implicit_barrier_is_needed(ompcon t);

/* This simply prints the directive into a string and encloses it
 * in comments, adding also the source line number.
 * It returns a verbatim node with the comment.
 */
extern aststmt ompdir_commented(ompdir d);

#endif
