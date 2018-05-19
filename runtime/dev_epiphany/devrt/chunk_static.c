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

/* Return the sole chunk a thread gets assigned
 */
int ort_get_static_default_chunk(int niters, int *fiter, int *liter)
{
	private_eecb_t *me = __MYCB;
	div_t      dchunksize;
	int        chunksize, N = me->num_siblings, myid = me->thread_num;

	if (N == 1)
	{
		*fiter = 0;
		*liter = niters;
		return (*fiter != *liter);
	}
	if (niters <= N)    /* less iterations than threads */
	{
		*fiter = myid;
		*liter = (myid < niters) ? myid + 1 : myid;
		return (*fiter != *liter);
	}

	dchunksize = div(niters, N);
	chunksize = dchunksize.quot;                 /* iterations in a chunk */
	niters = dchunksize.rem;
	if (niters) chunksize++;     /* first niters threads get this chunksize */

	if (myid < niters || niters == 0)       /* I get a full chunk */
	{
		*fiter = myid * chunksize;
		*liter = *fiter + chunksize;
	}
	else                                  /* I get a smaller chunk */
	{
		*fiter = niters * chunksize + (myid - niters) * (chunksize - 1);
		*liter = *fiter + (chunksize - 1);
	}
	return (*fiter != *liter);
}