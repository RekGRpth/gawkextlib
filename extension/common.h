#include <string.h>
#include <stdlib.h>
#include "gawklib.h"
#include "gawkapi.h"

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t *ext_id;

int plugin_is_GPL_compatible;

static inline awk_value_t *
make_null_string(awk_value_t *result)
{
	result->val_type = AWK_UNDEFINED;
	return result;
}

#define RET_NULSTR	return make_null_string(result)
#define RET_NUM(X)	return make_number((X), result)


#define set_ERRNO(X)		update_ERRNO_string((X), 1)
#define set_ERRNO_no_gettext(X)	update_ERRNO_string((X), 0)

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
