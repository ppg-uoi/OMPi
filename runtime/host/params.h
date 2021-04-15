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

/* params.h for ORT */

/*
 * 2019/11/14:
 *   Values revised because of parade.
 * 2014/06/16:
 *   First time around.
 */

#ifndef __PARAMS_H__
#define __PARAMS_H__

/* ort_env.c */
#define MAX_PLACE_LEN      1023 /* M number of resources in a place */

/* ort_prive.h */
#define TASK_QUEUE_SIZE    24   /* Per-thread queue size for pending tasks  */
#define MAX_ACTIVE_REGIONS 50   /* Max number of concurrently active regions */
#define YIELD_IMMEDIATELY  0    /* # busy-waiting repetitions before yielding */
#define YIELD_FREQUENTLY   50   /* ditto */
#define YIELD_OCCASIONALY  150  /* ditto */
#define YIELD_RARELY       500  /* ditto */
#define BAR_YIELD          50   /* ditto for barriers */

#define MAX_NUMTHR_LEVELS  10   /* Max. number of levels for OMP_NUM_THREADS */
#define MAX_BIND_LEVELS    10   /* Max. thread binding level (OMP_PROC_BIND) */

#endif

