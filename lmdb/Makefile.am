# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = lmdb.la

lmdb_la_SOURCES	= lmdb.c
lmdb_la_LIBADD	= -llmdb -lgawkextlib $(LTLIBINTL)
lmdb_la_LDFLAGS	= $(GAWKEXT_MODULE_FLAGS)

TSFX = tMp.bLd8

# break into two commands to detect errors along the way
lmdb_header.h: headerparse.awk
	echo '#include <lmdb.h>' | $(CPP) $(CPPFLAGS) -M - > $@.$(TSFX)
	$(GAWKPROG) -f $< $@.$(TSFX) > $@
	$(RM) $@.$(TSFX)

GENHDR = lmdb_header.h

# declare additional dependencies
$(lmdb_la_OBJECTS): $(GENHDR)

CLEANFILES = $(GENHDR)

SUBDIRS = doc po packaging test

EXTRA_DIST = common.h unused.h headerparse.awk
