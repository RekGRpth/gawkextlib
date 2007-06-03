dnl
dnl configure.ac --- autoconf input file for gawk
dnl
dnl Copyright (C) 1995-2005 the Free Software Foundation, Inc.
dnl
dnl This file is part of GAWK, the GNU implementation of the
dnl AWK Programming Language.
dnl
dnl GAWK is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl GAWK is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
dnl

dnl Process this file with autoconf to produce a configure script.

AC_INIT([Extensible GNU Awk], 3.1.5g, xmlgawk-users@lists.sourceforge.net, xgawk)

# override gawk installation names
program_transform_name="s:^\(p\{0,1\}\)gawk:\1xgawk:"

dnl ----------
dnl configure date
dnl ----------
XGAWKBUILD="20070501"
AC_SUBST(XGAWKBUILD)

# This is a hack. Different versions of install on different systems
# are just too different. Chuck it and use install-sh.
#
# If the user supplies $INSTALL, figure they know what they're doing.
#
# With Autoconf 2.5x, this needs to come very early on, but *after*
# the INIT macro. Sigh.

if test "x$INSTALL" = "x"
then
	INSTALL="$srcdir/install-sh -c"
	export INSTALL
fi

AC_PREREQ(2.59)
AM_INIT_AUTOMAKE


dnl Additional argument stuff
AC_ARG_ENABLE(portals, [  --enable-portals	Enable /p as path prefix for portals],
	if test "$enableval" = yes
	then
		AC_DEFINE(HAVE_PORTALS, 1, [we have portals on /p on this system])
	fi
)
AC_ARG_WITH(whiny-user-strftime, [  --with-whiny-user-strftime	Force use of included version of strftime for deficient systems],
	if test "$withval" = yes
	then
		AC_DEFINE(USE_INCLUDED_STRFTIME, 1,
			  [force use of our version of strftime])
	fi
)
AC_ARG_ENABLE([lint], [  --disable-lint	Disable gawk lint checking],
	if test "$enableval" = no
	then
		AC_DEFINE(NO_LINT, 1, [disable lint checks])
	fi
)
AC_ARG_ENABLE(switch, AC_HELP_STRING([--disable-switch],[Disable switch statements for awk programs; it is enabled by default.]))
if test x"$enable_switch" != x"no" ; then
	AC_DEFINE(ALLOW_SWITCH, 1, [switch statements are enabled in awk programs])
fi
AC_ARG_ENABLE([directories-fatal], [  --disable-directories-fatal	Disable fatal errors on directories],
	if test "$enableval" = no
	then
		AC_DEFINE(NO_DIRECTORY_FATAL, 1, [disable fatal errors on directories])
	fi
)

dnl checks for programs
AC_PROG_EGREP
AC_PROG_YACC
AC_PROG_LN_S
AC_PROG_CC
AC_PROG_CPP

AC_OBJEXT
AC_EXEEXT

AC_PROG_INSTALL

AC_PROG_MAKE_SET

# This is mainly for my use during testing and development.
# Yes, it's a bit of a hack.
AC_MSG_CHECKING([for special development options])
if test -f $srcdir/.developing
then
	# add other debug flags as appropriate, save GAWKDEBUG for emergencies
	CFLAGS="$CFLAGS -DARRAYDEBUG -DYYDEBUG"
	if grep dbug $srcdir/.developing
	then
		CFLAGS="$CFLAGS -DDBUG"
		LIBS="$LIBS dbug/libdbug.a"
	fi
	# turn on compiler warnings if we're doing development
	if test "$GCC" = yes
	then
		CFLAGS="$CFLAGS -Wall -fno-builtin"
	fi
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
fi

AC_SUBST(CFLAGS)
AC_SUBST(shlibext)
AC_SUBST(libext)

# Automake supplies pkglibdir, pkgincludedir, and pkgdatadir
AC_SUBST([pkglibexecdir], ['${libexecdir}'/$PACKAGE])
AC_SUBST([pkgextensiondir], ['${pkglibdir}'/$VERSION])

dnl checks for systems
AC_USE_SYSTEM_EXTENSIONS
AC_ISC_POSIX
AC_SYS_LARGEFILE
GAWK_AC_AIX_TWEAK
GAWK_AC_LINUX_ALPHA

AC_PROG_LIBTOOL

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

AC_ARG_ENABLE(gawk,
	      AC_HELP_STRING([--enable-gawk],[Install links to provide gawk, awk, and igawk executables in addition to xgawk @<:@default=no@:>@]))

AM_CONDITIONAL(INSTALL_GAWK, test x"$enable_gawk" = x"yes")

AC_ARG_ENABLE(static-extensions,
	AC_HELP_STRING([--enable-static-extensions],[Link extension libraries statically into the gawk binary @<:@default=no@:>@]))

if test x"$enable_static_extensions" = x"yes" ; then
	AC_DEFINE(BUILD_STATIC_EXTENSIONS,,Build Static Extensions)
