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
#include "device.h"

pe_shared_t     mbox[PE_SHAREDDATA_SIZE] SECTION("section_shared_bank"); // Last 2KB of bank 3
private_eecb_t  default_eecb             SECTION("section_shared_bank"); // Last 2KB of bank 3
private_eecb_t *myeecb                   SECTION("section_shared_bank"); // Last 2KB of bank 3
ee_lock_t       atomic_lock              SECTION("section_shared_bank"); // Last 2KB of bank 3
ee_lock_t       sh_lock                  SECTION("section_shared_bank"); // Last 2KB of bank 3
ee_lock_t       critical_lock            SECTION("section_shared_bank"); // Last 2KB of bank 3
ee_lock_t       reduction_lock           SECTION("section_shared_bank"); // Last 2KB of bank 3

#ifdef USER_LOCKS
locking_t       lock_table;
#endif

#ifdef MEASURE
	unsigned int sleep_time              SECTION("section_shared_bank");
#endif

#ifndef OLD_BAR
	ee_barrier_t team_barrier[MAX_ACTIVE_LEVELS];
#endif


/*************************************************************************************/
extern void *_kernelFunc_(void *__arg, void *__decl_arg);
extern void *_bindFunc_(void * __decl_data);
/*************************************************************************************/

void __attribute__((interrupt)) bar_wake_isr(int signum)
{
	e_ctimer_stop(E_CTIMER_0);
	return;
}

/*
 * Coreid is 12-bit value, aligned to lsb of the result.
 * This function translates this value to range (0, 15)
 * and sets row and column to values (0, 3)
 */
unsigned get_my_core_from_coreid(unsigned coreid, unsigned *row,  unsigned *col)
{
	/*
	 * 4032 = 111111000000
	 *   63 = 000000111111
	 */
	unsigned row_mask = 4032, col_mask = 63;

	*row = ((coreid & row_mask) >> 6) - 32;
	*col = (coreid & col_mask) - 8;

	return (*row) * PARALLELLA_COLS + (*col);
}

int main(void)
{
	e_coreid_t coreid;
	unsigned int row, col, core;
	volatile parallella_runtime_mem_t *pll_ort;
	void *arg, *decl_arg;

	/* Find my id */
	coreid = e_get_coreid();
	core = get_my_core_from_coreid(coreid, &row, &col);

	default_eecb.thread_num = 0;
	default_eecb.core = core;
	__SETMYCB(&default_eecb);

	/* Get shared memory with HOST */
	pll_ort = (parallella_runtime_mem_t *)(EPIPHANY_BASE_ADDRESS +
	                                       PARALLELLA_ORT_MEM_OFFSET);

	/* Prepare to receive signals */
	e_irq_attach(E_TIMER0_INT, bar_wake_isr);
	e_irq_mask(E_TIMER0_INT, E_FALSE);

#ifdef MEASURE
	/* Measure barrier sleep time */
	sleep_time = 0;
	pll_ort->time[core] = 0;
#endif

	if (core == 0)
		sh_init();

	/* I am a stand alone thread */
	if (pll_ort->master_thread[core] == -1)
	{
		/* Initialize ort global variables */
		ort_init();

		__MYCB->activelevel = 0;
		__MYCB->num_siblings = 1;
		__MYCB->parent = NULL;

		/* Get kernel arguments */
		arg       = (void *)(EPIPHANY_BASE_ADDRESS + pll_ort->kernel_args_offset[core]);
		decl_arg = (void *)(EPIPHANY_BASE_ADDRESS + pll_ort->kernel_decl_args_offset[core]);

		/* Create implicit task here... */
		mbox[0].implicit_task.parent = NULL;
		mbox[0].implicit_task.isfinal = 0;
		mbox[0].implicit_task.num_children = 0;
		mbox[0].implicit_task.status = 2;
		ee_init_lock(&(mbox[0].implicit_task.lock), 0);

		/* TODO: Add icvs here. */
		mbox[0].implicit_task.icvs.dynamic = pll_ort->dynamic;
		mbox[0].implicit_task.icvs.nested  = pll_ort->nested;

		/* Set this task as current executing task */
		__SETCURRTASK(__MYCB, &(mbox[0].implicit_task));

		/* Initialize my tasking pool */
		ort_init_worker_tasking();

		/* PE will now execute kernel func */
		_kernelFunc_(arg, decl_arg);
	}
	else
	{
		default_eecb.parent_level = pll_ort->master_level [core];
		default_eecb.parent_core  = pll_ort->master_thread[core];
		default_eecb.parent_row   = pll_ort->master_thread[core] / PARALLELLA_COLS;
		default_eecb.parent_col   = pll_ort->master_thread[core] % PARALLELLA_COLS;

		decl_arg = (void *)(EPIPHANY_BASE_ADDRESS + pll_ort->kernel_decl_args_offset[default_eecb.parent_core]);

		/* Register my declared variables */
		_bindFunc_(decl_arg);

		do_work(); /* I am a worker thread... */
	}

	/* Denote that my job is finished */
	if (pll_ort->master_thread[core] == -1) /* If I am a stand alone thread */
	{
		pll_ort->pe_exit[core] = 1; /* HOST is waiting for me */
	}
	else
	{
		pll_ort->master_thread[core] = -1; /* May this is not needed */
		//pll_ort->pe_exit[core] = -1; /* NOBODY is waiting for me */
	}

	__asm__ __volatile__("idle");

	return 0;
}

/**
 * Returns a pointer in the device address space.
 * Called from within a device. Normally simply returns "uaddr".
 * Useful when hm_get_dev_address can't provide a pointer to the device address
 * space.
 *
 * @param uaddr (unmapped) device local address
 * @param size  the size of the memory that we need to map
 *
 * @return the mapped address accessible from kernel code
 */
void *devrt_get_dev_address(void *uaddr, unsigned long size)
{
	return uaddr;
}
