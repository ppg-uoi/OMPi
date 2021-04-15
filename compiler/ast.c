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

/* AST - the Abstract Syntax Tree */

/*
 * 2009/05/11:
 *   added AUTO schedule type
 * 2009/05/03:
 *   added ATNODE ompix clause
 */

#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include "ast.h"
#include "ast_copy.h"
#include "ast_show.h"
#include "ompi.h"
#include "dfa.h"
#include "set.h"
#include "ast_free.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     EXRESSION NODES                                           *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


char *UOP_symbols[11] =
{
	"&", "*", "-", "~", "!", "sizeof", "sizeof", "++", "--", "(", "\0"
},
*BOP_symbols[19] =
{
	"<<", ">>", "<=", ">=", "==", "!=", "&&", "||", "&", "|", "^",
	"+", "-", "<", ">", "*", "/", "%", "cast"
},
*ASS_symbols[11] =
{
	"=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|="
};


astexpr Astexpr(enum exprtype type, astexpr left, astexpr right)
{
	astexpr n = smalloc(sizeof(struct astexpr_));
	n->type   = type;
	n->left   = left;
	n->right  = right;
	n->opid   = 0;
	n->l      = sc_original_line();
	n->c      = sc_column();
	n->file   = Symbol(sc_original_file());
	return (n);
}

astexpr Identifier(symbol s)
{
	astexpr n = Astexpr(IDENT, NULL, NULL);
	n->u.sym = s;
	return (n);
}

astexpr Constant(char *s)
{
	astexpr n = Astexpr(CONSTVAL, NULL, NULL);
	n->u.str = s;
	return (n);
}

astexpr numConstant(int n)
{
	static char numstr[64];
	snprintf(numstr, 63, "%d", (n >= 0) ? n : -n);
	return n >= 0 ? Constant(strdup(numstr)) : 
	                UnaryOperator(UOP_neg, Constant(strdup(numstr)));
}

astexpr String(char *s)
{
	astexpr n = Astexpr(STRING, NULL, NULL);
	n->u.str = s;
	return (n);
}

astexpr DotField(astexpr e, symbol s)
{
	astexpr n = Astexpr(DOTFIELD, e, NULL);
	n->u.sym = s;
	return (n);
}

astexpr PtrField(astexpr e, symbol s)
{
	astexpr n = Astexpr(PTRFIELD, e, NULL);
	n->u.sym = s;
	return (n);
}

astexpr Operator(enum exprtype type, int opid, astexpr left, astexpr right)
{
	astexpr n = Astexpr(type, left, right);
	n->opid = opid;
	return (n);
}

astexpr ConditionalExpr(astexpr cond, astexpr t, astexpr f)
{
	astexpr n = Astexpr(CONDEXPR, t, f);
	n->u.cond = cond;
	return (n);
}

astexpr DotDesignator(symbol s)
{
	astexpr n = Astexpr(DOTDES, NULL, NULL);
	n->u.sym = s;
	return (n);
}

astexpr CastedExpr(astdecl d, astexpr e)
{
	astexpr n = Astexpr(CASTEXPR, e, NULL);
	n->u.dtype = d;
	return (n);
}

astexpr Sizeoftype(astdecl d)
{
	astexpr n = UnaryOperator(UOP_sizeoftype, NULL);
	n->u.dtype = d;
	return (n);
}


astexpr TypeTrick(astdecl d)
{
	astexpr n = UnaryOperator(UOP_typetrick, NULL);
	n->u.dtype = d;
	return (n);
}


