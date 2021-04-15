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

/* ast_show.c -- prints out the tree; makes it look good, too. */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "symtab.h"
#include "ast_show.h"
#include "ompi.h"

static symbol _curfile = NULL;
static int indlev = 0;    /* Indentation level */

static
void indent()
{
	int i;

	if (indlev > 0)
		for (i = 2 * indlev; i > 0; i--)
			putchar(' ');
}


void ast_stmt_jump_show(aststmt tree)
{
	switch (tree->subtype)
	{
		case SBREAK:
			printf("break;");
			break;
		case SCONTINUE:
			printf("continue;");
			break;
		case SRETURN:
			printf("return");
			if (tree->u.expr != NULL)
			{
				printf(" (");
				ast_expr_show(tree->u.expr);
				printf(")");
			}
			printf(";");
			break;
		case SGOTO:
			printf("goto %s;", tree->u.label->name);
			break;
		default:
			fprintf(stderr, "[ast_stmt_jump_show]: b u g !!\n");
	}
	printf("\n");
}


void ast_stmt_iteration_show(aststmt tree)
{
	switch (tree->subtype)
	{
		case SFOR:
			printf("for (");
			if (tree->u.iteration.init != NULL)
			{
				/* We dont do: ast_stmt_show(tree->u.iteration.init) to avoid the \n */
				if (tree->u.iteration.init->type == EXPRESSION)
				{
					if (tree->u.iteration.init->u.expr != NULL)
						ast_expr_show(tree->u.iteration.init->u.expr);
				}
				else          /* Declaration */
				{
					ast_spec_show(tree->u.iteration.init->u.declaration.spec);
					if (tree->u.iteration.init->u.declaration.decl)
					{
						printf(" ");
						ast_decl_show(tree->u.iteration.init->u.declaration.decl);
					}
				}
				printf("; ");
			}
			else
				printf(" ; ");
			if (tree->u.iteration.cond != NULL)
				ast_expr_show(tree->u.iteration.cond);
			printf("; ");
			if (tree->u.iteration.incr != NULL)
				ast_expr_show(tree->u.iteration.incr);
			printf(")\n");
			indlev++; indent();
			ast_stmt_show(tree->body);
			indlev--;
			break;
		case SWHILE:
			printf("while (");
			ast_expr_show(tree->u.iteration.cond);
			printf(")\n");
			indlev++; indent();
			ast_stmt_show(tree->body);
			indlev--;
			break;
		case SDO:
			printf("do\n");
			indlev++; indent();
			ast_stmt_show(tree->body);
			indlev--; indent();
			printf("while (");
			ast_expr_show(tree->u.iteration.cond);
			printf(");\n\n");
			break;
		default:
			fprintf(stderr, "[ast_stmt_iteration_show]: b u g !!\n");
	}
}


void ast_stmt_selection_show(aststmt tree)
{
	switch (tree->subtype)
	{
		case SSWITCH:
			printf("switch (");
			ast_expr_show(tree->u.selection.cond);
			printf(")\n");
			indent();
			ast_stmt_show(tree->body);
			break;
		case SIF:
			printf("if (");
			ast_expr_show(tree->u.selection.cond);
			printf(")\n");
			indlev++; indent();
			ast_stmt_show(tree->body);
			indlev--;
			if (tree->u.selection.elsebody)
			{
				indent();
				printf("else\n");
				indlev++; indent();
				ast_stmt_show(tree->u.selection.elsebody);
				indlev--;
			}
			break;
		default:
			fprintf(stderr, "[ast_stmt_selection_show]: b u g !!\n");
	}
}


void ast_stmt_labeled_show(aststmt tree)
{
	switch (tree->subtype)
	{
		case SLABEL:
			printf("%s :\n", tree->u.label->name);
			break;
		case SCASE:
			printf("case ");
			ast_expr_show(tree->u.expr);
			printf(" :\n");
			break;
		case SDEFAULT:
			printf("default :\n");
			break;
		default:
			fprintf(stderr, "[ast_stmt_labeled_show]: b u g !!\n");
	}
	indlev++; indent();
	ast_stmt_show(tree->body);
	indlev--;
}


