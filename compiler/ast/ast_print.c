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

/* ast_print.c -- a non-reentrant way to print the AST onto a string buffer */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "symtab.h"
#include "ast_print.h"

static str bf = NULL;  /* Everything is printed on this buffer */

static void ast_expr_prn(astexpr tree);
static void ast_stmt_prn(aststmt stmt);
static void ast_decl_prn(astdecl tree);
static void ast_spec_prn(astspec tree);
static void ast_ompdir_prn(ompdir tree);
static void ast_ompcon_prn(ompcon tree);
static void ast_ompclause_prn(ompclause t);
static void ast_oxcon_prn(oxcon t);

static int indlev;

static
void indent()
{
	int i;

	if (indlev > 0)
		for (i = 2 * indlev; i > 0; i--)
			str_putc(bf, ' ');
}


static void ast_stmt_jump_prn(aststmt tree)
{
	switch (tree->subtype)
	{
		case SBREAK:
			str_printf(bf, "break;\n");
			break;
		case SCONTINUE:
			str_printf(bf, "continue;\n");
			break;
		case SRETURN:
			str_printf(bf, "return");
			if (tree->u.expr != NULL)
			{
				str_printf(bf, " (");
				ast_expr_prn(tree->u.expr);
				str_printf(bf, ")");
			}
			str_printf(bf, ";\n");
			break;
		case SGOTO:
			str_printf(bf, "goto %s;\n", tree->u.label->name);
			break;
		default:
			fprintf(stderr, "[ast_stmt_jump_prn]: b u g !!\n");
	}
}


static void ast_stmt_iteration_prn(aststmt tree)
{
	switch (tree->subtype)
	{
		case SFOR:
			str_printf(bf, "for (");
			if (tree->u.iteration.init != NULL)
				ast_stmt_prn(tree->u.iteration.init);
			else
				str_printf(bf, " ; ");
			if (tree->u.iteration.cond != NULL)
				ast_expr_prn(tree->u.iteration.cond);
			str_printf(bf, "; ");
			if (tree->u.iteration.incr != NULL)
				ast_expr_prn(tree->u.iteration.incr);
			str_printf(bf, ")\n");
			indlev++; indent();
			ast_stmt_prn(tree->body);
			indlev--;
			break;
		case SWHILE:
			str_printf(bf, "while (");
			ast_expr_prn(tree->u.iteration.cond);
			str_printf(bf, ")\n");
			indlev++; indent();
			ast_stmt_prn(tree->body);
			indlev--;
			break;
		case SDO:
			str_printf(bf, "do\n");
			indlev++; indent();
			ast_stmt_prn(tree->body);
			indlev--; indent();
			str_printf(bf, "while (");
			ast_expr_prn(tree->u.iteration.cond);
			str_printf(bf, ");\n");
			break;
		default:
			fprintf(stderr, "[ast_stmt_iteration_prn]: b u g !!\n");
	}
}


static void ast_stmt_selection_prn(aststmt tree)
{
	switch (tree->subtype)
	{
		case SSWITCH:
			str_printf(bf, "switch (");
			ast_expr_prn(tree->u.selection.cond);
			str_printf(bf, ")\n");
			indent();
			ast_stmt_prn(tree->body);
			str_printf(bf, "\n");
			break;
		case SIF:
			str_printf(bf, "if (");
			ast_expr_prn(tree->u.selection.cond);
			str_printf(bf, ") { \n");
			indlev++; indent();
			ast_stmt_prn(tree->body);
			indlev--;
			str_printf(bf, "}\n");
			if (tree->u.selection.elsebody)
			{
				indent();
				str_printf(bf, "else {\n");
				indlev++; indent();
				ast_stmt_prn(tree->u.selection.elsebody);
				str_printf(bf, "}\n");
				indlev--;
			}
			break;
		default:
			fprintf(stderr, "[ast_stmt_selection_prn]: b u g !!\n");
	}
}


static void ast_stmt_labeled_prn(aststmt tree)
{
	switch (tree->subtype)
	{
		case SLABEL:
			str_printf(bf, "%s :\n", tree->u.label->name);
			break;
		case SCASE:
			str_printf(bf, "case ");
			ast_expr_prn(tree->u.expr);
			str_printf(bf, " :\n");
			break;
		case SDEFAULT:
			str_printf(bf, "default :\n");
			break;
		default:
			fprintf(stderr, "[ast_stmt_labeled_prn]: b u g !!\n");
	}
	indlev++; indent();
	ast_stmt_prn(tree->body);
	indlev--;
}


