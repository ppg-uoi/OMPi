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

/* ee_process/oprc.c
 * Implements OpenMP treading through processes
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "../ort.h"
#include "ee.h"

#define MAXSHMLOCKS 100                /* Maximum number of locks */
#define WAIT_WHILE(f) { FENCE; for(;f;) oprc_yield(); }

oprc_lock_t *slock = NULL;             /* User-defined lock */
int slock_id = -1;                     /* Memory ID of the shared lock */
int *lock_counter = NULL;              /* Counter of used locks */
int lock_counter_id = -1;              /* Memory ID of the lock counter */

static int eeproc_inited = 0;        /* Flag to avoid re-initializations */

/* Global stuff for the team */
static void *team_argument;            /* The argument each thread must init */

pid_t *p;
volatile int finished_flag = 0;        /* TRUE notifies that we are done */

pthread_mutex_t waitforparallel_lock, paralleldone_lock;
pthread_cond_t waitforparallel_cv, paralleldone_cv;

int  process_num;
int  ort_argc;
char **ort_argv;


int oprc_pid() {return (0);}


static int pfork(int N)
{
	int i = 0;

	for (; N > 0; N--)
	{
		if (fork() == 0) /* CHILD */
			return (N);
	}
	return (0);        /* PARENT */
}


void oprc_shmfree(int *p)
{
	if (shmctl(*p, IPC_RMID, 0) == -1)
		ort_error(0, "shmem free failed\n");
}


void oprc_shmalloc(void **p , size_t size, int *memid)
{
	*memid = shmget(IPC_PRIVATE, size, 0600 | IPC_CREAT);
	if (*memid == -1)
		ort_error(0, "shmem allocation failed\n");
	*p = shmat(*memid, 0, 0);
	if (p == (void **) - 1)
		ort_error(0, "shmem attach failed\n");
}


int oprc_initialize(int *argc, char ***argv, ort_icvs_t *icv, ort_caps_t *caps)
{
	pid_t id;
	int   nthr;

	/* Allocate space for a maximum allowable number of locks */
	oprc_shmalloc((void **) &lock_counter, (size_t)sizeof(int),
	              (int *) &lock_counter_id);
	oprc_shmalloc((void **) &slock, (size_t) MAXSHMLOCKS * sizeof(oprc_lock_t),
	              (int *) &slock_id);
	*lock_counter = 0;

	nthr = (icv->nthreads > 0) ?  /* Explicitely requested population */
	       icv->nthreads :
	       icv->ncpus - 1;      /* Use a pool of #cpus threads otherwise */
	caps->supports_nested            = 0;
	caps->supports_dynamic           = 1;
	caps->supports_nested_nondynamic = 0;
	caps->max_levels_supported       = 1;
	caps->default_numthreads         = nthr;
	caps->max_threads_supported      = 1 << 30;     /* No limit */
	caps->supports_proc_binding      = 0;

	if (eeproc_inited) return (0);

	   eeproc_inited = 1;
	return (0);
}


void oprc_finalize(int exitvalue)
{
	finished_flag = 1;
	FENCE;
	pthread_mutex_lock(&waitforparallel_lock);
	pthread_cond_signal(&waitforparallel_cv);
	pthread_mutex_unlock(&waitforparallel_lock);
}


/* Request for "numthr" threads to execute parallelism in level "level".
 * We only support 1 level of parallelism.
 */
int oprc_request(int numthr, int level, int oversubscribe)
{
	return ((level == 1) ? numthr : 0);
}


void oprc_create(int numthr, int level, void *arg, void **ignore)
{
	if (numthr <= 0 || level > 1) return;

	process_num = numthr;
	team_argument = arg;
	pthread_mutex_lock(&waitforparallel_lock);
	pthread_mutex_lock(&paralleldone_lock);
	pthread_cond_signal(&waitforparallel_cv);    /* Wake up the initial thread */
	pthread_mutex_unlock(&waitforparallel_lock);
	pthread_cond_wait(&paralleldone_cv, &paralleldone_lock);
	pthread_mutex_unlock(&paralleldone_lock);
}


/* Only the master thread can call this.
 * It blocks the thread waiting for all its children to finish their job.
 */
void oprc_waitall(void **ignore)
{
	int i;
	for (i = 0; i < process_num; i++)
		wait(NULL);
}


int oprc_bindme(int **places, int pindex)
{
	return (-1); /* Binding is unavailable */
}


int oprc_getselfid(void)
{
        return getpid();
}