fi
AM_CONDITIONAL(BUILD_STATIC_EXTENSIONS, test x"$enable_static_extensions" = x"yes")

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

if test "$ISC" = 1	# will be set by test for ISC
then
dnl	need -D_SYSV3 for ISC
	CFLAGS="$CFLAGS -D_SYSV3"
fi

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

dnl check for C compiler for automake. Needed for dfa.h.
AM_C_PROTOTYPES

dnl Cygwin doesn't like to get libs with full paths
dnl since that overrides linking against DLLs.
case `(uname) 2> /dev/null` in
*CYGWIN*)
	with_libiconv_prefix=no
	with_libintl_prefix=no
	;;
*)
	;;
esac

dnl initialize GNU gettext
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.16.1])
AM_LANGINFO_CODESET
gt_LC_MESSAGES

dnl checks for header files
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_CHECK_HEADERS(arpa/inet.h fcntl.h limits.h locale.h libintl.h mcheck.h \
	netdb.h netinet/in.h  signum.h stdarg.h stddef.h string.h \
	sys/param.h sys/socket.h sys/time.h unistd.h \
	termios.h stropts.h wchar.h wctype.h sys/select.h)

if test "$ac_cv_header_string_h" = yes
then
	AC_CHECK_HEADERS(memory.h)
else
	AC_CHECK_HEADERS(strings.h)
fi

dnl checks for typedefs
AC_TYPE_PID_T
AC_TYPE_SIGNAL
AC_SIZE_T
AC_TYPE_GETGROUPS
gl_AC_TYPE_LONG_LONG
gl_AC_TYPE_UNSIGNED_LONG_LONG
gl_AC_TYPE_INTMAX_T
gl_AC_TYPE_UINTMAX_T
AC_CHECK_TYPE(ssize_t, int)
AC_CHECK_SIZEOF(unsigned int)
AC_CHECK_SIZEOF(unsigned long)
AC_EGREP_HEADER([int.*sprintf], stdio.h,
	AC_DEFINE(SPRINTF_RET, int, [return type of sprintf]),
	AC_DEFINE(SPRINTF_RET, char *))
dnl see if time_t is defined in <sys/types.h>
AC_TRY_COMPILE([#include <sys/types.h>],[
	time_t foo;
	foo = 0;
],
	AC_DEFINE(TIME_T_IN_SYS_TYPES_H, 1,
		  [some systems define this type here]))
dnl check for wctype_t in <wctype.h>
AC_TRY_COMPILE([#include <wctype.h>],[
	wctype_t foo;
	foo = 0;
],
	AC_DEFINE(HAVE_WCTYPE_T, 1, [systems should define this type here]))
dnl check for wint_t in <wctype.h>
AC_TRY_COMPILE([#include <wctype.h>],[
	wint_t foo;
	foo = 0;
],
	AC_DEFINE(HAVE_WINT_T, 1, [systems should define this type here]))
dnl check for sockaddr_storage
AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>],[
	struct sockaddr_storage foo;
],
	AC_DEFINE(HAVE_SOCKADDR_STORAGE, 1, [newer systems define this type here]))

dnl Borrwed from rsync, thanks to to Jim Meyering.

dnl Check for socklen_t: historically on BSD it is an int, and in
dnl POSIX 1g it is a type of its own, but some platforms use different
dnl types for the argument to getsockopt, getpeername, etc.  So we
dnl have to test to find something that will work.

dnl This is no good, because passing the wrong pointer on C compilers is
dnl likely to only generate a warning, not an error.

AC_DEFUN([TYPE_SOCKLEN_T],
[
   AC_CHECK_TYPE([socklen_t], ,[
      AC_MSG_CHECKING([for socklen_t equivalent])
      AC_CACHE_VAL([rsync_cv_socklen_t_equiv],
      [
         # Systems have either "struct sockaddr *" or
         # "void *" as the second argument to getpeername
         rsync_cv_socklen_t_equiv=
         for arg2 in "struct sockaddr" void; do
            for t in int size_t unsigned long "unsigned long"; do
               AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>

                  int getpeername (int, $arg2 *, $t *);
               ],[
                  $t len;
                  getpeername(0,0,&len);
               ],[
                  rsync_cv_socklen_t_equiv="$t"
                  break
               ])
            done
         done

         if test "x$rsync_cv_socklen_t_equiv" = x; then
dnl		Some systems get this. Default to int.  -- ADR
dnl            AC_MSG_ERROR([Cannot find a type to use in place of socklen_t])
		rsync_cv_socklen_t_equiv=int
         fi
      ])
      AC_MSG_RESULT($rsync_cv_socklen_t_equiv)
      AC_DEFINE_UNQUOTED(socklen_t, $rsync_cv_socklen_t_equiv,
			[type to use in place of socklen_t if not defined])],
      [#include <sys/types.h>
#include <sys/socket.h>])
])
TYPE_SOCKLEN_T

dnl checks for functions
AC_FUNC_VPRINTF
AC_FUNC_MKTIME
case "$ac_cv_func_working_mktime" in
yes)	AC_DEFINE(HAVE_MKTIME, 1, [we have the mktime function])
	;;
