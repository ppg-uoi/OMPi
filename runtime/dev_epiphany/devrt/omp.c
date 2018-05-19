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
#include "device_globals.h"

int omp_is_initial_device(void)
{
	return 0;
}

int omp_get_thread_num(void)
{
	return __MYCB->thread_num;
}

int omp_in_parallel(void)
{
	return (__MYCB->activelevel != 0);
}

int omp_get_num_threads(void)
{
	return __MYCB->num_siblings;
}

int omp_in_final(void)
{
	return __CURRTASK(__MYCB)->isfinal;
}

int omp_get_num_procs(void)
{
	return PARALLELLA_CORES;
}

double omp_get_wtime(void)
{
	return 0.0;
}

double omp_get_wtick(void)
{
	return 0.0;
}
