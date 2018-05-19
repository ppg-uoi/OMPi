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

#ifndef __MMAIN_H__
#define __MMAIN_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <stdint.h>
#include <time.h>
#include <e-hal.h>
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

#include <limits.h>
#include "shareddata.h"
#include "loader.h"
#include "../../rt_common.h"

#define OMPI_SHMEM_OFFSET   0x00000000
#define OMPI_SHMEM_SIZE     0x02000000

//#define HOST_VERBOSE

#define HOST_ADDR             0
#define DEVICE_ADDR           1

#define OMPI_WAIT_FOR_KERNEL 1000000
#define my_offsetof(st, m) ((size_t)(&((st *)0)->m))

extern char *modulename;

extern void (*init_lock)(void **lock, int type);
extern void (*lock)(void **lock);
extern void (*unlock)(void **lock);
extern int  (*hyield)(void);

typedef struct
{
	e_platform_t plat;
	e_epiphany_t dev[PARALLELLA_ROWS][PARALLELLA_COLS];
	e_mem_t      mem;
	void       **tdvar_map[PARALLELLA_CORES][2];   // Mapping of target data vars
	uint32_t     tdvar_map_size[PARALLELLA_CORES]; // Size of map
	uint32_t     used_tdvars[PARALLELLA_CORES];    // How many vars are allocated per td
	uint32_t     used_tds[PARALLELLA_CORES];       // Which td is allcoated

	ort_icvs_t   dev_icvs;                         // ICV initial values
} parallella_globals_t;

int   hm_get_num_devices(void);
void  hm_print_information(int device_offset);
void *hm_initialize(int dev_num, ort_icvs_t *ort_env);
void  hm_finalize(void *device_info);
void  hm_offload(void *device_info, void *(*host_func)(void *), void *dev_data,
                 void *decl_data, char *kernel_filename_prefix, int num_teams,
                 int thread_limit, va_list argptr);
void *hm_dev_alloc(void *device_info, size_t size, int map_memory);
void  hm_dev_free(void *device_info, void *devaddr, int unmap_memory);
void  hm_todev(void *device_info, void *hostaddr, void *devaddr, size_t size);
void  hm_fromdev(void *device_info, void *hostaddr, void *devaddr,
                 size_t size);
void *hm_get_dev_address(void *device_info, void *hostaddr);

#endif     /* __ORT_PARALLELLA_H__ */
