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
noinst_PROGRAMS = ompi
AM_CFLAGS = @DEBUGFLAG@ -I$(srcdir) -I$(srcdir)/ast -I$(srcdir)/omp -I$(srcdir)/oxt -I$(top_srcdir)/common/
AM_YFLAGS = -d -v

BUILT_SOURCES = ort.defs git_version.h
CLEANFILES    = ort.defs git_version.h

ABS_SYNTAX_TREE_SRC = ast/ast_free.c ast/ast_print.c    ast/ast_renlabs.c \
                      ast/ast_copy.c ast/ast_xform.c    ast/ast_csource.c \
                      ast/ast_vars.c ast/ast_types.c    ast/ast_gv.c      \
                      ast/ast_show.c ast/ast_traverse.c
ABS_SYNTAX_TREE_HDR = ast/ast_free.h ast/ast_print.h    ast/ast_renlabs.h \
                      ast/ast_copy.h ast/ast_xform.h    ast/ast_csource.h \
                      ast/ast_vars.h ast/ast_types.h    ast/ast_gv.h      \
                      ast/ast_show.h ast/ast_traverse.h

TRANSFORM_SRC       = omp/x_arith.c   omp/x_clauses.c   omp/x_decltarg.c \
                      omp/x_for.c     omp/x_parallel.c  omp/x_sections.c \
                      omp/x_shglob.c  omp/x_single.c    omp/x_target.c   \
                      omp/x_task.c    omp/x_thrpriv.c   omp/x_types.c    \
                      omp/x_cars.c    omp/x_teams.c     omp/x_map.c      \
                      omp/x_arrays.c  omp/x_reduction.c
TRANSFORM_HDR       = omp/x_arith.h   omp/x_clauses.h   omp/x_decltarg.h \
                      omp/x_for.h     omp/x_parallel.h  omp/x_sections.h \
                      omp/x_shglob.h  omp/x_single.h    omp/x_target.h   \
                      omp/x_task.h    omp/x_thrpriv.h   omp/x_types.h    \
                      omp/x_cars.h    omp/x_teams.h     omp/x_map.h      \
                      omp/x_arrays.h  omp/x_reduction.h

OMPI_EXTENSIONS_SRC = oxt/ox_xform.c
OMPI_EXTENSIONS_HDR = oxt/ox_xform.h

COMMON_SRC          = $(top_srcdir)/common/str.c $(top_srcdir)/common/keyval.c

#ompi
ompi_SOURCES  = $(ABS_SYNTAX_TREE_SRC) $(TRANSFORM_SRC) $(COMMON_SRC)          \
                $(OMPI_EXTENSIONS_SRC) parser.y scanner.l ompi.c ast.c         \
                symtab.c dfa.c cfg.c outline.c builder.c callgraph.c
ompi_LDFLAGS  = -all-static

noinst_HEADERS = $(ABS_SYNTAX_TREE_HDR) $(TRANSFORM_HDR) $(OMPI_EXTENSIONS_HDR)\
                 ort.defs.head ort.defs.tail parser.h scanner.h ompi.h ast.h   \
                 symtab.h outline.h dfa.h cfg.h builder.h callgraph.h

nodist_ompi_SOURCES = ort.defs

#This rule will run if there is no git_version.h or if the VERSION file in the
#top folder changed
git_version.h: $(top_srcdir)/VERSION
	@echo '#define GIT_VERSION "'`cat $<`'"' > git_version.h;

ort.defs: $(top_srcdir)/common/ort_defs.h $(srcdir)/ort.defs.head $(srcdir)/ort.defs.tail
	@echo Generating $@...
	@cat $(srcdir)/ort.defs.head > $@
	@while IFS="" read -r line; do \
		echo '"'"$$line"'\n"' >> $@ ; \
	done < $<
	@cat $(srcdir)/ort.defs.tail >> $@

install-exec-hook:
	@if [ ! -d $(DESTDIR)$(bindir) ]; then \
		mkdir -p $(DESTDIR)$(bindir) ; \
	fi ; \
	cp -f ompi $(DESTDIR)$(bindir)/_@PACKAGE_TARNAME@

uninstall-hook:
	rm -f $(DESTDIR)$(bindir)/_@PACKAGE_TARNAME@
	rm -f $(DESTDIR)$(bindir)/@PACKAGE_TARNAME@cc
