#ifndef CONFIG_H
#define CONFIG_H
/*
 * config.h -- configuration definitions for gawk.
 *
 * For VMS (assumes V4.6 or later; tested on V5.5-2 and V7.1)
 */

/* 
 * Copyright (C) 1991-1992, 1995-1996, 1999, 2001, 2002, 2003, 2005 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/* switch statements are enabled in awk programs */
#undef ALLOW_SWITCH

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
#undef CRAY_STACKSEG_END

#if 0
/* Define to 1 if using alloca.c.  */
#define C_ALLOCA 1
#else
#define NO_ALLOCA	/* vms/vms_fwrite.c needs this */
#endif

/* dynamic loading is possible */
#undef DYNAMIC

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#undef ENABLE_NLS

/* Define to the type of elements in the array set by `getgroups'. Usually
   this is either `int' or `gid_t'. */
#define GETGROUPS_T int

/* Define to 1 if the `getpgrp' function requires zero arguments. */
#define GETPGRP_VOID	1

/* Define to 1 if you have the `alarm' function. */
#define HAVE_ALARM 1

/* Define to 1 if you have `alloca', as a function or macro. */
#undef HAVE_ALLOCA

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
   */
#undef HAVE_ALLOCA_H

/* Define to 1 if you have the <argz.h> header file. */
#undef HAVE_ARGZ_H

/* Define to 1 if you have the `asprintf' function. */
#undef HAVE_ASPRINTF

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
#undef HAVE_DCGETTEXT

/* Define to 1 if you have the declaration of `feof_unlocked', and to 0 if you
   don't. */
#define HAVE_DECL_FEOF_UNLOCKED 0

/* Define to 1 if you have the declaration of `fgets_unlocked', and to 0 if
   you don't. */
#define HAVE_DECL_FGETS_UNLOCKED 0

/* Define to 1 if you have the declaration of `getc_unlocked', and to 0 if you
   don't. */
#define HAVE_DECL_GETC_UNLOCKED 0

/* Define to 1 if you have the declaration of `_snprintf', and to 0 if you
   don't. */
#define HAVE_DECL__SNPRINTF 0

/* Define to 1 if you have the declaration of `_snwprintf', and to 0 if you
   don't. */
#define HAVE_DECL__SNWPRINTF 0

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
#undef HAVE_DOPRNT

/* Define to 1 if you have the <fcntl.h> header file. */
#undef HAVE_FCNTL_H

/* Define to 1 if you have the `fmod' function. */
#define HAVE_FMOD 1

/* Define to 1 if you have the `fwprintf' function. */
#undef HAVE_FWPRINTF

/* Define to 1 if you have the `getcwd' function. */
#define HAVE_GETCWD 1

/* Define to 1 if you have the `getegid' function. */
#undef HAVE_GETEGID

/* Define to 1 if you have the `geteuid' function. */
#define HAVE_GETEUID 1

/* Define to 1 if you have the `getgid' function. */
#define HAVE_GETGID 1

/* Define to 1 if you have the `getgrent' function. */
#undef HAVE_GETGRENT

/* Define to 1 if you have the `getgroups' function. */
#undef HAVE_GETGROUPS

/* Define if you have the getpagesize function.  */
#undef HAVE_GETPAGESIZE

/* Define if the GNU gettext() function is already present or preinstalled. */
#undef HAVE_GETTEXT

/* Define to 1 if you have the `getuid' function. */
#define HAVE_GETUID 1

/* Define to 1 if you have the `grantpt' function. */
#undef HAVE_GRANTPT

/* Define if you have the iconv() function. */
#undef HAVE_ICONV

/* Define if you have the 'intmax_t' type in <stdint.h> or <inttypes.h>. */
#undef HAVE_INTMAX_T

/* Define if <inttypes.h> exists and doesn't clash with <sys/types.h>. */
#undef HAVE_INTTYPES_H

