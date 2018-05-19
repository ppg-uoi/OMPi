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

/* ort_omp.c -- OMPi RunTime library, standard OpenMP calls */

/*
 * 2010/11/20:
 *   fixed bug in omp_get_max_active_levels().
 * Version 1.0.1j:
 *   first time around, out of ort.c code.
 */

#include "ort_prive.h"
#include <stdlib.h>
#include <time.h>
#if !defined(HAVE_TIMESPEC) || defined(NO_CLOCK_GETTIME)
	#include <sys/time.h>  /* For gettimeofday() */
#endif


int  omp_in_parallel(void)     { return (__MYCB->activelevel != 0); }

int  omp_get_thread_num(void)  { return (__MYCB->thread_num); }

int  omp_get_num_threads(void) { return (__MYCB->num_siblings); }

int  omp_get_max_threads(void) { return (__CURRTASK(__MYCB)->icvs.nthreads); }

int  omp_get_num_procs(void)   { return (ort->icvs.ncpus); }

void omp_set_dynamic(int dyn)
{
	ort_eecb_t *me = __MYCB;
	ort_task_icvs_t *ti = &(__CURRTASK(me)->icvs);

	if (__INHERITASK(me))
		ort_create_task_immediate_node(me);
	if (!dyn || ort->eecaps.supports_dynamic)
		ti->dynamic = dyn;
	check_nested_dynamic(ti->nested, ti->dynamic); /* is eelib ok? */
}

int  omp_get_dynamic(void)     { return (__CURRTASK(__MYCB)->icvs.dynamic); }

void omp_set_nested(int nest)
{
	ort_eecb_t      *me = __MYCB;
	ort_task_icvs_t *ti = &(__CURRTASK(me)->icvs);

	if (__INHERITASK(me))
		ort_create_task_immediate_node(me);
	if (!nest || ort->eecaps.supports_nested)
		ti->nested = nest;
	check_nested_dynamic(ti->nested, ti->dynamic); /* is eelib ok? */
}

int  omp_get_nested(void)      { return (__CURRTASK(__MYCB)->icvs.nested); }

void omp_set_num_threads(int num_threads)
{
	/* if (!omp_in_parallel() && num_threads > 0)  was used <= (V.2.5) */
	ort_eecb_t *me = __MYCB;

	if (__INHERITASK(me))
		ort_create_task_immediate_node(me);
	__CURRTASK(me)->icvs.nthreads = num_threads;
}


void omp_init_lock(omp_lock_t *lock)
{ *lock = NULL; ort_prepare_omp_lock(lock, ORT_LOCK_NORMAL); }

void omp_set_lock(omp_lock_t *lock)
{ ee_set_lock((ee_lock_t *) *lock); }

void omp_unset_lock(omp_lock_t *lock)
{ ee_unset_lock((ee_lock_t *) *lock); }

int  omp_test_lock(omp_lock_t *lock)
{ return (ee_test_lock((ee_lock_t *) *lock)); }

void omp_init_nest_lock(omp_nest_lock_t *lock)
{ *lock = NULL; ort_prepare_omp_lock(lock, ORT_LOCK_NEST); }

void omp_set_nest_lock(omp_nest_lock_t *lock)
{
	ort_eecb_t *me = __MYCB;

	if (__INHERITASK(me))
		ort_create_task_immediate_node(me);

	ee_set_lock((ee_lock_t *) *lock);
}

void omp_unset_nest_lock(omp_nest_lock_t *lock)
{ ee_unset_lock((ee_lock_t *) *lock); }

int  omp_test_nest_lock(omp_nest_lock_t *lock)
{
	ort_eecb_t *me = __MYCB;

	if (__INHERITASK(me))
		ort_create_task_immediate_node(me);

	return (ee_test_lock((ee_lock_t *) *lock));
}

void omp_destroy_lock(omp_lock_t *lock)
{
	ee_destroy_lock((ee_lock_t *) *lock);
#if defined(EE_TYPE_PROCESS)
	ort_shmfree(*lock);
#else
	free(*lock);
#endif
}

void omp_destroy_nest_lock(omp_nest_lock_t *lock)
{
	ee_destroy_lock((ee_lock_t *) *lock);
#if defined(EE_TYPE_PROCESS)
	ort_shmfree(*lock);
#else
	free(*lock);
#endif
}

double omp_get_wtime(void)
{
#if !defined(HAVE_TIMESPEC) || defined(NO_CLOCK_GETTIME)  /* PEH */
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (((double) tv.tv_sec) + ((double) tv.tv_usec) * 1.0E-6);
#else
	struct timespec ts;
	clock_gettime(SYS_CLOCK, &ts);
	return (((double) ts.tv_sec) + ((double) ts.tv_nsec) * 1.0E-9);
#endif
}

