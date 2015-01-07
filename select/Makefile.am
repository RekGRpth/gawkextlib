# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = select.la

select_la_SOURCES	= select.c
select_la_LIBADD	= -lgawkextlib $(LTLIBINTL)
select_la_LDFLAGS	= $(GAWKEXT_MODULE_FLAGS)

GENHDR = name2signal.h

$(GENHDR): siglist.awk
	echo '#include <signal.h>' | cpp -M | $(AWK) -f $< > $@

# declare additional dependencies
$(select_la_OBJECTS): $(GENHDR)

CLEANFILES = $(GENHDR)

SUBDIRS = doc po packaging test

EXTRA_DIST = common.h unused.h *.awk
