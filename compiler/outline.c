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

#include <stdlib.h>
#include <assert.h>

#include "ast_copy.h"
#include "ast_free.h"
#include "ast_print.h"
#include "ast_show.h"
#include "ast_xform.h"
#include "ast_vars.h"
#include "ompi.h"
#include "str.h"
#include "symtab.h"
#include "x_clauses.h"
#include "x_target.h"
#include "x_thrpriv.h"
#include "x_types.h"

#include "outline.h"


static void add_comment(aststmt *stmt, char *comment)
{
	*stmt = (*stmt == NULL) ? verbit(comment) : BlockList(*stmt, verbit(comment));
}

/**
 * Adds __OPENCL_ASQ to the declaration specifier e.g.
 * <spec> <var> -> __OPENCL_ASQ <spec> <var>
 * To be used in conjunction with xform_clone_declaration()
 *
 * @param orig the original declaration statement. Warning if not a declaration
 *             it will crash
 *
 * @return the modified declaration
 */
static inline aststmt add_opencl_asq(aststmt orig)
{
	orig->u.declaration.spec = Speclist_right(Usertype(Symbol("__OPENCL_ASQ")),
	                                          orig->u.declaration.spec);
	return orig;
}

#define COND_ADD_CL_ASQ(cond, decl) (cond) ? \
	Speclist_right(Usertype(Symbol("__OPENCL_ASQ")), decl) : decl


/**
 * Creates the struct fields
 *
 * @param e  The original variable
 * @param pt If set turn the struct field into a pointer
 * @return   A struct field declaration
 */
static void struct_create(stentry e, hanpars_t *hp, bool pt)
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
	    xc_decl_topointer(
	      (threadmode && e->isthrpriv) ?  /* cause of changed name */
	      xc_decl_rename(ast_decl_copy(e->decl), e->key) :
	      ast_decl_copy(e->decl)
	    ) :
	    ast_decl_copy(e->decl)
	  );
	hp->structFields = (hp->structFields  == NULL) ?
	                   tmp :
	                   StructfieldList(hp->structFields , tmp);
}

/**
 * Handles struct field initialization for variables that are byref, byname or
 * threadprivate. The struct field is a reference to the original variable
 *
 * Called from byref_handler(), out_handle_reduction() and byname_handler()
 */
static void repl_byref(stentry  orig, hanpars_t *hp)
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
			astdecl temp = xc_decl_topointer(ast_decl_copy(orig->decl));
			/* If in target don't use the address directly. Instead we ask the
			 * runtime for the address incase the local variable does not have a
			 * common address between threads
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
	hp->repl_structInit = BlockList(hp->repl_structInit, AssignStmt(left, right));
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

	hp->repl_structInit = BlockList(hp->repl_structInit, stmt);
}

/**
 * Copy by result variables from struct fields to the original variables
 */
static void repl_byresult(stentry  e, hanpars_t *hp)
{
	astexpr structfield = PtrField(Identifier(hp->structName), e->key);
	aststmt stmt;

	if (!hp->repl_results)
		add_comment(&(hp->repl_results), "/* copy back the results */");

	if (e->isarray)
	{
		//Create a memcopy
		// memcpy((void *) var, (void *) struct->var, sizeof(var))
		stmt =  xc_memcopy(
		          Identifier(e->key),
		          structfield,
		          Sizeof(Identifier(e->key))
		        );
	}
	else
		// var = struct->var
		stmt = AssignStmt(Identifier(e->key), structfield);

	hp->repl_results = BlockList(hp->repl_results, stmt);
}

/**
 * Handles struct field initialization for variables that are in a device data
 * environment
 */
