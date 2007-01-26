#
# awklib/Makefile.am --- automake input file for gawk
#
# Copyright (C) 1995-2006 the Free Software Foundation, Inc.
#
# This file is part of GAWK, the GNU implementation of the
# AWK Programming Language.
#
# GAWK is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# GAWK is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
#

## process this file with automake to produce Makefile.in

# With some locales, the script extract.awk fails.
# So we fix the locale to some sensible value.
AWKPROG = LC_ALL=C LANG=C ../gawk$(EXEEXT)

EXTRA_DIST = \
	*ChangeLog \
	extract.awk \
	xml/*.awk \
	xml/*.sh \
	xml/*ChangeLog

# Get config.h from the build directory and custom.h from the source directory.
# It'll be in the build directory, not the source directory.
AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)
  
pkglibexec_PROGRAMS = pwcat grcat

if BUILD_XML
bin_SCRIPTS = xmlgawk
else
# We dont need igawk anymore, but the official distribution still has it.
#bin_SCRIPTS = igawk
bin_SCRIPTS = 
endif

AUXAWK = passwd.awk group.awk
nodist_grcat_SOURCES = grcat.c
nodist_pwcat_SOURCES = pwcat.c

all: stamp-eg $(AUXPROGS) $(bin_SCRIPTS) $(AUXAWK)

install-exec-hook: $(AUXAWK)
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	for i in $(AUXAWK) eg/lib/*.awk $(srcdir)/xml/*.awk ; do \
		progname=`echo $$i | sed 's;.*/;;'` ; \
		$(INSTALL_DATA) $$i $(DESTDIR)$(pkgdatadir)/$$progname ; \
	done

# pkglibexecdir and pkgdatadir are removed in the top level Makefile's uninstall
uninstall-local:
	rm -fr $(DESTDIR)$(pkglibexecdir)/* $(DESTDIR)$(pkgdatadir)/*
	rm -f $(DESTDIR)$(bindir)/xmlgawk

clean-local:
	rm -f $(AUXAWK) xmlgawk *.exe

stamp-eg: $(srcdir)/../doc/gawk.texi $(srcdir)/../doc/gawkinet.texi
	rm -fr eg stamp-eg
	$(AWKPROG) -f $(srcdir)/extract.awk $(srcdir)/../doc/gawk.texi $(srcdir)/../doc/gawkinet.texi
	@echo 'some makes are stupid and will not check a directory' > stamp-eg
	@echo 'against a file, so this file is a place holder. gack.' >> stamp-eg

pwcat$(EXEEXT): stamp-eg
	$(COMPILE) `dirname $^`/eg/lib/pwcat.c $(LDFLAGS) -o $@

grcat$(EXEEXT): stamp-eg
	$(COMPILE) `dirname $^`/eg/lib/grcat.c $(LDFLAGS) -o $@

xmlgawk: $(srcdir)/xml/xmlgawk.sh
	cp $^ $@ ; chmod 755 $@

passwd.awk: stamp-eg
	sed 's;/usr/local/libexec/awk;$(pkglibexecdir);' < `dirname $^`/eg/lib/passwdawk.in > passwd.awk

group.awk: stamp-eg
	sed 's;/usr/local/libexec/awk;$(pkglibexecdir);' < `dirname $^`/eg/lib/groupawk.in > group.awk
