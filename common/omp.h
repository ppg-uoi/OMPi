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

#ifndef __OMP_H__
#define __OMP_H__

#ifndef _OPENMP
	#define _OPENMP 201107
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma omp declare target

/* execution environment functions */
int  omp_in_parallel(void);
int  omp_get_thread_num(void);
void omp_set_num_threads(int num_threads);
int  omp_get_num_threads(void);
int  omp_get_max_threads(void);
int  omp_get_num_procs(void);
void omp_set_dynamic(int dynamic_threads);
int  omp_get_dynamic(void);
void omp_set_nested(int nested);
int  omp_get_nested(void);

/* schedule types */
typedef enum omp_sched_t
{
	omp_sched_static = 1,
	omp_sched_dynamic = 2,
	omp_sched_guided = 3,
	omp_sched_auto = 4
} omp_sched_t;

/* proc bind values */
typedef enum omp_proc_bind_t
{
	omp_proc_bind_false = 0,
	omp_proc_bind_true = 1,
	omp_proc_bind_master = 2,
	omp_proc_bind_close = 3,
	omp_proc_bind_spread = 4
} omp_proc_bind_t;

/* lock functions */
typedef void *omp_lock_t;

void omp_init_lock(omp_lock_t *lock);
void omp_destroy_lock(omp_lock_t *lock);
void omp_set_lock(omp_lock_t *lock);
void omp_unset_lock(omp_lock_t *lock);
int  omp_test_lock(omp_lock_t *lock);

/* nestable lock fuctions */
typedef void *omp_nest_lock_t;

void omp_init_nest_lock(omp_nest_lock_t *lock);
void omp_destroy_nest_lock(omp_nest_lock_t *lock);
void omp_set_nest_lock(omp_nest_lock_t *lock);
void omp_unset_nest_lock(omp_nest_lock_t *lock);
int  omp_test_nest_lock(omp_nest_lock_t *lock);

/* timing routines */
double omp_get_wtime(void);
double omp_get_wtick(void);

/* OpenMP 3.0 */
void omp_set_schedule(omp_sched_t kind, int chunk);
void omp_get_schedule(omp_sched_t *kind, int *chunk);
int  omp_get_thread_limit(void);
void omp_set_max_active_levels(int levels);
int  omp_get_max_active_levels(void);
int  omp_get_level(void);
int  omp_get_ancestor_thread_num(int level);
int  omp_get_team_size(int level);
int  omp_get_active_level(void);

/* OpenMP 3.1 */
int  omp_in_final(void);

/* OpenMP 4.0 */
int             omp_get_cancellation(void);
omp_proc_bind_t omp_get_proc_bind(void);
int             omp_get_num_teams(void);
int             omp_get_team_num(void);
int             omp_is_initial_device(void);

#pragma omp end declare target

void omp_set_default_device(int device_num);
int  omp_get_default_device(void);
int  omp_get_num_devices(void);

#ifdef __cplusplus
}
#endif

#endif
