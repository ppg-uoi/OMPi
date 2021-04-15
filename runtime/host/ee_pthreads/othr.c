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

/* Jan, 2020
 *   New env variable OMPI_PTHREADS_MAXTHR to set a total maximum number
 *   of running threads. We needed this since thread_limit is now a
 *   per-league limit, not an overall limit.
 * Mar. 2019
 *   New env variable OMPI_PTHREADS_POLICY to select thread handling,
 *   i.e. whether to use a (possibly pre-created) thread pool or not.
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
 * A simple, thread library for (probably inefficient) multilevel parallelism.
 *
 * There are two operating modes: threadjoin or pool [variable "keepthreads"].
 * In the first case, threads die when the team terminates.
 * In the second case, threads join a pool and get re-used in subsequent teams.
 * The pool can be pre-created in the beggining or not. In addition, when ORT
 * requests threads and the pool has not enough to supply, it is selectable
 * whether new threads will be created (certainly causing oversubscription) or
 * only the existing ones will be employed.
 *
 * Check the environmental variable OMPI_PTHREADS_POLICY.
 *
 * It uses pure PTHREADS and a pool of threads, endlessly spinning for work.
 * The pool is of fixed size.
 */

#include "config.h"

#if defined(HAVE_PTHREAD_AFFINITY) && (defined(__SYSCOMPILER_gcc) || \
                   defined(__SYSCOMPILER_intel) || defined(__SYSCOMPILER_mpicc))
	#define _GNU_SOURCE
#endif

#if defined(HAVE_GETTID)
	#include <sys/types.h>
#elif defined(HAVE_GETTID_SYSCALL)
	#define _GNU_SOURCE
	#include <unistd.h>
	#include <sys/syscall.h>
#endif

#include "../ort.h"
#include <stdlib.h>
#include <stdio.h>
#include "ee.h"

#if defined(__SYSCOMPILER_sun)
	#undef HAVE_PTHREAD_AFFINITY   /* Problem with CPU_SET */
#endif

#ifdef HAVE_PTHREAD_AFFINITY
	#include <sched.h>
#endif

#ifdef __SYSOS_solaris
	#include <sys/pset.h>
	#include <sys/types.h>
	#include <sys/processor.h>
	#include <sys/procset.h>
	#include <unistd.h>
#endif

/* Yield timeouts */
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

/* Threshold beyond thrinfo_t arrays are reduced */
#define ALLOC_THRESHOLD    16

/* Thread types:
 * a) FORKJOIN threads are used if the user does not want a pool;
 *    every created team is destroyed at the end of the parallel region.
 *    That is, forkjoin threads die; and they become joinable.
 * b) PERSISTENT threads never die; when done, they enter a pool of threads
 *    ready to be utilized in subsequent parallel regions.
 * c) TRANSIENT threads function like forkjoin threads but are only used
 *    when persistent threading is in effect. They are created only when
 *    the creation of more persistent threads is about to cause
 *    oversubscription of the processors; they are created just to execute
 *    the particular parallel regiona and then they die. In contrast to
 *    forkjoin threads, they are not joinable.
 * By default, we use peristent threading (which utilizes transient threads
 * whenever needed). To enforce forkjoin threading, one must set
 * OMPI_PTHREADS_POLICY to "kill".
 */
#define PERSISTENT 0
#define TRANSIENT  1
#define FORKJOIN   2


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                 *
 *  TYPES, GLOBALS, UTILITIES                                      *
 *                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* TODO: check padding (it shows as 128 bytes but you never know) */
typedef struct othr_pool_s
{
	void *arg;                    /* Argument for the thread function */
	void *info;
	int id;                       /* A sequential id given by OMPI */
	int type;
	struct  othr_pool_s *next;    /* Next node into the list */
	volatile short  wait;         /* Block/spin here waiting for work */
	pthread_cond_t  wait_cond;    /* For passive policy */
	pthread_mutex_t wait_mut;

#ifdef __SYSOS_solaris
	int creation_id;
#endif
} thrpool_t;

typedef struct
{
	int level;
	volatile int *running;                /* # running threads in a team */
	pthread_t  *tids;              /* the thread ids of FORKJOIN threads */
	thrpool_t  **tp;
	pthread_t *actual_run_ptr;   /* Pointer returned by calloc, realloc. */
	thrpool_t *actual_tid_ptr;   /* Pointer returned by calloc, realloc. */
	int teamsize;
	int alloc_size;
} thrinfo_t;

