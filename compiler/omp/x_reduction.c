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

/* x_reduction.c -- everything related to openmp reduction clauses */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast_free.h"
#include "ast_xform.h"
#include "ast_copy.h"
#include "ast_types.h"
#include "x_clauses.h"
#include "x_arrays.h"
#include "x_arith.h"
#include "ompi.h"
#include "builder.h"


static int  red_num = -1;
static char _rl[128];
#define redlock() _rl

static
void old_add_reduction()
{
	red_num++;
	sprintf(_rl, "_redlock%d", red_num);

	/* Add a global definition, too, for the lock (avoid omp_lock_t) */
	bld_globalvar_add(Declaration(Speclist_right(
	                           StClassSpec(SPEC_static),
	                           Declspec(SPEC_void)
	                         ),
	                         Declarator(
	                           Pointer(),
	                           IdentifierDecl(Symbol(_rl))
	                         )));
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     INITIALIZERS FOR SCALARS AND ARRAYS                       *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Correct initializer depending on the reduction operator */
astexpr red_scalar_initializer(ompclsubt_e op, symbol var)
{
	if (op == OC_min || op == OC_max)
	{
		stentry e = symtab_get(stab, var, IDNAME);

		switch (speclist_basetype(e->spec))
		{
			case UBOOL:
				return (numConstant(op == OC_min ? 1 : 0));
			case CHAR:
				if (speclist_sign(e->spec) == UNSIGNED)
				{
					if (op == OC_max)
						return (numConstant(0));
					needLimits = true;
					return (IdentName("UCHAR_MAX"));
				}
				else
				{
					needLimits = true;
					return (IdentName(op == OC_max ? "SCHAR_MIN" : "SCHAR_MAX"));
				}
			case INT:
				if (op == OC_max && speclist_sign(e->spec) == UNSIGNED)
					return (numConstant(0));
				else
					needLimits = true;
				switch (speclist_size(e->spec))
				{
					case SHORT:
						return (IdentName(op == OC_max ?
						                  "SHRT_MIN" :
						                  (speclist_sign(e->spec) == SIGNED ?
						                   "SHRT_MAX" : "USHRT_MAX")
						                 ));
					case LONG:
						return (IdentName(op == OC_max ?
						                  "LONG_MIN" :
						                  (speclist_sign(e->spec) == SIGNED ?
						                   "LONG_MAX" : "ULONG_MAX")
						                 ));
					case LONGLONG:
						return (IdentName(op == OC_max ?
						                  "LLONG_MIN" :
						                  (speclist_sign(e->spec) == SIGNED ?
						                   "LLONG_MAX" : "ULLONG_MAX")
						                 ));
					default:
						return (IdentName(op == OC_max ?
						                  "INT_MIN" :
						                  (speclist_sign(e->spec) == SIGNED ?
						                   "INT_MAX" : "UINT_MAX")
						                 ));
				}
			case FLOAT:
				needFloat = true;
				return (IdentName(op == OC_max ? "-FLT_MAX" : "FLT_MAX"));
			case DOUBLE:
				needFloat = true;
				return (IdentName(speclist_size(e->spec) == LONG ?
				                  (op == OC_max ? "-LDBL_MAX" : "LDBL_MAX") :
				                  (op == OC_max ? "-DBL_MAX" : "DBL_MAX")
				                 ));
			default:
				exit_error(1, "[xc_reduction_initializer]: !!BUG!! bad type ?!\n");
		}
	}
	if (op == OC_times || op == OC_land)
		return (numConstant(1));
	if (op == OC_band)
		return (UnaryOperator(UOP_bnot, numConstant(0)));
	return (numConstant(0));
}


/* Produces correct array initializers for array/pointer based reductions.
 * @param op the reduction operation
 * @param e  the original variable (from the symbol table)
 * @param xl the array section
 * @return the intialization statement or NULL if not an array section.
 */
aststmt red_array_initializer(int op, stentry e, ompxli xl)
{
	int isptr = decl_ispointer(e->decl);
	
	if (xl->xlitype == OXLI_IDENT && !e->isarray && !isptr)
		return (NULL);
	if (isptr && xl->xlitype != OXLI_ARRSEC)
		exit_error(1, "(%s, %d) OpenMP error:\n\t"
		          "zero-length pointer array section %d not allowed in reduction\n",
		          xl->file->name, xl->l, e->key->name);
		
	if (e->isarray && xl->xlitype != OXLI_ARRSEC)
		return xc_memfill( Identifier(e->key),
		                   arr_num_elems(e->decl, 0),
		                   Sizeof(Identifier(e->key)),
		                   red_scalar_initializer(op, e->key) );
	else
		return xc_memfill( xc_xlitem_baseaddress(xl),
		                   xc_xlitem_length(xl),    // TODO: check for zero-len
		                   BinaryOperator(BOP_mul, 
		                     xc_xlitem_length(xl), 
		                     Sizeof(arr_section_baseelement(xl, NULL))),
		                   red_scalar_initializer(op, e->key) );
}


static
aststmt redarray_initializations_from_xlist(ompxli xl, int op)
{
	aststmt list = NULL, st = NULL;
	stentry e;

	for (; xl; xl = xl->next)
	{
		e = symtab_get(stab, xl->id, IDNAME);
		
		if ((st = red_array_initializer(op, e, xl)) == NULL)
			continue;
		if (!list && st)
			list = verbit("/* Arrays initializations for reduction */");
		if (st)
			list = BlockList(list, st);
	}
	return (list);
}


static
aststmt redarray_initializations_from_clauses(ompclause t, ompdir d)
{
	aststmt list = NULL, st = NULL;

	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			list = redarray_initializations_from_clauses(t->u.list.next, d);
		t = t->u.list.elem;
		assert(t != NULL);
	}

	if (t->type == OCREDUCTION)
		(st = redarray_initializations_from_xlist(t->u.xlist, t->subtype))
		&& (list = ((list != NULL) ? BlockList(list, st) : st));
	return (list);
}