static void repl_ddenv(stentry  orig, hanpars_t *hp)
{
	astdecl tmp = xc_decl_topointer(ast_decl_copy(orig->decl));

	add_comment(&hp->repl_structInit, "  /* moved to device data environment */");

	// struct[->|.]var = (<spec> *)ort_get_vaddress(&var, device);
	hp->repl_structInit =
	  BlockList(
	    hp->repl_structInit,
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
	          IdentName("ort_get_vaddress"),
	          UOAddress(Identifier(orig->key))
	        )
	      )
	    )
	  );

	ast_decl_free(tmp);
}

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
			// __OPENCL_ASQ <spec> (* var) = devrt_get_dev_address(struct->var, sizeof(struct->var))
			tmp = add_opencl_asq(
			        xform_clone_declaration(
			          e->key,
			          FunctionCall(IdentName("devrt_get_dev_address"),
			            CommaList(
			              PtrField(Identifier(hp->structName), e->key),
			              Sizeof(
			                UnaryOperator(UOP_star,
			                  Parenthesis(
			                    PtrField(Identifier(hp->structName), e->key)
			                  )
			                )
			              )
			            )
			          ),
			          true
			        )
			      );
		else
			// <spec> (* var) = struct->var
			tmp = xform_clone_declaration(
			        e->key,
			        PtrField(Identifier(hp->structName), e->key),
			        true
			      );
		set_put(hp->func_mkpointers, e->key);
	}
	hp->func_varDecls = BlockList(hp->func_varDecls, tmp);
}

/**
 * Handles declaration and initialization of by value (by copy) variables in the
 * outlined function.
 * The struct has copies of the original variables. In case of array we use a
 * pointer to the struct variable to reserve space
 */
static void func_bycopy(stentry  e, hanpars_t *hp)
{
	astexpr init;
	aststmt stmt;

	//The initializer (used either as is or to produce array initializer)
	// struct->var
	init = PtrField(Identifier(hp->structName), e->key);

	if (e->isarray)
	{
		// &(struct->var)
		init = UOAddress(Parenthesis(init));

		//Use a pointer
		if (hp->deviceexpr)
			// __OPENCL_ASQ <spec> (* var) = &(struct->var)
			stmt = add_opencl_asq(xform_clone_declaration(e->key, init, true));
		else
			// <spec> (* var) = &(struct->var)
			stmt = xform_clone_declaration(e->key, init, true);
		set_put(hp->func_mkpointers, e->key);
	}
	else
		// <spec> var = struct->var
		stmt = xform_clone_declaration(e->key, init, false);

	hp->func_varDecls = BlockList(hp->func_varDecls, stmt);
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
	init = UnaryOperator(
	         UOP_star,
	         Parenthesis(PtrField(Identifier(hp->structName), e->key))
	       );

	if (e->isarray)
	{
		//Create a memcopy
		// <spec> var[size]
		stmt = xform_clone_declaration(e->key, NULL, false);

		if (hp->func_varInits == NULL)
			add_comment(&(hp->func_varInits),
			            "/* byvalue variable initializations */");
		// memcpy((void *) var, (void *) *(struct->var), sizeof(var))
		hp->func_varInits = BlockList(
		                      hp->func_varInits,
		                      xc_memcopy(Identifier(e->key), init, Sizeof(Identifier(e->key)))
		                    );
	}
	else
		// <spec> var = *(struct->var)
		stmt = xform_clone_declaration(e->key, init, false);

	hp->func_varDecls = BlockList(hp->func_varDecls, stmt);
}

/**
 * Handles declaration of by result variables in the outlined function.
 * For array variables we point directly to the struct.
 */
static void func_byresult(stentry  e, hanpars_t *hp)
{
	aststmt stmt;

	//Declare the local variable
	if (e->isarray)
	{
		//struct->var
		astexpr init = PtrField(Identifier(hp->structName), e->key);
		// &(struct->var)
		init = UOAddress(Parenthesis(init));

		//Use a pointer
		// __OPENCL_ASQ <spec> (* var) = &(struct->var)
		stmt = add_opencl_asq(xform_clone_declaration(e->key, init, true));
		set_put(hp->func_mkpointers, e->key);
	}
	else
		// <spec> var;
		stmt = xform_clone_declaration(e->key, NULL, false);

	hp->func_varDecls = BlockList(hp->func_varDecls, stmt);
}

/**
 * Copy by result variables from local variables to struct fields.
 * For array variables we don't need to do anything since the local variables
 * already point to the struct.
 */
