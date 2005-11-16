/*
 * time.c - Builtin functions that provide time-related functions.
 *
 */

/*
 * Copyright (C) 2001, 2004 the Free Software Foundation, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "awk.h"
#if defined(HAVE_GETTIMEOFDAY) && defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif
#if defined(HAVE_SELECT) && defined(HAVE_SYS_SELECT_H)
#include <sys/select.h>
#endif

#define RETURN return tmp_number((AWKNUM) 0)

/* returns time since 1/1/1970 UTC as a floating point value; should
   have sub-second precision, but the actual precision will vary based
   on the platform */
static NODE *
do_gettimeofday(NODE *tree ATTRIBUTE_UNUSED)
{
	double curtime;

	if  (do_lint && get_curfunc_arg_count() > 0)
		lintwarn("gettimeofday: ignoring arguments");

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
	set_ERRNO("gettimeofday: not supported on this platform");
#endif

	set_value(tmp_number((AWKNUM) curtime));
	/* Just to make the interpreter happy */
	RETURN;
}

/* returns 0 if successful in sleeping the requested time;
   returns -1 if there is no platform support, or if the sleep request
      did not complete successfully (perhaps interrupted)
*/
static NODE *
do_sleep(NODE *tree)
{
	NODE *arg;
	double secs;
	int rc;

	if  (do_lint && get_curfunc_arg_count() > 1)
		lintwarn("sleep: called with too many arguments");

	arg = get_scalar_argument(tree, 0, FALSE);
	secs = force_number(arg);
	free_temp(arg);

	if (secs < 0) {
		set_value(tmp_number((AWKNUM)-1));
		set_ERRNO("sleep: argument is negative");
		RETURN;
	}

#if defined(HAVE_NANOSLEEP)
	{
		struct timespec req;

		req.tv_sec = secs;
		req.tv_nsec = (secs-(double)req.tv_sec)*1000000000.0;
		if ((rc = nanosleep(&req,NULL)) < 0)
			/* probably interrupted */
			update_ERRNO();
	}
#elif defined(HAVE_SELECT)
	{
		struct timeval timeout;

		timeout.tv_sec = secs;
		timeout.tv_usec = (secs-(double)timeout.tv_sec)*1000000.0;
		if ((rc = select(0,NULL,NULL,NULL,&timeout)) < 0)
			/* probably interrupted */
			update_ERRNO();
	}
#else
	/* no way to sleep on this platform */
	rc = -1;
	set_ERRNO("sleep: not supported on this platform");
#endif

	set_value(tmp_number((AWKNUM) rc));
	/* Just to make the interpreter happy */
	RETURN;
}


/* dlload --- load new builtins in this library */

#ifdef BUILD_STATIC_EXTENSIONS
#define dlload dlload_time
#endif

NODE *
dlload(NODE *tree ATTRIBUTE_UNUSED, void *dl ATTRIBUTE_UNUSED)
{
	make_builtin("gettimeofday", do_gettimeofday, 0);
	make_builtin("sleep", do_sleep, 1);
	RETURN;
}
