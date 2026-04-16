#include "flat_vector.h"

void micro_flat_vector_init(micro_flat_vector_t *vec, size_t type_sz, size_t ext_sz)
{
    *vec = (micro_flat_vector_t){
        .data = malloc(ext_sz * type_sz),
        .size = 0,
        .real_size = ext_sz,
        .__type_size = type_sz,
        .__extend_size = ext_sz,
    };
}

void micro_flat_vector_deinit(micro_flat_vector_t *vec)
{
    free(vec->data);
}

void __micro_flat_vector_extend(micro_flat_vector_t *vec)
{
    void *new_data = malloc((vec->real_size += vec->__extend_size) * vec->__type_size);
    for (size_t i = 0; i < vec->size; i++) {
        memcpy(new_data + i * vec->__type_size, vec->data + i * vec->__type_size, vec->__type_size);
    }
    free(vec->data);
    vec->data = new_data;
}

void micro_flat_vector_push(micro_flat_vector_t *vec, void *val)
{
    if (vec->size + 1 >= vec->real_size) {
        __micro_flat_vector_extend(vec);
        return;
    }
    memcpy(vec->data + vec->size * vec->__type_size, val, vec->__type_size);
    vec->size++;
}

void *micro_flat_vector_get(micro_flat_vector_t *vec, size_t index)
{
    return vec->data + index * vec->__type_size;
}