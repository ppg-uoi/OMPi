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

/* This is the psthreads threading library for OMPi
 */

#ifndef __EE_H__
#define __EE_H__

#include "config.h"

#ifdef HAVE_SPINLOCKS
	/* Needed to get spin locks, before including <pthread.h> */
	#define _XOPEN_SOURCE 600
#endif

#include <unistd.h>
#include <torc.h>
#include <torc_internal.h>
#include "../sysdeps.h"

/* The threading library should provide the following types:
 *    othr_key_t
 *    othr_lock_t
 */

typedef struct              /* For nested locks */
{
	_lock_t lock;     /* The lock in question */
	_lock_t ilock;    /* Lock to access the whole struct */
	torc_cond_t  cond;     /* For waiting until lock is available */
	int count;                /* # times locked by the same thread */
	torc_t *owner;          /* Which thread owns the nestable lock */
} othr_nestlock_t;

typedef union
{
	struct
	{
		int   type;                 /* normal/spin/nested */
		union
		{
			_lock_t normal;   /* normal lock */
			othr_nestlock_t nest;      /* nest lock */
			struct
			{
				int             rndelay;     /* Used for initial spin delays */
				_lock_t mutex;
			} spin;
		} data;
	} lock;

	char padding[CACHE_LINE];
} othr_lock_t;


/*
 * The threading library should provide the functions that follow.
 */

/* Base functions */
extern int  othr_initialize(int *argc, char ***argv,
                            ort_icvs_t *icv, ort_caps_t *cap);
extern void othr_finalize(int exitvalue);
extern int  othr_request(int numthreads, int level);
extern void othr_create(int numthreads, int level, void *arg, void **info);
extern void othr_waitall(void **info);
#define othr_yield() sched_yield()

/* Thread-specifics */
#define othr_key_t            torc_key_t
#define othr_key_create(a,b)  _torc_key_create(a,b)
#define othr_getspecific(a)   _torc_getspecific(a)
#define othr_setspecific(a,b) _torc_setspecific(a,b)

/* Locks */
extern int  othr_init_lock(othr_lock_t *lock, int type);
extern int  othr_destroy_lock(othr_lock_t *lock);
extern int  othr_set_lock(othr_lock_t *lock);
extern int  othr_unset_lock(othr_lock_t *lock);
extern int  othr_test_lock(othr_lock_t *lock);


/* Barriers */
//#define AVOID_OMPI_DEFAULT_BARRIER      1
//typedef psthread_barrier_t othr_barrier_t;

//extern void othr_barrier_init(othr_barrier_t *bar, int n);
//extern void othr_barrier_wait(othr_barrier_t *bar, int id);
//extern void othr_barrier_destroy(othr_barrier_t *bar);

//#define OTHR_TASK_SUPPORT   1
//extern void othr_new_task(int flag, void (*func)(void *arg), void *arg);
//extern void othr_task_waitall(void);

#endif
