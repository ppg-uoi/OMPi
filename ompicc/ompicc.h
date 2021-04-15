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

/* OMPICC
 * A driver for the OMPi compiler.
 */

#ifndef __OMPICC_H__
#define __OMPICC_H__

#include "stddefs.h"

#define LEN  4096
#define SLEN 1024

/* Flags collected from the OMPI_CPP, OMPI_CPPFLAGS,
 * OMPI_CC, OMPI_CFLAGS and OMPI_LDFLAGS, from the ./configure info and the
 * library-specific configuration file.
 */
#ifdef PATH_MAX
	#define PATHSIZE PATH_MAX
	#define FLAGSIZE PATH_MAX
#else
	#define PATHSIZE 4096
	#define FLAGSIZE 4096
#endif

typedef struct arg_s
{
	char opt;
	char val[SLEN];
	struct arg_s *next;
} arg_t;

typedef struct
{
	arg_t *head, *tail;
} arglist_t;

extern int  keep;              /* Becomes 1/2 if -k/K parameter is specified */
extern bool verbose;
extern bool usecarstats;
extern arglist_t user_outfile; /* Output, -o XXX */
extern char COMPILER[PATHSIZE], CFLAGS[FLAGSIZE], LDFLAGS[FLAGSIZE];
#ifdef PORTABLE_BUILD
	#undef LibDir
	#undef IncludeDir
	extern char InstallPath[PATHSIZE], LibDir[PATHSIZE], IncludeDir[PATHSIZE];
#endif


extern int sysexec(char *cmd, int *exit_flag);

#endif /* __OMPICC_H__ */
