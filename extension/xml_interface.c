/*
 * xml_interface.c --- an interface between gawk and the xml_puller code
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2004 the Free Software Foundation, Inc.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "common.h"
#include "xml_puller.h"
#include <langinfo.h>

struct xml_state {
	XML_Puller puller;	/* XML parser */
	long depth;		/* current element depth */
	char *space;		/* space character in XMLCHARSET */
	size_t spacelen;	/* # of bytes in space character */
	char *slash;		/* slash character in XMLCHARSET */
	size_t slashlen;	/* # of bytes in slash character */
	char *attrnames;	/* buffer for attribute names */
	size_t bufsize;		/* length of attrnames buffer */
	char *path;		/* buffer for tag names of nested elements */
	size_t pathsize;	/* size of path buffer */
	size_t pathlen;		/* length of path */
};

#define XML(IOP) ((struct xml_state *)((IOP)->opaque))

/* Set by user: */
static awk_scalar_t XMLMODE_node;
static awk_scalar_t XMLCHARSET_node;

/* Scalars set by xml_get_record: */
static awk_scalar_t XMLDECLARATION_node, XMLSTARTELEM_node, XMLENDELEM_node; 
static awk_scalar_t XMLCHARDATA_node, XMLPROCINST_node, XMLCOMMENT_node;
static awk_scalar_t XMLSTARTCDATA_node, XMLENDCDATA_node;
static awk_scalar_t XMLSTARTDOCT_node, XMLENDDOCT_node;
static awk_scalar_t XMLUNPARSED_node;
static awk_scalar_t XMLERROR_node, XMLROW_node, XMLCOL_node, XMLLEN_node;
static awk_scalar_t XMLDEPTH_node, XMLENDDOCUMENT_node, XMLEVENT_node, XMLNAME_node;
static awk_scalar_t XMLPATH_node;

/* Arrays set by xml_get_record: */
static awk_array_t XMLATTR_array;


struct varinit {
	awk_scalar_t *spec;
	const char *name;
};

#define ENTRY(VAR) { &VAR##_node, #VAR },

#define SET_ARRAY_ELEMENT(A,I,V) {	\
	if (!set_array_element(A,I,V))	\
		fatal(ext_id, "set_array_element failed");	\
}

/* These are all the scalar variables set by xml getline: */
static const struct varinit varinit[] = {
	ENTRY(XMLDECLARATION)
	ENTRY(XMLSTARTELEM)
	ENTRY(XMLENDELEM)
	ENTRY(XMLCHARDATA)
	ENTRY(XMLPROCINST)
	ENTRY(XMLCOMMENT)
	ENTRY(XMLSTARTCDATA)
	ENTRY(XMLENDCDATA)
	ENTRY(XMLSTARTDOCT)
	ENTRY(XMLENDDOCT)
	ENTRY(XMLUNPARSED)
	ENTRY(XMLERROR)
	ENTRY(XMLROW)
	ENTRY(XMLCOL)
	ENTRY(XMLLEN)
	ENTRY(XMLDEPTH)
	ENTRY(XMLENDDOCUMENT)
	ENTRY(XMLEVENT)
	ENTRY(XMLNAME)

	/* XMLPATH & XMLCHARSET should be last.  They are treated differently
	   in resetXMLvars: we never reset XMLCHARSET, but XMLPATH is always
	   updated to the correct value. */
	ENTRY(XMLPATH)
	ENTRY(XMLCHARSET)
};
#define NUM_SCALARS	(sizeof(varinit)/sizeof(varinit[0]))
#define NUM_RESET	(NUM_SCALARS-2)

/* We can make the resetXMLvars function more elegant by defining RESET_ARRAY,
   but the code seems to be a few percent slower in that case, even if
   we compile with -funroll-loops. */
/* #define RESET_ARRAY */

#ifdef RESET_ARRAY
static awk_scalar_t scalars[NUM_SCALARS];
#endif

