# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = timex.la

timex_la_SOURCES	= timex.c
timex_la_LIBADD	= $(LTLIBINTL)
timex_la_LDFLAGS	= $(GAWKEXT_MODULE_FLAGS)

SUBDIRS = doc po packaging test

# Note: please modify this to contain the header files actually used by
# this extension, as well as other files needed to build the library.
EXTRA_DIST = common.h unused.h
