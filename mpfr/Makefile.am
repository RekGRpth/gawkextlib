# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = mpfr.la

mpfr_la_SOURCES		= mpfr.c
mpfr_la_LIBADD		= -lmpfr -lgmp -lgawkextlib $(LTLIBINTL)
mpfr_la_LDFLAGS		= -module -avoid-version -no-undefined

SUBDIRS = doc po packaging test

EXTRA_DIST = common.h unused.h