esac

AC_CHECK_FUNC(getaddrinfo, [AC_DEFINE(HAVE_GETADDRINFO, 1, [have getaddrinfo])],
	      [AC_CHECK_LIB(socket, getaddrinfo,
			    [AC_DEFINE(HAVE_GETADDRINFO, 1,
				       [have getaddrinfo])])])

AC_CHECK_LIB(m, fmod)

# Need the check for mkstemp and tmpfile for missing_d/snprintf.c.
AC_CHECK_FUNCS(btowc fmod getgrent getgroups grantpt \
	isascii iswctype iswlower iswupper mbrlen \
	memcmp memcpy memcpy_ulong memmove memset \
	memset_ulong mkstemp setlocale snprintf strchr \
	strerror strftime strncasecmp strtod strtoul \
	system tmpfile towlower towupper tzset wcrtomb \
	wcscoll wcscoll wctype \
	nanosleep select gettimeofday GetSystemTimeAsFileTime \
	)

dnl this check is for both mbrtowc and the mbstate_t type, which is good
AC_FUNC_MBRTOWC

dnl check for dynamic linking
dnl This is known to be very primitive
AC_CHECK_HEADER(dlfcn.h,
	[AC_DEFINE([DYNAMIC], 1, [dynamic loading is possible])
	# note: there should be a better way, something like
	#   LDFLAGS="$LDFLAGS $export_dynamic_flag_spec"
	# but I cannot figure out how to get this to work.
	# So, for now, I hope the following will work on all platforms
	# that use gcc and ELF:
	if test "$GCC" = yes
	then
		LDFLAGS="$LDFLAGS -rdynamic"
	fi

	# Check this separately. Some systems have dlopen
	# in libc. Notably freebsd and cygwin.
	# HP-NSK has it in zrldsrl
	AC_SEARCH_LIBS(dlopen, dl zrldsrl)
])

dnl check for how to use getpgrp
dnl have to hardwire it for VMS POSIX. Sigh.
dnl ditto for BeOS, OS/2, and MS-DOS.
case `(uname) 2> /dev/null` in
*VMS*|*BeOS*|*OS/2*|*MS-DOS*)
	AC_DEFINE(GETPGRP_VOID, 1,
	  [Define to 1 if the getpgrp function requires zero arguments.])
	;;
*)	AC_FUNC_GETPGRP
	;;
esac

dnl check for printf %F format
AC_MSG_CHECKING([for printf %F format])
AC_RUN_IFELSE([
#include <stdio.h>

int main()
{
	char buf[[100]];

	sprintf(buf, "%F", 123.45);

	if (strcmp(buf, "123.450000") == 0)
		return 0;
	else
		return 1;
}
],
	has_f_format=yes,
	has_f_format=no,
	has_f_format=no  dnl Cross-compiling, assuming the worst.
)
if test "$has_f_format" = yes; then
	AC_DEFINE(PRINTF_HAS_F_FORMAT, 1, [Define to 1 if *printf supports %F format])
fi
AC_MSG_RESULT($has_f_format)

dnl check for sockets
GAWK_AC_LIB_SOCKETS

dnl checks for structure members
AC_STRUCT_ST_BLKSIZE
AC_HEADER_TIME
AC_STRUCT_TM
AC_STRUCT_TIMEZONE

dnl checks for compiler characteristics
AC_C_CHAR_UNSIGNED
AC_C_CONST
AC_C_RESTRICT
AC_C_INLINE
AC_C_STRINGIZE

AC_CONFIG_HEADERS([config.h:configh.in])
AH_BOTTOM([#include "custom.h"])

AC_CONFIG_FILES(Makefile
	awklib/Makefile
	doc/Makefile
	extension/Makefile
	po/Makefile.in
	test/Makefile
	packaging/Makefile
	packaging/xgawk.spec
	[version.c:version.in])

dnl This is a significant and rather ugly hack.  We want to keep
dnl version.c from being removed upon `make distclean'.
dnl We put the `$$' on the front for old systems with 14-char filenames.

dnl At this point, `configure' has finished creating `config.status' which
dnl actually does all the checking and configuring. `config.status' is run
dnl by the AC_OUTPUT macro below.  This step comes in between the two: it
dnl adds code to `config.status'.  Doing it there ensures that this step
dnl happens *every* time `config.status' is run, such as when `make' decides
dnl to update something.  Fun, fun, fun.

dnl Be careful of multiple levels of shell quoting!

dnl The trailing newline in the sed command is needed for Mac OS X. Sigh.

AC_CONFIG_COMMANDS_POST([sed '$i\
	echo $as_me: fixing Makefile to keep version.c\
	sed "/CONFIG_CLEAN_FILES/s/version.c//" < Makefile > $$.Makefile\
	mv $$.Makefile Makefile
	' < $CONFIG_STATUS > $$.gawk-hack
	mv $$.gawk-hack $CONFIG_STATUS])


AC_OUTPUT
