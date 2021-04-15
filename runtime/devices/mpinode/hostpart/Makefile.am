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

# MODULE: mpinode
#   PART: host-interface dynamic library

# -----
# Don't change the following 3 lines
# -----
moduledir = $(pkglibdir)/devices/$(MODULE_NAME)
module_LTLIBRARIES = hostpart.la
AM_LIBTOOLFLAGS = --silent

# -----
# Just list your files in the hostpart_la_SOURCES and EXTRA_DIST rules.
# Compiled files go to _SOURCES; utility, non-compiled files go to _DIST.
# -----
hostpart_la_SOURCES = master.c slave.c memory.c dev_manager.c
hostpart_la_LDFLAGS = -no-undefined -module -avoid-version -shared -lrt

AM_CPPFLAGS = -I$(top_srcdir)/runtime/host/ee_pthreads

EXTRA_DIST = slave.h memory.h dev_manager.h
