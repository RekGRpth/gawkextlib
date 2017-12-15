dnl Process this file with autoconf to produce a configure script.

AC_INIT([Gawk Redis Extension], 1.7.0, gawkextlib-users@lists.sourceforge.net, gawk-redis)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([-Wall])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_GAWK_EXTENSION

AC_ARG_WITH(hiredis,
	[AS_HELP_STRING([--with-hiredis=PATH],[Use hiredis in PATH])],
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

AC_CHECK_HEADERS(hiredis/hiredis.h)

if test x"$ac_cv_header_hiredis_hiredis_h" = x"no" ; then
	AC_MSG_ERROR([Cannot find hiredis/hiredis.h.  Please use --with-hiredis to supply a location for your hiredis build.])
fi

# test if we can connect to a Redis server to run tests
AS_ECHO_N(["checking for ability to connect to a Redis server on localhost port 6379... "])
# test using the client
redisres=`redis-cli ping`
if test x"$redisres" = x"PONG" ; then
        AS_ECHO(["yes"])
        got_redisconn=1
else
        AS_ECHO(["no"])
        got_redisconn=0
fi

AM_CONDITIONAL(TEST_REDIS, test x"$got_redisconn" = x"1")

AC_CONFIG_HEADERS([config.h:configh.in])

AC_CONFIG_FILES(Makefile
	packaging/Makefile
	packaging/gawk-redis.spec
	po/Makefile.in
	test/Makefile)
AC_OUTPUT
