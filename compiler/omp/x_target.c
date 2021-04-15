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

/* x_target.c */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "callgraph.h"
#include "ast_copy.h"
#include "ast_free.h"
#include "ast_print.h"
#include "ast_vars.h"
#include "ast_xform.h"
#include "x_target.h"
#include "x_map.h"
#include "x_decltarg.h"
#include "x_clauses.h"
#include "x_types.h"
#include "x_arith.h"
#include "x_task.h"
#include "symtab.h"
#include "ompi.h"
#include "outline.h"
#include "str.h"
#include "builder.h"

//#define DEVENV_DBG
#ifdef DEVENV_DBG
#include "ast_show.h"
#endif

#define HOSTDEV_ID  0 /* We should probably move these into common */
#define AUTODEV_ID -1 /* (along with the ones from ort.h)          */

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
	bool                 emptyde;              /* true if empty data env */
	astexpr              decl_struct;
	set(cgfun)           calledfuncs;          /* all called funcs */
	aststmt              gpu_wrapper_body;     /* for OpenCL/CUDA */
	astdecl              gpu_wrapper_params;   /* for OpenCL/CUDA */
	targstats_t         *ts;                   /* For CARS */
	struct target_list_ *next;
} *target_list_t;

/* A list containing all the target trees */
target_list_t Targets = NULL;

#define WRAPPER_ARG_NAME "__devdata"
#define DEVENV_STRUCT_NAME "_dev_data"
#define KERNELHEADER "#if defined(__OMPI_CL_KERNEL__)\n"\
                     "\t#define __DEVSPEC global\n"\
                     "\t#define __DEVQLFR\n"\
                     "\t#define __DEVKERN __kernel void\n"\
                     "#elif defined(__OMPI_CUDA_KERNEL__)\n"\
                     "\t#define __DEVSPEC\n"\
                     "\t#define __DEVQLFR __device__\n"\
                     "\t#define __DEVKERN extern \"C\" __global__ void\n"\
                     "#else\n"\
                     "\t#define __DEVSPEC\n"\
                     "\t#define __DEVQLFR\n"\
                     "\t#define __DEVKERN\n"\
                     "#endif\n\n"
#define HOSTKERNDEFS "#define __DEVSPEC\n"\
                     "#define __DEVQLFR\n"\
                     "#define __DEVKERN\n\n"
  

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     KERNEL-RELATED CODE                                       *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/**
 * Here we produce code for the declared variables and add it to the outlined
 * areas. This function is called just before *_ompi.c file is printed.
 */
void produce_decl_var_code()
{
	aststmt  varinits = NULL, regstmts = NULL, structinit;
	target_list_t t;

	bld_head_add(verbit(HOSTKERNDEFS));   /* Definitions for the host file */

	/* If there are no declared variables we don't need nothing */
	if (!declvars_exist())
		return;

	decltarg_struct_code(&varinits, &regstmts, &structinit);

	/* Insert the code for the variables on each target */
	for (t = Targets; t; t = t->next)
	{
		/* Place the decl struct right after the _data_denv code.
		 * This must be obeyed since at runtime, it is assumed that
		 * any link() variables have already been mapped before getting
		 * their address from this decl struct.
		 */
		ast_stmt_append(t->rep_struct, ast_copy(structdecl));
		/* Add the initialization of the struct */
		ast_stmt_append(t->rep_struct, ast_copy(structinit));
		/* Add the struct to the offload function.
		 * Here we replace the '0' with '_decl_data'
		 */
		free(t->decl_struct->left);  /* Free the "numConstant(0)" */
		t->decl_struct->left = Identifier(declstructVar);
	}
	ast_free(structinit);

	if (varinits)
		bld_ortinits_add(varinits);
	bld_ortinits_add(regstmts);
}


/* Typical wrapper for the actual _kernelFuncXX_():
 * 
 * void * _bindFunc_(void * __decl_data) {
 *   <bindcmds>          // bind declared variables
 * }
 * void * _kernelFunc_(void * __dev_data, void * __decl_data) {
 *   <body>              // call _bindFunc() and actual _kernelFuncXX_() 
 * }
 */
static 
aststmt produce_typical_wrapper(char *kfuncname, aststmt bindstruct)
{
	aststmt binder, wrapr;

	/* _bindFunc_(void * __decl_data) */
	binder = Return(NullExpr());
	if (structdecl)
		binder = Block3(ast_stmt_copy(structdecl),ast_stmt_copy(bindstruct),binder);
	binder = FuncDef(
	           Declspec(SPEC_void),
	           Declarator(
	             Pointer(),
	             FuncDecl(
	               IdentifierDecl(Symbol("_bindFunc_")),
	               ParamDecl(
	                 Declspec(SPEC_void),
	                 Declarator(Pointer(), IdentifierDecl(declstructArg))
	               )
	             )
	           ),
	           NULL,
	           Compound(binder)
	         );

	/* _kernelFunc_(void * __dev_data, void * __decl_data) */
	wrapr = FuncCallStmt(IdentName(kfuncname),IdentName(WRAPPER_ARG_NAME));
	wrapr = BlockList(wrapr, Return(NullExpr()));
	if (structdecl)
		wrapr = BlockList(/* _bindFunc_(__decl_data); */
		          Expression(
		            FunctionCall(IdentName("_bindFunc_"), IdentName("__decl_data"))
		          ),
		          wrapr
		        );
	wrapr = FuncDef(
	          Declspec(SPEC_void),
	          Declarator(
	            Pointer(),
	            FuncDecl(
	              IdentifierDecl(Symbol("_kernelFunc_")),
	              ParamList(
	                ParamDecl(
	                  Declspec(SPEC_void),
	                  Declarator(
	                    Pointer(), IdentifierDecl(Symbol(WRAPPER_ARG_NAME))
	                  )
	                ),
	                ParamDecl(
	                  Declspec(SPEC_void),
	                  Declarator(Pointer(), IdentifierDecl(declstructArg))
	                )
	              )
	            )
	          ),
	          NULL,
	          Compound(wrapr)
	        );

	return ( BlockList(binder, wrapr) );
}


#define DEVSPECit(stmt,str) ast_declordef_addspec(stmt, Usertype(Symbol(str)))


/* Wrapper for the actual _kernelFuncXX_(), specific to GPU devices
 * 
 * void * _kernelFunc_(void * __dev_data, void * __decl_data) {
 *   <body>              // Initialize struct and call actual _kernelFuncXX_() 
 * }
 */
static 
aststmt produce_gpu_wrapper(char *kfuncname, target_list_t t)
{
	astdecl dtmp;
	aststmt wrapr;
	
	dtmp = ParamDecl(
	         Speclist_right(
	           Usertype(Symbol(DEVSPEC)), 
	           Declspec(SPEC_void)
	         ), 
	         Declarator(Pointer(), IdentifierDecl(Symbol(WRAPPER_ARG_NAME)))
	       );
	if (t->gpu_wrapper_params)
		dtmp = ParamList(dtmp, t->gpu_wrapper_params);

	wrapr = FuncCallStmt(IdentName(kfuncname), t->emptyde ? NullExpr() :
	                                    UOAddress(IdentName(DEVENV_STRUCT_NAME)));
	/* Add #declare target variable bindings */
	if (!set_isempty(declare_variables))
		wrapr = BlockList(decltarg_gpu_kernel_varinits(), wrapr);
	if (t->gpu_wrapper_body)
		t->gpu_wrapper_body = BlockList(t->gpu_wrapper_body, wrapr);
	else
		t->gpu_wrapper_body = wrapr;
	wrapr = FuncDef(
	          Usertype(Symbol("__DEVKERN")), 
	          Declarator(
	            NULL, FuncDecl(IdentifierDecl(Symbol("_kernelFunc_")), dtmp)
	          ),
	          NULL,
	          Compound(t->gpu_wrapper_body)
	        );
	return wrapr;
}


