/*
 * xml_puller.c --- routines for reading XML input with expat
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2003 the Free Software Foundation, Inc.
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <xml_puller.h>

#define XML_PullerAllocateAndCheck(SRC, LEN, NEWLEN, PULLER) \
	XML_PullerIconv((PULLER), (SRC), (LEN), (NEWLEN))

static void
set_row_col(XML_Puller puller, int *row, int *col)
{
  int thisrow = XML_GetCurrentLineNumber(puller->parser); /* starts at 1 */
  int thiscol = XML_GetCurrentColumnNumber(puller->parser); /* starts at 0 */

  *col = ((thisrow == 1) ? puller->prev_last_col+thiscol : thiscol+1);
  *row = puller->prev_last_row+thisrow-1;
}

static void XML_PullerSetError (XML_Puller puller)
{
  /* We want to report the first error, so we forbid overwriting. */
  if (puller->status == XML_STATUS_OK) {
    char buf[256];
    const char *es;

    puller->status = XML_STATUS_ERROR;
    set_row_col(puller, &puller->row, &puller->col);
    puller->len   = XML_GetCurrentByteCount(puller->parser);
    if ((es = XML_ErrorString(XML_GetErrorCode(puller->parser))) != NULL)
      snprintf(buf, sizeof(buf), "Expat XML Parser error: %s", es);
    else
      snprintf(buf, sizeof(buf),
	       "Expat XML Parser error: unknown error code %d",
	       XML_GetErrorCode(puller->parser));
    puller->error  = XML_PullerAllocateAndCheck(buf, strlen(buf),
    						&puller->error_len, puller);
  }
}

static void
internal_error(XML_Puller puller, const char *error_string)
{
  if (puller->status == XML_STATUS_OK) {
    puller->status = XML_STATUS_ERROR;
    puller->error  = XML_PullerAllocateAndCheck(error_string,
    						strlen(error_string),
    						&puller->error_len, puller);
    if (puller->parser) {
      set_row_col(puller, &puller->row, &puller->col);
      puller->len = XML_GetCurrentByteCount(puller->parser);
    }
    else {
      puller->row = puller->prev_last_row;
      puller->col = puller->prev_last_col;
      puller->len = 0;
    }
  }
}

char *
XML_PullerIconv (
  XML_Puller puller,
  const char * source,
  size_t length,
  size_t *new_length)
{
  char *dst;

  if (puller->converter) {
    size_t ibl = length;
    char * input = (char *) source;
    size_t maxoutlen = MB_LEN_MAX*length;
    char * output;
    size_t obl;

    if ((obl = maxoutlen) > puller->conv_buflen) {
      char *newbuf;
      if (!(newbuf = (char *)malloc(obl+puller->conv_buflen))) {
        internal_error(puller, "XML Puller: out of memory");
        return NULL;
      }
      free(puller->conv_buf);
      puller->conv_buf = newbuf;
      puller->conv_buflen += obl;
    }
    output = puller->conv_buf;
    /* Multibyte characters which are split upon buffers will
     * not occur because expat prevents it.
     */
    if ((iconv(puller->converter, &input, &ibl, &output, &obl) == (size_t)(-1))
        || (ibl != 0)) {
      internal_error(puller, "XML Puller: iconv failed");
      return NULL;
    }
    source = puller->conv_buf;
    length = maxoutlen-obl;
  }

  /* Leave 2 extra bytes because gawk does this in make_str_node.  Add NUL
     termination chars just in case. */
  if (!(dst = (char *)malloc(length+2))) {
    internal_error(puller, "XML Puller: out of memory");
    return NULL;
  }
  memcpy(dst, source, length);
  /* Just to be safe, add a NUL termination char to help out callers using
     8-bit encodings who are not careful to use the counted string length. */
  dst[length] = '\0';
  *new_length = length;
  return dst;
}

