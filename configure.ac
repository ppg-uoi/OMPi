dnl
dnl  OMPi OpenMP Compiler
dnl  == Copyright since 2001 the OMPi Team
dnl  == Dept. of Computer Science & Engineering, University of Ioannina
dnl
dnl This file is part of OMPi.
dnl
dnl OMPi is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl OMPi is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with OMPi; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
dnl
dnl This file must be processed by autoconf

dnl -----
dnl Configure.in file for OMPi
dnl -----

AC_INIT([OMPi compiler with torc extensions],[obsolete],[],[ompi])
AC_CONFIG_HEADERS([common/config.h])
AM_INIT_AUTOMAKE(-Wportability)

# -----
# Before using AC_PROG_CC, check to see if the user has no CFLAGS
# so as to bypass configure's default autoconf flags (-g -O2).
# We don't want -g to be the default.
# -----
AC_MSG_CHECKING([for user-specified CFLAGS])
if test "x${CFLAGS}set" = xset; then
  userCFLAGS=no
else
  userCFLAGS=yes
fi
AC_MSG_RESULT($userCFLAGS)

# Default CPPFLAG
AC_MSG_CHECKING([for user-specified CPPFLAGS])
if test "x${CPPFLAGS}set" = xset; then
  CPPFLAGS='-D_REENTRANT'
  AC_MSG_RESULT(no)
else
  AC_MSG_RESULT(yes)
fi


# ----------------------------------------------------------------
# Standard stuff
# ----------------------------------------------------------------
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CPP
AM_PROG_CC_C_O
AC_LANG([C])
AC_SEARCH_LIBS([strerror],[cposix])
AC_SEARCH_LIBS([dlopen], [dl dld],
               [AC_DEFINE([HAVE_DLOPEN], [1], [Define if dlopen exists.])], [])
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_YACC
AM_PROG_LEX
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
LT_INIT

# ----------------------------------------------------------------
# Figure out the system compiler
# ----------------------------------------------------------------
syscomp=""
# Intel
AC_CHECK_DECL(__INTEL_COMPILER, [ syscomp="intel" ])
if eval "test x$syscomp = x"; then
# Sun Studio
  AC_CHECK_DECL(__SUNPRO_C, [ syscomp="sun" ])
  if eval "test x$syscomp = x"; then
# MIPSpro
    AC_CHECK_DECL(_SGI_COMPILER_VERSION, [ syscomp="mipspro" ])
    if eval "test x$syscomp = x"; then
# GCC Cygwin
      AC_CHECK_DECL(__CYGWIN__, [ syscomp="cygwin" ])
      if eval "test x$syscomp = x"; then
# GCC
        AC_CHECK_DECL(__GNUC__, [ syscomp="gcc" ])
      fi
    fi
  fi
# Assume it is gcc after all
  if eval "test x$syscomp = x"; then
    echo "*** Cannot determine what the system compiler is!"
    echo "*** I am assuming it is a GNU compatible compiler."
    syscomp="gcc"
  fi
fi
echo ">> the system compiler is $syscomp"
AH_TEMPLATE(__SYSCOMPILER_intel, )
AH_TEMPLATE(__SYSCOMPILER_sun, )
AH_TEMPLATE(__SYSCOMPILER_mipspro, )
AH_TEMPLATE(__SYSCOMPILER_cygwin, )
AH_TEMPLATE(__SYSCOMPILER_gcc, )
AC_DEFINE_UNQUOTED( [__SYSCOMPILER_$syscomp ], [ 1 ],[ System compiler ])

# ----------------------------------------------------------------
# Figure out the OS
# ----------------------------------------------------------------
sysos=""
AC_CANONICAL_HOST
case "${host}" in
  *-sgi-irix6* ) sysos=irix       ;;
  *-linux*     ) sysos=linux      ;;
  *-solaris2*  ) sysos=solaris    ;;
  *-cygwin*    ) sysos=cygwin     ;;
  *            ) sysos=linux
                 echo "*** Cannot determine what the operating system is!"
                 echo "*** I am assuming it is GNU/Linux compatible."
                 ;;