/**
 * Here we produce the kernel file for each target.
 * This is called from ompi.c AFTER all the transformations have finished,
 * i.e. after ast_xform(). Thus, all outlined functions have also been
 * transformed and placed in the AST. Also, all functions have been recorded
 * at the call graph module.
 */
void produce_target_files()
{
	setelem(vars)  e;
	setelem(cgfun) caf;
	stentry        ste;
	target_list_t  t;
	FILE          *fp;
	int            globalpos;
	aststmt        declare_functions = NULL, /* Tree of declared functions code */
	               sue = copy_sue_declarations(true), kstructinit = NULL,
	               locals, tmp;

	/* VVD - here check for decltarg_ids that have no prototype/definition */

	/* First, for each kernel prepare #declare'd global variables 
	 * and possibly bind non-#declared called functions.
	 */
	for (t = Targets; t; t = t->next)
		analyze_pointerize_decltarg_varsfuncs(t->tree);

	/* Empty the 'A' scratchpad */
	A_str_truncate();

	str_printf(strA(), KERNELHEADER);

	/* If there are any global structs add them to the code */
	if (sue)
	{
		ast_stmt_print(strA(), sue);
		ast_stmt_free(sue);
	}

	/* Add any declared function prototypes */
	if (declare_funcproto && !set_isempty(declare_funcproto))
		for (e = declare_funcproto->first; e; e = e->next)
		{
			if (e->key == Symbol("devpart_med2dev_addr"))    /* Bad guy */
				tmp = verbit("%s void *devpart_med2dev_addr(%s void *,unsigned long); ",
				             DEVSPECQUAL, DEVSPEC);
			else
			{
				tmp = xform_clone_funcdecl(e->key);
				/* If it is an extern function and returns a pointer, add __DEVSPEC */
				//if (func_returnspointer(tmp->u.declaration.decl))
				//	DEVSPECit(tmp, DEVSPECQUAL);
				//else
					DEVSPECit(tmp, DEVQUAL);
			}
			ast_stmt_print(strA(), tmp);
			ast_stmt_free(tmp);
		};

	/* Prepare code for declared variables in the wrapper function */
	if (declvars_exist())
	{
		locals = decltarg_kernel_globals();
		kstructinit = decltarg_kernel_struct_code();
		ast_stmt_print(strA(), locals);
	}

	/* Remember the current position in the scratchpad */
	globalpos = A_str_tell();

	/* For each target in the 'Targets' list produce a kernel file */
	while (Targets)
	{
		t = Targets;

		if ((fp = fopen(t->kernelfile, "w")) == NULL) 
		{
			warning("[%s]: failed to create '%s'\n", __func__, t->kernelfile);
			goto CONTINUELOOP;
		}

		/* Return the scratchpad to the position of the declared stuff */
		A_str_seek(globalpos);

		/* Find and include all called functions from this kernel.
		 * Unfortunately, t->tree may not contain only the kernel function,
		 * thus _kernelFuncXX_ is going to be included in the called function
		 * list. We need to remove it by hand...
		 * We need first to output the prototypes of any extern functions and 
		 * then output the defined functions.
		 */
		for (caf = cg_find_called_funcs(t->tree)->first; caf; caf = caf->next)
			if (caf->key != Symbol(t->functionName))
			{
				if ((ste = symtab_get(stab,caf->key,FUNCNAME)) == NULL)
					continue;
				/* Declare if not already declared */
				if (ste->funcdef == NULL && !set_get(declare_funcproto, caf->key)) 
				{
					tmp = xform_clone_funcdecl(caf->key);
					DEVSPECit(tmp, DEVQUAL);
					ast_stmt_print(strA(), tmp);
				}
			};
		for (caf = cg_find_called_funcs(t->tree)->first; caf; caf = caf->next)
			if (caf->key != Symbol(t->functionName))
			{
				if ((ste = symtab_get(stab, caf->key, FUNCNAME)) == NULL) 
					continue;
				if (ste->funcdef)
				{
					/* Although the AST has been output, we cannot fiddle with the
					 * function definition since it may be needed in multiple kernels.
					 */
					aststmt funccopy = ast_stmt_copy(ste->funcdef);
					analyze_pointerize_decltarg_varsonly(funccopy);
					DEVSPECit(funccopy, DEVQUAL);
					ast_stmt_print(strA(), funccopy);
					ast_free(funccopy);
				}
			};

		/* Produce two wrapper versions: 
		 * one for gpu devices (opencl/cuda) and one for all the other devices 
		 */
		t->tree = BlockList(t->tree,
		                    verbit("#if !defined(__OMPI_CL_KERNEL__) && "
		                           "!defined(__OMPI_CUDA_KERNEL__)"));
		t->tree = BlockList(t->tree, 
		                    produce_typical_wrapper(t->functionName, kstructinit));
		t->tree = BlockList(t->tree, verbit("#else"));
		t->tree = BlockList(t->tree, 
		                    produce_gpu_wrapper(t->functionName, t));
		t->tree = BlockList(t->tree, verbit("#endif"));

		/* Add the new globals */
		if (t->newglobals)
		{
			if (cppLineNo)
				t->tree = Block4(
				            verbit("# 1 \"%s-newglobals\"", filename),
				            t->newglobals,
				            verbit("# 1 \"%s\"", filename),
				            t->tree
				          );
			else
				t->tree = BlockList(t->newglobals, t->tree);
		}

		/* Comment @ top */
		fprintf(fp, "%s\n", advert);

		/* CARS comment comes next */
		if (analyzeKernels && t->ts != NULL)
		{
			B_str_truncate();
			str_printf(strB(), "/* $OMPi__CARS:\n");
			cars_stringify_stats(strB(), t->ts);
			str_printf(strB(), "*/");
			fprintf(fp, "%s\n", B_str_string());
		}

		/* Print the target tree */
		ast_stmt_print(strA(), t->tree);
		fprintf(fp, "%s", A_str_string());
		fclose(fp);

		CONTINUELOOP:
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
	if (declvars_exist())
	{
		ast_stmt_free(kstructinit);
		ast_stmt_free(structdecl);
		structdecl = NULL;
	}
}


/**
 * Prepare the target task, if needed, based on OpenMP v45. This is needed
 * by #target, #target update, #target enter data and #target exit data.
 * We have 2 cases:
 * 1) if there is no nowait, we do not create a task but we rather emit the
 *    ultra-fast code, which only needs to create the task data environment
 *    (firstprivate); a false is returned.
 * 2) if there is a nowait clause, we have to produce a complete task,
 *    so we create a task construct with all (used) firstprivate variables
 *    and a default(shared) so no others are included; in this case true
 *    is returned to signify the need to transform the new task construct.
 *
 * From v45 #target rules (p. 105):
 * a) if and device expressions are evaluated when encoutering the #target
 * b) the target task data environment is created from the data sharing
 *    clauses and all other relevant rules; all mapped variables are
 *    considered as shared wrt to the target task.
 * c) all assignments associated to mappings, occur when the target task
 *    is executed.
 * Thus "devicestmt" must be at the very top; local copies for firstprivate
 * vars should follow and then a taskwait(0) comes to block on dependences.
 *
 * @param t         the replacement code (in place of the target construct)
 * @param devicestm the device id declaration
 * @param deps      the depend clauses
 * @param nowait    true if the construct has a nowait clause
 * @param usedvars  all used vars as calculated by the outline procedure
 * @
 */
static
bool targettask(aststmt *t, aststmt devicestm,
                ompclause deps, bool nowait, set(vars) usedvars[])
{
	if (!deps && !nowait)  /* Redundant check but better safe than sorry */
		return false;

	if (deps && !nowait)   /* Ultra-fast: just replay the task data environment */
	{
		aststmt de = NULL;

		if (usedvars && !set_isempty(usedvars[DCT_BYVALUE]))
		{
			aststmt varinits;
			de = out_inline_firstprivate(usedvars[DCT_BYVALUE], &varinits);
			if (de && varinits)
				de = BlockList(de, varinits);
		}
		if (de)
			de = BlockList(de, verbit("/* wait for target dependences */"));
		else
			de = verbit("/* wait for target dependences */");
		de = BlockList(de, FuncCallStmt(IdentName("ort_taskwait"), numConstant(0)));
		if (devicestm)
			ast_stmt_append(devicestm, de);
		*t = Compound(*t);
		return false;         /* No further processing needed */
	}

	/* Prepare for a new surrounding task construct.
	 * Clauses: dependences, default(shared) and firstprivate (incl. devid).
	 */
	deps = (deps) ? OmpClauseList(deps, DefaultClause(OC_defshared)) :
	                DefaultClause(OC_defshared);
	if (devicestm)
		deps = OmpClauseList(             /* The device id should be firstprivate */
		         deps,
		         VarlistClause(OCFIRSTPRIVATE,
		           IdentifierDecl( Symbol(currdevvarName) ))
		       );
	if (usedvars && !set_isempty(usedvars[DCT_BYVALUE]))
		deps = OmpClauseList(deps,
		         VarlistClause(OCFIRSTPRIVATE,
		           ast_set2varlist(usedvars[DCT_BYVALUE]))
		       );
	*t = OmpStmt( OmpConstruct(DCTASK, OmpDirective(DCTASK,deps), Compound(*t)) );
	return true;
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
	           Declarator(NULL, IdentifierDecl(Symbol(currdevvarName))),
	           /* If there is an if clause and it evaluates to false use
	            * device 0 (host) */
	           (ifexpr) ?
	           ConditionalExpr(
	             Parenthesis(ifexpr),
	             Parenthesis(deviceexpr),
	             numConstant(HOSTDEV_ID)
	           ) :
	           Parenthesis(deviceexpr)
	         )
	       );
}