static XML_PullerToken
token_get_internal(XML_Puller puller, XML_PullerTokenKindType kind)
{
  XML_PullerToken tok;

  if (puller->status != XML_STATUS_OK)
    return NULL;

  if (puller->free_list) {
    tok = puller->free_list;
    puller->free_list = tok->next;
  }
  else if (!(tok = (XML_PullerToken)
		   malloc(sizeof(struct XML_PullerTokenDataType)))) {
    internal_error(puller, "XML Puller: out of memory");
    return NULL;
  }

  /* Initialize token fields: */
  tok->next = NULL;
  tok->kind = kind;
  tok->name = NULL;
  if (kind == XML_PULLER_START_ELEMENT)
    tok->u.a.attr = NULL;
  else
    tok->u.d.data = tok->u.d.pubid = NULL;
  return tok;
}

static inline void
token_release(XML_Puller puller, XML_PullerToken tok)
{
  tok->next = puller->free_list;
  puller->free_list = tok;
}

static inline void
token_enqueue(XML_Puller puller, XML_PullerToken tok)
{
  /* Append the token to the list of pending tokens. */
  if (puller->tok_head == NULL)
    puller->tok_head = tok;
  else
    puller->tok_tail->next = tok;
  puller->tok_tail = tok;
}

static void
flush_pending(XML_Puller puller)
{
  XML_PullerToken tok;

  if (!(tok = token_get_internal(puller, puller->cdata_kind))) {
    puller->cdata_len = 0;
    return;
  }
  tok->row = puller->row;
  tok->col = puller->col;
  tok->len = puller->len;
  tok->u.d.data = XML_PullerAllocateAndCheck(puller->cdata, puller->cdata_len,
					     &tok->u.d.data_len, puller);
  puller->cdata_len = 0;
  if (tok->u.d.data == NULL) {
    token_release(puller, tok);
    return;
  }
  token_enqueue(puller, tok);
}

static inline XML_PullerToken
token_get(XML_Puller puller, XML_PullerTokenKindType kind)
{
  XML_PullerToken tok;

  if (puller->cdata_len > 0)
    /* must flush pending cdata (or unparsed data) first. */
    flush_pending(puller);
  if (!(tok = token_get_internal(puller, kind)))
    return NULL;
  set_row_col(puller, &tok->row, &tok->col);
  tok->len  = XML_GetCurrentByteCount(puller->parser);
  return tok;
}

static inline void
token_insert_no_data(XML_Puller puller, XML_PullerTokenKindType kind)
{
  XML_PullerToken tok;

  if ((tok = token_get(puller, kind)) != NULL)
    token_enqueue(puller, tok);
}

static void
free_token_contents(XML_PullerToken tok)
{
  if (tok->name)
    free(tok->name);
  if (tok->kind == XML_PULLER_START_ELEMENT) {
    if (tok->u.a.attr) {
      size_t i;
      struct XML_PullerAttributeInfo *ap;

      for (i = 0, ap = tok->u.a.attr; i < tok->u.a.numattr; i++, ap++) {
	if (ap->name)
	  free(ap->name);
	if (ap->value)
	  free(ap->value);
      }
      free(tok->u.a.attr);
    }
  }
  else {
    if (tok->u.d.data)
      free(tok->u.d.data);
    if (tok->u.d.pubid)
      free(tok->u.d.pubid);
  }
}

void
XML_PullerFreeTokenData(XML_Puller puller, XML_PullerToken tok)
{
  free_token_contents(tok);
  token_release(puller, tok);
}


