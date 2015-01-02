# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = select.la

select_la_SOURCES	= select.c
select_la_LIBADD	= $(LTLIBINTL)
select_la_LDFLAGS	= $(GAWKEXT_MODULE_FLAGS)

signal2name.h name2signal: siglist.awk
	echo '#include <signal.h>' | cpp -M | $(AWK) -v mode=$< -f $< > $@

GENHDR = signal2name.h name2signal.h

# declare additional dependencies
$(select_la_OBJECTS): $(GENHDR)

CLEANFILES = $(GENHDR)

SUBDIRS = doc po packaging test

EXTRA_DIST = common.h unused.h *.awk
