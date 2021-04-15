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

/* ort_omp.c -- OMPi RunTime library, standard OpenMP calls */

/*
 * 2010/11/20:
 *   fixed bug in omp_get_max_active_levels().
 * Version 1.0.1j:
 *   first time around, out of ort.c code.
 */

#include "ort_prive.h"
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#if !defined(HAVE_TIMESPEC) || defined(NO_CLOCK_GETTIME)
	#include <sys/time.h>  /* For gettimeofday() */
#endif

/*
 * Execution environment routines
 */


void omp_set_num_threads(int num_threads)
{
	/* if (!omp_in_parallel() && num_threads > 0)  was used <= (V.2.5) */
	ort_eecb_t *me = __MYCB;

	if (__INHERITASK(me))
		ort_create_task_immediate_node(me);
	__CURRTASK(me)->icvs.nthreads = num_threads;
}

int  omp_get_num_threads(void) { return (__MYCB->num_siblings); }

int  omp_get_max_threads(void) { return (__CURRTASK(__MYCB)->icvs.nthreads); }

int  omp_get_thread_num(void)  { return (__MYCB->thread_num); }

int  omp_get_num_procs(void)   { return (ort->icvs.ncpus); }

int  omp_in_parallel(void)     { return (__MYCB->activelevel != 0); }

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

/* OpenMP 4.0 */
int omp_get_cancellation(void)
{
	return CANCEL_ENABLED();
}

/* DEPRECATED 
 * OpenMP 5.0
 */
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

/* DEPRECATED 
 * OpenMP 5.0
 */
int  omp_get_nested(void)      { return (__CURRTASK(__MYCB)->icvs.nested); }

/* OpenMP 3.0 */
void omp_set_schedule(omp_sched_t kind, int chunk)
{
	ort_eecb_t *me = __MYCB;
	ort_task_icvs_t *icvs = &(__CURRTASK(me)->icvs);

	if (__INHERITASK(me))
		ort_create_task_immediate_node(me);
	icvs->rtschedule = kind;
	icvs->rtchunk = (chunk <= 0) ? 0 : chunk;  /* Use default if < 0 */
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
	return (icvs->threadlimit);
}

/* OpenMP 4.0*/
void omp_set_max_active_levels(int levels)
{
	if (levels >= 0)
		if (ort->eecaps.max_levels_supported == -1 ||
		    levels <= ort->eecaps.max_levels_supported)
			ort->icvs.levellimit = levels;
}

/* OpenMP 4.0*/
int omp_get_max_active_levels(void)
{
	return (ort->icvs.levellimit);
}

