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

#ifndef __RT_COMMON_H__
#define __RT_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "omp.h"

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
	int         rtchunk;        /* ditto */
	int         nthreads;       /* default # threads for a team */
	/* OpenMP 4.0.0 */
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

	/* OpenMP 4.0.0 */
	omp_proc_bind_t proc_bind;
	int         cancel;
	int         def_device;
	int       **place_partition;
} ort_icvs_t;                  /* global */


int  **places_dup(int **from);
void   places_free(int **place);

#endif /* __RT_COMMON_H__ */
