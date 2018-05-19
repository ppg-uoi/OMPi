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
 * An implementation of the othr API using user-level torc library.
 * A simple, thread library for multilevel parallelism.
 */



#include "../ort.h"
#include "ee.h"
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/*
 * TORC specific
 */
struct team_info;

/* Global stuff for the team */
typedef struct thr_info
{
	torc_t *thr;
	int thr_id;
	struct team_info *teaminfo;
	int status;
} thr_info_t;

struct team_info
{
	void *team_argument;            /* The function's argument */
	int  team_nthr;                 /* # threads in the current team */
	torc_cond_t cond;
	int counter;
	_lock_t lock;

	thr_info_t *team_thr;
	void *team_thr_actual;
};


static int   torc_lib_inited = 0;     /* Flag to avoid re-initializations */

extern void prepare_omp_thread();
int task_key;


/* Library initialization */
int othr_initialize(int *argc, char ***argv, ort_icvs_t *icv, ort_caps_t *caps)
{
	int         nthr, i, e;
	void        threadjob(void *);

	nthr = (icv->nthreads >= 0) ?  /* Explicitely requested population */
	       icv->nthreads :
	       icv->ncpus - 1;   /* we don't handle the initial thread */
	caps->supports_nested            = 1;
	caps->supports_dynamic           = 1;
	caps->supports_nested_nondynamic = 1;
	caps->max_levels_supported       = -1;     /* No limit */
	caps->default_numthreads         = nthr;
	caps->max_threads_supported      = -1;     /* No limit */

	if (torc_lib_inited) return (0);

	if (nthr > 0)
	{
		int vcpus = nthr + 1;
		if (vcpus > icv->ncpus) vcpus = icv->ncpus;
		//torc_init(vcpus, 0);
	}

	_torc_key_create(&task_key, 0);
	torc_lib_inited = 1;
	torc_set_task_omp_init_routine(prepare_omp_thread);
	torc_init(*argc, *argv, 0);
	return (0);
}


void othr_finalize(int exitvalue)
{
	torc_end();
}


/* The function executed by each thread */
void threadjob(void *env)
{
	thr_info_t *thri = (thr_info_t *)env;
	int  myid = (int) thri->thr_id;
	void *team_argument;

	team_argument = thri->teaminfo->team_argument;
	ort_ee_dowork(myid, team_argument); /* Execute requested code */

	_lock_acquire(&thri->teaminfo->lock);
	thri->teaminfo->counter--;
	_lock_release(&thri->teaminfo->lock);
	if (thri->teaminfo->counter == 1) _torc_cond_signal(&thri->teaminfo->cond);
	/*    FENCE;*/                    /* Spin locks do not guarantee memory ordering */

	return;
}

/* Request for "numthr" threads to execute parallelism in level "level" */
int othr_request(int numthr, int level)
{
	return (numthr);
}

/* Dispatches numthreads from the pool-list and gives them work to do */
void othr_create(int numthr, int level, void *arg, void **info)
{
	int i;
	struct team_info *ti;
	int inited = 0;
	int curr_vp;
	void *actual;
	/*int nproc = othr_num_procs;*/

	if (numthr < 0) return;
	if (*info == NULL)
	{
		ti = ort_calloc_aligned(sizeof(struct team_info), NULL);/* check included */
	}
	else
		ti = *info;

	ti->team_argument = arg;

	_lock_init(&ti->lock);
	_torc_cond_init(&ti->cond);
	ti->counter = numthr + 1;

	if (ti->team_thr == NULL)
	{
		ti->team_thr = ort_calloc_aligned((numthr + 1) * sizeof(thr_info_t), &actual);
		ti->team_thr_actual = actual;
		inited = 1;
	}
	else
	{
		if (ti->team_nthr < (numthr + 1))
		{
			actual = ti->team_thr_actual;
			ti->team_thr = ort_realloc_aligned((numthr + 1) * sizeof(thr_info_t), &actual);
			memset(ti->team_thr, 0, (numthr + 1)*sizeof(thr_info_t));
			ti->team_thr_actual = actual;
		}
	}

	ti->team_nthr = numthr + 1;

	*info = (void *)ti;

#if 1
	ti->team_thr[0].teaminfo = ti;
	ti->team_thr[0].thr_id = 0;
	ti->team_thr[0].thr = _torc_self();
#endif

	for (i = 1; i <= numthr; i++)
	{
		ti->team_thr[i].teaminfo = ti;
		ti->team_thr[i].thr_id = i;

		torc_create_local(threadjob, 1, (void *) &ti->team_thr[i]);
	}

}


/* Only the master thread can call this.
 * It blocks the thread waiting for all its children to finish their job.
 */