static int          pthread_lib_inited = 0;  /* To avoid re-initializations */
static int          keepthreads = true;      /* Retired threads form a pool */
static thrpool_t    *H = NULL;                  /* Pool = a list of threads */
static volatile int plen;                          /* # threads in the pool */
static othr_lock_t  plock;                 /* A lock for accessing the pool */

/* OpenMP 3.0/3.1 stuff */
static int            threadlimit, threadsalive, proc_bind, waitpolicy;
static int            throttling;  /* Oversubscription threshold */
static ort_icvs_t     *ompi_icv; /* OpenMP 3.0 - User can change some values */
static pthread_attr_t *globalattr = NULL;      /* For stacksize */

#ifdef HAVE_PTHREAD_AFFINITY

/* Binding info */
static volatile int bind_first_cpu = -1, bind_next_cpu = 0, bind_cpu_limit;
static int *cpus_onln;

/**
 * Allocates and initializes array 'cpus_onln', returns array size (# of cpus)
 * and the index of the cpu the initial thread executes on (passed by reference)
 */
static
int init_cpus_onln(cpu_set_t *set, volatile int *first_cpu)
{
	int i, ncpus, first_cpu_index;

	cpus_onln = (int *) ort_alloc(sizeof(int) * CPU_COUNT(set));
	for (i = ncpus = 0; i < CPU_SETSIZE; i++)
		if (CPU_ISSET(i, set))
		{
			if (*first_cpu == i)
				first_cpu_index = ncpus;
			cpus_onln[ncpus++] = i;
		};
	*first_cpu = first_cpu_index;
	return ncpus;
}

#endif /* HAVE_PTHREAD_AFFINITY */


/* The function executed by persistent threads
 */
void *persistent_thread(void *env)
{
	volatile thrinfo_t *myinfo;
	volatile thrpool_t *env_t = (thrpool_t *) env;

#ifdef __SYSOS_solaris
	if (env_t->creation_id != -1)
		processor_bind(P_LWPID, env_t->creation_id, env_t->creation_id - 1, NULL);
#endif

	for (;;)
	{
		/* Wait for work */
		if (waitpolicy == _OMP_ACTIVE)
		{
			WAIT_WHILE(env_t->wait, WORKER_YIELD);
			env_t->wait = 1;                      /* Prepare me for next round */
		}
		else
		{
			pthread_mutex_lock((pthread_mutex_t *) &env_t->wait_mut);
			while (env_t->wait != 0)
				pthread_cond_wait((pthread_cond_t *) &env_t->wait_cond,
				                  (pthread_mutex_t *) &env_t->wait_mut);
			env_t->wait = 1;
			pthread_mutex_unlock((pthread_mutex_t *) &env_t->wait_mut);
		}

		ort_ee_dowork(env_t->id, env_t->arg); /* Execute requested code */

		myinfo = env_t->info;                 /* Moved this up - thanks to M-CC */
		myinfo->running[env_t->id] = 0;       /* Notify my parent I'm done */
	}
}


/* The function executed by transient threads
 * (the same as the persistent threads, without the infinite loop).
 */
void *transient_thread(void *env)
{
	volatile thrinfo_t *myinfo;
	volatile thrpool_t *env_t = (thrpool_t *) env;

#ifdef __SYSOS_solaris
	if (env_t->creation_id != -1)
		processor_bind(P_LWPID, env_t->creation_id, env_t->creation_id - 1, NULL);
#endif

	/* Wait for work */
	if (waitpolicy == _OMP_ACTIVE)
	{
		WAIT_WHILE(env_t->wait, WORKER_YIELD);
		env_t->wait = 1;       /* Not needed for transient threads */
	}
	else
	{
		pthread_mutex_lock((pthread_mutex_t *) &env_t->wait_mut);
		while (env_t->wait != 0)
			pthread_cond_wait((pthread_cond_t *) &env_t->wait_cond,
			                  (pthread_mutex_t *) &env_t->wait_mut);
		env_t->wait = 1;
		pthread_mutex_unlock((pthread_mutex_t *)&env_t->wait_mut);
	}

	ort_ee_dowork(env_t->id, env_t->arg); /* Execute requested code */

	myinfo = env_t->info;                 /* Moved this up - thanks to M-CC */
	myinfo->running[env_t->id] = 0;       /* Notify my parent I'm done */

	ort_ee_cleanup();                     /* Cleanup (necessary!) */
	return NULL;
}


