# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = json.la

json_la_SOURCES	= json.cpp awkjsonhandler.cpp
json_la_LIBADD	= $(LTLIBINTL)
json_la_LDFLAGS	= $(GAWKEXT_MODULE_FLAGS)

SUBDIRS = doc po packaging test

EXTRA_DIST = awkjsonhandler.h
