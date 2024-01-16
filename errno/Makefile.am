# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = errno.la

errno_la_SOURCES	= errno.c
errno_la_LIBADD		= $(LTLIBINTL)
errno_la_LDFLAGS	= $(GAWKEXT_MODULE_FLAGS)

# break pipelines into separate commands so we can detect errors
TSFX = tMp.bLd8

errno2name.h: errlist.awk
	echo '#include <errno.h>' | $(CPP) -M - > $@.$(TSFX)
	$(AWK) -v dupes=no -f $< $@.$(TSFX) > $@
	$(RM) $@.$(TSFX)

name2errno.h: errlist.awk
	echo '#include <errno.h>' | $(CPP) -M - > $@.$(TSFX)
	$(AWK) -v dupes=yes -f $< $@.$(TSFX) > $@
	$(RM) $@.$(TSFX)

GENHDR = errno2name.h name2errno.h

# declare additional dependencies
$(errno_la_OBJECTS): $(GENHDR)

CLEANFILES = $(GENHDR)

SUBDIRS = doc po packaging test

EXTRA_DIST = common.h unused.h errlist.awk
