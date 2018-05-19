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

/* ast_traverse.c -- generic traversal of the AST using callbacks @ each node */

/* While, in many cases, a module needs to be in full control of the 
 * AST traversal, in other cases only a portion of the AST is required.
 * For the latter, this should be useful.
 * Lines saved: 
 *   ast_vars > 350, ast_renlabs > 130, ast_free > 400, ...
 */

#include "ast_traverse.h"


#define visit_stmt(trop,f,t);\
	{	if ((trop)->stmtc.f) ((trop)->stmtc.f)(t,(trop)->starg,PREVISIT);	}
#define pre_visit_stmt(trop,f,t);\
	{	if ((trop)->stmtc.f && (trop)->when & PREVISIT)\
			((trop)->stmtc.f)(t,(trop)->starg,PREVISIT);	}
#define post_visit_stmt(trop,f,t);\
	{	if ((trop)->stmtc.f && (trop)->when & POSTVISIT)\
			((trop)->stmtc.f)(t,(trop)->starg,POSTVISIT);	}
#define mid_visit_stmt(trop,f,t);\
	{	if ((trop)->stmtc.f && (trop)->when & MIDVISIT)\
			((trop)->stmtc.f)(t,(trop)->starg,MIDVISIT);	}


void ast_stmt_jump_traverse(aststmt tree, travopts_t *trop)
{
	switch (tree->subtype)
	{
		case SBREAK:
			visit_stmt(trop, break_c, tree);
			break;
		case SCONTINUE:
			visit_stmt(trop, continue_c, tree);
			break;
		case SRETURN:
			pre_visit_stmt(trop, return_c, tree);
			if (tree->u.expr != NULL)
				ast_expr_traverse(tree->u.expr, trop);
			post_visit_stmt(trop, return_c, tree);
			break;
		case SGOTO:
			visit_stmt(trop, goto_c, tree);
			break;
		default:
			fprintf(stderr, "[ast_stmt_jump_traverse]: b u g !!\n");
	}
}


void ast_stmt_iteration_traverse(aststmt tree, travopts_t *trop)
{
	switch (tree->subtype)
	{
		case SFOR:
			pre_visit_stmt(trop, for_c, tree);
			if (tree->u.iteration.init != NULL)
			{
				if (tree->u.iteration.init->type == EXPRESSION)
				{
					if (tree->u.iteration.init->u.expr != NULL)
						ast_expr_traverse(tree->u.iteration.init->u.expr, trop);
				}
				else          /* Declaration */
				{
					ast_spec_traverse(tree->u.iteration.init->u.declaration.spec, trop);
					if (tree->u.iteration.init->u.declaration.decl)
						ast_decl_traverse(tree->u.iteration.init->u.declaration.decl, trop);
				}
			}
			if (tree->u.iteration.cond != NULL)
				ast_expr_traverse(tree->u.iteration.cond, trop);
			if (tree->u.iteration.incr != NULL)
				ast_expr_traverse(tree->u.iteration.incr, trop);
			ast_stmt_traverse(tree->body, trop);
			post_visit_stmt(trop, for_c, tree);
			break;
		case SWHILE:
			pre_visit_stmt(trop, while_c, tree);
			ast_expr_traverse(tree->u.iteration.cond, trop);
			ast_stmt_traverse(tree->body, trop);
			post_visit_stmt(trop, while_c, tree);
			break;
		case SDO:
			pre_visit_stmt(trop, do_c, tree);
			ast_stmt_traverse(tree->body, trop);
			ast_expr_traverse(tree->u.iteration.cond, trop);
			post_visit_stmt(trop, do_c, tree);
			break;
		default:
			fprintf(stderr, "[ast_stmt_iteration_traverse]: b u g !!\n");
	}
}


void ast_stmt_selection_traverse(aststmt tree, travopts_t *trop)
{
	switch (tree->subtype)
	{
		case SSWITCH:
			pre_visit_stmt(trop, switch_c, tree);
			ast_expr_traverse(tree->u.selection.cond, trop);
			ast_stmt_traverse(tree->body, trop);
			post_visit_stmt(trop, switch_c, tree);
			break;
		case SIF:
			pre_visit_stmt(trop, if_c, tree);
			ast_expr_traverse(tree->u.selection.cond, trop);
			ast_stmt_traverse(tree->body, trop);
			if (tree->u.selection.elsebody)
				ast_stmt_traverse(tree->u.selection.elsebody, trop);
			post_visit_stmt(trop, if_c, tree);
			break;
		default:
			fprintf(stderr, "[ast_stmt_selection_traverse]: b u g !!\n");
	}
}