/* The function executed by forkjoin threads
 */
void *forkjoin_thread(void *env)
{
	volatile thrpool_t *env_t = (thrpool_t *) env;

#ifdef __SYSOS_solaris
	if (env_t->creation_id != -1)
		processor_bind(P_LWPID, env_t->creation_id, env_t->creation_id - 1, NULL);
#endif

	ort_ee_dowork(env_t->id, env_t->arg);    /* Execute requested code */
	ort_ee_cleanup();                        /* Cleanup (necessary!) */
	return NULL;
}


/**
 * Create a number of possibly blocked, possibly bound threads.
 * @param n       the number of threads to create
 * @param thrtype the type of threads (PERSISTENT, TRANSIENT, FORKJOIN)
 * @param tail    the pool of threads to join (PERSISTENT & TRANSIENT threads)
 *                the argument to give to each thread (FORKJOIN threads)
 * @return        the new head of the pool (n/a in FORKJOIN threads)
 */
static
thrpool_t *_new_bunch_of_threads(int n, int thrtype, thrpool_t **tail)
{
	pthread_t thr, *thrids;
	volatile thrpool_t *first_node, *node, *prev_node;
	int         i, e, NPC;
	static int  NP;
	pthread_attr_t localattr;

#ifdef __SYSOS_solaris
	static int created_threads = 0;
#endif

#ifdef HAVE_PTHREAD_AFFINITY
	int        j, local_next_cpu;
	cpu_set_t  cpuset, *cpusetptr;
	static cpu_set_t fullset;
	static int gotaffinity = 1; /* Whether pthread_getaffinity_np succeeded */
	// TODO move bind_first_cpu declaration here

	if (proc_bind && H == NULL) /* 1st call from initial thread; find its cpu */
	{
		if (globalattr == NULL)   /* In case the user didn't set the stacksize */
		{
			globalattr = (pthread_attr_t *) malloc(sizeof(pthread_attr_t));
			pthread_attr_init(globalattr);
		}

		/* prepare a full cpuset */
		if ( pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &fullset) )
		{
			ort_warning("pthread_getaffinity_np() failed;\n\t could not get CPU "
			            "affinity mask of the initial thread\n");
			NPC = ort_get_num_procs_conf();
			gotaffinity = 0;
			CPU_ZERO(&fullset);
			for (j = 0; j < NPC && j < CPU_SETSIZE; j++)
				CPU_SET(j, &fullset);
		}

		if ((bind_first_cpu = sched_getcpu()) < 0)
		{
			if (!gotaffinity)
				bind_first_cpu = 0;   /* Assume initial thread @cpu0 */
			else
				for (j = 0; j < CPU_SETSIZE; j++)
					if (CPU_ISSET(j, &fullset))
					{
						bind_first_cpu = j;
						break;
					};
		}

		NP = init_cpus_onln(&fullset, &bind_first_cpu);
		bind_next_cpu = bind_first_cpu + 1;
		bind_cpu_limit = bind_first_cpu + NP;
		CPU_ZERO(&cpuset);
		CPU_SET(cpus_onln[bind_first_cpu], &cpuset);
		if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0)
			ort_warning("pthread_getaffinity_np() failed;\n\t could not bind "
			            "the initial thread\n");
	}