static void ast_asmop_prn(asmop op)
{
	if (!op->constraint)   /* a list */
	{
		ast_asmop_prn(op->op);
		str_printf(bf, ", ");
		ast_asmop_prn(op->next);
	}
	else
	{
		if (op->symbolicname)
		{
			str_printf(bf, "[");
			ast_expr_prn(op->symbolicname);
			str_printf(bf, "]");
		}
		str_printf(bf, " %s(", op->constraint);
		ast_expr_prn(op->var);
		str_printf(bf, ")");
	}
}


static void ast_stmt_prn(aststmt tree)
{
	if (!tree)
		return;
	switch (tree->type)
	{
		case JUMP:
			ast_stmt_jump_prn(tree);
			break;
		case ITERATION:
			ast_stmt_iteration_prn(tree);
			break;
		case SELECTION:
			ast_stmt_selection_prn(tree);
			break;
		case LABELED:
			ast_stmt_labeled_prn(tree);
			break;
		case EXPRESSION:
			if (tree->u.expr != NULL)
				ast_expr_prn(tree->u.expr);
			str_printf(bf, ";\n");
			break;
		case COMPOUND:
			str_printf(bf, "{\n");
			if (tree->body)
			{
				indlev++; indent();
				ast_stmt_prn(tree->body);
				indlev--;
			}
			indent();
			str_printf(bf, "}\n");
			break;
		case STATEMENTLIST:
			ast_stmt_prn(tree->u.next);
			indent();
			ast_stmt_prn(tree->body);
			break;
		case DECLARATION:
			ast_spec_prn(tree->u.declaration.spec);
			if (tree->u.declaration.decl)
			{
				str_printf(bf, " ");
				ast_decl_prn(tree->u.declaration.decl);
			}
			str_printf(bf, ";\n");
			break;
		case FUNCDEF:
			indent();
			if (tree->u.declaration.spec)
			{
				ast_spec_prn(tree->u.declaration.spec);
				str_printf(bf, " ");
			}
			ast_decl_prn(tree->u.declaration.decl);
			str_printf(bf, "\n");
			if (tree->u.declaration.dlist)
			{
				indlev++; indent();
				ast_stmt_prn(tree->u.declaration.dlist);
				indlev--;
			}
			indent();
			ast_stmt_prn(tree->body);
			str_printf(bf, "\n");
			break;
		case ASMSTMT:
			str_printf(bf, "__asm__ ");
			if (tree->u.assem->qualifiers)
			{
				ast_spec_prn(tree->u.assem->qualifiers);
				str_printf(bf, " ");
			}
			if (tree->subtype == SGOTO)
				str_printf(bf, "goto ");
			str_printf(bf, "(%s", tree->u.assem->template);
			if (tree->subtype == SXTENDED)
			{
				if (tree->u.assem->outs || tree->u.assem->ins || tree->u.assem->clobbers)
				{
					str_printf(bf, " : "); 
					if (tree->u.assem->outs)
						ast_asmop_prn(tree->u.assem->outs);
				}
				if (tree->u.assem->ins || tree->u.assem->clobbers)
				{
					str_printf(bf, " : ");
					if (tree->u.assem->ins)
						ast_asmop_prn(tree->u.assem->ins);
				}
				if (tree->u.assem->clobbers)
				{
					str_printf(bf, " : ");
					ast_expr_prn(tree->u.assem->clobbers);
				}
			}
			if (tree->subtype == SGOTO)
			{
				str_printf(bf, " : : "); 
				if (tree->u.assem->ins)
					ast_asmop_prn(tree->u.assem->ins);
				str_printf(bf, " : ");
				if (tree->u.assem->clobbers)
					ast_expr_prn(tree->u.assem->clobbers);
				str_printf(bf, " : ");
				if (tree->u.assem->labels)
					ast_expr_prn(tree->u.assem->labels);
			}
			str_printf(bf, ")\n");
			break;
		case OMPSTMT:
			ast_ompcon_prn(tree->u.omp);
			str_printf(bf, "\n");
			break;
		case VERBATIM:
			str_printf(bf, "%s\n", tree->u.code);
			break;
		case OX_STMT:
			ast_oxcon_prn(tree->u.ox);
			str_printf(bf, "\n");
			break;
		default:
			fprintf(stderr, "[ast_stmt_prn]: b u g !!\n");
	}
}


