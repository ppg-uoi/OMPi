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

/* ort_barrier.c */

/*
 * 2010/12/18:
 *   First time around.
 */

#include "ort_prive.h"

#define NO_CANCEL_ACTIVATED         0
#define CANCEL_PARALLEL_ACTIVATED   1
#define CANCEL_TASKGROUP_ACTIVATED  2

/* Is is assumed here that only 1 thread calls this (so as to avoid
 * expensive bookkeeping). Reinitializations cause no harm.
 */
void ort_default_barrier_init(ort_defbar_t *bar, int nthr)
{
	if (nthr > MAX_BAR_THREADS)
		ort_error(1, "barrier cannot support > %d threads; "
		          "change MAX_BAR_THREADS in ort.h\n", MAX_BAR_THREADS);

	bar->nthr = nthr;
	for (--nthr;  nthr >= 0; nthr--)
	{
		bar->arrived[nthr].value = 0;
		bar->arrived2[nthr].value = 0;
		bar->released[nthr].value = 0;
	}
}


void ort_default_barrier_destroy(ort_defbar_t *bar)
{
}


// *INDENT-OFF*
#define check_parallel_cancellation() (me->parent->cancel_parallel)

#define check_taskgroup_cancellation() \
   ((__CURRTASK(me)->taskgroup != NULL) ? \
     __CURRTASK(me)->taskgroup->is_canceled : \
     0 \
   )
// *INDENT-ON*

/* This function checks for active cancellation, because
 * barrier is a cancellation point. A spining (in barrier) thread only checks
 * for cancel parallel, while at the end of barrier also checks for
 * cancel taskgroup. This is because we must allow threads to early exit the
 * barrier when parallel is canceled even if not all siblings have entered
 * the barrier function.
 */
int task_barrier_wait_with_cancel(ort_defbar_t *bar, int eeid)
{
#if !defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_eecb_t   *me = __MYCB;
	int time = 0;

	if (eeid > 0)
	{
		bar->arrived2[eeid].value = 1;
		
		if (check_parallel_cancellation() == CANCEL_ACTIVATED)
			return CANCEL_PARALLEL_ACTIVATED;

		for (; (bar->arrived2[eeid].value == 1); time++)
		{
			ort_taskwait(1);
			
			if (check_parallel_cancellation() == CANCEL_ACTIVATED)
				return CANCEL_PARALLEL_ACTIVATED;

			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			}
		}
		
		if (check_parallel_cancellation() == CANCEL_ACTIVATED)
			return CANCEL_PARALLEL_ACTIVATED;

		/* We check again to make sure that we searched once */
		ort_taskwait(1);

		bar->released[eeid].value = 1;
		for (; (bar->released[eeid].value == 1); time++)
			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			}
	}
	else     /* Let the master do the work */
	{
		if (check_parallel_cancellation() == CANCEL_ACTIVATED)
			return CANCEL_PARALLEL_ACTIVATED;
		
		/* Wait for task completion */
		for (eeid = 1; eeid < bar->nthr; eeid++)
		{
			for (; (bar->arrived2[eeid].value == 0); time++)
				if (time == BAR_YIELD)
				{
					ort_taskwait(1);
					
					if (check_parallel_cancellation() == CANCEL_ACTIVATED)
						return CANCEL_PARALLEL_ACTIVATED;

					time = -1;
					ee_yield();
				}

			if (check_parallel_cancellation() == CANCEL_ACTIVATED)
				return CANCEL_PARALLEL_ACTIVATED;

			ort_taskwait(1);
		}

		/* No more tasks */
		me->parent->tasking.never_task = 0;
		/* Release my mates */
		for (eeid = 1; eeid < bar->nthr; eeid++)
		{
			bar->arrived[eeid].value = 0;
			bar->arrived2[eeid].value = 0;
		}

		/* Gather them again to ensure that all threads have executed tasks */
		for (eeid = 1; eeid < bar->nthr; eeid++)
			for (; (bar->released[eeid].value == 0); time++)
				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				}
		/* Make sure that worksharing cancellation flags
		 * are always reinitialized for future use. */
		me->parent->cancel_for = 0;
		me->parent->cancel_sections = 0;
		/* Release them from barrier */
		for (eeid = 1; eeid < bar->nthr; eeid++)
			bar->released[eeid].value = 0;
		FENCE;
	}
	
	/* Must check for active cancellation, because
	 * barrier is a cancellation point. The check for cancel taskgoup
	 * must be done here (at the end of function) because all the barrier
	 * functionality must be present to ensure correct thread synchronization.
	 */
	if (check_parallel_cancellation() == CANCEL_ACTIVATED)
		return CANCEL_PARALLEL_ACTIVATED;
	else if(check_taskgroup_cancellation() == CANCEL_ACTIVATED)
		return CANCEL_TASKGROUP_ACTIVATED;
	else
		return NO_CANCEL_ACTIVATED;