#endif

	/* Get a local copy of thread attributes */
	if (globalattr != NULL)
		localattr = *globalattr;
	else /* in case no global attribute exists, initialize the local copy */
		pthread_attr_init(&localattr);

	/* All transient threads are created in detached mode */
	if (thrtype == TRANSIENT)
		pthread_attr_setdetachstate(&localattr, PTHREAD_CREATE_DETACHED);

	if (thrtype == FORKJOIN)                      /* Get the table of ids */
		thrids = ort_alloc(n*sizeof(pthread_t));

	for (i = 0; i < n; i++)
	{
		node = (thrpool_t *) ort_alloc(sizeof(thrpool_t));
		if (i == 0)
			first_node = prev_node = node;
		else
		{
			prev_node->next = (thrpool_t *) node;
			prev_node = node;
		}

#ifdef HAVE_PTHREAD_AFFINITY
		/* If there are cpus without bound threads */
		if (proc_bind)
		{
			cpusetptr = &fullset;
			if (bind_next_cpu < bind_cpu_limit)
			{
				/* Get the next available free cpu id and store it in a local copy */
#if defined(HAVE_ATOMIC_FAA)
				local_next_cpu = _faa(&(bind_next_cpu), 1);
#else
				ee_set_lock(&plock);
				local_next_cpu = bind_next_cpu;
				bind_next_cpu++;
				ee_unset_lock(&plock);
#endif

				/* Re-check the value of cpu to bind */
				if (local_next_cpu < bind_cpu_limit)
					if (gotaffinity && CPU_ISSET((cpus_onln[local_next_cpu % NP]), &fullset))
					{
						CPU_ZERO(&cpuset);
						CPU_SET((cpus_onln[local_next_cpu % NP]), &cpuset);
						cpusetptr = &cpuset;
					};
			}

			if ( pthread_attr_setaffinity_np(&localattr,sizeof(cpu_set_t),cpusetptr) )
			{
				if (cpusetptr != &fullset)
					ort_warning("could not set affinity mask of thread %d to cpu %d\n",
					            i, cpus_onln[local_next_cpu % NP]);
				else
					ort_warning("could not set affinity mask of thread %d\n", i);
			}
		}
#endif

		if (waitpolicy != _OMP_ACTIVE)
		{
			pthread_mutex_init((pthread_mutex_t *) &node->wait_mut, NULL);
			pthread_cond_init((pthread_cond_t *) &node->wait_cond, NULL);
		}
		node->wait = 1;
		node->type = thrtype;
		if (thrtype == FORKJOIN)
		{
			node->id = i+1;                 /* other thread types get an id later */
			node->arg = tail;               /* hack ;-) */
		}
		FENCE;

		if ((e = pthread_create(&thr, &localattr,
		                        (thrtype == PERSISTENT) ? persistent_thread :
		                        ((thrtype == TRANSIENT) ? transient_thread :
		                                                  forkjoin_thread),
		                        (void *) node)))
			ort_error(5, "pthread_create() failed with %d\n", e);

		(thrtype == FORKJOIN) && (thrids[i] = thr);

#ifdef __SYSOS_solaris
		if (created_threads < NP)
		{
#if defined(HAVE_ATOMIC_FAA)
			node->creation_id = _faa(&(created_threads), 1) + 1;
#else
			othr_set_lock(&plock);
			node->creation_id = ++created_threads;
			othr_unset_lock(&plock);
#endif
		}
		else
			node->creation_id = -1;
#endif
	}

	node->next = NULL;
	if (tail)
		*tail = (thrpool_t *) node;
	if (thrtype == FORKJOIN)
		return ((thrpool_t *) thrids);
	else
		return ((thrpool_t *) first_node);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                 *
 *  MAIN THREADING INTERFACE                                       *
 *                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void _othr_info()  /* Useless for now */
{
	puts("OMPI_PTHREADS_MAXTHR : overall limit of running threads [ <int> ]");
  puts("OMPI_PTHREADS_POLICY : threads lifetime [ pool|prepool|kill ]");
}


/* Library initialization
 */
