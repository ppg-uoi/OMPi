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
 * The inner workings of what follows is documented seperately in
 * the OMPi docs/ directory.
 */

int ort_get_dynamic_chunk(int niters, int chunksize, int *fiter, int *liter,
                          int *ignored, ort_gdopt_t *t)
{
	if (chunksize <= 0) return 0;

	if (t->nth == 1)
	{
		if (t->data == NULL) return (0);      /* t->data used specially here */
		*fiter = 0;                    /* Get just 1 chunk: all iterations */
		*liter = niters;
		t->data = NULL;
		return (1);
	}
	else
	{
		/* t->data shall hold the next iter to give away */
		if (*(t->data) >= niters) { *fiter = niters + 1; return (0); } /* done */

		ee_set_lock((ee_lock_t *) t->lock);
		*fiter = *(t->data);
		(*(t->data)) += chunksize;
		ee_unset_lock((ee_lock_t *) t->lock);

		*liter = *fiter + chunksize;
		if (*liter > niters)
			*liter = niters;
	}
	return (1);
}
