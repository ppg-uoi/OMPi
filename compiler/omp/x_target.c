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

/* x_target.c */

#include "ast_copy.h"
#include "ast_free.h"
#include "ast_print.h"
#include "ast_vars.h"
#include "ast_xform.h"
#include "x_target.h"
#include "x_clauses.h"
#include "x_types.h"
#include "symtab.h"
#include "ompi.h"
#include "outline.h"
#include "str.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define HOST_ID      0 /* We should probably move these into common */
#define AUTO_DEVICE -1 /* (along with the ones from ort.h)          */

/* TODO should probably split this into multiple files */

/* A tree holding the current target code
 */
aststmt targtree = NULL, targnewglobals = NULL;
int     targetnum;

typedef struct target_list_
{
	char                *kernelfile;
	char                *functionName;
	aststmt              tree, newglobals, rep_struct;
	astexpr              decl_struct;
	aststmt              CL1_2_wrapper_body;   /* OpenCL 1.2 */
	astdecl              CL1_2_wrapper_params; /* OpenCL 1.2 */
	struct target_list_ *next;
} *target_list_t;

/* A list containing all the target trees */
target_list_t Targets = NULL;

aststmt declstruct = NULL;


/* Iterates over a set of variables "s" and creates a function call to
 * "functionname" for manipulating the variable on "device". The generated code
 * is returned through "stmt"
 */
