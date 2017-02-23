/*
 * csv_parser.c - Functions to parse csv data.
 */

/*
 * Copyright (C) 2016 the Free Software Foundation, Inc.
 *
 * This file is part of GAWK-CSV, the GAWK extension for handling CSV data.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

//#include <stdio.h>
//#include "common.h"
// #if 0
// #include "config.h"
// #include <string.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <unistd.h>
// #include <sys/stat.h>
// #include <gawkapi.h>
// extern const gawk_api_t *api;	/* for convenience macros to work */
// extern awk_ext_id_t *ext_id;
// #endif
#include "csv_parser.h"

/* Parser states */
#define BEFORE_FIELD           0
#define IN_QUOTED_FIELD        1
#define AFTER_QUOTE            2
#define IN_UNQUOTED_FIELD      3
#define AFTER_FIELD            4
#define AFTER_DELIM            5


/*  csv_parse --- parse a csv record
 *  from a generic source stream into a generic output structure */
void
csv_parse(csv_parser_p p) {
//fprintf(stderr, "api-3:%d, id-3:%d\n", api, ext_id);
//printf("parse\n");
    int state = BEFORE_FIELD;  /* Current parser state */
    int c = p->next_char();    /* The character we are currently processing */

    /* Process input characters */
    while (c) {
        switch (state) {
          case AFTER_DELIM:
//            if (c==CSV_LF) {
//                p->begin_field();
//                p->end_field();
//            }
          case BEFORE_FIELD:
//          case AFTER_FIELD:
            if (c==p->delim_char) {
                p->begin_field();
                p->end_field();
                state = AFTER_DELIM;
            } else if (c==CSV_LF) {
//                state = AFTER_FIELD;
                c = CSV_NULL;
            } else if (c==p->quote_char) {
                p->begin_field();
                state = IN_QUOTED_FIELD;
            } else {
                p->begin_field();
                p->put_char(c);
                state = IN_UNQUOTED_FIELD;
            }
            break;
          case IN_QUOTED_FIELD:
            if (c==p->quote_char) {
                state = AFTER_QUOTE;
            } else {
                p->put_char(c);
            }
            break;
          case AFTER_QUOTE:
            if (c==p->quote_char) {
                p->put_char(c);
                state = IN_QUOTED_FIELD;
            } else if (c==p->delim_char) {
                p->end_field();
                state = AFTER_DELIM;
            } else if (c==CSV_LF) {
                p->end_field();
                state = AFTER_FIELD;
                c = CSV_NULL;
            } else {
                p->error("Unexpected character");
//                warning(ext_id, "Unexpected character <%c>", c);
                p->put_char(p->quote_char);
                p->put_char(c);
                state = IN_UNQUOTED_FIELD;
            }
            break;
          case IN_UNQUOTED_FIELD:
            if (c==p->delim_char) {
                p->end_field();
                state = AFTER_DELIM;
            } else if (c==CSV_LF) {
                p->end_field();
                state = AFTER_FIELD;
                c = CSV_NULL;
            } else if (c==p->quote_char) {
                p->error("Unexpected quote");
                p->put_char(c);
            } else {
                p->put_char(c);
            }
            break;
          default:
            break;
        }
        if (c) c = p->next_char();
    }
    
    /* Finalize the record */
    switch (state) {
      case BEFORE_FIELD:
      case AFTER_FIELD:
        break;
      case IN_QUOTED_FIELD:
        p->error("Missing closing quote");
      case AFTER_QUOTE:
      case IN_UNQUOTED_FIELD:
        p->end_field();
        break;
      case AFTER_DELIM:
        p->begin_field();
        p->end_field();
        break;
      default:
        break;
    }    

}