void othr_waitall(void **info)
{
	struct team_info *ti = *info;

	_lock_acquire(&ti->lock);
#if 0
	while (ti->counter != 1)
	{
		_lock_release(&ti->lock);
		_torc_yield();
		_lock_acquire(&ti->lock);
	}
#else
	while (ti->counter != 1)
		_torc_cond_wait(&ti->cond, &ti->lock);
#endif
	_lock_release(&ti->lock);
	//  torc_waitall();
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                 *
 *  LOCKS                                                          *
 *                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


int othr_init_lock(othr_lock_t *lock, int type)
{
	switch (lock->lock.type = type)
	{
		case ORT_LOCK_NEST:
		{
			othr_nestlock_t *l = &(lock->lock.data.nest);

			_lock_init(&l->ilock);
			_lock_init(&l->lock);
			l->count = 0;
			_torc_cond_init(&l->cond);
			return (0);
		}

		case ORT_LOCK_SPIN:
		{
			lock->lock.data.spin.rndelay = 0;
			return _lock_init(&(lock->lock.data.spin.mutex));
		}

		default: /* ORT_LOCK_NORMAL */
			return _lock_init(&(lock->lock.data.normal));
	}
}

int othr_destroy_lock(othr_lock_t *lock)
{
	switch (lock->lock.type)
	{
		case ORT_LOCK_NEST:
		{
			othr_nestlock_t *l = &(lock->lock.data.nest);

			_lock_destroy(&l->lock);
			_torc_cond_destroy(&l->cond);
			_lock_destroy(&l->ilock);
			return 0;
		}

		case ORT_LOCK_SPIN:
			return _lock_destroy(&(lock->lock.data.spin.mutex));

		default: /* ORT_LOCK_NORMAL */
			return _lock_destroy(&(lock->lock.data.normal));
	}
}

int othr_set_lock(othr_lock_t *lock)
{
	switch (lock->lock.type)
	{
		case ORT_LOCK_NEST:
		{
			othr_nestlock_t *l = &(lock->lock.data.nest);
			torc_t *me = _torc_self();

			_lock_acquire(&l->ilock);
			if (_lock_try_acquire(&l->lock) == 0) /* If not locked, lock it */
			{
				l->owner = me;              /* Get ownership */
				l->count++;
			}
			else
				if (l->owner == me)  /* Did i do it? */
					l->count++;
				else                                    /* Locked by someone else */
				{
					while (_lock_try_acquire(&l->lock))
						_torc_cond_wait(&l->cond, &l->ilock);
					l->owner = me;
					l->count++;
				}
			_lock_release(&l->ilock);
			return (0);
		}

		case ORT_LOCK_SPIN:
		{
			/* General, portable solution: spin trying with exponential backoff */
			volatile int count, delay, dummy;
			for (delay = lock->lock.data.spin.rndelay;
			     _lock_try_acquire(&(lock->lock.data.spin.mutex));)
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
		}

		default: /* ORT_LOCK_NORMAL */
			return _lock_acquire(&(lock->lock.data.normal));
	}
}


int othr_unset_lock(othr_lock_t *lock)
{
	switch (lock->lock.type)
	{
		case ORT_LOCK_NEST:
		{
			othr_nestlock_t *l = &(lock->lock.data.nest);
			torc_t *me = _torc_self();

			_lock_acquire(&l->ilock);
			if ((l->owner == me) && (l->count > 0))
			{
				l->count--;
				if (l->count == 0)
				{
					_lock_release(&l->lock);
					_torc_cond_signal(&l->cond);
				}
			}
			_lock_release(&l->ilock);
			return 0;
		}

		case ORT_LOCK_SPIN:
			lock->lock.data.spin.rndelay = 0;   /* reset it */
			return _lock_release(&(lock->lock.data.spin.mutex));

		default: /* ORT_LOCK_NORMAL */
			return _lock_release(&(lock->lock.data.normal));
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
			torc_t *me = _torc_self();

			_lock_acquire(&l->ilock);
			if (_lock_try_acquire(&l->lock) == 0)
			{
				l->owner = me;
				res = ++l->count;
			}
			else
				if (l->owner == me)
					res = ++l->count;
				else
					res = 0;
			_lock_release(&l->ilock);
			return res;
		}

		case ORT_LOCK_SPIN:
			return (!_lock_try_acquire(&(lock->lock.data.spin.mutex)));

		default: /* ORT_LOCK_NORMAL */
			return (!_lock_try_acquire(&(lock->lock.data.normal)));
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                 *
 *  BARRIERS                                                       *
 *                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#if 0
void othr_barrier_init(othr_barrier_t *bar, int n)
{
	psthread_barrier_init(bar, n);
}

void othr_barrier_wait(othr_barrier_t *bar, int id)
{
	othr_task_waitall();
	psthread_barrier_wait(bar /*,id*/);
	othr_task_waitall();
}

void othr_barrier_destroy(othr_barrier_t *bar)
{
	psthread_barrier_destroy(bar);
}
#endif
