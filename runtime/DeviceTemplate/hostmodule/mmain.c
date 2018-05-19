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

#include "../../rt_common.h"

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
void  hm_set_module_name(char *modname){}

/**
 * Calculates the number of available devices supported by this module
 *
 * @return number of devices
 */
int   hm_get_num_devices(void){}

/**
 * Prints information for this module and the available devices
 *
 * @param device_offset the id of the first device available from this module
 */
void  hm_print_information(int device_offset) {}

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
void *hm_initialize(int dev_num, ort_icvs_t *ort_icv) {}

/**
 * Finalizes a device
 *
 * @param device_info the device to finalize
 */
void  hm_finalize(void *device_info) {}

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
                 int thread_limit, va_list argptr){}

/**
 * Allocates memory on the device
 *
 * @param device_info the device
 * @param size        the number of bytes to allocate
 * @param map_memory  used in OpenCL, when set to 1 additionaly to the memory
 *                    allocation in shared virtual address space, the memory
 *                    is mapped with read/write permissions so the host cpu
 *                    can utilize it.
 * @return hostaddr a pointer to the allocated space
 */
void *hm_dev_alloc(void *device_info, size_t size, int map_memory) {}

/**
 * Frees data allocated with hm_dev_alloc
 *
 * @param device_info  the device
 * @param hostaddr     pointer to the memory that will be released
 * @param unmap_memory used in OpenCL, when set to 1 prior to the memory
 *                     deallocation, the memory is unmapped.
 */
void  hm_dev_free(void *device_info, void *devaddr, int unmap_memory) {}

/**
 * Transfers data from the host to a device
 *
 * @param device_info the device
 * @param hostaddr    the source memory
 * @param devaddr     the target memory
 * @param size        the size of the memory block
 */
void  hm_todev(void *device_info, void *hostaddr, void *devaddr, size_t size) {}

/**
 * Transfers data from a device to the host
 *
 * @param device_info the source device
 * @param hostaddr    the target memory
 * @param devaddr     the source memory
 * @param size        the size of the memory block
 */
void  hm_fromdev(void *device_info, void *hostaddr, void *devaddr,
                 size_t size) {}

/**
 *  Returns a pointer in the device address space
 *
 * @param device_info the device
 * @param devaddr     allocated memory from hm_dev_alloc
 *
 * @return pointer containing the address on which code running on the device
 *         can access hostaddr
 */
void *hm_get_dev_address(void *device_info, void *devaddr) {}
