/*
 * csv_split.c - Functions to split a csv record into an array of fields.
 */

/*
 * Copyright (C) 2018 the Free Software Foundation, Inc.
 *
 * This file is part of GAWK-CSV, the GAWK extension for handling CSV data.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "common_aux.h"

#include "strbuf.h"
#include "csv_parser.h"
#include "csv_split.h"

static char* csv_record;    /* pointer to the csv input string */
static int pos;             /* input offset */
static struct strbuf sbuf;  /* buffer to build a field */
static int fieldcount = 0;  /* number of completed fields */
static awk_array_t oaf;     /* output array of fields */

static unsigned char next_char() {  /* get the next input char */
    return csv_record[pos++];
}

static void begin_field() {         /* start a new output field */
    strbuf_start(&sbuf);
}

static void end_field() {           /* end of the current output field */
    awk_value_t index, value;
    ++fieldcount;
    (void) make_number(fieldcount, &index);
    (void) make_const_user_input(sbuf.str, sbuf.length, &value);
    if (!set_array_element(oaf, &index, &value) && do_lint) {
        lintwarn(ext_id, _("%s: set_array_element failed"), "csvsplit");
    }
}

static void mark_field() {          /* mark the current output field position */
    /* RESERVED FOR FUTURE USE */
}

static void backspace_field() {     /* discard data after the current output field mark */
    /* RESERVED FOR FUTURE USE */
}

static void put_char(unsigned char c) { /* append a character to the current output field */
    strbuf_put_char(&sbuf, c);
}

static void error(const char* msg) { /* report error message */
    const int WIDTH = 65;
    const int TAIL = 10;
    const int DOTS = 3;
    int k;
    char* textline  = csv_record;
    int cursor = pos;
    int len = strlen(csv_record);
    int offset = 0;
    int last = len;
    char buffer[WIDTH+1];

    if (len > WIDTH) {            /* too long record */
        if (len > pos + TAIL) {   /* truncate at the end */
            last = pos + TAIL;
        }
        if (last < WIDTH) last = WIDTH;
        offset = last - WIDTH;
        textline = strncpy(buffer, &csv_record[offset], WIDTH);
        buffer[WIDTH] = '\0';
        cursor = pos - offset;
        if (len > last) { /* ... at the end */
            for( k = 1; k<=DOTS; k++) buffer[WIDTH-k] = '.';
        }
        if (offset > 0) { /* ... at the beginning */
            for( k = 0; k<DOTS; k++) buffer[k] = '.';
        }
    }

    nonfatal(ext_id, "csvsplit: %s\n  %s\n  %*c", msg, textline, cursor, '^');
}

/* csv_split_record --- split a csv record into an array of fields */
int
csv_split_record(const char* s, awk_array_t af, char comma, char quote, char options) {
    csv_record = s;
    pos = 0;
    fieldcount = 0;
    oaf = af;

    struct csv_parser p;
    p.delim_char = comma;
    p.quote_char = quote;
    p.options = options;
    p.next_char = &next_char;
    p.begin_field = &begin_field;
    p.end_field = &end_field;
    p.mark_field = &mark_field;
    p.backspace_field = &backspace_field;
    p.put_char = &put_char;
    p.error = &error;

    strbuf_init(&sbuf);
    csv_parse(&p);
    return fieldcount;
}
