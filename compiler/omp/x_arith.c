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

/* x_arith.c -- some expression calculations */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include "symtab.h"
#include "x_arith.h"


#define IsOct(c) ((c) >= '0' && (c) <= '7')
#define IsHex(c) (((c) >= '0' && (c) <= '9') || ((c) >= 'a' && (c) <= 'f') \
                  || ((c) >= 'A' && (c) <= 'F'))


static int atoi_oct(char *s)
{
	int num = 0;

	for (; *s != 0; s++)
		num = (num << 3) + (*s - '0');
	return (num);
}


static int atoi_hex(char *s)
{
	int num = 0;

	for (; *s != 0; s++)
		if (isdigit(*s))
			num = (num << 4) + (*s - '0');
		else
			if (*s >= 'a' && *s <= 'f')
				num = (num << 4) + (*s - 'a' + 10);
			else
				num = (num << 4) + (*s - 'A' + 10);
	return (num);
}


/* Calculate the value of a number (integer or floating point)
 * Returns -1  on error, 1 for integer and 2 for float.
 */
static
int str2number(char *numstr, int *intval, double *fltval)
{
	int  isflp = 0, seemsoct = 0, isoct = 0, ishex = 0;
	char *s = numstr;

	if (!isdigit(*s) && *s != '.')
		return (-1);

	if (*s == '0')      /* Check for oct/hex */
	{
		s++;
		if (isdigit(*s))
			for (seemsoct = isoct = 1; isdigit(*s) && *s; s++)
			{
				if (!IsOct(*s))
					isoct = 0;
			}
		else
			if (*s == 'x' || *s == 'X')
				for (s++, ishex = 1; IsHex(*s) && *s; s++)
					;
	}
	else    /* Decimal */
		for (; isdigit(*s) && *s; s++)
			;

	if (*s == '.')         /* float */
		for (s++, isflp = 1; ((ishex && IsHex(*s)) || (!ishex && isdigit(*s)))
		     && *s; s++)
			;

	if (*s == 'e' || *s == 'E' ||  *s == 'p' || *s == 'P')  /* An e/p-number */
	{
		if (ishex && (*s == 'e' || *s == 'E')) return (-1);
		if (!ishex && (*s == 'p' || *s == 'P')) return (-1);
		isflp = 1;
		s++;
		if (*s == '-' || *s == '+')    /* Expect a sign */
			s++;
		for (; isdigit(*s) && *s; s++)
			;
	}

	if (*s) return (-1);

	if (isflp)
	{
		if (fltval) *fltval = atof(numstr);
		return (2);
	}
	else
	{
		if (seemsoct && !isoct) return (-1);
		if (intval)
		{
			if (isoct)
				*intval = atoi_oct(numstr + 1); /* Skip 1st '0' */
			else
				if (ishex)
					*intval = atoi_hex(numstr + 1);
				else
					*intval = atoi(numstr);
		}
		return (1);
	}
}


int xar_expr_is_zero(astexpr tree)
{
	int n = 1, err = 1;
	
	if (xar_expr_is_constant(tree))
		n = xar_calc_int_expr(tree, &err);
	return (!err && n == 0);
}


int xar_expr_is_constant(astexpr tree)
{
	if (!tree) return 0;
	
	switch (tree->type)
	{
		case IDENT:
			return (0);
		case CONSTVAL:
			return (1);
		case STRING:
			return (1);
		case FUNCCALL:
		case ARRAYIDX:
		case DOTFIELD:
		case PTRFIELD:
		case BRACEDINIT:
			return (0);
		case CASTEXPR:
			return (xar_expr_is_constant(tree->left));
		case CONDEXPR:
			return (xar_expr_is_constant(tree->u.cond) &&
			        xar_expr_is_constant(tree->left) &&
			        xar_expr_is_constant(tree->right));
		case UOP:
			switch (tree->opid)
			{
				case UOP_addr:
				case UOP_star:
				case UOP_sizeof:
				case UOP_typetrick:
				case UOP_sizeoftype:
					return (0);
				case UOP_neg:
				case UOP_bnot:
				case UOP_lnot:
				case UOP_paren:
					return (xar_expr_is_constant(tree->left));
				default:
					return (0);
			}
		case BOP:
			/* Here we could check if any of the operands is 0/1/etc */
			/* But this would be incorrect since the other operant may contain
			 * an assignment expression */
			return (xar_expr_is_constant(tree->left) &&
			        xar_expr_is_constant(tree->right));
		case PREOP:
		case POSTOP:
		case ASS:
		case DESIGNATED:
		case IDXDES:
		case DOTDES:
		case COMMALIST:
		case SPACELIST:
			return (0);
		default:
			fprintf(stderr, "[xar_expr_is_constant]: b u g !!\n");
	}
	return (0);
}


/* The difference with the one above is that, some simple expressions 
 * are counted as giving constant value, even if they involve non-constant
 * terms. E.g. 0*(x++), which is a non-constant expression with side effects,
 * gives always a result of 0. This involves calculations, of course...
 */
