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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* ompi.h -- the core */

#ifndef __OMPI_H__
#define __OMPI_H__

#include "scanner.h"
#include "symtab.h"
#include "boolean.h"

extern bool   enableOpenMP;  /* If false, ignore OpenMP constructs */
extern bool   enableOmpix;   /* Enable OMPi-extensions */
extern char   *filename;     /* The file we parse */
extern char   advert[256];   /* A string added to the top of generated files */
extern symtab stab;          /* The symbol table */
extern bool   testingmode;   /* Internal tests */
extern bool   processmode;   /* If true, turn on process mode */
extern bool   threadmode;
extern bool   showdbginfo;   /* If true, some debugging info is shown */

extern bool   enableCodeDup; /* Duplicate code where appropriate for speed */

extern bool   cppLineNo;     /* Output precompiler line directives (#n) */

/* This is for taking care of main() in the parsed code;
 * OMPi generates its own main() and replaces the original one.
 */
extern char *MAIN_NEWNAME;   /* Rename the original main */
extern bool hasMainfunc;     /* true if main() function is defined in the file */
extern bool needMemcpy;      /* true if generated code includes memcpy() calls */
extern bool needLimits;      /* true if need limits.h constants (min/max) */
extern bool needFloat;       /* true if need float.h constants (min/max) */
extern int  mainfuncRettype; /* 0 = int, 1 = void */

/* These are implemented in parser.y
 */
extern int __has_target;
extern aststmt parse_file(char *fname, int *error);
extern astexpr parse_expression_string(char *format, ...);
extern aststmt parse_blocklist_string(char *format, ...);
extern aststmt parse_and_declare_blocklist_string(char *format, ...);
extern aststmt parse_transunit_string(char *format, ...);

/* Utilities
 */
extern void exit_error(int exitvalue, char *format, ...);
extern void warning(char *format, ...);

#endif

