#ifndef MICRO_TYPE_FLAT_VEC
#define MICRO_TYPE_FLAT_VEC

#include "../common.h"

typedef struct {
    void    *data;
    size_t   size;
    size_t   real_size;
    size_t __type_size;
    size_t __extend_size;
} micro_flat_vector_t;

void micro_flat_vector_init(micro_flat_vector_t *vec, size_t type_sz, size_t ext_sz);

void micro_flat_vector_deinit(micro_flat_vector_t *vec);

void __micro_flat_vector_extend(micro_flat_vector_t *vec);

void micro_flat_vector_push(micro_flat_vector_t *vec, void *val);

void *micro_flat_vector_get(micro_flat_vector_t *vec, size_t index);

#endif