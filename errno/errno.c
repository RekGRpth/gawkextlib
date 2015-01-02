/*
 * errno.c - Builtin functions to map errno values.
 */

/*
 * Copyright (C) 2013 the Free Software Foundation, Inc.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "common.h"
#include <errno.h>

static const char *const errno2name[] = {
#define init_errno(A, B) [A] = B,
#include "errno2name.h"
#undef init_errno
};
#define NUMERR	sizeof(errno2name)/sizeof(errno2name[0])

static const struct {
   const char *name;
   int n;
} name2errno[] = {
#define init_errno(A, B) { B, A },
#include "name2errno.h"
#undef init_errno
};
#define NUM_NAME2ERR	sizeof(name2errno)/sizeof(name2errno[0])

/*  do_strerror --- call strerror */

static awk_value_t *
do_strerror(int nargs, awk_value_t *result)
{
	awk_value_t errnum;

	if (do_lint && nargs > 1)
		lintwarn(ext_id, _("strerror: called with too many arguments"));

	if (get_argument(0, AWK_NUMBER, & errnum)) {
		const char *str = gettext(strerror(errnum.num_value));
		return make_const_string(str, strlen(str), result);
	}
	if (do_lint) {
		if (nargs == 0)
			lintwarn(ext_id, _("strerror: called with no arguments"));
		else
			lintwarn(ext_id, _("strerror: called with inappropriate argument(s)"));
	}
	return make_null_string(result);
}

/*  do_errno2name --- convert an integer errno value to it's symbolic name */

static awk_value_t *
do_errno2name(int nargs, awk_value_t *result)
{
	awk_value_t errnum;

	if (do_lint && nargs > 1)
		lintwarn(ext_id, _("errno2name: called with too many arguments"));

	if (get_argument(0, AWK_NUMBER, & errnum)) {
		int i = errnum.num_value;

		if ((i == errnum.num_value) && (i >= 0) && ((size_t)i < NUMERR) && errno2name[i])
			return make_const_string(errno2name[i], strlen(errno2name[i]), result);
		warning(ext_id, _("errno2name: called with invalid argument"));
	} else if (do_lint) {
		if (nargs == 0)
			lintwarn(ext_id, _("errno2name: called with no arguments"));
		else
			lintwarn(ext_id, _("errno2name: called with inappropriate argument(s)"));
	}
	return make_null_string(result);
}

/*  do_name2errno --- convert a symbolic errno name to an integer */

static awk_value_t *
do_name2errno(int nargs, awk_value_t *result)
{
	awk_value_t err;

	if (do_lint && nargs > 1)
		lintwarn(ext_id, _("name2errno: called with too many arguments"));

	if (get_argument(0, AWK_STRING, & err)) {
		size_t i;

		for (i = 0; i < NUM_NAME2ERR; i++) {
			if (! strcasecmp(err.str_value.str, name2errno[i].name))
				return make_number(name2errno[i].n, result);
		}
		warning(ext_id, _("name2errno: called with invalid argument"));
	} else if (do_lint) {
		if (nargs == 0)
			lintwarn(ext_id, _("name2errno: called with no arguments"));
		else
			lintwarn(ext_id, _("name2errno: called with inappropriate argument(s)"));
	}
	return make_number(-1, result);
}

static awk_ext_func_t func_table[] = {
	{ "strerror", do_strerror, 1 },
	{ "errno2name", do_errno2name, 1 },
	{ "name2errno", do_name2errno, 1 },
};

static awk_bool_t
init_my_module(void)
{
  GAWKEXTLIB_COMMON_INIT
  return awk_true;
}

static awk_bool_t (*init_func)(void) = init_my_module;
static const char *ext_version = PACKAGE_STRING;

dl_load_func(func_table, errno, "")
