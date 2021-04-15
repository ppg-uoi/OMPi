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

static autoshattr_t as;


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
	if (s->value.clause == OCAUTO)
		return xp_implicitDefaultAuto(s);
	return DCT_BYREF;
}


/* Returns the type of the proc_bind clause
 */
static int xp_procbind(ompcon t)
{
	ompclause c = xc_ompcon_get_unique_clause(t, OCPROCBIND);

	if (c == NULL)
		return omp_proc_bind_false; /* No proc_bind() clause */

	switch (c->subtype)
	{
		case OC_bindprimary:
			return omp_proc_bind_primary;
		case OC_bindmaster: /* Deprecated as of OpenMP v5.1 */
			return omp_proc_bind_master;
		case OC_bindclose:
			return omp_proc_bind_close;
		case OC_bindspread:
			return omp_proc_bind_spread;
		default:
			break;
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
		if (e->value.clause != OCCOPYIN)
			continue;

		stentry orig = symtab_get(stab, e->key, IDNAME);
		astexpr init;

		init = /* ( e->scopelevel == -10 ) ?
		         Identifier( tp_new_name(var) ) : */  /* The initializer/source */
		       DerefParen(PtrField(IdentName(structVariable), e->key));

		if (!orig->isarray)           /* Plain assignment */
			tmp = Expression(            /* *var = <orig_var> */
			        Assignment(
			          Deref(Identifier(e->key)),
			          ASS_eq,
			          init
			        )
			      );
		else                       /* Array assignment, like in firstprivates */
			tmp = xc_memcopy(Deref(Identifier(e->key)), init,
			                 Sizeof(Deref(Identifier(e->key))));

		st = (st) ? BlockList(st, tmp) : tmp;
	}

	return st;
}


void xform_parallel(aststmt *t, int iscombined)
{
	static int thrnum = 0;

	astexpr    numthrexpr = NULL, ifexpr = NULL;
	aststmt    copyininit;
	ompclause  c, def = xc_ompcon_get_clause((*t)->u.omp, OCDEFAULT);
	int        procbind_type;
	outcome_t  op;
	char       clabel[22];

	static outpars_t oo =
	{
		true,                   // structbased
		"",                     //+functionName
		"ort_execute_parallel", // functionCall
		NULL,                   //+extraParameters
		BYVAL_byname,           // byvalue_type (by name)
		false,                  //+global_byref_in_struct
		"__shvt__",             // structName
		"_shvars",              // structVariable
		NULL,                   // structInitializer
		xp_implicitDefault,     // implicitDefault function
		NULL,                   // deviceexpr
		true,                   // addComment
		NULL                    // thestmt
	};
	
	/* The name of the label used for canceling. We use line number to avoid
	 * conflicts (there shouldn't be any since the code is outlined but we
	 * use it anyway incase we inline the parallel code in the future)
	 */
	snprintf(clabel, 22, "CANCEL_parallel_%d", (*t)->l);

	if (enableAutoscope) /* Agelos */
	{
		assert(dfa_parreg_get_results((*t)->l) != NULL);
		as = *dfa_parreg_get_results((*t)->l);
		if (def && def->subtype == OC_auto)
			oo.implicitDefault = xp_implicitDefaultAuto;
	}

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
	oo.extraParameters = Comma3(  // <numthread>, combined, <procbind>
	                       numthrexpr ? numthrexpr : numConstant(-1),
	                       numConstant(iscombined),
	                       numConstant(procbind_type)
	                     );
	//If we are in a target region add global shared variables to the struct
	if (inTarget())
		oo.global_byref_in_struct = true;
	else
		oo.global_byref_in_struct = false;
	oo.thestmt = *t;
	op = outline_OpenMP(t, oo);

	/* (4) Add if clause
	 */
	if (ifexpr)                    /* Check if we have an if() clause */
	{
		aststmt parent = op.repl_funcall->parent;
		aststmt new = If(ifexpr, ast_stmt_copy(op.repl_funcall),
		                  /* ort_execute_serial(thrFunc, (void *) __shvt__/0); */
		                  FuncCallStmt(
		                    IdentName("ort_execute_serial"),
		                    CommaList(
		                      IdentName(oo.functionName),
		                      CastVoidStar(
		                        op.func_struct ?
		                        UOAddress(IdentName(oo.structName)) :
		                        numConstant(0)
		                      )
		                    )
		                  )
		                 );
		*(op.repl_funcall) = *new;
		ast_stmt_parent(parent, op.repl_funcall);

		//Replace call with if(ifexpr) execute_parallel else execute_serial
		//WARNING replacing parent->body can cause problems, for example if
		//parent is IF or STATEMENTLIST we may not be the body of our parent
		//In this case if there are any shared/firstprivate variables the parent
		//is a STATEMENTLIST with us as body, else it is a COMPOUND
// 		parent->body = If(ifexpr, op.repl_funcall,
// 		                  /* ort_execute_serial(thrFunc, (void *) __shvt__/0); */
// 		                  FuncCallStmt(
// 		                    IdentName("ort_execute_serial"),
// 		                    CommaList(
// 		                      IdentName(oo.functionName),
// 		                      CastVoidStar(
// 		                        op.func_struct ?
// 		                        UOAddress(IdentName(oo.structName)) :
// 		                        numConstant(0)
// 		                      )
// 		                    )
// 		                  )
// 		                 );
// 		//Parentize
// 		ast_stmt_parent(parent, parent->body);
	}

	/* (5) Handle copyin variables
	 */
	copyininit = xp_handle_copyin(op.usedvars[DCT_BYREF], oo.structName);
	if (copyininit)
		ast_stmt_prepend(
		  op.func_regcode,
		  Block3(
		    verbit("/* copyin initialization(s) */"), copyininit, BarrierCall()
		  ));

	/* (6) Place a barrier at the end of the function
	 */
	ast_stmt_prepend(
	  op.func_return,  /* ort_taskwait(2);  */
	  Labeled(
	    Symbol(clabel), /* label used for cancel */
	    FuncCallStmt(
	      IdentName("ort_taskwait"),
	      numConstant(2)
	    )
	  )
	);
}
