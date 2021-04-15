#!/bin/sh

#  OMPi OpenMP Compiler
#  == Copyright since 2001 the OMPi Team
#  == Dept. of Computer Science & Engineering, University of Ioannina
#
#  This file is part of OMPi.
#
#  OMPi is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  OMPi is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with OMPi; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

# MODULE: mpinode
#   PART: script to check if module can be supported
# ACTION: exits with 1 if the module cannot be make'd
#

OMPI_MPI_NODES_FILE="${HOME}/.ompi_mpi_nodes"


create_default_nodefile () {
cat >> "${1}" << __EOF__
# Put each node's hostname or IP address in a separate line.
# Anything following the '#' character (up to the end of line) is considered a comment.
# You can use a hostname more that once to create multiple processes (devices) on the same host.
# You can use 'localhost' to create a process on local host (very useful when debugging).
# You must have configured the local machine and all the hosts you enter with the same
# version of MPI and have access to them.
#
# Example:
#   localhost        # device 1 is localhost
#   195.251.100.230  # device 2
#   195.251.100.230  # device 3 will also be on the same machine
__EOF__
}


# We check if both mpicc and mpirun executables can be found.
if which mpicc &> /dev/null && which mpirun &> /dev/null; then
	# We check if mpirun uses Open MPI
	if mpirun --version | grep -q 'Open MPI'; then
		if [ ! -f "${OMPI_MPI_NODES_FILE}" ]; then
			create_default_nodefile "${OMPI_MPI_NODES_FILE}"
		fi
		exit 0
	fi
fi

exit 1

