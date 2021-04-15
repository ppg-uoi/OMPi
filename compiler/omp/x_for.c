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

/* x_for.c */

/*
 * 2020/01/15
 *   Added not-equal conditional operator as per OpenMP 5.0
 * 2019/10/27
 *   ort_num_iters() not needed any more; #iterations calculated directly
 *   in the output code.
 * 2015/04/09
 *   changed ort_num_iters from variadic
 * 2011/03/17:
 *   ort_leaving_for() not needed for parallel for constructs.
 * 2010/12/21:
 *   bug fix to avoid wrong test for lastprivate iteration.
 * 2010/11/20:
 *   bug fixes in: runtime schedule code, a decreasing step case,
 *                 lastprivate code for collape()d loops
 * 2010/11/06:
 *   major changes; support for COLLAPSE().
 * 2009/05/11:
 *   added AUTO schedule type - implemented as STATIC for now.
 */


#include <string.h>
#include <assert.h>
#include "stddefs.h"
#include "x_for.h"
#include "x_clauses.h"
#include "x_reduction.h"
#include "x_types.h"
#include "ast_xform.h"
#include "ast_free.h"
#include "ast_copy.h"
#include "ast_print.h"
#include "x_arith.h"
#include "str.h"
#include "ompi.h"

#define ITERCNT_SPECS \
        Speclist_right(Declspec(SPEC_unsigned), Declspec(SPEC_long))


/**
 * Get FOR loop nest indicies; their number is given by the number in the
 * ordered clause. This is a slimmed version of analyze_omp_for().
 * @param s  the body of the #for construct
 * @param orderednum the number in the ordered() clause (i.e. the nest depth)
 * @return an array of symbols (freeable)
 */
symbol *ompfor_get_indices(aststmt s, int orderednum)
{
	aststmt init, tmp = s;
	int     lid = 0;
	symbol  *vars = smalloc(orderednum*sizeof(symbol));
	
	do 
	{
		assert(s != NULL && s->type == ITERATION && s->subtype == SFOR);
		init = s->u.iteration.init;
		if (init == NULL)
		{
		OMPFOR_ERROR:
			exit_error(1, "(%s, line %d) openmp error:\n\t"
				"non-conformant FOR statement\n", s->file->name, s->l);
		}

		/* Get var from the init part of the FOR */
		if (init->type == EXPRESSION)     /* assignment: var = lb */
		{
			astexpr e = init->u.expr;
			if (e == NULL || e->type != ASS || e->left->type != IDENT)
				goto OMPFOR_ERROR;
			vars[lid] = e->left->u.sym;
		}
		else
			if (init->type == DECLARATION)  /* declaration: type var = lb */
			{
				astdecl d = init->u.declaration.decl;

				if (d->type != DINIT)
					goto OMPFOR_ERROR;
				if (d->decl->type != DECLARATOR)
					goto OMPFOR_ERROR;
				if (d->decl->decl->type != DIDENT)
					goto OMPFOR_ERROR;
				vars[lid] = d->decl->decl->u.id;
			}
			else
				goto OMPFOR_ERROR;

		if (lid < orderednum - 1)
		{
			s = s->body;
			if (s != NULL && s->type == COMPOUND && s->body != NULL &&
			    s->body->type == ITERATION && s->body->subtype == SFOR)
				s = s->body;  /* { For } -> For */
			if (s == NULL || s->type != ITERATION || s->subtype != SFOR)
				exit_error(1, "(%s, line %d) openmp error:\n\t"
				      "an ordered(%d) clause requires %d perfectly nested FOR loops.\n",
				      tmp->u.omp->directive->file->name, tmp->u.omp->directive->l,
				      orderednum, orderednum);
		}
	}
	while ((++lid) < orderednum);
	return vars;
}


/* Analyze a FOR statement and determine conformance to OpenMP (canonicality)
 * & other stuff that matter.
 */
