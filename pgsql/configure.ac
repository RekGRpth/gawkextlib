dnl Process this file with autoconf to produce a configure script.

AC_INIT([Gawk PostgreSQL Extension], 1.1.0, gawkextlib-users@lists.sourceforge.net, gawk-pgsql)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([-Wall])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_GAWK_EXTENSION

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
		if test "$wldfound" = 0; then
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

AC_CHECK_HEADERS(libpq-fe.h)

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

AC_CONFIG_HEADERS([config.h:configh.in])

AC_CONFIG_FILES(Makefile
	doc/Makefile
	packaging/Makefile
	packaging/gawk-pgsql.spec
	po/Makefile.in
	test/Makefile)
AC_OUTPUT
