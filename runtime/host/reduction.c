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

/* reduction.c -- new ORT reduction module */

#include <stdio.h>
#include "ort_prive.h"

#define ALLOC_THRESHOLD 16

/* For keeping partial (per child) reduction results */
typedef union
{
	struct {
		void *addr;      /* The address of the datum (array) */
		int  nelems;     /* The number of elements of the array */
		int  spin;       /* Flag to use when spinning */
	} value;
	char padding[CACHE_LINE];
} redelem_t;


typedef struct
{
	redelem_t *redtable;
	redelem_t *actual_arr_ptr;
	int        alloc_size;
	// Currently there is no need to store team_size
} red_t;


/* Next 2 functions represent the "old" mechanism; used to live in ort.c
 */
void ort_reduction_begin(omp_lock_t *redlock)
{
	/* Because OMPi's parser declares all reduction locks as globals,
	 * they are initialized to NULL, thus we can safely differentiate
	 * between uninitialized and initialized ones.
	 */
	if (*redlock == NULL) ort_prepare_omp_lock(redlock, ORT_LOCK_SPIN);

#if defined(EE_TYPE_PROCESS_MPI)
	ee_set_lock((ee_lock_t *) redlock);
#else
	ee_set_lock((ee_lock_t *) *redlock);
#endif
}


void ort_reduction_end(omp_lock_t *redlock)
{
#if defined(EE_TYPE_PROCESS_MPI)
	ee_unset_lock((ee_lock_t *) redlock);
#else
	ee_unset_lock((ee_lock_t *) *redlock);
#endif
}


/* 
 * NEW MECHANISM (it does not work for process-type EEs)
 */


/* Parent inits the reduction table for the team it is about to create */
void ort_reductions_init(ort_eecb_t *me, int teamsize)
{
	red_t *redinfo = me->mf->redinfo;

	if (redinfo == NULL)
	{
		redinfo = (red_t *) (me->mf->redinfo = ort_calloc(sizeof(red_t)));
		// redinfo->alloc_size = 0;         // Not required due to calloc
		// redinfo->actual_arr_ptr = NULL;  // Not required due to calloc
	}

	if ((teamsize > redinfo->alloc_size) ||
			(redinfo->alloc_size >= ALLOC_THRESHOLD) &&
			(teamsize <= (redinfo->alloc_size >> 1)))
	{
		redinfo->redtable = ort_realloc_aligned(teamsize * sizeof(redelem_t),
				(void **) &redinfo->actual_arr_ptr);
		redinfo->alloc_size = teamsize;
	}

	for(--teamsize; teamsize >= 0; teamsize--)
		redinfo->redtable[teamsize].value.spin = 0;
}


void ort_reductions_finalize(ort_eecb_t *me)
{
	ort_mcbf_t *mcbf = me->mf;
	if (mcbf == NULL || mcbf->redinfo == NULL)
		return;

	free(((red_t *) mcbf->redinfo)->actual_arr_ptr);
	free(mcbf->redinfo);
	mcbf->redinfo = NULL;
}


/* 
 * Reduction function macros
 */


/* The top part of a reduction function */
#define reduction_function_prologue(COPERATOR, SUFFIX, TYPE)\
\
void reduce##COPERATOR##SUFFIX(TYPE *local, TYPE *global, int nelems) \
{\
	ort_eecb_t *me = __MYCB;\
	int time = 0, *spin;\
	int myid = me->thread_num, numberOfThreads = me->num_siblings;\
	redelem_t *partialResults;\
	\
	if (numberOfThreads == 1) /* special case */ {\
		int j;\
		for (j = 0; j < nelems; j++) { 
				
			/* Here goes the operation for the serial case */

#define reduction_function_after_serial\
		}\
		return;\
	}\
	partialResults = ((red_t *) TEAMINFO(me)->redinfo)->redtable;\
	if (myid > 0)  /* all other threads */{\
		for (spin = &(partialResults[myid].value.spin); *spin == 1; time++)\
			if (time == BAR_YIELD) {\
				time = -1;\
				ee_yield();\
			};\
		partialResults[myid].value.addr = local;\
		FENCE;\
		*spin = 1;\
	}\
	else {         /* The master */\
		int i, j;\
		partialResults[myid].value.addr = local;\
		/* Ensure that all threads are in the barrier */\
		for (i = 1; i < numberOfThreads; i++) {\
			for (; partialResults[i].value.spin == 0; time++)\
				if (time == BAR_YIELD) {\
					time = -1;\
					ee_yield();\
				};\
		}\
		\
		for (i = 0 ; i < numberOfThreads ; i++)\
			for (j = 0; j < nelems; j++) { 
				
				/* Here goes the operation */

/* ... and the bottom part */
#define reduction_function_epilogue\
			};\
		FENCE;\
		\
		/* Release the others */\
		for (i = 1; i < numberOfThreads; i++)\
			partialResults[i].value.spin = 0;\
	}\
}\


