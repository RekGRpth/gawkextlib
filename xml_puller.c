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
#include <xml_puller.h>


static void XML_PullerSetError (XML_Puller puller)
{
  /* We want to report the first error, so we forbid overwriting. */
  if (puller->status == XML_STATUS_OK) {
    puller->status = XML_STATUS_ERROR;
    puller->row   = XML_GetCurrentLineNumber(puller->parser);
    puller->col   = XML_GetCurrentColumnNumber(puller->parser) + 1;
    puller->len   = XML_GetCurrentByteCount(puller->parser);
    puller->error  = XML_ErrorString(XML_GetErrorCode(puller->parser));
  }
}


static char * XML_PullerAllocateAndCheck (
  const char * source,
  int length,
  int * new_length,
  iconv_t converter)
{
  size_t ibl    =     length;
  size_t obl    = 4 * length;
  char * retval = (char *) malloc(obl);
  char * input  = (char *) source;
  char * output =     retval;

  if (retval != NULL) {
    int actual_length = length;
    if (converter == NULL) {
      memcpy(retval, source, length);
    } else {
      /* Multibyte characters which are split upon buffers will
       * not occur because expat prevents it.
       */
      if (iconv(converter, & input, &ibl, & output, & obl) == (size_t)(-1)) {
        switch (errno) {
          case E2BIG:  /* insufficient memory           */
            break;
          case EILSEQ: /* invalid multibyte sequence    */
            break;
          case EINVAL: /* incomplete multibyte sequence */
            break;
          default:
            break;
        }
      } else {
        actual_length = 4 * length - obl;
      }
    }
    retval = (char *) realloc(retval, actual_length); 
    if (new_length != NULL)
      * new_length = actual_length;
  }
  return retval;
}


