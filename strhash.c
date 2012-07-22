/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2012 the Free Software Foundation, Inc.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/*
 * The decision is made to grow the array if the average chain length is
 * ``too big''. This is defined as the total number of entries in the table
 * divided by the size of the array being greater than some constant.
 *
 * 11/2002: We make the constant a variable, so that it can be tweaked
 * via environment variable.
 */

static int AVG_CHAIN_MAX = 2;	/* 11/2002: Modern machines are bigger, cut this down from 10. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif

#include <stdio.h>
#include <errno.h>

#include "gawkextlib.h"

/* Are 2 counted strings equal?  Note that STREQNN will match two empty ""
   strings, whereas STREQN does not! */
/* Optimized version: check first char before trying memcmp.  Is this
   really faster with a modern compiler that may inline memcmp? */
#define STREQNN(S1,L1,S2,L2) \
	(((L1) == (L2)) && \
	 (((L1) == 0) || ((*(S1) == *(S2)) && \
			  (memcmp((S1)+1,(S2)+1,(L1)-1) == 0))))

/* nuke definition in gawkapi.h */
#undef emalloc
#define	emalloc(var,ty,x,str) \
	do {	\
		if (!(var=(ty)malloc(x))) {	\
			fprintf(stderr,	\
				"%s: %s: can't allocate %ld bytes of memory (%s)",	\
			        (str), #var, (long) (x), strerror(errno)); \
			exit(1);	\
		}	\
	} while (0)

#define hash my_hash

/* my_hash --- calculate the hash function of the string in subs (copied
   from gawk str_array.c, but not masking HASHC to 32 bits) */

static unsigned long
my_hash(const char *s, size_t len, unsigned long hsize, size_t *code)
{
	unsigned long h = 0;
	unsigned long htmp;

	/*
	 * Ozan Yigit's original sdbm hash, copied from Margo Seltzers
	 * db package.
	 *
	 * This is INCREDIBLY ugly, but fast.  We break the string up into
	 * 8 byte units.  On the first time through the loop we get the
	 * "leftover bytes" (strlen % 8).  On every other iteration, we
	 * perform 8 HASHC's so we handle all 8 bytes.  Essentially, this
	 * saves us 7 cmp & branch instructions.  If this routine is
	 * heavily used enough, it's worth the ugly coding.
	 */

	/*
	 * Even more speed:
	 * #define HASHC   h = *s++ + 65599 * h
	 * Because 65599 = pow(2, 6) + pow(2, 16) - 1 we multiply by shifts
	 */
#define HASHC   htmp = (h << 6);  \
		h = *s++ + htmp + (htmp << 10) - h

	h = 0;

	/* "Duff's Device" */
	if (len > 0) {
		size_t loop = (len + 8 - 1) >> 3;

		switch (len & (8 - 1)) {
		case 0:
			do {	/* All fall throughs */
				HASHC;
		case 7:		HASHC;
		case 6:		HASHC;
		case 5:		HASHC;
		case 4:		HASHC;
		case 3:		HASHC;
		case 2:		HASHC;
		case 1:		HASHC;
			} while (--loop);
		}
	}

	if (code != NULL)
		*code = h;

	if (h >= hsize)
		h %= hsize;
	return h;
}

static unsigned long
get_table_size(unsigned long oldsize)
{
	size_t i, j;

	/*
	 * This is an array of primes. We grow the table by an order of
	 * magnitude each time (not just doubling) so that growing is a
	 * rare operation. We expect, on average, that it won't happen
	 * more than twice.  The final size is also chosen to be small
	 * enough so that MS-DOG mallocs can handle it. When things are
	 * very large (> 8K), we just double more or less, instead of
	 * just jumping from 8K to 64K.
	 */
	static const unsigned long sizes[] = {
				13, 127, 1021, 8191, 16381, 32749, 65497,
				131101, 262147, 524309, 1048583, 2097169,
				4194319, 8388617, 16777259, 33554467, 
				67108879, 134217757, 268435459, 536870923,
				1073741827
	};

	/* find next biggest hash size */
	for (i = 0, j = sizeof(sizes)/sizeof(sizes[0]); i < j; i++) {
		if (oldsize < sizes[i])
			return sizes[i];
	}
	return sizes[j-1];
}


/* Generic string hash routines: */
struct _strhash {
	strhash_entry **ht_index;
	size_t index_size;
	size_t entries;
	int size_maxed;
};

strhash *
strhash_create(size_t min_table_size)
{
	strhash *ht;

	emalloc(ht, strhash *, sizeof(*ht), "strhash_create");
	ht->index_size = get_table_size(min_table_size);
	/* We need an ecalloc macro. */
	emalloc(ht->ht_index, strhash_entry **,
		ht->index_size*sizeof(*ht->ht_index), "strhash_create");
	memset(ht->ht_index, 0, ht->index_size*sizeof(*ht->ht_index));
	ht->entries = 0;
	ht->size_maxed = 0;
	return ht;
}

static void
strhash_grow(strhash *ht)
{
	unsigned long newsize;
	strhash_entry **new_index;
	strhash_entry **bptr;
	size_t i;

	if ((newsize = get_table_size(ht->index_size)) <= ht->index_size) {
		ht->size_maxed = 1;
		return;
	}
	/* We need an ecalloc macro. */
	emalloc(new_index, strhash_entry **,
		newsize*sizeof(*new_index), "strhash_grow");
	memset(new_index, 0, newsize*sizeof(*new_index));
	for (bptr = ht->ht_index, i = 0; i < ht->index_size; i++, bptr++) {
		strhash_entry *ent;
		strhash_entry *nent;
		for (ent = *bptr; ent; ent = nent) {
			strhash_entry **nbptr;
			nent = ent->next;
			nbptr = &new_index[hash(ent->s, ent->len, newsize, NULL)];
			ent->next = *nbptr;
			*nbptr = ent;
		}
	}
	free(ht->ht_index);
	ht->ht_index = new_index;
	ht->index_size = newsize;
}

strhash_entry *
strhash_get(strhash *ht, const char *s, size_t len, int insert_if_missing)
{
	strhash_entry **bucket = &ht->ht_index[hash(s, len, ht->index_size, NULL)];
	strhash_entry *ent;

	/* Check if already present. */
	for (ent = *bucket; ent; ent = ent->next) {
		if (STREQNN(s, len, ent->s, ent->len))
			return ent;
	}
	if (!insert_if_missing)
		return NULL;

	ht->entries++;
	if (!ht->size_maxed && (ht->entries > AVG_CHAIN_MAX*ht->index_size))
		strhash_grow(ht);

	emalloc(ent, strhash_entry *, sizeof(*ent)+len, "strhash_insert");
	ent->next = *bucket;
	*bucket = ent;
	ent->data = NULL;
	ent->len = len;
	memcpy(ent->s, s, len);
	/* Add terminating NUL char just to be safe. */
	ent->s[len] = '\0';
	return ent;
}

int
strhash_delete(strhash *ht, const char *s, size_t len,
	       strhash_delete_func func, void *opaque)
{
	strhash_entry **bucket = &ht->ht_index[hash(s, len, ht->index_size, NULL)];
	strhash_entry *ent;
	strhash_entry *prev = NULL;

	/* Check if already present. */
	for (ent = *bucket; ent; prev = ent, ent = ent->next) {
		if (STREQNN(s, len, ent->s, ent->len)) {
			/* Remove entry first before calling delete_func
			   to make sure the hashtable is in a consistent
			   state. */
			if (prev)
				prev->next = ent->next;
			else
				*bucket = ent->next;
			ht->entries--;
			if (func)
				(*func)(ent->data, opaque, ht, ent);
			free(ent);
			return 0;
		}
	}
	return -1;
}

void
strhash_destroy(strhash *ht, strhash_delete_func func, void *opaque)
{
	size_t i;
	strhash_entry **bptr;

	for (bptr = ht->ht_index, i = 0; i < ht->index_size; i++, bptr++) {
		while (*bptr != NULL) {
			strhash_entry *ent;

			ent = *bptr;
			*bptr = ent->next;
			if (func)
				(*func)(ent->data, opaque, ht, ent);
			free(ent);
		}
	}
	free(ht->ht_index);
	free(ht);
}
