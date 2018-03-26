#include "awk_fieldwidth_info.h"

#if gawk_api_major_version >= 2

#define INITSIZE 10

/* Allocate initial space */
int awk_fieldwidth_info_init(awk_fieldwidth_info_t **info) {
    emalloc(*info, awk_fieldwidth_info_t *, awk_fieldwidth_info_size(INITSIZE), "csv_take_control_of");
    (*info)->nf = 0;
    return INITSIZE;
}

/* Reuse without deallocating */
void awk_fieldwidth_info_start(awk_fieldwidth_info_t *info) {
    info->nf = 0;
}

/* Add entry, reallocate if grows beyond current capacity */
int awk_fieldwidth_info_add(awk_fieldwidth_info_t **info, size_t capacity, size_t skip, size_t len) {
    if ((*info)->nf >= capacity) {
        capacity *= 2;
        erealloc(*info, awk_fieldwidth_info_t *, awk_fieldwidth_info_size(capacity), "field_info_add");
    }
    (*info)->fields[(*info)->nf].skip = skip;
    (*info)->fields[(*info)->nf].len = len;
    (*info)->nf++;
    return capacity;
}

/* Deallocate storage */
void awk_fieldwidth_info_free(awk_fieldwidth_info_t *info) {
    gawk_free(info);
}

#endif