/* Define if <inttypes.h> exists, doesn't clash with <sys/types.h>, and
   declares uintmax_t. */
#undef HAVE_INTTYPES_H_WITH_UINTMAX

/* Define to 1 if you have the `iswctype' function. */
#undef HAVE_ISWCTYPE

/* Define to 1 if you have the `iswlower' function. */
#undef HAVE_ISWLOWER

/* Define to 1 if you have the `iswupper' function. */
#undef HAVE_ISWUPPER

/* Define if you have <langinfo.h> and nl_langinfo(CODESET). */
#undef HAVE_LANGINFO_CODESET

/* Define if your <locale.h> file defines LC_MESSAGES. */
#undef HAVE_LC_MESSAGES

/* Define to 1 if you have the `dl' library (-ldl). */
#undef HAVE_LIBDL

/* Define to 1 if you have the <libintl.h> header file. */
#undef HAVE_LIBINTL_H

/* Define to 1 if you have the `m' library (-lm). */
#undef HAVE_LIBM

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <locale.h> header file. */
#undef HAVE_LOCALE_H

/* Define if you have the 'long double' type. */
#undef HAVE_LONG_DOUBLE

/* Define if you have the 'long long' type. */
#undef HAVE_LONG_LONG

/* Define to 1 if you have the <malloc.h> header file. */
#undef HAVE_MALLOC_H

/* Define to 1 if you have the `mbrlen' function. */
#undef HAVE_MBRLEN

/* Define to 1 if mbrtowc and mbstate_t are properly declared. */
#undef HAVE_MBRTOWC

/* Define to 1 if you have the <mcheck.h> header file. */
#undef HAVE_MCHECK_H

/* Define to 1 if you have the `memcmp' function. */
#define HAVE_MEMCMP 1

/* Define to 1 if you have the `memcpy' function. */
#define HAVE_MEMCPY 1

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#undef HAVE_MEMORY_H

/* Define to 1 if you have the `mempcpy' function. */
#undef HAVE_MEMPCPY

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* we have the mktime function */
#define HAVE_MKTIME 1

/* Define to 1 if you have a working `mmap' system call. */
#undef HAVE_MMAP

/* Define to 1 if you have the `munmap' function. */
#undef HAVE_MUNMAP

/* Define to 1 if you have the <netdb.h> header file. */
#undef HAVE_NETDB_H

/* Define to 1 if you have the <netinet/in.h> header file. */
#undef HAVE_NETINET_IN_H

/* Define to 1 if you have the <nl_types.h> header file. */
#undef HAVE_NL_TYPES_H

/* we have portals on /p on this system */
#undef HAVE_PORTALS

/* Define if your printf() function supports format strings with positions. */
#undef HAVE_POSIX_PRINTF

/* Define to 1 if you have the `putenv' function. */
#undef HAVE_PUTENV

/* Define to 1 if you have the `setenv' function. */
#undef HAVE_SETENV

/* Define to 1 if you have the `setlocale' function. */
#undef HAVE_SETLOCALE

/* Define to 1 if you have the <signum.h> header file. */
#undef HAVE_SIGNUM_H

/* Define to 1 if you have the `snprintf' function. */
#undef HAVE_SNPRINTF

/* we have sockets on this system */
#undef HAVE_SOCKETS

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define if <stdint.h> exists, doesn't clash with <sys/types.h>, and declares
   uintmax_t. */
#undef HAVE_STDINT_H_WITH_UINTMAX

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `stpcpy' function. */
#undef HAVE_STPCPY

/* Define to 1 if you have the `strcasecmp' function. */
#undef HAVE_STRCASECMP

/* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR 1

/* Define to 1 if you have the `strdup' function. */
#undef HAVE_STRDUP

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the `strftime' function. */
#undef HAVE_STRFTIME		/* use the missing_d/strfime.c version */

/* Define to 1 if cpp supports the ANSI # stringizing operator. */
#ifdef VAXC
#undef HAVE_STRINGIZE
#else
#define HAVE_STRINGIZE 1
#endif

/* Define to 1 if you have the <strings.h> header file. */
#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strncasecmp' function. */
#undef HAVE_STRNCASECMP

/* Define to 1 if you have the <stropts.h> header file. */
#undef HAVE_STROPTS_H

/* Define to 1 if you have the `strtod' function. */
#define HAVE_STRTOD 1

/* Define to 1 if you have the `strtoul' function. */
#define HAVE_STRTOUL 1

/* Define to 1 if `st_blksize' is member of `struct stat'. */
#undef HAVE_STRUCT_STAT_ST_BLKSIZE

/* Define to 1 if `tm_zone' is member of `struct tm'. */
#undef HAVE_STRUCT_TM_TM_ZONE

/* Define to 1 if your `struct stat' has `st_blksize'. Deprecated, use
   `HAVE_STRUCT_STAT_ST_BLKSIZE' instead. */
#undef HAVE_ST_BLKSIZE

/* Define to 1 if you have the `system' function. */
#define HAVE_SYSTEM 1

/* Define to 1 if you have the <sys/param.h> header file. */
#undef HAVE_SYS_PARAM_H

/* Define to 1 if you have the <sys/socket.h> header file. */
#undef HAVE_SYS_SOCKET_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#undef HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/time.h> header file. */
#undef HAVE_SYS_TIME_H

/* Define to 1 if you have the <sys/types.h> header file. */
#undef HAVE_SYS_TYPES_H

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#undef HAVE_SYS_WAIT_H

/* Define to 1 if you have the <termios.h> header file. */
#undef HAVE_TERMIOS_H

/* Define to 1 if your `struct tm' has `tm_zone'. Deprecated, use
   `HAVE_STRUCT_TM_TM_ZONE' instead. */
#undef HAVE_TM_ZONE

/* Define to 1 if you have the `towlower' function. */
#undef HAVE_TOWLOWER

/* Define to 1 if you have the `towupper' function. */
#undef HAVE_TOWUPPER

/* Define to 1 if you have the `tsearch' function. */
#undef HAVE_TSEARCH

/* Define to 1 if you don't have `tm_zone' but do have the external array
   `tzname'. */
#define HAVE_TZNAME 1		/* (faked in vms/vms_misc.c) */

/* Define to 1 if you have the `tzset' function. */
#define HAVE_TZSET 1		/* (faked in vms/vms_misc.c) */

/* Define if you have the 'uintmax_t' type in <stdint.h> or <inttypes.h>. */
#undef HAVE_UINTMAX_T

/* Define to 1 if you have the <unistd.h> header file. */
#ifdef __DECC
#define HAVE_UNISTD_H 1
#else
#undef HAVE_UNISTD_H
#endif

/* Define if you have the unsigned long long type. */
#undef HAVE_UNSIGNED_LONG_LONG

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define to 1 if you have the <wchar.h> header file. */
#undef HAVE_WCHAR_H

/* Define if you have the 'wchar_t' type. */
#undef HAVE_WCHAR_T

/* Define to 1 if you have the `wcrtomb' function. */
#undef HAVE_WCRTOMB

/* Define to 1 if you have the `wcscoll' function. */
#undef HAVE_WCSCOLL

/* Define to 1 if you have the `wcslen' function. */
#undef HAVE_WCSLEN

/* Define to 1 if you have the `wctype' function. */
#undef HAVE_WCTYPE

/* Define to 1 if you have the <wctype.h> header file. */
#undef HAVE_WCTYPE_H

/* systems should define this type here */
#undef HAVE_WCTYPE_T

/* systems should define this type here */
#undef HAVE_WINT_T

/* Define to 1 if you have the `__argz_count' function. */
#undef HAVE___ARGZ_COUNT