static void
start_element_handler(void *userData, const char *name, const char **attr)
{
  XML_Puller puller = (XML_Puller) userData;
  XML_PullerToken tok;
  size_t i;
  struct XML_PullerAttributeInfo *ap;
  int failed;

  puller->depth++;
  puller->elements++;
  if (!(tok = token_get(puller, XML_PULLER_START_ELEMENT)))
    return;

  if (!(tok->name = XML_PullerAllocateAndCheck(name, strlen(name),
					       &tok->name_len, puller))) {
    token_release(puller, tok);
    return;
  }

  for (tok->u.a.numattr = 0; attr[2*tok->u.a.numattr]; tok->u.a.numattr++)
    ;
  if (tok->u.a.numattr > 0) {
    if (!(tok->u.a.attr = (struct XML_PullerAttributeInfo *)
			  malloc(tok->u.a.numattr*
				 sizeof(struct XML_PullerAttributeInfo)))) {
      internal_error(puller, "XML Puller: out of memory");
      XML_PullerFreeTokenData(puller, tok);
      return;
    }
    failed = 0;
    /* Proceed through whole array to make sure all name and value pointers
       are initialized, even if there is a failure along the way.  That way,
       it will be safe to call XML_PullerFreeTokenData. */
    for (i = 0, ap = tok->u.a.attr; i < tok->u.a.numattr; i++, ap++) {
      if (!(ap->name = XML_PullerAllocateAndCheck(attr[2*i], strlen(attr[2*i]),
						  &ap->name_len, puller)))
	failed = 1;
      if (!(ap->value = XML_PullerAllocateAndCheck(attr[2*i+1],
						   strlen(attr[2*i+1]),
						   &ap->value_len, puller)))
	failed = 1;
    }
    if (failed) {
      XML_PullerFreeTokenData(puller, tok);
      return;
    }
  }
  token_enqueue(puller, tok);
}


static void
end_element_handler(void *userData, const char *name)
{
  XML_Puller puller = (XML_Puller) userData;
  XML_PullerToken tok;

  puller->depth--;
  if (!(tok = token_get(puller, XML_PULLER_END_ELEMENT)))
    return;
  if (!(tok->name = XML_PullerAllocateAndCheck(name, strlen(name),
					       &tok->name_len, puller))) {
    token_release(puller, tok);
    return;
  }
  token_enqueue(puller, tok);
}

/* Just append the data to the pending CDATA buffer. */
static void
add_pending(XML_Puller puller, XML_PullerTokenKindType kind,
	    const XML_Char *s, int len)
{
  if ((puller->cdata_len > 0) && (puller->cdata_kind != kind))
    flush_pending(puller);

  if (puller->cdata_len == 0) {
    puller->cdata_kind = kind;
    /* save starting position */
    set_row_col(puller, &puller->row, &puller->col);
    puller->len = XML_GetCurrentByteCount(puller->parser);
  }
  else
    puller->len += XML_GetCurrentByteCount(puller->parser);

  if (puller->cdata_len+len > puller->cdata_bufsize) {
    char * newbuf = (char *) realloc(puller->cdata,
				     puller->cdata_bufsize +
				     puller->cdata_len + len);
    if (newbuf == NULL) {
      puller->cdata_len = 0;
      internal_error(puller, "XML Puller: out of memory");
      return;
    }
    puller->cdata = newbuf;
    puller->cdata_bufsize += puller->cdata_len + len;
  }
  memcpy(puller->cdata + puller->cdata_len, s, len);
  puller->cdata_len += len;
}

static void
chardata_handler(void *userData, const XML_Char *s, int len)
{
  add_pending((XML_Puller) userData, XML_PULLER_CHARDATA, s, len);
}


static void
proc_inst_handler(void *userData, const XML_Char *target, const XML_Char *data)
{
  XML_Puller puller = (XML_Puller) userData;
  XML_PullerToken tok;

  if (!(tok = token_get(puller, XML_PULLER_PROC_INST)))
    return;

  if (!(tok->name = XML_PullerAllocateAndCheck(target, strlen(target),
					       &tok->name_len, puller))) {
    token_release(puller, tok);
    return;
  }
  if (!(tok->u.d.data = XML_PullerAllocateAndCheck(data, strlen(data),
						   &tok->u.d.data_len,
						   puller))) {
    XML_PullerFreeTokenData(puller, tok);
    return;
  }
  token_enqueue(puller, tok);
}


static void
comment_handler(void *userData, const XML_Char *data)
{
  XML_Puller puller = (XML_Puller) userData;
  XML_PullerToken tok;

  if (!(tok = token_get(puller, XML_PULLER_COMMENT)))
    return;

  if (!(tok->u.d.data = XML_PullerAllocateAndCheck(data, strlen(data),
						   &tok->u.d.data_len,
						   puller))) {
    token_release(puller, tok);
    return;
  }
  token_enqueue(puller, tok);
}