static void func_byres_copyback(stentry  e, hanpars_t *hp)
{
	if (e->isarray)
		return;

	if (!hp->func_results)
		add_comment(&(hp->func_results), "/* copy back the results */");

	// struct->var = var
	hp->func_results = BlockList(
	                     hp->func_results,
	                     AssignStmt(
	                       PtrField(Identifier(hp->structName), e->key),
	                       Identifier(e->key))
	                   );
}


/**
 * Produces a list of declarations.
 *
 * @param s   A set with the private variables
 * @param dec A reference to the variable which will hold the declarations
 */
void out_handle_private(set(vars) s, hanpars_t *hp)
{
	setelem(vars) e;
	aststmt       tmp;

	if (set_isempty(s))
		return;

	//Comment
	add_comment(&(hp->func_varDecls), "/* Private variables */");

	for (e = s->first; e; e = e->next)
	{
		tmp = xform_clone_declaration(e->key, NULL, false);
		hp->func_varDecls = BlockList(hp->func_varDecls, tmp);
	}
}

static void ddenv_handler(stentry e, hanpars_t *hp);
static void iterate_set(set(vars) s, hanpars_t *hp, char *comment_type,
                        void (*handler)(stentry, hanpars_t *))
{
	setelem(vars) e;
	stentry       orig;

	if (set_isempty(s))
		return;

	if (comment_type)
	{
		char comment[64];
		snprintf(comment, 64, "/* %s variables */", comment_type);

		//Comment
		add_comment(&hp->repl_structInit, comment);
		add_comment(&hp->func_varDecls, comment);
	}

	for (e = s->first; e; e = e->next)
	{
		orig = symtab_get(stab, e->key, IDNAME);

		/* If we are in a target directive and the variable was in a device data
		 * environment (or if we chose to put all target variables in the device
		 * data environment) use "DDENV" handler instead of the default.
		 */
		extern char allvarsindevenv;
		if (hp->deviceexpr && (orig->isindevenv || allvarsindevenv))
			ddenv_handler(orig, hp);
		else
			handler(orig, hp);
	}
}

static void byref_handler(stentry e, hanpars_t *hp)
{
	/* (1) Creation of struct fields
	*/
	struct_create(e, hp, true);

	/* (2) Initialization of struct fields
	 */
	repl_byref(e, hp);

	/* (3) Initialization of function variables from struct fields
	 */
	func_byref(e, hp);
}

void out_handle_byref(set(vars) s, hanpars_t *hp)
{
	iterate_set(s, hp, "byref", byref_handler);
}

static void byname_handler(stentry e, hanpars_t *hp)
{
	/* (1) Creation of struct fields
	 */
	struct_create(e, hp, true);

	/* (2) Initialization of struct fields
	 */
	repl_byref(e, hp);

	/* (3) Initialization of function variables from struct fields
	 */
	func_byname(e, hp);
}

static void bycopy_handler(stentry e, hanpars_t *hp)
{
	/* (1) Creation of struct fields
	 */
	struct_create(e, hp, false);

	/* (2) Initialization of struct fields
	 */
	repl_bycopy(e, hp);

	/* (3) Initialization of function variables from struct fields
	 */
	func_bycopy(e, hp);
}

void out_handle_byval(set(vars) s, hanpars_t *hp, char byvalue_type)
{
	if (byvalue_type)
		iterate_set(s, hp, "byvalue", byname_handler);
	else
		iterate_set(s, hp, "byvalue", bycopy_handler);
}

static void byresult_handler(stentry e, hanpars_t *hp)
{
	/* (1) Creation of struct fields
	 */
	struct_create(e, hp, false);

	/* (2) Declaration of function variables
	 */
	func_byresult(e, hp);

	/* (3) Copying to the struct
	 */
	func_byres_copyback(e, hp);

	/* (4) Copying back to the original variables
	 */
	repl_byresult(e, hp);
}

void out_handle_byres(set(vars) s, hanpars_t *hp)
{
	add_comment(&hp->func_varDecls, "/* byresult variables */");
	iterate_set(s, hp, NULL, byresult_handler);
}