#endif
}


int task_barrier_wait_without_cancel(ort_defbar_t *bar, int eeid)
{
#if !defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_eecb_t   *me = __MYCB;
	int time = 0;

	if (eeid > 0)
	{
		bar->arrived2[eeid].value = 1;
		for (; (bar->arrived2[eeid].value == 1); time++)
		{
			ort_taskwait(1);

			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			}
		}

		/* We check again to make sure that we searched once */
		ort_taskwait(1);


		bar->released[eeid].value = 1;
		for (; (bar->released[eeid].value == 1); time++)
			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			}
	}
	else     /* Let the master do the work */
	{
		/* Wait for task completion */
		for (eeid = 1; eeid < bar->nthr; eeid++)
		{
			for (; (bar->arrived2[eeid].value == 0); time++)
				if (time == BAR_YIELD)
				{
					ort_taskwait(1);

					time = -1;
					ee_yield();
				}

			ort_taskwait(1);
		}

		/* No more tasks */
		me->parent->tasking.never_task = 0;
		/* Release my mates */
		for (eeid = 1; eeid < bar->nthr; eeid++)
		{
			bar->arrived[eeid].value = 0;
			bar->arrived2[eeid].value = 0;
		}

		/* Gather them again to ensure that all threads have executed tasks */
		for (eeid = 1; eeid < bar->nthr; eeid++)
			for (; (bar->released[eeid].value == 0); time++)
				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				}
		/* Release them from barrier */
		for (eeid = 1; eeid < bar->nthr; eeid++)
			bar->released[eeid].value = 0;
		FENCE;
	}
	return 0;
#endif
}


int task_barrier_wait(ort_defbar_t *bar, int eeid)
{
	if (omp_get_cancellation())
		return task_barrier_wait_with_cancel(bar, eeid);
	else
		return task_barrier_wait_without_cancel(bar, eeid);
}

/* This function checks for active cancellation, because
 * barrier is a cancellation point. A spining (in barrier) thread only checks
 * for cancel parallel, while at the end of barrier also checks for
 * cancel taskgroup. This is because we must allow threads to early exit the
 * barrier when parallel is canceled even if not all siblings have entered
 * the barrier function.
 */
int ort_default_barrier_wait_with_cancel(ort_defbar_t *bar, int eeid)
{
#if !defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_eecb_t   *me = __MYCB;
	int time = 0;
	volatile int *task_exist = &(me->parent->tasking.never_task);

	if (*task_exist == 1)
		ort_execute_my_tasks(me);

	if (eeid > 0)
	{
		bar->arrived[eeid].value = 1;
		for (; (bar->arrived[eeid].value == 1); time++)
		{
			if (*task_exist == 1)
				return task_barrier_wait(bar, eeid);

			if (check_parallel_cancellation() == CANCEL_ACTIVATED)
				return CANCEL_PARALLEL_ACTIVATED;

			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			}
		}

		/* We check again to make sure that we searched once */
		if (*task_exist == 1)
			return task_barrier_wait(bar, eeid);

		bar->released[eeid].value = 1;
		for (; (bar->released[eeid].value == 1); time++)
			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();

			}
	}
	else     /* Let the master do the work */
	{
		if (check_parallel_cancellation() == CANCEL_ACTIVATED)
		{
			me->parent->cancel_sections = 0;
			me->parent->cancel_for = 0;
			return CANCEL_PARALLEL_ACTIVATED;
		}

		/* Ensure that all my mates are in the barrier */
		for (eeid = 1; eeid < bar->nthr; eeid++)
		{
			for (; (bar->arrived[eeid].value == 0); time++)
			{

				/* Try to help */
				if (*task_exist == 1)
					return task_barrier_wait(bar, 0);
				
				if (check_parallel_cancellation() == CANCEL_ACTIVATED)
				{
					me->parent->cancel_sections = 0;
					me->parent->cancel_for = 0;
					return CANCEL_PARALLEL_ACTIVATED;
				}

				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				}
			}
			
			if (check_parallel_cancellation() == CANCEL_ACTIVATED)
			{
				me->parent->cancel_sections = 0;
				me->parent->cancel_for = 0;
				return CANCEL_PARALLEL_ACTIVATED;
			}

			/* Try to help */
			if (*task_exist == 1)
				return task_barrier_wait(bar, 0);
		}

		/* Release my mates */
		for (eeid = 1; eeid < bar->nthr; eeid++)
			bar->arrived[eeid].value = 0;

		/* Gather them again to ensure that all threads have checked for tasks */
		for (eeid = 1; eeid < bar->nthr; eeid++)
			for (; (bar->released[eeid].value == 0); time++)
				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				}

		/* Release them from barrier */
		for (eeid = 1; eeid < bar->nthr; eeid++)
			bar->released[eeid].value = 0;
		FENCE;
		/* Make sure that worksharing cancellation flags
		 * are always reinitialized for future use. Only
		 * master should do this. */
		me->parent->cancel_sections = 0;
		me->parent->cancel_for = 0;
	}
	
	/* Must check for active cancellation, because
	 * barrier is a cancellation point. The check for cancel taskgoup
	 * must be done here (at the end of function) because all the barrier
	 * functionality must be present to ensure correct thread synchronization.
	 */
	if (check_parallel_cancellation() == CANCEL_ACTIVATED)
		return CANCEL_PARALLEL_ACTIVATED;
	else if(check_taskgroup_cancellation() == CANCEL_ACTIVATED)
		return CANCEL_TASKGROUP_ACTIVATED;
	else
		return NO_CANCEL_ACTIVATED;
