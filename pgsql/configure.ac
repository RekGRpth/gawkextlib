dnl Process this file with autoconf to produce a configure script.

AC_INIT([Gawk PostgreSQL Extension], 1.0.0, gawkextlib-users@lists.sourceforge.net, gawk-pgsql)

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
		# some platforms use lib64 and others use lib
		wldfound=0
		for wldir in "$withval"/lib* ; do
			if test -d "$wldir"; then
				LDFLAGS="-L${wldir} ${LDFLAGS}"
				AC_SUBST([GAWKEXTLIBDIR],"$wldir")
				wldfound=1
			fi
		done
		if [ "$wldfound" = 0 ]; then
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

AC_ARG_WITH(libpq,
	[AS_HELP_STRING([--with-libpq=PATH],[Use PostgreSQL libpq in PATH])],
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
		LIBPQ_PATH="${withval}"
	]
)

AC_ARG_WITH(libpq-conninfo,
	[AS_HELP_STRING([--with-libpq-conninfo=CONNINFO],[Use CONNINFO to connect to PostgreSQL server for pgsql tests])],
	[PQCONNINFO="$withval"],
	[PQCONNINFO=""]
)
AC_SUBST([PQCONNINFO])

AC_CHECK_HEADERS(gawkapi.h gawkextlib.h libpq-fe.h unistd.h stdlib.h string.h libintl.h)

if test x"$ac_cv_header_gawkapi_h" = x"no" ; then
	AC_MSG_ERROR([Cannot find gawkapi.h.  Please use --with-gawk to supply a location for your gawk build.])
fi

if test x"$ac_cv_header_gawkextlib_h" = x"no" ; then
	AC_MSG_ERROR([Cannot find gawkextlib.h.  Please use --with-gawkextlib to supply a location for your gawkextlib build.])
fi

if test x"$ac_cv_header_libpq_fe_h" = x"no" ; then
	AC_MSG_ERROR([Cannot find libpq-fe.h.  Please use --with-libpq to supply a location for your libpq build.])
fi

# test if we can connect to a PostgreSQL server to run tests
AS_ECHO_N(["checking for ability to connect to a PostgreSQL server... "])
if test x"$LIBPQ_PATH" = x ; then
	pqpath="$PATH"
else
	pqpath="$LIBPQ_PATH/bin:$PATH"
fi
# sadly, newer psql gets confused by -d ""
if test x"$PQCONNINFO" = x ; then
	pgres=`PATH="$pqpath" psql -At -c "SELECT 1"`
else
	pgres=`PATH="$pqpath" psql -At -c "SELECT 1" -d "$PQCONNINFO"`
fi
if test x"$pgres" = x"1" ; then
	AS_ECHO(["yes"])
	got_pgconn=1
else
	AS_ECHO(["no"])
	got_pgconn=0
	test x"$PQCONNINFO" = x || AC_MSG_ERROR([Configured with --with-libpq-conninfo, but unable to connect to PostgreSQL server])
fi
AM_CONDITIONAL(TEST_PGSQL, test x"$got_pgconn" = x"1")

AC_C_INLINE

AC_CONFIG_HEADERS([config.h:configh.in])

AC_CONFIG_FILES(Makefile
	doc/Makefile
	packaging/Makefile
	packaging/gawk-pgsql.spec
	po/Makefile.in
	test/Makefile)
AC_OUTPUT
