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

void ort_atomic_begin(void)
{
	private_eecb_t *find_lp = __MYCB->parent;

	if (find_lp == NULL) return; /* Serial code */

	/* Find the owner of the lock */
	while (find_lp->parent != NULL) find_lp = find_lp->parent;

	ee_set_lock(ort_e_get_global_address(find_lp->core,
	                                     &atomic_lock));    /* ##### */
}


void ort_atomic_end(void)
{
	private_eecb_t *find_lp = __MYCB->parent;

	if (find_lp == NULL) return; /* Serial code */

	/* Find the owner of the lock */
	while (find_lp->parent != NULL) find_lp = find_lp->parent;

	ee_unset_lock(ort_e_get_global_address(find_lp->core,
	                                       &atomic_lock));    /* ##### */
}