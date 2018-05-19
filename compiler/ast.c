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
#include "ompi.h"
#include "dfa.h"
#include "set.h"


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
	snprintf(numstr, 63, "%d", n);
	return (Constant(strdup(numstr)));
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


astspec SUdecl(int type, symbol sym, astdecl decl)
{
	astspec d = Specifier(SUE, type, sym, NULL);
	d->u.decl = decl;
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
	addToFuncList(decl->decl->decl->u.id, ast_stmt_copy(body), &dfa_funcListStart);

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
                        "sections", "for", "taskgroup"
                      };
char *clausesubs[] =  { "static", "dynamic", "guided", "runtime",
                        "shared", "none", "+", "*", "-", "&", "|",
                        "^", "&&", "||", "affinity", "auto",
                        "min", "max",          /* OpenMP 3.1 */
                        /* OpenMP 4.0 */
                        "master", "close", "spread",
                        "alloc", "to", "from", "tofrom"
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
                        "taskgroup"
                      };


ompclause
OmpClause(enum clausetype type, enum clausesubt subtype, astexpr expr,
          astdecl varlist)
{
	ompclause c    = smalloc(sizeof(struct ompclause_));
	c->parent      = NULL;
	c->type        = type;
	c->subtype     = subtype;
	if (varlist == NULL)
		c->u.expr    = expr;
	else
		c->u.varlist = varlist;
	c->l           = sc_original_line();
	c->c           = sc_column();
	c->file        = Symbol(sc_original_file());
	return (c);
}


ompclause OmpClauseList(ompclause next, ompclause elem)
{
	ompclause c    = OmpClause(OCLIST, 0, NULL, NULL);
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


ompdir OmpCriticalDirective(symbol r)
{
	ompdir d = OmpDirective(DCCRITICAL, NULL);
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


char *oxclausenames[15] = { NULL, "IN", "OUT", "INOUT", "<list>", "reduction",
                            "atnode", "atnode(*)", "detached", "tied",
                            "untied", "stride", "start", "scope",
                            "atworker"
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


/* Given a statement, we return the function it belongs to.
 */
aststmt ast_get_enclosing_function(aststmt t)
{
	for (; t != NULL && t->type != FUNCDEF; t = t->parent)
		;     /* Go up the tree till we hit our current FUNCDEF */
	return (t);
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
