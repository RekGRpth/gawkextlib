dnl Process this file with autoconf to produce a configure script.

AC_INIT([Gawk Haru Extension], 1.0.2, gawkextlib-users@lists.sourceforge.net, gawk-haru)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([-Wall])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_GAWK_EXTENSION

AC_ARG_WITH(haru,
	[AS_HELP_STRING([--with-haru=PATH],[Use haru in PATH])],
	[
		# some platforms use lib64 and others use lib
		wldfound=0
		for wldir in "$withval"/lib* ; do
			if test -d "$wldir"; then
				LDFLAGS="-L${wldir} ${LDFLAGS}"
				wldfound=1
			fi
		done
		if test "$wldfound" = 0; then
			LDFLAGS="-L${withval} ${LDFLAGS}"
		fi
		if test -d "$withval/include"; then
			CPPFLAGS="-I${withval}/include ${CPPFLAGS}"
		else
			CPPFLAGS="-I${withval} ${CPPFLAGS}"
		fi
	]
)


AC_CHECK_HEADERS(hpdf.h)

if test x"$ac_cv_header_hpdf_h" = x"no" ; then
	AC_MSG_ERROR([Cannot find hpdf.h.  Please use --with-haru to supply a location for your haru build.])
fi

AC_CONFIG_HEADERS([config.h:configh.in])

AC_CONFIG_FILES(Makefile
	packaging/Makefile
	packaging/gawk-haru.spec
	po/Makefile.in
	test/Makefile)
AC_OUTPUT
