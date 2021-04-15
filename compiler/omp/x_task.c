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

/* x_task.c -- too big a transformation to be part of ast_xfrom */

#include "ast_xform.h"
#include "ast_copy.h"
#include "ast_free.h"
#include "ast_renlabs.h"
#include "x_task.h"
#include "x_clauses.h"
#include "ompi.h"
#include "outline.h"

#define DEPARRAY "_deps"

static vartype_t xt_implicitDefault(setelem(vars) s)
{
	stentry orig = symtab_get(stab, s->key, IDNAME);
	if ((orig->scopelevel == 0 && closest_parallel_scope < 0) ||
	    orig->scopelevel <= closest_parallel_scope ||
	    orig->isthrpriv)
		return DCT_BYREF;
	return DCT_BYVALUE;
}


/* Produces an array initialized with the dependence addresses */
static
aststmt xt_assemble_deps(ompcon t, int *nin, int *nout, int *ninout)
{
	static set(xlitems) indeps = NULL, outdeps = NULL, inoutdeps = NULL;
	set(xlitems)        big;
	setelem(xlitems)    e;
	astexpr             inits = NULL;

	set_init(xlitems, &indeps);
	set_init(xlitems, &outdeps);
	set_init(xlitems, &inoutdeps);
	
	xc_ompcon_get_xlitems(t, OCDEPEND, OC_in, indeps);
	xc_ompcon_get_xlitems(t, OCDEPEND, OC_out, outdeps);
	xc_ompcon_get_xlitems(t, OCDEPEND, OC_inout, inoutdeps);
	
	*nin    = set_size(indeps);
	*nout   = set_size(outdeps);
	*ninout = set_size(inoutdeps);
	if (*nout + *nin + *ninout == 0)
		return NULL;
	
	if (*nout)
	{
		e = outdeps->first;
		inits = xc_xlitem_baseaddress(e->value.xl);
		e = e->next;
		for (; e; e = e->next)
			inits = CommaList(inits, xc_xlitem_baseaddress(e->value.xl));
	}
	if (*nin)
	{
		e = indeps->first;
		if (!inits)
		{
			inits = xc_xlitem_baseaddress(e->value.xl);
			e = e->next;
		}
		for (; e; e = e->next)
			inits = CommaList(inits, xc_xlitem_baseaddress(e->value.xl));
	}
	if (*ninout)
	{
		e = inoutdeps->first;
		if (!inits)
		{
			inits = xc_xlitem_baseaddress(e->value.xl);
			e = e->next;
		}
		for (; e; e = e->next)
			inits = CommaList(inits, xc_xlitem_baseaddress(e->value.xl));
	}
	inits = BracedInitializer(inits);
	
	/* void *DEPARRAY[] = <inits>; */
	return 
		Declaration(
		  Declspec(SPEC_void), 
		  InitDecl(
		    Declarator(
		      Declspec(SPEC_star), 
		      ArrayDecl(IdentifierDecl(Symbol(DEPARRAY)), NULL, NULL)
		    ), 
		    inits
		  )
		);
}


/* Generate minimal, non-optimized tasking code 
 * @param ifexpr    Copy of the expression in the if() clause, if any. 
 *                  Must be freed if not needed.
 * @param finalexpr Copy of the expression in the final() clause, if any.
 *                  Must be freed if not needed.
 * @param mergeable True of the construct has a mergeable clause.
 * @param hasdeps   True if the construct has dependence clauses.
 * @param body      Copy of the construct body. 
 *                  Must be freed if not needed.
 * 
 * @return The final statment for the replacemnt code.
 */
static
aststmt gencode_noopt(astexpr ifexpr, astexpr finalexpr, bool mergeable,
                      bool hasdeps, aststmt body, outcome_t *op)
{
	if (ifexpr) ast_expr_free(ifexpr);
	if (finalexpr) ast_expr_free(finalexpr);
	if (body) ast_stmt_free(body);
	return ( op->replacement ); 
}


/* Generate optimized, fast tasking code; con: code duplication
 * @param ifexpr    Copy of the expression in the if() clause, if any. 
 *                  Must be freed if not needed.
 * @param finalexpr Copy of the expression in the final() clause, if any.
 *                  Must be freed if not needed.
 * @param mergeable True of the construct has a mergeable clause.
 * @param hasdeps   True if the construct has dependence clauses.
 * @param body      Copy of the construct body. 
 *                  Must be freed if not needed.
 * 
 * @return The final statment for the replacemnt code.
 */
