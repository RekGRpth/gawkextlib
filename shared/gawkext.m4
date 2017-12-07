AC_DEFUN([AC_GAWKEXTLIB],
[
INSTALL="$ac_aux_dir/install-sh -c"
export INSTALL

dnl We need AC_SYS_LARGEFILE because gawk uses it, and there can otherwise
dnl be a discrepancy in the struct stat layout in IOBUF_PUBLIC
AC_SYS_LARGEFILE

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

AC_CHECK_HEADERS(gawkapi.h unistd.h stdlib.h string.h libintl.h)

if test x"$ac_cv_header_gawkapi_h" = x"no" ; then
	AC_MSG_ERROR([Cannot find gawkapi.h.  Please use --with-gawk to supply a location for your gawk build.])
fi

AC_C_INLINE
])

# for packages that use gawkextlib
AC_DEFUN([AC_GAWK_EXTENSION],
[
AC_GAWKEXTLIB

AC_ARG_WITH(gawkextlib,
	[AS_HELP_STRING([--with-gawkextlib=PATH],[Use gawkextlib in PATH])],
	[
		# some platforms use lib64 and others use lib
		wldfound=0
		for wldir in "$withval"/lib* ; do
			if test -d "$wldir"; then
				LDFLAGS="-L${wldir} ${LDFLAGS}"
				AC_SUBST([GAWKEXTLIBDIR],"$wldir")
				wldfound=1
			fi
		done
		if test "$wldfound" = 0; then
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

AC_CHECK_HEADERS(gawkextlib.h)

if test x"$ac_cv_header_gawkextlib_h" = x"no" ; then
	AC_MSG_ERROR([Cannot find gawkextlib.h.  Please use --with-gawkextlib to supply a location for your gawkextlib build.])
fi
])

# for packages that do not use gawkextlib
AC_DEFUN([AC_PURE_GAWK_EXTENSION],
[
AC_GAWKEXTLIB

AM_CONDITIONAL(GELIBDIR, false)
])
