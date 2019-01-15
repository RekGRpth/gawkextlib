# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = aregex.la

aregex_la_SOURCES	= aregex.c
aregex_la_LIBADD	= -lgawkextlib -ltre $(LTLIBINTL)
aregex_la_LDFLAGS	= $(GAWKEXT_MODULE_FLAGS)

SUBDIRS = doc po packaging test

# Note: please modify this to contain the header files actually used by
# this extension, as well as other files needed to build the library.
EXTRA_DIST = common.h unused.h
