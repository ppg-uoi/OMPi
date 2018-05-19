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

/* Returns 1 if the current thread should execute the SINGLE block
 */
int ort_mysingle(int nowait)
{
	private_eecb_t *me = __MYCB;
	if (me->num_siblings == 1)
		return (1);
	else
		return (enter_workshare_region(me, _OMP_SINGLE, nowait, 0, 0));
}


void ort_leaving_single()
{
	private_eecb_t *me = __MYCB;
	if (me->num_siblings != 1)
		leave_workshare_region(me, _OMP_SINGLE);
}