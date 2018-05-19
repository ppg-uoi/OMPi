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

/* ort_params.h */

/*
 * 2014/06/16:
 *   First time around.
 */

#ifndef __PARAMS_H__
#define __PARAMS_H__

/* ort.h */
#define MAX_BAR_THREADS   64   /* Maximum number of threads per team */

/* ort_env.c */
#define MAXPLACELEN       1024 /* Maximum number of OpenMP places */

/* ort_prive.h */
#define TASK_QUEUE_SIZE   24   /* Size of per-thread queues that store pending tasks  */
#define MAXACTIVEREGIONS  50   /* Maximum number of concurrent active regions */
#define YIELD_IMMEDIATELY 0    /* Milliseconds used to yield immediately */
#define YIELD_FREQUENTLY  50   /* Milliseconds used to yield frequently */
#define YIELD_OCCASIONALY 150  /* Milliseconds used to yield occasionaly */
#define YIELD_RARELY      500  /* Milliseconds used to yield barely */
#define BAR_YIELD         50   /* Milliseconds used to for barrier yield */

#define MAXNTHRLEVS       10   /* Max. number of levels for OMP_NUM_THREADS */
#define MAXBINDLEVS       10   /* Max. thread binding level (OMP_PROC_BIND) */

#endif
