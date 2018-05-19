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

#ifndef __SHARED_DATA_H__
#define __SHARED_DATA_H__

#include <stdint.h>

/*
 * Memory utilization:
 * --> Start memory at 0x01FFFFFF
 * --> Use all 32MB of shared memory (for now...)
 * a) 4096 bytes for runtime (2628 bytes are used actually)
 * b) Rest for kernel variables
 */

#define MAX_ACTIVE_LEVELS                  1
#define PE_SHAREDDATA_SIZE                 (MAX_ACTIVE_LEVELS + 1)

#define USER_LOCKS

//#define OLD_BAR
//#define SLEEP
//#define MEASURE

#define LOCK_TASK_QUEUE

#define PARALLELLA_ORT_MEM_OFFSET          0x01FFF000
#define PARALLELLA_TARGET_DATA_VARS_OFFSET 0x01FFF000

/* Last 12MB of shared memory will be used for task environment data */
#define OMPI_TASKENV_MEM_BEGIN             0x8ec00000
#define OMPI_TASKENV_OFFSET                0x00c00000

#define EPIPHANY_BASE_ADDRESS              0x8e000000
#define PARALLELLA_ROWS                    4
#define PARALLELLA_COLS                    4

#define ALIGN8                             8
#define PARALLELLA_CORES                   16

typedef struct __attribute__((aligned(ALIGN8)))
{
	volatile int pe_exit            [PARALLELLA_CORES]; // To ensure that PE's have finished a kernel
	volatile int master_thread      [PARALLELLA_CORES]; // ID of current team's master PEpe_request
	volatile int master_level       [PARALLELLA_CORES]; // level of current team's master PEpe_request
	volatile int team_threads       [MAX_ACTIVE_LEVELS][PARALLELLA_CORES]; // PEs of current OpenMP team
	volatile int openmp_id          [PARALLELLA_CORES]; // PEs of current OpenMP team
	uint32_t kernel_args_offset     [PARALLELLA_CORES]; // Pointer to kernel arguments
	uint32_t kernel_decl_args_offset[PARALLELLA_CORES]; // Pointer to kernel declare arguments
	volatile int team_members       [MAX_ACTIVE_LEVELS][PARALLELLA_CORES][PARALLELLA_CORES]; // Which cores are in my team?
#ifdef MEASURE
	uint32_t time                   [PARALLELLA_CORES];    // To measure cycles...
	uint32_t sleep                  [PARALLELLA_CORES];   // To measure barrier sleep cycles...
#endif
	uint32_t free_memory_offset;
	uint32_t sh_lock_ready;
	uint32_t te_free_memory_offset;

	/* Initial ICVS here... */
	int         dynamic;        /* For the initial task */
	int         nested;
	int         levellimit;

} parallella_runtime_mem_t;

#endif     /* __SHARED_DATA_H__ */
