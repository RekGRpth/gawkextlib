#ifndef CSV_PARSER_H__
#define CSV_PARSER_H__

/* Parser options */
#define CSV_TRIM 1                 /* discard unquoted space */
#define CSV_IGNORE_EXTRA_SPACE 2   /* discard unquoted space around quoted fields */
#define CSV_MULTILINE 4            /* accept newlines inside fields */

/* Character values */
#define CSV_TAB    0x09
#define CSV_SPACE  0x20
#define CSV_CR     0x0d
#define CSV_LF     0x0a
#define CSV_COMMA  0x2c
#define CSV_QUOTE  0x22
#define CSV_NULL   0x00

/* CSV parser descriptor */
/* Uses generic functions for input and output,
   so it can be used in a variety of contexts */
struct csv_parser {
    unsigned char delim_char;       /* actual delimitar (comma or other) */
    unsigned char quote_char;       /* actual quote char (double quote or other) */
    unsigned char options;          /* packed set of flags */
    unsigned char (*next_char)();   /* get the next input char */
    void (*begin_field)();          /* start a new output field */
    void (*end_field)();            /* end the current output field */
    void (*mark_field)();           /* mark the current output field position */
    void (*backspace_field)();      /* discard data after the current output field mark */
    void (*put_char)(unsigned char); /* append a character to the current output field */
    void (*error)(const char*);     /* report error message */
};

typedef struct csv_parser * csv_parser_p;

/* Function prototypes */
void csv_parse(csv_parser_p p);

#endif
