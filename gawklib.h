/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2012 the Free Software Foundation, Inc.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

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

#define dl_load_func_stub(func_table, module, name_space) \
	size_t i, j; \
	int errors = 0; \
\
	api = api_p; \
	ext_id = id; \
\
	if (api->major_version != GAWK_API_MAJOR_VERSION \
	    || api->minor_version < GAWK_API_MINOR_VERSION) { \
		fprintf(stderr, #module ": version mismatch with gawk!\n"); \
		fprintf(stderr, "\tmy version (%d, %d), gawk version (%d, %d)\n", \
			GAWK_API_MAJOR_VERSION, GAWK_API_MINOR_VERSION, \
			api->major_version, api->minor_version); \
		exit(1); \
	} \
\
	/* load functions */ \
	for (i = 0, j = sizeof(func_table) / sizeof(func_table[0]); i < j; i++) { \
		if (! add_ext_func(& func_table[i], name_space)) { \
			warning(ext_id, #module ": could not add %s\n", \
					func_table[i].name); \
			errors++; \
		} \
	}