static
aststmt gencode_fast(astexpr ifexpr, astexpr finalexpr, bool mergeable,
                      bool hasdeps, aststmt body, outcome_t *op)
{
	aststmt fast, res, vardecls, varinits;
	astexpr infinalexpr, fastifcond;
	
	/* Save omp_in_final() value only in the ultra-fast case */
	infinalexpr = mergeable ? IdentName("_ompinfinal"):Call0_expr("omp_in_final");

	/* Here is the "fast" path:
	 *   {
	 *     <possible local declarations / initializations>
	 *     if (needed)
	 *       ort_task_immediate_start(...)
	 *     <original code (body)>
	 *     if (needed)
	 *       ort_task_immediate_end(...)
	 *   }
	 */
	if (!mergeable)
		fast = Block3(
		         Expression(    /* _itn = ort_task_immediate_start(<finalexpr>); */
		           Assignment(IdentName("_itn"), ASS_eq,
		                      FunctionCall(
		                        IdentName("ort_task_immediate_start"),
		                        finalexpr ? finalexpr : numConstant(0)
		                      ))
		         ),
		         body,
		         Expression(  /* ort_task_immediate_end(_itn); */
		           FunctionCall(IdentName("ort_task_immediate_end"),
		                        IdentName("_itn"))
		         )
		       );
	else /* mergeable */
		if (hasdeps && !ifexpr)
			fast = body;
		else
			fast = Block3(
			         If(
			           UnaryOperator(UOP_lnot, ast_expr_copy(infinalexpr)),
			           Expression( /* _itn = ort_task_immediate_start(<finalexpr>); */
			             Assignment(IdentName("_itn"), ASS_eq,
			                        FunctionCall(
			                          IdentName("ort_task_immediate_start"),
			                          finalexpr ? finalexpr : numConstant(0)
			                        ))
			           ),
			           NULL
			         ),
			         body,
			         If(
			           UnaryOperator(UOP_lnot, ast_expr_copy(infinalexpr)),
			           Expression( /* ort_task_immediate_end(_itn); */
			             FunctionCall(IdentName("ort_task_immediate_end"),
			                          IdentName("_itn"))
			           ),
			           NULL
			         )
			       );
	
	/* When we have dependencies, immediate execution of the task
	 * can only be started when all dependencies are satisfied. 
	 * We guarantee this with an explicit #taskwait for my child tasks
	 * (this task will be a sibling of my other tasks...).
	 */
	if (ifexpr && hasdeps)
		fast = BlockList(
		         If(UnaryOperator(UOP_lnot, ast_expr_copy(infinalexpr)),
		            FuncCallStmt(IdentName("ort_taskwait"), numConstant(0)),
		            NULL),
		         fast
		       );
	
	/* Insert local declarations in the top of the fast path */
	vardecls = out_inline_firstprivate(op->usedvars[DCT_BYVALUE], &varinits);
	if (vardecls)
	{
		if (varinits)
			vardecls = BlockList(vardecls, varinits);
		fast = BlockList(vardecls, fast);
	}
	vardecls = out_inline_private(op->usedvars[DCT_PRIVATE]);
	if (vardecls)
		fast = BlockList(vardecls, fast);

	/* Insert declaration: "void *_itn;" */
	if (!mergeable || !hasdeps || ifexpr)
		fast = BlockList(
		         Declaration(
		           Declspec(SPEC_void),
		           Declarator(Declspec(SPEC_star), IdentifierDecl(Symbol("_itn")))
		         ),
		         fast
		       );

	/* Compound */
	fast = Compound(fast);

	/* Form the selection condition in order to combine slow & fast paths */
	fastifcond = ast_expr_copy(infinalexpr);
	if (ifexpr)
		fastifcond = BinaryOperator(
		               BOP_lor,
		               UnaryOperator(UOP_lnot, Parenthesis(ifexpr)),
		               fastifcond
		             );
	if (!hasdeps)             /* Don't check for throttling otherwise */
		fastifcond = BinaryOperator(
		               BOP_lor,
		               fastifcond,
		               Call0_expr("ort_task_throttling")
		             );
	res = If(fastifcond, fast, op->replacement);

	/* If needed, insert the infinal value at the top */
	if (mergeable)
		res = Compound(
		       BlockList(
		         Declaration(
		           Declspec(SPEC_int),
		           InitDecl(
		             Declarator(NULL, IdentifierDecl(Symbol("_ompinfinal"))),
		             Call0_expr("omp_in_final")
		           )
		         ),
		         res
		       )
		     );
		
	ast_expr_free(infinalexpr);
	return res;
}


/* Generate optimized, ultra fast tasking code; con: possible code triplication
 * @param ifexpr    Copy of the expression in the if() clause, if any. 
 *                  Must be freed if not needed.
 * @param finalexpr Copy of the expression in the final() clause, if any.
 *                  Must be freed if not needed.
 * @param mergeable True of the construct has a mergeable clause.
 * @param hasdeps   True if the construct has dependence clauses.
 * @param body      Copy of the construct body. 
 *                  Must be freed if not needed.
 * 
 * @return The final statment for the replacemnt code.
 */
