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

/* x_arrays.c -- Utilities for arrays and array sections */

#include <assert.h>
#include "ast_free.h"
#include "ast_copy.h"
#include "ompi.h"
#include "x_arith.h"
#include "x_arrays.h"


/**
 * Gets/sets the #elements of a given array dimension from the array declaration
 *
 * @param arr       The root DINIT/DECLARATOR/DARRAY node
 * @param dimidx    The dimension (starting from 0)
 * @param newexpr   If non-NULL, this replaces the existing dimension size 
 *                  as-is (no copy made)
 * @return          An expression with the (final) size of the dimension; if
 *                  asked for dimension 0, and the size is missing, it 
 *                  returns the cardinality of the initializer.
 */
astexpr arr_dimension_size(astdecl arr, int dimidx, astexpr newexpr)
{
	int     i; 
	astdecl d = arr, tmp;
	
	if (d->type == DINIT)
		d = d->decl;
	while (d->type == DECLARATOR || d->type == DPAREN)   
		d = d->decl;
	assert(d->type == DARRAY);
	
	/* Count the dimensions (the chain is reverse) */
	for (i = 0, tmp = d; tmp->type == DARRAY; i++)
		tmp = tmp->decl;
  if (i <= dimidx || dimidx < 0)      /* caller is way off */
		return (NULL);
	
	/* Walk the required dimensions */
	for (i = i - dimidx -1; i > 0; i--)
		d = d->decl;
	
	if (newexpr)                        /* replace size */
	{
		if (d->u.expr) 
			ast_expr_free(d->u.expr);
		return ( d->u.expr = newexpr );
	}
	
	/* Get the size from the initializer */
	if (dimidx == 0 && d->u.expr == NULL && arr->type == DINIT)
		/* FIXME: memory leak here since a copy will be made later on ... */
		return ( numConstant( decl_initializer_cardinality(arr) ) );
	return (d->u.expr);
}


/**
 * Gets the number of elements of an array, from dimension dimidx on.
 * Pass dimidx=0 to count all array elememts.
 *
 * @param arr       The root DINIT/DECLARATOR/DARRAY node
 * @param dimidx    The starting dimension (numbering starts from 0)
 * @return          An expression with the number of elements.
 */
astexpr arr_num_elems(astdecl arr, int dimidx)
{
	int     i; 
	astdecl d = arr, tmp;
	astexpr nelems = NULL, nd;
	
	if (d->type == DINIT)
		d = d->decl;
	while (d->type == DECLARATOR || d->type == DPAREN)   
		d = d->decl;
	assert(d->type == DARRAY);

	/* Count the dimensions (the chain is reverse) */
	for (i = 0, tmp = d; tmp->type == DARRAY; i++)
		tmp = tmp->decl;
  if (dimidx >= i || dimidx < 0)      /* caller is way off */
		return (NULL);
	
	/* Walk the required dimensions */
	for (--i; i >= dimidx; i--)
	{
		/* Get the size from the initializer */
		if (i == 0 && d->u.expr == NULL && arr->type == DINIT)
			nd = numConstant( decl_initializer_cardinality(arr) );
		else
			nd = ast_expr_copy(d->u.expr);
		nelems = nelems ? BinaryOperator(BOP_mul, nelems, Parenthesis(nd)) : nd;
		d = d->decl;
	}
	return (nelems);
}


/**
 * Returns an expression with the base element of an array section
 * @param arrsec  The xlitem (assumed to be an array section)
 * @param base    If non-null, this will be the base (instead of the xlitem id)
 * @return        The expression
 */
astexpr arr_section_baseelement(ompxli arrsec, astexpr base)
{
	omparrdim d;
	
	if (!base)
		base = Identifier(arrsec->id);
	for (d = arrsec->dim; d; d = d->next)
		base = ArrayIndex(base, ast_expr_copy(d->lb));
	return ( Parenthesis(base) );
}


/**
 * Returns an expression with the offset of the base element of an array section
 * wrt to the beginning of the array. Theoretically this offset is a number that
 * is independent of the name of the array but because pointers may be involved,
 * we do not have all the necessary dimensionality information to calculate it.
 * Thus we resort to just subtracting the arr_section_baseelement from the 
 * beginning of the array. However, because we may need the same offset for
 * some other array, we optionally allow any other identifier to be used.
 * The offset is in bytes (not elements).
 * 
 * @param arrsec The xlitem (assumed to be an array section)
 * @param wrt    Any identifier w.r.t. which the expression will be formed.
 *               If NULL, the xlitem name is used.
 * @return       The expression (#bytes)
 */