static int data_mem_op(set(vars) s, aststmt *stmt, char *functionname,
                       char *comment, astexpr device, bool onlyexisting,
                       bool *hasdeclvars)
{
	int           n = 0;
	setelem(vars) e;
	astexpr       functioncall = IdentName(functionname);

	for (e = s->first; e; e = e->next)
	{
		int isindevenv = symtab_get(stab, e->key, IDNAME)->isindevenv;
		/* If onlyexisting is true we only create calls for variables that are
		 * already in a device data environment
		 */
		if (onlyexisting && !isindevenv)
			continue;

		if (isindevenv == 1 && hasdeclvars)
			*hasdeclvars = true;

		//functionname(&var, sizeof(var), device, DENV_VAR_NAME);
		*stmt = BlockList(
		          (n == 0) ? //Comment
		          ((*stmt == NULL) ? verbit(comment) : BlockList(*stmt, verbit(comment))) :
		          *stmt,
		          FuncCallStmt(
		            functioncall,
		            CommaList(
		              UOAddress(Identifier(e->key)),
		              device ?
		                CommaList(
		                  Sizeof(Identifier(e->key)),
		                  ast_expr_copy(device)
		                ) :
		                Sizeof(Identifier(e->key))
		            )
		          )
		        );

		n++;
	}

	return n;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     DECLARE TARGET                                            *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static set(vars) declare_funcproto = NULL; /* Declared function prototypes */
static aststmt   declare_functions = NULL; /* Code of declared functions */
static set(vars) declare_variables = NULL; /* Declared variables */


/* VVD--add totally global var (e.g. reduction locks) */
void declaretarget_inject_ident(symbol s)
{
	set_put(declare_variables, s);
}


void declaretarget_idents(astdecl t)
{
	if (!t) return;
	switch (t->type)
	{
		case DIDENT:
			set_put(declare_variables, t->u.id);
			symtab_get(stab, t->u.id, IDNAME)->isindevenv = 1;
			break;
		case DFUNC:
			set_put(declare_funcproto, t->decl->u.id);
			symtab_get(stab, t->decl->u.id, FUNCNAME)->isindevenv = 1;
			break;
		case DLIST:
			declaretarget_idents(t->u.next);
		default:
			declaretarget_idents(t->decl);
	}
}


void declaretarget_discover(aststmt t)
{
	if (!t) return;
	switch (t->type)
	{
		case DECLARATION:
			if (t->u.declaration.decl)
				declaretarget_idents(t->u.declaration.decl);
			break;
		case FUNCDEF:
			declare_functions = (declare_functions) ? BlockList(declare_functions, t) : t;
			symtab_get(stab, decl_getidentifier_symbol(t->u.declaration.decl),
			           FUNCNAME)->isindevenv = 1;
			break;
		case STATEMENTLIST:
			declaretarget_discover(t->u.next);
			declaretarget_discover(t->body);
			break;
	}
}


void xform_declaretarget(aststmt *t)
{
	aststmt body = (*t)->u.omp->body, cdirective, tmp = body;

	/* Create sets */
	if (!declare_variables) declare_variables = set_new(vars);
	if (!declare_funcproto) declare_funcproto = set_new(vars);

	/* Search body for declarations */
	declaretarget_discover(body);

	/* Create a comment containing the directive */
	cdirective = ompdir_commented((*t)->u.omp->directive);

	(*t)->u.omp->body = NULL;     /* Make it NULL so as to free it easily */
	ast_free(*t);                 /* Get rid of the OmpStmt */
	*t = body;
}

/* if (__register_global_decl_p)
 *   ort_call_decl_reg_func(&__register_global_decl_p);
 */
static inline aststmt call_register_func()
{
	return If(IdentName("__register_global_decl_p"),
	          FuncCallStmt(
	            IdentName("ort_call_decl_reg_func"),
	            UOAddress(IdentName("__register_global_decl_p"))
	          ),
	          NULL
	         );
}


/**
 * Here we produce code for the declared variables and add it to the outlined
 * areas. This function is called just before *_ompi.c file is printed.
 */
void produce_decl_var_code()
{
	setelem(vars) e;
	stentry  orig;
	aststmt  tmp, vars = NULL, regfunc = NULL, structinit = NULL;
	astdecl  strfields = NULL, dtmp;
	symbol   structVar = Symbol("_decl_data"),
	         structType = Symbol("__decl_struct");
	astspec  stmp;
	target_list_t t;

	head_add(verbit("#define __OPENCL_ASQ"));

	/* If there are no declared variables we don't need a register function */
	if (!declare_variables || set_isempty(declare_variables) == true)
	{
		/* Since there are no variables we declare a null pointer
		 * void (* __register_global_decl_p)(void) = (void *) 0;
		 */
		newglobalvar(
		  Declaration(
		    Speclist_right(
		      StClassSpec(SPEC_static),
		      Declspec(SPEC_void)
		    ),
		    InitDecl(
		      Declarator(
		        NULL,
		        FuncDecl(
		          ParenDecl(
		            Declarator(
		              Declspec(SPEC_star),
		              IdentifierDecl(Symbol("__register_global_decl_p"))
		            )
		          ),
		          ParamDecl(Declspec(SPEC_void), NULL)
		        )
		      ),
		      NullExpr()
		    )
		  ));

		return;
	}

	for (e = declare_variables->first; e; e = e->next)
	{
		orig = symtab_get(stab, e->key, IDNAME);

		/* If the variable had an initializer copy it as is in the initializer
		 * function we make it static const and add "init_" to it's name
		 */
		if (orig->idecl)
		{
			A_str_truncate();
			str_printf(strA(), "init_%s", e->key->name);

			//static const init_<original name> = <original initializer>
			tmp = Declaration(
			        Speclist_right(
			          Speclist_right(
			            StClassSpec(SPEC_static),
			            StClassSpec(SPEC_const)),
			          ast_spec_copy_nosc(orig->spec)
			        ),
			        xc_decl_rename(
			          ast_decl_copy(orig->idecl), Symbol(A_str_string())
			        )
			      );
			vars = vars ? BlockList(vars, tmp) : tmp;
		}

		/* Register the variables */
		//ort_register_declvar(&<var>, sizeof(<var>), [&init_<var>|(void *) 0]);
		tmp = FuncCallStmt(
		        IdentName("ort_register_declvar"),
		        CommaList(
		          UOAddress(Identifier(e->key)),
		          CommaList(
		            Sizeof(Identifier(e->key)),
		            orig->idecl ?
		            /* &init_<var> */
		            UOAddress(IdentName(A_str_string())) :
		            /* (void *) 0 */
		            NullExpr()
		          )
		        )
		      );
		regfunc = regfunc ? BlockList(regfunc, tmp) : tmp;

		/* Create the fields for the struct used to pass declared variable's
		 * values
		 */
		dtmp = StructfieldDecl(ast_spec_copy_nosc(orig->spec),
		                       xc_decl_topointer(ast_decl_copy(orig->decl)));
		strfields = strfields ? StructfieldList(strfields, dtmp) : dtmp;

		dtmp = xc_decl_topointer(ast_decl_copy(orig->decl));
		/* _decl_data-><var> = (<type> *) ort_get_declvar(&<var>, __ompi_devID); */
		tmp = AssignStmt(
		        PtrField(Identifier(structVar), e->key),
		        CastedExpr(
		          Casttypename(
		            ast_spec_copy_nosc(orig->spec),
		            xt_concrete_to_abstract_declarator(dtmp)
		          ),
		          FunctionCall(
		            IdentName("ort_get_declvar"),
		            CommaList(
		              UOAddress(Identifier(orig->key)),
		              IdentName("__ompi_devID")
		            )
		          )
		        )
		      );
		free(dtmp);
		structinit = structinit ? BlockList(structinit, tmp) : tmp;
	}

	/* Create the struct using the fields created in the previous loop
	 * struct __decl_struct {
	 *   <strfields>
	 * } *_decl_data;
	 */
	declstruct  = Declaration(
	                SUdecl(SPEC_struct, structType, ast_decl_copy(strfields)),
	                Declarator(Pointer(), IdentifierDecl(structVar))
	              );

	//_decl_data = (struct __decl_struct *) ort_devdata_alloc(sizeof(struct ___decl_struct), __ompi_devID);
	tmp = AssignStmt(
	        Identifier(structVar),
	        CastedExpr(
	          Casttypename(
	            SUdecl(SPEC_struct, structType, NULL),
	            AbstractDeclarator(Pointer(), NULL)
	          ),
	          FunctionCall(
	            IdentName("ort_devdata_alloc"),
	            CommaList(
	              Sizeoftype(
	                Casttypename(
	                  SUdecl(SPEC_struct, structType, NULL),
	                  NULL
	                )),
	              IdentName("__ompi_devID")
	            )
	          )
	        )
	      );
	structinit = BlockList(tmp, structinit);

	/* Insert the code for the variables on each target */
	for (t = Targets; t; t = t->next)
	{
		/* The struct */
		out_insert_after(t->rep_struct, ast_copy(declstruct));
		/* Call to the register function */
		out_insert_after(t->rep_struct, ast_copy(call_register_func()));
		/* The initialization of the struct */
		out_insert_after(t->rep_struct, ast_copy(structinit));

		/* Add the struct to the offload function
		 * Here we replace the '0' with '_decl_data'
		 */
		free(t->decl_struct->left);  /* Free the "numConstant(0)" */
		t->decl_struct->left = Identifier(structVar);
	}
	ast_free(structinit);

	if (vars)
		regfunc = BlockList(vars, regfunc);

	//static void
	stmp = Speclist_right(StClassSpec(SPEC_static), Declspec(SPEC_void));
	dtmp = Declarator(NULL, FuncDecl(
	                    IdentifierDecl(Symbol("__register_global_decl")), NULL));

	/* Add the register function
	 * static void __register_global_decl()
	 * {
	 *   <regfunc>
	 * }
	 */
	regfunc = FuncDef(stmp, dtmp, NULL, Compound(regfunc));
	tail_add(regfunc);

	/* Add the protype of the register function:
	 * static void __register_global_decl();
	 */
	newglobalvar(Declaration(stmp, dtmp));

	/* Add a variable pointing to the register function. It will be used for
	 * calling the function once and then it will be nulled.
	 * void (* __register_global_decl_p)(void) = __register_global_decl;
	 */
	newglobalvar(Declaration(
	               Declspec(SPEC_void),
	               InitDecl(
	                 Declarator(
	                   NULL,
	                   FuncDecl(
	                     ParenDecl(
	                       Declarator(
	                         Declspec(SPEC_star),
	                         IdentifierDecl(Symbol("__register_global_decl_p"))
	                       )
	                     ),
	                     ParamDecl(Declspec(SPEC_void), NULL)
	                   )
	                 ),
	                 IdentName("__register_global_decl")
	               )));
}

/**
 * Here we produce the kernel file for each target
 */
void produce_target_files()
{
	setelem(vars)  e;
	target_list_t  t;
	FILE          *fp;
	aststmt        sue = copy_sue_declarations(true), inits = NULL, tmp, 
	               body, bindcmds;
	astexpr        tmpexpr;
	symbol         structVar = Symbol("_decl_data"),
	               structArg = Symbol("__decl_data"),
	               structType = Symbol("__decl_struct");
	astdecl        dtmp;
	int            globalpos;

	/* Empty the 'A' scratchpad */
	A_str_truncate();

	str_printf(strA(), "#ifdef __OMPI_CL_KERNEL__\n\t#define __OPENCL_ASQ "
	                   "global\n#else\n\t#define __OPENCL_ASQ\n#endif\n\n");

	/* If there are any global structs add them to the code */
	if (sue)
	{
		ast_stmt_print(strA(), sue);
		ast_stmt_free(sue);
	}

	/* Add any declared function prototypes */
	if (declare_funcproto && !set_isempty(declare_funcproto))
		for (e = declare_funcproto->first; e; e = e->next)
			ast_stmt_print(strA(), xform_clone_funcdecl(e->key));

	/* Prepare code for declared variables in the wrapper function */
	if (declare_variables && !set_isempty(declare_variables))
	{
		for (e = declare_variables->first; e; e = e->next)
		{
			astexpr structVarId = Identifier(structVar);

			/* Copy the variable declarations in the kernel file and turn them
			 * into pointers
			 */
			ast_stmt_print(strA(), xform_clone_declaration(e->key, NULL, true));

			/* Initialize the variable pointers */
			//_decl_data-><var>
			tmpexpr = PtrField(structVarId, e->key);
			//<var> = devrt_get_dev_address(<tmpexpr>, sizeof(*(<tmpexpr>))
			tmp = AssignStmt(Identifier(e->key),
			        FunctionCall(
			          IdentName("devrt_get_dev_address"),
			          CommaList(
			            tmpexpr,
			            Sizeof(
			              UnaryOperator(
			                UOP_star,
			                Parenthesis(ast_expr_copy(tmpexpr))
			              )
			            )
			          )
			        )
			);
			inits = inits ? BlockList(inits, tmp) : tmp;
		}

		/* Cast the wrapper function parameter into the struct */
		// _decl_data = (struct __decl_struct  *) __decl_data;
		inits = BlockList(
		          AssignStmt(
		            Identifier(structVar),
		            CastedExpr(
		              Casttypename(
		                SUdecl(SPEC_struct, structType, NULL),
		                AbstractDeclarator(Pointer(), NULL)
		              ),
		              Identifier(structArg)
		            )
		          ), inits);
	}

	/* Add declared functions */
	if (declare_functions)
	{
		/* Pointerize declared variables */
		analyze_pointerize_declared_vars(declare_functions);
		ast_stmt_print(strA(), declare_functions);
	}

	/* Keep the position of the prototypes/variables/functions in the scratchpad */
	globalpos = A_str_tell();

	//TODO use call graph.
	/* For each target in the 'Targets' list produce a kernel file */
	while (Targets)
	{
		t = Targets;

		if ((fp = fopen(t->kernelfile, "w")) == NULL) return;

		/* Pointerize declared variables */
		analyze_pointerize_declared_vars(t->tree);

		t->tree = BlockList(t->tree,
		                    verbit("#ifndef __OMPI_CL_KERNEL__")); /* OpenCL 1.2 */

		/* Create a single binding point for declared variables
		* void * _bindFunc_(void * __decl_data)
		* {
		*   <bindcmds>
		* }
		*/
		bindcmds = Return(NullExpr());
		if (declstruct)  
			bindcmds = BlockList(
			             BlockList(
			               ast_stmt_copy(declstruct), 
			               ast_stmt_copy(inits)
			             ),
			             bindcmds
			           );
		t->tree = BlockList(
		            t->tree,
		            FuncDef(
		              Usertype(Symbol("WRAPPER_RET_TYPE")),
		              Declarator(
		                NULL,
		                FuncDecl(
		                  IdentifierDecl(Symbol("_bindFunc_")),
		                  ParamDecl(
		                    Usertype(Symbol("WRAPPER_PARAM_TYPE")),
		                    Declarator(
		                      NULL,
		                      IdentifierDecl(structArg)
		                    )
		                  )
		                )
		              ),
		              NULL,
		              Compound(bindcmds)
		            )
		          );

		/* Create a single entry point (wrapper function)
		* void * _kernelFunc_(void * __dev_data, void * __decl_data)
		* {
		*   <body>
		* }
		*/
		//_kernelFunc[targetnum]_(__dev_data);
		body = BlockList(
		         FuncCallStmt(IdentName(t->functionName), IdentName("__dev_data")),
		         Return(NullExpr())
		       );

		if (t->CL1_2_wrapper_body) /* OpenCL 1.2 */
			t->CL1_2_wrapper_body = BlockList(t->CL1_2_wrapper_body, ast_copy(body));
		else
			t->CL1_2_wrapper_body = ast_copy(body);

		if (declstruct)
			body = BlockList(
			         Expression(   /* _bindFunc_(__decl_data); */
			           FunctionCall(
			             IdentName("_bindFunc_"),
			             IdentName("__decl_data")
			           )
			         ), 
			         body
			       );
		
		t->tree = BlockList(
		            t->tree,
		            FuncDef(
		              Usertype(Symbol("WRAPPER_RET_TYPE")),
		              Declarator(
		                NULL,
		                FuncDecl(
		                  IdentifierDecl(Symbol("_kernelFunc_")),
		                  ParamList(
		                    ParamDecl(
		                      Usertype(Symbol("WRAPPER_PARAM_TYPE")),
		                      Declarator(
		                        NULL,
		                        IdentifierDecl(Symbol("__dev_data"))
		                      )
		                    ),
		                    ParamDecl(
		                      Usertype(Symbol("WRAPPER_PARAM_TYPE")),
		                      Declarator(
		                        NULL,
		                        IdentifierDecl(structArg)
		                      )
		                    )
		                  )
		                )
		              ),
		              NULL,
		              Compound(body)
		            )
		          );

		/* --------------------------- OpenCL 1.2 --------------------------- */
		t->tree = BlockList(t->tree, verbit("#else"));

		dtmp = ParamDecl(Declspec(SPEC_void), Declarator(
		                   Pointer(), IdentifierDecl(Symbol("__dev_data"))));

		if (t->CL1_2_wrapper_params)
			dtmp = ParamList(dtmp, t->CL1_2_wrapper_params);

		t->tree = BlockList(
		            t->tree,
		            FuncDef(
		              Declspec(SPEC_void),
		              Declarator(
		                Pointer(),
		                FuncDecl(
		                  IdentifierDecl(Symbol("_kernelFunc_")), dtmp
		                )
		              ),
		              NULL,
		              Compound(t->CL1_2_wrapper_body)
		            )
		          );

		t->tree = BlockList(t->tree, verbit("#endif"));
		/* ------------------------- End OpenCL 1.2 --------------------------*/

		/* Add the new globals */
		if (t->newglobals)
		{
			if (cppLineNo)
				t->tree = BlockList(
				            BlockList(
				              verbit("# 1 \"%s-newglobals\"", filename),
				              BlockList(
				                t->newglobals,
				                verbit("# 1 \"%s\"", filename)
				              )
				            ),
				            t->tree
				          );
			else
				t->tree = BlockList(t->newglobals, t->tree);
		}

		/* Return the scratchpad to the position of the declared stuff */
		A_str_seek(globalpos);

		/* Comment @ top */
		fprintf(fp, "%s\n", advert);

		/* Print the target tree */
		ast_stmt_print(strA(), t->tree);
		fprintf(fp, "%s", A_str_string());
		fclose(fp);

		/* Move to the next item in the 'Targets' list and free the current one */
		Targets = Targets->next;
		free(t->kernelfile);
		free(t->functionName);
		ast_stmt_free(t->tree);
		free(t);
	}

	/* Free the statement blocks. It's not necessary since the compiler will
	 * terminate shortly afterwards but better safe than sorry.
	 */
	if (declare_variables && !set_isempty(declare_variables))
	{
		ast_stmt_free(inits);
		ast_stmt_free(declstruct);
		inits = declstruct = NULL;
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     TARGET UPDATE                                             *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void xform_targetupdate(aststmt *t)
{
	astexpr    deviceexpr = NULL, ifexpr = NULL;
	ompclause  c;
	aststmt    parent = (*t)->parent, v, st = NULL, devicestm = NULL;
	static set(vars) to = NULL, from = NULL, unique = NULL;
	setelem(vars) se;
	stentry    orig;
	char       *filename = (*t)->u.omp->directive->file->name;
	bool       hasdeclvars = false;
	int        line = (*t)->u.omp->directive->l;

	set_init(vars, &to);
	set_init(vars, &from);
	set_init(vars, &unique);

	/* (1) Check for device and if clauses and keep a copy
	 */
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCDEVICE)) != NULL)
		deviceexpr = ast_expr_copy(c->u.expr);
	else
		deviceexpr = numConstant(AUTO_DEVICE);
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCIF)) != NULL)
		ifexpr = ast_expr_copy(c->u.expr);

	/* (2) Store device id in a variable to avoid re-evaluating the expression
	 */
	//int __ompi_devID = (<deviceexpr>);
	devicestm = Declaration(
	              Declspec(SPEC_int),
	              InitDecl(
	                Declarator(
	                  NULL,
	                  IdentifierDecl(Symbol("__ompi_devID"))
	                ),
	                Parenthesis(deviceexpr)
	              )
	            );
	deviceexpr = IdentName("__ompi_devID");

	/* (3) Get the variables in to/from clauses
	 */
	xc_ompcon_get_vars((*t)->u.omp, OCTO, to);
	xc_ompcon_get_vars((*t)->u.omp, OCFROM, from);

	/* (4) Check if there was at least one motion clause
	 */
	if (set_isempty(to) && set_isempty(from))
		exit_error(1, "(%s, line %d) openmp error:\n\t"
		           "Target update construct needs at least one to/from clause\n",
		           filename, line);

	/* (5) Check if the variables in to/from clauses appear more than once
	 */
	for (se = to->first; se; se = se->next)
	{
		if (set_get(unique, se->key))
			exit_error(1, "(%s, line %d) openmp error:\n\t"
			           "variable `%s' appears more than once in the directive's"
			           " clause(s)\n", filename, line, se->key->name);
		set_put(unique, se->key);
	}
	for (se = from->first; se; se = se->next)
	{
		if (set_get(unique, se->key))
			exit_error(1, "(%s, line %d) openmp error:\n\t"
			           "variable `%s' appears more than once in the directive's"
			           " clause(s)\n", filename, line, se->key->name);
		set_put(unique, se->key);
	}

	/* (6) Check if the variables in to/from clauses are actually in a data env
	 */
	for (se = unique->first; se; se = se->next)
	{
		if ((orig = symtab_get(stab, se->key, IDNAME)) != NULL && !orig->isindevenv)
			exit_error(1, "(%s, line %d) openmp error:\n\t"
			           "variable `%s' cannot be updated because it does not "
			           "appear in a target data clause\n",
			           filename, line, se->key->name);
	}


	/* (7) Write function calls for transferring to/from the device
	 */
	data_mem_op(to, &st, "ort_write_tdvar", "/* to */", deviceexpr , false,
	            &hasdeclvars);
	data_mem_op(from, &st, "ort_read_tdvar", "/* from */", deviceexpr, false,
	            &hasdeclvars);

	/* (8) Comment the directive
	 */
	/* Create a comment containing the directive */
	v = ompdir_commented((*t)->u.omp->directive);

	/* Replace the directive with the comment */
	ast_free(*t);                 /* Get rid of the OmpStmt */
	*t = v;

	/* (9) Register declared variables
	 */
	if (hasdeclvars)
		st = BlockList(call_register_func(), st);

	/* (10) Add the code for the deviceid
	 */
	st = Compound(BlockList(devicestm, st));

	/* (11) Add if clause
	 */
	if (ifexpr)                    /* Check if we have an if() clause */
		st = If(ifexpr, st, NULL);

	*t = BlockList(*t, st);

	/* (12) Parentize new statements
	 */
	ast_stmt_parent(parent, *t);

	/* (13) Free the original deviceexpr since data_mem_operation creates copies.
	 */
	ast_expr_free(deviceexpr);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     TARGET DATA                                               *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * "__ort_denv" is a variable that holds the current data environment. Each time
 * we encounter a directive that creates a new data environment a new variable
 * is created. The code for creating this variable is normally generated inside
 * the function that transforms the directive. The problem is that the body of
 * the directive is transformed first, and the variable may be used inside the
 * body so it needs to be in the symbol table (e.g. when we have:
 * #omp declare target map(x)
 * #omp parallel
 * #omp target
 * { code using x }
 *
 * the "target" directive is using the "__ort_denv" variable generated by the
 * "declare target" directive and the variable must be available in the symbol
 * table so that the parallel will include it in the struct it creates.)
 *
 * The variable is inserted into the symbol table in ast_xform:xform_ompcon_body
 * and the actual code is used in x_target:xform_target, xform_targetdata
 *
 * Furthermore since we might have a directive create a new data environment
 * inside another directive that has already created a data environment a second
 * variable will be declared and used while the first is in the symbol table
 * but has not been inserted in the code. Therefore we need to keep a stack
 * of the variables "__ort_denv" that have been created but have not yet been
 * used.
 *
 * @param createNew when true create a new declaration, else return the top in
 *                  the stack
 *
 * @return a "void *__ort_denv" declaration
 */
