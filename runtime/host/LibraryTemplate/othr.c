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

/*
 * Nov. 2010
 *   Nested locks now owned by tasks (OpenMP3.0)
 * Jun. 2010
 *   Thread binding
 * Dec. 2009
 *   OpenMP 3.0 additions & support for dynamic pool (EL)
 * July 2007
 *   Bug fix (thanks to Marie-Christine Counilh)
 * Feb. 2007
 *   Finalized support for nested parallelism by George Filos
 * 3 Nov. 2006
 *   Modified othr_create() and moved here all the pool-related stuff.
 * 31 Oct 2006
 *   Added othr_yield()
 */

/*
 * An implementation of the othr API using plain & portable POSIX threads.
 * A simple, thread library for multilevel parallelism.
 * It uses pure PTHREADS and a pool of threads, endlessly spinning for work.
 * The pool is of fixed size.
 */

#include "config.h"

#if defined(CAN_BIND) && (defined(__SYSCOMPILER_gcc) || defined(__SYSCOMPILER_intel))
	#define _GNU_SOURCE
#endif

#include "../ort.h"
#include <stdlib.h>
#include <stdio.h>
#include "ee.h"

#if defined(__SYSCOMPILER_sun)
	#undef CAN_BIND   /* Problem with CPU_SET */
#endif

#ifdef CAN_BIND
	#include <sched.h>
#endif

#ifdef __SYSOS_solaris
	#include <sys/pset.h>
	#include <sys/types.h>
	#include <sys/processor.h>
	#include <sys/procset.h>
	#include <unistd.h>
#endif


#define WORKER_YIELD 50
#define MASTER_YIELD 1000
#define WAIT_WHILE(f, trials) { \
		int time = 0; \
		for ( ; (f); time++) \
			if (time == (trials)) { \
				time = -1; \
				othr_yield(); \
			}; \
	}

/* Types & declarations
 */

typedef struct othr_pool_s
{
	void *arg;                    /* Argument for the thread function */
	void *info;
	struct  othr_pool_s *next;    /* Next node into the list */
	volatile int spin;            /* Spin here waiting for work */
	int id;                       /* A sequential id given by OMPI */
#ifdef __SYSOS_solaris
	int creation_id;
	char pad[28];
#else
	char pad[32];
#endif
} othr_pool_t;

typedef struct
{
	int level;
	volatile int *running;         /* # running threads in a team */
	othr_pool_t  **tp;
	int team;
} othr_info;

static int          pthread_lib_inited = 0;  /* To avoid re-initializations */
static othr_pool_t  *H = NULL;                  /* Pool = a list of threads */
static volatile int plen;                          /* # threads in the pool */
static othr_lock_t  plock;                  /* A lock for accesing the pool */

#ifdef __SYSOS_solaris
	static int created_threads = 0;
#endif

/* OpenMP 3.0/3.1 stuff */
static int             threadlimit, threadscreated, levellimit, proc_bind;
static pthread_attr_t *globalattr = NULL;      /* For stacksize */


/* A list of blocked threads */
static
othr_pool_t *_new_bunch_of_threads(int n, othr_pool_t **tail)
{
	pthread_t   thr;
	volatile othr_pool_t *block, *node;
	int         i, e;
	void        *threadjob(void *);
	int         NP = ort_get_num_procs();

#ifdef CAN_BIND
	static int  first_cpu = 0, next_cpu = 0;
	cpu_set_t   cpuset;

	if (proc_bind)
		if (H == NULL)  /* first time called by initial thread; find its cpu. */
		{
			if (globalattr == NULL)
			{
				globalattr = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));
				pthread_attr_init(globalattr);
			}

			if (NP > 8 * sizeof(cpuset)) NP = 8 * sizeof(cpuset); /* just in case */
			if ((first_cpu = sched_getcpu()) < 0)
				first_cpu = 0;      /* assume initial thread @ cpu 0 */
			next_cpu = (first_cpu + 1) % NP;
		}