static void byvalueresult_handler(stentry e, hanpars_t *hp)
{
	/* (1) Creation of struct fields
	 */
	struct_create(e, hp, false);

	/* (2) Initialization of struct fields
	 */
	repl_bycopy(e, hp);

	/* (3) Initialization of function variables from struct fields
	 */
	func_bycopy(e, hp);

	/* (4) Copying to the struct
	 */
	func_byres_copyback(e, hp);

	/* (5) Copying back to the original variables
	 */
	repl_byresult(e, hp);
}


void out_handle_byvalres(set(vars) s, hanpars_t *hp)
{
	iterate_set(s, hp, "byvalueresult", byvalueresult_handler);
}

static void ddenv_handler(stentry e, hanpars_t *hp)
{
	/* (1) Creation of struct fields
	*/
	struct_create(e, hp, true);

	/* (2) Initialization of struct fields
	 */
	repl_ddenv(e, hp);

	/* (3) Initialization of function variables from struct fields
	 */
	func_byref(e, hp);
}

void out_handle_devdatenvironment(set(vars) s, hanpars_t *hp)
{
	iterate_set(s, hp, "implicit device data environment", ddenv_handler);
}

void out_handle_reduction(set(vars) s, hanpars_t *hp)
{
	static int    par_red_num = 0;
	static char   prl[128];
	setelem(vars) e;
	stentry       orig;
	int           op;

	/*
	 * Note: Code is generated only for variables that are used inside the block
	 * This may cause issues with custom reduction code
	 */

	if (set_isempty(s))
		return;

	//Comment
	add_comment(&hp->repl_structInit, "/* reduction variables */");
	add_comment(&hp->func_varDecls, "/* reduction variables */");

	/* The name for a new lock that will be used for the reduction */
	snprintf(prl, 128, "_paredlock%d", par_red_num);

	/* Add a global definition for the lock (avoid using omp_lock_t) */
	newglobalvar(Declaration(Speclist_right(
	                           StClassSpec(SPEC_static),
	                           Declspec(SPEC_void)
	                         ),
	                         Declarator(
	                           Pointer(),
	                           IdentifierDecl(Symbol(prl))
	                         )));

	/* Put "ort_reduction_begin(_parelockN);" before the reduction code */
	hp->func_results = FuncCallStmt(
	                     IdentName("ort_reduction_begin"),
	                     UOAddress(IdentName(prl))
	                   );

	for (e = s->first; e; e = e->next)
	{
		orig  = symtab_get(stab, e->key, IDNAME);
		op    = e->value; /* Get opid from ->int1 */

		/* (1) Creation of struct fields
		 */
		struct_create(orig, hp, true);

		/* (2) Initialization of struct fields
		 */
		repl_byref(orig, hp);

		/* (3) Initialization of function variables */
		hp->func_varDecls = BlockList(
		                      hp->func_varDecls,
		                      xform_clone_declaration(
		                        e->key,
		                        xc_reduction_initializer(op, e->key), false
		                      )
		                    );

		/* (4) Reduction code */
		hp->func_results = BlockList(
		                     hp->func_results,
		                     xc_reduction_code(
		                       op, Identifier(e->key),
		                       Parenthesis(
		                         PtrField(Identifier(hp->structName), e->key)
		                       )
		                     )
		                   );
	}

	/* Put "ort_reduction_end(_parelockN);" after the reduction code */
	hp->func_results = BlockList(
	                     hp->func_results,
	                     FuncCallStmt(
	                       IdentName("ort_reduction_end"),
	                       UOAddress(IdentName(prl))
	                     )
	                   );

	par_red_num++;
}

static void inline_byval_handler(stentry e, hanpars_t *hp)
{
	aststmt  tmp;

	//If array: <spec> *fip_var = &var, var;
	//else    : <spec> fip_var = var, var = fip_var;*/
	tmp = xc_firstprivate_declaration(e->key);
	hp->func_varDecls = BlockList(hp->func_varDecls, tmp);

	if (e->isarray)
	{
		char flvar[256];
		snprintf(flvar, 255, "_fip_%s", e->key->name);

		//memcpy((void *) var, (void *) *_fip_var, sizeof(var));
		tmp = xc_memcopy(Identifier(e->key),   /* *flvar */
		                 UnaryOperator(UOP_star, IdentName(flvar)),
		                 Sizeof(Identifier(e->key))
		                );

		if (hp->func_varInits == NULL)
			add_comment(&(hp->func_varInits),
			            "/* byvalue variable initializations */");
		hp->func_varInits = BlockList(hp->func_varInits , tmp);
	}
}

