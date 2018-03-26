#ifndef STRBUF_H__
#define STRBUF_H__

#define BLKSIZE 256

typedef struct strbuf {
    char* str;
    int length;
    int capacity;
} strbuf_t;

typedef struct strbuf * strbuf_p;

/* Function prototypes */
int strbuf_init(strbuf_p sb);
void strbuf_start(strbuf_p sb);
int strbuf_put_char(strbuf_p sb, char c);
int strbuf_put_string(strbuf_p sb, char* s);
char* strbuf_value(strbuf_p sb);
int strbuf_length(strbuf_p sb);
void strbuf_free(strbuf_p sb);

#endif
