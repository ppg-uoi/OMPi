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

/* hostdev.c -- module interface for the host "device" */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ort_prive.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * THE MODULE FUNCTIONS FOR THE HOST DEVICE                          *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


extern int avoid_optimizer;


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
static
void host_offload(void *device_info, void *(*host_func)(void *), void *dev_data,
                  void *decl_data, char *kernel_fname, int num_teams,
                  int thread_limit, va_list argptr)
{
	ort_execute_kernel_on_host(host_func, dev_data);
	avoid_optimizer++;       /* gcc seems to inline and mesh up with the args */
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
 *                    allocating new space, we should return a pointer to
 *                    their original address.
 * @return hostaddr a pointer to the allocated space
 */
static
void *host_dev_alloc(void *device_info, size_t size, int map_memory, void *hostaddr)
{
	return ( ort_alloc(size) );
}


/**
 * Frees data allocated with host_dev_alloc
 *
 * @param device_info  the device
 * @param imedaddr     pointer to the memory that will be released
 * @param unmap_memory used in OpenCL, when set to 1 prior to the memory
 *                     deallocation, the memory is unmapped.
 */
static
void host_dev_free(void *device_info, void *imedaddr, int unmap_memory)
{
	free(imedaddr);
}


/**
 * Transfers data from the host to a device
 *
 * @param device_info the device
 * @param hostaddr    the source memory
 * @param hostoffset  offset from hostaddr
 * @param imedaddr    the target memory
 * @param size        the size of the memory block
 */
static
void host_todev(void *device_info, void *hostaddr, size_t hostoffset,
                void *imedaddr, size_t devoffset, size_t size)
{
	memcpy(imedaddr + devoffset, hostaddr + hostoffset, size);
}


/**
 * Transfers data from a device to the host
 *
 * @param device_info the source device
 * @param hostaddr    the target memory
 * @param hostoffset  offset from hostaddr
 * @param imedaddr    the source memory
 * @param size        the size of the memory block
 */
static
void host_fromdev(void *device_info, void *hostaddr, size_t hostoffset,
                  void *imedaddr, size_t devoffset, size_t size)
{
	memcpy(hostaddr + hostoffset, imedaddr + devoffset, size);
}


/**
 * Given an internal mediary address, it returns a usable mediary address
 *
 * @param device_info the device
 * @param iaddr       allocated memory from host_dev_alloc
 *
 * @return usable mediary address to pass to a kernel
 */
static
void *host_imed2umed_addr(void *device_info, void *iaddr)
{
  return iaddr;
}


/**
 * Given a usable mediary address, it returns the internal mediary address
 *
 * @param device_info the device
 * @param umedaddr    allocated memory from hm_dev_alloc
 *
 * @return internal mediary address to be used by ORT
 */
static
void *host_umed2imed_addr(void *device_info, void *umedaddr)
{
  return umedaddr;
}


/* ATTENTION:
 * OMPI_HOSTTARGET_SHARE is used to sets module_host.sharedspace to true/false;
 * It is false by default and it is made available only for experimentation.
 * If true, the host device tries to emulate non-shared address space devices,
 * e.g. a mapped item is actually malloc()'ed.
 * Be sure that this will BREAK existing user code, so set it to true only
 * if you know what you are doing!
 * For one, when the host becomes a fallback device (e.g. due to an if clause)
 * it won't behave as it should ;-)
 */
ort_module_t *ort_get_host_module()
{
  ort_module_t *m = &(ort->module_host);

	/* Fields:
	m->initialize     =
	m->finalize       =
	m->sharedspace    = set to 1 by default by ort_initialize, changed by env.c
	*/
	m->offload        = host_offload;
	m->dev_alloc      = host_dev_alloc;
	m->dev_free       = host_dev_free;
	m->todev          = host_todev;
	m->fromdev        = host_fromdev;
	m->imed2umed_addr = host_imed2umed_addr;
	m->umed2imed_addr = host_umed2imed_addr;
	m->initialized    = 1;
	m->initialized_succesful = 1;

	return m;
}


int avoid_optimizer = 11;



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * DEVICE-PART FUNCTIONS FOR THE HOST DEVICE                         *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* This function emulates the device ort_dev_gaddr() call on the host. */
// FIXME: WHAT THE HECK IS THIS CALL FOR ????????????
void *ort_dev_gaddr(void *local_address)
{
	return local_address;
}


static char *hostdev_devpart_med2dev_addr(void *medaddr, unsigned long size)
{
	return medaddr;
}


/* you may want to override the default behavior; mpinode module does */
char *(*actual_devpart_med2dev_addr)(void *, u_long) = 
                                                   hostdev_devpart_med2dev_addr;


char *devpart_med2dev_addr(void *medaddr, u_long size)
{
	return actual_devpart_med2dev_addr(medaddr, size);
}
