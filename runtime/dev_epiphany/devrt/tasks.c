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

void *ort_task_immediate_start(int final)
{
	return NULL;
}

void ort_task_immediate_end(void *my_eecb)
{
}

void *ort_taskenv_alloc(int size, void *(*task_func)(void *))
{
	volatile parallella_runtime_mem_t *pll_ort;
	unsigned int memory;

	/* Get shared memory with HOST */
	pll_ort = (parallella_runtime_mem_t *)(EPIPHANY_BASE_ADDRESS +
	                                       PARALLELLA_ORT_MEM_OFFSET);

	/* Wait while sh_lock is initialized... :-( */
	while (pll_ort->sh_lock_ready != 1);

	/* Lock shared memory */
	ee_set_lock(ort_e_get_global_address(0, &sh_lock));

	if (pll_ort->te_free_memory_offset - size < 0)
		pll_ort->te_free_memory_offset =
		  OMPI_TASKENV_MEM_BEGIN; /* Rewind shared memory */

	/* Get the pointer to shared memory */
	pll_ort->te_free_memory_offset -= size;
	memory = pll_ort->te_free_memory_offset;

	ee_unset_lock(ort_e_get_global_address(0, &sh_lock));

	return (void *) memory;
}

void ort_taskenv_free(void *ptr, void *(*task_func)(void *))
{
}

#ifdef LOCK_TASK_QUEUE

int ort_task_throttling(void)
{
	private_eecb_t  *me = __MYCB;

	if (me->num_siblings == 1)
		return 1;

	if (me->parent->tasking.free_cell < TASK_QUEUE_SIZE)
		return 0;

	return 1;
}

void ort_task_check_throttling(private_eecb_t *me)
{
}

#else

int ort_task_throttling(void)
{
	return 0;
}

void ort_task_check_throttling(private_eecb_t *me)
{
}

#endif