static void
start_cdata_handler(void *userData)
{
  token_insert_no_data((XML_Puller) userData, XML_PULLER_START_CDATA);
}


static void
end_cdata_handler(void *userData)
{
  token_insert_no_data((XML_Puller) userData, XML_PULLER_END_CDATA);
}


static void
decl_handler(void *userData, const XML_Char *version, const XML_Char *encoding, int standalone)
{
  XML_Puller puller = (XML_Puller) userData;
  XML_PullerToken tok;

  if (!(tok = token_get(puller, XML_PULLER_DECL)))
    return;

  if (version &&
      !(tok->name = XML_PullerAllocateAndCheck(version, strlen(version),
					       &tok->name_len, puller))) {
    token_release(puller, tok);
    return;
  }
  if (encoding &&
      !(tok->u.d.data = XML_PullerAllocateAndCheck(encoding, strlen(encoding),
						   &tok->u.d.data_len,
						   puller))) {
    XML_PullerFreeTokenData(puller, tok);
    return;
  }
  tok->u.d.number = standalone;
  token_enqueue(puller, tok);
}


static void
start_doct_handler(void *userData,
                   const XML_Char *doctypeName,
                   const XML_Char *sysid,
                   const XML_Char *pubid,
                   int   has_internal_subset)
{
  XML_Puller puller = (XML_Puller) userData;
  XML_PullerToken tok;

  if (!(tok = token_get(puller, XML_PULLER_START_DOCT)))
    return;

  if (doctypeName &&
      !(tok->name = XML_PullerAllocateAndCheck(doctypeName, strlen(doctypeName),
					       &tok->name_len, puller))) {
    token_release(puller, tok);
    return;
  }
  if (sysid &&
      !(tok->u.d.data = XML_PullerAllocateAndCheck(sysid, strlen(sysid),
						   &tok->u.d.data_len,
						   puller))) {
    XML_PullerFreeTokenData(puller, tok);
    return;
  }
  if (pubid &&
      !(tok->u.d.pubid = XML_PullerAllocateAndCheck(pubid, strlen(pubid),
						    &tok->u.d.pubid_len,
						    puller))) {
    XML_PullerFreeTokenData(puller, tok);
    return;
  }
  tok->u.d.number = has_internal_subset;
  token_enqueue(puller, tok);
}


static void
end_doct_handler(void *userData)
{
  token_insert_no_data((XML_Puller) userData, XML_PULLER_END_DOCT);
}


static void
unparsed_handler(void *userData, const XML_Char *s, int len)
{
  add_pending((XML_Puller) userData, XML_PULLER_UNPARSED, s, len);
}


XML_Puller XML_PullerCreate (int filedesc, char * encoding, int buffer_length)
{
  XML_Puller puller;

  if ((filedesc < 0) || (buffer_length < 1))
    return NULL;

  if (!(puller = (XML_Puller) calloc(1, sizeof(struct XML_PullerDataType))))
    return NULL;

  puller->input.bufsize = puller->input.read_size = buffer_length;
  puller->prev_last_row = 1;
  puller->prev_last_col = 1;
  puller->status	= XML_STATUS_OK;
  puller->filedesc	= filedesc;

  if (!(puller->input.buf = (char *) malloc(puller->input.bufsize))) {
    free(puller);
    return NULL;
  }

#define EXPAT_ENCODING	"utf-8"
  if (encoding && strcasecmp(encoding, EXPAT_ENCODING)) {
    puller->converter = iconv_open(encoding, EXPAT_ENCODING);
    if (puller->converter == (iconv_t) -1) {
      free(puller->input.buf);
      free(puller);
      return NULL;
    }

    /* Make sure converter works and discard leading BOM (byte order mark) */
    {
      char *tmp;
      size_t reslen;

      if (!(tmp = XML_PullerAllocateAndCheck("  ", 2, &reslen, puller))) {
	iconv_close(puller->converter);
	free(puller->input.buf);
	free(puller);
	return NULL;
      }
      free(tmp);
    }
  }
#undef EXPAT_ENCODING

  puller->parser = XML_ParserCreate(NULL);
  if (puller->parser == NULL) {
    iconv_close(puller->converter);
    free(puller->input.buf);
    free(puller);
    return NULL;
  }

  XML_SetUserData(puller->parser, (void *) puller);

  /* ==ST== */
  extern int unknownEncoding();
  XML_SetUnknownEncodingHandler(puller->parser, unknownEncoding, 0);

  return puller;
}

