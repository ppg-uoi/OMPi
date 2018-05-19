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

void omp_set_num_threads(int num_threads)
{
	return;
}

int  omp_get_max_threads(void)
{
	return PARALLELLA_CORES;
}

void omp_set_dynamic(int dyn)
{
	__CURRTASK(__MYCB)->icvs.dynamic = dyn;
}

int omp_get_dynamic(void)
{
	return (__CURRTASK(__MYCB)->icvs.dynamic);
}

void omp_set_nested(int nest)
{
	__CURRTASK(__MYCB)->icvs.nested = nest;
}

int  omp_get_nested(void)
{
	return (__CURRTASK(__MYCB)->icvs.nested);
}

void omp_set_schedule(omp_sched_t kind, int chunk)
{
	return;
}

void omp_get_schedule(omp_sched_t *kind, int *chunk)
{
	return;
}

int  omp_get_thread_limit(void)
{
	return PARALLELLA_CORES;
}

void omp_set_max_active_levels(int levels)
{
	return;
}

int  omp_get_max_active_levels(void)
{
	return MAX_ACTIVE_LEVELS;
}

int  omp_get_level(void)
{
	return __MYCB->activelevel;
}

int  omp_get_ancestor_thread_num(int level)
{
	return 0;
}

int  omp_get_team_size(int level)
{
	return 0;
}

int  omp_get_active_level(void)
{
	return __MYCB->activelevel;
}
