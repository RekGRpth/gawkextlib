/*
 * timex - Time extensions:
 * strptime - Wrapper around C library strptime(3) function.
 * sleep - Wrapper around C sleep function.
 * gettimeofdate - Wrapper around gettimeofday.
 *
 * Arnold Robbins
 */

/*
 * Copyright (C) 2012, 2013, 2014, 2018
 * the Free Software Foundation, Inc.
 *
 * Copyright (C) 2019 Arnold David Robbins.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _XOPEN_SOURCE	1
#define GAWKEXTLIB_NOT_NEEDED
#include "common.h"

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <time.h>
#if defined(HAVE_GETTIMEOFDAY) && defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif
#if defined(HAVE_SELECT) && defined(HAVE_SYS_SELECT_H)
#include <sys/select.h>
#endif
#if defined(HAVE_GETSYSTEMTIMEASFILETIME)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __VMS
#define HAVE_NANOSLEEP
#define HAVE_GETTIMEOFDAY
#ifdef gettimeofday
#undef gettimeofday
#endif
#ifdef __ia64__
/* nanosleep not working correctly on IA64 */
static int
vms_fake_nanosleep(struct timespec *rqdly, struct timespec *rmdly)
{
	int result;
	struct timespec mtime1, mtime2;

	result = nanosleep(rqdly, &mtime1);
	if (result == 0)
		return 0;

	/* On IA64 it returns 100 nanoseconds early with an error */
	if ((mtime1.tv_sec == 0) && (mtime1.tv_nsec <= 100)) {
		mtime1.tv_nsec += 100;
		result = nanosleep(&mtime1, &mtime2);
		if (result == 0)
			return 0;
		if ((mtime2.tv_sec == 0) && (mtime2.tv_nsec <= 100)) {
			return 0;
		}
	}
	return result;
}
#define nanosleep(x,y) vms_fake_nanosleep(x, y)
#endif
#endif

/*
 * Returns time since 1/1/1970 UTC as a floating point value; should
 * have sub-second precision, but the actual precision will vary based
 * on the platform
 */
static awk_value_t *
do_gettimeofday(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	double curtime;

	assert(result != NULL);

#if defined(HAVE_GETTIMEOFDAY)
	{
		struct timeval tv;
		gettimeofday(&tv,NULL);
		curtime = tv.tv_sec+(tv.tv_usec/1000000.0);
	}
#elif defined(HAVE_GETSYSTEMTIMEASFILETIME)
	/* based on perl win32/win32.c:win32_gettimeofday() implementation */
	{
		union {
			unsigned __int64 ft_i64;
			FILETIME ft_val;
		} ft;

		/* # of 100-nanosecond intervals since January 1, 1601 (UTC) */
		GetSystemTimeAsFileTime(&ft.ft_val);
#ifdef __GNUC__
#define Const64(x) x##LL
#else
#define Const64(x) x##i64
#endif
/* Number of 100 nanosecond units from 1/1/1601 to 1/1/1970 */
#define EPOCH_BIAS  Const64(116444736000000000)
		curtime = (ft.ft_i64 - EPOCH_BIAS)/10000000.0;
#undef Const64
	}
#else
	/* no way to retrieve system time on this platform */
	curtime = -1;
	update_ERRNO_string(_("gettimeofday: not supported on this platform"));
#endif

	return make_number(curtime, result);
}

/*
 * Returns 0 if successful in sleeping the requested time;
 * returns -1 if there is no platform support, or if the sleep request
 * did not complete successfully (perhaps interrupted)
 */
static awk_value_t *
do_sleep(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t num;
	double secs;
	int rc;

	assert(result != NULL);

	if (! get_argument(0, AWK_NUMBER, &num)) {
		update_ERRNO_string(_("sleep: missing required numeric argument"));
		return make_number(-1, result);
	}
	secs = num.num_value;

	if (secs < 0) {
		update_ERRNO_string(_("sleep: argument is negative"));
		return make_number(-1, result);
	}

#if defined(HAVE_NANOSLEEP)
	{
		struct timespec req;

		req.tv_sec = secs;
		req.tv_nsec = (secs-(double)req.tv_sec)*1000000000.0;
		if ((rc = nanosleep(&req,NULL)) < 0)
			/* probably interrupted */
			update_ERRNO_int(errno);
	}
#elif defined(HAVE_SELECT)
	{
		struct timeval timeout;

		timeout.tv_sec = secs;
		timeout.tv_usec = (secs-(double)timeout.tv_sec)*1000000.0;
		if ((rc = select(0,NULL,NULL,NULL,&timeout)) < 0)
			/* probably interrupted */
			update_ERRNO_int(errno);
	}
#elif defined(HAVE_GETSYSTEMTIMEASFILETIME)
	{
		DWORD milliseconds = secs * 1000;

		Sleep (milliseconds);
		rc = 0;
	}
#else
	/* no way to sleep on this platform */
	rc = -1;
	update_ERRNO_string(_("sleep: not supported on this platform"));
#endif

	return make_number(rc, result);
}

/*  do_strptime --- call strptime */

static awk_value_t *
do_strptime(int nargs, awk_value_t *result API_FINFO_ARG)
{
	awk_value_t string, format;

	assert(result != NULL);
	make_number(0.0, result);

#if gawk_api_major_version < 2
	if (do_lint && nargs > 2)
		lintwarn(ext_id, _("strptime: called with too many arguments"));
#endif
	if (do_lint) {
		if (nargs == 0) {
			lintwarn(ext_id, _("strptime: called with no arguments"));
			make_number(-1.0, result);
			goto done0;
		}
	}

	/* string is first arg, format is second arg */
	if (! get_argument(0, AWK_STRING, & string)) {
		fprintf(stderr, _("do_strptime: argument 1 is not a string\n"));
		errno = EINVAL;
		goto done1;
	}
	if (! get_argument(1, AWK_STRING, & format)) {
		fprintf(stderr, _("do_strptime: argument 2 is not a string\n"));
		errno = EINVAL;
		goto done1;
	}

	struct tm broken_time;
	memset(& broken_time, 0, sizeof(broken_time));
	broken_time.tm_isdst = -1;
	if (strptime(string.str_value.str, format.str_value.str, & broken_time) == NULL) {
		make_number(-1.0, result);
		goto done0;
	}

	time_t epoch_time = mktime(& broken_time);
	make_number((double) epoch_time, result);
	goto done0;

done1:
	update_ERRNO_int(errno);

done0:
	return result;
}

/*
 * The third argument to API_FUNC_MAXMIN is the maximum number of expected
 * arguments, and the fourth is the minimum number of required arguments.
 * If they are the same, you may use the API_FUNC macro instead, which takes
 * only three arguments. Note that in version 1 of the API, the third value
 * in the awk_ext_func_t struct called num_expected_args is actually the
 * maximum number of allowed arguments.
 */
static awk_ext_func_t func_table[] = {
	API_FUNC_MAXMIN("gettimeofday", do_gettimeofday, 0, 0)
	API_FUNC_MAXMIN("sleep", do_sleep, 1, 1)
	API_FUNC_MAXMIN("strptime", do_strptime, 2, 2)
};

static awk_bool_t
init_my_module(void)
{
  GAWKEXTLIB_COMMON_INIT
  return awk_true;
}

static awk_bool_t (*init_func)(void) = init_my_module;
static const char *ext_version = PACKAGE_STRING;

dl_load_func(func_table, timex, "")
