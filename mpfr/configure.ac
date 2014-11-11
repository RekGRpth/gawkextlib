dnl Process this file with autoconf to produce a configure script.

AC_INIT([Gawk MPFR Extension], 1.0.1, gawkextlib-users@lists.sourceforge.net, gawk-mpfr)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([-Wall])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_GAWK_EXTENSION

AC_ARG_WITH(mpfr,
	[AS_HELP_STRING([--with-mpfr=PATH],[Use MPFR libmpfr in PATH])],
	[
		# some platforms use lib64 and others use lib
		wldfound=0
		for wldir in "$withval"/lib* ; do
			if test -d "$wldir"; then
				LDFLAGS="-L${wldir} ${LDFLAGS}"
				wldfound=1
			fi
		done
		if [ "$wldfound" = 0 ]; then
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
	[AS_HELP_STRING([--with-gmp=PATH],[Use GMP libgmp in PATH])],
	[
		# some platforms use lib64 and others use lib
		wldfound=0
		for wldir in "$withval"/lib* ; do
			if test -d "$wldir"; then
				LDFLAGS="-L${wldir} ${LDFLAGS}"
				wldfound=1
			fi
		done
		if [ "$wldfound" = 0 ]; then
			LDFLAGS="-L${withval} ${LDFLAGS}"
		fi
		if test -d "$withval/include"; then
			CPPFLAGS="-I${withval}/include ${CPPFLAGS}"
		else
			CPPFLAGS="-I${withval} ${CPPFLAGS}"
		fi
	]
)

dnl Recognition of MPFR could be done by recognizing the header file mpfr.h.
dnl This led to trouble because GMP comes with an incompatible version of mpfr.h
dnl So, after detecting presence of the GMP lib, the only test that will be done
dnl is recognition of the function mpfr_free_str which is expected to reside in
dnl libmpfr since version 2.1 of MPFR.
AC_CHECK_LIB(gmp, __gmpz_init, [true])
if test x"$ac_cv_lib_gmp___gmpz_init" = x"yes" ; then
	# Since libmpfr is typically static, it may not pull in
	# libgmp automatically, so we specify it explicitly to be safe.
	AC_CHECK_LIB(mpfr, mpfr_free_str, [true], [true], -lgmp)
	if test x"$ac_cv_lib_mpfr_mpfr_free_str" = x"no" ; then
		AC_MSG_ERROR([MPFR was not found or does not have required features of MPFR 2.2. Go install MPFR 2.2 or later from http://www.mpfr.org/mpfr-current and try again.])
	fi
else
	AC_MSG_ERROR([GMP was not found. Go install GMP 4.1.4 or later from http://www.swox.com/gmp and try again.])
fi

AC_CONFIG_HEADERS([config.h:configh.in])

AC_CONFIG_FILES(Makefile
	doc/Makefile
	packaging/Makefile
	packaging/gawk-mpfr.spec
	po/Makefile.in
	test/Makefile)
AC_OUTPUT
