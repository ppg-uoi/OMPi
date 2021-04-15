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

/* builder.c -- bits and pieces needed for building the final code */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include "ompi.h"
#include "ast_xform.h"
#include "x_clauses.h"
#include "x_target.h"
#include "x_decltarg.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *  TOP AND BOTTOM STATEMENT LISTS GOING INTO GENERATED CODE     *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static aststmt build_head = NULL, /* Statements injected @ top and .. */
               build_tail = NULL; /* .. injected @ bottom of generated code */

               
static void aststmt_add(aststmt *to, aststmt s)
{
	if (*to == NULL)
		*to = s;
	else
	{
		aststmt t = *to;
		*to = BlockList(t, s);
		t->parent = *to;          /* Parentize correctly */
		s->parent = *to;
	}
}


void bld_head_add(aststmt s)
{
	aststmt_add(&build_head, s);
}


void bld_tail_add(aststmt s)
{
	aststmt_add(&build_tail, s);
}


stentry bld_globalvar_add(aststmt s)
{
	astdecl decl;
	stentry e;

	/* Add in the statement list */
	bld_head_add(s);

	/* Add in the symbol table */
	assert(s->type == DECLARATION);    /* Declare it, too */
	decl = s->u.declaration.decl;
	e = symtab_insert_global(stab, decl_getidentifier_symbol(decl), IDNAME);
	if (decl->type == DINIT)
		decl = (e->idecl = decl)->decl;
	e->decl       = decl;
	e->spec       = s->u.declaration.spec;
	e->isarray    = (decl_getkind(decl) == DARRAY);
	e->isthrpriv  = false;
	e->pval       = NULL;
	e->scopelevel = 0;

	/* If we are in a target, add the variable to the decltarg sets */
	if (inTarget() || inDeclTarget)
	{
		decltarg_inject_newglobal(e->key);
		e->isindevenv = due2DECLTARG; /* could have due2INJECTED but no need */
	}

	return (e);
}


static aststmt head_place(aststmt tree)
{
	aststmt p;

	if (testingmode)
	{
		/* Just place them at the very beginning;
		 * it is guaranteed that the tree begins with a statementlist.
		 */
		for (p = tree; p->type == STATEMENTLIST; p = p->u.next)
			;               /* Go down to the leftmost leaf */
		p = p->parent;    /* Up to parent */
		p->u.next = BlockList(p->u.next, build_head);
		p->u.next->parent       = p;           /* Parentize correctly */
		p->u.next->body->parent = p->u.next;
		build_head->parent      = p->u.next;
		return (tree);
	}

	/* We put all globals right after __ompi_defs__'s declaration
	 * (see bottom of ort.defs).
	 */
	if (tree->type == STATEMENTLIST)
	{
		if ((p = head_place(tree->u.next)) != NULL)
			return (p);
		else
			return (head_place(tree->body));
	}

	/* Try to find where __ompi_defs__ is declared. */
	if (tree->type == DECLARATION &&
	    tree->u.declaration.decl != NULL &&
	    decl_getidentifier(tree->u.declaration.decl)->u.id ==
	    Symbol("__ompi_defs__"))
	{
		p = smalloc(sizeof(struct aststmt_));
		*p = *tree;
		if (cppLineNo)
			*tree = *Block4(
			          p,
			          verbit("# 1 \"%s-newglobals\"", filename),
			          build_head,
			          verbit("# 1 \"%s\"", filename)
			        );
		else
			*tree = *BlockList(p, build_head);
		tree->parent = p->parent;
		p->parent = tree;
		build_head->parent = tree;
		return (tree);
	}
	return (NULL);
}


void bld_headtail_place(aststmt *tree)
{
	if (tree != NULL && build_head != NULL)
		head_place(*tree);
	if (tree != NULL && build_tail != NULL)  /* Cannot ast_stmt_xform(&newtail) */
		*tree = BlockList(*tree, build_tail);  /* .. see x_shglob.c why */

	if (build_head != NULL)
		build_head = NULL;
	if (build_tail != NULL);
		build_tail = NULL;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     THE LIST OF THREAD FUNCTIONS                              *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* All produced thread functions are inserted in this LIFO list.
 * They are transformed seperately and each one gets inserted in the AST,
 * just before the function that created it. The LIFO way guarantees
 * correct placement of nested threads.
 * The whole code here is based on the assumption that the FUNCDEF nodes
 * in the original AST won't change (which does hold since the
 * transformation code---see above---only transforms the body of the FUNCDEF,
 * not the FUNCDEF code itself). Otherwise the ->fromfunc pointers might
 * point to invalid nodes.
 */
typedef struct funclist_ *funclist;
struct funclist_
{
	aststmt  funcdef;        /* The thread/task function */
	aststmt  fromfunc;       /* Must be placed after this function */
	symbol   fname;          /* The name of the thread/task function */
	funclist next;
};
static funclist outfuncs = NULL;


/* New funcs are inserted @ front */
void bld_outfuncs_add(symbol name, aststmt fd, aststmt curfunc)
{
	funclist e   = (funclist) smalloc(sizeof(struct funclist_));
	e->fname     = name;
	e->funcdef   = fd;
	e->fromfunc  = curfunc;
	e->next      = outfuncs;
	outfuncs     = e;
}


/* Takes the list with the produced outlined functions (from xform_parallel(),
 * xfrom_task() etc.), and transforms them. Notice that no new outlined
 * functions can be added here since before a construct is transformed, all
 * nested constructs have already been transformed (so there will be no openmp
 * constructs in any of the functions here).
 */
