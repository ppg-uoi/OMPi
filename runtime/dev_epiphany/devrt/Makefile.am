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
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
##   This file must be processed by automake

# MODULE: epiphany 
#   PART: device-side runtime library

# -----
# Don't change the moduledir value.
# -----
moduledir = $(pkglibdir)/devices/$(MODULE_NAME)
module_LIBRARIES = libdevrt.a

# -----
# Just list your files in the libdevrt_a_SOURCES and EXTRA_DIST rules.
# Compiled files go to _SOURCES; utility, non-compiled files go to _DIST.
# -----
CC = e-gcc
AR = e-ar
RANLIB = e-ranlib
libdevrt_a_SOURCES = atomic.c barrier.c barrier_init.c chunk_dynamic.c \
    chunk_guided.c chunk_static.c com.c critical.c device.c epiphany.c \
    executetasks.c for.c icvs_omp.c init_ws.c locks.c newtask.c omp.c othr.c \
    ordered.c parallel.c reduction.c sections.c single.c tasks.c taskwait.c ws.c
libdevrt_a_CFLAGS  = -Wall -O3 -funroll-loops -mfp-mode=round-nearest -ffast-math -T ./ompi_internal.ldf

EXTRA_DIST = device.h device_globals.h shared_data.h ompi_internal.ldf \
    MakeKernel.$(MODULE_NAME)

# -----
# The following files should also be installed at "make install" time
# -----
module_DATA = ompi_internal.ldf MakeKernel.$(MODULE_NAME)