static
aststmt gencode_ufast(astexpr ifexpr, astexpr finalexpr, bool mergeable,
                      bool hasdeps, aststmt body, outcome_t *op)
{
	aststmt ufast, fast, vardecls, varinits;
	astexpr fastifcond;
	
	/* We differ only when a mergeable clause is present; even in this case,
	 * we require an if clause or no dependencies.
	 */ 
	if (!mergeable || (hasdeps && !ifexpr))
		return gencode_fast(ifexpr, finalexpr, mergeable, hasdeps, body, op);
	
	/* Here is the code:
	 *   if (ufastifcond)
	 *     <ufastpath>
	 *   else if (fastifcond)
	 *     <fastpath>
	 *   else
	 *     <normalpath>
	 * 
	 * where ufastifcond is "if (omp_in_final())" and fastifcond depends
	 * on the presence of an if clause and dependencies.
	 * 
	 * Here is the "fast" path:
	 *   {
	 *     <possible local declarations / initializations>
	 *     ort_task_immediate_start(...)
	 *     <original code (body)>
	 *     ort_task_immediate_end(...)
	 *   }
	 * 
	 * Here is the "ultra fast" path (no declarations at all):
	 *   {
	 *     <original code (body)>
	 *   }
	 */
	ufast = ast_stmt_copy(body);
	ast_stmt_renlabs(ufast);       /* Rename labels */
	
	fast = Block3(
	         Expression( /* _itn = ort_task_immediate_start(<finalexpr>); */
		         Assignment(
		           IdentName("_itn"), 
		           ASS_eq,
		           FunctionCall(IdentName("ort_task_immediate_start"),
		                        finalexpr ? finalexpr : numConstant(0))
		         )
		       ),
		       body,
		       Expression( /* _itn = ort_task_immediate_end(<_itn>); */
		         FunctionCall(IdentName("ort_task_immediate_end"),
		                      IdentName("_itn"))
		       )
		     );
	/* Add a taskwait() if needed */
	if (hasdeps && ifexpr)
		fast = BlockList(
		         FuncCallStmt(IdentName("ort_taskwait"), numConstant(0)),
		         fast
		       );

	/* Insert local declarations in the top of the fast path */
	vardecls = out_inline_firstprivate(op->usedvars[DCT_BYVALUE], &varinits);
	if (vardecls)
	{
		if (varinits)
			vardecls = BlockList(vardecls, varinits);
		fast = BlockList(vardecls, fast);
	}
	vardecls = out_inline_private(op->usedvars[DCT_PRIVATE]);
	if (vardecls)
		fast = BlockList(vardecls, fast);
	
	/* Insert the declaration: "void *_itn;" in the fast path */
	fast = BlockList(
	         Declaration(
	           Declspec(SPEC_void),
	           Declarator(Declspec(SPEC_star), IdentifierDecl(Symbol("_itn")))
	         ),
	         fast
	       );

	/* Compounds */
	ufast = Compound(ufast);
	fast = Compound(fast);

	/* Form the selection condition in order to drive the fast path */
	if (hasdeps)
		fastifcond = UnaryOperator(UOP_lnot, Parenthesis(ifexpr));
	else if (!ifexpr)
		fastifcond = Call0_expr("ort_task_throttling");
	else
		fastifcond = BinaryOperator(
		               BOP_lor,
		               UnaryOperator(UOP_lnot, Parenthesis(ifexpr)),
		               Call0_expr("ort_task_throttling")
		             );
	
	return
		If(Call0_expr("omp_in_final"),
			ufast,
			If(fastifcond,
				fast,
				op->replacement
			)
		);
}


