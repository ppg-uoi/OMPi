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

/* x_clauses.h -- everything derived from openmp data clauses */

#ifndef __X_CLAUSES_H__
#define __X_CLAUSES_H__

#include "symtab.h"
#include "ast.h"
#include "stddefs.h"
#include "ast_vars.h"


/* 
 * Array sections / extendied list items
 */
/* Returns an expression with the base element of an xlitem */
extern astexpr xc_xlitem_baseelement(ompxli t);
/* Returns an expression with the base address of an xlitem */
#define xc_xlitem_baseaddress(t) UOAddress(xc_xlitem_baseelement(t))
/* The expression that gives the total number of elements in an array section */
extern astexpr xc_xlitem_length(ompxli arrsec);
/* Gives expressions for the basic quantities of an array section (xlitem) */
void xc_xlitem_copy_info(ompxli xl,
       astexpr *itemaddr, astexpr *nbytes, astexpr *addrlb);

/* Find if a variable is in a clause list and return the xlitem. */
extern ompxli  xc_xlitem_find_in_clause(ompclt_e, ompclause, symbol);

/*
 * Firstprivate
 */
extern aststmt xc_firstprivate_declaration(symbol var);
/* Memory copying list of statements for all fip vars (firstprivate) */
extern aststmt xc_ompdir_fiparray_initializers(ompdir t);

/*
 * Lastprivate
 */
/* Lastprivate assignments */
extern aststmt xc_ompdir_lastprivate_assignments(ompdir t);

/*
 * Declarations for all data clause vars
 */
extern aststmt xc_ompdir_declarations(ompdir t);

/*
 * Checks for duplicates & grouping
 */
/* Checks for duplicates AND keeps the list of vars; returns the table */
extern symtab xc_validate_store_dataclause_vars(ompdir d);
/* Only checks for duplicates */
extern void xc_validate_only_dataclause_vars(ompdir d);

/* The first one searches to see whether var appears in a type c
 * data clauses of d; returns 1 if yes, 0 if no.
 * The second one searches to find whether var appears in any of the
 * data clauses of d and, if found, it returns the clause type, else 0.
 */
extern int xc_isvar_in_dataclause(symbol var, ompdir d, ompclt_e c);
extern ompclt_e xc_dataclause_of_var(symbol var, ompdir d);

/*
 * Declarations from the vars collected by xc_validate_store_dataclause_vars().
 */
extern
aststmt xc_stored_vars_declarations(bool *haslast, bool *hasboth, bool *hasred);
/**
 * Creates a local variable with the same name (with an optional initializer), 
 * plus a copy of/pointer to the original one.
 */
extern
aststmt flr_privatize(symbol var, symbol varbak, int isptr, astexpr initer);

/* Take the clauses of a combined parallel-workshare statement and
 * split them; some are given to parallel and some to the workshare.
 * Parallel actually takes all the clauses it can take except for
 * private and firstprivate ones (so as to conform with the restrictions
 * on lastprivate clause that possibly exist)
 */
extern void xc_split_combined_clauses(ompclause all,
                                      ompclause *parc, ompclause *wshc);
/* Take the clauses of a V4.5 target construct and split them in two:
 * the ones that refer to the actual (V4.0) target and the ones that
 * refer to the implied target task.
 */
extern void xc_split_target_clauses(ompclause all, 
                                    ompclause *targc, ompclause *taskc);

/*
 * Various utilities
 */

/* Applies rules to ensure validity & OpenMP compliance of all clauses.
 * It won't check the variables inside the clauses.
 */
extern void xc_validate_clauses(enum dircontype dirtype, ompclause t);

/* Returns the leftmost/all clause(s) of the given type (or NULL) */
extern ompclause xc_clauselist_get_clause(ompclause t,ompclt_e type,char what);
/* Returns the leftmost clause of the given type (or NULL) */
extern ompclause xc_ompcon_get_clause(ompcon t, ompclt_e type);
/* Returns the clause of the given type (or NULL) and checks if it is unique */
extern ompclause xc_ompcon_get_unique_clause(ompcon t, ompclt_e type);
/* Returns a list of all clauses of the given (or NULL) -- freeable */
extern ompclause xc_ompcon_get_every_clause(ompcon t, ompclt_e type);

/* Finds all the variables that appear in varlist clauses of given (sub)type */
extern void xc_ompcon_get_vars(ompcon t, 
                ompclt_e type, ompclsubt_e subt, set(vars) s);

/* Finds all extended list items appearing in clauses of given (sub)type
 * (DEPEND/MAP/REDUCTION)
 */
extern void xc_ompcon_get_xlitems(ompcon t, 
         ompclt_e type, ompclsubt_e subt, set(xlitems) s);

/* Produce a memory copy / fill statement */
extern aststmt xc_memcopy(astexpr to, astexpr from, astexpr size);
extern aststmt xc_memfill(astexpr addr, astexpr nelems, astexpr elemsize, 
                          astexpr what);

#endif