void ast_asmop_show(asmop op)
{
	if (!op->constraint)   /* a list */
	{
		ast_asmop_show(op->op);
		printf(", ");
		ast_asmop_show(op->next);
	}
	else
	{
		if (op->symbolicname)
		{
			printf("[");
			ast_expr_show(op->symbolicname);
			printf("]");
		}
		printf(" %s(", op->constraint);
		ast_expr_show(op->var);
		printf(")");
	}
}


void ast_stmt_show(aststmt tree)
{
	if (tree->file != NULL && tree->file != _curfile)
		if (tree->type != FUNCDEF && tree->type != STATEMENTLIST
		    && tree->type != VERBATIM)
		{
			_curfile = tree->file;
			if (strcmp("injected_code", tree->file->name))
			{
				if (cppLineNo)
					printf("# %d \"%s\"\n", tree->l, tree->file->name);
				indent();
			}
		}

	switch (tree->type)
	{
		case JUMP:
			ast_stmt_jump_show(tree);
			break;
		case ITERATION:
			ast_stmt_iteration_show(tree);
			break;
		case SELECTION:
			ast_stmt_selection_show(tree);
			break;
		case LABELED:
			ast_stmt_labeled_show(tree);
			break;
		case EXPRESSION:
			if (tree->u.expr != NULL)
				ast_expr_show(tree->u.expr);
			printf(";\n");
			break;
		case COMPOUND:
			printf("{\n");
			if (tree->body)
			{
				indlev++; indent();
				ast_stmt_show(tree->body);  /* Ends in \n */
				indlev--;
			}
			indent();
			printf("}\n");
			break;
		case STATEMENTLIST:
		{
			aststmt ch;
			int     lastdef = 0;

			/* If my rightmost child of the left subtree is a DECLARATION and
			 * the leftmost child of my right subtree is not, then this is
			 * where declarations end.
			 */
			for (ch = tree->u.next; ch->type == STATEMENTLIST; ch = ch->body)
				;
			if (ch->type == DECLARATION)
			{
				for (ch = tree->body; ch->type == STATEMENTLIST; ch = ch->u.next)
					;
				if (ch->type != DECLARATION)
					lastdef = 1;
			}
			ast_stmt_show(tree->u.next);
			if (lastdef)
				printf("\n");    /* 1 empty line after the last declaration */
			indent();
			ast_stmt_show(tree->body);
			break;
		}
		case DECLARATION:
			ast_spec_show(tree->u.declaration.spec);
			if (tree->u.declaration.decl)
			{
				printf(" ");
				ast_decl_show(tree->u.declaration.decl);
			}
			printf(";\n");
			break;
		case FUNCDEF:
			printf("\n");               /* Make it stand out */
			indent();
			if (tree->u.declaration.spec)
			{
				ast_spec_show(tree->u.declaration.spec);
				printf(" ");
			}
			ast_decl_show(tree->u.declaration.decl);
			printf("\n");
			if (tree->u.declaration.dlist)
			{
				indlev++; indent();
				ast_stmt_show(tree->u.declaration.dlist);
				indlev--;
			}
			indent();
			ast_stmt_show(tree->body);
			printf("\n");               /* Make it stand out */
			break;
		case OMPSTMT:
			ast_ompcon_show(tree->u.omp);
			printf("\n");
			break;
		case VERBATIM:
			printf("%s\n", tree->u.code);
			break;
		case ASMSTMT:
			printf("__asm__ ");
			if (tree->u.assem->qualifiers)
			{
				ast_spec_show(tree->u.assem->qualifiers);
				printf(" ");
			}
			if (tree->subtype == SGOTO)
				printf("goto ");
			printf("(%s", tree->u.assem->template);
			if (tree->subtype == SXTENDED)
			{
				if (tree->u.assem->outs || tree->u.assem->ins || tree->u.assem->clobbers)
				{
					printf(" : ");
					if (tree->u.assem->outs)
						ast_asmop_show(tree->u.assem->outs);
				}
				if (tree->u.assem->ins || tree->u.assem->clobbers)
				{
					printf(" : ");
					if (tree->u.assem->ins)
						ast_asmop_show(tree->u.assem->ins);
				}
				if (tree->u.assem->clobbers)
				{
					printf(" : ");
					ast_expr_show(tree->u.assem->clobbers);
				}
			}
			if (tree->subtype == SGOTO)
			{
				printf(" : : "); 
				if (tree->u.assem->ins)
					ast_asmop_show(tree->u.assem->ins);
				printf(" : ");
				if (tree->u.assem->clobbers)
					ast_expr_show(tree->u.assem->clobbers);
				printf(" : ");
				if (tree->u.assem->labels)
					ast_expr_show(tree->u.assem->labels);
			}
			printf(")\n");
			break;
		case OX_STMT:
			ast_oxcon_show(tree->u.ox);
			printf("\n");
			break;
		default:
			fprintf(stderr, "[ast_stmt_show]: b u g !!\n");
	}
}


