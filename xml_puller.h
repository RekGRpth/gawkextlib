/*
 * xml_puller.h --- a pull-parser API for reading XML input
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

#ifndef _XML_PULLER_H
#define _XML_PULLER_H 1

#include <stdio.h>
#include <iconv.h>

/* We want to use expat for parsing XML files.
 * It is quite a good guess that expat might
 * be installed as xmlparser.h on systems which
 * have an older GNU compiler or none at all.
 * But not on RedHat 7.2, which has gcc 2.96
 * and expat.h.
 */
#if __GNUC__ < 3
 #include <xmlparse.h>
#else
 #include <expat.h>
#endif 

/* We intend to support older versions of expat. See expat.h. */
#ifndef XML_STATUS_OK
#define XML_STATUS_OK    1
#define XML_STATUS_ERROR 0
#endif


/* These are the kinds of token which are returned by
 * the XML pull parser. By default, no kind of token
 * will be returned. You have to switch on each kind
 * you want to see by setting the respective bit in
 * the mask. You can switch these bits on and off during
 * the parsing process at any time.
 * The expat callback functions are not mapped one-to-one
 * on the token kinds. For example, all occurences of
 * character data are accumulated before they are returned
 * in one token.
 */
typedef int XML_PullerTokenKindType;
#define  XML_PULLER_START_ELEMENT 1
#define  XML_PULLER_END_ELEMENT   2
#define  XML_PULLER_CHARDATA      4
#define  XML_PULLER_START_CDATA   8
#define  XML_PULLER_END_CDATA    16 
#define  XML_PULLER_PROC_INST    32 
#define  XML_PULLER_COMMENT      64
#define  XML_PULLER_DECL        128
#define  XML_PULLER_START_DOCT  256
#define  XML_PULLER_END_DOCT    512
#define  XML_PULLER_UNPARSED   1024


/* The pull parser returns pointers to tokens.
 * This is the contents of a token. When such
 * a pointer is returned to you, remember that
 * its contents was properly allocated and has
 * to be freed some time later. By default, you
 * do not have to care about memory allocation;
 * the pull parser will do it for you. But this
 * comfort has a price: The content of one token
 * will only be valid until you ask for the next
 * token. If you want a token's content to survive
 * the succeeding tokens, you can let a different
 * function of the pull parser return this token
 * to you (see below).
 */
struct XML_PullerTokenDataType {
  XML_PullerTokenKindType kind;
  char * name;
  char * data;
  char ** attr;
  int number;
  struct XML_PullerTokenDataType *next;

  /* The coordinates and the length of the current token
   * in the source file.
   */
  int row;
  int col;
  int len;
};
typedef struct XML_PullerTokenDataType * XML_PullerToken;


/* This is the pull parser's content. You have to
 * create one for each file you are going to process.
 * Create as many as needed and none of them will
 * interfere with another one's data. Do not change
 * any of the fields in this struct.
 * If, at any time, an error occurs, the "status" field
 * will be set to XML_STATUS_ERROR, the "row" field
 * contains the number of the line in the source file
 * where the error occured and the "error" field contains
 * an explanatory text of the error. In case of flawless
 * completion of file reading, the "status" field will
 * be set to XML_STATUS_OK.
 */
struct XML_PullerDataType
{
  int    filedesc;
  char * buffer;
  int    buffer_length;
  iconv_t converter;
  XML_Parser parser;
  volatile XML_PullerToken tok_head;
  volatile XML_PullerToken tok_tail;
  XML_PullerToken to_be_freed;
  char * cdata;
  int    cdata_len;
  int status;
  int row;
  int col;
  int len;
  const char * error;
};
typedef struct XML_PullerDataType * XML_Puller;


/* These are the functions for creating, destroying and
 * using the XML pull parser. You will need one parser
 * for each file to process. If you pass 0 as the
 * file descriptor, standard input will be used. File
 * descriptors are not  closed upon destruction of the
 * pull parser. For the characters returned by the XML
 * pull parser, you can choose among all the charactor
 * encodings supported by your local iconv library.
 * Just pass the name of the encoding or pass NULL if
 * you do not care and UTF-8 is good enough.
 * Each pull parser is associated with a character buffer
 * whose length is determined at time of creation. Remember
 * that this buffer length influences the blocking behavior
 * of the parser when reading the file. If you need a parser
 * with minimum lookahead, use a buffer_length of 1.
 */
XML_Puller       XML_PullerCreate    (int filedesc, char * encoding, int buffer_length);

/* Each XML pull parser object has to be destroyed after use,
 * in order to properly free all ressources allocated to the
 * parser instance.
 */ 
void             XML_PullerFree      (XML_Puller puller);

/* This function is probably most important to you.
 * No line will be read from the XML file, unless you invoke it.
 * It reads the next token from the XML file and passes
 * it to you. Remember that the token is actually a pointer
 * to a chunk of memory which will be freed upon next invokation
 * of this function. So, do not dereference one token after
 * having called for its successor.
 * The end of the token stream will be indicated by a
 * NULL pointer being returned. A NULL pointer is also
 * returned in case of an error while reading. So, you
 * have to check the above mentioned status indicators
 * for detecting the presence and the cause of an error.
 * Upon first occurence of an error, all subsequent invokations
 * of this functions will return NULL and the status of
 * the XML puller will remain unchanged for you to find
 * the cause of the first error.
 */
XML_PullerToken  XML_PullerNext      (XML_Puller puller);

/* Use this function if you want the XML pull parser not
 * to free a specific token at any time. Remember that you are
 * responsible for proper deallocation of the token by
 * invokation of a function below. You can freely mix
 * invokations of this function and the previous one.
 * The result will be that some tokens are freed automatically
 * and some have to be freed by you.
 */
XML_PullerToken  XML_PullerNext_m    (XML_Puller puller);

/* This is the function that actually frees the ressources
 * allocated to a token.
 */
void             XML_PullerFreeToken (XML_PullerToken token);

/* Use the following two functions to switch on or off the
 * production of specific kinds of tokens. If you do not
 * invoke the first function, you will never receive a
 * token at all from your parser (although the XML file
 * is actually read properly).
 */
void             XML_PullerEnable    (XML_Puller puller,
                                      XML_PullerTokenKindType enabledTokenKindSet);
void             XML_PullerDisable   (XML_Puller puller,
                                      XML_PullerTokenKindType disabledTokenKindSet);

#endif /* _XML_PULLER_H */

