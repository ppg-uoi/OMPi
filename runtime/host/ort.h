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

/* ORT.H
 * Prototypes and declarations needed for building OMPi.
 * Threading libraries should include only this header file.
 */

#ifndef __ORT_H__
#define __ORT_H__

/*#define ORT_DEBUG*/

#define ORT_LOCK_NORMAL 0
#define ORT_LOCK_NEST   1
#define ORT_LOCK_SPIN   2

#define _OMP_ACTIVE   0          /* Wait policies */
#define _OMP_PASSIVE  1

#define CANCELLED_NONE      0    /* Barrier responses */
#define CANCELLED_PARALLEL  1
#define CANCELLED_TASKGROUP 2

#define BIND_UNAVAILABLE -1      /* EE bindme responses */
#define BIND_ERROR       -2

#define OFFLOAD_DEFAULT   0      /* OpenMP 5.0 */
#define OFFLOAD_DISABLED  1
#define OFFLOAD_MANDATORY 2

#include "params.h"

#include "stddefs.h"
#include "sysdeps.h"
#include "omp.h"
#include "../rt_common.h"
#include <stdarg.h>

#define KBytes(n) ((n) << 10)
#define MBytes(n) ((n) << 20)

/* The capabilities of the eelib */
typedef struct
{
	int supports_nested;
	int supports_dynamic;
	int supports_nested_nondynamic;
	int max_levels_supported;   /* -1 if no limit, else >= 0. */
	int max_threads_supported;  /* -1 if no limit */
	int default_numthreads;     /* the default # threads for a team */
	int supports_proc_binding;
} ort_caps_t;

/* Modules */
typedef struct {
	char *name;
	void *handle;
	bool  initialized;
	bool  initialized_succesful;
	int   number_of_devices;
	int   sharedspace; /* True if the module address space is identical to host */

	/* Functions */
	void *(*initialize)(int, ort_icvs_t *, int *, char ***);
	void  (*finalize)(void *);
	void  (*offload)(void *, void *(*host_func)(void *), void *, void *,
                   char *, int, int, va_list);
	void *(*dev_alloc)(void *, size_t, int, void *);
	void  (*dev_free)(void *, void *, int);
	void  (*todev)(void *, void *, size_t, void *, size_t, size_t);
	void  (*fromdev)(void *, void *, size_t, void *, size_t, size_t);
	void *(*imed2umed_addr)(void *, void *);
	void *(*umed2imed_addr)(void *, void *);
} ort_module_t;

/* Devices */
typedef struct
{
	int            id;           /* Real (global) id */
	int            id_in_module; /* The id of the device within the module */
	ort_module_t  *module;       /* Pointer to the module structure */
	bool           initialized;  /* True if device has been initialized */
	void          *device_info;  /* The pointer returned from init */
	volatile void *lock;         /* Lock used for declare target items */
} ort_device_t;


/*
 * The following 4 functions are the core of the interface
 * between the threading library and ORT
 */
void  ort_ee_dowork(int thrid, void *parent_info);
void *ort_get_parent_othr_info(void);
void  ort_ee_cleanup(void);

/* This is only used in nest locks to know who the owener of the lock is
 */
void *ort_get_current_task();

/* This is a padded int type so as to occupy a whole cache line.
 * We use it mainly for per-thread flags so that there is no
 * contention among the threads.
 */
typedef union
{
	int  value;
	char padding[CACHE_LINE];
} aligned_int;

/* This is a padded int[] type so as to occupy a whole cache line.
 * We use it for barrier per-thread flags so that there is no contention.
 * TODO move this in barrier.c
 */
typedef union
{
	int  value[3]; // Assuming: 3 * sizeof(int) <= CACHE_LINE
	char padding[CACHE_LINE];
} aligned_3int;

/*
 * Memory allocators.
 * The _aligned versions return a cache aligned pointer. (*actual)
 * will contain the actual non-aligned memory obtained, to be used
 * later with free().
 */
extern void *ort_alloc(int size),
            *ort_calloc(int size),
            *ort_realloc(void *original, int size),
            *ort_alloc_aligned(int size, void **actual),
            *ort_calloc_aligned(int size, void **actual),
            *ort_realloc_aligned(int size, void **actual);

/*
 * Errors (force exit), warnings, etc.
 */
extern void ort_error(int exitcode, char *fmt, ...),
            ort_warning(char *fmt, ...),
            ort_debug_thread(char *fmt, ...);

/*
 * Barrier
 */

/* This is a default barrier provided and used by ORT.
 * EE libraries that want to make use of their own barriers
 * should #define AVOID_OMPI_DEFAULT_BARRIER in their
 * ee.h.
 */
typedef struct
{
	volatile aligned_3int *status; /* Pointer to aligned address. */
	aligned_3int *actual_arr_ptr; /* Pointer returned by calloc, realloc. */
	/* State (releasing or not) of DB. One state per phase. */
	volatile int db_state[2];
	int alloc_size;
	int team_size;
} ort_defbar_t;
extern void ort_default_barrier_init(ort_defbar_t **barp, int team_size);
extern int  ort_default_barrier_wait(ort_defbar_t *bar, int thrid);
extern void ort_default_barrier_destroy(ort_defbar_t **barp);

extern int ort_barrier_me(void);

#endif     /* __ORT_H__ */