void ast_expr_show(astexpr tree)
{
	switch (tree->type)
	{
		case IDENT:
			printf("%s", tree->u.sym->name);
			break;
		case CONSTVAL:
			printf("%s", tree->u.str);
			break;
		case STRING:
			printf("%s", tree->u.str);
			break;
		case FUNCCALL:
			ast_expr_show(tree->left);
			printf("(");
			if (tree->right)
				ast_expr_show(tree->right);
			printf(")");
			break;
		case ARRAYIDX:
			ast_expr_show(tree->left);
			printf("[");
			ast_expr_show(tree->right);
			printf("]");
			break;
		case DOTFIELD:
			ast_expr_show(tree->left);
			printf(".%s", tree->u.sym->name);
			break;
		case PTRFIELD:
			ast_expr_show(tree->left);
			printf("->%s", tree->u.sym->name);
			break;
		case BRACEDINIT:
			if (tree->left->type != COMMALIST)
				printf("{ ");
			else
			{
				printf("{\n");
				indlev += 2; indent();
			}
			ast_expr_show(tree->left);
			if (tree->left->type != COMMALIST)
				printf(" }");
			else
			{
				printf("\n");
				indlev--; indent(); indlev--;
				printf("}");
			}
			break;
		case CASTEXPR:
			printf("(");
			ast_decl_show(tree->u.dtype);
			printf(") ");
			ast_expr_show(tree->left);
			break;
		case CONDEXPR:
			ast_expr_show(tree->u.cond);
			printf(" ? ");
			ast_expr_show(tree->left);
			printf(" : ");
			ast_expr_show(tree->right);
			break;
		case UOP:
			printf("%s", UOP_symbols[tree->opid]);
			if (tree->opid == UOP_sizeoftype || tree->opid == UOP_sizeof)
				printf("(");
			if (tree->opid == UOP_sizeoftype || tree->opid == UOP_typetrick)
				ast_decl_show(tree->u.dtype);
			else
				ast_expr_show(tree->left);
			if (tree->opid == UOP_paren || tree->opid == UOP_sizeoftype
			    || tree->opid == UOP_sizeof)
				printf(")");
			break;
		case BOP:
			ast_expr_show(tree->left);
			printf(" %s ", BOP_symbols[tree->opid]);
			ast_expr_show(tree->right);
			break;
		case PREOP:
			printf("%s", UOP_symbols[tree->opid]);
			ast_expr_show(tree->left);
			break;
		case POSTOP:
			ast_expr_show(tree->left);
			printf("%s", UOP_symbols[tree->opid]);
			break;
		case ASS:
			ast_expr_show(tree->left);
			printf(" %s ", ASS_symbols[tree->opid]);
			ast_expr_show(tree->right);
			break;
		case DESIGNATED:
			ast_expr_show(tree->left);
			printf(" = ");
			ast_expr_show(tree->right);
			break;
		case IDXDES:
			printf("[");
			ast_expr_show(tree->left);
			printf("]");
			break;
		case DOTDES:
			printf(".%s", tree->u.sym->name);
			break;
		case COMMALIST:
		case SPACELIST:
			ast_expr_show(tree->left);
			printf("%s", tree->type == COMMALIST ? ", " : " ");
			ast_expr_show(tree->right);
			break;
		default:
			fprintf(stderr, "[ast_expr_show]: b u g (type=%d)!!\n", tree->type);
	}
}


