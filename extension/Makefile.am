## Process this file with automake to produce Makefile.in.

INCLUDES = -I.. -I$(top_srcdir)

lib_LTLIBRARIES = libxml.la
#libxml_la_LDFLAGS = -version 0:0:0

libxml_la_SOURCES = \
	xml_puller.c xml_interface.c

libxml_la_DEPENDENCIES = -lexpat

libxml_la_LIBADD = -lexpat