int othr_initialize(int *argc, char ***argv, ort_icvs_t *icv, ort_caps_t *caps)
{
	int  nthr;
	bool prepool;
	char *s;

	nthr = (icv->nthreads >= 0) ?  /* Explicitely requested population */
	       icv->nthreads :   /* this does not include the initial thread */
	       icv->ncpus - 1;   /* we don't handle the initial thread */

	if (icv->threadlimit != -1 && nthr > icv->threadlimit)
		nthr = icv->threadlimit;

	caps->supports_nested            = 1;
	caps->supports_dynamic           = 1;
	caps->supports_nested_nondynamic = 1;
	caps->max_levels_supported       = 1<<30;  /* infinity */
	caps->default_numthreads         = nthr;
	caps->max_threads_supported      = 1<<30;  /* infinity */
#ifdef HAVE_PTHREAD_AFFINITY
	caps->supports_proc_binding      = 1;
#else
	caps->supports_proc_binding      = 0;
#endif

	if (pthread_lib_inited) return (0);

	threadsalive = 0;
	waitpolicy     = icv->waitpolicy;
	proc_bind      = icv->proc_bind;
	ompi_icv       = icv;
	throttling     = icv->ncpus;        /* Oversubscription threshold */

	if (icv->stacksize != -1)
	{
		globalattr = (pthread_attr_t *)
		             ort_alloc_aligned(sizeof(pthread_attr_t), NULL);
		pthread_attr_init(globalattr);
		if (pthread_attr_setstacksize(globalattr, icv->stacksize) != 0)
			ort_error(5, "pthread_attr_setstacksize() failed.\n");
	}

	/* Default behavior; but be flexible */
	keepthreads = true;     /* Mainain a thread pool */
	prepool = false;        /* but don't create it in advance */
	if ((s = getenv("OMPI_PTHREADS_POLICY")) != NULL)
	{
		if (strncasecmp(s, "KILL", 4) == 0)
			keepthreads = false;
		else
			if (strncasecmp(s, "PREPOOL", 7) == 0)
				prepool = true;
	}
	threadlimit = -1;
	if ((s = getenv("OMPI_PTHREADS_MAXTHR")) != NULL)
		threadlimit = atoi(s);
	if (threadlimit < 1)
		threadlimit = 1<<30;

#ifdef HAVE_PTHREAD_SETCONCURRENCY
	if (nthr > 0)
		pthread_setconcurrency(icv->ncpus);
	else
		pthread_setconcurrency(1);
#endif

	othr_init_lock(&plock, ORT_LOCK_SPIN);

	if (keepthreads && prepool && nthr > 0)
	{
		/* Create the initial pool of persistent threads */
		H = _new_bunch_of_threads(nthr, PERSISTENT, NULL);
		threadsalive = nthr+1;   /* +1 for the initial thread */
		plen = nthr;
	}
	else
	{
		threadsalive = 1;
		plen = 0;
	}

	pthread_lib_inited = 1;
	return (0);
}


void othr_finalize(int exitvalue)
{
}


/**
 * Request for a number of threads to execute parallelism in a certain level
 * @param numthr the requested number of threads
 * @param level  the level of parallelism
 * @param oversubscribe if false, no new threads will be created; only threads
 *               from the pool will be used
 * @return       the actual number of threads the library can provide
 */
int othr_request(int numthr, int level, int oversubscribe)
{
	int        new, ntrans, tmpplen;
	thrpool_t *bunch, *tail;

	if (numthr <= 0) return (0);

	if (level == 1) /* Initial thread; no need for locking */
	{
		if (numthr <= plen)          /* We have enough threads already */
			plen -= numthr;
		else
			if (oversubscribe == 0)    /* Not enough; but create conservatively */
			{
				if (threadsalive < throttling)    /* don't oversubscribe */
				{
					new = numthr - plen;   /* Need that many more ideally */
					if (new > throttling - threadsalive)    /* but force limits */
						new = throttling - threadsalive;
					if (new > threadlimit)
						new = threadlimit;
					if (keepthreads)
					{
						bunch = _new_bunch_of_threads(new, PERSISTENT, &tail);
						tail->next = H;
						H = bunch;
						plen += new;
						threadsalive += new;
					}
				}
				numthr = plen;
				plen = 0;
			}
			else                       /* Create as many as needed */
			{
				new = numthr - plen;
				if (new + threadsalive > threadlimit)
				{
					new = threadlimit - threadsalive;
					if (new < 0)
						new = 0;
				}

				if (!keepthreads)        /* Forkjoin threads */
					return new;

				ntrans = (new+threadsalive > throttling) ? /* Excess are transient */
				            new + threadsalive - throttling : 0;
				if (new-ntrans > 0)
				{
					bunch = _new_bunch_of_threads(new - ntrans, PERSISTENT, &tail);
					tail->next = H;
					H = bunch;
				}
				if (ntrans)                                 /* The remaining threads */
				{                                           /* will be transient */
					bunch = _new_bunch_of_threads(ntrans, TRANSIENT, &tail);
					tail->next = H;
					H = bunch;
				}
				threadsalive += new;
				numthr = plen + new;
				plen = 0;
			}
		return numthr;
	}

	/* level >= 2 */
	if (level > ompi_icv->levellimit)
		return 0;

	othr_set_lock(&plock);  /* lock in order to check the limits */
	if (numthr <= plen)
	{
		plen -= numthr;
		othr_unset_lock(&plock);
	}
	else
		if (oversubscribe == 0)
		{
			if (threadsalive < throttling)    /* don't oversubscribe */
			{
				new = numthr - plen;   /* Need that many more ideally */
				if (new > throttling - threadsalive)    /* but force limits */
					new = throttling - threadsalive;
				if (new > threadlimit)
					new = threadlimit;
				if (keepthreads)
				{
					othr_unset_lock(&plock);
					bunch = _new_bunch_of_threads(new, PERSISTENT, &tail);
					othr_set_lock(&plock);
					tail->next = H;
					H = bunch;
					plen += new;
					threadsalive += new;
				}
			}
			numthr = plen;
			plen = 0;
			othr_unset_lock(&plock);
		}
		else
		{
			new = numthr - plen;
			if (new + threadsalive > threadlimit)
			{
				new = threadlimit - threadsalive;
				if (new < 0)
					new = 0;
			}
			ntrans = (new+threadsalive > throttling) ?   /* Excess are transient */
			               new + threadsalive - throttling : 0;
			if (keepthreads)
			{
				threadsalive += new;
				tmpplen = plen;
				plen = 0;
			}
			othr_unset_lock(&plock);

			if (!keepthreads)
				return new;
			if (new == 0)
				return (tmpplen);

			if (new-ntrans > 0)
			{
				bunch = _new_bunch_of_threads(new - ntrans, PERSISTENT, &tail);
				othr_set_lock(&plock);
				tail->next = H;
				H = bunch;
				othr_unset_lock(&plock);
			}
			if (ntrans)                                 /* The remaining threads */
			{                                           /* will be transient */
				bunch = _new_bunch_of_threads(ntrans, TRANSIENT, &tail);
				othr_set_lock(&plock);
				tail->next = H;
				H = bunch;
				othr_unset_lock(&plock);
			}

			numthr = tmpplen + new;
		}
	return numthr;
}