void ast_spec_show(astspec tree)
{
	switch (tree->type)
	{
		case SPEC:
		case STCLASSSPEC:
			printf("%s", SPEC_symbols[tree->subtype]);
			break;
		case USERTYPE:
			printf("%s", tree->name->name);
			break;
		case SUE:
			switch (tree->subtype)
			{
				case SPEC_enum:
					printf("enum");
					if (tree->name)
						printf(" %s", tree->name->name);
					if (tree->sueattr)
					{
						printf(" ");
						ast_spec_show(tree->sueattr);
					}
					if (tree->body)
					{
						printf(" {\n");
						indlev += 2 ; indent();
						ast_spec_show(tree->body);
						printf("\n");
						indlev--; indent(); indlev--;
						printf("}");
					}
					break;
				case SPEC_struct:
				case SPEC_union:
					printf(tree->subtype == SPEC_struct ? "struct" : "union");
					if (tree->sueattr)
					{
						printf(" ");
						ast_spec_show(tree->sueattr);
					}
					if (tree->name)
						printf(" %s", tree->name->name);
					if (tree->u.decl)
					{
						printf(" {\n");
						indlev += 2; indent();
						ast_decl_show(tree->u.decl);
						printf("\n");
						indlev--; indent(); indlev--;
						printf("}");
					}
					break;
				default:
					fprintf(stderr, "[ast_spec_show]: SUE b u g !!\n");
			}
			break;
		case ENUMERATOR:
			printf("%s", tree->name->name);
			if (tree->u.expr)
			{
				printf(" = ");
				ast_expr_show(tree->u.expr);
			}
			break;
		case SPECLIST:
			switch (tree->subtype)
			{
				case SPEC_Rlist:
					ast_spec_show(tree->body);
					if (tree->body->type != SPEC || tree->body->subtype != SPEC_star)
						printf(" ");       /* No spaces among consecutive stars */
					ast_spec_show(tree->u.next);
					break;
				case SPEC_Llist:
					ast_spec_show(tree->u.next);
					printf(" ");
					ast_spec_show(tree->body);
					break;
				case SPEC_enum:
					ast_spec_show(tree->u.next);
					printf(", ");
					ast_spec_show(tree->body);
					break;
				default:
					fprintf(stderr, "[ast_spec_show]: list b u g !!\n");
			}
			break;
		case ATTRSPEC:
			printf("__attribute__(( %s ))", tree->u.txt ? tree->u.txt : "");
			break;
		default:
			fprintf(stderr, "[ast_spec_show]: b u g !!\n");
	}
	fflush(stdout);
}


