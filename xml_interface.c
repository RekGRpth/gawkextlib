/*
 * xml_interface.c --- an interface between gawk and the xml_puller code
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2004 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "awk.h"
#include <xml_puller.h>

struct xml_state {
	XML_Puller puller;	/* XML parser */
	long depth;		/* current element depth */
	char *space;		/* space character in XMLCHARSET */
	size_t spacelen;	/* # of bytes in space character */
	char *attrnames;	/* buffer for attribute names */
	size_t bufsize;		/* length of attrnames buffer */
	NODE *string_cache[12];
};

#define XML(IOP) ((struct xml_state *)((IOP)->opaque))

/* Set by user: */
static NODE *XMLMODE_node;
static NODE *XMLCHARSET_node;

/* Scalars set by xml_get_record: */
static NODE *XMLSTARTELEM_node, *XMLENDELEM_node; 
static NODE *XMLCHARDATA_node, *XMLPROCINST_node, *XMLCOMMENT_node;
static NODE *XMLSTARTCDATA_node, *XMLENDCDATA_node;
static NODE *XMLVERSION_node, *XMLENCODING_node;
static NODE *XMLSTARTDOCT_node, *XMLENDDOCT_node;
static NODE *XMLDOCTPUBID_node, *XMLDOCTSYSID_node;
static NODE *XMLUNPARSED_node;
static NODE *XMLERROR_node, *XMLROW_node, *XMLCOL_node, *XMLLEN_node;
static NODE *XMLDEPTH_node, *XMLENDDOCUMENT_node, *XMLEVENT_node, *XMLNAME_node;

/* Arrays set by xml_get_record: */
static NODE *XMLATTR_node;

struct varinit {
	NODE **spec;
	const char *name;
};

#define ENTRY(VAR) { &VAR##_node, #VAR },

/* These are all the scalar variables set by xml getline: */
static const struct varinit varinit[] = {
	ENTRY(XMLSTARTELEM)
	ENTRY(XMLENDELEM)
	ENTRY(XMLCHARDATA)
	ENTRY(XMLPROCINST)
	ENTRY(XMLCOMMENT)
	ENTRY(XMLSTARTCDATA)
	ENTRY(XMLENDCDATA)
	ENTRY(XMLVERSION)
	ENTRY(XMLENCODING)
	ENTRY(XMLSTARTDOCT)
	ENTRY(XMLENDDOCT)
	ENTRY(XMLDOCTPUBID)
	ENTRY(XMLDOCTSYSID)
	ENTRY(XMLUNPARSED)
	ENTRY(XMLERROR)
	ENTRY(XMLROW)
	ENTRY(XMLCOL)
	ENTRY(XMLLEN)
	ENTRY(XMLDEPTH)
	ENTRY(XMLENDDOCUMENT)
	ENTRY(XMLEVENT)
	ENTRY(XMLNAME)

	/* XMLCHARSET should be last.  It is different in that we do
	   not reset the value when getline is called. */
	ENTRY(XMLCHARSET)
};
#define NUM_SCALARS	(sizeof(varinit)/sizeof(varinit[0]))
#define NUM_RESET	(NUM_SCALARS-1)

/* We can make the resetXMLvars function more elegant by defining RESET_ARRAY,
   but the code seems to be a few percent slower in that case, even if
   we compile with -funroll-loops. */
/* #define RESET_ARRAY */

#ifdef RESET_ARRAY
static NODE *scalars[NUM_SCALARS];
#endif

/* Forward function declarations: */
static void *xml_iop_open(IOBUF *iop);
static void xml_iop_close(IOBUF *iop);
static int xml_get_record(char **out, IOBUF *, int *errcode);
static NODE *xml_load_vars(void);

#ifndef BUILD_XMLGAWK
#define DYNAMIC_LOADING
#endif

#ifndef DYNAMIC_LOADING

#define DEFAULT_XMLMODE	0

void
xml_extension_init()
{
   register_deferred_variable("XMLMODE", xml_load_vars);
}

#else /* DYNAMIC_LOADING */

/* Should this be 1 or -1? */
#define DEFAULT_XMLMODE	-1

NODE *
dlload(NODE *tree, void *dl)
{
   xml_load_vars();
   return tmp_number((AWKNUM) 0);
}

#endif /* DYNAMIC_LOADING */


