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

#include <stdio.h>
#include <string.h>
#include "mmain.h"

parallella_globals_t pll_gl;
parallella_runtime_mem_t pll_ort;

void  *sh_mem_lock;
void  *offload_lock;
char  *modulename;

/* Pointers to lock functions of the host runtime
 */
void (*init_lock)(void **lock, int type);
void (*lock)(void **lock);
void (*unlock)(void **lock);
int  (*hyield)(void);

/**
 * Host passes the name of the module
 *
 * @param modname the name of the module
 *
 */
void  hm_set_module_name(char *modname)
{
	modulename = strdup(modname ? modname : "noname");
}

/**
 * Calculates the number of available devices supported by this module
 *
 * @return number of devices
 */
int   hm_get_num_devices(void)
{
	/* Only one Epiphany is supported now */
	return 1;
}

/**
 * Prints information for this module and the available devices
 *
 * @param device_offset the id of the first device available from this module
 */
void  hm_print_information(int device_offset)
{
	fprintf(stderr, "OMPi module for Epiphany-16 accelerator devices.\n");
	fprintf(stderr, "Available devices : 1\n\n");
	fprintf(stderr, "device id < %d > { \n", device_offset);
	fprintf(stderr, "  16 cores\n");
	fprintf(stderr, "  32 KBytes core local memory\n");
	fprintf(stderr, "}\n");
}

/**
 * Registers host runtime functions (currently it registers functions for locks)
 *
 * @param init_lock_in pointer to the function used for initializing a lock.
 *                     It's parameters are the address of a "void *" variable
 *                     and one of the "ORT_LOCK_*" defines denoting the type of
 *                     the lock
 * @param lock_in      pointer to the function used for acquiring a lock
 * @param unlock_in    pointer to the function used for releasing a lock
 * @param hyield_in    pointer to the function used for thread yield
 */
void  hm_register_ee_calls(void (*init_lock_in)(void **lock, int type),
                           void (*lock_in)(void **lock),
                           void (*unlock_in)(void **lock),
                           int  (*hyield_in)(void))
{
	init_lock = init_lock_in;
	lock      = lock_in;
	unlock    = unlock_in;
	hyield    = hyield_in;
}


static void initialize_tdspace(void)
{
	/* All shared memory but the one used for host runtime is free */
	pll_ort.free_memory_offset = PARALLELLA_TARGET_DATA_VARS_OFFSET;

	return;
}

/**
 * Initializes a device
 *
 * @param dev_num the id of the device to initialize
 *                (0 <= dev_num < hm_get_num_devices())
 * @param ort_icv Pointer to struct with
 *                initial values for the device ICVs.
 *
 * @return device_info pointer that will be used in further calls.
 *                     Return null only if it failed to initialize
 */
void *hm_initialize(int dev_num, ort_icvs_t *ort_icv)
{
	int i, j, err;

	e_init(NULL);
	e_reset_system();
	e_get_platform_info(&(pll_gl.plat));

	/* Copy icv initial values. */
	pll_gl.dev_icvs = *ort_icv;
	pll_gl.dev_icvs.place_partition =
    places_dup(ort_icv->place_partition);

	/* Init lock for shared memory allocations */
	init_lock(&sh_mem_lock, ORT_LOCK_NORMAL);
	init_lock(&offload_lock, ORT_LOCK_NORMAL);

	/* Define shared memory between HOST and EPIPHANY */
	e_alloc(&(pll_gl.mem), OMPI_SHMEM_OFFSET, OMPI_SHMEM_SIZE);

	for (i = 0; i < PARALLELLA_CORES; i++)
	{
		pll_ort.pe_exit[i] = -1;
		pll_ort.master_thread[i] = -1;
		pll_ort.kernel_args_offset[i] = 0;

		for (j = 0; j < MAX_ACTIVE_LEVELS; j++)
			pll_ort.team_threads[j][i] = -1;
	}
	/* Initialize target data mapping */
	initialize_tdspace();
	init_kernel_buffers();

	/* Open parallella board */
	for (i = 0; i < PARALLELLA_ROWS; i++)
		for (j = 0; j < PARALLELLA_COLS; j++)
		{
			err = e_open(&(pll_gl.dev[i][j]), i, j, 1, 1);
			if (err != E_OK)
			{
				printf("Error while opening PE[i][j]\n");
				exit(1);
			}
		}

	/* Ensure that PEs will get an initialized pointer and lock for shared memory */
	pll_ort.te_free_memory_offset = OMPI_TASKENV_MEM_BEGIN;

	pll_ort.sh_lock_ready = 0;

	/* Pass some ICVs to shared memory */
	pll_ort.dynamic = pll_gl.dev_icvs.dynamic;
	pll_ort.nested = pll_gl.dev_icvs.nested;
	pll_ort.levellimit = pll_gl.dev_icvs.levellimit;

	/* Write ort vars to shared memory */
	e_write(&(pll_gl.mem), 0, 0, (off_t)(PARALLELLA_ORT_MEM_OFFSET), &pll_ort,
	        sizeof(parallella_runtime_mem_t));

	/* return the pointer to epiphany specific data */
	return (void *)(&pll_gl);
}

