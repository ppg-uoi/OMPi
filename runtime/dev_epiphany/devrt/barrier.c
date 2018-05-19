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

#ifdef OLD_BAR

int barrier(void)
{
	private_eecb_t  *me = __MYCB;
	volatile ee_barrier_t *bar =  &(me->parent->team_barrier);
	volatile int local_sense, bar_sense;
	volatile int local_in;
	volatile int local_team_members;

	/* Read bar sense */
	local_sense = read_from_master_local_address(&(bar->sense));
	/* Read team members */
	local_team_members = me->num_siblings;

	if (local_team_members == 1) return 0;

	/* Execute tasks... */
	ort_execute_tasks(me, 1);

	/* Lock barrier lock */
	ee_set_lock(&(bar->barlock));

	/* Let team know that i arrived */
	local_in = read_from_master_local_address(&(bar->in_barrier));
	local_in = local_in + 1;

	write_to_master_local_address(&(bar->in_barrier), (void *)local_in);

	if (local_in < local_team_members)
	{
		/* Release lock */
		ee_unset_lock(&(bar->barlock));

		/* Wait untill all threads arrive in barrier */
		bar_sense = read_from_master_local_address(&(bar->sense));
		while (local_sense == bar_sense)
		{
			/* Execute tasks while waiting */
			ort_execute_tasks(me, 1);
#ifdef SLEEP
			// Set timer to wake me up...
			e_ctimer_stop(E_CTIMER_0);
			e_ctimer_set(E_CTIMER_0, ORT_BARRIER_DELAY);
			e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
			// Fall to sleep
			parallella_ort_power_save();
#ifdef MEASURE
			sleep_time += ORT_BARRIER_DELAY - e_ctimer_get(E_CTIMER_0);
#endif
#endif
			bar_sense = read_from_master_local_address(&(bar->sense));
		}
	}
	else /* I am the last thread in gathering step */
	{
		volatile parallella_runtime_mem_t *pll_ort;
		pll_ort = (parallella_runtime_mem_t *)(EPIPHANY_BASE_ADDRESS +
		                                       PARALLELLA_ORT_MEM_OFFSET);
		int sibling_core, thread_num;
		int my_bar_level = me->activelevel - 1;
		int i, j;
		/* Rewind barrier */
		local_in = 0;
		write_to_master_local_address(&(bar->in_barrier), (void *)local_in);

		/* Release other PEs */
		local_sense = !local_sense;
		write_to_master_local_address(&(bar->sense), (void *)local_sense);

#ifdef SLEEP
		if (me->thread_num == 0) /* Lucky master thread... */
		{
			/* Wake up sleeping child threads */
			for (thread_num = 1; thread_num < local_team_members; thread_num++)
			{
				sibling_core =
				  pll_ort->team_members[my_bar_level][me->parent->core][thread_num];

				i = sibling_core / PARALLELLA_COLS;
				j = sibling_core % PARALLELLA_COLS;
				e_irq_set(i, j, E_TIMER0_INT);
			}
		}
		else
		{
			/* Wake up sleeping siblings threads but me */
			for (thread_num = 1; thread_num < local_team_members; thread_num++)
			{
				sibling_core =
				  pll_ort->team_members[my_bar_level][me->parent->core][thread_num];
				if (sibling_core != me->core)
				{
					i = sibling_core / PARALLELLA_COLS;
					j = sibling_core % PARALLELLA_COLS;
					e_irq_set(i, j, E_TIMER0_INT);
				}
			}

			/* Wake up master thread */
			i = (me->parent->core) / PARALLELLA_COLS;
			j = (me->parent->core) % PARALLELLA_COLS;
			e_irq_set(i, j, E_TIMER0_INT);
		}
#endif

		/* Release lock */
		ee_unset_lock(&(bar->barlock));
	}

	/* Just to be sure */
	ort_execute_tasks(me, 1);

	return 0;
}

#else

int e_my_barrier(void)
{
	int thread_num, numcores, i;
	private_eecb_t  *me = __MYCB;
	int my_bar_level;
	volatile e_barrier_t *bar_array;
	         e_barrier_t **tgt_bar_array;

	numcores   = me->num_siblings;
	if (numcores == 1) return 0;

	my_bar_level = me->activelevel - 1;

	bar_array       = team_barrier[my_bar_level].barriers;
	tgt_bar_array  = team_barrier[my_bar_level].tgt_bars;

	numcores   = me->num_siblings;
	thread_num = me->thread_num;

	// Barrier as a Flip-Flop
	if (thread_num == 0)
	{
#ifdef SLEEP
		volatile parallella_runtime_mem_t *pll_ort;
		pll_ort = (parallella_runtime_mem_t *)(EPIPHANY_BASE_ADDRESS +
		                                       PARALLELLA_ORT_MEM_OFFSET);
		int sibling_core;
		int j;
#endif

		// Flip pass
		// set "my" slot
		bar_array[thread_num] = 1;

		/* Execute tasks... */
		ort_execute_tasks(me, 1);

		// poll on all slots
		for (i = 1; i < numcores; i++)
			while (bar_array[i] == 0)
			{
				/* Execute tasks while waiting... */
				ort_execute_tasks(me, 1);
			};

		/* Just to be sure... */
		ort_execute_tasks(me, 1);
		// Flop pass
		// clear all local slots
		for (i = 0; i < numcores; i++)
			bar_array[i] = 0;

		// set remote slots
		for (i = 1; i < numcores; i++)
			*(tgt_bar_array[i]) = 1;

#ifdef SLEEP
		/* Wake up sleeping threads */
		for (thread_num = 1; thread_num < numcores; thread_num++)
		{
			sibling_core = pll_ort->team_members[my_bar_level][me->core][thread_num];

			i = sibling_core / PARALLELLA_COLS;
			j = sibling_core % PARALLELLA_COLS;
			e_irq_set(i, j, E_TIMER0_INT);
		}
#endif
	}
	else
	{
		/* Execute tasks... */
		ort_execute_tasks(me, 1);

		// Flip pass
		// set "my" remote slot
		*(tgt_bar_array[0]) = 1;

		// Flop pass
		// poll on "my" local slot
		while (bar_array[0] == 0)
		{
			/* Execute tasks while waiting... */
			ort_execute_tasks(me, 1);
#ifdef SLEEP
			e_ctimer_stop(E_CTIMER_0);
			e_ctimer_set(E_CTIMER_0, ORT_BARRIER_DELAY);
			e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
			// Fall to sleep
			parallella_ort_power_save();
#ifdef MEASURE
			sleep_time += ORT_BARRIER_DELAY - e_ctimer_get(E_CTIMER_0);
#endif
#endif
		}

		/* Just to be sure... */
		ort_execute_tasks(me, 1);

		// clear "my" local slot
		bar_array[0] = 0;
	}

	return 0;
}

#endif

int ort_barrier_me(void)
{
	return ort_taskwait(2);
}

