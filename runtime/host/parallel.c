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

/* parallel.c -- OMPi RunTime library; parallel construct */

#include "ort_prive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>


#define initialize_mcbf(mcbf) {\
		(mcbf)->num_children        = 0;\
		(mcbf)->barrier             = NULL;  /* *Must* init to NULL */\
		(mcbf)->redinfo             = NULL;  /* *Must* init to NULL */\
	}

#define HAVE_CREATED_TEAM(eecb) ((eecb)->mf != NULL)


/* This is the function which physical threads (execution entities) 
 * call to execute the user code.
 * This is where physical threads become OpenMP threads (i.e. they
 * obtain a control block (EECB); and then the user func is called directly.
 */
void ort_ee_dowork(int eeid, void *parent_info)
{
	ort_eecb_t *t = __MYCB, *parent = (ort_eecb_t *) parent_info;

	if (t == NULL)         /* 1st time around; get an EECB */
	{
		__SETMYCB(t = (ort_eecb_t *) eecb_alloc());
		t->ee_info = NULL;
	}

	/* Prepare my control block */
	t->parent         = parent;
	t->num_siblings   = parent->mf->num_children;
	t->level          = parent->level + 1;       /* 1 level deeper */
	t->thread_num     = eeid;                    /* Thread id within the team */
	t->shared_data    = NULL;
	t->sdn            = parent;
	t->mynextNWregion = 0;
	t->nowaitregion   = 0;          /* VVD--actually we don't need to do this */
	t->activelevel    = parent->activelevel +       /* OpenMP 3.0 - team of 1 */
	                    ((t->num_siblings > 1) ? 1 : 0);  /* implies inactive */
	t->currplace      = -1;  /* implies thread is not (yet?) bound to a place */
	t->pfrom          = parent->pfrom;    /* Inherit parent's place partition */
	t->pto            = parent->pto;
	t->cgid           = parent->cgid;                /* Same contention group */

#ifdef ORT_DEBUG
	ort_debug_thread("in ort_ee_dowork(); about to execute func.");
#endif

	ort_start_implicit_task(t);                 /* This is an implicit task */
	if (ort->eecaps.supports_proc_binding)
		bindme(eeid, t, parent);                  /* Bind current thread */
	(*(parent->mf->workfunc))(t->sdn->shared_data); /* Execute the actual function */
	ort_finish_implicit_task(t);                /* Implicit task done */
}


/* Non-peristent physical threads call this
 */
void ort_ee_cleanup(void)
{
	eecb_free(__MYCB);
}


/**
 * This prepares everything so that I become the master of a new team
 * @param me my    control block
 * @param teamsize the number of threads in the team
 * @param combined_teamsize (??) the number of threads + special tasks
 * @param func     the function the team threads must execute
 * @param shared   ptr to a struct with pointers to all shared variables
 */
void prepare_master(ort_eecb_t *me, int teamsize,
                    int combined_teamsize, void *(*func)(void *), void *shared)
{
	if (!HAVE_CREATED_TEAM(me))
	{
		me->mf = (ort_mcbf_t *) mcbf_alloc();
	}

	ort_mcbf_t *mf = me->mf;

	me->shared_data     = (shared == NULL) ? me->sdn->shared_data : shared;

	mf->num_children    = teamsize;
	mf->workfunc        = func;
	mf->cancel_par_active = 0;
	mf->cancel_sec_active = 0;
	mf->cancel_for_active = 0;
	ee_barrier_init(&mf->barrier, combined_teamsize);
	ort_reductions_init(me, combined_teamsize);
	ort_task_queues_init(me, teamsize - 1);

	/* We never shrink the tpkeys array. we may consider freeing the
	 * actual variables some day.
	 */
	if (mf->tpksize < teamsize)    /* Need more space for children thrpriv vars */
	{
		mf->tpkeys = (mf->tpksize == 0) ?
		             ort_alloc((teamsize + 3) * sizeof(ort_tptable_t)) :
		             ort_realloc(mf->tpkeys, (teamsize+3) * sizeof(ort_tptable_t));
		memset(&mf->tpkeys[mf->tpksize], 0,  /* zero out new entries */
		       ((teamsize + 3) - mf->tpksize)*sizeof(ort_tptable_t));
		mf->tpksize = teamsize + 3;
		FENCE;
	}

	if (teamsize > 1)
		init_workshare_regions(me);
	assert(mf->me_master != NULL);
}


