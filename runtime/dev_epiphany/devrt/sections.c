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


/* This is called in the parser-generated code *only* if we are in a
 * parallel region AND the number of threads is > 1.
 */
void ort_entering_sections(int nowait, int numberofsections)
{
	enter_workshare_region(__MYCB, _OMP_SECTIONS, nowait, 0, numberofsections);
}


/* Ditto */
void ort_leaving_sections()
{
	leave_workshare_region(__MYCB, _OMP_SECTIONS);
}


/* Returns the id of the next section to execute.
 * Returns < 0 if no more sections left.
 * This is guaranteed (by the parser) to be called only if we are
 * within a PARALLEL region with > 1 threads.
 */
int ort_get_section()
{
	wsregion_t *r;
	int        s;
	private_eecb_t *me = __MYCB;

	s = me->mynextNWregion - 1;  /* My region ('cause it is 1 ahead) */

	r = &(me->parent->workshare.blocking);

#ifdef FNW
	if (me->nowaitregion)
		r = &(me->parent->workshare.REGION(s));
#endif

	if (r->sectionsleft < 0) return (-1);

	ee_set_lock(&r->reglock);
	s = --(r->sectionsleft);
	ee_unset_lock(&r->reglock);

	return (s);
}