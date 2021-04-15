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

/* ort_barrier.c */

/*
 * 2010/12/18:
 *   First time around.
 */

#include "ort_prive.h"

#define CANCELLED_NONE      0    /* Barrier responses */
#define CANCELLED_PARALLEL  1
#define CANCELLED_TASKGROUP 2

#define NOT_DB_RELEASING    0
#define DB_RELEASING        1

/* Used to index aligned_3int.value[] */
enum {ARRIVED_INDEX, RELEASED_INDEX, PHASE_INDEX};

#define ARRIVED(eeid)   bar->status[eeid].value[ARRIVED_INDEX]
#define RELEASED(eeid)  bar->status[eeid].value[RELEASED_INDEX]
#define PHASE(eeid)     bar->status[eeid].value[PHASE_INDEX]

/* Threshold beyond which the barrier array is reduced */
#define ALLOC_THRESHOLD 16

/* Is is assumed here that only 1 thread calls this (so as to avoid
 * expensive bookkeeping).
 */
void ort_default_barrier_init(ort_defbar_t **barp, int team_size)
{
	ort_defbar_t *bar = *barp;

	if (bar == NULL)
	{
		bar = *barp = (ort_defbar_t *) ort_calloc(sizeof(ort_defbar_t));
		// me->barrier->alloc_size = 0;        // Not required due to calloc
		// me->barrier->actual_arr_ptr = NULL; // Not required due to calloc
	}

	if ((team_size > bar->alloc_size) ||
			((bar->alloc_size >= ALLOC_THRESHOLD) &&
			 (team_size <= (bar->alloc_size >> 1))))
	{
		bar->status = (volatile aligned_3int *)
			ort_realloc_aligned(team_size * sizeof(aligned_3int),
					(void **) &bar->actual_arr_ptr);
		bar->alloc_size = team_size;
	}

	/* Initialize */
	bar->db_state[0] = bar->db_state[1] = NOT_DB_RELEASING;
	bar->team_size = team_size;
	for (--team_size; team_size >= 0; team_size--)
	{
		ARRIVED(team_size) = 0;
		RELEASED(team_size) = 0;
		PHASE(team_size) = 0;
	}
}


/* Currently unused */
void ort_default_barrier_destroy(ort_defbar_t **barp)
{
	if (barp == NULL || *barp == NULL)
		return;
	free((*barp)->actual_arr_ptr);
	free(*barp);
	*barp = NULL;
}


// *INDENT-OFF*
#define check_parallel_cancellation() (TEAMINFO(me)->cancel_par_active)

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
static int task_barrier_wait_with_cancel(ort_defbar_t *bar, int eeid)
{
#if !defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_eecb_t   *me = __MYCB;
	int time = 0;

	int phase = (PHASE(eeid) ^= 1);

	if (eeid > 0)
	{
		/* Reach 1st synchronization point */
		ARRIVED(eeid) = 2;
		for (; (ARRIVED(eeid) == 2); time++)
		{
			if (check_parallel_cancellation())
				return CANCELLED_PARALLEL;

			ort_taskwait(1);

			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			}
		}

		if (check_parallel_cancellation())
			return CANCELLED_PARALLEL;

		/* We check again to make sure that we searched once */
		ort_taskwait(1);

		/* Reach 2nd synchronization point. Just making sure all
		 * tasks have been completed before leaving the barrier.
		 */
		RELEASED(eeid) = 1;
		for (; (RELEASED(eeid) == 1); time++)
			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			};
	}
	else     /* Let the master do the work */
	{
		/* Ensure all my mates arrived in the 1st synchronization point */
		/* Wait for task completion */
		for (eeid = 1; eeid < bar->team_size; eeid++)
		{
			for (; (ARRIVED(eeid) != 2); time++)
			{
				if (check_parallel_cancellation())
					return CANCELLED_PARALLEL;

				ort_taskwait(1);

				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				}
			}
		}

		if (check_parallel_cancellation())
			return CANCELLED_PARALLEL;

		ort_taskwait(1);

		/* Release my mates from the 1st synchronization point */
		for (eeid = 1; eeid < bar->team_size; eeid++)
			ARRIVED(eeid) = 0;

		/* Ensure all my mates arrived in the 2nd synchronization point */
		for (eeid = 1; eeid < bar->team_size; eeid++)
			for (; (RELEASED(eeid) != 1); time++)
				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				};

		/* Release my mates from the 2nd synchronization point */
		for (eeid = 1; eeid < bar->team_size; eeid++)
			RELEASED(eeid) = 0;

		/* Reinitialize worksharing cancellation flags for future use. */
		TEAMINFO(me)->cancel_for_active = 0;
		TEAMINFO(me)->cancel_sec_active = 0;
		FENCE;
	}

	/* Must check for active cancellation, because
	 * barrier is a cancellation point. The check for cancel taskgoup
	 * must be done here (at the end of function) because all the barrier
	 * functionality must be present to ensure correct thread synchronization.
	 */
	if (check_parallel_cancellation())
		return CANCELLED_PARALLEL;
	if(check_taskgroup_cancellation())
		return CANCELLED_TASKGROUP;
	return CANCELLED_NONE;
