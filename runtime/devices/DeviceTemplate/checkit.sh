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

# MODULE: <modulename> here
#   PART: script to check if module can be supported
# ACTION: exits with 1 if the module cannot be make'd
#

# This file is run during the configuration of ompi. It should call "exit 1" 
# if the module can't be used for any reason (e.g. missing SDKs, compiler ...)

