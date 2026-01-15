#ifndef SCT_VECTOR_H
#define SCT_VECTOR_H

#include <malloc.h>

typedef struct {
    void **arr;
    size_t size;
    size_t real_size;
    size_t _extend_size;
} sct_vector_t;

sct_vector_t *sct_vector_create(size_t extend_size)
{
    sct_vector_t *vec = malloc(sizeof(sct_vector_t));
    vec->_extend_size = extend_size;
    vec->arr = malloc(sizeof(void*) * extend_size);
    vec->real_size = extend_size;
    vec->size = 0;
    return vec;
}

void __sct_vector_extend(sct_vector_t *vec)
{
    void **new_arr = malloc(sizeof(void*) * (vec->real_size += vec->_extend_size));
    for (size_t i = 0; i < vec->size; i++) {
        new_arr[i] = vec->arr[i];
    }
    free(vec->arr);
    vec->arr = new_arr;
}

void sct_vector_push_back(sct_vector_t *vec, void *val)
{
    if (vec->size + 1 >= vec->real_size) {
        __sct_vector_extend(vec);
    }
    vec->arr[vec->size++] = val;
}

void sct_vector_delete(sct_vector_t *vec, size_t index)
{
    for (size_t i = index + 1; i < vec->size; i++) {
        vec->arr[i - 1] = vec->arr[i];
    }
    vec->size--;
}

void sct_vector_free(sct_vector_t *vec)
{
    free(vec->arr);
    free(vec);
}

void sct_vector_full_free(sct_vector_t *vec)
{
    for (size_t i = 0; i < vec->size; i++) {
        free(vec->arr[i]);
    }
    free(vec->arr);
    free(vec);
}

#endif