/**
 * Finalizes a device
 *
 * @param device_info the device to finalize (ignored)
 */
void  hm_finalize(void *device_info){
	int i, j;

	/* Close workgroups */
	for (i = 0; i < PARALLELLA_ROWS; i++)
		for (j = 0; j < PARALLELLA_COLS; j++)
			e_close(&(pll_gl.dev[i][j]));

	/* Free memory */
	e_free(&(pll_gl.mem));
	e_finalize();
}

static int create_parallella_team(int master_id, int nthr,
                                  char *kernel_filename, int req_level)
{
	int core, reserved = 0, wake_up;
	unsigned pe_row, pe_col;
	int err;

	/* Lock in order to occupy a PE */
	lock(&offload_lock);

	if ((pll_ort.pe_exit[1] == -1)
	    && ((nthr & 3) ==
	        0)) /* I am the only kernel, speed up the waking up procedure */
	{
		e_epiphany_t dev_first_row, dev_rest_rows;
		int err;
		int extra_rows;

		/* Info for children, first available core is PE1 */
		for (wake_up = 0, core = 1; wake_up < nthr - 1; wake_up++, core++)
		{
			pll_ort.pe_exit[core] = 0;    /* PE is now reserved */
			pll_ort.master_thread[core] = master_id; /* Save id of master thread */
			pll_ort.master_level[core] =
			  req_level;  /* Save nesting level of master thread, PE needs this in order to */
			pll_ort.openmp_id[core] =
			  ++reserved;     /* know where to search for func and args... */
			pe_row = core / PARALLELLA_COLS;
			pe_col = core % PARALLELLA_COLS;

			/* Save this core to the table of master's children */
			pll_ort.team_members[req_level][master_id][reserved] = core;
		}

		/* Write bookkeeping of all new starting PEs to shared memory */
		e_write(&(pll_gl.mem), 0, 0,
		        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
		                                                        master_level) +  sizeof(int)),
		        &(pll_ort.master_level[1]), (nthr - 1)*sizeof(int));
		e_write(&(pll_gl.mem), 0, 0,
		        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
		                                                        pe_exit) +  sizeof(int)),
		        &(pll_ort.pe_exit[1]), (nthr - 1)*sizeof(int));
		e_write(&(pll_gl.mem), 0, 0,
		        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
		                                                        master_thread) + sizeof(int)),
		        &(pll_ort.master_thread[1]), (nthr - 1)*sizeof(int));
		e_write(&(pll_gl.mem), 0, 0,
		        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
		                                                        openmp_id) + sizeof(int)),
		        &(pll_ort.openmp_id[1]), (nthr - 1)*sizeof(int));

		/* Form 2 groups of PEs */
		err = e_open(&(dev_first_row), 0, 1, 1, PARALLELLA_COLS - 1);

		if (err != E_OK)
		{
			printf("Error while opening first group of PEs\n");
			exit(1);
		}

		extra_rows = nthr / 4 - 1;
		if (extra_rows > 0)
		{
			e_epiphany_t *devs[2];

			err = e_open(&(dev_rest_rows), 1, 0, extra_rows, PARALLELLA_COLS);
			if (err != E_OK)
			{
				printf("Error while opening second group of PEs\n");
				exit(1);
			}

			devs[0] = &dev_first_row;
			devs[1] = &dev_rest_rows;

			my_e_reset_group(devs, 2);
		}
		else
			e_reset_group(&(dev_first_row));

		err = my_e_load_group(kernel_filename, &(dev_first_row), 0, 0, 1,
		                      PARALLELLA_COLS - 1, E_TRUE);
		if (err == E_ERR)
		{
			printf("ERROR in e_load_group!\n");
			exit(1);
		}