static void XML_PullerInsertTokenData (
  void * userData,
  XML_PullerTokenKindType kind,
  const char * name,
  const char * data,
  const char ** attr,
  int number
)
{
  XML_Puller puller = (XML_Puller) userData;
  XML_PullerToken tok;
  int i;
  int num_attr;

  if (puller->status != XML_STATUS_OK)
    return;

  if (kind == XML_PULLER_CHARDATA) {
    if (data == NULL) {
      /* The CDATA token is complete. Enlist token below in the switch statement. */
    } else {
      /* This is some more CDATA to be appended to puller->cdata. */
      char * append = (char *) realloc(puller->cdata, puller->cdata_len + number);
      if (append == NULL) {
        free(puller->cdata);
        puller->cdata = NULL;
        puller->cdata_len = 0;
        return;
      }
      memcpy(append + puller->cdata_len, data, number);
      puller->cdata = append;
      puller->cdata_len += number;
      return;
    }
  } else if (puller->cdata != NULL) {
    /* The token to be enlisted is not a CDATA token.
     * Before we can enlist it, we must enlist the pending CDATA.
     */
    XML_PullerInsertTokenData (userData, XML_PULLER_CHARDATA, NULL, NULL, NULL, 0);
  }

  tok = (XML_PullerToken) malloc(sizeof(struct XML_PullerTokenDataType));
  if (tok == NULL) {
    XML_PullerSetError(puller);
    return;
  }

  tok->next = NULL;
  tok->kind = kind;
  tok->row  = XML_GetCurrentLineNumber(puller->parser);
  tok->col  = XML_GetCurrentColumnNumber(puller->parser) + 1;
  tok->len  = XML_GetCurrentByteCount(puller->parser);

  switch (kind) {
    case XML_PULLER_START_ELEMENT:
      for (num_attr = 0; attr[2*num_attr] != NULL ; num_attr ++)
        ;
      tok->attr = (char **) malloc((2*num_attr+1) * sizeof(char *));
      if (tok->attr == NULL) {
        free(tok);
        XML_PullerSetError(puller);
        return;
      }
      for (i = 0; i<num_attr; i++) {
        tok->attr[2*i  ] = XML_PullerAllocateAndCheck(
                             attr[2*i  ], strlen(attr[2*i  ])+1, NULL, puller->converter);
        tok->attr[2*i+1] = XML_PullerAllocateAndCheck(
                             attr[2*i+1], strlen(attr[2*i+1])+1, NULL, puller->converter);
      }
      tok->attr[2*num_attr] = NULL;
    case XML_PULLER_END_ELEMENT:
      tok->name = XML_PullerAllocateAndCheck(name, strlen(name)+1, NULL, puller->converter);
      break;

    case XML_PULLER_CHARDATA:
      /* Allocating once more is not a good idea, but we do it because of iconv. */
      tok->data = XML_PullerAllocateAndCheck(puller->cdata, puller->cdata_len,
                                             & tok->number, puller->converter);
      free(puller->cdata);
      puller->cdata = NULL;
      puller->cdata_len = 0;
      if (tok->data == NULL) {
        free(tok);
        return;
      }
      break;

    case XML_PULLER_START_CDATA:
      break;

    case XML_PULLER_END_CDATA:
      break;

    case XML_PULLER_PROC_INST:
      tok->name = XML_PullerAllocateAndCheck(name, strlen(name)+1, NULL, puller->converter);
      if (tok->name == NULL) {
        free(tok);
        return;
      }
      tok->data = XML_PullerAllocateAndCheck(data, strlen(data)+1, NULL, puller->converter);
      if (tok->data == NULL) {
        free(tok->name);
        free(tok);
        return;
      }
      break;

    case XML_PULLER_COMMENT:
      tok->data = XML_PullerAllocateAndCheck(data, strlen(data)+1, NULL, puller->converter);
      if (tok->data == NULL) {
        free(tok);
        return;
      }
      break;

    case XML_PULLER_DECL:
      tok->name = (char *) name;
      tok->data = (char *) data;
      tok->number = number;
      if (tok->name != NULL) {
        tok->name = XML_PullerAllocateAndCheck(name, strlen(name)+1, NULL, puller->converter);
        if (tok->name == NULL) {
          free(tok);
          return;
        }
      }
      if (tok->data != NULL) {
        tok->data = XML_PullerAllocateAndCheck(data, strlen(data)+1, NULL, puller->converter);
        if (tok->data == NULL) {
          free(tok->name);
          free(tok);
          return;
        }
      }
      break;

    case XML_PULLER_START_DOCT:
      tok->name = (char *) name;
      tok->data = (char *) data;
      tok->attr = (char **) attr;
      if (tok->name != NULL) {
        tok->name = XML_PullerAllocateAndCheck(name, strlen(name)+1, NULL, puller->converter);
        if (tok->name == NULL) {
          free(tok);
          return;
        }
      }
      if (tok->data != NULL) {
        tok->data = XML_PullerAllocateAndCheck(data, strlen(data)+1, NULL, puller->converter);
        if (tok->data == NULL) {
          free(tok->name);
          free(tok);
          return;
        }
      }
      if (tok->attr != NULL) {
        tok->attr = (char **) XML_PullerAllocateAndCheck((char *)attr, strlen((char *)attr)+1, NULL, puller->converter);
        if (tok->attr == NULL) {
          free(tok->data);
          free(tok->name);
          free(tok);
          return;
        }
      }
      break;

    case XML_PULLER_END_DOCT:
      break;

    case XML_PULLER_UNPARSED:
      tok->data = (char *) data;
      tok->number = number;
      if (tok->data != NULL) {
        tok->data = XML_PullerAllocateAndCheck(data, number, NULL, NULL);
        if (tok->data == NULL) {
          free(tok);
          return;
        }
      }
      break;
  }

  /* Append the token to the list of pending tokens. */
  if (puller->tok_head == NULL)
  {
    puller->tok_head = tok;
  }
  else
  {
    puller->tok_tail->next = tok;
  }

  puller->tok_tail = tok;
}


void XML_PullerFreeTokenData(XML_PullerToken tok)
{
  int i;

  if (tok == NULL)
    return;

  XML_PullerFreeTokenData(tok->next);

  switch (tok->kind) {
    case XML_PULLER_START_ELEMENT:
      for (i=0; tok->attr[i] != NULL; i++)
        free(tok->attr[i]);
      free(tok->attr);
    case XML_PULLER_END_ELEMENT:
      free(tok->name);
      break;

    case XML_PULLER_CHARDATA:
      free(tok->data);
      break;

    case XML_PULLER_START_CDATA:
      break;

    case XML_PULLER_END_CDATA:
      break;

    case XML_PULLER_PROC_INST:
      free(tok->name);
      free(tok->data);
      break;

    case XML_PULLER_COMMENT:
      free(tok->data);
      break;

    case XML_PULLER_DECL:
      free(tok->name);
      free(tok->data);
      break;

    case XML_PULLER_START_DOCT:
      free(tok->name);
      free(tok->data);
      free((char *) tok->attr);
      break;

    case XML_PULLER_END_DOCT:
      break;

    case XML_PULLER_UNPARSED:
      free(tok->data);
      break;

  }

  free(tok);
}


static void
start_element_handler(void *userData, const char *name, const char **attr)
{
  XML_PullerInsertTokenData (userData, XML_PULLER_START_ELEMENT, name, NULL, attr, 0);
}


static void
end_element_handler(void *userData, const char *name)
{
  XML_PullerInsertTokenData (userData, XML_PULLER_END_ELEMENT, name, NULL, NULL, 0);
}


