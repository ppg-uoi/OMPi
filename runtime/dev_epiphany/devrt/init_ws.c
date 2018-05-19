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

void init_workshare_regions(private_eecb_t *me)
{
	ort_workshare_t *ws = &me->workshare;

	if (!ws->blocking.inited)
	{
		ee_init_lock((void *)&(ws->blocking.reglock), 0);
		ee_init_lock((void *)&(ws->blocking.forloop.ordering.lock), 0);
		ws->blocking.inited = 1;
	}

	ws->blocking.empty = 1;
	ws->blocking.left = 0;       /* Needed for optimizing blocking for regions */
	ws->blocking.forloop.iter = 0; /* ditto */
	ws->blocking.forloop.ordering.next_iteration = 0;  /* ditto */

#ifdef FNW
	if (!ws->REGION(0).inited)
	{
		ee_init_lock(&ws->REGION(0).reglock, 0);
		ee_init_lock(&ws->REGION(0).forloop.ordering.lock, 0);
		ws->REGION(0).inited = 1;
	}
	ws->REGION(0).empty = 1;
	ws->headregion = ws->tailregion = 0;
#endif

	/* Default case; only needed for combined parallel for/sections */
	me->nowaitregion = 0;
}