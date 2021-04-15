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

void init_workshare_regions(private_eecb_t *me)
{
	wsregion_t      *r = &(me->blocking);

	if (!r->inited)
	{
		ee_init_lock((void *)&(r->reglock), 0);
		r->inited = 1;
	}

	r->empty = 1;
	r->left = 0;       /* Needed for optimizing blocking for regions */
}
