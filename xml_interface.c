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

/* Global: */
int XMLMODE;
NODE *XMLMODE_node;

/* Set by user: */
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
};
#define NUM_SCALARS	(sizeof(varinit)/sizeof(varinit[0]))

void
xml_init_vars()
{
	const struct varinit *vp;
	size_t i;

	for (vp = varinit, i = 0; i < NUM_SCALARS; i++, vp++)
		*vp->spec = install((char *)vp->name,
				    node(Nnull_string, Node_var, NULL));

	/* Special cases: */
	XMLMODE_node = install((char *)"XMLMODE",
			       node(make_number(0), Node_XMLMODE, NULL));
	XMLCHARSET_node = install((char *)"XMLCHARSET",
				  node(Nnull_string, Node_var, NULL));
	XMLATTR_node = install((char *)"XMLATTR",
			       node(NULL, Node_var_array, NULL));
}

/* set_XMLMODE --- set parsing mode */
void
set_XMLMODE()
{
	static int warned = FALSE;
 
	if ((do_lint || do_traditional) && ! warned) {
		warned = TRUE;
		lintwarn(_("`XMLMODE' is a gawk extension"));
	}
	if (do_traditional)
		XMLMODE = 0;
	else if ((XMLMODE_node->var_value->flags & NUMBER) != 0)
		XMLMODE = (int) force_number(XMLMODE_node->var_value);
	else if ((XMLMODE_node->var_value->flags & STRING) != 0) {
		/* arbitrary string, assume XML */
		XMLMODE = 1;
		warning("XMLMODE: arbitrary string value treated as \"1\"");
	} else
		XMLMODE = 0;            /* shouldn't happen */
}

void
xml_iop_open(IOBUF *iop)
{
	iop->xml.puller = XML_PullerCreate(
				iop->fd,
				XMLCHARSET_node->var_value->stptr,
				8192);
	if (iop->xml.puller == NULL)
		fatal(_("cannot create XML puller"));
	XML_PullerEnable (iop->xml.puller,
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
	iop->xml.depth = 0;
	if (XMLMODE < 0)
		XML_PullerEnable (iop->xml.puller,
				  XML_PULLER_END_DOCUMENT);
	emalloc(iop->xml.attrnames, char *, iop->xml.bufsize = 128,
		"iop_alloc");
	if (!(iop->xml.space = XML_PullerIconv(iop->xml.puller, " ", 1,
					       &iop->xml.spacelen)))
		fatal(_("cannot convert space to XMLCHARSET"));
}

void
xml_iop_close(IOBUF *iop)
{
	size_t i;

	XML_PullerFree(iop->xml.puller);
	iop->xml.puller = NULL;
	if (iop->xml.attrnames) {
		free(iop->xml.attrnames);
		iop->xml.attrnames = NULL;
	}
	if (iop->xml.space) {
		free(iop->xml.space);
		iop->xml.space = NULL;
	}
	for (i = 0; i < sizeof(iop->xml.string_cache)/
			sizeof(iop->xml.string_cache[0]); i++) {
		if (iop->xml.string_cache[i]) {
			unref(iop->xml.string_cache[i]);
			iop->xml.string_cache[i] = NULL;
		}
	}
}

static void
resetXMLvars(void)
{
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
			attrbytes += iop->xml.spacelen;
		attrbytes += ap->name_len;
	}
	if (attrbytes > iop->xml.bufsize) {
		/* Need a larger buffer. */
		free(iop->xml.attrnames);
		iop->xml.bufsize += 2*(attrbytes-iop->xml.bufsize)+32;
		emalloc(iop->xml.attrnames, char *, iop->xml.bufsize,
			"update_xmlattr");
	}

	/* Take each attribute and enter it into the XMLATTR array.
	 * attributes[i  ] is the pointer to the name  of the attribute.
	 * attributes[i+1] is the pointer to the value of the attribute.
	 *
	 * Also, populate iop->xml.attrnames with the concatenated attribute
	 * names separated by the space encoding in iop->xml.space.
	 */
	attrbytes = 0;
	for (i = 0, ap = tok->u.a.attr; i < tok->u.a.numattr; i++, ap++) {
		NODE *tmpstr;
		NODE **aptr;

		/* First, copy attribute name into $0 buffer. */
		if (attrbytes != 0) {
			/* Insert space character. */
			memcpy(iop->xml.attrnames+attrbytes,
			       iop->xml.space, iop->xml.spacelen);
			attrbytes += iop->xml.spacelen;
		}
		memcpy(iop->xml.attrnames+attrbytes, ap->name, ap->name_len);
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
	return iop->xml.attrnames;
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

	tmpstr = get_xml_string(iop->xml.puller, attr);
	tmpstr->flags |= TEMP;

	aptr = assoc_lookup(XMLATTR_node, tmpstr, FALSE);
	*aptr = dupnode(value);
	(*aptr)->flags |= MAYBE_NUM;
}

/* get_xml_record --- read an XML token from IOP into out, return length of EOF, do not set RT */
int
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
	XMLEVENT_node->var_value = dupnode(iop->xml.string_cache[N] ? iop->xml.string_cache[N] : (iop->xml.string_cache[N] = get_xml_string(iop->xml.puller, #EV)))

#define SET_NAME(EL) XMLNAME_node->var_value = dupnode(EL##_node->var_value);

	token = XML_PullerNext(iop->xml.puller);
	resetXMLvars();
	*out = NULL;
	if (token == NULL) {
		if (iop->xml.puller->status != XML_STATUS_OK) {
			if (iop->xml.puller->error)
				SET_XMLSTR(XMLERROR, iop->xml.puller->error)
			else {
				static const char oops[] = "XML Puller: unknown error";
				XMLERROR_node->var_value = make_string(oops, strlen(oops));
			}
			if (errcode)
				*errcode = -1;
			ERRNO_node->var_value = dupnode(XMLERROR_node->var_value);
			SET_NUMBER(XMLROW, iop->xml.puller->row);
			SET_NUMBER(XMLCOL, iop->xml.puller->col);
			SET_NUMBER(XMLLEN, iop->xml.puller->len);
			SET_NUMBER(XMLDEPTH, iop->xml.depth);
/*
			warning(_("XML error: %s at line %d\n"),
				iop->xml.puller->error,
				iop->xml.puller->line);
*/
		}
		iop->flag |= IOP_AT_EOF;
		cnt = EOF;
	} else {
		SET_NUMBER(XMLROW, token->row);
		SET_NUMBER(XMLCOL, token->col);
		SET_NUMBER(XMLLEN, token->len);
		SET_NUMBER(XMLDEPTH, iop->xml.depth);
		switch (token->kind) {
		case XML_PULLER_START_ELEMENT:
			SET_EVENT(STARTELEM, 0);
			SET_XMLSTR(XMLSTARTELEM, token->name)
			SET_NAME(XMLSTARTELEM)
			*out = update_xmlattr(token, iop, &cnt);
			iop->xml.depth++;
			XMLDEPTH_node->var_value->numbr++;
			break;
		case XML_PULLER_END_ELEMENT:
			SET_EVENT(ENDELEM, 1);
			SET_XMLSTR(XMLENDELEM, token->name)
			SET_NAME(XMLENDELEM)
			iop->xml.depth--;
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