/**
 * Dispatches numthr threads from the pool (or creates numthr FORKJOIN threads)
 * to execute a parallel region.
 * @param numthr the numer of threads
 * @param level  the level of parallelism
 * @param arg    an opaque argument that threads should pass to ort_ee_dowork()
 * @param info   (return) stored at the master for controling the threads
 */
void othr_create(int numthr, int level, void *arg, void **info)
{
	volatile thrpool_t *p;
	volatile thrinfo_t *t = (thrinfo_t *) *info;
	int                i;

	if (t == NULL)  /* Thread becomes parent for the first time */
	{
		t = (thrinfo_t *) ort_calloc_aligned(sizeof(thrinfo_t), NULL);
		// t->alloc_size = 0;         // Not required due to calloc
		// t->actual_run_ptr = NULL;  // Not required due to calloc
		// t->actual_tid_ptr = NULL;  // Not required due to calloc
	}

	if ((numthr > t->teamsize) || ((t->alloc_size >= ALLOC_THRESHOLD) &&
				(numthr <= (t->alloc_size >> 1))))
	{
		t->running = (volatile int *)
		             ort_realloc_aligned(numthr * sizeof(int),
					     (void **) &(t->actual_run_ptr));
		t->tp = (thrpool_t **)
		             ort_realloc_aligned(numthr * sizeof(thrpool_t *),
					     (void **) &(t->actual_tid_ptr));
		t->alloc_size = numthr;
	}

	t->teamsize = numthr;
	t->level = level;

	if (!keepthreads)   /* Create threads and return */
	{
		(level > 1) && othr_set_lock(&plock);
		threadsalive += numthr;
		(level > 1) && othr_unset_lock(&plock);
		t->tids = (pthread_t *)
		          _new_bunch_of_threads(numthr, FORKJOIN, (thrpool_t **) arg);
		if (info)
			*info = (thrinfo_t *) t;
		return;
	}

	/* Wake up "numthr" threads and give them work to do; lock even at level 0 */
	for (i = 1; i <= numthr; i++)
	{
		othr_set_lock(&plock);
		p = H;
		H = H->next;
		othr_unset_lock(&plock);

		t->running[i] = 1;
		t->tp[i - 1] = (thrpool_t *) p;

		p->arg  = arg;
		p->info = (thrinfo_t *) t;
		p->id   = i;

		if (waitpolicy == _OMP_ACTIVE)
		{
			FENCE;
			p->wait = 0;   /* Release thread i */
		}
		else
		{
			pthread_mutex_lock((pthread_mutex_t *) &p->wait_mut);
			p->wait = 0;
			pthread_cond_signal((pthread_cond_t *) &p->wait_cond);
			pthread_mutex_unlock((pthread_mutex_t *) &p->wait_mut);
		}
	}

	if (info)
		*info = (thrinfo_t *) t;
}


