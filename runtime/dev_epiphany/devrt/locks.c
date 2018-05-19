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

#ifdef USER_LOCKS
/*
 * Initialize the table of locks.
 * This function is called only once, when the first
 * master core (usally eCore 0) is on business.
 */
void init_user_locks(void)
{
	int i;
	locking_t *ecore0_lock_table = ort_e_get_global_address(0, &lock_table);

	/* Initialize access lock, user locks and define them as unoccupied */
	ee_init_lock(&(ecore0_lock_table->lock), 0);
	for(i=0; i<MAX_USER_LOCKS; i++)
	{
		ee_init_lock(&(ecore0_lock_table->user_locks[i]), 0);
		ecore0_lock_table->occupied[i] = 0;
	}
}

/* Extract a lock from the pool and assign it user. */
void omp_init_lock(omp_lock_t **lock)
{
	int i;
	locking_t *ecore0_lock_table = ort_e_get_global_address(0, &lock_table);

	/* Restrict access to table */
	ee_set_lock(&(ecore0_lock_table->lock));

	/* Search for unoccupied lock */
	for(i=0; i<MAX_USER_LOCKS; i++)
	{
		if(ecore0_lock_table->occupied[i] == 0)
		{ /* Got it... */
			ecore0_lock_table->occupied[i] = 1;
			*lock = &(ecore0_lock_table->user_locks[i]);
			ee_unset_lock(&(ecore0_lock_table->lock));

			return;
		}
	}

	/* If all lock are occupied reassign the last lock */
	ecore0_lock_table->occupied[MAX_USER_LOCKS-1] ++;
	*lock = &(ecore0_lock_table->user_locks[MAX_USER_LOCKS-1]);
	ee_unset_lock(&(ecore0_lock_table->lock));
}

void omp_set_lock(omp_lock_t **lock)
{
	ee_set_lock(*lock);
}

void omp_unset_lock(omp_lock_t **lock)
{
	ee_unset_lock(*lock);
}

int  omp_test_lock(omp_lock_t **lock)
{
	return (ee_test_lock(*lock));
}

/* Returns a lock back to the pool */
void omp_destroy_lock(omp_lock_t **lock)
{
	int i;
	locking_t *ecore0_lock_table = ort_e_get_global_address(0, &lock_table);

	/* Restrict access to table */
	ee_set_lock(&(ecore0_lock_table->lock));

	/* Search for the lock */
	for(i=0; i<MAX_USER_LOCKS; i++)
	{
		if(*lock == &(ecore0_lock_table->user_locks[i]))
		{
			/*
			 * Reduce lock reference, last lock may be occupied by more
			 * threads...
			 */
			ecore0_lock_table->occupied[i] --;
			*lock = NULL;
			ee_unset_lock(&(ecore0_lock_table->lock));

			return;
		}
	}

	/* Just in case... */
	*lock = NULL;
	ee_unset_lock(&(ecore0_lock_table->lock));
}
#endif

