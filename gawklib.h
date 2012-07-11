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
