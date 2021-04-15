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

# MODULE: yourmodulename
#   PART: main makefile

# -----
# The name of the module should be on the following line.
# -----
export MODULE_NAME = yourmodulename

# ----- 
# The following two subdirectories should me make'd.
# -----
SUBDIRS=devpart hostpart

# -----
# Obligatory script that should exit(1) if the module cannot be make'd
# -----
EXTRA_DIST = checkit.sh

# -----
# The following should be uncommented and unmodified if you have devpart flavors
# (if you don't know what these are, just don't touch below).
# -----
#moduledir = $(pkglibdir)/devices/$(MODULE_NAME)
#module_DATA = rules.mal
