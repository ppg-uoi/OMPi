#
#    OMPi OpenMP Compiler
#    == Copyright since 2001, the OMPi Team
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


# We could just use bin_PROGRAMS, without any hooks. However, we want
# to be able to make it under a different name.
noinst_PROGRAMS = ompicc
AM_CFLAGS = @DEBUGFLAG@ -I$(srcdir) -I$(srcdir)/mal/ -I$(top_srcdir)/common/
AM_YFLAGS = -d -v

BUILT_SOURCES = git_version.h
CLEANFILES    = git_version.h

COMMON_SRC = $(top_srcdir)/common/str.c $(top_srcdir)/common/keyval.c
MAL_SRC = mal/mal_scan.c mal/mal_graph.c mal/mal_parse.c
MAL_HDR = mal/mal_scan.h mal/mal_graph.h mal/mal.h

## CFLAGS to make ompicc. Could be CPPFLAGS. No -I. needed since it is
# added by default (except if nostdinc was specified).
ompicc_CFLAGS = @DEBUGFLAG@ -DOmpiName='"_@PACKAGE_TARNAME@"' \
            -DCPPcmd='"@CPP@"' -DCCcmd='"@CC@"' \
            -DPreprocFlags='"@CPPFLAGS@ @OPENMP_CFLAGS@"' -DCompileFlags='"@CFLAGS@"' \
            -DLinkFlags='"@LDFLAGS@ @LIBS@"' -DIncludeDir='"@includedir@/@PACKAGE_TARNAME@"' \
            -DLibDir='"@libdir@/@PACKAGE_TARNAME@"' -I$(srcdir)/mal/ -I$(top_srcdir)/common/ 
ompicc_SOURCES = $(MAL_SRC) ompicc.c kernels.c mapper.c $(COMMON_SRC) 
noinst_HEADERS = $(MAL_HDR) ompicc.h kernels.h mapper.h
#ompicc_LDFLAGS = -all-static

#This rule will run if there is no git_version.h or if the VERSION file in the
#top folder changed
git_version.h: $(top_srcdir)/VERSION
	@echo '#define GIT_VERSION "'`cat $<`'"' > git_version.h;

install-exec-hook:
	@if [ ! -d $(DESTDIR)$(bindir) ]; then \
		mkdir -p $(DESTDIR)$(bindir) ; \
	fi ; \
	cp -f ompicc $(DESTDIR)$(bindir)/@PACKAGE_TARNAME@cc

uninstall-hook:
	rm -f $(DESTDIR)$(bindir)/@PACKAGE_TARNAME@cc
