/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */
/*
 * acconfig.h -- configuration definitions for gawk.
 */

/*
 * Copyright (C) 1995-2005 the Free Software Foundation, Inc.
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


/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define if using alloca.c.  */
/* #undef C_ALLOCA */

/* Define if type char is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* #undef __CHAR_UNSIGNED__ */
#endif

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
/* #undef CRAY_STACKSEG_END */

/* Define to the type of elements in the array set by `getgroups'.
   Usually this is either `int' or `gid_t'.  */
#define GETGROUPS_T gid_t

/* Define if the `getpgrp' function takes no argument.  */
#define GETPGRP_VOID 1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if you have alloca, as a function or macro.  */
#define HAVE_ALLOCA 1

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
/* #undef HAVE_ALLOCA_H */

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if you have a working `mmap' system call.  */
/* #undef HAVE_MMAP */

/* Define if your struct stat has st_blksize.  */
#define HAVE_ST_BLKSIZE 1

/* Define if you have the ANSI # stringizing operator in cpp. */
#define HAVE_STRINGIZE 1

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
/* #undef HAVE_SYS_WAIT_H */

/* Define if your struct tm has tm_zone.  */
/* #undef HAVE_TM_ZONE */

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
#define HAVE_TZNAME 1

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define as __inline if that's what the C compiler calls it.  */
#if defined (_MSC_VER)
#define inline
#endif

/* Define if on MINIX.  */
/* #undef _MINIX */

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
/* #undef _POSIX_1_SOURCE */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
 STACK_DIRECTION > 0 => grows toward higher addresses
 STACK_DIRECTION < 0 => grows toward lower addresses
 STACK_DIRECTION = 0 => direction of growth unknown
 */
/* #undef STACK_DIRECTION */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

#define REGEX_MALLOC 1 /* use malloc instead of alloca in regex.c */
#define SPRINTF_RET int /* return type of sprintf */
/* #undef HAVE_MKTIME */ /* we have the mktime function */
/* #undef HAVE_SOCKETS */ /* we have sockets on this system */
/* #undef HAVE_PORTALS */ /* we have portals on /p on this system */
/* #undef DYNAMIC */  /* allow dynamic addition of builtins */
/* #undef STRTOD_NOT_C89 */ /* strtod doesn't have C89 semantics */

/* Define if you have the __argz_count function.  */
#define HAVE___ARGZ_COUNT 1

/* Define if you have the __argz_next function.  */
#define HAVE___ARGZ_NEXT 1

/* Define if you have the __argz_stringify function.  */
#define HAVE___ARGZ_STRINGIFY 1

/* Define if you have the alarm function.  */
#define HAVE_ALARM 1

/* Define if you have the dcgettext function.  */
/* #undef HAVE_DCGETTEXT */

/* Define if you have the fmod function.  */
#define HAVE_FMOD 1

/* Define if you have the getcwd function.  */
/* #undef HAVE_GETCWD */

/* Define if you have the getpagesize function.  */
/* #undef HAVE_GETPAGESIZE */

/* Define if you have the memcmp function.  */
#define HAVE_MEMCMP 1

/* Define if you have the memcpy function.  */
#define HAVE_MEMCPY 1

/* Define if you have the memset function.  */
#define HAVE_MEMSET 1

/* Define if you have the munmap function.  */
/* #undef HAVE_MUNMAP */

/* Define if you have the putenv function.  */
/* #undef HAVE_PUTENV */

/* Define if you have the setenv function.  */
/* #define HAVE_SETENV */

/* Define if you have the setlocale function.  */
/* #undef HAVE_SETLOCALE */

/* Define if you have the stpcpy function.  */
/* #undef HAVE_STPCPY */

/* Define if you have the strcasecmp function.  */
/* #undef HAVE_STRCASECMP */

/* Define if you have the strchr function.  */
#define HAVE_STRCHR 1

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strftime function.  */
#define HAVE_STRFTIME 1

/* Define if you have the strncasecmp function.  */
#define HAVE_STRNCASECMP 1

/* Define if you have the strtod function.  */
#define HAVE_STRTOD 1

/* Define if you have the system function.  */
#define HAVE_SYSTEM 1

/* Define if you have the tzset function.  */
#define HAVE_TZSET 1

/* Define if you have the <argz.h> header file.  */
/* #undef HAVE_ARGZ_H */

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <libintl.h> header file.  */
/* #undef HAVE_LIBINTL_H */

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <locale.h> header file.  */
/* #undef HAVE_LOCALE_H */

/* Define if you have the <malloc.h> header file.  */
/* #undef HAVE_MALLOC_H */

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if you have the <netdb.h> header file.  */
/* #undef HAVE_NETDB_H */

/* Define if you have the <netinet/in.h> header file.  */
/* #undef HAVE_NETINET_IN_H */

/* Define if you have the <nl_types.h> header file.  */
/* #undef HAVE_NL_TYPES_H */

/* Define if you have the <signum.h> header file.  */
/* #undef HAVE_SIGNUM_H */

/* Define if you have the <stdarg.h> header file.  */
#define HAVE_STDARG_H 1

/* Define if you have the <stdlib.h> header file.  */
/* #undef HAVE_STDLIB_H */

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <strings.h> header file.  */
/* #undef HAVE_STRINGS_H */

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 1

/* Define if you have the <sys/socket.h> header file.  */
/* #undef HAVE_SYS_SOCKET_H */

/* Define if you have the <sys/time.h> header file.  */
#if defined(DJGPP)
# define HAVE_SYS_TIME_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <unistd.h> header file.  */
#if defined(DJGPP)
# define HAVE_UNISTD_H 1
#endif

/* Define if you have the i library (-li).  */
/* #undef HAVE_LIBI */

/* Define if you have the intl library (-lintl).  */
/* #undef HAVE_LIBINTL */

/* Define if you have the m library (-lm).  */
#define HAVE_LIBM 1

/* Name of package */
#define PACKAGE "gawk"

/* Version number of package */
#define VERSION "3.0.89"

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define to make ftello visible on some hosts (e.g. HP-UX 10.20). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to make ftello visible on some hosts (e.g. glibc 2.1.3). */
/* #undef _XOPEN_SOURCE */

/* Define if compiler has function prototypes */
#define PROTOTYPES 1

/* Define to 1 if you have the stpcpy function. */
/* #undef HAVE_STPCPY */

/* Define if your locale.h file contains LC_MESSAGES. */
/* #undef HAVE_LC_MESSAGES */

/* Define to 1 if NLS is requested. */
/* #undef ENABLE_NLS */

/* Define to 1 if you have gettext and don't want to use GNU gettext. */
/* #undef HAVE_GETTEXT */

/* Define as 1 if you have catgets and don't want to use GNU gettext. */
/* #undef HAVE_CATGETS */

/* The size of `unsigned int' & `unsigned long', as computed by sizeof. */
#if defined(DJGPP) || defined(_MSC_VER)
# include <limits.h>
#endif