#endif
}


/* Optimized task_barrier_wait for the case cancellation is disabled.
 */
static int task_barrier_wait_without_cancel(ort_defbar_t *bar, int eeid)
{
#if !defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_eecb_t   *me = __MYCB;
	int time = 0;

	int phase = (PHASE(eeid) ^= 1);

	if (eeid > 0)
	{
		/* Reach 1st synchronization point */
		ARRIVED(eeid) = 2;
		for (; (ARRIVED(eeid) == 2); time++)
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

		/* Reach 2nd synchronization point. Just making sure all
		 * tasks have been completed before leaving the barrier.
		 */
		RELEASED(eeid) = 1;
		for (; (RELEASED(eeid) == 1); time++)
			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			};
	}
	else     /* Let the master do the work */
	{
		/* Ensure all my mates arrived in the 1st synchronization point */
		/* Wait for task completion */
		for (eeid = 1; eeid < bar->team_size; eeid++)
		{
			for (; (ARRIVED(eeid) != 2); time++)
			{
				ort_taskwait(1);

				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				}
			}
		}

		ort_taskwait(1);

		/* Release my mates from the 1st synchronization point */
		for (eeid = 1; eeid < bar->team_size; eeid++)
			ARRIVED(eeid) = 0;

		/* Ensure all my mates arrived in the 2nd synchronization point */
		for (eeid = 1; eeid < bar->team_size; eeid++)
			for (; (RELEASED(eeid) != 1); time++)
				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				};

		/* Release my mates from the 2nd synchronization point */
		for (eeid = 1; eeid < bar->team_size; eeid++)
			RELEASED(eeid) = 0;
		FENCE;
	}
	return CANCELLED_NONE;
#endif
}


static int task_barrier_wait(ort_defbar_t *bar, int eeid)
{
	if (CANCEL_ENABLED())
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
static int ort_default_barrier_wait_with_cancel(ort_defbar_t *bar, int eeid)
{
#if !defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_eecb_t   *me = __MYCB;
	int time = 0;
	volatile int *task_exist = &(me->parent->tasking.never_task);

	int phase = (PHASE(eeid) ^= 1);

	/* First i have to execute my tasks to empty my queue
	 * and prepare for a possible cancellation
	 */
	FENCE;
	if (*task_exist == 1)
		ort_execute_my_tasks(me);

	if (eeid > 0)
	{
		ARRIVED(eeid) = 1;
		for (; (ARRIVED(eeid) == 1); time++)
		{
			if (check_parallel_cancellation())
				return CANCELLED_PARALLEL;

			if (bar->db_state[phase] == NOT_DB_RELEASING && *task_exist == 1)
				return task_barrier_wait(bar, eeid);

			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			}
		}
	}
	else     /* Let the master do the work */
	{
		/* Ensure that all my mates are in the barrier */
		for (eeid = 1; eeid < bar->team_size; eeid++)
		{
			for (; (ARRIVED(eeid) != 1); time++)
			{
				if (check_parallel_cancellation())
				{
					TEAMINFO(me)->cancel_sec_active = 0;
					TEAMINFO(me)->cancel_for_active = 0;
					FENCE;
					return CANCELLED_PARALLEL;
				}

				if (*task_exist == 1)
					return task_barrier_wait(bar, 0);

				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				}
			}
		}

		if (check_parallel_cancellation())
		{
			TEAMINFO(me)->cancel_sec_active = 0;
			TEAMINFO(me)->cancel_for_active = 0;
			FENCE;
			return CANCELLED_PARALLEL;
		}

		FENCE;
		if (*task_exist == 1)
			return task_barrier_wait(bar, 0);

		/* If required, reset db_state of previous DB. */
		int opphase = !phase;
		if (bar->db_state[opphase] == DB_RELEASING)
			bar->db_state[opphase] = NOT_DB_RELEASING;

		/* Release my mates */
		bar->db_state[phase] = DB_RELEASING;
		for (eeid = 1; eeid < bar->team_size; eeid++)
			ARRIVED(eeid) = 0;

		/* Make sure that worksharing cancellation flags
		 * are always reinitialized for future use. Only
		 * master should do this. */
		TEAMINFO(me)->cancel_sec_active = 0;
		TEAMINFO(me)->cancel_for_active = 0;
		FENCE;
	}

	/* Must check for active cancellation, because
	 * barrier is a cancellation point. The check for cancel taskgoup
	 * must be done here (at the end of function) because all the barrier
	 * functionality must be present to ensure correct thread synchronization.
	 */
	if (check_parallel_cancellation())
		return CANCELLED_PARALLEL;
	if(check_taskgroup_cancellation())
		return CANCELLED_TASKGROUP;
	return CANCELLED_NONE;