#define define_reduction_function_anyop(COPERATOR, SUFFIX, TYPE, OPERATOR)\
	reduction_function_prologue(COPERATOR, SUFFIX, TYPE)\
				global[j] = global[j] OPERATOR local[j];\
	reduction_function_after_serial\
				global[j] = global[j] OPERATOR ((TYPE *) (partialResults[i].value.addr))[j];\
	reduction_function_epilogue
    
#define define_reduction_function_minmax(COPERATOR, SUFFIX, TYPE, OPERATOR)\
	reduction_function_prologue(COPERATOR, SUFFIX, TYPE)\
				if ( local[j] OPERATOR global[j] )\
						global[j] = local[j];\
	reduction_function_after_serial\
				if ( (( TYPE *) (partialResults[i].value.addr))[j] OPERATOR global[j] )\
					global[j] = ((TYPE *) (partialResults[i].value.addr))[j];\
	reduction_function_epilogue


/*
 * REDUCTION FUNCTIONS GENERATION
 */

/* add (+) */
define_reduction_function_anyop(_add, ___i, int, + )
define_reduction_function_anyop(_add, __si, short int, + )
define_reduction_function_anyop(_add, __li, long int, + )
define_reduction_function_anyop(_add, __Li, long long int, + )
define_reduction_function_anyop(_add, _u_i, unsigned int, + )
define_reduction_function_anyop(_add, _usi, unsigned short int, + )
define_reduction_function_anyop(_add, _uli, unsigned long int, + )
define_reduction_function_anyop(_add, _uLi, unsigned long long int, + )
define_reduction_function_anyop(_add, ___c, char, + )
define_reduction_function_anyop(_add, ___d, double, +  )
define_reduction_function_anyop(_add, ___f, float, +  )
define_reduction_function_anyop(_add, __ld, long double, + )
define_reduction_function_anyop(_add, _u_c, unsigned char, + )

/* subtract (-) */
define_reduction_function_anyop(_subtract, ___i, int, + )
define_reduction_function_anyop(_subtract, __si, short int, + )
define_reduction_function_anyop(_subtract, __li, long int, + )
define_reduction_function_anyop(_subtract, __Li, long long int, + )
define_reduction_function_anyop(_subtract, _u_i, unsigned int, + )
define_reduction_function_anyop(_subtract, _usi, unsigned short int, + )
define_reduction_function_anyop(_subtract, _uli, unsigned long int, + )
define_reduction_function_anyop(_subtract, _uLi, unsigned long long int, + )
define_reduction_function_anyop(_subtract, ___c, char, + )
define_reduction_function_anyop(_subtract, ___d, double, + )
define_reduction_function_anyop(_subtract, ___f, float, + )
define_reduction_function_anyop(_subtract, __ld, long double, + )
define_reduction_function_anyop(_subtract, _u_c, unsigned char, + )

/* multiply (*) */
define_reduction_function_anyop(_multiply, ___i, int, * )
define_reduction_function_anyop(_multiply, __si, short int, * )
define_reduction_function_anyop(_multiply, __li, long int, * )
define_reduction_function_anyop(_multiply, __Li, long long int, * )
define_reduction_function_anyop(_multiply, _u_i, unsigned int, * )
define_reduction_function_anyop(_multiply, _usi, unsigned short int, * )
define_reduction_function_anyop(_multiply, _uli, unsigned long int, * )
define_reduction_function_anyop(_multiply, _uLi, unsigned long long int, * )
define_reduction_function_anyop(_multiply, ___c, char, * )
define_reduction_function_anyop(_multiply, ___d, double, * )
define_reduction_function_anyop(_multiply, ___f, float, * )
define_reduction_function_anyop(_multiply, __ld, long double, * )
define_reduction_function_anyop(_multiply, _u_c, unsigned char, * )