void ast_stmt_labeled_traverse(aststmt tree, travopts_t *trop)
{
	switch (tree->subtype)
	{
		case SLABEL:
			pre_visit_stmt(trop, label_c, tree);
			ast_stmt_traverse(tree->body, trop);
			post_visit_stmt(trop, label_c, tree);
			break;
		case SCASE:
			pre_visit_stmt(trop, case_c, tree);
			ast_expr_traverse(tree->u.expr, trop);
			ast_stmt_traverse(tree->body, trop);
			post_visit_stmt(trop, case_c, tree);
			break;
		case SDEFAULT:
			pre_visit_stmt(trop, default_c, tree);
			ast_stmt_traverse(tree->body, trop);
			post_visit_stmt(trop, default_c, tree);
			break;
		default:
			fprintf(stderr, "[ast_stmt_labeled_traverse]: b u g !!\n");
	}
}


void ast_stmt_traverse(aststmt tree, travopts_t *trop)
{
	if (tree == NULL) return;
	
	switch (tree->type)
	{
		case JUMP:
			ast_stmt_jump_traverse(tree, trop);
			break;
		case ITERATION:
			ast_stmt_iteration_traverse(tree, trop);
			break;
		case SELECTION:
			ast_stmt_selection_traverse(tree, trop);
			break;
		case LABELED:
			ast_stmt_labeled_traverse(tree, trop);
			break;
		case EXPRESSION:
			pre_visit_stmt(trop, expression_c, tree);
			if (tree->u.expr != NULL)
				ast_expr_traverse(tree->u.expr, trop);
			post_visit_stmt(trop, expression_c, tree);
			break;
		case COMPOUND:
			pre_visit_stmt(trop, compound_c, tree);
			if (tree->body)
				ast_stmt_traverse(tree->body, trop);
			post_visit_stmt(trop, compound_c, tree);
			break;
		case STATEMENTLIST:
			pre_visit_stmt(trop, stmtlist_c, tree);
			ast_stmt_traverse(tree->u.next, trop);
			ast_stmt_traverse(tree->body, trop);
			post_visit_stmt(trop, stmtlist_c, tree);
			break;
		case DECLARATION:
			pre_visit_stmt(trop, declaration_c, tree);
			ast_spec_traverse(tree->u.declaration.spec, trop);
			mid_visit_stmt(trop, declaration_c, tree);
			if (tree->u.declaration.decl)
				ast_decl_traverse(tree->u.declaration.decl, trop);
			post_visit_stmt(trop, declaration_c, tree);
			break;
		case FUNCDEF:
			pre_visit_stmt(trop, funcdef_c, tree);
			if (tree->u.declaration.spec)
				ast_spec_traverse(tree->u.declaration.spec, trop);
			ast_decl_traverse(tree->u.declaration.decl, trop);
			if (tree->u.declaration.dlist)
				ast_stmt_traverse(tree->u.declaration.dlist, trop);
			ast_stmt_traverse(tree->body, trop);
			post_visit_stmt(trop, funcdef_c, tree);
			break;
		case OMPSTMT:
			ast_ompcon_traverse(tree->u.omp, trop);
			break;
		case VERBATIM:
			visit_stmt(trop, verbatim_c, tree);
			break;
		case OX_STMT:
			ast_oxcon_traverse(tree->u.ox, trop);
			break;
		default:
			fprintf(stderr, "[ast_stmt_traverse]: b u g !!\n");
	}
}


#define visit_expr(trop,f,t);\
	{	if ((trop)->exprc.f) ((trop)->exprc.f)(t,(trop)->starg,PREVISIT);	}
#define pre_visit_expr(trop,f,t);\
	{	if ((trop)->exprc.f && (trop)->when & PREVISIT)\
			((trop)->exprc.f)(t,(trop)->starg,PREVISIT);	}
#define post_visit_expr(trop,f,t);\
	{	if ((trop)->exprc.f && (trop)->when & POSTVISIT)\
			((trop)->exprc.f)(t,(trop)->starg,POSTVISIT);	}
