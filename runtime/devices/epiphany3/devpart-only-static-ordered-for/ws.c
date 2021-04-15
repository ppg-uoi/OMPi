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

/* Enter a workshare region, update the corresponding counters
 * and return 1 if i am the first thread to enter the region.
 * It is guaranteed (by the parser and the runtime routines)
 * that we are within a parallel region with > 1 threads.
 */
int enter_workshare_region(private_eecb_t *me,
                           int wstype, int nowait, int hasordered,
                           int nsections)
{
	wsregion_t      *r;
	int             notfirst = 1;

	r = &(me->parent->blocking);

	if (!r->empty) return (0);             /* We are not the first for sure */

	ee_set_lock(&r->reglock);       /* We'll be short */
	if (r->empty)                           /* I am the first to enter */
	{
		if (wstype == _OMP_FOR)
		{
			r->forloop.iter = 0;    /* The very 1st iteration to be given away */
			if (hasordered)
				r->forloop.ordering.next_iteration = 0;
		}

		r->left = 0;      /* None left yet */
		r->empty = notfirst = 0;
	}
	ee_unset_lock(&r->reglock);

	return (!notfirst);
}

/* Returns the # of threads that have not yet left the region.
 * A zero means that I am the last thread to leave.
 */
int leave_workshare_region(private_eecb_t *me, int wstype)
{
	wsregion_t      *r;
	int             left;

	r = &(me->parent->blocking);

	/* Well, it would be better if we used a seperate lock for leaving */
	ee_set_lock(&r->reglock);
	left = ++(r->left);
	ee_unset_lock(&r->reglock);

	if (left == me->num_siblings)
	{
		r->empty = 1;                     /* The region is now empty */
		r->left = 0;

		if (wstype == _OMP_FOR)         /* blocking for */
		{
			r->forloop.iter = 0;          /* Prepare for next use */
			r->forloop.ordering.next_iteration = 0;
		}
	}

	return (left);
}
