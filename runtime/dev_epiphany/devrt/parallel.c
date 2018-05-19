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

#include "e_lib.h"
#include "shared_data.h"
#include "device_globals.h"

#ifdef LOCK_TASK_QUEUE

void ort_init_master_tasking(int teamsize)
{
	private_eecb_t  *me = __MYCB;
	int i;

	/* All cells are free */
	me->tasking.free_cell = 0;

	/* Clear pointers for taskwait */
	for (i = 0; i < teamsize; i++)
		me->tasking.pending_tasks[i] = NULL;

	/* Initialize lock for task table */
	ee_init_lock(&(me->tasking.lock), 0);
}

#else

void ort_init_master_tasking(int teamsize)
{
}

#endif

/* This prepares everything so that I become the master of a new team
 */
void prepare_master(int teamsize)
{
	private_eecb_t *me = __MYCB;
	me->num_children = teamsize;

#ifdef OLD_BAR
	/* Initialize barrier */
	if (teamsize > 1)
		ort_barrier_init(&(me->team_barrier), teamsize);
#endif

	if (!me->have_created_team)
	{
		me->workshare.blocking.inited = 0;
		me->have_created_team = 1;
	}

	if (teamsize > 1)
		init_workshare_regions(me);

	ort_init_master_tasking(teamsize);
}

static int parallella_request_parallel_team(void *(*func)(void *), void *shared,
                                            int nthr)
{
	int my_core = __MYCB->core;
	int my_active_level = __MYCB->activelevel;
	volatile parallella_runtime_mem_t *pll_ort;
	volatile int *my_team_threads;

	/* Get shared memory with HOST */
	pll_ort = (parallella_runtime_mem_t *)(EPIPHANY_BASE_ADDRESS +
	                                       PARALLELLA_ORT_MEM_OFFSET);

	/* Initialize bookkeeping for team threads */
	pll_ort->team_threads[my_active_level][my_core] = -1;
	mbox[my_active_level].func = NULL;
	mbox[my_active_level].args = NULL;
	mbox[my_active_level].team_parent = NULL;

	/* Request HOST for nthr threads.. */
	if (my_active_level == 0)
		pll_ort->pe_exit[my_core] = nthr;
	else
		if (my_active_level == 1)       // In second level I ask
			pll_ort->pe_exit[my_core] = nthr + 100; // 100 + nthr...

	/* Wait untill HOST wakes up PEs */
	do
	{
		my_team_threads = &(pll_ort->team_threads[my_active_level][my_core]);
	}
	while (*my_team_threads == -1);

	pll_ort->pe_exit[my_core] = 0; // Just to be sure...

	return (*my_team_threads) + 1;
}


static void parallella_ort_signal_group(void *(*func)(void *), void *shared,
                                        private_eecb_t *team_parent)
{
	int my_active_level = __MYCB->activelevel;
	mbox[my_active_level].team_parent = team_parent;
	mbox[my_active_level].args = shared;
	mbox[my_active_level].func = func;
}

static void parallella_parallel_ended(void)
{
	int my_core = __MYCB->core;
	int my_active_level = __MYCB->activelevel;
	volatile parallella_runtime_mem_t *pll_ort;
	volatile int *unoccupy_ok;

	/* Get shared memory with HOST */
	pll_ort = (parallella_runtime_mem_t *)(EPIPHANY_BASE_ADDRESS +
	                                       PARALLELLA_ORT_MEM_OFFSET);

	/* Inform HOST that my parallel is over so it can unoccupy cores... */
	if (my_active_level == 1)
		pll_ort->pe_exit[my_core] = -2; /* First level parallel team */
	else
		pll_ort->pe_exit[my_core] = -3; /* Second level parallel team */

	/* Wait untill HOST unoccupy team cores */
	do
	{
		unoccupy_ok = &(pll_ort->pe_exit[my_core]);
	}
	while (*unoccupy_ok != 0);

	return;
}

void ort_execute_parallel(void *(*func)(void *), void *shared, int nthr,
                          int iscombined, int procbind_type)
{
	private_eecb_t me_master;
	private_eecb_t  *me = __MYCB;

	/* Wake up PEs in order to run a parallel region */
	if (nthr == -1)
		nthr = 16; // Default team in parallella is 16 PEs TODO: icvs...
	else
		if (me->activelevel >= MAX_ACTIVE_LEVELS) /* Cannot go any deeper */
			nthr = 1;


	if (nthr > 1)
		nthr = parallella_request_parallel_team(func, shared, nthr);

	/* Prepare everything for the execution of parallel section */
	prepare_master(nthr);


	if (nthr > 1)
		/* Order PEs to begin execution */
		parallella_ort_signal_group(func, shared, me);

	/* Prepare and change to new eecb */
	me_master.thread_num = 0;
	me_master.parent_row = me->core / PARALLELLA_COLS;
	me_master.parent_col = me->core % PARALLELLA_COLS;
	me_master.core = me->core;
	me_master.num_siblings = nthr;
	me_master.num_children = 0;
	me_master.have_created_team = 0;
	me_master.parent = me;
	me_master.activelevel = me->activelevel + 1;
	__SETMYCB(&(me_master));

#ifndef OLD_BAR
	if (nthr > 1)
		ort_barrier_init(); /* All PEs must init their barrier */
#endif

	/* New implicit task... */
	mbox[me_master.activelevel].implicit_task.parent =
	  NULL; /* No bookkeeping needed here */
	mbox[me_master.activelevel].implicit_task.isfinal = 0;
	mbox[me_master.activelevel].implicit_task.num_children = 0;
	mbox[me_master.activelevel].implicit_task.status = 2;
	ee_init_lock(&(mbox[me_master.activelevel].implicit_task.lock), 0);

	/* TODO: Add icvs here. */
	mbox[me_master.activelevel].implicit_task.icvs.dynamic =
    mbox[me->activelevel].implicit_task.icvs.dynamic;
	mbox[me_master.activelevel].implicit_task.icvs.nested =
    mbox[me->activelevel].implicit_task.icvs.nested;

	/* Set this task as current executing task */
	__SETCURRTASK(__MYCB, &(mbox[me_master.activelevel].implicit_task));

	/* Prepare my virtual me tasking environment */
	ort_init_worker_tasking();

#ifdef MEASURE
	volatile parallella_runtime_mem_t *pll_ort;
	pll_ort = (parallella_runtime_mem_t *)(EPIPHANY_BASE_ADDRESS +
	                                       PARALLELLA_ORT_MEM_OFFSET);

	e_ctimer_set(E_CTIMER_1, E_CTIMER_MAX);
	e_ctimer_start(E_CTIMER_1, E_CTIMER_CLK);
	/* Now participate in team */
	func(shared);
	unsigned int end = e_ctimer_get(E_CTIMER_1);
	pll_ort->time[me->thread_num] = E_CTIMER_MAX - end;
	pll_ort->sleep[me->thread_num] = sleep_time;
#else
	/* Now participate in team */
	func(shared);
#endif

	/* Inform HOST that parallel team has closed */
	if (nthr > 1)
		parallella_parallel_ended();

	/* Restore my old eecb, old implicit_task is restored as well */
	__SETMYCB(me);

	return;
}