void xform_task(aststmt *t, taskopt_e opt)
{
	static int tasknum = 0;

	aststmt   body_copy, parent, v;
	aststmt   deparray = NULL;                          /* For dependencies */
	astexpr   ifexpr = NULL, finalexpr = NULL, prioexpr = NULL;
	ompclause c;
	bool      untied, mergeable;
	int       nindeps, noutdeps, ninoutdeps;
	outcome_t op;
	char      clabel[22];

	static outpars_t oo =
	{
		true,                   // structbased
		"",                     //+functionName
		"ort_new_task",         // functionCall;
		NULL,                   //+extraParameters
		BYVAL_bycopy,           // byvalue_type (by copy)
		false,                  // global_byref_in_struct
		"__taskenv__",          // structName;
		"_tenv",                // structVariable;
		NULL,                   //+structInitializer;
		xt_implicitDefault,     // implicitDefault function;
		NULL,                   // deviceexpr
		false,                  // addComment
		NULL                    // thestmt
	};

	v = ompdir_commented((*t)->u.omp->directive); /* Put directive in comments */
	
	/* The name of the label used for canceling. We use line number to avoid
	 * conflicts
	 */
	snprintf(clabel, 22, "CANCEL_task_%d", (*t)->u.omp->l);

	/* We insert the label before creating a copy, to prevent doing it twice and
	 * to make sure that the label gets renamed correctly (in case mergeable is
	 * present)
	 */
	if ((*t)->u.omp->body->type == COMPOUND)
		ast_stmt_append((*t)->u.omp->body->body,
		                 Labeled(Symbol(clabel), Expression(NULL)));
	else
		(*t)->u.omp->body = Compound(
		                      BlockList(
		                        (*t)->u.omp->body,
		                        Labeled(Symbol(clabel), Expression(NULL))
		                      )
		                    );

	body_copy = ast_stmt_copy((*t)->u.omp->body);         /* Keep a full copy */

	/* (1) Take care of special clauses:
	 *     - Check for untied, mergeable, if and final clauses and keep a copy
	 *     - Prepare a dependence array, if corresponding clauses are present
	 */
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCIF)) != NULL)
		ifexpr = ast_expr_copy(c->u.expr);
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCFINAL)) != NULL)
		finalexpr = ast_expr_copy(c->u.expr);
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCPRIORITY)) != NULL)
		prioexpr = ast_expr_copy(c->u.expr);
	untied = (xc_ompcon_get_clause((*t)->u.omp, OCUNTIED) != NULL);
	mergeable = (xc_ompcon_get_clause((*t)->u.omp, OCMERGEABLE) != NULL);
	deparray = xt_assemble_deps((*t)->u.omp, &nindeps, &noutdeps, &ninoutdeps);

	/* (2) Outline
	 */
	sprintf(oo.functionName, "_taskFunc%d_", tasknum++);
	/* (struct structType *) ort_taskenv_alloc(sizeof(struct), functionName); */
	oo.structInitializer =
	  CastedExpr(
	    Casttypename(
	      SUdecl(SPEC_struct, Symbol(oo.structType), NULL, NULL),
	      AbstractDeclarator(Pointer(), NULL)
	    ),
	    FunctionCall(
	      IdentName("ort_taskenv_alloc"),
	      CommaList(
	        Sizeoftype(Casttypename(
	                     SUdecl(SPEC_struct, Symbol(oo.structType), NULL, NULL),
	                     NULL
	                   )),
	        IdentName(oo.functionName)
	      )
	    )
	  );
	oo.extraParameters = 
		Comma8(
			ifexpr ? ast_expr_copy(ifexpr) : numConstant(0),       /* if expression */
			finalexpr ? ast_expr_copy(finalexpr) : numConstant(0),/* finalxpression */
			numConstant(untied ? 1 : 0),                       /* tied(0)/untied(1) */
			prioexpr ? ast_expr_copy(prioexpr) : numConstant(0),   /* priority expr */
			deparray ? Identifier(Symbol(DEPARRAY)) : NullExpr(),/* dependece array */
			numConstant(noutdeps),                                      /* #outdeps */
			numConstant(nindeps),                                        /* #indeps */
			numConstant(ninoutdeps)                                   /* #inoutdeps */
		);
	oo.thestmt = *t;
	
	op = outline_OpenMP(t, oo);
	
	if (deparray)        /* Add dependence array after the struct declaration */
	{
		if (op.repl_struct)
			ast_stmt_append(op.repl_struct, deparray);
		else 
			/* Ideal:  ast_compound_insert_statement(op.replacement, deparray);
			 * Fact:   op.replacement is not always a compound.
			 * Result:
			 */
			ast_stmt_prepend(op.repl_funcall, deparray);
	}
	
	parent = op.replacement->parent;

	/* (3) Generate the code 
	 */
	switch (opt)
	{
		case OPT_NONE: 
			*t = gencode_noopt(ifexpr, finalexpr, mergeable, deparray != NULL,
			                   body_copy, &op);
			break;
		case OPT_FAST:
			*t = gencode_fast(ifexpr, finalexpr, mergeable, deparray != NULL,
			                   body_copy, &op);
			break;
		case OPT_ULTRAFAST:
			*t = gencode_ufast(ifexpr, finalexpr, mergeable, deparray != NULL,
			                   body_copy, &op);
			break;
	}
	*t = BlockList(v, *t);         /* Add comment */
	ast_stmt_parent(parent, *t);

	//If we used a struct for byref/byvalue variables call ort_taskenv_free
	if (op.func_struct)
		ast_stmt_prepend(op.func_return,
		                  FuncCallStmt(
		                    IdentName("ort_taskenv_free"),
		                    CommaList(
		                      IdentName(oo.structName),
		                      IdentName(oo.functionName)
		                    )
		                  )
		                 );
}