#ifdef HOST_VERBOSE
		printf("Just started %d cores for core:%d....\n", PARALLELLA_COLS - 1,
		       master_id);
#endif

		if (extra_rows > 0)
		{
			err = my_e_load_group(kernel_filename, &(dev_rest_rows), 0, 0, extra_rows,
			                      PARALLELLA_COLS, E_TRUE);
			if (err == E_ERR)
			{
				printf("ERROR in e_load_group!\n");
				exit(1);
			}

#ifdef HOST_VERBOSE
			printf("Just started %d cores for core:%d....\n", nthr - 4, master_id);
#endif
		}

		/* Now inform master PE about its children... */
		e_write(&(pll_gl.mem), 0, 0,
		        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
		                                                        team_members) +
		                ((req_level * PARALLELLA_CORES * PARALLELLA_CORES) + (master_id *
		                    PARALLELLA_CORES))*sizeof(int)),
		        &(pll_ort.team_members[req_level][master_id]), (reserved + 1)*sizeof(int));
	}
	else /* Fallback to slow PE waking up */
	{
		e_epiphany_t *devs[PARALLELLA_CORES];

		for (wake_up = 0; wake_up < nthr - 1; wake_up++)
		{
			pe_row = pe_col = INT_MAX;

			/* Search for PEs */
			for (core = 0; core < PARALLELLA_CORES; core++)
				if (pll_ort.pe_exit[core] == -1) /* I found an unoccupied PE */
				{
					pll_ort.pe_exit[core] = 0;    /* PE is now reserved */
					pll_ort.master_thread[core] = master_id; /* Save id of master thread */
					pll_ort.master_level[core] =
					  req_level;  /* Save nesting level of master thread, PE needs this in order to */
					pll_ort.openmp_id[core] =
					  ++reserved;     /* know where to search for func and args... */
					pe_row = core / PARALLELLA_COLS;
					pe_col = core % PARALLELLA_COLS;

					/* Save this core to the table of master's children */
					pll_ort.team_members[req_level][master_id][reserved] = core;

					break;
				}

			/* If I found a free PE */
			if (pe_row != INT_MAX)
			{
				/* Write bookkeeping to shared memory */
				e_write(&(pll_gl.mem), 0, 0,
				        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
				                                                        master_level) +  core * sizeof(int)),
				        &(pll_ort.master_level[core]), sizeof(int));
				e_write(&(pll_gl.mem), 0, 0,
				        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
				                                                        pe_exit) +  core * sizeof(int)),
				        &(pll_ort.pe_exit[core]), sizeof(int));
				e_write(&(pll_gl.mem), 0, 0,
				        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
				                                                        master_thread) + core * sizeof(int)),
				        &(pll_ort.master_thread[core]), sizeof(int));
				e_write(&(pll_gl.mem), 0, 0,
				        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
				                                                        openmp_id) + core * sizeof(int)),
				        &(pll_ort.openmp_id[core]), sizeof(int));

				/* Save workgroup data */
				devs[reserved - 1] = &(pll_gl.dev[pe_row][pe_col]);
			}
		}

		/* OK, now reset and load kernel to PE */
		my_e_reset_group(devs, reserved);

		for (core = 0; core < reserved; core++)
		{
			//err = e_load(kernel_filename, devs[core], 0, 0, E_TRUE);
			err = my_e_load_group(kernel_filename, devs[core], 0, 0, 1, 1, E_TRUE);
			if (err == E_ERR)
			{
				printf("ERROR in e_load!\n");
				exit(1);
			}

#ifdef HOST_VERBOSE
			printf("Just started core [%d] for core:%d....\n", core, master_id);
#endif
		}

		/* Now inform master PE about its children... */
		e_write(&(pll_gl.mem), 0, 0,
		        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
		                                                        team_members) +
		                ((req_level * PARALLELLA_CORES * PARALLELLA_CORES) + (master_id *
		                    PARALLELLA_CORES))*sizeof(int)),
		        &(pll_ort.team_members[req_level][master_id]), (reserved + 1)*sizeof(int));
	}

	/* Job is done */
	unlock(&offload_lock);

	return reserved; /* How many PE I just started... */
}

