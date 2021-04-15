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

/* slave.c -- All slave (remote; with rank != 0) MPI processes execute
 *            these functions. Their purpose is to respond to their
 *            master's requests. Note that the master and the slaves do
 *            **NOT** run in a shared memory environment. Communication is
 *            achieved with MPI.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <mpi.h>
#include "../../../rt_common.h"
#include "slave.h"
#include "memory.h"


/* #define DEBUG */


static MPI_Comm communicator; /* the MPI communicator to talk to the master process */
static int cmd_params[4];     /* parameters host sent with each call */


static void dbg(char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	fprintf(stderr, "  DBG(slave@%d): ", getpid());
	vfprintf(stderr, format, ap);
	va_end(ap);
}


/* Jump table */
void cmdinit(), cmdget(), cmdput(), cmdalloc(), cmdfree(),
     cmdexecute(), cmdshutdown();
void (*cmdfunc[])() = {
	cmdinit, cmdget, cmdput, cmdalloc, cmdfree, cmdexecute, cmdshutdown
};


int read_command(void)
{
	MPI_Recv(cmd_params, 4, MPI_INT, 0, MPI_ANY_TAG, communicator, MPI_STATUS_IGNORE);
	return cmd_params[0]; /* Type of command to execute */
}


void wait_and_execute(MPI_Comm merged_comm)
{
	int cmd;

	communicator = merged_comm;

	/* loop waiting for commands */
	for (cmd = read_command(); cmd >= 0 && cmd <= MPI_CMD_LAST; cmd= read_command())
		(cmdfunc[cmd])();
	dbg("illegal command %d received\n", cmd);
	exit(2);
}


/* Made weak so that if found at load time they get overridden.
 * We need (weak) definitions to cover the case where the module is only 
 * loaded for queries e.g. by ompicc --devinfo; in such cases ORT is not 
 * present and those symbols would otherwise be undefined.
 */
#pragma weak actual_devpart_med2dev_addr
char *(*actual_devpart_med2dev_addr)(void *, unsigned long);
#pragma weak actual_omp_is_initial_device
int (*actual_omp_is_initial_device)(void);


char *mpinode_devpart_med2dev_addr(void *medaddr, unsigned long size)
{
	void *devaddr;
	int rank;

	devaddr = alloc_items_get((size_t)medaddr);
#ifdef DEBUG
	dbg("  devpart_med2dev_addr %p --> %p\n", medaddr, devaddr);
#endif

	return devaddr;
}


void override_devpart_med2dev_addr(void)
{
	extern char *(*actual_devpart_med2dev_addr)(void *, unsigned long);
	actual_devpart_med2dev_addr = mpinode_devpart_med2dev_addr;
}


int mpinode_omp_is_initial_device(void)
{
	return 0;
}


void override_omp_is_initial_device(void)
{
	extern int (*actual_omp_is_initial_device)(void);
	actual_omp_is_initial_device = mpinode_omp_is_initial_device;
}


/* Host performs INIT (devid) */
void cmdinit()
{
	int devid = cmd_params[1];

	alloc_items_init(1); /* Slave process cares only for itself */
	alloc_items_init_global_vars(devid, 0);
}


/* Host performs GET (medaddress, offset, size) */
void cmdget()
{
	size_t maddr = cmd_params[1];
	int offset = cmd_params[2], nbytes = cmd_params[3];
	void *devaddr; /* The actual memory space on device */

#ifdef DEBUG
	dbg(" GET cmd from %d (offset:%d, size:%d)\n", maddr, offset, nbytes);
#endif

	devaddr = alloc_items_get(maddr);
	MPI_Send(devaddr + offset, nbytes, MPI_BYTE, 0, 0, communicator);
}


/* Host performs WRITE (medaddress, offset, size) */
void cmdput()
{
	size_t maddr = cmd_params[1];
	int offset = cmd_params[2], nbytes = cmd_params[3];
	void *devaddr; /* The actual memory space on device */

#ifdef DEBUG
	dbg(" PUT cmd to %p (offset:%d, size:%d)\n", maddr, offset, nbytes);
#endif

	devaddr = alloc_items_get(maddr);
	MPI_Recv(devaddr + offset, nbytes, MPI_BYTE, 0, MPI_ANY_TAG, communicator,
			MPI_STATUS_IGNORE);
}


/* Host performs DEVALLOC (size, medaddress) */
void cmdalloc()
{
	int nbytes = cmd_params[1];
	size_t maddr = cmd_params[2];

	alloc_items_add(maddr, nbytes);
#ifdef DEBUG
	dbg(" ALLOC cmd for %d bytes --> maddr: %p\n", nbytes, maddr);
#endif
}


/* Host performs DEVFREE (medaddress) */
void cmdfree()
{
	size_t maddr = cmd_params[1];

#ifdef DEBUG
	dbg(" FREE cmd for %d at %p\n", maddr, alloc_items_get(maddr));
#endif
	alloc_items_remove(maddr);
}


/* Host performs OFFLOAD */
void cmdexecute()
{
	void *devdata; /* usuable (on host) mediary addresses of struct */
	int exec_result, kernel_id = cmd_params[1];
	size_t size = cmd_params[2], devdata_maddr = cmd_params[3];

	/* Get the kernel info */
	if (size == 0)
		devdata = NULL;
	else
	{
		devdata = alloc_items_add(devdata_maddr, size);
		MPI_Recv(devdata, size, MPI_BYTE, 0, MPI_ANY_TAG, communicator,
				MPI_STATUS_IGNORE); /* Memory copy */
#ifdef DEBUG
	dbg(" EXECUTE cmd: alloced devdata (i:%d, u:%p, size:%d)\n", devdata_maddr,
			devdata, size);
#endif
	}

	void *(*kernel_function)(void *);
	kernel_function = get_kernel_function_from_id(kernel_id);
#ifdef DEBUG
	dbg(" EXECUTE cmd: will execute kernel with id %d at address %p\n",
			kernel_id, kernel_function);
#endif
	kernel_function((void *)devdata); /* execute kernel */

	if (devdata)
		alloc_items_remove(devdata_maddr);

	exec_result = 11; /* all OK */
	MPI_Send(&exec_result, 1, MPI_INT, 0, 0, communicator); /* Notify host */
#ifdef DEBUG
	dbg(" EXECUTE cmd done for kernel with id %d\n", kernel_id);
#endif
}


/* Host performs SHUTDOWN */
void cmdshutdown()
{
#ifdef DEBUG
	dbg("<<< SHUTDOWN cmd, MPI device finalizing and exitting...\n");
#endif
	alloc_items_free_all();
	free_kerneltable();
	MPI_Comm_free(&communicator);
	MPI_Finalize();
	exit(0);
}


#ifdef DEBUG
#undef DEBUG
#endif