/**
 * Generates the code for creating and destroying a device data environment.
 * It wraps statement t with mapping and unmpapping calls for normal and
 * link variables. Called when transforming #target and #targetdata.
 * @param t         The statment we want to wrap
 * @param construct The OpenMP construct (just to get the map clauses)
 * @param usedvars  All sets of used variable categories
 * @param implink   A set with implicitly used link variables
 * @param devexpr   A expression with the device id
 */
static
void create_devdata_env(aststmt *t, ompcon construct,
         set(vars) *usedvars, set(vars) implink, astexpr devexpr)
{
	aststmt before, after, tmp;
	int     nvars;

	if (usedvars != NULL)  /* start VVD-new-{ */
	{
	/* (1) Map/unmap all vars (except link ones) */
	if ((nvars = xm_usedvars_mappings(usedvars, &before, &after)) == 0)
		ast_stmt_free(get_denv_var_decl(false)); /* no vars => no device data env */
	else
	{
		/* Mark start/end of target data:
		 * We replace the initializer of the __ort_denv variable that was stored in
		 * get_denv_var_decl())
		 *   0 -> ort_start_target_data(numberofvars, deviceexpr)
		 */
		tmp = get_denv_var_decl(false);           /* Get the variable declaration */
		free(tmp->u.declaration.decl->u.expr);       /* Free the "numConstant(0)" */
		tmp->u.declaration.decl->u.expr =                   /* Place the new call */
			FunctionCall(
				IdentName("ort_start_target_data"),
				CommaList(numConstant(nvars),ast_expr_copy(devexpr))
			);
		before = (before) ? BlockList(tmp, before) : tmp;

		//ort_end_target_data(__ort_denv);
		tmp = FuncCallStmt(IdentName("ort_end_target_data"),IdentName("__ort_denv"));
		after = (after) ? BlockList(after, tmp) : tmp;

		*t = BlockList(before, *t);  /* Insert mappings */
		*t = BlockList(*t , after);  /* Insert unmappings */
	}
	}
	
	/* (2) Map/unmap link vars (if any) */
	xm_linkvars_mappings(devexpr, construct, implink, &before, &after);
	if (before)
		*t = BlockList(before, *t);
	if (after)
		*t = BlockList(*t , after);
}


static
void use_device_pointers(aststmt *t, set(vars) ptrvars, astexpr devexpr)
{
	setelem(vars) v;
	astdecl udpdecl, id;
	aststmt declstmt;
	stentry orig;
	char    tmpname[256];
	
	if (set_isempty(ptrvars))
		return;
	/* Add device pointers and pointerize use_device_ptr vars
	 */
	analyze_rename_vars(*t, ptrvars, "_udp_");
	for (v = ptrvars->first; v; v = v->next)
	{
		/* Sanity check */
		orig = symtab_get(stab, v->key, IDNAME);
		if (!orig->isarray && !(orig->decl && decl_ispointer(orig->decl)))
			exit_error(1, "(%s, line %d) openmp error:\n\t"
		            "use_device_ptr() variable '%s' is neither pointer nor array\n",
		            (*t)->file->name, (*t)->l, v->key->name);

		/* Now do it */
		snprintf(tmpname, 255, "_udp_%s", v->key->name);          /* The new var */
		udpdecl = ast_decl_copy(orig->decl);                   /* Clone the decl */
		id = IdentifierDecl(Symbol(tmpname));                 /* Change the name */
		*(decl_getidentifier(udpdecl)) = *id;
		free(id);
		if (orig->isarray)
			decl_arr2ptr(udpdecl);                     /* Reduce to simple pointer */
		udpdecl = InitDecl(                          /* Declare with initializer */
		            udpdecl,
		            FunctionCall(
		              IdentName("ort_host2med_addr"),
		              Comma2(
		                UOAddress( Identifier(v->key) ),
		                ast_expr_copy(devexpr)
		              )
		            )
		          );
		declstmt = Declaration(ast_spec_copy_nosc(orig->spec), udpdecl);
		*t = BlockList(declstmt, *t);
	}
}


void xform_targetdata(aststmt *t)
{
	astexpr   deviceexpr = NULL, ifexpr = NULL;
	aststmt   s = (*t)->u.omp->body, parent = (*t)->parent, tmp, v, devicestm;
	ompclause c;

	/* (1) Find used variables; add explicitly mapped declare-target link vars too
	 */
	set(vars) *usedvars = analyze_used_vars(*t);

	/* (2) Check for device and if clauses and keep a copy
	 */
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCDEVICE)) != NULL)
		deviceexpr = ast_expr_copy(c->u.expr);
	else
		deviceexpr = numConstant(AUTODEV_ID);
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCIF)) != NULL)
		ifexpr = ast_expr_copy(c->u.expr);

	/* (3) Store device id in a variable to avoid re-evaluating the expression
	 */
	devicestm = device_statement(ifexpr, deviceexpr);
	deviceexpr = IdentName(currdevvarName);

	/* (4) Create the code for the data environment */
		/* Force all clause variables to enter the sets. */
	xc_ompcon_get_vars((*t)->u.omp, OCMAP, OC_alloc, usedvars[DCT_MAPALLOC]);
	xc_ompcon_get_vars((*t)->u.omp, OCMAP, OC_to, usedvars[DCT_MAPTO]);
	xc_ompcon_get_vars((*t)->u.omp, OCMAP, OC_tofrom, usedvars[DCT_MAPTOFROM]);
	xc_ompcon_get_vars((*t)->u.omp, OCMAP, OC_from, usedvars[DCT_MAPFROM]);