static void parellella_free_previous_team_cores(int core, int req_level)
{
	int i;
	if (pll_ort.team_threads[req_level][core] != -1
	    && req_level < MAX_ACTIVE_LEVELS) /* If my core had opened a parallel team */
	{
		/* I must set its children ready for business */
		lock(&offload_lock);
		for (i = 0; i < pll_ort.team_threads[req_level][core]; i++)
		{
			pll_ort.pe_exit[pll_ort.team_members[req_level][core][i + 1]] = -1;
#ifdef HOST_VERBOSE
			printf("\t\tJust unoccupied child core:%d from core %d at level %d...\n",
			       pll_ort.team_members[req_level][core][i + 1], core, req_level);
#endif
		}

		/* Erase memory from previous parallel teams */
		pll_ort.team_threads[req_level][core] = -1;

		unlock(&offload_lock);
	}
}

/**
 * Offloads and executes a kernel file.
 *
 * @param device_info         the device
 * @param host_func pointer   to offload function on host address space
 * @param dev_data pointer    to a struct containing kernel variables
 * @param decl_data pointer   to a struct containing globally declared variables
 * @param kernel_filename_prefix filename of the kernel (without the suffix)
 * @param num_teams           num_teams clause from "teams" construct
 * @param thread_limit        thread_limit clause from "teams" construct
 * @param argptr              The addresses of all target data and target
 *                            declare variables (only used in OpenCL devices)
 */