/* This is called upon entry in a parallel region.
 *   (1) I inquire OTHR for num_threads threads
 *   (2) I set up my eecb fields for my children to use
 *   (3) I create the team
 *   (4) I participate, having acquired a new eecb
 *   (5) I wait for my children to finish and resume my old eecb
 *
 * If num_threads = -1, the team will have icvs.nthreads threads.
 * In the code belows:
 * 
 * - num_threads is what the user requested
 * - nthr is what will be actually created
 * 
 * @param func        the function the execution entities will execute
 * @param shared      ptr to a struct with pointers to all shared variables
 * @param num_threads the number of requested threads (-1 if unspecified)
 * @param iscombined  true coming from a combined parallel-workshare construct
 * @param bind_req    the type of binding requested 
 */
void ort_execute_parallel(void *(*func)(void *), void *shared, int num_threads,
                          int iscombined, int bind_req)
{
	ort_eecb_t      *me = __MYCB;
	ort_task_node_t *mytask = __CURRTASK(me);
	int              nthr = 0;          /* the final number of created threads */
	int              threadlimit = ort->league.threadlimit;
	int volatile     *cgsize = &CG_SIZE(me->cgid);
	int              create_tasks = 0;
	int              special_tasks = 0;

	/*
	 * First determine how many threads will be created
	 */

	if (num_threads <= 0)                    /* No num_threads() clause */
		num_threads = mytask->icvs.nthreads;   /* => get the default number */

	if (num_threads > 1 && ort->icvs.levellimit > me->activelevel)
	{
		if (me->activelevel == 0 && INITLEAGUE())   /* 1st level of parallelism */
		{
			/* no locking needed in this case */
			if (*cgsize + num_threads - 1 <= threadlimit)
				nthr = ee_request(num_threads - 1, 1, 1);
			else
			{
				if (!mytask->icvs.dynamic)
					goto TEAM_FAILURE;
				nthr = ee_request(threadlimit - *cgsize, 1, 1);
			}
			if (nthr != num_threads - 1 && !mytask->icvs.dynamic)
			{
				TEAM_FAILURE:
				ort_error(3, "failed to create the requested number (%d) of threads.\n"
				          "   Try enabling dynamic adjustment using either of:\n"
				          "    >> OMP_DYNAMIC environmental variable, or\n"
				          "    >> omp_set_dynamic() call.\n", num_threads);
			}
			*cgsize += nthr;
		}
		else                                    /* Nested level */
		{
			/* Check if nested parallism is enabled and we are within limits */
			if (mytask->icvs.nested && ort->icvs.levellimit > me->activelevel)
			{
				if (iscombined && ort->partotask_policy == TRUE)
				{
					special_tasks = num_threads - 1;
					nthr = 0;
				}
				else 
					if (iscombined && ort->partotask_policy == AUTO)
					{
						/* Don't oversubscribe; request only available threads */
						ee_set_lock((ee_lock_t *) &ort->eecb_rec_lock);
						if (*cgsize + num_threads - 1 <= threadlimit)
							nthr = ee_request(num_threads-1, me->activelevel + 1, 0);
						else
							nthr = ee_request(threadlimit - *cgsize, 1, 0);
						*cgsize += nthr;
						ee_unset_lock((ee_lock_t *) &ort->eecb_rec_lock);
						/* The remaining threads will be transformed to tasks */
						special_tasks = num_threads - nthr - 1;
						if (special_tasks > 0)
							create_tasks = 1;
					}
					else /* normal case; allow oversubscription if needed */
					{
						ee_set_lock((ee_lock_t *) &ort->eecb_rec_lock);
						if (*cgsize + num_threads - 1 <= threadlimit)
							nthr = ee_request(num_threads-1, me->activelevel + 1, 1);
						else
							nthr = mytask->icvs.dynamic ? 
							         ee_request(threadlimit - *cgsize, 1, 1) : 0;
						if (nthr != num_threads - 1 && !mytask->icvs.dynamic)
							goto TEAM_FAILURE;                /* No need to unlock */
						*cgsize += nthr;
						FENCE;
						ee_unset_lock((ee_lock_t *) &ort->eecb_rec_lock);
					}

				if (nthr == 0 && special_tasks == 0)
				{
#if 0
					ort_warning("level %d parallelism disabled for thread %d due "
					            "to lack of threads\n   >> Using a team of 1 thread.\n",
					            me->activelevel + 1, me->thread_num);   /* GF */
#endif
				}
				else
					if (nthr != num_threads - 1 && !mytask->icvs.dynamic
					    && special_tasks == 0)
						goto TEAM_FAILURE;
			}
			else
				nthr = 0;      /* Only me will execute it */
		}
	}

	/*
	 * Next, initialize everything needed, create the team & participate
	 */

	prepare_master(me, nthr+special_tasks+1, nthr+1, func, shared);

	if (ort->icvs.proc_bind != omp_proc_bind_false && bind_req != omp_proc_bind_false)
		me->mf->bind_override = bind_req;
	else
		me->mf->bind_override = omp_proc_bind_false;

	/* Special case.
	 * Check whether we are in a nested combined parallel-workshare construct 
	 * and we are allowed to optimize through special tasks (see S.N. Agathos, 
	 * P.E. Hadjidoukas, V.V. Dimakopoulos, "Task-based Execution of Nested 
	 * OpenMP Loops", in IWOMP 2012).
	 */
	if (mytask->icvs.nested && me->activelevel > 0 && iscombined &&
	    ort->icvs.levellimit > me->activelevel)
	{
		if (ort->partotask_policy == TRUE)
		{ 
			/* Create tasks only */
			spftasks_create(me, special_tasks+1, 0, func);
			ort_taskwait(0);  /* Execute all tasks created */
			return;
		}
		
		if (ort->partotask_policy == AUTO && create_tasks == 1)
		{
			/* Create threads and tasks */
			if (nthr != 0)
				ee_create(nthr, me->activelevel, me, &me->ee_info);
			spftasks_create(me, special_tasks, nthr+1, func);
			
			__SETMYCB(me->mf->me_master);        /* Change my cb */
			ort_ee_dowork(0, me);            /* Participate in new team */
			if (nthr != 0)                   /* Destroy the team */
			{
				ee_waitall(&me->ee_info);      /* Wait till all children finish */
				ee_set_lock((ee_lock_t *) &ort->eecb_rec_lock);
				*cgsize -= nthr;
				ee_unset_lock((ee_lock_t *) &ort->eecb_rec_lock);
			}
			__SETMYCB(me);                   /* assume my parent eecb */

			ort_taskwait(0);  /* Execute all tasks created */
			me->mf->num_children = 0;
			return;
		}
	}

	/* Normal case 
	 */
	if (nthr > 0)   /* Start the threads (except myself) */
		ee_create(nthr, me->activelevel, me, &me->ee_info);

#ifdef ORT_DEBUG
	ort_debug_thread("just created team and about to participate");
#endif

	__SETMYCB(me->mf->me_master);        /* Change my cb */

	ort_ee_dowork(0, me);

	/*
	 * All done; destroy the team (me now points to my "parent")
	 */
	if (nthr > 0)
	{
		ee_waitall(&me->ee_info);      /* Wait till all children finish */
		if (me->activelevel == 0)      /* I was an initial thread; no locking */
			*cgsize -= nthr;                  
		else
		{
			ee_set_lock((ee_lock_t *) &ort->eecb_rec_lock);
			*cgsize -= nthr;
			ee_unset_lock((ee_lock_t *) &ort->eecb_rec_lock);
		}
	}
	me->mf->num_children = 0;
	__SETMYCB(me);                   /* assume my parent eecb */
}


/* Execute a function only by the running thread. This is only called
 * in the parser-generated code when checking an IF condition at a
 * parallel section.
 */
void ort_execute_serial(void *(*func)(void *), void *shared)
{
	ort_eecb_t *me = __MYCB;

	prepare_master(me, 1, 1, func, shared);
	__SETMYCB(me->mf->me_master);         /* Change my key */

	ort_ee_dowork(0, me);

	me->mf->num_children = 0;
	__SETMYCB(me);                   /* assume my parent eecb */
}