/* Forward function declarations: */
static void *xml_iop_open(IOBUF_PUBLIC *iop);
static void xml_iop_close(IOBUF_PUBLIC *iop);
static int xml_get_record(char **out, IOBUF_PUBLIC *, int *errcode);
static void xml_load_vars(void);

/* Should this be 1 or -1? */
#define DEFAULT_XMLMODE	-1

static awk_ext_func_t func_table[] = {
};

static awk_bool_t
init_my_module(void)
{
	xml_load_vars();
	return TRUE;
}

static awk_bool_t (*init_func)(void) = init_my_module;

dl_load_func(func_table, xml, "")

static void
xml_load_vars(void)
{
	const struct varinit *vp;
	size_t i;

	/* Register our file open handler */
	register_open_hook(xml_iop_open);

	/* N.B. This initializes all of the variables, except XMLCHARSET,
	   to a value of "".  For XMLCHARSET, it should be OK to initialize
	   to "" also, since "" seems to default to the codeset of the current
	   locale, but it is more clear to use nl_langinfo(CODESET). */
	for (vp = varinit, i = 0; i < NUM_SCALARS; i++, vp++) {
		awk_value_t val;

		/* find default initial value if not set already */
		if (strcmp(vp->name, "XMLCHARSET"))
			make_null_string(&val);
		else {
			char *charset = nl_langinfo(CODESET);
			make_string_malloc(charset, strlen(charset), &val);
		}
		/* get the cookie */
		if (!gawk_varinit_scalar(vp->name, &val, 0, vp->spec))
			fatal(ext_id, _("XML reserved scalar variable `%s' already used with incompatible type."), vp->name);
#ifdef RESET_ARRAY
		scalars[i] = *vp->spec;
#endif
	}

	if (!gawk_varinit_array("XMLATTR", 0, &XMLATTR_array))
		fatal(ext_id, _("XML reserved array variable `%s' already used with incompatible type."), "XMLATTR");

	/* If dynamically loaded, "XMLMODE" could exist already. */
	{
		awk_value_t val;

		if (!gawk_varinit_scalar("XMLMODE",
					 make_number(DEFAULT_XMLMODE, &val),
					 0, &XMLMODE_node))
			fatal(ext_id, _("XML reserved scalar variable `%s' already used with incompatible type."), "XMLMODE");
	}
}

static void *
xml_iop_open(IOBUF_PUBLIC *iop)
{
	static int warned = FALSE;
	awk_value_t xmlmode, xmlcharset;
	struct xml_state *xml;

	if ((do_lint || do_traditional) && ! warned) {
		warned = TRUE;
		lintwarn(ext_id, _("`XMLMODE' is a gawk extension"));
	}
	if (do_traditional ||
	    !sym_lookup_scalar(XMLMODE_node, AWK_NUMBER, &xmlmode)
	    || ((int)(xmlmode.num_value) == 0))
		return NULL;
	
	emalloc(xml, struct xml_state *, sizeof(*xml), "xml_iop_open");
	memset(xml, 0, sizeof(*xml));

	/* Set function methods. */
	iop->get_record = xml_get_record;
	iop->close_func = xml_iop_close;

	if (!sym_lookup_scalar(XMLCHARSET_node, AWK_STRING, &xmlcharset))
		xmlcharset.str_value.str = NULL;

	xml->puller = XML_PullerCreate(
				iop->fd,
				xmlcharset.str_value.str,
				8192);
	if (xml->puller == NULL)
		fatal(ext_id, _("cannot create XML puller"));
	XML_PullerEnable (xml->puller,
			XML_PULLER_START_ELEMENT |
			XML_PULLER_END_ELEMENT   |
			XML_PULLER_CHARDATA      |
			XML_PULLER_START_CDATA   |
			XML_PULLER_END_CDATA     |
			XML_PULLER_PROC_INST     |
			XML_PULLER_COMMENT       |
			XML_PULLER_DECL          |
			XML_PULLER_START_DOCT    |
			XML_PULLER_END_DOCT      |
			XML_PULLER_UNPARSED);
	xml->depth = 0;
	if (xmlmode.num_value < 0)
		XML_PullerEnable (xml->puller,
				  XML_PULLER_END_DOCUMENT);
	emalloc(xml->attrnames, char *, xml->bufsize = 128,
		"xml_iop_open");
	emalloc(xml->path, char *, xml->pathsize = 128,
		"xml_iop_open");
	/* Path buffer has some space, but actual length is still 0. */
	xml->pathlen = 0;
	if (!(xml->space = XML_PullerIconv(xml->puller, " ", 1,
					       &xml->spacelen)))
		fatal(ext_id, _("cannot convert space to XMLCHARSET"));
	if (!(xml->slash = XML_PullerIconv(xml->puller, "/", 1,
					       &xml->slashlen)))
		fatal(ext_id, _("cannot convert slash to XMLCHARSET"));
	return xml;
}

