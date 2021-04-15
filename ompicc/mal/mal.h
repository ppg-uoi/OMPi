/*
	OMPi OpenMP Compiler
	== Copyright since 2001, the OMPi Team
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

#ifndef __MAL_H__
#define __MAL_H__

#include <stdio.h>

typedef void * malgr_t;     /* Opaque */
typedef int (*mg_queryfunc_t)(void *userarg, char *metric, ...);

extern int   mal_parse(FILE *mal_rules_file, malgr_t *g);
extern char *mal_graph_traverse(malgr_t g, mg_queryfunc_t *qfuncs, void *userarg);
extern void  mal_graph_free(malgr_t g);

#endif