static void
free_token_list(XML_PullerToken tok, int free_contents)
{
  XML_PullerToken next;

  while (tok != NULL) {
    next = tok->next;
    if (free_contents)
      free_token_contents(tok);
    free(tok);
    tok = next;
  }
}

void XML_PullerFree(XML_Puller puller)
{
  if (puller == NULL)
    return;

  free(puller->input.buf);

  if (puller->converter != NULL)
    iconv_close(puller->converter);

  if (puller->parser != NULL)
    XML_ParserFree(puller->parser);

  free_token_list(puller->to_be_freed, 1);
  free_token_list(puller->tok_head, 1);
  free_token_list(puller->free_list, 0); /* token contents were already freed */

  free(puller->cdata);
  free(puller->conv_buf);
  free(puller->error);
  free(puller);
}


void XML_PullerEnable (XML_Puller puller,
                       XML_PullerTokenKindType enabledTokenKindSet)
{
  if (enabledTokenKindSet & XML_PULLER_START_ELEMENT)
    XML_SetStartElementHandler(puller->parser, start_element_handler);

  if (enabledTokenKindSet & XML_PULLER_END_ELEMENT)
    XML_SetEndElementHandler(puller->parser, end_element_handler);

  if (enabledTokenKindSet & XML_PULLER_CHARDATA)
    XML_SetCharacterDataHandler(puller->parser, chardata_handler);

  if (enabledTokenKindSet & XML_PULLER_START_CDATA)
    XML_SetStartCdataSectionHandler(puller->parser, start_cdata_handler);

  if (enabledTokenKindSet & XML_PULLER_END_CDATA)
    XML_SetEndCdataSectionHandler(puller->parser, end_cdata_handler);

  if (enabledTokenKindSet & XML_PULLER_PROC_INST)
    XML_SetProcessingInstructionHandler(puller->parser, proc_inst_handler);

  if (enabledTokenKindSet & XML_PULLER_COMMENT)
    XML_SetCommentHandler(puller->parser, comment_handler);

  if (enabledTokenKindSet & XML_PULLER_DECL)
    XML_SetXmlDeclHandler(puller->parser, decl_handler);

  if (enabledTokenKindSet & XML_PULLER_START_DOCT)
    XML_SetStartDoctypeDeclHandler(puller->parser, start_doct_handler);

  if (enabledTokenKindSet & XML_PULLER_END_DOCT)
    XML_SetEndDoctypeDeclHandler(puller->parser, end_doct_handler);

  if (enabledTokenKindSet & XML_PULLER_UNPARSED)
    XML_SetDefaultHandler(puller->parser, unparsed_handler);

  puller->enabledTokenKindSet |= enabledTokenKindSet;
}


