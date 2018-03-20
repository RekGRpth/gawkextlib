/*
 * csv.c - Builtin facilities to handle CSV data.
 */

/*
 * Copyright (C) 2018 the Free Software Foundation, Inc.
 *
 * This file is part of GAWK-CSV, the GAWK extension for handling CSV data.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1335, USA
 */

#include "common.h"
#include "csv_convert.h"
#include "csv_split.h"
#include "strbuf.h"

/*  get a single char argument */
static char
get_char_argument(int k, int nargs, char defval, const char* funcname)
{
	if (nargs > k) {
		awk_value_t arg;
		if (get_argument(k, AWK_STRING, & arg) && arg.str_value.len >0) {
			if (do_lint && arg.str_value.len > 1) {
				lintwarn(ext_id, _("%s: argument %d <%s> truncated to a single char"), funcname, k+1, arg.str_value.str);
			}
			return arg.str_value.str[0];
		} else if (do_lint) {
			lintwarn(ext_id, _("%s: wrong argument %d"), funcname, k+1);
		}
	}
	return defval;
}

/*  do_csvconvert --- convert a csv record to a plain text record with fixed field separators */

static awk_value_t *
do_csvconvert(int nargs, awk_value_t *result API_FINFO_ARG)
{
	awk_value_t csv_record;
	awk_value_t arg;
	const char* csvfs = "\0";
	char csvcomma;
	char csvquote;
#if gawk_api_major_version < 2
	check_nargs("csvconvert", 1, 4)
#endif
	if (!get_argument(0, AWK_STRING, & csv_record)) {
		if (do_lint) lintwarn(ext_id, _("%s: wrong argument %d"), "csvconvert", 1);
		return make_null_string(result);
	}
	if (nargs>1) {
		if (get_argument(1, AWK_STRING, & arg)) {
			csvfs = arg.str_value.str;
		} else {
			lintwarn(ext_id, _("%s: wrong argument %d"), "csvconvert", 2);
		}
	}
	csvcomma = get_char_argument(2, nargs, ',', "csvconvert");
	csvquote = get_char_argument(3, nargs, '"', "csvconvert");

	strbuf_p record = csv_convert_record(csv_record.str_value.str, csvfs, csvcomma, csvquote, '\0');
	return make_const_string(strbuf_value(record), record->length, result);
}

/*  do_csvsplit --- split a csv record into an array of fields */

static awk_value_t *
do_csvsplit(int nargs, awk_value_t *result API_FINFO_ARG)
{
	awk_value_t csv_record;
	awk_value_t fields;
	char csvcomma;
	char csvquote;
#if gawk_api_major_version < 2
	check_nargs("csvsplit", 2, 4)
#endif
	if (!get_argument(0, AWK_STRING, & csv_record)) {
		if (do_lint) lintwarn(ext_id, _("%s: wrong argument %d"), "csvsplit", 1);
		return make_number(-1, result);
	}
	if (!get_argument(1, AWK_ARRAY, & fields)) {
		fatal(ext_id, _("%s: argument %d must be an array"), "csvsplit", 2);
	}
	csvcomma = get_char_argument(2, nargs, ',', "csvsplit");
	csvquote = get_char_argument(3, nargs, '"', "csvsplit");

	clear_array(fields.array_cookie);
	int nfields = csv_split_record(csv_record.str_value.str, fields.array_cookie, csvcomma, csvquote, '\0');
	return make_number(nfields, result);
}

static awk_ext_func_t func_table[] = {
	API_FUNC_MAXMIN("csvconvert", do_csvconvert, 4, 1)
	API_FUNC_MAXMIN("csvsplit", do_csvsplit, 4, 2)
};

static awk_bool_t
init_my_module(void)
{
	GAWKEXTLIB_COMMON_INIT
	return awk_true;
}

static awk_bool_t (*init_func)(void) = init_my_module;
static const char *ext_version = PACKAGE_STRING;

dl_load_func(func_table, csv, "")