static
void analyze_omp_for(aststmt s,
                     symbol *var, astexpr *lb, astexpr *b, astexpr *step,
                     int *condop, int *incrop)
{
	aststmt init;
	astexpr cond, incr, tmp;
	int     rel;
	char    *xtramsg = NULL;

	assert(s != NULL && s->type == ITERATION && s->subtype == SFOR);
	init = s->u.iteration.init;
	cond = s->u.iteration.cond;
	incr = s->u.iteration.incr;
	if (init == NULL || cond == NULL || incr == NULL)
	{
	OMPFOR_ERROR:
		exit_error(1, "(%s, line %d) openmp error:\n\t"
		           "non-conformant FOR statement %s\n", s->file->name, s->l,
		           xtramsg ? xtramsg : "");
	}

	/* Get var and lb from the init part of the FOR
	 */
	xtramsg = "(first part of the for)";
	if (init->type == EXPRESSION)     /* assignment: var = lb */
	{
		astexpr e = init->u.expr;

		if (e == NULL || e->type != ASS || e->left->type != IDENT)
			goto OMPFOR_ERROR;
		*var = e->left->u.sym;
		*lb  = e->right;
	}
	else
		if (init->type == DECLARATION)  /* declaration: type var = lb */
		{
			astdecl d = init->u.declaration.decl;

			/* must check for integral type, too ... */

			if (d->type != DINIT)
				goto OMPFOR_ERROR;
			if (d->decl->type != DECLARATOR)
				goto OMPFOR_ERROR;
			if (d->decl->decl->type != DIDENT)
				goto OMPFOR_ERROR;
			*var = d->decl->decl->u.id;
			*lb  = d->u.expr;
		}
		else
			goto OMPFOR_ERROR;

	/* Get condition operator and ub from the cond part of the FOR
	 */
	xtramsg = "(condition operator)";
	if (cond->type != BOP) goto OMPFOR_ERROR;
	rel = cond->opid;
	if (rel != BOP_lt && rel != BOP_gt && rel != BOP_leq && rel != BOP_geq && 
	    rel != BOP_neq) /* OpenMP 5.0 */
		goto OMPFOR_ERROR;
	/* OpenMP 3.0 allows swapping the left & right sides */
	if (cond->left->type != IDENT || cond->left->u.sym != *var)
	{
		tmp = cond->left;
		cond->left = cond->right;
		cond->right = tmp;
		rel = (rel == BOP_lt) ? BOP_gt : (rel == BOP_leq) ? BOP_geq :
		      (rel == BOP_gt) ? BOP_lt : (rel == BOP_geq) ? BOP_leq : 
		      BOP_neq;   /* stays the same */
	}
	if (cond->left->type != IDENT || cond->left->u.sym != *var) /* sanity check */
		goto OMPFOR_ERROR;
	*condop = rel;
	*b = cond->right;

	/* Last part: get step and increment operator from the incr part of the FOR
	 */
	xtramsg = "(increment part)";
	if (incr->type != PREOP && incr->type != POSTOP && incr->type != ASS)
		goto OMPFOR_ERROR;
	if (incr->left->type != IDENT || incr->left->u.sym != *var) /* sanity check */
		goto OMPFOR_ERROR;
	if (incr->type != ASS)
	{
		/* step is only needed for printing to a string; nothing else. Thus we can
		   leave it as is and "create" it when printing to the string */
		*step = NULL;  /* signal special case of pre/postop */
		*incrop = (incr->opid == UOP_inc) ? BOP_add : BOP_sub;
	}
	else   /* ASS */
	{
		if (incr->opid != ASS_eq && incr->opid != ASS_add && incr->opid != ASS_sub)
			goto OMPFOR_ERROR;
		if (incr->opid != ASS_eq)
		{
			*incrop = (incr->opid == ASS_add) ? BOP_add : BOP_sub;
			*step   = incr->right;
		}
		else
		{
			tmp = incr->right;
			*incrop = tmp->opid;
			if (tmp->type != BOP || (*incrop != BOP_add && *incrop != BOP_sub))
				goto OMPFOR_ERROR;
			if (*incrop == BOP_sub)      /* var = var - incr */
			{
				if (tmp->left->type != IDENT || tmp->left->u.sym != *var)
					goto OMPFOR_ERROR;
				*step = tmp->right;
			}
			else                         /* var = var + incr / incr + var */
			{
				if (tmp->left->type != IDENT || tmp->left->u.sym != *var)
				{
					/* var = incr + var */
					if (tmp->right->type != IDENT || tmp->right->u.sym != *var)
						goto OMPFOR_ERROR;
					*step = tmp->left;
				}
				else /* var = var + incr */
					*step = tmp->right;
			}
		}
		/* OpenMP 5.0: check that step is +/-1 if condop is != */
		if (*condop == BOP_neq)
		{
			int err;
			
			xtramsg = "('!=' condition requires unit increment)";
			rel = xar_calc_int_expr(*step, &err);
			if (err || (rel != 1 && rel != -1))
				goto OMPFOR_ERROR;
		}
	}
}


#define CastLong(t) \
	CastedExpr(Casttypename(Declspec(SPEC_long), NULL), Parenthesis(t))

/* Generates "{ (long) ((u)-(l)), (long) s) }" */
#define LongArray2Initer(u,l,s) \
	BracedInitializer(\
	  CommaList(\
	    CastLong(\
	      Parenthesis( \
	        BinaryOperator(BOP_sub, \
	          ast_expr_copy(u), \
	          ast_expr_copy(l) \
	        )\
	      )\
	    ), \
	    CastLong(ast_expr_copy(s))\
	  )\
	)

/* Generates "{ (long) ((u) > (l) ? (u)-(l) : (l)-(u)), 
 *              (long) ((u) > (l) ? (s) : -(s)) }" 
#define LongArray2IniterUnsigned(u,l,s) \
	BracedInitializer(\
	  CommaList(\
	    CastLong(\
	      Parenthesis(\
	        ConditionalExpr(\
	          BinaryOperator(BOP_gt, ast_expr_copy(u), ast_expr_copy(l)),\
	          BinaryOperator(BOP_sub, ast_expr_copy(u), ast_expr_copy(l)),\
	          BinaryOperator(BOP_sub, ast_expr_copy(l), ast_expr_copy(u))\
	        )\
	      )\
	    ),\
	    CastLong(\
	      Parenthesis(\
	        ConditionalExpr(\
	          BinaryOperator(BOP_gt,ast_expr_copy(u),ast_expr_copy(l)),\
	          ast_expr_copy(s),\
	          UnaryOperator(UOP_neg, ast_expr_copy(s))\
	        )\
	      )\
	    )\
	  )\
	)
 */


