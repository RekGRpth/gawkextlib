## Process this file with automake to produce Makefile.in.

# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

pkgextension_LTLIBRARIES = pgsql.la

pgsql_la_SOURCES	= pgsql.c
pgsql_la_LIBADD		= -lpq -lgawkextlib $(LTLIBINTL)
pgsql_la_LDFLAGS	= -module -avoid-version -no-undefined

SUBDIRS = doc po packaging test

# Remove .la files that are not useful in this context.
install-data-hook:
	for i in $(pkgextension_LTLIBRARIES) ; do \
	   $(RM) $(DESTDIR)$(pkgextensiondir)/$$i ; \
	done

EXTRA_DIST = common.h unused.h *.awk

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
