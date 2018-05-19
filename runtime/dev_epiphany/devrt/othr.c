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
#include "device_globals.h"

int ee_init_lock(ee_lock_t *lock, int type)
{
	e_mutex_t *gmutex;

	gmutex = (e_mutex_t *) ort_e_get_global_address(__MYCB->core, &(lock->mutex));
	lock->owner = __MYCB->core;
	*gmutex = 0x0;

	return 0;
}

int ee_set_lock(ee_lock_t *lock)
{
	e_mutex_t *gmutex;
	register unsigned coreid, offset;
	coreid = e_get_coreid();
	gmutex = (e_mutex_t *) ort_e_get_global_address(lock->owner, &(lock->mutex));
	offset = 0x0;
	do
	{
		__asm__ __volatile__("testset %[r0], [%[r1], %[r2]]" : [r0] "+r"(coreid) : [r1] "r"(gmutex), [r2] "r"(offset));
	}
	while (coreid != 0);

	return 0;
}

int ee_unset_lock(ee_lock_t *lock)
{
	e_mutex_t *gmutex;
	gmutex = (e_mutex_t *) ort_e_get_global_address(lock->owner, &(lock->mutex));
	*gmutex = 0x0;

	return 0;
}

int ee_test_lock(ee_lock_t *lock)
{
	e_mutex_t *gmutex;
	register unsigned coreid, offset;

	coreid = e_get_coreid();
	gmutex = (e_mutex_t *) ort_e_get_global_address(lock->owner, &(lock->mutex));
	offset = 0x0;

	__asm__ __volatile__("testset %[r0], [%[r1], %[r2]]" : [r0] "+r"(coreid) : [r1] "r"(gmutex), [r2] "r"(offset));

	return coreid;
}
