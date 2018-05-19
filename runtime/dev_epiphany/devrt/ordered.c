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
#include "device_globals.h"
#include <stdarg.h>
#include <stdlib.h>

/*
 * ORDERED
 */

void ort_thischunk_range(int lb, int ub)
{
	private_eecb_t *me;
	(me = __MYCB)->chunklb = lb;
	me->chunkub = ub;
}


void ort_ordered_begin()
{
	volatile ort_ordered_info_t *o;
	volatile int *oi;
	private_eecb_t     *me = __MYCB;
	volatile int delay, delay2;

	o = (volatile ort_ordered_info_t *) &
	    (me->parent->workshare.blocking.forloop.ordering);

	if (me->num_siblings == 1)
		return;

#ifdef FNW
	if (me->nowaitregion)
		o = /* 1 less since mynextNWregion has been increased upon entrance */
		  &(me->parent->workshare.REGION(me->mynextNWregion).forloop.ordering);
#endif

	/* Busy wait (we could make it more sophisticated) */
	//while(me->chunklb > o->next_iteration || o->next_iteration > me->chunkub);
	ee_set_lock((void *)&o->lock);
	oi = &(o->next_iteration);
	while (me->chunklb > *oi || *oi > me->chunkub)
	{
		ee_unset_lock((void *)&o->lock);
		for (delay = 0, delay2 = 0; delay < 100; delay++)
		{
			delay2++;
			if (delay2 < 0)
				delay2 = 0;
		}
		ee_set_lock((void *)&o->lock);
		oi = &(o->next_iteration);
	}
	ee_unset_lock((void *)&o->lock);
}


void ort_ordered_end()
{
	volatile ort_ordered_info_t *o;
	volatile int *oi;
	private_eecb_t     *me = __MYCB;

	if (me->num_siblings == 1)
		return;

	o = &(me->parent->workshare.blocking.forloop.ordering);

#ifdef FNW
	if (me->nowaitregion)
		o = /* 1 less since mynextNWregion has been increased upon entrance */
		  &(me->parent->workshare.REGION(me->mynextNWregion - 1).forloop.ordering);
#endif

	ee_set_lock((void *)&o->lock);
	oi = &(o->next_iteration);
	*oi = *oi + 1;
	ee_unset_lock((void *)&o->lock);
}