/* Produces a list of byvalue declarations. Used for inlining task code
 */
void out_inline_byval(set(vars) s, hanpars_t *hp)
{
	iterate_set(s, hp, "byvalue", inline_byval_handler);
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

/**
 * Filter non global and copyin variables
 *
 * @param e the variable to check
 *
 * @return true if the variable is local or copyin
 */
bool set_filter_nonglobals_cpin(setelem(vars) e)
{
	if (symtab_get(stab, e->key, IDNAME)->scopelevel == 0 && e->value != OCCOPYIN)
		return false;
	return true;
}


/*bool set_filter_nonthreadprivates(setentry e) {
  if (symtab_get(stab, e->key, IDNAME)->isthrpriv)
    return false;
  return true;
}*/


outcome_t outline(aststmt *b, outpars_t oo, set(vars) *usedvars)
{
	aststmt    tmp;
	astexpr    tmpexpr;
	symbol     structName     = Symbol(oo.structName);
	symbol     structType     = Symbol(oo.structType);
	symbol     functionName   = Symbol(oo.functionName);
	static set(vars) byrefNonGlobal = NULL, pointers = NULL, byref;
	hanpars_t  hp = { structName, (oo.structInitializer != NULL), NULL, NULL,
	                  NULL, NULL, NULL, NULL, NULL, NULL
	                };
	outcome_t  ret;

	ret.usedvars = usedvars;

	ret.rep_struct = ret.funcstruct = ret.rep_copyback = NULL;

	//Clear the pointers set
	set_init(vars, &pointers);

	hp.func_mkpointers = pointers;
	hp.deviceexpr      = oo.deviceexpr;

	//Declare the private variables
	out_handle_private(usedvars[DCT_PRIVATE], &hp);

	//Use the global variables directly or pass them using the new struct
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

	//Handle variables passed by reference
	out_handle_byref(byref, &hp);

	//Handle variables passed by value
	out_handle_byval(usedvars[DCT_BYVALUE], &hp, oo.byvalue_type);

	//Handle variables passed by result
	out_handle_byres(usedvars[DCT_BYRESULT], &hp);

	//Handle variables passed by value-result
	out_handle_byvalres(usedvars[DCT_BYVALRES], &hp);

	//Handle device data environment variables
	out_handle_devdatenvironment(usedvars[DCT_DDENV], &hp);

	//Handle reductions
	out_handle_reduction(usedvars[DCT_REDUCTION], &hp);

	//Change the references to all the variables that are used as pointers
	analyze_pointerize_vars(*b, pointers);

	ret.function = ret.functionbody = *b;

	/* Add the code for copying results back to the struct (reduction/from) */
	if (hp.func_results)
		ret.function = BlockList(ret.function, hp.func_results);

	if (hp.func_varDecls)
	{
		if (hp.func_varInits) /* Add any memcopies */
			ret.function = BlockList(hp.func_varInits, ret.function);

		ret.function = BlockList(hp.func_varDecls, ret.function);

		if (hp.structFields) /* Add the struct to the function */
		{
			// Define the struct & declare
			ret.funcstruct = Declaration(
			                   COND_ADD_CL_ASQ(
			                     oo.deviceexpr,
			                     SUdecl(SPEC_struct, structType, hp.structFields)
			                   ),
			                   InitDecl(
			                     Declarator(
			                       Pointer(),
			                       IdentifierDecl(structName)
			                     ),
			                     CastedExpr(
			                       Casttypename(
			                         COND_ADD_CL_ASQ(
			                           oo.deviceexpr,
			                           SUdecl(SPEC_struct, structType, NULL)
			                         ),
			                         AbstractDeclarator(Pointer(), NULL)
			                       ),
			                       IdentName("__arg")
			                     )
			                   )
			                 );
			ret.function = BlockList(ret.funcstruct, ret.function);
		}
	}

	/* Repeat all visible non-global struct/union/enum declarations */
	if ((tmp = copy_sue_declarations(0)) != NULL)
		ret.function = BlockList(tmp, ret.function);

	//Add a return( (void *) 0 ) statement to avoid warnings
	ret.returnstm = Return(NullExpr());
	ret.function = BlockList(ret.function, ret.returnstm);

	ret.function = Compound(ret.function);

	/* Define the new function.
	 *   static void *func ( void *__arg ) <body>
	 */
	ret.function = FuncDef(
	                 Speclist_right(StClassSpec(SPEC_static), Declspec(SPEC_void)),
	                 Declarator(
	                   Pointer(),
	                   FuncDecl(
	                     IdentifierDecl(functionName),
	                     ParamDecl(
	                       COND_ADD_CL_ASQ(oo.deviceexpr, Declspec(SPEC_void)),
	                       Declarator(
	                         Pointer(),
	                         IdentifierDecl(Symbol("__arg"))
	                       )
	                     )
	                   )
	                 ),
	                 NULL, ret.function
	               );

	/* Prepare the function parameters
	 */
	tmpexpr = CastVoidStar( // (void *) structVariable/&structVariable/0
	            hp.structFields ? // structVariable or 0
	            (oo.structInitializer ? // structVariable or &structVariable
	             Identifier(structName) :
	             UOAddress(Identifier(structName))
	            ) :
	            numConstant(0)
	          );
	if (oo.extraParameters) // Add any extra parameters
		tmpexpr = CommaList(tmpexpr, oo.extraParameters);

	if (oo.functionCall)
	{
		// functionCall(functionName, structVariable/0 [, extraParameters])
		ret.functioncall = FuncCallStmt(
		                     IdentName(oo.functionCall),
		                     CommaList(
		                       Identifier(functionName),
		                       tmpexpr
		                     )
		                   );
	}
	else
	{
		// functionName((void *)structVariable/0 [, extraParameters])
		ret.functioncall = FuncCallStmt(
		                     Identifier(functionName),
		                     tmpexpr
		                   );
	}

	ret.replacement = ret.functioncall;
	if (hp.structFields)
	{
		// Define the struct
		tmp = Declaration(SUdecl(
		                    SPEC_struct, structType,
		                    ast_decl_copy(hp.structFields)
		                  ),
		                  Declarator(
		                    (oo.structInitializer ? Pointer() : NULL),
		                    IdentifierDecl(structName)
		                  )
		                 );

		ret.rep_struct = tmp;

		// Add initialization of struct variable if it exists
		if (oo.structInitializer)
			tmp = BlockList(
			        ret.rep_struct,
			        AssignStmt(Identifier(structName) , oo.structInitializer)
			      );

		// Add initialization of the struct fields
		if (hp.repl_structInit) //If we only have map(from:) this will be empty
			tmp = BlockList(tmp, hp.repl_structInit);

		// Combine it with the function call
		ret.replacement = BlockList(tmp, ret.replacement);

		//Add the code for copying back byresult variables
		if (hp.repl_results)
		{
			ret.rep_copyback = hp.repl_results;
			ret.replacement = BlockList(ret.replacement, hp.repl_results);
		}
	}
	else
		if (oo.structInitializer)
		{
			//Free it since we didn't use it
			ast_expr_free(oo.structInitializer);
		}
	ret.replacement = Compound(ret.replacement);

	/* Parentize new statements
	 */
	ast_stmt_parent((*b)->parent, ret.replacement);
	ast_parentize(ret.function);     /* Parentize nicely */

	/* Add the replacement block
	 */
	*b = ret.replacement;

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

	//Discover the variables used in block b
	usedvars = analyze_used_vars(*b);

	//If there are any copyin variables we need to include them no matter
	//if they are used or not
	//We use a new set instead of putting them directly into usedvars[DCT_BYREF]
	//because xc_ompcon_get_vars doesn't check if the variables already exist
	//(uses set_put() instead of set_put_unique()) and we also need to mark them
	copyinvars = set_new(vars);
	xc_ompcon_get_vars((*b)->u.omp, OCCOPYIN, copyinvars);
	for (e = copyinvars->first; e; e = e->next)
		set_put_unique(usedvars[DCT_BYREF], e->key)->value = OCCOPYIN;
	set_free(copyinvars);


	//Check for default clause
	def = xc_ompcon_get_unique_clause((*b)->u.omp, OCDEFAULT);

	//If default(none) is specified and we have implicit vars throw an error
	if (def && def->subtype == OC_defnone && usedvars[DCT_UNSPECIFIED]->first)
	{
		exit_error(1,
		           "(%s) openmp error:\n\t"
		           "variable `%s' must be explicitely declared as shared/private\n\t"
		           "due to the default(none) clause at line %d.\n",
		           (*b)->file->name, usedvars[DCT_UNSPECIFIED]->first->key->name,
		           (*b)->u.omp->directive->l);
	}
	else
		if (def && def->subtype == OC_defshared)
			oo.implicitDefault = implicitDefaultByRef;

	//Move the implicit variables to the appropriate set
	for (e = usedvars[DCT_UNSPECIFIED]->first; e; e = e->next)
		set_put(usedvars[oo.implicitDefault(e)], e->key);

	//Remove the directive from the code
	d  = *b;
	*b = (*b)->u.omp->body;
	(*b)->parent = d->parent;

	ret = outline(b, oo, usedvars);

	/* (1) Put the directive in a comment
	 */
	ast_ompdir_print(st1, (d->u.omp->directive));
	commented_directive = (cppLineNo) ?
	                      BlockList(
	                        verbit("/* (l%d) %s -- body moved below */",
	                               d->l, str_string(st1)),
	                        verbit("# %d \"%s\"", d->l, d->file->name)
	                      ) :
	                      verbit("/* (l%d) %s -- body moved below */",
	                             d->l, str_string(st1));
	out_insert_before(ret.replacement, verbit("/* (l%d) %s */", d->l,
	                                          str_string(st1)));
	out_insert_before(ret.function, verbit("/* Outlined code for (l%d) %s */",
	                                       d->l, str_string(st1)));
	out_insert_before(ret.functionbody, commented_directive);

	/* (10) Add the new function, along with the struct type definition
	 */
	xform_add_threadfunc(Symbol(oo.functionName), ret.function,
	                     ast_get_enclosing_function(*b));

	str_free(st1);

	//Free the directive
	d->u.omp->body = NULL;     /* Make it NULL so as to free it easily */
	ast_free(d);                /* Get rid of the OmpStmt */

	//If we are in a target region write the generated function to the target
	//tree
	if (inTarget())
	{
		targtree = BlockList(targtree, ret.function);
		symtab_insert_global(stab, Symbol(oo.functionName), FUNCNAME)->isindevenv = 3;
	}

	if (inDeclTarget)
		symtab_insert_global(stab, Symbol(oo.functionName), FUNCNAME)->isindevenv = 3;

	return ret;
}


/**
 * Add a statement before another statement
 *
 * @param where The old statement
 * @param what  The statement you want to insert
 */
void out_insert_before(aststmt where, aststmt what)
{
	aststmt cp = smalloc(sizeof(struct aststmt_));

	//Copy the where statement
	*cp = *where;

	//Change the where statement into a BlockList
	where->type    = STATEMENTLIST;
	where->subtype = 0;

	//Put the old where and what into the BlockList
	where->u.next  = what;
	where->body    = cp;

	//Parentize
	ast_stmt_parent(where->parent, where);
}

/**
 * Add a statement after another statement
 *
 * @param where The old statement
 * @param what  The statement you want to insert
 */
void out_insert_after(aststmt where, aststmt what)
{
	aststmt cp = smalloc(sizeof(struct aststmt_));

	//Copy the where statement
	*cp = *where;

	//Change the where statement into a BlockList
	where->type    = STATEMENTLIST;
	where->subtype = 0;

	//Put the old where and what into the BlockList
	where->u.next  = cp;
	where->body    = what;

	//Parentize
	ast_stmt_parent(where->parent, where);
}
