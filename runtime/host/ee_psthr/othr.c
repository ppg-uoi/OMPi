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
 * An implementation of the othr API using user-level psthreads.
 * A simple, thread library for multilevel parallelism.
 */


#include "../ort.h"
#include "ee.h"
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#if 1
	#define OTHR_TASKS    1 /* moved in ee.h */
	//#define OTHR_DUMMY_TASKS  1
	#define IMPLICIT_TASKS_FIRST  1 /* true */
	#define OTHR_CURRTASK   1 /* XX: bug in nested parallelism: child threads do not own a tw structure */
	#define EECB_OWNED    0
	#define EECB_FREE   1
#endif


#define RECYCLE_NODES
#define RECYCLE_ARGS

#if defined(RECYCLE_NODES)||defined(RECYCLE_ARGS)
	#include "queues.h"
#endif

#if defined(RECYCLE_NODES)
	QUEUE_DEFINE(struct Node, qnode_t);
	qnode_t QN[64];
#endif

#if defined(RECYCLE_ARGS)
	QUEUE_DEFINE(struct _Args, qargs_t);
	qargs_t QA[64];
#endif

/* Recycling queues for task env too
   Max data size = x */


/*
 * PSthreads specific
 */
struct team_info;

/* Global stuff for the team */
typedef struct thr_info
{
	psthread_t thr;
	int thr_id;
	ort_task_node_t *tw;
#if defined(OTHR_CURRTASK)
	ort_task_node_t *currtask;
#endif
	struct team_info *teaminfo;
#if defined(OTHR_TASKS)
	int status;
#endif
	char pad[CACHE_LINE_SIZE];
} thr_info_t;

struct team_info
{
	void *team_argument;            /* The function's argument */
	int  team_nthr;                 /* # threads in the current team */
	psthread_cond_t cond;
	//  char p00[CACHE_LINE_SIZE];
	int counter;
	//  char p01[CACHE_LINE_SIZE];
	_lock_t clock;
#if defined(OTHR_TASKS)
	int level;
	char p1[CACHE_LINE_SIZE];
	int flag_throt;
	char p2[CACHE_LINE_SIZE];
	int cnt_throt;
	char p3[CACHE_LINE_SIZE];
	int max_level;
	char p4[CACHE_LINE_SIZE];
	int ttcounter;
	char p5[CACHE_LINE_SIZE];
	int ttcreated;
	char p6[CACHE_LINE_SIZE];
	int never_task;
	char p7[CACHE_LINE_SIZE];
	_lock_t lock;
	char p8[CACHE_LINE_SIZE];
	psthread_cond_t ttcond;
	psthread_cond_t ttcond2;
	ort_task_node_t *twpar;
#endif
	thr_info_t *team_thr;
	void *team_thr_actual;
};

#if defined(OTHR_TASKS)
	int task_key;
	extern int eecb_key;
	int othr_max_task_level = 64;
	int othr_high_throttling_factor = 32;
	int othr_low_throttling_factor = 1;
#endif

static int   psthread_lib_inited = 0;     /* Flag to avoid re-initializations */
_lock_t at_lock;
unsigned long activethreads = 1;
unsigned long threadlimit;


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

	if (psthread_lib_inited) return (0);

#if 1
	{
		char *s;
		int  n;

		if ((s = getenv("OMPI_MTL")) != NULL)
			if (sscanf(s, "%d", &n) == 1 && n > 0)
				othr_max_task_level = n;

		if ((s = getenv("OMPI_HTF")) != NULL)
			if (sscanf(s, "%d", &n) == 1 && n >= 0)
				othr_high_throttling_factor = n;

		if ((s = getenv("OMPI_LTF")) != NULL)
			if (sscanf(s, "%d", &n) == 1 /*&& n >= 0*/)
				othr_low_throttling_factor = n;
	}
#endif
	if (nthr >= 0)
	{
		int vcpus = nthr + 1;
		if ((icv->threadlimit > -1)
		    && (vcpus > icv->threadlimit)) vcpus = icv->threadlimit;
		if (vcpus > icv->ncpus) vcpus = icv->ncpus;
		psthread_init(vcpus, (icv->stacksize == -1) ? 0 : icv->stacksize);
	}

	/*psthread_version();*/
	psthread_lib_inited = 1;
	_lock_init(&at_lock);
	threadlimit = icv->threadlimit;
#if defined(OTHR_TASKS)
	psthread_key_create(&task_key, 0);
#if defined(RECYCLE_NODES)
	for (i = 0; i < 64; i++) _queue_init(&QN[i]);
#endif
#if defined(RECYCLE_ARGS)
	for (i = 0; i < 64; i++) _queue_init(&QA[i]);
#endif
#endif
	return (0);
}


//#define TASKCNTR
#if defined(TASKCNTR)
	static long cnt_throt;
	static long max_level;
	static long tcnt[16][8] /*= 0*/;
	static long xcnt[16][8] /*= 0*/;
	static long dcnt[16][8] /*= 0*/;
	static long icnt1[16][8] /*= 0*/;
	static long icnt2[16][8] /*= 0*/;
	static long icnt3[16][8] /*= 0*/;
	static long exec[16][8] /*= 0*/;
#endif

