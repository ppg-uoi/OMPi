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

/* x_teams.c */

#include <stdio.h>
#include "dfa.h"
#include "ompi.h"
#include "outline.h"
#include "x_clauses.h"
#include "ast_copy.h"


static autoshattr_t as;


vartype_t xtm_implicitDefaultAuto(setelem(vars) s)
{
	if (set_get(as.autoShared, s->key))
		return DCT_BYREF;
	else
		if (set_get(as.autoPrivate, s->key))
			return DCT_PRIVATE;
		else
			if (set_get(as.autoFirstPrivate, s->key))
				return DCT_BYVALUE;
	/*else if (isInSet( as.autoReduction, s->key))
	clausetype = OCREDUCTION;
	return DCT_REDUCTION*/

	return DCT_BYREF;
}


vartype_t xtm_implicitDefault(setelem(vars) s)
{
	if (s->value.clause == OCAUTO)
		return xtm_implicitDefaultAuto(s);
	return DCT_BYREF;
}


/* OpenMP v5.0, p. 329:
 *  distribute, distribute simd, distribute parallel worksharing-loop, 
 * distribute parallel worksharing-loop SIMD, loop, parallel regions, 
 * including any parallel regions arising from combined constructs,
 * omp_get_num_teams() regions, and omp_get_team_num() regions are the only 
 * OpenMP regions that may be strictly nested inside the teams region.
 */
void xform_teams(aststmt *t)
{
	static int teamfuncid = 0;

	astexpr    numteamsexpr = NULL, thrlimitexpr = NULL;
	ompclause  c, def = xc_ompcon_get_clause((*t)->u.omp, OCDEFAULT);
	outcome_t  op;
	static outpars_t oo =
	{
		true,                   // structbased
		"",                     //+functionName
		"ort_start_teams",      // functionCall
		NULL,                   //+extraParameters
		BYVAL_byname,           // byvalue_type (by name)
		false,                  //+global_byref_in_struct
		"__shvt__",             // structName
		"_shvars",              // structVariable
		NULL,                   // structInitializer
		xtm_implicitDefault,    // implicitDefault function
		NULL,                   // deviceexpr
		true,                   // addComment
		NULL                    // thestmt
	};

#if 0
	if (enableAutoscope) /* Agelos */
	{
		assert(dfa_parreg_get_results((*t)->l) != NULL);
		as = *dfa_parreg_get_results((*t)->l);
		if (def && def->subtype == OC_auto)
			oo.implicitDefault = xp_implicitDefaultAuto;
	}
#endif

	/* (1) Check for num_teams and thead_limit clauses and keep a copy
	 */
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCNUMTEAMS)) != NULL)
		numteamsexpr = ast_expr_copy(c->u.expr);
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCTHREADLIMIT)) != NULL)
		thrlimitexpr = ast_expr_copy(c->u.expr);

	/* (2) Call outline_OpenMP
	 */
	sprintf(oo.functionName, "_teamthrFunc%d_", teamfuncid++);
	oo.extraParameters = Comma2(  // <numteams>, <theadlimit>
	                       numteamsexpr ? numteamsexpr : numConstant(-1),
	                       thrlimitexpr ? thrlimitexpr : numConstant(-1)
	                     );
	oo.thestmt = *t;
	op = outline_OpenMP(t, oo);
}


void xform_targetteams(aststmt *t)
{
	fprintf(stderr, "Please wait a bit...\n");
}
