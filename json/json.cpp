/*
 * json.cpp - Extensions functions to implement encoding an array to JSON
 * and decoding JSON into an array.
 */

/*
 * Copyright (C) 2017 the Free Software Foundation, Inc.
 * Copyright (C) 2017 Arnold David Robbins.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1335, USA
 */

// This code adapted from the rwarray.c extension in the gawk distribution.
// Primarily for the logic to walk an array and encode / decode it.

#include "config.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include "rapidjson/writer.h"
#include <gawkapi.h>

#include "awkjsonhandler.h"


const gawk_api_t *api;	/* for convenience macros to work */
awk_ext_id_t *ext_id;

int plugin_is_GPL_compatible;

#ifdef HAVE_LIBINTL_H
#include <libintl.h>
#define _(msgid)  dgettext(PACKAGE, msgid)

#define GAWKEXTLIB_COMMON_INIT { \
	if (!bindtextdomain(PACKAGE, LOCALEDIR)) \
		warning(ext_id, _("bindtextdomain(`%s', `%s') failed"), \
			PACKAGE, LOCALEDIR); \
}
#else
#define _(msgid)  msgid
#define GAWKEXTLIB_COMMON_INIT
#endif

// Forward declarations:
static bool write_array(rapidjson::Writer<rapidjson::StringBuffer>& writer, awk_array_t array, bool do_linear_arrays);
static bool write_elem(rapidjson::Writer<rapidjson::StringBuffer>& writer, awk_element_t *element, bool do_linear_arrays);


// Borrowed from gawk:
/* double_to_int --- convert double to int, used several places */

static double
double_to_int(double d)
{
	if (d >= 0)
		d = floor(d);
	else
		d = ceil(d);
	return d;
}

/* write_value --- write a number or a string or a strnum or a regex or an array */

static bool
write_value(rapidjson::Writer<rapidjson::StringBuffer>& writer, awk_value_t *val, bool do_linear_arrays)
{
	switch (val->val_type) {
	case AWK_ARRAY:
		return write_array(writer, val->array_cookie, do_linear_arrays);

	case AWK_NUMBER:
		if (double_to_int(val->num_value) == val->num_value)
			return writer.Int64((int64_t) val->num_value);
		else
			return writer.Double(val->num_value);

	case AWK_STRING:
	case AWK_STRNUM:
		return writer.String(val->str_value.str, val->str_value.len, true);

	case AWK_REGEX:
	{
		std::string full("@/");
		std::string text(val->str_value.str, val->str_value.len);
		full += text;
		full += "/";
		return writer.String(full.c_str(), full.length(), true);
	}

	case AWK_UNDEFINED:
		return writer.Null();

	default:
		/* XXX can this happen? */
		warning(ext_id, _("array value has unknown type %d"), val->val_type);
		return false;
	}

	return true;
}

/* write_elem --- write out a single element */

static bool
write_elem(rapidjson::Writer<rapidjson::StringBuffer>& writer, awk_element_t *element, bool do_linear_arrays)
{
	std::string key(element->index.str_value.str, element->index.str_value.len);

	if (! writer.Key(key.c_str(), key.length(), true))
		return false;

	return write_value(writer, & element->value, do_linear_arrays);
}

// compare --- comparison function for qsort, sorting array of pointers
//
// We want to order an integer-indexed array by integer value. But awk
// indices are strings, so convert them to numbers.

static int
compare(const void *l, const void *r)
{
	awk_element_t **ll = (awk_element_t **) l;
	awk_element_t **rr = (awk_element_t **) r;

	awk_element_t *left_elem = *ll;
	awk_element_t *right_elem = *rr;

	char *endptr_l, *endptr_r;
	const char *left_str = left_elem->index.str_value.str;
	const char *right_str = right_elem->index.str_value.str;

	long left_val = strtol(left_str, & endptr_l, 10);
	long right_val = strtol(right_str, & endptr_r, 10);

	// check for invalid conversions
	if (endptr_l == left_str || endptr_r == right_str)
		return strcmp(left_str, right_str);

	return left_val - right_val;
}

/* write_array --- write out an array or a sub-array */

static bool
write_array(rapidjson::Writer<rapidjson::StringBuffer>& writer, awk_array_t array, bool do_linear_arrays)
{
	uint32_t i;
	awk_flat_array_t *flat_array;

	if (! flatten_array(array, & flat_array)) {
		warning(ext_id, _("write_array: could not flatten array\n"));
		errno = ENOMEM;
		return false;
	}

	bool retval = false;
	awk_element_t **sorted_elems = NULL;

	if (do_linear_arrays) {
		sorted_elems = new awk_element_t*[flat_array->count];
		for (i = 0; i < flat_array->count; i++)
			sorted_elems[i] = & flat_array->elements[i];

		// sort the array of pointers
		qsort(sorted_elems, flat_array->count, sizeof(*sorted_elems), compare);

		// check that this is a linear array
		for (i = 0; i < flat_array->count; i++) {
			char buf[100];

			sprintf(buf, "%d", i + 1);
			if (strlen(buf) != sorted_elems[i]->index.str_value.len
			    || strcmp(buf, sorted_elems[i]->index.str_value.str) != 0) {
				// nope!
				goto regular_array;
			}
		}

		writer.StartArray();
		// now traverse
		for (i = 0; i < flat_array->count; i++) {
			if (! write_value(writer, & sorted_elems[i]->value, do_linear_arrays)) {
				goto done;
			}
		}
		writer.EndArray();
	} else {
regular_array:
		writer.StartObject();
		for (i = 0; i < flat_array->count; i++) {
			if (! write_elem(writer, & flat_array->elements[i], do_linear_arrays)) {
				goto done;
			}
		}
		writer.EndObject();
	}

	retval = true;

done:
	if (sorted_elems != NULL)
		delete[] sorted_elems;

	if (! release_flattened_array(array, flat_array)) {
		warning(ext_id, _("write_array: could not release flattened array\n"));
		retval = false;
	}

	return retval;
}


