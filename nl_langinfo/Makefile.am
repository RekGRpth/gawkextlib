# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = nl_langinfo.la

nl_langinfo_la_SOURCES	= nl_langinfo.c
nl_langinfo_la_LIBADD	= $(LTLIBINTL)
nl_langinfo_la_LDFLAGS	= $(GAWKEXT_MODULE_FLAGS)

SUBDIRS = doc po packaging test

EXTRA_DIST = common.h unused.h