static void outfuncs_xform(funclist l)
{
	for (; l != NULL; l = l->next)
		ast_stmt_xform(&(l->funcdef));
}


void bld_outfuncs_xform() 
{ 
	outfuncs_xform(outfuncs); 
}


static void outfuncs_place(funclist l)
{
	aststmt neu, bl;
	funclist nl;


	if (l == NULL) return;

	/* Replace l->fromfunc by a small BlockList; notice that anything pointing
	 * to the same func will now "see" this new blocklist. This is why far below
	 * we change the symbol table to point to the correct node.
	 *   ----> (fromfunc) now becomes
	 *
	 *   ---> (BL) -----------> (BL2) --> (outfunc)
	 *         \                   \
	 *          -->(outfunc decl)   -->(fromfunc)
	 */
	neu = (aststmt) smalloc(sizeof(struct aststmt_));
	*neu = *(bl = l->fromfunc);                     /* new node for pfunc */
	*(bl) = *Block3(                                /* BL */
	          Declaration(                          /* outfunc decl */
	            Speclist_right(StClassSpec(SPEC_static), Declspec(SPEC_void)),
	            Declarator(
	              Pointer(),
	              FuncDecl(
	                IdentifierDecl(l->fname) ,
	                ParamDecl(
	                  Declspec(SPEC_void),
	                  AbstractDeclarator(
	                    Pointer(),
	                    NULL
	                  )
	                )
	              )
	            )
	          ),
	          neu, 
	          l->funcdef  /* BL2 */
	        );
	bl->parent         = neu->parent;   /* Parentize (BL) */
	neu->parent        = bl->body;      /* pfuncs's parent */
	l->funcdef->parent = bl->body;      /* outfunc's parent */
	bl->u.next->parent = bl;            /* Declarations's parent */
	bl->body->parent   = bl;            /* BL2's parent */
	bl->file           = NULL;
	bl->u.next->file   = NULL;
	bl->body->file     = NULL;
	l->funcdef->file   = NULL;

	/* Change funcdef link in the symbol table and the rest of the list */
	symtab_get(stab, decl_getidentifier_symbol(neu->u.declaration.decl), 
	           FUNCNAME)->funcdef = neu;
	for (nl=l; nl; nl = nl->next)
		if (nl->fromfunc == bl)
			nl->fromfunc = neu;
		
	if (l->next != NULL)
		outfuncs_place(l->next);
	free(l);    /* No longer needed */
}


void bld_outfuncs_place()
{
	outfuncs_place(outfuncs);
	outfuncs = NULL;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * CONSTRUCTORS                                                      *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* We are going to have the following two functions:
 * 
 * void _<filename>_init_<time>(void) {
 *   ...   // All auto initializations which can be done *after* main() 
 *         // starts should be added here. This will be called by
 *         // ort_initialize().
 * }
 * 
 * static void _<filename>_ctor_<time>(void) {
 *   <register the above function with ORT>
 *   ...   // All other initializations that must be done BEFORE main()
 *         // starts should go in here. This function is called automatically.
 *         // In the thread model, its sole purpose is to register the above. 
 * }
 */

static aststmt ortinits, autoinits;


void bld_ortinits_add(aststmt st)
{
	ortinits = ortinits ? BlockList(ortinits, st) : st;
}


void bld_autoinits_add(aststmt st)
{
	autoinits = autoinits ? BlockList(autoinits, st) : st;
}


void bld_ctors_build()
{
	stentry e;
	aststmt st, l = NULL;
	astexpr initer;
	struct timeval ts;
	char    funcname[32];
	
	if (!ortinits && !autoinits) return;

	gettimeofday(&ts, NULL); /* unique names for the constructors */
	
	if (ortinits)
	{
		sprintf(funcname,"_ompi_init_%X%X_",(unsigned)ts.tv_sec,(unsigned)ts.tv_usec);
		ortinits = 
			FuncDef(
				Declspec(SPEC_void),
				Declarator(
					NULL,
					FuncDecl(
						IdentifierDecl(Symbol(funcname)),
						ParamDecl(Declspec(SPEC_void), NULL)
					)
				),
				NULL, 
				Compound(ortinits)
			);
		
		/* Add registration to the above function */
		st = FuncCallStmt(Identifier(Symbol("ort_initreqs_add")), 
		                  Identifier(Symbol(funcname)));
		autoinits = autoinits ? BlockList(st, autoinits) : st;
	}
	
	sprintf(funcname,"_ompi_ctor_%X%X_",(unsigned)ts.tv_sec,(unsigned)ts.tv_usec);
	autoinits = 
		FuncDef(
			Declspec(SPEC_void),
			Declarator(
				NULL,
				FuncDecl(
					IdentifierDecl(Symbol(funcname)),
					ParamDecl(Declspec(SPEC_void), NULL)
				)
			),
			NULL, 
			Compound(autoinits)
		);
	
	autoinits = 
		BlockList(
			verbit("#ifdef __SUNPRO_C\n"
			       "  #pragma init(%s)\n"
			       "#else \n"  /* gcc assumed */
			       "  static void __attribute__ ((constructor)) %s(void);\n"
			       "#endif\n", funcname, funcname),
			autoinits
		);
	if (ortinits)
		autoinits = BlockList(ortinits, autoinits);
		
	bld_tail_add(autoinits);   /* Add to tail */
}
