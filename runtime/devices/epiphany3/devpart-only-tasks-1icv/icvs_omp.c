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
	private_eecb_t  *find_icvp = __MYCB->parent;

	if (find_icvp == NULL)
	{
		global_icvs.dynamic = dyn;
	}
	else
	{
		ort_task_icvs_t *gl_icv;

		/* Find the owner of the icvs */
		while (find_icvp->parent != NULL)
			find_icvp = find_icvp->parent;

		gl_icv = ort_e_get_global_address(find_icvp->core, &global_icvs);
		gl_icv->dynamic = dyn;
	}
}

int omp_get_dynamic(void)
{
	private_eecb_t  *find_icvp = __MYCB->parent;

	if (find_icvp == NULL)
	{
		return global_icvs.dynamic;
	}
	else
	{
		ort_task_icvs_t *gl_icv;

		/* Find the owner of the icvs */
		while (find_icvp->parent != NULL)
			find_icvp = find_icvp->parent;

		gl_icv = ort_e_get_global_address(find_icvp->core, &global_icvs);
		return gl_icv->dynamic;
	}
}

void omp_set_nested(int nest)
{
	private_eecb_t  *find_icvp = __MYCB->parent;

	if (find_icvp == NULL)
	{
		global_icvs.nested = nest;
	}
	else
	{
		ort_task_icvs_t *gl_icv;

		/* Find the owner of the icvs */
		while (find_icvp->parent != NULL)
			find_icvp = find_icvp->parent;

		gl_icv = ort_e_get_global_address(find_icvp->core, &global_icvs);
		gl_icv->nested = nest;
	}
}

int  omp_get_nested(void)
{
	private_eecb_t  *find_icvp = __MYCB->parent;

	if (find_icvp == NULL)
	{
		return global_icvs.nested;
	}
	else
	{
		ort_task_icvs_t *gl_icv;

		/* Find the owner of the icvs */
		while (find_icvp->parent != NULL)
			find_icvp = find_icvp->parent;

		gl_icv = ort_e_get_global_address(find_icvp->core, &global_icvs);
		return gl_icv->nested;
	}
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
