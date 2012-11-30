#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include "gawkextlib.h"

#undef DBGMSG


awk_bool_t
gawk_api_varinit_scalar(const gawk_api_t *api, awk_ext_id_t ext_id,
			const char *name, awk_value_t *initial_value,
			awk_bool_t override_existing_value,
			awk_scalar_t *cookie_result)
{
  awk_value_t val;
  awk_bool_t rc = awk_true;
  awk_bool_t dofree = awk_true;

  if (override_existing_value || !sym_lookup(name, AWK_UNDEFINED, &val) ||
      (val.val_type == AWK_UNDEFINED)) {
    /* either does not exist already, or we want to override it anyway */
    if (sym_update(name, initial_value))
      dofree = awk_false;	/* memory now belongs to gawk */
    else {
      rc = awk_false;
      goto freeit;
    }
  }
  if (sym_lookup(name, AWK_SCALAR, &val))
    *cookie_result = val.scalar_cookie;
  else
    rc = awk_false;

#ifdef DBGMSG
  if (rc == awk_true) {
    sym_lookup_scalar(*cookie_result, AWK_STRING, &val);
    fprintf(stderr,
	    "DEBUG: initialized scalar %s with cookie %p and value [%s]\n",
	    name, *cookie_result, val.str_value.str);
  }
  else
    fprintf(stderr, "DEBUG: failed to initialize scalar %s\n", name);
#endif

freeit:
  if ((dofree == awk_true) && (initial_value->val_type == AWK_STRING))
    free(initial_value->str_value.str);
  return rc;
}

awk_bool_t
gawk_api_varinit_constant(const gawk_api_t *api, awk_ext_id_t ext_id,
			  const char *name, awk_value_t *initial_value,
			  awk_scalar_t *cookie_result)
{
  awk_value_t val;

  if (sym_lookup(name, AWK_UNDEFINED, &val) ||
      !sym_constant(name, initial_value)) {
    if (initial_value->val_type == AWK_STRING)
      free(initial_value->str_value.str);
    return awk_false;
  }

  if (sym_lookup(name, AWK_SCALAR, &val)) {
    *cookie_result = val.scalar_cookie;
    return awk_true;
  }
  return awk_false;
}

awk_bool_t
gawk_api_varinit_array(const gawk_api_t *api, awk_ext_id_t ext_id,
		       const char *name, awk_bool_t clear_it,
		       awk_array_t *cookie_result)
{
  awk_value_t val;

  if (sym_lookup(name, AWK_UNDEFINED, &val)) {
    if (val.val_type != AWK_ARRAY)
      return awk_false;
    if (clear_it)
      clear_array(val.array_cookie);
  }
  else {
    val.val_type = AWK_ARRAY;
    val.array_cookie = create_array();
    /* Note: the sym_update call updates the array_cookie */
    if (!sym_update(name, &val))
      return awk_false;
  }
  *cookie_result = val.array_cookie;
  return awk_true;
}
