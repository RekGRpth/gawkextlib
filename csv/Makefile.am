# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = csv.la

csv_la_SOURCES	= csv.c csv_parser.c csv_convert.c csv_split.c csv_input.c strbuf.c awk_fieldwidth_info.c
csv_la_LIBADD	= -lgawkextlib $(LTLIBINTL)
csv_la_LDFLAGS	= $(GAWKEXT_MODULE_FLAGS)

SUBDIRS = awklib doc po packaging test

EXTRA_DIST = common.h common_aux.h unused.h strbuf.h csv_convert.h csv_split.h csv_parser.h csv_input.h awk_fieldwidth_info.h
