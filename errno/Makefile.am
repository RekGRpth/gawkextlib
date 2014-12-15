# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = errno.la

errno_la_SOURCES	= errno.c
errno_la_LIBADD		= $(LTLIBINTL)
errno_la_LDFLAGS	= $(GAWKEXT_MODULE_FLAGS)

SUBDIRS = doc po packaging test

EXTRA_DIST = common.h unused.h errlist.h
