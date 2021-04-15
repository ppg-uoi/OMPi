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

#ifndef __RT_COMMON_H__
#define __RT_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "omp.h"


typedef unsigned long u_long;


/* For initializing locks */
#define ORT_LOCK_NORMAL 0
#define ORT_LOCK_NEST   1
#define ORT_LOCK_SPIN   2

/* Array of pointers; each one points to a place (i.e. an array of ints).
 * places[0][0] gives the total number of places;
 * places[i][0] (i > 0) gives the length of the i-th place array.
 */
#define numplaces(p)   ((p) ? (p)[0][0] : 0)
#define placelen(p,i)  ((p) ? (((i) >= 0) && ((i) < (p)[0][0]) ? ( (p)[(i)+1] ? (p)[(i)+1][0] : 0 ) : 0) : 0)

/* The ICVs (internal control variables -- OpenMP V.4.0) */
typedef struct
{
	int         dynamic;
	int         nested;
	omp_sched_t rtschedule;     /* For runtime schedules */
	u_long      rtchunk;        /* ditto */
	int         nthreads;       /* default # threads for a team */
	/* OpenMP 4.0 */
	int         threadlimit;
	omp_proc_bind_t proc_bind;
	int         def_device;
	void       *cur_de;         /* current device data environment */
} ort_task_icvs_t;            /* per task */

typedef struct
{
	int         dynamic;        /* For the initial task */
	int         nested;
	omp_sched_t rtschedule;     /* For runtime schedules */
	int         rtchunk;        /* ditto */
	int         nthreads;       /* default # threads for a team */

	int         ncpus;          /* Global */
	size_t      stacksize;
	int         waitpolicy;
	int         threadlimit;
	int         levellimit;

	/* OpenMP 4.0 */
	omp_proc_bind_t proc_bind;
	int         cancel;
	int         def_device;
	/* OpenMP 4.5 */
	int         max_task_prio;
	 /* OpenMP 5.0 */
	int         display_affinity;  /* Global */
	char       *affinity_format;   /* per device */
	int         targetoffload;     /* Global */
} ort_icvs_t;            /* global */


int  **places_dup(int **from);
void   places_free(int **place);


/*
 * When using MPI all nodes get a copy of the user's executable that
 * contains all kernel functions. However, because of potential differences
 * in architecture, the memory address at which each kernel function
 * resides might be different for every node. To solve this problem, each
 * node creates an array (i.e. kerneltable) that has each kernel's name
 * and a pointer to its function. When node A needs to offload a kernel to
 * node B, it searches its kerneltable and transmits the **index** of the
 * requested kernel to node B. Node B then executes the kernel function
 * that corresponds to the received index.
 */

/**
 * Add a kernel function to the kerneltable. OMPi automatically puts calls
 * to this function when compiling user programs with kernels. NOTE that
 * this function is called *before* main function, so ort is not initialized
 * yet.
 * @param name             Name of the kernel (e.g. "enterexit_d00")
 * @param kernel_function  Pointer to the kernel function
 */
void ort_kerneltable_add(char *name, void *(*kernel_function)(void *));

/* Return the kernel id that corresponds to the requested kernel name */
int get_kernel_id_from_name(char *name);

/* Return a pointer to the kernel function that corresponds to the
 * requested kernel id */
void *(*get_kernel_function_from_id(int id))(void *);

/* Free allocated memory. Should be called once on shutdown. */
void free_kerneltable(void);

#endif /* __RT_COMMON_H__ */