void othr_finalize(int exitvalue)
{
#if defined(TASKCNTR)
	{
		int i;
		unsigned long tcnt_tot = 0, xcnt_tot = 0, dcnt_tot = 0;
		unsigned long icnt1_tot = 0, icnt2_tot = 0, icnt3_tot = 0, exec_tot = 0;

		for (i = 0; i < 8; i++)
		{
			tcnt_tot += tcnt[i][0];
			xcnt_tot += xcnt[i][0];
			dcnt_tot += dcnt[i][0];
			icnt1_tot += icnt1[i][0];
			icnt2_tot += icnt2[i][0];
			icnt3_tot += icnt3[i][0];
			exec_tot += exec[i][0];
		}
		printf("vpid:");
		for (i = 0; i < 8; i++) printf("%7d ", i); printf("%8s\n", "total");
		printf("-----");
		for (i = 0; i < 8; i++) printf("%7s-", "-------"); printf("%8s\n", "--------");
		printf("tcnt:");
		for (i = 0; i < 8; i++) printf("%7d ", tcnt[i][0]); printf("%8d\n", tcnt_tot);
		printf("xcnt:");
		for (i = 0; i < 8; i++) printf("%7d ", xcnt[i][0]); printf("%8d\n", xcnt_tot);
		printf("dcnt:");
		for (i = 0; i < 8; i++) printf("%7d ", dcnt[i][0]); printf("%8d\n", dcnt_tot);
		printf("ict1:");
		for (i = 0; i < 8; i++) printf("%7d ", icnt1[i][0]); printf("%8d\n", icnt1_tot);
		printf("ict2:");
		for (i = 0; i < 8; i++) printf("%7d ", icnt2[i][0]); printf("%8d\n", icnt2_tot);
		printf("ict3:");
		for (i = 0; i < 8; i++) printf("%7d ", icnt3[i][0]); printf("%8d\n", icnt3_tot);
		printf("exec:");
		for (i = 0; i < 8; i++) printf("%7d ", exec[i][0]); printf("%8d\n", exec_tot);
		printf("cnt_throt: %d\n", cnt_throt);
		printf("max_level: %d\n", max_level);
	}
#endif
	printf("OMPI_MTL (MAX_TASK_LEVEL)         = %d\n", othr_max_task_level);
	printf("OMPI_HTF (HIGH_THROTTLING_FACTOR) = %d\n", othr_high_throttling_factor);
	printf("OMPI_LTF (LOW_THROTTLING_FACTOR)  = %d\n", othr_low_throttling_factor);

	psthread_stats();
}

static int inline othr_get_spec_id(struct team_info *ti, int id);

/* The function executed by each thread */
void threadjob(void *env)
{
	thr_info_t *thri = (thr_info_t *)env;
	int  myid = (int) thri->thr_id;
	void *team_argument;
	struct team_info *ti = thri->teaminfo;
	int counter;
	team_argument = ti->team_argument;

#if !defined(IMPLICIT_TASKS_FIRST) && !defined(OTHR_DUMMY_TASKS)
	/* get specific id */
	_lock_acquire(&ti->lock);
	while ((ti->ttcounter >= ti->team_nthr)
	       || (othr_get_spec_id(ti, myid) < 0)) psthread_cond_wait(&ti->ttcond, &ti->lock);
	ti->ttcounter++;
	_lock_release(&ti->lock);
	/* ti->tid = myid; */
#endif

#if defined(OTHR_CURRTASK)
	ti->team_thr[myid].currtask = ti->team_thr[myid].tw;
#endif

	ort_ee_dowork(myid, team_argument);  /* Execute requested code */

#if defined(OTHR_CURRTASK)
	ti->team_thr[myid].currtask = NULL;
#endif

#if 0
	counter = _faa(&ti->counter, -1) - 1;
#else
	_lock_acquire(&ti->clock);
	counter = --ti->counter;
	_lock_release(&ti->clock);
#endif
	if (counter == 1) psthread_cond_signal(&ti->cond); /* xyz */
	//FENCE;                  /* Spin locks do not guarantee memory ordering */
	return;
}

/* Request for "numthr" threads to execute parallelism in level "level" */
int othr_request(int numthr, int level)
{
	int res, tmp;
	int omp_get_dynamic();

	if ((threadlimit == -1) || !omp_get_dynamic())
		return (numthr);

	_lock_acquire(&at_lock);
	tmp = activethreads + numthr;
	if (tmp <= threadlimit)
		activethreads = tmp;
	else
	{
		numthr = threadlimit - activethreads;
		activethreads = threadlimit;
	}
	/*printf("a: numthr->%d, at->%d\n", numthr, activethreads);*/
	_lock_release(&at_lock);

	return (numthr);
}

