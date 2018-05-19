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
#include <psthread.h>
#include "../sysdeps.h"

/* The threading library should provide the following types:
 *    othr_key_t
 *    othr_lock_t
 */

typedef struct              /* For nested locks */
{
	_lock_t lock;     /* The lock in question */
	_lock_t ilock;    /* Lock to access the whole struct */
	psthread_cond_t  cond;     /* For waiting until lock is available */
	int count;                /* # times locked by the same thread */
	psthread_t owner;          /* Which thread owns the nestable lock */
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


#if 1
//#define OTHR_TASKS    1

typedef struct Node
{
	void *(*func)(void *);
	void *task_argument;    /* arg */
	void *info;
	int  num_children;    /* taskcreated */
	psthread_cond_t taskcond;
	psthread_t thr;
	void *mem;
	struct Node *parent;
	struct Node *prev, *next;
	_lock_t lock;

	int finished;
	int untied;
	int bare;
	int tid;
	int cid;
	int throttled;
	int staticmem;

	int level;
	int maxchildren;
	int recursive;
	int null;
	int isfinal;
	int inherit_task_node;

	ort_task_icvs_t icvs;              /* OpenMP3.0 */
} ort_task_node_t;


typedef struct _Args
{
	//  char data[128];
	struct _Args *prev, *next;
	_lock_t lock;
	int cid;
	int size;
#if 0
	void *function_arguments;     /* Task's function arguments */
	ort_task_node_t *task_parent; /* Parent task structure */
#endif
	//  char data[128];
} ort_task_args_t;

#endif

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
#if 0
	#define othr_yield() sched_yield()
#else
	#define othr_yield() psthread_yield()
#endif

/* Thread-specifics */
#define othr_key_t            psthread_key_t
#define othr_key_create(a,b)  psthread_key_create(a,b)
#define othr_getspecific(a)   psthread_getspecific(a)
#define othr_setspecific(a,b) psthread_setspecific(a,b)

/* Locks */
extern int  othr_init_lock(othr_lock_t *lock, int type);
extern int  othr_destroy_lock(othr_lock_t *lock);
extern int  othr_set_lock(othr_lock_t *lock);
extern int  othr_unset_lock(othr_lock_t *lock);
extern int  othr_test_lock(othr_lock_t *lock);


/* Barriers */
#if 1
	#define AVOID_OMPI_DEFAULT_BARRIER      1
#endif
typedef psthread_barrier_ex_t othr_barrier_t;

extern void othr_barrier_init(othr_barrier_t *bar, int n);
extern void othr_barrier_wait(othr_barrier_t *bar, int id);
extern void othr_barrier_destroy(othr_barrier_t *bar);

#define AVOID_OMPI_DEFAULT_TASKS      1
extern void othr_new_task(int final, int flag, void *(*func)(void *arg),
                          void *arg);
extern void othr_new_task_exec(int flag, void *(*func)(void *arg), void *arg);
extern void othr_taskwait(int how, void *info, int ompid);
extern void othr_set_status(int ompid);
extern void *othr_task_immediate_start(int final);
extern void othr_task_immediate_end(void *new_node);
extern int othr_check_throttling();
extern void othr_set_currtask(ort_task_node_t *tw);
extern ort_task_node_t *othr_get_currtask(void *info, int thread_num);
extern void *othr_taskenv_alloc(int size, void *task_func);
extern void othr_taskenv_free(void *arg);
#endif