astexpr arr_section_offset(ompxli arrsec, symbol arrid)
{
	astexpr   e = Identifier(arrid ? arrid : arrsec->id);
	omparrdim d;
	int       err, n, zero = 1;
	
	assert(arrsec->dim != NULL);
	
	/* Just a single dimension */
	if (arrsec->dim->next == NULL)
		return BinaryOperator(BOP_mul, 
		         Parenthesis(ast_expr_copy(arrsec->dim->lb)),
		         Sizeof(ArrayIndex(e, ast_expr_copy(arrsec->dim->lb))));
	
	for (d = arrsec->dim; d; d = d->next)
	{
		e = ArrayIndex(e, ast_expr_copy(d->lb));
		/* try to keep it minimal for simple cases */
		if (!xar_expr_is_constant(d->lb))
			zero = 0;
		else
		{
			n = xar_calc_int_expr(d->lb, &err);
			if (!err && n != 0)
				zero = 0;
		}
	}
	
	/* ( (void*) &var[lb][lb]..[lb] - (void*) var ) */
	return Parenthesis(
	         BinaryOperator(BOP_sub,
	           CastVoidStar( UOAddress(e) ), 
	           CastVoidStar( Identifier(arrid ? arrid : arrsec->id) ) 
	         )
	       );
}


/**
 * The expression that gives the total number of ELEMENTS in an array section.
 * @param arrsec    the xlitem (assumed to be an array section)
 * @param whichdims which dimensions to include in the calculation; use the
 *                  macros UPTO(n), ALLDIMS or JUST(n).
 * @return an expression with the total size of the array section (#elememts)
 */
astexpr arr_section_length(ompxli arrsec, int whichdims)
{
	omparrdim dim;
	int       i, n, isptr = 0, err, isconst = true;
	astexpr   length = NULL, dimlen;
	stentry   e = symtab_get(stab, arrsec->id, IDNAME);
	
	if (!arrsec->dim)
		return NULL;

	if (!(e->isarray))
	{
		isptr = decl_ispointer(e->decl);
		if (!isptr)
			exit_error(1, "(%s, line %d): cannot determine section size for "
		             "non-array '%s'.\n",
		             arrsec->file->name, arrsec->l, arrsec->id->name);
	}

	for (dim = arrsec->dim, i = 0; dim; dim = dim->next, i++)
	{
		if (whichdims < 0 && whichdims != -i)  /* wait till a specific dim is met */
			continue;
		if (dim->len)        /* length given */
			dimlen = ast_expr_copy(dim->len);
		else
		{
			if (i == 0 && isptr)
				exit_error(1, "(%s, line %d): need explicit section length for pointer-"
				           "to-array '%s'.\n",
				           arrsec->file->name, arrsec->l, arrsec->id->name);
			/* If it is a pointer (to array) then subtract 1 from the dim index */
			dimlen = arr_dimension_size(e->idecl ? e->idecl : e->decl, i-isptr, NULL);
			if (!dimlen)
				exit_error(1, "(%s, line %d): cannot determine length for "
				           "dimension %d of array '%s'.\n",
				           arrsec->file->name, arrsec->l, i-isptr, arrsec->id->name);
			if (!xar_expr_is_zero(dim->lb))
				dimlen = BinaryOperator(BOP_sub, 
				                        dimlen, Parenthesis(ast_expr_copy(dim->lb)));
		}
		if (!xar_expr_is_constant(dimlen))
			isconst = false;
		else
		{
			n = xar_calc_int_expr(dimlen, &err);
			if (!err && n == 0 && whichdims >= 0)
			{
				if (length) ast_expr_free(length);
				return ( numConstant(0) );
			}
		}
		dimlen = Parenthesis(dimlen);
		length = length ? BinaryOperator(BOP_mul, length, dimlen) : dimlen;
		if (whichdims < 0 || i == whichdims)
			break;
	}
	
	if (isconst)
	{
		n = xar_calc_int_expr(length, &err);
		if (!err)
			return ( numConstant(n) );
	}
	return (length);
}


/**
 * The expression that gives the total number of BYTES in an array section.
 * @param arrsec  the xlitem (assumed to be an array section)
 * @return an expression with the total size of the array section (#elememts)
 */
