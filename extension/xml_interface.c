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

/* use int instead of short to reduce frequency of rollover (it does not cost
   us any storage space, since it is inside a structure with a pointer).
   Avoid long because perhaps updating an 8-byte value may require more
   memory bandwidth. */
typedef unsigned int gencounter_t;

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
	gencounter_t pathvers;	/* version of path for efficient updating */
	awk_value_cookie_t string_cache[12];
};

#define XML(IOP) ((struct xml_state *)((IOP)->opaque))

typedef struct {
   awk_scalar_t sc;
   gencounter_t gen;	/* 0 indicates Null_string (""), else when set */
} MYNODE;
static gencounter_t curgen = 1;

/* Set by user: */
static awk_scalar_t XMLMODE_node;
static awk_scalar_t XMLCHARSET_node;

/* Scalars set by xml_get_record: */
static MYNODE XMLDECLARATION_node, XMLSTARTELEM_node, XMLENDELEM_node; 
static MYNODE XMLCHARDATA_node, XMLPROCINST_node, XMLCOMMENT_node;
static MYNODE XMLSTARTCDATA_node, XMLENDCDATA_node;
static MYNODE XMLSTARTDOCT_node, XMLENDDOCT_node;
static MYNODE XMLUNPARSED_node;
static MYNODE XMLERROR_node, XMLROW_node, XMLCOL_node, XMLLEN_node;
static MYNODE XMLDEPTH_node, XMLENDDOCUMENT_node, XMLEVENT_node, XMLNAME_node;

/* note: the XMLPATH gencounter works differently.  It is maintained
   on a per-file basis.  If there is a mismatch in xmlpath_file or
   in the counter, we update the value. */
static MYNODE XMLPATH_node;
static const struct xml_state *xmlpath_file;

/* Arrays set by xml_get_record: */
static awk_array_t XMLATTR_array;


struct varinit {
	MYNODE *spec;
	const char *name;
};

#define ENTRY(VAR) { &VAR##_node, #VAR },

#define SET_ARRAY_ELEMENT(A,I,V) {	\
	if (!set_array_element(A,I,V))	\
		fatal(ext_id, _("set_array_element failed"));	\
}

/* These are all the scalar variables set by xml getline: */
static const struct varinit varinit[] = {
	/* read-write to support xmlwrite.awk and xmlcopy.awk: */
	ENTRY(XMLEVENT)
	ENTRY(XMLNAME)

	/* read-only: */
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

	/* XMLPATH should be last.  It is treated differently
	   in resetXMLvars: XMLPATH is updated only if it has changed. */
	ENTRY(XMLPATH)
};
#define NUM_SCALARS	(sizeof(varinit)/sizeof(varinit[0]))
#define NUM_RW		2	/* XMLEVENT and XMLNAME */

/* We can make the resetXMLvars function more elegant by defining RESET_ARRAY,
   but the code seems to be a few percent slower in that case, even if
   we compile with -funroll-loops. */
/* #define RESET_ARRAY */

#ifdef RESET_ARRAY
#define NUM_RESET	(NUM_SCALARS-1)
static MYNODE *scalars[NUM_SCALARS];
#endif

/* Forward function declarations: */
static int can_take_file(const awk_input_buf_t *iop);
static int take_control_of(awk_input_buf_t *iop);
static void xml_iop_close(awk_input_buf_t *iop);
static int xml_get_record(char **out, awk_input_buf_t *, int *errcode,
				char **rt_start, size_t *rt_len);
static void xml_load_vars(void);

/* Should this be 1 or -1? */
#define DEFAULT_XMLMODE	-1

static awk_ext_func_t func_table[] = {
};

static awk_bool_t
init_my_module(void)
{
	xml_load_vars();
	return awk_true;
}

static awk_bool_t (*init_func)(void) = init_my_module;
static const char ext_version[] = "XML extension: version 1.0";

dl_load_func(func_table, xml, "")

static awk_input_parser_t xml_parser = {
	"xml",
	can_take_file,
	take_control_of,
	NULL
};