/* Dispatches numthreads from the pool-list and gives them work to do */
void othr_create(int numthr, int level, void *arg, void **info)
{
	int i;
	struct team_info *ti;
	int inited = 0;
	int curr_vp;
	psthread_t parent, me;
	psthread_attr_t attr = {PSTHREAD_DETACHED, NULL};
	void *actual;
	int nproc = psthread_nvps();
	ort_task_node_t *tw0, *twpar;
	int ps_ws_flag;
	psthread_t thr[64];

	if (numthr <= 0)
		return;
	if (*info == NULL)
	{
		ti = ort_calloc_aligned(sizeof(struct team_info), NULL);  /* check included */
		_lock_init(&ti->clock);
		_lock_init(&ti->lock);
		psthread_cond_init(&ti->cond);
#if defined(OTHR_TASKS)
		_lock_init(&ti->lock);
		psthread_cond_init(&ti->ttcond);
		psthread_cond_init(&ti->ttcond2);
#endif
	}
	else
		ti = *info;

	ti->team_argument = arg;
	ti->counter = numthr + 1;

#if defined(OTHR_TASKS)
	ti->level = level;
	ti->flag_throt = 0;
	ti->cnt_throt = 0;
	ti->max_level = level;
#if defined(IMPLICIT_TASKS_FIRST)
	ti->ttcounter = numthr + 1;
#else
	ti->ttcounter = 1;
#endif
	ti->ttcreated = 0;
	ti->never_task = 0;
#endif
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
	/*if (level > 0)*/ curr_vp = psthread_current_vp();

	me = attr.parent = psthread_self();
#if 1
	ti->team_thr[0].teaminfo = ti;
	ti->team_thr[0].thr_id = 0;
	ti->team_thr[0].thr = me; //attr.parent;
#endif
#if defined(OTHR_TASKS)
	for (i = 0; i <= numthr; i++)
		ti->team_thr[i].status = EECB_OWNED;
#endif
#if 1 //defined(PSTHREAD_VERSION) && (PSTHREAD_VERSION >= 101)
#else
	for (i = 0; i <= numthr; i++)
	{
		if (i > 0) ti->team_thr[i].thr = NULL;
		ti->team_thr[i].tw = NULL;
	}
#endif
#if defined(OTHR_TASKS) /* perhaps later (after setmycb in ort_execute ???) SOS XXX */
	{
		int flag;
		twpar = psthread_getspecific_ex(me, task_key);
		if (twpar->info == NULL)
		{
			twpar->thr = me;
			twpar->info = ti /*info*/;
			_lock_init(&twpar->lock);
		}

		tw0 = ti->team_thr[0].tw;
		if (tw0 == NULL)
		{
			void *mem;
			tw0 = ort_calloc_aligned(sizeof(ort_task_node_t), &mem);  /* check included */
			tw0->mem = mem;
			tw0->func = NULL;
			tw0->task_argument = NULL;
			tw0->info = ti;
			_lock_init(&tw0->lock);
			psthread_cond_init(&tw0->taskcond);
			tw0->num_children = 0;
			tw0->untied = 0;
			tw0->bare = 0;
			ti->team_thr[0].tw = tw0;
			psthread_setspecific_ex(me, task_key, tw0);
			tw0->thr = me;
		}
		tw0->tid = 0;
		tw0->parent = twpar;
		tw0->icvs = twpar->icvs;
		ti->twpar = twpar;
#if defined(OTHR_CURRTASK)
		ti->team_thr[0].currtask = tw0;
#endif
	}
#endif

	if (level == 0) ps_ws_flag = psthread_stealing(0);
	for (i = 1; i <= numthr;
	     i++)   /* which is really better: x*(create+enqueue) or x*create+x*enqueue? */
	{
		ti->team_thr[i].teaminfo = ti;
		ti->team_thr[i].thr_id = i;
#if defined(PSTHREAD_VERSION) && (PSTHREAD_VERSION >= 101)
		psthread_create_ex(&ti->team_thr[i].thr, &attr, threadjob,
		                   (void *) &ti->team_thr[i]);
#else
		psthread_create(&ti->team_thr[i].thr, &attr, threadjob,
		                (void *) &ti->team_thr[i]);
#endif

#if defined(OTHR_TASKS) && (1)  /* ZZZ: moved here */
		{
			ort_task_node_t *tw = NULL;
			int flag;

			//  tw = (ort_task_node_t *)psthread_getspecific_ex(ti->team_thr[i].thr, task_key);
			tw = ti->team_thr[i].tw;
			//  flag = (tw == NULL);
			if (tw == NULL)
			{
				void *mem;
				tw = ort_calloc_aligned(sizeof(ort_task_node_t), &mem); /* check included */
				tw->mem = mem;
				tw->func = NULL;
				tw->task_argument = NULL;
				tw->info = ti;
				_lock_init(&tw->lock);
				psthread_cond_init(&tw->taskcond);
				tw->num_children = 0;
				tw->untied = 0;
				tw->bare = 0;
				ti->team_thr[i].tw = tw;
				tw->tid = i; //myid;
				tw->parent = twpar;
				psthread_setspecific_ex(ti->team_thr[i].thr, task_key, tw);
				tw->thr = ti->team_thr[i].thr;
			}
			tw->icvs = twpar->icvs;
#if defined(OTHR_CURRTASK)
#if defined(IMPLICIT_TASKS_FIRST)
			ti->team_thr[i].tw = ti->team_thr[i].currtask = tw;
#else
			ti->team_thr[i].currtask = NULL;
#endif
#endif
		}

		if (1)
		{
			void *ort_alloc_eecb(void *t, int thrid, void *parent_info);
			int flag;
			void *peecb;
			peecb = (void *)psthread_getspecific_ex(ti->team_thr[i].thr, eecb_key);
			flag = (peecb == NULL);
			peecb = (void *) ort_alloc_eecb(peecb, i, ti->team_argument);
			if (flag) psthread_setspecific_ex(ti->team_thr[i].thr, eecb_key, peecb);
#if !defined(IMPLICIT_TASKS_FIRST)
			ti->team_thr[i].status = EECB_FREE;
#endif
		}
#endif
		thr[i] = ti->team_thr[i].thr;
#if 0
		if (level == 0)
		{
			psthread_enqueue_head(ti->team_thr[i].thr,
			                      (i + curr_vp) % nproc);    /* requires stealing */
			//    psthread_enqueue_tail(ti->team_thr[i].thr, -1);   /* central queue */
		}
		else
		{
			psthread_enqueue_head(ti->team_thr[i].thr, curr_vp);
			//    psthread_enqueue_tail(ti->team_thr[i].thr, -1);   /* central queue */
		}
#endif

	}

#if 1
#if 1
	for (i = 1; i <= numthr;
	     i++)   /* what is really better: x*(create+enqueue) or x*create+x*enqueue? */
	{
		if (level == 0)
		{
			psthread_enqueue_head(ti->team_thr[i].thr,
			                      (i + curr_vp) % nproc);    /* requires stealing */
			//        psthread_enqueue_tail(ti->team_thr[i].thr, -1);   /* central queue */
		}
		else
		{
			psthread_enqueue_head(ti->team_thr[i].thr, curr_vp);
			//      psthread_enqueue_tail(ti->team_thr[i].thr, -1);   /* central queue */
		}
	}
#else
	psthread_enqueue_bunch_tail(&thr[1], numthr, curr_vp);    /* central queue */
#endif
#endif
	//FENCE;

	if (level == 0) psthread_stealing(ps_ws_flag);  /* to its previous value */
}


