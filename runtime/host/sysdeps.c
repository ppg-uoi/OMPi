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


/* Determine the # of processors available (online)
 */
int ort_get_num_procs(void)
{
	int np;
#ifdef __SYSOS_irix
	np = (int) sysconf(_SC_NPROC_ONLN);
#else
	np = (int) sysconf(_SC_NPROCESSORS_ONLN);
#endif
	if (np <= 0)
	{
		extern void ort_warning(char *, ...);   /* So icc does not complain */
		ort_warning("cannot determine the number of processors available; assuming %d.\n",
		            np = 1);
	}
	return (np);
}

/* Determine the # of processors configured
 */
int ort_get_num_procs_conf(void)
{
	int np;
#ifdef __SYSOS_irix
	np = (int) sysconf(_SC_NPROC_CONF);
#else
	np = (int) sysconf(_SC_NPROCESSORS_CONF);
#endif
	if (np <= 0)
	{
		extern void ort_warning(char *, ...);   /* So icc does not complain */
		ort_warning("cannot determine the number of processors configured; assuming %d.\n",
		            np = 1);
	}
	return (np);
}