esac
echo ">> the operating system is $sysos"
AH_TEMPLATE(__SYSOS_irix, )
AH_TEMPLATE(__SYSOS_linux, )
AH_TEMPLATE(__SYSOS_solaris, )
AH_TEMPLATE(__SYSOS_cygwin, )
AC_DEFINE_UNQUOTED( [ __SYSOS_$sysos ], [ 1 ], [ Host operating system ])

# -----
# We won't use $host_cpu to figure out the CPU type.
# Since the compilers predefine it, and the user may force a partiular cpu type
# with his args, we let it be discovered at sysdeps.h
# -----

# ----------------------------------------------------------------
# Now, we know the OS and the system compiler.
# If there are no user CFLAGS, we set some default ones.
# ----------------------------------------------------------------
if test "$userCFLAGS" = "no"; then
  case "${syscomp}" in
    gcc     ) CFLAGS='-O3'
              if test "${sysos}" = "irix"; then
                CPPFLAGS=${CPPFLAGS}' -D__SGI_LIBC_NAMESPACE_QUALIFIER=  '
                CFLAGS='-O3'
              fi
              ;;
    cygwin  ) CFLAGS='-O3' ;;
    intel   ) CFLAGS='-O3 -U__STRICT_ANSI__ -diag-disable 3180 -no-offload' ;;
    mipspro ) CFLAGS='-O3' ;;
    sun     ) CPPFLAGS=${CPPFLAGS}' -Dasm=__asm'
              CFLAGS='-xO4' ;;
  esac
fi
echo ">> CPPFLAGS = $CPPFLAGS"
echo ">> CFLAGS = $CFLAGS"

# ----------------------------------------------------------------
# More stuff we need
# ----------------------------------------------------------------
AC_C_INLINE
AC_TYPE_LONG_LONG_INT
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(char *)
AC_CHECK_FUNCS(pthread_setconcurrency)
AC_CHECK_FUNCS(realpath)
AC_CHECK_FUNCS(posix_memalign)
# Check if -lrt is needed: we use sched_yield() and clock_gettime()
needRT=no
AC_CHECK_FUNCS(sched_yield, [ break ],
               [ AC_CHECK_LIB(rt, sched_yield, [ needRT=yes ],
                           [ AC_DEFINE_UNQUOTED(NO_SCHED_YIELD, , not found) ])
               ])
AC_CHECK_FUNCS(clock_gettime, [ break ],
               [ AC_CHECK_LIB(rt, clock_gettime, [ needRT=yes ],
                         [ AC_DEFINE_UNQUOTED(NO_CLOCK_GETTIME, , not found) ])
               ])
AC_MSG_CHECKING([for struct timespec])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[ #include <time.h> ]],
                                   [[ int x = sizeof(struct timespec); ]])],
   AC_DEFINE_UNQUOTED(HAVE_TIMESPEC, 1, Define to 1 if wave struct timespec)
   AC_MSG_RESULT([yes]),
   AC_MSG_RESULT([no])
)
if test "$needRT" = "yes"; then
  LIBS="${LIBS} -lrt"
fi

# -----
# Check if we have thread local storage
# -----
# Avoid __declspec(thread) as some compiler versions pass it with
# a plain warning...
#AC_MSG_CHECKING([for thread local storage])
#checks="__thread __declspec(thread) bummer"
#for word in $checks; do
#  case $word in
#    bummer ) AC_MSG_RESULT([no])
#             ;;
#    * )      AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[ #include <stdlib.h> ]],
#                               [[ static ] $word [ int i; i++; ]])],
#                AC_DEFINE_UNQUOTED(TLS_KEYWORD, $word, For thread local storage)
#                AC_MSG_RESULT($word)
#                break,
#             )
#  esac
#done
AC_MSG_CHECKING([for thread local storage])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[ #include <stdlib.h> ]],
                                   [[ static __thread int i; i++; ]])],
   AC_DEFINE_UNQUOTED(TLS_KEYWORD, __thread, For thread local storage)
   AC_MSG_RESULT([yes]),
   AC_MSG_RESULT([no])
)

