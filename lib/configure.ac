dnl Process this file with autoconf to produce a configure script.

AC_INIT([Gawk Extension Library], 1.0.1, gawkextlib-users@lists.sourceforge.net, gawkextlib)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([-Wall])

# At the moment, this library does not contain any translatable messages.
#AM_GNU_GETTEXT([external])
#AM_GNU_GETTEXT_VERSION([0.18.1])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_GAWKEXTLIB

AC_CONFIG_HEADERS([config.h:configh.in])

AC_CONFIG_FILES(Makefile
	packaging/Makefile
	packaging/gawkextlib.spec)
AC_OUTPUT
