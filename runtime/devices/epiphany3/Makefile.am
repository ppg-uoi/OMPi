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
#   PART: main makefile

# -----
# The name of the module should be on the following line.
# -----
export MODULE_NAME = epiphany3

# ----- 
# The following two subdirectories should be make'd.
# -----
SUBDIRS=devpart devpart-blockingws-only-task-single devpart-blockingws devpart-noexpltasks-blockingws devpart-noexpltasks devpart-noopenmp devpart-only-atomic devpart-only-critical devpart-only-parallel-locks devpart-only-parallel-reduction devpart-only-parallel devpart-only-single devpart-only-static-for devpart-only-static-guided-dynamic-for devpart-only-static-ordered-for devpart-only-tasks-1icv devpart-only-tasks hostpart

# -----
# Obligatory script that should exit(1) if the module cannot be make'd
# -----
EXTRA_DIST = checkit.sh rules.mal

# -----
# The following should be uncommented and unmodified if you have devpart flavors
# (if you don't know what these are, just don't touch below).
# -----
moduledir = $(pkglibdir)/devices/$(MODULE_NAME)
module_DATA = rules.mal
