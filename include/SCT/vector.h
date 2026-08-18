#ifndef SCT_VECTOR_H
#define SCT_VECTOR_H

#include "common.h"

#define SCT_VECTOR_ALLOC_SIZE 128

typedef struct {
    u8    *data;
    size_t size;
    size_t cap;
    size_t _item_size;
} sct_vector_t;

void sct_vector_init(sct_vector_t *vec, size_t item_size);

void sct_vector_deinit(sct_vector_t *vec);

void sct_vector_push(sct_vector_t *vec, void *item);

void sct_vector_push_array(sct_vector_t *vec, void *arr, size_t size);

void *sct_vector_get(sct_vector_t *vec, size_t index);

int sct_vector_set(sct_vector_t *vec, size_t index, void *item);

void *sct_vector_pop(sct_vector_t *vec);

void sct_vector_erase(sct_vector_t *vec, size_t index);

void sct_vector_insert(sct_vector_t *vec, size_t index, void *item);

#endif