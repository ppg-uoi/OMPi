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

# MODULE: epiphany4
#   PART: script to check if module can be supported
# ACTION: exits with 1 if the module cannot be make'd
#

ESDK=${EPIPHANY_HOME}
ELIBS=${ESDK}/tools/host/lib
EINCS=${ESDK}/tools/host/include
EHDF=${EPIPHANY_HDF}
LDLIBS=${ESDK}/tools/host/lib:${LD_LIBRARY_PATH}

# Check if library directories and e-gcc are known

if [ -z "${EPIPHANY_HOME}" ] || [ -z "${ELIBS}" ] || [ -z "${EINCS}" ] || [ -z "${EHDF}" ] || [ -z "${LDLIBS}" ] || ! command -v e-gcc > /dev/null ; then
	exit 1
fi

# Compile and run check_cores.c with root privileges

# gcc ./check_cores.c -o check_cores -I ${EINCS} -L ${ELIBS} -le-hal
printf "#include <stdio.h>\\n#include <stdlib.h>\\n#include <unistd.h>\\n#include <e-hal.h>\\n\\nint main() { \\n\\te_platform_t platform;\\n \\te_epiphany_t dev;\\n\\te_mem_t emem;\\n\\te_init(NULL);\\n\\te_reset_system();\\n\\te_get_platform_info(&platform);\\n\\te_finalize();\\n \\treturn platform.rows*platform.cols;\\n}\\n" | gcc -xc - -o check_cores -I "${EINCS}" -L "${ELIBS}" -le-hal
sudo -E LD_LIBRARY_PATH="${LDLIBS}" EPIPHANY_HDF="${EHDF}" ./check_cores

# Check if the available cores are 16

exitval=$?

if [ ${exitval} -ne 16 ] ; then
	exit 1
fi

exit 0
