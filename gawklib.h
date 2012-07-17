/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2012 the Free Software Foundation, Inc.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/* Note: before including this file, you need to include something
   to declare the size_t type, typically <unistd.h>.  We do not do that
   here to avoid platform-specific code. */

#ifndef _GAWK_LIB_H
#define _GAWK_LIB_H

#include <gawkapi.h>

/* Generic string hash routines: */
typedef struct _strhash_entry {
	struct _strhash_entry *next;
	void *data;	/* set by caller of strhash_get to application data */
	size_t len;	/* length of string */
	char s[1];	/* variable-length array */
} strhash_entry;

typedef struct _strhash strhash;
extern strhash *strhash_create(size_t min_table_size);
/* Find an entry in the hash table.  If it is not found, the insert_if_missing
   argument indicates whether a new entry should be created.  The caller
   may set the "data" field to any desired value.  If it is a new entry,
   "data" will be initialized to NULL. */
extern strhash_entry *strhash_get(strhash *, const char *s, size_t len,
				  int insert_if_missing);
typedef void (*strhash_delete_func)(void *data, void *opaque,
				    strhash *, strhash_entry *);
extern int strhash_delete(strhash *, const char *s, size_t len,
			  strhash_delete_func, void *opaque);
extern void strhash_destroy(strhash *, strhash_delete_func, void *opaque);


/* Initialize a scalar variable.  Returns false if it is unable to
   get a scalar variable.  If the variable does not exist, it will be
   set to the value in the <initial_value> parameter.  If it exists
   already, the value will be set to <initial_value> if and only
   if <override_existing_value> is non-zero.  On success, the scalar_cookie
   value is stored in *cookie_result.  The current value can then easily be
   retrieved using the sym_lookup_scalar function.
   
   Note: if <initial_value> contains a string value, it is the caller's
   responsibility to provide a properly malloced string.  If there is
   an error, or if the variable exists already and <override_existing_value>
   is 0, this function will free the string memory.  So the caller will
   never be responsible for freeing any malloced string it supplies. */
extern awk_bool_t gawk_api_varinit_scalar(const gawk_api_t *a, awk_ext_id_t id,
					  const char *name,
					  awk_value_t *initial_value,
					  awk_bool_t override_existing_value,
					  awk_scalar_t *cookie_result);
#define gawk_varinit_scalar(N, I, O, C) \
	gawk_api_varinit_scalar(api, ext_id, N, I, O, C)

/* Initialize a scalar constant.  The extension library may change the value,
   but any user awk code that tries to change the value will trigger a fatal
   error.  If the variable exists already, this function fails.  If there
   is an error, this function will free the string memory, so the caller
   is never responsible for doing this. */
extern awk_bool_t gawk_api_varinit_constant(const gawk_api_t *a,
					    awk_ext_id_t id,
					    const char *name,
					    awk_value_t *initial_value,
					    awk_scalar_t *cookie_result);
#define gawk_varinit_constant(N, I, C) \
	gawk_api_varinit_constant(api, ext_id, N, I, C)

/* Initialize an array.  Returns false if it cannot create an array with
   the given name.  If the array exists already, it will be cleared if and
   only if the <clear_it> argument is non-zero.  On success, the array_cookie
   is stored in *cookie_result. */
extern awk_bool_t gawk_api_varinit_array(const gawk_api_t *a, awk_ext_id_t id,
					 const char *name, awk_bool_t clear_it,
					 awk_array_t *cookie_result);
#define gawk_varinit_array(N, F, C) \
	gawk_api_varinit_array(api, ext_id, N, F, C)

#endif /* _GAWK_LIB_H */
