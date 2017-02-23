# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = csv.la

csv_la_SOURCES	= csv.c csv_parser.c csv_convert.c csv_split.c strbuf.c
csv_la_LIBADD	= $(LTLIBINTL)
csv_la_LDFLAGS	= $(GAWKEXT_MODULE_FLAGS)

SUBDIRS = awklib doc po packaging test

EXTRA_DIST = common.h common_aux.h unused.h strbuf.h csv_convert.h csv_split.h csv_parser.h