static void
xml_iop_close(IOBUF_PUBLIC *iop)
{
	XML_PullerFree(XML(iop)->puller);
	XML(iop)->puller = NULL;
	if (XML(iop)->attrnames) {
		free(XML(iop)->attrnames);
		XML(iop)->attrnames = NULL;
	}
	if (XML(iop)->path) {
		free(XML(iop)->path);
		XML(iop)->path = NULL;
	}
	if (XML(iop)->space) {
		free(XML(iop)->space);
		XML(iop)->space = NULL;
	}
	if (XML(iop)->slash) {
		free(XML(iop)->slash);
		XML(iop)->slash = NULL;
	}
	free(XML(iop));
	iop->opaque = NULL;
}

static void
resetXMLvars(const struct xml_state *xmlstate, XML_PullerToken token)
{
#ifdef RESET_ARRAY
	/* More elegant, but slower, even if compiled with -funroll-loops. */

	size_t i;

	for (i = 0; i < NUM_RESET; i++) {
		if (scalars[i]->var_value != Nnull_string) {
			unref(scalars[i]->var_value);
			scalars[i]->var_value=Nnull_string;
  		}
	}

#else
	awk_value_t ns;

	make_null_string(&ns);

#define RESET(FLD) sym_update_scalar(FLD##_node, &ns);

	RESET(XMLDECLARATION)
	RESET(XMLSTARTELEM)
	RESET(XMLENDELEM)
	RESET(XMLCHARDATA)
	RESET(XMLPROCINST)
	RESET(XMLCOMMENT)
	RESET(XMLSTARTCDATA)
	RESET(XMLENDCDATA)
	RESET(XMLSTARTDOCT)
	RESET(XMLENDDOCT)
	RESET(XMLUNPARSED)
	RESET(XMLERROR)
	RESET(XMLROW)
	RESET(XMLCOL)
	RESET(XMLLEN)
	RESET(XMLDEPTH)
	RESET(XMLENDDOCUMENT)
	RESET(XMLEVENT)
	RESET(XMLNAME)
#undef RESET

#endif /* RESET_ARRAY */

	clear_array(XMLATTR_array);

	/* Copy the already allocated and initialized path into
	 * the XMLPATH variable.
	 */
#define RESET_XMLPATH(XMLSTATE) {	\
	awk_value_t _t;	\
	sym_update_scalar(XMLPATH_node, \
			  make_string_malloc(XMLSTATE->path, XMLSTATE->pathlen,\
					     &_t)); \
}

/* Are 2 counted strings equal?  Note that STREQNN will match two empty ""
   strings, whereas STREQN does not! */
/* Optimized version: check first char before trying memcmp.  Is this
   really faster with a modern compiler that may inline memcmp? */
