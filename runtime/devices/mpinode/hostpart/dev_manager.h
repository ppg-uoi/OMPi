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

/* This file provides an API for easy identification of supported devices
 * of the mpinode module based on a configuration file. It also provides a
 * way to start such devices.
 */

#ifndef _DEV_MANAGER_H
#define _DEV_MANAGER_H

#include <mpi.h>

#define MPI_CONF_FILE "/.ompi_mpi_nodes" /* it actually is "~/.ompi_mpi_nodes" */


/* Initialize this module and if called with argc != NULL and argv != NULL
 * start all supported devices. NOTE that this function should be called
 * only once BY EVERY PROCESS (master or slave) and before any other
 * function in this file is called. argc and argv are pointers to main
 * function's parameters. Slave processes should call this function with
 * NULL argc and argv. Returns the merged communicator containing all
 * processes (master and slaves) if called with argc != NULL and
 * argv != NULL. Returns NULL otherwise. */
MPI_Comm dev_manager_initialize(int *argc, char ***argv);


/* Return the number of supported MPI node devices. */
int get_num_mpi_devices();


/* Return a string containing the name of the requested device. NOTE that
 * you should not modify or free the returned pointer. */
char *get_device_name(int devid);


/* Free all allocated memory. Should be called once when finalizing.
 * NOTE: No other functions of this file should be called after that. */
void dev_manager_finalize();


#endif /* _DEV_MANAGER_H */