# -----
# Check if we have atomic operations
# Make sure that we have it on the rhs
# -----
AC_MSG_CHECKING([for atomic fetch&add])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[int i; i=__sync_fetch_and_add(&i,1); ]])],
  [AC_DEFINE_UNQUOTED(HAVE_ATOMIC_FAA,, Atomic fetch & add)
   AC_DEFINE_UNQUOTED(_faa, __sync_fetch_and_add, FAA function)
   AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)]
)
AC_MSG_CHECKING([for atomic compare&swap])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[int i,j; j=__sync_bool_compare_and_swap(&i,0,1); ]])],
  [AC_DEFINE_UNQUOTED(HAVE_ATOMIC_CAS,, Atomic boolean compare & swap)
   AC_DEFINE_UNQUOTED(_cas, __sync_bool_compare_and_swap, CAS function)
   AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)]
)
AC_MSG_CHECKING([for memory barrier])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[__sync_synchronize(); ]])],
  [AC_DEFINE_UNQUOTED(HAVE_MEMBAR,, Memory barrier)
   AC_DEFINE_UNQUOTED(_mb, __sync_synchronize, Membar function)
   AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)]
)


# -----
# Check if we have all we need for explicit thread binding
# -----
AC_MSG_CHECKING([for pthread affinity])
# AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[ #include <pthread.h> #include <sched.h>]],
AC_LINK_IFELSE([AC_LANG_PROGRAM(
[[
   #define _GNU_SOURCE
   #include <pthread.h>
   #include <sched.h>]],
[[ cpu_set_t cpuset; pthread_attr_t attr;
   if (sched_getcpu()) {
     CPU_ZERO(&cpuset);
     CPU_SET(1, &cpuset);
   }
   pthread_attr_init(&attr);
   pthread_setaffinity_np(&attr, sizeof(cpuset), (const cpu_set_t *)&cpuset); ]])],
   AC_DEFINE_UNQUOTED(CAN_BIND, 1, For pthread affinity)
   AC_MSG_RESULT([yes]),
   AC_MSG_RESULT([no])
)

# ----------------------------------------------------------------
# Give a debugging option
# ----------------------------------------------------------------
AC_ARG_ENABLE(debug,
   AS_HELP_STRING([--enable-debug],[enable debugging (default: disabled)]),
   if eval "test x$enable_debug = xyes"; then
     DEBUGFLAG="-g"
   else
     DEBUGFLAG=""
   fi,
   DEBUGFLAG="")

# ----------------------------------------------------------------
# The user chooses the execution entity library.
# A choice of xxx implies files in runtime/host/ee_xxx/ which will
# be compiled and used as the default library.
# ----------------------------------------------------------------
ortlib=""
AC_ARG_WITH(ortlib,
    AS_HELP_STRING([--with-ortlib=libname],[the host runtime libraries to build (see runtime/host/ee_*; default: pthreads). If you want to build multiple libraries surround them with ""]),
    [ ortlib=$withval ],
    [ ortlib=pthreads ])
# Check if the thread libraries exist
ortlibpath=""
for lib in $ortlib
do
  if test -f $srcdir/runtime/host/ee_$lib/ee.h && test -f $srcdir/runtime/host/ee_$lib/ortconf.$lib; then :;
  else
    AC_MSG_ERROR([>> cannot find ee.h or ortconf.${lib} in runtime/ee_${lib} (check --with-ortlib)])
  fi
  ortlibpath="$ortlibpath ee_$lib"
done
AC_MSG_NOTICE([building with "${ortlib}" host libraries])

# -----
# Give the user the option of not making this the default library
# -----
AC_ARG_ENABLE(defaultlib,
   AS_HELP_STRING([--disable-defaultlib],[don't make this the default library]),
   ,
   defaultlib=yes)

# ----------------------------------------------------------------
# Bits and pieces
# ----------------------------------------------------------------
# -----
# Check for spin locks
# -----
# Usually they are available with the _XOPEN_SOURCE macro
#AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[ #define _XOPEN_SOURCE 600
#                                      #include <pthread.h>
#                                      pthread_spinlock_t lock; ]],
#                                   [[ pthread_spin_init(&lock,0); ]])],
#                  AC_DEFINE([HAVE_SPINLOCKS],[],[Through _XOPEN_SOURCE]), )

# -----
# Discover the available libraries
# -----
libpaths=""
for lib in $srcdir/runtime/host/ee_*/ #Find all library directories
do
  if test -f ${lib}Makefile.am; then  #Make sure there is a Makefile.am
    tmp="${lib#$srcdir/runtime/host/}" #Remove "$srcdir/runtime/host/" from the path
    libpaths="$libpaths ${tmp%/}"     #Add it to "libpaths"
  fi
done

# ----------------------------------------------------------------
# Modules
# ----------------------------------------------------------------
# -----
# Discover the available modules
# -----
modpaths=""
allmods=""
for mod in $srcdir/runtime/dev_*/ #Find all module directories
do
  #Make sure there is a Makefile.am and a checkit.sh
  if test -f ${mod}Makefile.am && test -f ${mod}/checkit.sh; then
    tmp="${mod#$srcdir/runtime/}"   #Remove "$srcdir/runtime/" from the path
    tmp="${tmp%/}"                  #Remove '/' from the end
    modpaths="$modpaths ${tmp}"     #Add it to "modpaths"
    allmods="$allmods ${tmp#dev_}"  #Add only the name of the module to "allmods" (sans "dev_")
  fi
done

# -----
# The user chooses which modules to install. If no modules are selected all
# the available modules are compiled and installed.
# -----
modules=""
AC_ARG_WITH(modules,
    AS_HELP_STRING([--with-modules="module1 module2"],[the device libraries to build (see runtime/dev_*; default: all available).]),
    [ modules=$withval ],
    [ modules=$allmods ])
# Check if the modules exist, and we can compile them
availmods=""
availmodpaths=""
for mod in $modules
do
  if test -f $srcdir/runtime/dev_$mod/checkit.sh && $srcdir/runtime/dev_$mod/checkit.sh; then
    availmods="$availmods $mod"
    availmodpaths="$availmodpaths dev_$mod"
  else
    AC_MSG_WARN([>> cannot build "${mod}" module (check --with-modules)])
  fi
done
availmods=${availmods:1} #Get rid of the space infront of the first module
AC_MSG_NOTICE([building with "${availmods}" device libraries])
AC_DEFINE_UNQUOTED([MODULES_CONFIG], ["${availmods}"], [The installed device modules.])

# ----------------------------------------------------------------
# The following variables should be substituted in Makefile.in
# ----------------------------------------------------------------
AC_SUBST(DEBUGFLAG)
AC_SUBST(ortlib)
AC_SUBST(ortlibpath)
AC_SUBST(defaultlib)
AC_SUBST(libpaths)
AC_SUBST(modpaths)
AC_SUBST(availmodpaths)

# -----
# Finally, output the makefiles
# If you created a new library you should add it here
# -----
AC_CONFIG_FILES([Makefile
          common/Makefile
          compiler/Makefile
          runtime/Makefile
          runtime/host/Makefile
          runtime/host/ee_pthreads/Makefile
          runtime/host/ee_psthr/Makefile
          runtime/host/ee_torc/Makefile
          runtime/host/ee_process/Makefile
          runtime/dev_epiphany/Makefile
          runtime/dev_epiphany/devrt/Makefile
          runtime/dev_epiphany/hostmodule/Makefile])
AC_OUTPUT