#if 0
	/* Well the following should be enough; there should be no need to 
	 * analyze_used_vars since we do not outline. However for some reason
	 * the following does not work - I have the impression that something
	 * is fishy with the runtime and the data environment hierarchy (because
	 * the following calls map in the global environment).
	 */
	/* VVD-new-{ */
	/* (4) Create the code for the data environment */
		/* The following generally works BUT fails if a var is an array section */
	xc_ompcon_get_vars((*t)->u.omp, OCMAP, OC_alloc, usedvars[DCT_MAPALLOC]);
	xc_ompcon_get_vars((*t)->u.omp, OCMAP, OC_to, usedvars[DCT_MAPTO]);
	xc_ompcon_get_vars((*t)->u.omp, OCMAP, OC_tofrom, usedvars[DCT_MAPTOFROM]);
	xc_ompcon_get_vars((*t)->u.omp, OCMAP, OC_from, usedvars[DCT_MAPFROM]);

		/* The following does not work in some cases and I cannot understand why */
	aststmt maps = NULL, unmaps = NULL;
	set(xlitems) mapvars = set_new(xlitems);
	xc_ompcon_get_xlitems((*t)->u.omp, OCMAP, OC_alloc, mapvars);
	if (!set_isempty(mapvars))
	{
		xm_mup_xliset(mapvars, &maps, deviceexpr, xm_map_xlitem, UPDATE_DISABLE, 
		           "/* map alloc */");
		xm_mup_xliset(mapvars, &unmaps, deviceexpr, xm_unmap_xlitem, UPDATE_DISABLE, 
		           "/* unmap alloc */");
		set_drain(mapvars);
	}
	xc_ompcon_get_xlitems((*t)->u.omp, OCMAP, OC_to, mapvars);
	if (!set_isempty(mapvars))
	{
		xm_mup_xliset(mapvars, &maps, deviceexpr, xm_map_xlitem, UPDATE_NORMAL, 
		           "/* map to */");
		xm_mup_xliset(mapvars, &unmaps, deviceexpr, xm_unmap_xlitem, UPDATE_DISABLE, 
		           "/* unmap to */");
		set_drain(mapvars);
	}
	xc_ompcon_get_xlitems((*t)->u.omp, OCMAP, OC_tofrom, mapvars);
	if (!set_isempty(mapvars))
	{
		xm_mup_xliset(mapvars, &maps, deviceexpr, xm_map_xlitem, UPDATE_NORMAL, 
		           "/* map tofrom */");
		xm_mup_xliset(mapvars, &unmaps, deviceexpr, xm_unmap_xlitem, UPDATE_NORMAL,
		           "/* unmap tofrom */");
		set_drain(mapvars);
	}
	xc_ompcon_get_xlitems((*t)->u.omp, OCMAP, OC_from, mapvars);
	if (!set_isempty(mapvars))
	{
		xm_mup_xliset(mapvars, &maps, deviceexpr, xm_map_xlitem, UPDATE_DISABLE, 
		           "/* map from */");
		xm_mup_xliset(mapvars, &unmaps, deviceexpr, xm_unmap_xlitem, UPDATE_NORMAL,
		           "/* unmap from */");
		set_drain(mapvars);
	}
	/* }-VVD-new */
#endif

	/* (4a) Comment the directive
	 */
	/* Create a comment containing the directive */
	v = ompdir_commented((*t)->u.omp->directive);

	tmp = *t;                   /* So that we can get rid of the OmpStmt later */
	*t = s;

	/* (5) Create the code for the data environment
	 */
#ifdef DEVENV_DBG
	fprintf(stderr, "[target data env]:\n");
	ast_ompdir_show_stderr(tmp->u.omp->directive);
#endif

#if 0
	/* VVD-new-{ */
	if (maps)
		*t = BlockList(maps, *t);  /* Insert mappings */
	if (unmaps)
		*t = BlockList(*t , unmaps);  /* Insert unmappings */
	/* }-VVD-new */
#endif

	create_devdata_env(t, tmp->u.omp, usedvars, NULL, deviceexpr);
	use_device_pointers(t, usedvars[DCT_BYREF], deviceexpr);

	/* (4b) Now that clause xlitems were used, get rid of the OmpStmt */
	tmp->u.omp->body = NULL;    /* Make it NULL so as to free it easily */
	ast_free(tmp);              /* Get rid of the OmpStmt */

	/* (6) Add extra code
	 */
	*t = BlockList(devicestm, *t);
	*t = Compound(*t);   // Wrap with {} to place the new variables on a new scope
	*t = BlockList(v, *t);  // Add the commented directive

	/* (7) Parentize new statements
	 */
	ast_stmt_parent(parent, *t);

	/* (8) Free the original deviceexpr since data_mem_operation creates copies.
	 */
	ast_expr_free(deviceexpr);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     TARGET ENTER/EXIT DATA                                    *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void xform_targetenterdata(aststmt *t)
{
	astexpr      deviceexpr = NULL, ifexpr = NULL;
	ompclause    c, deps;
	aststmt      v, parent = (*t)->parent, devicestm = NULL, xfers = NULL;
	bool         nowait = false, xformtask = false;
	set(xlitems) mapvars = set_new(xlitems);

#ifdef DEVENV_DBG
	fprintf(stderr, "[target enter data]:\n");
	ast_ompdir_show_stderr((*t)->u.omp->directive);
#endif
	/* (1) Comment the directive */
	v = ompdir_commented((*t)->u.omp->directive);

	/* (2) Check for device and if clauses and keep a copy */
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCDEVICE)) != NULL)
		deviceexpr = ast_expr_copy(c->u.expr);
	else
		deviceexpr = numConstant(AUTODEV_ID);
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCIF)) != NULL)
		ifexpr = ast_expr_copy(c->u.expr);
	if (!xar_expr_is_constant(deviceexpr) || ifexpr)
	{
		/* Store device id in a variable to avoid re-evaluating the expression */
		devicestm = device_statement(ifexpr, deviceexpr);
		deviceexpr = IdentName(currdevvarName);
	}
	deps = xc_ompcon_get_every_clause((*t)->u.omp, OCDEPEND);
	nowait = (xc_ompcon_get_unique_clause((*t)->u.omp, OCNOWAIT) != NULL);

	/* (3) Create the code for the data environment	 */
	xc_ompcon_get_xlitems((*t)->u.omp, OCMAP, OC_to, mapvars);
	if (!set_isempty(mapvars))
	{
		xm_mup_xliset(mapvars, &xfers, deviceexpr, xm_map_xlitem, UPDATE_NORMAL, 
		           "/* enter-to */");
		set_drain(mapvars);
	}
	xc_ompcon_get_xlitems((*t)->u.omp, OCMAP, OC_alloc, mapvars);
	if (!set_isempty(mapvars))
	{
		xm_mup_xliset(mapvars, &xfers, deviceexpr, xm_map_xlitem, UPDATE_DISABLE, 
		           "/* enter-alloc */");
		set_drain(mapvars);
	}

	/* (4) Replace */
	ast_free(*t);
	ast_expr_free(deviceexpr); /* since data_mem_operation creates copies */
	*t = xfers ? xfers : Expression(NULL);

	/* (6) Add extra code */
	if (deps || nowait)
		xformtask = targettask(t, devicestm, deps, nowait, NULL);
	if (devicestm)
		*t = BlockList(devicestm, *t);
	*t = Compound(*t);
	*t = BlockList(v, *t);  /* Add the commented directive */

	/* (7) Parentize */
	ast_stmt_parent(parent, *t);

	/* (8) Possibly produce a task */
	if (xformtask)
	{
		taskopt_e bak = taskoptLevel;

		taskoptLevel = OPT_NONE;
		ast_stmt_xform(t);
		taskoptLevel = bak;
	}
}