void XML_PullerDisable (XML_Puller puller,
                        XML_PullerTokenKindType disabledTokenKindSet)
{
  if (disabledTokenKindSet & XML_PULLER_START_ELEMENT)
    XML_SetStartElementHandler(puller->parser, NULL);

  if (disabledTokenKindSet & XML_PULLER_END_ELEMENT)
    XML_SetEndElementHandler(puller->parser, NULL);

  if (disabledTokenKindSet & XML_PULLER_CHARDATA) {
    free(puller->cdata);
    puller->cdata = NULL;
    puller->cdata_len = 0;
    puller->cdata_bufsize = 0;
    XML_SetCharacterDataHandler(puller->parser, NULL);
  }

  if (disabledTokenKindSet & XML_PULLER_START_CDATA)
    XML_SetStartCdataSectionHandler(puller->parser, NULL);

  if (disabledTokenKindSet & XML_PULLER_END_CDATA)
    XML_SetEndCdataSectionHandler(puller->parser, NULL);

  if (disabledTokenKindSet & XML_PULLER_PROC_INST)
    XML_SetProcessingInstructionHandler(puller->parser, NULL);

  if (disabledTokenKindSet & XML_PULLER_COMMENT)
    XML_SetCommentHandler(puller->parser, NULL);

  if (disabledTokenKindSet & XML_PULLER_DECL)
    XML_SetXmlDeclHandler(puller->parser, NULL);

  if (disabledTokenKindSet & XML_PULLER_START_DOCT)
    XML_SetDefaultHandler(puller->parser, NULL);

  if (disabledTokenKindSet & XML_PULLER_END_DOCT)
    XML_SetDefaultHandler(puller->parser, NULL);

  if (disabledTokenKindSet & XML_PULLER_UNPARSED)
    XML_SetDefaultHandler(puller->parser, NULL);

  puller->enabledTokenKindSet &= ~disabledTokenKindSet;
}


