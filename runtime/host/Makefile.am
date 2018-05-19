#
#    OMPi OpenMP Compiler
#    == Copyright since 2001 the OMPi Team
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
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
##   This file must be processed by automake

# -----
# All directories known to us
# -----
DIST_SUBDIRS = @libpaths@

# Which ones we should be making
SUBDIRS=@ortlibpath@

# More files (needed for making the distribution)
EXTRA_DIST = ort.h ort_prive.h sysdeps.h params.h dataenv.h LibraryTemplate

# Create the default directories if needed
install-data-hook:
	@set -- @ortlib@;if [ "@defaultlib@" = "yes" ]; then \
          if [ ! -d $(DESTDIR)$(libdir)/@PACKAGE_TARNAME@/default ]; then \
             mkdir -p $(DESTDIR)$(libdir)/@PACKAGE_TARNAME@/default ; \
           fi ; \
           cp -f $(DESTDIR)$(libdir)/@PACKAGE_TARNAME@/$$1/* $(DESTDIR)$(libdir)/@PACKAGE_TARNAME@/default/ ; \
           mv -f $(DESTDIR)$(libdir)/@PACKAGE_TARNAME@/default/ortconf.$$1 \
                 $(DESTDIR)$(libdir)/@PACKAGE_TARNAME@/default/ortconf.default ; \
        fi

# Uninstall default directories

uninstall-hook:
	cd $(DESTDIR)$(libdir)/@PACKAGE_TARNAME@/; \
	rm -rf @ortlib@; \
	if [ "@defaultlib@" = "yes" ]; then rm -rf default; fi;