/* bitwise AND (&) */
define_reduction_function_anyop(_bitand, ___i, int, & )
define_reduction_function_anyop(_bitand, __si, short int, & )
define_reduction_function_anyop(_bitand, __li, long int, & )
define_reduction_function_anyop(_bitand, __Li, long long int, & )
define_reduction_function_anyop(_bitand, _u_i, unsigned int, & )
define_reduction_function_anyop(_bitand, _usi, unsigned short int, & )
define_reduction_function_anyop(_bitand, _uli, unsigned long int, & )
define_reduction_function_anyop(_bitand, _uLi, unsigned long long int, & )
define_reduction_function_anyop(_bitand, ___c, char, & )
define_reduction_function_anyop(_bitand, _u_c, unsigned char, & )

/* bitwise OR (|) */
define_reduction_function_anyop(_bitor, ___i, int, | )
define_reduction_function_anyop(_bitor, __si, short int, | )
define_reduction_function_anyop(_bitor, __li, long int, | )
define_reduction_function_anyop(_bitor, __Li, long long int, | )
define_reduction_function_anyop(_bitor, _u_i, unsigned int, | )
define_reduction_function_anyop(_bitor, _usi, unsigned short int, | )
define_reduction_function_anyop(_bitor, _uli, unsigned long int, | )
define_reduction_function_anyop(_bitor, _uLi, unsigned long long int, | )
define_reduction_function_anyop(_bitor, ___c, char, | )
define_reduction_function_anyop(_bitor, _u_c, unsigned char, | )

/* bitwise XOR (^) */
define_reduction_function_anyop(_bitxor, ___i, int, ^ )
define_reduction_function_anyop(_bitxor, __si, short int, ^ )
define_reduction_function_anyop(_bitxor, __li, long int, ^ )
define_reduction_function_anyop(_bitxor, __Li, long long int, ^ )
define_reduction_function_anyop(_bitxor, _u_i, unsigned int, ^ )
define_reduction_function_anyop(_bitxor, _usi, unsigned short int, ^ )
define_reduction_function_anyop(_bitxor, _uli, unsigned long int, ^ )
define_reduction_function_anyop(_bitxor, _uLi, unsigned long long int, ^ )
define_reduction_function_anyop(_bitxor, ___c, char, ^ )
define_reduction_function_anyop(_bitxor, _u_c, unsigned char, ^ )

/* logical AND (&&) */
define_reduction_function_anyop(_and, ___i, int, && )
define_reduction_function_anyop(_and, __si, short int, && )
define_reduction_function_anyop(_and, __li, long int, && )
define_reduction_function_anyop(_and, __Li, long long int, && )
define_reduction_function_anyop(_and, _u_i, unsigned int, && )
define_reduction_function_anyop(_and, _usi, unsigned short int, && )
define_reduction_function_anyop(_and, _uli, unsigned long int, && )
define_reduction_function_anyop(_and, _uLi, unsigned long long int, && )
define_reduction_function_anyop(_and, ___c, char, && )
define_reduction_function_anyop(_and, ___d, double, && )
define_reduction_function_anyop(_and, ___f, float, && )
define_reduction_function_anyop(_and, __ld, long double, && )
define_reduction_function_anyop(_and, _u_c, unsigned char, && )

/* logical OR (||) */
define_reduction_function_anyop(_or, ___i, int, || )
define_reduction_function_anyop(_or, __si, short int, || )
define_reduction_function_anyop(_or, __li, long int, || )
define_reduction_function_anyop(_or, __Li, long long int, || )
define_reduction_function_anyop(_or, _u_i, unsigned int, || )
define_reduction_function_anyop(_or, _usi, unsigned short int, || )
define_reduction_function_anyop(_or, _uli, unsigned long int, || )
define_reduction_function_anyop(_or, _uLi, unsigned long long int, || )
define_reduction_function_anyop(_or, ___c, char, || )
define_reduction_function_anyop(_or, ___d, double, || )
define_reduction_function_anyop(_or, ___f, float, || )
define_reduction_function_anyop(_or, __ld, long double, || )
define_reduction_function_anyop(_or, _u_c, unsigned char, || )

/* max */
define_reduction_function_minmax(_max, ___i, int, > )
define_reduction_function_minmax(_max, __si, short int, > )
define_reduction_function_minmax(_max, __li, long int, > )
define_reduction_function_minmax(_max, __Li, long long int, > )
define_reduction_function_minmax(_max, _u_i, unsigned int, > )
define_reduction_function_minmax(_max, _usi, unsigned short int, > )
define_reduction_function_minmax(_max, _uli, unsigned long int, > )
define_reduction_function_minmax(_max, _uLi, unsigned long long int, > )
define_reduction_function_minmax(_max, ___c, char, > )
define_reduction_function_minmax(_max, ___d, double, > )
define_reduction_function_minmax(_max, ___f, float, > )
define_reduction_function_minmax(_max, __ld, long double, > )
define_reduction_function_minmax(_max, _u_c, unsigned char, > )

