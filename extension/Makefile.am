## Process this file with automake to produce Makefile.in.

AM_CPPFLAGS = -I$(srcdir)/..

if BUILD_XML
xml_lib = xml.la
else
xml_lib = 
endif

if BUILD_PGSQL
pgsql_lib = pgsql.la
else
pgsql_lib = 
endif

if BUILD_MPFR
mpfr_lib = mpfr.la
else
mpfr_lib = 
endif

if BUILD_GD
gd_lib = gd.la
else
gd_lib =
endif

if BUILD_PDF
pdf_lib = pdf.la
else
pdf_lib =
endif

pkgextension_LTLIBRARIES =	\
	$(xml_lib)	\
	$(pgsql_lib)	\
	$(gd_lib)	\
	$(pdf_lib)	\
	$(mpfr_lib)

xml_enc_converted.touch: $(srcdir)/xml-conv-enc
	LANG=C LC_ALL=C $(SHELL) $(srcdir)/xml-conv-enc && touch $@

xml_enc_handler.lo: xml_enc_converted.touch

MY_MODULE_FLAGS = -module -avoid-version -no-undefined

MY_LIBS = ../libgawkextlib.la $(LTLIBINTL)

xml_la_SOURCES		= xml_puller.c xml_interface.c xml_enc_handler.c
xml_la_LIBADD		= -lexpat $(MY_LIBS)
xml_la_LDFLAGS		= $(MY_MODULE_FLAGS)

pgsql_la_SOURCES	= pgsql.c
pgsql_la_LIBADD		= -lpq $(MY_LIBS)
pgsql_la_LDFLAGS	= $(MY_MODULE_FLAGS)

gd_la_SOURCES		= gd.c
gd_la_LIBADD		= -lgd $(MY_LIBS)
gd_la_LDFLAGS		= $(MY_MODULE_FLAGS)

pdf_la_SOURCES		= pdf.c
pdf_la_LIBADD		= -lhpdf $(MY_LIBS)
pdf_la_LDFLAGS		= $(MY_MODULE_FLAGS)

mpfr_la_SOURCES		= mpfr.c
mpfr_la_LIBADD		= -lmpfr -lgmp $(MY_LIBS)
mpfr_la_LDFLAGS		= $(MY_MODULE_FLAGS)

# Remove .la files that are not useful in this context.
install-data-hook:
	for i in $(pkgextension_LTLIBRARIES) ; do \
	   $(RM) $(pkgextensiondir)/$$i ; \
	done

# Extra files to remove when cleaning
CLEANFILES = xml_enc_converted.touch xml_enc_registry.inc xml_enc_tables.inc \
		extension.list static.link

EXTRA_DIST = \
	*ChangeLog \
	*.awk \
	common.h \
	encodings/README \
	encodings/*.enc \
	encodings/*.msg \
	encoding.h \
	sample.xml \
	unused.h \
	xml-conv-enc \
	xml_enc_handler.h \
	xml_puller.h