static void ast_expr_prn(astexpr tree)
{
	switch (tree->type)
	{
		case IDENT:
			str_printf(bf, "%s", tree->u.sym->name);
			break;
		case CONSTVAL:
			str_printf(bf, "%s", tree->u.str);
			break;
		case STRING:
			str_printf(bf, "%s", tree->u.str);
			break;
		case FUNCCALL:
			ast_expr_prn(tree->left);
			str_printf(bf, "(");
			if (tree->right)
				ast_expr_prn(tree->right);
			str_printf(bf, ")");
			break;
		case ARRAYIDX:
			ast_expr_prn(tree->left);
			str_printf(bf, "[");
			ast_expr_prn(tree->right);
			str_printf(bf, "]");
			break;
		case DOTFIELD:
			ast_expr_prn(tree->left);
			str_printf(bf, ".%s", tree->u.sym->name);
			break;
		case PTRFIELD:
			ast_expr_prn(tree->left);
			str_printf(bf, "->%s", tree->u.sym->name);
			break;
		case BRACEDINIT:
			str_printf(bf, "{ ");
			ast_expr_prn(tree->left);
			str_printf(bf, " }");
			break;
		case CASTEXPR:
			str_printf(bf, "(");
			ast_decl_prn(tree->u.dtype);
			str_printf(bf, ") ");
			ast_expr_prn(tree->left);
			break;
		case CONDEXPR:
			ast_expr_prn(tree->u.cond);
			str_printf(bf, " ? ");
			ast_expr_prn(tree->left);
			str_printf(bf, " : ");
			ast_expr_prn(tree->right);
			break;
		case UOP:
			str_printf(bf, "%s", UOP_symbols[tree->opid]);
			if (tree->opid == UOP_sizeoftype || tree->opid == UOP_sizeof)
				str_printf(bf, "(");
			if (tree->opid == UOP_sizeoftype || tree->opid == UOP_typetrick)
				ast_decl_prn(tree->u.dtype);
			else
				ast_expr_prn(tree->left);
			if (tree->opid == UOP_paren || tree->opid == UOP_sizeoftype
			    || tree->opid == UOP_sizeof)
				str_printf(bf, ")");
			break;
		case BOP:
			ast_expr_prn(tree->left);
			str_printf(bf, " %s ", BOP_symbols[tree->opid]);
			ast_expr_prn(tree->right);
			break;
		case PREOP:
			str_printf(bf, "%s", UOP_symbols[tree->opid]);
			ast_expr_prn(tree->left);
			break;
		case POSTOP:
			ast_expr_prn(tree->left);
			str_printf(bf, "%s", UOP_symbols[tree->opid]);
			break;
		case ASS:
			ast_expr_prn(tree->left);
			str_printf(bf, " %s ", ASS_symbols[tree->opid]);
			ast_expr_prn(tree->right);
			break;
		case DESIGNATED:
			ast_expr_prn(tree->left);
			str_printf(bf, " = ");
			ast_expr_prn(tree->right);
			break;
		case IDXDES:
			str_printf(bf, "[");
			ast_expr_prn(tree->left);
			str_printf(bf, "]");
			break;
		case DOTDES:
			str_printf(bf, ".%s", tree->u.sym->name);
			break;
		case COMMALIST:
		case SPACELIST:
			ast_expr_prn(tree->left);
			str_printf(bf, "%s", tree->type == COMMALIST ? ", " : " ");
			ast_expr_prn(tree->right);
			break;
		default:
			fprintf(stderr, "[ast_expr_prn]: b u g !!\n");
	}
}


