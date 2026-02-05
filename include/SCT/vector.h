#ifndef SCT_VECTOR_H
#define SCT_VECTOR_H

#include <malloc.h>

typedef struct {
    void **arr;
    size_t size;
    size_t real_size;
    size_t _extend_size;
} sct_vector_t;

sct_vector_t *sct_vector_create(size_t extend_size);

void __sct_vector_extend(sct_vector_t *vec);

void sct_vector_push_back(sct_vector_t *vec, void *val);

void sct_vector_delete(sct_vector_t *vec, size_t index);

void sct_vector_free(sct_vector_t *vec);

void sct_vector_full_free(sct_vector_t *vec);

#endif