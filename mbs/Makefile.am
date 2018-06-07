# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = mbs.la

mbs_la_SOURCES	= mbs.c
mbs_la_LIBADD	= $(LTLIBINTL)
mbs_la_LDFLAGS	= $(GAWKEXT_MODULE_FLAGS)

SUBDIRS = doc po packaging test

# Note: please modify this to contain the header files actually used by
# this extension, as well as other files needed to build the library.
EXTRA_DIST = common.h unused.h
