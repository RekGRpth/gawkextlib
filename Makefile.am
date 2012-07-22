## process this file with automake to produce Makefile.in

# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

# This insures that make flags get passed down to child makes.
AM_MAKEFLAGS = 'CFLAGS=$(CFLAGS)' 'LDFLAGS=$(LDFLAGS)'

lib_LTLIBRARIES = libgawkextlib.la

libgawkextlib_la_SOURCES	= strhash.c varinit.c
libgawkextlib_la_LDFLAGS	= -no-undefined

# Stuff to include in the dist that doesn't need it's own
# Makefile.am files
EXTRA_DIST = \
	build-aux/config.rpath  \
	COPYING \
	m4/*.m4 \
	update-autotools \
	*ChangeLog

# Build explicitly in "." in order to build gawkextlib first
SUBDIRS = \
	. \
	awklib \
	doc \
	po \
	extension \
	packaging \
	test

include_HEADERS = gawkextlib.h

# This is for my development & testing.
efence: gawk
	$(CC) $(LDFLAGS) -o gawk $$(ls *.o | grep -v '_p.o$$') $(LIBS) -lefence

diffout valgrind-scan:
	@cd test && $(MAKE) $(AM_MAKEFLAGS) $@

valgrind:
	cd test; rm -f log.[0-9]*; \
	make check VALGRIND="valgrind --leak-check=full --log-file=log.%p"; \
	make valgrind-scan

valgrind-noleak:
	cd test; rm -f log.[0-9]*; \
	make check VALGRIND="valgrind --leak-check=no --log-file=log.%p"; \
	make valgrind-scan
