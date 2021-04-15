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

/* x_cars.h -- statistics for Compiler-Assisted omp Runtime Support */

#ifndef __X_CARS_H__
#define __X_CARS_H__

#include "ast.h"
#include "set.h"
#include "str.h"

SET_TYPE_DEFINE(critnames_st, symbol, char, 53)

#define NUM_METRICS 21

typedef enum {
	CARS_nparallel = 0, /* # 1st-level constructs */
	CARS_nsingle, 
	CARS_nsections, 
	CARS_nfor,
	CARS_ntask, 
	CARS_nreduction, 
	CARS_nordered, 
	
	CARS_nschedstatic, 
	CARS_nscheddynamic, 
	CARS_nschedguided, 
	CARS_nnowait, 
	CARS_natomic, 
	CARS_nforordered, 
	CARS_nuncritical,   /* # unnamed critical regions */
	
	CARS_cur_par_lev,   /* current parallelism level */
	CARS_max_par_lev,   /* maximum parallelism level */
	
	CARS_hasfuncptr,    /* has function pointers */
	CARS_hasrecursion,  /* if non-zero, cannot do exact calculations... */
	CARS_haslocks, 
	CARS_hasextraicvs,
	
	CARS_maxmetric      /* the size of the enum */
} carsmetrics_t;


typedef struct { 
  int mtr[CARS_maxmetric];     /* An array containing all metrics */
  set(critnames_st) critnames; /* Named critcal region names */
} targstats_t;


#define carsmtr(s,a) (s)->mtr[CARS_## a]

/* Put stats on a string with key-value lines */
extern void cars_stringify_stats(str s, targstats_t *stats);

/* This gathers statistics for the target construct in question. */
extern targstats_t *cars_analyze_target(aststmt t);

/* This search the tree for function definitions inside TARGET DECLARE
 * regions and analyzes them.
 */
extern void cars_analyze_declared_funcs(aststmt wholetree);

#endif /* __X_CARS_H__ */