/* Memory copying statements for reduction array vars */
aststmt red_array_initializers_from_ompdir(ompdir t)
{
	return (t->clauses ? redarray_initializations_from_clauses(t->clauses, t)
	        : NULL);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *  DECLARATIONS REQUIRED FOR REDUCTION SUPPORT                  *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static
aststmt redarray_simplify_xlist(ompxli xl)
{
	aststmt list = NULL, st = NULL;
	ompxli  newx, tmpx;

	for (; xl; xl = xl->next)
	{
		if ((st = arr_section_params_varinits(xl, xl->id->name)) == NULL) /* vars */
			continue;
		
		/* replace xlitem with no mem leaks */
		tmpx = OmpXLItem(OXLI_ARRSEC, NULL, NULL);
		*tmpx = *xl;
		newx = arr_section_replace_params(xl, NULL, xl->id->name);
		newx->next = xl->next;
		newx->l = xl->l;
		newx->c = xl->c;
		newx->file = xl->file;
		*xl = *newx;
		free(newx);
		ast_ompxli_free(tmpx);
		
		list = list ? BlockList(list, st) : st;
	}
	return (list);
}


static
aststmt redarray_simplify_from_clauses(ompclause t, ompdir d)
{
	aststmt list = NULL, st = NULL;

	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			list = redarray_simplify_from_clauses(t->u.list.next, d);
		t = t->u.list.elem;
		assert(t != NULL);
	}

	if (t->type == OCREDUCTION)
		(st = redarray_simplify_xlist(t->u.xlist))
		&& (list = ((list != NULL) ? BlockList(list, st) : st));
	return (list);
}


/**
 * This replaces any non-constant parameters by variables, named with a
 * given prefix, in all array sections. In addition, it returns a list of
 * declaration statements for those variables.
 * (this is destructive; all affected array sections change irrevocably)
 * @param t   the OpenMP directive
 * @param pre the name prefix for the variables
 * @return    NULL or a list of declaration statments
 */
aststmt red_arrayexpr_simplify(ompdir t)
{
	return (t->clauses ? redarray_simplify_from_clauses(t->clauses, t)
	        : NULL);
}


static astexpr xlitem_dim0length(ompxli arrsec)
{
	assert(arrsec != NULL);
	if (arrsec->xlitype == OXLI_ARRSEC)
		return ( arr_section_length(arrsec, JUST(0)) );
	else     /* pointers?? */
	{
		stentry e = symtab_get(stab, arrsec->id, IDNAME);
		if (e->isarray)
			return ( arr_dimension_size(e->decl, 0, NULL) );
		else
			return ( numConstant(1) );
	}
}


