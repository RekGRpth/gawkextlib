/*
 * strbuf -- variable length string buffer
 */

/*
 * Copyright (C) 2018 the Free Software Foundation, Inc.
 *
 * This file is part of gawk-csv, the GAWK extension for handling CSV data.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <stdlib.h>
#include "strbuf.h"

#define BLKSIZE 256

/* Initialize a 1-block buffer */
int strbuf_init(strbuf_p sb) {
    if (sb->str = malloc(BLKSIZE)) {
        sb->length = 0;
        sb->capacity = BLKSIZE;
        return 0;  /* success */
    } else {
        return 1;  /* failure */
    }
}

/* Reuse an existing buffer */
void strbuf_start(strbuf_p sb) {
    sb->length = 0;
}

/* Append a character to the buffer. Expand it if necessary */
int strbuf_put_char(strbuf_p sb, char c) {
    char* newstr;
    int newcap;
    if (sb->length+1 >= sb->capacity) {
        newcap = sb->capacity + BLKSIZE;
        newstr = realloc(sb->str, newcap);
        if (newstr == NULL) {
            free(sb->str);
            return 1;
        } else {
            sb->str = newstr;
            sb->capacity = newcap;
        }
    }
    sb->str[sb->length++] = c;
    return 0;
}

/* Append a string to the buffer. Expand it if necessary */
int strbuf_put_string(strbuf_p sb, char* s) {
    char c = (s == NULL) ? '\0' : s[0];
    int pos = 0;
    while (c) {
        if (strbuf_put_char(sb, c)) return 1;
        c = s[++pos];
    }
    return 0;
}

/* Get a pointer to the C-string value of the buffer */
char* strbuf_value(strbuf_p sb) {
    if (sb->str) sb->str[sb->length] = '\0';
    return sb->str;
}

/* Get the length of the string value of the buffer */
int strbuf_length(strbuf_p sb) {
    return sb->length;
}

/* Free allocated memory */
void strbuf_free(strbuf_p sb) {
    free(sb->str);
    sb->str = NULL;
    sb->length = 0;
    sb->capacity = 0;
}
