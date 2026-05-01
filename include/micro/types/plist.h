#ifndef MICRO_TYPE_PLIST
#define MICRO_TYPE_PLIST

#include "../common.h"

#define foreach(iter_name, list_start) for (sct_list_pair_t *(iter_name) = (list_start); (iter_name) && (iter_name)->val; (iter_name) = (iter_name)->next)
#define it_data_as(it, ty) ((ty)((it)->data))

typedef struct {
    void          *data;
    micro_plist_t *next;
} micro_plist_t;

void micro_plist_init(micro_plist_t *list);

void micro_plist_deinit(micro_plist_t *list, int datafree);

int micro_plist_push(micro_plist_t *list, void *val);

void *micro_plist_get(micro_plist_t *list, size_t index);

size_t micro_plist_size(micro_plist_t *list);

#endif