static NODE *
xml_load_vars()
{
	const struct varinit *vp;
	size_t i;

	/* Register our file open handler */
	register_open_hook(xml_iop_open);

	for (vp = varinit, i = 0; i < NUM_SCALARS; i++, vp++) {
		if ((*vp->spec = lookup(vp->name)) != NULL) {
#define N (*vp->spec)
			/* The name is already in use.  Check the type. */
			if (N->type == Node_var_new) {
				N->type = Node_var;
				N->var_value = Nnull_string;
			}
			else if (N->type != Node_var)
				fatal(_("XML reserved scalar variable `%s' already used with incompatible type."), vp->name);
#undef N
		}
		else
			*vp->spec = install((char *)vp->name,
					    node(Nnull_string, Node_var, NULL));
#ifdef RESET_ARRAY
		scalars[i] = *vp->spec;
#endif
	}

	if ((XMLATTR_node = lookup("XMLATTR")) != NULL) {
		if (XMLATTR_node->type == Node_var_new) {
			XMLATTR_node->type = Node_var_array;
			XMLATTR_node->var_array = NULL;
		}
		else if (XMLATTR_node->type != Node_var_array)
				fatal(_("XML reserved array variable `%s' already used with incompatible type."), "XMLATTR");
	}
	else
		XMLATTR_node = install((char *)"XMLATTR",
				       node(NULL, Node_var_array, NULL));


	/* If dynamically loaded, "XMLMODE" could exist already. */
	if ((XMLMODE_node = lookup("XMLMODE")) != NULL) {
		/* The name is already in use.  Check the type. */
		if (XMLMODE_node->type == Node_var_new) {
			XMLMODE_node->type = Node_var;
			XMLMODE_node->var_value = make_number(DEFAULT_XMLMODE);
		}
		else if (XMLMODE_node->type != Node_var)
			fatal(_("XML reserved scalar variable `%s' already used with incompatible type."), "XMLMODE");
	}
	else
		XMLMODE_node = install((char *)"XMLMODE",
				       node(make_number(DEFAULT_XMLMODE),
					    Node_var, NULL));
	return XMLMODE_node;
}

static void *
xml_iop_open(IOBUF *iop)
{
	static int warned = FALSE;
	int xmlmode;
	struct xml_state *xml;

	if ((do_lint || do_traditional) && ! warned) {
		warned = TRUE;
		lintwarn(_("`XMLMODE' is a gawk extension"));
	}
	if (do_traditional ||
	    ((xmlmode = (int) force_number(XMLMODE_node->var_value)) == 0))
		return NULL;
	
	emalloc(xml, struct xml_state *, sizeof(*xml), "xml_iop_open");
	memset(xml, 0, sizeof(*xml));

	/* Set function methods. */
	iop->get_record = xml_get_record;
	iop->close_func = xml_iop_close;

	xml->puller = XML_PullerCreate(
				iop->fd,
				XMLCHARSET_node->var_value->stptr,
				8192);
	if (xml->puller == NULL)
		fatal(_("cannot create XML puller"));
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
	if (xmlmode < 0)
		XML_PullerEnable (xml->puller,
				  XML_PULLER_END_DOCUMENT);
	emalloc(xml->attrnames, char *, xml->bufsize = 128,
		"iop_alloc");
	if (!(xml->space = XML_PullerIconv(xml->puller, " ", 1,
					       &xml->spacelen)))
		fatal(_("cannot convert space to XMLCHARSET"));
	return xml;
}

static void
xml_iop_close(IOBUF *iop)
{
	size_t i;

	XML_PullerFree(XML(iop)->puller);
	XML(iop)->puller = NULL;
	if (XML(iop)->attrnames) {
		free(XML(iop)->attrnames);
		XML(iop)->attrnames = NULL;
	}
	if (XML(iop)->space) {
		free(XML(iop)->space);
		XML(iop)->space = NULL;
	}
	for (i = 0; i < sizeof(XML(iop)->string_cache)/
			sizeof(XML(iop)->string_cache[0]); i++) {
		if (XML(iop)->string_cache[i]) {
			unref(XML(iop)->string_cache[i]);
			XML(iop)->string_cache[i] = NULL;
		}
	}
	free(XML(iop));
	iop->opaque = NULL;
}

static void
resetXMLvars(void)
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

