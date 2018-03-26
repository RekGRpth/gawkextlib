/*
 * csv.c - Builtin facilities to handle CSV data.
 */

/*
 * Copyright (C) 2018 the Free Software Foundation, Inc.
 *
 * This file is part of GAWK-CSV, the GAWK extension for handling CSV data.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1335, USA
 */

#include "common.h"
#include "csv_convert.h"
#include "csv_split.h"
#include "csv_input.h"
#include "strbuf.h"

/*-------------------------------------------------------------*\
 *                     BUILT-IN FUNCTIONS
\*-------------------------------------------------------------*/
 
/*  Get a single char argument */

static char
get_char_argument(int k, int nargs, char defval, const char* funcname)
{
    if (nargs > k) {
        awk_value_t arg;
        if (get_argument(k, AWK_STRING, & arg) && arg.str_value.len >0) {
            if (do_lint && arg.str_value.len > 1) {
                //lintwarn(ext_id, _("%s: argument %d <%s> truncated to a single char"), funcname, k+1, arg.str_value.str);
            }
            return arg.str_value.str[0];
        } else if (do_lint) {
            //lintwarn(ext_id, _("%s: wrong argument %d"), funcname, k+1);
        }
    }
    return defval;
}

/*  do_csvconvert --- convert a csv record to a plain text record with fixed field separators */

static awk_value_t *
do_csvconvert(int nargs, awk_value_t *result API_FINFO_ARG)
{
    awk_value_t csv_record;
    awk_value_t arg;
    const char* csvfs = "\0";
    char csvcomma;
    char csvquote;
    CHECK_NARGS("csvconvert", 4, 1)
    if (!get_argument(0, AWK_STRING, & csv_record)) {
        if (do_lint) {
            //lintwarn(ext_id, _("%s: wrong argument %d"), "csvconvert", 1);
        }
        return make_null_string(result);
    }
    if (nargs>1) {
        if (get_argument(1, AWK_STRING, & arg)) {
            csvfs = arg.str_value.str;
        } else {
            //lintwarn(ext_id, _("%s: wrong argument %d"), "csvconvert", 2);
        }
    }
    csvcomma = get_char_argument(2, nargs, ',', "csvconvert");
    csvquote = get_char_argument(3, nargs, '"', "csvconvert");

    strbuf_p record = csv_convert_record(csv_record.str_value.str, csvfs, csvcomma, csvquote, '\0');
    return make_const_string(strbuf_value(record), record->length, result);
}

/*  do_csvsplit --- split a csv record into an array of fields */

static awk_value_t *
do_csvsplit(int nargs, awk_value_t *result API_FINFO_ARG)
{
    awk_value_t csv_record;
    awk_value_t fields;
    char csvcomma;
    char csvquote;
    CHECK_NARGS("csvsplit", 4, 2)
    if (!get_argument(0, AWK_STRING, & csv_record)) {
        if (do_lint) {
            //lintwarn(ext_id, _("%s: wrong argument %d"), "csvsplit", 1);
        }
        return make_number(-1, result);
    }
    if (!get_argument(1, AWK_ARRAY, & fields)) {
        fatal(ext_id, _("%s: argument %d must be an array"), "csvsplit", 2);
    }
    csvcomma = get_char_argument(2, nargs, ',', "csvsplit");
    csvquote = get_char_argument(3, nargs, '"', "csvsplit");

    clear_array(fields.array_cookie);
    int nfields = csv_split_record(csv_record.str_value.str, fields.array_cookie, csvcomma, csvquote, '\0');
    return make_number(nfields, result);
}


/*-------------------------------------------------------------*\
 *                     INTERFACE VARIABLES
\*-------------------------------------------------------------*/

typedef struct {
    const char *name;
    const int default_type;  /* 0 = null, 1 = string, 2 = int */
    const char *default_string;
    const double default_num;
    awk_scalar_t cookie;
} VARNODE;

static void
csv_varinit_scalar(VARNODE *node,
                   awk_bool_t override)
{
    awk_value_t initial;
    awk_scalar_t cookie;
    
    switch (node->default_type) {
        case 1: make_string_malloc(node->default_string,
                                   strlen(node->default_string),
                                   &initial); break;
        case 2: make_number(node->default_num, &initial); break;
        default:
        case 0: make_null_string(&initial); break;
    }
    if (!gawk_varinit_scalar(node->name, &initial, override, &cookie))
        fatal(ext_id, _("CSV reserved scalar variable `%s' already used with incompatible type."), node->name);
    node->cookie = cookie;
}

/*  Reserved variables */

/* Set by the user: */
static VARNODE CSVMODE = {"CSVMODE", 2, "", 1, NULL};
static VARNODE CSVCOMMA = {"CSVCOMMA", 1, ",", 0, NULL};
static VARNODE CSVQUOTE = {"CSVQUOTE", 1, "\"", 0, NULL};
static VARNODE CSVFS = {"CSVFS", 1, "\0", 0, NULL};