#endif

	if ((block = (othr_pool_t *)ort_alloc_aligned(n * sizeof(othr_pool_t),
	                                              NULL)) == NULL)
		ort_error(5, "othr_init() failed; could not allocate memory\n");

	for (i = 0, node = block; i < n; i++)
	{
		node->spin = 1;
		FENCE;
		node->next = (othr_pool_t *)(node + 1);

#ifdef __SYSOS_solaris
		if (created_threads < NP)
		{
			othr_set_lock(&plock);
			node->creation_id = ++created_threads;
			othr_unset_lock(&plock);
		}
		else
			node->creation_id = -1;
#endif

#ifdef CAN_BIND
		if (proc_bind)
			if (next_cpu != first_cpu)  /* only bind NP first threads */
			{
				CPU_ZERO(&cpuset);
				CPU_SET(next_cpu, &cpuset);
				if (pthread_attr_setaffinity_np(globalattr, sizeof(cpuset),
				                                (const cpu_set_t *) &cpuset))
					ort_warning("could not bind thread %d/%d on cpu %d\n", i, n, next_cpu);
			}
#endif

		if ((e = pthread_create(&thr, globalattr, threadjob, (void *) node)))
			ort_error(5, "pthread_create() failed with %d\n", e);
		node++;

#ifdef CAN_BIND
		if (proc_bind)
			if (next_cpu != first_cpu)  /* only bind NP first threads */
			{
				next_cpu = (next_cpu + 1) % NP;
				if (next_cpu == first_cpu)    /* next threads will all be unbound, */
				{
					/* so fill the cpu mask */
					int j;
					for (j = 0; j < NP; j++)
						CPU_SET(j, &cpuset);
					pthread_attr_setaffinity_np(globalattr, sizeof(cpuset),
					                            (const cpu_set_t *) &cpuset);
				}
			}
#endif
	}

	(--node)->next = NULL;
	if (tail) *tail = (othr_pool_t *)node;
	return ((othr_pool_t *)block);
}


/* Library initialization
 */
int othr_initialize(int *argc, char ***argv, ort_icvs_t *icv, ort_caps_t *caps)
{
	int nthr;

	nthr = (icv->nthreads >= 0) ?  /* Explicitely requested population */
	       icv->nthreads :
	       icv->ncpus - 1;   /* we don't handle the initial thread */
	if (icv->threadlimit != -1 && nthr > icv->threadlimit)
		nthr = icv->threadlimit;

	caps->supports_nested            = 1;
	caps->supports_dynamic           = 1;
	caps->supports_nested_nondynamic = 1;
	caps->max_levels_supported       = -1;     /* No limit */
	caps->default_numthreads         = nthr;
	caps->max_threads_supported      = -1;     /* no limit */

	if (pthread_lib_inited) return (0);

	threadlimit    = icv->threadlimit;
	threadscreated = 0;
	levellimit     = icv->levellimit;
	proc_bind      = icv->proc_bind;

	if (icv->stacksize != -1)
	{
		if ((globalattr = (pthread_attr_t *)
		                  ort_alloc_aligned(sizeof(pthread_attr_t), NULL)) == NULL)
			ort_error(5, "othr_init() failed; could not create stack attributes\n");
		pthread_attr_init(globalattr);
		if (pthread_attr_setstacksize(globalattr, icv->stacksize) != 0)
			ort_error(5, "pthread_attr_setstacksize() failed.\n");
	}

	if (nthr > 0)
	{
#ifdef HAVE_PTHREAD_SETCONCURRENCY
		pthread_setconcurrency(nthr + 1);
#endif

		H = _new_bunch_of_threads(nthr, NULL);     /* Create the initial pool */
		threadscreated = nthr + 1;
		othr_init_lock(&plock, ORT_LOCK_SPIN);
	}

	plen = nthr;
	pthread_lib_inited = 1;
	return (0);
}


void othr_finalize(int exitvalue)
{
}


/* The function executed by each thread */
void *threadjob(void *env)
{
	volatile othr_info *myinfo;
	volatile othr_pool_t *env_t = (othr_pool_t *)env;

#ifdef __SYSOS_solaris
	if (env_t->creation_id != -1)
		processor_bind(P_LWPID, env_t->creation_id, env_t->creation_id - 1, NULL);
#endif

	while (1)
	{
		/* Wait for work */
		WAIT_WHILE(env_t->spin, WORKER_YIELD);

		env_t->spin = 1;                  /* Prepare me for next round */
		ort_ee_dowork(env_t->id, env_t->arg); /* Execute requested code */

		myinfo = env_t->info;             /* Moved this up - thanks to M-CC */
		/* Update the "running" field of my parent's node */
		myinfo->running[env_t->id] = 0;
	}
}


/* Request for "numthr" threads to execute parallelism in level "level" */
int othr_request(int numthr, int level)
{
	int         new, tmpplen;
	othr_pool_t *bunch, *tail;

	if (level == 1)      /* Only the master thread is here, no need for locking */
	{
		if (numthr <= plen)
			plen -= numthr;
		else
		{
			new = numthr - plen;
			if (threadlimit != -1 && new + threadscreated > threadlimit)
				new = threadlimit - threadscreated;

			bunch = _new_bunch_of_threads(new, &tail);
			tail->next = H;
			H = bunch;

			threadscreated += new;
			numthr = plen + new;
			plen = 0;
		}

	}
	else
	{
		if (levellimit != -1 && level > levellimit)
			return 0;
		othr_set_lock(&plock);  /* lock in order to check the limits */
		if (numthr <= plen)
		{
			plen -= numthr;
			othr_unset_lock(&plock);
		}
		else
		{
			new = numthr - plen;
			if (threadlimit != -1 && new + threadscreated > threadlimit)
				new = threadlimit - threadscreated;

			threadscreated += new;
			tmpplen = plen;
			plen = 0;
			othr_unset_lock(&plock);

			if (new == 0) return (tmpplen);

			bunch = _new_bunch_of_threads(new, &tail); /* Augment the pool */
			othr_set_lock(&plock);
			tail->next = H;
			H = bunch;
			othr_unset_lock(&plock);
			numthr = tmpplen + new;
		}
	}
	return (numthr);
}