aststmt get_denv_var_decl(bool createNew)
{
	typedef struct _stack
	{
		aststmt denv_var_decl;
		struct _stack *next;
	} list;

	static list *top = NULL;

	if (createNew)
	{
		list *new = (list *) smalloc(sizeof(list));
		new->next = top;
		top       = new;

		/* We initialize with 0 and change it later in "createDeviceDataEnv"*/
		// void *__ort_denv = 0;
		new->denv_var_decl = Declaration(
		                       Declspec(SPEC_void),
		                       InitDecl(
		                         Declarator(
		                           Pointer(),
		                           IdentifierDecl(Symbol("__ort_denv"))
		                         ),
		                         numConstant(0)
		                       )
		                     );

		return top->denv_var_decl;
	}

	/* Top should never be null when we retrieve a declaration. For each "push"
	 * there is exactly one "pop"*/
	assert(top);


	aststmt ret = top->denv_var_decl;
	list   *tmp = top;

	top = top->next;

	free(tmp);

	return ret;
}


//int __ompi_devID = [(<deviceexpr>)|(<ifexpr>) ? (<deviceexpr>) : 0];
static inline aststmt device_statement(astexpr ifexpr, astexpr deviceexpr)
{
	return Declaration(
	         Declspec(SPEC_int),
	         InitDecl(
	           Declarator(NULL, IdentifierDecl(Symbol("__ompi_devID"))),
	           /* If there is an if clause and it evaluates to false use
	            * device 0 (host) */
	           (ifexpr) ?
	           ConditionalExpr(
	             Parenthesis(ifexpr),
	             Parenthesis(deviceexpr),
	             numConstant(HOST_ID)
	           ) :
	           Parenthesis(deviceexpr)
	         )
	       );
}


