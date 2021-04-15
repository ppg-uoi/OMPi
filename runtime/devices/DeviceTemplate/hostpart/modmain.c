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

/* This is the host-side part of the module; it should be compiled
 * to a shared library. It is dynamically linked to the host runtime at runtime.
 */

#include "../../../rt_common.h"


int hm_sharedspace = 0; /* use 1 if module address shares space with the host */


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
void hm_set_module_name(char *modname)
{
}


/**
 * Calculates the number of available devices supported by this module
 *
 * @return number of devices
 */
int hm_get_num_devices(void)
{
	return 1;
}


/**
 * Prints information for this module and its available devices.
 * While the devices this module serves are numbered starting from 0 (local
 * device id), the global device ids are set by ORT; the devoffset parameter
 * gives the global device id of local device 0.
 *
 * @param devid_offset  the global id of the 1st device served by this module
 */
void hm_print_information(int devid_offset)
{
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
 * @param dev_num the (local) id of the device to initialize
 *                (0 <= dev_num < hm_get_num_devices())
 * @param ort_icv Pointer to struct with
 *                initial values for the device ICVs.
 * @param argc    Pointer to main function's argc.
 * @param argv    Pointer to main function's argv.
 *
 * @return        devinfo: arbitrary pointer that will be passed back in
 *                following calls (see below).
 *                Return NULL only if it failed to initialize.
 */
void *hm_initialize(int dev_num, ort_icvs_t *ort_icv, int *argc, char ***argv)
{
	return NULL;
}


/**
 * Finalizes a device
 *
 * @param devinfo the pointer returned by hm_initialize()
 */
void hm_finalize(void *devinfo)
{
}


/**
 * Offloads and executes a kernel file.
 *
 * @param devinfo             the pointer returned by hm_initialize()
 * @param host_func pointer   to offload function on host address space
 * @param dev_data pointer    to a struct containing kernel variables
 * @param decl_data pointer   to a struct containing globally declared variables
 * @param kernel_filename_prefix filename of the kernel (without the suffix)
 * @param num_teams           num_teams clause from "teams" construct
 * @param thread_limit        thread_limit clause from "teams" construct
 * @param argptr              The addresses of all target data and target
 *                            declare variables (only used in OpenCL devices)
 */
void hm_offload(void *devinfo, void *(*host_func)(void *), void *dev_data,
                 void *decl_data, char *kernel_filename_prefix, int num_teams,
                 int thread_limit, va_list argptr)
{
}


/**
 * Allocates memory "on the device"
 *
 * @param devinfo     the pointer returned by hm_initialize()
 * @param size        the number of bytes to allocate
 * @param map_memory  used in OpenCL, when set to 1 additionaly to the memory
 *                    allocation in shared virtual address space, the memory
 *                    is mapped with read/write permissions so the host cpu
 *                    can utilize it.
 * @param hostaddr    used in MPI to allocate #declare target link variables;
 *                    you can safely ignore this argument.
 * @return            pointer to the allocated space (internal mediary address)
 */
void *hm_dev_alloc(void *devinfo, size_t size, int map_memory, void *hostaddr)
{
}


/**
 * Frees data allocated with hm_dev_alloc
 *
 * @param devinfo      the pointer returned by hm_initialize()
 * @param imedaddr     pointer to the memory that will be released
 * @param unmap_memory used in OpenCL, when set to 1 prior to the memory
 *                     deallocation, the memory is unmapped.
 */
void hm_dev_free(void *devinfo, void *imedaddr, int unmap_memory)
{
}


/**
 * Transfers data from the host to a device
 *
 * @param devinfo    the pointer returned by hm_initialize()
 * @param hostaddr   the source memory
 * @param hostoffset offset from hostaddr
 * @param imedaddr   the target memory (internal mediary address)
 * @param devoffset  offset from imedaddr
 * @param size       the size of the memory block
 */
void hm_todev(void *devinfo, void *hostaddr, size_t hostoffset,
               void *imedaddr, size_t devoffset, size_t size)
{
}


/**
 * Transfers data from a device to the host
 *
 * @param devinfo    the pointer returned by hm_initialize()
 * @param hostaddr   the target memory
 * @param hostoffset offset from hostaddr
 * @param imedaddr   the source memory (internal mediary address)
 * @param devoffset  offset from imedaddr
 * @param size       the size of the memory block
 */
void hm_fromdev(void *devinfo, void *hostaddr, size_t hostoffset,
                 void *imedaddr, size_t devoffset, size_t size)
{
}


/**
 * Given an internal mediary address, it returns a usable mediary address
 *
 * @param devinfo  the pointer returned by hm_initialize()
 * @param imedaddr allocated memory from hm_dev_alloc
 *
 * @return usable mediary address to pass to a kernel
 */
void *hm_imed2umed_addr(void *devinfo, void *imedaddr)
{
	return imedaddr;
}


/**
 * Given a usable mediary address, it returns the internal mediary address
 *
 * @param device_info the device
 * @param umedaddr    allocated memory from hm_dev_alloc
 *
 * @return internal mediary address to be used by ORT
 */
void *hm_imed2umed_addr(void *device_info, void *umedaddr)
{
	return umedaddr;
}