astexpr arr_section_size(ompxli arrsec)
{
	return ( BinaryOperator(BOP_mul,
	                        arr_section_length(arrsec, ALLDIMS),
	                        Sizeof(arr_section_baseelement(arrsec, NULL))) );
}


/* An array section is in contiguous memory only if all dimensions,
 * except possibly the first one, use their full size. I.e. if arrsec is
 * A[a:al][b:bl][c:cl]..., then it represents contiguous memory only when
 * bl = size of dimension 2, cl = size of dimenstion 3, etc.
 * Because we may not be able to prove or disprove the above, this function 
 * conservatively returns false only if the section is contiguous with 100% 
 * certainty. In any other case, it returns true, with the following tip: 
 * a positive return value means that it is 100% sure the section is 
 * non-contiguous. A negative value means that we cannot be sure.
 */
int arrsec_is_noncontiguous(ompxli arrsec)
{
	omparrdim d;
	stentry   e = symtab_get(stab, arrsec->id, IDNAME);
	int       dimid, v, err, isptr=0;
	
	if (arrsec->xlitype != OXLI_ARRSEC) 
		return 0;               /* 100% contiguous */
	assert(arrsec->dim != NULL);
	if (!(e->isarray)) 
		isptr = decl_ispointer(e->decl);
	
	for (d = arrsec->dim->next, dimid = 1; d; d = d->next, dimid++)
	{
		if (!d->len) continue;  /* yep, the whole dimension */
		v = xar_calc_int_expr(d->len, &err);
		if (err) return -1;     /* Inconclusive */
		/* If it is a pointer (to array) then subtract 1 from the dim index */
		v -= xar_calc_int_expr(
		       arr_dimension_size(e->idecl ? e->idecl : e->decl, dimid-isptr, NULL), 
		       &err);
		if (err) return -1;
		if (v != 0) return 1;   /* 100% noncontiguous */
	}
	return 0;                 /* It has to be contiguous */
}


/**
 * [ Handle non-constant array section parameters ]
 * 
 * The following functions cope with arrsec parameters (offsets, lengths)
 * which involve non-constant expressions. The idea is that all such
 * parameters be replaced by identifiers that are previously assigned the
 * expression results. 
 * 
 * For example, consider the array section: 
 *   sec[a:b+1][3:c-1] 
 * These functions modify the array section to look like: 
 *   sec[pre_lb0:pre_sz0][3:pre_sz1]
 * (where pre_ is some given prefix) while the following assignments preceed:
 *   pre_lb0 = a;
 *   pre_sz0 = b+1;
 *   pre_sz1 = c-1;
 * 
 * The identifiers are be either variables or structure fields.
 * 
 */


static char *_ARRSEC_LB_FMT = "%s_lb%d_", *_ARRSEC_SIZE_FMT = "%s_sz%d_";


static omparrdim arrdim_copy_replace(omparrdim d, symbol st, char *pre, int ctr)
{
	omparrdim new = NULL;
	
	if (d)
	{
		astexpr   lb, len;
		char      s[128];   
		
		if (d->lb == NULL)
			d->lb = NULL;
		else
			if (xar_expr_is_constant(d->lb))
				lb = ast_expr_copy(d->lb);
			else
			{
				snprintf(s, 127, _ARRSEC_LB_FMT, pre, ctr);
				lb = (st) ? PtrField(Identifier(st), Symbol(s)) : IdentName(s);
			};
		if (d->len == NULL)
			len = NULL;
		else
			if (xar_expr_is_constant(d->len))
				len = ast_expr_copy(d->len);
			else
			{
				snprintf(s, 127, _ARRSEC_SIZE_FMT, pre, ctr);
				len = (st) ? PtrField(Identifier(st), Symbol(s)) : IdentName(s);
			};
		new = OmpArrDim(lb, len);
		new->next = arrdim_copy_replace(d->next, st, pre, ctr+1);
	}
	return (new);
}


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
ompxli arr_section_replace_params(ompxli arrsec, symbol st, char *pre)
{
	if (arrsec->xlitype == OXLI_IDENT)
		return PlainXLI(arrsec->id);
	else
		return ArraySection(arrsec->id, arrdim_copy_replace(arrsec->dim,st,pre,0));
}


