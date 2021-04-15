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
#   PART: device-side runtime library

# -----
# Don't change the moduledir value.
# -----
moduledir = $(pkglibdir)/devices/$(MODULE_NAME)
module_LIBRARIES = libdevpart.a

# -----
# Just list your files in the libdevpart_a_SOURCES and EXTRA_DIST rules.
# Compiled files go to _SOURCES; utility, non-compiled files go to _DIST.
# -----
libdevpart_a_SOURCES = devpart.c

EXTRA_DIST = MakeKernel.$(MODULE_NAME)

# -----
# The following files should also be installed at "make install" time
# -----
module_DATA = MakeKernel.$(MODULE_NAME)
