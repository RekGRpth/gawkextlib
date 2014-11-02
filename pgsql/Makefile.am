# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = pgsql.la

pgsql_la_SOURCES	= pgsql.c
pgsql_la_LIBADD		= -lpq -lgawkextlib $(LTLIBINTL)
pgsql_la_LDFLAGS	= -module -avoid-version -no-undefined

SUBDIRS = doc po packaging test

EXTRA_DIST = common.h unused.h *.awk