/* Only the master thread can call this.
 * It blocks the thread waiting for all its children to finish their job.
 */
void othr_waitall(void **info)
{
	struct team_info *ti = *info;
	psthread_t me = ti->team_thr[0].thr;

#if 1 /* xyz */
	_lock_acquire(&ti->clock);
	while (ti->counter != 1) psthread_cond_wait_ex(me, &ti->cond, &ti->clock);
	_lock_release(&ti->clock);
#else
	while (ti->counter != 1) psthread_yield();
#endif
#if defined(OTHR_TASKS)
	psthread_setspecific_ex(me, task_key, ti->twpar); /* restore master task */
#if defined(TASKCNTR)
	cnt_throt += ti->cnt_throt;
	if (max_level < ti->max_level) max_level = ti->max_level;
#endif
#if defined(OTHR_CURRTASK)
	ti->team_thr[0].currtask = ti->twpar;
#endif
#endif

	if ((threadlimit != -1) || omp_get_dynamic())
	{
		_lock_acquire(&at_lock);
		activethreads -= ti->team_nthr;
		_lock_release(&at_lock);
	}
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
			psthread_cond_init(&l->cond);
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
			psthread_cond_destroy(&l->cond);
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
			psthread_t me = psthread_self();

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
						psthread_cond_wait_ex(me, &l->cond, &l->ilock);
					l->owner = me;
					l->count++;
				}
			_lock_release(&l->ilock);
			return (0);
		}

		case ORT_LOCK_SPIN:
#if 0
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
#else
			_lock_acquire(&(lock->lock.data.spin.mutex));
			return (0);