#define RESET(FLD) { \
	if (FLD##_node->var_value != Nnull_string) {	\
		unref(FLD##_node->var_value);		\
		FLD##_node->var_value=Nnull_string;	\
  	}	\
}
	
	RESET(XMLSTARTELEM)
	RESET(XMLENDELEM)
	RESET(XMLCHARDATA)
	RESET(XMLPROCINST)
	RESET(XMLCOMMENT)
	RESET(XMLSTARTCDATA)
	RESET(XMLENDCDATA)
	RESET(XMLVERSION)
	RESET(XMLENCODING)
	RESET(XMLSTARTDOCT)
	RESET(XMLENDDOCT)
	RESET(XMLDOCTPUBID)
	RESET(XMLDOCTSYSID)
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

	if (XMLATTR_node->var_array != NULL)
		assoc_clear(XMLATTR_node);
}

/* update_xmlattr --- populate the XMLATTR array
 * Upon invokation, We assume that all previous entries in the
 * XMLATTR array are already deleted.
 */

static char *
update_xmlattr(XML_PullerToken tok, IOBUF *iop, int *cnt)
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
		NODE *tmpstr;
		NODE **aptr;

		/* First, copy attribute name into $0 buffer. */
		if (attrbytes != 0) {
			/* Insert space character. */
			memcpy(XML(iop)->attrnames+attrbytes,
			       XML(iop)->space, XML(iop)->spacelen);
			attrbytes += XML(iop)->spacelen;
		}
		memcpy(XML(iop)->attrnames+attrbytes, ap->name, ap->name_len);
		attrbytes += ap->name_len;

		/* This implements tmp_string combined with ALREADY_MALLOCED */
		tmpstr = make_str_node(ap->name, ap->name_len,
				       ALREADY_MALLOCED);
		ap->name = NULL;	/* Take ownership of the memory. */
		tmpstr->flags |= TEMP;

		/* Now enter this attribute into XMLATTR. */
		aptr = assoc_lookup(XMLATTR_node, tmpstr, FALSE);
		*aptr = make_str_node(ap->value, ap->value_len,
				      ALREADY_MALLOCED);
		ap->value = NULL;	/* Take ownership of the memory. */
		(*aptr)->flags |= MAYBE_NUM;
	}

	*cnt = attrbytes;
	return XML(iop)->attrnames;
}

static NODE *
get_xml_string(XML_Puller puller, const char *str)
{
	char *s;
	size_t s_len;

	if (!(s = XML_PullerIconv(puller, str, strlen(str), &s_len)))
		fatal(_("XML_PullerIconv failed to convert event string"));
	return make_str_node(s, s_len, ALREADY_MALLOCED);	\
}

static void
set_xml_attr(IOBUF *iop, const char *attr, NODE *value)
{
	NODE **aptr;
	NODE *tmpstr;

	tmpstr = get_xml_string(XML(iop)->puller, attr);
	tmpstr->flags |= TEMP;

	aptr = assoc_lookup(XMLATTR_node, tmpstr, FALSE);
	*aptr = dupnode(value);
	(*aptr)->flags |= MAYBE_NUM;
}

/* get_xml_record --- read an XML token from IOP into out, return length of EOF, do not set RT */
static int
xml_get_record(char **out,        /* pointer to pointer to data */
	IOBUF *iop,             /* input IOP */
	int *errcode)           /* pointer to error variable */
{
	int cnt = 0;
	XML_PullerToken token;

#define SET_XMLSTR(n, s) { \
	n##_node->var_value = make_str_node(s, s##_len, ALREADY_MALLOCED); \
	s = NULL;	\
}

#define SET_NUMBER(n, v) 	\
	n##_node->var_value = make_number((AWKNUM) (v))

