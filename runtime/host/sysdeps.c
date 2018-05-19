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

#include "sysdeps.h"
#include <stdio.h>


#ifdef __FENCE_MUTEX__
#include <pthread.h>
pthread_mutex_t _flusher = PTHREAD_MUTEX_INITIALIZER;
void _bad_fence(void)
{
	pthread_mutex_lock(&_flusher);
	pthread_mutex_unlock(&_flusher);
}
#undef __FENCE_MUTEX__
#endif


/* Determine the # of processors
 */
int ort_get_num_procs(void)
{
	int np;
#ifdef __SYSOS_irix
	np = sysconf(_SC_NPROC_ONLN);
#else
	np = sysconf(_SC_NPROCESSORS_ONLN);
#endif
	if (np <= 0)
	{
		extern void ort_warning(char *, ...);   /* So icc does not complain */
		ort_warning("cannot determine the number of processors; assuming %d.\n",
		            np = 1);
	}
	return (np);
}
