#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#ifdef GAWKEXTLIB_NOT_NEEDED
#include <gawkapi.h>
#else
#include "gawkextlib.h"
#endif
#include "unused.h"

#ifndef SKIP_GLOBALS_CREATION
const gawk_api_t *api;	/* for convenience macros to work */
awk_ext_id_t ext_id;
int plugin_is_GPL_compatible;
#else
extern const gawk_api_t *api;	/* for convenience macros to work */
extern awk_ext_id_t ext_id;
#endif

#ifdef HAVE_LIBINTL_H
#include <libintl.h>
#define _(msgid)  dgettext(PACKAGE, msgid)
#else
#define _(msgid)  msgid
#endif


/* same as gawkapi.h:make_null_string but avoids needless memset */
static inline awk_value_t *
make_nul_string(awk_value_t *result)
{
	result->val_type = AWK_UNDEFINED;
	return result;
}

/* performance tweak: */
#define make_null_string(X) make_nul_string(X)

#define RET_NULSTR	return make_null_string(result)
#define RET_NUM(X)	return make_number((X), result)


#define set_ERRNO(X)		update_ERRNO_string(X)

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define make_string_malloc(str, len, result)	\
	r_make_string(api, ext_id, str, len, 1, result)

#define make_string_no_malloc(str, len, result)	\
	r_make_string(api, ext_id, str, len, 0, result)

#if gawk_api_major_version >= 2

#define __UNUSED_V2 __UNUSED

#define API_FINFO_ARG , struct awk_ext_func *finfo __UNUSED

#define API_FUNC_MAXMIN(NAME, FUNC, MAXARGS, MINARGS) \
	{ .name = NAME, .function = FUNC, .max_expected_args = MAXARGS, .min_required_args = MINARGS },

#define make_user_input_malloc(str, len, result) \
	r_make_string_type(api, ext_id, str, len, 1, result, AWK_STRNUM)

#define make_user_input_no_malloc(str, len, result) \
	r_make_string_type(api, ext_id, str, len, 0, result, AWK_STRNUM)

#else /* gawk_api_major_version < 2 */

/* backwards compatibility */

#define __UNUSED_V2 __UNUSED

#define API_FINFO_ARG

#define API_FUNC_MAXMIN(NAME, FUNC, MAXARGS, MINARGS) \
	{ .name = NAME, .function = FUNC, .num_expected_args = MAXARGS },

#define make_user_input_malloc(str, len, result) \
	make_string_malloc(str, len, result)
	
#define make_user_input_no_malloc(str, len, result) \
	make_string_no_malloc(str, len, result)

#ifndef gawk_calloc
/* wow. really old API version before we had memory allocation functions! */
#define gawk_calloc calloc
#endif

#define ezalloc(pointer, type, size, message) \
	do { \
		if ((pointer = (type) gawk_calloc(1, size)) == 0) \
			fatal(ext_id, "%s: calloc of %d bytes failed\n", message, size); \
	} while(0)

#endif /* gawk_api_major_version >= 2 */

/* for functions that have a fixed number of arguments: */
#define API_FUNC(NAME, FUNC, ARGS) \
	API_FUNC_MAXMIN(NAME, FUNC, ARGS, ARGS)

#ifdef HAVE_LIBINTL_H
#define GAWKEXTLIB_COMMON_INIT { \
	if (!bindtextdomain(PACKAGE, LOCALEDIR)) \
		warning(ext_id, _("bindtextdomain(`%s', `%s') failed"), \
			PACKAGE, LOCALEDIR); \
}
#else
#define GAWKEXTLIB_COMMON_INIT
#endif


/* Convenience macro to check the number of arguments */
#define check_nargs(name, min, max) { \
    if (nargs < min)  \
        fatal(ext_id, _("%s: requires at least %d argument(s)"), name, min); \
    else if (do_lint && max >= min && nargs > max) \
        lintwarn(ext_id, _("%s: called with more than %d argument(s)"), name, max); \
}
