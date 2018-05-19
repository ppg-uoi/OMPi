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

/* Enter a workshare region, update the corresponding counters
 * and return 1 if i am the first thread to enter the region.
 * It is guaranteed (by the parser and the runtime routines)
 * that we are within a parallel region with > 1 threads.
 */
int enter_workshare_region(private_eecb_t *me,
                           int wstype, int nowait, int hasordered,
                           int nsections)
{
	ort_workshare_t *ws;
	wsregion_t      *r;
	int             notfirst = 1;
#ifdef FNW
	int             myreg;
	myreg = me->mynextNWregion;
#endif

	ws = &(me->parent->workshare);
	r = &(ws->blocking);

	//me->nowaitregion = nowait;  /* Remember the status of this region */

#ifdef FNW
	if (nowait)
	{
		r = &(ws->REGION(myreg));
		/* If too many workshare regions are active, we must block here
		 * until the tail has advanced a bit.
		 */
		while (ws->headregion - ws->tailregion == MAXACTIVEREGIONS  &&
		       myreg == ws->headregion);
		(me->mynextNWregion)++;    /* Make me ready for the next region */
	}
#endif

	if (!r->empty) return (0);             /* We are not the first for sure */

	ee_set_lock(&r->reglock);       /* We'll be short */
	if (r->empty)                           /* I am the first to enter */
	{
		if (wstype == _OMP_SECTIONS)          /* Initialize */
			r->sectionsleft = nsections;
		else
			if (wstype == _OMP_FOR)
			{
				r->forloop.iter = 0;    /* The very 1st iteration to be given away */
				if (hasordered)
					r->forloop.ordering.next_iteration = 0;
			}

		r->left = 0;      /* None left yet */
#ifdef FNW
		/* Now, prepare/initialize the next region, if not already inited.
		 * This is done so as to avoid serially initializing all regions when
		 * the team was created
		 */
		if (nowait && myreg + 1 < MAXACTIVEREGIONS)
		{
			if (!ws->REGION(myreg + 1).inited)
			{
				ee_init_lock(&ws->REGION(myreg + 1).reglock, ORT_LOCK_SPIN);
				ee_init_lock(&ws->REGION(myreg + 1).forloop.ordering.lock, ORT_LOCK_SPIN);
				ws->REGION(myreg + 1).inited = 1;
			}
			ws->REGION(myreg + 1).empty = 1;
		}

		/* Do this last to avoid races with threads that test without locking */
		if (nowait)(ws->headregion)++;                    /* Advance the head */
#endif

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
	ort_workshare_t *ws;
	wsregion_t      *r;
	int             left;

#ifdef FNW
	int             myreg;
	myreg = me->mynextNWregion - 1;   /* It is ahead by 1 */
#endif

	ws = &(me->parent->workshare);
	r = &(ws->blocking);

	/* Well, it would be better if we used a seperate lock for leaving */
	ee_set_lock(&r->reglock);
	left = ++(r->left);
	ee_unset_lock(&r->reglock);

	if (left == me->num_siblings)
	{
		r->empty = 1;                     /* The region is now empty */
		r->left = 0;
#ifdef FNW
		if (me->nowaitregion)
			(ws->tailregion)++;             /* Advance the tail */
		else
#endif
			if (wstype == _OMP_FOR)         /* blocking for */
			{
				r->forloop.iter = 0;          /* Prepare for next use */
				r->forloop.ordering.next_iteration = 0;
			}
	}

	return (left);
}
