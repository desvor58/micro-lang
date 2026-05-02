#include <micro/types/plist.h>

micro_plist_t *micro_plist_init()
{
    micro_plist_t *list = malloc(sizeof(micro_plist_t));
    *list = (micro_plist_t){
        .data = 0,
        .next = 0,
    };
    return list;
}

void micro_plist_deinit(micro_plist_t *list, int datafree)
{
    if (list->next) {
        micro_plist_deinit(list->next, datafree);
    }
    if (datafree) {
        free(list->data);
    }
    free(list->next);
}

int micro_plist_push(micro_plist_t *list, void *val)
{
    if (!list) {
        return 1;
    }
    micro_plist_t *it = list;
    while (it && it->data) {
        it = it->next;
    }
    if (!it->data) {
        it->data = val;
        return 0;
    }
    it->next = malloc(sizeof(micro_plist_t));
    it->next->data = val;
    return 0;
}

void *micro_plist_get(micro_plist_t *list, size_t index)
{
    if (!list) {
        return 0;
    }
    micro_plist_t *it = list;
    for (size_t i = 0; i < index && it && it->data; i++) {
        it = it->next;
    }
    return it->data;
}

size_t micro_plist_size(micro_plist_t *list)
{
    if (!list) {
        return 0;
    }
    micro_plist_t *it = list;
    size_t i = 0;
    while (it && it->data) {
        it = it->next;
        i++;
    }
    return i;
}
