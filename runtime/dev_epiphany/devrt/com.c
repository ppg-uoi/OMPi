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

/* This function reads the data written in master's local address */
void *read_address_from_master_local_address(void *local_address)
{
	void  **parent_data_p;

	/* Get the global address of parent data pointer */
	parent_data_p = ort_e_get_global_master_address(local_address);

	/* Get the global address of data stored in master's memory */
	return ort_e_get_global_master_address(*parent_data_p);
}

/* This function reads data from master's local address
 * (Sizeof data is only 4bytes)
 */
void *read_from_master_local_address(void *local_address)
{
	void  **parent_data_p;

	/* Get the global address of parent data pointer */
	parent_data_p = ort_e_get_global_master_address(local_address);

	return *parent_data_p;
}

/* This function writes data in master's local address
 * (Sizeof data is only 4bytes)
 */
void write_to_master_local_address(void *local_address, void *data)
{
	void  **parent_data_p;

	/* Get the global address of parent data pointer */
	parent_data_p = ort_e_get_global_master_address(local_address);

	*parent_data_p = data;
}

void *ort_e_get_global_address(int core, const void *ptr)
{
	unsigned uptr;
	e_coreid_t coreid;

	/* If the address is global, return the pointer unchanged */
	if (((unsigned) ptr) & 0xfff00000)
	{
		uptr = (unsigned) ptr;
		return (void *) uptr;
	}
	else
		if (core == __MYCB->core)
			coreid = e_get_coreid();
		else
			coreid = (core / PARALLELLA_COLS  + 32) * 0x40 + (core % PARALLELLA_COLS + 8);

	/* Get the 20 ls bits of the pointer and add coreid. */
	uptr = (unsigned) ptr;
	uptr = (coreid << 20) | uptr;

	return (void *) uptr;
}

void *ort_e_get_global_master_address(const void *ptr)
{
	unsigned uptr;
	e_coreid_t coreid;

	/* If the address is global, return the pointer unchanged */
	if (((unsigned) ptr) & 0xfff00000)
	{
		uptr = (unsigned) ptr;
		return (void *) uptr;
	}
	else
		coreid = (__MYCB->parent_row + 32) * 0x40 + (__MYCB->parent_col + 8);

	/* Get the 20 ls bits of the pointer and add coreid. */
	uptr = (unsigned) ptr;
	uptr = (coreid << 20) | uptr;

	return (void *) uptr;
}

/*
 * This function returns the global shared address of variable
 * that is stored in a PE's local memory.
 */
void *ort_dev_gaddr(void *local_address)
{
	return ort_e_get_global_address(__MYCB->core, local_address);
}
