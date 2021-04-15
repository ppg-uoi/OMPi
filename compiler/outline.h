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

#ifndef OUTLINE_H
#define OUTLINE_H

#ifdef  __cplusplus
extern "C" {
#  endif

#include "ast_vars.h"
#include "stddefs.h"


/* The produced function has the following structure:
 * 
 * function(arg)
 * {
 *   <local declarations>       (includes the struct)
 *   <code before the region>   (e.g initializers from the struct fields)
 *   <the region>               (the oroginal code is moved here)
 *   <code after the region>   
 *   <return statement>         (return (void*)0)
 * }
 * 
 * The replacement code has the following structure:
 * 
 * {
 *   <declarations>             (includes the struct)
 *   <code before the call>     (e.g. struct field initializers)
 *   <the function call>        
 *   <code after the call>      
 * }
 * 
 */   
typedef struct _outline_parts
{
		// The new function definition
		// Contains the new struct, variable declarations/initializations,
		// the functionbody and the return statement;
	aststmt function;

		// A Compound containing the code that was put in the place of the old code
		// Contains the new struct, the struct initialization and the function call
	aststmt replacement;

	/* Hooks in the function; if needed, they should only be changed in-place.
	 */
	
		// The definition of the struct used for byref/byvalue variables inside the
		// new function. Although it is included in the func_locals that follows,
		// everybody needs direct access to the definition of this struct.
		// This is the only field that can be NULL to signify that there is no 
		// struct at all.
	aststmt func_struct;

		// The declaration statements at the top of the new function.
		// May contain the func_struct and other local variables.
	aststmt func_decls;

		// The statements right after the declarations of the new function.
		// May contain e.g. local variable initializations.
	aststmt func_befcode;

		// The statement that contains the body of the new function
		// Basically the old code with pointerized variables
	aststmt func_regcode;

		// The statements right after the main body of the new function.
		// May contain e.g. write backs to the struct fields.
	aststmt func_aftcode;
	
		// The return statement (return (void *) 0;)
	aststmt func_return;

	/* Hooks in the replacement code; if needed, only change in-place.
	 */
	
		// The declaration of the struct used for byref/byvalue variables where the
		// original code was. Although included in the repl_locals that follows,
		// everybody needs direct access to the definition of this struct.
		// This is the only field that can be NULL to signify that there is no 
		// struct at all.
	aststmt repl_struct;

		// Declarations at the top of the replacement code.
	  // Contains possibly the struct and any other temporary variables.
	aststmt repl_decls;   // NEW
	
		// The statements right after the declarations in the replacement code.
		// May contain e.g. temporary variable and/or struct field initializations.
	aststmt repl_befcall; // NEW
	
		// The call to the function
	aststmt repl_funcall;

		// The code that is inserted after the functioncall for copy back of variables
	aststmt repl_aftcall;

		// The variables that were used in the code
	set(vars) *usedvars;
} outcome_t;


/* How to handle byvalue (firstprivate) variables: 
 * - byname passes pointers and the values are copied in the function
 * - bycopy passes copies of the variables to the function
 * - scalarbycopy treats scalars as bycopy and arrays as byname
 */
typedef enum { BYVAL_byname, BYVAL_bycopy, BYVAL_scalarbycopy } byval_e;

/**
 * A struct with the options passed into the
 */
typedef struct _outline_parameters
{
	/* If true, the common 1-parameter function will be output, with all 
	 * variables passed to it through the fields of a struct. Otherwise, they 
	 * are passed directly as multiple arguments to the function.
	 * The latter is of no use currently.
	 */
	bool structbased;
	
	//The name of the new function
	char functionName[20];

	//An expression that is used to call the new function
	//This is the identifier only
	//If NULL is used then the outlined function will be called directly
	char *functionCall;

	//Parameters that will be passed to functionCall along with the function
	//name and the struct
	astexpr extraParameters;

	// How to handle byvalue variables (see comments @ the union definition)
	byval_e byvalue_type;

	//If set, all byref are passed as references in the new struct
	bool global_byref_in_struct;

	//The name of the new struct that will hold the byref and byvalue variables
	//Must not be NULL
	char *structType;

	//The name of the variable that will be of type structName
	//Must not be NULL
	char *structName;

	//The expression that will be used to initialize the struct variable
	//If NULL an instance of the struct is created
	astexpr structInitializer;

	//A function that determines how an implicit variable will be used
	//Must not be NULL
	vartype_t(*implicitDefault)(setelem(vars));

	//The device on which the code will be executed. Only used for device data
	//environment variables
	astexpr deviceexpr;

	//aststmt inlineCondition;
	//TODO a char * with the name (type) of the outline e.g. parallel
	
	/* Whether to comment the original construct @ the replacement part */
	bool addComment;
	
	/* The statement which is being outlined (used only for reductions) --VVD */
	aststmt thestmt;
} outpars_t;

/* Produces a list of declarations. */
aststmt out_inline_private(set(vars) s);
/* Produces a list of byvalue declarations. Used for inlining task code. */
aststmt out_inline_firstprivate(set(vars) s, aststmt *varinits);

outcome_t outline(aststmt *b, outpars_t oo, set(vars) *usedvars);
outcome_t outline_OpenMP(aststmt *b, outpars_t oo);

/**
 * Repeats all visible struct/union/enum declarations
 * @param globals If true only copy globals else only copy non globals
 * @return a copy of the declarations
 */
aststmt copy_sue_declarations(bool globals);

#  ifdef  __cplusplus
}
#endif

#endif  /* OUTLINE_H */
