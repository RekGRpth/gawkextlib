#
# Makefile.am --- automake input file for gawk
#
# Copyright (C) 2000-2005 the Free Software Foundation, Inc.
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

# Automatic de-ANSI-fication if needed, make .bz2 files also.
AUTOMAKE_OPTIONS = ansi2knr dist-bzip2

# Especially for distributions of the XMLgawk project.
distdir = x$(PACKAGE)-$(VERSION)-$(CONFDATE)


# This undocumented variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

# This insures that make flags get passed down to child makes.
AM_MAKEFLAGS = 'CFLAGS=$(CFLAGS)' 'LDFLAGS=$(LDFLAGS)'

# Stuff to include in the dist that doesn't need it's own
# Makefile.am files
EXTRA_DIST = \
	COPYING \
	FUTURES \
	INSTALL \
	LIMITATIONS \
	NEWS \
	POSIX.STD \
	PROBLEMS \
	README_d/README.* \
	bisonfix.awk \
	config.guess \
	config.rpath  \
	config.sub \
	depcomp \
	m4/*.m4 \
	m4/*ChangeLog \
	missing \
	missing_d/*.c \
	missing_d/*ChangeLog \
	missing_d/COPYING.LIB \
	pc/*.[ch] \
	pc/Makefile* \
	pc/*ChangeLog \
	pc/*.def \
	pc/*.sh \
	pc/*.awk \
	pc/*.pc \
	pc/include/*.h \
	pc/include/sys/*.h \
	posix/*.c \
	posix/*ChangeLog \
	regcomp.c \
	regex_internal.c \
	regex_internal.h \
	regexec.c \
	unsupported/atari/*.[ch] \
	unsupported/atari/Makefile* \
	unsupported/atari/*ChangeLog \
	unsupported/atari/README* \
	unsupported/atari/*.atr \
	unsupported/tandem/*.[ch] \
	unsupported/tandem/*ChangeLog \
	unsupported/tandem/compit \
	vms/*.[ch] \
	vms/*.sh \
	vms/*.com \
	vms/*ChangeLog \
	vms/descrip.mms \
	vms/gawk*.* \
	ylwrap \
	*ChangeLog

# The order to do things in.
# Build explicitly in "." in order to build gawk first, so
# that `make check' without a prior `make' works.
# Must build "." before "awklib" because "awklib" uses built
# gawk to generate the examples.
# Since 3.1.5, Arnold removed "intl", why ?
SUBDIRS = \
        intl \
	. \
	awklib \
	doc \
	po \
	extension \
	test

# what to make and install
bin_PROGRAMS = gawk pgawk

# do we need extra libraries?
if BUILD_STATIC_EXTENSIONS

ext.o: ext.init ext.decl

ext.init: extension/extension.list
	while read libraries ; do \
		for lib in $$libraries ; do \
			echo '{ "'$$lib'", dlload_'$$lib', 0 },' ; \
		done ; \
	done < $< > $@

ext.decl: extension/extension.list
	while read libraries ; do \
		for lib in $$libraries ; do \
		  echo 'extern NODE *dlload_'$$lib' P((NODE *, void *));' ; \
		done ; \
	done < $< > $@

extension/extension.list:
	cd extension && $(MAKE) $(AM_MAKEFLAGS) extension.list

extension/static.link:
	cd extension && $(MAKE) $(AM_MAKEFLAGS) static.link

staticlibs = `cat extension/static.link`

# I think there should be some way to do this without setting it separately
# for gawk and pgawk, but just setting DEPENDENCIES does not work
gawk_DEPENDENCIES = extension/static.link
pgawk_DEPENDENCIES = $(gawk_DEPENDENCIES)

endif

# sources for both gawk and pgawk
base_sources = \
	array.c \
	awk.h \
	awkgram.y \
	builtin.c \
	custom.h \
	dfa.c \
	dfa.h \
	ext.c \
	field.c \
	gawkmisc.c \
	getopt.c \
	getopt.h \
	getopt1.c \
	getopt_int.h \
	gettext.h \
	hard-locale.h \
	io.c \
	mbsupport.h \
	main.c \
	msg.c \
	node.c \
	protos.h \
	random.c \
	random.h \
	re.c \
	regex.c \
	regex.h \
	replace.c \
	version.c

gawk_SOURCES = $(base_sources) eval.c profile.c
pgawk_SOURCES = $(base_sources) eval_p.c profile_p.c

# Get extra libs as needed, Automake will supply LIBINTL and SOCKET_LIBS.
LDADD = $(staticlibs) $(LIBINTL) $(SOCKET_LIBS)

# Directory for gawk's data files. Automake supplies datadir.
pkgdatadir = $(datadir)/awk
libexecdir = @libexecdir@/awk
extensiondir = @libdir@/awk/@VERSION@

SHLIBEXT = "\"$(shlibext)"\"

# stuff for compiling gawk/pgawk
DEFPATH="\".$(PATH_SEPARATOR)$(pkgdatadir)\""
DEFLIBPATH="\"$(extensiondir)\""

DEFS= -DDEFPATH=$(DEFPATH) -DDEFLIBPATH=$(DEFLIBPATH) -DSHLIBEXT=$(SHLIBEXT) -DHAVE_CONFIG_H -DGAWK -DLOCALEDIR="\"$(datadir)/locale\""

AM_CPPFLAGS =

# Get rid of core files when cleaning
CLEANFILES = core core.*

MAINTAINERCLEANFILES = version.c awkgram.c

# We want hard links for install-exec-hook, below
LN= ln

SUFFIXES = .i
.c.i:
	$(COMPILE) -E $< > $@

# First, add a link from gawk to gawk-X.Y.Z.
# Same for pgawk.
#
# For GNU systems where gawk is awk, add a link to awk.
# (This is done universally, which may not always be right, but
# there's no easy way to distinguish GNU from non-GNU systems.)
install-exec-hook:
	[ "$(transform)" = "s,x,x," ] && (cd $(DESTDIR)$(bindir); \
	$(LN) gawk$(EXEEXT) gawk-$(VERSION)$(EXEEXT) 2>/dev/null ; \
	$(LN) pgawk$(EXEEXT) pgawk-$(VERSION)$(EXEEXT) 2>/dev/null ; \
	if [ ! -f awk$(EXEEXT) ]; \
	then	$(LN_S) gawk$(EXEEXT) awk$(EXEEXT); \
	fi; \
	if [ ! -f igawk$(EXEEXT) ]; \
	then	$(LN_S) gawk$(EXEEXT) igawk$(EXEEXT); \
	fi; \
	exit 0) || :

# Undo the above when uninstalling
uninstall-links:
	[ "$(transform)" = "s,x,x," ] && (cd $(DESTDIR)$(bindir); \
	if [ -f awk$(EXEEXT) ] && cmp awk$(EXEEXT) gawk$(EXEEXT) > /dev/null; then rm -f awk$(EXEEXT); fi ; \
	if [ -f igawk$(EXEEXT) ] && cmp igawk$(EXEEXT) gawk$(EXEEXT) > /dev/null; then rm -f igawk$(EXEEXT); fi ; \
	rm -f gawk-$(VERSION)$(EXEEXT) pgawk-$(VERSION)$(EXEEXT); exit 0) || :

uninstall-recursive: uninstall-links

# force there to be a gawk executable before running tests
check-local: gawk$(EXEEXT) pgawk$(EXEEXT)

# Special rules for individual files
awkgram.c: awkgram.y
	$(YACC) $(AM_YFLAGS) $(YFLAGS) $<
	awk -f $(srcdir)/bisonfix.awk y.tab.c > $*.c && rm y.tab.c
	if test -f y.tab.h; then \
	if cmp -s y.tab.h $*.h; then rm -f y.tab.h; else mv y.tab.h $*.h; fi; \
	else :; fi

# This is for my development & testing.
efence: gawk
	$(CC) $(LDFLAGS) -o gawk $$(ls *.o | grep -v '_p.o$$') $(LIBS) -lefence

diffout:
	@$(MAKE) -C test $@