XML_PullerToken XML_PullerNext (XML_Puller puller)
{
  XML_PullerToken tok = NULL;

  if (puller == NULL)
    return NULL;

  if (puller->to_be_freed) {
    XML_PullerFreeTokenData(puller, puller->to_be_freed);
    puller->to_be_freed = NULL;
  }

#define INPUT puller->input

  /* Read blocks of characters until there is at least one token. */
  while (puller->tok_head == NULL) {

    /* We check for previous errors as late as here because
     * we want to make sure that every correct token can be
     * read by the user if he chooses to do so.
     */
    if ((puller->status != XML_STATUS_OK) || (puller->filedesc < 0))
      return NULL;

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

    INPUT.new_bytes = read(puller->filedesc, INPUT.buf+INPUT.new_start,
			   MIN(INPUT.read_size,INPUT.bufsize-INPUT.new_start));

    if (INPUT.new_bytes < 0) {
      char buf[256];
      const char *estr;
      snprintf(buf, sizeof(buf), "XML Puller: read error: %s",
	       ((estr = strerror(errno)) ? estr : "unknown I/O error"));
      internal_error(puller, buf);
      puller->filedesc = -1;	/* Make sure no further reads are attempted. */
      break;
    }

    if (INPUT.new_bytes == 0) {
      enum XML_Status rc;

      /* EOF, just finish parsing. */
      puller->filedesc = -1;	/* Make sure no further reads are attempted. */
      rc = XML_Parse(puller->parser, NULL, 0, 1);
      if (puller->cdata_len > 0)
        flush_pending(puller);
      if (rc == XML_STATUS_ERROR)
        XML_PullerSetError(puller);
      else if (puller->enabledTokenKindSet & XML_PULLER_END_DOCUMENT)
	token_insert_no_data(puller, XML_PULLER_END_DOCUMENT);
      break;
    }

    while (XML_Parse(puller->parser, INPUT.buf+INPUT.new_start,
		     INPUT.new_bytes, 0) == XML_STATUS_ERROR) {
      long errloc = XML_GetCurrentByteIndex(puller->parser);
      if (!(puller->enabledTokenKindSet & XML_PULLER_END_DOCUMENT) ||
	  !((puller->elements > 0) && (puller->depth == 0)) ||
          (errloc < INPUT.doc_offset) ||
          (errloc > INPUT.doc_offset+INPUT.saved_bytes+INPUT.new_bytes)) {
        /* Not in multi-document mode, or not end of document, or
	   unrecoverable error (because we cannot get a reasonable
	   error location). */
	if (puller->cdata_len > 0)
	  flush_pending(puller);
        XML_PullerSetError(puller);
	break;
      }

      /* End of document was encountered.  Note that this will have the side
         effect of flushing any pending unparsed data. */
      token_insert_no_data(puller, XML_PULLER_END_DOCUMENT);

      /* Reset input buffer state appropriately to start new document. */
      INPUT.new_start = INPUT.saved_start+(errloc-INPUT.doc_offset);
      INPUT.new_bytes += INPUT.saved_bytes-(errloc-INPUT.doc_offset);
      INPUT.saved_start = INPUT.new_start;
      INPUT.saved_bytes = 0;
      INPUT.doc_offset = 0;

      /* Save position: */
      set_row_col(puller, &puller->prev_last_row, &puller->prev_last_col);

      /* Is it safe to use XML_ParserReset instead of XML_ParserFree
	 followed by XML_ParserCreate?  The header file says that
	 XML_ParserReset does not reset the values of ns and ns_triplets... */
      XML_ParserFree(puller->parser);

      /* Create a new parser. */
      if ((puller->parser = XML_ParserCreate(NULL)) == NULL) {
	internal_error(puller, "XML Puller: cannot create new Expat parser");
	break;
      }
      puller->elements = 0;
      XML_SetUserData(puller->parser, (void *) puller);
      XML_PullerEnable(puller, puller->enabledTokenKindSet);
    }
    if (puller->status != XML_STATUS_OK)
      break;

    if (!(puller->enabledTokenKindSet & XML_PULLER_END_DOCUMENT)) {
      /* No need to save any data */
      INPUT.doc_offset += INPUT.saved_bytes+INPUT.new_bytes;
      INPUT.saved_start = INPUT.saved_bytes = 0;
      INPUT.new_start = INPUT.new_bytes = 0;
      continue;
    }

    /* Update input buffer to reflect that the new data has been parsed. */
    INPUT.saved_bytes += INPUT.new_bytes;
    INPUT.new_start += INPUT.new_bytes;
    INPUT.new_bytes = 0;

    /* Delete parsed data from start of buffer. */
    {
      long last_ok = XML_GetCurrentByteIndex(puller->parser);
      /* the first last_ok bytes in the document have already been
         successfully processed */
      if ((last_ok < INPUT.doc_offset) ||
      	  (last_ok > INPUT.doc_offset+INPUT.saved_bytes)) {
	internal_error(puller, "XML Puller: corrupt parser state detected (document offset out of range)");
	break;
      }
      if (last_ok > INPUT.doc_offset) {
	 /* discard parsed data */
	 INPUT.saved_start += (last_ok-INPUT.doc_offset);
	 INPUT.saved_bytes -= (last_ok-INPUT.doc_offset);
	 INPUT.doc_offset = last_ok;
      }
    }

    /* Move saved data to start of buffer. */
    if (INPUT.saved_start > 0) {
      if (INPUT.saved_bytes > 0)
        memmove(INPUT.buf,INPUT.buf+INPUT.saved_start,INPUT.saved_bytes);
      INPUT.saved_start = 0;
      INPUT.new_start = INPUT.saved_start+INPUT.saved_bytes;
    }

    /* Make sure INPUT.buffer is large enough. */
    if (INPUT.bufsize < INPUT.new_start+INPUT.read_size) {
      char *newbuf;
      INPUT.bufsize += 2*(INPUT.new_start+INPUT.read_size-INPUT.bufsize)+128;
      if (!(newbuf = (char *)realloc(INPUT.buf,INPUT.bufsize))) {
	internal_error(puller, "XML Puller: out of memory");
        break;
      }
      INPUT.buf = newbuf;
    }
  }

#undef INPUT

  if (puller->tok_head != NULL) {
    /* Remove the token from the list of pending tokens and deliver it. */
    tok = puller->to_be_freed = puller->tok_head;
    puller->tok_head = puller->tok_head->next;
    tok->next = NULL;
  }

  return tok;
}


XML_PullerToken XML_PullerNext_m (XML_Puller puller)
{
  XML_PullerToken tok = XML_PullerNext(puller);

  puller->to_be_freed = NULL;

  return tok;
}


