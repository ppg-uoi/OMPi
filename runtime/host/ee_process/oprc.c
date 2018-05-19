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
#define WAIT_WHILE(f) { FENCE; for(;f;) oprc_yield(); }

oprc_lock_t *slock = NULL;             /* User-defined lock */
int slock_id = -1;                     /* Memory ID of the shared lock */
int *number_of_lock = NULL;            /* Counter of used locks */
int number_of_lock_id = -1;            /* Memory ID of the lock counter */

static int proc_lib_inited = 0;     /* Flag to avoid re-initializations */
static int num_created;                /* How many threads exist in total */

/* Global stuff for the team */
static void *team_argument;            /* The argument each thread must init */

pid_t *p;
volatile int flag = 0;

pthread_mutex_t count_mutex1, count_mutex2;
pthread_cond_t flag1, flag2;

int number_of_process;
int ort_argc;
char **ort_argv;

int oprc_pid() {return (0);}

int pfork(int N)
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
	{
		printf("Shared Memory Free error\n");
		exit(0);
	}
}

void oprc_shmalloc(void **p , size_t size, int *memid)
{
	*memid = shmget(IPC_PRIVATE, size, 0600 | IPC_CREAT);

	if (*memid == -1)
	{
		printf("Shared Memory allocation error\n");
		exit(0);
	}

	*p = shmat(*memid, 0, 0);
	if (p == (void **) - 1)
	{
		printf("Shared Memory attach error\n");
		exit(0);
	}
}


/* Library initialization.
 */
int oprc_initialize(int *argc, char ***argv, ort_icvs_t *icv, ort_caps_t *caps)
{
	pid_t id;
	int   nthr;

	oprc_shmalloc((void **)&number_of_lock, (size_t)sizeof(int),
	              (int *) &number_of_lock_id);
	oprc_shmalloc((void **)&slock, (size_t)100 * sizeof(oprc_lock_t),
	              (int *) &slock_id);
	*number_of_lock = 0;

	nthr = (icv->nthreads > 0) ?  /* Explicitely requested population */
	       icv->nthreads :
	       icv->ncpus - 1;      /* Use a pool of #cpus threads otherwise */
	caps->supports_nested            = 0;
	caps->supports_dynamic           = 1;
	caps->supports_nested_nondynamic = 0;
	caps->max_levels_supported       = 1;
	caps->default_numthreads         = nthr;
	caps->max_threads_supported      = -1;     /* No limit */

	if (proc_lib_inited) return (0);

	proc_lib_inited = 1;
	return (0);
}

void oprc_finalize(int exitvalue)
{
	flag = 1;
	FENCE;

	pthread_mutex_lock(&count_mutex1);
	pthread_cond_signal(&flag1);
	pthread_mutex_unlock(&count_mutex1);
}

/* Request for "numthr" threads to execute parallelism in nest
 * level "level".
 * We only support 1 level of parallelism, so we always return 0
 * if level > 1
 */
int oprc_request(int numthr, int level)
{
	return ((level == 1) ? numthr : 0);
}


/* Only the master thread can call this.
 * It blocks the thread waiting for all its children to finish their job.
 */
void oprc_waitall(void **ignore)
{
	int i;
	for (i = 0; i < number_of_process; i++)
		wait(0);
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
		lock = (slock + (*number_of_lock));
		*number_of_lock = ((*number_of_lock) + 1) % 100;
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

void oprc_create(int numthr, int level, void *arg, void **ignore)
{
	if (numthr <= 0) return;

	number_of_process = numthr;
	team_argument = arg;

	pthread_mutex_lock(&count_mutex1);
	pthread_mutex_lock(&count_mutex2);
	pthread_cond_signal(&flag1);
	pthread_mutex_unlock(&count_mutex1);

	pthread_cond_wait(&flag2, &count_mutex2);
	pthread_mutex_unlock(&count_mutex2);
}

extern int __ompi_main(int argc, char **argv);

void oprc__ompi_main(void)
{
	__ompi_main(ort_argc, ort_argv);
}

int main(int argc, char **argv)
{
	ort_argc = argc;
	ort_argv = argv;
	pthread_attr_t tattr;
	pthread_mutexattr_t mattr;
	pthread_t tid;
	int ret = 0, memid, id;
	void *arg;

	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&count_mutex1, &mattr);
	pthread_mutex_init(&count_mutex2, &mattr);

	pthread_cond_init(&flag1, NULL);
	pthread_cond_init(&flag2, NULL);

	oprc_shmalloc((void **)&p, (size_t)1024 * 1024, (int *)&memid);

	ret = pthread_attr_init(&tattr);
	if (ret)
	{
		printf("pthread_attr_init error\n");
		exit(0);
	}

	ret = pthread_attr_setstack(&tattr, p, 1024 * 1024);
	if (ret)
	{
		printf("pthread_attr_setstackaddr error\n");
		exit(0);
	}

	pthread_mutex_lock(&count_mutex1);

	ret = pthread_create(&tid, &tattr, (void *)oprc__ompi_main, NULL);
	if (ret != 0)
	{
		printf("pthread_create error %d\n", ret);
		exit(0);
	}

	while (flag == 0)
	{
		pthread_cond_wait(&flag1, &count_mutex1);
		pthread_mutex_unlock(&count_mutex1);
		FENCE;

		if (flag == 0)
		{
			id = pfork(number_of_process);

			if (id != 0)
			{
				/* Execute requested code */
				ort_ee_dowork(id, team_argument);
				exit(0);
			}
		}
		pthread_mutex_lock(&count_mutex2);
		pthread_mutex_lock(&count_mutex1);
		pthread_cond_signal(&flag2);
		pthread_mutex_unlock(&count_mutex2);
	}

	oprc_shmfree(&memid);
	oprc_shmfree(&number_of_lock_id);
	oprc_shmfree(&slock_id);

	pthread_attr_destroy(&tattr);
	pthread_mutex_destroy(&count_mutex1);
	pthread_mutex_destroy(&count_mutex2);

	pthread_cond_destroy(&flag1);
	pthread_cond_destroy(&flag2);

	exit(0);
}