void  hm_offload(void *device_info, void *(*host_func)(void *), void *dev_data,
                 void *decl_data, char *kernel_filename_prefix, int num_teams,
                 int thread_limit, va_list argptr)
{
	int core;
	unsigned pe_row = INT_MAX, pe_col = INT_MAX;
	char *obj_filename;
	int pe_request;
	int deb;
	int my_child_core;

	do
	{
		/* Lock in order to occupy a PE */
		lock(&offload_lock);

		for (core = 0; core < PARALLELLA_CORES; core++)
			if (pll_ort.pe_exit[core] == -1) /* I found an unoccupied PE */
			{
				pll_ort.pe_exit[core] = 0;    /* PE is now reserved */
				pe_row = core / PARALLELLA_COLS;
				pe_col = core % PARALLELLA_COLS;
				break;
			}
		/* PE found unlock now */
		unlock(&offload_lock);
		hyield();
	}
	while (pe_row == INT_MAX);  /* Wait until a PEs is unoccupied */

	/* A stand alone PE */
	pll_ort.master_thread[core] = -1;
	/* Store kernel arguments offset */
	pll_ort.kernel_args_offset[core]      = (uint32_t)(dev_data  -
	  pll_gl.mem.base);
	pll_ort.kernel_decl_args_offset[core] = (uint32_t)(decl_data -
	  pll_gl.mem.base);

	/* Write bookkeeping to shared memory */
	e_write(&(pll_gl.mem), 0, 0, (off_t)(PARALLELLA_ORT_MEM_OFFSET +
	  my_offsetof(parallella_runtime_mem_t, pe_exit) +
	  core * sizeof(int)), &(pll_ort.pe_exit[core]), sizeof(int));

	e_write(&(pll_gl.mem), 0, 0, (off_t)(PARALLELLA_ORT_MEM_OFFSET +
	  my_offsetof(parallella_runtime_mem_t, master_thread) +
	  core * sizeof(int)), &(pll_ort.master_thread[core]), sizeof(int));

	e_write(&(pll_gl.mem), 0, 0, (off_t)(PARALLELLA_ORT_MEM_OFFSET +
	  my_offsetof(parallella_runtime_mem_t, kernel_args_offset) +
	  core * sizeof(uint32_t)), &(pll_ort.kernel_args_offset[core]),
	  sizeof(uint32_t));

	e_write(&(pll_gl.mem), 0, 0, (off_t)(PARALLELLA_ORT_MEM_OFFSET +
	  my_offsetof(parallella_runtime_mem_t, kernel_decl_args_offset) +
	  core * sizeof(uint32_t)), &(pll_ort.kernel_decl_args_offset[core]),
	  sizeof(uint32_t));

	/* Execute kernel */
	/* first we construct the object file name */
	obj_filename = (char *)malloc((strlen(kernel_filename_prefix)  +
	                              strlen(modulename) + 7) * sizeof(char));

	sprintf(obj_filename, "%s-%s.srec", kernel_filename_prefix, modulename);

	e_reset_group(&(pll_gl.dev[pe_row][pe_col]));
	lock(&offload_lock);
	e_load(obj_filename, &(pll_gl.dev[pe_row][pe_col]), 0, 0, E_TRUE);
	unlock(&offload_lock);
	hyield();
#ifdef HOST_VERBOSE
	printf("MASTER: Just started core:%d...\n", core);
#endif
	/* Wait until PE has finished its work */
	while (1)
	{
		e_read(&(pll_gl.mem), 0, 0,
		       (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
		                                                       pe_exit) +
		               core * sizeof(int)), &(pll_ort.pe_exit[core]), sizeof(int));

		//printf("MASTER: pll_ort.pe_exit[%d]=%p\n", core, pll_ort.pe_exit[core]);

		if (pll_ort.team_threads[0][core] != -1)
		{
			int weaken_pe;
			for (weaken_pe = 1; weaken_pe <= pll_ort.team_threads[0][core]; weaken_pe++)
			{
				/* Try to find a PE that I am responsible of */
				my_child_core = pll_ort.team_members[0][core][weaken_pe];

				e_read(&(pll_gl.mem), 0, 0,
				       (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
				                                                       pe_exit) +
				               my_child_core * sizeof(int)), &(pll_ort.pe_exit[my_child_core]), sizeof(int));

				//printf("CHILD: pll_ort.pe_exit[%d]=%p\n", my_child_core, pll_ort.pe_exit[my_child_core]);

				if (pll_ort.pe_exit[my_child_core] > 100
				    && pll_ort.pe_exit[my_child_core] < 200)
				{
					pe_request = pll_ort.pe_exit[my_child_core] - 100; /* Save request */
#ifdef HOST_VERBOSE
					printf("CHILD: Just received request from CHILD core:%d for %d threads...\n",
					       my_child_core, pe_request);
#endif

					/* PE is still occupied */
					pll_ort.pe_exit[my_child_core] = 0;

					/* Wake up PEs */
					pll_ort.team_threads[1][my_child_core] = create_parallella_team(
					                                           my_child_core, pe_request, obj_filename, 1);
#ifdef HOST_VERBOSE
					printf("\tCHILD: Just started %d cores for CHILD core:%d....\n",
					       pll_ort.team_threads[1][my_child_core], my_child_core);
#endif

					/* Inform master PE how many PEs were available */
					e_write(&(pll_gl.mem), 0, 0,
					        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
					                                                        pe_exit) +
					                my_child_core * sizeof(int)), &(pll_ort.pe_exit[my_child_core]), sizeof(int));

					e_write(&(pll_gl.mem), 0, 0,
					        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
					                                                        team_threads) +
					                (PARALLELLA_CORES + my_child_core)*sizeof(int)),
					        &(pll_ort.team_threads[1][my_child_core]), sizeof(int));
				}
				else
					if (pll_ort.pe_exit[my_child_core] < -1)
					{
						if (pll_ort.pe_exit[my_child_core] == -2)
						{
							printf("Error in nested my_child_core deallocation!\n");
							exit(1);
						}

						/* Unoccupy second nested level cores */
						parellella_free_previous_team_cores(my_child_core, 1);

						/* PE is still occupied */
						pll_ort.pe_exit[my_child_core] = 0;

						/* Inform my_child_core that cores are unoccupied so it can resume working.. */
						e_write(&(pll_gl.mem), 0, 0,
						        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
						                                                        pe_exit) +
						                my_child_core * sizeof(int)), &(pll_ort.pe_exit[my_child_core]), sizeof(int));
					}
			}
		}

		if (pll_ort.pe_exit[core] == 1)
		{
			/* PE is now ready for business */
			lock(&offload_lock);
			pll_ort.pe_exit[core] = -1;
			unlock(&offload_lock);

#ifdef HOST_VERBOSE
			printf("\t\tMASTER: Just unoccupied core:%d...\n", core);
#endif

			e_write(&(pll_gl.mem), 0, 0,
			        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
			                                                        pe_exit) +
			                core * sizeof(int)), &(pll_ort.pe_exit[core]), sizeof(int));

			break;
		}
		else
			if (pll_ort.pe_exit[core] > 1
			    && pll_ort.pe_exit[core] <
			    17) /* PE is requesting for a first level parallel team */
			{
				pe_request = pll_ort.pe_exit[core]; /* Save request */
#ifdef HOST_VERBOSE
				printf("MASTER: Just received request from core:%d for %d threads...\n", core,
				       pll_ort.pe_exit[core]);
#endif

				/* PE is still occupied */
				pll_ort.pe_exit[core] = 0;

				/* Wake up PEs */
				pll_ort.team_threads[0][core] = create_parallella_team(core, pe_request,
				                                                           obj_filename, 0);
#ifdef HOST_VERBOSE
				printf("\tMASTER: Just started %d cores for core:%d....\n",
				       pll_ort.team_threads[0][core], core);
#endif

				/* Inform master PE how many PEs were available */
				e_write(&(pll_gl.mem), 0, 0,
				        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
				                                                        pe_exit) +
				                core * sizeof(int)), &(pll_ort.pe_exit[core]), sizeof(int));
				e_write(&(pll_gl.mem), 0, 0,
				        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
				                                                        team_threads) +
				                core * sizeof(int)), &(pll_ort.team_threads[0][core]), sizeof(int));
			}
			else
				if (pll_ort.pe_exit[core] > 100
				    && pll_ort.pe_exit[core] <
				    200) /* PE is requesting for a second level parallel team */
				{
					pe_request = pll_ort.pe_exit[core] - 100; /* Save request */
#ifdef HOST_VERBOSE
					printf("MASTER: Just received request from CHILD core:%d for %d threads...\n",
					       core, pe_request);
#endif

					/* PE is still occupied */
					pll_ort.pe_exit[core] = 0;

					/* Wake up PEs */
					pll_ort.team_threads[1][core] = create_parallella_team(core, pe_request,
					                                                           obj_filename, 1);
#ifdef HOST_VERBOSE
					printf("\tMASTER: Just started %d cores for CHILD core:%d....\n",
					       pll_ort.team_threads[1][core], core);
#endif

					/* Inform master PE how many PEs were available */
					e_write(&(pll_gl.mem), 0, 0,
					        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
					                                                        pe_exit) +
					                core * sizeof(int)), &(pll_ort.pe_exit[core]), sizeof(int));
					e_write(&(pll_gl.mem), 0, 0,
					        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
					                                                        team_threads) +
					                (PARALLELLA_CORES + core)*sizeof(int)), &(pll_ort.team_threads[1][core]),
					        sizeof(int));
				}
				else
					if (pll_ort.pe_exit[core] < -1) /* My core has ended a parallel region */
					{
#ifdef HOST_VERBOSE
						printf("\tMASTER: Just received request %d from CHILD core:%d to deallocate its children\n",
						       pll_ort.pe_exit[core], core);
#endif
						/* Unoccupy team cores... */
						if (pll_ort.pe_exit[core] == -2) /* Unoccupy first nested level cores */
							parellella_free_previous_team_cores(core, 0);
						else                            /* Unoccupy second nested level cores */
							parellella_free_previous_team_cores(core, 1);

						/* PE is still occupied */
						pll_ort.pe_exit[core] = 0;

						/* Inform master core that cores are unoccupied so it can resume working.. */
						e_write(&(pll_gl.mem), 0, 0,
						        (off_t)(PARALLELLA_ORT_MEM_OFFSET + my_offsetof(parallella_runtime_mem_t,
						                                                        pe_exit) +
						                core * sizeof(int)), &(pll_ort.pe_exit[core]), sizeof(int));
					}

		hyield();
	}

