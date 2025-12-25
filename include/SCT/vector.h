#ifndef VECTOR_H
#define VECTOR_H

#include <malloc.h>

#define genvector(Ty, ExSz)  \
typedef struct {  \
    Ty  *arr;  \
    size_t size;  \
    size_t aloc_size;  \
} vector_##Ty##_t;  \
vector_##Ty##_t *vector_##Ty##_create()  \
{  \
    vector_##Ty##_t *vec = (vector_##Ty##_t*)malloc(sizeof(vector_##Ty##_t));  \
    vec->arr = (Ty*)malloc(sizeof(Ty) * ExSz);  \
    vec->size = 0;  \
    vec->aloc_size = ExSz;  \
    return vec;  \
}  \
void __vector_##Ty##_realoc(vector_##Ty##_t *vec)  \
{  \
    Ty *new_arr = (Ty*)malloc(sizeof(Ty) * (vec->aloc_size += ExSz));  \
    for (size_t i = 0; i < vec->size; i++) {  \
        new_arr[i] = vec->arr[i];  \
    }  \
    free(vec->arr);  \
    vec->arr = new_arr;  \
}  \
void vector_##Ty##_push_back(vector_##Ty##_t *vec, Ty val)  \
{  \
    if (vec->size + 1 > vec->aloc_size) {  \
        __vector_##Ty##_realoc(vec);  \
    }  \
    vec->arr[vec->size++] = val;  \
}  \
void vector_##Ty##_free(vector_##Ty##_t *vec)  \
{  \
    free(vec->arr);  \
    free(vec);  \
}  \

#endif