#define mid_visit_expr(trop,f,t);\
	{	if ((trop)->exprc.f && (trop)->when & MIDVISIT)\
			((trop)->exprc.f)(t,(trop)->starg,MIDVISIT);	}


void ast_expr_traverse(astexpr tree, travopts_t *trop)
{
	if (!trop->doexpr)
		return;
	
	switch (tree->type)
	{
		case IDENT:
			visit_expr(trop, ident_c, tree);
			break;
		case CONSTVAL:
			visit_expr(trop, constval_c, tree);
			break;
		case STRING:
			visit_expr(trop, string_c, tree);
			break;
		case FUNCCALL:
			pre_visit_expr(trop, funccall_c, tree);
			ast_expr_traverse(tree->left, trop);
			if (tree->right)
				ast_expr_traverse(tree->right, trop);
			post_visit_expr(trop, funccall_c, tree);
			break;
		case ARRAYIDX:
			pre_visit_expr(trop, arrayidx_c, tree);
			ast_expr_traverse(tree->left, trop);
			ast_expr_traverse(tree->right, trop);
			post_visit_expr(trop, arrayidx_c, tree);
			break;
		case DOTFIELD:
			pre_visit_expr(trop, dotfield_c, tree);
			ast_expr_traverse(tree->left, trop);
			post_visit_expr(trop, dotfield_c, tree);
			break;
		case PTRFIELD:
			pre_visit_expr(trop, ptrfield_c, tree);
			ast_expr_traverse(tree->left, trop);
			post_visit_expr(trop, ptrfield_c, tree);
			break;
		case BRACEDINIT:
			pre_visit_expr(trop, bracedinit_c, tree);
			ast_expr_traverse(tree->left, trop);
			post_visit_expr(trop, bracedinit_c, tree);
			break;
		case CASTEXPR:
			pre_visit_expr(trop, castexpr_c, tree);
			ast_decl_traverse(tree->u.dtype, trop);
			ast_expr_traverse(tree->left, trop);
			post_visit_expr(trop, castexpr_c, tree);
			break;
		case CONDEXPR:
			pre_visit_expr(trop, condexpr_c, tree);
			ast_expr_traverse(tree->u.cond, trop);
			ast_expr_traverse(tree->left, trop);
			ast_expr_traverse(tree->right, trop);
			post_visit_expr(trop, condexpr_c, tree);
			break;
		case UOP:
			pre_visit_expr(trop, uop_c, tree);
			if (tree->opid == UOP_sizeoftype || tree->opid == UOP_typetrick)
				ast_decl_traverse(tree->u.dtype, trop);
			else
				ast_expr_traverse(tree->left, trop);
			post_visit_expr(trop, uop_c, tree);
			break;
		case BOP:
			pre_visit_expr(trop, bop_c, tree);
			ast_expr_traverse(tree->left, trop);
			ast_expr_traverse(tree->right, trop);
			post_visit_expr(trop, bop_c, tree);
			break;
		case PREOP:
			pre_visit_expr(trop, preop_c, tree);
			ast_expr_traverse(tree->left, trop);
			post_visit_expr(trop, preop_c, tree);
			break;
		case POSTOP:
			pre_visit_expr(trop, postop_c, tree);
			ast_expr_traverse(tree->left, trop);
			post_visit_expr(trop, postop_c, tree);
			break;
		case ASS:
			pre_visit_expr(trop, ass_c, tree);
			ast_expr_traverse(tree->left, trop);
			mid_visit_expr(trop, ass_c, tree);
			ast_expr_traverse(tree->right, trop);
			post_visit_expr(trop, ass_c, tree);
			break;
		case DESIGNATED:
			pre_visit_expr(trop, designated_c, tree);
			ast_expr_traverse(tree->left, trop);
			ast_expr_traverse(tree->right, trop);
			post_visit_expr(trop, designated_c, tree);
			break;
		case IDXDES:
			pre_visit_expr(trop, idxdes_c, tree);
			ast_expr_traverse(tree->left, trop);
			post_visit_expr(trop, idxdes_c, tree);
			break;
		case DOTDES:
			visit_expr(trop, dotdes_c, tree);
			break;
		case COMMALIST:
		case SPACELIST:
			pre_visit_expr(trop, list_c, tree);
			ast_expr_traverse(tree->left, trop);
			ast_expr_traverse(tree->right, trop);
			post_visit_expr(trop, list_c, tree);
			break;
		default:
			fprintf(stderr, "[ast_expr_traverse]: b u g !!\n");
	}
}