/* Dispatches numthreads from the pool and gives them work to do */
void othr_create(int numthr, int level, void *arg, void **info)
{
	volatile othr_pool_t *p;
	volatile othr_info   *t = (othr_info *) *info;
	int         i;

	if (t == NULL)  /* Thread becomes a parent for the first time */
	{
		t = (othr_info *) ort_alloc_aligned(sizeof(othr_info), NULL);
		t->running = (volatile int *) ort_alloc_aligned(MAX_BAR_THREADS * sizeof(int),
		                                                NULL);
		t->tp = (othr_pool_t **) ort_alloc_aligned(MAX_BAR_THREADS * sizeof(
		                                             othr_pool_t *), NULL);
	}

	t->team = numthr;
	t->level = level;

	/* Wake up "nthr" threads and give them work to do */
	if (level == 0)
		for (i = 1; i <= numthr; i++)
		{
			p = H;
			H = H->next;

			t->running[i] = 1;
			t->tp[i - 1] = (othr_pool_t *)p;

			p->arg  = arg;
			p->info = (othr_info *)t;
			p->id   = i;
			p->spin = 0;   /* Release thread i */

			FENCE;
		}
	else
		for (i = 1; i <= numthr; i++)
		{
			othr_set_lock(&plock);
			p = H;
			H = H->next;
			othr_unset_lock(&plock);

			t->running[i] = 1;
			t->tp[i - 1] = (othr_pool_t *)p;

			p->arg  = arg;
			p->info = (othr_info *)t;
			p->id   = i;
			p->spin = 0;   /* Release thread i */

			FENCE;
		}

	*info = (othr_info *)t;
}


/* Only the master thread can call this.
 * It blocks the thread waiting for all its children to finish their job.
 */