/* Generates the code for a device data environment
 * If onlyexisting is true we only create calls for variables that are
 * already in a device data environment
 */
static void createDeviceDataEnv(aststmt *t, set(vars) *usedvars,
                                astexpr deviceexpr, bool onlyexisting)
{
	aststmt before = NULL, after = NULL, tmp;
	int     nvars;
	bool    dvars = false;

	/* Handle the variables
	 */
	//map(alloc: )
	nvars = data_mem_op(usedvars[DCT_PRIVATE], &before, "ort_alloc_tdvar",
	                    "/* alloc */", NULL, onlyexisting, &dvars);

	//map(to: )
	nvars += data_mem_op(usedvars[DCT_BYVALUE], &before, "ort_init_tdvar",
	                     "/* to */", NULL, onlyexisting, &dvars);

	//map(from: )
	nvars += data_mem_op(usedvars[DCT_BYRESULT], &before, "ort_alloc_tdvar",
	                     "/* from */", NULL, onlyexisting, &dvars);
	data_mem_op(usedvars[DCT_BYRESULT], &after, "ort_finalize_tdvar",
	            "/* from */", NULL, onlyexisting, NULL);

	//map(tofrom: )
	nvars += data_mem_op(usedvars[DCT_BYVALRES], &before, "ort_init_tdvar",
	                     "/* tofrom */", NULL, onlyexisting, &dvars);
	data_mem_op(usedvars[DCT_BYVALRES], &after, "ort_finalize_tdvar",
	            "/* tofrom */", NULL, onlyexisting, NULL);

	//Variables used in a "target" that have not been explicitly defined and are
	//not in a device data environment
	nvars += data_mem_op(usedvars[DCT_DDENV], &before, "ort_init_tdvar",
	                     "/* undefined (ddenv/tofrom) */", NULL, 0, &dvars);
	data_mem_op(usedvars[DCT_DDENV], &after, "ort_finalize_tdvar",
	            "/* undefined (ddenv/tofrom) */", NULL, 0, NULL);

	/* There should be no declared variables in the usedvars */
	assert(dvars == false);

	/*Don't create a device data environment if there are no variables */
	if (nvars == 0)
	{
		ast_stmt_free(get_denv_var_decl(false));
		return;
	}

	/* Mark start/end of target data
	 */
	//We replace the initializer of the __ort_denv variable that was stored in
	//get_denv_var_decl())
	//0 -> ort_start_target_data(numberofvars, deviceexpr)
	tmp = get_denv_var_decl(false);             /* Get the variable declaration */
	free(tmp->u.declaration.decl->u.expr);         /* Free the "numConstant(0)" */
	tmp->u.declaration.decl->u.expr =                     /* Place the new call */
	  FunctionCall(
	    IdentName("ort_start_target_data"),
	    CommaList(
	      numConstant(nvars),
	      ast_expr_copy(deviceexpr)
	    )
	  );
	before = (before) ? BlockList(tmp, before) : tmp;

	//ort_end_target_data(__ort_denv);
	tmp = FuncCallStmt(IdentName("ort_end_target_data"),
	                   IdentName("__ort_denv"));
	after = (after) ? BlockList(after, tmp) : tmp;

	/* Insert all the code
	 */
	//Insert allocations and initializations
	*t = BlockList(before, *t);

	//Insert reads
	*t = BlockList(*t , after);
}