static void
chardata_handler(void *userData, const XML_Char *s, int len)
{
  XML_PullerInsertTokenData (userData, XML_PULLER_CHARDATA, NULL, s, NULL, len);
}


static void
proc_inst_handler(void *userData, const XML_Char *target, const XML_Char *data)
{
  XML_PullerInsertTokenData (userData, XML_PULLER_PROC_INST, target, data, NULL, 0);
}


static void
comment_handler(void *userData, const XML_Char *data)
{
  XML_PullerInsertTokenData (userData, XML_PULLER_COMMENT, NULL, data, NULL, 0);
}


static void
start_cdata_handler(void *userData)
{
  XML_PullerInsertTokenData (userData, XML_PULLER_START_CDATA, NULL, NULL, NULL, 0);
}


static void
end_cdata_handler(void *userData)
{
  XML_PullerInsertTokenData (userData, XML_PULLER_END_CDATA, NULL, NULL, NULL, 0);
}


static void
decl_handler(void *userData, const XML_Char *version, const XML_Char *encoding, int standalone)
{
  XML_PullerInsertTokenData (userData, XML_PULLER_DECL, version, encoding, NULL, standalone);
}


static void
start_doct_handler(void *userData,
                   const XML_Char *doctypeName,
                   const XML_Char *sysid,
                   const XML_Char *pubid,
                   int   has_internal_subset)
{
  XML_PullerInsertTokenData (userData, XML_PULLER_START_DOCT, doctypeName, sysid,
                             (const char **) pubid, has_internal_subset);
}


static void
end_doct_handler(void *userData)
{
  XML_PullerInsertTokenData (userData, XML_PULLER_END_DOCT, NULL, NULL, NULL, 0);
}


static void
unparsed_handler(void *userData, const XML_Char *s, int len)
{
  XML_PullerInsertTokenData (userData, XML_PULLER_UNPARSED, NULL, s, NULL, len);
}


XML_Puller XML_PullerCreate (int filedesc, char * encoding, int buffer_length)
{
  XML_Puller puller;

  if (buffer_length < 1)
    return NULL;

  puller = (XML_Puller) malloc(sizeof(struct XML_PullerDataType));
  if (puller == NULL)
    return NULL;

  puller->buffer        = NULL;
  puller->buffer_length = buffer_length;
  puller->converter     = NULL;
  puller->to_be_freed   = NULL;
  puller->cdata         = NULL;
  puller->cdata_len     = 0;
  puller->tok_head      = NULL;
  puller->tok_tail      = NULL;
  puller->status        = XML_STATUS_OK;
  puller->row           = 0;
  puller->col           = 0;
  puller->len           = 0;
  puller->error         = NULL;
  puller->filedesc      = filedesc;

  if (puller->filedesc < 0) {
    free(puller);
    return NULL;
  }

  puller->buffer = (char *) malloc(puller->buffer_length);
  if (puller->buffer == NULL) {
    free(puller);
    return NULL;
  }

  if (encoding != NULL) {
    puller->converter = iconv_open(encoding, "utf-8");
    if (puller->converter == (iconv_t) -1) {
      free(puller->buffer);
      free(puller);
      return NULL;
    }
  }

  puller->parser = XML_ParserCreate(NULL);
  if (puller->parser == NULL) {
    iconv_close(puller->converter);
    free(puller->buffer);
    free(puller);
    return NULL;
  }

  XML_SetUserData(puller->parser, (void *) puller);

  return puller;
}


void XML_PullerFree(XML_Puller puller)
{
  if (puller == NULL)
    return;

  free(puller->buffer);

  if (puller->converter != NULL)
    iconv_close(puller->converter);

  if (puller->parser != NULL)
    XML_ParserFree(puller->parser);

  XML_PullerFreeTokenData(puller->to_be_freed);
  XML_PullerFreeTokenData(puller->tok_head);

  free(puller->cdata);
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

}


XML_PullerToken XML_PullerNext (XML_Puller puller)
{
  XML_PullerToken tok = NULL;

  if (puller == NULL)
    return NULL;

  XML_PullerFreeTokenData(puller->to_be_freed);
  puller->to_be_freed = NULL;

  /* Read blocks of characters until there is at least one token. */
  while (puller->tok_head == NULL) {
    int len;

    /* We check for previous errors as late as here because
     * we want to make sure that every correct token can be
     * read by the user if he chooses to do so.
     */
    if (puller->status != XML_STATUS_OK)
      return NULL;

    len = read(puller->filedesc, puller->buffer, puller->buffer_length);

    if (len < 0)
      break;

    if (XML_Parse(puller->parser, puller->buffer, len, len == 0) == XML_STATUS_ERROR) {
      XML_PullerSetError(puller);
      break;
    }

    if (len == 0)
      break;
  }

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


