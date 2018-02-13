# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = redis.la

redis_la_SOURCES	= redis.c
redis_la_LIBADD		= -lhiredis $(LTLIBINTL)
redis_la_LDFLAGS	= $(GAWKEXT_MODULE_FLAGS)

SUBDIRS = po packaging test

EXTRA_DIST = common.h unused.h doc/README.md
