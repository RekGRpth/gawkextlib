/*
* csv_convert.c - Functions to convert csv data.
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

#include "common_aux.h"

#include "csv_parser.h"
#include "csv_convert.h"

static char* csv_record;    /* pointer to the csv input string */
static int pos;             /* input offset */
static struct strbuf sbuf;  /* buffer to build the output */
static int fieldcount = 0;  /* number of completed fields */
static char* ofs;           /* output field separator */

static unsigned char next_char() {  /* get the next input char */
    return csv_record[pos++];
}

static void begin_field() {         /* start a new output field */
    if (fieldcount) strbuf_put_string(&sbuf, ofs);
}

static void begin_field0() {        /* start a new '\0' delimited output field */
    if (fieldcount) strbuf_put_char(&sbuf, '\0');
}

static void end_field() {           /* end of the current output field */
    ++fieldcount;
}

static void mark_field() {          /* mark the current output field position */
}

static void backspace_field() {     /* discard data after the current output field mark */
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
    nonfatal(ext_id, "csvconvert: %s\n  %s\n  %*c", msg, textline, cursor, '^');
}

/* csv_convert_record --- convert a csv record to a regular awk record
using the given field separator */
strbuf_p csv_convert_record(const char* s, const char* fs, char comma, char quote, char options) {
    csv_record = s;
    pos = 0;
    fieldcount = 0;
    ofs = fs;

    struct csv_parser p;
    p.delim_char = comma;
    p.quote_char = quote;
    p.options = options;
    p.next_char = &next_char;
    if (*ofs=='\0') {
        p.begin_field = &begin_field0;
    } else {
        p.begin_field = &begin_field;
    }
    p.end_field = &end_field;
    p.mark_field = &mark_field;
    p.backspace_field = &backspace_field;
    p.put_char = &put_char;
    p.error = &error;

    strbuf_init(&sbuf);
    csv_parse(&p);
    return &sbuf;
}
