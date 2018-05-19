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

#ifndef __SYSDEPS_H__
#define __SYSDEPS_H__

#include "config.h"
#include <unistd.h>
#include <time.h>


/*
 * Processors
 */
#if defined(pentium4) || defined(__pentium4)
	#define __cpu_x86 686
#elif defined(i686) || defined(__i686) || \
	defined(pentiumpro) || defined(__pentiumpro) || \
	defined(pentium2) || defined(__pentium2) || \
	defined(pentium3) || defined(__pentium3)
	#define __cpu_x86 686
	#define __cpu_686_class p6       /* Only used for CACHE_LINE size */
#elif defined(i586) || defined(__i586) || defined(pentium) || defined(__pentium)
	#define __cpu_x86 586
#elif defined(i486) || defined(__i486)
	#define __cpu_x86 486
#elif defined(i386) || defined(__i386)
	#define __cpu_x86 386
#elif defined(__x86_64)
	#define __cpu_x86 686
#endif

#if defined(sparcv9) || defined(__sparcv9)
	#define __cpu_sparc v9
#elif defined(sparc) || defined(__sparc)
	#define __cpu_sparc v8
#endif

#if defined(mips) || defined(__mips)
	#define __cpu_mips 1
#endif

/*
 * CACHE_LINE size (for L2 cache).
 * A generally good value here is 128 bytes, so it is the default, to be
 * on the safe side.
 * However, different processors have different sizes, so below
 * we try to match their values.
 */
#if defined(__cpu_x86)
	/* Up to and including P6 cpus (Pentium Pro, Pentium II, Pentium III),
	 * the L1 & L2 line size is 32 bytes. All newer CPUs have cache lines
	 * of 64 bytes. Netbutst CPUs (Pentium 4, Pentium D, Pentium Extreme and
	 * some Xeons), in addition have a "sector" size of 128 bytes.
	 * #if __cpu_686_class == p6
	 *   #define CACHE_LINE 32
	 * #else
	 *   #define CACHE_LINE 64
	 * #endif
	 * All known Athlons have 64-byte cache lines.
	 */
	#define CACHE_LINE 64
#elif defined(__cpu_sparc)
	/* UltraSparcs (v9) use L2 cache lines of 64 bytes, up to IIIi.
	 * IIIcu has line sizes 64 - 512 bytes (!) but anyways, they have
	 * the 64 bytes as "a unit of fill & coherency".
	 * IV ahs 64 - 128 bytes
	 * IV+ and T1 came back to 64-byte cache lines.
	 *
	 * So, 64 bytes should be ok.
	 */
	#define CACHE_LINE 64
#elif defined(__cpu_mips)
	/* The specifications say that the R4000 has a variable size for
	 * its cache lines, between 4 and 32 words. MIPS 10000 (MIPS IV)
	 * has 16-32 words lines, and it is a 64-bit CPU, which means
	 * that it has lines of size 128-256 bytes.
	 * The following may need to be modified to 256(?).
	 */
	#define CACHE_LINE 128
#else
	#define CACHE_LINE 128       /* A default value */
#endif


/*
 * Platforms
 */
#if defined(sgi) || defined(__sgi)
	#define __sys_sgi 1
#endif

/*
 * Clocks for timing routines
 */
#ifdef CLOCK_SGI_CYCLE    /* Irix */
	#define SYS_CLOCK CLOCK_SGI_CYCLE
#else
	#ifdef CLOCK_HIGHRES    /* Solaris */
		#define SYS_CLOCK CLOCK_HIGHRES
	#else
		#ifdef CLOCK_REALTIME /* General solution */
			#define SYS_CLOCK CLOCK_REALTIME
		#else                 /* Last option */
			#define SYS_CLOCK 0
		#endif
	#endif
#endif

/*
 * A memory fence implementation;
 * We basically assume a gcc-compatible compiler for inline assembly.
 */
#if defined(HAVE_MEMBAR)

	#define FENCE _mb()

#else

	#if defined(__cpu_x86)
		#if defined(__SYSCOMPILER_sun)
			#define FENCE __asm("cpuid")
		#else /* Assume gcc compatible */
			#define FENCE asm volatile("cpuid" : : : "%eax", "%ebx", "%ecx", "%edx")
		#endif
	#elif defined(__cpu_sparc)
		#if defined(__SYSCOMPILER_sun)
			#if __cpu_sparc == v8
				#define FENCE __asm("stbar")
			#else
				#define FENCE __asm("membar #Sync")
			#endif
		#else
			#if __cpu_sparc == v8
				#define FENCE asm volatile("stbar")
			#else
				#define FENCE asm volatile("membar #Sync")
			#endif
		#endif
	#elif defined(__cpu_mips)
		#ifdef __sys_sgi
			/* Some SGI systems, such as the Origins & Onyxes guarantee
			 * sequential consistency so nothing needs to be done.
			 */
			#define FENCE
		#else
			#define FENCE asm volatile("sync")
		#endif
	#else
		/* We have to play safe; a POSIX-threads way of guaranteeing sequential
		 * consistency is by locking & unlocking a mutex.
		 */
		#define __FENCE_MUTEX__          /* Used in sysdeps.c */
		extern void _bad_fence(void);
		#define FENCE _bad_fence()
	#endif

#endif

/* We need this for the memory allocators; try to match the
 * widest possible integer to cover all address bits.
 */
#if SIZEOF_INT == SIZEOF_CHAR_P
	#define ptrint int
#elif SIZEOF_LONG_INT == SIZE_OF_CHAR_P
	#define ptrint long int
#elif defined(HAVE_LONG_LONG_INT)
	#define ptrint long long int
#else
	#define ptrint int     /* default */
#endif

/*
 * Other stuff
 */
#ifdef NO_SCHED_YIELD
	#define sched_yield() (0)
#else
	#include <sched.h>
#endif
extern int ort_get_num_procs(void);

#endif  /* __SYSDEPS_H__ */