#endif
}


int ort_default_barrier_wait_without_cancel(ort_defbar_t *bar, int eeid)
{
#if !defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_eecb_t   *me = __MYCB;
	int time = 0;
	volatile int *task_exist = &(me->parent->tasking.never_task);

	if (*task_exist == 1)
		ort_execute_my_tasks(me);

	if (eeid > 0)
	{

		bar->arrived[eeid].value = 1;
		for (; (bar->arrived[eeid].value == 1); time++)
		{
			if (*task_exist == 1)
				return task_barrier_wait(bar, eeid);

			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			}
		}

		/* We check again to make sure that we searched once */
		if (*task_exist == 1)
			return task_barrier_wait(bar, eeid);

		bar->released[eeid].value = 1;
		for (; (bar->released[eeid].value == 1); time++)
			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			}
	}
	else     /* Let the master do the work */
	{

		/* Ensure that all my mates are in the barrier */
		for (eeid = 1; eeid < bar->nthr; eeid++)
		{

			for (; (bar->arrived[eeid].value == 0); time++)
			{

				/* Try to help */
				if (*task_exist == 1)
					return task_barrier_wait(bar, 0);

				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				}
			}

			/* Try to help */
			if (*task_exist == 1)
				return task_barrier_wait(bar, 0);
		}

		/* Release my mates */
		for (eeid = 1; eeid < bar->nthr; eeid++)
			bar->arrived[eeid].value = 0;

		/* Gather them again to ensure that all threads have checked for tasks */
		for (eeid = 1; eeid < bar->nthr; eeid++)
			for (; (bar->released[eeid].value == 0); time++)
				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				}

		/* Release them from barrier */
		for (eeid = 1; eeid < bar->nthr; eeid++)
			bar->released[eeid].value = 0;
		FENCE;
	}
	return 0;
#endif
}


int ort_default_barrier_wait(ort_defbar_t *bar, int eeid)
{
	if (omp_get_cancellation())
		return ort_default_barrier_wait_with_cancel(bar, eeid);
	else
		return ort_default_barrier_wait_without_cancel(bar, eeid);
}


int ort_barrier_me(void)
{
	ort_eecb_t *me = __MYCB;
	if (me->num_siblings == 1)
		return 0;
	return ee_barrier_wait(&me->parent->barrier, me->thread_num);
}

/* This function checks for active cancellation, because
 * barrier is a cancellation point. A spining (in barrier) thread only checks
 * for cancel parallel, while at the end of barrier also checks for
 * cancel taskgroup. This is because we must allow threads to early exit the
 * barrier when parallel is canceled even if not all siblings have entered
 * the barrier function.
 */