static void ast_spec_prn(astspec tree)
{
	switch (tree->type)
	{
		case SPEC:
		case STCLASSSPEC:
			str_printf(bf, "%s", SPEC_symbols[tree->subtype]);
			break;
		case USERTYPE:
			str_printf(bf, "%s", tree->name->name);
			break;
		case SUE:
			switch (tree->subtype)
			{
				case SPEC_enum:
					str_printf(bf, "enum");
					if (tree->name)
						str_printf(bf, " %s", tree->name->name);
					if (tree->sueattr)
					{
						str_printf(bf, " ");
						ast_spec_prn(tree->sueattr);
					}
					if (tree->body)
					{
						str_printf(bf, " {\n");
						indlev += 2 ; indent();
						ast_spec_prn(tree->body);
						str_printf(bf, "\n");
						indlev--; indent(); indlev--;
						str_printf(bf, "}");
					}
					break;
				case SPEC_struct:
				case SPEC_union:
					str_printf(bf, "%s", tree->subtype == SPEC_struct ? "struct" : "union");
					if (tree->sueattr)
					{
						str_printf(bf, " ");
						ast_spec_prn(tree->sueattr);
					}
					if (tree->name)
						str_printf(bf, " %s", tree->name->name);
					if (tree->u.decl)
					{
						str_printf(bf, " {\n");
						indlev += 2; indent();
						ast_decl_prn(tree->u.decl);
						str_printf(bf, "\n");
						indlev--; indent(); indlev--;
						str_printf(bf, "}");
					}
					break;
				default:
					fprintf(stderr, "[ast_spec_prn]: SUE b u g !!\n");
			}
			break;
		case ENUMERATOR:
			str_printf(bf, "%s", tree->name->name);
			if (tree->u.expr)
			{
				str_printf(bf, " = ");
				ast_expr_prn(tree->u.expr);
			}
			break;
		case SPECLIST:
			switch (tree->subtype)
			{
				case SPEC_Rlist:
					ast_spec_prn(tree->body);
					if (tree->body->type != SPEC || tree->body->subtype != SPEC_star)
						str_printf(bf, " ");   /* No spaces among consecutive stars */
					ast_spec_prn(tree->u.next);
					break;
				case SPEC_Llist:
					ast_spec_prn(tree->u.next);
					str_printf(bf, " ");
					ast_spec_prn(tree->body);
					break;
				case SPEC_enum:
					ast_spec_prn(tree->u.next);
					str_printf(bf, ", ");
					ast_spec_prn(tree->body);
					break;
				default:
					fprintf(stderr, "[ast_spec_prn]: list b u g !!\n");
			}
			break;
		case ATTRSPEC:
			str_printf(bf, "__attribute__(( %s ))", tree->u.txt ? tree->u.txt : "");
			break;
		default:
			fprintf(stderr, "[ast_spec_prn]: b u g !!\n");
	}
}


