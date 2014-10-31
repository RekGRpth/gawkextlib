/* put the GNU header here */
/* 2005-03-27 ST tramms */

#include <string.h>

#include <expat.h>

/* We intend to support older versions of expat. See expat.h. */
#ifndef XML_STATUS_OK
#define XML_STATUS_OK    1
#define XML_STATUS_ERROR 0
#endif

#include "encoding.h"
#include "xml_enc_handler.h"
#include "unused.h"

#include "xml_enc_tables.inc"
#include "xml_enc_registry.inc"

/* the following code is copied from Perl Expat.xs and was
   written originally by:
    Larry Wall <larry@wall.org>
    Clark Cooper <clark@coopercc.net>
*/

/*================================================================
** This is the function that expat calls to convert multi-byte sequences
** for external encodings. Each byte in the sequence is used to index
** into the current map to either set the next map or, in the case of
** the final byte, to get the corresponding Unicode scalar, which is
** returned.
*/

static int
convert_to_unicode(void *data, const char *seq) {
  Encinfo *enc = (Encinfo *) data;
  PrefixMap *curpfx;
  int count;
  int index = 0;

  for (count = 0; count < 4; count++) {
    unsigned char byte = (unsigned char) seq[count];
    unsigned char bndx;
    unsigned char bmsk;
    int offset;

    curpfx = &enc->prefixes[index];
    offset = ((int) byte) - curpfx->min;

    if (offset < 0)
      break;
    if (offset >= curpfx->len && curpfx->len != 0)
      break;

    bndx = byte >> 3;
    bmsk = 1 << (byte & 0x7);

    if (curpfx->ispfx[bndx] & bmsk) {
      index = enc->bytemap[curpfx->bmap_start + offset];
    }
    else if (curpfx->ischar[bndx] & bmsk) {
      return enc->bytemap[curpfx->bmap_start + offset];
    }
    else {
      break;
    }
  }
  return -1;
}  /* End convert_to_unicode */

int
unknownEncoding(void *opaque __UNUSED, const char *name, XML_Encoding *info)
{
  Encinfo *enc;
  int namelen;
  int i;
  char buff[42];
    
  namelen = strlen(name);
  if (namelen > 40)
    return XML_STATUS_ERROR;
    
  /* Make uppercase, do not use toupper() because the encoding
     of the XML instance may be completely different to the
     current locale
  */
  for (i = 0; i < namelen; i++) {
    char c = name[i];
    if (c >= 'a' && c <= 'z')
       c -= 'a' - 'A';
    buff[i] = c;
  }
  buff[i] = 0;

  /* search the encoding table */
  i = 0;
  enc = NULL;
  while (encs[i].name != 0) {
    if (strcmp(encs[i].name, buff) == 0) {
      enc = encs[i].enc;
      break;
    }
    i++;
  }
  if (enc == NULL) return XML_STATUS_ERROR; 

  /* install handler */
  memcpy(info->map, enc->firstmap, 256 * sizeof(int));
  if (enc->prefixes_size > 0) {
    info->data = (void *) enc;
    info->convert = convert_to_unicode;
  }
  else {
    info->data = NULL;
    info->convert = NULL;
  }
  info->release = NULL;

  return XML_STATUS_OK;
} /* End unknownEncoding */