#endif
}


/* Optimized default_barrier_wait for the case cancellation is disabled.
 */
static int ort_default_barrier_wait_without_cancel(ort_defbar_t *bar, int eeid)
{
#if !defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_eecb_t   *me = __MYCB;
	int time = 0;
	volatile int *task_exist = &(me->parent->tasking.never_task);

	int phase = (PHASE(eeid) ^=  1);

	FENCE;
	if (*task_exist == 1)
		ort_execute_my_tasks(me);

	if (eeid > 0)
	{
		ARRIVED(eeid) = 1;
		for (; (ARRIVED(eeid) == 1); time++)
		{
			if (bar->db_state[phase] == NOT_DB_RELEASING && *task_exist == 1)
				return task_barrier_wait(bar, eeid);

			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			}
		}
	}
	else     /* Let the master do the work */
	{
		/* Ensure that all my mates are in the barrier */
		for (eeid = 1; eeid < bar->team_size; eeid++)
		{
			for (; (ARRIVED(eeid) != 1); time++)
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
		}

		FENCE;
		if (*task_exist == 1)
			return task_barrier_wait(bar, 0);

		/* If required, reset db_state of previous DB. */
		int opphase = !phase;
		if (bar->db_state[opphase] == DB_RELEASING)
			bar->db_state[opphase] = NOT_DB_RELEASING;

		/* Release my mates */
		bar->db_state[phase] = DB_RELEASING;
		for (eeid = 1; eeid < bar->team_size; eeid++)
			ARRIVED(eeid) = 0;
	}
	return CANCELLED_NONE;
#endif
}


int ort_default_barrier_wait(ort_defbar_t *bar, int eeid)
{
	if (CANCEL_ENABLED())
		return ort_default_barrier_wait_with_cancel(bar, eeid);
	else
		return ort_default_barrier_wait_without_cancel(bar, eeid);
}


int ort_barrier_me(void)
{
	ort_eecb_t *me = __MYCB;
	if (me->num_siblings == 1)
		return 0;
	return ( ee_barrier_wait(TEAMINFO(me)->barrier, me->thread_num) );
}


/* This function checks for active cancellation, because
 * barrier is a cancellation point. A spining (in barrier) thread only checks
 * for cancel parallel, while at the end of barrier also checks for
 * cancel taskgroup. This is because we must allow threads to early exit the
 * barrier when parallel is canceled even if not all siblings have entered
 * the barrier function.
 */