void xform_targetdata(aststmt *t)
{
	astexpr   deviceexpr = NULL, ifexpr = NULL;
	ompclause c;
	aststmt   s = (*t)->u.omp->body, parent = (*t)->parent, v, devicestm;

	/* (1) Find used variables
	 */
	set(vars) *usedvars = analyze_used_vars(*t);

	/* (2) Check for device and if clauses and keep a copy
	 */
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCDEVICE)) != NULL)
		deviceexpr = ast_expr_copy(c->u.expr);
	else
		deviceexpr = numConstant(AUTO_DEVICE);
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCIF)) != NULL)
		ifexpr = ast_expr_copy(c->u.expr);

	/* (3) Store device id in a variable to avoid re-evaluating the expression
	 */
	devicestm = device_statement(ifexpr, deviceexpr);
	deviceexpr = IdentName("__ompi_devID");

	/* (4) Comment the directive
	 */
	/* Create a comment containing the directive */
	v = ompdir_commented((*t)->u.omp->directive);

	(*t)->u.omp->body = NULL;     /* Make it NULL so as to free it easily */
	ast_free(*t);                 /* Get rid of the OmpStmt */
	*t = s;

	/* (5) Create the code for the data environment
	 */
	createDeviceDataEnv(t, usedvars, deviceexpr, false);

	/* (6) Add extra code
	 */
	*t = BlockList(devicestm, *t);

	//Surround with {} to give place the new variables on a new scope
	*t = Compound(*t);

	//Add the commented directive
	*t = BlockList(v, *t);

	/* (7) Parentize new statements
	 */
	ast_stmt_parent(parent, *t);

	/* (8) Free the original deviceexpr since data_mem_operation creates copies.
	 */
	ast_expr_free(deviceexpr);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     TARGET                                                    *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* If true all variables are added in the device data environment, else only the
 * ones that already are in a device data environment are treated as "DDENV"
 */
