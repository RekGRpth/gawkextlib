/*
 * strptime.c - Wrapper around C library strptime(3) function.
 *
 * Arnold Robbins
 * October 2019
 */

/*
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

#define _XOPEN_SOURCE	1
#define GAWKEXTLIB_NOT_NEEDED
#include "common.h"

#include <assert.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

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

dl_load_func(func_table, strptime, "")
