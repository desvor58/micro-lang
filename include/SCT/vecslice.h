#ifndef SCT_VECSLICE_H
#define SCT_VECSLICE_H

#include "vector.h"

typedef struct {
    sct_vector_t *src_vec;
    size_t        start_index;
    size_t        size;
    size_t        _item_size;
} sct_vecslice_t;

int sct_vecslice_init(sct_vecslice_t *slice, sct_vector_t *src_vec, size_t index, size_t size);

void *sct_vecslice_get(sct_vecslice_t *slice, size_t index);

int sct_vecslice_extend(sct_vecslice_t *slice, size_t size);

#endif