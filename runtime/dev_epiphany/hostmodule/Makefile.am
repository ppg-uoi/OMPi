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
#   PART: host-interface dynamic library

# -----
# Don't change the following 3 lines
# -----
moduledir = $(pkglibdir)/devices/$(MODULE_NAME)
module_LTLIBRARIES = hostmodule.la
AM_LIBTOOLFLAGS = --silent

# -----
# Just list your files in the hostmodule_la_SOURCES and EXTRA_DIST rules.
# Compiled files go to _SOURCES; utility, non-compiled files go to _DIST.
# -----
ESDK=$(EPIPHANY_HOME)
ELIBS=$(ESDK)/tools/host/lib
EINCS=$(ESDK)/tools/host/include
hostmodule_la_SOURCES = mmain.c loader.c ../../rt_common.c
hostmodule_la_CFLAGS  = -I $(EINCS) -DESDKVERSION=`./esdkversion.sh cflags`
hostmodule_la_LDFLAGS = -no-undefined -module -avoid-version -shared -lrt -L$(ELIBS) `./esdkversion.sh ldflags`

EXTRA_DIST = mmain.h loader.h shareddata.h esdkversion.sh
