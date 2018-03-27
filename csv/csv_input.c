/*
 * csv_input.c - Functions to read csv records.
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

#include "strbuf.h"
#include "csv_parser.h"
#include "csv_input.h"
#include "awk_fieldwidth_info.h"

/* Shared among handlers */
static csv_reader_p rdr;
static int fd;
static csv_parser_p parser;
static strbuf_p input;
static strbuf_p awk_rec;
static char csv_quote;
static char *csv_fs;
static int odd_quotes;
static int fieldcount = 0;

#if gawk_api_major_version >= 2
static int field_skip = 0;
static int field_len = 0;
#endif


static unsigned char next_char() {  /* get the next input char */
    char c;
    int len;
    do { len = read(fd, &c, 1); } while (len>0 && c == CSV_CR) ;  /* ignore CRs */
    if (len == 0 || c==EOF) {
        return CSV_NULL;
    } else if (c == csv_quote) {
        odd_quotes = 1 - odd_quotes;
    } else if (c == CSV_LF) {
        if (odd_quotes == 0) return CSV_NULL;
    }
    strbuf_put_char(input, c);
    return c;
}

static void begin_field() {         /* start a new output field */
    if (fieldcount) {
        strbuf_put_string(awk_rec, csv_fs);
#if gawk_api_major_version >= 2
        field_skip = strlen(csv_fs);
    }
    field_len = 0;
#else
    }
#endif
}

static void end_field() {           /* end of the current output field */
    ++fieldcount;
#if gawk_api_major_version >= 2
    rdr->max_fields = awk_fieldwidth_info_add(&(*rdr).csv_fields, rdr->max_fields, field_skip, field_len);
    field_skip = 0;
    field_len =0;
#endif
}

static void mark_field() {          /* mark the current output field position */
    // /* RESERVED FOR FUTURE USE */
}

static void backspace_field() {     /* discard data after the current output field mark */
    // /* RESERVED FOR FUTURE USE */
}

static void put_char(unsigned char c) { /* append a character to the current output field */
    strbuf_put_char(awk_rec, c);
#if gawk_api_major_version >= 2
    ++field_len;
#endif
}

static void error(const char* msg) { /* report error message */
    const int WIDTH = 65;
    const int TAIL = 10;
    const int DOTS = 3;
    int k;
    char* textline  = strbuf_value(input);
    int len = input->length;
    int pos = len;
    int cursor = pos;
    int offset = 0;
    int last = len;
    char buffer[WIDTH+1];

    if (len > WIDTH) {            /* too long record */
        if (len > pos + TAIL) {   /* truncate at the end */
            last = pos + TAIL;
        }
        if (last < WIDTH) last = WIDTH;
        offset = last - WIDTH;
        textline = strncpy(buffer, &(*input).str[offset], WIDTH);
        buffer[WIDTH] = '\0';
        cursor = pos - offset;
        if (len > last) { /* ... at the end */
            for( k = 1; k<=DOTS; k++) buffer[WIDTH-k] = '.';
        }
        if (offset > 0) { /* ... at the beginning */
            for( k = 0; k<DOTS; k++) buffer[k] = '.';
        }
    }

    nonfatal(ext_id, "csvinput: %s\n  %s\n  %*c", msg, textline, cursor, '^');
}


/* Create a csv input reader */
void csv_reader_init(csv_reader_p reader, int fdes, int csvmode, char csvcomma, char csvquote, char *csvfs) {
    reader->fd = fdes;                     /* input file descriptor */
    reader->csv_mode = csvmode;            /* input mode */
    reader->csv_fs = csvfs;                /* awk_record field separator */

    reader->parser.delim_char = csvcomma;  /* input parser ... */
    reader->parser.quote_char = csvquote;
    reader->parser.next_char = &next_char;
    reader->parser.begin_field = &begin_field;
    reader->parser.end_field = &end_field;
    reader->parser.mark_field = &mark_field;
    reader->parser.backspace_field = &backspace_field;
    reader->parser.put_char = &put_char;
    reader->parser.error = &error;

    strbuf_init(&(*reader).csv_record);    /* original CSV record */
    strbuf_init(&(*reader).awk_record);    /* equivalent awk record */
    fieldcount = 0;
#if gawk_api_major_version >= 2
    reader->max_fields = awk_fieldwidth_info_init(&(*reader).csv_fields);
    reader->csv_fields->use_chars = awk_false;
#endif
    reader->rt_len = 0;            /* length of the record terminator */
}

/* Read the next csv record */
int csv_read(csv_reader_p reader) {
    rdr = reader;
    fd = rdr->fd;
    csv_fs = rdr->csv_fs;
    input = &(*rdr).csv_record;
    awk_rec = &(*rdr).awk_record;
    parser = &(*rdr).parser;
    csv_quote = parser->quote_char;
    odd_quotes = 0;
    fieldcount = 0;
#if gawk_api_major_version >= 2
    field_skip = 0;
    field_len = 0;
#endif

    strbuf_start(input);
    strbuf_start(awk_rec);
#if gawk_api_major_version >= 2
    awk_fieldwidth_info_start(reader->csv_fields);
#endif
    csv_parse(parser);

    int len = awk_rec->length;
    if (len <= 0) return -1;
    
    return len;
}

/* Destroy the csv input reader */
void csv_reader_close(csv_reader_p reader) {
    strbuf_free(&(*reader).csv_record);
    strbuf_free(&(*reader).awk_record);
#if gawk_api_major_version >= 2
    awk_fieldwidth_info_free(reader->csv_fields);
#endif
}
