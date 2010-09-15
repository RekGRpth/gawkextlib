## Process this file with automake to produce Makefile.in.

INCLUDES = -I.. -I$(top_srcdir) -I$(top_srcdir)/intl 

if BUILD_XML
xml_lib = libxml.la
else
xml_lib = 
endif

if BUILD_PGSQL
pgsql_lib = libpgsql.la
else
pgsql_lib = 
endif

if BUILD_MPFR
mpfr_lib = libmpfr.la
else
mpfr_lib = 
endif

if BUILD_GD
gd_lib = libgd.la
else
gd_lib =
endif

lib_LTLIBRARIES =	\
	libarrayparm.la	\
	libfilefuncs.la	\
	libfork.la	\
	libordchr.la	\
	libreadfile.la	\
	libtime.la	\
	libtestarg.la	\
	libzaxxon.la	\
	$(xml_lib)	\
	$(pgsql_lib)	\
	$(mpfr_lib) 	\
	$(gd_lib)

# The arrayparm, zaxxon, and testarg libraries do not do anything useful,
# so do not install them...

install_libraries =	\
	libfilefuncs.la	\
	libfork.la	\
	libordchr.la	\
	libreadfile.la	\
	libtime.la	\
	$(xml_lib)	\
	$(pgsql_lib)	\
	$(mpfr_lib) 	\
	$(gd_lib)

xml_enc_converted.touch: $(srcdir)/xml-conv-enc
	$(SHELL) $(srcdir)/xml-conv-enc && touch $@

xml_enc_handler.lo: xml_enc_converted.touch

libarrayparm_la_SOURCES  = arrayparm.c
libfilefuncs_la_SOURCES  = filefuncs.c
libfork_la_SOURCES       = fork.c
libordchr_la_SOURCES     = ordchr.c
libreadfile_la_SOURCES   = readfile.c
libtime_la_SOURCES       = time.c
libtestarg_la_SOURCES    = testarg.c
libzaxxon_la_SOURCES     = zaxxon.c

libxml_la_SOURCES	= xml_puller.c xml_interface.c xml_enc_handler.c
libxml_la_LIBADD	= -lexpat

libpgsql_la_SOURCES	= pgsql.c
libpgsql_la_LIBADD	= -lpq

libmpfr_la_SOURCES	= mpfr.c
libmpfr_la_LIBADD	= -lmpfr -lgmp

libgd_la_SOURCES	= gd.c
libgd_la_LIBADD		= -lgd

install: $(install_libraries)
if BUILD_STATIC_EXTENSIONS
	@echo "The extensions are static, so we do not install them."
else
	for p in $(install_libraries); do \
		. ./$$p ; \
		f=`echo $$dlname | sed -e 's/^lib//' -e 's/\.0//g'` ; \
		echo "Installing $$dlname in $(DESTDIR)$(pkgextensiondir)/$$f" ; \
		$(INSTALL_PROGRAM) .libs/$$dlname $(DESTDIR)$(pkgextensiondir)/$$f ; \
	done
endif

uninstall: $(install_libraries)
if BUILD_STATIC_EXTENSIONS
	@echo "The extensions are static, so there is nothing to remove."
else
	for p in $(install_libraries); do \
		. ./$$p ; \
		f=`echo $$dlname | sed -e 's/^lib//' -e 's/\.0//g'` ; \
		echo "Removing $$dlname in $(DESTDIR)$(pkgextensiondir)/$$f" ; \
		rm -f $(DESTDIR)$(pkgextensiondir)/$$f ; \
	done
endif

static.link: $(install_libraries) Makefile
	for p in $(install_libraries); do \
		. ./$$p ; \
		echo "extension/.libs/$$old_library" ; \
	done > $@
if BUILD_XML
	echo " $(libxml_la_LIBADD)" >> $@
endif
if BUILD_PGSQL
	echo " $(libpgsql_la_LIBADD)" >> $@
endif
if BUILD_MPFR
	echo " $(libmpfr_la_LIBADD)" >> $@
endif
if BUILD_GD
	echo " $(libgd_la_LIBADD)" >> $@
endif

extension.list: Makefile
	echo $(install_libraries) | sed -e 's/lib\([^.]*\)\.la/\1/g' > $@

# Extra files to remove when cleaning
CLEANFILES = xml_enc_converted.touch xml_enc_registry.inc xml_enc_tables.inc \
		extension.list static.link

EXTRA_DIST = \
	*ChangeLog \
	*.awk \
	encodings/README \
	encodings/*.enc \
	encodings/*.msg \
	xml-conv-enc \
	Makefile.pc \
	pcext.def \
	doit \
	steps \
	encoding.h \
	xml_puller.h \
	sample.xml