const bool allvarsindevenv = false;

static vartype_t xtarget_implicitDefault(setelem(vars) s)
{
	stentry orig = symtab_get(stab, s->key, IDNAME);

	//Check whether the variable was declared in a target data or declare target
	return (orig->isindevenv == 1) ? DCT_IGNORE : (orig->isindevenv == 2) ?
	       DCT_DDENV : DCT_BYVALRES;
}

static void OpenCL1_2_iter(astexpr commalist, set(vars) usedvars, target_list_t t)
{
	setelem(vars) e;

	for (e = usedvars->first; e; e = e->next)
	{
		stentry orig = symtab_get(stab, e->key, IDNAME);
		astdecl tmp;

		if (!allvarsindevenv && !orig->isindevenv)
			continue;

		/* Variadic call */
		commalist->right = CommaList(
		                     commalist->right,
		                     UOAddress(Identifier(e->key))
		                   );
		commalist->right = CommaList(       /* For OpenCL 2.0 */
		                     commalist->right,
		                     UOAddress(PtrField(IdentName("_dev_data"), e->key))
		                   );

		/* Function parameters */
		tmp = ParamDecl(Speclist_right(Usertype(Symbol("global")),
		                               ast_spec_copy_nosc(orig->spec)), ast_decl_copy(orig->decl));
		t->CL1_2_wrapper_params = t->CL1_2_wrapper_params ?
		                          ParamList(t->CL1_2_wrapper_params, tmp) : tmp;

		/* Struct initialization */
		tmp = xc_decl_topointer(ast_decl_copy(orig->decl));
		t->CL1_2_wrapper_body =
		  BlockList(
		    t->CL1_2_wrapper_body,
		    AssignStmt(
		      PtrField(IdentName("_dev_data"), e->key),
		      CastedExpr(
		        Casttypename(
		          Speclist_right(
		            Usertype(Symbol("global")),
		            ast_spec_copy_nosc(orig->spec)
		          ),
		          xt_concrete_to_abstract_declarator(tmp)
		        ),
		        Identifier(e->key)
		      )
		    )
		  );
		free(tmp);
	}
}