static void parallel_barrier_wait_with_cancel(ort_defbar_t *bar, int eeid)
{
#if !defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_eecb_t   *me = __MYCB;
	int time = 0;
	volatile int *task_exist = &(me->parent->tasking.never_task);

	int phase = (PHASE(eeid) ^= 1);

	/* First I have to execute my tasks to empty my queue
	 * and prepare for a possible cancellation
	 */
	FENCE;
	if (*task_exist == 1)
		ort_execute_my_tasks(me);

	if (eeid > 0)
	{
		/* Reach 1st synchronization point */
		ARRIVED(eeid) = 2;
		for (; (ARRIVED(eeid) == 2); time++)
		{
			if (check_parallel_cancellation())
				return;

			/* ort_taskwait must be called once only  */
			if (*task_exist == 1)
				ort_taskwait(1);

			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			}
		}

		if (check_parallel_cancellation())
			return;

		FENCE;
		if (*task_exist == 1)
			ort_taskwait(1);

		/* Reach 2nd synchronization point. Just making sure all
		 * tasks have been completed before leaving the barrier.
		 */
		RELEASED(eeid) = 1;
		for (; (RELEASED(eeid) == 1); time++)
			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			};
	}
	else     /* Let the master do the work */
	{
		/* Ensure all my mates arrived in the 1st synchronization point */
		for (eeid = 1; eeid < bar->team_size; eeid++)
		{
			for (; (ARRIVED(eeid) != 2); time++)
			{
				if (check_parallel_cancellation())
				{
					TEAMINFO(me)->cancel_sec_active = 0;
					TEAMINFO(me)->cancel_for_active = 0;
					FENCE;
					return;
				}

				/* Try to help */
				if (*task_exist == 1)
					ort_taskwait(1);

				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				}
			}
		}

		if (check_parallel_cancellation())
		{
			TEAMINFO(me)->cancel_sec_active = 0;
			TEAMINFO(me)->cancel_for_active = 0;
			FENCE;
			return;
		}

		FENCE;
		if (*task_exist == 1)
			ort_taskwait(1);

		/* Release my mates from the 1st synchronization point */
		for (eeid = 1; eeid < bar->team_size; eeid++)
			ARRIVED(eeid) = 0;

		/* Ensure all my mates arrived in the 2nd synchronization point */
		for (eeid = 1; eeid < bar->team_size; eeid++)
			for (; (RELEASED(eeid) != 1); time++)
				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				};

		/* If required, reset db_state of previous DB. */
		int opphase = !phase;
		if (bar->db_state[opphase] == DB_RELEASING)
			bar->db_state[opphase] = NOT_DB_RELEASING;

		/* Release my mates from the 2nd synchronization point */
		for (eeid = 1; eeid < bar->team_size; eeid++)
			RELEASED(eeid) = 0;

		/* No more tasks */
		*task_exist = 0;
		/* Master reinitializes worksharing cancellation flags for future use. */
		TEAMINFO(me)->cancel_sec_active = 0;
		TEAMINFO(me)->cancel_for_active = 0;
		FENCE;
	}
#endif
}


static void parallel_barrier_wait_without_cancel(ort_defbar_t *bar, int eeid)
{
#if !defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_eecb_t   *me = __MYCB;
	int time = 0;
	volatile int *task_exist = &(me->parent->tasking.never_task);

	int phase = (PHASE(eeid) ^= 1);

	FENCE;
	if (*task_exist == 1)
		ort_execute_my_tasks(me);

	if (eeid > 0)
	{
		/* Reach 1st synchronization point */
		ARRIVED(eeid) = 2;
		for (; (ARRIVED(eeid) == 2); time++)
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

		FENCE;
		if (*task_exist == 1)
			ort_taskwait(1);

		/* Reach 2nd synchronization point. Just making sure all
		 * tasks have been completed before leaving the barrier.
		 */
		RELEASED(eeid) = 1;
		for (; (RELEASED(eeid) == 1); time++)
			if (time == BAR_YIELD)
			{
				time = -1;
				ee_yield();
			};
	}
	else     /* Let the master do the work */
	{
		/* Ensure all my mates arrived in the 1st synchronization point */
		for (eeid = 1; eeid < bar->team_size; eeid++)
		{
			for (; (ARRIVED(eeid) != 2); time++)
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
		}

		FENCE;
		if (*task_exist == 1)
			ort_taskwait(1);

		/* Release my mates from the 1st synchronization point */
		for (eeid = 1; eeid < bar->team_size; eeid++)
			ARRIVED(eeid) = 0;

		/* Ensure all my mates arrived in the 2nd synchronization point */
		for (eeid = 1; eeid < bar->team_size; eeid++)
			for (; (RELEASED(eeid) != 1); time++)
				if (time == BAR_YIELD)
				{
					time = -1;
					ee_yield();
				};

		/* If required, reset db_state of previous DB. */
		int opphase = !phase;
		if (bar->db_state[opphase] == DB_RELEASING)
			bar->db_state[opphase] = NOT_DB_RELEASING;

		/* Release my mates from the 2nd synchronization point */
		for (eeid = 1; eeid < bar->team_size; eeid++)
			RELEASED(eeid) = 0;

		/* No more tasks */
		*task_exist = 0;
		FENCE;
	}
#endif
}


/* At the end of a parallel region the compiler injects an
 * ort_taskwait(2) call ("2" signifying the end of a parallel region,
 * i.e. not a plain taskwait but an end-of-parallel barrier).
 * Then parallel_barrier_wait ort_taskwait() calls parallel_barrier_wait().
 */
void parallel_barrier_wait(ort_defbar_t *bar, int eeid)
{
	if (CANCEL_ENABLED())
		parallel_barrier_wait_with_cancel(bar, eeid);
	else
		parallel_barrier_wait_without_cancel(bar, eeid);
}