/*
 * TODO: Implement the following: free info that is known by ort.
 */
void othr_free_info(void **info)
{
	;
}


/* Only the master thread can call this.
 * It blocks the thread, waiting for all its children to finish their job.
 */
void othr_waitall(void **info)
{
	volatile thrinfo_t *t = *info;
	thrpool_t    *normal_list_tail = NULL, *normal_list_head = NULL;
	int          i, threads_entered = 0, temp_threads = 0;
	volatile int *x;

	if (!keepthreads)           /* FORKJOIN threads */
	{
		for (i = 1; i <= t->teamsize; i++)
			pthread_join(t->tids[i-1], NULL);
		othr_set_lock(&plock);
		threadsalive -= (t->teamsize - 1);
		othr_unset_lock(&plock);
		free(t->tids);            /* no longer needed */
		return;
	}

	for (i = 1; i <= t->teamsize; i++)
	{
		x = &(t->running[i]);
		WAIT_WHILE((*x == 1), MASTER_YIELD);
	}

	for (i = 0; i < t->teamsize ; i++)
	{
		if (threads_entered == 0 && t->tp[i]->type == PERSISTENT) /* the 1st one */
		{
			normal_list_head = t->tp[i];              /* Store first normal thread */
			normal_list_tail = normal_list_head;
			threads_entered ++;
		}
		else  /* Enqueue persistent threads in the same order for locality */
		{
			if (t->tp[i]->type == PERSISTENT)
			{
				normal_list_tail->next = t->tp[i];       /* Store next normal thread */
				normal_list_tail = normal_list_tail->next;
				threads_entered ++;
			}
			else
			{
				free(t->tp[i]);                      /* Free transient thread's data */
				temp_threads++;
			}
		}
	}

	if (normal_list_head != NULL)
	{
		if (t->level != 0)
			othr_set_lock(&plock);

		normal_list_tail->next = H;              /* Put them back in the pool */
		H = normal_list_head;
		plen += threads_entered;

		if (t->level != 0)
			othr_unset_lock(&plock);
	}

	if (temp_threads > 0)
	{
		if (t->level != 0)
			othr_set_lock(&plock);

		threadsalive -= temp_threads;          /* Just reduce the # of threads */

		if (t->level != 0)
			othr_unset_lock(&plock);
	}
}


/**
 * Binds calling thread to any member of the given place
 * @param places The array of places
 * @param pindex The place to bind to
 * @return       pindex if successful, < 0 otherwise
 */
int othr_bindme(int **places, int pindex)
{
#ifndef HAVE_PTHREAD_AFFINITY
	return (BIND_UNAVAILABLE); /* Binding is unavailable */
#else
	int j;
	cpu_set_t cpuset;

	CPU_ZERO(&cpuset);
	if (!places)
		ort_warning("ee_bindme() call with NULL place partition ptr; ignoring\n");
	else
		if (pindex < 0 || pindex >= numplaces(places))
			ort_warning("ee_bindme() call for illegal place (%d); ignoring\n",pindex);
		else
		{
			for (j = 0; j < placelen(places, pindex); j++) /* placemembers2bitset */
				CPU_SET(places[pindex+1][j+1], &cpuset);
			if (pthread_setaffinity_np(pthread_self(),sizeof(cpu_set_t),&cpuset) == 0)
				return (pindex);
			else
				ort_warning("othr_bindme() failed; could not bind thread\n");
		}
	return (BIND_ERROR); /* Binding failure */
#endif
}


int othr_getselfid(void)
{
#if defined(HAVE_GETTID)
	return gettid();
#elif defined(HAVE_GETTID_SYSCALL)
	return syscall(SYS_gettid);
#else
	return (-1); // Invalid ID
#endif
}


void *othr_getself(unsigned int *size)
{
        return NULL;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                 *
 *  LOCKS (identical to othr_pthreads1)                            *
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
