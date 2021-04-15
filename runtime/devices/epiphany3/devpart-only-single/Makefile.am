#
#    OMPi OpenMP Compiler
#    == Copyright since 2001 the OMPi Team
#    == Dept. of Computer Science & Engineering, University of Ioannina
#
#    This file is part of OMPi.
#
#    OMPi is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    OMPi is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with OMPi; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
##   This file must be processed by automake

# MODULE: epiphany3
#   PART: device-side runtime library

# -----
# Don't change the moduledir value.
# -----
moduledir = $(pkglibdir)/devices/$(MODULE_NAME)
module_LIBRARIES = libdevpart-only-single.a

# -----
# Just list your files in the libdevpart_a_SOURCES and EXTRA_DIST rules.
# Compiled files go to _SOURCES; utility, non-compiled files go to _DIST.
# -----
CC = e-gcc
AR = e-ar
RANLIB = e-ranlib
libdevpart_only_single_a_SOURCES = barrier.c barrier_init.c com.c device.c epiphany.c \
                  icvs_omp.c init_ws.c omp.c othr.c parallel.c single.c ws.c
libdevpart_only_single_a_CFLAGS  = -Wall -O3 -funroll-loops -mfp-mode=round-nearest -ffast-math -T ./ompi_internal.ldf

EXTRA_DIST = device.h device_globals.h shared_data.h ompi_internal.ldf \
    MakeKernel-only-single.$(MODULE_NAME)

# -----
# The following files should also be installed at "make install" time
# -----
module_DATA = ompi_internal.ldf MakeKernel-only-single.$(MODULE_NAME)