/* Define to 1 if you have the `__argz_next' function. */
#undef HAVE___ARGZ_NEXT

/* Define to 1 if you have the `__argz_stringify' function. */
#undef HAVE___ARGZ_STRINGIFY

/* Define to 1 if you have the `__fsetlocking' function. */
#undef HAVE___FSETLOCKING

/* Define as const if the declaration of iconv() needs const. */
#undef ICONV_CONST

/* Define if integer division by zero raises signal SIGFPE. */
#define INTDIV0_RAISES_SIGFPE 1

/* disable lint checks */
#undef NO_LINT

/* Name of package */
#define PACKAGE "gawk"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "bug-gawk@gnu.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "GNU Awk"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "GNU Awk 3.1.4c"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "gawk"

/* Define to the version of this package. */
#define PACKAGE_VERSION "3.1.4c"

/* Define to 1 if *printf supports %F format */
#undef PRINTF_HAS_F_FORMAT

/* Define if <inttypes.h> exists and defines unusable PRI* macros. */
#undef PRI_MACROS_BROKEN

/* Define if compiler has function prototypes */
#define PROTOTYPES 1

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* The size of a `unsigned int', as computed by sizeof. */
#define SIZEOF_UNSIGNED_INT 4

/* The size of a `unsigned long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG 4

/* Define as the maximum value of type 'size_t', if the system doesn't define
   it. */
#define SIZE_MAX 0xffffffffUL

/* return type of sprintf */
#define SPRINTF_RET int

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
        STACK_DIRECTION > 0 => grows toward higher addresses
        STACK_DIRECTION < 0 => grows toward lower addresses
        STACK_DIRECTION = 0 => direction of growth unknown */
#define STACK_DIRECTION (-1)

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* strtod doesn't have C89 semantics */
#define STRTOD_NOT_C89 1

/* some systems define this type here */
#undef TIME_T_IN_SYS_TYPES_H

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#undef TIME_WITH_SYS_TIME

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
#undef TM_IN_SYS_TIME

/* force use of our version of strftime */
#define USE_INCLUDED_STRFTIME 1

/* Version number of package */
#define VERSION "3.1.4"

/* Define to 1 if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
# undef _ALL_SOURCE
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#undef _FILE_OFFSET_BITS

/* Define for large files, on AIX-style hosts. */
#undef _LARGE_FILES

/* Define to 1 if on MINIX. */
#undef _MINIX

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
#undef _POSIX_1_SOURCE

/* Define to 1 if you need to in order for `stat' and other things to work. */
#undef _POSIX_SOURCE

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
# undef __CHAR_UNSIGNED__
#endif

/* Define like PROTOTYPES; this can be used by system headers. */
#define __PROTOTYPES 1

/* Define to empty if `const' does not conform to ANSI C. */
#undef const

/* Define to `int' if <sys/types.h> doesn't define. */
#undef gid_t

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#undef inline
#endif

/* Define to widest signed type if <inttypes.h> doesn't define. */
#define intmax_t long int

/* Define to `long' if <sys/types.h> doesn't define.  */
#undef off_t

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef pid_t

/* Define as the type of the result of subtracting two pointers, if the system
   doesn't define it. */
#undef ptrdiff_t

/* Define to empty if the C compiler doesn't support this keyword. */
#undef signed

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
#undef size_t

/* Define to `int' if <sys/types.h> does not define. */
#define ssize_t int

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef uid_t

/* Define to unsigned long or unsigned long long if <stdint.h> and
   <inttypes.h> don't define. */
#define uintmax_t unsigned long

#if 0
#include <custom.h>	/* overrides for stuff autoconf can't deal with */
#else

/* Whether `time_t' is an unsigned type. */
#define TIME_T_UNSIGNED 1

/*******************************/
/* Gawk configuration options. */
/*******************************/

#define ALLOW_SWITCH 1

