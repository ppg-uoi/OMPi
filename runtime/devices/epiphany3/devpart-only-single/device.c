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

#include "e_lib.h"
#include "shared_data.h"
#include "device_globals.h"

void parallella_ort_power_save(void)
{
	__asm__ __volatile__("idle");
}

void do_work(void)
{
	private_eecb_t *me = __MYCB;
	void  *(** volatile local_func)(void *);
	void  *local_args;
	private_eecb_t *local_team_parent;
	int   level_of_parent_data = me->parent_level;

	volatile parallella_runtime_mem_t *pll_ort;
	pll_ort = (parallella_runtime_mem_t *)(EPIPHANY_BASE_ADDRESS +
	                                       PARALLELLA_ORT_MEM_OFFSET);

	/* Wait until master thread is ready... */
	do
	{
		local_func = ort_e_get_global_master_address((void *)(
		                                               &mbox[level_of_parent_data].func));
	}
	while (*local_func == NULL);

	/* I get the right values from team's master */
	local_team_parent = read_address_from_master_local_address(&
	                                                           (mbox[level_of_parent_data].team_parent));
	local_args = read_address_from_master_local_address(&
	                                                    (mbox[level_of_parent_data].args));

	/* Prepare my eecb first */
	me->thread_num = pll_ort->openmp_id[me->core]; /* HOST has given me this ID */
	me->num_children = 0;
	me->have_created_team = 0;
	me->parent = local_team_parent;
	me->num_siblings = me->parent->num_children;
	me->activelevel = me->parent->activelevel + 1;

	ort_barrier_init(); /* All PEs must init their barrier */

#ifdef MEASURE
	e_ctimer_set(E_CTIMER_1, E_CTIMER_MAX);
	e_ctimer_start(E_CTIMER_1, E_CTIMER_CLK);
	/* Finally I am about to execute parallel func */
	(*local_func)((void *)(local_args));
	unsigned int end = e_ctimer_get(E_CTIMER_1);
	pll_ort->time[me->thread_num] = E_CTIMER_MAX - end;
	pll_ort->sleep[me->thread_num] = sleep_time;
#else
	/* Finally I am about to execute parallel func */
	(*local_func)((void *)(local_args));
#endif
}

