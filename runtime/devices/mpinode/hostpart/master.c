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

/* master.c -- The master (running on host; with rank == 0) MPI process
 *             executes these functions. Their purpose is to exchange
 *             messages with the slave (remote; with rank != 0) processes.
 *             Note that the master and the slaves do **NOT** run in a
 *             shared memory environment.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <mpi.h>
#include "../../../rt_common.h"
#include "../../../host/ort_prive.h"

#include "slave.h"
#include "memory.h"
#include "dev_manager.h"

#define MPI_DUMMY_ARG -1

/* Only one thread should perform MPI calls to a single device. To achieve
 * this, we use a mutex if we are in a parallel region.
 */
#define MPI_PAR_LOCK int _cur_level = __MYCB->level; \
								  if (_cur_level != 0) pthread_mutex_lock(&mpi_lock[dev->devid])
#define MPI_PAR_UNLOCK if (_cur_level != 0) pthread_mutex_unlock(&mpi_lock[dev->devid])


int hm_sharedspace = 0;           /* No sharing space with the host */
static pthread_mutex_t *mpi_lock; /* MPI calls must be serialized per device */
static MPI_Comm communicator;     /* MPI communicator with all devices */
static int running_devices;       /* Number of running devices */
static char *devices_array;       /* Running devices array */


static
void proc_error(int exitcode, char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	fprintf(stderr, "[MPI module] ");
	vfprintf(stderr, format, ap);
	va_end(ap);

	exit(exitcode);
}


/* We use this function to send a bunch of ints with one MPI Send call and
 * improve performance.
 */
static
void send_to_dev(int devid, int arg1, int arg2, int arg3, int arg4)
{
	int array[] = {arg1, arg2, arg3, arg4};

	MPI_Send(array, 4, MPI_INT, devid, 0, communicator);
}


/* Pointers to lock functions of the host runtime
 */
void (*init_lock)(void **lock, int type);
void (*lock)(void **lock);
void (*unlock)(void **lock);
int  (*hyield)(void);

char *modulename;

typedef struct
{
	int devid; /* global device id */
} devinfo_t;


/**
 * Host passes the name of the module
 *
 * @param modname the name of the module
 *
 */
void hm_set_module_name(char *modname)
{
	modulename = strdup(modname ? modname : "noname");
}


/**
 * Returns the number of available devices supported by this module
 *
 * @return number of devices
 */
int hm_get_num_devices(void)
{
	dev_manager_initialize(NULL, NULL);
	return get_num_mpi_devices();
}


/**
 * Prints information for this module and the available devices.
 *
 * @param device_offset the id of the first device available from this module
 */
