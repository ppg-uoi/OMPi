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

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ast_copy.h"
#include "ast_free.h"
#include "ast_print.h"
#include "ast_show.h"
#include "ast_xform.h"
#include "ast_vars.h"
#include "ompi.h"
#include "str.h"
#include "symtab.h"
#include "builder.h"
#include "x_clauses.h"
#include "x_reduction.h"
#include "x_target.h"
#include "x_thrpriv.h"
#include "x_types.h"
#include "x_arrays.h"
#include "x_decltarg.h"
#include "outline.h"


/* outline.c -- code outlining infrastructure.
 * 
 * A given statement gets moved (outlined) to a new function and the 
 * original statement is replaced by call to the new function, plus any
 * required glue code.
 * 
 * The variables used in the original site, are passed appropriately 
 * to the new function in two possible ways: 
 *   - a single argument: a struct with fields that contain all required values
 *   - multiple arguments to the new function, one for each of the variables
 * 
 * The first way is what is implemented below; it is the most suitable one if 
 * the outlined function is going to be a thread function.
 *
 * Some facilities exist for the second way (which, of course, is not suitable
 * for thread functions) for the shake of completeness.
 */


/* A struct with the parameters of handlers */
typedef struct _handler_parameters
{
	/* Input fields 
	 */
	
		// If true, the arguments are passed through a single struct to the function
	bool      structbased;
	
		// A symbol containing the name of the struct instance
	symbol    structName;

		// True if the struct variable is a pointer. Required so that "." or "->" 
	  // is used to access the fields
	bool      repl_structVarIsPointer;
		// The device used for device data environment variables
	astexpr   deviceexpr; // remove it and add ompcon type

	/* Output fields regarding the replacement code
	 */
	
		// The struct fields
	astdecl   structFields;
		/* Statements after the declarations and before the function call */
	aststmt   replBefcall;
		// Statments to copy the results from the struct to the original vars
	aststmt   repl_Uponreturn;
		/* (non-structbased) Declarations @ top of replacement code */
 	aststmt   replDecls;
		/* (non-structbased) The function call arguments (ns) */
	astexpr   replCallargs;

	/* Output fields regarding the generated function
	 */
	
		// Declarations of variables in the new function
	aststmt   funcDecls;
		/* Statements after the declarations and before the main body */
	aststmt   funcBefcode;
		// Copy results back to the struct in the new functions
	aststmt   funcAftcode;
		// Variables to be pointerized when moving the original code to the function
	set(vars) func_Mkptrs;
		/* (non-structbased) The function parameters (ns) */
	astdecl   func_params;
} hanpars_t;


#define MAX_VARNAME_LEN 128

#define NS_BYCOPY_ARRAY_PREFIX "_bycopy_"
#define NS_BYNAME_PREFIX       "_byname_"
#define NS_THRPRIV_PREFIX      "_thrprv_"
#define NS_MAPPED_PREFIX       "_medadr_"


/* Must be freed */
static char *prefix_name(char *name, char *prefix)
{
	char *newname = smalloc(strlen(name)+strlen(prefix)+1);
	sprintf(newname, "%s%s", prefix, name);
	return newname;
}


static symbol prefix_name_symbol(char *name, char *prefix)
{
	static char newname[MAX_VARNAME_LEN];
	snprintf(newname, MAX_VARNAME_LEN-1, "%s%s", prefix, name);
	return Symbol(newname);
}


static void add_comment(aststmt *stmt, char *comment)
{
	*stmt = (*stmt == NULL) ? verbit(comment) : BlockList(*stmt, verbit(comment));
}


static 
astexpr add_cast(astexpr expr, astspec spec, astdecl decl, bool mkptr)
{
	return 
		CastedExpr(
			Casttypename(
				Speclist_right(Usertype(Symbol("__DEVSPEC")), ast_spec_copy_nosc(spec)),
				xt_concrete_to_abstract_declarator(
					mkptr ? decl_topointer(ast_decl_copy(decl)) : ast_decl_copy(decl)
				)
			),
			expr
		);
}


#define COND_ADD_DEVSPEC(cond, decl) (cond) ? \
	Speclist_right(Usertype(Symbol("__DEVSPEC")), decl) : decl
#define COND_ADD_DEVQUAL(cond, decl) (cond) ? \
	Speclist_right(Usertype(Symbol(DEVQUAL)), decl) : decl


/**
 * Creates the struct fields
 *
 * @param e  The original variable
 * @param pt If set turn the struct field into a pointer
 */
static void struct_field(stentry e, hanpars_t *hp, bool pt)
{
	astdecl tmp =
	  StructfieldDecl(
	    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! *\
	     * UGLY HACK should be removed as soon as possible *
	     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
	    hp->deviceexpr && e->isarray ?
	      Speclist_right(Usertype(Symbol("__attribute__((aligned(8)))")),
	                     ast_spec_copy_nosc(e->spec)) :
	      ast_spec_copy_nosc(e->spec),
	    (pt) ?
	      decl_topointer(
	        (threadmode && e->isthrpriv) ?  /* cause of changed name */
	          decl_rename(ast_decl_copy(e->decl), e->key) : ast_decl_copy(e->decl)
	      ) :
	      ast_decl_copy(e->decl)
	  );
	hp->structFields = (hp->structFields  == NULL) ?
	                   tmp :
	                   StructfieldList(hp->structFields, tmp);
}


/**
 * Creates an offset struct field (used only for targets)
 *
 * @param e  The original variable
 * @param hp The handler parameters
 */