double omp_get_wtick(void)
{
#if !defined(HAVE_TIMESPEC) || defined(NO_CLOCK_GETTIME)  /* PEH */
	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);
	do
	{
		gettimeofday(&tv2, NULL);
	}
	while (tv1.tv_usec == tv2.tv_usec);
	return (((double)(tv2.tv_sec - tv1.tv_sec)) +
	        ((double)(tv2.tv_usec - tv1.tv_usec)) * 1.0E-6);
#else
	struct timespec ts;
	clock_getres(SYS_CLOCK, &ts);
	return (((double) ts.tv_sec) + ((double) ts.tv_nsec) * 1.0E-9);
#endif
}

/* OpenMP 3.0 */
void omp_set_schedule(omp_sched_t kind, int chunk)
{
	ort_eecb_t *me = __MYCB;
	ort_task_icvs_t *icvs = &(__CURRTASK(me)->icvs);

	if (__INHERITASK(me))
		ort_create_task_immediate_node(me);
	icvs->rtschedule = kind;
	icvs->rtchunk = (chunk < 1) ? -1 : chunk;  /* Use default if < 0 */
}

/* OpenMP 3.0 */
void omp_get_schedule(omp_sched_t *kind, int *chunk)
{
	ort_task_icvs_t *icvs = &(__CURRTASK(__MYCB)->icvs);
	*kind = icvs->rtschedule;
	*chunk = icvs->rtchunk;
}

/* OpenMP 4.0 */
int omp_get_thread_limit(void)
{
	ort_task_icvs_t *icvs = &(__CURRTASK(__MYCB)->icvs);
	return ((icvs->threadlimit == -1) ? (1 << 30) : icvs->threadlimit);
}

/* OpenMP 4.0.0*/
void omp_set_max_active_levels(int levels)
{
	if (levels >= 0)
		if (ort->eecaps.max_levels_supported == -1 ||
		    levels <= ort->eecaps.max_levels_supported)
			ort->icvs.levellimit = levels;
}

/* OpenMP 4.0.0*/
int omp_get_max_active_levels(void)
{
	return ((ort->icvs.levellimit == -1) ? (1 << 30) :
	        ort->icvs.levellimit);
}

/* OpenMP 3.0 */
int omp_get_level(void)
{
	return (__MYCB->level);
}

/* OpenMP 3.0 */
int omp_get_ancestor_thread_num(int level)
{
	if (level < 0) return (-1);
	else
		if (level == 0) return (0);   /* master thread is parent of them all */
		else
		{
			ort_eecb_t *me = __MYCB;
			if (me->level < level) return (-1);
			for (; me->level != level; me = me->parent)
				;
			return (me->thread_num);
		};
}

/* OpenMP 3.0 */
int omp_get_team_size(int level)
{
	if (level < 0) return (-1);
	else
		if (level == 0) return (1);
		else
		{
			ort_eecb_t *me = __MYCB;
			if (me->level < level) return (-1);
			for (; me->level != level; me = me->parent)
				;
			return (me->num_siblings);
		};
}

/* OpenMP 3.0 */
int omp_get_active_level(void)
{
	return (__MYCB->activelevel);
}

/* OpenMP 3.1 */
int omp_in_final(void)
{
	return __CURRTASK(__MYCB)->isfinal;
}

/* OpenMP 4.0 */
omp_proc_bind_t omp_get_proc_bind(void)
{
	return (__CURRTASK(__MYCB)->icvs).proc_bind;
}

/* OpenMP 4.0 */
void omp_set_default_device(int device_num)
{
	if (device_num >= 0 && device_num < ort->added_devices)
		(__CURRTASK(__MYCB)->icvs).def_device = device_num;
}

/* OpenMP 4.0 */
int  omp_get_default_device(void)
{
	return (__CURRTASK(__MYCB)->icvs).def_device;
}

/* OpenMP 4.0 */
int omp_get_cancellation(void)
{
	return ort->icvs.cancel;
}
/* OpenMP 4.0 */
int  omp_get_num_devices(void)
{
	return ort->added_devices;
}

/* OpenMP 4.0 */
int  omp_get_num_teams(void)
{
	/* Only one team per parallel for now */
	return 1;
}

/* OpenMP 4.0 */
int  omp_get_team_num(void)
{
	/* Only one team per parallel for now */
	return 0;
}

/* OpenMP 4.0 */
int  omp_is_initial_device(void)
{
	/* Programm is always executed at HOST device */
	return 1;
}