void hm_print_information(int device_offset)
{
	int num_mpi_devices, i;
	char *node_name;

	dev_manager_initialize(NULL, NULL);
	num_mpi_devices = get_num_mpi_devices();
	fprintf(stderr, "OMPi MPI node device module.\n");
	fprintf(stderr, "To configure devices for this module, edit ~/.ompi_mpi_nodes.\n");
	fprintf(stderr, "Put each node's hostname or IP address in a separate line.\n");
	fprintf(stderr, "Available devices : %d\n\n", num_mpi_devices);
	for (i = 0; i < num_mpi_devices; i++)
	{

		node_name = get_device_name(i);
		fprintf(stderr, "device id < %d > { \n", device_offset + i);
		fprintf(stderr, "  Node: %s\n", node_name);
		fprintf(stderr, "}\n");
	}
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
void hm_register_ee_calls(void (*init_lock_in)(void **lock, int type),
                          void (*lock_in)(void **lock),
                          void (*unlock_in)(void **lock),
                          int  (*hyield_in)(void))
{
	init_lock = init_lock_in;
	lock      = lock_in;
	unlock    = unlock_in;
	hyield    = hyield_in;
}


/**
 * Initializes a device
 *
 * @param devid   the id of the device to initialize
 *                (0 <= dev_num < hm_get_num_devices())
 * @param ort_icv Pointer to struct with
 *                initial values for the device ICVs.
 * @param argc    Pointer to main function's argc.
 * @param argv    Pointer to main function's argv.
 * @return        devinfo_t pointer that will be used in further calls.
 *                Returns null only if it failed to initialize.
 */
void *hm_initialize(int devid, ort_icvs_t *ort_icv, int *argc, char ***argv)
{
	static int master_initialized = 0;
	int i, mpi_devices;
	devinfo_t *info = malloc(sizeof(devinfo_t));
	if (!info)
	{
		perror("hm_initialize");
		exit(EXIT_FAILURE);
	}
	info->devid = devid + 1;

	if (!master_initialized) /* master process needs to do that once */
	{
		master_initialized = 1;
		if ((communicator = dev_manager_initialize(argc, argv)) == MPI_COMM_NULL)
		{
			fprintf(stderr, "[ORT warning]: Failed to initialize mpinode module.\n");
			return NULL;
		}
		mpi_devices = get_num_mpi_devices() + 1;
		alloc_items_init(mpi_devices);

		mpi_lock = malloc(mpi_devices * sizeof(pthread_mutex_t));
		if (!mpi_lock)
		{
			perror("hm_initialize");
			exit(EXIT_FAILURE);
		}
		for (i = 0; i < mpi_devices; ++i)
		{
			pthread_mutex_init(&mpi_lock[i], NULL);
		}

		devices_array = malloc(mpi_devices);
		if (!devices_array)
		{
			perror("hm_initialize");
			exit(EXIT_FAILURE);
		}
		memset(devices_array, 1, mpi_devices);
	}

	alloc_items_init_global_vars(info->devid, 1);
	++running_devices;

	/* device initialization is serialized; no mutex need here */
	send_to_dev(info->devid, MPI_CMD_INIT, info->devid, MPI_DUMMY_ARG, MPI_DUMMY_ARG);

	return info;
}


/**
 * Finalizes a device
 *
 * @param devinfo the device to finalize
 */
void hm_finalize(void *devinfo)
{
	devinfo_t *dev = (devinfo_t *) devinfo;
	int i;

	send_to_dev(dev->devid, MPI_CMD_SHUTDOWN, MPI_DUMMY_ARG, MPI_DUMMY_ARG,
			MPI_DUMMY_ARG);
	devices_array[dev->devid] = 0;

	if (--running_devices == 0)
	{
		for (i = 0; i < get_num_mpi_devices(); ++i)
		{
			if (devices_array[i])
			{
				send_to_dev(dev->devid, MPI_CMD_SHUTDOWN, MPI_DUMMY_ARG,
						MPI_DUMMY_ARG, MPI_DUMMY_ARG);
			}
			pthread_mutex_destroy(&mpi_lock[i]);
		}
		free(devices_array);
		alloc_items_free_all();
		dev_manager_finalize();
		MPI_Comm_free(&communicator);
		MPI_Finalize();
	}
}


/**
 * Offloads and executes a kernel file.
 *
 * @param device_info         the device
 * @param host_func pointer   to offload function on host address space
 * @param devdata pointer     to a struct containing kernel variables
 * @param decldata pointer    to a struct containing globally declared variables
 * @param kernel_filename_prefix filename of the kernel (without the suffix)
 * @param num_teams           num_teams clause from "teams" construct
 * @param thread_limit        thread_limit clause from "teams" construct
 * @param argptr              The addresses of all target data and target
 *                            declare variables (only used in OpenCL devices)
 *
 * NOTE: In MPI module, decldata variables are dealt with in dev_alloc
 * function, not here.
 *
 */
void hm_offload(void *device_info, void *(*host_func)(void *), void *devdata,
                 void *decldata, char *kernel_fname, int num_teams,
                 int thread_limit, va_list argptr)
{
	int kernel_id, devdata_maddr, exec_result;
	size_t len;
	devinfo_t *dev = (devinfo_t *) device_info;

	kernel_id = get_kernel_id_from_name(kernel_fname);

	MPI_PAR_LOCK;
	if (!devdata)
	{
		len = 0;
		send_to_dev(dev->devid, MPI_CMD_EXECUTE, kernel_id, len, MPI_DUMMY_ARG);
	}
	else
	{
		len = *((int *) (devdata - sizeof(size_t)));
		/* Send the devdata struct */
		devdata_maddr = alloc_items_register(dev->devid);
		send_to_dev(dev->devid, MPI_CMD_EXECUTE, kernel_id, len, devdata_maddr);
		MPI_Send(devdata, len, MPI_BYTE, dev->devid, 0, communicator);
	}

	/* wait till the device finishes */
	MPI_Recv(&exec_result, 1, MPI_INT, dev->devid, MPI_ANY_TAG, communicator,
			MPI_STATUS_IGNORE);
	MPI_PAR_UNLOCK;

	if (exec_result != 11)
		proc_error(11, "kernel execution failed at the end %d\n", kernel_id);

	if (devdata)
	{
		MPI_PAR_LOCK;
		alloc_items_unregister(dev->devid, devdata_maddr);
		MPI_PAR_UNLOCK;
	}
}


/**
 * Allocates memory on the device
 *
 * @param device_info the device
 * @param size        the number of bytes to allocate
 * @param map_memory  used in OpenCL, when set to 1 additionaly to the memory
 *                    allocation in shared virtual address space, the memory
 *                    is mapped with read/write permissions so the host cpu
 *                    can utilize it.
 * @param hostaddr    used in MPI to allocate #declare target link variables;
 *                    when we encounter such variables instead of
 *                    allocating new space, we should return the mediary
 *                    address of the original address.
 * @return hostaddr a pointer to the allocated space (mediary address)
 */
void *hm_dev_alloc(void *device_info, size_t size, int map_memory, void *hostaddr)
{
	size_t maddr;
	void *map_memory_addr;
	devinfo_t *dev = (devinfo_t *) device_info;

	int decltarg_is_var(void *var, int devid);

	if (map_memory)                /* Only needed for devdata struct */
	{
		map_memory_addr = malloc(size + sizeof(size_t));  /* Store the size */
		if (!map_memory_addr)
		{
			perror("hm_dev_alloc");
			exit(EXIT_FAILURE);
		}
		*((size_t *) map_memory_addr) = size;
		return ( map_memory_addr + sizeof(size_t) );    /* Will be handled @ offload time */
	}

	if (decltarg_is_var(hostaddr, dev->devid))
	{
		return (void *)alloc_items_get_global(dev->devid, hostaddr);
	}

	MPI_PAR_LOCK;
	maddr = alloc_items_register(dev->devid);
	send_to_dev(dev->devid, MPI_CMD_ALLOC, size, maddr, MPI_DUMMY_ARG);
	MPI_PAR_UNLOCK;

	return (void *) maddr;
}


/**
 * Frees data allocated with hm_dev_alloc
 *
 * @param device_info  the device
 * @param maddr        pointer to the memory that will be released
 * @param unmap_memory used in OpenCL, when set to 1 prior to the memory
 *                     deallocation, the memory is unmapped.
 */
void hm_dev_free(void *device_info, void *maddr, int unmap_memory)
{
	devinfo_t *dev = (devinfo_t *) device_info;

	if (unmap_memory)              /* Only true for devdata/decldata structs */
		return;

	MPI_PAR_LOCK;
	alloc_items_unregister(dev->devid, (size_t)maddr);
	send_to_dev(dev->devid, MPI_CMD_FREE, (size_t)maddr, MPI_DUMMY_ARG, MPI_DUMMY_ARG);
	MPI_PAR_UNLOCK;
}


/**
 * Transfers data from a device to the host
 *
 * @param device_info the source device
 * @param hostaddr    the target memory
 * @param hostoffset  offset from hostaddr
 * @param maddr       the source memory mediary address
 * @param size        the size of the memory block
 */
void hm_fromdev(void *device_info, void *hostaddr, size_t hostoffset,
                void *maddr, size_t devoffset, size_t size)
{
	devinfo_t *dev = (devinfo_t *) device_info;

	MPI_PAR_LOCK;
	send_to_dev(dev->devid, MPI_CMD_GET, (size_t)maddr, devoffset, size);
	MPI_Recv(hostaddr + hostoffset, size, MPI_BYTE, dev->devid, MPI_ANY_TAG,
			communicator, MPI_STATUS_IGNORE);
	MPI_PAR_UNLOCK;
}


/**
 * Transfers data from the host to a device
 *
 * @param device_info the device
 * @param hostaddr    the source memory
 * @param hostoffset  offset from hostaddr
 * @param maddr       the target memory mediary address
 * @param size        the size of the memory block
 */
void hm_todev(void *device_info, void *hostaddr, size_t hostoffset,
              void *maddr, size_t devoffset, size_t size)
{
	devinfo_t *dev = (devinfo_t *) device_info;

	MPI_PAR_LOCK;
	send_to_dev(dev->devid, MPI_CMD_PUT, (size_t)maddr, devoffset, size);
	MPI_Send(hostaddr + hostoffset, size, MPI_BYTE, dev->devid, 0, communicator);
	MPI_PAR_UNLOCK;
}


/**
 * Given an internal mediary address, it returns a usable mediary address
 *
 * @param device_info the device
 * @param iaddr       allocated memory from hm_dev_alloc
 *
 * @return usuable mediary address to pass to a kernel
 */
void *hm_imed2umed_addr(void *device_info, void *iaddr)
{
	return iaddr; /* internal and usable addresses are the same for us */
}


/**
 * Given a usable mediary address, it returns the internal mediary address
 *
 * @param device_info the device
 * @param umedaddr    allocated memory from hm_dev_alloc
 *
 * @return internal mediary address to be used by ORT
 */
void *hm_umed2imed_addr(void *device_info, void *umedaddr)
{
	return umedaddr; /* internal and usable addresses are the same for us */
}


#undef MPI_PAR_LOCK
#undef MPI_PAR_UNLOCK
