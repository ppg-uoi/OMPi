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


/* This is called by the parser-generated code, even if we are
 * not in a parallel region.
 */
void ort_entering_for(int nowait, int hasordered)
{
	private_eecb_t *me = __MYCB;

	/* Check if we are not in a parallel region or the team has only 1 thread */
	if (me->num_siblings == 1)
		return;

#ifdef FNW
	if (nowait)
		enter_workshare_region(me, _OMP_FOR, nowait, hasordered, 0);
	else
#endif
		/* enter_workshare_region() is not needed anymore for blocking
		 * regions. Fields have been pre-initialized by the parent of the team
		 * and @ every use, the last thread to leave reinitilizes them.
		 */
		me->nowaitregion = nowait;  /* Remember the status of this region */
}

int ort_leaving_for(void)
{
	private_eecb_t *me = __MYCB;
	if (me->num_siblings == 1)
		return (0);
	else
		return (leave_workshare_region(me, _OMP_FOR));
}