static void ast_decl_prn(astdecl tree)
{
	switch (tree->type)
	{
		case DIDENT:
			str_printf(bf, "%s", tree->u.id->name);
			break;
		case DPAREN:
			str_printf(bf, "(");
			ast_decl_prn(tree->decl);
			str_printf(bf, ") ");
			break;
		case DARRAY:
			if (tree->decl)
				ast_decl_prn(tree->decl);
			str_printf(bf, "[");
			if (tree->spec)
				ast_spec_prn(tree->spec);
			if (tree->u.expr)
			{
				str_printf(bf, " ");
				ast_expr_prn(tree->u.expr);
			}
			str_printf(bf, "]");
			break;
		case DFUNC:
			if (tree->decl)
				ast_decl_prn(tree->decl);
			str_printf(bf, "(");
			if (tree->u.params)
				ast_decl_prn(tree->u.params);
			str_printf(bf, ")");
			break;
		case DINIT:
			ast_decl_prn(tree->decl);
			if (tree->u.expr != NULL)
			{
				str_printf(bf, " = ");
				ast_expr_prn(tree->u.expr);
			}
			break;
		case DECLARATOR:
			if (tree->spec)      /* pointer */
			{
				ast_spec_prn(tree->spec);
				str_printf(bf, " ");
			}
			ast_decl_prn(tree->decl);
			break;
		case ABSDECLARATOR:
			if (tree->spec)      /* pointer */
				ast_spec_prn(tree->spec);
			if (tree->decl)      /* could be NULL */
			{
				str_printf(bf, " ");
				ast_decl_prn(tree->decl);
			}
			break;
		case DPARAM:
			ast_spec_prn(tree->spec);
			if (tree->decl)
			{
				str_printf(bf, " ");
				ast_decl_prn(tree->decl);
			}
			break;
		case DELLIPSIS:
			str_printf(bf, "...");
			break;
		case DBIT:
			if (tree->decl)
				ast_decl_prn(tree->decl);
			str_printf(bf, " : ");
			ast_expr_prn(tree->u.expr);
			break;
		case DSTRUCTFIELD:
			if (tree->spec)      /* pointer */
			{
				ast_spec_prn(tree->spec);
				str_printf(bf, " ");
			}
			if (tree->decl)
				ast_decl_prn(tree->decl);
			str_printf(bf, ";\n");
			break;
		case DCASTTYPE:
			ast_spec_prn(tree->spec);
			if (tree->decl)
			{
				str_printf(bf, " ");
				ast_decl_prn(tree->decl);
			}
			break;
		case DLIST:
			switch (tree->subtype)
			{
				case DECL_decllist:
				case DECL_idlist:
				case DECL_paramlist:
					ast_decl_prn(tree->u.next);
					str_printf(bf, ", ");
					ast_decl_prn(tree->decl);
					break;
				case DECL_fieldlist:
					ast_decl_prn(tree->u.next);
					str_printf(bf, "\n"); indent();
					ast_decl_prn(tree->decl);
					break;
				default:
					fprintf(stderr, "[ast_decl_prn]: dlist b u g !!\n");
			}
			break;
		default:
			fprintf(stderr, "[ast_decl_prn]: b u g !!\n");
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OpenMP NODES                                              *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static void ast_omparrdim_prn(omparrdim d)
{
	str_printf(bf, "[");
	ast_expr_prn(d->lb);
	str_printf(bf, " : ");
	if (d->len)
		ast_expr_prn(d->len);
	str_printf(bf, "]");
}


static void ast_ompxli_prn(ompxli xl)
{
	omparrdim s;
	
	for (; xl; xl = xl->next)
	{
		str_printf(bf, "%s", xl->id->name);
		if (xl->xlitype == OXLI_ARRSEC)
			for (s = xl->dim; s; s = s->next)
				ast_omparrdim_prn(s);
		if (xl->next)
			str_printf(bf, ", ");
	}
}


static void ast_ompclause_prn(ompclause t)
{
	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
		{
			ast_ompclause_prn(t->u.list.next);
			str_printf(bf, " ");
		}
		assert((t = t->u.list.elem) != NULL);
	}

	str_printf(bf, "%s", clausenames[t->type]);
	switch (t->type)
	{
		case OCIF:
			if (t->modifier != OCM_none)
				str_printf(bf, "(%s: ", clausemods[t->modifier]);
			else
				str_printf(bf, "("); 
			ast_expr_prn(t->u.expr); str_printf(bf, ")");
			break;
		case OCFINAL:
		case OCNUMTHREADS:
		case OCDEVICE:
		case OCNUMTEAMS:
		case OCTHREADLIMIT:
		case OCHINT:
		case OCPRIORITY:
			str_printf(bf, "("); ast_expr_prn(t->u.expr); str_printf(bf, ")");
			break;
		case OCSCHEDULE:
			str_printf(bf, "(");
			if (t->modifier != OCM_none)
				str_printf(bf, "%s:", clausemods[t->modifier]);
			str_printf(bf, "%s%s", clausesubs[t->subtype], t->u.expr ? ", " : " ");
			if (t->u.expr)
				ast_expr_prn(t->u.expr);
			str_printf(bf, ")");
			break;
		case OCDEFAULT:
		case OCPROCBIND:
			str_printf(bf, "(%s)", clausesubs[t->subtype]);
			break;
		case OCDEPEND:
			if (t->subtype == OC_sink)
			{
				str_printf(bf, "(%s: ", clausesubs[t->subtype]);
				if (t->u.expr)
					ast_expr_prn(t->u.expr);
				str_printf(bf, ")");
				break;
			}
			if (t->subtype == OC_source)
			{
				str_printf(bf, "(%s)", clausesubs[t->subtype]);
				break;
			}

		case OCREDUCTION:
		case OCMAP:
			if (t->modifier != OCM_none)
				str_printf(bf,"(%s,%s: ",clausemods[t->modifier],clausesubs[t->subtype]);
			else
				str_printf(bf, "(%s: ", clausesubs[t->subtype]);
			ast_ompxli_prn(t->u.xlist);
			str_printf(bf, ")");
			break;
		case OCTO:
		case OCFROM:
		case OCLINK:
			str_printf(bf, "(");
			ast_ompxli_prn(t->u.xlist);
			str_printf(bf, ")");
			break;
		case OCCOPYIN:
		case OCPRIVATE:
		case OCCOPYPRIVATE:
		case OCFIRSTPRIVATE:
		case OCLASTPRIVATE:
		case OCSHARED:
		case OCISDEVPTR:
		case OCUSEDEVPTR:
		case OCAUTO:
			str_printf(bf, "("); ast_decl_prn(t->u.varlist); str_printf(bf, ")");
			break;
		case OCNOWAIT:
		case OCORDERED:
		case OCUNTIED:
		case OCMERGEABLE:
		case OCPARALLEL:
		case OCSECTIONS:
		case OCFOR:
		case OCTASKGROUP:
		case OCDEFAULTMAP:
			break;
		case OCCOLLAPSE:
		case OCORDEREDNUM:
			str_printf(bf, "(%d)", t->subtype);
			break;
		default:
			fprintf(stderr, "[ast_ompclause_prn]: b u g !! (%d)\n", t->type);
	}
}


static void ast_ompdir_prn(ompdir t)
{
	str_printf(bf, "#pragma omp %s ", ompdirnames[t->type]);
	switch (t->type)
	{
		case DCCRITICAL:
			if (t->u.region)
			{
				str_printf(bf, "(%s)", t->u.region->name);
				if (t->clauses)
					ast_ompclause_prn(t->clauses);
			}
			break;
		case DCFLUSH:
			if (t->u.varlist)
			{
				str_printf(bf, "(");
				ast_decl_prn(t->u.varlist);
				str_printf(bf, ")");
			}
			break;
		case DCTHREADPRIVATE:
			if (t->u.varlist)
			{
				str_printf(bf, "(");
				ast_decl_prn(t->u.varlist);
				str_printf(bf, ")");
			}
			break;
		default:
			if (t->clauses)
				ast_ompclause_prn(t->clauses);
			break;
	}
	str_printf(bf, "\n");
}


static void ast_ompcon_prn(ompcon t)
{
	ast_ompdir_prn(t->directive);
	if (t->body)     /* stand-alones don't have a body */
		ast_stmt_prn(t->body);
	if (t->type == DCDECLTARGET && t->body)  /* v4.0 style */
		str_printf(bf, "#pragma omp end %s\n", ompdirnames[t->type]);
}


void ast_expr_print(str s, astexpr tree)
{
	if (s == NULL) return;
	bf = s;
	ast_expr_prn(tree);
}


void ast_stmt_print(str s, aststmt tree)
{
	if (s == NULL) return;
	bf = s;
	ast_stmt_prn(tree);
}


void ast_decl_print(str s, astdecl tree)
{
	if (s == NULL) return;
	bf = s;
	ast_decl_prn(tree);
}


void ast_spec_print(str s, astspec tree)
{
	if (s == NULL) return;
	bf = s;
	ast_spec_prn(tree);
}


void ast_ompdir_print(str s, ompdir tree)
{
	if (s == NULL) return;
	bf = s;
	ast_ompdir_prn(tree);
}

void ast_ompcon_print(str s, ompcon tree)
{
	if (s == NULL) return;
	bf = s;
	ast_ompcon_prn(tree);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OMPi-EXTENSION NODES                                      *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static
void ast_oxclause_prn(oxclause t)
{
	if (t->type == OX_OCLIST)
	{
		if (t->u.list.next != NULL)
		{
			ast_oxclause_prn(t->u.list.next);
			str_printf(bf, " ");
		}
		assert((t = t->u.list.elem) != NULL);
	}

	str_printf(bf, "%s", oxclausenames[t->type]);
	switch (t->type)
	{
		case OX_OCREDUCE:
			str_printf(bf, "(%s : ", clausesubs[t->operator]);
			ast_decl_prn(t->u.varlist);
			str_printf(bf, ")");
			break;
		case OX_OCIN:
		case OX_OCOUT:
		case OX_OCINOUT:
			str_printf(bf, "("); ast_decl_prn(t->u.varlist); str_printf(bf, ")");
			break;
		case OX_OCATNODE:
		case OX_OCATWORKER:
		case OX_OCIF:
		case OX_OCSTART:
		case OX_OCSTRIDE:
			str_printf(bf, "("); ast_expr_prn(t->u.expr); str_printf(bf, ")");
			break;
		case OX_OCLOCAL:
		case OX_OCREMOTE:
			break;
		case OX_OCSCOPE:
			str_printf(bf, "scope(%s)", t->u.value == OX_SCOPE_NODES ? "nodes" :
			           t->u.value == OX_SCOPE_WLOCAL ? "workers,local" :
			           t->u.value == OX_SCOPE_WGLOBAL ? "workers,global" :
			           "???");
			break;
	}
}


static
void ast_oxdir_prn(oxdir t)
{
	str_printf(bf, "#pragma ompix %s ", oxdirnames[t->type]);
	if (t->clauses)
		ast_oxclause_prn(t->clauses);
}


static
void ast_oxcon_prn(oxcon t)
{
	ast_oxdir_prn(t->directive);
	if (t->body)
		ast_stmt_prn(t->body);
}


void ast_oxdir_print(str s, oxdir tree)
{
	if (s == NULL) return;
	bf = s;
	ast_oxdir_prn(tree);
}

void ast_oxcon_print(str s, oxcon tree)
{
	if (s == NULL) return;
	bf = s;
	ast_oxcon_prn(tree);
}