extern "C" {
/*  do_json_fromJSON --- convert JSON string to an array */

static awk_value_t *
do_json_fromJSON(int nargs, awk_value_t *result, awk_ext_func_t *unused)
{
	bool success = false;
	errno = 0;

	if (nargs != 2) {
		if (do_lint)
			lintwarn(ext_id, _("json_fromJSON: expecting two arguments, received %d"), nargs);
		goto out;
	}

	// get first argument, should be a string
	awk_value_t data, array;
	if (! get_argument(0, AWK_STRING, & data)) {
		nonfatal(ext_id, _("json_fromJSON: first argument is not a string"));
		errno = EINVAL;
		goto done;
	}

	// get second argument, should be a target array
	if (! get_argument(1, AWK_ARRAY, & array)) {
		nonfatal(ext_id, _("json_fromJSON: second argument is not an array"));
		errno = EINVAL;
		goto done;
	}

	// Clear the target array
	if (! clear_array(array.array_cookie)) {
		nonfatal(ext_id, _("json_fromJSON: clear_array failed"));
		errno = ENOMEM;
		goto done;
	}

	// open new scope for object creation / destruction
	// lets us use goto in the above code
	{
		// Create SAX object
		AwkJsonHandler handler(array.array_cookie);
		rapidjson::Reader reader;
		rapidjson::StringStream ss(data.str_value.str);

		// convert from JSON
		success = reader.Parse(ss, handler);
	}
	if (! success)
		errno = EINVAL;

done:
	if (errno != 0)
		update_ERRNO_int(errno);

out:
	// return success / failure
	return make_number(success, result);
}

/*  do_json_toJSON --- convert an array to JSON */

static awk_value_t *
do_json_toJSON(int nargs, awk_value_t *result, awk_ext_func_t *unused)
{
	awk_value_t source;

	if (do_lint && (nargs == 0 || nargs > 2))
		lintwarn(ext_id, _("json_toJSON: expecting one or two arguments, received %d"), nargs);

	errno = 0;
	if (! get_argument(0, AWK_ARRAY, & source)) {
		nonfatal(ext_id, _("json_toJSON: first argument is not an array\n"));
		errno = EINVAL;
		update_ERRNO_int(errno);
		return make_const_string("", 0, result);
	}

	bool do_linear_arrays = false;
	awk_value_t linear_arrays;
	if (nargs == 2) {
		if (get_argument(1, AWK_NUMBER, & linear_arrays)) {
			do_linear_arrays = (linear_arrays.num_value != 0);
		} else {
			errno = EINVAL;
			update_ERRNO_int(errno);
			return make_const_string("", 0, result);
		}
	}

	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	if (write_array(writer, source.array_cookie, do_linear_arrays)) {
		std::string final_json = s.GetString();

		return make_const_string(final_json.c_str(),
					final_json.length(), result);
	} else if (errno == 0)
		errno = EINVAL;	// best guess

	update_ERRNO_int(errno);

	return make_null_string(result);
}


/*
 * N.B. the 3rd value in the awk_ext_func_t struct called num_expected_args is
 * actually the maximum number of allowed args. A better name would be
 * max_allowed_args.
 */
static awk_ext_func_t func_table[] = {
	{ "json_toJSON",   do_json_toJSON,   2, 1 },
	{ "json_fromJSON", do_json_fromJSON, 2, 2 },
};

static awk_bool_t
init_my_module(void)
{
	GAWKEXTLIB_COMMON_INIT
	return awk_true;
}

static awk_bool_t (*init_func)(void) = init_my_module;
static const char *ext_version = PACKAGE_STRING;

int dl_load(const gawk_api_t *const api_p, awk_ext_id_t id) 
{
	size_t i, j;
	int errors = 0;

	api = api_p;
	ext_id = (void **) id;

	if (api->major_version != GAWK_API_MAJOR_VERSION
	    || api->minor_version < GAWK_API_MINOR_VERSION) {
		fprintf(stderr, "json" ": version mismatch with gawk!\n");
		fprintf(stderr, "\tmy version (%d, %d), gawk version (%d, %d)\n",
			GAWK_API_MAJOR_VERSION, GAWK_API_MINOR_VERSION,
			api->major_version, api->minor_version);
		exit(1);
	}

	check_mpfr_version(extension);

	/* load functions */
	for (i = 0, j = sizeof(func_table) / sizeof(func_table[0]); i < j; i++) {
		if (func_table[i].name == NULL)
			break;
		if (! add_ext_func("", & func_table[i])) {
			warning(ext_id, "json" ": could not add %s\n",
					func_table[i].name);
			errors++;
		}
	}

	if (init_func != NULL) {
		if (! init_func()) {
			warning(ext_id, "json" ": initialization function failed\n");
			errors++;
		}
	}

	if (ext_version != NULL)
		register_ext_version(ext_version);

	return (errors == 0);
}



}