void ast_decl_show(astdecl tree)
{
	switch (tree->type)
	{
		case DIDENT:
			printf("%s", tree->u.id->name);
			break;
		case DPAREN:
			printf("(");
			ast_decl_show(tree->decl);
			printf(")");
			break;
		case DARRAY:
			if (tree->decl) /* Maybe abstract declarator */
				ast_decl_show(tree->decl);
			printf("[");
			if (tree->spec)
				ast_spec_show(tree->spec);
			if (tree->u.expr)
			{
				printf(" ");
				ast_expr_show(tree->u.expr);
			}
			printf("]");
			break;
		case DFUNC:      /* Maybe abstract declarator */
			if (tree->decl)
				ast_decl_show(tree->decl);
			printf("(");
			if (tree->u.params)
				ast_decl_show(tree->u.params);
			printf(")");
			break;
		case DINIT:
			ast_decl_show(tree->decl);
			if (tree->u.expr != NULL)
			{
				printf(" = ");
				ast_expr_show(tree->u.expr);
			}
			break;
		case DECLARATOR:
			if (tree->spec)      /* pointer */
			{
				ast_spec_show(tree->spec);
				printf(" ");
			}
			ast_decl_show(tree->decl);
			break;
		case ABSDECLARATOR:
			if (tree->spec)      /* pointer */
				ast_spec_show(tree->spec);
			if (tree->decl)
			{
				if (tree->spec) printf(" ");
				ast_decl_show(tree->decl);
			}
			break;
		case DPARAM:
			ast_spec_show(tree->spec);
			if (tree->decl)
			{
				printf(" ");
				ast_decl_show(tree->decl);
			}
			break;
		case DELLIPSIS:
			printf("...");
			break;
		case DBIT:
			if (tree->decl)
				ast_decl_show(tree->decl);
			printf(" : ");
			ast_expr_show(tree->u.expr);
			break;
		case DSTRUCTFIELD:
			if (tree->spec)      /* pointer */
			{
				ast_spec_show(tree->spec);
				printf(" ");
			}
			if (tree->decl)
				ast_decl_show(tree->decl);
			printf(";");
			break;
		case DCASTTYPE:
			ast_spec_show(tree->spec);
			if (tree->decl)
			{
				printf(" ");
				ast_decl_show(tree->decl);
			}
			break;
		case DLIST:
			switch (tree->subtype)
			{
				case DECL_decllist:
				case DECL_idlist:
				case DECL_paramlist:
					if (tree->u.next == NULL || tree->decl == NULL)
					{
						fprintf(stderr, "[ast_decl_show]: list next/body NULL !!\n");
						break;
					}
					ast_decl_show(tree->u.next);
					printf(", ");
					ast_decl_show(tree->decl);
					break;
				case DECL_fieldlist:
					ast_decl_show(tree->u.next);
					printf("\n"); indent();
					ast_decl_show(tree->decl);
					break;
				default:
					fprintf(stderr, "[ast_decl_show]: list b u g !!\n");
			}
			break;
		default:
			fprintf(stderr, "[ast_decl_show]: b u g !!\n");
	}
	fflush(stdout);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OpenMP NODES                                              *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static void ast_omparrdim_show(omparrdim d)
{
	printf("[");
	ast_expr_show(d->lb);
	printf(" : ");
	if (d->len)
		ast_expr_show(d->len);
	printf("]");
}


static void ast_ompxlione_show(ompxli xl)
{
	omparrdim s;
	
	printf("%s", xl->id->name);
	if (xl->xlitype == OXLI_ARRSEC)
		for (s = xl->dim; s; s = s->next)
			ast_omparrdim_show(s);
}


static void ast_ompxli_show(ompxli xl)
{
	for (; xl; xl = xl->next)
	{
		ast_ompxlione_show(xl);
		if (xl->next) printf(", ");
	}
	fflush(stdout);
}


void ast_ompclause_show(ompclause t)
{
	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
		{
			ast_ompclause_show(t->u.list.next);
			printf(" ");
		}
		assert((t = t->u.list.elem) != NULL);
	}

	printf("%s", clausenames[t->type]);
	switch (t->type)
	{
		case OCIF:
			if (t->modifier != OCM_none)
				printf("(%s: ", clausemods[t->modifier]);
			else
				printf("("); 
			ast_expr_show(t->u.expr); printf(")");
			break;
		case OCFINAL:
		case OCNUMTHREADS:
		case OCDEVICE:
		case OCNUMTEAMS:
		case OCTHREADLIMIT:
		case OCHINT:
		case OCPRIORITY:
			printf("("); ast_expr_show(t->u.expr); printf(")");
			break;
		case OCSCHEDULE:
			printf("(");
			if (t->modifier != OCM_none)
				printf("%s:", clausemods[t->modifier]);
			printf("%s%s", clausesubs[t->subtype], t->u.expr ? ", " : " ");
			if (t->u.expr)
				ast_expr_show(t->u.expr);
			printf(")");
			break;
		case OCDEFAULT:
		case OCPROCBIND:
			printf("(%s)", clausesubs[t->subtype]);
			break;
		case OCDEPEND:
			if (t->subtype == OC_sink)
			{
				printf("(%s: ", clausesubs[t->subtype]);
				if (t->u.expr)
					ast_expr_show(t->u.expr);
				printf(")");
				break;
			}
			if (t->subtype == OC_source)
			{
				printf("(%s)", clausesubs[t->subtype]);
				break;
			}
		case OCREDUCTION:
		case OCMAP:
			if (t->modifier != OCM_none)
				printf("(%s,%s:  ", clausemods[t->modifier], clausesubs[t->subtype]);
			else
				printf("(%s: ", clausesubs[t->subtype]);
			ast_ompxli_show(t->u.xlist);
			printf(")");
			break;
		case OCTO:
		case OCFROM:
		case OCLINK:
			printf("(");
			ast_ompxli_show(t->u.xlist);
			printf(")");
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
			printf("("); ast_decl_show(t->u.varlist); printf(")");
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
			printf("(%d)", t->subtype);
			break;
	}
}