static void
xml_load_vars(void)
{
	const struct varinit *vp;
	size_t i;
	awk_value_t ns;

	make_null_string(&ns);

	/* Register our file open handler */
	register_input_parser(&xml_parser);

	/* This initializes all of the XML* variables to "" */
	for (vp = varinit, i = 0; i < NUM_SCALARS; i++, vp++) {
		if (i < NUM_RW) {
			if (!gawk_varinit_scalar(vp->name, &ns, 0, &vp->spec->sc))
				fatal(ext_id, _("Cannot create XML reserved scalar variable `%s'."), vp->name);
		}
		else {
			if (!gawk_varinit_constant(vp->name, &ns, &vp->spec->sc))
				fatal(ext_id, _("Cannot create XML reserved scalar constant `%s'."), vp->name);
		}
		vp->spec->gen = curgen;
#ifdef RESET_ARRAY
		scalars[i] = vp->spec;
#endif
	}

	/* XMLCHARSET is a special case.  If the user has not set this,
	   we supply a default value.  It should be OK to initialize
	   to "", since "" seems to default to the codeset of the current
	   locale, but it is more clear to use nl_langinfo(CODESET). */
	{
		awk_value_t val;
		char *charset = nl_langinfo(CODESET);

		if (!gawk_varinit_scalar("XMLCHARSET",
					 make_string_malloc(charset,
					 		    strlen(charset),
							    &val),
					 0, &XMLCHARSET_node))
			fatal(ext_id, _("XML reserved scalar variable `%s' already used with incompatible type."), "XMLCHARSET");
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

/* value should not change between can_take_file and take_control_of */
static awk_value_t xmlmode;

static int
can_take_file(const awk_input_buf_t *iop __UNUSED)
{

	return sym_lookup_scalar(XMLMODE_node, AWK_NUMBER, &xmlmode) &&
	       ((int)(xmlmode.num_value) != 0);
}

static int
take_control_of(awk_input_buf_t *iop)
{
	static int warned = FALSE;
	awk_value_t xmlcharset;
	struct xml_state *xml;

	if (do_lint && !warned) {
		warned = TRUE;
		lintwarn(ext_id, _("`XMLMODE' is a gawk extension"));
	}

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
	xml->pathvers = 1;
	if (!(xml->space = XML_PullerIconv(xml->puller, " ", 1,
					       &xml->spacelen)))
		fatal(ext_id, _("cannot convert space to XMLCHARSET"));
	if (!(xml->slash = XML_PullerIconv(xml->puller, "/", 1,
					       &xml->slashlen)))
		fatal(ext_id, _("cannot convert slash to XMLCHARSET"));
	iop->opaque = xml;
	return 1;
}

static void
xml_iop_close(awk_input_buf_t *iop)
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
	{
		size_t i;
		for (i = 0; i < sizeof(XML(iop)->string_cache)/
				sizeof(XML(iop)->string_cache[0]); i++) {
			if (XML(iop)->string_cache[i]) {
				release_value(XML(iop)->string_cache[i]);
				XML(iop)->string_cache[i] = NULL;
			}
		}
	}
	free(XML(iop));
	iop->opaque = NULL;
}

static void
resetXMLvars_after(void)
{
	awk_value_t ns;

	make_null_string(&ns);

#ifdef RESET_ARRAY
	/* More elegant, but slower, even if compiled with -funroll-loops. */

	size_t i;

	for (i = 0; i < NUM_RW; i++) {
		if (scalars[i]->gen != curgen) {
			sym_update_scalar(scalars[i]->sc, &ns);
			scalars[i]->gen = 0;
		}
	}
	for ( ; i < NUM_RESET; i++) {
		if ((scalars[i]->gen != 0) && (scalars[i]->gen != curgen)) {
			sym_update_scalar(scalars[i]->sc, &ns);
			scalars[i]->gen = 0;
		}
	}

#else

#define RESET(FLD) \
	if ((FLD##_node.gen != 0) && (FLD##_node.gen != curgen)) {	\
		sym_update_scalar(FLD##_node.sc, &ns);	\
		FLD##_node.gen = 0;	\
	}

#define RESET_RW(FLD) \
	if (FLD##_node.gen != curgen) {	\
		sym_update_scalar(FLD##_node.sc, &ns);	\
		FLD##_node.gen = 0;	\
	}

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
	RESET_RW(XMLEVENT)
	RESET_RW(XMLNAME)
#undef RESET
#undef RESET_RW

#endif /* RESET_ARRAY */
}

static void
resetXMLvars_before(const struct xml_state *xmlstate, XML_PullerToken token)
{
	if (++curgen == 0)
		/* unlikely */
		curgen = 1;

	/* Note: this is very efficient for empty arrays, since deleting
	   an array resets the array to null status, and deleting a null
	   array is a no-op.  So there is no performance gain from avoiding
	   this call in the case of an empty array. */
	clear_array(XMLATTR_array);

	/* Copy the already allocated and initialized path into
	 * the XMLPATH variable.
	 */
#define RESET_XMLPATH(XMLSTATE) {	\
	awk_value_t _t;	\
	sym_update_scalar(XMLPATH_node.sc, \
			  make_string_malloc(XMLSTATE->path, XMLSTATE->pathlen,\
					     &_t)); \
	xmlpath_file = XMLSTATE; \
	XMLPATH_node.gen = XMLSTATE->pathvers; \
}

	/* must set before chopping ENDELEM, but do not set for STARTELEM,
	   since that must be updated after appending new element name */
	if (((token == NULL) || (token->kind != XML_PULLER_START_ELEMENT)) &&
	    ((xmlpath_file != xmlstate) ||
	     (XMLPATH_node.gen != xmlstate->pathvers)))
		RESET_XMLPATH(xmlstate)
}

/* update_xmlattr --- populate the XMLATTR array
 * Upon invokation, We assume that all previous entries in the
 * XMLATTR array are already deleted.
 */

static char *
update_xmlattr(XML_PullerToken tok, awk_input_buf_t *iop, int *cnt)
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
	xmlstate->pathvers++;
}

static inline void
chop_xmlpath(struct xml_state * xmlstate, XML_PullerToken token)
{
	/* Shorten the path. Let buffer keep its size. */
	xmlstate->pathlen -= token->name_len + xmlstate->slashlen;
	xmlstate->pathvers++;
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
set_xml_attr(awk_input_buf_t *iop, const char *attr, awk_value_t *value)
{
	awk_value_t idx;

	SET_ARRAY_ELEMENT(XMLATTR_array,
			  get_xml_string(XML(iop)->puller, attr, &idx), value)
}

/* get_xml_record --- read an XML token from IOP into out, return length of EOF, do not set RT */
static int
xml_get_record(char **out,		/* pointer to pointer to data */
	awk_input_buf_t *iop,		/* input IOP */
	int *errcode,			/* pointer to error variable */
	char **rt_start __UNUSED,	/* output: pointer to RT */
	size_t *rt_len)			/* output: length of RT */
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
	sym_update_scalar(n##_node.sc, make_string_no_malloc(s, s##_len, &_t));\
	s = NULL;	\
	n##_node.gen = curgen; \
}

#define SET_NUMBER(n, v) { 	\
	awk_value_t _t;	\
	sym_update_scalar(n##_node.sc, make_number(v, &_t));	\
	n##_node.gen = curgen; \
}

#define SET_EVENT(EV, N)	{	\
	awk_value_t _t;	\
	if (!XML(iop)->string_cache[N])	{ \
		awk_value_t _x; \
		create_value(get_xml_string(XML(iop)->puller, #EV, &_x), \
			     &(XML(iop)->string_cache[N])); \
	} \
	_t.val_type = AWK_VALUE_COOKIE; \
	_t.value_cookie = XML(iop)->string_cache[N]; \
	sym_update_scalar(XMLEVENT_node.sc, &_t); \
	XMLEVENT_node.gen = curgen; \
}

#define SET_NAME(EL) {	\
	awk_value_t _t;	\
	_t.val_type = AWK_SCALAR; \
	_t.scalar_cookie = EL##_node.sc; \
	sym_update_scalar(XMLNAME_node.sc, &_t); \
	XMLNAME_node.gen = curgen; \
}


	token = XML_PullerNext(XML(iop)->puller);
	resetXMLvars_before(XML(iop), token);
	*out = NULL;
	if (token == NULL) {
		if (XML(iop)->puller->status != XML_STATUS_OK) {
			if (XML(iop)->puller->error)
				SET_XMLSTR(XMLERROR, XML(iop)->puller->error)
			else {
				awk_value_t _t;
				static char s[] = "XML Puller: unknown error";
				sym_update_scalar(XMLERROR_node.sc,
				      make_string_malloc(s, sizeof(s)-1, &_t));
				XMLERROR_node.gen = curgen;
			}
			*errcode = -1;
			/* copy XMLERROR into ERRNO */
			{
				awk_value_t _t;
				sym_lookup_scalar(XMLERROR_node.sc, AWK_STRING,
						  &_t);
				set_ERRNO(_t.str_value.str);
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
		if (token->kind != XML_PULLER_START_ELEMENT)
			SET_NUMBER(XMLDEPTH, XML(iop)->depth)
		switch (token->kind) {
		case XML_PULLER_START_ELEMENT:
			append_xmlpath(XML(iop), token);
			RESET_XMLPATH(XML(iop))
			SET_EVENT(STARTELEM, 0)
			SET_XMLSTR(XMLSTARTELEM, token->name)
			SET_NAME(XMLSTARTELEM)
			*out = update_xmlattr(token, iop, &cnt);
			XML(iop)->depth++;
			SET_NUMBER(XMLDEPTH, XML(iop)->depth)
			break;
		case XML_PULLER_END_ELEMENT:
			chop_xmlpath(XML(iop), token);
			SET_EVENT(ENDELEM, 1)
			SET_XMLSTR(XMLENDELEM, token->name)
			SET_NAME(XMLENDELEM)
			XML(iop)->depth--;
			break;
		case XML_PULLER_CHARDATA:
			SET_EVENT(CHARDATA, 2)
			SET_NUMBER(XMLCHARDATA, 1)
			*out = token->u.d.data;
			cnt = token->u.d.data_len;
			break;
		case XML_PULLER_START_CDATA:
			SET_EVENT(STARTCDATA, 3)
			SET_NUMBER(XMLSTARTCDATA, 1)
			break;
		case XML_PULLER_END_CDATA:
			SET_EVENT(ENDCDATA, 4)
			SET_NUMBER(XMLENDCDATA, 1)
			break;
		case XML_PULLER_PROC_INST:
			SET_EVENT(PROCINST, 5)
			SET_XMLSTR(XMLPROCINST, token->name)
			SET_NAME(XMLPROCINST)
			*out = token->u.d.data;
			cnt = token->u.d.data_len;
			break;
		case XML_PULLER_COMMENT:
			SET_EVENT(COMMENT, 6)
			SET_NUMBER(XMLCOMMENT, 1)
			*out = token->u.d.data;
			cnt = token->u.d.data_len;
			break;
		case XML_PULLER_DECL:
			SET_EVENT(DECLARATION, 7)
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
			SET_EVENT(STARTDOCT, 8)
			SET_XMLSTR(XMLSTARTDOCT, token->name)
			SET_NAME(XMLSTARTDOCT)
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
			SET_EVENT(ENDDOCT, 9)
			SET_NUMBER(XMLENDDOCT, 1)
			break;
		case XML_PULLER_UNPARSED:
			SET_EVENT(UNPARSED, 10)
			SET_NUMBER(XMLUNPARSED, 1)
			*out = token->u.d.data;
			cnt = token->u.d.data_len;
			break;
		case XML_PULLER_END_DOCUMENT:
			SET_EVENT(ENDDOCUMENT, 11)
			SET_NUMBER(XMLENDDOCUMENT, 1)
			break;
		}
	}
#undef SET_NAME
#undef SET_EVENT
#undef SET_XMLSTR
#undef SET_NUMBER

	resetXMLvars_after();
	*rt_len = 0;	/* set RT to "" */
	return cnt;
}
