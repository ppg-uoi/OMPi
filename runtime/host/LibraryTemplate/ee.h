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

/* This is the pthreads threading library for OMPi
 */

#ifndef __EE_H__
#define __EE_H__

#include "config.h"

#ifdef HAVE_SPINLOCKS
	/* Needed to get spin locks, before including <pthread.h> */
	#define _XOPEN_SOURCE 600
#endif

#include <unistd.h>
#include <pthread.h>
#include "../sysdeps.h"

/* The threading library should provide the following types:
 *    othr_key_t
 *    othr_lock_t
 */

typedef struct                /* For nested locks */
{
	pthread_mutex_t lock;     /* The lock in question */
	pthread_mutex_t ilock;    /* Lock to access the whole struct */
	pthread_cond_t  cond;     /* For waiting until lock is available */
	int             count;    /* # times locked by the same thread */
	void           *owner;    /* The owner (task) of the nestable lock */
} othr_nestlock_t;

typedef union
{
	struct
	{
		int   type;                 /* normal/spin/nested */
		union
		{
			pthread_mutex_t normal;   /* normal lock */
			othr_nestlock_t nest;     /* nest lock */
#ifdef HAVE_SPINLOCKS           /* spin lock */
			pthread_spinlock_t spin;
#else
			struct
			{
				int             rndelay;     /* Used for initial spin delays */
				pthread_mutex_t mutex;
			} spin;
#endif
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
#ifdef TLS_KEYWORD
	#define USE_TLS  /* This is only for ort.c */
#endif
#define othr_key_t            pthread_key_t
#define othr_key_create(a,b)  pthread_key_create(a,b)
#define othr_getspecific(a)   pthread_getspecific(a)
#define othr_setspecific(a,b) pthread_setspecific(a,b)

/* Locks */
extern int  othr_init_lock(othr_lock_t *lock, int type);
extern int  othr_destroy_lock(othr_lock_t *lock);
extern int  othr_set_lock(othr_lock_t *lock);
extern int  othr_unset_lock(othr_lock_t *lock);
extern int  othr_test_lock(othr_lock_t *lock);

#endif

