## Process this file with automake to produce Makefile.in.

INCLUDES = -I.. -I$(top_srcdir)

libdir = ${exec_prefix}/lib/awk/@VERSION@

lib_LTLIBRARIES =        \
        libarrayparm.la  \
        libdl.la         \
        libfilefuncs.la  \
        libfork.la       \
        libordchr.la     \
        libreadfile.la   \
        libtestarg.la    \
        libxml.la

# Should we really install all of these libraries?  The arrayparm, dl,
# and testarg libraries do not do anything useful...

#install_libraries =        \
#        libfilefuncs.la  \
#        libfork.la       \
#        libordchr.la     \
#        libreadfile.la   \
#        libxml.la

install_libraries = $(lib_LTLIBRARIES)

#libxml_la_LDFLAGS = -version 0:0:0

libarrayparm_la_SOURCES  = arrayparm.c
libdl_la_SOURCES         = dl.c
libfilefuncs_la_SOURCES  = filefuncs.c
libfork_la_SOURCES       = fork.c
libordchr_la_SOURCES     = ordchr.c
libreadfile_la_SOURCES   = readfile.c
libtestarg_la_SOURCES    = testarg.c
libxml_la_SOURCES        = xml_puller.c xml_interface.c

libxml_la_LIBADD = -lexpat

install: $(install_libraries)
	for p in $(install_libraries); do \
		. $$p ; \
		f=`echo $$dlname | sed -e 's/^lib//' -e 's/\(\.0\)*$$//'` ; \
		echo "Installing $$dlname in $(DESTDIR)$(libdir)/$$f" ; \
		$(INSTALL_PROGRAM) .libs/$$dlname $(DESTDIR)$(libdir)/$$f ; \
	done