#ifdef MEASURE
	if (omp_get_thread_num() == 0)
	{
		e_read(&(pll_gl.mem), 0, 0, (off_t)(PARALLELLA_ORT_MEM_OFFSET), &(pll_ort),
		       sizeof(parallella_runtime_mem_t));

		uint32_t mo_time = 0;
		uint32_t mo_sleep = 0;
		for (core = 0; core < PARALLELLA_CORES; core++)
		{
			mo_time += pll_ort.time[core];
			mo_sleep += pll_ort.sleep[core];
		}
		float work_time = (float)((mo_time / PARALLELLA_CORES)) / (float)600000000.0;
		float sleep_time = (float)((mo_sleep / PARALLELLA_CORES)) / (float)600000000.0;
		printf("work  = %u cycles,\twork time  = %f sec\n", mo_time / PARALLELLA_CORES,
		       work_time);
		printf("sleep = %u cycles,\tsleep time = %f sec\n", mo_sleep / PARALLELLA_CORES,
		       sleep_time);
		printf("sleep_time/work_time = %f\n", sleep_time / work_time);
	}
#endif

	free(obj_filename);
}

/**
 * Allocates memory on the device
 *
 * @param device_info the device (ignored)
 * @param size        the number of bytes to allocate
 * @param map_memory  used in OpenCL, when set to 1 additionaly to the memory
 *                    allocation in shared virtual address space, the memory
 *                    is mapped with read/write permissions so the host cpu
 *                    can utilize it.
 * @return hostaddr   a pointer to the allocated space
 */
