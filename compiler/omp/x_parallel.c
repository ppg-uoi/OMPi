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

/* x_parallel.c -- too big a transformation to be part of ast_xform! */

#include <assert.h>
#include "ast_xform.h"
#include "ast_copy.h"
#include "x_parallel.h"
#include "x_target.h"
#include "x_clauses.h"
#include "omp.h"
#include "ompi.h"
#include "outline.h"
#include "ast_show.h"
#include "dfa.h"

static autoScopeSets as;

vartype_t xp_implicitDefaultAuto(setelem(vars) s)
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

vartype_t xp_implicitDefault(setelem(vars) s)
{
	if (s->value == OCAUTO)
		return xp_implicitDefaultAuto(s);
	return DCT_BYREF;
}

/* Returns the type of the proc_bind clause
 */
static int xp_procbind(ompcon t)
{
	ompclause c = xc_ompcon_get_unique_clause(t, OCPROCBIND);

	if (c == NULL)
		return omp_proc_bind_true;

	switch (c->subtype)
	{
		case OC_bindmaster:
			return omp_proc_bind_master;
		case OC_bindclose:
			return omp_proc_bind_close;
		case OC_bindspread:
			return omp_proc_bind_spread;
	}

	// Bug
	exit_error(1,
	           "(%s) openmp error:\n\t"
	           "unknown proc_bind type '%s'.\n\t",
	           t->directive->file->name, clausesubs[c->subtype]);
}

aststmt xp_handle_copyin(set(vars) s, char *structVariable)
{
	setelem(vars) e;
	aststmt       st = NULL, tmp;

	if (set_isempty(s))
		return NULL;

	for (e = s->first; e; e = e->next)
	{
		/* Copyin variables have been handled as shared by outline. We first
		 * check if the variable is indeed copyin
		 */
		if (e->value != OCCOPYIN)
			continue;

		stentry orig = symtab_get(stab, e->key, IDNAME);
		astexpr init;


		init = /* ( e->scopelevel == -10 ) ?
		         Identifier( tp_new_name(var) ) : */  /* The initializer/source */
		         UnaryOperator(
		           UOP_star,
		           Parenthesis(                       /* = *(_thrarg->var) */
		             PtrField(IdentName(structVariable), e->key)
		           )
		         );

		if (!orig->isarray)           /* Plain assignment */
			tmp = Expression(            /* *var = <orig_var> */
			        Assignment(
			          UnaryOperator(UOP_star, Identifier(e->key)),
			          ASS_eq,
			          init
			        )
			      );
		else                       /* Array assignment, like in firstprivates */
			tmp = xc_memcopy(UnaryOperator(UOP_star, Identifier(e->key)), init,
			                 Sizeof(UnaryOperator(UOP_star, Identifier(e->key))));

		st = (st) ? BlockList(st, tmp) : tmp;
	}

	return st;
}

void xform_parallel(aststmt *t, int iscombined)
{
	static int thrnum = 0;

	astexpr      numthrexpr = NULL, ifexpr = NULL;
	aststmt      copyininit;
	ompclause    c;
	int          procbind_type;
	outcome_t    op;
	char         clabel[22];

	static outpars_t oo =
	{
		"",                     //+functionName
		"ort_execute_parallel", // functionCall
		NULL,                   //+extraParameters
		true,                   // byvalue_type (by name)
		false,                  //+global_byref_in_struct
		"__shvt__",             // structName
		"_shvars",              // structVariable
		NULL,                   // structInitializer
		xp_implicitDefault,     // implicitDefault function
		NULL                    // deviceexpr
	};

	/* The name of the label used for canceling. We use line number to avoid
	 * conflicts (there shouldn't be any since the code is outlined but we
	 * use it anyway incase we inline the parallel code in the future)
	 */
	snprintf(clabel, 22, "CANCEL_parallel_%d", (*t)->l);

	/**************************************************************************/
	/*                                 Agelos                                 */
	/**************************************************************************/
	/* Search the list for the autoscoping results of current parallel region */
	/* TODO: add when bug fixed 
	parNode *p = isInAutoList((*t)->l);
	*/
	
	/* We store them at set group "as" */
	/* TODO: add when bug fixed 
	assert(p != NULL);
	as = p->a;
	*/

	//Check for default auto clause
	ompclause def = xc_ompcon_get_clause((*t)->u.omp, OCDEFAULT);
	if (def && def->subtype == OC_auto)
		oo.implicitDefault = xp_implicitDefaultAuto;
	else
		oo.implicitDefault = xp_implicitDefault;
	/**************************************************************************/
	/*                                                                        */
	/**************************************************************************/


	/* (1) Check for if and num_threads clauses and keep a copy
	 */
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCNUMTHREADS)) != NULL)
		numthrexpr = ast_expr_copy(c->u.expr);
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCIF)) != NULL)
		ifexpr = ast_expr_copy(c->u.expr);

	/* (2) Retrieve bind type
	 */
	procbind_type = xp_procbind((*t)->u.omp);

	/* (3) Call outline_OpenMP
	 */
	sprintf(oo.functionName, "_thrFunc%d_", thrnum++);
	oo.extraParameters = CommaList(  // <numthread>, combined, <procbind>
	                       CommaList(
	                         numthrexpr ? numthrexpr : numConstant(-1),
	                         numConstant(iscombined)
	                       ),
	                       numConstant(procbind_type)
	                     );
	//If we are in a target region add global shared variables to the struct
	if (inTarget())
		oo.global_byref_in_struct = true;
	else
		oo.global_byref_in_struct = false;
	op = outline_OpenMP(t, oo);

	/* (4) Add if clause
	 */
	if (ifexpr)                    /* Check if we have an if() clause */
	{
		aststmt parent = op.functioncall->parent;

		//Replace call with if(ifexpr) execute_parallel else execute_serial
		//WARNING replacing parent->body can cause problems, for example if
		//parent is IF or STATEMENTLIST we may not be the body of our parent
		//In this case if there are any shared/firstprivate variables the parent
		//is a STATEMENTLIST with us as body, else it is a COMPOUND
		parent->body = If(ifexpr, op.functioncall,
		                  /* ort_execute_serial(thrFunc, (void *) __shvt__/0); */
		                  FuncCallStmt(
		                    IdentName("ort_execute_serial"),
		                    CommaList(
		                      IdentName(oo.functionName),
		                      CastVoidStar(
		                        op.funcstruct ?
		                        UOAddress(IdentName(oo.structName)) :
		                        numConstant(0)
		                      )
		                    )
		                  )
		                 );
		//Parentize
		ast_stmt_parent(parent, parent->body);
	}

	/* (5) Handle copyin variables
	 */
	copyininit = xp_handle_copyin(op.usedvars[DCT_BYREF], oo.structName);
	if (copyininit)
		out_insert_before(
		  op.functionbody,
		  BlockList(
		    BlockList(verbit("/* copyin initialization(s) */"), copyininit),
		    BarrierCall()
		  ));

	/* (6) Place a barrier at the end of the function
	 */
	out_insert_before(
	  op.returnstm,  /* ort_taskwait(2);  */
	  Labeled(
	    Symbol(clabel), /* label used for cancel */
	    FuncCallStmt(
	      IdentName("ort_taskwait"),
	      numConstant(2)
	    )
	  )
	);
}
