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