void xform_targetexitdata(aststmt *t)
{
	astexpr   deviceexpr = NULL, ifexpr = NULL;
	ompclause c, deps;
	aststmt   v, parent = (*t)->parent, devicestm = NULL, xfers = NULL;
	bool      nowait = false, xformtask = false;
	set(xlitems) mapvars = set_new(xlitems);

#ifdef DEVENV_DBG
	fprintf(stderr, "[target exit data]:\n");
	ast_ompdir_show_stderr((*t)->u.omp->directive);
#endif
	/* (1) Comment the directive */
	v = ompdir_commented((*t)->u.omp->directive);

	/* (2) Check for device and if clauses and keep a copy */
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCDEVICE)) != NULL)
		deviceexpr = ast_expr_copy(c->u.expr);
	else
		deviceexpr = numConstant(AUTODEV_ID);
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCIF)) != NULL)
		ifexpr = ast_expr_copy(c->u.expr);
	if (!xar_expr_is_constant(deviceexpr) || ifexpr)
	{
		/* Store device id in a variable to avoid re-evaluating the expression */
		devicestm = device_statement(ifexpr, deviceexpr);
		deviceexpr = IdentName(currdevvarName);
	}
	deps = xc_ompcon_get_every_clause((*t)->u.omp, OCDEPEND);
	nowait = (xc_ompcon_get_unique_clause((*t)->u.omp, OCNOWAIT) != NULL);

	/* (3) Create the code for the data environment	 */
	xc_ompcon_get_xlitems((*t)->u.omp, OCMAP, OC_from, mapvars);
	if (!set_isempty(mapvars))
	{
		xm_mup_xliset(mapvars, &xfers, deviceexpr, xm_unmap_xlitem, 
		              UPDATE_NORMAL, "/* exit-from */");
		set_drain(mapvars);
	}
	xc_ompcon_get_xlitems((*t)->u.omp, OCMAP, OC_release, mapvars);
	if (!set_isempty(mapvars))
	{
		xm_mup_xliset(mapvars, &xfers, deviceexpr, xm_unmap_xlitem, 
		              UPDATE_DISABLE, "/* exit-release */");
		set_drain(mapvars);
	}
	xc_ompcon_get_xlitems((*t)->u.omp, OCMAP, OC_delete, mapvars);
	if (!set_isempty(mapvars))
	{
		xm_mup_xliset(mapvars, &xfers, deviceexpr, xm_unmap_xlitem, 
		              REFER_DELETE, "/* exit-delete */");
		set_drain(mapvars);
	}

	/* (4) Replace */
	ast_free(*t);
	ast_expr_free(deviceexpr); /* since data_mem_operation creates copies */
	*t = xfers ? xfers : Expression(NULL);

	/* (6) Add extra code */
	if (deps || nowait)
		xformtask = targettask(t, devicestm, deps, nowait, NULL);
	if (devicestm)
		*t = BlockList(devicestm, *t);
	*t = Compound(*t);
	*t = BlockList(v, *t);  /* Add the commented directive */

	/* (7) Parentize */
	ast_stmt_parent(parent, *t);

	/* (8) Possibly produce a task */
	if (xformtask)
	{
		taskopt_e bak = taskoptLevel;

		taskoptLevel = OPT_NONE;
		ast_stmt_xform(t);
		taskoptLevel = bak;
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     TARGET UPDATE                                             *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


char *currdevvarName = "__ompi_devID";

#define FUNCALL3(f,p1,p2,p3) FuncCallStmt(IdentName(f),\
	Comma3((p1), (p2), (p3)))
#define FUNCALL4(f,p1,p2,p3,p4) FuncCallStmt(IdentName(f),\
	Comma4((p1), (p2), (p3), (p4)))


/**
 * Iterates over a set of xlitems "s" and creates a function call to
 * "funcname" for manipulating the variable on "device". The generated
 * code is returned through "stmt"
 * Called from xform_targetupdate().
 * @param s         the set of xlitems
 * @param stmt      (ret) the statement with all transfers
 * @param devexpr   an expression with the device id
 * @param funcname  the function to call to do the transfers
 * @param comment   a comment to preceed the transfers
 * @return The number of processed items.
 */
static
int update_set(set(xlitems) s,
               aststmt *stmt, astexpr devexpr, char *funcname, char *comment)
{
	int     n = 0;
	astexpr nbytes, itemaddr, addrlb;
	aststmt st;
	setelem(xlitems) e;

	for (e = s->first; e; e = e->next)
	{
		xc_xlitem_copy_info(e->value.xl, &itemaddr, &nbytes, &addrlb);
		st = FUNCALL4(funcname, itemaddr, nbytes, addrlb, ast_expr_copy(devexpr));

		if (n == 0)   /* Comment */
			*stmt = (*stmt == NULL) ?
			           verbit(comment) : BlockList(*stmt, verbit(comment));
		*stmt = BlockList(*stmt, st);
		n++;
	}
	return n;
}


