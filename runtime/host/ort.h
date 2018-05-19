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

#define HOST_INDEX      0
#define HOST_ID         0
#define AUTO_DEVICE    -1
#define HOST_DENVVARS_SHARE


#include "params.h"

#include "boolean.h"
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
} ort_caps_t;

/* Modules */
typedef struct {
	char *name;
	void *handle;
	bool  initialized;
	bool  initialized_succesful;
	int   number_of_devices;

	/* Functions */
	void *(*initialize)(int, ort_icvs_t *);
	void  (*finalize)(void *);
	void  (*offload)(void *, void *(*host_func)(void *), void *, void *,
                   char *, int, int, va_list);
	void *(*dev_alloc)(void *, size_t, int);
	void  (*dev_free)(void *, void *, int);
	void  (*todev)(void *, void *, void *, size_t);
	void  (*fromdev)(void *, void *, void *, size_t);
	void *(*get_dev_address)(void *, void *);
} ort_module_t;

/* Devices */
typedef struct
{
	int                id;           /* Real id */
	int                id_in_module; /* The index of the device in the module */
	ort_module_t      *module;       /* Pointer to the module structure */
	bool               initialized;  /* True if device has been initialized */
	void              *device_info;  /* The pointer returned from init */
	volatile void     *lock;         /* Lock for used for declare vars */
} ort_device_t;


/*
 * The following 4 functions are the core of the interface
 * between the threading library and ORT
 */
void  ort_ee_dowork(int thrid, void *parent_info);
void *ort_get_parent_othr_info(void);
void *ort_allocate_eecb_from_recycle(void);
void  ort_ee_exit(void);


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
	volatile aligned_int arrived[MAX_BAR_THREADS];
	volatile aligned_int released[MAX_BAR_THREADS];
	volatile aligned_int arrived2[MAX_BAR_THREADS];
	int nthr;
} ort_defbar_t;
extern void ort_default_barrier_init(ort_defbar_t *bar, int numthreads);
extern int ort_default_barrier_wait(ort_defbar_t *bar, int thrid);
extern void ort_default_barrier_destroy(ort_defbar_t *bar);

extern int ort_barrier_me(void);
#endif     /* __ORT_H__ */