#define STREQNN(S1,L1,S2,L2) \
	(((L1) == (L2)) && \
	 (((L1) == 0) || ((*(S1) == *(S2)) && \
			  (memcmp((S1)+1,(S2)+1,(L1)-1) == 0))))

	if ((token == NULL) || (token->kind != XML_PULLER_START_ELEMENT)) {
#if 0
/* N.B. This is supposed to be a performance optimization, but benchmarks
   show that adding this check is no faster than just updating the state
   regardless of whether it has changed (the timing are almost the same). */
		awk_value_t cv;
		if (!sym_lookup_scalar(XMLPATH_node, AWK_STRING, &cv) ||
		    !STREQNN(cv.str_value.str, cv.str_value.len,
			     xmlstate->path, xmlstate->pathlen))
#endif
			RESET_XMLPATH(xmlstate)
	}
}

/* update_xmlattr --- populate the XMLATTR array
 * Upon invokation, We assume that all previous entries in the
 * XMLATTR array are already deleted.
 */

static char *
update_xmlattr(XML_PullerToken tok, IOBUF_PUBLIC *iop, int *cnt)
{
	size_t i;
	struct XML_PullerAttributeInfo *ap;
	size_t attrbytes = 0;

	/* First scan to find out how may bytes we need to set $0
	 * to the attribute names in position order. */
	for (i = 0, ap = tok->u.a.attr; i < tok->u.a.numattr; i++, ap++) {
		if (attrbytes != 0)
			attrbytes += XML(iop)->spacelen;
		attrbytes += ap->name_len;
	}
	if (attrbytes > XML(iop)->bufsize) {
		/* Need a larger buffer. */
		free(XML(iop)->attrnames);
		XML(iop)->bufsize += 2*(attrbytes-XML(iop)->bufsize)+32;
		emalloc(XML(iop)->attrnames, char *, XML(iop)->bufsize,
			"update_xmlattr");
	}

	/* Take each attribute and enter it into the XMLATTR array.
	 * attributes[i  ] is the pointer to the name  of the attribute.
	 * attributes[i+1] is the pointer to the value of the attribute.
	 *
	 * Also, populate XML(iop)->attrnames with the concatenated attribute
	 * names separated by the space encoding in XML(iop)->space.
	 */
	attrbytes = 0;
	for (i = 0, ap = tok->u.a.attr; i < tok->u.a.numattr; i++, ap++) {
		awk_value_t idx, val;

		/* First, copy attribute name into $0 buffer. */
		if (attrbytes != 0) {
			/* Insert space character. */
			memcpy(XML(iop)->attrnames+attrbytes,
			       XML(iop)->space, XML(iop)->spacelen);
			attrbytes += XML(iop)->spacelen;
		}
		memcpy(XML(iop)->attrnames+attrbytes, ap->name, ap->name_len);
		attrbytes += ap->name_len;

		SET_ARRAY_ELEMENT(XMLATTR_array,
				  make_string_no_malloc(ap->name, ap->name_len,
				  			&idx),
				  make_string_no_malloc(ap->value,
				  			ap->value_len,
				  			&val))
		ap->name = NULL;	/* Take ownership of the memory. */
		ap->value = NULL;	/* Take ownership of the memory. */
	}

	*cnt = attrbytes;
	return XML(iop)->attrnames;
}

/* append_xmlpath --- append to internal copy of XMLPATH */
static void
append_xmlpath(struct xml_state * xmlstate, XML_PullerToken token)
{
	/* First update the iop-local path string and re-allocate
	 * memory as necessary. Take care of tag name separators.
	 */
	size_t new_len = xmlstate->pathlen + xmlstate->slashlen +
			 token->name_len;

	/* Is the buffer large enough? */
	if (new_len > xmlstate->pathsize) {
		/* Need a larger path buffer. */
		xmlstate->pathsize = 2 * new_len;
		erealloc(xmlstate->path, char *, xmlstate->pathsize,
			 "append_xmlpath");
	}

	/* Append the slash character. */
	memcpy(xmlstate->path + xmlstate->pathlen, xmlstate->slash,
	       xmlstate->slashlen);
	/* Append the new tag name. */
	memcpy(xmlstate->path + xmlstate->pathlen + xmlstate->slashlen,
	       token->name, token->name_len);
	xmlstate->pathlen = new_len;
}