static void struct_field_offset(stentry e, hanpars_t *hp)
{
	astdecl tmp =
		StructfieldDecl(
			Speclist_right(Declspec(SPEC_unsigned), Declspec(SPEC_long)),
			Declarator(NULL, IdentifierDecl(targstruct_offsetname(e->key)))
	  );
	hp->structFields = (hp->structFields  == NULL) ?
	                     tmp :
	                     StructfieldList(hp->structFields , tmp);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                 *
 * Actions for constructing the replacement code   *
 *                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * */


/*
 * Standard struct-based construction
 */


/**
 * Handles struct field initialization for variables that are byref, byname or
 * threadprivate. The struct field is a reference to the original variable
 *
 * Called from byref_handler(), out_handle_reduction() and byname_handler()
 */
static void repl_byref(stentry orig, hanpars_t *hp)
{
	astexpr left, right;

	// structvar[->|.]id = [&id|ort_dev_gaddr(&id)];

	// If structInitFromPointer: struct->var
	// Else:                     struct.var
	if (hp->repl_structVarIsPointer)
		left = PtrField(Identifier(hp->structName), orig->key);
	else
		left = DotField(Identifier(hp->structName), orig->key);

	// If threadprivate:  var
	// Else if in target: ort_dev_gaddr(&var)
	// Else:              &var
	if (threadmode && orig->isthrpriv)
		right = Identifier(orig->key); /* 'cause it is already a pointer */
	else
		if (inTarget() || inDeclTarget)
		{
			astdecl temp = decl_topointer(ast_decl_copy(orig->decl));
			/* If in target don't use the address directly. Instead we ask the
			 * runtime for the address in case the local variable does not have
			 * a common address between threads
			 */
			right = CastedExpr(
			          Casttypename(
			            ast_spec_copy_nosc(orig->spec),
			            xt_concrete_to_abstract_declarator(temp)
			          ),
			          FunctionCall(
			            IdentName("ort_dev_gaddr"),
			            UOAddress(Identifier(orig->key))
			          )
			        );
			free(temp);
		}
		else
			right = UOAddress(Identifier(orig->key));

	//left = right;
	hp->replBefcall = BlockList(hp->replBefcall, AssignStmt(left, right));
}


/**
 * Handles struct field initialization for variables that are bycopy.
 * The value of the variable is copied in the struct.
 * Called from handle_byval()
 */
static void repl_bycopy(stentry  e, hanpars_t *hp)
{
	aststmt  stmt;
	char array = e->isarray;

	astexpr structField = hp->repl_structVarIsPointer ?
	                      PtrField(Identifier(hp->structName), e->key) :
	                      DotField(Identifier(hp->structName), e->key);
	if (array)
		// memcpy((void *) structvar[->|.]id, (void *) id, sizeof(id));
		stmt = xc_memcopy(structField, Identifier(e->key),
		                  Sizeof(Identifier(e->key)));
	else
		// structvar[->|.]id = id;
		stmt = AssignStmt(structField, Identifier(e->key));

	hp->replBefcall = BlockList(hp->replBefcall, stmt);
}


/**
 * Handles struct field initialization for variables that are byref to a 
 * local copy. Used only for firstprivate variables in #target constructs.
 * A local copy of the original variable is created and its address is
 * passed on the struct.
 * Called from handle_byval()
 */
static void repl_bylocalcopyref(stentry e, hanpars_t *hp)
{
	aststmt fr;
	astexpr init, 
	        structField = hp->repl_structVarIsPointer ?
	                        PtrField(Identifier(hp->structName), e->key) :
	                        DotField(Identifier(hp->structName), e->key);

	init = FunctionCall(
	         IdentName("ort_unmappedcopy_dev"), 
	         Comma3(
	           e->isarray ? Identifier(e->key) : UOAddress(Identifier(e->key)),
	           Sizeof(Identifier(e->key)), 
	           ast_expr_copy(hp->deviceexpr)
	         )
	       );        
	hp->replBefcall = BlockList(hp->replBefcall, 
	                                AssignStmt(ast_expr_copy(structField), init));
	/* Free allocation */
	fr = Expression(FunctionCall(
	                  IdentName("ort_unmappedfree_dev"), 
	                  CommaList(structField,ast_expr_copy(hp->deviceexpr))
	                ));
	hp->repl_Uponreturn = hp->repl_Uponreturn ? BlockList(hp->repl_Uponreturn, fr) : fr;
}


/**
 * Handles struct field initialization for variables that are in a device data
 * environment
 */
static void repl_ddenv(setelem(vars) s, stentry orig, hanpars_t *hp)
{
	astdecl tmp = decl_topointer(ast_decl_copy(orig->decl));
	ompxli  xl;              /* for initializing the offset field */
	astexpr off;
	bool    is_ptrarrsec;    /* true if an array section from a pointer */
	
	/* xl will be NULL for injected globals like reduction/critical locks */
	xl = (ompxli)( (s->value.clause != OCNOCLAUSE) ? s->value.ptr : orig->pval );
	//assert(!e->isindevenv || xl != NULL);

	is_ptrarrsec = (orig->isindevenv || s->value.clause != OCNOCLAUSE) &&
	               xl && xl->xlitype == OXLI_ARRSEC && decl_ispointer(orig->decl);

	add_comment(&hp->replBefcall, "  /* moved to device data environment */");

	// struct[->|.]var = (<spec> *)ort_host2med_addr(&var, devid);
	hp->replBefcall =
	  BlockList(
	    hp->replBefcall,
	    AssignStmt(
	      hp->repl_structVarIsPointer ?
	      PtrField(Identifier(hp->structName), orig->key) :
	      DotField(Identifier(hp->structName), orig->key),
	      CastedExpr(
	        Casttypename(
	          ast_spec_copy_nosc(orig->spec),
	          xt_concrete_to_abstract_declarator(tmp)
	        ),
	        FunctionCall(
	          IdentName("ort_host2med_addr"),
	          /* For pointer array sections, pass the pointer, not its address
	          is_ptrarrsec ? Identifier(orig->key) : UOAddress(Identifier(orig->key)) */
	          Comma2(
	            UOAddress(Identifier(orig->key)),
	            hp->deviceexpr ? 
	               ast_expr_copy(hp->deviceexpr) : numConstant(AUTODEV_ID)
	          )
	        )
	      )
	    )
	  );

	ast_decl_free(tmp);

	/* Offset field initialization (VVD) 
	 */
	if ((!orig->isindevenv && s->value.clause == OCNOCLAUSE) || !xl ||
	    xl->xlitype == OXLI_IDENT)
		off = numConstant(0);
	else    /* Array section */
		if (xl->dim->next == NULL)   /* Beautify usual small (1-dim) cases */
		{
			if (xl->dim->lb->type == CONSTVAL && atoi(xl->dim->lb->u.str) == 0)
				off = numConstant(0);
			else
				off = BinaryOperator(BOP_mul, 
				        Parenthesis( ast_expr_copy(xl->dim->lb) ), 
				        Sizeof(Deref(Identifier(s->key)))
				      );
		}
		else
			off = BinaryOperator(BOP_sub, 
			        Parenthesis( CastVoidStar(xc_xlitem_baseaddress(xl)) ),
			        CastVoidStar(Identifier(s->key))
			      );
			/* alternative:
			off = BinaryOperator(BOP_mul, 
			        Parenthesis( 
			          BinaryOperator(BOP_sub, xc_xlitem_baseaddress(xl), Identifier(s->key)) 
			        ),
			        Sizeof(Deref(Identifier(s->key)))
			      );
			*/

	// struct[->|.]_<var>_offset = <expr>
	hp->replBefcall =
	  BlockList(
	    hp->replBefcall,
	    AssignStmt(
	      hp->repl_structVarIsPointer ?
	        PtrField(Identifier(hp->structName), targstruct_offsetname(orig->key)) :
	        DotField(Identifier(hp->structName), targstruct_offsetname(orig->key)),
	      off
	    )
	  );
}


/*
 * Non-struct-based construction
 */


/* byref: just pass pointers to the variables */
static void repl_byref_ns(stentry orig, hanpars_t *hp)
{
	astexpr arg;

	/* If threadprivate: var, else: &var
	 * No use in a #target construct (only indirectly, if it contains OpenMP).
	 */
	if (threadmode && orig->isthrpriv)    /* ... it is already a pointer */
		arg = Identifier(prefix_name_symbol(orig->key->name, NS_THRPRIV_PREFIX)); 
	else
		if (!inTarget() && !inDeclTarget)
			arg = UOAddress(Identifier(orig->key));

	hp->replCallargs = hp->replCallargs ? Comma2(hp->replCallargs, arg) : arg;
}


static void repl_bycopy_ns(stentry e, hanpars_t *hp)
{
	astexpr arg;

	if (e->isarray)   /* Make a copy here and pass address to function */
	{
		symbol  copy = prefix_name_symbol(e->key->name, NS_BYCOPY_ARRAY_PREFIX);
		aststmt stmt;
		
		/* Add declaration: <type> copy[...] -- cloned from e */
		stmt = xform_clone_declaration(e->key, NULL, false, copy);
		hp->replDecls = hp->replDecls ? BlockList(hp->replDecls, stmt) : stmt;
		/* Add initializer: memcpy((void *) copy, (void *) id, sizeof(id)); */
		stmt = xc_memcopy(Identifier(copy), Identifier(e->key),
		                  Sizeof(Identifier(e->key)));
		hp->replBefcall = hp->replBefcall ? 
		                      BlockList(hp->replBefcall, stmt) : stmt;
		
		arg = Identifier(copy);
	}
	else
		arg = Identifier(e->key);

	hp->replCallargs = hp->replCallargs ? Comma2(hp->replCallargs, arg) : arg;
}


static void repl_ddenv_ns(setelem(vars) s, stentry orig, hanpars_t *hp)
{
	ompxli  xl;              /* for initializing the offset field */
	astexpr arg;
	
	/* xl will be NULL for injected globals like reduction/critical locks */
	xl = (ompxli)( (s->value.clause != OCNOCLAUSE) ? s->value.ptr : orig->pval );

	arg = FunctionCall(
	        IdentName("ort_host2med_addr"),
	        Comma2(
	          UOAddress(Identifier(orig->key)),
	          hp->deviceexpr ? 
	             ast_expr_copy(hp->deviceexpr) : numConstant(AUTODEV_ID)
	        )
	      );
	hp->replCallargs = hp->replCallargs ? Comma2(hp->replCallargs, arg) : arg;

	/* Offset */
	if ((!orig->isindevenv && s->value.clause == OCNOCLAUSE) || !xl ||
	    xl->xlitype == OXLI_IDENT)
		arg = numConstant(0);
	else    /* Array section */
		if (xl->dim->next == NULL)   /* Beautify usual small (1-dim) cases */
		{
			if (xl->dim->lb->type == CONSTVAL && atoi(xl->dim->lb->u.str) == 0)
				arg = numConstant(0);
			else
				arg = BinaryOperator(BOP_mul, 
				        Parenthesis( ast_expr_copy(xl->dim->lb) ), 
				        Sizeof(Deref(Identifier(s->key)))
				      );
		}
		else
			arg = BinaryOperator(BOP_sub, 
			        Parenthesis( CastVoidStar(xc_xlitem_baseaddress(xl)) ),
			        CastVoidStar(Identifier(s->key))
			      );
	hp->replCallargs = Comma2(hp->replCallargs, arg);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                 *
 * Actions for constructing the new function       *
 *                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * */


/*
 * Standard struct-based construction
 */


/**
 * Handles declaration and initialization of by reference variables in the
 * outlined function.
 * We initialize a pointer from the reference in the struct
 *
 */
static void func_byref(stentry  e, hanpars_t *hp)
{
	aststmt  tmp;

	if (threadmode && e->isthrpriv)
		tmp = tp_declaration(e, e->key,
		                     PtrField(Identifier(hp->structName), e->key), true);
	else
	{
		if (hp->deviceexpr)
			// __DEVSPEC <spec> (* var) = devpart_med2dev_addr(struct->var, 
			//                            sizeof(struct->var)) - struct->_var_offset;
			ast_declordef_addspec(
			  tmp = xform_clone_declaration(
			          e->key,
			          add_cast(
			            Parenthesis(
			              BinaryOperator(BOP_sub,
			                FunctionCall(IdentName("devpart_med2dev_addr"),
			                  CommaList(
			                    PtrField(Identifier(hp->structName), e->key),
			                    Sizeof(
			                      DerefParen( PtrField(Identifier(hp->structName), e->key) )
			                    )
			                  )
			                ),
			                PtrField(Identifier(hp->structName), targstruct_offsetname(e->key))
			              )
			            ),
			            e->spec, e->decl, true
			          ),
			          true,
			          NULL
			        ),
			  Usertype(Symbol("__DEVSPEC"))
			);
		else
			// <spec> (* var) = struct->var
			tmp = xform_clone_declaration(
			        e->key,
			        PtrField(Identifier(hp->structName), e->key),
			        true,
			        NULL
			      );
		set_put(hp->func_Mkptrs, e->key);
	}
	hp->funcDecls = BlockList(hp->funcDecls, tmp);
}


/**
 * Handles declaration and initialization of by reference variables in the
 * outlined function.
 * The difference with func_byref() is that a local copy is created for
 * scalar variables, initialized form the original variable and copied 
 * back to it at the end. 
 * Pointers are regular scalars, too, but they are treated differently if 
 * they are used as array sections: different initialization and no copy-back.
 * Implicit zero-length array sections must be given to us (we cannot find it
 * outselves).
 * Finally, #declare-target (link) variables are excluded.
 */
static 
void func_byref_local(setelem(vars) s, stentry  e, hanpars_t *hp, bool iszlas)
{
	aststmt stmt;
	astdecl tmpdecl = decl_topointer(ast_decl_copy(e->decl));
	astexpr init;
	ompxli  xl;
	bool    is_ptrarrsec;    /* true if an array section from a pointer */
	
	if ((threadmode && e->isthrpriv) || !(hp->deviceexpr) || e->isarray || 
	    e->isindevenv == due2DECLTARG)
	{
		func_byref(e, hp);
		return;
	}

	/* xl will be NULL for injected globals like reduction/critical locks */
	xl = (ompxli)( (s->value.clause != OCNOCLAUSE) ? s->value.ptr : e->pval );
	//assert(!e->isindevenv || xl != NULL);
	
	is_ptrarrsec = (e->isindevenv || s->value.clause != OCNOCLAUSE) &&
	               xl && xl->xlitype == OXLI_ARRSEC && decl_ispointer(e->decl);
	
	// *((origtypecase *) devpart_med2dev_addr(struct->var, sizeof(struct->var)))
	init = FunctionCall(IdentName("devpart_med2dev_addr"),
	         CommaList(
	           PtrField(Identifier(hp->structName), e->key),
	           Sizeof(
	             DerefParen( PtrField(Identifier(hp->structName), e->key) )
	           )
	         )
	       );
	if (is_ptrarrsec || iszlas)     /* ( subtract the offset ) */
		init = Parenthesis(
		         BinaryOperator(
		           BOP_sub,
		           init,
		           PtrField(Identifier(hp->structName), targstruct_offsetname(e->key))
		         )
		       );
	/* Add a cast (should not be needed, but CUDA complains unfortunately) */
	init = add_cast(init, e->spec, e->decl, !is_ptrarrsec && !iszlas);

#if 0
	/* Local copy optimization disabled because of possible pointers to var.
	 * If there is no ZLAS pointing to this variable, then we could safely
	 * localize it.
	 */
	else
		init = DerefParen(
			       CastedExpr(
			         Casttypename(
			           ast_spec_copy_nosc(e->spec),
			           xt_concrete_to_abstract_declarator(tmpdecl)
			         ),
			         init
			       )
			     );
#endif

	free(tmpdecl);
	// __DEVSPEC <spec> var = *(init)
	if (!is_ptrarrsec && !iszlas)
	{
		ast_declordef_addspec( 
			stmt = xform_clone_declaration(e->key, init, true, NULL), 
			Usertype(Symbol("__DEVSPEC")) );
		set_put(hp->func_Mkptrs, e->key);
	}
	else
		ast_declordef_addspec( 
			stmt = xform_clone_declaration(e->key, init, false, NULL),
			Usertype(Symbol("__DEVSPEC")) );
	
	hp->funcDecls = BlockList(hp->funcDecls, stmt);
	
#if 0
	/* Local copy optimization disabled because of possible pointers to var */
	if (!hp->func_results)
		add_comment(&(hp->func_results), "/* copy back the results */");
	// *(init) = var
	if (!is_ptrarrsec && !iszlas)
		hp->func_results = BlockList(
		                     hp->func_results,
		                     AssignStmt(ast_expr_copy(init), Identifier(e->key))
		                   );
#endif
}


/**
 * Handles declaration and initialization of by value (by copy) variables in the
 * outlined function.
 * The struct has copies of the original variables. In case of array we use a
 * pointer to the struct variable to reserve space
 */
static void func_bycopy(stentry e, hanpars_t *hp)
{
	astexpr init;
	aststmt stmt;

	//The initializer (used either as is or to produce array initializer)
	if (hp->deviceexpr)
		// ((origtypecase *) devpart_med2dev_addr(struct->var, sizeof(struct->var)))
		init = add_cast( 
		         FunctionCall(IdentName("devpart_med2dev_addr"),
		           CommaList(
		             PtrField(Identifier(hp->structName), e->key),
		             Sizeof(
		               DerefParen( PtrField(Identifier(hp->structName), e->key) )
		             )
		           )
		         ),
		         e->spec, e->decl, true
		       );
	else
		// struct->var
		init = PtrField(Identifier(hp->structName), e->key);

	if (e->isarray)
	{
		if (!hp->deviceexpr) 
			init = UOAddress(Parenthesis(init));   // &(struct->var)

		//Use a pointer
		if (hp->deviceexpr)
			// __DEVSPEC <spec> (* var) = &(struct->var)
			ast_declordef_addspec(
				stmt = xform_clone_declaration(e->key, init, true,NULL),
				Usertype(Symbol("__DEVSPEC")) );
		else
			// <spec> (* var) = &(struct->var)
			stmt = xform_clone_declaration(e->key, init, true, NULL);
		set_put(hp->func_Mkptrs, e->key);
	}
	else
	{
		if (hp->deviceexpr)
			init = DerefParen(init);  // *(init)
		stmt = xform_clone_declaration(e->key, init, false, NULL);
	}

	hp->funcDecls = BlockList(hp->funcDecls, stmt);
}


/**
 * Handles declaration and initialization of isdevptr variables in the
 * outlined kernel.
 * Clone of func_bycopy(), only for scalars, where the initalizer undergoes
 * address translation.
 */
static void func_bytranscopy(stentry e, hanpars_t *hp)
{
	astexpr init;
	aststmt stmt;

	/* The initializer: devpart_med2dev_addr(struct->var) */
	init = FunctionCall(IdentName("devpart_med2dev_addr"),
	         Comma2(PtrField(Identifier(hp->structName), e->key),numConstant(0)));
	init = add_cast(init, e->spec, e->decl, false);
	
	if (e->isarray)
		exit_error(1, "is_device_ptr cannot handle arrays ('%s')\n", e->key->name);
	/* <spec> var = <initializer> */
	stmt = xform_clone_declaration(e->key, init, false, NULL);
	if (decl_ispointer(e->decl))
		ast_declordef_addspec(stmt, Usertype(Symbol("__DEVSPEC")));
	hp->funcDecls = BlockList(hp->funcDecls, stmt);
}


/**
 * Handles declaration and initialization of by value (by name) variables in the
 * outlined function.
 * The struct holds references to the original variables so we make a copy.
 */
static void func_byname(stentry  e, hanpars_t *hp)
{
	astexpr init;
	aststmt stmt;

	// *(struct->var)
	init = DerefParen( PtrField(Identifier(hp->structName), e->key) );

	if (e->isarray)
	{
		//Create a memcopy
		// <spec> var[size]
		stmt = xform_clone_declaration(e->key, NULL, false, NULL);

		if (hp->funcBefcode == NULL)
			add_comment(&(hp->funcBefcode),
			            "/* byvalue variable initializations */");
		// memcpy((void *) var, (void *) *(struct->var), sizeof(var))
		hp->funcBefcode = BlockList(
		                      hp->funcBefcode,
		                      xc_memcopy(Identifier(e->key), init, Sizeof(Identifier(e->key)))
		                    );
	}
	else
		// <spec> var = *(struct->var)
		stmt = xform_clone_declaration(e->key, init, false, NULL);

	hp->funcDecls = BlockList(hp->funcDecls, stmt);
}


/*
 * Non-struct-based construction
 */


/* Never used for #target constructs */
static void func_byref_ns(stentry e, hanpars_t *hp)
{
	aststmt stmt = NULL;
	astdecl param;

	if (threadmode && e->isthrpriv)
	{
		param = ParamDecl(ast_spec_copy_nosc(e->spec),   /* has changed name */
		                  decl_rename(ast_decl_copy(e->decl), e->key));
		stmt = tp_declaration(e, e->key,
		                     Identifier(
		                       prefix_name_symbol(e->key->name, NS_BYCOPY_ARRAY_PREFIX)
		                     ),
		                     true);
	}
	else
	{
		param = ParamDecl(ast_spec_copy_nosc(e->spec),
		                  decl_topointer(ast_decl_copy(e->decl)));
		set_put(hp->func_Mkptrs, e->key);
		stmt = NULL;
	}
	hp->func_params = hp->func_params ? 
	                  ParamList(hp->func_params, param) : param;
	if (stmt)
		hp->funcDecls = hp->funcDecls ? 
		                    BlockList(hp->funcDecls, stmt) : stmt;
}


/**
 * Handles declaration and initialization of by value (by copy) variables in the
 * outlined function.
 * We keep the function argument as-is, unless it is a target region, which
 * needs to be locally translated.
 */
static void func_bycopy_ns(stentry e, hanpars_t *hp)
{
	astexpr init;
	aststmt stmt = NULL;
	astdecl param;

	if (hp->deviceexpr)
		param = ParamDecl(ast_spec_copy_nosc(e->spec),
		                  decl_topointer(ast_decl_copy(e->decl)));
		// I think not: set_put(hp->func_Mkptrs, e->key);
	else
		param = ParamDecl(ast_spec_copy_nosc(e->spec), ast_decl_copy(e->decl));
	hp->func_params = hp->func_params ? ParamList(hp->func_params, param) : param;

	if (hp->deviceexpr)
	{
		/* __DEVSPEC <spec> (* var) = (origtypecase *) 
		 *                            devpart_med2dev_addr(param, sizeof(*param))
		 */
		symbol psym = prefix_name_symbol(e->key->name, "_tmp_");
		astexpr init = add_cast( 
		                 FunctionCall(IdentName("devpart_med2dev_addr"),
		                   CommaList(
		                     Identifier(psym),
		                     Sizeof(
		                       DerefParen(Identifier(psym))
		                     )
		                   )
		                 ),
		                 e->spec, e->decl, true
		               );
		ast_declordef_addspec(
			stmt = xform_clone_declaration(e->key, init, true, NULL),
			Usertype(Symbol("__DEVSPEC")) 
		);
	}
	
	if (stmt)
		hp->funcDecls = hp->funcDecls ? 
		                    BlockList(hp->funcDecls, stmt) : stmt;
}


static void func_byname_ns(stentry  e, hanpars_t *hp)
{
	aststmt stmt = NULL, local;
	astdecl param;
	symbol  paramsym = prefix_name_symbol(e->key->name, NS_BYNAME_PREFIX);
	
	if (e->isarray)
	{
//		param = ParamDecl(ast_spec_copy_nosc(e->spec),  /* has changed name */
//		                  decl_rename(ast_decl_copy(e->decl), paramsym));
param = ParamDecl(ast_spec_copy_nosc(e->spec),  /* has changed name */
		             decl_topointer(decl_rename(ast_decl_copy(e->decl), paramsym)));		/* clone declaration */
		local = xform_clone_declaration(e->key, NULL, false, NULL);
		/* memcpy(local, param, sizeof(local)); */
		stmt = xc_memcopy(
		         Identifier(e->key), 
		         Deref(Identifier(paramsym)),
		         Sizeof(Identifier(e->key))
		       );
	}
	else
	{
		param = ParamDecl(ast_spec_copy_nosc(e->spec),  /* has changed name */
		             decl_topointer(decl_rename(ast_decl_copy(e->decl), paramsym)));
		/* clone declaration */
		local = xform_clone_declaration(e->key, Deref(Identifier(paramsym)), false, 
		                                NULL);
	}

	hp->func_params = hp->func_params ? ParamList(hp->func_params, param) : param;
	hp->funcDecls = hp->funcDecls ? BlockList(hp->funcDecls, local) : local;
	if (stmt)
		hp->funcBefcode = hp->funcBefcode ? 
		                      BlockList(hp->funcBefcode, stmt) : stmt;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                     *
 * Handlers coordinating the actions to construct the replacement code *
 * and the new function for each variable type.                        *
 *                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static void iterate_set(set(vars) s, hanpars_t *hp, char *comment_type,
                        void (*handler)(setelem(vars), stentry, hanpars_t *))
{
	setelem(vars) e;

	if (set_isempty(s))
		return;
	if (comment_type)
	{
		char comment[64];
		snprintf(comment, 64, "/* %s variables */", comment_type);
		add_comment(&hp->replBefcall, comment);
		add_comment(&hp->funcDecls, comment);
	}
	for (e = s->first; e; e = e->next)
		handler(e, symtab_get(stab, e->key, IDNAME), hp);
}


/* 
 * normal (non-mapped) variables handlers
 */

static void byref_handler(setelem(vars) s, stentry e, hanpars_t *hp)
{
	if (hp->structbased)
	{
		/* (1) Creation of struct fields */
		struct_field(e, hp, true);

		/* (2) Prepare replacement code */
		repl_byref(e, hp);

		/* (3) Prepare the outlined function */
		func_byref(e, hp);
	}
	else
	{
		/* (1) Prepare replacement code */
		repl_byref_ns(e, hp);

		/* (2) Prepare the outlined function */
		func_byref_ns(e, hp);
	}
}


void out_handle_byref(set(vars) s, hanpars_t *hp)
{
	iterate_set(s, hp, "byref", byref_handler);
}


static void private_handler(setelem(vars) s, stentry e, hanpars_t *hp)
{
	hp->funcDecls = 
		BlockList(hp->funcDecls, 
		          xform_clone_declaration(s->key, NULL, false, NULL));
}


static void out_handle_private(set(vars) s, hanpars_t *hp)
{
	/*
	if (inTarget())    // These are map(alloc:) variables
	{
		iterate_set(s, hp, "alloc", alloc_handler);
		return;
	}
	*/
	if (!set_isempty(s))
	{
		add_comment(&(hp->funcDecls), "/* private variables */");
		iterate_set(s, hp, NULL, private_handler);
	}
}


static void byname_handler(setelem(vars) s, stentry e, hanpars_t *hp)
{
	if (hp->structbased)
	{
		/* (1) Creation of struct fields */
		struct_field(e, hp, true);

		/* (2) Prepare replacement code */
		repl_byref(e, hp);

		/* (3) Prepare the outlined function */
		func_byname(e, hp);
	}
	else
	{
		/* (1) Prepare replacement code */
		repl_byref_ns(e, hp);

		/* (2) Prepare the outlined function */
		func_byname_ns(e, hp);
	}
}


static void bycopy_handler(setelem(vars) s, stentry e, hanpars_t *hp)
{
	if (hp->structbased)
	{
		/* (1) Creation of struct fields */
		struct_field(e, hp, (hp->deviceexpr && s->value.clause != OCISDEVPTR));

		/* (2) Prepare the replacement code */
		if (hp->deviceexpr && s->value.clause != OCISDEVPTR)
			repl_bylocalcopyref(e, hp);   /* firstprivate @ targets */
		else
			repl_bycopy(e, hp);

		/* (3) Prepare the outlined function */
		if (s->value.clause == OCISDEVPTR)
			func_bytranscopy(e, hp);   /* OpenMP 4.5; converts the address */
		else
			func_bycopy(e, hp);
	}
	else
	{
		/* (1) Prepare the replacement code */
		if (hp->deviceexpr && s->value.clause != OCISDEVPTR)
			repl_bylocalcopyref(e, hp);   /* firstprivate @ targets */
		else
			repl_bycopy_ns(e, hp);

		/* (2) Prepare the outlined function */
		if (s->value.clause == OCISDEVPTR)
			func_bytranscopy(e, hp);   /* OpenMP 4.5; converts the address */
		else
			func_bycopy_ns(e, hp);
	}
}


static void byscalarcopy_handler(setelem(vars) s, stentry e, hanpars_t *hp)
{
	if (e->isarray)
		byname_handler(s, e, hp);
	else
		bycopy_handler(s, e, hp);
}


/* (VVD): in #target we handle firstprivate (byvalue) similar to bycopy for now.
 * In particular, we create a local copy @ the host (we need to since target
 * is nowadays a task) but we pass its value fully on the struct.
 * The best thing would be to just pass its address and let the kernel
 * initiate a transfer so as to get the copy's value from the host.
 * If we guarantee the mechanism, we will do it...
 * Within the kernel scalars are re-privatized (good) and arrays are
 * used through a pointer to the struct (probably not good). We may need
 * to re-privatize it.
 */
void out_handle_byval(set(vars) s, hanpars_t *hp, char byvalue_type)
{
	switch (byvalue_type)
	{
		case BYVAL_byname:
			iterate_set(s, hp, "byvalue", byname_handler);
			break;
		case BYVAL_bycopy:
			iterate_set(s, hp, "byvalue", bycopy_handler);
			break;
		default:
			iterate_set(s, hp, "byvalue", byscalarcopy_handler);
	}
}


void out_handle_reduction(set(vars) s, hanpars_t *hp, aststmt b)
{
	static int    par_red_num = 0;
	static char   prl[128];
	setelem(vars) e;
	stentry       orig;
	ompclsubt_e   op;
	int           isptr;
	ompxli        xl;
	astdecl       xlpfields; /* For arrsecs with non-constant parameters */
	aststmt       xlpfinits;

	/*
	 * Note: Code is generated only for variables that are used inside the block
	 * This may cause issues with custom reduction code
	 */
	if (set_isempty(s) || b->type != OMPSTMT)
		return;

	/* Comment */
	add_comment(&hp->replBefcall, "/* reduction variables */");
	add_comment(&hp->funcDecls, "/* reduction variables */");

	/* The name for a new lock that will be used for the reduction */
	snprintf(prl, 128, "_paredlock%d", par_red_num);

	if (oldReduction)
	{
		/* Add a global definition for the lock (avoid using omp_lock_t) */
		bld_globalvar_add(Declaration(Speclist_right(
		                                StClassSpec(SPEC_static),
		                                Declspec(SPEC_void)
		                              ),
		                              Declarator(
		                                Pointer(),
		                                IdentifierDecl(Symbol(prl))
		                              )));

		/* Put "ort_reduction_begin(_parelockN);" before the reduction code */
		hp->funcAftcode = FuncCallStmt(
		                     IdentName("ort_reduction_begin"),
		                     UOAddress(IdentName(prl))
		                   );
	}
	else
		add_comment(&hp->funcAftcode, "/* reduce */");  /* Have at least 1 node */

	for (e = s->first; e; e = e->next)
	{
		orig  = symtab_get(stab, e->key, IDNAME);
		op    = e->value.clsubt;               /* Get opid */
		isptr = decl_ispointer(orig->decl);    /* We may have ptr-based reduction */
		xl    = (ompxli) e->value.ptr;
		xlpfields = NULL;
		xlpfinits = NULL;
		
		/* (1) Creation of struct fields
		 */
		struct_field(orig, hp, true);

		/* (2) Initialization of struct fields
		 */
		repl_byref(orig, hp);

		/* (3) Take care for array sections with non-constant parameters */
		arr_section_params_fields_inits(xl, hp->structName, e->key->name, 
		                                &xlpfields, &xlpfinits);
		if (xlpfields)
		{
			/* Add new fields to the struct */
			hp->structFields = (hp->structFields  == NULL) ?
				xlpfields : StructfieldList(hp->structFields, xlpfields);
			/* Add new field initialization statements */
			hp->replBefcall = BlockList(hp->replBefcall, xlpfinits);
			/* Get an array section replacement */
			xl = arr_section_replace_params(xl, hp->structName, e->key->name);
		}
		
		/* (4) Declaration / initialization of function variables */
		if (orig->isarray || isptr)
		{
			hp->funcDecls = BlockList(       /* No initializer here */
			                      hp->funcDecls,
			                      isptr ?
			                        red_privatize_ptr2arr(e->key, xl, hp->structName) :
		                          xform_clone_declaration(e->key, NULL, false, NULL)
			                    );
			/* We need explicit array initialization here */
			if (hp->funcBefcode == NULL)
				add_comment(&(hp->funcBefcode), "/* array reduction initializers */");
			hp->funcBefcode = BlockList(
			                      hp->funcBefcode,
			                      red_array_initializer(op, orig, xl)
			                    );
		}
		else
			hp->funcDecls = BlockList(
			                      hp->funcDecls,
			                      xform_clone_declaration(
			                        e->key,
			                        red_scalar_initializer(op, e->key), 
			                        false,
			                        NULL
			                      )
			                    );

		/* (5) Reduction code */
		hp->funcAftcode = BlockList(
		                     hp->funcAftcode,
		                     red_generate_code(
		                       op, 
		                       //xc_xlitem_find_in_clause(OCREDUCTION, b->u.omp->directive->clauses, e->key),
		                       xl,
		                       Parenthesis(
		                         PtrField(Identifier(hp->structName), e->key)
		                       )
		                     )
		                   );
		
		/* TODO
		 * Possibly free any temporary array (sections)
		 * Notice though that this has to be freed *after* the barrier (taskwait(2))
		 * So, it should be placed elsewhere, not here...
		 * Another trick would be to use alloca()-style allocations.
		 * For the moment, we have to do with memory leaks :-(
		 * 
		if ((xlpfinits = red_generate_deallocation(xl)) != NULL)
			hp->func_results = BlockList(hp->func_results, xlpfinits);
		 *
		 */

		if (xlpfields)
			ast_ompxli_free(xl);    /* ditch the replacement, if any */
	}

	if (oldReduction)
		/* Put "ort_reduction_end(_parelockN);" after the reduction code */
		hp->funcAftcode = BlockList(
		                     hp->funcAftcode,
		                     FuncCallStmt(
		                       IdentName("ort_reduction_end"),
		                       UOAddress(IdentName(prl))
		                     )
		                   );

	par_red_num++;
}


static void inline_byval_handler(setelem(vars) s, stentry e, hanpars_t *hp)
{
	aststmt  tmp;

	//If array: <spec> *fip_var = &var, var;
	//else    : <spec> fip_var = var, var = fip_var;*/
	tmp = xc_firstprivate_declaration(e->key);
	hp->funcDecls = hp->funcDecls? BlockList(hp->funcDecls,tmp) : tmp;

	if (e->isarray)
	{
		char flvar[256];
		snprintf(flvar, 255, "_fip_%s", e->key->name);

		//memcpy((void *) var, (void *) *_fip_var, sizeof(var));
		tmp = xc_memcopy(Identifier(e->key),   /* *flvar */
		                 Deref(IdentName(flvar)),
		                 Sizeof(Identifier(e->key))
		                );

		if (hp->funcBefcode == NULL)
			add_comment(&(hp->funcBefcode),
			            "/* inlined byvalue variable initializations */");
		hp->funcBefcode = BlockList(hp->funcBefcode , tmp);
	}
}


/* Produces a list of byvalue declarations. Used for inlining task code
 */
static void out_inline_byval(set(vars) s, hanpars_t *hp)
{
	iterate_set(s, hp, "byvalue", inline_byval_handler);
}


aststmt out_inline_firstprivate(set(vars) s, aststmt *varinits)
{
	hanpars_t hp = { true, NULL, false, NULL, NULL, NULL, NULL, 
	                 NULL, NULL, NULL, NULL, NULL, NULL, NULL
	               };
	if (!set_isempty(s))
	{
		iterate_set(s, &hp, "byvalue", inline_byval_handler);
		if (varinits)
			*varinits = hp.funcBefcode;
	}
	return (hp.funcDecls);
}


aststmt out_inline_private(set(vars) s)
{
	hanpars_t hp = { true, NULL, false, NULL, NULL, NULL, NULL, 
	                 NULL, NULL, NULL, NULL, NULL, NULL, NULL
	               };
	if (!set_isempty(s))
	{
		add_comment(&(hp.funcDecls), "/* inlined private variables */");
		iterate_set(s, &hp, NULL, private_handler);
	}
	return (hp.funcDecls);
}


/* 
 * mapped variables handlers
 */

static void mapall_handler(setelem(vars) s, stentry e, hanpars_t *hp)
{
	/* (1) Creation of struct fields
	*/
	struct_field(e, hp, true);
	struct_field_offset(e, hp);
	
	/* (2) Initialization of struct fields
	 */
	repl_ddenv(s, e, hp);

	/* (3) Initialization of function variables from struct fields
	 */
	// (VVD) - optimize byref by treating scalars as byvalueresult
	func_byref_local(s, e, hp, false);
}


/* need a bit different treatment for implicit zero-length array sections */
static void mapall_handler_zlas(setelem(vars) s, stentry e, hanpars_t *hp)
{
	/* (1) Creation of struct fields
	*/
	struct_field(e, hp, true);
	struct_field_offset(e, hp);
	
	/* (2) Initialization of struct fields
	 */
	repl_ddenv(s, e, hp);

	/* (3) Initialization of function variables from struct fields
	 */
	// (VVD) - optimize byref by treating scalars as byvalueresult
	func_byref_local(s, e, hp, true);
}


void out_handle_mapvars(set(vars) s, hanpars_t *hp, char *comment, bool iszlas)
{
	if (iszlas)
		iterate_set(s, hp, comment, mapall_handler_zlas);
	else
		iterate_set(s, hp, comment, mapall_handler);
}


/**
 * Repeats all visible struct/union/enum declarations
 *
 * @param globals If true only copy globals else only copy non globals
 *
 * @return a copy of the declarations
 */
aststmt copy_sue_declarations(bool globals)
{
	stentry e;
	aststmt all = NULL, d;

	for (e = stab->top; e; e = e->stacknext)
	{
		if (!globals && e->scopelevel <= 1) break;   /* stop @ globals/funcparams */
		if (globals && e->scopelevel > 0) continue;  /* skip until globals */
		if (e->space == SUNAME || e->space == ENUMNAME)
			if (e->spec)
			{
				d = Declaration(ast_spec_copy_nosc(e->spec), ast_decl_copy(e->decl));
				all = (all) ? BlockList(d, all) : d;
			};
	}
	return (all);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                     *
 *  Main interface                                                     *
 *                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/**
 * Filter non global and copyin variables
 *
 * @param e the variable to check
 *
 * @return true if the variable is local or copyin
 */
bool set_filter_nonglobals_cpin(setelem(vars) e)
{
	return !(symtab_get(stab, e->key, IDNAME)->scopelevel == 0 && 
		       e->value.clause != OCCOPYIN);
}


void dbg_show_vars(set(vars) s, char *header)
{
	setelem(vars) e;
	fprintf(stderr, "%s\n", header);
	for (e = s->first; e; e = e->next)
		fprintf(stderr, "\t%s\n", e->key->name);
}


outcome_t outline(aststmt *b, outpars_t oo, set(vars) *usedvars)
{
	aststmt    tmp;
	astexpr    tmpexpr;
	symbol     structName     = Symbol(oo.structName);
	symbol     structType     = Symbol(oo.structType);
	symbol     functionName   = Symbol(oo.functionName);
	static set(vars) byrefNonGlobal = NULL, pointers = NULL, byref;
	outcome_t  ret;
	hanpars_t  hp = { oo.structbased, structName, (oo.structInitializer != NULL), 
	                  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
	                  NULL, NULL, NULL, NULL
	                };

	ret.usedvars = usedvars;

	ret.repl_struct = ret.func_struct = ret.repl_aftcall = NULL;

	// Clear the pointers set
	set_init(vars, &pointers);

	hp.func_Mkptrs = pointers;
	hp.deviceexpr  = oo.deviceexpr;

	/* ==================================================
	 * Create the struct, replacement region and function 
	 * statements based on the used variables
	 */
	
	// Declare the private variables
	out_handle_private(usedvars[DCT_PRIVATE], &hp);

	// Use the global variables directly or pass them using the new struct
	if (!oo.global_byref_in_struct)
	{
		set_init(vars, &byrefNonGlobal);
		//Don't put global variables in the struct
		set_copy_filtered(byrefNonGlobal, usedvars[DCT_BYREF],
		                 set_filter_nonglobals_cpin);
		byref = byrefNonGlobal;
	}
	else
		byref = usedvars[DCT_BYREF];

	// Handle variables passed by reference
	out_handle_byref(byref, &hp);
	// Handle variables passed by value
	out_handle_byval(usedvars[DCT_BYVALUE], &hp, oo.byvalue_type);

	out_handle_mapvars(usedvars[DCT_MAPALLOC], &hp, "mapalloc", false);
	out_handle_mapvars(usedvars[DCT_MAPTO], &hp, "mapto", false);
	out_handle_mapvars(usedvars[DCT_MAPFROM], &hp, "mapfrom", false);
	out_handle_mapvars(usedvars[DCT_MAPTOFROM], &hp, "maptofrom", false);
	out_handle_mapvars(usedvars[DCT_DDENV], &hp, "mappedalready", false);
	out_handle_mapvars(usedvars[DCT_ZLAS], &hp, "zero-length arrsec", true);

	// Handle reductions
	out_handle_reduction(usedvars[DCT_REDUCTION], &hp, oo.thestmt);

	/* ===============================
	 * Assemble the generated function
	 */
	
	// Change the references to all the variables that are used as pointers
	analyze_pointerize_vars(*b, pointers);

	ret.func_regcode = *b;

	/* Add the code for copying results back to the struct (reduction/from) */
	ret.func_aftcode = hp.funcAftcode ? hp.funcAftcode : 
	                                      verbit("/* no other operations */");

	if (hp.funcDecls)
	{
		ret.func_decls = hp.funcDecls;
		if (hp.structFields) /* Add the struct to the function */
		{
			// Define the struct & declare
			ret.func_struct = Declaration(
			                   SUdecl(SPEC_struct, structType, hp.structFields,NULL),
			                   InitDecl(
			                     Declarator(
			                       Pointer(),
			                       IdentifierDecl(structName)
			                     ),
			                     CastedExpr(
			                       Casttypename(
			                         SUdecl(SPEC_struct, structType, NULL, NULL),
			                         AbstractDeclarator(Pointer(), NULL)
			                       ),
			                       IdentName("__arg")
			                     )
			                   )
			                 );
			ret.func_decls = BlockList(ret.func_struct, ret.func_decls);
		}
		ret.func_befcode = hp.funcBefcode ? hp.funcBefcode : 
		                                      verbit("/* no initializations */");
	}
	else
	{
		ret.func_decls  = verbit("/* no local declarations */");
		ret.func_struct  = NULL;
		ret.func_befcode = verbit("/* no initializations */");
	}

	/* Repeat all visible non-global struct/union/enum declarations */
	if ((tmp = copy_sue_declarations(0)) != NULL)
		ret.func_decls = BlockList(tmp, ret.func_decls);

	/* Add a return( (void *) 0 ) statement to avoid warnings */
	ret.func_return = Return(NullExpr());
	
	ret.function = Compound(
	                 Block5(ret.func_decls, ret.func_befcode, ret.func_regcode, 
	                        ret.func_aftcode, ret.func_return)
	               );

	/* Define the new function.
	 *   (__DEVQAUL) static void *func ( void *__arg ) <body>
	 */
	ret.function = FuncDef(
	                 Speclist_right(
	                   COND_ADD_DEVQUAL(oo.deviceexpr, StClassSpec(SPEC_static)),
	                   Declspec(SPEC_void)
	                 ),
	                 Declarator(
	                   Pointer(),
	                   FuncDecl(
	                     IdentifierDecl(functionName),
	                     oo.structbased ?
	                       ParamDecl(
	                         Declspec(SPEC_void),
	                         Declarator(
	                           Pointer(),
	                           IdentifierDecl(Symbol("__arg"))
	                         )
	                       ) :
	                       hp.func_params
	                   )
	                 ),
	                 NULL, ret.function
	               );

	/* ====================================
	 * Assemble the replacement code region
	 */
	
	/* Prepare the function call parameters
	 */
	tmpexpr = oo.structbased ?
	            CastVoidStar( // (void *) structVariable/&structVariable/0
	              hp.structFields ?  // structVariable or 0
	              (oo.structInitializer ? // structVariable or &structVariable
	                Identifier(structName) :
	                UOAddress(Identifier(structName))
	              ) :
	              numConstant(0)
	            ) :
	            hp.replCallargs;
	if (oo.extraParameters) // Add any extra parameters
		tmpexpr = tmpexpr ? CommaList(tmpexpr, oo.extraParameters) :
		                    oo.extraParameters;
	if (oo.functionCall)
		ret.repl_funcall = FuncCallStmt(  // functionCall(functionName, arg(s) )

		                     IdentName(oo.functionCall),
		                     CommaList(
		                       Identifier(functionName),
		                       tmpexpr ? tmpexpr : NullExpr()
		                     )
		                   );
	else
		// functionName( arg(s) )
		ret.repl_funcall = FuncCallStmt(  // functionName( arg(s) )
		                     Identifier(functionName),
		                     tmpexpr
		                   );

	/* Add any declarations and statements before and after the function call
	 */
	if (hp.structFields)   /* This also means that we are structbased */
	{
		/* Define the struct */
		tmp = Declaration(SUdecl(
		                    SPEC_struct, structType,
		                    ast_decl_copy(hp.structFields), NULL
		                  ),
		                  Declarator(
		                    (oo.structInitializer ? Pointer() : NULL),
		                    IdentifierDecl(structName)
		                  )
		                 );

		ret.repl_struct = ret.repl_decls = tmp;

		tmp = NULL;
		if (oo.structInitializer)    /* initialization of struct var if it exists */
			tmp = AssignStmt(Identifier(structName) , oo.structInitializer);
		if (hp.replBefcall)  /* If we only have map(from:) this will be empty */
			tmp = tmp ? BlockList(tmp, hp.replBefcall) : hp.replBefcall;   
		ret.repl_befcall = tmp ? tmp : verbit("/* no initializations */");

		/* Add code to copy back byresult variables */
		ret.repl_aftcall = hp.repl_Uponreturn ? hp.repl_Uponreturn :
		                                        verbit("/* no other operations */");
	}
	else
	{
		if (oo.structbased)
		{
			if (oo.structInitializer)
				ast_expr_free(oo.structInitializer);  // Free it since we didn't use it
			ret.repl_decls   = verbit("/* no declarations needed */"); 
			ret.repl_befcall = verbit("/* no initializations */");
		}
		else
		{
			ret.repl_decls   = hp.replDecls ? 
			                   hp.replDecls : verbit("/* no declarations needed */"); 
			ret.repl_befcall = hp.replBefcall ?
			                   hp.replBefcall : verbit("/* no initializations */");
		}
		ret.repl_struct  = NULL;
		ret.repl_aftcall = verbit("/* no other operations */");
	}
	
	ret.replacement = Compound(Block4(ret.repl_decls, ret.repl_befcall, 
	                                  ret.repl_funcall, ret.repl_aftcall));

	/* Parentize new statements */
	ast_stmt_parent((*b)->parent, ret.replacement);
	ast_parentize(ret.function);

	*b = ret.replacement;    /* Replace the code block */

	return ret;
}


static vartype_t implicitDefaultByRef(setelem(vars) s)
{
	return DCT_BYREF;
}


outcome_t outline_OpenMP(aststmt *b, outpars_t oo)
{
	setelem(vars) e;
	set(vars)    *usedvars, copyinvars;
	outcome_t     ret;
	aststmt       d, commented_directive;
	str           st1 = Strnew();
	ompclause     def;
	stentry       fdecl;

	/* (1) Discover the variables used in block b */
	usedvars = analyze_used_vars(*b);

	/* If there are any copyin variables we need to include them no matter
	 * if they are used or not. We use a new set instead of putting them directly 
	 * into usedvars[DCT_BYREF] because xc_ompcon_get_vars doesn't check if the 
	 * variables already exist (it uses set_put() instead of set_put_unique()) 
	 * and we also need to mark them
	 */
	copyinvars = set_new(vars);
	xc_ompcon_get_vars((*b)->u.omp, OCCOPYIN, OC_DontCare, copyinvars);
	for (e = copyinvars->first; e; e = e->next)
		set_put_unique(usedvars[DCT_BYREF], e->key)->value.clause = OCCOPYIN;
	set_free(copyinvars);

	/* (2) Check for default clause */
	def = xc_ompcon_get_unique_clause((*b)->u.omp, OCDEFAULT);

	// If default(none) is specified and we have implicit vars throw an error
	if (def && def->subtype == OC_defnone && usedvars[DCT_UNSPECIFIED]->first)
	{
		exit_error(1, "(%s) openmp error:\n\t"
		          "variable `%s' must be explicitely declared as shared/private\n\t"
		          "due to the default(none) clause at line %d.\n",
		          (*b)->file->name, usedvars[DCT_UNSPECIFIED]->first->key->name,
		          (*b)->u.omp->directive->l);
	}
	else
		if (def && def->subtype == OC_defshared)
			oo.implicitDefault = implicitDefaultByRef;

	/* (3) Move the implicit variables to the appropriate set */
	for (e = usedvars[DCT_UNSPECIFIED]->first; e; e = e->next)
		set_put(usedvars[oo.implicitDefault(e)], e->key)->value = e->value;

	/* (4) Remove the directive from the code */
	oo.thestmt = *b;
	d  = *b;
	*b = (*b)->u.omp->body;
	(*b)->parent = d->parent;

	/* (5) Outline */
	ret = outline(b, oo, usedvars);

	/* (6) Put the directive in a comment */
	ast_ompdir_print(st1, (d->u.omp->directive));
	commented_directive = (cppLineNo) ?
	                      BlockList(
	                        verbit("/* (l%d) %s -- body moved below */",
	                               d->l, str_string(st1)),
	                        verbit("# %d \"%s\"", d->l, d->file->name)
	                      ) :
	                      verbit("/* (l%d) %s -- body moved below */",
	                             d->l, str_string(st1));
	if (oo.addComment)
		ast_stmt_prepend(ret.replacement, verbit("/* (l%d) %s */", d->l,
		                                          str_string(st1)));
	ast_stmt_prepend(ret.func_regcode, commented_directive);
	str_free(st1);

	/* (7) Add the new function to the output list */
	bld_outfuncs_add(Symbol(oo.functionName), ret.function,
	                 ast_get_enclosing_function(*b));

	/* (8) Put the function declaration (not definition) in the symbol table */
	fdecl = symtab_insert_global(stab, Symbol(oo.functionName), FUNCNAME);
	fdecl->spec = ret.function->u.declaration.spec;
	fdecl->decl = ret.function->u.declaration.decl;
	if ((inTarget() && !oo.deviceexpr) || inDeclTarget)
	{
		 /* Outlined from a target region: make it a #declared function */
		fdecl->isindevenv = due2DECLTARG; /* could have due2INJECTED but no need */
		decltarg_add_calledfunc(fdecl->key);
		decltarg_bind_id(fdecl);
	}

	/* (9) Free the WHOLE directive; unprocessed stuff will be left dangling.
	 * That's why the ugly target hack.
	 */
	d->u.omp->body = NULL;   /* Make it NULL so as to free it easily */
	if (!inTarget())
		ast_free(d);           /* Get rid of the OmpStmt (target does it later) */

	/* (10) If it is a target region, add generated function to the kernel list */
	if (inTarget() && oo.deviceexpr)
		targtree = BlockList(targtree, ret.function);

	return ret;
}