static 
astdecl arrdim_params_fields(omparrdim d, char *pre, int ctr)
{
	astdecl flist = NULL, tmp;
	char s[128];   
	
	if (!d) return (NULL);
	
	if (d->lb && !xar_expr_is_constant(d->lb))
	{
		snprintf(s, 127, _ARRSEC_LB_FMT, pre, ctr);
		tmp = StructfieldDecl( Declspec(SPEC_int), 
		                       Declarator(NULL, IdentifierDecl(Symbol(s))));
		flist = flist ? StructfieldList(flist, tmp) : tmp;
	}
	if (d->len && !xar_expr_is_constant(d->len))
	{
		snprintf(s, 127, _ARRSEC_SIZE_FMT, pre, ctr);
		tmp = StructfieldDecl( Declspec(SPEC_int), 
		                       Declarator(NULL, IdentifierDecl(Symbol(s))));
		flist = flist ? StructfieldList(flist, tmp) : tmp;
	}
	if ((tmp = arrdim_params_fields(d->next, pre, ctr+1)) != NULL)
		flist = flist ? StructfieldList(flist, tmp) : tmp;
	return (flist);
}


static 
aststmt arrdim_params_fieldinits(omparrdim d, symbol st, char *pre, int ctr)
{
	aststmt alist = NULL, tmp;
	char s[128];   
	
	if (!d) return (NULL);
	if (d->lb && !xar_expr_is_constant(d->lb))
	{
		snprintf(s, 127, _ARRSEC_LB_FMT, pre, ctr);
		tmp = AssignStmt(DotField(Identifier(st), Symbol(s)), ast_expr_copy(d->lb));
		alist = alist ? BlockList(alist, tmp) : tmp;
	}
	if (d->len && !xar_expr_is_constant(d->len))
	{
		snprintf(s, 127, _ARRSEC_SIZE_FMT, pre, ctr);
		tmp = AssignStmt(DotField(Identifier(st), Symbol(s)),ast_expr_copy(d->len));
		alist = alist ? BlockList(alist, tmp) : tmp;
	}
	if ((tmp = arrdim_params_fieldinits(d->next, st, pre, ctr+1)) != NULL)
		alist = alist ? BlockList(alist, tmp) : tmp;
	return (alist);
}


/**
 * This one creates struct fields out of the non-constant arrsec parameters
 * along with their initializer statements.
 * @param arrsec the array section 
 * @param st     the struct
 * @param pre    the prefix
 */
void arr_section_params_fields_inits(ompxli arrsec, symbol st, char *pre,
                                     astdecl *fields, aststmt *finits)
{
	if (arrsec->xlitype == OXLI_IDENT)
	{
		*fields = NULL; 
		*finits = NULL;
	}
	else
	{
		*fields = arrdim_params_fields(arrsec->dim, pre, 0);
		*finits = arrdim_params_fieldinits(arrsec->dim, st, pre, 0);
	}
}


static 
aststmt arrdim_params_inidecls(omparrdim d, char *pre, int ctr)
{
	aststmt alist = NULL, tmp;
	char s[128];   
	
	if (!d) return (NULL);
	if (d->lb && !xar_expr_is_constant(d->lb))
	{
		snprintf(s, 127, _ARRSEC_LB_FMT, pre, ctr);
		tmp = Declaration(
		        Declspec(SPEC_int), 
		        InitDecl(
		          Declarator(NULL, IdentifierDecl(Symbol(s))),
		          ast_expr_copy(d->lb)
		        )
		      );
		alist = alist ? BlockList(alist, tmp) : tmp;
	}
	if (d->len && !xar_expr_is_constant(d->len))
	{
		snprintf(s, 127, _ARRSEC_SIZE_FMT, pre, ctr);
		tmp = Declaration(
		        Declspec(SPEC_int), 
		        InitDecl(
		          Declarator(NULL, IdentifierDecl(Symbol(s))),
		          ast_expr_copy(d->len)
		        )
		      );
		alist = alist ? BlockList(alist, tmp) : tmp;
	}
	if ((tmp = arrdim_params_inidecls(d->next, pre, ctr+1)) != NULL)
		alist = alist ? BlockList(alist, tmp) : tmp;
	return (alist);
}


/**
 * This one creates new variable declarations, initialized from the 
 * non-constant arrsec parameters.
 * @param arrsec the array section 
 * @param pre    the prefix (normally, the variable name)
 * @return       the list of declaration statements
 */
aststmt arr_section_params_varinits(ompxli arrsec, char *pre)
{
	return (arrsec->xlitype == OXLI_IDENT) ? 
	           NULL : arrdim_params_inidecls(arrsec->dim, pre, 0);
}
