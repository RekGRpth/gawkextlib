## Process this file with automake to produce Makefile.in.

INCLUDES = -I.. -I$(top_srcdir) -I$(top_srcdir)/intl 

libdir = ${exec_prefix}/lib/awk/@VERSION@

if BUILD_XMLGAWK
xmlgawk_LIBRARY = 
else
xmlgawk_LIBRARY = libxml.la
libxml_la_SOURCES        = xml_puller.c xml_interface.c
libxml_la_LIBADD = -lexpat
endif

lib_LTLIBRARIES =	\
	libarrayparm.la	\
	libfilefuncs.la	\
	libfork.la	\
	libordchr.la	\
	libreadfile.la	\
	libtestarg.la	\
	libzaxxon.la	\
	$(xmlgawk_LIBRARY)

# The arrayparm, zaxxon, and testarg libraries do not do anything useful,
# so do not install them...

install_libraries =	\
	libfilefuncs.la	\
	libfork.la	\
	libordchr.la	\
	libreadfile.la	\
	$(xmlgawk_LIBRARY)

#install_libraries = $(lib_LTLIBRARIES)

libarrayparm_la_SOURCES  = arrayparm.c
libfilefuncs_la_SOURCES  = filefuncs.c
libfork_la_SOURCES       = fork.c
libordchr_la_SOURCES     = ordchr.c
libreadfile_la_SOURCES   = readfile.c
libtestarg_la_SOURCES    = testarg.c
libzaxxon_la_SOURCES     = zaxxon.c


install: $(install_libraries)
	for p in $(install_libraries); do \
		. $$p ; \
		f=`echo $$dlname | sed -e 's/^lib//' -e 's/\.0//g'` ; \
		echo "Installing $$dlname in $(DESTDIR)$(libdir)/$$f" ; \
		$(INSTALL_PROGRAM) .libs/$$dlname $(DESTDIR)$(libdir)/$$f ; \
	done

