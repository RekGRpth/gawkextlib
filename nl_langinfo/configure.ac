dnl Process this file with autoconf to produce a configure script.

AC_INIT([Gawk nl_langinfo Extension], 1.1.0, gawkextlib-users@lists.sourceforge.net, gawk-nl_langinfo)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([-Wall])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_PURE_GAWK_EXTENSION

AC_CONFIG_HEADERS([config.h:configh.in])

AC_CONFIG_FILES(Makefile
	doc/Makefile
	packaging/Makefile
	packaging/gawk-nl_langinfo.spec
	po/Makefile.in
	test/Makefile)
AC_OUTPUT
