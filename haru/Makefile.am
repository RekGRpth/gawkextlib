# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = haru.la

haru_la_SOURCES		= pdf.c
haru_la_LIBADD		= -lhpdf -lgawkextlib $(LTLIBINTL)
haru_la_LDFLAGS		= -module -avoid-version -no-undefined

SUBDIRS = po packaging test

EXTRA_DIST = common.h unused.h pdf.h