/* min */
define_reduction_function_minmax(_min, ___i, int, < )
define_reduction_function_minmax(_min, __si, short int, < )
define_reduction_function_minmax(_min, __li, long int, < )
define_reduction_function_minmax(_min, __Li, long long int, < )
define_reduction_function_minmax(_min, _u_i, unsigned int, < )
define_reduction_function_minmax(_min, _usi, unsigned short int, < )
define_reduction_function_minmax(_min, _uli, unsigned long int, < )
define_reduction_function_minmax(_min, _uLi, unsigned long long int, < )
define_reduction_function_minmax(_min, ___c, char, < )
define_reduction_function_minmax(_min, ___d, double, < )
define_reduction_function_minmax(_min, ___f, float, < )
define_reduction_function_minmax(_min, __ld, long double, < )
define_reduction_function_minmax(_min, _u_c, unsigned char, < )

/*
 * JUMP TABLES
 */

typedef void (*redfunc_t)(void *, void *, int);

/* 
 * ENCODING OF OPERAND TYPE:  see x_clauses.c:
 * __i = 0
 * _si = 1
 * _li = 2
 * _Li = 3
 * u_i = 4
 * usi = 5
 * uli = 6
 * uLi = 7
 * __c = 8
 * __d = 9
 * __f = 10
 * _ld = 11
 * u_c = 12
 */
#define REDFUNC_FULL_JUMP_TABLE(op) \
static redfunc_t op ## _jump[] = { \
	(redfunc_t) reduce_##op##___i, \
	(redfunc_t) reduce_##op##__si, \
	(redfunc_t) reduce_##op##__li, \
	(redfunc_t) reduce_##op##__Li, \
	(redfunc_t) reduce_##op##_u_i, \
	(redfunc_t) reduce_##op##_usi, \
	(redfunc_t) reduce_##op##_uli, \
	(redfunc_t) reduce_##op##_uLi, \
	(redfunc_t) reduce_##op##___c, \
	(redfunc_t) reduce_##op##___d, \
	(redfunc_t) reduce_##op##___f, \
	(redfunc_t) reduce_##op##__ld, \
	(redfunc_t) reduce_##op##_u_c  \
}

REDFUNC_FULL_JUMP_TABLE(add);
REDFUNC_FULL_JUMP_TABLE(subtract);
REDFUNC_FULL_JUMP_TABLE(multiply);
REDFUNC_FULL_JUMP_TABLE(and);
REDFUNC_FULL_JUMP_TABLE(or);
REDFUNC_FULL_JUMP_TABLE(max);
REDFUNC_FULL_JUMP_TABLE(min);

#define REDFUNC_INT_JUMP_TABLE(op)\
static redfunc_t op ## _jump[] = { \
	(redfunc_t) reduce_##op##___i, \
	(redfunc_t) reduce_##op##__si, \
	(redfunc_t) reduce_##op##__li, \
	(redfunc_t) reduce_##op##__Li, \
	(redfunc_t) reduce_##op##_u_i, \
	(redfunc_t) reduce_##op##_usi, \
	(redfunc_t) reduce_##op##_uli, \
	(redfunc_t) reduce_##op##_uLi, \
	(redfunc_t) reduce_##op##___c, \
	(redfunc_t) NULL, \
	(redfunc_t) NULL, \
	(redfunc_t) NULL, \
	(redfunc_t) reduce_##op##_u_c \
} 

REDFUNC_INT_JUMP_TABLE(bitand);
REDFUNC_INT_JUMP_TABLE(bitor);
REDFUNC_INT_JUMP_TABLE(bitxor);

/* 
 * THE INTERFACE
 */

#define EXPORTED_REDUCTION_FUNCTION(op) \
void ort_reduce_##op(int type, void *local, void *global, int nelems)\
{\
	(* op##_jump[type])(local, global, nelems);\
}

EXPORTED_REDUCTION_FUNCTION(add)
EXPORTED_REDUCTION_FUNCTION(subtract)
EXPORTED_REDUCTION_FUNCTION(multiply)
EXPORTED_REDUCTION_FUNCTION(and)
EXPORTED_REDUCTION_FUNCTION(or)
EXPORTED_REDUCTION_FUNCTION(max)
EXPORTED_REDUCTION_FUNCTION(min)
EXPORTED_REDUCTION_FUNCTION(bitand)
EXPORTED_REDUCTION_FUNCTION(bitor)
EXPORTED_REDUCTION_FUNCTION(bitxor)