void ast_ompdir_show(ompdir t)
{
	printf("#pragma omp %s ", ompdirnames[t->type]);
	switch (t->type)
	{
		case DCCRITICAL:
			if (t->u.region)
			{
				printf("(%s)", t->u.region->name);
				if (t->clauses)
					ast_ompclause_show(t->clauses);
			}
			break;
		case DCFLUSH:
			if (t->u.varlist)
			{
				printf("(");
				ast_decl_show(t->u.varlist);
				printf(")");
			}
			break;
		case DCTHREADPRIVATE:
			if (t->u.varlist)
			{
				printf("(");
				ast_decl_show(t->u.varlist);
				printf(")");
			}
			break;
		default:
			if (t->clauses)
				ast_ompclause_show(t->clauses);
			break;
	}
	printf("\n");
}


void ast_ompcon_show(ompcon t)
{
	ast_ompdir_show(t->directive);
	if (t->body)     /* barrier & flush don't have a body. */
	{
		indent();
		ast_stmt_show(t->body);
	}
	if (t->type == DCDECLTARGET && t->body)  /* v4.0 style */
		printf("#pragma omp end %s\n", ompdirnames[t->type]);
}


void ast_show(aststmt tree)
{
	_curfile = Symbol(filename);
	indlev = 0;
	ast_stmt_show(tree);
}

static void show_stderr(void *tree, void (*func)(void *))
{
	int backup = dup(1);     /* duplicate stdout */
	
	fflush(stdout);          /* drain it */
	dup2(2, 1);              /* stderr to stdout */

	func(tree);

	fflush(stdout);          /* drain */

	dup2(backup, 1);         /* restore stdout */
	close(backup);
}

void ast_expr_show_stderr(astexpr tree)
{
	show_stderr(tree, (void (*)(void *)) ast_expr_show);
}


void ast_spec_show_stderr(astspec tree)
{
	show_stderr(tree, (void (*)(void *)) ast_spec_show);
}


void ast_decl_show_stderr(astdecl tree)
{
	show_stderr(tree, (void (*)(void *)) ast_decl_show);
}


void ast_show_stderr(aststmt tree)
{
	show_stderr(tree, (void (*)(void *)) ast_show);
}


void ast_ompdir_show_stderr(ompdir tree)
{
	show_stderr(tree, (void (*)(void *)) ast_ompdir_show);
}


void ast_ompclause_show_stderr(ompclause tree)
{
	show_stderr(tree, (void (*)(void *)) ast_ompclause_show);
}


void ast_ompxli_show_stderr(ompxli tree)
{
	show_stderr(tree, (void (*)(void *)) ast_ompxlione_show);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OMPi-EXTENSION NODES                                      *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void ast_oxclause_show(oxclause t)
{
	if (t->type == OX_OCLIST)
	{
		if (t->u.list.next != NULL)
		{
			ast_oxclause_show(t->u.list.next);
			printf(" ");
		}
		assert((t = t->u.list.elem) != NULL);
	}

	printf("%s", oxclausenames[t->type]);
	switch (t->type)
	{
		case OX_OCREDUCE:
			printf("(%s : ", clausesubs[t->operator]);
			ast_decl_show(t->u.varlist);
			printf(")");
			break;
		case OX_OCIN:
		case OX_OCOUT:
		case OX_OCINOUT:
			printf("("); ast_decl_show(t->u.varlist); printf(")");
			break;
		case OX_OCATNODE:
		case OX_OCATWORKER:
		case OX_OCIF:
		case OX_OCHINTS:
		case OX_OCSTART:
		case OX_OCSTRIDE:
			printf("("); ast_expr_show(t->u.expr); printf(")");
			break;
		case OX_OCLOCAL:
		case OX_OCREMOTE:
			break;
		case OX_OCSCOPE:
			printf("scope(%s)", t->u.value == OX_SCOPE_NODES ? "nodes" :
			       t->u.value == OX_SCOPE_WLOCAL ? "workers,local" :
			       t->u.value == OX_SCOPE_WGLOBAL ? "workers,global" :
			       "???");
			break;
	}
}


void ast_oxdir_show(oxdir t)
{
	printf("#pragma ompix %s ", oxdirnames[t->type]);
	if (t->clauses)
		ast_oxclause_show(t->clauses);
	printf("\n");
}


void ast_oxcon_show(oxcon t)
{
	ast_oxdir_show(t->directive);
	if (t->body)
	{
		indent();
		ast_stmt_show(t->body);
	}
}