#define visit_spec(trop,f,t);\
	{	if ((trop)->specc.f) ((trop)->specc.f)(t,(trop)->starg,PREVISIT);	}
#define pre_visit_spec(trop,f,t);\
	{	if ((trop)->specc.f && (trop)->when & PREVISIT)\
			((trop)->specc.f)(t,(trop)->starg,PREVISIT);	}
#define post_visit_spec(trop,f,t);\
	{	if ((trop)->specc.f && (trop)->when & POSTVISIT)\
			((trop)->specc.f)(t,(trop)->starg,POSTVISIT);	}


void ast_spec_traverse(astspec tree, travopts_t *trop)
{
	if (!trop->dospec)
		return;
	
	switch (tree->type)
	{
		case SPEC:
		case STCLASSSPEC:
			visit_spec(trop, spec_c, tree);
			break;
		case USERTYPE:
			visit_spec(trop, usertype_c, tree);
			break;
		case SUE:
			switch (tree->subtype)
			{
				case SPEC_enum:
					pre_visit_spec(trop, specenum_c, tree);
					if (tree->body)
						ast_spec_traverse(tree->body, trop);
					post_visit_spec(trop, specenum_c, tree);
					break;
				case SPEC_struct:
					pre_visit_spec(trop, specstruct_c, tree);
					if (tree->u.decl)
						ast_decl_traverse(tree->u.decl, trop);
					post_visit_spec(trop, specstruct_c, tree);
					break;
				case SPEC_union:
					pre_visit_spec(trop, specunion_c, tree);
					if (tree->u.decl)
						ast_decl_traverse(tree->u.decl, trop);
					post_visit_spec(trop, specunion_c, tree);
					break;
				default:
					fprintf(stderr, "[ast_spec_traverse]: SUE b u g !!\n");
			}
			break;
		case ENUMERATOR:
			pre_visit_spec(trop, enumerator_c, tree);
			if (tree->u.expr)
				ast_expr_traverse(tree->u.expr, trop);
			post_visit_spec(trop, enumerator_c, tree);
			break;
		case SPECLIST:
			pre_visit_spec(trop, speclist_c, tree);
			switch (tree->subtype)
			{
				case SPEC_Rlist:
					ast_spec_traverse(tree->body, trop);
					ast_spec_traverse(tree->u.next, trop);
					break;
				case SPEC_Llist:
					ast_spec_traverse(tree->u.next, trop);
					ast_spec_traverse(tree->body, trop);
					break;
				case SPEC_enum:
					ast_spec_traverse(tree->u.next, trop);
					ast_spec_traverse(tree->body, trop);
					break;
				default:
					fprintf(stderr, "[ast_spec_traverse]: list b u g !!\n");
			}
			post_visit_spec(trop, speclist_c, tree);
			break;
		default:
			fprintf(stderr, "[ast_spec_traverse]: b u g !!\n");
	}
}


#define visit_decl(trop,f,t);\
	{	if ((trop)->declc.f) ((trop)->declc.f)(t,(trop)->starg,PREVISIT);	}
#define pre_visit_decl(trop,f,t);\
	{	if ((trop)->declc.f && (trop)->when & PREVISIT)\
			((trop)->declc.f)(t,(trop)->starg,PREVISIT);	}
#define post_visit_decl(trop,f,t);\
	{	if ((trop)->declc.f && (trop)->when & POSTVISIT)\
			((trop)->declc.f)(t,(trop)->starg,POSTVISIT);	}


