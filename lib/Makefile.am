## process this file with automake to produce Makefile.in

# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libgawkextlib.la

libgawkextlib_la_SOURCES	= strhash.c varinit.c
libgawkextlib_la_LDFLAGS	= -no-undefined

# The contents of XMLChangeLog are now so old that I don't think we need
# to bother distributing it.
# EXTRA_DIST = XMLChangeLog

SUBDIRS = packaging

include_HEADERS = gawkextlib.h