void xform_targetupdate(aststmt *t)
{
	static set(xlitems) to = NULL, from = NULL, unique = NULL;
	setelem(xlitems)    se;
	astexpr   deviceexpr = NULL, ifexpr = NULL;
	ompclause c, deps;
	aststmt   v, parent = (*t)->parent, st = NULL, devicestm = NULL;
	bool      nowait = false, xformtask = false;
	char      *filename = (*t)->u.omp->directive->file->name;
	int       line = (*t)->u.omp->directive->l;

	set_init(xlitems, &to);
	set_init(xlitems, &from);
	set_init(xlitems, &unique);

#ifdef DEVENV_DBG
	fprintf(stderr, "[target update]:\n");
	ast_ompdir_show_stderr((*t)->u.omp->directive);
#endif
	
	/* (1) Check for device and if clauses and keep a copy
	 */
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCDEVICE)) != NULL)
		deviceexpr = ast_expr_copy(c->u.expr);
	else
		deviceexpr = numConstant(AUTODEV_ID);
	if (!xar_expr_is_constant(deviceexpr))
	{
		devicestm = device_statement(NULL, deviceexpr);
		deviceexpr = IdentName(currdevvarName);
	}
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCIF)) != NULL)
		ifexpr = ast_expr_copy(c->u.expr);

	/* (2) Get new (v45) clauses
	 */
	deps = xc_ompcon_get_every_clause((*t)->u.omp, OCDEPEND);
	nowait = (xc_ompcon_get_unique_clause((*t)->u.omp, OCNOWAIT) != NULL);

	/* (3) Get the items in to/from clauses
	 */
	xc_ompcon_get_xlitems((*t)->u.omp, OCTO, OC_DontCare, to);
	xc_ompcon_get_xlitems((*t)->u.omp, OCFROM, OC_DontCare, from);

	/* (4) Check if there was at least one motion clause
	 */
	if (set_isempty(to) && set_isempty(from))
		exit_error(1, "(%s, line %d) openmp error:\n\t"
		           "target update construct needs at least one to/from clause\n",
		           filename, line);

	/* (5) Check if the items in to/from clauses appear more than once
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

	/* (6) Write function calls for transferring to/from the device
	 */
	update_set(to, &st, deviceexpr, "ort_write_var_dev", "/* to */");
	update_set(from, &st, deviceexpr, "ort_read_var_dev", "/* from */");

	/* (7) Comment the directive
	 */
	v = ompdir_commented((*t)->u.omp->directive);
	ast_free(*t);                  /* Get rid of the OmpStmt */
	ast_expr_free(deviceexpr);     /* since data_mem_operation creates copies */
	*t = v;                        /* Replace the directive with the comment */

	/* (8) Add extra code
	 */
	if (deps || nowait)
		xformtask = targettask(&st, devicestm, deps, nowait, NULL);
	if (devicestm)
		st = BlockList(devicestm, st);
	st = Compound(st);

	/* (9) Add if clause
	 */
	if (ifexpr)                    /* Check if we have an if() clause */
		st = If(ifexpr, st, NULL);
	*t = BlockList(*t, st);

	/* (10) Parentize new statements
	 */
	ast_stmt_parent(parent, *t);

	/* (11) Possibly produce a task
	 */
	if (xformtask)
	{
		taskopt_e bak = taskoptLevel;

		taskoptLevel = OPT_NONE;
		ast_stmt_xform(t);
		taskoptLevel = bak;
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     TARGET                                                    *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* If true all variables get added in the device data environment, else only
 * the ones that already are in a device data environment are treated as "DDENV"
 *   -- if true, #target map variables all enter the d.d.env. and
 *      the struct passed to the kernel will have ONLY pointers;
 *      if false, scalars get passed directly within the struct (VVD)
 */
const bool allvarsindevenv = true;

/* Quick but ugly flag to remember whether there is a defaultmap clause
 * in the current #target construct (it works since no nesting is allowed for
 * target constructs.
 * The best would be to have all *implicit*() functions accept a second
 * argument
 */
static bool hasdefaultmap = false;


/**
 * Given a variable symbol, it produces the corresponding offset symbol
 * to be used in data environment structs.
 * @param var The data var symbol
 * @return    The corresponding symbol for the offset field name
 */
symbol targstruct_offsetname(symbol var)
{
	static char name[256];

	sprintf(name, "_%s_offset", var->name);
	return ( Symbol(name) );
}


char *strdupquoted(char *s)
{
	char *t = (char *) smalloc(strlen(s) + 3);

	sprintf(t, "\"%s\"", s);
	return (t);
}


/**
 * Called for each variable in the DCT_UNSPECIFIED set to implicitly determine
 * its mapping attributes. All #declare target vars are ignored.
 * The rules of OpenMP 4.5 are obeyed.
 * @return The decided mapping attribute (i.e. the corresponding set to join).
 */
static vartype_t xtarget_implicitDefault(setelem(vars) s)
{
	stentry orig = symtab_get(stab, s->key, IDNAME);

	if (orig->isindevenv == due2DECLTARG)
		return DCT_IGNORE;
	if (orig->isindevenv == due2TARGDATA)
		return DCT_DDENV;
	if (hasdefaultmap)
		return DCT_MAPTOFROM;   /* all treated as map(tofrom:) */
	if (orig->isarray)        /* arrays treated as tofrom */
		return DCT_MAPTOFROM;
	if (decl_ispointer(orig->decl))
		return DCT_ZLAS;        /* zero-length pointer-based array section */
	else
		return DCT_BYVALUE;     /* scalars treated as firstprivate */
}


/*
 * Gets all is_device_ptr() vars and checks if they are ptrs/arrays indeed.
 * One day, this would handle such vars on its own but for now we just
 * check for validity; we let outline handle them as firstprivate.
 */
void get_and_check_device_ptrs(ompcon t, set(vars) s)
{
	setelem(vars) e;
	stentry orig;
	
	xc_ompcon_get_vars(t, OCISDEVPTR, OC_DontCare, s);
	for (e = s->first; e; e = e->next)
	{
		orig = symtab_get(stab, e->key, IDNAME);
		if (!orig->isarray && !(orig->decl && decl_ispointer(orig->decl)))
			exit_error(1, "(%s, line %d) openmp error:\n\t"
		            "is_device_ptr() variable '%s' is neither pointer nor array\n",
		            t->file->name, t->l, e->key->name);
		if (orig->isarray)
			/* because we implement it as firstprivate... */
			exit_error(1, "(%s, line %d) openmp error:\n\t"
		            "array is_device_ptr() variables not supported\n",
		            t->file->name, t->l);
	}
}


/* Add argument to variadic call to ort_offload_kernel() */
static void _add_ortarg(stentry e, void *arg)
{
	astexpr arglist = (astexpr) arg;

	arglist->right = Comma3(
	                   arglist->right,
	                   PtrField(IdentName(DEVENV_STRUCT_NAME), e->key),
	                   PtrField(IdentName(DEVENV_STRUCT_NAME),
	                            targstruct_offsetname(e->key))
	                 );
}


/* Add argument to variadic call to ort_offload_kernel() - for firstprivates */
static void _add_ortarg_fip(stentry e, void *arg)
{
	astexpr arglist = (astexpr) arg;

	arglist->right = CommaList(
	                   arglist->right,
	                   PtrField(IdentName(DEVENV_STRUCT_NAME), e->key)
	                 );
}


/* Add kernel wrapper parameter (no offset) for firstprivates */
static void _add_wrapr_param_fip(stentry e, void *arg)
{
	struct { astdecl *params; set(vars) devptrs; } *fipparm = arg;
	astdecl *params = fipparm->params, tmp;

	tmp = ParamDecl(
	        Speclist_right(
	          Usertype(Symbol(DEVSPEC)),
	          ast_spec_copy_nosc(e->spec)
	        ), 
	        set_get(fipparm->devptrs, e->key) ?
	          ast_decl_copy(e->decl) :
	          decl_topointer(ast_decl_copy(e->decl))
	      );
	*params = *params ? ParamList(*params, tmp) : tmp;
}


/* Add kernel wrapper parameter and corresponding offset */
static void _add_wrapr_param(stentry e, void *arg)
{
	astdecl *params = (astdecl *) arg, tmp;

	tmp = ParamDecl(
	        Speclist_right(
	          Usertype(Symbol(DEVSPEC)),
	          ast_spec_copy_nosc(e->spec)
	        ), 
	        decl_topointer(ast_decl_copy(e->decl))
	      );
	*params = *params ? ParamList(*params, tmp) : tmp;
	
	tmp = ParamDecl(
	        Speclist_right(Declspec(SPEC_unsigned), Declspec(SPEC_long)),
	        Declarator(NULL, IdentifierDecl(targstruct_offsetname(e->key)))
	      );
	*params = ParamList(*params, tmp);
}


/* Add kernel wrapper struct field initializer */
static void _add_wrapr_initer_fip(stentry e, void *arg)
{
	aststmt *initer = (aststmt *) arg;

	*initer =
	  BlockList(
	    *initer,      /* it is non-NULL for sure */
	    AssignStmt(
	      DotField(IdentName(DEVENV_STRUCT_NAME), e->key),
	      Identifier(e->key)
	    )
	  );
}


/* Add kernel wrapper struct field initializer */
static void _add_wrapr_initer(stentry e, void *arg)
{
	aststmt *initer = (aststmt *) arg;

	*initer =
	  Block3(
	    *initer,      /* it is non-NULL for sure */
	    AssignStmt(
	      DotField(IdentName(DEVENV_STRUCT_NAME), e->key),
	      Identifier(e->key)
	    ),
	    AssignStmt(
	      DotField(IdentName(DEVENV_STRUCT_NAME),targstruct_offsetname(e->key)),
	      Identifier(targstruct_offsetname(e->key))
	    )
	  );
}


/* Iterate over the kernel variables of the given set */
static 
void gpu_structvars_iter(set(vars) usedvars, 
                         void (*func)(stentry,void*), void *funcarg)
{
	setelem(vars) e;
	stentry orig;
	
	for (e = usedvars->first; e; e = e->next)
	{
		orig = symtab_get(stab, e->key, IDNAME);
		if (!allvarsindevenv && !orig->isindevenv)
			continue;
		(*func)(orig, funcarg);
	}
}


static void gpuize_struct_fields(astdecl f)
{
	if (f->type == DLIST)
	{
		gpuize_struct_fields(f->decl);
		gpuize_struct_fields(f->u.next);
	}
	else
	{
		assert(f->type == DSTRUCTFIELD);
		if (decl_ispointer(f))
			f->spec = Speclist_right(Usertype(Symbol(DEVSPEC)), f->spec);
	}
}


/* 1) The first nfip fields are turned into pointers (first private vars)
 * 2) All pointer fields get prepended by a DEVSPEC specifier
 */
static aststmt gpuize_struct(aststmt s, int numfips)
{
	int i=0;
	
	assert(s->type == DECLARATION);
	assert(s->u.declaration.spec->type == SUE);
	assert(s->u.declaration.spec->subtype == SPEC_struct);
	gpuize_struct_fields(s->u.declaration.spec->u.decl);
	return s;
}


static void prepare_gpu_ortargs(set(vars) *usedvars, astexpr commalist)
{
	commalist->right = Comma2(
	                     commalist->right,   /* # firstprivate vars */
	                     numConstant(set_size(usedvars[DCT_BYVALUE]))
	                   );
	gpu_structvars_iter(usedvars[DCT_BYVALUE],   _add_ortarg_fip, commalist);
	gpu_structvars_iter(usedvars[DCT_MAPALLOC],  _add_ortarg, commalist);
	gpu_structvars_iter(usedvars[DCT_MAPTO],     _add_ortarg, commalist);
	gpu_structvars_iter(usedvars[DCT_MAPFROM],   _add_ortarg, commalist);
	gpu_structvars_iter(usedvars[DCT_MAPTOFROM], _add_ortarg, commalist);
	gpu_structvars_iter(usedvars[DCT_ZLAS],      _add_ortarg, commalist);
	gpu_structvars_iter(usedvars[DCT_DDENV],     _add_ortarg, commalist);
}


static void prepare_gpu_wrapperparams(set(vars) *usedvars, astdecl *params,
                                      set(vars) devptrs)
{
	struct { astdecl *params; set(vars) devptrs; } fipparm = { params, devptrs };
	gpu_structvars_iter(usedvars[DCT_BYVALUE],   _add_wrapr_param_fip, &fipparm);
	gpu_structvars_iter(usedvars[DCT_MAPALLOC],  _add_wrapr_param, params);
	gpu_structvars_iter(usedvars[DCT_MAPTO],     _add_wrapr_param, params);
	gpu_structvars_iter(usedvars[DCT_MAPFROM],   _add_wrapr_param, params);
	gpu_structvars_iter(usedvars[DCT_MAPTOFROM], _add_wrapr_param, params);
	gpu_structvars_iter(usedvars[DCT_ZLAS],      _add_wrapr_param, params);
	gpu_structvars_iter(usedvars[DCT_DDENV],     _add_wrapr_param, params);
}


static void prepare_gpu_wrapperinits(set(vars) *usedvars, aststmt *inits)
{
	gpu_structvars_iter(usedvars[DCT_BYVALUE],   _add_wrapr_initer_fip, inits);
	gpu_structvars_iter(usedvars[DCT_MAPALLOC],  _add_wrapr_initer, inits);
	gpu_structvars_iter(usedvars[DCT_MAPTO],     _add_wrapr_initer, inits);
	gpu_structvars_iter(usedvars[DCT_MAPFROM],   _add_wrapr_initer, inits);
	gpu_structvars_iter(usedvars[DCT_MAPTOFROM], _add_wrapr_initer, inits);
	gpu_structvars_iter(usedvars[DCT_ZLAS],      _add_wrapr_initer, inits);
	gpu_structvars_iter(usedvars[DCT_DDENV],     _add_wrapr_initer, inits);
}


static 
void prepare_gpu_wrapper(astexpr commalist, set(vars) *usedvars, 
       aststmt rep_struct, char *structType, set(vars) devptrs, target_list_t t)
{
	if (rep_struct)
	{
		t->gpu_wrapper_body = gpuize_struct(ast_copy(rep_struct), 
		                                    set_size(usedvars[DCT_BYVALUE]));
		/* Not a pointer */
		ast_spec_free(t->gpu_wrapper_body->u.declaration.decl->spec);
		t->gpu_wrapper_body->u.declaration.decl->spec = NULL;
	}
	else
		t->gpu_wrapper_body = verbit("/* no struct */");
	t->gpu_wrapper_params = NULL;
	t->emptyde = (rep_struct == NULL);

	commalist->right = CommaList(commalist->right, decltarg_offload_arguments());
	
	prepare_gpu_ortargs(usedvars, commalist);
	prepare_gpu_wrapperparams(usedvars, &t->gpu_wrapper_params, devptrs);
	prepare_gpu_wrapperinits(usedvars, &t->gpu_wrapper_body);
	
	/* Pass #declare variable pointers as wrapper params */
	if (!set_isempty(declare_variables))
	{
		if (t->gpu_wrapper_params)
			t->gpu_wrapper_params = ParamList(decltarg_gpu_kernel_parameters(), 
			                                  t->gpu_wrapper_params);
		else
			t->gpu_wrapper_params = decltarg_gpu_kernel_parameters();
	}
	
	/* Pass a null to mark the end of the variable list */
	commalist->right = CommaList(commalist->right, NullExpr());
}


void xform_target(aststmt *t, targstats_t *ts)
{
	astexpr    deviceexpr = NULL, ifexpr = NULL;
	aststmt    devicestm = NULL, *producedc, repstruct_pin, parent = (*t)->parent;
	ompclause  c, deps;
	outcome_t  oc;
	char      *basename;
	bool       nowait = false, xformtask = false;
	target_list_t newtarget;
	set(vars)  devptrs = set_new(vars);
	setelem(cgfun) caf;
	stentry    e;

	/* 1) Preparations
	 */
	newtarget = (target_list_t) smalloc(sizeof(struct target_list_));
	newtarget->kernelfile = (char *) smalloc((strlen(filename)+7) * sizeof(char));
	snprintf(newtarget->kernelfile, (strlen(filename) + 5), "%.*s_d%02d",
	         (int)(strlen(filename) - 3), filename, targetnum);
	A_str_truncate();
	str_printf(strA(), "\"%s\"", newtarget->kernelfile);
	basename = strdupquoted(newtarget->kernelfile);
	strcat(newtarget->kernelfile, ".c");
	newtarget->ts = ts;                      /* For CARS */
	newtarget->calledfuncs = set_new(cgfun);

	/* Mark and store all the called functions.
	 * Notice that if e.g. the kernel has a #parallel or #task, then the outlined 
	 * function is not directly called from the kernel and thus it is not inclued
	 * here. However, it was included in the global symbol table by outline.c
	 */
	for (caf = cg_find_called_funcs(*t)->first; caf != NULL; caf = caf->next)
	{
		decltarg_add_calledfunc(caf->key);
		if ((e = symtab_get(stab, caf->key, FUNCNAME)) != NULL)
		{
			decltarg_bind_id(e);   /* Do it now in case it was previously analyzed */
			set_put(newtarget->calledfuncs, caf->key);
		}
	}
	
	/* 2) Check for device, if and other clauses
	 */
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCDEVICE)) != NULL)
		deviceexpr = ast_expr_copy(c->u.expr);
	else
		deviceexpr = numConstant(AUTODEV_ID);
	if ((c = xc_ompcon_get_unique_clause((*t)->u.omp, OCIF)) != NULL)
		ifexpr = ast_expr_copy(c->u.expr);
	hasdefaultmap =
		(xc_ompcon_get_unique_clause((*t)->u.omp, OCDEFAULTMAP) != NULL);
	deps = xc_ompcon_get_every_clause((*t)->u.omp, OCDEPEND);
	nowait = (xc_ompcon_get_unique_clause((*t)->u.omp, OCNOWAIT) != NULL);
	get_and_check_device_ptrs((*t)->u.omp, devptrs);

	/* 3) Store device id in a variable to avoid re-evaluating the expression
	 */
	devicestm = device_statement(ifexpr, deviceexpr);
	deviceexpr = IdentName(currdevvarName);

	/* 4) Outline
	 */
	static outpars_t op =
	{
		true,                    // structbased
		"test",                  //+functionName
		"ort_offload_kernel",    // functionCall
		NULL,                    //+extraParameters
		BYVAL_bycopy,            // byvalue_type (by copy)
		true,                    // global_byref_in_struct
		"__dev_struct",          // structType
		DEVENV_STRUCT_NAME,      // structName
		NULL,                    // structInitializer
		xtarget_implicitDefault, // implicitDefault function
		NULL,                    //+deviceexpr
		true,                    // addComment
		NULL                     // thestmt
	};
	sprintf(op.functionName, "_kernelFunc%d_", targetnum++);

	/* The NULL is replaced later with the declared variables struct */
	newtarget->decl_struct = NullExpr();

	//(void *) 0, "<kernelfilename>", <deviceexpr>
	op.extraParameters = Comma3(newtarget->decl_struct,
	                            IdentName(A_str_string()),
	                            deviceexpr);
	//(struct __dev_struct *) ort_devdata_alloc(sizeof(struct __dev_struct), <deviceexpr>)
	op.structInitializer =
	  CastedExpr(
	    Casttypename(
	      SUdecl(SPEC_struct, Symbol(op.structType), NULL, NULL),
	      AbstractDeclarator(Pointer(), NULL)
	    ),
	    FunctionCall(
	      IdentName("ort_devdata_alloc"),
	      CommaList(
	        Sizeoftype(
	          Casttypename(
	            SUdecl(SPEC_struct, Symbol(op.structType), NULL, NULL),
	            NULL
	          )),
	        ast_expr_copy(deviceexpr)
	      )
	    )
	  );
	op.deviceexpr = deviceexpr;

	op.thestmt = *t;
	oc = outline_OpenMP(t, op);

	if (oc.func_struct)
		gpuize_struct(oc.func_struct, set_size(oc.usedvars[DCT_BYVALUE]));
	
	/* 5) Check if a struct was created and free it
	 *   -- do the same for the decldata struct (VVD)
	 */
	if (oc.func_struct)
		//ort_devdata_free(DEVENV_STRUCT_NAME, <deviceexpr>);
		ast_stmt_append(oc.repl_aftcall ? oc.repl_aftcall : oc.repl_funcall,
		                 FuncCallStmt(
		                   IdentName("ort_devdata_free"),
		                   CommaList(
		                     IdentName(op.structName),
		                     ast_expr_copy(deviceexpr)
		                   )
		                 )
		                );
	if (declvars_exist())
		//ort_devdata_free(_decl_data, <deviceexpr>);
		ast_stmt_append(oc.repl_aftcall ? oc.repl_aftcall : oc.repl_funcall,
		                 FuncCallStmt(
		                   IdentName("ort_devdata_free"),
		                   CommaList(
		                     Identifier(declstructVar),
		                     ast_expr_copy(deviceexpr)
		                   )
		                 )
		                );

	//In order to place it at the start of the generated code we have to go past
	//the commented directive and into the compound
	producedc = &oc.replacement->body->body;

	/* When there is no _dev_data struct, we need to remember where the
	 * offload statment is located so as to insert (possibly) the _decl_data
	 * struct just before it; in fact because produce_decl_var_code()
	 * places it right *after* rep_struct, we must actualy remember the
	 * statement right before the offload. Thus, when no _dev_data exists,
	 * we add an artificial comment to use as the spot after which the
	 * _decl_data struct will be placed, if neeed.
	 */
	if (!oc.repl_struct)
		ast_stmt_prepend(*producedc, repstruct_pin = verbit("/* no_data_denv */"));
	else
		repstruct_pin = oc.repl_struct;

	/* 6) Create the code for the device data environment
	 */
