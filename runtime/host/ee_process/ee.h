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
 *    oprc_key_t
 *    oprc_lock_t
 */

typedef union
{
	struct
	{
		int   val;             /* for user-defined locks */
		int   count;           /* # a nested lock is locked */
		int   owner;           /* lock owner for nested locks*/
		int   type;            /* normal/spin/nested */
	} lock;

	char padding[CACHE_LINE];
} oprc_lock_t;


/*
 * The threading library should provide the functions that follow.
 */

/* Base functions */

extern void oprc_shm_free(int *p);
extern void oprc_shmalloc(void **p, size_t size, int *memid);
extern int  oprc_initialize(int *argc, char ***argv,
                            ort_icvs_t *icv, ort_caps_t *cap);
extern void oprc_finalize(int exitvalue);
extern int  oprc_request(int numthreads, int level);
extern void oprc_create(int numthreads, int level, void *arg,
                        void **info);
extern void oprc_waitall(void **info);
#define oprc_yield() sched_yield()
#define EE_TYPE_PROCESS
/* Thread-specifics */
#define oprc_key_t            pthread_key_t
#define oprc_key_create(a,b)  pthread_key_create(a,b)
#define oprc_getspecific(a)   pthread_getspecific(a)
#define oprc_setspecific(a,b) pthread_setspecific(a,b)

/* Locks */
extern void   *oprc_init_lock(oprc_lock_t *lock, int type);
extern int  oprc_destroy_lock(oprc_lock_t *lock);
extern int  oprc_set_lock(oprc_lock_t *lock);
extern int  oprc_unset_lock(oprc_lock_t *lock);
extern int  oprc_test_lock(oprc_lock_t *lock);
#endif

