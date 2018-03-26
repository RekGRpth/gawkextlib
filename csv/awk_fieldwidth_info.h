#ifndef AWK_FIELDWIDTH_INFO_H__
#define AWK_FIELDWIDTH_INFO_H__

#include "common_aux.h"

#if gawk_api_major_version >= 2

/* Function prototypes */
int awk_fieldwidth_info_init(awk_fieldwidth_info_t **info);
void awk_fieldwidth_info_start(awk_fieldwidth_info_t *info);
int awk_fieldwidth_info_add(awk_fieldwidth_info_t **info, size_t capacity, size_t skip, size_t len);
void awk_fieldwidth_info_free(awk_fieldwidth_info_t *info);

#endif

#endif