/**
 * Consider reduction through a pointer-based array section. We need to 
 * privatize both the pointer and the array part it points to. This function
 * produces the privatization declaration as follows:
 *   < spec > vararr[size], var = vararr - offset;
 * where vararr is the local array part (based on the size of the section)
 * and var is the local pointer which points to vararr, shifted by the array 
 * section offset (according to OpenMP V4.5, p. 206, access to elements outside 
 * the array section is illegal).
 * 
 * @param var       The original variable (the pointer)
 * @param xlitem    The array section
 * @param st        If non-null, it is the struct of which var is a field
 * @return          A statement with the declaration
 */
aststmt red_privatize_ptr2arr(symbol var, ompxli xlitem, symbol st)
{
	astdecl id, localptr, localarr;
	stentry e = symtab_get(stab, var, IDNAME);
	astexpr localarrsize, base;
	char    localarrname[256];

	snprintf(localarrname, 255, "%s_local_", var->name);
	localarr = xform_clone_declonly(e);    /* Make the local array declarator */ 
	id = IdentifierDecl(Symbol(localarrname));
	*(decl_getidentifier(localarr)) = *id;
	free(id);
	
	if (xar_expr_is_constant(localarrsize = xlitem_dim0length(xlitem)))
		decl_ptr2arr(localarr, localarrsize);      /* Turn to array */
	else
	{
		base = st ? DerefParen(PtrField(Identifier(st), xlitem->id)) : NULL;
		localarr = InitDecl(                       /* Keep pointer, malloced */
		             localarr, 
		             FunctionCall(
		               IdentName("ort_memalloc"), 
		               BinaryOperator(BOP_mul, 
		                 localarrsize, 
		                 Sizeof(arr_section_baseelement(xlitem, base))
		               )
		             )
		           ); 
	}
	localptr = xform_clone_declonly(e);    /* Make the local var declarator */
	localptr = InitDecl(localptr,                      /* = arrvar - offset */
	             BinaryOperator(BOP_sub, 
	               CastVoidStar( IdentName(localarrname) ),
	               arr_section_offset(xlitem, Symbol(localarrname))));
	
	/* <spec> *varbak = &var, arrvar[size], var = <initializer>; */
	return Declaration(ast_spec_copy_nosc(e->spec), 
	                   DeclList(localarr, localptr));
}


/* Produces a statement that declares and initializes 1 reduction var
 * (plus another necessary one)
 */
aststmt red_generate_declaration(symbol var, int redop, ompxli xl)
{
	char    flvarname[256];
	symbol  flvar;
	stentry e = symtab_get(stab, var, IDNAME);
 
	if (e->isarray && oldReduction)
		exit_error(1, "OMPi error: old-style mode reduction variable "
		              "`%s' is non-scalar.\n", var->name);
	snprintf(flvarname, 255, "_red_%s", var->name);   /* a temp var _red_<name> */
	flvar = Symbol(flvarname);
	
	if (e->isarray)
		return ( flr_privatize(var, flvar, 1, NULL) );
	if (!decl_ispointer(e->decl))
		return ( flr_privatize(var, flvar, 1, red_scalar_initializer(redop, var)) );
		
	/* If we have a pointer, we need to declare a local array out of it:
	 *   <spec> *flvar = &var;
	 *   <spec> *arrvar = allocate(size), var = arrvar - offset;
	 */
	if (!xl || xl->xlitype == OXLI_IDENT)
		exit_error(1, "OpenMP error: size required in pointer-based reduction "
		              "on %d.\n", var->name);
	return 
		BlockList(
		  xform_clone_declaration(var, UOAddress(Identifier(var)), true, flvar),
		  red_privatize_ptr2arr(var, xl, NULL)
		);
}


/* Produces a statement that frees (if applicable) 1 reduction var;
 * It does (and returns non-NULL), only in cases of PBASs.
 * All this is duplication of effort as it could be immediately 
 * available when generating the declarations...
 */
