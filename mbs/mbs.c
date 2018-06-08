/*
 * mbs.c - Extension functions for working with multibyte strings.
 */

/*
 * Copyright (C) 2018 the Free Software Foundation, Inc.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1335, USA
 */

#define GAWKEXTLIB_NOT_NEEDED
#include "common.h"

#include <stdbool.h>
#define __USE_XOPEN 1	/* at least for linux */
#include <wchar.h>

/*  do_mbs_length --- return length in bytes */

static awk_value_t *
do_mbs_length(int nargs, awk_value_t *result API_FINFO_ARG)
{
	awk_value_t x;

	// checking
	if (do_lint) {
		if (nargs == 0) {
			lintwarn(ext_id, _("mbs_length: called with no arguments"));
			return make_number(-1.0, result);
		} else if (nargs > 1)
			lintwarn(ext_id, _("mbs_length: called with too many arguments"));
	}

	// get argument(s)
	if (! get_argument(0, AWK_STRING, & x)) {
		lintwarn(ext_id, _("mbs_length: called with inappropriate argument(s)"));
		return make_number(-1.0, result);
	}

	// computation
	return make_number((double) x.str_value.len, result);
}

/*  do_mbs_split --- split strings into array of byte values */

static awk_value_t *
do_mbs_split(int nargs, awk_value_t *result API_FINFO_ARG)
{
	awk_value_t string_param, array_param;
	awk_array_t array;
	int i, len;

	// checking
	if (do_lint) {
		if (nargs == 0) {
			lintwarn(ext_id, _("mbs_split: called with no arguments"));
			return make_number(-1.0, result);
		} else if (nargs > 2)
			lintwarn(ext_id, _("mbs_split: called with too many arguments"));
	}

	// get argument(s)
	/* string is first arg, array to hold results is second */
	if (   ! get_argument(0, AWK_STRING, & string_param)
	    || ! get_argument(1, AWK_ARRAY, & array_param)) {
		warning(ext_id, _("mbs_split: bad parameters"));
		return make_number(-1.0, result);
	}

	// computation

	array = array_param.array_cookie;

	// always empty out the array
	clear_array(array);

	len = string_param.str_value.len;
	for (i = 0; i < len; i++) {
		char buf[200];
		awk_value_t index;
		awk_value_t value;

		sprintf(buf, "%d", i+1);
		make_number((double) (((int) string_param.str_value.str[i]) & 0xff), & value);
		set_array_element(array,
				make_const_string(buf, strlen(buf), & index),
				& value);
	}

	return make_number((double) len, result);
}

/*  do_mbs_join --- join array of byte values into string */

static awk_value_t *
do_mbs_join(int nargs, awk_value_t *result API_FINFO_ARG)
{
	awk_value_t array_param;
	awk_array_t array;
	size_t count = 0;
	char *buffer;
	size_t i;
	int j;

	// checking
	if (do_lint) {
		if (nargs == 0) {
			lintwarn(ext_id, _("mbs_join: called with no arguments"));
			return make_const_string("", 0, result);
		} else if (nargs > 1)
			lintwarn(ext_id, _("mbs_join: called with too many arguments"));
	}

	// get argument(s)
	if (! get_argument(0, AWK_ARRAY, & array_param)) {
		warning(ext_id, _("mbs_join: bad parameter(s)"));
		return make_number(-1.0, result);
	}

	// computation
	array = array_param.array_cookie;

	if (! get_element_count(array, & count) || count == 0)
		return make_const_string("", 0, result);

	buffer = (char *) gawk_malloc(count + 1);
	if (buffer == NULL)
		return make_const_string("", 0, result);

	j = 0;
	for (i = 1; i <= count; i++) {
		awk_value_t index, value;

		make_number((double) i, & index);
		if (! get_array_element(array, & index, AWK_NUMBER, & value))
			continue;
		buffer[j++] = ((int) value.num_value) & 0xFF;
	}
	buffer[j] = '\0';

	return make_malloced_string(buffer, j, result);
}

/*  do_mbs_wcswidth --- do the same as the C wcswidth(3) function */

static awk_value_t *
do_mbs_wcswidth(int nargs, awk_value_t *result API_FINFO_ARG)
{
	awk_value_t string;
	awk_value_t not_utf;
	bool using_utf = true;
	mbstate_t mbs;
	wchar_t wc;
	size_t src_count, count, column_count;
	int i, width;
	char *sp;

	// checking
	if (do_lint) {
		if (nargs == 0) {
			lintwarn(ext_id, _("mbs_wcswidth: called with no arguments"));
			return make_number(-1.0, result);
		} else if (nargs > 2)
			lintwarn(ext_id, _("mbs_wcswidth: called with too many arguments"));
	}

	// get argument(s)
	if (   ! get_argument(0, AWK_STRING, & string)) {
		warning(ext_id, _("mbs_wcswidth: called with inappropriate argument(s)"));
		return make_number(-1.0, result);
	}

	if (nargs > 1) {
		if (get_argument(1, AWK_NUMBER, & not_utf)) {
			lintwarn(ext_id, _("mbs_wcswidth: called with inappropriate argument(s)"));
			return make_number(-1.0, result);
		}
		else
			using_utf = (not_utf.num_value == 0);
	}

	// computation
	if (string.str_value.len == 0)
		return make_number(0.0, result);

	// FIXME: Gaping API hole here. Gawk knows how to convert mbs strings
	// to wchar_t strings. If we could just let it do that for us, we
	// could loop over the string and collect the count. Instead, we have
	// to duplicate some of that code. An upside to doing it here is that
	// we don't allocate the wide character string since we don't need it.

	sp = string.str_value.str;
	src_count = string.str_value.len;
	memset(& mbs, 0, sizeof(mbs));
	column_count = 0;

	for (i = 0; src_count > 0; i++) {
		count = mbrtowc(& wc, sp, src_count, & mbs);
		switch (count) {
		case (size_t) -2:
		case (size_t) -1:
			/*
			 * mbrtowc(3) says the state of mbs becomes undefined
			 * after a bad character, so reset it.
			 */
			memset(& mbs, 0, sizeof(mbs));

			/*
			 * If we're using UTF, then instead of just
			 * skipping the character, plug in the Unicode
			 * replacement character. In most cases this gives
			 * us "better" results, in that character counts
			 * and string lengths tend to make more sense.
			 *
			 * Otherwise, just skip the bad byte and keep going,
			 * so that we get a more-or-less full string, instead
			 * of stopping early.
			 */
			if (using_utf) {
				count = 1;
				wc = 0xFFFD;	/* unicode replacement character */
				goto set_wc;
			} else {
				/* skip it and keep going */
				sp++;
				src_count--;
			}
			break;

		case 0:
			count = 1;
			/* fall through */
		default:
		set_wc:
			width = wcwidth(wc);
			if (width > 0)
				column_count += width;
			src_count -= count;
			sp += count;
			break;
		}
	}

	return make_number((double) column_count, result);
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
	API_FUNC("mbs_length", do_mbs_length, 1)
	API_FUNC("mbs_split", do_mbs_split, 2)
	API_FUNC("mbs_join", do_mbs_join, 1)
	API_FUNC_MAXMIN("mbs_wcswidth", do_mbs_wcswidth, 2, 1)
};

static awk_bool_t
init_my_module(void)
{
  GAWKEXTLIB_COMMON_INIT
  return awk_true;
}

static awk_bool_t (*init_func)(void) = init_my_module;
static const char *ext_version = PACKAGE_STRING;

dl_load_func(func_table, mbs, "")
