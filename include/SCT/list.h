#ifndef SCT_LIST_H
#define SCT_LIST_H

#include <malloc.h>

#define foreach(iter_name, list_start) for (sct_list_pair_t *(iter_name) = (list_start); (iter_name) && (iter_name)->val; (iter_name) = (iter_name)->next)

typedef struct sct_list_pair_t {
    void *val;
    struct sct_list_pair_t *next;
} sct_list_pair_t;

sct_list_pair_t *sct_list_pair_create(void *val)
{
    sct_list_pair_t *pair = malloc(sizeof(sct_list_pair_t));
    pair->val = val;
    pair->next = 0;
    return pair;
}

size_t sct_list_size(sct_list_pair_t *list_start)
{
    size_t size = 0;
    foreach (cur, list_start) {
        size++;
    }
    return size;
}

void sct_list_push_back(sct_list_pair_t *list_start, void *val)
{
    sct_list_pair_t *cur = list_start;
    while (cur->next && cur->val) {
        cur = cur->next;
    }
    if (!cur->val) {
        cur->val = val;
        return;
    }
    cur->next = sct_list_pair_create(val);
}

void *sct_list_get(sct_list_pair_t *list_start, size_t index)
{
    sct_list_pair_t *cur = list_start;
    size_t i = 0;
    while (i < index && cur->next) {
        cur = cur->next;
        i++;
    }
    return cur->val;
}

void sct_list_set(sct_list_pair_t *list_start, size_t index, void *val)
{
    sct_list_pair_t *el = sct_list_get(list_start, index);
    el->val = val;
}

sct_list_pair_t *sct_list_delete(sct_list_pair_t *list_start, size_t index)
{
    if (index == 0) {
        sct_list_pair_t *acc = list_start->next;
        free(list_start);
        return acc;
    }
    sct_list_pair_t *prev = list_start;
    sct_list_pair_t *cur  = list_start;
    size_t i = 0;
    while (i < index && cur->next) {
        prev = cur;
        cur = cur->next;
        i++;
    }
    prev->next = cur->next;
    free(cur);
    return list_start;
}

sct_list_pair_t *sct_list_full_delete(sct_list_pair_t *list_start, size_t index)
{
    if (index == 0) {
        sct_list_pair_t *acc = list_start->next;
        free(list_start->val);
        free(list_start);
        if (!acc) {
            return sct_list_pair_create(0);
        }
        return acc;
    }
    sct_list_pair_t *prev = list_start;
    sct_list_pair_t *cur  = list_start;
    size_t i = 0;
    while (i < index && cur->next) {
        prev = cur;
        cur = cur->next;
        i++;
    }
    prev->next = cur->next;
    free(cur->val);
    free(cur);
    return list_start;
}

void sct_list_free(sct_list_pair_t *list_start)
{
    if (list_start->next) sct_list_free(list_start->next);
    free(list_start);
}

void sct_list_full_free(sct_list_pair_t *list_start)
{
    if (list_start->next) sct_list_full_free(list_start->next);
    free(list_start->val);
    free(list_start);
}

#endif