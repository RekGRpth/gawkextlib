#ifndef CSV_CONVERT_H__
#define CSV_CONVERT_H__

#include "strbuf.h"

strbuf_p csv_convert_record(const char* s, const char* fs, char comma, char quote, char options);

#endif
