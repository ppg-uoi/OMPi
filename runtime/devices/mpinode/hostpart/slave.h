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

/* slave.h -- All slave (remote; with rank != 0) MPI processes execute
 *            these functions. Their purpose is to respond to their
 *            master's requests. Note that the master and the slaves do
 *            **NOT** run in a shared memory environment. Communication is
 *            achieved with MPI.
 */

#ifndef __SLAVE_H__
#define __SLAVE_H__

#include <mpi.h>

/* Commands (1 byte each) the device can understand */
#define MPI_CMD_INIT     0
#define MPI_CMD_GET      1
#define MPI_CMD_PUT      2
#define MPI_CMD_ALLOC    3
#define MPI_CMD_FREE     4
#define MPI_CMD_EXECUTE  5
#define MPI_CMD_SHUTDOWN 6
#define MPI_CMD_LAST     6

/**
 * Start the device and wait for commands to execute.
 *
 * @param communicator the merged communicator containing all processes
 *                     (master and slaves). Process with rank == 0 is the
 *                     master.
 */
void wait_and_execute(MPI_Comm communicator);


/**
 * Make sure we use our implementation of devpart_med2dev_addr function.
 * Should be called once during startup.
 */
void override_devpart_med2dev_addr(void);

/**
 * Make sure we use our implementation of devpart_med2dev_addr function.
 * Should be called once during startup.
 */
void override_omp_is_initial_device(void);

#endif  /* __SLAVE_H__ */