/*
 * DEFPATH
 *	VMS: "/AWK_LIBRARY" => "AWK_LIBRARY:"
 * The default search path for the -f option of gawk.  It is used
 * if the AWKPATH environment variable is undefined.
 *
 * Note: OK even if no AWK_LIBRARY logical name has been defined.
 */

#define DEFPATH	".,/AWK_LIBRARY"
#define ENVSEP	','

/*
 * Extended source file access.
 */
#define DEFAULT_FILETYPE ".awk"

/*
 * Pipe handling.
 */
#define PIPES_SIMULATED	1

/*
 * VAXCRTL is pre-ANSI and does some variations of numeric formatting
 * differently than gawk expects.
 */
#if defined(VAX) && !defined(__DECC)
/* '0' format modifier for %e,%f,%g gives wrong results in many cases */
#define VAXCRTL
/* %g format chooses %e format when should use %f */
#define GFMT_WORKAROUND	1
#endif

/*
 * VAX C
 *
 * As of V3.2, VAX C is not yet ANSI-compliant.  But it's close enough
 * for GAWK's purposes.  Comment this out for VAX C V2.4 and earlier.
 * YYDEBUG definition is needed for combination of VAX C V2.x and Bison.
 */
#if defined(VAXC) && !defined(__STDC__)
#define __STDC__	0
#define NO_TOKEN_PASTING
#define signed  /*empty*/
#define inline	/*empty*/
#ifndef __DECC	/* DEC C does not support #pragma builtins even in VAXC mode */
#define VAXC_BUILTINS
#endif
/* #define YYDEBUG 0 */
#define NO_MBSUPPORT  /* VAX C's preprocessor can't handle mbsupport.h */
#define RE_TOKEN_INIT_BUG     /* regcomp.c */
#endif

/*
 * DEC C
 *
 * Digital's ANSI complier.
 */
#ifdef __DECC
 /* DEC C implies DECC$SHR, which doesn't have the %g problem of VAXCRTL */
#undef GFMT_WORKAROUND
 /* DEC C V5.x introduces incompatibilities with prior porting efforts */
#define _DECC_V4_SOURCE
#define __SOCKET_TYPEDEFS
#if __VMS_VER >= 60200000
# undef __VMS_VER
# define __VMS_VER 60100000
#endif
#if __CRTL_VER >= 60200000
# if __CRTL_VER >= 70301000
#  define CRTL_VER_V731
# endif
# undef __CRTL_VER
# define __CRTL_VER 60100000
#endif
#if __DECC_VER >= 60400000 && !defined(DEBUG)
/* disable "new feature in C99" diagnostics (for regex code);
   NEWC99 ought to suffice but doesn't (at least in V6.4) */
#pragma message disable (NEWC99,DESIGNATORUSE)
#endif
#endif	/* __DECC */

/*
 * GNU C
 *
 * Versions of GCC (actually GAS) earlier than 1.38 don't produce the
 * right code for ``extern const'' constructs, and other usages of
 * const might not be right either.  The old set of include files from
 * the gcc-vms distribution did not contain prototypes, and this could
 * provoke some const-related compiler warnings.  If you've got an old
 * version of gcc for VMS, define 'const' out of existance, and by all
 * means obtain the most recent version!
 *
 * Note: old versions of GCC should also avoid defining STDC_HEADERS,
 *       because most of the ANSI-C required header files are missing.
 */
#ifdef __GNUC__
/* #define const */
/* #undef STDC_HEADERS */
#ifndef STDC_HEADERS
#define alloca __builtin_alloca
#define environ $$PsectAttributes_NOSHR$$environ	/* awful GAS kludge */
#endif
#undef  REGEX_MALLOC	/* use true alloca() in regex.c */
#endif

#define IN_CONFIG_H
#include "vms/redirect.h"
#undef  IN_CONFIG_H

#endif	/*<custom.h>*/

#endif	/*CONFIG_H*/
