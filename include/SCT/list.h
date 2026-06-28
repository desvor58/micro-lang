#ifndef SCT_LIST_H
#define SCT_LIST_H

#include "common.h"

#define sct_get_val(pair, T) *(T*)((u8*)pair + sizeof(void*))

#define foreach(list)  \
    for (void *cur_pair = (list)->first_pair, *next_pair = cur_pair ? *(void**)cur_pair : NULL;  \
         cur_pair && *(void**)cur_pair;  \
         cur_pair = next_pair, next_pair = cur_pair ? *(void**)cur_pair : NULL)

typedef struct {
    u8    *first_pair;
    size_t size;
    size_t _item_size;
} sct_list_t;

void sct_list_init(sct_list_t *list, size_t item_size);

void sct_list_deinit(sct_list_t *list);

void sct_list_push(sct_list_t *list, void *item);

void *sct_list_get(sct_list_t *list, size_t index);

void sct_list_erase(sct_list_t *list, size_t index);

#endif