int parallel_barrier_wait_with_cancel(ort_defbar_t *bar, int eeid)
{
#if !defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_eecb_t   *me = __MYCB;
	int time = 0;
	volatile int *task_exist = &(me->parent->tasking.never_task);
	if (*task_exist == 1)
		ort_execute_my_tasks(me);

	if (eeid > 0)
	{
		bar->arrived[eeid].value = 1;
		
		if (check_parallel_cancellation() == CANCEL_ACTIVATED)
			return CANCEL_PARALLEL_ACTIVATED;

		for (; (bar->arrived[eeid].value == 1); time++)
		{
			/* ort_taskwait must be called once only  */

			if (check_parallel_cancellation() == CANCEL_ACTIVATED)
				return CANCEL_PARALLEL_ACTIVATED;
		
			if (*task_exist == 1)
				ort_taskwait(1);

			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			}

		}

		if (*task_exist == 1)
			ort_taskwait(1);
	}
	else     /* Let the master do the work */
	{

		/* Ensure that all my mates are in the barrier */
		for (eeid = 1; eeid < bar->nthr; eeid++)
		{

			for (; (bar->arrived[eeid].value == 0); time++)
			{
				/* Try to help */
				if (*task_exist == 1)
					ort_taskwait(1);
				
				if (check_parallel_cancellation() == CANCEL_ACTIVATED)
				{
					me->parent->cancel_sections = 0;
					me->parent->cancel_for = 0;
					FENCE;
					return CANCEL_PARALLEL_ACTIVATED;
				}

				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				}
			}
			
			if (check_parallel_cancellation() == CANCEL_ACTIVATED)
			{
				me->parent->cancel_sections = 0;
				me->parent->cancel_for = 0;
				FENCE;
				return CANCEL_PARALLEL_ACTIVATED;
			}

			/* Try to help */
			if (*task_exist == 1)
				ort_taskwait(1);
		}

		/* Release my mates */
		for (eeid = 1; eeid < bar->nthr; eeid++)
			bar->arrived[eeid].value = 0;

		/* No more tasks */
		*task_exist = 0;
		/* Make sure that worksharing cancellation flags
		 * are always reinitialized for future use. Only
		 * master should do this.*/
		me->parent->cancel_sections = 0;
		me->parent->cancel_for = 0;
	}
	
	/* Must check for active cancellation, because
	 * barrier is a cancellation point. The check for cancel taskgoup
	 * must be done here (at the end of function) because all the barrier
	 * functionality must be present to ensure correct thread synchronization.
	 */
	if (check_parallel_cancellation() == CANCEL_ACTIVATED)
		return CANCEL_PARALLEL_ACTIVATED;
	else if(check_taskgroup_cancellation() == CANCEL_ACTIVATED)
		return CANCEL_TASKGROUP_ACTIVATED;
	else
		return NO_CANCEL_ACTIVATED;
#endif
}


int parallel_barrier_wait_without_cancel(ort_defbar_t *bar, int eeid)
{

#if !defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_eecb_t   *me = __MYCB;
	int time = 0;
	volatile int *task_exist = &(me->parent->tasking.never_task);
	if (*task_exist == 1)
		ort_execute_my_tasks(me);

	if (eeid > 0)
	{
		bar->arrived[eeid].value = 1;

		for (; (bar->arrived[eeid].value == 1); time++)
		{
			/* ort_taskwait must be called once only  */

			if (*task_exist == 1)
				ort_taskwait(1);

			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			}

		}

		if (*task_exist == 1)
			ort_taskwait(1);
	}
	else     /* Let the master do the work */
	{

		/* Ensure that all my mates are in the barrier */
		for (eeid = 1; eeid < bar->nthr; eeid++)
		{

			for (; (bar->arrived[eeid].value == 0); time++)
			{

				/* Try to help */
				if (*task_exist == 1)
					ort_taskwait(1);

				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				}
			}

			/* Try to help */
			if (*task_exist == 1)
				ort_taskwait(1);
		}

		/* Release my mates */
		for (eeid = 1; eeid < bar->nthr; eeid++)
			bar->arrived[eeid].value = 0;

		/* No more tasks */
		*task_exist = 0;
	}
	return 0;
#endif
}


int parallel_barrier_wait(ort_defbar_t *bar, int eeid)
{
	if (omp_get_cancellation())
		return parallel_barrier_wait_with_cancel(bar, eeid);
	else
		return parallel_barrier_wait_without_cancel(bar, eeid);
}
