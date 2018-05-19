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

# -----
# Don't change this; change next 2 lines
# -----
-include Makelib.conf

# -----
# Put the name of the library on the next line.
# -----
LIBRARY_NAME = torc

# -----
# Don't change this
# -----
noinst_LIBRARIES = libort.a
ompilibdir = $(libdir)/@PACKAGE_TARNAME@/$(LIBRARY_NAME)
AM_CFLAGS = @DEBUGFLAG@  -I. -I.. -I$(top_srcdir)/common -DLibDir='"'$(pkglibdir)'"'

# -----
# Just list your files in the libort_a_SOURCES and EXTRA_DIST rules.
# -----

COMMON_SRC = $(top_srcdir)/common/str.c

libort_a_SOURCES = othr.c \
    ../ort.c ../omp.c ../tasks.c ../worksharing.c ../barrier.c ../sysdeps.c \
    ../stealing.c ../pools.c ../env.c ../target.c ../dataenv.c ../modules.c \
    ../../rt_common.c $(COMMON_SRC)

EXTRA_DIST = ortconf.$(LIBRARY_NAME) ee.h

# -----
# The files to install in the lib directory
# -----
ompilib_DATA = libort.a ortconf.$(LIBRARY_NAME)