/* Set by csv_get_record: */
static VARNODE CSVRECORD = {"CSVRECORD", 0, "", 0, NULL};

static void
csv_load_vars(void)
{
    csv_varinit_scalar(&CSVMODE, 0);
    csv_varinit_scalar(&CSVCOMMA, 0);
    csv_varinit_scalar(&CSVQUOTE, 0);
    csv_varinit_scalar(&CSVFS, 0);
    csv_varinit_scalar(&CSVRECORD, 1);
}

/* Cached values of control variables */

static awk_value_t csvmode;
static awk_value_t csvcomma;
static awk_value_t csvquote;
static awk_value_t csvfs;


/*-------------------------------------------------------------*\
 *                     INPUT PARSER
\*-------------------------------------------------------------*/

/*  csv_get_record --- read a CVS record from iobuf into out, return length or EOF, set RT */

static int
csv_get_record(char **out,  /* output: pointer to pointer to data */
    awk_input_buf_t *iobuf, /* input: io descriptor */
    int *errcode,           /* output: pointer to error variable */
    char **rt_start,        /* output: pointer to RT */
    size_t *rt_len          /* output: length of RT */
#if gawk_api_major_version >= 2
    , const awk_fieldwidth_info_t **field_width
                            /* output: optional field widths */
#endif
    )
{
    csv_reader_p cr = iobuf->opaque;
    int len;

    len = csv_read(cr);
    if (len>=0) {
        *out = strbuf_value(&(*cr).awk_record);
        *errcode = 0;
        *rt_len = 0;    /* set RT to "" */
        {
            char *csv_rec = strbuf_value(&(*cr).csv_record);
            int csv_len = strbuf_length(&(*cr).csv_record);
            awk_value_t rec;
            sym_update_scalar(CSVRECORD.cookie, make_const_string(csv_rec, csv_len, &rec));
        }
#if gawk_api_major_version >= 2
        *field_width = cr->csv_fields;
                            /* output: optional field widths */
#endif
    } else {
        len = EOF;
    }
    return len;
}

/*  csv_close --- free memory used by the input reader */

static void
csv_close(awk_input_buf_t *iobuf) {
    csv_reader_close(iobuf->opaque);
}

/*  BEGINFILE negotiation */

static awk_bool_t
csv_can_take_file(const awk_input_buf_t *iobuf)
{
    /* depends on CSVMODE */
    awk_bool_t ret = sym_lookup_scalar(CSVMODE.cookie, AWK_NUMBER, &csvmode);
    ret = ret && sym_lookup_scalar(CSVCOMMA.cookie, AWK_STRING, &csvcomma);
    ret = ret && sym_lookup_scalar(CSVQUOTE.cookie, AWK_STRING, &csvquote);
    ret = ret && sym_lookup_scalar(CSVFS.cookie, AWK_STRING, &csvfs);
    return ret && ((int)(csvmode.num_value) != 0);
}

/*
 * csv_take_control_of --- set up input parser.
 * We can assume that csv_can_take_file just returned true,
 * and no state has changed since then.
 */
static awk_bool_t
csv_take_control_of(awk_input_buf_t *iobuf)
{
    static int warned = FALSE;
    csv_reader_p csv_rdr;
    awk_value_t val;

    if (do_lint && !warned) {
        warned = TRUE;
        lintwarn(ext_id, _("`CSVMODE' is a gawk extension"));
    }

    /* Create and initialize an input reader */
    ezalloc(csv_rdr, struct csv_reader *, sizeof(*csv_rdr), "csv_take_control_of");
    csv_reader_init(csv_rdr, iobuf->fd, (int)(csvmode.num_value),
                    csvcomma.str_value.str[0], csvquote.str_value.str[0],
                    csvfs.str_value.str);
    iobuf->opaque = csv_rdr;

    /* Set interface methods. */
    iobuf->get_record = csv_get_record;
    iobuf->close_func = csv_close;

    /* Clear variables to be set by csv_get_record */
    sym_update_scalar(CSVRECORD.cookie, make_null_string(&val));

    return awk_true;
}


/*-------------------------------------------------------------*\
 *                     REGISTER THE EXTENSION
\*-------------------------------------------------------------*/

static awk_input_parser_t csv_parser = {
    "csv",
    csv_can_take_file,
    csv_take_control_of,
    NULL
};

static awk_ext_func_t func_table[] = {
    API_FUNC_MAXMIN("csvconvert", do_csvconvert, 4, 1)
    API_FUNC_MAXMIN("csvsplit", do_csvsplit, 4, 2)
};

static awk_bool_t
init_my_module(void)
{
    GAWKEXTLIB_COMMON_INIT
    csv_load_vars();
    register_input_parser(&csv_parser);
    return awk_true;
}

static awk_bool_t (*init_func)(void) = init_my_module;
static const char *ext_version = PACKAGE_STRING;

dl_load_func(func_table, csv, "")