#define SET_EVENT(EV,N)	\
	XMLEVENT_node->var_value = dupnode(XML(iop)->string_cache[N] ? XML(iop)->string_cache[N] : (XML(iop)->string_cache[N] = get_xml_string(XML(iop)->puller, #EV)))

#define SET_NAME(EL) XMLNAME_node->var_value = dupnode(EL##_node->var_value);

	token = XML_PullerNext(XML(iop)->puller);
	resetXMLvars();
	*out = NULL;
	if (token == NULL) {
		if (XML(iop)->puller->status != XML_STATUS_OK) {
			if (XML(iop)->puller->error)
				SET_XMLSTR(XMLERROR, XML(iop)->puller->error)
			else {
				static const char oops[] = "XML Puller: unknown error";
				XMLERROR_node->var_value = make_string(oops, strlen(oops));
			}
			if (errcode)
				*errcode = -1;
			ERRNO_node->var_value = dupnode(XMLERROR_node->var_value);
			SET_NUMBER(XMLROW, XML(iop)->puller->row);
			SET_NUMBER(XMLCOL, XML(iop)->puller->col);
			SET_NUMBER(XMLLEN, XML(iop)->puller->len);
			SET_NUMBER(XMLDEPTH, XML(iop)->depth);
		}
		iop->flag |= IOP_AT_EOF;
		cnt = EOF;
	} else {
		SET_NUMBER(XMLROW, token->row);
		SET_NUMBER(XMLCOL, token->col);
		SET_NUMBER(XMLLEN, token->len);
		SET_NUMBER(XMLDEPTH, XML(iop)->depth);
		switch (token->kind) {
		case XML_PULLER_START_ELEMENT:
			SET_EVENT(STARTELEM, 0);
			SET_XMLSTR(XMLSTARTELEM, token->name)
			SET_NAME(XMLSTARTELEM)
			*out = update_xmlattr(token, iop, &cnt);
			XML(iop)->depth++;
			XMLDEPTH_node->var_value->numbr++;
			break;
		case XML_PULLER_END_ELEMENT:
			SET_EVENT(ENDELEM, 1);
			SET_XMLSTR(XMLENDELEM, token->name)
			SET_NAME(XMLENDELEM)
			XML(iop)->depth--;
			break;
		case XML_PULLER_CHARDATA:
			SET_EVENT(CHARDATA, 2);
			SET_NUMBER(XMLCHARDATA, 1);
			*out = token->u.d.data;
			cnt = token->u.d.data_len;
			break;
		case XML_PULLER_START_CDATA:
			SET_EVENT(STARTCDATA, 3);
			SET_NUMBER(XMLSTARTCDATA, 1);
			break;
		case XML_PULLER_END_CDATA:
			SET_EVENT(ENDCDATA, 4);
			SET_NUMBER(XMLENDCDATA, 1);
			break;
		case XML_PULLER_PROC_INST:
			SET_EVENT(PROCINST, 5);
			SET_XMLSTR(XMLPROCINST, token->name)
			SET_NAME(XMLPROCINST)
			*out = token->u.d.data;
			cnt = token->u.d.data_len;
			break;
		case XML_PULLER_COMMENT:
			SET_EVENT(COMMENT, 6);
			SET_NUMBER(XMLCOMMENT, 1);
			*out = token->u.d.data;
			cnt = token->u.d.data_len;
			break;
		case XML_PULLER_DECL:
			SET_EVENT(DECLARATION, 7);
			if (token->name != NULL) {
				SET_XMLSTR(XMLVERSION, token->name)
				set_xml_attr(iop, "VERSION",
					     XMLVERSION_node->var_value);
			}
			if (token->u.d.data != NULL) {
				SET_XMLSTR(XMLENCODING, token->u.d.data)
				set_xml_attr(iop, "ENCODING",
					     XMLENCODING_node->var_value);
			}
			/* Ignore token->u.d.number ("standalone"). */
			break;
		case XML_PULLER_START_DOCT:
			SET_EVENT(STARTDOCT, 8);
			if (token->name != NULL) {
				SET_XMLSTR(XMLSTARTDOCT, token->name)
				SET_NAME(XMLSTARTDOCT)
			}
			if (token->u.d.pubid != NULL) {
				SET_XMLSTR(XMLDOCTPUBID, token->u.d.pubid)
				set_xml_attr(iop, "PUBLIC",
					     XMLDOCTPUBID_node->var_value);
			}
			if (token->u.d.data != NULL) {
				SET_XMLSTR(XMLDOCTSYSID, token->u.d.data)
				set_xml_attr(iop, "SYSTEM",
					     XMLDOCTSYSID_node->var_value);
			}
			/* Ignore token->u.d.number ("has_internal_subset"). */
			break;
		case XML_PULLER_END_DOCT:
			SET_EVENT(ENDDOCT, 9);
			SET_NUMBER(XMLENDDOCT, 1);
			break;
		case XML_PULLER_UNPARSED:
			SET_EVENT(UNPARSED, 10);
			SET_NUMBER(XMLUNPARSED, 1);
			*out = token->u.d.data;
			cnt = token->u.d.data_len;
			break;
		case XML_PULLER_END_DOCUMENT:
			SET_EVENT(ENDDOCUMENT, 11);
			SET_NUMBER(XMLENDDOCUMENT, 1);
			break;
		}
	}
#undef SET_NAME
#undef SET_EVENT
#undef SET_XMLSTR
#undef SET_NUMBER

	return cnt;
}
