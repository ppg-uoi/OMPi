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

void ort_barrier_init(ee_barrier_t *bar, int team_members)
{
	bar->team_members = team_members;
	bar->in_barrier   = 0;
	bar->sense        = 0;

	if (__MYCB->have_created_team == 0)
		ee_init_lock(&(bar->barlock), 0);
}

#else

/*
 * This function must be called from all team threads,
 * not just the master of the team!
 */
void ort_barrier_init(void)
{
	int thread_num, numcores;
	private_eecb_t  *me = __MYCB;
	int my_bar_level = me->activelevel - 1;
	volatile e_barrier_t *bar_array       = team_barrier[my_bar_level].barriers;
	         e_barrier_t **tgt_bar_array  = team_barrier[my_bar_level].tgt_bars;

	numcores   = me->num_siblings;

	for (thread_num = 0; thread_num < numcores; thread_num++)
		bar_array[thread_num] = 0;

	thread_num = me->thread_num;

	if (thread_num == 0)
	{
		volatile parallella_runtime_mem_t *pll_ort;
		pll_ort = (parallella_runtime_mem_t *)(EPIPHANY_BASE_ADDRESS +
		                                       PARALLELLA_ORT_MEM_OFFSET);

		for (thread_num = 1; thread_num < numcores; thread_num++)
			tgt_bar_array[thread_num] = (e_barrier_t *) ort_e_get_global_address
			                            (pll_ort->team_members[my_bar_level][me->core][thread_num],
			                             (void *) & (bar_array[0]));
	}
	else
		tgt_bar_array[0] = (e_barrier_t *) ort_e_get_global_address(me->parent_core,
		                                                            (void *) & (bar_array[thread_num]));

	return;
}

#endif
