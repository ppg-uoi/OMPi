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

#ifndef OUTLINE_H
#define OUTLINE_H

#ifdef  __cplusplus
extern "C" {
#  endif

#include "ast_vars.h"
#include "boolean.h"


typedef struct _outline_parts
{
	//The new function
	//Contains the new struct, variable declarations/initializations,
	//the functionbody and the return statement;
	aststmt function;

	//The statement that contains the body of the new function
	//Basically the old code with pointerized variables
	aststmt functionbody;

	//The declaration of the struct used for byref/byvalue variables inside the
	//new function
	aststmt funcstruct;

	//The return statement (return (void *) 0;)
	aststmt returnstm;

	//A Compound containing the code that was put in the place of the old code
	//Contains the new struct, the struct initialization and the function call
	aststmt replacement;

	//The declaration of the struct used for byref/byvalue variables where the
	//original code was
	aststmt rep_struct;

	//The code that is inserted after the functioncall for copy back of variables
	aststmt rep_copyback;

	//The call to the function
	aststmt functioncall;

	//The variables that were used in the code
	set(vars) *usedvars;
} outcome_t;

/**
 * A struct with the options passed into the
 */
typedef struct _outline_parameters
{
	//The name of the new function
	char functionName[20];

	//An expression that is used to call the new function
	//This is the identifier only
	//If NULL is used then the outlined function will be called directly
	char *functionCall;

	//Parameters that will be passed to functionCall along with the function
	//name and the struct
	astexpr extraParameters;

	//If 1 byvalue variables are call-by-name (they are passed in as pointers
	//and then their values are copied on a new variable)
	//Else the variables are call-by-copy (their value is copied in the argument
	//struct and then the code uses them directly from the struct using pointers
	bool byvalue_type;

	//If set all byref are passed as references in the new struct
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
} outpars_t;

/**
 * A struct with the paramenters of handler functions
 */
typedef struct _handler_parameters
{
	/* Input */
	//A symbol containing the name of the struct instance
	symbol    structName;

	//A boolean telling whether the struct variable is a pointer or not. It's
	//used to determine whether "." or "->" is used to access the fields where
	//the original code was
	//(determined by whether or not there was a struct initializer)
	bool      repl_structVarIsPointer;

	//The device used for device data environment variables
	astexpr   deviceexpr; //remove it and add ompcon type

	/* Output */
	//The struct fields
	astdecl   structFields;

	//Initialization of the struct where the original code was
	aststmt   repl_structInit;

	//Declarations of variables in the new function
	aststmt   func_varDecls;

	//Initialization of variables in the new function
	aststmt   func_varInits;

	//Moving the results to the struct
	aststmt   func_results;

	//Moving the results from the struct to the original variables
	aststmt   repl_results;

	//Variables that need to be turned into pointers in the original code
	set(vars) func_mkpointers;
} hanpars_t ;

/* Produces a list of declarations. (Uses only hp.func_varDecls)
 */
void out_handle_private(set(vars) s, hanpars_t *hp);
void out_handle_byref(set(vars) s, hanpars_t *hp);
void out_handle_byval(set(vars) s, hanpars_t *hp, char ptInStruct);
void out_handle_reduction(set(vars) s, hanpars_t *hp);

/* Produces a list of byvalue declarations. Used for inlining task code.
 * (Uses only hp.func_varDecls andhp.func_varInits)
 */
void out_inline_byval(set(vars) s, hanpars_t *hp);

outcome_t outline(aststmt *b, outpars_t oo, set(vars) *usedvars);
outcome_t outline_OpenMP(aststmt *b, outpars_t oo);

/**
 * Repeats all visible struct/union/enum declarations
 *
 * @param globals If true only copy globals else only copy non globals
 *
 * @return a copy of the declarations
 */
aststmt copy_sue_declarations(bool globals);

/**
 * Add a statement before another statement
 *
 * @param where The old statement
 * @param what  The statement you want to insert
 */
void out_insert_before(aststmt where, aststmt what);

/**
 * Add a statement after another statement
 *
 * @param where The old statement
 * @param what  The statement you want to insert
 */
void out_insert_after(aststmt where, aststmt what);
#  ifdef  __cplusplus
}
#endif

#endif  /* OUTLINE_H */

