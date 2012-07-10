dnl Process this file with autoconf to produce a configure script.

AC_INIT([Gawk Libraries], 0.1.1, xmlgawk-users@lists.sourceforge.net, gawklib)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

INSTALL="$ac_aux_dir/install-sh -c"
export INSTALL

AM_INIT_AUTOMAKE([-Wall -Werror])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST([pkgextensiondir], ['${libdir}/gawk'])
AC_SUBST([pkgdatadir], ['${datadir}/awk'])

if test "$GCC" = yes
then
	CFLAGS="$CFLAGS -Wall -Wextra"
fi

AC_SUBST([GAWKPROG],"gawk${EXEEXT}")
AC_ARG_WITH(gawk,
	[  --with-gawk=PATH        Use gawk in PATH],
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

AC_ARG_WITH(expat,
	[  --with-expat=PATH        Use expat in PATH],
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

AC_ARG_WITH(libpq,
	[  --with-libpq=PATH        Use PostgreSQL libpq in PATH],
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

AC_ARG_WITH(mpfr,
	[  --with-mpfr=PATH        Use MPFR libmpfr in PATH],
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

AC_ARG_WITH(gmp,
	[  --with-gmp=PATH        Use GMP libgmp in PATH],
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

AC_ARG_WITH(gd,
	[  --with-gd=PATH        Use GD in PATH],
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

AC_ARG_ENABLE(xml,
	AC_HELP_STRING([--disable-xml],[Do not build XML extension @<:@default=build if <expat.h> is present@:>@]))

if test x"$enable_xml" != x"no" ; then
	AC_CHECK_HEADER(expat.h)
	if test x"$enable_xml" = x"yes" -a x"$ac_cv_header_expat_h" = x"no" ; then
		AC_MSG_ERROR([Configured with --enable-xml, but C compiler cannot find expat.h])
	fi
fi
AM_CONDITIONAL(BUILD_XML, test x"$ac_cv_header_expat_h" = x"yes")

AC_ARG_ENABLE(pgsql,
	AC_HELP_STRING([--disable-pgsql],[Do not build pgsql extension @<:@default=build if <libpq-fe.h> is present@:>@]))

if test x"$enable_pgsql" != x"no" ; then
	AC_CHECK_HEADER(libpq-fe.h)
	if test x"$enable_pgsql" = x"yes" -a x"$ac_cv_header_libpq_fe_h" = x"no" ; then
		AC_MSG_ERROR([Configured with --enable-pgsql, but C compiler cannot find libpq-fe.h])
	fi
fi
AM_CONDITIONAL(BUILD_PGSQL, test x"$ac_cv_header_libpq_fe_h" = x"yes")

AC_ARG_ENABLE(mpfr,
	AC_HELP_STRING([--disable-mpfr],[Do not build MPFR extension @<:@default=build if libmpfr has function mpfr_free_str@:>@]))
dnl Recognition of MPFR could be done by recognizing the header file mpfr.h.
dnl This led to trouble because GMP comes with an incompatible version of mpfr.h
dnl So, after detecting presence of the GMP lib, the only test that will be done
dnl is recognition of the function mpfr_free_str which is expected to reside in
dnl libmpfr since version 2.1 of MPFR.
if test x"$enable_mpfr" != x"no" ; then
	AC_CHECK_LIB(gmp, __gmpz_init, [true])
	if test x"$ac_cv_lib_gmp___gmpz_init" = x"yes" ; then
		# Since libmpfr is typically static, it may not pull in
		# libgmp automatically, so we specify it explicitly to be safe.
		AC_CHECK_LIB(mpfr, mpfr_free_str, [true], [true], -lgmp)
		if test x"$ac_cv_lib_mpfr_mpfr_free_str" = x"no" ; then
			AC_MSG_WARN([Configured with --enable-mpfr, but MPFR was not found or does not have required features of MPFR 2.2. Go install MPFR 2.2 or later from http://www.mpfr.org/mpfr-current and try again.])
		fi
	else
		AC_MSG_WARN([Configured with --enable-mpfr, but GMP was not found. Go install GMP 4.1.4 or later from http://www.swox.com/gmp and try again.])
	fi
fi
AM_CONDITIONAL(BUILD_MPFR, test x"$ac_cv_lib_mpfr_mpfr_free_str" = x"yes")

AC_ARG_ENABLE(gd,
	AC_HELP_STRING([--disable-gd],[Do not build GD graphics extension @<:@default=build if <gd.h> is present@:>@]))

if test x"$enable_gd" != x"no" ; then
	AC_CHECK_HEADER(gd.h)
	if test x"$enable_gd" = x"yes" -a x"$ac_cv_header_gd_h" = x"no" ; then
		AC_MSG_ERROR([Configured with --enable-gd, but C compiler cannot find gd.h])
	fi
fi
AM_CONDITIONAL(BUILD_GD, test x"$ac_cv_header_gd_h" = x"yes")

AC_CHECK_HEADERS(gawkapi.h unistd.h stdlib.h string.h)

if test x"$ac_cv_header_gawkapi_h" = x"no" ; then
	AC_MSG_ERROR([Cannot find gawkapi.h.  Please use --with-gawk to supply a location for your gawk build.])
fi

AC_C_INLINE

AC_CONFIG_HEADERS([config.h:configh.in])

# temporarily omit po/Makefile.in
AC_CONFIG_FILES(Makefile
	awklib/Makefile
	doc/Makefile
	extension/Makefile
	packaging/Makefile
	packaging/gawklib.spec
	test/Makefile)
AC_OUTPUT