void *oprc_getself(unsigned int *size)
{
        return NULL;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                 *
 *  LOCKS                                                          *
 *                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void *oprc_init_lock(oprc_lock_t *lock, int type)
{
	if (lock == (oprc_lock_t *) - 1)
	{
		lock = (slock + (*lock_counter));
		*lock_counter = ((*lock_counter) + 1) % 100;
		FENCE;
	}

	switch (lock->lock.type = type)
	{
		case ORT_LOCK_NEST:
		{
			lock->lock.val = 0;
			lock->lock.count = 0;
			return (lock);
		}

		default: /* ORT_LOCK_NORMAL && ORT_LOCK_SPIN*/
		{
			lock->lock.val = 0;
			return (lock);
		}
	}
}


int oprc_destroy_lock(oprc_lock_t *lock)
{
	return 0;
}


int oprc_set_lock(oprc_lock_t *lock)
{
	switch (lock->lock.type)
	{
		case ORT_LOCK_NEST:
		{
			if (_cas(&(lock->lock.val), 0, 1))
			{
				lock->lock.owner = getpid(); /* Get ownership */
				lock->lock.count++;
			}
			else
			{
				if (lock->lock.owner == getpid())  /* Did i do it? */
					lock->lock.count++;
				else                                    /* Locked by someone else */
				{
#if defined(HAVE_ATOMIC_CAS)
					while (! _cas(&(lock->lock.val), 0, 1)) {}
#endif
					lock->lock.owner = getpid();
					lock->lock.count++;
				}
			}
			return (0);
		}

		default: /* ORT_LOCK_NORMAL && ORT_LOCK_SPIN*/
		{
#if defined(HAVE_ATOMIC_CAS)
			while (! _cas(&(lock->lock.val), 0, 1)) {}
#endif
			return 0;
		}
	}
}


int oprc_unset_lock(oprc_lock_t *lock)
{
	switch (lock->lock.type)
	{
		case ORT_LOCK_NEST:
		{
			if (lock->lock.owner == getpid() && lock->lock.count > 0)
				lock->lock.count--;

			if (lock->lock.owner == getpid() && lock->lock.count == 0)
				lock->lock.val = 0;
			return 0;
		}

		default: /* ORT_LOCK_NORMAL && ORT_LOCK_SPIN*/
		{
			lock->lock.val = 0;
			FENCE;
			return 0;
		}
	}
}


int oprc_test_lock(oprc_lock_t *lock)
{
	int delay = 1, nest_delay = 1;

	switch (lock->lock.type)
	{
		case ORT_LOCK_NEST:
		{
			if (_cas(&(lock->lock.val), 0, 1))
			{
				lock->lock.owner = getpid(); /* Get ownership */
				lock->lock.count++;
			}
			else
			{
				if (lock->lock.owner == getpid())  /* Did i do it? */
					lock->lock.count++;
				else                                    /* Locked by someone else */
				{
#if defined(HAVE_ATOMIC_CAS)
					while (! _cas(&(lock->lock.val), 0, 1)) {}
#endif
					lock->lock.owner = getpid();
					lock->lock.count++;
				}
			}
			return (0);
		}

		default: /* ORT_LOCK_NORMAL && ORT_LOCK_SPIN*/
		{
#if defined(HAVE_ATOMIC_CAS)
			while (! _cas(&(lock->lock.val), 0, 1)) {}
#endif
			return 1;
		}
	}
}


/* The function of the runner thread */
void *oprc__ompi_main(void *ignore)
{
	extern int __ompi_main(int argc, char **argv);
	__ompi_main(ort_argc, ort_argv);
	return NULL;
}


/* main(): the initial thread
 * It's sole purpose is to wait for parallel regions and then fork processes
 * to execute the region. 
 * It also creates the runner thread.
 */
int main(int argc, char **argv)
{
	pthread_attr_t tattr;
	pthread_mutexattr_t mattr;
	pthread_t tid;
	int ret = 0, memid, id;
	void *arg;

	ort_argc = argc;
	ort_argv = argv;
	
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&waitforparallel_lock, &mattr);
	pthread_mutex_init(&paralleldone_lock, &mattr);
	pthread_cond_init(&waitforparallel_cv, NULL);
	pthread_cond_init(&paralleldone_cv, NULL);

	/* Allocate space for the runner's stack */
	oprc_shmalloc((void **) &p, (size_t) 1024 * 1024, (int *) &memid);
	ret = pthread_attr_init(&tattr);
	if (ret)
		ort_error(0, "pthread_attr_init error\n");
	ret = pthread_attr_setstack(&tattr, p, 1024 * 1024);
	if (ret)
		ort_error(0, "pthread_attr_setstackaddr error\n");

	/* Create the runner thread to execute the original main */
	pthread_mutex_lock(&waitforparallel_lock);
	ret = pthread_create(&tid, &tattr, oprc__ompi_main, NULL);
	if (ret != 0)
		ort_error(0, "pthread_create error %d\n", ret);

	while (!finished_flag)
	{
		/* Block till the next parallel region */
		pthread_cond_wait(&waitforparallel_cv, &waitforparallel_lock);
		pthread_mutex_unlock(&waitforparallel_lock);
		FENCE;

		if (!finished_flag)
		{
			id = pfork(process_num);             /* fork; all children will execute */
			if (id != 0)
			{
				ort_ee_dowork(id, team_argument);  /* execute the requested code */
				exit(0);
			}
		}
		pthread_mutex_lock(&paralleldone_lock);
		pthread_mutex_lock(&waitforparallel_lock);
		pthread_cond_signal(&paralleldone_cv);            /* wakeup the runner thread */
		pthread_mutex_unlock(&paralleldone_lock);
	}

	oprc_shmfree(&memid);
	oprc_shmfree(&lock_counter_id);
	oprc_shmfree(&slock_id);

	pthread_attr_destroy(&tattr);
	pthread_mutex_destroy(&waitforparallel_lock);
	pthread_mutex_destroy(&paralleldone_lock);

	pthread_cond_destroy(&waitforparallel_cv);
	pthread_cond_destroy(&paralleldone_cv);

	exit(0);
}