static void OpenCL1_2(astexpr commalist, set(vars) *usedvars,
                      aststmt rep_struct, target_list_t t)
{
	t->CL1_2_wrapper_body = ast_copy(rep_struct);
	t->CL1_2_wrapper_params = NULL;

	OpenCL1_2_iter(commalist, usedvars[DCT_PRIVATE], t);
	OpenCL1_2_iter(commalist, usedvars[DCT_BYVALUE], t);
	OpenCL1_2_iter(commalist, usedvars[DCT_BYRESULT], t);
	OpenCL1_2_iter(commalist, usedvars[DCT_BYVALRES], t);
	OpenCL1_2_iter(commalist, usedvars[DCT_DDENV], t);

	//Pass a null to mark the end of the variable list
	commalist->right = CommaList(commalist->right, NullExpr());
}

void xform_target(aststmt *t)
{
	astexpr    deviceexpr = NULL, ifexpr = NULL;
	aststmt    devicestm = NULL, *producedc;
	ompclause  c;
	outcome_t  oc;
	target_list_t newtarget;

	newtarget = (target_list_t) smalloc(sizeof(struct target_list_));
	newtarget->kernelfile = (char *) smalloc((strlen(filename) + 7) * sizeof(char));
	snprintf(newtarget->kernelfile, (strlen(filename) + 5), "%.*s_d%02d",
	         (int)(strlen(filename) - 3), filename, targetnum);
	A_str_truncate();
	str_printf(strA(), "\"%s\"", newtarget->kernelfile);
	strcat(newtarget->kernelfile, ".c");

	/* (2) Check for device and if clauses and keep a copy
	 */
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCDEVICE)) != NULL)
		deviceexpr = ast_expr_copy(c->u.expr);
	else
		deviceexpr = numConstant(AUTO_DEVICE);
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCIF)) != NULL)
		ifexpr = ast_expr_copy(c->u.expr);

	/* () Store device id in a variable to avoid re-evaluating the expression
	 */
	devicestm = device_statement(ifexpr, deviceexpr);
	deviceexpr = IdentName("__ompi_devID");

	static outpars_t op =
	{
		"test",                  //+functionName
		"ort_offload_kernel",    // functionCall
		NULL,                    //+extraParameters
		false,                   // byvalue_is_byname
		true,                    // global_byref_in_struct
		"__dev_struct",          // structName
		"_dev_data",             // structVariable
		NULL,                    // structInitializer
		xtarget_implicitDefault, // implicitDefault function
		NULL                     //+deviceexpr
	};
	sprintf(op.functionName, "_kernelFunc%d_", targetnum++);

	/* The NULL is replaced later with the declared variables struct */
	newtarget->decl_struct = NullExpr();

	//(void *) 0, "<kernelfilename>", <deviceexpr>
	op.extraParameters = CommaList(
	                       newtarget->decl_struct,
	                       CommaList(IdentName(A_str_string()), deviceexpr)
	                     );
	//(struct __dev_struct *) ort_devdata_alloc(sizeof(struct __dev_struct), <deviceexpr>)
	op.structInitializer =
	  CastedExpr(
	    Casttypename(
	      SUdecl(SPEC_struct, Symbol(op.structType), NULL),
	      AbstractDeclarator(Pointer(), NULL)
	    ),
	    FunctionCall(
	      IdentName("ort_devdata_alloc"),
	      CommaList(
	        Sizeoftype(
	          Casttypename(
	            SUdecl(SPEC_struct, Symbol(op.structType), NULL),
	            NULL
	          )),
	        ast_expr_copy(deviceexpr)
	      )
	    )
	  );
	op.deviceexpr = deviceexpr;

	oc = outline_OpenMP(t, op);

	/* () Check if a struct was created and free it
	 */
	if (oc.funcstruct)
		//ort_devdata_free(_dev_data, <deviceexpr>);
		out_insert_after(oc.rep_copyback ? oc.rep_copyback : oc.functioncall,
		                 FuncCallStmt(
		                   IdentName("ort_devdata_free"),
		                   CommaList(
		                     IdentName(op.structName),
		                     ast_expr_copy(deviceexpr)
		                   )
		                 )
		                );

	//In order to place it at the start of the generated code we have to go past
	//the commented directive and into the compound
	producedc = &oc.replacement->body->body;

	/* () Create the code for the data environment
	 */
	createDeviceDataEnv(producedc, oc.usedvars, deviceexpr, !allvarsindevenv);

	OpenCL1_2(op.extraParameters, oc.usedvars, oc.rep_struct, newtarget);

	/* () Insert the variable generated for the device id
	 */
	out_insert_before(*producedc, devicestm);

	ast_parentize(targtree);

	/* () Store the generated code
	 */
	newtarget->tree = ast_stmt_copy(targtree); //TODO memleak?
	newtarget->newglobals = targnewglobals;
	newtarget->rep_struct = oc.rep_struct ? oc.rep_struct : (*producedc)->u.next;
	newtarget->functionName = strdup(op.functionName);
	newtarget->next = Targets;
	Targets = newtarget;

	targtree       = NULL;
	targnewglobals = NULL;
}