#endif
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
			psthread_t me = psthread_self();

			_lock_acquire(&l->ilock);
			if ((l->owner == me) && (l->count > 0))
			{
				l->count--;
				if (l->count == 0)
				{
					_lock_release(&l->lock);
					psthread_cond_signal(&l->cond);
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
			psthread_t me = psthread_self();

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
#ifdef AVOID_OMPI_DEFAULT_BARRIER

#ifdef OTHR_TASKS
void ppf(void *vti, int n)
{
	int i;
	struct team_info *ti = (struct team_info *) vti;
	//_lock_acquire(&ti->lock);
	if (ti->never_task) ti->ttcounter = n;
	else                ti->ttcounter += n; // ???
	for (i = 0; i < n; i++) ti->team_thr[i].status = EECB_OWNED;  // memset
	//_lock_release(&ti->lock);
}
#endif

void othr_barrier_init(othr_barrier_t *bar, int n)
{
#if defined(OTHR_TASKS) && !defined(OTHR_DUMMY_TASKS)
	ort_task_node_t *tw;
	tw = psthread_getspecific(task_key);
#endif

	psthread_barrier_init_ex(bar, n);

#if defined(OTHR_TASKS) && !defined(OTHR_DUMMY_TASKS)
	bar->aux = tw->info;
#if defined(IMPLICIT_TASKS_FIRST)
	bar->ppf = ppf;
#endif
#endif
}

void othr_barrier_wait(othr_barrier_t *bar, int id)
{
#if defined(OTHR_TASKS) && !defined(OTHR_DUMMY_TASKS)
	struct team_info *ti;
	ti = (struct team_info *) bar->aux;
	if (ti == NULL)
		ti = ((ort_task_node_t *)psthread_getspecific(task_key))->info;
	othr_taskwait(1, ti, id);
#endif

	psthread_barrier_wait_ex(bar, id);

#if defined(OTHR_TASKS) && !defined(OTHR_DUMMY_TASKS)
#if defined(IMPLICIT_TASKS_FIRST)       /* moved into ppf */
	//  _lock_acquire(&ti->lock);
	//  ti->ttcounter++;
	//  ti->team_thr[id].status = EECB_OWNED;
	//  _lock_release(&ti->lock);
#else
	/* get specific id */
	_lock_acquire(&ti->lock);
	while ((ti->ttcounter >= ti->team_nthr)
	       || (othr_get_spec_id(ti, id) < 0)) psthread_cond_wait(&ti->ttcond,
		           &ti->lock);        // ex
	ti->ttcounter++;
	_lock_release(&ti->lock);
	/* twpar->tid = id; */
#endif
#endif
}

void othr_barrier_destroy(othr_barrier_t *bar)
{
	psthread_barrier_destroy_ex(bar);
}
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                 *
 *  TASKS                                                          *
 *                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#if defined(OTHR_TASKS)

static inline int othr_get_free_id(struct team_info *ti, int favor)
{
#if 1
	int i, ii, res = -1;
	static int sstart = 0;
	int start;

	//  _lock_acquire(&ti->lock);
	if (favor >= 0) start = favor;
	else    start = 0; //sstart;
	for (i = start; i < ti->team_nthr + start; i++)
	{
		ii = i % ti->team_nthr;
		if (ti->team_thr[ii].status == EECB_FREE)
		{
			ti->team_thr[ii].status = EECB_OWNED;
			res = ii;
			break;
		}
	}
	sstart = res + 1;
	//  _lock_release(&ti->lock);
	return res;
#else
	int i, ii, res = -1;
	static int start = 0;
	thr_info_t *team_thr = ti->team_thr;
	int team_nthr = ti->team_nthr;
	int end = team_nthr + start;

	//start = 0;
	for (i = start; i < end; i++)
	{
		ii = i % team_nthr;
		if (team_thr[ii].status == EECB_FREE)
		{
			team_thr[ii].status = EECB_OWNED;
			res = ii;
			break;
		}
	}
	start = res + 1;
	return res;
#endif
}


static int inline othr_get_spec_id(struct team_info *ti, int id)
{
	int i, res = -1;

	if (id == -1) return -1;

	if (ti->team_thr[id].status == EECB_FREE)
	{
		ti->team_thr[id].status = EECB_OWNED;
		res = id;
	}
	return res;
}

void tdrv(void *arg)
{
	ort_task_node_t *tw = (ort_task_node_t *) arg;
	struct team_info *ti = tw->info;
	int ompid;
	psthread_t p, me = tw->thr;
	ort_task_node_t *parent = tw->parent;
	void *pp;
	//  me = psthread_self();

	if (!tw->bare)
	{
		_lock_acquire(&ti->lock);
		while (ti->ttcounter >= ti->team_nthr) psthread_cond_wait_ex(me, &ti->ttcond,
			    &ti->lock); //ex
		ti->ttcounter++;
		_lock_release(&ti->lock);

		_lock_acquire(&ti->lock);
		while ((ompid = othr_get_free_id(ti, tw->tid)) < 0)
		{
			_lock_release(&ti->lock);
			printf("\nWhy?\n");
			//printf("othr_get_free_id returned %d\n", ompid);
			psthread_yield();
			_lock_acquire(&ti->lock);
		}
		_lock_release(&ti->lock);
		tw->tid = ompid;
	}
	else
		tw->tid = ompid = 0;

	p = ti->team_thr[ompid].thr;
	//  if (p == NULL) {
	//    printf("ERROR\n");
	//    exit(1);
	//  }
	psthread_key_clone(p, me);

	psthread_setspecific_ex(me, task_key, tw);
#if defined(OTHR_CURRTASK)
	ti->team_thr[ompid].currtask = tw;
#endif
#ifdef TASKCNTR
	exec[psthread_current_vp()][0]++;
#endif

	tw->func(tw->task_argument);

	if (!tw->bare)
	{
		//  ompid = omp_get_thread_num(); /* not required ? */
		//  if (tw->tid != ompid) {printf("XXXZ\n"); /*exit(1);*/ }
		//  tw->tid = ompid;    /* already set */
		ompid = tw->tid;
	}

	/* activeregions */
#if 0
	_faa(&ti->ttcreated, -1);
#else
	_lock_acquire(&ti->lock);
	ti->ttcreated--;
#endif

#if 1 /* barrier_stress.c -> #if 0 */
	if (ti->team_nthr > 1
	    && (ti->ttcreated < (ti->team_nthr * othr_low_throttling_factor)))
	{
		ti->flag_throt = 0;
		ti->cnt_throt++;
	}
#endif

	if (!tw->bare)
	{
#if 0
		_faa(&ti->ttcounter, -1);
#else
		ti->ttcounter--;
#endif
		ti->team_thr[ompid].status = EECB_FREE;
		psthread_cond_signal(&ti->ttcond);//ex
		/* restore task_icvs ?? */
	}
	if (ti->ttcreated == 0) psthread_cond_broadcast(&ti->ttcond2); // SOS
#if 1
	_lock_release(&ti->lock);
#endif
	if (parent)
	{
		int rem;
#if 0
		rem = _faa(&parent->num_children, -1) - 1;
#else
		_lock_acquire(&parent->lock);
		rem = --parent->num_children;
		_lock_release(&parent->lock);
#endif
		if (rem == 0)
		{
			if (parent->finished)
			{
				if (!parent->staticmem)
				{
#if defined(RECYCLE_NODES)
					//_enqueue_head(&QN[psthread_current_vp()], parent);
					_enqueue_head(&QN[parent->cid], parent);
#else
					free(parent);
#endif
				}
			}
			else
				psthread_cond_signal(&parent->taskcond);
		}
	}

	_lock_acquire(&tw->lock);
	if (tw->num_children == 0)
	{
		_lock_release(&tw->lock);
#if defined(RECYCLE_NODES)
		//    _enqueue_head(&QN[psthread_current_vp()], tw);
		_enqueue_head(&QN[tw->cid], tw);
#else
		free(tw->mem);
#endif
	}
	else
	{
		tw->finished = 1;
		_lock_release(&tw->lock);
	}
}

void othr_set_currtask(ort_task_node_t *tw)
{
	psthread_setspecific(task_key, tw);
}

ort_task_node_t *othr_get_currtask(void *vti, int thread_num)
{
	ort_task_node_t *tw = NULL;
	struct team_info *ti = (struct team_info *)vti;
	int flag = 0;

#if defined(OTHR_CURRTASK)
	if (ti) tw = ti->team_thr[thread_num].currtask;
	if (ti && tw == NULL) flag = 1;
	if ((ti == NULL) || (tw == NULL))
#endif
		tw = psthread_getspecific(task_key);
#if defined(OTHR_CURRTASK)
	if (flag) ti->team_thr[thread_num].currtask = tw;
#endif
	//  printf("ti = %p tw = %p\n", ti, tw);
	return tw;
}

void othr_new_task(int final, int flag, void *(*func)(void *arg), void *arg)
{
	int i, ompid, vpid, task_limit;
	int team_nthr = 1;
	psthread_t t, me, p;
	psthread_attr_t attr = {PSTHREAD_DETACHED, NULL};
	ort_task_node_t *this_task, *tw, *twpar;
	struct team_info *ti;

	twpar = psthread_getspecific(task_key);
	ti = twpar->info;
	if (ti) team_nthr = ti->team_nthr;

	if ((team_nthr == 1) || (twpar->level >= othr_max_task_level))
	{
		ti->flag_throt = 1;
		othr_new_task_exec(flag, func, arg);
		return;
	}


	/* throttling here */
#if 1
	if (ti->flag_throt || (twpar->throttled))
	{
		othr_new_task_exec(flag, func, arg);
		return;
	}
#endif

#if 1
	/* throttling here */
	if (ti->level ==  0)  task_limit = team_nthr *
		                                   othr_high_throttling_factor; //1*16; /* CILK=4,8 */
	else      task_limit = team_nthr * (othr_high_throttling_factor / 2);
	if (ti->ttcreated > task_limit)
	{
		//    printf("setting flag_throt\n");
		ti->flag_throt = 1;
		othr_new_task_exec(flag, func, arg);
		return;
	}
#endif

#if defined(TASKCNTR)
	tcnt[twpar->tid][0]++;
	//  printf("tcnt = %d\n", tcnt);
#endif

#if 0
	_faa(&ti->ttcreated, 1);
#else
	_lock_acquire(&ti->lock);
	ti->ttcreated++;
	_lock_release(&ti->lock);
#endif
	{
		tw = NULL;
		vpid = psthread_current_vp();
#if defined(RECYCLE_NODES)
		_dequeue(&QN[vpid], &tw);
		//  _dequeue(&QN[twpar->tid], &tw);
#endif
	}

	//  printf("xxx\n");
	if (tw == NULL)
	{
		void *mem;
		tw = ort_calloc_aligned(sizeof(ort_task_node_t), &mem); /* check included */
		tw->mem = mem;
	}
	tw->func = func;
	if (tw->func == twpar->func) tw->recursive = 1;
	tw->task_argument = arg;
	tw->info = ti;
	_lock_init(&tw->lock);
	psthread_cond_init(&tw->taskcond);
	tw->num_children = 0;
	tw->untied = flag % 10;
	tw->bare = flag / 10;
	tw->throttled = 0;
	tw->isfinal = 0;
	tw->inherit_task_node = 0;
	tw->cid = vpid;
	tw->tid = twpar->tid; //psthread_current_vp();
	tw->level = twpar->level + 1;
	if (tw->level > ti->max_level) ti->max_level = tw->level;

	tw->parent = twpar;
	tw->icvs = twpar->icvs;
#if 0
	_faa(&twpar->num_children, 1);
#else
	_lock_acquire(&twpar->lock);
	twpar->num_children++;
	_lock_release(&twpar->lock);
#endif
	if (!ti->never_task) ti->never_task = 1;

	psthread_create(&t, &attr, tdrv, tw);
	tw->thr = t;
	//  psthread_create_ex(&tw->thr, &attr, tdrv, tw);
	//  t = tw->thr;
#if 1
	//  if (twpar->func) {  /* i am an explicit task - no difference*/
	if (1)
	{
#if 1
		psthread_enqueue_head(t, vpid);
#else /* CILK */
		me = psthread_self();

		{
			_lock_acquire(&ti->lock);
			ti->ttcounter--;
			ti->team_thr[twpar->tid].status = EECB_FREE;
			_lock_release(&ti->lock);
		}

		//    printf("before switch (%p - %p - %d)\n", twpar, psthread_self(), twpar->tid);
		psthread_switch_and_yield(me, t);
		//    printf("after switch (%p - %p - %d)\n", twpar, psthread_self(), twpar->tid);

		/* get id */
		_lock_acquire(&ti->lock);
		if (!twpar->untied)
			while ((ti->ttcounter >= ti->team_nthr)
			       || ((ompid = othr_get_spec_id(ti, twpar->tid))) < 0) psthread_cond_wait_ex(me,
				           &ti->ttcond, &ti->lock);
		else
			while ((ti->ttcounter >= ti->team_nthr)
			       || ((ompid = othr_get_free_id(ti, twpar->tid))) < 0) psthread_cond_wait_ex(me,
				           &ti->ttcond, &ti->lock);
		ti->ttcounter++;
		_lock_release(&ti->lock);

		twpar->tid = ompid;

		p = ti->team_thr[ompid].thr;
		if (p == NULL)
		{
			printf("ERROR! ompid=%d\n", ompid);
			exit(1);
		}
		if (p != me)
		{
			psthread_key_clone(p, me);
			psthread_setspecific_ex(me, task_key, twpar); // ??
		}
#if defined(OTHR_CURRTASK)
		ti->team_thr[ompid].currtask = twpar;
#endif
#endif
	}
	else
#endif
	{

		psthread_enqueue_tail(t, vpid);
	}

}

void othr_new_task_exec(int flag, void *(*func)(void *arg), void *arg)
{
	struct team_info *ti;
	ort_task_node_t *twpar, *tw;
	int vpid;

	twpar = psthread_getspecific(task_key);

	if (twpar->throttled == 0)
	{
#if defined(TASKCNTR)
		xcnt[twpar->tid][0]++;
#endif

		{
			tw = NULL;
			vpid = psthread_current_vp();
#if defined(RECYCLE_NODES)
			_dequeue(&QN[vpid], &tw);
			//  _dequeue(&QN[twpar->tid], &tw);
#endif
		}
		if (tw == NULL)
		{
			void *mem;
			tw = ort_calloc_aligned(sizeof(ort_task_node_t), &mem); /* check included */
			tw->mem = mem;
		}
		tw->func = NULL;
		tw->task_argument = NULL;
		_lock_init(&tw->lock);
		psthread_cond_init(&tw->taskcond);
		tw->num_children = 0;
		tw->untied = flag % 10;
		tw->bare = flag / 10;
		tw->throttled = 1;
		tw->tid = twpar->tid;
		tw->level = twpar->level/*+1*/;
		tw->cid = vpid;
		tw->parent = twpar;
		tw->icvs = twpar->icvs;
		//_lock_acquire(&twpar->lock);
		//twpar->num_children++;
		//_lock_release(&twpar->lock);
		tw->info = twpar->info;

		psthread_setspecific(task_key, tw);
		ti = tw->info;
#if defined(OTHR_CURRTASK)
		ti->team_thr[tw->tid].currtask = tw;
#endif
		func(arg);

		if (!tw->staticmem)
		{
#if defined(RECYCLE_NODES)
			//    _enqueue_head(&QN[psthread_current_vp()], tw);
			_enqueue_head(&QN[tw->cid], tw);
#else
			free(tw->mem);
#endif
		}
		psthread_setspecific(task_key, twpar);
#if defined(OTHR_CURRTASK)
		ti->team_thr[tw->tid].currtask = twpar;
#endif
	}
	else
	{

#if defined(TASKCNTR)
		dcnt[twpar->tid][0]++;
#endif
		func(arg);
	}

	//twpar->throttled = 0;

}

void *othr_task_immediate_start(int final)
{
	ort_task_node_t *twpar, *tw;
	struct team_info *ti;
	int vpid;

	//_lock_acquire(&ti->lock);
	//ti->ttcreated++;
	//_lock_release(&ti->lock);

	twpar = psthread_getspecific(task_key); /* throttling is active */
	ti = twpar->info;
	if (ti == NULL)
		return NULL;

	if ((twpar->throttled == 1) || (final))
	{
#if defined(TASKCNTR)
		icnt1[twpar->tid][0]++;
#endif
		return NULL;
	}

	if ((twpar->throttled == 0) && (ti->flag_throt))
	{
#if defined(TASKCNTR)
		icnt2[twpar->tid][0]++;
#endif
		twpar->throttled = 2;
		return twpar;
	}


#if defined(TASKCNTR)
	icnt3[twpar->tid][0]++;
#endif

	{
		tw = NULL;
		vpid = psthread_current_vp();
#if defined(RECYCLE_NODES)
		_dequeue(&QN[vpid], &tw);
		//  _dequeue(&QN[twpar->tid], &tw);
#endif
	}
	if (tw == NULL)
	{
		void *mem;
		tw = ort_calloc_aligned(sizeof(ort_task_node_t), &mem); /* check included */
		tw->mem = mem;
	}
	tw->func = NULL;
	tw->task_argument = NULL;
	_lock_init(&tw->lock);
	psthread_cond_init(&tw->taskcond);
	tw->num_children = 0;
	tw->untied = 0; /* no matter */
	tw->bare = 0;
	tw->throttled = 1;
	tw->cid = vpid;
	tw->tid = twpar->tid;
	tw->level = twpar->level/*+1*/;
	tw->icvs = twpar->icvs;
	tw->info = twpar->info;
	tw->parent = twpar;
	//  tw->thr = NULL;

	psthread_setspecific(task_key, tw);
#if defined(OTHR_CURRTASK)
	ti->team_thr[tw->tid].currtask = tw;
#endif
	return tw;
}


void othr_task_immediate_end(void *new_node)
{
	ort_task_node_t *tw = (ort_task_node_t *) new_node;
	struct team_info *ti;

	if (tw == NULL) return;

	if ((tw != NULL) && (tw->throttled == 2))
	{
		tw->throttled = 0;
		return;
	}

	psthread_setspecific(task_key, tw->parent);
	ti = tw->info;
#if defined(OTHR_CURRTASK)
	ti->team_thr[tw->tid].currtask = tw->parent;
#endif
	if (!tw->staticmem)
	{
#if defined(RECYCLE_NODES)
		_enqueue_head(&QN[tw->cid], tw);
#else
		free(tw->mem);
#endif
	}
}

int othr_check_throttling()
{
	ort_task_node_t *twpar;
	struct team_info *ti;

	twpar = psthread_getspecific(task_key);
	ti = twpar->info;
	if (ti)
	{
		if (ti->team_nthr == 1) return 1;
		return (ti->flag_throt);
	}
	return 1;
}

void othr_taskwait(int how, void *vti, int thread_num)
{
#if defined(OTHR_TASKS) && !defined(OTHR_DUMMY_TASKS)
	int team_nthr = 1;
	int ompid, getid;
	struct team_info *ti = (struct team_info *)vti;
	ort_task_node_t *tw, *tw0;
	psthread_t me, p, me0;

	//  printf("othr_taskwait(%d, %p, %d)\n", how, vti, thread_num);

	if (ti) team_nthr = ti->team_nthr;
	if (team_nthr == 1) return;

	if (how == 0)   /* explicit taskwait */
	{
		me = psthread_self();
		tw = (ort_task_node_t *) psthread_getspecific_ex(me, task_key);
		//    tw0 = ti->team_thr[thread_num].currtask;
		//    me0 = tw0->thr;
		//    if (tw != tw0) printf("tw = %p tw0 = %p\n", tw, tw0);
		//    if (me != me0) printf("me = %p me0 = %p\n", me, me0);

		/* PHASE 1: PREPARING */
		if (tw->throttled)
			return;

		if (!tw->bare)
		{
			ompid = tw->tid;  /* omp_get_thread_num() */
			_lock_acquire(&ti->lock);
			ti->ttcounter--;
			ti->team_thr[ompid].status = EECB_FREE;
			psthread_cond_signal_ex(&ti->ttcond);
			_lock_release(&ti->lock);
		} /* !bare */

		/* PHASE 2: WAITING */
		_lock_acquire(&tw->lock);
		while (tw->num_children != 0) psthread_cond_wait_ex(me, &tw->taskcond,
			                                                    &tw->lock);
		_lock_release(&tw->lock);

		/* PHASE 3: RESUMING */
		if (!tw->bare)
		{
			/* get new id */
			_lock_acquire(&ti->lock);
			if (!tw->untied)    /* tied */
			{
				while ((getid = othr_get_spec_id(ti, ompid)) < 0) psthread_cond_wait_ex(me,
					    &ti->ttcond, &ti->lock);
			}
			else
			{
				while ((getid = othr_get_free_id(ti, ompid)) < 0) psthread_cond_wait_ex(me,
					    &ti->ttcond, &ti->lock);
			}
			ti->ttcounter++;
			_lock_release(&ti->lock);

			tw->tid = ompid = getid;

			p = ti->team_thr[ompid].thr;
			if (p == NULL)
			{
				printf("ERROR! ompid=%d\n", ompid);
				exit(1);
			}
			if (p != me)    /* usual case */
			{
				psthread_key_clone(p, me);
				psthread_setspecific_ex(me, task_key, tw);  // reset
			}
		} /* !tw->bare */
#if defined(OTHR_CURRTASK)
		ti->team_thr[tw->tid].currtask = tw;
#endif
	}
	else
		if ((how == 1)
		    || (how == 2))      /* explicit barrier || end of parallel region */
		{
			ti->team_thr[thread_num].status = EECB_FREE;
			FENCE;
			if (ti->never_task)
			{
				_lock_acquire(&ti->lock);
				ti->ttcounter--;
				psthread_cond_signal_ex(&ti->ttcond);//ex
				me = ti->team_thr[thread_num].thr;
				while (ti->ttcreated != 0)
				{
					psthread_cond_wait_ex(me, &ti->ttcond2, &ti->lock);
					//        psthread_cond_wait(&ti->ttcond2, &ti->lock);
				}
				_lock_release(&ti->lock);
			}
			/*
			else ti->ttcounter will be set equal to n later
			*/
			////    if (ti->never_task) psthread_cond_signal(&ti->ttcond);  //for how==1 not needed
		}
#endif
}


//#define HEADER_SIZE 32
#define HEADER_SIZE 64
//#define ROUNDUP(x,n) ((x+n-1)&(~(n-1)))

void *othr_taskenv_alloc(int size, void *task_func)
{
#if defined(RECYCLE_ARGS)
	ort_task_args_t *arg = NULL;
	int vpid;
	void *res;
	int rsize;
#if 1
	unsigned int v = size + HEADER_SIZE;
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		rsize = v;
	}
#else
	rsize = size + HEADER_SIZE;
#endif
	//  printf("task_func: %p size %d\n", task_func, size);
	vpid = psthread_current_vp();
	_dequeue(&QA[vpid], &arg);
	if (arg == NULL) arg = malloc(rsize);
	if (arg == NULL) { printf("%s: malloc failed\n", __FUNCTION__); exit(1); }
	//  printf("%20s(%d): %p %p\n", __FUNCTION__, vpid, arg, arg->data);
	arg->cid = vpid;
	arg->size = rsize;
	res = (void *)((char *)arg + HEADER_SIZE);

	return res;
#else
	return (malloc(size));
#endif
}

void othr_taskenv_free(void *ptr)
{
#if defined(RECYCLE_ARGS)
	ort_task_args_t *arg;

	arg = (ort_task_args_t *)((char *)ptr - HEADER_SIZE);
	//  printf("%20s(%d): %p %p\n", __FUNCTION__, arg->cid, ptr, arg);
	_enqueue_head(&QA[arg->cid], arg);
#else
	free(ptr);
#endif
}



#if 0
void tf(void *ts)
{
	void *ort_get_func_task(void *);
	void (*wf)(void *arg);

	psthread_setspecific(eecb_key, ts);
	wf = ort_get_func_task(ts);
	wf(ts);
}

void othr_tcreate(void (*func)(void *arg), void *arg)
{
	psthread_t t;
	psthread_create(&t, NULL, tf, arg);
	psthread_enqueue(t, -1);
}


void othr_twait()
{
	psthread_waitall(); /* counter + cond wait */
}
#endif


#else

void othr_taskwait(int how, void *vti, int thread_num) {}
void othr_new_task(int flag, void *(*func)(void *arg), void *arg) {}
void othr_new_task_exec(int flag, void *(*func)(void *arg), void *arg) {}
ort_task_node_t *othr_get_currtask() {return NULL;}
void othr_set_currtask(ort_task_node_t *tw) {}
void *othr_task_immediate_start(void) {return NULL;}
void othr_task_immediate_end(void *new_node) {}
int othr_check_throttling() {return 1;}
void *othr_taskenv_alloc(int size) {return NULL;}
void othr_taskenv_free(void *ptr, int size) {}


#endif