aststmt red_generate_deallocation(ompxli var)
{
	stentry e = symtab_get(stab, var->id, IDNAME);
 
	if (e->isarray || !decl_ispointer(e->decl))
		return NULL;
	if (xar_expr_is_constant(xlitem_dim0length(var)))
		return NULL;
	else
	{
		char localarrname[256];
		
		snprintf(localarrname, 255, "%s_local_", var->id->name);
		return
			Expression(FunctionCall(IdentName("ort_memfree"),IdentName(localarrname)));
	}
}


/* Generates deallocation code for a list of variables/array sections.
 */
static
aststmt reduction_dealloc_from_xlist(ompxli xl)
{
	aststmt list = NULL, del;

	if (!xl) 
		return (NULL);
	for (; xl; xl = xl->next)
		if ((del = red_generate_deallocation(xl)) != NULL)
			list = list ? BlockList(list, del) : del;
	return (list);
}


static
aststmt reduction_dealloc_from_clauses(ompclause t)
{
	aststmt list = NULL, st = NULL;

	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			list = reduction_dealloc_from_clauses(t->u.list.next);
		t = t->u.list.elem;
		assert(t != NULL);
	}
	if (t->type == OCREDUCTION)
	{
		st = reduction_dealloc_from_xlist(t->u.xlist);
		list = ((list != NULL) ? BlockList(list, st) : st);
	}
	return (list);
}


