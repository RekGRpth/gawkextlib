/*
 * nl_langinfo.c - Provide an interface to nl_langinfo(3) routine
 *
 * Arnold Robbins
 * arnold@skeeve.com
 * Written 5/2015, cloned off fnmatch.c.
 */

/*
 * Copyright (C) 2012, 2013 the Free Software Foundation, Inc.
 * Copyright (C) 2015, 2017 Arnold David Robbins.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1335, USA
 */

#include "common.h"

#include <langinfo.h>

/* do_nl_langinfo --- call nl_langinfo */

static awk_value_t *
do_nl_langinfo(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG)
{
	awk_value_t val_to_get;
	char *val;

#if gawk_api_major_version < 2
	if (do_lint && nargs > 1)
		lintwarn(ext_id, _("nl_langinfo: called with too many arguments"));
#endif

	if (! get_argument(0, AWK_NUMBER, & val_to_get)) {
		warning(ext_id, _("nl_langinfo: could not get argument"));
		RET_NULSTR;
	}

	/* get the value */
	val = nl_langinfo((nl_item) val_to_get.num_value);

	/* turn it into the awk function return value */
	make_const_string(val, strlen(val), result);
	return result;
}

#define ENTRY(x)	{ #x, x }

static struct langinfo_flags {
	const char *name;
	int value;
} flagtable[] = {
	ENTRY(CODESET),
	ENTRY(D_T_FMT),
	ENTRY(D_FMT),
	ENTRY(T_FMT),
	ENTRY(T_FMT_AMPM),
	ENTRY(AM_STR),
	ENTRY(PM_STR),
	ENTRY(DAY_1),
	ENTRY(DAY_2),
	ENTRY(DAY_3),
	ENTRY(DAY_4),
	ENTRY(DAY_5),
	ENTRY(DAY_6),
	ENTRY(DAY_7),
	ENTRY(ABDAY_1),
	ENTRY(ABDAY_2),
	ENTRY(ABDAY_3),
	ENTRY(ABDAY_4),
	ENTRY(ABDAY_5),
	ENTRY(ABDAY_6),
	ENTRY(ABDAY_7),
	ENTRY(MON_1),
	ENTRY(MON_2),
	ENTRY(MON_3),
	ENTRY(MON_4),
	ENTRY(MON_5),
	ENTRY(MON_6),
	ENTRY(MON_7),
	ENTRY(MON_8),
	ENTRY(MON_9),
	ENTRY(MON_10),
	ENTRY(MON_11),
	ENTRY(MON_12),
	ENTRY(ABMON_1),
	ENTRY(ABMON_2),
	ENTRY(ABMON_3),
	ENTRY(ABMON_4),
	ENTRY(ABMON_5),
	ENTRY(ABMON_6),
	ENTRY(ABMON_7),
	ENTRY(ABMON_8),
	ENTRY(ABMON_9),
	ENTRY(ABMON_10),
	ENTRY(ABMON_11),
	ENTRY(ABMON_12),
	ENTRY(RADIXCHAR),
	ENTRY(ERA),
	ENTRY(ERA_D_FMT),
	ENTRY(ERA_D_T_FMT),
	ENTRY(ERA_T_FMT),
	ENTRY(ALT_DIGITS),
	ENTRY(THOUSEP),
	ENTRY(YESEXPR),
	ENTRY(NOEXPR),
	ENTRY(CRNCYSTR),
	{ NULL, 0 }
};

static awk_bool_t
init_my_module(void)
{
	GAWKEXTLIB_COMMON_INIT
	int errors = 0;

	awk_value_t index, value, the_array;
	awk_array_t new_array;
	int i;

	new_array = create_array();
	for (i = 0; flagtable[i].name != NULL; i++) {
		(void) make_const_string(flagtable[i].name,
				strlen(flagtable[i].name), & index);
		(void) make_number(flagtable[i].value, & value);
		if (! set_array_element(new_array, & index, & value)) {
			warning(ext_id, _("nl_langinfo init: could not set array element %s"),
					flagtable[i].name);
			errors++;
		}
	}

	the_array.val_type = AWK_ARRAY;
	the_array.array_cookie = new_array;

	if (! sym_update("LANGINFO", & the_array)) {
		warning(ext_id, _("nl_langinfo init: could not install LANGINFO array"));
		errors++;
	}


	return errors == 0;
}

static awk_bool_t (*init_func)(void) = init_my_module;
static const char *ext_version = PACKAGE_STRING;

static awk_ext_func_t func_table[] = {
	API_FUNC("nl_langinfo", do_nl_langinfo, 1)
};

dl_load_func(func_table, nl_langinfo, "")