/* Generates "{ (long) (l), (long) s, (long) iters_xx }" 
 * The step (second field) has a sign depending on the direction increment.
 */
#define LongArray3Initer(l,s,d,itid) \
	BracedInitializer(\
	    Comma3(\
	        CastLong(ast_expr_copy(l)), \
          CastLong( \
            ast_expr_copy((d) == BOP_add ? \
              (s) : UnaryOperator(UOP_neg, Parenthesis(s)))), \
	        CastLong(itid) \
	    )\
	)


/**
 * Given the loop specifications (l, u, s), this produces a correct
 * (and possibly simplified) expression for the number of iterations,
 * irrespectively of the type of the index variable.
 * There are two cases, depending on the direction of the increment.
 * If stepdir is BOP_add, then the # iterations is given by:
 *   (u > l) ? ( st > 0 ? ( (u - l + s - 1) / s ) : 0 ) :
 *             ( st < 0 ? ( (l - u - s - 1) / (-s) ) : 0)
 * If stepdir is BOP_sub, then the # iterations is given by:
 *   (u > l) ? ( st < 0 ? ( (u - l - s - 1) / (-s) ) : 0 ) :
 *             ( st > 0 ? ( (l - u + s - 1) / s ) : 0)
 * 
 * In the usual case of st > 0, the above is simplified as:
 *   (u > l) ? ( (u - l + s - 1) / s ) : 0   (for BOP_add)
 *   (u > l) ? ( (l - u + s - 1) / s ) : 0   (for BOP_sub)
 * and if s==1,
 *   (u > l) ? (u - l) : 0       (for BOP_add)
 *   (u > l) ? 0 : (l - u)       (for BOP_sub)
 * 
 * @param l the lower bound of the loop
 * @param u the upper bound of the loop
 * @param s the step increment
 * @param stepdir the direction of the increment (BOP_add / BOP_sub)
 * @param plainstep is 0 if the step increment is a full expression, 
 *                  2 if it is a constant, or 1 if it is equal to 1.
 * @return an expression for the total number of iterations
 */
static 
astexpr specs2iters(astexpr l, astexpr u, astexpr s, int stepdir, int plainstep)
{
	if (plainstep)
	{
		if (plainstep == 1)       /* step = 1 */
			return
				ConditionalExpr(
					Parenthesis(BinaryOperator(BOP_geq, ast_expr_copy(u), ast_expr_copy(l))),
					(stepdir == BOP_add ? 
						Parenthesis(BinaryOperator(BOP_sub,ast_expr_copy(u),ast_expr_copy(l))) 
						: numConstant(0)),
					(stepdir == BOP_sub ? 
						Parenthesis(BinaryOperator(BOP_sub,ast_expr_copy(l),ast_expr_copy(u)))
						: numConstant(0))
				);
		else                      /* step = positive constant */
			return
				ConditionalExpr(
					Parenthesis(BinaryOperator(BOP_geq,ast_expr_copy(u),ast_expr_copy(l))),
					(stepdir == BOP_add ? 
						Parenthesis(
							BinaryOperator(BOP_div,
								Parenthesis(
									BinaryOperator(BOP_add,
										BinaryOperator(BOP_sub,ast_expr_copy(u),ast_expr_copy(l)),
										BinaryOperator(BOP_sub,ast_expr_copy(s),numConstant(1))
									)
								),
								ast_expr_copy(s)
							)
						) 
						: numConstant(0)),
					(stepdir == BOP_sub ? 
						Parenthesis(
							BinaryOperator(BOP_div,
								Parenthesis(
									BinaryOperator(BOP_add,
										BinaryOperator(BOP_sub,ast_expr_copy(l),ast_expr_copy(u)),
										BinaryOperator(BOP_sub,ast_expr_copy(s),numConstant(1))
									)
								),
								ast_expr_copy(s)
							)
						) 
						: numConstant(0))
				);
	}
	
	/* General case */
	return
		ConditionalExpr(
			Parenthesis(BinaryOperator(BOP_geq, ast_expr_copy(u), ast_expr_copy(l))),
			Parenthesis(
				ConditionalExpr(
					BinaryOperator(stepdir == BOP_add ? BOP_gt : BOP_lt, 
						ast_expr_copy(s), 
						numConstant(0)
					),
					Parenthesis(
						BinaryOperator(BOP_div,
							Parenthesis(
								BinaryOperator(BOP_sub,
									BinaryOperator(stepdir,
										BinaryOperator(BOP_sub, ast_expr_copy(u), ast_expr_copy(l)),
										ast_expr_copy(s)
									),
									numConstant(1)
								)
							),
							stepdir == BOP_add ? 
								ast_expr_copy(s) : UnaryOperator(UOP_neg, ast_expr_copy(s))
						)
					),
					numConstant(0)
				)
			),
			Parenthesis(
				ConditionalExpr(
					BinaryOperator(stepdir == BOP_add ? BOP_lt : BOP_gt, 
						ast_expr_copy(s), 
						numConstant(0)
					),
					Parenthesis(
						BinaryOperator(BOP_div,
							Parenthesis(
								BinaryOperator(BOP_sub,
									BinaryOperator(stepdir == BOP_add ? BOP_sub : BOP_add,
										BinaryOperator(BOP_sub, ast_expr_copy(l), ast_expr_copy(u)),
										ast_expr_copy(s)
									),
									numConstant(1)
								)
							),
							stepdir == BOP_add ? 
								UnaryOperator(UOP_neg, ast_expr_copy(s)) : ast_expr_copy(s)
						)
					),
					numConstant(0)
				)
			)
		);
}