/* Find the number of elements in a list (1 if not a COMMA/SPACE LIST) */
int expr_list_cardinality(astexpr expr)
{
	if (expr->type != COMMALIST && expr->type != SPACELIST)
		return (1);
	return ( expr_list_cardinality(expr->left) + 
	         expr_list_cardinality(expr->right) );
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     DECLARATION NODES                                         *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/*
 * SPECIFIER NODES
 */

char *SPEC_symbols[28] =
{
	NULL, "typedef", "extern", "static", "auto", "register", "void",
	"char", "short", "int", "long", "float", "double", "signed",
	"unsigned", "_Bool", "_Complex", "_Imaginary", "struct",
	"union", "enum", "const", "restrict", "volatile", "inline",
	"*", "specslistL", "speclistR"
};


astspec Specifier(enum spectype type, int subtype, symbol name, astspec body)
{
	astspec d  = smalloc(sizeof(struct astspec_));
	d->type    = type;
	d->subtype = subtype;
	d->name    = name;
	d->body    = body;
	d->sueattr = NULL;
	d->u.next  = NULL;
	d->l       = sc_original_line();
	d->c       = sc_column();
	d->file    = Symbol(sc_original_file());
	return (d);
}


astspec Enumerator(symbol name, astexpr expr)
{
	astspec d  = Specifier(ENUMERATOR, 0, name, NULL);
	d->u.expr = expr;
	return (d);
}


astspec Specifierlist(int type, astspec e, astspec l)
{
	astspec d  = Specifier(SPECLIST, type, NULL, e);
	d->u.next = l;
	return (d);
}


astspec SUdecl(int type, symbol sym, astdecl decl, astspec attr)
{
	astspec d = Specifier(SUE, type, sym, NULL);
	d->sueattr = attr;
	d->u.decl = decl;
	return (d);
}


astspec Enumdecl(symbol sym, astspec body, astspec attr)
{
	astspec d = Specifier(SUE, SPEC_enum, sym, body);
	d->sueattr = attr;
	return (d);
}


astspec AttrSpec(char *s)
{
	astspec d = Specifier(ATTRSPEC, 0, NULL, NULL);
	d->u.txt = s;
	return (d);
}


/*
 * DECLARATOR NODES
 */


astdecl Decl(enum decltype type, int subtype, astdecl decl, astspec spec)
{
	astdecl d  = smalloc(sizeof(struct astdecl_));
	d->type    = type;
	d->subtype = subtype;
	d->decl    = decl;
	d->spec    = spec;
	d->u.next  = NULL;
	d->l       = sc_original_line();
	d->c       = sc_column();
	d->file    = Symbol(sc_original_file());
	return (d);
}


astdecl IdentifierDecl(symbol s)
{
	astdecl d = Decl(DIDENT, 0, NULL, NULL);
	d->u.id   = s;
	return (d);
}


astdecl ArrayDecl(astdecl decl, astspec s, astexpr e)
{
	astdecl d = Decl(DARRAY, 0, decl, s);
	d->u.expr = e;
	return (d);
}


astdecl FuncDecl(astdecl decl, astdecl p)
{
	astdecl d = Decl(DFUNC, 0, decl, NULL);
	d->u.params = p;
	return (d);
}


astdecl InitDecl(astdecl decl, astexpr e)
{
	astdecl d = Decl(DINIT, 0, decl, NULL);
	d->u.expr = e;
	return (d);
}


astdecl BitDecl(astdecl decl, astexpr e)
{
	astdecl d = Decl(DBIT, 0, decl, NULL);
	d->u.expr = e;
	return (d);
}


astdecl Declanylist(int subtype, astdecl l, astdecl e)
{
	astdecl d = Decl(DLIST, subtype, e, NULL);
	d->u.next = l;
	return (d);
}


/* Get the type of identifier declared - can be:
 *   - a scalar one
 *   - an array
 *   - a function
 * d is assumed to be a declarator node. As such it only has a ptr and
 * a direct_declarator child.
 */
int decl_getkind(astdecl d)
{
	assert(d->type == DECLARATOR);
	if (decl_ispointer(d)) return (DIDENT);   /* pointers are scalar */
	d = d->decl;                              /* direct_declarator */

	switch (d->type)
	{
		case DPAREN:
			return (decl_getkind(d->decl));
		case DFUNC:
			return (DFUNC);
		case DARRAY:
			return (DARRAY);
		case DIDENT:
			return (DIDENT);
		case DECLARATOR:       /* Should not happen normally */
			return (decl_getkind(d));
		default:
			exit_error(1, "[decl_getkind]: unexpected declarator type %d\n", d->type);
	}
	return (0);
}


/* Determine whether the declarator is a pointer. Here we are based on the
 * fact that the parser has removed redundant parenthesis, i.e.
 * (IDENT) has been converted to IDENT. Thus we can only have a pointer
 * if we are in a situation like ...(*IDENT)...; i.e. an identifier
 * declarator with pointer specifier, or ...(*)... for an abstract declarator.
 */
int decl_ispointer(astdecl d)
{
	if (d->type == DECLARATOR || d->type == ABSDECLARATOR)
	{
		if (d->spec != NULL  &&
		    (d->decl == NULL || (d->decl->type == DIDENT &&
		                         speclist_getspec(d->spec, SPEC, SPEC_star) != NULL)))
			return (1);
	}
	if (d->type == DIDENT)
		return (0);
	else
		if (d->type == DLIST)  /* Should be DECL_decllist */
			return (decl_ispointer(d->u.next));
		else
			return (decl_ispointer(d->decl));
}


/* Determine whether the declarator is a function returning a pointer. 
 * It ia assumed that this comes from a declaration statement or a function
 * definition statement.
 * We keep recursing on the decl field until we hit a DFUNC.
 */
int func_returnspointer(astdecl d)
{
	if (d == NULL) return 0;
	
	if (d->type == DECLARATOR)
		if (d->decl && d->decl->type == DFUNC)
			return (d->spec && speclist_getspec(d->spec, SPEC, SPEC_star));
	if (d->type == DIDENT)
		return (0);
	else
		if (d->type == DLIST)  /* Should never happen */
			return (func_returnspointer(d->u.next));
		else
			return (func_returnspointer(d->decl));
}


/* Get the identifier name of the declarator.
 * d is assumed to be the declarator part of a declaration (top-level).
 * It will crash if given an ABSDECLARATOR with no identifier!
 */
astdecl decl_getidentifier(astdecl d)
{
	if (d->type == DIDENT)
		return (d);
	else
		if (d->type == DLIST)  /* Should be DECL_decllist */
			return (decl_getidentifier(d->u.next));
		else
			return (decl_getidentifier(d->decl));
}


/* Finds the identifier and replaces it with a pointer to it.
 * Returns the declarator itself.
 */
astdecl decl_topointer(astdecl decl)
{
	astdecl newdecl, id = decl_getidentifier(decl);

	newdecl = ParenDecl(Declarator(Pointer(), ast_decl_copy(id)));
	*id = *newdecl;
	free(newdecl);
	return (decl);
}


/* Finds the identifier and replaces its name.
 * Returns the declarator itself.
 */
astdecl decl_rename(astdecl decl, symbol newname)
{
	astdecl newid, id = decl_getidentifier(decl);

	newid = IdentifierDecl(newname);
	*id = *newid;
	free(newid);
	return (decl);
}


/* Checks whether the speclist includes the given SPEC type */
astspec speclist_getspec(astspec s, int type, int subtype)
{
	if (s == NULL) return (NULL);
	if (s->type == SPECLIST)
	{
		astspec p;
		if ((p = speclist_getspec(s->body, type, subtype)) != NULL) return (p);
		return (speclist_getspec(s->u.next, type, subtype));
	}
	if (s->type != type) return (NULL);
	if (type != SPEC && type != STCLASSSPEC) return (s);
	return ((s->subtype == subtype) ? s : NULL);
}


/* Removes the given SPEC type from the speclist (once and if present).
 * Returns true if a removal was actually made.
 * We assume the list is indeed a list and not a tree...
 */
static int speclist_delspec(astspec *s, int type, int subtype)
{
	astspec curr = *s, prev = NULL;
	
	if (*s == NULL) return (0);
	for ( ; curr && curr->type == SPECLIST; curr = curr->u.next, prev = curr)
	{
		if (curr->body == NULL) continue;  /* Should never be NULL */
		if (curr->body->type == type && ((type != SPEC && type != STCLASSSPEC) || 
		                                 (curr->body->subtype == subtype)))
		{
			if (prev == NULL)                /* Found it */
				*s = curr->u.next;
			else
				prev->u.next = curr->u.next;
			ast_spec_free(curr);
			return (1);
		}
	}
	if (curr && curr->type != SPECLIST)        /* Last element */
	{
		if (curr->type == type && ((type != SPEC && type != STCLASSSPEC) || 
		                                 (curr->subtype == subtype)))
		{
			if (prev == NULL)                /* Found it */
				*s = NULL;
			else
				prev->u.next = NULL;
			ast_spec_free(curr);
			return (1);
		}
	}
	return (0);
}


/* Take a pointer declarator and transform it to an array of the given size;
 * obviously this works on non-abstract declarators.
 * Returns NULL if not a pointer declarator.
 */
void decl_ptr2arr(astdecl d, astexpr size)
{
	if (d == NULL) 
		return;              /* When recursing from ABSDECLARATOR */
	if (d->type == DECLARATOR)
	{
		if (d->spec != NULL  && (d->decl->type == DIDENT &&
		                         speclist_getspec(d->spec, SPEC,SPEC_star) != NULL))
		{
			astdecl ident = d->decl;
			
			speclist_delspec(&(d->spec), SPEC, SPEC_star);  /* Remove the star */
			d->decl = ArrayDecl(ident, NULL, size);
			return;
		}
	}
	if (d->type == DIDENT)   /* Not a pointer after all */
		return;
	else
		if (d->type == DLIST)  /* Should be DECL_decllist */
			decl_ptr2arr(d->u.next, size);
		else
			decl_ptr2arr(d->decl, size);
}


/* This takes an array declarator and reduces it to a simple pointer
 */
void decl_arr2ptr(astdecl d)
{
	if (!d);
	switch (d->type)
	{
		case DECLARATOR:
			decl_arr2ptr(d->decl);
			break;
		case DPAREN:         /* cannot have (id)[10] -- see parser.y */
			decl_arr2ptr(d->decl);
			break;
		case DARRAY:
			if (d->decl->type != DIDENT)
				decl_arr2ptr(d->decl);
			else   /* Got it */
			{
				astdecl t = ParenDecl(Declarator(Pointer(), d->decl));
				if (d->u.expr) ast_expr_free(d->u.expr);
				if (d->spec)   ast_spec_free(d->spec);
				*d = *t;
				free(t);
			}
			break;
	}
}


/* Find the number of top-level elements in an initializer (DINIT) */
int decl_initializer_cardinality(astdecl decl)
{
	int     count;
	astexpr expr = decl->u.expr;
	
	if (decl->type != DINIT)    /* No initializer */
		return (0);              
	if (expr->type != BRACEDINIT)     /* No braces */
		return (1);
	return ( expr_list_cardinality(expr->left) );
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     STATEMENT NODES                                           *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


aststmt Statement(enum stmttype type, int subtype, aststmt body)
{
	aststmt s  = smalloc(sizeof(struct aststmt_));
	s->type    = type;
	s->subtype = subtype;
	s->body    = body;
	s->parent  = NULL;
	s->l       = sc_original_line();
	s->c       = sc_column();
	s->file    = Symbol(sc_original_file());
	return (s);
}


aststmt Goto(symbol s)
{
	aststmt n = Statement(JUMP, SGOTO, NULL);
	n->u.label = s;
	return (n);
}


aststmt Jumpstatement(int subtype, astexpr expr)
{
	aststmt s = Statement(JUMP, subtype, NULL);
	s->u.expr = expr;
	return (s);
}


aststmt Iterationstatement(int subtype,
                           aststmt init, astexpr cond, astexpr incr, aststmt body)
{
	aststmt n = Statement(ITERATION, subtype, body);
	n->u.iteration.init = init;   /* Maybe declaration or expression */
	n->u.iteration.cond = cond;
	n->u.iteration.incr = incr;
	return (n);
}


aststmt Selectionstatement(int subtype,
                           astexpr cond, aststmt body, aststmt elsebody)
{
	aststmt n = Statement(SELECTION, subtype, body);
	n->u.selection.cond = cond;
	n->u.selection.elsebody = elsebody;
	return (n);
}


aststmt LabeledStatement(int subtype, symbol l, astexpr e, aststmt st)
{
	aststmt s = Statement(LABELED, subtype, st);
	if (subtype == SLABEL)
		s->u.label = l;
	else
		s->u.expr = e;
	return (s);
}


aststmt Expression(astexpr expr)
{
	aststmt s = Statement(EXPRESSION, 0, NULL);
	s->u.expr = expr;
	return (s);
}


aststmt Declaration(astspec spec, astdecl decl)
{
	aststmt s = Statement(DECLARATION, 0, NULL);
	s->u.declaration.spec = spec;
	s->u.declaration.decl = decl;
	return (s);
}


aststmt BlockList(aststmt l, aststmt st)
{
	aststmt s = Statement(STATEMENTLIST, 0, st);
	s->u.next = l;
	return (s);
}


aststmt FuncDef(astspec spec, astdecl decl, aststmt dlist, aststmt body)
{
	aststmt s = Statement(FUNCDEF, 0, body);
	s->u.declaration.spec  = spec;
	s->u.declaration.decl  = decl;
	s->u.declaration.dlist = dlist;

	/***************** Agelos ***************************************/
	/* We store the funcdef node for interprocedural data flow analysis purposes.
	* **************************************************************/
	dfa_userfunc_add(decl->decl->decl->u.id, ast_stmt_copy(body));

	return (s);
}


aststmt OmpStmt(ompcon omp)
{
	aststmt s = Statement(OMPSTMT, 0, NULL);
	s->u.omp = omp;
	return (s);
}


aststmt OmpixStmt(oxcon ox)
{
	aststmt s = Statement(OX_STMT, 0, NULL);
	s->u.ox = ox;
	return (s);
}


/* This is used only by the transformation routines,
 * to produce code that is outputed verbatim (as is).
 */
aststmt Verbatim(char *code)
{
	aststmt s = Statement(VERBATIM, 0, NULL);
	s->u.code = code;
	return (s);
}


aststmt verbit(char *format, ...)
{
	static char str[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(str, 1023, format, ap);
	va_end(ap);
	return (Verbatim(strdup(str)));
}


asmop AsmOp(astexpr id, char *con, astexpr var, asmop op, asmop nxt)
{
	asmop s = smalloc(sizeof(struct asmop_));
	s->symbolicname = id;
	s->constraint = con;
	s->var = var;
	s->next = op;
	s->op = nxt;
	return (s);
}


asmnode Asmnode(astspec qual, char *tpl, 
                asmop out, asmop in, astexpr clob, astexpr labs)
{
	asmnode s = smalloc(sizeof(struct asmnode_));
	s->qualifiers = qual;
	s->template   = tpl;
	s->clobbers   = clob;
	s->labels     = labs;
	s->ins        = in;
	s->outs       = out;
	return (s);
}


/* Inline assembly (asm) statements */
aststmt AsmStmt(int subtype, astspec qual, char *tpl, 
                asmop out, asmop in, astexpr clob, astexpr labs)
{
	aststmt s = Statement(ASMSTMT, subtype, NULL);
	s->u.assem = Asmnode(qual, tpl, out, in, clob, labs);
	return (s);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OPENMP NODES                                              *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


char *clausenames[] = { NULL, "nowait", "if", "num_threads", "ordered",
                        "schedule", "copyin", "private", "copyprivate",
                        "firstprivate", "lastprivate", "shared",
                        "default", "reduction", "<list>",
                        "first/lastprivate",
                        "untied", "collapse",  /* OpenMP 3.0 */
                        "final", "mergeable",  /* OpenMP 3.1 */
                        /* OpenMP 4.0 */
                        "proc_bind", "map", "device", "to", "from", "parallel",
                        "sections", "for", "taskgroup", "depend", 
                        "num_teams", "thread_limit",
                        /* OpenMP 4.5 */
                        "hint", "priority", "is_device_ptr", "use_device_ptr",
                        "threads", "link", "defaultmap(tofrom:scalar)",
                        "ordered",
                        /* Aggelos */
                        "auto"
                      };
char *clausesubs[] =  { "static", "dynamic", "guided", "runtime",
                        "shared", "none", "+", "*", "-", "&", "|",
                        "^", "&&", "||", "affinity", "auto",
                        "min", "max",          /* OpenMP 3.1 */
                        /* OpenMP 4.0 */
                        "master", "close", "spread",
                        "alloc", "to", "from", "tofrom",
                        "in", "out", "inout",
                        /* OpenMP 4.5 */
                        "source", "sink", "release", "delete",
                        /* OpenMP 4.5 modifiers */
                        "always",
			/* OpenMP 5.1 */
			"primary"
                      };
char *clausemods[]  = { NULL, "always", "monotonic", "nonmonotonic", "simd",
                        "parallel", "task", "target", "target data",
                        "target enter data", "target exit data", 
                        "target update", "cancel"
                      };
char *ompdirnames[] = { NULL, "parallel", "for", "sections", "section",
                        "single", "parallel for", "parallel sections",
                        "for", "master", "critical", "atomic",
                        "ordered", "barrier", "flush", "threadprivate",
                        "task", "taskwait",     /* OpenMP 3.0 */
                        "taskyield",            /* OpenMP 3.1 */
                        /* OpenMP 4.0 */
                        "target", "target data", "target update",
                        "declare target", "cancel", "cancellation point",
                        "taskgroup", "teams", "target teams",
                        /* OpenMP 4.5 */
                        "target enter data", "target exit data"
                      };


/* (OpenMP 4.0) Extended lists of variables or array sections */
ompxli OmpXLItem(enum ompxli_type type, symbol id, omparrdim dim)
{
	ompxli x   = smalloc(sizeof(struct ompxli_));
	x->xlitype = type;
	x->id      = id;
	x->dim     = dim;
	x->next    = NULL;
	x->l       = sc_original_line();
	x->c       = sc_column();
	x->file    = Symbol(sc_original_file());
	return (x);
}


omparrdim OmpArrDim(astexpr lb, astexpr len)
{
	omparrdim d = smalloc(sizeof(struct omparrdim_));
	d->lb       = lb;
	d->len      = len;
	d->next     = NULL;
}


ompclause
OmpClause(ompclt_e type, ompclsubt_e subtype, ompclmod_e mod, astexpr expr,
          astdecl varlist)
{
	ompclause c    = smalloc(sizeof(struct ompclause_));
	c->parent      = NULL;
	c->type        = type;
	c->subtype     = subtype;
	c->modifier    = mod;
	if (varlist == NULL)
		c->u.expr    = expr;
	else
		c->u.varlist = varlist;
	c->l           = sc_original_line();
	c->c           = sc_column();
	c->file        = Symbol(sc_original_file());
	return (c);
}


ompclause 
OmpXlistClause(ompclt_e type, ompclsubt_e subtype, ompclmod_e mod, ompxli xlist)
{
	ompclause c = OmpClause(type, subtype, mod, NULL, NULL);
	c->u.xlist  = xlist;
	return (c);
}


ompclause OmpClauseList(ompclause next, ompclause elem)
{
	ompclause c    = OmpClause(OCLIST, 0, OCM_none, NULL, NULL);
	c->u.list.elem = elem;
	c->u.list.next = next;
	return (c);
}


ompdir OmpDirective(enum dircontype type, ompclause cla)
{
	ompdir d     = smalloc(sizeof(struct ompdir_));
	d->parent    = NULL;
	d->type      = type;
	d->clauses   = cla;
	d->u.varlist = NULL;
	d->l         = sc_original_line() - 1; /* Cause of the \n at the end */
	d->c         = sc_column();
	d->file      = Symbol(sc_original_file());
	return (d);
}


ompdir OmpCriticalDirective(symbol r, ompclause cla)
{
	ompdir d = OmpDirective(DCCRITICAL, cla);
	d->u.region = r;
	return (d);
}


ompdir OmpFlushDirective(astdecl a)
{
	ompdir d = OmpDirective(DCFLUSH, NULL);
	d->u.varlist = a;
	return (d);
}


ompdir OmpThreadprivateDirective(astdecl a)
{
	ompdir d = OmpDirective(DCTHREADPRIVATE, NULL);
	d->u.varlist = a;
	return (d);
}


ompcon OmpConstruct(enum dircontype type, ompdir dir, aststmt body)
{
	ompcon c     = smalloc(sizeof(struct ompcon_));
	c->parent    = NULL;
	c->type      = type;
	c->directive = dir;
	c->body      = body;
	c->l         = sc_original_line();
	c->c         = sc_column();
	c->file      = Symbol(sc_original_file());
	return (c);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OMPi-EXTENSION NODES                                      *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


char *oxclausenames[19] = { NULL, "IN", "OUT", "INOUT", "<list>", "reduction",
                            "atnode", "atnode(*)", "detached", "tied",
                            "untied", "stride", "start", "scope",
                            "atworker", "if", "atnode(here)", "atnode(remote)",
                            "hints"
                          };
char *oxdirnames[5] = { NULL, "taskdef", "task", "tasksync", "taskschedule" };


oxclause OmpixClause(enum oxclausetype type, astdecl varlist, astexpr expr)
{
	oxclause c     = smalloc(sizeof(struct oxclause_));
	c->parent      = NULL;
	c->type        = type;
	if (expr == NULL)
		c->u.varlist = varlist;
	else
		c->u.expr    = expr;
	c->l           = sc_original_line();
	c->c           = sc_column();
	c->file        = Symbol(sc_original_file());
	return (c);
}


oxclause OmpixClauseList(oxclause next, oxclause elem)
{
	oxclause c     = OmpixClause(OX_OCLIST, NULL, NULL);
	c->u.list.elem = elem;
	c->u.list.next = next;
	return (c);
}


oxclause OmpixReductionClause(int op, astdecl varlist)
{
	oxclause c = OmpixClause(OX_OCREDUCE, varlist, NULL);
	c->operator = op;
	return (c);
}


oxclause OmpixScopeClause(int scope)
{
	oxclause c = OmpixClause(OX_OCSCOPE, NULL, NULL);
	c->u.value = scope;
	return (c);
}


oxdir OmpixDirective(enum oxdircontype type, oxclause cla)
{
	oxdir d    = smalloc(sizeof(struct oxdir_));
	d->parent  = NULL;
	d->type    = type;
	d->clauses = cla;
	d->l       = sc_original_line() - 1; /* Cause of the \n at the end */
	d->c       = sc_column();
	d->file    = Symbol(sc_original_file());
	return (d);
}


oxcon OmpixConstruct(enum oxdircontype type, oxdir dir, aststmt body)
{
	oxcon c      = smalloc(sizeof(struct oxcon_));
	c->parent    = NULL;
	c->type      = type;
	c->directive = dir;
	c->body      = body;
	c->callback  = NULL;
	c->l         = sc_original_line();
	c->c         = sc_column();
	c->file      = Symbol(sc_original_file());
	return (c);
}


oxcon OmpixTaskdef(oxdir dir, aststmt body, aststmt callbackblock)
{
	oxcon c = OmpixConstruct(OX_DCTASKDEF, dir, body);
	c->callback = callbackblock;
	return (c);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     UTILITY FUNCTIONS                                         *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Parentize:
 *    register the parent statement of every statement node.
 * No parents within expressions or declarators/specifiers.
 * For an openmp statement O, the construct's parent and the
 * construct's body parent is node O. The directive's parent is
 * the construct and the clauses' parent is the directive.
 */


void ast_ompclause_parent(ompdir parent, ompclause t)
{
	if (t->type == OCLIST)
	{
		if (t->u.list.next != NULL)
			ast_ompclause_parent(parent, t->u.list.next);
		assert((t = t->u.list.elem) != NULL);
	}
	t->parent = parent;
}


void ast_ompdir_parent(ompcon parent, ompdir t)
{
	t->parent = parent;
	if (t->clauses)
		ast_ompclause_parent(t, t->clauses);
}


void ast_ompcon_parent(aststmt parent, ompcon t)
{
	t->parent = parent;
	ast_ompdir_parent(t, t->directive);
	if (t->body)     /* barrier & flush don't have a body */
		ast_stmt_parent(parent, t->body);
}


void ast_oxclause_parent(oxdir parent, oxclause t)
{
	if (t->type == OX_OCLIST)
	{
		if (t->u.list.next != NULL)
			ast_oxclause_parent(parent, t->u.list.next);
		assert((t = t->u.list.elem) != NULL);
	}
	t->parent = parent;
}


void ast_oxdir_parent(oxcon parent, oxdir t)
{
	t->parent = parent;
	if (t->clauses)
		ast_oxclause_parent(t, t->clauses);
}


void ast_oxcon_parent(aststmt parent, oxcon t)
{
	t->parent = parent;
	ast_oxdir_parent(t, t->directive);
	if (t->body)     /* barrier & flush don't have a body */
		ast_stmt_parent(parent, t->body);
}


void ast_stmt_parent(aststmt parent, aststmt t)
{
	switch (t->type)
	{
		case JUMP:
			break;
		case ITERATION:
			if (t->subtype == SFOR)
				if (t->u.iteration.init != NULL)
					ast_stmt_parent(t, t->u.iteration.init);
			ast_stmt_parent(t, t->body);
			break;
		case SELECTION:
			if (t->subtype == SIF && t->u.selection.elsebody)
				ast_stmt_parent(t, t->u.selection.elsebody);
			ast_stmt_parent(t, t->body);
			break;
		case LABELED:
			ast_stmt_parent(t, t->body);
			break;
		case EXPRESSION:
			break;
		case COMPOUND:
			if (t->body)
				ast_stmt_parent(t, t->body);
			break;
		case STATEMENTLIST:
			ast_stmt_parent(t, t->u.next);
			ast_stmt_parent(t, t->body);
			break;
		case DECLARATION:
			break;
		case FUNCDEF:
			if (t->u.declaration.dlist)
				ast_stmt_parent(t, t->u.declaration.dlist);
			ast_stmt_parent(t, t->body);    /* always non-NULL */
			break;
		case ASMSTMT:
			break;
		case OMPSTMT:
			/* The parent of the construct and its body is this node here */
			ast_ompcon_parent(t, t->u.omp);
			break;
		case VERBATIM:
			break;
		case OX_STMT:
			/* The parent of the construct and its body is this node here */
			ast_oxcon_parent(t, t->u.ox);
			break;
		default:
			fprintf(stderr, "[ast_stmt_parent]: b u g (type = %d)!!\n", t->type);
	}
	t->parent = parent;
}


void ast_parentize(aststmt tree)
{
	ast_stmt_parent(tree, tree);
}


/**
 * Add a statement before another statement in-place
 *
 * @param where The old statement
 * @param what  The statement you want to insert
 */
void ast_stmt_prepend(aststmt where, aststmt what)
{
	aststmt cp = smalloc(sizeof(struct aststmt_));

	*cp = *where;                    // Copy the original

	where->type    = STATEMENTLIST;  // Make it a block list
	where->subtype = 0;

	where->body    = cp;             // First the original one
	where->u.next  = what;           // Then the new one

	where->parent = cp->parent;      // Parentize
	cp->parent = where;
	what->parent = where;
}


/**
 * Add a statement after another statement in-place
 *
 * @param where The old statement
 * @param what  The statement you want to insert
 */
void ast_stmt_append(aststmt where, aststmt what)
{
	aststmt cp = smalloc(sizeof(struct aststmt_));

	*cp = *where;                    // Copy the original

	where->type    = STATEMENTLIST;  // Make it a block list
	where->subtype = 0;

	where->body    = what;           // First the new one
	where->u.next  = cp;             // Then the original one

	where->parent = cp->parent;      // Parentize
	cp->parent = where;
	what->parent = where;
}


/* Given a statement, we return the function it belongs to.
 */
aststmt ast_get_enclosing_function(aststmt t)
{
	for (; t != NULL && t->type != FUNCDEF; t = t->parent)
		;     /* Go up the tree till we hit our current FUNCDEF */
	return (t);
}


/* Prepends a specifier to a declaration or function definition statement */
void ast_declordef_addspec(aststmt orig, astspec spec)
{
	if (orig->type != DECLARATION && orig->type != FUNCDEF)
		warning("[ast_declordef_addspec]: (bug) expected a declaration or "
		        "function definition\n\tbut got a node of type %d instead.\n", 
		        orig->type);
	else
		orig->u.declaration.spec = Speclist_right(spec, orig->u.declaration.spec);
}


/* Finds the first non-declaration node or NULL if none
 */
static
aststmt first_nondeclaration(aststmt tree)
{
	aststmt p;

	if (!tree || tree->type == DECLARATION)
		return (NULL);
	if (tree->type != STATEMENTLIST)
		return (tree);
	if ((p = first_nondeclaration(tree->u.next)) != NULL)
		return (p);
	else
		return (first_nondeclaration(tree->body));
}


/* Inserts a statement after the declaration section in a compound
 */
void ast_compound_insert_statement(aststmt tree, aststmt t)
{
	aststmt p;

	if (tree->type != COMPOUND)
		return;
	if ((p = first_nondeclaration(tree->body)) == NULL)  /* NULL or only decls */
	{
		if (tree->body == NULL)
		{
			tree->body = t;
			t->parent = tree;
		}
		else
		{
			tree->body = BlockList(tree->body, t);
			tree->body->parent = tree;
			tree->body->u.next->parent = tree->body;
			t->parent = tree->body;
		}
		return;
	}
	tree = Statement(p->type, p->subtype, p->body);
	*tree = *p;
	tree->parent = p;
	t->parent = p;
	p->type = STATEMENTLIST;
	p->subtype = 0;
	p->body = tree;
	p->u.next = t;
}


/* Given a statement, return the closest enclosing OpenMP construct of the
 * given type (or of any type if type is 0); returns NULL if none found.
 */
ompcon ast_get_enclosing_ompcon(aststmt t, enum dircontype type)
{
	for (; t != NULL && t->type != FUNCDEF; t = t->parent)
		if (t->type == OMPSTMT)
		{
			/* While "sections" is a construct, "section" is not */
			if (type == 0 && t->u.omp->type == DCSECTION)
				continue;

			if (type == 0 || t->u.omp->type == type)
				return (t->u.omp);
		};
	return (NULL);
}


/* LINEARIZE:
 * convert a STATEMENTLIST-based tree to an equivallent one
 * where every STATEMENTLIST node has a STATEMENTLIST as a body and
 * a STATEMENT as u.next (except the last one, of course).
 * This form allows easy splitting of the statement list.
 */

SET_TYPE_DEFINE(linearize, aststmt, char, 1031)
SET_TYPE_IMPLEMENT(linearize)

static set(linearize) progstmts;
static void progorder(aststmt tree, int dofree)
{
	if (tree->type != STATEMENTLIST)
		set_put(progstmts, tree);   /* visited in program order */
	else
	{
		progorder(tree->u.next, 1);
		progorder(tree->body, 1);
		if (dofree)
			free(tree);
	}
}


void ast_linearize(aststmt tree)
{
	aststmt            s, parent = tree->parent;
	setelem(linearize) e;

	if (tree->type != STATEMENTLIST)
		return;

	set_init(linearize, &progstmts);
	progorder(tree, 0);
	e = progstmts->last;
	if (set_size(progstmts) == 1)   /* Just 1 statement */
	{
		*tree = *(e->key);
		free(e->key);
	}
	else  /* Two or more elements - form a rightward spine */
	{
		s = BlockList(e->next->key, e->key);
		e = e->next;
		while ((e = e->next) != NULL)
			s = BlockList(e->key, s);
		*tree = *s;
		free(e);
	}
	tree->parent = parent;
	set_drain(progstmts);
}