/* OpenMP 5.0*/
int omp_get_supported_active_levels(void)
{
	return (ort->eecaps.max_levels_supported == -1) ? 
	       INT_MAX : ort->eecaps.max_levels_supported;
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

/* OpenMP 4.5 */
int omp_get_num_places(void)
{
	return (numplaces(ort->place_partition));
}

/* OpenMP 4.5 */
int omp_get_place_num_procs(int place_num)
{
	return (placelen(ort->place_partition,place_num));
}

/* OpenMP 4.5 */
void omp_get_place_proc_ids(int place_num, int *ids)
{
	int i;

	if (!ids) return;
	for (i = 0; i < placelen(ort->place_partition,place_num); i++)
		ids[i] = ort->place_partition[place_num+1][i+1];
}

/* OpenMP 4.5 */
int omp_get_place_num(void)
{
	ort_eecb_t *t = __MYCB;
	return (t->currplace < 0) ? -1 : t->currplace;
}

/* OpenMP 4.5 */
int omp_get_partition_num_places(void)
{
	ort_eecb_t *t = __MYCB;
	return (t->pto - t->pfrom + 1);
}

/* OpenMP 4.5 */
void omp_get_partition_place_nums(int *place_nums)
{
	ort_eecb_t *t = __MYCB;
	int         i;

	if (!place_nums) return;
	for (i = t->pfrom; i <= t->pto; i++)
		*(place_nums++) = i;
}


/* OpenMP 5.0 */
void omp_set_affinity_format(const char *format)
{
	ort_set_affinity_format(format);
}


/* OpenMP 5.0 */
unsigned long omp_get_affinity_format(char *buffer, unsigned long size)
{
	return ort_get_affinity_format(buffer, size);
}


/* OpenMP 5.0 */
void omp_display_affinity(const char *format)
{
	ort_display_affinity(format);
}


/* OpenMP 5.0 */
unsigned long omp_capture_affinity(char *buffer, unsigned long size, const char *format)
{
	return ort_capture_affinity(buffer, size, format);
}

/* OpenMP 4.0 */
void omp_set_default_device(int devid)
{
	if (devid >= 0 && devid < ort->num_devices)
		(__CURRTASK(__MYCB)->icvs).def_device = devid;
	else
		(__CURRTASK(__MYCB)->icvs).def_device =
			ort_illegal_device("omp_set_default_device()", devid);
}

/* OpenMP 4.0 */
int omp_get_default_device(void)
{
	return (__CURRTASK(__MYCB)->icvs).def_device;
}

/* OpenMP 4.0 */
int omp_get_num_devices(void)
{
	return ort->num_devices;
}

/* OpenMP 4.0/5.0 */
int omp_get_num_teams(void)
{
	return (INITLEAGUE() ? 1 : ort->league.numteams);
}

/* OpenMP 4.0/5.0 */
int omp_get_team_num(void)
{
	return (INITLEAGUE() ? 0 : __MYCB->cgid);
}

/* OpenMP 4.0 */
static int hostdev_omp_is_initial_device(void)
{
	/* Programm is always executed at HOST device */
	return 1;
}

/* you may want to override the default behavior; mpinode module does */
int (*actual_omp_is_initial_device)(void) = hostdev_omp_is_initial_device;

/* The definition of this function must exist since the compiler produces
 * calls to it */
int omp_is_initial_device(void)
{
	return actual_omp_is_initial_device();
}

/* OpenMP 4.5 */
int  omp_get_initial_device(void)
{
	return HOSTDEV_ID;
}

/* OpenMP 4.5 */
int omp_get_max_task_priority(void)
{
	/* Only one team per parallel for now */
	return ort->icvs.max_task_prio;
}


/*
 * Lock routines
 */


void omp_init_lock(omp_lock_t *lock)
{ *lock = NULL; ort_prepare_omp_lock(lock, ORT_LOCK_NORMAL); }

void omp_init_lock_with_hint(omp_lock_t *lock, omp_lock_hint_t hint)
{ omp_init_lock(lock); }

void omp_destroy_lock(omp_lock_t *lock)
{
	ee_destroy_lock((ee_lock_t *) *lock);
#if defined(EE_TYPE_PROCESS)
	ort_shmfree(*lock);
#else
	free(*lock);
#endif
}

void omp_set_lock(omp_lock_t *lock)
{ ee_set_lock((ee_lock_t *) *lock); }

void omp_unset_lock(omp_lock_t *lock)
{ ee_unset_lock((ee_lock_t *) *lock); }

int  omp_test_lock(omp_lock_t *lock)
{ return (ee_test_lock((ee_lock_t *) *lock)); }

void omp_init_nest_lock(omp_nest_lock_t *lock)
{ *lock = NULL; ort_prepare_omp_lock(lock, ORT_LOCK_NEST); }

void omp_init_next_lock_with_hint(omp_nest_lock_t *lock, omp_lock_hint_t hint)
{ omp_init_nest_lock(lock); }

void omp_destroy_nest_lock(omp_nest_lock_t *lock)
{
	ee_destroy_lock((ee_lock_t *) *lock);
#if defined(EE_TYPE_PROCESS)
	ort_shmfree(*lock);
#else
	free(*lock);
#endif
}

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


/*
 * Timing routines
 */


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


/*
 * Device memory routines
 */


static int checkdevice(int devid, char *msg)
{
	if (devid < 0 || devid >= ort->num_devices)
	{
		ort_warning("%s request on illegal device (%d); ignoring.\n", msg, devid);
		return true;
	}
	return false;
}

/* OpenMP 4.5 */
void *omp_target_alloc(unsigned long size, int devid)
{
	if (ort->embedmode) return (NULL);
	if (size <= 0) return (NULL);
	if (checkdevice(devid, "alloc"))
		return (NULL);
	else
	{
		ort_device_t *d = ort_get_device(devid);
		void *imedaddr = d->module->dev_alloc(d->device_info, size, 0, NULL);
		/* returns the usable mediary address */
		return ( d->module->imed2umed_addr(d->device_info, imedaddr) );
	}
}

/* OpenMP 4.5 */
void omp_target_free(void *imedaddr, int devid)
{
	if (ort->embedmode) return;
	if (checkdevice(devid, "free"))
		return;
	else
	{
		ort_device_t *d = ort_get_device(devid);
		d->module->dev_free(d->device_info, imedaddr, 0);
	}
}

/* OpenMP 4.5 */
/* Assumptions:
 * a) it should fail misearably for the HOST device (since v45 specs, 3.5.5,
 *    p. 284, request the item to have been mapped through a map() clause).
 * b) we only check for the *name*; we don't consider the space pointed by a
 *    possible pointer.
 */
int omp_target_is_present(void *ptr, int devid)
{
	return ( checkdevice(devid, "is_present") ? 0 :
             (ort_checkmapped_var(ptr, ort_get_device(devid), NULL) != NULL) );
}

/**
 * Unchecked memcpy to/from host.
 * @param dst    destination internal mediary address
 * @param src    source internal mediary address
 * @param len    size in bytes
 * @param doff   destination offset
 * @param soff   source offset
 * @param dstdev destivation device
 * @param srcdev source device
 */
static
int _target_memcpy(void *dst, void *src, unsigned long len,
                   unsigned long doff, unsigned long soff,
                   ort_device_t *dstdev, ort_device_t *srcdev)
{
	if (dstdev->id != HOSTDEV_ID || srcdev->id != HOSTDEV_ID)
	{
		if (dstdev->id == HOSTDEV_ID)
		{
			if (!(srcdev->module->sharedspace))
			{
				srcdev->module->fromdev(srcdev->device_info, dst, doff, src, soff, len);
				return 0;
			}
		}
		else
			if (!(dstdev->module->sharedspace))
			{
				dstdev->module->todev(dstdev->device_info, src, soff, dst, doff, len);
				return 0;
			}
	}
	memcpy(dst + doff, src + soff, len);
	return (0);
}

/* OpenMP 4.5 */
/* We currently insist that one of the devices must be the host device */
int omp_target_memcpy(void *dst, void *src, unsigned long length,
                      unsigned long dst_off, unsigned long src_off,
                      int dst_devid, int src_devid)
{
	ort_device_t *dstdev, *srcdev;

	if (checkdevice(dst_devid, "memcpy") || checkdevice(src_devid, "memcpy"))
		return -1;
	if (dst_devid != HOSTDEV_ID && src_devid != HOSTDEV_ID)
	{
		ort_warning("[omp_target_memcpy]: only memcpy to/from host allowed\n");
		return -1;
	}
	if (dst == NULL || src == NULL)
		return -1;

	srcdev = ort_get_device(src_devid);
	dstdev = ort_get_device(dst_devid);
	return
		_target_memcpy(dstdev->module->umed2imed_addr(dstdev->device_info, dst),
		               srcdev->module->umed2imed_addr(srcdev->device_info, src),
		               length, dst_off, src_off, dstdev, srcdev);
}

/* Unchecked, recursive copy */
static
int _target_memcpy_rect_rec(void *dst, void *src, unsigned long elemsize,
                      int numdims, unsigned long *volume,
                      unsigned long *dstoffs, unsigned long *srcoffs,
                      unsigned long *dstdims, unsigned long *srcdims,
                      unsigned long dstinitoff, unsigned long srcinitoff,
                      ort_device_t *dstdev, ort_device_t *srcdev)
{
	unsigned long i, srcblocksize = elemsize, dstblocksize = elemsize;

	/* Usual 1D and 2D cases */
	if (numdims == 1)
		return ( _target_memcpy(dst, src, elemsize*volume[0],
		                        dstinitoff + elemsize*dstoffs[0],
		                        srcinitoff + elemsize*srcoffs[0],
		                        dstdev, srcdev) );
	if (numdims == 2)
	{
		/* Optimize if space is contiguous */
		if (srcdims[1] == volume[1] && dstdims[1] == volume[1])   /* Contiguous */
			return ( _target_memcpy(dst, src, elemsize*volume[0]*volume[1],
			                        dstinitoff + elemsize*dstoffs[0]*dstdims[1],
			                        srcinitoff + elemsize*srcoffs[0]*srcdims[1],
			                        dstdev, srcdev) );
		/* Copy row-by-row */
		for (i = 0; i < volume[0]; i++)
			if ( _target_memcpy(dst, src, elemsize*volume[1],
		               dstinitoff+elemsize*((i+dstoffs[0])*dstdims[1] + dstoffs[1]),
		               srcinitoff+elemsize*((i+srcoffs[0])*srcdims[1] + srcoffs[1]),
		               dstdev, srcdev) )
				return -1;
		return 0;
	}

	/* Generic case: arr[n][m][o]... = (arr[n])[B], [B] being a block mxnxox... */
	for (i = 1; i < numdims; i++)
	{
		srcblocksize *= srcdims[i];
		dstblocksize *= dstdims[i];
	}
	for (i = 0; i < volume[0]; i++)
		if (_target_memcpy_rect_rec(dst, src, elemsize, numdims-1, volume+1,
		                 dstoffs+1, srcoffs+1, dstdims+1, srcdims+1,
		                 dstinitoff+(i+dstoffs[0])*dstblocksize,
		                 srcinitoff+(i+srcoffs[0])*srcblocksize,
		                 dstdev, srcdev))
			return -1;
	return 0;
}

/* OpenMP 4.5 */
/* We currently insist that one of the devices must be the host device */
int omp_target_memcpy_rect(void *dst, void *src, unsigned long elemsize,
                      int numdims, unsigned long *volume,
                      unsigned long *dst_offs, unsigned long *src_offs,
                      unsigned long *dst_dims, unsigned long *src_dims,
                      int dst_devid, int src_devid)
{
	ort_device_t *dstdev, *srcdev;
	int          i, j;

	if (checkdevice(dst_devid, "memcpy_rect") ||
	    checkdevice(src_devid, "memcpy_rect"))
		return -1;
	if (dst_devid != HOSTDEV_ID && src_devid != HOSTDEV_ID)
	{
		ort_warning("[omp_target_memcpy_rect]: only memcpy to/from host allowed\n");
		return -1;
	}
	if (!dst && !src)   /* must return max # dimensions supported */
		return INT_MAX;
	if (!dst || !src || !volume || !dst_offs  || !src_offs || !dst_dims ||
	    !src_dims || numdims < 1)   /* sanity checks */
		return -1;

	srcdev = ort_get_device(src_devid);
	dstdev = ort_get_device(dst_devid);
	src = srcdev->module->umed2imed_addr(srcdev->device_info, src);
	dst = dstdev->module->umed2imed_addr(dstdev->device_info, dst);
	return
		_target_memcpy_rect_rec(dst, src, elemsize, numdims, volume,
	              dst_offs, src_offs, dst_dims, src_dims, 0L, 0L, dstdev, srcdev);
}

/* OpenMP 4.5 */
int omp_target_associate_ptr(void *hostptr, void *devptr,
                            unsigned long size, unsigned long devoff, int devid)
{
	if (checkdevice(devid, "associate_ptr") || hostptr == NULL || devptr == NULL)
		return -1;
	if (devid == HOSTDEV_ID)
	{
		ort_warning("[omp_target_associate_ptr]: disallowed for the host device\n");
		return -1;
	}
	return target_associate_ptr(hostptr, devptr, size, devoff, devid);
}

/* OpenMP 4.5 */
int omp_target_disassociate_ptr(void *hostptr, int devid)
{
	if (checkdevice(devid, "disassociate_ptr") || hostptr == NULL)
		return -1;
	if (devid == HOSTDEV_ID)
	{
		ort_warning("[omp_target_disassociate_ptr]: disallowed for the host device\n");
		return -1;
	}
	return target_disassociate_ptr(hostptr, devid);
}