#define MAXLOOPS 96


/* Possible clauses:
 * private, firstprivate, lastprivate, reduction, nowait, ordered, schedule,
 * collapse.
 */
void xform_for(aststmt *t)
{
	aststmt   s = (*t)->u.omp->body, parent = (*t)->parent, v, realbody,
	          decls, inits = NULL, lasts = NULL, reds = NULL, redarrinits = NULL, 
	          redfree = NULL, stmp, embdcls = NULL, arrsecxvars = NULL;
	astexpr   lb, ub, step, lbs[MAXLOOPS], ubs[MAXLOOPS], steps[MAXLOOPS], 
	          expr, elems;
	symbol    var, realvar, vars[MAXLOOPS];
	int       incrop, condop, stepdir[MAXLOOPS];
	int       schedtype = OC_static /* default */, modifer = OCM_none,
	          static_chunk = 0, i = 0, collapsenum = 1, doacrossnum = 0, nestnum;
	bool      ispfor = ((*t)->u.omp->type == DCFOR_P);
	bool      haslast, hasboth, hasred;
	astexpr   schedchunk = NULL;    /* the chunksize expression */
	char      *chsize = NULL,       /* the chunksize value or variable */
	          iters[MAXLOOPS][128],
	          plainstep,
	          plainsteps[MAXLOOPS],
	          clabel[22];
	ompclause nw  = xc_ompcon_get_clause((*t)->u.omp, OCNOWAIT),
	          sch = xc_ompcon_get_clause((*t)->u.omp, OCSCHEDULE),
	          ord = xc_ompcon_get_clause((*t)->u.omp, OCORDERED),
	          ordnum = xc_ompcon_get_clause((*t)->u.omp, OCORDEREDNUM),
	          col = xc_ompcon_get_clause((*t)->u.omp, OCCOLLAPSE);
	bool      needbarrier = (nw == NULL &&
	                         xform_implicit_barrier_is_needed((*t)->u.omp));
	symtab    dvars;
	stentry   varentry;

	v = ompdir_commented((*t)->u.omp->directive); /* Put directive in comments */

	/*
	 * Preparations
	 */

	/* The name of the label used for canceling. We use line number to avoid
	 * conflicts
	 */
	snprintf(clabel, 22, "CANCEL_for_%d", (*t)->u.omp->l);

	if (sch)
	{
		if (sch->subtype == OC_auto)      /* Implement AUTO as STATIC for now */
			schedtype = OC_static;          /* but we will ignore any chunksize */
		else
			schedtype  = sch->subtype;      /* OC_static, OC_... */
		schedchunk = sch->u.expr;
		if (schedtype == OC_static && sch->subtype != OC_auto && schedchunk)
			static_chunk = 1;
		if (schedtype == OC_affinity && schedchunk)
			schedchunk = ast_expr_copy(schedchunk);
		/* Optimize: if schedchunk is a constant, don't use a variable for it */
		if (schedchunk && schedchunk->type == CONSTVAL)
			chsize = strdup(schedchunk->u.str);    /* memory leak */
		modifer = sch->modifier;
	}

	if (ord && modifer == OCM_nonmonotonic)
		exit_error(1, "(%s, line %d) openmp error:\n\t"
		     "nonmonotonic schedules are not allowed along with ordered clauses.\n",
		     (*t)->u.omp->directive->file->name, (*t)->u.omp->directive->l);
	
	if (ord && ordnum)
		exit_error(1, "(%s, line %d) openmp error:\n\t"
		     "plain ordered clauses are not allowed in doacross loops.\n",
		     (*t)->u.omp->directive->file->name, (*t)->u.omp->directive->l);

	if (col)
	{
		if ((collapsenum = col->subtype) >= MAXLOOPS)
			exit_error(1, "(%s, line %d) ompi error:\n\t"
				"cannot collapse more than %d FOR loops.\n",
				(*t)->u.omp->directive->file->name, (*t)->u.omp->directive->l,MAXLOOPS);
	}

	if (ordnum)
	{
		if ((doacrossnum = ordnum->subtype) >= MAXLOOPS)
			exit_error(1, "(%s, line %d) ompi error:\n\t"
				"doacross loop nests should have up to %d FOR loops.\n",
				(*t)->u.omp->directive->file->name, (*t)->u.omp->directive->l,MAXLOOPS);
		if (doacrossnum < collapsenum)
			exit_error(1, "(%s, line %d) ompi error:\n\t"
		             "doacross loop collapse number cannot be larger "
		             "than its ordered number.\n",
		             (*t)->u.omp->directive->file->name, (*t)->u.omp->directive->l);
	}
	
	/* Collect all data clause vars - we need to check if any vars
	 * are both firstprivate and lastprivate
	 */
	dvars = xc_validate_store_dataclause_vars((*t)->u.omp->directive);

	/* Analyze the loop(s) */
	nestnum = (doacrossnum > collapsenum) ? doacrossnum : collapsenum;
	i = 0;
	do
	{
		analyze_omp_for(s, &var, &lb, &ub, &step, &condop, &incrop);

		/* First check if the loop variable has been enlisted; if not,
		 * it is automatically considered private (v25) - so we make it
		 * appear as if there was a private(var) clause.
		 */
		if ((varentry = symtab_get(dvars, var, IDNAME)) == NULL)
		{
			if (s->u.iteration.init->type == EXPRESSION)
				symtab_put(dvars, var, IDNAME)->ival = OCPRIVATE;
			else
				embdcls = (embdcls) ?
					BlockList(
						embdcls,
						Declaration( /* without the initializer */
							ast_spec_copy(s->u.iteration.init->u.declaration.spec),
							ast_decl_copy(s->u.iteration.init->u.declaration.decl->decl)
						)
					) :
					Declaration(
						ast_spec_copy(s->u.iteration.init->u.declaration.spec),
						ast_decl_copy(s->u.iteration.init->u.declaration.decl->decl)
					);
		}
		else
		{
			if (s->u.iteration.init->type != EXPRESSION)  /* a declaration */
				exit_error(1, "(%s, line %d) openmp error:\n\t"
					"iteration variable '%s' is declared within the FOR statement\n\t"
					"and thus it cannot appear in the directive's data clauses.\n",
					(*t)->u.omp->directive->file->name, (*t)->u.omp->directive->l, var->name);
			/* Remove the FIRSTPRIVATE attribute if any (there is no use for it) */
			/* Actually, v25 (p.64,l.23) specifies that the iteration variable
			 * can only appear in a PRIVATE or LASTPRIVATE clause, so we should
			 * emit at least a warning.
			 */
			if (varentry->ival == OCFIRSTPRIVATE || varentry->ival == OCFIRSTLASTPRIVATE)
				warning("(%s, line %d) warning:\n\t"
					"iteration variable '%s' cannot appear in a FIRSTPRIVATE clause..\n\t"
					"  .. let's pretend it was in a PRIVATE clause.\n",
					(*t)->u.omp->directive->file->name, (*t)->u.omp->directive->l, var->name);
			if (varentry->ival == OCFIRSTPRIVATE)
				varentry->ival = OCPRIVATE;
			else
				if (varentry->ival == OCFIRSTLASTPRIVATE)
					varentry->ival = OCLASTPRIVATE;
		}
		
		if (step == NULL || step->type == CONSTVAL)   /* ++/-- or += constant */
		{
			plainstep = (step == NULL) ? 1 : 2;
			step = (step == NULL) ? numConstant(1) : ast_expr_copy(step);
		}
		else /* step != NULL && general expression for step */
		{
			step = Parenthesis(ast_expr_copy(step));   /* An expression */
			plainstep = 0;
		}

		vars[i] = var;
		lbs[i]  = Parenthesis(ast_expr_copy(lb));
		ubs[i]  = Parenthesis(
		            (condop == BOP_leq || condop == BOP_geq) ?  /* correct ub */
		            BinaryOperator((condop == BOP_leq) ? BOP_add : BOP_sub,
		                           Parenthesis(ast_expr_copy(ub)),
		                           numConstant(1)) :
		            ast_expr_copy(ub)
		          );
		steps[i] = step;
		stepdir[i] = incrop;
		sprintf(iters[i], "iters_%s_", var->name);
		plainsteps[i] = plainstep;      /* TODO: actually use this info */

		if (i == collapsenum-1)
		{
			realbody = s;                 /* Remember where the real body is */
			realvar = var;
		}
		if (i < nestnum - 1)
		{
			s = s->body;
			if (s != NULL && s->type == COMPOUND && s->body != NULL &&
			    s->body->type == ITERATION && s->body->subtype == SFOR)
				s = s->body;  /* { For } -> For */
			if (s == NULL || s->type != ITERATION || s->subtype != SFOR)
				exit_error(1, "(%s, line %d) openmp error:\n\t"
					"%d perfectly nested FOR loops were expected.\n",
					(*t)->u.omp->directive->file->name, (*t)->u.omp->directive->l,
					nestnum, nestnum);
		}
	}
	while ((++i) < nestnum);
	s = realbody;
	var = realvar;

	/* get possibly new variables for array section parameters */
	arrsecxvars = red_arrayexpr_simplify((*t)->u.omp->directive);

	/* declarations from the collected vars (not the clauses!) */
	decls = xc_stored_vars_declarations(&haslast, &hasboth, &hasred);
	if (arrsecxvars)
		decls = decls ? Block2(arrsecxvars, decls) : arrsecxvars;
	/* initialization statements for firstprivate non-scalar vars */
	if (decls)
		inits = xc_ompdir_fiparray_initializers((*t)->u.omp->directive);
	/* assignments for lastprivate vars */
	if (haslast)
		lasts = xc_ompdir_lastprivate_assignments((*t)->u.omp->directive);
	if (hasred)
	{
		/* Temporary local variables should be kept till the reduction operation
		 * is fully completed; this is guaranteed if after a barrier, so we must
		 * turn off any barrier removals.
		 * TODO: maybe we should re-design reductions...
		 */
		if (!oldReduction)
			needbarrier = true;
		/* Initializers for array reductions */
		redarrinits = red_array_initializers_from_ompdir((*t)->u.omp->directive);
		if (redarrinits)
			inits = (inits) ? BlockList(inits, redarrinits) : redarrinits;
		/* Code to do the reductions */
		reds = red_generate_code_from_ompdir((*t)->u.omp->directive);
		/* Possible de-allocations to go after the barrier */
		redfree = red_generate_deallocations_from_ompdir((*t)->u.omp->directive);
	}

	stmp  = (embdcls) ? embdcls : verbit(" ");
	if (schedtype == OC_affinity)
	{
		if (haslast)
			stmp = BlockList(
			         stmp,
			         Declaration(    /* declare: <specs> niters_,iter_=1; */
			           ITERCNT_SPECS,
			           DeclList(
			             Declarator(NULL, IdentifierDecl(Symbol("niters_"))),
			             InitDecl(
			               Declarator(NULL, IdentifierDecl(Symbol(NORMALIZEDITER))),
			               numConstant(1)
			             )
			           )
			         )
			       );
	}
	else
	{
		stmp = BlockList(
		         stmp,  /* Initialize because if a thread gets no iterations, the */
		         Declaration(  /* lastprivate check for iter==niters may succeed! */
		           ITERCNT_SPECS,  /*  <specs> niters_=0,iter_=0,fiter_,liter_=0; */
		           DeclList(
		             DeclList(
		               DeclList(
		                 InitDecl(
		                   Declarator(NULL, IdentifierDecl(Symbol("niters_"))),
		                   numConstant(0)
		                 ),
		                 InitDecl(
		                   Declarator(NULL, IdentifierDecl(Symbol(NORMALIZEDITER))),
		                   numConstant(0)
		                 )
		               ),
		               Declarator(NULL, IdentifierDecl(Symbol("fiter_")))
		             ),
		             InitDecl(
		               Declarator(NULL, IdentifierDecl(Symbol("liter_"))),
		               numConstant(0)
		             )
		           )
		         )
		       );
		if (collapsenum > 1 || doacrossnum > 0)  /* We need vars for # iterations */
		{
			if (collapsenum > 1)
				stmp = BlockList(
				         stmp,
				         Declaration(ITERCNT_SPECS,
				                     InitDecl(
				                       Declarator(NULL, IdentifierDecl(Symbol("pp_"))),
				                       numConstant(1)
				                     ))
				       );
			for (i = 0; i < nestnum; i++)
				stmp = BlockList(
				         stmp,
				         Declaration(
				           ITERCNT_SPECS,
				           InitDecl(
				             Declarator(NULL, IdentifierDecl(Symbol(iters[i]))),
			               specs2iters(lbs[i],ubs[i],steps[i],stepdir[i],plainsteps[i])
				           )
				         )
				       );
		}
		if (chsize == NULL)
		{
			chsize = "chunksize_";
			if (schedchunk || schedtype == OC_runtime)     /* expr for chunk size */
			{
				stmp = BlockList(
				         stmp,
				         Declaration(
				           ITERCNT_SPECS,
				           (schedtype == OC_runtime) ?
				             Declarator(NULL, IdentifierDecl(Symbol(chsize))) :
				             InitDecl(
				               Declarator(NULL, IdentifierDecl(Symbol(chsize))),
				               ast_expr_copy(schedchunk)
				             )
				         )
				       );
			}
		}
		/* we may need 2 more variables (int) */
		if (static_chunk)
			stmp = BlockList(
			         stmp,
			         Declaration( /* declare: int chid_, TN_=omp_get_num_threads(); */
			           Declspec(SPEC_int),
			           DeclList(
			             Declarator(NULL, IdentifierDecl(Symbol("chid_"))),
			             InitDecl(
			               Declarator(NULL, IdentifierDecl(Symbol("TN_"))),
			               Call0_expr("omp_get_num_threads")
			             )
			           )
			         )
			       );
		/* we may need 2 more variables: get_chunk_ & staticextra_ */
		if (schedtype == OC_runtime)
		{
			aststmt xdc = Declaration(
			                Usertype(Symbol("chunky_t")),
			                Declarator(NULL, IdentifierDecl(Symbol("get_chunk_")))
			              );
			/* Substitute type by hand since the produced code may not get xformed */
			xt_barebones_substitute(&(xdc->u.declaration.spec),
			                        &(xdc->u.declaration.decl));
			xdc = BlockList(
			        xdc,
			        Declaration(
			          Declspec(SPEC_int),
			          InitDecl(
			            Declarator(NULL, IdentifierDecl(Symbol("staticextra_"))),
			            numConstant(-1)
			          )
			        )
			      );
			stmp = BlockList(stmp, xdc);
		}
	}

	/* Finally, we need the loop parameters for doacross loops */
	if (ordnum)
	{
		/* Form the initializer */
		elems = LongArray3Initer(lbs[0], steps[0], stepdir[0], IdentName(iters[0]));
		for (i = 1; i < doacrossnum; i++)
			elems = 
				CommaList(
					elems, 
					LongArray3Initer(lbs[i], steps[i], stepdir[i], IdentName(iters[i]))
				);
		/* Declare and initialize _doacc_params_[][3] */
		stmp = BlockList(
				stmp, 
				Declaration(
					Declspec(SPEC_long),
					InitDecl(
						Declarator(
							NULL,
							ArrayDecl(
								ArrayDecl(IdentifierDecl(Symbol(DOACCPARAMS)),NULL,NULL),
								NULL,
								Constant("3")
							)
						),
						BracedInitializer(elems)
					)
				)
			);
	}

	decls = (decls) ? BlockList(decls, stmp) : stmp;

	/*
	 * Do the job
	 */

	(*t)->u.omp->body = NULL;     /* Make it NULL so as to free it easily */
	ast_free(*t);                 /* Get rid of the OmpStmt */

	if (inits)
		decls = BlockList(decls, inits);  /* Append the initialization statements */

	/* Append our new code: niters_ = ...; ort_entering_for(...); */
	if (collapsenum == 1 && doacrossnum == 0)
		elems = CastLong( 
		          specs2iters(lbs[0], ubs[0], steps[0], stepdir[0], plainsteps[0]) 
		        );
	else
		for (elems = IdentName(iters[0]), i = 1; i < collapsenum; i++)
			elems = BinaryOperator(BOP_mul, elems, IdentName(iters[i]));
	expr = elems;

	if (ordnum)               /* Need more info for doacross loops */
		stmp = Expression(      /* ort_entering_doacross(nw,doacnum,collnum,...); */
	           FunctionCall(
	             IdentName("ort_entering_doacross"),
	             Comma6(
	               numConstant(nw ? 1 : 0),
	               numConstant(doacrossnum),
	               numConstant(collapsenum),
	               numConstant(FOR_CLAUSE2SCHED(schedtype, static_chunk)),
	               schedchunk ? IdentName(chsize) : numConstant(-1),
	               IdentName(DOACCPARAMS)
	             )
	           )
	         );
	else
		stmp = Expression(      /* ort_entering_for(nw,ord); */
	           FunctionCall(
	             IdentName("ort_entering_for"),
	             Comma2(numConstant(nw ? 1 : 0), numConstant(ord ? 1 : 0))
	           )
	         );

		
	stmp = BlockList(
	         Expression(     /* niters_ = ... */
	           Assignment(IdentName("niters_"), ASS_eq, expr)
	         ),
	         stmp
	       );
	stmp = BlockList(decls, stmp);
	if (hasboth)   /* a var is both fip & lap; this needs a barrier here :-( */
		stmp = BlockList(stmp, BarrierCall());

	/* Prepare the main loop */
	if (schedtype == OC_affinity)
	{
		/* same loop, new body */
		if (lasts || ord)
		{
			if (lasts)
				s->body = BlockList(   /* iter++ */
				            Expression(PostOperator(IdentName("iter"), UOP_inc)),
				            s->body
				          );
			s->body = Compound(s->body);
		}
		s->body = If(
		            FunctionCall(
		              IdentName("ort_affine_iteration"), schedchunk
		            ),
		            s->body,
		            NULL
		          );
	}
	else
	{
		aststmt idx = NULL;

		if (collapsenum > 1)
		{
			idx = AssignStmt(IdentName("pp_"), numConstant(1));
			for (i = collapsenum - 1; i >= 0; i--)
			{
				idx = BlockList(
				        idx,
				        AssignStmt(
				          Identifier(vars[i]),
				          BinaryOperator(
				            stepdir[i], //BOP_add,
				            ast_expr_copy(lbs[i]),
				            BinaryOperator(
				              BOP_mul,
				              ast_expr_copy(steps[i]),
				              Parenthesis(
				                BinaryOperator(
				                  BOP_mod,
				                  Parenthesis(
				                    BinaryOperator(
				                      BOP_div,
				                      IdentName(NORMALIZEDITER),
				                      IdentName("pp_")
				                    )
				                  ),
				                  IdentName(iters[i])
				                )
				              )
				            )
				          )
				        )
				      );
				if (i != 0)
					idx = BlockList(
					        idx,
					        Expression(Assignment(IdentName("pp_"), ASS_mul,
					                              IdentName(iters[i]))
					                  )
					      );
			}
		}
		/* Loop becomes:
		 *   for (iter = fiter; iter < liter; iter++) {
		 *     <var> = lb +/- iter*step
		 *     <body>
		 *   }
		 * optimized as:
		 *   for (iter = fiter, var = ...; iter < liter; iter++, var +/-= step) {
		 *     <body>
		 *   }
		 * If there is an ordered clause, we insert "ort_for_curriter(iter_)"
		 * just before the body, to let the runtime know our current iteration.
		 */

#define ORTCURRITER \
     Expression(FunctionCall(IdentName("ort_for_curriter"), \
     IdentName(NORMALIZEDITER)))

		s = For(Expression((collapsenum > 1) ?
		                   Assignment(IdentName(NORMALIZEDITER),
		                              ASS_eq,
		                              IdentName("fiter_"))
		                   :
		                   CommaList(
		                     Assignment(IdentName(NORMALIZEDITER),
		                                ASS_eq,
		                                IdentName("fiter_")),
		                     Assignment(Identifier(var),
		                                ASS_eq,
		                                BinaryOperator(
		                                  stepdir[0], 
		                                  ast_expr_copy(lbs[0]),
		                                  BinaryOperator(BOP_mul,
		                                    IdentName("fiter_"),
		                                    ast_expr_copy(steps[0]))
		                                  )
		                               )
		                   )
		                  ),
		        BinaryOperator(BOP_lt, IdentName(NORMALIZEDITER),
		                       IdentName("liter_")
		                      ),
		        ((collapsenum > 1) ?
		         PostOperator(IdentName(NORMALIZEDITER), UOP_inc) :
		         CommaList(
		           PostOperator(IdentName(NORMALIZEDITER), UOP_inc),
		           Assignment(Identifier(var), 
		                      stepdir[0]==BOP_add ? ASS_add : ASS_sub,
		                      ast_expr_copy(steps[0]))
		         )
		        ),
		        (collapsenum > 1) ? 
		          ( ord ? Compound(BlockList(BlockList(idx, ORTCURRITER), s->body)) :
		                  Compound(BlockList(idx, s->body)) ) : 
		          ( ord ? Compound(BlockList(ORTCURRITER, s->body)) : s->body )
		       );

#undef ORTCURRITER
	}

	/* Schedule-dependent code */
	switch (schedtype)
	{
		case OC_static:
			if (static_chunk)
				stmp = BlockList(
				         stmp,
				         For(
				           parse_blocklist_string("chid_ = omp_get_thread_num();"),
				           NULL,
				           parse_expression_string("chid_ += TN_"),
				           Compound(
				             BlockList(
				               parse_blocklist_string(
				                 "fiter_ = chid_*(%s);"
				                 "if (fiter_ >= niters_) break;"
				                 "liter_ = fiter_ + (%s);"
				                 "if (liter_ > niters_) liter_ = niters_;",
				                 chsize, chsize
				               ),
				               s
				             )
				           )
				         )
				       );
			else
				stmp = BlockList(
				         stmp,
				         If(
				           parse_expression_string(
				             "ort_get_static_default_chunk(niters_, &fiter_, &liter_)"),
				           Compound(s),
				           NULL
				         )
				       );
			break;

		case OC_guided:
		case OC_dynamic:
			stmp = BlockList(
			         stmp,
			         While(
			           parse_expression_string(
			             "ort_get_%s_chunk(niters_,%s,%s,&fiter_,&liter_,(int*)0)",
			             schedtype == OC_guided ? "guided" : "dynamic",
			             schedchunk ? chsize : "1",
			             (modifer == OCM_none || modifer == OCM_monotonic) ? "1":"0"),
			           Compound(s)
			         )
			       );
			break;

		case OC_runtime:
			stmp = Block3(
			         stmp,
			         /* ort_get_runtime_schedule_stuff(&get_chunk_, &chunksize_); */
			         FuncCallStmt(
			           IdentName("ort_get_runtime_schedule_stuff"),
			           CommaList(
			             UOAddress(IdentName("get_chunk_")),
			             UOAddress(IdentName("chunksize_"))
			           )
			         ),
			         While(
			           parse_expression_string(  /* Too big to do it by hand */
			             "(*get_chunk_)(niters_, chunksize_, %s, &fiter_, &liter_, "
			             "&staticextra_)", 
			             (modifer == OCM_none || modifer == OCM_monotonic) ? "0":"1"),
			           Compound(s)
			         )
			       );
			break;

		case OC_affinity:
			stmp = BlockList(stmp, s);
			break;
	}

	*t = BlockList(v, stmp);

	/* Add a label that is used when canceling */
	*t = BlockList(*t, Labeled(Symbol(clabel), Expression(NULL)));
	if (!ispfor || ord || ordnum)   /* Still need it if ordered clause exists */
		*t = BlockList(*t, Call0_stmt("ort_leaving_for"));
	if (lasts)
	{
		if (collapsenum > 1)
		{
			aststmt idx;     /* Need to set explicitly the correct index values */

			idx = Expression(
			        Assignment(Identifier(vars[0]), stepdir[0] == BOP_add ? ASS_add : ASS_sub, ast_expr_copy(steps[0]))
			      );
			for (i = 1; i < collapsenum; i++)
				idx = BlockList(
				        idx,
				        Expression(
				          Assignment(Identifier(vars[i]), stepdir[i] == BOP_add ? ASS_add : ASS_sub, ast_expr_copy(steps[i]))
				        )
				      );
			lasts = BlockList(idx, lasts);
		}

		*t = BlockList(
		       *t,
		       If(
		         BinaryOperator(BOP_land,
		           IdentName(NORMALIZEDITER),
		           BinaryOperator(BOP_eqeq,
		             IdentName(NORMALIZEDITER),
		             IdentName("niters_")
		           )
		         ),
		         lasts->type == STATEMENTLIST ?  Compound(lasts) : lasts,
		         NULL
		       )
		     );
	}
	if (reds)
		*t = BlockList(*t, reds);
	if (needbarrier)
		*t = BlockList(*t, BarrierCall());
	else
		if (!nw)   /* We ditched the barrier; but should at least flush */
			*t = BlockList(*t, Call0_stmt("ort_fence")); 
	if (redfree)
		*t = BlockList(*t, redfree);

	*t = Compound(*t);
	ast_stmt_parent(parent, *t);

	/* Must free the array
	 */
}