void othr_waitall(void **info)
{
	volatile othr_info *t = *info;
	int i;
	volatile int *x;

	for (i = 1; i <= t->team; i++)
	{
		x = &(t->running[i]);
		WAIT_WHILE((*x == 1), MASTER_YIELD);
	}

	for (i = 0; i < (t->team) - 1; i++)
		t->tp[i]->next = t->tp[i + 1];              /* Re-enter the pool */

	if (t->level != 0)
		othr_set_lock(&plock);

	t->tp[(t->team) - 1]->next = H;
	H = t->tp[0];

	plen += t->team;

	if (t->level != 0)
		othr_unset_lock(&plock);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                 *
 *  LOCKS (identical to othr_pthreads1)                             *
 *                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


int othr_init_lock(othr_lock_t *lock, int type)
{
	switch (lock->lock.type = type)
	{
		case ORT_LOCK_NEST:
		{
			othr_nestlock_t *l = &(lock->lock.data.nest);

			pthread_mutex_init(&l->ilock, NULL);
			pthread_mutex_init(&l->lock, NULL);
			l->count = 0;
			pthread_cond_init(&l->cond, 0);
			return (0);
		}

		case ORT_LOCK_SPIN:
		{
#ifdef HAVE_SPINLOCKS
			return pthread_spin_init(&(lock->lock.data.spin), 0);
#else
#ifdef PTHREAD_MUTEX_SPINBLOCK_NP                           /* e.g. IRIX */
			static pthread_mutexattr_t spinblock_attr;
			static int firstCall = 1;

			if (firstCall)
			{
				firstCall = 0;
				pthread_mutexattr_init(&spinblock_attr);
				pthread_mutexattr_settype(&spinblock_attr, PTHREAD_MUTEX_SPINBLOCK_NP);
			}
			lock->lock.data.spin.rndelay = 0;
			return pthread_mutex_init(&(lock->lock.data.spin.mutex), &spinblock_attr);
#else
			lock->lock.data.spin.rndelay = 0;
			return pthread_mutex_init(&(lock->lock.data.spin.mutex), NULL);
#endif
#endif
		}

		default: /* ORT_LOCK_NORMAL */
			return pthread_mutex_init(&(lock->lock.data.normal), NULL);
	}
}

int othr_destroy_lock(othr_lock_t *lock)
{
	switch (lock->lock.type)
	{
		case ORT_LOCK_NEST:
		{
			othr_nestlock_t *l = &(lock->lock.data.nest);

			pthread_mutex_destroy(&l->lock);
			pthread_cond_destroy(&l->cond);
			pthread_mutex_destroy(&l->ilock);
			return 0;
		}

		case ORT_LOCK_SPIN:
#ifdef HAVE_SPINLOCKS
			return pthread_spin_destroy(&(lock->lock.data.spin));
#else
			return pthread_mutex_destroy(&(lock->lock.data.spin.mutex));
#endif

		default: /* ORT_LOCK_NORMAL */
			return pthread_mutex_destroy(&(lock->lock.data.normal));
	}
}

int othr_set_lock(othr_lock_t *lock)
{
	switch (lock->lock.type)
	{
		case ORT_LOCK_NEST:
		{
			othr_nestlock_t *l = &(lock->lock.data.nest);
			void            *me = ort_get_current_task();

			pthread_mutex_lock(&l->ilock);
			if (pthread_mutex_trylock(&l->lock) == 0) /* If not locked, lock it */
			{
				l->owner = me;                          /* Get ownership */
				l->count++;
			}
			else
			{
				if (l->owner == me)                     /* Did i do that? */
					l->count++;
				else                                    /* Locked by someone else */
				{
					while (pthread_mutex_trylock(&l->lock))
						pthread_cond_wait(&l->cond, &l->ilock);
					l->owner = me;
					l->count++;
				}
			}
			pthread_mutex_unlock(&l->ilock);
			return (0);
		}

		case ORT_LOCK_SPIN:
		{
#ifdef HAVE_SPINLOCKS
			return pthread_spin_lock(&(lock->lock.data.spin));
#else
#ifdef PTHREAD_MUTEX_SPINBLOCK_NP                             /* e.g. IRIX */
			return pthread_mutex_lock(&lock->lock.data.spin.mutex);
#else
			/* General, portable solution: spin trying with exponential backoff */
			volatile int count, delay, dummy;
			for (delay = lock->lock.data.spin.rndelay;
			     pthread_mutex_trylock(&(lock->lock.data.spin.mutex));)
			{
				for (count = dummy = 0; count < delay; count++)
					dummy += count;      /* To avoid compiler optimizations */
				if (delay == 0)
					delay = 1;
				else
					if (delay < 10000)     /* Don't delay too much */
						delay = delay << 1;
			}
			lock->lock.data.spin.rndelay++;    /* Next thread would wait a bit more */
			return (0);
#endif
#endif
		}

		default: /* ORT_LOCK_NORMAL */
			return pthread_mutex_lock(&(lock->lock.data.normal));
	}
}


int othr_unset_lock(othr_lock_t *lock)
{
	switch (lock->lock.type)
	{
		case ORT_LOCK_NEST:
		{
			othr_nestlock_t *l = &(lock->lock.data.nest);

			pthread_mutex_lock(&l->ilock);
			if (l->owner == ort_get_current_task() && l->count > 0)
			{
				l->count--;
				if (l->count == 0)
				{
					pthread_mutex_unlock(&l->lock);
					pthread_cond_signal(&l->cond);
				}
			}
			pthread_mutex_unlock(&l->ilock);
			return 0;
		}

		case ORT_LOCK_SPIN:
#ifdef HAVE_SPINLOCKS
			return pthread_spin_unlock(&(lock->lock.data.spin));
#else
			lock->lock.data.spin.rndelay = 0;   /* reset it */
			return pthread_mutex_unlock(&(lock->lock.data.spin.mutex));
#endif

		default: /* ORT_LOCK_NORMAL */
			return pthread_mutex_unlock(&(lock->lock.data.normal));
	}
}


int othr_test_lock(othr_lock_t *lock)
{
	switch (lock->lock.type)
	{
		case ORT_LOCK_NEST:
		{
			othr_nestlock_t *l = &(lock->lock.data.nest);
			int res;

			pthread_mutex_lock(&l->ilock);
			if (pthread_mutex_trylock(&l->lock) == 0)
			{
				l->owner = ort_get_current_task();
				res = ++l->count;
			}
			else
				if (l->owner == ort_get_current_task())
					res = ++l->count;
				else
					res = 0;
			pthread_mutex_unlock(&l->ilock);
			return res;
		}

		case ORT_LOCK_SPIN:
#ifdef HAVE_SPINLOCKS
			return pthread_spin_trylock(&(lock->lock.data.spin));
#else
			return (!pthread_mutex_trylock(&(lock->lock.data.spin.mutex)));
#endif

		default: /* ORT_LOCK_NORMAL */
			return (!pthread_mutex_trylock(&(lock->lock.data.normal)));
	}
}
