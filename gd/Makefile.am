# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = gd.la

gd_la_SOURCES		= gd.c
gd_la_LIBADD		= -lgd -lgawkextlib $(LTLIBINTL)
gd_la_LDFLAGS		= $(GAWKEXT_MODULE_FLAGS)

SUBDIRS = doc po packaging test

EXTRA_DIST = common.h unused.h testgd.awk