static inline void
chop_xmlpath(struct xml_state * xmlstate, XML_PullerToken token)
{
	/* Shorten the path. Let buffer keep its size. */
	xmlstate->pathlen -= token->name_len + xmlstate->slashlen;
}

static awk_value_t *
get_xml_string(XML_Puller puller, const char *str, awk_value_t *res)
{
	char *s;
	size_t s_len;

	if (!(s = XML_PullerIconv(puller, str, strlen(str), &s_len)))
		fatal(ext_id, _("XML_PullerIconv failed to convert event string"));
	return make_string_no_malloc(s, s_len, res);
}

static void
set_xml_attr(IOBUF_PUBLIC *iop, const char *attr, awk_value_t *value)
{
	awk_value_t idx;

	SET_ARRAY_ELEMENT(XMLATTR_array,
			  get_xml_string(XML(iop)->puller, attr, &idx), value)
}

/* get_xml_record --- read an XML token from IOP into out, return length of EOF, do not set RT */
static int
xml_get_record(char **out,        /* pointer to pointer to data */
	IOBUF_PUBLIC *iop,             /* input IOP */
	int *errcode)           /* pointer to error variable */
{
	int cnt = 0;
	XML_PullerToken token;

#define SET_XML_ATTR_STR(a, v) {	\
	awk_value_t _t;	\
	set_xml_attr(iop, a, make_string_no_malloc(v, v##_len, &_t));	\
	v = NULL;	\
}

#define SET_XMLSTR(n, s) { \
	awk_value_t _t;	\
	sym_update_scalar(n##_node, make_string_no_malloc(s, s##_len, &_t)); \
	s = NULL;	\
}

#define SET_NUMBER(n, v) { 	\
	awk_value_t _t;	\
	sym_update_scalar(n##_node, make_number(v, &_t));	\
}

#define SET_EVENT(EV)	{	\
	awk_value_t _t;	\
	sym_update_scalar(XMLEVENT_node, get_xml_string(XML(iop)->puller, #EV, \
							&_t)); \
}

#define SET_NAME(s) {	\
	awk_value_t _t;	\
	sym_update_scalar(XMLNAME_node, make_string_malloc(s, s##_len, &_t)); \
}

	token = XML_PullerNext(XML(iop)->puller);
	resetXMLvars(XML(iop), token);
	*out = NULL;
	if (token == NULL) {
		if (XML(iop)->puller->status != XML_STATUS_OK) {
			if (XML(iop)->puller->error)
				SET_XMLSTR(XMLERROR, XML(iop)->puller->error)
			else {
				awk_value_t _t;
				static char s[] = "XML Puller: unknown error";
				sym_update_scalar(XMLERROR_node,
				      make_string_malloc(s, sizeof(s)-1, &_t));
			}
			if (errcode)
				*errcode = -1;
			/* copy XMLERROR into ERRNO */
			{
				awk_value_t _t;
				sym_lookup_scalar(XMLERROR_node, AWK_STRING,
						  &_t);
				update_ERRNO_string(_t.str_value.str, 0);
			}
			SET_NUMBER(XMLROW, XML(iop)->puller->row)
			SET_NUMBER(XMLCOL, XML(iop)->puller->col)
			SET_NUMBER(XMLLEN, XML(iop)->puller->len)
			SET_NUMBER(XMLDEPTH, XML(iop)->depth)
		}
		cnt = EOF;
	} else {
		SET_NUMBER(XMLROW, token->row)
		SET_NUMBER(XMLCOL, token->col)
		SET_NUMBER(XMLLEN, token->len)
		SET_NUMBER(XMLDEPTH, XML(iop)->depth)
		switch (token->kind) {
		case XML_PULLER_START_ELEMENT:
			append_xmlpath(XML(iop), token);
			RESET_XMLPATH(XML(iop))
			SET_EVENT(STARTELEM)
			SET_NAME(token->name)
			SET_XMLSTR(XMLSTARTELEM, token->name)
			*out = update_xmlattr(token, iop, &cnt);
			XML(iop)->depth++;
			SET_NUMBER(XMLDEPTH, XML(iop)->depth)
			break;
		case XML_PULLER_END_ELEMENT:
			chop_xmlpath(XML(iop), token);
			SET_EVENT(ENDELEM)
			SET_NAME(token->name)
			SET_XMLSTR(XMLENDELEM, token->name)
			XML(iop)->depth--;
			break;
		case XML_PULLER_CHARDATA:
			SET_EVENT(CHARDATA)
			SET_NUMBER(XMLCHARDATA, 1)
			*out = token->u.d.data;
			cnt = token->u.d.data_len;
			break;
		case XML_PULLER_START_CDATA:
			SET_EVENT(STARTCDATA)
			SET_NUMBER(XMLSTARTCDATA, 1)
			break;
		case XML_PULLER_END_CDATA:
			SET_EVENT(ENDCDATA)
			SET_NUMBER(XMLENDCDATA, 1)
			break;
		case XML_PULLER_PROC_INST:
			SET_EVENT(PROCINST)
			SET_NAME(token->name)
			SET_XMLSTR(XMLPROCINST, token->name)
			*out = token->u.d.data;
			cnt = token->u.d.data_len;
			break;
		case XML_PULLER_COMMENT:
			SET_EVENT(COMMENT)
			SET_NUMBER(XMLCOMMENT, 1)
			*out = token->u.d.data;
			cnt = token->u.d.data_len;
			break;
		case XML_PULLER_DECL:
			SET_EVENT(DECLARATION)
			SET_NUMBER(XMLDECLARATION, 1)
			if (token->name != NULL)
				SET_XML_ATTR_STR("VERSION", token->name)
			if (token->u.d.data != NULL)
				SET_XML_ATTR_STR("ENCODING", token->u.d.data)
			if (token->u.d.number >= 0) {
				/* -1 means there was no standalone parameter
				   in the declaration.  0 means it was "no",
				   and 1 means it was "yes". */
				awk_value_t _t;
				set_xml_attr(iop, "STANDALONE",
					     get_xml_string(XML(iop)->puller,
					     		    (token->u.d.number ?
							     "yes" : "no"),
							    &_t));
			}
			break;
		case XML_PULLER_START_DOCT:
			SET_EVENT(STARTDOCT)
			SET_NAME(token->name)
			SET_XMLSTR(XMLSTARTDOCT, token->name)
			if (token->u.d.pubid != NULL)
				SET_XML_ATTR_STR("PUBLIC", token->u.d.pubid)
			if (token->u.d.data != NULL)
				SET_XML_ATTR_STR("SYSTEM", token->u.d.data)
			if (token->u.d.number) {
				/* will be non-zero if the DOCTYPE declaration
				   has an internal subset. */
				awk_value_t _t;
				set_xml_attr(iop, "INTERNAL_SUBSET",
					     make_number(token->u.d.number,
							 &_t));
			}
			break;
		case XML_PULLER_END_DOCT:
			SET_EVENT(ENDDOCT)
			SET_NUMBER(XMLENDDOCT, 1)
			break;
		case XML_PULLER_UNPARSED:
			SET_EVENT(UNPARSED)
			SET_NUMBER(XMLUNPARSED, 1)
			*out = token->u.d.data;
			cnt = token->u.d.data_len;
			break;
		case XML_PULLER_END_DOCUMENT:
			SET_EVENT(ENDDOCUMENT)
			SET_NUMBER(XMLENDDOCUMENT, 1)
			break;
		}
	}
#undef SET_NAME
#undef SET_EVENT
#undef SET_XMLSTR
#undef SET_NUMBER

	return cnt;
}
