#ifndef LIST_H
#define LIST_H

#include <malloc.h>

#define foreach(LtTy, list) for (LtTy *cur = list; cur && cur->val; cur = cur->next)

#define genlist(Ty)  \
struct list_##Ty##_pair_t {  \
    Ty                        *val;  \
    struct list_##Ty##_pair_t *next;  \
};  \
typedef struct list_##Ty##_pair_t list_##Ty##_pair_t;  \
list_##Ty##_pair_t *list_##Ty##_create()  \
{  \
    list_##Ty##_pair_t *start  \
        = (list_##Ty##_pair_t*)malloc(sizeof(list_##Ty##_pair_t));  \
    start->val  = 0;  \
    start->next = 0;  \
    return start;  \
}  \
int list_##Ty##_add(list_##Ty##_pair_t *start, Ty *val)  \
{  \
    if (start == (list_##Ty##_pair_t*)0) {  \
        return 1;  \
    }  \
    if (start->val == 0) {  \
        start->val = val;  \
        return 0;  \
    }  \
    list_##Ty##_pair_t *cur = start;  \
    while (cur->next != (list_##Ty##_pair_t*)0) {  \
        cur = cur->next;  \
    }  \
    cur->next = list_##Ty##_create();  \
    cur->next->val = val;  \
    return 0;  \
}  \
long long list_##Ty##_size(list_##Ty##_pair_t *start)  \
{  \
    if (start == (list_##Ty##_pair_t*)0) {  \
        return -1;  \
    }  \
    if (start->val == 0) {  \
        return 0;  \
    }  \
    list_##Ty##_pair_t *cur = start;  \
    size_t size = 1;  \
    while (cur->next != (list_##Ty##_pair_t*)0) {  \
        cur = cur->next;  \
        size++;  \
    }  \
    return size;  \
}  \
int list_##Ty##_set(list_##Ty##_pair_t *start, size_t index, Ty *val)  \
{  \
    if (start == (list_##Ty##_pair_t*)0 || index >= list_##Ty##_size(start)) {  \
        return 1;  \
    }  \
    list_##Ty##_pair_t *cur = start;  \
    size_t i = 0;  \
    while (i < index) {  \
        cur = cur->next;  \
        i++;  \
    }  \
    cur->val = val;  \
    return 0;  \
}  \
Ty *list_##Ty##_get(list_##Ty##_pair_t *start, size_t index)  \
{  \
    if (start == (list_##Ty##_pair_t*)0 || index >= list_##Ty##_size(start)) {  \
        return (Ty*)0;  \
    }  \
    list_##Ty##_pair_t *cur = start;  \
    size_t i = 0;  \
    while (i < index) {  \
        cur = cur->next;  \
        i++;  \
    }  \
    return cur->val;  \
}  \
list_##Ty##_pair_t *list_##Ty##_delete(list_##Ty##_pair_t *start, size_t index)  \
{  \
    if (start == (list_##Ty##_pair_t*)0 || index >= list_##Ty##_size(start)) {  \
        return 0;  \
    }  \
    list_##Ty##_pair_t *acc;  \
    if (index == 0) {  \
        acc = start->next;  \
        free(start);  \
        return acc;  \
    }  \
    list_##Ty##_pair_t *cur = start;  \
    size_t i = 0;  \
    while (i < index - 1) {  \
        cur = cur->next;  \
        i++;  \
    }  \
    acc = cur->next->next;  \
    free(cur->next);  \
    cur->next = acc;  \
    return start;  \
}  \
int list_##Ty##_free(list_##Ty##_pair_t *start)  \
{  \
    if (start == (list_##Ty##_pair_t*)0) {  \
        return 1;  \
    }  \
    list_##Ty##_free(start->next);  \
    free(start);  \
    return 0;  \
}  \
int list_##Ty##_full_free(list_##Ty##_pair_t *start)  \
{  \
    if (start == (list_##Ty##_pair_t*)0) {  \
        return 1;  \
    }  \
    foreach (list_##Ty##_pair_t, start) {  \
        free(cur->val);  \
    }  \
    list_##Ty##_free(start->next);  \
    free(start);  \
    return 0;  \
}

#endif