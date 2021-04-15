/*
	OMPi OpenMP Compiler
	== Copyright since 2001 the OMPi Team
	== Dept. of Computer Science & Engineering, University of Ioannina

	This file is part of OMPi.

	OMPi is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	OMPi is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with OMPi; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef __MAPPER_H__
#define __MAPPER_H__

typedef void * mapmod_t;     /* Opaque descriptor */

  /* Load the rules file, parse the rules and build the rule graph */
extern mapmod_t mapper_load_module(char *modname);
  /* Given the mapper module and the kernel file, select best flavor */
extern char *mapper_select_flavor(mapmod_t mm, char *kernpath);
  /* Relinquish the given mapper module structure */
extern void mapper_free_module(mapmod_t mm);

#endif /* __MAPPER_H__ */