void ast_decl_traverse(astdecl tree, travopts_t *trop)
{
	if (!trop->dodecl || tree == NULL)
		return;
	
	switch (tree->type)
	{
		case DIDENT:
			visit_decl(trop, dident_c, tree);
			break;
		case DPAREN:
			pre_visit_decl(trop, dparen_c, tree);
			ast_decl_traverse(tree->decl, trop);
			post_visit_decl(trop, dparen_c, tree);
			break;
		case DARRAY:
			pre_visit_decl(trop, darray_c, tree);
			if (tree->decl) /* Maybe abstract declarator */
				ast_decl_traverse(tree->decl, trop);
			if (tree->spec)
				ast_spec_traverse(tree->spec, trop);
			if (tree->u.expr)
				ast_expr_traverse(tree->u.expr, trop);
			post_visit_decl(trop, darray_c, tree);
			break;
		case DFUNC:      /* Maybe abstract declarator */
			pre_visit_decl(trop, dfunc_c, tree);
			if (tree->decl)
				ast_decl_traverse(tree->decl, trop);
			if (tree->u.params)
				ast_decl_traverse(tree->u.params, trop);
			post_visit_decl(trop, dfunc_c, tree);
			break;
		case DINIT:
			pre_visit_decl(trop, dinit_c, tree);
			ast_decl_traverse(tree->decl, trop);
			if (tree->u.expr != NULL)
				ast_expr_traverse(tree->u.expr, trop);
			post_visit_decl(trop, dinit_c, tree);
			break;
		case DECLARATOR:
			pre_visit_decl(trop, declarator_c, tree);
			if (tree->spec)      /* pointer */
				ast_spec_traverse(tree->spec, trop);
			ast_decl_traverse(tree->decl, trop);
			post_visit_decl(trop, declarator_c, tree);
			break;
		case ABSDECLARATOR:
			pre_visit_decl(trop, absdeclarator_c, tree);
			if (tree->spec)      /* pointer */
				ast_spec_traverse(tree->spec, trop);
			if (tree->decl)
				ast_decl_traverse(tree->decl, trop);
			post_visit_decl(trop, absdeclarator_c, tree);
			break;
		case DPARAM:
			pre_visit_decl(trop, dparam_c, tree);
			ast_spec_traverse(tree->spec, trop);
			if (tree->decl)
				ast_decl_traverse(tree->decl, trop);
			post_visit_decl(trop, dparam_c, tree);
			break;
		case DELLIPSIS:
			visit_decl(trop, dellipsis_c, tree);
			break;
		case DBIT:
			pre_visit_decl(trop, dbit_c, tree);
			if (tree->decl)
				ast_decl_traverse(tree->decl, trop);
			ast_expr_traverse(tree->u.expr, trop);
			post_visit_decl(trop, dbit_c, tree);
			break;
		case DSTRUCTFIELD:
			pre_visit_decl(trop, dstructfield_c, tree);
			if (tree->spec)      /* pointer */
				ast_spec_traverse(tree->spec, trop);
			if (tree->decl)
				ast_decl_traverse(tree->decl, trop);
			post_visit_decl(trop, dstructfield_c, tree);
			break;
		case DCASTTYPE:
			pre_visit_decl(trop, dcasttype_c, tree);
			ast_spec_traverse(tree->spec, trop);
			if (tree->decl)
				ast_decl_traverse(tree->decl, trop);
			post_visit_decl(trop, dcasttype_c, tree);
			break;
		case DLIST:
			pre_visit_decl(trop, dlist_c, tree);
			switch (tree->subtype)
			{
				case DECL_decllist:
				case DECL_idlist:
				case DECL_paramlist:
					if (tree->u.next == NULL || tree->decl == NULL)
					{
						fprintf(stderr, "[ast_decl_traverse]: list next/body NULL !!\n");
						break;
					}
					ast_decl_traverse(tree->u.next, trop);
					ast_decl_traverse(tree->decl, trop);
					break;
				case DECL_fieldlist:
					ast_decl_traverse(tree->u.next, trop);
					ast_decl_traverse(tree->decl, trop);
					break;
				default:
					fprintf(stderr, "[ast_decl_traverse]: list b u g !!\n");
			}
			post_visit_decl(trop, dlist_c, tree);
			break;
		default:
			fprintf(stderr, "[ast_decl_traverse]: b u g (%d) !!\n", tree->type);
			*((int *)0L)=1;
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OpenMP NODES                                              *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#define visit_ompclause(trop,f,t);\
	{	if ((trop)->ompclausec.f) ((trop)->ompclausec.f)(t,(trop)->starg,PREVISIT);}
#define pre_visit_ompclause(trop,f,t);\
	{	if ((trop)->ompclausec.f && (trop)->when & PREVISIT)\
			((trop)->ompclausec.f)(t,(trop)->starg,PREVISIT);	}
#define post_visit_ompclause(trop,f,t);\
	{	if ((trop)->ompclausec.f && (trop)->when & POSTVISIT)\
			((trop)->ompclausec.f)(t,(trop)->starg,POSTVISIT);	}
			
#define PRUNING(trop,f) ((trop)->ompclausec.prune_##f)

void ast_ompclause_traverse(ompclause t, travopts_t *trop)
{
	if (!trop->doomp || t == NULL)
		return;
	
	switch (t->type)
	{
		case OCIF:
		case OCFINAL:
		case OCNUMTHREADS:
		case OCDEVICE:
		case OCSCHEDULE:
			pre_visit_ompclause(trop, ompclexpr_c, t);
			if (!PRUNING(trop, ompclexpr_c))
				if (t->u.expr)
					ast_expr_traverse(t->u.expr, trop);
			post_visit_ompclause(trop, ompclexpr_c, t);
			break;
		case OCLIST:
			pre_visit_ompclause(trop, ompcllist_c, t);
			if (t->u.list.next)
				ast_ompclause_traverse(t->u.list.next, trop);
			if (t->u.list.elem)
				ast_ompclause_traverse(t->u.list.elem, trop);
			post_visit_ompclause(trop, ompcllist_c, t);
			break;
		case OCCOPYIN:
		case OCPRIVATE:
		case OCCOPYPRIVATE:
		case OCFIRSTPRIVATE:
		case OCLASTPRIVATE:
		case OCSHARED:
		case OCTO:
		case OCFROM:
		case OCAUTO:
		case OCMAP:
		case OCREDUCTION:
			pre_visit_ompclause(trop, ompclvars_c, t);
			if (!PRUNING(trop, ompclvars_c))
				ast_decl_traverse(t->u.varlist, trop);
			post_visit_ompclause(trop, ompclvars_c, t);
			break;
		case OCDEFAULT:
		case OCPROCBIND:
		case OCCOLLAPSE:
		case OCNOWAIT:
		case OCORDERED:
		case OCUNTIED:
		case OCMERGEABLE:
		case OCPARALLEL:
		case OCSECTIONS:
		case OCFOR:
		case OCTASKGROUP:
			visit_ompclause(trop, ompclplain_c, t);
			break;
	}
}


#define visit_ompdircon(trop,f,t);\
	{	if ((trop)->ompdcc.f) ((trop)->ompdcc.f)(t,(trop)->starg,PREVISIT);	}
#define pre_visit_ompdircon(trop,f,t);\
	{	if ((trop)->ompdcc.f && (trop)->when & PREVISIT)\
			((trop)->ompdcc.f)(t,(trop)->starg,PREVISIT);	}
#define post_visit_ompdircon(trop,f,t);\
	{	if ((trop)->ompdcc.f && (trop)->when & POSTVISIT)\
			((trop)->ompdcc.f)(t,(trop)->starg,POSTVISIT);	}


void ast_ompdir_traverse(ompdir t, travopts_t *trop)
{
	if (!trop->doomp)
		return;
	
	switch (t->type)
	{
		case DCCRITICAL:
			visit_ompdircon(trop, ompdircrit_c, t);
			break;
		case DCFLUSH:
			pre_visit_ompdircon(trop, ompdirflush_c, t);
			if (t->u.varlist)
				ast_decl_traverse(t->u.varlist, trop);
			post_visit_ompdircon(trop, ompdirflush_c, t);
			break;
		case DCTHREADPRIVATE:
			pre_visit_ompdircon(trop, ompdirthrpriv_c, t);
			if (t->u.varlist)
				ast_decl_traverse(t->u.varlist, trop);
			post_visit_ompdircon(trop, ompdirthrpriv_c, t);
			break;
		default:
			pre_visit_ompdircon(trop, ompdirrest_c, t);
			if (t->clauses)
				ast_ompclause_traverse(t->clauses, trop);
			post_visit_ompdircon(trop, ompdirrest_c, t);
			break;
	}
}


void ast_ompcon_traverse(ompcon t, travopts_t *trop)
{
	if (trop->doomp)   /* We still have to do its body! */
	{
		pre_visit_ompdircon(trop, ompconall_c, t);
		ast_ompdir_traverse(t->directive, trop);
	}
	
	if (t->body)     /* barrier & flush don't have a body. */
		ast_stmt_traverse(t->body, trop);
	
	if (trop->doomp)
		post_visit_ompdircon(trop, ompconall_c, t);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OMPi-EXTENSION NODES                                      *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#define visit_oxnode(trop,f,t);\
	{	if ((trop)->oxc.f) ((trop)->oxc.f)(t,(trop)->starg,PREVISIT);	}
#define pre_visit_oxnode(trop,f,t);\
	{	if ((trop)->oxc.f && (trop)->when & PREVISIT)\
			((trop)->oxc.f)(t,(trop)->starg,PREVISIT);	}
#define post_visit_oxnode(trop,f,t);\
	{	if ((trop)->oxc.f && (trop)->when & POSTVISIT)\
			((trop)->oxc.f)(t,(trop)->starg,POSTVISIT);	}


void ast_oxclause_traverse(oxclause t, travopts_t *trop)
{
	if (!trop->doox || t == NULL)
		return;
	
	switch (t->type)
	{
		case OX_OCREDUCE:
		case OX_OCIN:
		case OX_OCOUT:
		case OX_OCINOUT:
			pre_visit_oxnode(trop, oxclvars_c, t);
			ast_decl_traverse(t->u.varlist, trop);
			post_visit_oxnode(trop, oxclvars_c, t);
			break;
		case OX_OCATNODE:
		case OX_OCATWORKER:
		case OX_OCSTART:
		case OX_OCSTRIDE:
			pre_visit_oxnode(trop, oxclexpr_c, t);
			ast_expr_traverse(t->u.expr, trop);
			post_visit_oxnode(trop, oxclexpr_c, t);
			break;
		case OX_OCSCOPE:
			visit_oxnode(trop, oxclrest_c, t);
			break;
		case OX_OCLIST:
			pre_visit_oxnode(trop, oxcllist_c, t);
			if (t->u.list.next)
				ast_oxclause_traverse(t->u.list.next, trop);
			if (t->u.list.elem)
				ast_oxclause_traverse(t->u.list.elem, trop);
			post_visit_oxnode(trop, oxcllist_c, t);
			break;
	}
}


void ast_oxdir_traverse(oxdir t, travopts_t *trop)
{
	if (!trop->doox)
		return;
	
	pre_visit_oxnode(trop, oxdirall_c, t);
	if (t->clauses)
		ast_oxclause_traverse(t->clauses, trop);
	post_visit_oxnode(trop, oxdirall_c, t);
}


void ast_oxcon_traverse(oxcon t, travopts_t *trop)
{
	if (trop->doox)   /* We still have to do its body! */
	{
		pre_visit_oxnode(trop, oxconall_c, t);
		ast_oxdir_traverse(t->directive, trop);
	}
	
	if (t->body)
		ast_stmt_traverse(t->body, trop);
	
	if (trop->doox)
		post_visit_oxnode(trop, oxconall_c, t);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     UTILITY FUNCTIONS                                         *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void travopts_init_batch(travopts_t *trop, 
                    void (*stmtcb)(aststmt t,        void *starg, int vistime), 
                    void (*exprcb)(astexpr t,        void *starg, int vistime),
                    void (*speccb)(astspec t,        void *starg, int vistime), 
                    void (*declcb)(astdecl t,        void *starg, int vistime),
                    void (*ompclausecb)(ompclause t, void *starg, int vistime), 
                    void (*ompdircb)(ompdir t,       void *starg, int vistime),
                    void (*ompconcb)(ompcon t,       void *starg, int vistime), 
                    void (*oxclausecb)(oxclause t,   void *starg, int vistime),
                    void (*oxdircb)(oxdir t,         void *starg, int vistime), 
                    void (*oxconcb)(oxcon t,         void *starg, int vistime))
{
	trop->when   = PREVISIT;              /* Just give a default */
	trop->starg  = NULL;
	
	trop->doexpr = (exprcb != NULL);
	trop->dospec = (speccb != NULL);
	trop->dodecl = (declcb != NULL);
	trop->doomp  = (ompclausecb != NULL || ompdircb != NULL || ompconcb != NULL);
	trop->doox   = (oxclausecb != NULL || oxdircb != NULL || oxconcb != NULL);

	trop->stmtc.break_c = stmtcb;
	trop->stmtc.continue_c = stmtcb;
	trop->stmtc.return_c = stmtcb;
	trop->stmtc.goto_c = stmtcb;
	trop->stmtc.for_c = stmtcb;
	trop->stmtc.while_c = stmtcb;
	trop->stmtc.do_c = stmtcb;
	trop->stmtc.switch_c = stmtcb;
	trop->stmtc.if_c = stmtcb;
	trop->stmtc.label_c = stmtcb;
	trop->stmtc.case_c = stmtcb;
	trop->stmtc.default_c = stmtcb;
	trop->stmtc.expression_c = stmtcb;
	trop->stmtc.compound_c = stmtcb;
	trop->stmtc.declaration_c = stmtcb;
	trop->stmtc.funcdef_c = stmtcb;
	trop->stmtc.stmtlist_c = stmtcb;
	trop->stmtc.verbatim_c = stmtcb;

	trop->exprc.ident_c = exprcb;
	trop->exprc.constval_c = exprcb;
	trop->exprc.string_c = exprcb;
	trop->exprc.funccall_c = exprcb;
	trop->exprc.arrayidx_c = exprcb;
	trop->exprc.dotfield_c = exprcb;
	trop->exprc.ptrfield_c = exprcb;
	trop->exprc.bracedinit_c = exprcb;
	trop->exprc.castexpr_c = exprcb;
	trop->exprc.condexpr_c = exprcb;
	trop->exprc.uop_c = exprcb;
	trop->exprc.bop_c = exprcb;
	trop->exprc.preop_c = exprcb;
	trop->exprc.postop_c = exprcb;
	trop->exprc.ass_c = exprcb;
	trop->exprc.designated_c = exprcb;
	trop->exprc.idxdes_c = exprcb;
	trop->exprc.dotdes_c = exprcb;
	trop->exprc.list_c = exprcb;

	trop->specc.spec_c = speccb;
	trop->specc.usertype_c = speccb;
	trop->specc.specenum_c = speccb;
	trop->specc.specstruct_c = speccb;
	trop->specc.specunion_c = speccb;
	trop->specc.enumerator_c = speccb;
	trop->specc.speclist_c = speccb;

	trop->declc.dident_c = declcb;
	trop->declc.dparen_c = declcb;
	trop->declc.darray_c = declcb;
	trop->declc.dfunc_c = declcb;
	trop->declc.dinit_c = declcb;
	trop->declc.declarator_c = declcb;
	trop->declc.absdeclarator_c = declcb;
	trop->declc.dparam_c = declcb;
	trop->declc.dellipsis_c = declcb;
	trop->declc.dbit_c = declcb;
	trop->declc.dstructfield_c = declcb;
	trop->declc.dcasttype_c = declcb;
	trop->declc.dlist_c = declcb;

	trop->ompclausec.ompclexpr_c = ompclausecb;
	trop->ompclausec.ompclvars_c = ompclausecb;
	trop->ompclausec.ompclplain_c = ompclausecb;
	trop->ompclausec.ompcllist_c = ompclausecb;
	trop->ompclausec.prune_ompclexpr_c = 0;
	trop->ompclausec.prune_ompclvars_c = 0;

	trop->ompdcc.ompdircrit_c = ompdircb;
	trop->ompdcc.ompdirflush_c = ompdircb;
	trop->ompdcc.ompdirthrpriv_c = ompdircb;
	trop->ompdcc.ompdirrest_c = ompdircb;
	trop->ompdcc.ompconall_c = ompconcb;

	trop->oxc.oxclvars_c = oxclausecb;
	trop->oxc.oxclexpr_c = oxclausecb;
	trop->oxc.oxclrest_c = oxclausecb;
	trop->oxc.oxcllist_c = oxclausecb;
	trop->oxc.oxdirall_c = oxdircb;
	trop->oxc.oxconall_c = oxconcb;
};


void travopts_init_noop(travopts_t *trop)
{
	travopts_init_batch(trop, NULL, NULL, NULL, NULL, NULL, 
	                          NULL, NULL, NULL, NULL, NULL);
	trop->doexpr = 1;
	trop->dospec = 1;
	trop->dodecl = 1;
	trop->doomp  = 1;
	trop->doox   = 1;
}
