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

/*************************************************************************************/
extern void *_kernelFunc_(void *__arg, void *__decl_arg);
/*************************************************************************************/

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

int omp_get_thread_num() {

	e_coreid_t coreid;
	coreid = e_get_coreid();
	unsigned row_mask = 4032, col_mask = 63;
  unsigned row, col;

	row = ((coreid & row_mask) >> 6) - 32;
	col = (coreid & col_mask) - 8;

	return (row) * PARALLELLA_COLS + (col);
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

	/* Get shared memory with HOST */
	pll_ort = (parallella_runtime_mem_t *)(EPIPHANY_BASE_ADDRESS +
	                                       PARALLELLA_ORT_MEM_OFFSET);

	/* Get kernel arguments */
	arg       = (void *)(EPIPHANY_BASE_ADDRESS + pll_ort->kernel_args_offset[core]);
	decl_arg = (void *)(EPIPHANY_BASE_ADDRESS + pll_ort->kernel_decl_args_offset[core]);

	/* PE will now execute kernel func */
	_kernelFunc_(arg, decl_arg);

	/* HOST is waiting for me */
	pll_ort->pe_exit[core] = 1;

	__asm__ __volatile__("idle");

	return 0;
}

/**
 * Returns a pointer in the device address space.
 * Called from within a device. Normally simply returns "uaddr".
 * Useful when hm_imed2umed_addr can't provide a pointer to the device address
 * space.
 *
 * @param uaddr (unmapped) device local address
 * @param size  the size of the memory that we need to map
 *
 * @return the mapped address accessible from kernel code
 */
char *devpart_med2dev_addr(void *uaddr, unsigned long size)
{
  if (uaddr == NULL) return (NULL);   /* Handle NULL */
  return uaddr;
}