#if UINT_MAX == 65536
# define SIZEOF_UNSIGNED_INT 2
#endif

#if UINT_MAX == 4294967295U
# define SIZEOF_UNSIGNED_INT 4
#endif

#if ULONG_MAX == 4294967295UL
# define SIZEOF_UNSIGNED_LONG 4
#endif

/* Library search path */
#define DEFPATH  ".;c:/lib/awk;c:/gnu/lib/awk"

#if defined (_MSC_VER)
#if !defined(__STDC__)
# define __STDC__ 1
#endif
#undef HAVE_UNISTD_H
#undef HAVE_SYS_PARAM_H
#undef HAVE_RANDOM
/* msc strftime is incomplete, use supplied version */
#undef HAVE_STRFTIME
/* #define HAVE_TM_ZONE */
#define altzone timezone
#endif

# define HAVE_POPEN_H

#if defined(_MSC_VER) && defined(MSDOS)
#define system(s) os_system(s)
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#define ssize_t long int /* DJGPP has ssize_t */
#define intmax_t long
#define uintmax_t unsigned long
#endif

#if defined (_MSC_VER) || defined(__EMX__)
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

#if defined(DJGPP)
# define HAVE_LIMITS_H 1
# undef HAVE_POPEN_H
#define intmax_t long long
#define uintmax_t unsigned long long
#endif

#if defined(__WIN32__) && defined(__CRTRSXNT__)
#include <crtrsxnt.h>
#endif

/* For vcWin32 */
#if defined(WIN32) && defined(_MSC_VER)
#define alloca _alloca
#define system(s) os_system(s)
#endif

#if defined(__MINGW32__)
#undef HAVE_SYS_PARAM_H
#endif


/* #define NO_LINT 1 */