void *hm_dev_alloc(void *device_info, size_t size, int map_memory)
{
	void *allocated_memory;

	/* Align memory to 4bytes */
	if (size % ALIGN8 != 0) size = (size / ALIGN8 + 1) * ALIGN8;
	/* Lock shared memory in order to define space for kernel args */
	lock(&sh_mem_lock);

	if (pll_ort.free_memory_offset - size < OMPI_TASKENV_OFFSET)
		/* All shared memory but the one used for host runtime is free */
		pll_ort.free_memory_offset = PARALLELLA_TARGET_DATA_VARS_OFFSET;

	/* New data was added to shared memory */
	pll_ort.free_memory_offset -= size;

	/* The offset address of kernel vars for epiphany is later stored
		* in ort_offload_kernel and is calculated with the following formula:
		* offset = allocated_memory - pll_gl.mem.base
		*/
	allocated_memory = (void *)(pll_gl.mem.base + pll_ort.free_memory_offset);

	/* All done, unlock shared memory */
	unlock(&sh_mem_lock);

	return allocated_memory;
}

/**
 * Frees data allocated with hm_dev_alloc
 *
 * @param device_info  the device
 * @param hostaddr     pointer to the memory that will be released
 * @param unmap_memory used in OpenCL, when set to 1 prior to the memory
 *                     deallocation, the memory is unmapped.
 * TODO: Implement data deallocation
 */
void  hm_dev_free(void *device_info, void *devaddr, int unmap_memory)
{
	;
}

///*
// * This function emulates the ort_dev_gaddr
// * function of device.
// */
//void *ort_dev_gaddr(void *local_address)
//{
//	return local_address;
//}

/**
 * Transfers data from the host to a device
 *
 * @param device_info the targeted device (ignored)
 * @param hostaddr    the source memory
 * @param devaddr     the target memory
 * @param size        the size of the memory block
 */
void  hm_todev(void *device_info, void *hostaddr, void *devaddr, size_t size)
{
	/* Copy data from shared to local var */
	e_write(&(pll_gl.mem), 0, 0, (off_t)(devaddr - pll_gl.mem.base),
	       hostaddr, size);
}

/**
 * Transfers data from a device to the host
 *
 * @param device_info the source device (ignore)
 * @param hostaddr    the target memory
 * @param devaddr     the source memory
 * @param size        the size of the memory block
 */
void  hm_fromdev(void *device_info, void *hostaddr, void *devaddr,
                 size_t size)
{
	e_read(&(pll_gl.mem), 0, 0, (off_t)(devaddr - pll_gl.mem.base),
	        hostaddr, size);
}

/**
 *  Returns a pointer in the device address space
 *
 * @param device_info the device
 * @param devaddr     allocated memory from hm_dev_alloc
 *
 * @return pointer containing the address on which code running on the device
 *         can access hostaddr
 */
void *hm_get_dev_address(void *device_info, void *devaddr)
{
	return (void *)(devaddr-pll_gl.mem.base + EPIPHANY_BASE_ADDRESS);
}
