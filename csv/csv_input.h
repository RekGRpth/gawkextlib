#ifndef CSV_INPUT_H__
#define CSV_INPUT_H__

//#include "common_aux.h"

#include "strbuf.h"
#include "csv_parser.h"

typedef struct csv_reader {
    int fd;                 /* input file descriptor */
    int csv_mode;           /* input mode */
    char *csv_fs;           /* awk_record field separator */
    int csv_fs_len;         /* length of the above */
    csv_parser_t parser;    /* input parser */
    strbuf_t csv_record;    /* original CSV record */
    strbuf_t awk_record;    /* equivalent awk record */
#if gawk_api_major_version >= 2
    awk_fieldwidth_info_t *csv_fields; /* field positions */
    int max_fields;         /* capacity of the above */
#endif
    char rt[3];             /* record terminator */
    int rt_len;             /* length of the record terminator */
} csv_reader_t;

typedef struct csv_reader * csv_reader_p;

/* Create a csv input reader */
void csv_reader_init(csv_reader_p reader, int fd, int csvmode, char csvcomma, char csvquote, char *csvfs, int csvfslen);

/* Read the next csv record */
int csv_read(csv_reader_p reader);

/* Destroy the csv input reader */
void csv_reader_close(csv_reader_p reader);

#endif
