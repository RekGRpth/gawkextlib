dnl Process this file with autoconf to produce a configure script.

AC_INIT([Gawk Haru Extension], 1.0.0, gawkextlib-users@lists.sourceforge.net, gawk-haru)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

INSTALL="$ac_aux_dir/install-sh -c"
export INSTALL

AM_INIT_AUTOMAKE([-Wall])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
dnl We need AC_SYS_LARGEFILE because gawk uses it, and there can otherwise
dnl be a discrepancy in the struct stat layout in IOBUF_PUBLIC
AC_SYS_LARGEFILE
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST([pkgextensiondir], ['${libdir}/gawk'])
AC_SUBST([pkgdatadir], ['${datadir}/awk'])

if test "$GCC" = yes
then
	CFLAGS="$CFLAGS -Wall -Wextra -Wpointer-arith -Wmissing-prototypes -Wcast-qual -Wwrite-strings -Wshadow"
fi

AC_SUBST([GAWKPROG],"gawk${EXEEXT}")
AC_ARG_WITH(gawk,
	[AS_HELP_STRING([--with-gawk=PATH],[Use gawk in PATH])],
	[
		if test -d "$withval/lib"; then
			LDFLAGS="-L${withval}/lib ${LDFLAGS}"
		else
			LDFLAGS="-L${withval} ${LDFLAGS}"
		fi
		if test -d "$withval/include"; then
			CPPFLAGS="-I${withval}/include ${CPPFLAGS}"
		else
			CPPFLAGS="-I${withval} ${CPPFLAGS}"
		fi
		if test -x "$withval/gawk${EXEEXT}"; then
			AC_SUBST([GAWKPROG],"$withval/gawk${EXEEXT}")
		elif test -x "$withval/bin/gawk${EXEEXT}"; then
			AC_SUBST([GAWKPROG],"$withval/bin/gawk${EXEEXT}")
		elif test -x "$withval/usr/bin/gawk${EXEEXT}"; then
			AC_SUBST([GAWKPROG],"$withval/usr/bin/gawk${EXEEXT}")
		fi
	]
)

AC_ARG_WITH(gawkextlib,
	[AS_HELP_STRING([--with-gawkextlib=PATH],[Use gawkextlib in PATH])],
	[
		if test -d "$withval/lib"; then
			LDFLAGS="-L${withval}/lib ${LDFLAGS}"
			AC_SUBST([GAWKEXTLIBDIR],"$withval/lib")
		else
			LDFLAGS="-L${withval} ${LDFLAGS}"
			AC_SUBST([GAWKEXTLIBDIR],"$withval")
		fi
		if test -d "$withval/include"; then
			CPPFLAGS="-I${withval}/include ${CPPFLAGS}"
		else
			CPPFLAGS="-I${withval} ${CPPFLAGS}"
		fi
		# Path for Cygwin to find the DLL
		if test -d "$withval/bin"; then
			AC_SUBST([GAWKEXTBINDIR],"$withval/bin")
		else
			AC_SUBST([GAWKEXTBINDIR],"$withval")
		fi
	]
)
AM_CONDITIONAL(GELIBDIR, test -n "$GAWKEXTLIBDIR")

AC_ARG_WITH(haru,
	[AS_HELP_STRING([--with-haru=PATH],[Use haru in PATH])],
	[
		if test -d "$withval/lib"; then
			LDFLAGS="-L${withval}/lib ${LDFLAGS}"
		else
			LDFLAGS="-L${withval} ${LDFLAGS}"
		fi
		if test -d "$withval/include"; then
			CPPFLAGS="-I${withval}/include ${CPPFLAGS}"
		else
			CPPFLAGS="-I${withval} ${CPPFLAGS}"
		fi
	]
)


AC_CHECK_HEADERS(gawkapi.h gawkextlib.h hpdf.h unistd.h stdlib.h string.h libintl.h)

if test x"$ac_cv_header_gawkapi_h" = x"no" ; then
	AC_MSG_ERROR([Cannot find gawkapi.h.  Please use --with-gawk to supply a location for your gawk build.])
fi

if test x"$ac_cv_header_gawkextlib_h" = x"no" ; then
	AC_MSG_ERROR([Cannot find gawkextlib.h.  Please use --with-gawkextlib to supply a location for your gawkextlib build.])
fi

if test x"$ac_cv_header_hpdf_h" = x"no" ; then
	AC_MSG_ERROR([Cannot find hpdf.h.  Please use --with-haru to supply a location for your haru build.])
fi

AC_C_INLINE

AC_CONFIG_HEADERS([config.h:configh.in])

AC_CONFIG_FILES(Makefile
	packaging/Makefile
	packaging/gawk-haru.spec
	po/Makefile.in
	test/Makefile)
AC_OUTPUT
