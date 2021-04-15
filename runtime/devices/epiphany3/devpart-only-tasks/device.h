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

#ifndef __ORT_DEVICE_H__
#define __ORT_DEVICE_H__

#include "shared_data.h"

#define ALIGN8                        8
#define PARALLELLA_CORES              16
#define MAXACTIVEREGIONS              1

#define TASK_QUEUE_SIZE               PARALLELLA_CORES

#define __SETMYCB(v) (myeecb = (private_eecb_t *)(v))
#define __MYCB       ((private_eecb_t *) myeecb)

/* WARNING: Use only __SETCURRTASK to assign a task!!!! */
#define __SETCURRTASK(eecb, task) ((eecb)->tasking.current_executing_task = \
        ort_e_get_global_address(eecb->core, task))
#define __CURRTASK(eecb)         ((eecb)->tasking.current_executing_task)

#define _OMP_SINGLE   0        /* Workshare types */
#define _OMP_SECTIONS 1
#define _OMP_FOR      2

#define ORT_BARRIER_DELAY            100

#define omp_lock_t       ee_lock_t

/* schedule types */
typedef struct ee_lock_s
{
	int owner;
	int initialized;
	e_mutex_t mutex;
} ee_lock_t;

/* Structure to optimize guided/dynamic schedules.
 * We compute it once and reuse it in every call to ort_get_xxx_chunk().
 * Such things are declared by the compiler (as void *), are initialized
 * during ort_entering_for() and are utilized in every call to
 * ort_get_xxx_chunk() (static schedules ignore this, though).
 */

typedef struct __attribute__((aligned(ALIGN8)))
{
#ifdef OLD_BAR
	volatile int team_members;
	volatile int in_barrier;  /* All team threads access */
	volatile int sense;       /* these variables */
	ee_lock_t  barlock;       /* through team master */
#else
	volatile e_barrier_t  barriers[PARALLELLA_CORES]; /* Each PE */
	         e_barrier_t *tgt_bars[PARALLELLA_CORES]; /* has private variables */
#endif
} ee_barrier_t;


typedef struct __attribute__((aligned(ALIGN8))) Node
{
	void           *(*func)(void *);    /* Task function */
	void           *funcarg;            /* Task function argument */
	struct Node    *parent;             /* Task's parent id */
	int             isfinal;            /* OpenMP 3.1 */

	ee_lock_t       lock;               /* Lock queue for atomic access */

	volatile int    num_children;       /* Number of task's descendants*/
	volatile int    status;             /* 0: Uninitialized, 1: free, 2: occupied */
} ort_task_node_t;

#ifdef LOCK_TASK_QUEUE
typedef struct __attribute__((aligned(ALIGN8)))
{
	ort_task_node_t *tasks[TASK_QUEUE_SIZE];
	ort_task_node_t  tpool[TASK_QUEUE_SIZE];
	ort_task_node_t *current_executing_task;
	volatile int    *pending_tasks[PARALLELLA_CORES];

	int              free_cell;
	ee_lock_t        lock;
} ort_tasking_t;
#else
typedef struct __attribute__((aligned(ALIGN8)))
{
	ort_task_node_t *current_executing_task;
} ort_tasking_t;
#endif

typedef struct __attribute__((aligned(ALIGN8))) private_eecb_s
{
	int thread_num;              // Id of a PE
	int parent_row;              // Row of master PE for use in ort_e_get_global_address
	int parent_col;              // Column of master PE for use in ort_e_get_global_address
	int parent_core;             // Master PE core
	int parent_level;            // Nested level of parent PE
	int core;
	int num_siblings;            // # threads in my team
	int num_children;
	int have_created_team;
	int activelevel;             // Parallel level
#ifdef OLD_BAR
	volatile ee_barrier_t team_barrier;
#endif
	struct private_eecb_s *parent;      // Pointer to parent thread

	ort_tasking_t tasking;

} private_eecb_t;


typedef struct __attribute__((aligned(ALIGN8)))
{
	void  *(* volatile func)(void *);      // parallel func
	volatile void  *args;                  // parallel func arguments
	volatile private_eecb_t *team_parent;  // parent of parallel team
	ort_task_node_t implicit_task;
} pe_shared_t;

int   omp_in_parallel(void);
int   omp_get_num_threads(void);
int   omp_get_thread_num(void);
int   omp_is_initial_device(void);
int   omp_get_num_procs(void);

int   ort_taskwait(int how);
int   ort_barrier_me(void);

void  init_user_locks (void);
void  omp_init_lock   (omp_lock_t **lock);
void  omp_set_lock    (omp_lock_t **lock);
void  omp_unset_lock  (omp_lock_t **lock);
int   omp_test_lock   (omp_lock_t **lock);
void  omp_destroy_lock(omp_lock_t **lock);

#ifdef OLD_BAR
int   barrier(void);
void  ort_barrier_init(ee_barrier_t *bar, int team_members);
#else
int   e_my_barrier(void);
void  ort_barrier_init(void);
#endif

void  prepare_master(int teamsize);
void  ort_execute_parallel(void *(*func)(void *), void *shared, int nthr,
                           int iscombined, int procbind_type);
void  parallella_ort_power_save(void);
void  do_work(void);
void *ort_dev_gaddr(void *local_address);

void *read_address_from_master_local_address(void *local_address);
void *read_from_master_local_address(void *local_address);
void  write_to_master_local_address(void *local_address, void *data);
void *ort_e_get_global_address(int core, const void *ptr);
void *ort_e_get_global_master_address(const void *ptr);

int   ee_init_lock(ee_lock_t *lock, int type);
int   ee_set_lock(ee_lock_t *lock);
int   ee_unset_lock(ee_lock_t *lock);
int   ee_test_lock(ee_lock_t *lock);

/* Tasking... */
void  ort_init_worker_tasking(void);
void  ort_init_master_tasking(int teamsize);
void  ort_new_task(void *(*func)(void *arg), void *arg, int final, int untied);
int   ort_pending_tasks_left(private_eecb_t *me);
void  ort_execute_tasks(private_eecb_t *me, int mode);
void  ort_task_check_throttling(private_eecb_t *me);
int   omp_in_final(void);
int   ort_task_throttling(void);
void *ort_task_immediate_start(int final);
void  ort_task_immediate_end(void *my_eecb);
void *ort_taskenv_alloc(int size, void *(*task_func)(void *));
void  ort_taskenv_free(void *ptr, void *(*task_func)(void *));

void  sh_init(void);

#endif /* __DEVICE_H__ */
