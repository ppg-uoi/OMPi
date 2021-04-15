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
#include "device.h"
#include "device_globals.h"
#include <stdarg.h>
#include <stdlib.h>


int ort_get_dynamic_chunk(int niters, int chunksize, int monotonic, int *fiter, 
                          int *liter, int *ignored)
{
	private_eecb_t *me = __MYCB;
	int            *iter;
	ee_lock_t      *lock;
	
	if (chunksize <= 0) return 0;

	if (me->num_siblings == 1)
	{
		if (me->nowaitregion == 0) return 0;
		*fiter = 0;              /* Get just 1 chunk: all iterations */
		*liter = niters;
		me->nowaitregion = 0;    /* Ugly hack to mark 1st chunk */
		return (1);
	}
	else
	{
		/* it shall hold the next iter to give away */
		iter = &(me->parent->blocking.forloop.iter);
		if (*iter >= niters) { *fiter = niters + 1; return (0); } /* done */

		lock = &(me->parent->blocking.reglock);
		ee_set_lock(lock);
		*fiter = *iter;
		(*iter) += chunksize;
		ee_unset_lock(lock);

		*liter = *fiter + chunksize;
		if (*liter > niters)
			*liter = niters;
	}
	return (1);
}