int xar_expr_has_constant_value(astexpr tree)
{
	if (!tree) return 0;
	
	switch (tree->type)
	{
		case IDENT:
			return (0);
		case CONSTVAL:
			return (1);
		case STRING:
			return (1);
		case FUNCCALL:
		case ARRAYIDX:
		case DOTFIELD:
		case PTRFIELD:
		case BRACEDINIT:
			return (0);
		case CASTEXPR:
			return ( xar_expr_has_constant_value(tree->left) );
		case CONDEXPR:
		{
			int err, cond = xar_calc_int_expr(tree->u.cond, &err);
			if (err) 
				return 0;
			else
				return (cond ? xar_expr_has_constant_value(tree->left) :
				               xar_expr_has_constant_value(tree->right));
		}
		case UOP:
			switch (tree->opid)
			{
				case UOP_addr:
				case UOP_star:
				case UOP_sizeof:
				case UOP_typetrick:
				case UOP_sizeoftype:
					return (0);
				case UOP_neg:
				case UOP_bnot:
				case UOP_lnot:
				case UOP_paren:
					return (xar_expr_has_constant_value(tree->left));
				default:
					return (0);
			}
		case BOP:
		{
			int err1, err2;
			int t1 = xar_calc_int_expr(tree->left, &err1),
				  t2 = xar_calc_int_expr(tree->right, &err2);

			switch (tree->opid)
			{
				case BOP_land:
					/* Check if any of the two is false */
					return ((!err1 && !t1) || (!err2 && !t2)) ? 1 : (!err1 && !err2);
				case BOP_lor:
					/* Check if any of the two is true */
					return ((!err1 && t1) || (!err2 && t2)) ? 1 : (!err1 && !err2);
				case BOP_mul:
					/* Check if any of the two is zero */
					return ((!err1 && t1==0) || (!err2 && t2==0)) ? 1 : (!err1 && !err2);
				case BOP_div:
				case BOP_mod:
					/* Check if the first one is zero (we loose here if 0/0) */
					return (!err1 && t1 == 0) ? 1 : (!err1 && !err2);
				default:
					return (xar_expr_has_constant_value(tree->left) &&
					        xar_expr_has_constant_value(tree->right));
			}
		}
		case PREOP:
		case POSTOP:
		case ASS:
		case DESIGNATED:
		case IDXDES:
		case DOTDES:
		case COMMALIST:
		case SPACELIST:
			return (0);
		default:
			fprintf(stderr, "[xar_expr_has_constant_value]: b u g !!\n");
	}	printf("1\n");

	return (0);
}


int xar_calc_int_expr(astexpr tree, int *error)
{
	if (!tree) { *error = 1; return 0; }
	
	switch (tree->type)
	{
		case CONSTVAL:
		{
			int v;
			*error = (str2number(tree->u.str, &v, NULL) != 1);
			return (v);
		}
		case CASTEXPR:
			return (xar_calc_int_expr(tree->left, error));
		case CONDEXPR:
		{
			int cond = xar_calc_int_expr(tree->u.cond, error);
			
			if (*error) return 0;
			return (cond ?
			        xar_calc_int_expr(tree->left, error) :
			        xar_calc_int_expr(tree->right, error));
		}
		case UOP:
			switch (tree->opid)
			{
				case UOP_neg:
					return (-xar_calc_int_expr(tree->left, error));
				case UOP_bnot:
					return (~xar_calc_int_expr(tree->left, error));
				case UOP_lnot:
					return (!xar_calc_int_expr(tree->left, error));
				case UOP_paren:
					return (xar_calc_int_expr(tree->left, error));
				case UOP_addr:
				case UOP_star:
				case UOP_sizeof:
				case UOP_typetrick:
				case UOP_sizeoftype:
				default:
					*error = 1;
					return (0);
			}
		case BOP:
		{
			int err1, err2;
			int t1 = xar_calc_int_expr(tree->left, &err1),
			    t2 = xar_calc_int_expr(tree->right, &err2);
			
			*error = err1 || err2;
			switch (tree->opid)
			{
				case BOP_shl:
					return (t1 << t2);
				case BOP_shr:
					return (t1 >> t2);
				case BOP_leq:
					return (t1 <= t2);
				case BOP_geq:
					return (t1 >= t2);
				case BOP_eqeq:
					return (t1 == t2);
				case BOP_neq:
					return (t1 != t2);
				case BOP_land:
					/* If any of the two is false, return false */
					if ((err1 && !err2 && !t2) || (!err1 && err2 && !t1))
					{
						*error = 0;
						return 0;
					}
					return (t1 && t2);
				case BOP_lor:
					/* If any of the two is true, return true */
					if ((err1 && !err2 && t2) || (!err1 && err2 && t1))
					{
						*error = 0;
						return 1;
					}
					return (t1 || t2);
				case BOP_band:
					return (t1 & t2);
				case BOP_bor:
					return (t1 | t2);
				case BOP_xor:
					return (t1 ^ t2);
				case BOP_add:
					return (t1 + t2);
				case BOP_sub:
					return (t1 - t2);
				case BOP_lt:
					return (t1 < t2);
				case BOP_gt:
					return (t1 > t2);
				case BOP_mul:
					/* If any of the two is zero, return zero */
					if ((err1 && !err2 && t2 == 0) || (!err1 && err2 && t1 == 0))
					{
						*error = 0;
						return 0;
					}
					return (t1 * t2);
				case BOP_div:
					/* If t1 is zero, return zero (well, we don't cover 0/0) */
					if (!err1 && t1 == 0)
					{
						*error = 0;
						return 0;
					}
					return (t1 / t2);
				case BOP_mod:
					/* If t1 is zero, return zero */
					if (!err1 && t1 == 0)
					{
						*error = 0;
						return 0;
					}
					return (t1 % t2);
				case BOP_cast:
				default:
					*error = 1;
					return (0);
			}
		}
		case PREOP:
		case POSTOP:
		case IDENT:
		case STRING:
		case FUNCCALL:
		case ARRAYIDX:
		case DOTFIELD:
		case PTRFIELD:
		case BRACEDINIT:
		case ASS:
		case DESIGNATED:
		case IDXDES:
		case DOTDES:
		case COMMALIST:
		case SPACELIST:
		default:
			*error = 1;
			return (0);
	}
}