#ifdef DEVENV_DBG
	fprintf(stderr, "[target env]:\n");
	ast_ompdir_show_stderr(op.thestmt->u.omp->directive);
#endif
	create_devdata_env(producedc, op.thestmt->u.omp,
	                   oc.usedvars, oc.usedvars[DCT_IGNORE], deviceexpr);

	prepare_gpu_wrapper(op.extraParameters, oc.usedvars, oc.repl_struct,
	                    op.structType, devptrs, newtarget);

	/* 7) Now that clause xlitems were used, get rid of the OmpStmt
	 */
	ast_free(op.thestmt);          /* Get rid of the OmpStmt */

	/* 8) Prepare the task data environment, if needed.
	 */
	if (deps || nowait)
		xformtask = targettask(producedc, devicestm, deps, nowait, oc.usedvars);
	/* Insert the variable generated for the device id (and any tasking stuff) */
	ast_stmt_prepend(*producedc, devicestm);

	/* 9) Store the generated code
	 */
	ast_parentize(targtree);
	newtarget->tree = ast_stmt_copy(targtree);
	newtarget->newglobals = targnewglobals;
	newtarget->rep_struct = repstruct_pin;
	newtarget->functionName = strdup(op.functionName);
	newtarget->next = Targets;
	Targets = newtarget;

	/* Let the runtime know about this kernel */
	bld_autoinits_add(
		Expression(
			FunctionCall(
				IdentName("ort_kerneltable_add"),
				CommaList(String(basename), IdentName(newtarget->functionName))
			)
		)
	);

	targtree       = NULL;
	targnewglobals = NULL;

	if (xformtask)
	{
		taskopt_e bak = taskoptLevel;

		taskoptLevel = OPT_NONE;
		ast_stmt_parent(parent, *t);
		ast_stmt_xform(t);
		taskoptLevel = bak;
	}
}
