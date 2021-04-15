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

/* x_arrays.h -- Utilities for arrays and array sections */

#ifndef __X_ARRAYS_H__
#define __X_ARRAYS_H__

#include <limits.h>
#include "ast.h"

/**
 * Gets/sets the size of a given array dimension from the array declaration. 
 *
 * @param arr       The root DINIT/DECLARATOR/DARRAY node
 * @param dimidx    The dimension (starting from 0)
 * @param newexpr   If non-NULL, this replaces the existing dimension size 
 *                  as-is (no copy made)
 * @return          An expression with the (final) size of the dimension; if
 *                  asked for dimension 0, and the size is missing, it 
 *                  returns the cardinality of the initializer.
 */
extern astexpr arr_dimension_size(astdecl arr, int dimidx, astexpr newexpr);

/**
 * Gets the number of elements of an array, from dimension dimidx on.
 * Pass dimidx=0 to count all array elememts.
 *
 * @param arr       The root DINIT/DECLARATOR/DARRAY node
 * @param dimidx    The starting dimension (numbering starts from 0)
 * @return          An expression with the number of elements.
 */
extern astexpr arr_num_elems(astdecl arr, int dimidx);

/**
 * Returns an expression with the base element of an array section
 * @param arrsec  The xlitem (assumed to be an array section)
 * @param base    If non-null, this will be the base (instead of the xlitem id)
 * @return        The expression
 */
extern astexpr arr_section_baseelement(ompxli arrsec, astexpr base);

#define arr_section_baseaddress(t) UOAddress(arr_section_baseelement(t,NULL))

extern astexpr arr_section_offset(ompxli arrsec, symbol arrid);

/**
 * The expression that gives the total number of ELEMENTS in an array section.
 * @param arrsec    the xlitem (assumed to be an array section)
 * @param whichdims which dimensions to include in the calculation; use the
 *                  macros UPTO(n), ALLDIMS or JUST(n).
 * @return an expression with the total size of the array section (#elememts)
 */
#define UPTO(n) (n)  
#define ALLDIMS UPTO(INT_MAX)
#define JUST(n) (-(n))
extern astexpr arr_section_length(ompxli arrsec, int whichdims);
/**
 * The expression that gives the total number of BYTES in an array section.
 * @param  arrsec  the xlitem (assumed to be an array section)
 * @return an expression with the total size of the array section (#elememts)
 */
extern astexpr arr_section_size(ompxli arrsec);

/* It returns 0 (false) if the array section lies in contiguous memmory.
 * it returns a positive value if it is 100% sure memory is non-contiguous. 
 * It returns a negative value if we cannot be 100% sure.
 */
extern int arrsec_is_noncontiguous(ompxli arrsec);

/**
 * This one replaces all array section parameters which are not constant
 * expressions, with variables or struct fields of a given prefix. You can
 * get the variable or the struct field declaration statements by calling,
 * correspondingly, arr_section_params_varinits() or 
 * arr_section_params_fields_inits().
 * @param arrsec the array section 
 * @param st     the struct (if NULL, then replaces parameters by variables)
 * @param pre    the prefix for each field
 * @return       the new array section (freeable)
 */
extern ompxli arr_section_replace_params(ompxli arrsec, symbol st, char *pre);

/**
 * This one creates struct fields out of the non-constant arrsec parameters
 * along with their initializer statements.
 * @param arrsec the array section 
 * @param st     the struct
 * @param pre    the prefix
 * @param fields (return) the list of struct fields
 * @param finits (return) list of statements initializing the struct fields
 */
extern void arr_section_params_fields_inits(ompxli arrsec, symbol st, char *pre,
                                            astdecl *fields, aststmt *finits);

/**
 * This one creates new variable declarations, initialized from the 
 * non-constant arrsec parameters.
 * @param arrsec the array section 
 * @param pre    the prefix (normally, the variable name)
 * @return       the list of declaration statements
 */
extern aststmt arr_section_params_varinits(ompxli arrsec, char *pre);

#endif /* __X_ARRAYS_H__ */
