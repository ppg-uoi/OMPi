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

#ifndef __STDDEFS_H__
#define __STDDEFS_H__

#include "config.h"

/* Boolean */
#ifdef HAVE_STDBOOL_H
	#include <stdbool.h>
#else
	#ifndef HAVE__BOOL
		#ifdef __cplusplus
			typedef bool _Bool;
		#else
			#define _Bool signed char
		#endif
	#endif
	#define bool _Bool
	#define false 0
	#define true 1
	#define __bool_true_false_are_defined 1
#endif

/* Integer/pointer */
#ifdef HAVE_STDINT_H
	#include <stdint.h>
#else
	#ifndef HAVE_UINTPTR_T
	/* Heuristic */
		#if SIZEOF_CHAR_P == SIZEOF_INT
			typedef unsigned int uintptr_t;
		#else
			typedef unsigned long uintptr_t;
		#endif
	#endif
#endif

/* OMPi standard device IDs */
#define HOSTDEV_ID  0       /* The host device */
#define AUTODEV_ID -1       /* Alias for the default device */

/* Loop schedule types */
#define FOR_SCHED_NONE          0
#define FOR_SCHED_AUTO          0
#define FOR_SCHED_STATIC        0
#define FOR_SCHED_STATIC_CHUNK  1
#define FOR_SCHED_DYNAMIC       2
#define FOR_SCHED_GUIDED        3
#define FOR_SCHED_RUNTIME       4
#define FOR_SCHED_AFFINITY      5
#define FOR_CLAUSE2SCHED(c,ch) \
	(((c) == OC_static && !(ch)) ? FOR_SCHED_STATIC : \
	(((c) == OC_static && (ch)) ? FOR_SCHED_STATIC_CHUNK : \
	 ((c) == OC_dynamic ? FOR_SCHED_DYNAMIC : \
	 ((c) == OC_guided ? FOR_SCHED_GUIDED : \
	 ((c) == OC_runtime ? FOR_SCHED_RUNTIME : \
	 ((c) == OC_affinity ? FOR_SCHED_AFFINITY : \
	 ((c) == OC_auto ? FOR_SCHED_AUTO : FOR_SCHED_NONE)))))))
	
#endif /* __STDDEFS_H__ */
