# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = xml.la

xml_enc_converted.touch: $(srcdir)/xml-conv-enc
	AWK="$(GAWKPROG)" LANG=C LC_ALL=C $(SHELL) $(srcdir)/xml-conv-enc && touch $@

xml_enc_handler.lo: xml_enc_converted.touch

xml_la_SOURCES		= xml_puller.c xml_interface.c xml_enc_handler.c
xml_la_LIBADD		= -lexpat -lgawkextlib $(LTLIBINTL) $(LTLIBICONV)
xml_la_LDFLAGS		= $(GAWKEXT_MODULE_FLAGS)

# Put po/ at the end to avoid premature recurse abort (missing install-html/pdf targets)
# Put doc/ after . to have a working xml extension available for building docs
SUBDIRS = awklib packaging test . doc po

# Extra files to remove when cleaning
CLEANFILES = xml_enc_converted.touch xml_enc_registry.inc xml_enc_tables.inc

EXTRA_DIST = \
	XMLChangeLog \
	common.h \
	encoding.h \
	unused.h \
	xml_enc_handler.h \
	xml_puller.h \
	encodings/README \
	encodings/*.enc \
	encodings/*.msg \
	sample.xml \
	xml-conv-enc