/* Statements for possible deallocation related to reductions */
aststmt red_generate_deallocations_from_ompdir(ompdir t)
{
	if (oldReduction || t->clauses == NULL)
		return NULL;
	else
		return reduction_dealloc_from_clauses(t->clauses);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     GENERATION OF REDUCTION CODE                              *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Generates code for reduction of a variable.
 *   *(_red_var) op= var   or
 *   *(_red_var) = *(_red_var) op var   (for && and ||)
 *   if (*(_red_var) >(<) var) *(_red_var) = var (for min/max)
 * [ deprecated, old-style transformation; kept for reference ]
 * 
 * If arrays are involved, then a loop is output:
 *   {
 *     int n = <array section length>;
 *     <basetype> *gptr = xc_xlitem_baseaddress(orivar), 
 *                *lptr = BinaryOperator(BOP_add,
 *                 CastVoidStar(Deref(redvar)),arr_section_offset(orivar,NULL));
 * 
 *     for (--n; n >=0; --n) {
 *       <reduction code for *gptr and *lptr>
 *       gptr++; lptr++
 *     }
 *   }
 */
static
aststmt old_reduction_code(ompclsubt_e op, astexpr orivar, astexpr redvar)
{
	aststmt st = NULL;

	if (op == OC_min || op == OC_max)
		st = If(
		       BinaryOperator(
		         (op == OC_min) ?  BOP_gt : BOP_lt,
		         Deref(redvar),
		         orivar
		       ),
		       AssignStmt(
		         Deref(redvar),
		         orivar
		       ),
		       NULL
		     );
	else
		st = Expression(
		       Assignment(
		         Deref(redvar),

		         (op == OC_plus)  ? ASS_add :
		         (op == OC_minus) ? ASS_add :  /* indeed! */
		         (op == OC_times) ? ASS_mul :
		         (op == OC_band)  ? ASS_and :
		         (op == OC_bor)   ? ASS_or  :
		         (op == OC_xor)   ? ASS_xor : ASS_eq,

		         (op != OC_land && op != OC_lor) ?
		         orivar :
		         BinaryOperator(
		           (op == OC_land) ? BOP_land : BOP_lor,
		           Deref(redvar),
		           orivar
		         )
		       )
		     );

	return st;
}


/* Generates code for reductions:
 *   ort_reduce_<op>(&var, nelems, _red_var) // _red_var is a ptr
 */
static
aststmt reduction_code(ompclsubt_e op, ompxli orivar, astexpr redvar)
{
	aststmt st = NULL;
	char    funcname[256];
	astspec spec = symtab_get(stab, orivar->id, IDNAME)->spec;
	int     optype;
	
	/* Encode the operation type [ 0 - 12 ]: 
	 *   ints are from 0 to 7 (pure int is 0),
	 *   char is 8, double is 9 and float is 10;
	 *   short adds 1, long adds 2, longlong adds 3, unsigned adds 4.
	 */
	optype = speclist_basetype(spec) == CHAR   ?  8 :
	         speclist_basetype(spec) == DOUBLE ?  9 :
	         speclist_basetype(spec) == FLOAT  ? 10 : 0;  /* No UBOOL yet */
	optype += ( (speclist_size(spec) == SHORT)    ? 1 :
	            (speclist_size(spec) == LONG)     ? 2 :
	            (speclist_size(spec) == LONGLONG) ? 3 : 0 );
	if (speclist_sign(spec) == UNSIGNED)
		optype += 4;
	
	snprintf(funcname, 255, "ort_reduce_%s", 
	           op == OC_plus  ? "add" : 
	           op == OC_minus ? "subtract" :
	           op == OC_times ? "multiply" :
	           op == OC_band  ? "bitand" :
	           op == OC_bor   ? "bitor" :
	           op == OC_xor   ? "bitxor" : 
	           op == OC_land  ? "and" :
	           op == OC_lor   ? "or" :
	           op == OC_min   ? "min" :
	           op == OC_max   ? "max" :
	           "impossible");
	st = FuncCallStmt(
	       IdentName(funcname),
	       Comma4(
	         numConstant(optype),
	         orivar->xlitype == OXLI_ARRSEC ? 
	             xc_xlitem_baseaddress(orivar) : 
	             UOAddress(Identifier(orivar->id)),
	         orivar->xlitype == OXLI_ARRSEC ? 
	             BinaryOperator(BOP_add,
	               CastVoidStar(Deref(redvar)), arr_section_offset(orivar, NULL)
	             ) : 
	             redvar,
	         xc_xlitem_length(orivar)   // TODO: check for zero-len
	       )
	     );
	return st;
}


/* Generates reduction code for a given variable/array section. 
 * A NULL third argument updates with the default reduction variable (*_red_*).
 */
aststmt red_generate_code(ompclsubt_e op, ompxli orivar, astexpr redvar)
{
	if (redvar == NULL)   /* Use default update variable */
	{
		char flvar[256];
		snprintf(flvar, 255, "_red_%s", orivar->id->name);
		redvar = IdentName(flvar);
	}
	
	if (!oldReduction)
		return ( reduction_code(op, orivar, redvar) );
	
	/* old-style transformation */
	if (orivar->xlitype == OXLI_ARRSEC)
		exit_error(1, "(%s, line %d) OMPi error:\n\t"
		              "array section (%s) found while in old reduction mode\n",
		              orivar->file->name, orivar->l, orivar->id->name);
	return ( old_reduction_code(op, Identifier(orivar->id), redvar) );
}


/* Generates reduction code for a list of variables/array sections.
 */
static
aststmt reduction_code_from_xlist(ompxli xl, ompclsubt_e op)
{
	aststmt list = NULL;

	if (!xl) 
		return (NULL);
	list = red_generate_code(op, xl, NULL);
	for (xl = xl->next; xl; xl = xl->next)
		list = BlockList( list, red_generate_code(op, xl, NULL) );
	return (list);
}


static
aststmt reduction_code_from_clauses(ompclause t)
{
	aststmt list = NULL, st = NULL;

	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			list = reduction_code_from_clauses(t->u.list.next);
		t = t->u.list.elem;
		assert(t != NULL);
	}
	if (t->type == OCREDUCTION)
	{
		if (!oldReduction)
			st = reduction_code_from_xlist(t->u.xlist, t->subtype);
		else
		{
			/* Old-style transformation */
			old_add_reduction();   /* Track reductions; add global lock */
			st = Block3(
			       FuncCallStmt(
			         IdentName("ort_reduction_begin"),
			         UOAddress(IdentName(redlock()))
			       ),
			       reduction_code_from_xlist(t->u.xlist, t->subtype),
			       FuncCallStmt(
			         IdentName("ort_reduction_end"),
			         UOAddress(IdentName(redlock()))
			       )
			     );
		}
		list = ((list != NULL) ? BlockList(list, st) : st);
	}
	return (list);
}


/* Statements for reductions */
aststmt red_generate_code_from_ompdir(ompdir t)
{
	return (t->clauses ? reduction_code_from_clauses(t->clauses) : NULL);
}
