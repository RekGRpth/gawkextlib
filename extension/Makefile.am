## Process this file with automake to produce Makefile.in.

INCLUDES = -I.. -I$(top_srcdir) -I$(top_srcdir)/intl 

libdir = ${exec_prefix}/lib/awk/@VERSION@

if BUILD_XML
xml_lib = libxml.la
else
xml_lib = 
endif

lib_LTLIBRARIES =	\
	libarrayparm.la	\
	libfilefuncs.la	\
	libfork.la	\
	libordchr.la	\
	libreadfile.la	\
	libtestarg.la	\
	libzaxxon.la	\
	$(xml_lib)

# The arrayparm, zaxxon, and testarg libraries do not do anything useful,
# so do not install them...

if BUILD_STATIC_EXTENSIONS

install_libraries = 

else

install_libraries =	\
	libfilefuncs.la	\
	libfork.la	\
	libordchr.la	\
	libreadfile.la	\
	$(xml_lib)

endif

xml_enc_converted.touch: xml-conv-enc
	$(SHELL) ./xml-conv-enc && touch $@

xml_enc_handler.lo: xml_enc_converted.touch

clean: clean-am
	rm -f xml_enc_converted.touch xml_enc_registry.inc xml_enc_tables.inc

libarrayparm_la_SOURCES  = arrayparm.c
libfilefuncs_la_SOURCES  = filefuncs.c
libfork_la_SOURCES       = fork.c
libordchr_la_SOURCES     = ordchr.c
libreadfile_la_SOURCES   = readfile.c
libtestarg_la_SOURCES    = testarg.c
libzaxxon_la_SOURCES     = zaxxon.c

libxml_la_SOURCES	= xml_puller.c xml_interface.c xml_enc_handler.c
libxml_la_LIBADD	= -lexpat


install: $(install_libraries)
	for p in $(install_libraries); do \
		. $$p ; \
		f=`echo $$dlname | sed -e 's/^lib//' -e 's/\.0//g'` ; \
		echo "Installing $$dlname in $(DESTDIR)$(libdir)/$$f" ; \
		$(INSTALL_PROGRAM) .libs/$$dlname $(DESTDIR)$(libdir)/$$f ; \
	done

