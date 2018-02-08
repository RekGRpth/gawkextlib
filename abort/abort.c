/*
 * abort.c - Extensions function to exit immediately
 */

/*
 * Copyright (C) 2018 the Free Software Foundation, Inc.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1335, USA
 */

#include "common.h"

/*  do_abort --- call abort */

static awk_value_t *
do_abort(int nargs, awk_value_t *result API_FINFO_ARG)
{
	awk_value_t exit_code;

	if (do_lint && nargs > 1)
		lintwarn(ext_id, _("abort: called with too many arguments"));

	if (get_argument(0, AWK_NUMBER, & exit_code)) {
		char str[256];
		snprintf(str, sizeof(str), "%g", exit_code.num_value);
		exit((int) exit_code.num_value);
	} else
		exit(0);	// default value

	return make_null_string(result);
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
	API_FUNC_MAXMIN("abort", do_abort, 1, 0)
};

static awk_bool_t
init_my_module(void)
{
  GAWKEXTLIB_COMMON_INIT
  return awk_true;
}

static awk_bool_t (*init_func)(void) = init_my_module;
static const char *ext_version = PACKAGE_STRING;

dl_load_func(func_table, abort, "")
