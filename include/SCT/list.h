#ifndef SCT_LIST_H
#define SCT_LIST_H

#include <malloc.h>

#define foreach(iter_name, list_start) for (sct_list_pair_t *(iter_name) = (list_start); (iter_name) && (iter_name)->val; (iter_name) = (iter_name)->next)

typedef struct sct_list_pair_t {
    void *val;
    struct sct_list_pair_t *next;
} sct_list_pair_t;

sct_list_pair_t *sct_list_pair_create(void *val);

size_t sct_list_size(sct_list_pair_t *list_start);

void sct_list_push_back(sct_list_pair_t *list_start, void *val);

void *sct_list_get(sct_list_pair_t *list_start, size_t index);

void sct_list_set(sct_list_pair_t *list_start, size_t index, void *val);

sct_list_pair_t *sct_list_delete(sct_list_pair_t *list_start, size_t index);

sct_list_pair_t *sct_list_full_delete(sct_list_pair_t *list_start, size_t index);

void sct_list_free(sct_list_pair_t *list_start);

void sct_list_full_free(sct_list_pair_t *list_start);

#endif