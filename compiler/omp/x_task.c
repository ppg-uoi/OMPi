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

/* x_task.c -- too big a transformation to be part of ast_xfrom */

#include "ast_xform.h"
#include "ast_copy.h"
#include "ast_renlabs.h"
#include "boolean.h"
#include "x_task.h"
#include "x_clauses.h"
#include "ompi.h"
#include "outline.h"

static vartype_t xt_implicitDefault(setelem(vars) s)
{
	stentry orig = symtab_get(stab, s->key, IDNAME);
	if ((orig->scopelevel == 0 && closest_parallel_scope < 0) ||
	    orig->scopelevel <= closest_parallel_scope ||
	    orig->isthrpriv)
		return DCT_BYREF;
	return DCT_BYVALUE;
}

void xform_task(aststmt *t)
{
	static int tasknum = 0;

	aststmt   body_copy, parent;
	aststmt   fast, fastdecls = NULL;                   /* For the fast path */
	astexpr   ifexpr = NULL, finalexpr = NULL, fastif;
	ompclause c;
	bool      untied, mergeable;
	hanpars_t hp = {NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	outcome_t op;
	char      clabel[22];

	static outpars_t oo =
	{
		"",                     //+functionName
		"ort_new_task",         // functionCall;
		NULL,                   //+extraParameters
		false,                  // byvalue_type (by copy)
		false,                  // global_byref_in_struct
		"__taskenv__",          // structName;
		"_tenv",                // structVariable;
		NULL,                   //+structInitializer;
		xt_implicitDefault,     // implicitDefault function;
		NULL                    // deviceexpr
	};

	/* The name of the label used for canceling. We use line number to avoid
	 * conflicts
	 */
	snprintf(clabel, 22, "CANCEL_task_%d", (*t)->u.omp->l);

	/* We insert the label before creating a copy, to prevent doing it twice and
	 * to make sure that the label gets renamed correctly (in case mergeable is
	 * present)
	 */
	if ((*t)->u.omp->body->type == COMPOUND)
		out_insert_after((*t)->u.omp->body->body,
		                 Labeled(Symbol(clabel), Expression(NULL)));
	else
		(*t)->u.omp->body = Compound(
		                      BlockList(
		                        (*t)->u.omp->body,
		                        Labeled(Symbol(clabel), Expression(NULL))
		                      )
		                    );

	body_copy = ast_stmt_copy((*t)->u.omp->body);         /* Keep a full copy */

	/* (1) Check for untied, mergeable, if and final clauses and keep a copy
	 */
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCIF)) != NULL)
		ifexpr = ast_expr_copy(c->u.expr);
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCFINAL)) != NULL)
		finalexpr = ast_expr_copy(c->u.expr);
	untied = (xc_ompcon_get_clause((*t)->u.omp, OCUNTIED) != NULL);
	mergeable = (xc_ompcon_get_clause((*t)->u.omp, OCMERGEABLE) != NULL);


	sprintf(oo.functionName, "_taskFunc%d_", tasknum++);
	/* (struct structType *) ort_taskenv_alloc(sizeof(struct), functionName); */
	oo.structInitializer =
	  CastedExpr(
	    Casttypename(
	      SUdecl(SPEC_struct, Symbol(oo.structType), NULL),
	      AbstractDeclarator(Pointer(), NULL)
	    ),
	    FunctionCall(
	      IdentName("ort_taskenv_alloc"),
	      CommaList(
	        Sizeoftype(Casttypename(
	                     SUdecl(SPEC_struct, Symbol(oo.structType), NULL),
	                     NULL
	                   )),
	        IdentName(oo.functionName)
	      )
	    )
	  );
	oo.extraParameters = CommaList(  /* <final expression>, tied(0)/untied(1) */
	                       finalexpr ? ast_expr_copy(finalexpr) : numConstant(0),
	                       numConstant(untied ? 1 : 0)
	                     );

	op = outline_OpenMP(t, oo); //TODO fix placement of comment

	/* (7) Also produce a "fast" path:
	 *       {
	 *         <possible local declarations / initializations>
	 *         ort_task_immediate_start(...)
	 *           <original code>
	 *         ort_task_immediate_end(...)
	 *       }
	 */
	parent = op.replacement->parent;

	//Declare the private variables
	out_handle_private(op.usedvars[DCT_PRIVATE], &hp);

	//Declare and initialize byvalue variables
	out_inline_byval(op.usedvars[DCT_BYVALUE], &hp);

	fast = BlockList(
	         Expression(    /* _itn = ort_task_immediate_start(<finalexpr>); */
	           Assignment(IdentName("_itn"), ASS_eq,
	                      FunctionCall(
	                        IdentName("ort_task_immediate_start"),
	                        finalexpr ? finalexpr : numConstant(0)
	                      ))
	         ),
	         BlockList(
	           body_copy,
	           Expression(  /* ort_task_immediate_end(_itn); */
	             FunctionCall(IdentName("ort_task_immediate_end"),
	                          IdentName("_itn"))
	           )
	         )
	       );

	if (hp.func_varDecls)
	{
		fastdecls = hp.func_varDecls;
		if (hp.func_varInits)
			fastdecls = BlockList(fastdecls, hp.func_varInits);
	}

	if (fastdecls)
		fast = BlockList(fastdecls, fast);

	/* void *_itn; */
	fast = BlockList(
	         Declaration(
	           Declspec(SPEC_void),
	           Declarator(Declspec(SPEC_star), IdentifierDecl(Symbol("_itn")))
	         ),
	         fast
	       );

	fast = Compound(fast);

	fastif = Call0_expr("ort_task_throttling");

	if (ifexpr)
		fastif = BinaryOperator(
		           BOP_lor,
		           UnaryOperator(UOP_lnot, Parenthesis(ifexpr)),
		           fastif
		         );

	/* (8) Combine slow & fast paths with a possibly even faster third path
	 */
	if (enableCodeDup && mergeable)
	{
		*t = If(Call0_expr("omp_in_final"),
		        Compound(
		          fastdecls ?
		          BlockList(ast_stmt_copy(fastdecls), ast_stmt_copy(body_copy)) :
		          ast_stmt_copy(body_copy)
		        ),
		        If(fastif, fast, op.replacement));
		//Rename labels on the first if block
		ast_stmt_renlabs((*t)->body);
	}
	else
	{
		fastif = BinaryOperator(
		           BOP_lor,
		           Call0_expr("omp_in_final"),
		           fastif
		         );
		*t = If(fastif, fast, op.replacement);
	}

	ast_stmt_parent(parent, *t);


	//If we used a struct for byref/byvalue variables call ort_taskenv_free
	if (op.funcstruct)
		out_insert_before(op.returnstm,
		                  FuncCallStmt(
		                    IdentName("ort_taskenv_free"),
		                    CommaList(
		                      IdentName(oo.structName),
		                      IdentName(oo.functionName)
		                    )
		                  )
		                 );
}

